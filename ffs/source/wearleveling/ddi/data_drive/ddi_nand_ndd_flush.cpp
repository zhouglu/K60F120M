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
 * $FileName: ddi_nand_ndd_flush.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file handles flushing and shutdown of the data drive.
 *
 *END************************************************************************/
#include "ddi/common/ddi_nand_ddi.h"
#include "ddi/data_drive/ddi_nand_data_drive.h"
#include "ddi/mapper/mapper.h"
#include "ddi_media_internal.h"
#include <string.h>

using namespace nand;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : flush
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   Flush the data drive.
*
*END*--------------------------------------------------------------------*/
RtStatus_t DataDrive::flush()
{ /* Body */
    /* Make sure we're initialized */
    if (!m_bInitialized)
    {
        return ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED;
    } /* Endif */

    DdiNandLocker locker;

    /* Flush NSSMs. */
    m_media->getNssmManager()->flushAll();
    
    /* Flush the mapper. */
    return m_media->getMapper()->flush();
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : shutdown
* Returned Value   : SUCCESS or an error from flushing the mapper.
* Comments         :
*   Do shutdown steps which are only to be called once, during shutdown.
*   This includes flushing the non-sequential sectors map, the zone map, and
*   the phy map to the NAND.
*
*END*--------------------------------------------------------------------*/
RtStatus_t DataDrive::shutdown()
{ /* Body */
    /* Make sure we're initialized */
    if (!m_bInitialized)
    {
        return ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED;
    } /* Endif */
    
    /* Flush everything. */
    flush();
    
    /* Free the region pointer array. */
    delete [] m_ppRegion;
    m_ppRegion = NULL;
    m_u32NumRegions = 0;
    
    m_bInitialized = false;

    return SUCCESS;
} /* Endbody */

/* EOF */
