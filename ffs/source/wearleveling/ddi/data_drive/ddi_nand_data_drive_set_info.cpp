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
 * $FileName: ddi_nand_data_drive_set_info.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains a function to set certain information about the data drive.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/data_drive/ddi_nand_data_drive.h"

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : setInfo
* Returned Value   : SUCCESS or ERROR
*   - ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED Drive is not initialised.
*   - ERROR_DDI_LDL_LDRIVE_INVALID_INFO_TYPE Cannot modify the requested data field.
*   - SUCCESS Data was set successfully.
* Comments         :
*   Set specified information about the data drive 
*
*   Only a small subset of drive info selectors can be modified. Attempting
*   to set a selector that cannot be changed will result in an error.
*
*END*--------------------------------------------------------------------*/
RtStatus_t DataDrive::setInfo
(
    /* [IN] Type of info requested: Tag, Component Version, Project Version, etc. */
    uint32_t Type, 
    
    /* [IN] Pointer to data to set. */
    const void * pInfo
)
{ /* Body */
    if (Type != kDriveInfoSectorSizeInBytes && !m_bInitialized)
    {
        return ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED;
    } /* Endif */
    
    switch (Type)
    {
        /* Change the number of non-sequential sector maps allocated for this drive. */
        case kDriveInfoNSSMCount:
            {
                uint32_t newCount;
                RtStatus_t result;
                
                DdiNandLocker locker;
                
                newCount = *(uint32_t *)pInfo;
                result = m_media->getNssmManager()->allocate(newCount);
                
                return result;
            }
            
        default:
            return LogicalDrive::setInfo(Type, pInfo);
    } /* Endswitch */
} /* Endbody */

/* EOF */
