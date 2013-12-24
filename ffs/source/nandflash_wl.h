#ifndef _nandflash_wl_h_
#define _nandflash_wl_h_
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
 * $FileName: nandflash_wl.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   The file contains functions prototype, defines, structure
 *   definitions specific for the NAND flash driver.
 *
 *END************************************************************************/
#include "nandflash_wl_config.h"
#include "ioctl.h"
#include "nandflash.h"

#define NFC_DEBUG                                       1

#define NANDFLASHERR_ECC_CORRECTED_EXCEED_THRESHOLD   (NANDFLASH_ERROR_BASE | 0x0A)
#define NANDFLASHERR_WL_BASE                          (NANDFLASH_ERROR_BASE | 0x0B)

typedef struct nandflash_page_buff
{
    uchar_ptr data;
    uchar_ptr metadata;
} NANDFLASH_PAGE_BUFF_STRUCT, _PTR_ NANDFLASH_PAGE_BUFF_STRUCT_PTR;

typedef struct io_nandflash_wl_struct
{
   /* Core NANDFLASH */
   IO_NANDFLASH_STRUCT          CORE_NANDFLASH;
   uint_32 (_CODE_PTR_          PHY_PAGE_READ_RAW)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);
   uint_32 (_CODE_PTR_          PHY_PAGE_WRITE_RAW)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);
   uint_32 (_CODE_PTR_          PAGE_READ_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);   
   uint_32 (_CODE_PTR_          PAGE_READ_WITH_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uchar_ptr, uint_32, uint_32);      
   uint_32 (_CODE_PTR_          PAGE_WRITE_WITH_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uchar_ptr, uint_32, uint_32);
} IO_NANDFLASH_WL_STRUCT, _PTR_ IO_NANDFLASH_WL_STRUCT_PTR;

typedef struct nandflash_wl_init_struct
{
   /* Core NANDFLASH init */
   NANDFLASH_INIT_STRUCT_CPTR   CORE_NANDFLASH_INIT_PTR;
   uint_32 (_CODE_PTR_          PHY_PAGE_READ_RAW)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);
   uint_32 (_CODE_PTR_          PHY_PAGE_WRITE_RAW)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);
   uint_32 (_CODE_PTR_          PAGE_READ_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uint_32, uint_32);   
   uint_32 (_CODE_PTR_          PAGE_READ_WITH_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uchar_ptr, uint_32, uint_32);      
   uint_32 (_CODE_PTR_          PAGE_WRITE_WITH_METADATA)(struct io_nandflash_wl_struct _PTR_, uchar_ptr, uchar_ptr, uint_32, uint_32);
} NANDFLASH_WL_INIT_STRUCT;

typedef const struct nandflash_wl_init_struct _PTR_ NANDFLASH_WL_INIT_STRUCT_CPTR;

#ifdef __cplusplus
extern "C" {
#endif

extern const NANDFLASH_WL_INIT_STRUCT _bsp_nandflash_wl_init;

#ifdef __cplusplus
}
#endif

#endif /* _nandflash_wl_h_ */
/* EOF */
