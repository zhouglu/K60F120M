#if !defined(__badblocktable_h__)
#define __badblocktable_h__
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
 * $FileName: bad_block_table.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains BadBlockTable class defination
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi_nand_hal.h"

/* 
** Class definations 
*/

namespace nand 
{
    class BadBlockTable 
    {
    public:
        /* Default constructor. */
        BadBlockTable();
        
        /* Destructor. */
        ~BadBlockTable();
        
        /* Allocate memory for input number of blocks. */
        RtStatus_t allocate(uint32_t numberBlocks);

        RtStatus_t scanAndBuildBBT();
        
        /* Free all memory owned by the object. */
        void release();
        
        bool isBlockBad(uint32_t blockAddress);    

        uint8_t * getEntries() { return m_entries; }

        uint8_t getEntry(uint32_t index);

        bool isAllocated() { return m_isAllocated; }

        void markBad(uint32_t blockAddress);

        uint32_t getNumberBadBlocks() { return m_numberBadBlocks; }

    protected:
        /* Bad blocks bitmap */
        uint8_t * m_entries;
        /* Size of bitmap */
        uint32_t m_size;
        /* Number of bad blocks */
        uint32_t m_numberBadBlocks;
        /* Flag is used to maked global BadBlockTable variable as allocated */
        bool m_isAllocated;
    };
} /* namespace nand */

#endif /* __badblocktable_h__ */

/* EOF */
