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
 * $FileName: ddi_nand.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *   This file contains declarations of public interfaces to
 *   the NAND driver.
 **END*************************************************************************/

#if !defined(__ddi_nand_h__)
#define __ddi_nand_h__
/*  
** Constants 
*/

enum
{
    /* DriveSetInfo() key for control over system drive recovery. [bool] 
    **
    ** Use DriveSetInfo() to modify this property of system drives. Setting it 
    ** to true will enable automatic recovery of system drives when an error is 
    ** encountered during a page read. The drive will be completely erased and 
    ** rewritten from the master copy. Setting this property to false will disable 
    ** the recovery functionality. 
    */
    kDriveInfoNandSystemDriveRecoveryEnabled = (('n'<<24)|('s'<<16)|('r'<<8)|('e')) /* 'nsre' */
};

/*  
** Prototypes 
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
*******************************************************************************
** Shutdown the NAND HAL and GPMI. 
*******************************************************************************
*/
void ddi_nand_hal_shutdown(void);

#ifdef __cplusplus
} /*  extern "C" */
#endif /*  __cplusplus */

#endif /* __ddi_nand_h__ */

/* EOF */
