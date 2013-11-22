/**HEADER********************************************************************
*
* Copyright (c) 2013 Freescale Semiconductor;
* All Rights Reserved
*
***************************************************************************
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
**************************************************************************
*
* $FileName: support.S$
*
* Comments:
*
*
*END************************************************************************/

#include <asm_mac.h>

#include "mqx_cnfg.h"
#include "types.inc"
#include "psp_prv.inc"

#define __ASM__
#include "psp_cpu.h"
#include "mqx_prv.h"
#undef __ASM__

                ASM_EXTERN(_mqx_kernel_data)
                ASM_CODE_SECTION(.text)

#if PSP_HAS_FPU
/*FUNCTION*******************************************************************
*
* Function Name    : _psp_push_fp_context
* Returned Value   : none
* Comments         : Save FPU and NEON registers to stack
*
*END************************************************************************/
                ASM_PUBLIC(_psp_push_fp_context)
                ASM_PUBLIC_BEGIN(_psp_push_fp_context)
                ASM_PUBLIC_FUNC(_psp_push_fp_context)
ASM_LABEL(_psp_push_fp_context)
#if (defined(__IASMARM__) || defined(__CC_ARM))
        		str r1, [sp, #-136]			/* save r1 to possition in stack (1word reg (r1) + 1word reg (FPSCR) + 32word registers) */
#else
 				ASM_CONST16(0xf84d)
 				ASM_CONST16(0x1c88)
#endif

                /* save FPU, FPU status registers */
                vmrs r1, FPSCR
                //str r1, [sp, #-4]!                       /* FPSCR */
                push {r1}                       /* FPSCR */

                vstmdb sp!, {s0-s31}                      /* restore fpu registers */

#if (defined(__IASMARM__) || defined(__CC_ARM))
                ldr r1, [sp, #-4]                       /* restore changed r1 */
#else
 				ASM_CONST16(0xf85d)
 				ASM_CONST16(0x1c04)
#endif

                bx lr
                ASM_PUBLIC_END(_psp_push_fp_context)

/*FUNCTION*******************************************************************
*
* Function Name    : _psp_pop_fp_context
* Returned Value   : none
* Comments         : Restore FPU and NEON registers from stack
*
*END************************************************************************/
                ASM_PUBLIC(_psp_pop_fp_context)
                ASM_PUBLIC_BEGIN(_psp_pop_fp_context)
                ASM_PUBLIC_FUNC(_psp_pop_fp_context)
ASM_LABEL(_psp_pop_fp_context)
#if (defined(__IASMARM__) || defined(__CC_ARM))
                str r1, [sp, #-4]                       /* save r1 */
#else
 				ASM_CONST16(0xf84d)
 				ASM_CONST16(0x1c04)
#endif

                /* restore FPU, FPU status and NEON registers */
                vldm sp!, {s0-s31}                      /* restore fpu registers */

                //ldr r1, [sp], #4                        /* FPSCR */
                pop {r1}                        /* FPSCR */
                vmsr FPSCR, r1

#if (defined(__IASMARM__) || defined(__CC_ARM))
                ldr r1, [sp, #-136]                     /* restore r1 */        
#else
 				ASM_CONST16(0xf85d)
 				ASM_CONST16(0x1c88)
#endif

                bx lr
                ASM_PUBLIC_END(_psp_pop_fp_context)

#endif // PSP_HAS_FPU
                ASM_END
