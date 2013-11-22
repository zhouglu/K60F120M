/**HEADER********************************************************************
*
* Copyright (c) 2013 Freescale Semiconductor;
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
* $FileName: quadspi_boot.c$
* $Version : 0.1$
* $Date    : April-23-2013$
*
* Comments:
*
*   This file contains the source for the bootloader_vybrid_qspixip example program.
*
*END************************************************************************/
#include <quadspi_boot.h>
#include <mqx.h>
#include <bsp.h>



#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".conf")))
#elif defined(__ICCARM__)
#pragma location=".conf"
#endif



  #define FLASH_BASE            0x20000000
  #define FLASH_END             0x20040000 //otherwise extend the iram space while copy image onto iram
  #define SCLK 1

/*
#define IMAGE_ENTRY_FUNCTION  0x3f040800
  #define FLASH_BASE            0x3f040000
  #define FLASH_END             0x3f080000
*/
const SFLASH_CONFIGURATION_PARAM quadspi_conf = {
  0,                      /* DQS LoopBack */
  0,                      /* Reserved 1*/
  0,                      /* Reserved 2*/
  0,                      /* Reserved 3*/
  0,                      /* Reserved 4*/
  0,                      /* cs_hold_time */
  0,                      /* cs_setup_time */
  0x400000,                 /* A1 flash size */
  0,                      /* A2 flash size */
  0,                      /* B1 flash size */
  0,                      /* B2 flash size */
 SCLK,                      /* SCLK Freq */
  0,                      /* Reserved 5*/
  1,                      /* Single Mode Flash */
  0,                      /* Port - Only A1 */
  0,                      /* DDR Mode Disable */
  0,                      /* DQS Disable */
  0,                      /* Parallel Mode Disable */
  0,                      /* Port A CS1 */
  0,                      /* Port B CS1 */
  0,                      /* FS Phase */
  0,                      /* FS Delay */
  0,                      /* DDR Sampling */
  /* LUT Programming */
  0x0403,
  0x0818,
  0x1c08,
  0x2400,
};
#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_data")))
#elif defined(__ICCARM__)
#pragma location=".boot_data"
#endif

const BOOT_DATA_T boot_data = {
  FLASH_BASE,                 /* boot start location */
  (FLASH_END-FLASH_BASE),     /* size */
  PLUGIN_FLAG,                /* Plugin flag*/
  0xFFFFFFFF  				  /* empty - extra data word */
};

#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".ivt")))
#elif defined(__ICCARM__)
#pragma location=".ivt"
#endif


const ivt image_vector_table = {
  IVT_HEADER,                         /* IVT Header */
  (uint_32)&__boot,     /* Image  Entry Function */
  IVT_RSVD,                           /* Reserved = 0 */
  (uint_32)DCD_ADDRESS,              /* Address where DCD information is stored */
  (uint_32)BOOT_DATA_ADDRESS,        /* Address where BOOT Data Structure is stored */
  (uint_32)&image_vector_table,      /* Pointer to IVT Self (absolute address */
  (uint_32)CSF_ADDRESS,              /* Address where CSF file is stored */
  IVT_RSVD                            /* Reserved = 0 */
};

#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".dcd")))
#elif defined(__ICCARM__)
#pragma location=".dcd"
#endif

const uint_32 device_config_data[DCD_ARRAY_SIZE] = {
  /* DCD Header */
  (uint_32)(DCD_TAG_HEADER |                       \
    (0x2000<< 8) | \
    (DCD_VERSION << 24)),
  /* DCD Commands */
  /* Add additional DCD Commands here */
};
const uint_8 fill_data[0x3CC]; //dc - fill data so ds-5 parser aligns correctly Fill 0x434-0x800.
