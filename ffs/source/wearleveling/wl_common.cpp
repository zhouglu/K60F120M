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
 * $FileName: wl_common.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains declaration of global variables of WL module
 *
 *END************************************************************************/

#include "wl_common.h"

#if  ( (WL_DEBUG))
uint_32 g_wl_log_module_attributes = 
    WL_MODULE_GENERAL 
    /*| WL_MODULE_HAL*/  
    | WL_MODULE_MAPPER 
    | WL_MODULE_MEDIA 
    | WL_MODULE_LOGICALDRIVE
    | WL_MODULE_DEFEERREDTASK 
    | WL_MODULE_MEDIABUFFER 
    | WL_MODULE_NANDWL 
    | WL_MODULE_NANDWL_TESTING
;

uint_32 g_wl_log_level = WL_LOG_DEBUG;
#else
uint_32 g_wl_log_module_attributes = WL_MODULE_GENERAL;
uint_32 g_wl_log_level = WL_LOG_DISABLE;
#endif

short countBits(short n, uint_8 bit)
{ /* Body */
    short i = 0;

    if ( n )
    {
        while ( bit == (n & 1) )
        {
            ++i;
            n >>= 1;
        } /* Endwhile */
    } /* Endif */

    return i;
} /* Endbody */

/* EOF */
