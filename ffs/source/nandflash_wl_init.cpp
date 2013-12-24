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
 * $FileName: init_nandflash.c$
 * $Version : 3.8.1.0$
 * $Date    : Sep-1-2011$
 *
 * Comments:
 *
 *   This file contains the default initialization record for the NAND
 *   flash.
 *
 *END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "nandflash_wl.h"
#include "nfc_wl.h"

extern const NANDFLASH_INIT_STRUCT _bsp_nandflash_init;

const NANDFLASH_WL_INIT_STRUCT _bsp_nandflash_wl_init =
{
   &_bsp_nandflash_init,
   nfc_read_phy_page_raw,       /* PHY_PAGE_READ_RAW */
   nfc_write_phy_page_raw,      /* PHY_PAGE_WRITE_RAW */
   nfc_read_page_metadata,      /* PAGE_READ_METADATA */
   nfc_read_page_with_metadata, /* PAGE_READ_WITH_METADATA */
   nfc_write_page_with_metadata /* PAGE_WRITE_WITH_METADATA */
};

/* EOF */
