/**HEADER**********************************************************************
** Copyright (c) 2013 Freescale Semiconductor, Inc. All rights reserved.
** 
** Freescale Semiconductor, Inc.
** Proprietary & Confidential
** 
** This source code and the algorithms implemented therein constitute
** confidential information and may comprise trade secrets of Freescale Semiconductor, Inc.
** or its associates, and any use thereof is subject to the terms and
** conditions of the Confidential Disclosure Agreement pursual to which this
** source code was originally received.
**************************************************************************
*
* $FileName: ddi_nand_hal_init.cpp$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments:
*
*   Routines for initializing the NANDs.
*
*END************************************************************************/

#include "hal/ddi_nand_hal_internal.h"
#include <string.h>
#include <stdlib.h>
#include "auto_free.h"
#include "hal/ddi_nand.h"
#include "ddi_nand_hal_nfcphymedia.h"
#include "simple_mutex.h"

/* 
** Definations 
*/

#if !defined(PREFER_ONFI_AUTO_CONFIG)
/*
** Set this macro to 1 to prefer automatic configuration for ONFI NANDs. When set to 0,
** the device code tables will be preferred and ONFI auto configuration will only be
** used if the NAND cannot be found in the tables. 
*/
#define PREFER_ONFI_AUTO_CONFIG 1
#endif

/*
** These are physical parameters that can be overruled in NandInitDescriptor_t
** by analyzing the data read during read IDs command. 
*/
typedef struct {
    /* (1/2/4/...) - number of chips pretending to be a single chip */
    uint32_t wTotalInternalDice;    
    
    /* (wTotalBlocks / wTotalInternalDice) */
    uint32_t wBlocksPerDie;         
} NandOverruledParameters_t;

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : NandHal::init
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   Initialise the NandHal
*
*END*--------------------------------------------------------------------*/
RtStatus_t NandHal::init()
{ /* Body */
    RtStatus_t status = SUCCESS;
    
    _wl_mem_set(&g_nandHalContext, 0, sizeof(g_nandHalContext));
    
    /*  Ask the HAL to initialize its synchronization objects.       */
    if (!SimpleMutex::InitMutex(&g_nandHalContext.serializationMutex))
    {
        return (MQX_INVALID_HANDLE);
    } /* Endif */

    /* factory here */
    NFCNandMedia * nand = new NFCNandMedia(0);
    
    nand->initChipParam();    
    nand->pNANDParams = & g_nandHalContext.parameters;
    nand->initHalContextParams();       
    
    g_nandHalContext.chipSelectCount = 1;

    g_nandHalContext.nands[0] = nand;
    
    /* We have only one NAND */
    g_nandHalContext.totalBlockCount = nand->wTotalBlocks;

    // TODO: Check spare area has enough space for all ECC bytes and metada 
    status = nand->checkSpaceForMetadata();
    if (status != SUCCESS)
    {
        return status;
    }
    
    return status;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : NandHal::shutdown
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   Shutdown the NandHal
*
*END*--------------------------------------------------------------------*/
RtStatus_t NandHal::shutdown()
{ /* Body */
    if (g_nandHalContext.nands[0]) {
        delete g_nandHalContext.nands[0];
        g_nandHalContext.nands[0] = NULL;
    } /* Endif */
    
    _wl_mem_set(&g_nandHalContext.parameters, 0, sizeof(g_nandHalContext.parameters));
    
    /*  Lastly, destroy the HAL mutex. */
    SimpleMutex::DestroyMutex(&g_nandHalContext.serializationMutex);
    
    return SUCCESS;
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : NandHal::shutdown
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This function exists solely to provide a method to call 
*   the NAND HAL shutdown routine from C linkage code.
*END*--------------------------------------------------------------------*/
extern "C" void ddi_nand_hal_shutdown(void)
{ /* Body */
    NandHal::shutdown();
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : NandHal::shutdown
* Returned Value   : SUCCESS or ERROR
* Comments         :
*   This constructor fills in the minimum fields required to be able to send reset and
*   read ID commands. No other commands should be attempted.
*END*--------------------------------------------------------------------*/
/*InitNand::InitNand
(
     [IN] ID of enabled chip 
    unsigned chipEnable
)
:   CommonNandBase(),
    m_isOnfi(false),
    m_mapEntry(0),
    m_newNand(0)
{  Body 
    wChipNumber = chipEnable;
    pNANDParams = &g_nandHalContext.parameters;
}  Endbody */

/* EOF */
