#if !defined(__page_h__)
#define __page_h__
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
 * $FileName: ddi_page.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains class to wrap a page of a NAND.
 *
 *END************************************************************************/

#include "ddi_nand_hal.h"
#include "media_buffer.h"
#include "ddi_metadata.h"

/* 
** Class definations 
*/

namespace nand {

/* Representation of one page of a NAND. */
class Page : public PageAddress
{
public:
    /* Init and cleanup */
    /* Default constructor, inits to absolute page 0. */
    Page();
    
    /* Constructor taking a page address. */
    explicit Page(const PageAddress & addr);
    
    /* Constructor taking a block address. */
    explicit Page(const BlockAddress & addr);
    
    /* Copy constructor. */
    explicit Page(const Page & other);
    
    /* Assignment operator. */
    Page & operator = (const Page & other);
    
    /* Destructor. */
    virtual ~Page();
    
    /* Addresses */
    /* Assignment operator. */
    Page & operator = (const PageAddress & addr);
    
    /* Assignment operator. */
    Page & operator = (const BlockAddress & addr);
    
    /* Change the address. */
    void set(const PageAddress & addr);
    
    /* Prefix increment operator to advance the page address to the next page. */
    Page & operator ++ ();
    
    /* Prefix decrement operator. */
    Page & operator -- ();
    
    /* Increment operator. */
    Page & operator += (uint32_t amount);
    
    /* Decrement operator. */
    Page & operator -= (uint32_t amount);
    
    /* Page sizes */
    /* Returns the page's data size in bytes. */
    inline unsigned getDataSize() const { return getNand()->pNANDParams->pageDataSize; }
    
    /* Returns the full page size in bytes. */
    inline unsigned getPageSize() const { return getNand()->pNANDParams->pageTotalSize; }
    
    /* Returns the size of the page's metadata in bytes. */
    inline unsigned getMetadataSize() const { return getNand()->pNANDParams->pageMetadataSize; }
    
    /* Buffers */
    /* Specify the buffers to use for reading and writing. */
    virtual void setBuffers(SECTOR_BUFFER * pageBuffer, SECTOR_BUFFER * auxBuffer);
    
    RtStatus_t allocateBuffers(bool page=true, bool aux=true);
    
    /* Force early release of any buffers that were allocated. */
    void releaseBuffers();
    
    /* Returns the metadata wrapper object for this page. */
    inline Metadata & getMetadata() { return m_metadata; }
    
    /* Returns the page buffer. */
    inline SECTOR_BUFFER * getPageBuffer() { return m_pageBuffer; }
    
    /* Returns the auxiliary buffer. */
    inline SECTOR_BUFFER * getAuxBuffer() { return m_auxBuffer; }
    
    /* Operations */
    /* Read the page into the provided buffer. */
    virtual RtStatus_t read();

    /* Read the page's metadata into the provided auxiliary buffer. */
    virtual RtStatus_t readMetadata();

    /* Write the page contents. */
    virtual RtStatus_t write();
    
    /* Write the page and mark the block bad if the write fails. */
    RtStatus_t writeAndMarkOnFailure();
    
    /* Check if the page is erased. */
    bool isErased(RtStatus_t * status=NULL);

protected:
    NandPhysicalMedia * m_nand;
    MediaBuffer m_pageBuffer;
    MediaBuffer m_auxBuffer;
    Metadata m_metadata;
    
    /* Method to let subclasses know that buffers were changed. */
    virtual void buffersDidChange();
};

/*
** Represents either a firmware or boot block page.
**
** The primary difference between this class and its Page superclass is that
** this one uses a different HAL API to write pages. Instead of using the normal
** #NandPhysicalMedia::writePage(), it uses #NandPhysicalMedia::writeFirmwarePage().
** In addition, it can optionally write the page as raw data, as is required for
** certain boot pages such as the NCB on the STMP3780.
*/
class BootPage : public Page
{
public:
    /* Default constructor. */
    BootPage();

    /* Constructor taking a page address. */
    BootPage(const PageAddress & addr);

    /* Write the page contents. */
    virtual RtStatus_t write();
    
    void setRequiresRawWrite(bool doRawWrite) { m_doRawWrite = doRawWrite; }

protected:
    /* Whether the page must be written raw. */
    bool m_doRawWrite;
};

} /* namespace nand */

#endif /* __page_h__ */

/* EOF */
