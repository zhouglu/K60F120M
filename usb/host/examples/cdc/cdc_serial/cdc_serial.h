#ifndef __cdc_serial_h__
#define __cdc_serial_h__
/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
* Copyright (c) 1989-2008 ARC International;
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
* $FileName: cdc_serial.h$
* $Version : 3.8.6.0$
* $Date    : Sep-13-2012$
*
* Comments:
*
*   This file contains keyboard-application types and definitions.
*
*END************************************************************************/

#ifdef __USB_OTG__
#include "otgapi.h"
#include "devapi.h"
#else
#include "hostapi.h"
#endif


/***************************************
**
** Application-specific definitions
*/

/* Used to initialize USB controller */
#define HOST_CONTROLLER_NUMBER      USBCFG_DEFAULT_HOST_CONTROLLER
#define CDC_EXAMPLE_USE_HW_FLOW     0

#define  USB_DEVICE_IDLE                   (0)
#define  USB_DEVICE_ATTACHED               (1)
#define  USB_DEVICE_CONFIGURED             (2)
#define  USB_DEVICE_SET_INTERFACE_STARTED  (3)
#define  USB_DEVICE_INTERFACED             (4)
#define  USB_DEVICE_DETACHED               (5)
#define  USB_DEVICE_OTHER                  (6)

#define DEVICE_REGISTERED_EVENT            (0x01)
#define DEVICE_CTRL_ATTACHED               (0x02)
#define DEVICE_CTRL_INTERFACED             (0x04)
#define DEVICE_DATA_ATTACHED               (0x08)
#define DEVICE_DATA_INTERFACED             (0x10)
#define DEVICE_CTRL_DETACHED               (0x20)

#define CDC_SERIAL_DEV_ATTACHED            (0x01)
#define CDC_SERIAL_DEV_REGISTERED          (0x02)
#define CDC_SERIAL_FUSB_OPENED_UART2USB    (0x04)
#define CDC_SERIAL_FUSB_OPENED_USB2UART    (0x08)
#define CDC_SERIAL_DEV_DETACH_USB2UART     (0x10)
#define CDC_SERIAL_DEV_DETACH_UART2USB     (0x20)
#define CDC_SERIAL_UART2USB_DONE           (0x40)
#define CDC_SERIAL_USB2UART_DONE           (0x80)
/*
** Following structs contain all states and pointers
** used by the application to control/operate devices.
*/

typedef struct acm_device_struct {
   CLASS_CALL_STRUCT                CLASS_INTF; /* Class-specific info */
   LWEVENT_STRUCT                   acm_event;
} ACM_DEVICE_STRUCT,  _PTR_ ACM_DEVICE_STRUCT_PTR;

typedef struct data_device_struct {
   CLASS_CALL_STRUCT                CLASS_INTF; /* Class-specific info */
   LWEVENT_STRUCT                   data_event;
} DATA_DEVICE_STRUCT,  _PTR_ DATA_DEVICE_STRUCT_PTR;

/* To maintain usb file handle */
typedef struct f_usb_info_struct {
   MQX_FILE_PTR f_usb;
   uint_32 cnt; /* how many times f_usb has been opened */
}F_USB_INFO_T;

/* Alphabetical list of Function Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void usb_host_cdc_acm_event(_usb_device_instance_handle, _usb_interface_descriptor_handle, uint_32);
void usb_host_cdc_data_event(_usb_device_instance_handle, _usb_interface_descriptor_handle, uint_32);

#ifdef __cplusplus
}
#endif


#endif

/* EOF */
