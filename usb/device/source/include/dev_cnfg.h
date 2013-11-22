#ifndef __usbdev_cnfg_h__
#define __usbdev_cnfg_h__
/*HEADER**********************************************************************
 *
 * Copyright 2008-2012 Freescale Semiconductor, Inc.
 * Copyright 1989-2008 ARC International
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
#include "mqx.h"
#include "bsp.h"        // bsp.h include user_config.h, too

#ifndef USBCFG_DEFAULT_DEVICE_CONTROLLER
#define USBCFG_DEFAULT_DEVICE_CONTROLLER  (0)
#endif

#ifndef USBCFG_MAX_DRIVERS
extern const pointer _bsp_usb_table[];
#define USBCFG_MAX_DRIVERS                (sizeof(_bsp_usb_table) / sizeof(_bsp_usb_table[0]))
#endif

#endif
