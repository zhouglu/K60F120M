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
* $FileName: main.c$
* $Version : 4.0.2.0$
* $Date    : May-3-2013$
*
* Comments:
*
*   This file contains the source for a simple example of an
*   application that writes and reads the QuadSPI memory using the QuadSPI driver.
*   It's already configured for onboard QSPI flash where available.
*
*END************************************************************************/


#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <qspi.h>
#include "qspi_memory.h"


#if ! BSPCFG_ENABLE_IO_SUBSYSTEM
#error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif


#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
#error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif

#ifndef BSP_QUADSPI_MEMORY_CHANNEL
#error This application requires BSP_QUADSPI_MEMORY_CHANNEL or BSP_QUADSPI_MEMORY_CHANNEL to be defined. Please set it to appropriate QSPI channel number in user_config.h and recompile BSP with this option.
#endif

#if BSP_QUADSPI_MEMORY_CHANNEL == 0
  #if ! BSPCFG_ENABLE_QUADSPI0
    #error This application requires BSPCFG_ENABLE_QUADSPI0 defined non-zero in user_config.h. Please recompile kernel with this option.
  #else
    #define TEST_CHANNEL "qspi0:"
  #endif

#elif BSP_QUADSPI_MEMORY_CHANNEL == 1
  #if ! BSPCFG_ENABLE_QUADSPI1
    #error This application requires BSPCFG_ENABLE_QUADSPI1 defined non-zero in user_config.h. Please recompile kernel with this option.
  #else
    #define TEST_CHANNEL "qspi1:"
  #endif
#endif

extern void main_task (uint_32);

const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
    /* Task Index,   Function,   Stack,  Priority,   Name,   Attributes,          Param, Time Slice */
    { 10L,          main_task,  1500L,  8L,         "Main", MQX_AUTO_START_TASK, 0,     0  },
    { 0 }
};

int_32 cmd_help(int_32 argc, char_ptr argv[]);

/*TASK*-------------------------------------------------------------------
*
* Task Name : main_task
* Comments  :
*
*END*----------------------------------------------------------------------*/
void main_task
   (
      uint_32 dummy
   )
{
    MQX_FILE_PTR           qspifd;
    int_32 ret = 0, i, byte_write, byte_read;
    uint_8_ptr data;
    uint_8 test_data[512];
    TIME_STRUCT start_time, end_time, diff_time;

    printf ("\n-------------- QSPI driver example --------------\n\n");
    printf ("This example application demonstrates usage of QSPI driver.\n");

    /* Open the QSPI driver */
    qspifd = fopen (TEST_CHANNEL, NULL);
    if (qspifd == NULL) {
        printf ("Error opening QSPI driver!\n");
        _time_delay (200L);
        _task_block ();
    }

    /* erase all */
    printf("\n\n************************************************************************\n");
    printf("Erase the first flash chip, for S25FL128S/256S, it might take 30s/60s....\n");
    printf("************************************************************************\n");
    _time_get(&start_time);

    memory_chip_erase(qspifd, FLASH_BASE_ADR);

    _time_get(&end_time);
    _time_diff(&start_time, &end_time, &diff_time);
    printf("\nErase whole flash %ld sec, %ld millisec\n", diff_time.SECONDS, diff_time.MILLISECONDS);
    printf("Finish erase all flash\n");

    printf("\n\n*****************************************\n");
    printf("*** Function Test <memory_read_data> ****\n");
    printf("*****************************************\n");
    printf("From Flash %08x: first 20 btyes\n", TEST_BUFFER1);
    byte_read = memory_read_data(qspifd, TEST_BUFFER1, 20, test_data);
    if (byte_read < 0) {
        printf("memory_read_data failed!\n");
        return;
    }

    for (i = 0; i < 20; i++) {
        printf("0x%02x ", test_data[i]);
    }

    printf("\n\n*****************************************\n");
    printf("*** Function Test <memory_read_byte> ****\n");
    printf("*****************************************\n");
    printf("From Flash %08x: first 20 bytes\n", TEST_BUFFER1);
    for (i = 0; i < 20; i++) {
        printf("0x%02x ", memory_read_byte(qspifd, TEST_BUFFER1 + i));
    }
    printf("\n");

    printf("\n\n*****************************************\n");
    printf("*** Function Test <memory_write_data> ***\n");
    printf("*****************************************\n");
    data = (uint_8_ptr)_mem_alloc_zero(TEST_BUF_SIZE1);
    if (data == NULL) {
        printf("fail to allocate write buffer\n");
        fclose(qspifd);
        return;
    }
    for (i = 0; i < TEST_BUF_SIZE1; i++) {
        data[i] = i % 256;
    }

    _time_get(&start_time);

    byte_write = memory_write_data (qspifd, TEST_BUFFER1, TEST_BUF_SIZE1, data);
    if (byte_write < 0) {
        printf("memory_write_data failed!\n");
        return;
    }

    _time_get(&end_time);
    _time_diff(&start_time, &end_time, &diff_time);
    printf("\ndata = %d, Time spends on Flash write is %ld sec, %ld millisec, rate = %ld kbps\n",
        byte_write, diff_time.SECONDS, diff_time.MILLISECONDS,
        (byte_write)/(diff_time.SECONDS * 1000 + diff_time.MILLISECONDS));

    printf("\n\n*****************************************\n");
    printf("***** Time Test <memory_read_data> ******\n");
    printf("*****************************************\n");
    for (i = 0; i < TEST_BUF_SIZE1; i++) {
        data[i] = 255;
    }

    _time_get(&start_time);

    byte_read = memory_read_data (qspifd, TEST_BUFFER1, TEST_BUF_SIZE1, data);
    if (byte_read < 0) {
        printf("memory_read_data failed!\n");
        return;
    }

    _time_get(&end_time);
    _time_diff(&start_time, &end_time, &diff_time);
    printf("\ndata = %d, Time spends on Flash read is %ld sec, %ld millisec, rate = %ld kbps\n\n",
        byte_write, diff_time.SECONDS, diff_time.MILLISECONDS,
        (byte_write)/(diff_time.SECONDS * 1000 + diff_time.MILLISECONDS));

    printf("memory_read_data read data from %08x: first 20 bytes \n", TEST_BUFFER1);
    for(i = 0; i < 20; i++) {
        printf ("0x%02x ", data[i]);
    }
    printf("\n");

    printf("\n\n*****************************************\n");
    printf("**** Compare Test <memory_read_byte> ****\n");
    printf("*****************************************\n");
    printf("memory_read_byte from %08x: first 20 bytes \n", TEST_BUFFER1);
    for (i = 0; i < 20; i++) {
        printf("0x%02x ", memory_read_byte(qspifd, TEST_BUFFER1 + i));
    }
    printf("\n");

    /* Close the SPI */
    _mem_free(data);
    ret = (uint_32)fclose (qspifd);
    if (ret) {
        printf ("Error closing QSPI, returned: 0x%08x\n", ret);
    }

    printf ("\n-------------- End of example --------------\n\n");

}
