/**HEADER********************************************************************
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
******************************************************************************
*
* $FileName: ddi_nand_hal_globals.cpp$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments:
*
*   Contains global definitions for the low level NAND driver.
*
**END**********************************************************************/

#include "wl_common.h"
#include "hal/ddi_nand_hal_internal.h"

/* 
** Global external declarations 
*/

/* Global context for the NAND HAL. */
NandHalContext_t    g_nandHalContext;
MQX_FILE_PTR        g_nandHalFDPTR[MAX_NAND_DEVICES] ;

/* EOF */
