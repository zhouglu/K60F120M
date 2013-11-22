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
* $FileName: quadspi_boot.h$
* $Version : 0.1$
* $Date    : April-23-2013$
*
* Comments:
*
*   This file contains the source for the bootloader_vybrid_qspixip example program.
*
*END************************************************************************/

#ifndef _QUADSPI_BOOT_H_
#define _QUADSPI_BOOT_H_



#include <mqx.h>

//typedef unsigned int uint32_t ;
//typedef unsigned char uint8_t ;
//typedef unsigned short int uint16_t;


#define DCD_ADDRESS           device_config_data
#define BOOT_DATA_ADDRESS     &boot_data
#define CSF_ADDRESS           0
#define PLUGIN_FLAG           (uint32_t)0


/*Quad SPI configuration structures */
typedef struct __sflash_configuration_parameter
{
  uint_32 dqs_loopback;              //Sets DQS LoopBack Mode to enable Dummy Pad MCR[24]
  uint_32 rsvd[4];
  uint_32 cs_hold_time;              //CS hold time in terms of serial clock.(for example 1 serial clock cyle)
  uint_32 cs_setup_time;             //CS setup time in terms of serial clock.(for example 1 serial clock cyle)
  uint_32 sflash_A1_size;            //interms of Bytes
  uint_32 sflash_A2_size;            //interms of Bytes
  uint_32 sflash_B1_size;            //interms of Bytes
  uint_32 sflash_B2_size;            //interms of Bytes
  uint_32 sclk_freq;                 //In  00 - 22MHz, 01 - 66MHz, 10 - 80MHz, 11 - 104MHz (only for SDR Mode)
  uint_32 rsvd5;                     //Reserved for Future Use 
  uint_8 sflash_type;               //0-Single,1--Dual 2--Quad
  uint_8 sflash_port;               //0--Only Port-A,1--Both PortA and PortB
  uint_8 ddr_mode_enable;          //Enable DDR mode if set to TRUE
  uint_8 dqs_enable;               //Enable DQS mode if set to TRUE.
  uint_8 parallel_mode_enable;     //Enable Individual or parrallel mode.
  uint_8 portA_cs1;
  uint_8 portB_cs1;
  uint_8 fsphs;
  uint_8 fsdly;
  uint_8 ddrsmp;
  uint_16  command_seq[128]; //set of seq to perform optimum read on SFLASH as as per vendor SFLASH

  /* added to make even 0x400 size */
  /* this is required to pad the memory space when using DS-5 fromelf parser to create C-array from elf file */
  uint_8 empty[0x2C0];

}SFLASH_CONFIGURATION_PARAM,*SFLASH_CONFIGURATION_PARAM_PTR;



/************************************* 
 *  DCD Data 
 *************************************/
#define DCD_TAG_HEADER            (0xD2)
#define DCD_TAG_HEADER_SHIFT      (24)
#define DCD_VERSION               (0x40)
#define DCD_ARRAY_SIZE             1


/************************************* 
 *  IVT Data 
 *************************************/
typedef struct _ivt_ {
    /** @ref hdr with tag #HAB_TAG_IVT, length and HAB version fields
     *  (see @ref data)
     */
    uint_32 hdr;
    /** Absolute address of the first instruction to execute from the
     *  image
     */
    uint_32 entry;
    /** Reserved in this version of HAB: should be NULL. */
    uint_32 reserved1;
    /** Absolute address of the image DCD: may be NULL. */
    uint_32 dcd;
    /** Absolute address of the Boot Data: may be NULL, but not interpreted
     *  any further by HAB
     */
    uint_32 boot_data;
    /** Absolute address of the IVT.*/
    uint_32 self;
    /** Absolute address of the image CSF.*/
    uint_32 csf;
    /** Reserved in this version of HAB: should be zero. */
    uint_32 reserved2;
} ivt;

#define IVT_MAJOR_VERSION           0x4
#define IVT_MAJOR_VERSION_SHIFT     0x4
#define IVT_MAJOR_VERSION_MASK      0xF
#define IVT_MINOR_VERSION           0x1
#define IVT_MINOR_VERSION_SHIFT     0x0
#define IVT_MINOR_VERSION_MASK      0xF

#define IVT_VERSION(major, minor)   \
  ((((major) & IVT_MAJOR_VERSION_MASK) << IVT_MAJOR_VERSION_SHIFT) |  \
  (((minor) & IVT_MINOR_VERSION_MASK) << IVT_MINOR_VERSION_SHIFT))


#define IVT_TAG_HEADER        (0xD1)       /**< Image Vector Table */
#define IVT_SIZE              0x2000
#define IVT_PAR               IVT_VERSION(IVT_MAJOR_VERSION, IVT_MINOR_VERSION)

#define IVT_HEADER          (IVT_TAG_HEADER | (IVT_SIZE << 8) | (IVT_PAR << 24))
#define IVT_RSVD            (uint32_t)(0x00000000)


/************************************* 
 *  Boot Data 
 *************************************/
typedef struct _boot_data_ {
  uint_32 start;           /* boot start location */
  uint_32 size;            /* size */
  uint_32 plugin;          /* plugin flag - 1 if downloaded application is plugin */
  uint_32 placeholder;		/* placehoder to make even 0x10 size */
}BOOT_DATA_T;

/* External Variables */
extern const uint_32 device_config_data[];
extern const BOOT_DATA_T boot_data;
extern void __boot(void);


#endif