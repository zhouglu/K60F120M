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
*   This file contains definitions private to the SPI driver.
*
*END************************************************************************/

#ifndef __spi_dspi_prv_h__
#define __spi_dspi_prv_h__

#include "spi.h"
#include "spi_dspi_common.h"


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** DSPI_INFO_STRUCT
** Run time state information for each spi channel
*/
typedef struct dspi_info_struct
{
    /* SPI channel number */
    uint_32                           CHANNEL;

    /* The input clock source of the module */
    CM_CLOCK_SOURCE                   CLOCK_SOURCE;

    /* Most recently used clock configuration (cached value) */
    BSP_CLOCK_CONFIGURATION           CLOCK_CONFIG;

    /* Most recently used baudrate */
    uint_32                           BAUDRATE;

    /* Most recently calculated timing parameters for CTAR register */
    uint_32                           CTAR_TIMING;

    /* The spi device registers */
    VDSPI_REG_STRUCT_PTR              DSPI_PTR;

    /* Pattern to transmit during half-duplex rx transfer */
    uint_32                           DUMMY_PATTERN;

    /* Additional attributes for the transfer */
    uint_32                           ATTR;

    /* Pointer to the buffer to use for RX data in ISR */
    uint_8_ptr                        RX_BUF;

    /* Remaining RX byte count for ISR */
    uint_32                           RX_LEN;

    /* Pointer to the buffer to use for TX data in ISR*/
    uint_8_ptr                        TX_BUF;

    /* Remaining TX byte count for ISR */
    uint_32                           TX_LEN;

    /* Event to signal that background job is done */
    LWSEM_STRUCT                      EVENT_IO_FINISHED;

    /* Number of interrupt vectors supported by the peripheral */
    uint_32                           NUM_VECTORS;

} DSPI_INFO_STRUCT, _PTR_ DSPI_INFO_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
