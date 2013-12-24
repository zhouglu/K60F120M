#if !defined(__persistent_map_h__)
#define __persistent_map_h__
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
 * $FileName: persistent_map.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains definition of the nand::PersistentMap class.
 *
 *END************************************************************************/

#include "ddi_block.h"
#include "page_order_map.h"

namespace nand
{

/* Forward declarations */
class Mapper;
class ZoneMapSectionPage;

/*
** Base class for a map that is stored on the NAND.
**
** This class implements a map composed of integer entries that is broken into one or
** more sections, each the size of a NAND page. The map is stored on the NAND in an
** efficient manner, by writing sections sequentially to pages within a block.
**
** As a new version of a section becomes available, it is written to the next page in the
** block. The sections can be in any order in the block, and there can be multiple copies
** of any given section, but only the most recent copy of a section will be recognized.
** Only when the block is completely full, with no free pages, will the map be copied
** (consolidated) to a new block.
**
** The content for sections of the map is not handled by this class. It is the
** responsibility of subclasses or users of the class to provide that content.
**
** Right now, this class only supports storing the map within a single block. But it is
** possible that in the future this restriction may be relaxed, in order to store maps
** that are larger than will fit within one block.
*/
class PersistentMap
{
public:

    /* Default constructor. */
    PersistentMap(Mapper & mapper, uint32_t mapType, uint32_t metadataSignature);
    
    /* Destructor. */
    virtual ~PersistentMap();
    
    /* Initializer. */
    void init(int entrySize, int entryCount);
    
    /* Does the given block belong to this map? */
    bool isMapBlock(const BlockAddress & address) { return m_block == address; }

    /* Get block holding PhyMap or ZoneMap */
    uint32_t getMapBlock() { 
        return m_block.get(); 
    }

    /* Rebuild the map into a new block. */
    virtual RtStatus_t consolidate(
    bool hasValidSectionData=false,
    uint32_t sectionNumber=0,
    uint8_t * sectionData=NULL,
    uint32_t sectionDataEntryCount=0);
    
    /* Write an updated section of the map. */
    RtStatus_t addSection(
    uint8_t *pMap,
    uint32_t u32StartingEntryNum,
    uint32_t u32NumEntriesToWrite);

    /* Load section of the map. */
    RtStatus_t retrieveSection(
    uint32_t u32EntryNum,
    uint8_t *pMap,
    bool shouldConsolidateOnRewriteSectorError);

    /* TRUST helper */
    /* Number is saved to metadata */
    uint16_t m_trustNumber; 
    /* A flag to ask lower classes to save data with TRUST number */
    bool m_trustFlag; 

protected:
    /* Our parent mapper instance. */
    Mapper & m_mapper;      
    
    /* The block containing this map. */
    BlockAddress m_block;   
    
    /* Size of each map entry in bytes. */
    int m_entrySize;        
    
    /* Number of entries that fit in one NAND page. */
    int m_maxEntriesPerPage;  

    /* The map type signature. */
    uint32_t m_signature;   
    
    /* A signature written into the metadata of each map section page. */
    uint32_t m_metadataSignature;   
    
    /* Number of sections currently in the map's block. */
    int m_topPageIndex;  

    /* Total number of entries in the entire map. */
    int m_totalEntryCount;  
    
    /* Total number of sections in the entire map. */
    int m_totalSectionCount;    
    
    /* Map from zone map section number to page offset within the zone map block. */
    PageOrderMap m_sectionPageOffsets;   
    
    /* Set to true if addSection() does a consolidate. */
    bool m_didConsolidateDuringAddSection;  
    int m_buildReadCount;

    /* Scan the map's block an build the section offset table. */
    RtStatus_t buildSectionOffsetTable();
    
    /* Do a binary search to find the first empty page. */
    RtStatus_t findTopPageIndex(ZoneMapSectionPage & mapPage, bool & needsRewrite);
    
    /* Scan to find the most recent copies of each section. */
    RtStatus_t fillUnknownSectionOffsets(ZoneMapSectionPage & mapPage, bool & needsRewrite);

    /* 
    ** Read a section during consolidation.
    **
    ** The default implementation simply uses retrieveSection() to load the data.
    ** Having this function virtual makes it possible for subclasses to override and provide
    ** additional methods for obtaining the section data, for instance from a cache. 
    */
    virtual RtStatus_t getSectionForConsolidate(
    uint32_t u32EntryNum,
    uint32_t thisSectionNumber,
    uint8_t *& bufferToWrite,
    uint32_t & bufferEntryCount,
    uint8_t * sectorBuffer);
    
};

} /* namespace nand */

#endif /* __persistent_map_h__ */

/* EOF */
