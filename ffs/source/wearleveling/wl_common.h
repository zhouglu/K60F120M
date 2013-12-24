#ifndef __wl_common_h__
#define __wl_common_h__
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
 * $FileName: wl_common.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains constants and macros of WL module
 *
 *END************************************************************************/
#include "user_config.h"

#ifndef NANDWL_USE_RTOS_MQX
#define NANDWL_USE_RTOS_MQX             1
#endif

#ifndef NANDWL_USE_MM_LEAK_DETECTION
#define NANDWL_USE_MM_LEAK_DETECTION    0
#endif

#define WL_DEBUG                        0

#if NANDWL_USE_RTOS_MQX

#include "mqx.h"
#include "bsp.h"
#include "nandflash_wl_ffs.h"
#include "mutex.h"
#include "timer.h"
#include "wl_types.h"
#include "errcode.h"
#endif /* Endif NANDWL_USE_RTOS_MQX */

#define MIN(a, b)           (((a) <= (b)) ? (a) : (b))
#define MAX(a, b)           (((a) >= (b)) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif
short countBits(short n, uint_8 bit);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __wl_common_h__ */

/* EOF */
