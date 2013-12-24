#if !defined(__persistent_phy_map_h__)
#define __persistent_phy_map_h__
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
 * $FileName: persistent_phy_map.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains declaration of the persistent phy map class.
 *
 *END************************************************************************/

#include "persistent_map.h"

/* 
** Class definations 
*/

namespace nand {

class Mapper;
class PhyMap;

/* Handles storage of a PhyMap on the NAND. */
class PersistentPhyMap : public PersistentMap
{
public:

    /* Constructor. */
    PersistentPhyMap(Mapper & mapper);
    
    /* Destructor. */
    virtual ~PersistentPhyMap();
    
    /* Initializer. */
    void init();
    
    /* Finds and loads the map. */
    RtStatus_t load();
    
    /* Saves the map into the current block, consolidating if necessary. */
    RtStatus_t save();
    
    /* Allocates a new block and writes the map to it. */
    RtStatus_t saveNewCopy();
    
    PhyMap * getPhyMap();
    void setPhyMap(PhyMap * theMap);

    /* set&get of trust element */
    inline void setTrustFlag(bool trustFlag) {
        m_trustFlag = trustFlag;
    }

    inline bool getTrustFlag() {
        return m_trustFlag;
    }

    inline void setTrustNumber(uint16_t trustNumber) {
        m_trustNumber = trustNumber;
    }

    inline uint16_t getTrustNumber() {
        return m_trustNumber;
    }

protected:
    /* The map that is being persisted. */
    PhyMap * m_phymap;

    /* True if we're in the middle of loading the phymap. */
    bool m_isLoading;   

    virtual RtStatus_t getSectionForConsolidate(
    uint32_t u32EntryNum,
    uint32_t thisSectionNumber,
    uint8_t *& bufferToWrite,
    uint32_t & bufferEntryCount,
    uint8_t * sectorBuffer);

    
};

} /* namespace nand */

#endif /* __persistent_phy_map_h__ */

/* EOF */
