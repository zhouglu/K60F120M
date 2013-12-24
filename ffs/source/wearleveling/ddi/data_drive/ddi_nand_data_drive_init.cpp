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
 * $FileName: ddi_nand_data_drive_init.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file handles the initialization of the data drive
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/data_drive/ddi_nand_data_drive.h"
#include "ddi_media.h"
#include "ddi/mapper/mapper.h"
#include "ddi_nand_hal.h"
#include <stdlib.h>

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : DataDrive
* Returned Value   : N/A
* Comments         :
*   Constructor
*
*END*--------------------------------------------------------------------*/
DataDrive::DataDrive
(
    /* [IN] Media pointer */
    Media * media, 
    
    /* [IN] Region pointer */
    Region * region
)
:   LogicalDrive(),
m_media(media),
m_u32NumRegions(0),
m_ppRegion(NULL)
{ /* Body */
    m_bInitialized = false;
    m_bPresent = true;
    m_bErased = false;
    m_bWriteProtected = false;
    m_Type = region->m_eDriveType;
    m_u32Tag = region->m_wTag;
    m_logicalMedia = media;

    NandParameters_t & params = NandHal::getParameters();
    m_u32SectorSizeInBytes = params.pageDataSize;
    m_nativeSectorSizeInBytes = m_u32SectorSizeInBytes;
    m_nativeSectorShift = 0;

    m_u32EraseSizeInBytes = params.pageDataSize * params.wPagesPerBlock;
    
    addRegion(region);
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : ~DataDrive
* Returned Value   : N/A
* Comments         :
*   Destructor
*
*END*--------------------------------------------------------------------*/
DataDrive::~DataDrive()
{ /* Body */
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : addRegion
* Returned Value   : void
* Comments         :
*   This function add a region to DataDrive object
*
*END*--------------------------------------------------------------------*/
void DataDrive::addRegion
(
    /* [IN] A new region */
    Region * region
)
{ /* Body */
    m_u32NumberOfSectors += (region->m_iNumBlks - region->getBadBlockCount())    /* Number of Good Blocks */
    * (NandHal::getParameters().wPagesPerBlock);
    m_numberOfNativeSectors = m_u32NumberOfSectors;

    m_u64SizeInBytes = ((uint64_t)m_u32NumberOfSectors * m_u32SectorSizeInBytes);
    
    region->m_pLogicalDrive = this;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : _taskq_test
* Returned Value   : SUCCESS or ERROR
*   - SUCCESS If no error has occurred.
*   - ERROR_DDI_LDL_LDRIVE_MEDIA_NOT_ALLOCATED
*   - ERROR_DDI_LDL_LDRIVE_LOW_LEVEL_MEDIA_FORMAT_REQUIRED
*   - ERROR_DDI_LDL_LDRIVE_WRITE_PROTECTED
*   - ERROR_DDI_LDL_LDRIVE_HARDWARE_FAILURE
* Comments         :
*   This function will initialize the Data drive which includes the following:
*   Initialize the Mapper interface if available.
*   - Fill in an array of Region structures for this Data Drive.
*   - Reconstruct Physical start address for each region.
*   - Allocate non-sequential sectors maps (NSSM) for the drive.
*
*   NANDDataDriveInit() sets up data structures used by the Data Drive routines.
*
*   Some data structures are expected to already be partly or wholey
*   set up by Media initialization routines
*
*END*--------------------------------------------------------------------*/
RtStatus_t DataDrive::init()
{ /* Body */
    RtStatus_t ret;
    
    DdiNandLocker locker;
    
    if (!m_bPresent)
    {
        return ERROR_DDI_LDL_LDRIVE_MEDIA_NOT_ALLOCATED;
    } /* Endif */
    
    /* If we've already been initialized, just return SUCCESS. */
    if (m_bInitialized)
    {
        return SUCCESS;
    } /* Endif */
    
    /* Build private list of Data Drive Regions */
    buildRegionsList();
    
    /* Partition NonSequential SectorsMaps memory. */
    ret = m_media->getNssmManager()->allocate(NUM_OF_MAX_SIZE_NS_SECTORS_MAPS);
    if (ret != SUCCESS)
    {
        return ret;
    } /* Endif */

    /* 
    ** The last thing we must do is initialize the mapper. This comes last because it uses
    ** the region structures and the NSSM. 
    */
    ret = m_media->getMapper()->init();
    if (ret)
    {
        return ret;
    } /* Endif */
    
    m_bInitialized = true;
    
    return SUCCESS;
} /* Endbody */

/*
** Scans for regions belonging to this drive.
** The drive's type and tag must have already been filled in when this is called. 
*/
void DataDrive::processRegions(Region ** regionArray, unsigned * regionCount)
{ /* Body */
    unsigned dataRegionsCount = 0;
    Region::Iterator it = m_media->createRegionIterator();
    Region * theRegion;
    theRegion = it.getNext();
    while (theRegion)
    {
        if (m_Type == theRegion->m_eDriveType && m_u32Tag == theRegion->m_wTag)
        {
            if (regionArray)
            {
                regionArray[dataRegionsCount] = theRegion;
            } /* Endif */

            dataRegionsCount++;
        } /* Endif */
        theRegion = it.getNext();
    } /* Endwhile */
    
    if (regionCount)
    {
        *regionCount = dataRegionsCount;
    } /* Endif */
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : buildRegionsList
* Returned Value   : void
* Comments         :
*   This function will build the list of data regions based upon the
*   number of regions.
*
*END*--------------------------------------------------------------------*/
void DataDrive::buildRegionsList()
{ /* Body */
    /* Scan once to get the number of data regions for this drive. */
    unsigned dataRegionsCount = 0;
    processRegions(NULL, &dataRegionsCount);
    
    /* Allocate an array of region pointers large enough to hold all of our regions. */
    m_u32NumRegions = dataRegionsCount;
    m_ppRegion = new Region*[dataRegionsCount];
    assert(m_ppRegion);
    
    /* Scan again to fill in the region pointer array. */
    processRegions(m_ppRegion, NULL);
    
    Region * pRegion;
    int iNumSectorsPerBlk = NandHal::getParameters().wPagesPerBlock;
    uint32_t u32TotalLogicalSectors = 0; /* Logical "native" sectors */
    Region::Iterator it(m_ppRegion, m_u32NumRegions);
    pRegion = it.getNext();
    while (pRegion)
    {
        /* 
        ** As far as the mapper is concerned, all these blocks can be allocated
        ** However, some of these blocks could go bad so... 
        */
        pRegion->m_u32NumLBlks = pRegion->m_iNumBlks - pRegion->getBadBlockCount();
        
        u32TotalLogicalSectors += (pRegion->m_u32NumLBlks * iNumSectorsPerBlk);
        
        pRegion = it.getNext();
    } /* Endwhile */
    
    /* Subtract out the reserved blocks but only for the Data Drive which is large. */
    if (m_Type == kDriveTypeData)
    {
        u32TotalLogicalSectors -= m_media->getReservedBlockCount() * iNumSectorsPerBlk;
        
        /* Also subtract out the number of blocks reserved for maps by the mapper. */
        u32TotalLogicalSectors -= kNandMapperReservedBlockCount * iNumSectorsPerBlk;
    } /* Endif */
    
    /* 
    ** Update the native sector count and recompute the total drive size using the
    ** total logical sector count. 
    */
    m_numberOfNativeSectors = u32TotalLogicalSectors;
    m_u64SizeInBytes = (uint64_t)u32TotalLogicalSectors * (uint64_t)m_nativeSectorSizeInBytes;
    
    /* Convert native to nominal sectors. */
    m_u32NumberOfSectors = m_numberOfNativeSectors << m_nativeSectorShift;
} /* Endbody */

/* EOF */
