#if !defined(__ddi_nand_hal_nfcphymedia_h__)
#define __ddi_nand_hal_nfcphymedia_h__
/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
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
 * $FileName: ddi_nand_hal_nfcphymedia.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   Header for Nand Physical Media for NFC.
 *
 *END************************************************************************/

class NFCNandMedia: public NandPhysicalMedia {
public:
    typedef struct {
        uint_32 ID;
    } NFCNandID_t;
    
    NFCNandMedia(uint32_t chipNumber) ;

    virtual ~NFCNandMedia() {}

    void initChipParam();
    void initHalContextParams();
    virtual RtStatus_t reset() { return SUCCESS;}
    virtual RtStatus_t readID(uint8_t * pReadIDCode);
    virtual RtStatus_t readRawData(uint32_t wSectorNum, uint32_t columnOffset,
    uint32_t readByteCount, SECTOR_BUFFER * pBuf);
    virtual RtStatus_t readPage(uint32_t uSectorNumber, SECTOR_BUFFER * pBuffer,
    SECTOR_BUFFER * pAuxiliary);
    virtual RtStatus_t readMetadata(uint32_t uSectorNumber, SECTOR_BUFFER * pBuffer) ;
    virtual RtStatus_t writeRawData(uint32_t pageNumber, uint32_t columnOffset,
    uint32_t writeByteCount, const SECTOR_BUFFER * data);
    virtual RtStatus_t writePage(uint32_t uSectorNum, const SECTOR_BUFFER * pBuffer,
    const SECTOR_BUFFER * pAuxiliary) ;
    virtual RtStatus_t writeFirmwarePage(uint32_t uSectorNum,
    const SECTOR_BUFFER * pBuffer, const SECTOR_BUFFER * pAuxiliary) ;
    virtual RtStatus_t readFirmwarePage(uint32_t uSectorNumber,
    SECTOR_BUFFER * pBuffer, SECTOR_BUFFER * pAuxiliary) ;

    virtual RtStatus_t eraseBlock(uint32_t uBlockNumber) ;
    virtual RtStatus_t eraseMultipleBlocks(uint32_t startBlockNumber,
    uint32_t requestedBlockCount, uint32_t * actualBlockCount) ;
    virtual RtStatus_t copyPages(NandPhysicalMedia * targetNand,
    uint32_t wSourceStartSectorNum, uint32_t wTargetStartSectorNum,
    uint32_t wNumSectors, SECTOR_BUFFER * sectorBuffer,
    SECTOR_BUFFER * auxBuffer, NandCopyPagesFilter * filter,
    uint32_t * successfulPages);
    virtual bool isBlockBad(uint32_t blockAddress, SECTOR_BUFFER * auxBuffer,
    bool checkFactoryMarkings = false, RtStatus_t * readStatus = NULL);
    virtual RtStatus_t markBlockBad(uint32_t blockAddress, SECTOR_BUFFER * pageBuffer,
    SECTOR_BUFFER * auxBuffer);
    
    virtual RtStatus_t enableSleep(bool isEnabled) { return SUCCESS; }
    virtual bool isSleepEnabled()  { return false; }
    virtual char * getDeviceName(){ return NULL; }

    // TODO: add getAuxSize function to get aux space
    uint32_t getAuxSize() { return pNANDParams->pageMetadataSize; }
    RtStatus_t checkSpaceForMetadata();



protected:
    MQX_FILE_PTR m_fd_ptr;
};

#endif /*  __ddi_nand_hal_nfcphymedia_h__ */

/* EOF */
