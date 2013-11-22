#ifndef __main_h_
#define __main_h_
/*HEADER**********************************************************************
 *
 * Copyright 2010 Freescale Semiconductor, Inc.
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
 * Comments:
 *
 *END************************************************************************/
#define MAIN_TASK 1

extern void Main_task(uint_32);

  
#if defined(CSCFG_MAX) 
    #define IF_MAX(x)       x
    #define IF_TYPICAL(x)   x
    #define IF_SMALL(x)     x
    #define IF_TINIEST(x)   x
#elif defined(CSCFG_TYPICAL) 
    #define IF_MAX(x)       
    #define IF_TYPICAL(x)   x
    #define IF_SMALL(x)     x
    #define IF_TINIEST(x)   x
#elif defined(CSCFG_SMALL)
    #define IF_MAX(x)       
    #define IF_TYPICAL(x)   
    #define IF_SMALL(x)     x
    #define IF_TINIEST(x)   x
#elif defined(CSCFG_TINIEST)
    #define IF_MAX(x)       
    #define IF_TYPICAL(x)   
    #define IF_SMALL(x)     
    #define IF_TINIEST(x)   x
#else
    #error Please define codesize build configuration       
#endif

#endif /* __main_h_ */

