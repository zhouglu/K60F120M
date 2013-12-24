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
 * $FileName: bad_block_table.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains details of BadBlockTable class
 *
 *END************************************************************************/
#include "ddi_block.h"
#include "bad_block_table.h"
#include <string.h>

using namespace nand;

#if defined(__ghs__)
#pragma ghs section text=".init.text"
#endif 

#if defined (__CODEWARRIOR__)
#endif 

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : N/A
* Returned Value   : N/A
* Comments         :
*   Constructor
*
*END*--------------------------------------------------------------------*/
BadBlockTable::BadBlockTable() 
{ /* Body */
    m_entries = NULL;
    m_size = 0;
    m_numberBadBlocks = 0;
    m_isAllocated = false;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : N/A
* Returned Value   : N/A
* Comments         :
*   Destructor calls release function
*
*END*--------------------------------------------------------------------*/
BadBlockTable::~BadBlockTable() 
{ /* Body */
    release();
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : allocate
* Returned Value   : MQX_OK or an error code
* Comments         :
*   This function tests all task queues for correctness
*
*END*--------------------------------------------------------------------*/
RtStatus_t BadBlockTable::allocate
(
    /* [IN] Number entries are in the bitmap */
    uint32_t numberBlocks
) 
{ /* Body */
    /* Shouldn't try to reallocate with first calling release(). */
    assert(!m_entries);
    
    /* Only actually allocate if the entry count is nonzero. */
    if (numberBlocks) 
    {
        /* Allocate the entry array. */
        m_entries = (uint8_t *)_wl_mem_alloc_zero(sizeof(uint8_t) * numberBlocks);
        if (m_entries == NULL) 
        {
            return ERROR_OUT_OF_MEMORY;
        } /* Endif */
        m_size = numberBlocks;
        m_isAllocated = true;
    } /* Endif */
    
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : scanAndBuildBBT
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function scans device and build Bad block table for used memory area
*
*END*--------------------------------------------------------------------*/
RtStatus_t BadBlockTable::scanAndBuildBBT() 
{ /* Body */
    for (uint32_t i = 0; i < m_size; i++) 
    {
        Block blk = Block(i);
        if (!blk.isMarkedBad()) 
        {
            m_entries[i] = 1;
        }
        else
        {
            m_numberBadBlocks++;
        }/* Endif */
    } /* Endfor */
    
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : release
* Returned Value   : void
* Comments         :
*   This function releases all allocated memory
*
*END*--------------------------------------------------------------------*/
void BadBlockTable::release() 
{ /* Body */
    if (m_entries) 
    {
        _wl_mem_free(m_entries);
        m_entries = NULL;
    } /* Endif */
    m_size = 0;
    m_numberBadBlocks = 0;
    m_isAllocated = false;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : isBlockBad
* Returned Value   : TRUE or FALSE
* Comments         :
*   This function returns state of a block
*
*END*--------------------------------------------------------------------*/
bool BadBlockTable::isBlockBad
(
    /* [IN] Block address */
    uint32_t blockAddress
) 
{ /* Body */
    assert(m_entries);
    assert(blockAddress < m_size);

    bool ret =  (m_entries[blockAddress] == 0);
    return ret;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getEntry
* Returned Value   : Block address
* Comments         :
*   This function returns block address depending on index
*
*END*--------------------------------------------------------------------*/
uint8_t BadBlockTable::getEntry
(
    /* [IN] Index in array */
    uint32_t index
) 
{ /* Body */
    assert(index < m_size);
    
    return m_entries[index];
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : markBad
* Returned Value   : void
* Comments         :
*   This function marks a block as bad
*
*END*--------------------------------------------------------------------*/
void BadBlockTable::markBad
(
    /* [IN] Block address */
    uint32_t blockAddress
) 
{ /* Body */
    assert(blockAddress < m_size);

    m_entries[blockAddress] = 0;
    m_numberBadBlocks++;
} /* Endbody */

#if defined(__ghs__)
#pragma ghs section text=default
#endif 

#if defined (__CODEWARRIOR__)
#endif 

/* EOF */
