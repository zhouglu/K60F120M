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
 * $FileName: ddi_nand_mapper_get_info.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains common NAND Logical Block Address Mapper functions.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "simple_timer.h"
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/data_drive/ddi_nand_data_drive.h"
#include "ddi_nand_hal.h"
#include "ddi_media.h"
#include "ddi/mapper/mapper.h"
#include <string.h>
#include "buffer_manager/media_buffer.h"
#include <stdlib.h>
#include "ddi/mapper/zone_map_cache.h"

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getBlockInfo
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function returns the Physical block address of an LBA
*
*END*--------------------------------------------------------------------*/
RtStatus_t ZoneMapCache::getBlockInfo
(
    /* [IN] Logical Block Address to inquire about. */
    uint32_t u32Lba, 
    
    /* [OUT] Physical Block address corresponding to LBA. */
    uint32_t * pu32PhysAddr
)
{ /* Body */
    int32_t i32SelectedEntryNum;
    RtStatus_t ret;

    assert(m_block.isValid());
    assert(m_topPageIndex);

    ret = lookupCacheEntry(u32Lba, &i32SelectedEntryNum);
    if (ret)
    {
        return ret;
    } /* Endif */

    ret = evictAndLoad(u32Lba, i32SelectedEntryNum);
    if (ret)
    {
        return ret;
    } /* Endif */

    /* Construct the entry's address from the 24-bit value. */
    *pu32PhysAddr = readMapEntry(&m_descriptors[i32SelectedEntryNum], u32Lba);
    
    /* Update the timestamp. */
    m_descriptors[i32SelectedEntryNum].m_timestamp = wl_hw_get_microsecond();
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getBlockInfo
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function returns Physical address of a Logical block
*
*END*--------------------------------------------------------------------*/
RtStatus_t Mapper::getBlockInfo
(   
    /* [IN] Logical block address */
    uint32_t u32Lba, 
    
    /* [OUT] Physical block address */
    uint32_t * pu32PhysAddr)
{ /* Body */
    /* Make sure that we are initialized */
    if( !m_isInitialized )
    {
        return ERROR_DDI_NAND_MAPPER_UNITIALIZED;
    } /* Endif */

    return m_zoneMap->getBlockInfo(u32Lba, pu32PhysAddr);
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : readMapEntry
* Returned Value   : The physical address associated with lba is returned.
* Comments         :
*   This function reads a value from a zone map section.
*
*END*--------------------------------------------------------------------*/
uint32_t ZoneMapCache::readMapEntry
(
    /* [IN] The section of the zone map containing the entry for lba. */
    CacheEntry * zoneMapSection, 
    
    /* [IN] The true LBA number being modified. zoneMapSection must contain this LBA. */
    uint32_t lba
)
{ /* Body */
    uint32_t u32StartingEntry = zoneMapSection->m_firstLBA;    
    uint8_t * entries = zoneMapSection->m_entries;
    uint32_t entryIndex = lba - u32StartingEntry;
    uint32_t physicalAddress;
    
    /* Handle the different entry sizes. */
    switch (m_entrySize)
    {
        /* 16-bit entries */
        case kNandZoneMapSmallEntry:
            /* We can simply read the value directly from the array. */
            physicalAddress = ((uint16_t *)entries)[entryIndex];
            break;

            /* 24-bit entries */
        case kNandZoneMapLargeEntry:
            /* Advance the entry pointer to the entry we are reading. */
            entries += entryIndex * kNandZoneMapLargeEntry;

            /* Construct the entry's physical address from the 24-bit value. */
            physicalAddress = entries[0] | (entries[1] << 8) | (entries[2] << 16);
            
            break;
    } /* Endswitch */
    
    return physicalAddress;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getPageInfo
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function returns the Physical block address and the logical page offset
*   of the logical sector address to the caller. 
*
*END*--------------------------------------------------------------------*/
RtStatus_t Mapper::getPageInfo
(
    /* [IN] Logical page logical address */
    uint32_t u32PageLogicalAddr,
    
    /* [OUT] Logical Block address */
    uint32_t *pu32LogicalBlkAddr,
    
    /* [OUT] Physical blk address */
    uint32_t *pu32PhysBlkAddr,
    
    /* [OUT] Logical page offset */
    uint32_t *pu32LbaPageOffset
)
{ /* Body */
    RtStatus_t ret;

    /* Make sure that we are initialized */
    if (!m_isInitialized)
    {
        return ERROR_DDI_NAND_MAPPER_UNITIALIZED;
    } /* Endif */

    /* Convert logical sector address to logical block number and logical page offset. */
    NandHal::getFirstNand()->pageToBlockAndOffset(u32PageLogicalAddr, pu32LogicalBlkAddr, pu32LbaPageOffset);

    /* Make sure that we are not go out of bound */
    if (*pu32LogicalBlkAddr > m_media->getTotalBlockCount())
    {
        /* logical page address is out of range */
        return ERROR_DDI_NAND_MAPPER_PAGE_OUTOFBOUND;  
    } /* Endif */

    /* Get the True LBA blk addr from the logical lba (may be offset by Total Blocks for HDD) */
    ret = getBlockInfo(*pu32LogicalBlkAddr, pu32PhysBlkAddr);
    if (ret)
    {
        return ret;
    } /* Endif */
    
    if (*pu32PhysBlkAddr >= m_media->getTotalBlockCount())
    {
        /* This also catches unallocated blocks, i.e. g_MapperDescriptor.unallocatedBlockAddress. */
        return ERROR_DDI_NAND_MAPPER_INVALID_PHYADDR;
    } /* Endif */

    return SUCCESS;
} /* Endbody */

/* EOF */
