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
* $FileName: qspi_memory.c$
* $Version : 4.0.2.0$
* $Date    : May-3-2013$
*
* Comments:
*
*   This file contains the functions which write and read the QuadSPI memories
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "qspi_memory.h"

void memory_wait_for_not_busy(MQX_FILE_PTR qspifd)
{
    uint_8 status = 0x01;

    while(status & 0x1) {
        if (memory_read_status1(qspifd, &status) == FALSE)
            status = 0x01;
    }
}

void memory_wait_for_write_en(MQX_FILE_PTR qspifd)
{
    uint_8 status = 0x0;

    while(!(status & 0x2)) {
        if (memory_read_status1(qspifd, &status) == FALSE)
            status = 0x0;
    }
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_set_write_en
* Comments  : This function enable/disable memory write operation
* Note:       QuadSPI_STOP must be send to indicate the end of the instructions.
*
*END*----------------------------------------------------------------------*/
static void memory_set_write_en(MQX_FILE_PTR qspifd, uint_32 addr, boolean enable)
{
    _mqx_int result;
    uint_8 buffer[4+QuadSPI_ADDR_BYTES] = {0};

    result = ioctl(qspifd, QuadSPI_IOCTL_SET_FLASH_ADDR, &addr);
    if (result != MQX_OK) {
        printf("memory_write_data: failed at ioctl set flash address!\n");
        return;
    }

    if (enable) {
        buffer[0] = QuadSPI_WRITE_EN & 0xFF;
        buffer[1] = (QuadSPI_WRITE_EN >> 8) & 0xFF;
    } else {
        buffer[0] = QuadSPI_WRITE_DISABLE & 0xFF;
        buffer[1] = (QuadSPI_WRITE_DISABLE >> 8) & 0xFF;
    }
    buffer[2] = QuadSPI_LOOKUP_STOP & 0xFF;
    buffer[3] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;

    /* Write instruction */
    result = fwrite (buffer, 1, 4 + QuadSPI_ADDR_BYTES, qspifd);
    if (result < 0) {
        printf ("ERROR\n");
        return;
    }
    /* Wait till transfer end (and deactivate CS) */
    fflush (qspifd);

    memory_wait_for_write_en(qspifd);
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_chip_erase
* Comments  : This function erases the whole memory QuadSPI chip
* Note:       QuadSPI_STOP must be send to indicate the end of the instructions.
*
*END*----------------------------------------------------------------------*/
int_32 memory_chip_erase(MQX_FILE_PTR qspifd, uint_32 addr)
{
    _mqx_int result;
    uint_8 buffer[4+QuadSPI_ADDR_BYTES] = {0};

    /* Enable flash memory write */
    memory_set_write_en(qspifd, addr, TRUE);

    /* Send erase command */
    buffer[0] = QuadSPI_CHIP_ERASE & 0xFF;
    buffer[1] = (QuadSPI_CHIP_ERASE >> 8) & 0xFF;
    buffer[2] = QuadSPI_LOOKUP_STOP & 0xFF;
    buffer[3] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;

    /* Write instruction */
    result = fwrite(buffer, 1, 4 + QuadSPI_ADDR_BYTES, qspifd);

    /* Wait till the flash is not busy at program */
    memory_wait_for_not_busy(qspifd);

    printf("Erase chip ... ");
    if (result < 0) {
        printf("ERROR\n");
        return -1;
    }

    printf("QuadSPI Successfully Erase Flash\n");
    return 0;
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_read_status1
* Comments  : This function reads memory status1 register
* Return:
*         Status read.
*
*END*----------------------------------------------------------------------*/
boolean memory_read_status1(MQX_FILE_PTR qspifd, uint_8_ptr status)
{
    _mqx_int result;
    uint_8 temp = *status;
    uint_8 buffer[6 + QuadSPI_ADDR_BYTES] = {0};

    buffer[0] = QuadSPI_READ_STATUS1 & 0xFF;
    buffer[1] = (QuadSPI_READ_STATUS1 >> 8) & 0xFF;
    /* 1 byte status-1*/
    buffer[2] = QuadSPI_READ_DATA(1) & 0xFF;
    buffer[3] = (QuadSPI_READ_DATA(1) >> 8) & 0xFF;
    buffer[4] = QuadSPI_LOOKUP_STOP & 0xFF;
    buffer[5] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;
    /* Write instruction */
    result = fwrite(buffer, 1, 6 + QuadSPI_ADDR_BYTES + 1, qspifd);
    if(result < 0) {
      /* Stop transfer */
        printf("ERROR (tx)\n");
        return FALSE;
    }

    /* Read memory status: 1byte */
    result = fread (status, 1, 1, qspifd);
    if(result < 1) {
        printf ("memory_read_status1: ERROR (rx)\n");
        *status = temp;
        return FALSE;
    } 

    return TRUE;
}

void addr_to_data_buf(uint_32 data_ptr, uint_8_ptr buffer)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        buffer[i] = (uint_8)(data_ptr & 0xFF);
        data_ptr >>= 8;
    }
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_read_byte
* Comments  : This function reads a data byte from memory
* Return:
*         If status read successfully.
*
*END*----------------------------------------------------------------------*/
uint_8 memory_read_byte(MQX_FILE_PTR qspifd, uint_32 addr)
{
    uint_8 buffer[12 + QuadSPI_ADDR_BYTES] = {0}, data;
    int result;

    /* Set as DDR mode */
    ioctl(qspifd, QuadSPI_IOCTL_SET_DDR, &addr);

    result = ioctl(qspifd, QuadSPI_IOCTL_SET_FLASH_ADDR, &addr);
    if (result != MQX_OK) {
        printf("memory_read_byte: failed at ioctl set flash address!\n");
        return 0xFF;
    }

    /* Read instruction, address */
    buffer[0] = QuadSPI_SET_DDR_FAST_RD & 0xFF;
    buffer[1] = (QuadSPI_SET_DDR_FAST_RD >> 8) & 0xFF;
    buffer[2] = QuadSPI_SET_DDR_ADDR & 0xFF;
    buffer[3] = (QuadSPI_SET_DDR_ADDR >> 8) & 0xFF;
    buffer[4] = QuadSPI_SET_MODE_DDR(0xFF) & 0xFF;
    buffer[5] = (QuadSPI_SET_MODE_DDR(0xFF) >> 8) & 0xFF;
    buffer[6] = QuadSPI_DUMMY_DATA(0x02) & 0xFF;
    buffer[7] = (QuadSPI_DUMMY_DATA(0x02) >> 8) & 0xFF;
    buffer[8] = QuadSPI_DDR_READ_DATA(1) & 0xFF;
    buffer[9] = (QuadSPI_DDR_READ_DATA(1) >> 8) & 0xFF;
    buffer[10] = QuadSPI_LOOKUP_STOP & 0xFF;
    buffer[11] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;

    /* Write instruction and address */
    result = fwrite(buffer, 1, 12 + QuadSPI_ADDR_BYTES + 1, qspifd);

    if (result < 0) {
        /* Stop transfer */
        printf("ERROR (tx)\n");
        return 0xFF;
    }

    /* Read data from memory */
    result = fread(&data, 1, 1, qspifd);

    if (result < 0) {
        printf ("ERROR (rx)\n");
        return 0xFF;
    }

    return data;
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_write_data
* Comments  : This function writes data buffer to memory using page write
* Return:
*         Number of bytes written.
*
*END*----------------------------------------------------------------------*/
int_32 memory_write_data (MQX_FILE_PTR qspifd, uint_32 addr, uint_32 size, uint_8_ptr data)
{
    _mqx_int result;
    uint_32 page_size = QuadSPI_MEMORY_PAGE_SIZE;
    uint_32 temp, count = size, write_size;
    uint_8 buffer[8 + QuadSPI_ADDR_BYTES];
    uint_8_ptr src_ptr = (uint_8_ptr)data;

    while (count > 0) {
        temp = addr;
        write_size = (page_size - (temp % page_size));
        write_size = (write_size <= count)?write_size:count;

        memory_set_write_en(qspifd, addr, TRUE);

        /* Write instruction, address and data to buffer */
        buffer[0] = QuadSPI_SET_PAGE_WR & 0xFF;
        buffer[1] = (QuadSPI_SET_PAGE_WR >> 8) & 0xFF;
        buffer[2] = QuadSPI_SET_ADDR & 0xFF;
        buffer[3] = (QuadSPI_SET_ADDR >> 8) & 0xFF;
        buffer[4] = QuadSPI_WRITE_DATA(page_size) & 0xFF;
        buffer[5] = (QuadSPI_WRITE_DATA(page_size) >> 8) & 0xFF;
        buffer[6] = QuadSPI_LOOKUP_STOP & 0xFF;
        buffer[7] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;
        addr_to_data_buf((uint_32) src_ptr, &(buffer[8]));

        result = fwrite (buffer, 1, 8 + QuadSPI_ADDR_BYTES + write_size, qspifd);
        if (result < 0) {
          printf ("ERROR\n");
          return -1;
        }

        /* Wait till the flash is not busy at program */
        memory_wait_for_not_busy(qspifd);

        count -= write_size;
        addr += write_size;
        src_ptr += write_size;
    }

    return size;
}


/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_read_data
* Comments  : This function reads data from memory into given buffer
* Return:
*         Number of bytes read.
*
*END*----------------------------------------------------------------------*/
int_32 memory_read_data (MQX_FILE_PTR qspifd, uint_32 addr, uint_32 size, uint_8_ptr data)
{
    _mqx_int result;
    uint_32 count = size, read_size;
    uint_8 buffer[12 + QuadSPI_ADDR_BYTES] = {0};
    uint_8_ptr src_ptr = data;

    /* Set as DDR mode */
    ioctl(qspifd, QuadSPI_IOCTL_SET_DDR, &addr);

    while (count > 0) {
        read_size = QuadSPI_READ_BUF_SIZE;
        read_size = (read_size <= count)?read_size:count;

        /* Read instruction, address */
        result = ioctl(qspifd, QuadSPI_IOCTL_SET_FLASH_ADDR, &addr);
        if (result != MQX_OK) {
            printf("memory_read_data: failed at ioctl set flash address!\n");
            return -1;
        }

        /* Read instruction, address */
        buffer[0] = QuadSPI_SET_DDR_FAST_RD & 0xFF;
        buffer[1] = (QuadSPI_SET_DDR_FAST_RD >> 8) & 0xFF;
        buffer[2] = QuadSPI_SET_DDR_ADDR & 0xFF;
        buffer[3] = (QuadSPI_SET_DDR_ADDR >> 8) & 0xFF;
        buffer[4] = QuadSPI_SET_MODE_DDR(0xFF) & 0xFF;
        buffer[5] = (QuadSPI_SET_MODE_DDR(0xFF) >> 8) & 0xFF;
        buffer[6] = QuadSPI_DUMMY_DATA(0x02) & 0xFF;
        buffer[7] = (QuadSPI_DUMMY_DATA(0x02) >> 8) & 0xFF;
        buffer[8] = QuadSPI_DDR_READ_DATA(read_size) & 0xFF;
        buffer[9] = (QuadSPI_DDR_READ_DATA(read_size) >> 8) & 0xFF;
        buffer[10] = QuadSPI_LOOKUP_STOP & 0xFF;
        buffer[11] = (QuadSPI_LOOKUP_STOP >> 8) & 0xFF;

        /* Write instruction and address */
        result = fwrite (buffer, 1, 12 + QuadSPI_ADDR_BYTES + read_size, qspifd);
        if (result < 0) {
            /* Stop transfer */
            printf("ERROR (tx)\n");
            return -1;
        }

        /* Read data from memory */
        result = fread (src_ptr, 1, read_size, qspifd);
        if (result < 0) {
            printf ("ERROR (rx)\n");
            return -1;
        }

        count -= read_size;
        addr += read_size;
        src_ptr += read_size;
    }

    return size;
}
