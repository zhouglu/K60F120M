#ifndef __qspi_memory_h__
#define __qspi_memory_h__
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
* $FileName: qspi_memory.h$
* $Version : 4.0.2.0$
* $Date    : May-3-2013$
*
* Comments:
*
*   This file contains definitions for the QuadSPI memory example.
*
*END************************************************************************/
#include "qspi_quadspi.h"

/* Lookup Table Instructions */ 
#define CMD_WR_REG              0x01
#define CMD_PAGE_WR             0x02
#define CMD_RD                  0x03
#define CMD_WR_DISABLE          0x04
#define CMD_RD_STATUS_1         0x05
#define CMD_WR_EN               0x06
#define CMD_RD_STATUS_2         0x07
#define CMD_FAST_RD             0x0B
#define CMD_FAST_RD_4           0x0C
#define CMD_DDR_FAST_RD         0x0D
#define CMD_DDR_FAST_RD_4       0x0E
#define CMD_PAGE_WR_4           0x12
#define CMD_RD_4                0x13
#define CMD_AUTOBOOT_RD         0x14
#define CMD_AUTOBOOT_WR         0x15
#define CMD_BANK_RD             0x16
#define CMD_BANK_WR             0x17
#define CMD_RESERVED_18         0x18
#define CMD_PAR_SEC_ERASE       0x20
#define CMD_PAR_SEC_ERASE_4     0x21
#define CMD_ASP_RD              0x2B
#define CMD_ASP_WR              0x2F
#define CMD_CLR_STATUS          0x30
#define CMD_QUAD_WR             0x32
#define CMD_QUAD_WR_4           0x34
#define CMD_RD_CONIG            0x35
//#define CMD_QUAD_WR           0x38
#define CMD_RD_DUAL_OUT         0x3B
#define CMD_RD_DUAL_OUT_4       0x3C
#define CMD_DATA_LEARN_PATN_RD  0x41
#define CMD_OTP_WR              0x42
#define CMD_WR_NV_DATA_LEARN    0x43
#define CMD_WR_VOL_DATA_LEARN   0x4A
#define CMD_OTP_RD              0x4B
#define CMD_BULK_ERASE          0x60
#define CMD_RD_QUAD_OUT         0x6B
#define CMD_RD_QUAD_OUT_4       0x6C
#define CMD_ERASE_SUSPEND       0x75
#define CMD_ERASE_RESUME        0x7A
#define CMD_WR_SUSPEND          0x85
#define CMD_WR_RESUMED          0x8A
#define CMD_RD_ID_REMS          0x90
#define CMD_RD_ID_JEDEC         0x9F
#define CMD_RESERVED_MPM        0xA3
#define CMD_PPB_LOCK_WR         0xA6
#define CMD_PPB_LOCK_RD         0xA7
#define CMD_RD_ELEC_SIG         0xAB
#define CMD_BANK_ACCESS         0xB9
#define CMD_DUAL_IO_RD          0xBB
#define CMD_DUAL_IO_RD_4        0xBC
#define CMD_DDR_DUAL_IO_RD      0xBD
#define CMD_DDR_DUAL_IO_RD_4    0xBE
#define CMD_BULK_ERASE2         0xC7
#define CMD_SEC_ERASE           0xD8
#define CMD_SEC_ERASE_4         0xDC
#define CMD_DYB_RD              0xE0
#define CMD_DYB_WR              0xE1
#define CMD_PPB_RD              0xE2
#define CMD_PPB_WR              0xE3
#define CMD_PPB_ERASE           0xE4
#define CMD_RESERVED_E5         0xE5
#define CMD_RESERVED_E6         0xE6
#define CMD_PASSWD_RD           0xE7
#define CMD_RASSWD_WR           0xE8
#define CMD_RASSWD_UNLOCK       0xE9
#define CMD_QUAD_IO_RD          0xEB
#define CMD_QUAD_IO_RD_4        0xEC
#define CMD_DDR_QUAD_IO_RD      0xED
#define CMD_DDR_QUAD_IO_RD_4    0xEE
#define CMD_RESET               0xF0
#define CMD_MODE_BIT_RESET      0xFF

#define QuadSPI_MEMORY_PAGE_SIZE   64
#define QuadSPI_WRITE_EN           QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_WR_EN)
#define QuadSPI_WRITE_DISABLE      QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_WR_DISABLE)
#define QuadSPI_CHIP_ERASE         QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_BULK_ERASE)
#define QuadSPI_READ_STATUS1       QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_RD_STATUS_1)

#ifdef BSP_QUADSPI_ENABLE_32BIT
#define QuadSPI_SET_PAGE_WR        QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_PAGE_WR_4)     //32bit
#define QuadSPI_SET_DDR_FAST_RD    QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_DDR_FAST_RD_4) //32bit
#else
#define QuadSPI_SET_PAGE_WR        QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_PAGE_WR)       //24bit
#define QuadSPI_SET_DDR_FAST_RD    QuadSPI_SEQ(QuadSPI_INST_CMD, QuadSPI_SINGLE_PAD, CMD_DDR_FAST_RD)   //24bit
#endif

#define QuadSPI_READ_DATA(x)       QuadSPI_SEQ(QuadSPI_INST_READ, QuadSPI_SINGLE_PAD, x)

#define QuadSPI_DUMMY_DATA(x)      QuadSPI_SEQ(QuadSPI_INST_DUMMY, QuadSPI_SINGLE_PAD, x)

#ifdef BSP_QUADSPI_ENABLE_32BIT
#define QuadSPI_SET_ADDR           QuadSPI_SEQ(QuadSPI_INST_ADDR, QuadSPI_SINGLE_PAD, 0x20)     //32bit
#define QuadSPI_SET_DDR_ADDR       QuadSPI_SEQ(QuadSPI_INST_ADDR_DDR, QuadSPI_SINGLE_PAD, 0x20) //32bit
#else
#define QuadSPI_SET_ADDR           QuadSPI_SEQ(QuadSPI_INST_ADDR, QuadSPI_SINGLE_PAD, 0x18)     //24bit
#define QuadSPI_SET_DDR_ADDR       QuadSPI_SEQ(QuadSPI_INST_ADDR_DDR, QuadSPI_SINGLE_PAD, 0x18) //24bit
#endif

#define QuadSPI_WRITE_DATA(x)      QuadSPI_SEQ(QuadSPI_INST_WRITE, QuadSPI_SINGLE_PAD, x)
#define QuadSPI_SET_MODE_DDR(x)    QuadSPI_SEQ(QuadSPI_INST_MODE_DDR, QuadSPI_SINGLE_PAD, x)

#define QuadSPI_DDR_READ_DATA(x)   QuadSPI_SEQ(QuadSPI_INST_READ_DDR, QuadSPI_SINGLE_PAD, x) 

#define QuadSPI_ADDR_BYTES 4
#define SECTOR_SIZE 0x10000
#define SECTOR_NUM  254

#define FLASH_BASE_ADR          0x20000000
#define TEST_BUFFER1            0x20008000
#define TEST_BUFFER2            0x20100000
#define TEST_BUF_SIZE1          16384
#define TEST_BUF_SIZE2          1024


/* Funtion prototypes */
extern boolean memory_read_status1(MQX_FILE_PTR qspifd, uint_8_ptr status);

extern int_32 memory_chip_erase(MQX_FILE_PTR qspifd, uint_32 addr);
extern uint_8 memory_read_byte(MQX_FILE_PTR qspifd, uint_32 addr);
extern int_32 memory_write_data (MQX_FILE_PTR qspifd, uint_32 addr, uint_32 size, uint_8_ptr data);
extern int_32 memory_read_data (MQX_FILE_PTR qspifd, uint_32 addr, uint_32 size, uint_8_ptr data);

#endif
