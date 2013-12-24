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
 * $FileName: ddi_nand_media_allocate.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file allocates the drives on the media
 *
 *END************************************************************************/
#include "wl_common.h"
#include <string.h>
#include <stdlib.h>
#include "ddi_media_internal.h"
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/mapper/mapper.h"
#include "ddi/media/ddi_nand_media.h"
#include "ddi_nand_hal.h"
#include "buffer_manager/media_buffer.h"

using namespace nand;

namespace nand {

#define MINIMUM_DATA_DRIVE_SIZE 8

} /* namespace nand */

extern nand::NandZipConfigBlockInfo_t g_nandZipConfigBlockInfo;


/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : allocateFromPreDefineRegions
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function allocates drives from pre-define structure
*
*END*--------------------------------------------------------------------*/
RtStatus_t Media::allocateFromPreDefineRegions()
{
    Region * newRegion;
    uint32_t i;
    
    if (m_bInitialized == false)
    {
        return ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED;
    } /* Endif */

    /* Lock the NAND for our purposes. */
    DdiNandLocker locker;

    /* If this assert hits, we're overflowing the regions map.  Investigate whether adding more regions is necessary. */
    assert(g_nandZipConfigBlockInfo.iNumEntries < MAX_NAND_REGIONS);

    for (i = 0; i < g_nandZipConfigBlockInfo.iNumEntries; i++)
    {
        if (g_nandZipConfigBlockInfo.Regions[i].eDriveType == kDriveTypeData)
        {
            /* Create a region object of the required type. */
            newRegion = Region::create(&g_nandZipConfigBlockInfo.Regions[i]);
            assert(newRegion);

            /* Add the region into the region array. */
            assert(m_iNumRegions < MAX_NAND_REGIONS);
            m_pRegionInfo[m_iNumRegions++] = newRegion;
        }        
    }

    if (!m_iNumRegions)
    {
        // m_eState = kMediaStateUnknown;
        return ERROR_DDI_NAND_LMEDIA_NO_REGIONS_IN_MEDIA;
    }

    return createDrives();
}

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : allocateFromPreDefine
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function allocates drives from pre-define structure
*
*END*--------------------------------------------------------------------*/
RtStatus_t Media::allocateFromPreDefine
(   
/* [IN] Media allocation table */
MediaAllocationTable_t * pTable
) 
{ /* Body */
    bool bDataDriveFound = false;
    RtStatus_t Status;
    NandConfigBlockRegionInfo_t zipRegionInfo;
    uint32_t mediaCount = NUM_LOGICAL_MEDIA;
    int mediaCounter;
    
    if (m_bInitialized == false)
    {
        return ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED;
    } /* Endif */

    for (mediaCounter = 0; mediaCounter < mediaCount; mediaCounter++) 
    {
        for (int i = 0; i < pTable[mediaCounter].u32NumEntries; i++) 
        {
            if (pTable[mediaCounter].Entry[i].Type == kDriveTypeData) 
            {
                bDataDriveFound = true;
                break;
            } /* Endif */
        }
        if (bDataDriveFound) 
        {
            break;
        } /* Endif */
    } /* Endfor */
    
    if (bDataDriveFound == true) 
    {
        NandParameters_t param = NandHal::getParameters();
        uint32_t blockSize = param.pageTotalSize * param.wPagesPerBlock;

        for (mediaCounter = 0 ; mediaCounter < mediaCount ; mediaCounter++) 
        {
            zipRegionInfo.iChip = 0;
            zipRegionInfo.iNumBlks = 0;
            for (int i = 0; i < pTable[mediaCounter].u32NumEntries; i++) 
            {
                zipRegionInfo.eDriveType = pTable[mediaCounter].Entry[i].Type;
                zipRegionInfo.wTag = pTable[mediaCounter].Entry[i].u32Tag;
                zipRegionInfo.iStartBlock = 0;
                if (pTable[mediaCounter].Entry[i].u64SizeInBytes % blockSize == 0) {
                    zipRegionInfo.iNumBlks = zipRegionInfo.iNumBlks + 
                    pTable[mediaCounter].Entry[i].u64SizeInBytes / blockSize;
                } 
                else 
                {
                    zipRegionInfo.iNumBlks = zipRegionInfo.iNumBlks + 
                    pTable[mediaCounter].Entry[i].u64SizeInBytes / blockSize + 1;
                } /* Endif */
                /* Create a region object of the required type. */
                Region * newRegion = Region::create(&zipRegionInfo);
                assert(newRegion);
                
                /* Add the region into the region array. */
                assert(m_iNumRegions < MAX_NAND_REGIONS);
                m_pRegionInfo[m_iNumRegions++] = newRegion;
            } /* Endfor */
        } /* Endfor */
        return createDrives();
    } /* Endif */
    
    return !SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : findNextRegionInChip
* Returned Value   : Region Number if exists, else returns -1.
* Comments         :
*   This function finds the allocated region that immediately follows the */
/* block number passed in as an argument.
*
*END*--------------------------------------------------------------------*/
int Media::findNextRegionInChip
(
/* [IN] Chip Number */
int iChip, 

/* [IN] Any block number allocated in the  preceeding region. */
int iBlock,

/* 
    ** [OUT] Pointer to NandZipConfigBlockInfo_t structure. 
    ** This structure has all the regions information for the entire media. 
    */
NandZipConfigBlockInfo_t * pNandZipConfigBlockInfo
)
{ /* Body */
    int i;
    int iRegion = -1, iUpperLimitBlock = 0x7fffffff;

    for(i = 0 ; i < pNandZipConfigBlockInfo->iNumEntries ; i++)
    {
        if(pNandZipConfigBlockInfo->Regions[i].iChip == iChip)
        {
            if(pNandZipConfigBlockInfo->Regions[i].iStartBlock > iBlock)
            {
                /* This Region is a good candidate */
                if(pNandZipConfigBlockInfo->Regions[i].iStartBlock < iUpperLimitBlock)
                {
                    iUpperLimitBlock = pNandZipConfigBlockInfo->Regions[i].iStartBlock;
                    iRegion = i;
                } /* Endif */
            } /* EndIf */
        } /* Endif */
    } /* Endfor */

    return(iRegion);
} /* Endbody */

/* EOF */
