#ifndef __ddi_nand_data_drive_h__
#define __ddi_nand_data_drive_h__
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
 * $FileName: ddi_nand_data_drive.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains definitions of the NAND data drive class.
 *
 *END************************************************************************/

#include "ddi_nand_media.h"
#include "ddi_nand_ddi.h"
#include "nonsequential_sectors_map.h"

/* 
** Class definations 
*/

namespace nand {

/* NAND data drive. */
class DataDrive : public LogicalDrive
{
public:
    
    /* Default constructor. */
    DataDrive(Media * media, Region * region);
    
    /* Destructor. */
    virtual ~DataDrive();

    void addRegion(Region * region);
    
    /* Logical drive API */
    virtual RtStatus_t init();
    virtual RtStatus_t shutdown();
    virtual RtStatus_t getInfo(uint32_t infoSelector, void * value);
    virtual RtStatus_t setInfo(uint32_t infoSelector, const void * value);
    virtual RtStatus_t readSector(uint32_t sector, SECTOR_BUFFER * buffer);
    virtual RtStatus_t writeSector(uint32_t sector, const SECTOR_BUFFER * buffer);
    virtual RtStatus_t erase();
    virtual RtStatus_t flush();
    virtual RtStatus_t repair(bool bIsScanBad = false);

protected:
    /* The NAND media object that we belong to. */
    Media * m_media;
    uint32_t m_u32NumRegions;
    Region ** m_ppRegion;

    void processRegions(Region ** regionArray, unsigned * regionCount);
    void buildRegionsList();

    bool shouldRepairEraseBlock(uint32_t u32BlockFirstPage, NandPhysicalMedia *pNandDesc);

    RtStatus_t logicalBlockToVirtual(uint32_t logicalBlock, Region ** region, uint32_t * virtualBlock);
    RtStatus_t logicalSectorToVirtual(uint32_t logicalSector, Region ** region, uint32_t * virtualSector);

    RtStatus_t readSectorInternal(uint32_t u32LogicalSectorNumber, SECTOR_BUFFER * pSectorData);
    RtStatus_t writeSectorInternal(uint32_t u32LogicalSectorNumber, const SECTOR_BUFFER * pSectorData);

    bool isBlockHidden(uint32_t u32BlockPhysAddr);
};

} /* namespace nand */

/* 
** Function prototypes 
*/

void log_ecc_failures(nand::Region * pRegion, uint32_t wPhysicalBlockNumber, uint32_t wSectorOffset);

#endif /* __ddi_nand_data_drive_h__ */

/* EOF */
