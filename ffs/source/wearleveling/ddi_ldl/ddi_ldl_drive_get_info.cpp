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
 * $FileName: ddi_ldl_drive_get_info.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file device driver interface Logical Drive Layer API to get info for any drive type.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi_media_errordefs.h"
#include "ddi_media.h"
#include "sectordef.h"
#include "ddi_media_internal.h"
#include <string.h>

/*SerialNumber_t g_InternalMediaSerialNumber; */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getInfo
* Returned Value   : SUCCESS or ERROR
*   - SUCCESS
*   - ERROR_DDI_LDL_LDRIVE_INVALID_INFO_TYPE
* Comments         :
*   This function handles the common drive info selectors that can be
*   serviced by reading fields of the logical drive descriptor structure alone.
*
*END*--------------------------------------------------------------------*/
RtStatus_t LogicalDrive::getInfo
(
    /* [IN] Type of information to return */
    uint32_t Type, 
    
    /* [IN] Buffer to fill with info */
    void * pInfo
)
{ /* Body */
    unsigned i;
    
    switch (Type)
    {
        case kDriveInfoSectorSizeInBytes:
            *((uint32_t *)pInfo) = m_u32SectorSizeInBytes;
            break;

        case kDriveInfoEraseSizeInBytes:
            *((uint32_t *)pInfo) = m_u32EraseSizeInBytes;
            break;

        case kDriveInfoSizeInBytes :
            *((uint64_t *)pInfo) = m_u64SizeInBytes;
            break;

        case kDriveInfoSizeInMegaBytes:
            *((uint32_t *)pInfo) = (m_u64SizeInBytes) >> 20;
            break;

        case kDriveInfoSizeInSectors:
            *((uint64_t *)pInfo) = m_u32NumberOfSectors;
            break;

        case kDriveInfoType:
            *((LogicalDriveType_t *)pInfo) = m_Type;
            break;

        case kDriveInfoTag:
            *((uint32_t *)pInfo) = m_u32Tag;
            break;

        case kDriveInfoIsWriteProtected:
            *((bool *)pInfo) = m_bWriteProtected;
            break;
            
        case kDriveInfoNativeSectorSizeInBytes:
            *(uint32_t *)pInfo = m_nativeSectorSizeInBytes;
            break;
            
        case kDriveInfoSizeInNativeSectors:
            *(uint32_t *)pInfo = m_numberOfNativeSectors;
            break;
            
        case kDriveInfoComponentVersion:
            _wl_mem_set(pInfo, 0, sizeof(SystemVersion_t));
            break;
            
        case kDriveInfoProjectVersion:
            _wl_mem_set(pInfo, 0, sizeof(SystemVersion_t));
            break;
            
        case kDriveInfoSectorOffsetInParent:
            *(uint32_t *)pInfo = m_pbsStartSector;
            break;
            
        case kDriveInfoMediaPresent:
            /* Assume the media is present by default. */
            *(bool *)pInfo = true;
            break;

        case kDriveInfoMediaChange:
            /* The default is that the media cannot change. */
            *((bool *)pInfo) = false;
            break;
        default:
            return ERROR_DDI_LDL_LDRIVE_INVALID_INFO_TYPE;
    } /* Endswitch */

    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : DriveGetInfo
* Returned Value   : SUCCESS or ERROR
*   - SUCCESS
*   - ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TAG
*   - ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED
* Comments         :
*   For the requested logical drive, get the info Type into *pInfo.
*
*END*--------------------------------------------------------------------*/
RtStatus_t DriveGetInfo
(
    /* [IN] Unique tag for the drive to operate on */
    DriveTag_t tag, 

    /* [IN] Type of information to write to *pInfo */
    uint32_t Type, 

    /* [OUT] Buffer to fill with requested information */
    void * pInfo
)
{ /* Body */
    /* Get drive dependings on its tag */
    LogicalDrive * drive = DriveGetDriveFromTag(tag);
    
    if (!drive)
    {
        return ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TAG;
    }
    else if (!drive->isInitialized() )
    {
        return ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED;
    } /* Endif */

    /* Give the drive API a first shot. */
    return drive->getInfo(Type, pInfo);
} /* Endbody */

/* EOF */
