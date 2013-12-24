#ifndef _nandflash_wl_ffs_h_
#define _nandflash_wl_ffs_h_
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

#include "nandflash_wl.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/
#define NANDFLASH_1ST_DATA_DRIVE_SIZE_IN_BLOCK 200
#define NANDFLASH_1ST_DATA_DRIVE_START_BLOCK 10

/*
** NAND Flash WL IOCTL calls
*/

#define NANDFLASH_IOCTL_WL_NR_START_BASE            _IOC_NR(_IO(IO_TYPE_NANDFLASH,0x11))

#define NANDFLASH_IOCTL_GET_PAGE_METADATA_AREA_SIZE _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 1)

#define NANDFLASH_IOCTL_READ_METADATA               _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 2)

#define NANDFLASH_IOCTL_READ_WITH_METADATA          _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 3)

#define NANDFLASH_IOCTL_WRITE_WITH_METADATA         _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 4)

#define NANDFLASH_IOCTL_WRITE_RAW                   _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 5)

#define NANDFLASH_IOCTL_READ_RAW                    _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 6)

#define NANDFLASH_IOCTL_REPAIR                      _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 7)

#define NANDFLASH_IOCTL_REPAIR_WITH_BAD_SCAN        _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 8)

#define NANDFLASH_IOCTL_ERASE                       _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 9)

#define NANDFLASH_IOCTL_WL_END_BASE                 _IO(IO_TYPE_NANDFLASH, NANDFLASH_IOCTL_WL_NR_START_BASE + 10)

/*----------------------------------------------------------------------*/
/*
**                          TYPE DEFINITIONS
*/
#define NANDWL_OPEN_NORMAL NULL
#define NANDWL_OPEN_IGNORE_INIT_MEDIA "I"
#define NANDWL_OPEN_FORCE_REPAIR_WHEN_FAILED "R"

/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
*/


#ifdef __cplusplus
extern "C" {
#endif

    _mqx_uint _io_nandflash_wl_install(NANDFLASH_WL_INIT_STRUCT_CPTR, char_ptr);
    _mqx_int  _io_nandflash_wl_uninstall(IO_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif /* _nandflash_wl_ffs_h_ */
/* EOF */
