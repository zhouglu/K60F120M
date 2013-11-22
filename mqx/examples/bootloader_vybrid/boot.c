/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: boot.c$
* $Version : 3.8.11.0$
* $Date    : Oct-3-2012$
*
* Comments:
*
*   This file contains the source for the bootloader_vybrid example program.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <mfs.h>
#include <fio.h>
#include <sdcard.h>
#include <string.h>
#include <part_mgr.h>
#include "boot_ivt_header.h"
#include "iniparser.h"
#include "md5.h"


#define MD5_LENGTH          (16)
#define IMAGES_MAX_COUNT    (2)
#define SDCARD_COM_CHANNEL  BSP_SDCARD_ESDHC_CHANNEL
#define BUFFER_LENGTH       (64)
#define LINE_LENGTH         (INIPARSER_LENGTH_LINE)
#define VALUE_LENGTH        (INIPARSER_LENGTH_PROPERTY_VALUE)
#define IMAGES_MAX_COUNT    (2)
#define BOOT_CORETYPE_A5    (0x43413500)
#define BOOT_CORETYPE_M4    (0x434D3400)

#define BOOT_SECTION_BASE 0x3f040000
#define BOOT_SECTION_END  0x3f06fff0

#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_data")))
#elif defined(__ICCARM__)
#pragma location=".boot_data"
#endif
const BOOT_DATA_T boot_data = {
  BOOT_SECTION_BASE,                 /* boot start location */
  (BOOT_SECTION_END - BOOT_SECTION_BASE),     /* size */
  PLUGIN_FLAG,                /* Plugin flag*/
  0xFFFFFFFF                    /* empty - extra data word */
};

#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".ivt")))
#elif defined(__ICCARM__)
#pragma location=".ivt"
#else
    #error "Unsupported compiler"
#endif
const ivt image_vector_table = {
  IVT_HEADER,                         /* IVT Header */
  (uint32_t)&__boot,     /* Image  Entry Function */
  IVT_RSVD,                           /* Reserved = 0 */
  (uint32_t)DCD_ADDRESS,              /* Address where DCD information is stored */
  (uint32_t)&boot_data,        /* Address where BOOT Data Structure is stored */
  (uint32_t)&image_vector_table,      /* Pointer to IVT Self (absolute address */
  (uint32_t)CSF_ADDRESS,              /* Address where CSF file is stored */
  IVT_RSVD                            /* Reserved = 0 */
};

#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".dcd")))
#elif defined(__ICCARM__)
#pragma location=".dcd"
#else
    #error "Unsupported compiler"
#endif
const uint32_t device_config_data[DCD_ARRAY_SIZE] = {
  /* DCD Header */
  (uint32_t)(DCD_TAG_HEADER |                       \
    (0x2000<< 8) | \
    (DCD_VERSION << 24)),
  /* DCD Commands */
  /* Add additional DCD Commands here */
};

typedef void (*fptr_void_t)(void);
typedef uint_8 md5digest_t[MD5_LENGTH];
char partman_name[] = "pm:";
char filesystem_name[] = "c:/";

uint_32 single_core;

MQX_FILE_PTR 
    com_handle = NULL,
    sdcard_handle = NULL, mfs_handle = NULL,
    partman_handle = NULL,
    file_handle = NULL,
    filesystem_handle = NULL;


/* Function prototypes */
void found_label(const char_ptr);
void found_property(const char_ptr, const char_ptr);
uint_32 copy_image(uint_32, const char_ptr);
void md5_of_memory(uint_32, uint_32, md5digest_t *);
void verify_parser(void);
uint_32 mount(void);
void main_task(uint_32);
void boot_process(void);
void boot_from_reserved(void);


const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
    /* Task Index, Function, Stack, Priority,   Name,          Attributes, Param, Time Slice */
    { 1,          main_task,  2000,        8, "main", MQX_AUTO_START_TASK,     0,          0 },
    { 0 }
};


typedef enum {
    PERFORM_TYPE_RUN    = 1,
    PERFORM_TYPE_LOAD   = 2,
} PERFORM_TYPE;


typedef enum {
    FLAGS_TYPE_RECOGNIZED_LABEL = 0x01,
    FLAGS_TYPE_RECOGNIZED_PATH  = 0x02,
    FLAGS_TYPE_RECOGNIZED_BASE  = 0x04,
    FLAGS_TYPE_RECOGNIZED_CORE  = 0x08,
    FLAGS_TYPE_RECOGNIZED_MD5   = 0x10,
    FLAGS_TYPE_PERFORM_RUN      = 0x20,
    FLAGS_TYPE_PERFORM_LOAD     = 0x40,
} FLAGS_TYPE;


typedef struct {
    /* memory address to load image */
    uint_32             base;
    /* coreid */
    uint_32             core;
    /* image size in B */
    uint_32             size;
    /* md5 hash */
    md5digest_t         md5;
    /* flags */
    FLAGS_TYPE          flags;
    /* image label of setup.ini */
    char                label[VALUE_LENGTH];
    /* path to .bin file */
    char                path[VALUE_LENGTH];
} img_info_struct;


typedef struct {
    /* number of boot_info records */
    uint32_t            count;
    /* primary coreid */
    uint32_t            index;
    /* list of boot_info records */
    img_info_struct     images[IMAGES_MAX_COUNT];
} img_context_struct;


typedef struct
{
    /* image base address */
    uint_32             base;
    /* coreid */
    uint_32             core;
    /* run flags */
    FLAGS_TYPE          flags;
} boot_info_struct;


typedef struct {
    /* number of boot_info records */
    uint32_t            count;
    /* primary coreid */
    uint32_t            primary;
    /* list of boot_info records */
    boot_info_struct    images[IMAGES_MAX_COUNT];
} boot_context_struct;

img_context_struct img_context;
extern boot_context_struct boot_context;


typedef enum {
    INI_STATE_START = 0,
    INI_STATE_BOOT,
    INI_STATE_IMAGE,
    INI_STATE_IMAGEPROP,
} INI_STATE;


INI_STATE ini_state = INI_STATE_START;


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : found_label
* Returned Value : None
* Comments       :
*     "label" callback, initialization of img_context structure
*END*--------------------------------------------------------------------*/

void found_label
(
    /* [IN] property name */
    const char_ptr name
)
{
    switch (ini_state)
    {
        case INI_STATE_START:
            if (strcmp(name, "boot"))
            {
                printf("Error : first label is not [boot] \n");
                _task_block();
            }
            ini_state = INI_STATE_BOOT;
        break;
        case INI_STATE_BOOT:
        case INI_STATE_IMAGEPROP:
            ini_state = INI_STATE_IMAGE;
        case INI_STATE_IMAGE:
            for (img_context.index = 0; img_context.index < img_context.count; img_context.index++)
            {
                if (!strcmp(img_context.images[img_context.index].label, name))
                {
                    img_context.images[img_context.index].flags |= FLAGS_TYPE_RECOGNIZED_LABEL;
                    ini_state = INI_STATE_IMAGEPROP;
                    break;
                }
            }
        break;
    }
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : found_property
* Returned Value : None
* Comments       :
*     "property" callback, initialization of img_context structure
*END*--------------------------------------------------------------------*/

void found_property
(
    /* [IN] property name */
    const char_ptr name,
    /* [IN] value */
    const char_ptr value
)
{
    img_info_struct * image = NULL;
    uint32_t convert[4] = {0};
    uint32_t i = 0;
    
    switch (ini_state)
    {
        case INI_STATE_BOOT:
            image = &img_context.images[img_context.count];
            /* property of [boot] label */
            if (!strcmp(name, "run"))
            {
                if (img_context.count < IMAGES_MAX_COUNT)
                {
                    strcpy(image->label, value);
                    image->flags |= FLAGS_TYPE_PERFORM_RUN;
                    img_context.count += 1;
                    break;
                }
            }
            if (!strcmp(name, "load"))
            {
                if (img_context.count < IMAGES_MAX_COUNT)
                {
                    strcpy(image->label, value);
                    image->flags |= FLAGS_TYPE_PERFORM_LOAD;
                    img_context.count += 1;
                    break;
                }
            }
        break;
        case INI_STATE_IMAGEPROP:
            image = &img_context.images[img_context.index];
            if (!strcmp(name, "base"))
            {
                sscanf(value, "0x%x", &image->base);
                image->flags |= FLAGS_TYPE_RECOGNIZED_BASE;
                break;
            }
            if (!strcmp(name, "core"))
            {
                sscanf(value, "%d", &image->core);
                image->flags |= FLAGS_TYPE_RECOGNIZED_CORE;
                break;
            }
            if (!strcmp(name, "path"))
            {
                strcpy(image->path, value);
                image->flags |= FLAGS_TYPE_RECOGNIZED_PATH;
                break;
            }
            if (!strcmp(name, "md5"))
            {
                sscanf(value, "%8x%8x%8x%8x", &convert[0], &convert[1], &convert[2], &convert[3]);
                for (i = 0; i < 4; i++)
                {
                    image->md5[(4 * i) + 0] = (uint_8)((convert[i] >> 24) & 0xff);    
                    image->md5[(4 * i) + 1] = (uint_8)((convert[i] >> 16) & 0xff);
                    image->md5[(4 * i) + 2] = (uint_8)((convert[i] >> 8) & 0xff);
                    image->md5[(4 * i) + 3] = (uint_8)(convert[i] & 0xff);
                }
                image->flags |= FLAGS_TYPE_RECOGNIZED_MD5;
                break;
            }
        break;
    }
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : copy_image
* Returned Value : number of copied bytes
* Comments       :
*     copy file content to memory, starting at "base_addr"
*END*--------------------------------------------------------------------*/

uint_32 copy_image
(
    /* [IN] memory base address */
    uint_32 base_addr,
    /* [IN] image filename */
    const char_ptr filename
)
{
    MQX_FILE_PTR file_handle = NULL;
    uint_8 buffer[BUFFER_LENGTH], * target_addr = NULL;
    uint_32 buffer_length;
    uint_32 copied = 0;
    
    /* open binary file, return 0 if failed */
    file_handle = fopen(filename, "r+");
    if (NULL == file_handle)
    {
        return 0;
    }
    
    /* copy content to memory */
    target_addr = (uint_8_ptr)base_addr;
    while ((buffer_length = read(file_handle, buffer, BUFFER_LENGTH)) > 0)
    {
        _mem_copy(buffer, target_addr, buffer_length);
        target_addr += buffer_length;
        copied += buffer_length;
    }
    
    _DCACHE_FLUSH_MLINES((uint_8 *)base_addr, copied);
    fclose(file_handle);
    return copied;
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : md5_of_memory
* Returned Value : none
* Comments       :
*     calculate memory MD5 hash, start at "base_address" and "size" bytes
*END*--------------------------------------------------------------------*/

void md5_of_memory
(
    /* [IN] memory base address */
    uint_32 base_addr,
    /* [IN] size in bytes */
    uint_32 size,
    /* [OUT] ptr to md5 hash */
    md5digest_t * md5
)
{
    md5_state_t state;
    uint_8_ptr text = (uint_8_ptr)base_addr;
    md5_init(&state);

    // append memory blocks
    for (; size > BUFFER_LENGTH; size -= BUFFER_LENGTH)
    {
        md5_append(&state, (const md5_byte_t *)text, BUFFER_LENGTH);
        text += BUFFER_LENGTH;
    }

    // append remains bytes
    if (size)
    {
        md5_append(&state, (const md5_byte_t *)text, size);
    }
    md5_finish(&state, *md5);
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : verify_parser
* Returned Value : none
* Comments       :
*     verify & stop program if img_context doesn't contain all 
*     necessary informations
*END*--------------------------------------------------------------------*/

void verify_parser(void)
{
    uint_32 i;
    img_info_struct * image = NULL;

    /* check whether all required parts were parsed */
    for (i = 0; i < img_context.count; i++)
    {
        image = &img_context.images[i];
        if (!(image->flags & FLAGS_TYPE_RECOGNIZED_LABEL))
        {
            printf("Error : missing label [%s] \n", image->label);
            _task_block();
        }
        if (!(image->flags & FLAGS_TYPE_RECOGNIZED_PATH))
        {
            printf("Error : missing 'path' property of label [%s] \n", image->label);
            _task_block();
        }
        if (!(image->flags & FLAGS_TYPE_RECOGNIZED_CORE))
        {
            printf("Error : missing 'core' property of label [%s] \n", image->label);
            _task_block();
        }
        if (!(image->flags & FLAGS_TYPE_RECOGNIZED_BASE))
        {
            printf("Error : missing 'base' property of label [%s] \n", image->label);
            _task_block();
        }
    }
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : mount
* Returned Value : none
* Comments       :
*     mount sdcard, partition, filesystem, .. to 'c:/' drive
*END*--------------------------------------------------------------------*/

uint_32 mount(void)
{
    uint_32 result = MQX_OK;
    
    com_handle = fopen(SDCARD_COM_CHANNEL, NULL);
    if (com_handle == NULL)
    {
        printf("Error : unable to open sdhc driver \n");
        _task_block();
    }
    
    result = _io_sdcard_install("sdcard:", (pointer)&_bsp_sdcard0_init, com_handle);
    if (result != MQX_OK)
    {
        printf("Error : unable to install sdcard driver \n");
        _task_block();
    }
    
    _time_delay (200);
    
    sdcard_handle = fopen("sdcard:", 0);
    if (sdcard_handle == NULL)
    {
        printf("Error : unable to open sdcard driver \n");
        _task_block();
    }
    
    result = _io_part_mgr_install(sdcard_handle, partman_name, 0);
    if (result != MQX_OK)
    {
        printf("Error : unable to install partition manager \n");
        _task_block();
    }
    
    partman_handle = fopen(partman_name, NULL);
    if (partman_handle == NULL)
    {
        printf("Error : unable to open partition manager \n");
        _task_block();
    }
    
    result = _io_mfs_install(partman_handle, filesystem_name, 1);
    if (result != MQX_OK)
    {
        printf("Error : unable to install MFS driver \n");
        _task_block();
    }
    
    filesystem_handle = fopen(filesystem_name, NULL);
    if (partman_handle == NULL)
    {
        printf("Error : unable to open MFS driver \n");
        _task_block();
    }
    
    return MQX_OK;
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : main_task
* Returned Value : none
* Comments       :
*     main_task function
*END*--------------------------------------------------------------------*/

void main_task
(
    uint_32 initial_data
)
{
    volatile uint_32 cpu_count = (*(uint_32_ptr)MSCM_CPxCOUNT);
    printf("Bootloader from ");
#if PSP_MQX_CPU_IS_VYBRID_A5
    printf("A5");
#elif PSP_MQX_CPU_IS_VYBRID_M4
    printf("M4");
#endif
    if (cpu_count & 0x1) {
        printf(" as primary core, dual core\n");
        single_core = 0;
    } else {
        printf(" single core\n");
        single_core = 1;
    }
    printf("======================\n");
    printf("Mounting filesystem \n");
    mount();
    printf("Starting bootloader \n");
    boot_process();
}


/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : boot_process
* Returned Value : none
* Comments       :
*     prepare img_context and call boot function
*END*--------------------------------------------------------------------*/

void boot_process(void)
{
    uint_32 i = 0;
    md5digest_t md5;
    img_info_struct * image = NULL;
#if PSP_MQX_CPU_IS_VYBRID_A5
    char *fname = "c:/setup_a5.ini";
#elif PSP_MQX_CPU_IS_VYBRID_M4
    char *fname = "c:/setup_m4.ini";
#endif
    /* call setup.ini parser */
    if (MQX_OK != iniparser_load(fname, &found_label, &found_property))
    {
        printf("Error : cannot open '%s' \n", fname);
        _task_block();
    }
    
    /* verify result of parser process */
    verify_parser();
    
    /* load & verify */
    for (i = 0; i < img_context.count; i++)
    {
        image = &img_context.images[i];

        /* skip image copying for secondary core, if it's a single core chip */
        if (single_core && (image->core != 0x0)) {
            image->flags &= ~(FLAGS_TYPE_PERFORM_RUN);
            printf("*** Skip image \"%s\" for secondary core.\n", image->path);
            continue;
        }


        /* copy binary images */
        image->size = copy_image(image->base, image->path);
        if (0 == image->size)
        {
            printf("Error : cannot load image %s \n", image->path);
            _task_block();
        }

        /* verify image */
        if (image->flags & FLAGS_TYPE_RECOGNIZED_MD5)
        {
            md5_of_memory(image->base, image->size, &md5);
            if (memcmp(image->md5, md5, MD5_LENGTH))
            {
                printf("Error : image verify fail %s \n", image->path);
                _task_block();
            }
        }
    }
    
    /* set primary core number */
    boot_context.primary = (*(uint_32_ptr)MSCM_CPxNUM);
    boot_context.primary &= 0x1;
    boot_context.count = img_context.count;
    
    /* copy necessary data from img_context to boot_context */
    for (i = 0; i < img_context.count; i++)
    {
        boot_context.images[i].base = img_context.images[i].base;
        boot_context.images[i].core = img_context.images[i].core;
        boot_context.images[i].flags = img_context.images[i].flags;
    }

#if PSP_MQX_CPU_IS_VYBRID_A5
    /* disable cache and MMU before overwriting MMU table */
    _ICACHE_DISABLE();
    _DCACHE_DISABLE();
    _mmu_vdisable();
#endif

    /* try to boot from .boot_reserved segment */
    boot_from_reserved();
}


#if defined(__ICCARM__)
    #pragma language = extended
    #pragma location = ".boot_reserved_data"
    #pragma segment  = ".boot_reserved_data"
#elif defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_reserved_data")))
#else
    #error "Unsupported compiler"
#endif
boot_context_struct boot_context = {0};


#if defined(__ICCARM__)
    #pragma language = extended
    #pragma location = ".boot_reserved_data"
    #pragma segment  = ".boot_reserved_data"
#elif defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_reserved_data")))
#else
    #error "Unsupported compiler"
#endif
static uint_32 i = 0;

#if   defined(__ICCARM__)
    #pragma language = extended
    #pragma location = ".boot_reserved_code"
    #pragma segment  = ".boot_reserved_code"
#elif defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_reserved_code")))
#else
    #error "Unsupported compiler"
#endif
void boot_from_reserved (void)
{
    /* from this point stack cannot be used */
    /* images cannot use reserved memmory .boot_reserved ! */
    
#if PSP_MQX_CPU_IS_VYBRID_A5
    __asm("cpsid ifa");
#elif PSP_MQX_CPU_IS_VYBRID_M4
    __asm("cpsid i");
#endif
    
    /* try to find and boot auxiliary core */
    for (i = 0; i < boot_context.count; i++)
    {
        if (
            (boot_context.images[i].core != boot_context.primary) &&
            (boot_context.images[i].flags & FLAGS_TYPE_PERFORM_RUN)
        )
        {
#if PSP_MQX_CPU_IS_VYBRID_A5
            // reset vector
            SRC_GPR(2) = ((_mqx_uint*)boot_context.images[i].base)[1];
            // stack pointer
            SRC_GPR(3) = ((_mqx_uint*)boot_context.images[i].base)[0];
#elif PSP_MQX_CPU_IS_VYBRID_M4
            SRC_GPR(2) = boot_context.images[i].base;
#endif
            CCM_CCOWR = 0x15a5a;
            break;
        }
    }

    /* try to find and boot primary core */
    for (i = 0; i < boot_context.count; i++)
    {
        if (
            (boot_context.images[i].core == boot_context.primary) &&
            (boot_context.images[i].flags & FLAGS_TYPE_PERFORM_RUN)
        )
        {
#if PSP_MQX_CPU_IS_VYBRID_A5
            (*((fptr_void_t)(boot_context.images[i].base)))();
#elif PSP_MQX_CPU_IS_VYBRID_M4
#if defined(__CC_ARM)
            __asm
            {
                msr     MSP, ((_mqx_uint*)boot_context.images[i].base)[0];
            }
#else
            __asm("msr     MSP, %0" : : "r" (((_mqx_uint*)boot_context.images[i].base)[0]));
#endif
            (*((fptr_void_t)(((_mqx_uint*)boot_context.images[i].base)[1])))();
#endif
            break;
        }
    }

    /* primary core is not used, go to infinite loop */
    while (1);
}

