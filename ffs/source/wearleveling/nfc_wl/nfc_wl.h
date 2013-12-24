#ifndef _nfc_wl_h_
#define _nfc_wl_h_
/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
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
 *****************************************************************************
 *
 * $FileName: nfc_wl.h$
 * $Version : 3.8.11.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains defines and functions prototype for the
 *   NAND Flash aController (NFC) low level driver.
 *
 *END************************************************************************/

#include "nandflash_wl.h"

/* NFC ECC status word */
#define NFC_ECC_STATUS_CORFAIL       (0x80)
#define NFC_ECC_STATUS_ERROR_COUNT   (0x3F)
#define NFC_ECC_STATUS_OFFSET        (0x0C)

/*----------------------------------------------------------------------*/
/*
**                     FUNCTION PROTOTYPES
*/
#ifdef __cplusplus
extern "C" {
#endif
    extern uint_32 nfc_wl_init( IO_NANDFLASH_STRUCT_PTR );
    extern uint_32 nfc_read_page_with_metadata( IO_NANDFLASH_WL_STRUCT_PTR, uchar_ptr, uchar_ptr,  uint_32, uint_32);
    extern uint_32 nfc_read_page_metadata (  IO_NANDFLASH_WL_STRUCT_PTR , uchar_ptr, uint_32, uint_32);
    extern uint_32 nfc_write_page_with_metadata( IO_NANDFLASH_WL_STRUCT_PTR,  uchar_ptr, uchar_ptr, uint_32, uint_32 );
    extern uint_32 nfc_write_phy_page_raw(IO_NANDFLASH_WL_STRUCT_PTR, uchar_ptr, uint_32, uint_32 );
    extern uint_32 nfc_read_phy_page_raw(IO_NANDFLASH_WL_STRUCT_PTR, uchar_ptr, uint_32, uint_32);
#ifdef __cplusplus
}
#endif

#endif
/* EOF */
