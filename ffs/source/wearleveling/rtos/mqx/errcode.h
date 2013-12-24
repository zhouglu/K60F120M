#ifndef _WL_ERRCODE_H_
#define _WL_ERRCODE_H_
/**HEADER********************************************************************
 * Copyright (c) 2013 Freescale Semiconductor, Inc. All rights reserved.
 * 
 * Freescale Semiconductor, Inc.
 * Proprietary & Confidential
 * 
 * This source code and the algorithms implemented therein constitute
 * confidential information and may comprise trade secrets of Freescale Semiconductor, Inc.
 * or its associates, and any use thereof is subject to the terms and
 * conditions of the Confidential Disclosure Agreement pursual to which this
 * source code was originally received.
 *****************************************************************************
 * $FileName: errcode.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments: Error code & types convert between WL and RTOS
 *END************************************************************************/

enum NANDWLContext
{
    kNandDontCare = 0,
    kNandReadPage,
    kNandWritePage,
    kNandReadMetadata,
    kNandEraseBlock,
    kNandEraseMultiBlock,
    kNandMarkBlockBad,
    kNandIsBlockBad
};

#ifdef __cplusplus

extern "C"
{
#endif /* __cplusplus*/

RtStatus_t os_err_code_to_wl(_mqx_uint status, uint_32 context);
_mqx_uint wl_err_code_to_os(RtStatus_t status, uint_32 context);
char_ptr wl_err_to_str(RtStatus_t wl_status, uint_32 context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WL_ERRCODE_H_*/

/* EOF */
