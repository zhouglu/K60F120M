/**HEADER********************************************************************
*
* Copyright (c) 2008 Freescale Semiconductor;
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
* $FileName: qpbridge.h$
* $Version : 3.8.2.0$
* $Date    : Jun-6-2012$
*
* Comments:
*
*   This file contains the type definitions for the MPX PBRIDGE module.
*
*END************************************************************************/

#ifndef __qpbridge_h__
#define __qpbridge_h__

#define __qpbridge_h__version "$Version:3.8.2.0$"
#define __qpbridge_h__date    "$Date:Jun-6-2012$"


// Flag used to identify the register type for a given peripheral ID
#define QPBRIDGE_MPROT_FLAG         0x10000000
#define QPBRIDGE_PACR_FLAG          0x20000000
#define QPBRIDGE_OPACR_FLAG         0x40000000
#define QPBRIDGE_DEVICE_TYPE_MASK   0xf0000000

// Register Bit Definitions
#define QPBRIDGE_MPROT_MTR          0x04
#define QPBRIDGE_MPROT_MTW          0x02
#define QPBRIDGE_MPROT_MPL          0x01

#define QPBRIDGE_PACR_SP            0x04
#define QPBRIDGE_PACR_WP            0x02
#define QPBRIDGE_PACR_TP            0x01

#define QPBRIDGE_NUM_MPROT          2
#define QPBRIDGE_NUM_PACR           4
#define QPBRIDGE_NUM_OPACR          16


typedef volatile struct qpbridge_reg_struct
{
   uint_32  MPROT[QPBRIDGE_NUM_MPROT];
   RESERVED_REGISTER(0x08,0x20);
   uint_32  PACR[QPBRIDGE_NUM_PACR];
   RESERVED_REGISTER(0x30,0x40);
   uint_32  OPACR[QPBRIDGE_NUM_OPACR];
} QPBRIDGE_REG_STRUCT;
typedef volatile struct qpbridge_reg_struct * VQPBRIDGE_REG_STRUCT_PTR;


#ifdef __cplusplus
extern "C" {
#endif

VQPBRIDGE_REG_STRUCT_PTR _bsp_get_qpbridge_base_address(void);
boolean _qpbridge_enable_access( uint_32 device);
void _qpbridge_init( const uint_32 config[], uint_32 size);

#ifdef __cplusplus
}
#endif


#endif
