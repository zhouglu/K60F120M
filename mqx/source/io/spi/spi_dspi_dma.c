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
* Comments:
*
*   The file contains low level SPI driver functions for DSPI module
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>

#include "spi.h"
#include "spi_prv.h"

#include "spi_dspi_common.h"
#include "spi_dspi_dma.h"
#include "spi_dspi_dma_prv.h"

#include "dma.h"


/* SPI low level driver interface functions */
static _mqx_int _dspi_dma_init(const void _PTR_ init_data_ptr, pointer *io_info_ptr_ptr);
static _mqx_int _dspi_dma_deinit(pointer io_info_ptr);
static _mqx_int _dspi_dma_setparam(pointer io_info_ptr, SPI_PARAM_STRUCT_PTR params);
static _mqx_int _dspi_dma_tx_rx(pointer io_info_ptr, uint_8_ptr txbuf, uint_8_ptr rxbuf, uint_32 len);
static _mqx_int _dspi_dma_cs_deassert(pointer io_info_ptr);
static _mqx_int _dspi_dma_ioctl(pointer io_info_ptr, SPI_PARAM_STRUCT_PTR params, uint_32 cmd, uint_32_ptr param_ptr);

const SPI_DEVIF_STRUCT _spi_dspi_dma_devif = {
    _dspi_dma_init,
    _dspi_dma_deinit,
    _dspi_dma_setparam,
    _dspi_dma_tx_rx,
    _dspi_dma_cs_deassert,
    _dspi_dma_ioctl
};


/* Forward declarations */
static void _dspi_dma_callback(pointer parameter);


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_init
* Returned Value   : MQX error code
* Comments         :
*    This function initializes the SPI driver
*
*END*********************************************************************/
static _mqx_int _dspi_dma_init
    (
        /* [IN] The initialization information for the device being opened */
        const void _PTR_               init_data_ptr,

        /* [OUT] The address to store device specific information */
        pointer                        *io_info_ptr_ptr
    )
{
    DSPI_DMA_INIT_STRUCT_PTR           dspi_init_ptr = (DSPI_DMA_INIT_STRUCT_PTR)init_data_ptr;

    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr;

    const uint_32 _PTR_                vectors;
    uint_32                            i;

    #if PSP_HAS_DEVICE_PROTECTION
    if (!_bsp_dspi_enable_access(dspi_init_ptr->CHANNEL)) {
        return SPI_ERROR_CHANNEL_INVALID;
    }
    #endif

    /* Check channel */
    dspi_ptr = _bsp_get_dspi_base_address (dspi_init_ptr->CHANNEL);
    if (NULL == dspi_ptr)
    {
        return SPI_ERROR_CHANNEL_INVALID;
    }

    if (_bsp_dspi_io_init (dspi_init_ptr->CHANNEL) == -1)
    {
        return SPI_ERROR_CHANNEL_INVALID;
    }

    /* Initialize internal data */
    dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)_mem_alloc_system_zero((uint_32)sizeof(DSPI_DMA_INFO_STRUCT));
    if (dspi_info_ptr == NULL)
    {
        return MQX_OUT_OF_MEMORY;
    }
    _mem_set_type(dspi_info_ptr, MEM_TYPE_IO_SPI_INFO_STRUCT);

    *io_info_ptr_ptr = (pointer)dspi_info_ptr;

    dspi_info_ptr->DSPI_PTR = dspi_ptr;
    dspi_info_ptr->CHANNEL = dspi_init_ptr->CHANNEL;
    dspi_info_ptr->CLOCK_SOURCE = dspi_init_ptr->CLOCK_SOURCE;

    _dspi_init_low(dspi_info_ptr->DSPI_PTR);

    _lwsem_create(&dspi_info_ptr->EVENT_IO_FINISHED, 0);

    /* Configure DMA */
    dma_channel_setup(dspi_init_ptr->DMA_RX_CHANNEL, dspi_init_ptr->DMA_RX_SOURCE);
    dma_channel_setup(dspi_init_ptr->DMA_TX_CHANNEL, dspi_init_ptr->DMA_TX_SOURCE);
    dspi_info_ptr->DMA_RX_CHANNEL = dspi_init_ptr->DMA_RX_CHANNEL;
    dspi_info_ptr->DMA_TX_CHANNEL = dspi_init_ptr->DMA_TX_CHANNEL;

    /* Allocate cache line aligned block of memory and split it in half to form RX and TX buffer */
    dspi_info_ptr->RX_BUF = _mem_alloc_system(4*PSP_CACHE_LINE_SIZE);
    dspi_info_ptr->TX_BUF = dspi_info_ptr->RX_BUF + 2*PSP_CACHE_LINE_SIZE;

    dma_callback_reg(dspi_info_ptr->DMA_RX_CHANNEL, _dspi_dma_callback, dspi_info_ptr);

    /* Route data requests to DMA */
    dspi_ptr->RSER = DSPI_RSER_RFDF_DIRS_MASK | DSPI_RSER_RFDF_RE_MASK | DSPI_RSER_TFFF_DIRS_MASK | DSPI_RSER_TFFF_RE_MASK;

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_deinit
* Returned Value   : MQX error code
* Comments         :
*    This function de-initializes the SPI module
*
*END*********************************************************************/
static _mqx_int _dspi_dma_deinit
    (
        /* [IN] the address of the device specific information */
        pointer                        io_info_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;

    const uint_32 _PTR_                vectors;
    int                                num_vectors;
    int                                i;

    if (NULL == dspi_info_ptr)
    {
        return SPI_ERROR_DEINIT_FAILED;
    }

    _dspi_deinit_low(dspi_info_ptr->DSPI_PTR);

    /* Unregister callback */
    dma_callback_reg(dspi_info_ptr->DMA_RX_CHANNEL, NULL, NULL);

    /* Free buffers */
    if (dspi_info_ptr->RX_BUF) {
        _mem_free(dspi_info_ptr->RX_BUF);
    }

    _lwsem_destroy(&dspi_info_ptr->EVENT_IO_FINISHED);

    _mem_free(dspi_info_ptr);
    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_setparam
* Returned Value   :
* Comments         :
*    Set parameters for following transfers.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_setparam
   (
        /* [IN] Device specific context structure */
        pointer                        io_info_ptr,

        /* [IN] Parameters to set */
        SPI_PARAM_STRUCT_PTR           params
   )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    BSP_CLOCK_CONFIGURATION clock_config;
    uint_32 clock_speed;

    uint_32 ctar;
    uint_32 cpol_invert;

    /* Transfer mode */
    if ((params->ATTR & SPI_ATTR_TRANSFER_MODE_MASK) != SPI_ATTR_MASTER_MODE)
        return SPI_ERROR_TRANSFER_MODE_INVALID;

    /* Set master mode */
    dspi_ptr->MCR |= DSPI_MCR_MSTR_MASK;

    clock_config = _bsp_get_clock_configuration();

    /* Check the parameter against most recent values to avoid time consuming baudrate finding routine */
    if ((dspi_info_ptr->CLOCK_CONFIG != clock_config) || (dspi_info_ptr->BAUDRATE != params->BAUDRATE))
    {
        dspi_info_ptr->CLOCK_CONFIG = clock_config;
        dspi_info_ptr->BAUDRATE = params->BAUDRATE;

        /* Find configuration of prescalers best matching the desired value */
        clock_speed = _bsp_get_clock(dspi_info_ptr->CLOCK_CONFIG, dspi_info_ptr->CLOCK_SOURCE);
        _dspi_find_baudrate(clock_speed, dspi_info_ptr->BAUDRATE, &(dspi_info_ptr->CTAR_TIMING));
    }

    /* Set up prescalers */
    ctar = dspi_info_ptr->CTAR_TIMING;

    /* Set up transfer parameters */
    _dspi_ctar_params(params, &ctar);

    /* Check whether it is necessary to invert idle clock polarity */
    cpol_invert = (dspi_ptr->CTAR[0] ^ ctar) & DSPI_CTAR_CPOL_MASK;

    /* Store to register */
    dspi_ptr->CTAR[0] = ctar;

    dspi_info_ptr->DUMMY_PATTERN = params->DUMMY_PATTERN;
    dspi_info_ptr->ATTR = params->ATTR;
    
    if (cpol_invert) {
        /* Dummy transfer with inactive CS to invert idle clock polarity */
        dspi_ptr->MCR = (dspi_ptr->MCR & ~(uint_32)DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(0xFF);
        _dspi_dma_tx_rx(io_info_ptr, NULL, NULL, (params->FRAMESIZE+7)/8);
    }

    /* Set CS signals */
    dspi_ptr->MCR = (dspi_ptr->MCR & ~DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(~(params->CS));

    return SPI_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_callback
* Returned Value   : SPI DMA callback routine
* Comments         :
*   Notifies task about transfer completion.
*
*END*********************************************************************/
static void _dspi_dma_callback
    (
        /* [IN] The address of the device specific information */
        pointer                        parameter
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = parameter;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

   _lwsem_post(&dspi_info_ptr->EVENT_IO_FINISHED);

   return;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_transfer
* Returned Value   : number of bytes transferred
* Comments         :
*   Internal routine performing actual DMA transfer of given width
*   If txbuf is NULL the function expects dummy pattern already prepared in dspi_info_ptr->TX_BUFFER
*
*END*********************************************************************/
static _mqx_int _dspi_dma_transfer
    (
        /* [IN] Device specific context structure */
        DSPI_DMA_INFO_STRUCT_PTR       dspi_info_ptr,

        /* [IN] Data to transmit */
        uint_8_ptr                     txbuf,

        /* [OUT] Received data */
        uint_8_ptr                     rxbuf,

        /* [IN] Length of transfer in bytes */
        uint_32                        len,

        /* [IN] Width of data register access for DMA transfer */
        int                            regw
    )
{
    DMA_TCD_STRUCT                     tx_tcd;
    DMA_TCD_STRUCT                     rx_tcd;

    #if PSP_ENDIAN == MQX_LITTLE_ENDIAN
        if (regw > 1) {
            regw = -regw;
        }
    #endif

    if (NULL != rxbuf) {
        dma_tcd_reg2mem(&rx_tcd, &(dspi_info_ptr->DSPI_PTR->POPR), regw, rxbuf, len);
    }
    else {
        dma_tcd_reg2mem(&rx_tcd, &(dspi_info_ptr->DSPI_PTR->POPR), regw, dspi_info_ptr->RX_BUF, len);
        rx_tcd.LOOP_DST_OFFSET = -regw;
    }

    if (NULL != txbuf) {
        dma_tcd_mem2reg(&tx_tcd, &(dspi_info_ptr->DSPI_PTR->PUSHR), regw, txbuf, len);
    }
    else {
        dma_tcd_mem2reg(&tx_tcd, &(dspi_info_ptr->DSPI_PTR->PUSHR), regw, dspi_info_ptr->TX_BUF, len);
        tx_tcd.LOOP_SRC_OFFSET = -regw;
    }

    /* ensure that the semaphore is at zero count */
    while (_lwsem_poll(&dspi_info_ptr->EVENT_IO_FINISHED)) {}

    dma_transfer_setup(dspi_info_ptr->DMA_RX_CHANNEL, &rx_tcd);
    dma_request_enable(dspi_info_ptr->DMA_RX_CHANNEL);

    dma_transfer_setup(dspi_info_ptr->DMA_TX_CHANNEL, &tx_tcd);
    dma_request_enable(dspi_info_ptr->DMA_TX_CHANNEL);

    /* block the task until completion of the background operation */
    _lwsem_wait(&dspi_info_ptr->EVENT_IO_FINISHED);

    return len;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_tx_rx
* Returned Value   : number of bytes transferred
* Comments         :
*   Actual transmit and receive function.
*   Overrun prevention used, no need to update statistics in this function
*
*END*********************************************************************/
#if PSP_HAS_DATA_CACHE

static _mqx_int _dspi_dma_tx_rx
    (
        /* [IN] Device specific context structure */
        pointer                        io_info_ptr,

        /* [IN] Data to transmit */
        uint_8_ptr                     txbuf,

        /* [OUT] Received data */
        uint_8_ptr                     rxbuf,

        /* [IN] Length of transfer in bytes */
        uint_32                        len
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    int                                regw;

    uint_32                            head_len;
    uint_32                            tail_len;
    uint_32                            zero_copy_len;

    _mqx_int                           result;

    /* Check whether there is at least something to transfer */
    if (0 == len) {
        return 0;
    }

    /* Check frame width */
    if (DSPI_CTAR_FMSZ_GET(dspi_info_ptr->DSPI_PTR->CTAR[0]) > 7)
    {
        len = len & (~1UL); /* Round down to whole frames */
        regw = 2;
    }
    else {
        regw = 1;
    }

    /* If there is no data to transmit, prepare dummy pattern in proper byte order */
    if (NULL == txbuf) {
        if (regw == 1) {
            dspi_info_ptr->TX_BUF[0] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        else {
            dspi_info_ptr->TX_BUF[0] = (dspi_info_ptr->DUMMY_PATTERN>>8) & 0xFF;
            dspi_info_ptr->TX_BUF[1] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        _dcache_flush_line(dspi_info_ptr->TX_BUF);
    }

    if (!(len % PSP_CACHE_LINE_SIZE) && !((uint_32)txbuf % PSP_CACHE_LINE_SIZE) && !((uint_32)rxbuf % PSP_CACHE_LINE_SIZE)) {
        /* Everything is perfectly aligned, perform single zero copy operation without any head or tail */
        head_len = 0;
        tail_len = 0;
    }
    else if (len <= 2*PSP_CACHE_LINE_SIZE) {
        /* The whole transfer fits into intermediate buffers, perform single transfer (head only) */
        head_len = len;
        tail_len = 0;
    }
    else {
        /* Split the transfer into head, zero copy portion and tail */
        uint_32 cache_line_offset;

        uint_32 tx_head_len;
        uint_32 tx_tail_len;

        uint_32 rx_head_len;
        uint_32 rx_tail_len;

        if (NULL != rxbuf) {
            cache_line_offset = (uint_32)rxbuf % PSP_CACHE_LINE_SIZE;
            rx_head_len = cache_line_offset ? PSP_CACHE_LINE_SIZE - cache_line_offset : 0;
            rx_tail_len = (((uint_32)rxbuf + len) % PSP_CACHE_LINE_SIZE);
        }
        else {
            rx_head_len = 0;
            rx_tail_len = 0;
        }

        if (NULL != txbuf) {
            cache_line_offset = (uint_32)txbuf % PSP_CACHE_LINE_SIZE;
            tx_head_len = cache_line_offset ? PSP_CACHE_LINE_SIZE - cache_line_offset : 0;
            tx_tail_len = (((uint_32)txbuf + len) % PSP_CACHE_LINE_SIZE);

        }
        else {
            tx_head_len = 0;
            tx_tail_len = 0;
        }

        head_len = (rx_head_len > tx_head_len) ? rx_head_len : tx_head_len;
        tail_len = (rx_tail_len > tx_tail_len) ? rx_tail_len : tx_tail_len;

        if (regw > 1) {
            head_len += (head_len & 1);
            tail_len += (tail_len & 1);
        }
    }

    zero_copy_len =  len - head_len - tail_len;

    /* Head processed through intermediate buffers */
    if (head_len) {
        if (txbuf) {
            _mem_copy(txbuf, dspi_info_ptr->TX_BUF, head_len);
            _dcache_flush_mlines(dspi_info_ptr->TX_BUF, len);
            result = _dspi_dma_transfer(dspi_info_ptr, dspi_info_ptr->TX_BUF, dspi_info_ptr->RX_BUF, head_len, regw);
        }
        else {
            result = _dspi_dma_transfer(dspi_info_ptr, NULL, dspi_info_ptr->RX_BUF, head_len, regw);
        }
        if (result != head_len) {
            return IO_ERROR;
        }
        /*
         * Copy to application buffer intentionally ommited.
         * It is done later after invalidation of zero copy area as it may overlap into it.
         */
    }

    /* Zero copy area */
    if (zero_copy_len) {
        uint_8 *txbuf_real;
        uint_8 *rxbuf_real;

        txbuf_real = txbuf ? txbuf + head_len : NULL;
        rxbuf_real = rxbuf ? rxbuf + head_len : NULL;

        if (txbuf_real) {
            _dcache_flush_mlines(txbuf_real, zero_copy_len);
        }
        result = _dspi_dma_transfer(dspi_info_ptr, txbuf_real, rxbuf_real, zero_copy_len, regw);
        if (rxbuf_real)
        {
            _dcache_invalidate_mlines(rxbuf_real, zero_copy_len);
        }

        if (result != zero_copy_len) {
            return IO_ERROR;
        }
    }

    /* Copy head data into application buffer if desired */
    if (head_len && rxbuf) {
       _dcache_invalidate_mlines(dspi_info_ptr->RX_BUF, head_len);
       _mem_copy(dspi_info_ptr->RX_BUF, rxbuf, head_len);
    }

    /* Tail processed through intermediate buffers */
    if (tail_len) {
        if (txbuf) {
            _mem_copy(txbuf + len - tail_len, dspi_info_ptr->TX_BUF, tail_len);
            _dcache_flush_mlines(dspi_info_ptr->TX_BUF, tail_len);
            result = _dspi_dma_transfer(dspi_info_ptr, dspi_info_ptr->TX_BUF, dspi_info_ptr->RX_BUF, tail_len, regw);
        }
        else {
            result = _dspi_dma_transfer(dspi_info_ptr, NULL, dspi_info_ptr->RX_BUF, tail_len, regw);
        }
        if (result != tail_len) {
            return IO_ERROR;
        }
        if (rxbuf) {
            _dcache_invalidate_mlines(dspi_info_ptr->RX_BUF, tail_len);
            _mem_copy(dspi_info_ptr->RX_BUF, rxbuf + len - tail_len, tail_len);
        }
    }

    return len;
}

#else /* PSP_HAS_DATA_CACHE */

static _mqx_int _dspi_dma_tx_rx
    (
        /* [IN] Device specific context structure */
        pointer                        io_info_ptr,

        /* [IN] Data to transmit */
        uint_8_ptr                     txbuf,

        /* [OUT] Received data */
        uint_8_ptr                     rxbuf,

        /* [IN] Length of transfer in bytes */
        uint_32                        len
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    int                                regw;

    /* Check whether there is at least something to transfer */
    if (0 == len) {
        return 0;
    }

    /* Check frame width */
    if (DSPI_CTAR_FMSZ_GET(dspi_info_ptr->DSPI_PTR->CTAR[0]) > 7)
    {
        len = len & (~1UL); /* Round down to whole frames */
        regw = 2;
    }
    else {
        regw = 1;
    }

    /* If there is no data to transmit, prepare dummy pattern in proper byte order */
    if (NULL == txbuf) {
        if (regw == 1) {
            dspi_info_ptr->TX_BUF[0] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
        else {
            dspi_info_ptr->TX_BUF[0] = (dspi_info_ptr->DUMMY_PATTERN>>8) & 0xFF;
            dspi_info_ptr->TX_BUF[1] = dspi_info_ptr->DUMMY_PATTERN & 0xFF;
        }
    }

    return _dspi_dma_transfer(dspi_info_ptr, txbuf, rxbuf, len, regw);
}

#endif /* PSP_HAS_DATA_CACHE */


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_cs_deassert
* Returned Value   :
* Comments         :
*   Deactivates chip select signals.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_cs_deassert
    (
        /* [IN] The address of the device registers */
        pointer                        io_info_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    dspi_ptr->MCR = (dspi_ptr->MCR & ~(uint_32)DSPI_MCR_PCSIS_MASK) | DSPI_MCR_PCSIS(0xFF);

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : _dspi_dma_ioctl
* Returned Value   : MQX error code
* Comments         :
*    This function performs miscellaneous services for
*    the SPI I/O device.
*
*END*********************************************************************/
static _mqx_int _dspi_dma_ioctl
    (
        /* [IN] The address of the device specific information */
        pointer                        io_info_ptr,

        /* [IN] SPI transfer parameters */
        SPI_PARAM_STRUCT_PTR           params,

        /* [IN] The command to perform */
        uint_32                        cmd,

        /* [IN] Parameters for the command */
        uint_32_ptr                    param_ptr
    )
{
    DSPI_DMA_INFO_STRUCT_PTR           dspi_info_ptr = (DSPI_DMA_INFO_STRUCT_PTR)io_info_ptr;
    VDSPI_REG_STRUCT_PTR               dspi_ptr = dspi_info_ptr->DSPI_PTR;

    uint_32                            result = SPI_OK;

    BSP_CLOCK_CONFIGURATION clock_config;
    uint_32 clock_speed;

    switch (cmd)
    {
        case IO_IOCTL_SPI_GET_BAUD:
            clock_config = _bsp_get_clock_configuration();
            clock_speed = _bsp_get_clock(clock_config, dspi_info_ptr->CLOCK_SOURCE);
            *((uint_32_ptr)param_ptr) = _dspi_find_baudrate(clock_speed, *((uint_32_ptr)param_ptr), NULL);
            break;

        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
            break;
    }
    return result;
}

