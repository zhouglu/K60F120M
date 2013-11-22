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
*   This file contains the definitions of constants and structures
*   required for the DSPI driver
*
*END************************************************************************/

#ifndef __spi_dspi_dma_h__
#define __spi_dspi_dma_h__

#include <bsp.h>


/*--------------------------------------------------------------------------*/
/*
**                    CONSTANT DEFINITIONS
*/


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** DSPI_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a spi port is initialized.
*/
typedef struct dspi_dma_init_struct
{
    /* SPI channel number */
    uint_32 CHANNEL;

    /* The input clock source of the module */
    CM_CLOCK_SOURCE CLOCK_SOURCE;

    /* DMA configuration */
    uint_32 DMA_RX_CHANNEL;
    uint_32 DMA_TX_CHANNEL;
    uint_32 DMA_RX_SOURCE;
    uint_32 DMA_TX_SOURCE;

} DSPI_DMA_INIT_STRUCT, _PTR_ DSPI_DMA_INIT_STRUCT_PTR;

typedef const DSPI_DMA_INIT_STRUCT _PTR_ DSPI_DMA_INIT_STRUCT_CPTR;


/*----------------------------------------------------------------------*/
/*
**              DEFINED VARIABLES
*/

extern const SPI_DEVIF_STRUCT _spi_dspi_dma_devif;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern pointer   _bsp_get_dspi_base_address(uint_8);
extern boolean   _bsp_dspi_enable_access(uint_32 device);

#ifdef __cplusplus
}
#endif

#endif
