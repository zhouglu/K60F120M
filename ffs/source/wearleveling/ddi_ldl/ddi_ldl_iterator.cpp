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
 * $FileName: ddi_ldl_iterator.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains utilities used by the logical drive layer.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "ddi_media_internal.h"
#include "ddi_media.h"
#include "ddi_media_errordefs.h"
#include <stdlib.h>

/* 
** Type definations 
*/


/* Internal drive iterator structure. */
struct OpaqueDriveIterator
{
    /* Index of the next drive to return the tag of. */
    unsigned nextIndex;
};

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : DriveCreateIterator
* Returned Value   : SUCCESS or ERROR
*   - SUCCESS
*   - ERROR_DDI_LDL_GENERAL
* Comments         :
*   This function creates drive iterator 
*
*END*--------------------------------------------------------------------*/
RtStatus_t DriveCreateIterator
(
    /* [IN/OUT] Drive iterator pointer */
    DriveIterator_t * iter
)
{ /* Body */
    /* Allocate an iterator. */
    // TODO: use _wl_mem_alloc
    DriveIterator_t newIter = (DriveIterator_t)_wl_mem_alloc(sizeof(OpaqueDriveIterator));
    // DriveIterator_t newIter = (DriveIterator_t)malloc(sizeof(OpaqueDriveIterator));
    if (newIter == NULL)
    {
        return ERROR_DDI_LDL_GENERAL;
    } /* Endif */
    
    /* Set up iterator. */
    newIter->nextIndex = 0;
    
    /* Return the new iterator to the caller. */
    *iter = newIter;
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : DriveIteratorNext
* Returned Value   : SUCCESS or ERROR
*   - SUCCESS
*   - ERROR_DDI_LDL_ITERATOR_DONE
* Comments         :
*   This function returns the next drive
*
*END*--------------------------------------------------------------------*/
RtStatus_t DriveIteratorNext
(
    /* [IN/OUT] Drive iterator */
    DriveIterator_t iter, 

    /* [IN] Drive tag */
    DriveTag_t * tag
)
{ /* Body */
    assert(iter);
    
    /* 
    ** Return the tag for this index to the caller 
    ** and increment the iterator's index for the next time through. 
    */
    while (iter->nextIndex < MAX_LOGICAL_DRIVES)
    {
        LogicalDrive * drive = g_ldlInfo.m_drives[iter->nextIndex++];
        
        /* Skip drives that do not have a valid API table set. */
        if (drive)
        {
            *tag = drive->getTag();
            break;
        }
    } /* Endwhile */
    
    /* Check if all drives have been returned through this iterator. */
    if (iter->nextIndex >= MAX_LOGICAL_DRIVES)
    {
        return ERROR_DDI_LDL_ITERATOR_DONE;
    } /* Endif */
    
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : DriveIteratorDispose
* Returned Value   : void
* Comments         :
*   This function disposes drive iterator
*
*END*--------------------------------------------------------------------*/
void DriveIteratorDispose
(
/* [IN/OUT] Drive iterator pointer */
DriveIterator_t iter
)
{ /* Body */
    if (iter)
    {
        // TODO: use _wl_mem_free
        _wl_mem_free(iter);
        // free(iter);
    } /* Endif */
} /* Endbody */

/* EOF */
