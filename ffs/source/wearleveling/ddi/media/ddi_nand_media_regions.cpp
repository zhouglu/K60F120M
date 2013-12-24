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
 * $FileName: ddi_nand_media_regions.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains functions used to manipulate and manage NAND regions
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi/common/ddi_nand_ddi.h"
#include <string.h>
#include "ddi/media/ddi_nand_media.h"

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : getRegionForBlock
* Returned Value   : A pointer to the region holding the physicalBlock is returned.
* Comments         :
*   This function  will return the region pointer parameter passed into it
*   by checking the physical sector parameter against the region boundaries.
*
*END*--------------------------------------------------------------------*/
Region * Media::getRegionForBlock
(
    /* [IN] Absolute physical block to match. */
    const BlockAddress & physicalBlock
)
{ /* Body */
    uint32_t u32AbsoluteOffset;
    int32_t i32NumBlocksInRegion;
    Region * pRegionInfo;  
    
    /* Search through all the regions */
    Region::Iterator it = createRegionIterator();
    pRegionInfo = it.getNext();
    while (pRegionInfo)
    {
        u32AbsoluteOffset = pRegionInfo->m_u32AbPhyStartBlkAddr;
        i32NumBlocksInRegion = pRegionInfo->m_iNumBlks;
        
        /* 
        ** We don't need to check for greater than start block address
        ** because we're scanning sequentially. 
        */
        if ((physicalBlock < (u32AbsoluteOffset + i32NumBlocksInRegion))
                && (physicalBlock >= u32AbsoluteOffset))
        {
            /* Return a pointer to the region found */
            return pRegionInfo;
        } /* Endif */
        pRegionInfo = it.getNext();
    } /* Endwhile */
    
    return NULL;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : setDirty
* Returned Value   : void
* Comments         :
*   This function marks region as dirty
*
*END*--------------------------------------------------------------------*/
void Region::setDirty()
{ /* Body */
    /* Figure out how and when to clear region dirty flag, or if it's even necessary. */
    m_bRegionInfoDirty = true;
} /* Endbody */

/* EOF */
