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
* Comments:
*   This file contains implementation of DMA driver for eDMA
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include "dma.h"
#include "edma.h"


/*
 * Peripheral base pointers initialized by a macro from generated header,
 * but may be overloaded by BSP.
 */
#ifdef BSP_DMA_BASE_PTRS
DMA_MemMapPtr edma_base[] = BSP_DMA_BASE_PTRS;
#else
DMA_MemMapPtr edma_base[] = DMA_BASE_PTRS;
#endif

#ifdef BSP_DMAMUX_BASE_PTRS
DMAMUX_MemMapPtr dmamux_base[] = BSP_DMAMUX_BASE_PTRS;
#else
DMAMUX_MemMapPtr dmamux_base[] = DMAMUX_BASE_PTRS;
#endif

#define EDMA_MODULES  (ELEMENTS_OF(edma_base))
#define EDMA_CHANNELS (ELEMENTS_OF(edma_base[0]->TCD))

#define DMAMUX_MODULES  (ELEMENTS_OF(dmamux_base))
#define DMAMUX_CHANNELS (ELEMENTS_OF(dmamux_base[0]->CHCFG))


/*
 * Compatibility macros for features implemented only on certain devices
 */
#ifndef DMA_CR_ERGA_MASK
#define DMA_CR_ERGA_MASK 0
#endif

#ifndef DMA_CR_GRP0PRI_MASK
#define DMA_CR_GRP0PRI_MASK 0
#endif

#ifndef DMA_CR_GRP1PRI_MASK
#define DMA_CR_GRP1PRI_MASK 0
#endif


/*
** Channel context data (driver private)
*/
static EDMA_CHANNEL_CONTEXT edma_channel_context[EDMA_MODULES*EDMA_CHANNELS];


/*FUNCTION****************************************************************
*
* Function Name    : edma_done_isr
* Returned Value   :
* Comments         :
*    EDMA interrupt service routine to handle transfer completion
*
*END**********************************************************************/
static void edma_done_isr(pointer parameter)
{
    int edma_module;
    int edma_channel;
    int vchannel;
    
    DMA_MemMapPtr base;

    uint_32 flags;
    uint_32 mask;

    edma_module = (int)parameter;
    base = edma_base[edma_module];

    /* read out channel int flags */
    flags = DMA_INT_REG(base);
    
    edma_channel = 0;
    mask = 1;
    while (0 != mask && flags >= mask) {
    
        if (flags & mask) {
            /* clear channel int flag (w1c) */
            DMA_INT_REG(base) = mask;

            /* calculate virtual channel number */
            vchannel = EDMA_CHANNELS * edma_module + edma_channel;

            /* set status but leave existing error condition until it is cleared */            
            if (edma_channel_context[vchannel].STATUS != DMA_STATUS_ERROR) {
                edma_channel_context[vchannel].STATUS = DMA_STATUS_DONE;            

                /* execute callback */            
                if (NULL != edma_channel_context[vchannel].EOT_CALLBACK) {
                    edma_channel_context[vchannel].EOT_CALLBACK(edma_channel_context[vchannel].EOT_USERDATA);
        
                }
            }
        }
        
        mask <<= 1;
        edma_channel++;
    }
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_err_isr
* Returned Value   :
* Comments         :
*    EDMA interrupt service routine to handle transfer error interrupt
*
*END**********************************************************************/
static void edma_err_isr(pointer parameter)
{
    int edma_module;
    int edma_channel;
    int vchannel;
    
    DMA_MemMapPtr base;

    uint_32 flags;
    uint_32 mask;

    edma_module = (int)parameter;
    base = edma_base[edma_module];

    /* read out channel int flags */
    flags = DMA_ERR_REG(base);
    
    edma_channel = 0;
    mask = 1;
    while (0 != mask && flags >= mask) {
    
        if (flags & mask) {
            /* disable request for the channel */
            DMA_CERQ_REG(base) = edma_channel;

            /* clear channel error flag (w1c) */
            DMA_ERR_REG(base) = mask;

            /* clear channel int flag (w1c) */
            DMA_INT_REG(base) = mask;

            /* calculate virtual channel number */
            vchannel = EDMA_CHANNELS * edma_module + edma_channel;

            /* set status */            
            edma_channel_context[vchannel].STATUS = DMA_STATUS_ERROR;            

            /* execute callback */            
            if (NULL != edma_channel_context[vchannel].EOT_CALLBACK) {
                edma_channel_context[vchannel].EOT_CALLBACK(edma_channel_context[vchannel].EOT_USERDATA);
            }
        }
        
        mask <<= 1;
        edma_channel++;
    }
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_init_module
* Returned Value   :
* Comments         :
*    Initialization of single EDMA module
*
*END**********************************************************************/
static int edma_init_module(int edma_module)
{
    int i;

    const uint_32 *vectors;
    int vectors_count;

    DMA_MemMapPtr base;
    
    _bsp_edma_enable(edma_module);

    base = edma_base[edma_module];

    DMA_ERQ_REG(base) = 0;
    DMA_CR_REG(base) = DMA_CR_CX_MASK | DMA_CR_HALT_MASK; /* cancel transfer and halt */

    /* wait until transfer canceled */
    while (DMA_CR_REG(base) & DMA_CR_CX_MASK)
        ;
    
    DMA_CERR_REG(base) = DMA_CERR_CAEI_MASK; /* clear all error flags */

    for (i = 0; i < EDMA_CHANNELS; i++) {
        int dmamux_module = i / DMAMUX_CHANNELS;
        int dmamux_channel = i % DMAMUX_CHANNELS; 
        
        DMAMUX_CHCFG_REG(dmamux_base[dmamux_module], dmamux_channel) = 0;
        DMA_CERQ_REG(base) = i;
    }

    /* Minor loop mapping enabled, round robin arbitration */
    DMA_CR_REG(base) = DMA_CR_EMLM_MASK | DMA_CR_ERGA_MASK | DMA_CR_ERCA_MASK; /* | DMA_CR_HOE_MASK */

    vectors_count = _bsp_get_edma_done_vectors(edma_module, &vectors);    
    for (i = 0; i < vectors_count; i++) {
        _int_install_isr(vectors[i], edma_done_isr, (void *)(edma_module));
        _bsp_int_init(vectors[i], BSP_EDMA_INT_LEVEL, 0, TRUE);
    }

    vectors_count = _bsp_get_edma_error_vectors(edma_module, &vectors);    
    for (i = 0; i < vectors_count; i++) {
        _int_install_isr(vectors[i], edma_err_isr, (void *)(edma_module));
        _bsp_int_init(vectors[i], BSP_EDMA_INT_LEVEL, 0, TRUE);
    }

    /* enable error interrupts */
    DMA_SEEI_REG(base) = DMA_SEEI_SAEE_MASK;
    
    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_init
* Returned Value   :
* Comments         :
*    Initialization of DMA driver
*
*END**********************************************************************/
int dma_init(void)
{
    int edma_module;
    int result;
    
    for (edma_module = 0; edma_module < EDMA_MODULES; edma_module++) {
        result = edma_init_module(edma_module);
        if (MQX_OK != result) {
            break;
        }
    }

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_deinit
* Returned Value   :
* Comments         :
*    DMA driver cleanup
*
*END**********************************************************************/
int dma_deinit(void)
{
    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_setup
* Returned Value   :
* Comments         :
*    Sets up data request source for given channel
*
*END**********************************************************************/
int dma_channel_setup(int vchannel, int source)
{
    int dmamux_module;
    int dmamux_channel;

    if (vchannel >= DMAMUX_CHANNELS * DMAMUX_MODULES)
        return MQX_INVALID_PARAMETER;

    if (source > DMAMUX_CHCFG_SOURCE_MASK)
        return MQX_INVALID_PARAMETER;
                
    dmamux_module = vchannel / DMAMUX_CHANNELS;
    dmamux_channel = vchannel % DMAMUX_CHANNELS;

    DMAMUX_CHCFG_REG(dmamux_base[dmamux_module], dmamux_channel) = 0;
    DMAMUX_CHCFG_REG(dmamux_base[dmamux_module], dmamux_channel) = source | DMAMUX_CHCFG_ENBL_MASK;
    
    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_channel_status
* Returned Value   :
* Comments         :
*    Returns current status of given channel
*
*END**********************************************************************/
int dma_channel_status(int vchannel)
{
    return edma_channel_context[vchannel].STATUS;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_transfer_setup
* Returned Value   :
* Comments         :
*    Prepares transfer on given channel according to parameters in DMA_TCD structure
*
*END**********************************************************************/
int dma_transfer_setup(int vchannel, DMA_TCD_PTR tcd)
{
    int edma_module;
    int edma_channel;
    int loop_offset;

    DMA_MemMapPtr base;

    uint_32 attr;
    uint_32 nbytes;
    
    
    /* basic sanity checks */    

    if (vchannel >= EDMA_CHANNELS * EDMA_MODULES)
        return MQX_INVALID_PARAMETER;
        
    edma_module = vchannel / EDMA_CHANNELS;
    edma_channel = vchannel % EDMA_CHANNELS;
    base = edma_base[edma_module];
        
    
    edma_channel_context[vchannel].STATUS = DMA_STATUS_IDLE;
    
    /* if both offsets are non zero then they have to be equal */
    if ((0 != tcd->LOOP_SRC_OFFSET) && (0 != tcd->LOOP_DST_OFFSET)) {
        if (tcd->LOOP_SRC_OFFSET != tcd->LOOP_DST_OFFSET) {
            return MQX_INVALID_PARAMETER;
        }
    }

    /* take any non-zero offset value */
    loop_offset = tcd->LOOP_SRC_OFFSET ? tcd->LOOP_SRC_OFFSET : tcd->LOOP_DST_OFFSET;
    
    if (loop_offset > ((1<<19)-1) || loop_offset < -((1<<19)))
        return MQX_INVALID_PARAMETER;
    
    if (tcd->LOOP_BYTES > 1023)
        return MQX_INVALID_PARAMETER;
        
    if (tcd->SRC_MODULO > 31)
        return MQX_INVALID_PARAMETER;

    if (tcd->DST_MODULO > 31)
        return MQX_INVALID_PARAMETER;
        
        
    attr = DMA_ATTR_SMOD(tcd->SRC_MODULO) | DMA_ATTR_DMOD(tcd->DST_MODULO);
    
    switch (tcd->SRC_WIDTH) {
        case 1:
            attr |= DMA_ATTR_SSIZE(0);
            break;
        case 2:
            attr |= DMA_ATTR_SSIZE(1);
            break;
        case 4:
            attr |= DMA_ATTR_SSIZE(2);
            break;
        case 16:
            attr |= DMA_ATTR_SSIZE(4);
            break;
        default:
            return MQX_INVALID_PARAMETER;
    }

    switch (tcd->DST_WIDTH) {
        case 1:
            attr |= DMA_ATTR_DSIZE(0);
            break;
        case 2:
            attr |= DMA_ATTR_DSIZE(1);
            break;
        case 4:
            attr |= DMA_ATTR_DSIZE(2);
            break;
        case 16:
            attr |= DMA_ATTR_DSIZE(4);
            break;
        default:
            return MQX_INVALID_PARAMETER;
    }

    
    /* loop has to divisible by transfer width */
    if ((tcd->LOOP_BYTES % tcd->SRC_WIDTH) || (tcd->LOOP_BYTES % tcd->DST_WIDTH))
        return MQX_INVALID_PARAMETER;
    
    DMA_CSR_REG(base, edma_channel) = DMA_CSR_DREQ_MASK | DMA_CSR_INTMAJOR_MASK;

    DMA_ATTR_REG(base, edma_channel) = attr;

    
    DMA_SADDR_REG(base, edma_channel) = tcd->SRC_ADDR;
    DMA_SOFF_REG(base, edma_channel) = tcd->SRC_OFFSET;

    DMA_DADDR_REG(base, edma_channel) = tcd->DST_ADDR;
    DMA_DOFF_REG(base, edma_channel) = tcd->DST_OFFSET;


    nbytes = DMA_NBYTES_MLOFFYES_NBYTES(tcd->LOOP_BYTES) | DMA_NBYTES_MLOFFYES_MLOFF(loop_offset);

    if (tcd->LOOP_SRC_OFFSET)
        nbytes |= DMA_NBYTES_MLOFFYES_SMLOE_MASK;

    if (tcd->LOOP_DST_OFFSET)
        nbytes |= DMA_NBYTES_MLOFFYES_DMLOE_MASK;
    
    DMA_NBYTES_MLOFFYES_REG(base, edma_channel) = nbytes;
    

    DMA_CITER_ELINKNO_REG(base, edma_channel) = tcd->LOOP_COUNT;
    DMA_BITER_ELINKNO_REG(base, edma_channel) = tcd->LOOP_COUNT;
    

    DMA_SLAST_REG(base, edma_channel) = 0;
    DMA_DLAST_SGA_REG(base, edma_channel) = 0;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_request_enable
* Returned Value   :
* Comments         :
*    Enables request on given channel to start/resume a transfer
*
*END**********************************************************************/
int dma_request_enable(int vchannel)
{
    int edma_module;
    int edma_channel;
    
    DMA_MemMapPtr base;

    if (vchannel >= EDMA_CHANNELS * EDMA_MODULES)
        return MQX_INVALID_PARAMETER;
        
    edma_module = vchannel / EDMA_CHANNELS;
    edma_channel = vchannel % EDMA_CHANNELS;
    base = edma_base[edma_module];    
    
    DMA_SERQ_REG(base) = edma_channel;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_request_disable
* Returned Value   :
* Comments         :
*    Disables request on given channel to pause a transfer
*
*END**********************************************************************/
int dma_request_disable(int vchannel)
{
    int edma_module;
    int edma_channel;
    
    DMA_MemMapPtr base;

    if (vchannel >= EDMA_CHANNELS * EDMA_MODULES)
        return MQX_INVALID_PARAMETER;
        
    edma_module = vchannel / EDMA_CHANNELS;
    edma_channel = vchannel % EDMA_CHANNELS;
    base = edma_base[edma_module];    

    DMA_CERQ_REG(base) = edma_channel;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : dma_callback_reg
* Returned Value   :
* Comments         :
*    Registers a callback function to be executed upon transfer completion or an error
*
*END**********************************************************************/
int dma_callback_reg(int vchannel, DMA_EOT_CALLBACK callback_func, pointer callback_data)
{
    if (vchannel >= EDMA_CHANNELS * EDMA_MODULES)
        return MQX_INVALID_PARAMETER;
        
    edma_channel_context[vchannel].EOT_CALLBACK = callback_func;
    edma_channel_context[vchannel].EOT_USERDATA = callback_data;

    return MQX_OK;
}
