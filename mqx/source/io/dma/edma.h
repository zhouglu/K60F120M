#ifndef _edma_h_
#define _edma_h_ 1
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
* $FileName: edma.h$
* $Version : 4.0.1$
* $Date    : Jan-17-2013$
*
* Comments:
*   This file contains implementation of DMA driver for eDMA
*
*END************************************************************************/

#include "dma.h"


/*
** EMA_CHANNEL_CONTEXT
**
** This structure defines the context kept by EDMA driver for each channel
*/
typedef struct edma_channel_context
{
    /* Current status of the channel */
    int              STATUS;

    /* Callback function to notify about end of transfer*/
    DMA_EOT_CALLBACK EOT_CALLBACK;

    /* Context for EOT callback function */
    pointer          EOT_USERDATA;
    
} EDMA_CHANNEL_CONTEXT;


/* Prototypes for board/platform specific functions */
int _bsp_get_edma_done_vectors(uint_32 dev_num, const uint_32 _PTR_ _PTR_ vectors_ptr);
int _bsp_get_edma_error_vectors(uint_32 dev_num, const uint_32 _PTR_ _PTR_ vectors_ptr);
int _bsp_edma_enable(uint_32 dev_num);


#endif
