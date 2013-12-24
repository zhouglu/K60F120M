#ifndef __drive_tags_h__
#define __drive_tags_h__
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
 * $FileName: drive_tags.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *   This file contains drive tags
 *
 *END************************************************************************/

/* 
** Constants 
*/

/* Drive tag definitions */
/* Player drive tag. */
#define DRIVE_TAG_STMPSYS_S                     0x00
#define DRIVE_TAG_STMPSYS_S_INFO                ("Player Firmware Drive")

/* UsbMscMtp drive tag, old name was DRIVE_TAG_USBMSC_S.  */
#define DRIVE_TAG_HOSTLINK_S                    0x01
#define DRIVE_TAG_HOSTLINK_S_INFO               ("Hostlink Firmware Drive")

/* Primary player resource drive tag. */
#define DRIVE_TAG_RESOURCE_BIN                  0x02
#define DRIVE_TAG_RESOURCE_BIN_INFO             ("Primary Resource Drive")

/* Backup player resource drive tag. */
#define DRIVE_TAG_RESOURCE2_BIN                 0x12
#define DRIVE_TAG_RESOURCE2_BIN_INFO            ("Backup Resource Drive")

/* Master player resource drive tag. */
#define DRIVE_TAG_RESOURCE_MASTER_BIN           0x22
#define DRIVE_TAG_RESOURCE_MASTER_BIN_INFO      ("Master Resource Drive")

/* The host has 0x05 reserved for OTG drive. */
#define DRIVE_TAG_OTGHOST_S                     0x05
#define DRIVE_TAG_OTGHOST_S_INFO                ("USB OTG Drive")

/* Internal data drive tag. */
#define DRIVE_TAG_DATA                          0x00
#define DRIVE_TAG_DATA_INFO                     ("Public Data Drive")

/* External data drive tag. */
#define DRIVE_TAG_DATA_EXTERNAL                 0x01
#define DRIVE_TAG_DATA_EXTERNAL_INFO            ("External Media Data Drive")

/* Hidden data drive tag for DRM information. */
#define DRIVE_TAG_DATA_HIDDEN                   0x02
#define DRIVE_TAG_DATA_HIDDEN_INFO              ("Hidden Data Drive")

/* Hidden data drive 2 tag for persistent settings. */
#define DRIVE_TAG_DATA_HIDDEN_2                 0x03
#define DRIVE_TAG_DATA_HIDDEN_2_INFO            ("Second Hidden Data Drive")

/* Primary player firmware drive tag. */
#define DRIVE_TAG_BOOTMANAGER_S                 0x50
#define DRIVE_TAG_BOOTMANAGER_S_INFO            ("Primary Firmware Drive")

/* Backup player firmware drive tag. */
#define DRIVE_TAG_BOOTMANAGER2_S                0x60
#define DRIVE_TAG_BOOTMANAGER2_S_INFO           ("Backup Firmware Drive")

/* Master player firmware drive tag. */
#define DRIVE_TAG_BOOTMANAGER_MASTER_S          0x70
#define DRIVE_TAG_BOOTMANAGER_MASTER_S_INFO     ("Master Firmware Drive")

/* Bootlet firmware drive tag. */
#define DRIVE_TAG_BOOTLET_S                     0xa0
#define DRIVE_TAG_BOOTLET_S_INFO                ("Bootlet Firmware Drive")

/* The host has 0xFF reserved for usbmsc.sb file used in recovery mode operation only. */
#define DRIVE_TAG_UPDATER_S                     0xFF
#define DRIVE_TAG_UPDATER_S_INFO                ("USB Recovery")

#endif /* __drive_tags_h__ */

/* EOF */
