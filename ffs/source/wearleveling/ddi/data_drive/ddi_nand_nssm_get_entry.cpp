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
 * $FileName: ddi_nand_nssm_get_entry.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains the utilities needed to handle LBA ddi layer functions.
 *
 *END************************************************************************/
#include "wl_common.h"
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/data_drive/ddi_nand_data_drive.h"
#include "ddi_nand_hal.h"
#include <string.h>
#include "ddi/mapper/mapper.h"
#include "buffer_manager/media_buffer.h"

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getMapForVirtualBlock
* Returned Value   : The map of the requested virtual block number. 
*   If NULL is returned, then an unrecoverable error occurred.
* Comments         :
*   Get the appropriate Non-Sequential Sector Map.
*   This function will return the NonSequential Sector map entry for the given
*   LBA Block.  If the NSSM is not in the table, then build it from the data 
*   in the NAND.
*
*END*--------------------------------------------------------------------*/
NonsequentialSectorsMap * NssmManager::getMapForVirtualBlock
(
    /* [IN] Virtual block number to search for. */
    uint32_t blockNumber
)
{ /* Body */
    RtStatus_t  ret;

    /* Use the index to search for a matching map. */
    NonsequentialSectorsMap * map = static_cast<NonsequentialSectorsMap *>(m_index.find(blockNumber));
    if (map)
    {
        /* Reinsert the map in LRU order. */
        map->removeFromLRU();
        map->insertToLRU();
        
        return map;
    } /* Endif */

    /* If it wasn't found, we'll need to build it. */
    ret = buildMap(blockNumber, &map);
    if (ret != SUCCESS)
    {
        /* Something bad happened... */
        return NULL;
    } /* Endif */

    /* Insert the newly built map into the LRU list. */
    map->insertToLRU();
    return map;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getEntry
* Returned Value   : Status of call or error.
* Comments         :
*   This function gets the physical sector in the remapped LBA corresponding
*   to a given linear sector offset.  Basically this function returns the
*   byte array entry of the linear sector offset which is the remapped
*   sector.
*
*END*--------------------------------------------------------------------*/
RtStatus_t NonsequentialSectorsMap::getEntry
(
    /* [IN] Logical sector index */
    uint32_t u32LBASectorIdx,
    
    /* [OUT] Physical block number */
    uint32_t * pu32PhysicalBlockNumber,
    
    /* [OUT] Sector offset of the actual sector. */
    uint32_t * pu32NS_ActualSectorOffset,
    
    /* [OUT] State of the sector */
    uint32_t * pu32Occupied
)
{ /* Body */
    uint32_t u32SectorOffset;

    /* 
    ** The LBA was found above so now using the linear expected LBA sector,
    ** grab the value in the NonSequential Map Sector. 
    */
    u32SectorOffset = m_map[u32LBASectorIdx];
    *pu32Occupied = m_map.isOccupied(u32LBASectorIdx);

    /* See if We have a back-up block. */
    if (!(*pu32Occupied) && (m_backupPhysicalBlock != kInvalidAddress))
    {
        /* 
        ** If we have a back-up block, return back-up block's physical block 
        ** number and physical offset into back-up block. 
        */
        u32SectorOffset = m_backupMap[u32LBASectorIdx];
        *pu32Occupied = m_backupMap.isOccupied(u32LBASectorIdx);
        
        *pu32PhysicalBlockNumber = m_backupPhysicalBlock;
    } /* Endif */
    
    *pu32NS_ActualSectorOffset = u32SectorOffset;

    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : insertToLRU
* Returned Value   : void
* Comments         :
*   This function inserts the nonsequentialsectormap object to list
*
*END*--------------------------------------------------------------------*/
void NonsequentialSectorsMap::insertToLRU()
{ /* Body */
    m_manager->m_lru.insert(this);
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : removeFromLRU
* Returned Value   : void
* Comments         :
*   This function removes a nonsequentialsectormap object from list
*
*END*--------------------------------------------------------------------*/
void NonsequentialSectorsMap::removeFromLRU()
{ /* Body */
    m_manager->m_lru.remove(this);
} /* Endbody */

/* EOF */
