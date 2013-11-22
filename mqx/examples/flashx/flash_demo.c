/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
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
* $FileName: flash_demo.c$
* $Version : 3.8.32.0$
* $Date    : Jun-6-2012$
*
* Comments:
*
*   This file contains the source for the FlashX example program.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <string.h>

#if (BSP_M51EMDEMO || BSP_TWRMCF51MM || BSP_TWRMCF51JE)
    #if !BSPCFG_ENABLE_FLASHX2
    #error This application requires BSPCFG_ENABLE_FLASHX1 defined non-zero in user_config.h. Please recompile BSP with this option.
    #endif
    #define         FLASH_NAME "flashx2:bank1"
#elif BSP_M54455EVB
    #if !BSPCFG_ENABLE_FLASHX0
    #error This application requires BSPCFG_ENABLE_FLASHX0 defined non-zero in user_config.h. Please recompile BSP with this option.
    #endif
    #define         FLASH_NAME "flashx0:bank0"
#else
    #if !BSPCFG_ENABLE_FLASHX
    #error This application requires BSPCFG_ENABLE_FLASHX defined non-zero in user_config.h. Please recompile BSP with this option.
    #endif
    #define         FLASH_NAME "flashx:bank0"
#endif

#define STRING_SIZE     (32)
#define BUFFER_SIZE     (80)

/* function prototypes */
void flash_task(uint_32);

static char_ptr buffer;

const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
   /* Task Index,   Function,          Stack,  Priority,   Name,           Attributes,          Param, Time Slice */
    {          1,   flash_task,   2000,   7,          "flash_test",   MQX_AUTO_START_TASK, 0,     0 },
    { 0 }
};

/*TASK*-----------------------------------------------------
*
* Task Name    : flash_task
* Comments     :
*    This task finds a string at 'the end' of flash memory
*    then it tries to rewrite it. The successfull rewrite
*    can be proved by re-executing this example application.
*
*END*-----------------------------------------------------*/
void flash_task
   (
      uint_32 initial_data
   )
{
   MQX_FILE_PTR   flash_file;
   _mqx_int       i;
   _mqx_int       len = 0;
   uint_32        ioctl_param;

   _int_install_unexpected_isr();

   printf("\n\nMQX Flash Example");

   buffer = (char_ptr)_mem_alloc_zero(BUFFER_SIZE);
   if (buffer == NULL) {
      printf("\nCan't allocate buffer");
      _task_block();
   }

   /* Open the flash device */
   flash_file = fopen(FLASH_NAME, NULL);
   if (flash_file == NULL) {
      printf("\nUnable to open file %s", FLASH_NAME);
      _task_block();
   } else {
      printf("\nFlash file %s opened", FLASH_NAME);
   }

   /* Get the size of the flash file */
   fseek(flash_file, 0, IO_SEEK_END);
   printf("\nSize of the flash file: 0x%x Bytes", ftell(flash_file));

   /* Disable sector cache */
   ioctl(flash_file, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
   printf("\nFlash sector cache enabled.");

   /* Move STRING_SIZE Bytes back */
   printf("\nReading last %d Bytes.", STRING_SIZE);
   fseek(flash_file, -STRING_SIZE, IO_SEEK_END);

   len = read(flash_file, buffer, STRING_SIZE);
   if (STRING_SIZE != len) {
      printf("\nERROR! Could not read from flash. Exiting...");
      goto example_end;
   }
   for (i = 0; i < STRING_SIZE; i++) {
      if (buffer[i] != (char)0xFF)
         break;
   }
   if (i == STRING_SIZE) {
      printf("\nBytes are blank.");
   }
   else {
      while ((buffer[i] != 0) && (i < STRING_SIZE)) {
         i++;
      }

      if (i == STRING_SIZE) {
         printf("\nFound non-blank data, but not zero-ended string.");
      }
      else {
         printf("\nString found: %s", buffer);
      }
   }

   do {
      printf("\nType a string to be written to the end of file (%d chars max.):", STRING_SIZE-1);
      fgets(buffer, BUFFER_SIZE, stdin);
      len = strlen(buffer);
   } while (!len || len > STRING_SIZE-1);

   /* Move STRING_SIZE Bytes back */
   fseek(flash_file, -STRING_SIZE, IO_SEEK_END);

   /* Unprotecting the the FLASH might be required */
   ioctl_param = 0;
   ioctl(flash_file, FLASH_IOCTL_WRITE_PROTECT, &ioctl_param);

   len = write(flash_file, buffer, STRING_SIZE);
   if (len != STRING_SIZE) {
      printf("\nError writing to the file. Error code: %d", _io_ferror(flash_file));
   }
   else {
      printf("\nData written to the flash.\nNow you can power down and power up your device");
      printf("\nand then retry the test to see if the string was written correctly.");
   }

example_end:
   fclose(flash_file);
   printf("\nFlash example finished.");

   _task_block();
}

/* EOF */
