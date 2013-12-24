#if !defined(__ddi_nand_block_h__)
#define __ddi_nand_block_h__
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
 * $FileName: ddi_block.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains a class to wrap a block of a NAND.
 *
 *END************************************************************************/

#include "ddi_nand_hal.h"

/* 
** Class definations 
*/

namespace nand
{

    /* Constant for the first page offset within a block. */
    const uint32_t kFirstPageInBlock = 0;

    /* Representation of one block of a NAND. */
    class Block : public BlockAddress
    {
    public:
        /* Init and cleanup */
        /* Default constructor, inits to block 0. */
        Block();
        
        explicit Block(const BlockAddress & addr);
        
        /* Assignment operator. */
        Block & operator = (const Block & other);
        
        /* Assignment operator to change block address. */
        Block & operator = (const BlockAddress & addr);

        /* Assignment operator to change block address. */
        Block & operator = (const PageAddress & page);

        /* Addresses */
        void set(const BlockAddress & addr);

        /* Prefix increment operator to advance the address to the next block. */
        Block & operator ++ ();
        
        /* Prefix decrement operator. */
        Block & operator -- ();
        
        /* Increment operator. */
        Block & operator += (uint32_t amount);
        
        /* Decrement operator. */
        Block & operator -= (uint32_t amount);
        
        /* Accessors */
        /* Returns the number of pages in this block. */
        inline unsigned getPageCount() const { return m_nand->pNANDParams->wPagesPerBlock; }
        
        /* Returns the NAND object owning this block. */
        inline NandPhysicalMedia * getNand() const { return m_nand; }

        /* Operations */
        RtStatus_t readPage(unsigned pageOffset, SECTOR_BUFFER * buffer, SECTOR_BUFFER * auxBuffer);

        RtStatus_t readMetadata(unsigned pageOffset, SECTOR_BUFFER * buffer);
        
        RtStatus_t writePage(unsigned pageOffset, const SECTOR_BUFFER * buffer, SECTOR_BUFFER * auxBuffer);
        
        /* Erase this block. */
        RtStatus_t erase();
        
        /* Test whether the block is marked bad. */
        bool isMarkedBad(SECTOR_BUFFER * auxBuffer=NULL, RtStatus_t * status=NULL);
        
        /* Erase this block and mark it bad. */
        RtStatus_t markBad();
        
        /* 
        ** Erase the block and mark it bad if the erase fails.
        **
        ** If the erase fails, then the erase error code will be returned even if marking the block
        ** bad succeeded. This lets the caller know not to use the block. 
        */
        RtStatus_t eraseAndMarkOnFailure();
        
        /* Tests whether the block is already erased. */
        bool isErased();

    protected:
        /* The physical NAND owning this block. */
        NandPhysicalMedia * m_nand;
    };

} /* namespace nand */

#endif /* __ddi_nand_block_h__ */

/* EOF */
