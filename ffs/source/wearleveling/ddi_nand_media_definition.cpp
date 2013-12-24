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
 * $FileName: ddi_nand_media_definition.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains default media definition file for NAND.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi_media.h"
#include "ddi_media_internal.h"
#include "ddi_nand.h"
#include "simple_mutex.h"
#include "ddi/media/ddi_nand_media.h"

/* 
** Constants 
*/

#define NUM_LOGICAL_MEDIA               1

#define MIN_DATA_DRIVE_SIZE             ( 8 )

/* 
** Global external declarations 
*/
/* NAND media */
/*
MediaAllocationTable_t g_mediaTablePreDefine[NUM_LOGICAL_MEDIA] = 
{
    
    {
        1,       
        { 0, kDriveTypeData, DRIVE_TAG_DATA, 13312000, true }
    } 
};
*/

nand::NandZipConfigBlockInfo_t g_nandZipConfigBlockInfo = 
{
    1,
    {
        kDriveTypeData,
        DRIVE_TAG_DATA,
        NANDFLASH_1ST_DATA_DRIVE_SIZE_IN_BLOCK, // 90
        0,
        NANDFLASH_1ST_DATA_DRIVE_START_BLOCK // 110
    }
    
    // TODO: multi fragment data drive example
    /*
    3,
    {
        {
            kDriveTypeData,
            DRIVE_TAG_DATA,
            10,
            0,
            0
        },
        {
            kDriveTypeUnknown,
            DRIVE_TAG_DATA,
            10,
            0,
            10
        },
        {
            kDriveTypeData,
            DRIVE_TAG_DATA,
            80,
            0,
            220
        },
    }
    */

};

const int g_MinDataDriveSize = MIN_DATA_DRIVE_SIZE;

WL_MUTEX g_NANDThreadSafeMutex;

/* EOF */
