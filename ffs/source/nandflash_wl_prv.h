#ifndef _nandflash_wl_prv_h_
#define _nandflash_wl_prv_h_
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
 * $FileName: nandflash_wl_prv.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   The file contains functions prototype, defines, structure
 *   definitions private to the NAND flash driver.
 *
 *END************************************************************************/


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/


/* Properties of device */
#define IO_NANDFLASH_ATTRIBS  (IO_DEV_ATTR_READ  | IO_DEV_ATTR_SEEK | \
    IO_DEV_ATTR_WRITE | IO_DEV_ATTR_BLOCK_MODE)


/*----------------------------------------------------------------------*/
/*
**                    Structure Definitions
*/



/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

    /* These are from nandflash.c */
    extern _mqx_int  _io_nandflash_wl_open(MQX_FILE_PTR, char_ptr, char_ptr);
    extern _mqx_int  _io_nandflash_wl_close(MQX_FILE_PTR);
    extern _mqx_int  _io_nandflash_wl_read (MQX_FILE_PTR, char_ptr, _mqx_int);

    extern _mqx_int  _io_nandflash_wl_write(MQX_FILE_PTR, char_ptr, _mqx_int);

    extern _mqx_int  _io_nandflash_wl_ioctl(MQX_FILE_PTR, _mqx_uint, pointer);

    _mqx_int _io_nandflash_wl_internal_read_raw(MQX_FILE_PTR fd_ptr, char_ptr data_ptr, uint_32_ptr);
    _mqx_int _io_nandflash_wl_internal_write_raw ( MQX_FILE_PTR fd_ptr, char_ptr data_ptr, uint_32_ptr);
    _mqx_int _io_nandflash_wl_internal_read_metadata (MQX_FILE_PTR fd_ptr, char_ptr data_ptr, uint_32_ptr);
    _mqx_int _io_nandflash_wl_internal_write_with_metadata ( MQX_FILE_PTR fd_ptr, char_ptr page_buff_struct, _mqx_int num , uint_32_ptr);
    _mqx_int _io_nandflash_wl_internal_read_with_metadata ( MQX_FILE_PTR fd_ptr, char_ptr page_buff_struct, _mqx_int num , uint_32_ptr);
    _mqx_int _io_nandflash_wl_internal_repair(MQX_FILE_PTR fd_ptr, boolean isBadScan);
    _mqx_int _io_nandflash_wl_internal_erase(MQX_FILE_PTR fd_ptr);


#ifdef __cplusplus
}
#endif

#endif
/* EOF */
