/**HEADER********************************************************************
*
* Copyright (c) 2012 Freescale Semiconductor;
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
* $FileName: spi_memory.c$
* $Version : 3.8.2.0$
* $Date    : Oct-2-2012$
*
* Comments:
*
*   This file contains the functions which write and read the SPI memories
*   using the SPI driver in polled mode.
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "spi_memory.h"


/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_addr_to_buffer
* Comments  : Fills in given address into buffer in correct byte order
*
*END*----------------------------------------------------------------------*/
static int memory_addr_to_buffer(uint_32 addr, uint_8 *buffer)
{
    int i;

    for (i = SPI_MEMORY_ADDRESS_BYTES; i; i--)
    {
        buffer[i-1] = (uint_8)(addr & 0xFF);
        addr >>= 8;
    }

    return SPI_MEMORY_ADDRESS_BYTES;
}


/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_chip_erase
* Comments  : This function erases the whole memory SPI chip
*
*END*----------------------------------------------------------------------*/
void memory_chip_erase (MQX_FILE_PTR spifd)
{
    _mqx_int result;
    uint_8 buffer[1];

    /* This operation must be write-enabled */
    memory_set_write_latch (spifd, TRUE);

    memory_read_status (spifd);

    printf ("Erase whole memory chip:\n");
    buffer[0] = SPI_MEMORY_CHIP_ERASE;

    /* Write instruction */
    result = fwrite (buffer, 1, 1, spifd);

    /* Wait till transfer end (and deactivate CS) */
    fflush (spifd);

    while (memory_read_status (spifd) & 1)
    {
        _time_delay (1000);
    }

    printf ("Erase chip ... ");
    if (result != 1)
    {
        printf ("ERROR\n");
    }
    else
    {
        printf ("OK\n");
    }
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_set_write_latch
* Comments  : This function sets latch to enable/disable memory write
*             operation
*
*END*----------------------------------------------------------------------*/
void memory_set_write_latch (MQX_FILE_PTR spifd, boolean enable)
{
    _mqx_int result;
    uint_8 buffer[1];

    if (enable)
    {
        printf ("Enable write latch in memory ... ");
        buffer[0] = SPI_MEMORY_WRITE_LATCH_ENABLE;
    } else {
        printf ("Disable write latch in memory ... ");
        buffer[0] = SPI_MEMORY_WRITE_LATCH_DISABLE;
    }

    /* Write instruction */
    result = fwrite (buffer, 1, 1, spifd);

    /* Wait till transfer end (and deactivate CS) */
    fflush (spifd);

    if (result != 1)
    {
        printf ("ERROR\n");
    }
    else
    {
        printf ("OK\n");
    }
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_set_protection
* Comments  : This function sets write protection in memory status register
*
*END*----------------------------------------------------------------------*/
void memory_set_protection (MQX_FILE_PTR spifd, boolean protect)
{
    _mqx_int result, i;
    uint_8 protection;
    uint_8 buffer[2];

    /* Must do it twice to ensure right transitions in protection status register */
    for (i = 0; i < 2; i++)
    {
        /* Each write operation must be enabled in memory */
        memory_set_write_latch (spifd, TRUE);

        memory_read_status (spifd);

        if (protect)
        {
            printf ("Write protect memory ... ");
            protection = 0xFF;
        } else {
            printf ("Write unprotect memory ... ");
            protection = 0x00;
        }

        buffer[0] = SPI_MEMORY_WRITE_STATUS;
        buffer[1] = protection;

        /* Write instruction */
        result = fwrite (buffer, 1, 2, spifd);

        /* Wait till transfer end (and deactivate CS) */
        fflush (spifd);

        if (result != 2)
        {
            printf ("ERROR\n");
        }
        else
        {
            printf ("OK\n");
        }
    }
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_read_status
* Comments  : This function reads memory status register
* Return:
*         Status read.
*
*END*----------------------------------------------------------------------*/
uint_8 memory_read_status (MQX_FILE_PTR spifd)
{
    _mqx_int result;
    uint_8 buffer[1];
    uint_8 state = 0xFF;

    printf ("Read memory status ... ");

    buffer[0] = SPI_MEMORY_READ_STATUS;

    /* Write instruction */
    result = fwrite (buffer, 1, 1, spifd);

    if (result != 1)
    {
      /* Stop transfer */
        printf ("ERROR (tx)\n");
        return state;
    }

    /* Read memory status */
    result = fread (&state, 1, 1, spifd);

    /* Deactivate CS */
    fflush (spifd);

    if (result != 1)
    {
        printf ("ERROR (rx)\n");
    }
    else
    {
        printf ("0x%02x\n", state);
    }

    return state;
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_write_byte
* Comments  : This function writes a data byte to memory
*
*
*END*----------------------------------------------------------------------*/
void memory_write_byte (MQX_FILE_PTR spifd, uint_32 addr, uchar data)
{
    _mqx_int result;
    uint_8 buffer[1 + SPI_MEMORY_ADDRESS_BYTES + 1];

    /* Each write operation must be enabled in memory */
    memory_set_write_latch (spifd, TRUE);

    memory_read_status (spifd);

    printf ("Write byte 0x%02x to location 0x%08x in memory ... ", data, addr);

    /* Write instruction, address and data to buffer */
    buffer[0] = SPI_MEMORY_WRITE_DATA;
    memory_addr_to_buffer(addr, &(buffer[1]));
    buffer[1 + SPI_MEMORY_ADDRESS_BYTES] = data;

    result = fwrite (buffer, 1, 1 + SPI_MEMORY_ADDRESS_BYTES + 1, spifd);

    /* Deactivate CS */
    fflush (spifd);

    if (result != 1 + SPI_MEMORY_ADDRESS_BYTES + 1)
    {
        printf ("ERROR\n");
    }
    else
    {
        printf ("done\n", data);
    }

    /* There is 5 ms internal write cycle needed for memory */
    _time_delay (5);
}

/*FUNCTION*---------------------------------------------------------------
*
* Function Name : memory_read_byte
* Comments  : This function reads a data byte from memory
* Return:
*         Byte read.
*
*END*----------------------------------------------------------------------*/
uint_8 memory_read_byte (MQX_FILE_PTR spifd, uint_32 addr)
{
    _mqx_int result;
    uint_8 buffer[1 + SPI_MEMORY_ADDRESS_BYTES];
    uint_8 data = 0;

    printf ("Read byte from location 0x%08x in memory ... ", addr);

    /* Read instruction, address */
    buffer[0] = SPI_MEMORY_READ_DATA;
    memory_addr_to_buffer(addr, &(buffer[1]));

    /* Write instruction and address */
    result = fwrite (buffer, 1, 1 + SPI_MEMORY_ADDRESS_BYTES, spifd);

    if (result != 1 + SPI_MEMORY_ADDRESS_BYTES)
    {
        /* Stop transfer */
        fflush (spifd);
        printf ("ERROR (tx)\n");
        return data;
    }

    /* Read data from memory */
    result = fread (&data, 1, 1, spifd);

    /* Deactivate CS */
    fflush (spifd);

    if (result != 1)
    {
        printf ("ERROR (rx)\n");
    }
    else
    {
        printf ("0x%02x\n", data);
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
uint_32 memory_write_data (MQX_FILE_PTR spifd, uint_32 addr, uint_32 size, uchar_ptr data)
{
    _mqx_int result;
    uint_32 i, len;
    uint_32 count = size;
    uint_8 buffer[1 + SPI_MEMORY_ADDRESS_BYTES];

    while (count > 0)
    {
        /* Each write operation must be enabled in memory */
        memory_set_write_latch (spifd, TRUE);

        memory_read_status (spifd);

        len = count;
        if (len > SPI_MEMORY_PAGE_SIZE - (addr & (SPI_MEMORY_PAGE_SIZE - 1))) len = SPI_MEMORY_PAGE_SIZE - (addr & (SPI_MEMORY_PAGE_SIZE - 1));
        count -= len;

        printf ("Page write %d bytes to location 0x%08x in memory:\n", len, addr);

        /* Write instruction, address */
        buffer[0] = SPI_MEMORY_WRITE_DATA;
        memory_addr_to_buffer(addr, &(buffer[1]));

        result = fwrite (buffer, 1, 1 + SPI_MEMORY_ADDRESS_BYTES, spifd);

        if (result != 1 + SPI_MEMORY_ADDRESS_BYTES)
        {
            /* Stop transfer */
            fflush (spifd);
            printf ("ERROR (tx cmd)\n");
            return 0;
        }

        /* Write data */
        result = fwrite (data, 1, (long)len, spifd);

        /* Deactivate CS */
        fflush (spifd);

        if (result != len)
        {
            printf ("ERROR (tx data)\n");
            return size - count;
        }

        for (i = 0; i < len; i++)
            printf ("%c", data[i]);
        printf ("\n");

        /* Move to next block */
        addr += len;
        data += len;

        /* There is 5 ms internal write cycle needed for memory */
        _time_delay (5);
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
uint_32 memory_read_data (MQX_FILE_PTR spifd, uint_32 addr, uint_32 size, uchar_ptr data)
{
    uint_32 i;
    _mqx_int result;
    uint_8 buffer[5];

    printf ("Reading %d bytes from location 0x%08x in memory: ", size, addr);

    /* Read instruction, address */
    buffer[0] = SPI_MEMORY_READ_DATA;
    memory_addr_to_buffer(addr, &(buffer[1]));

    result = fwrite (buffer, 1, 1 + SPI_MEMORY_ADDRESS_BYTES, spifd);

    if (result != 1 + SPI_MEMORY_ADDRESS_BYTES)
    {
        /* Stop transfer */
        fflush (spifd);
        printf ("ERROR (tx)\n");
        return 0;
    }

    /* Read size bytes of data */
    result = fread (data, 1, (_mqx_int)size, spifd);

    /* De-assert CS */
    fflush (spifd);

    if (result != size)
    {
        printf ("ERROR (rx)\n");
        return 0;
    }
    else
    {
        printf ("\n");
        for (i = 0; i < result; i++)
            printf ("%c", data[i]);
        printf ("\n");
    }

    return size;
}

