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
 * $FileName: ddi_nand_data_drive_get_info.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains a function to get certain information about the data drive
 *
 *END************************************************************************/
#include "wl_common.h"
#include "ddi_nand_ddi.h"
#include "ddi_nand_data_drive.h"
#include "mapper.h"

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : _taskq_test
* Returned Value   : MQX_OK or an error code
* Comments         :
*   This function tests all task queues for correctness
*
*END*--------------------------------------------------------------------*/
RtStatus_t DataDrive::getInfo(uint32_t selector, void * pInfo)
{ /* Body */
    int32_t i;
    uint32_t u32SectorsPerBlock = NandHal::getParameters().wPagesPerBlock;
    uint32_t temp;

    if (!m_bInitialized)
    {
        return ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED;
    } /* Endif */

    switch (selector)
    {
        case kDriveInfoSizeInSectors:
            temp = m_u32NumberOfSectors/2 - MAPPER_MINIMUM_THRESHOLD;
            temp /= u32SectorsPerBlock;
            temp *= u32SectorsPerBlock; 
            *((uint64_t *)pInfo) = temp;
            break;
            
        /* Always TRUE */
        case kDriveInfoMediaPresent:
            *((bool *)pInfo) = true;
            break;

        /* Always FALSE - Can't change. */
        case kDriveInfoMediaChange:
            *((bool *)pInfo) = false;
            break;
            
        /* Return the number of non-sequential sector maps allocated for this drive. */
        case kDriveInfoNSSMCount:
            *(uint32_t *)pInfo = g_nandMedia->getNssmManager()->getBaseNssmCount();
            break;
        
        default:
            return LogicalDrive::getInfo(selector, pInfo);
    } /* Endswitch */

    return SUCCESS;
} /* Endbody */

/* EOF */
