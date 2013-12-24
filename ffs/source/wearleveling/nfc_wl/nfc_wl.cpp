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
 * $FileName: nfc.c$
 * $Version : 3.8.16.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   The file contains functions to interface NAND Flash Controller module.
 *
 *END***************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "nandflash.h"
#include "nandflashprv.h"
#include "nfc.h"
#include "nandflash_wl.h"
#include "nfc_wl.h"

#ifndef NFC_DEBUG
#define NFC_DEBUG 0
#endif

#if NFC_DEBUG
boolean g_nfc_log_enable = FALSE;
#if defined (__GNUG__)
#define NFC_LOG(fmt, ...) { if (g_nfc_log_enable) printf("\n[NFC LOG] " fmt, __VA_ARGS__); }
#define NFC_LOG1(fmt, ...) { if (g_nfc_log_enable) printf( fmt, __VA_ARGS__); }
#else
#define NFC_LOG(fmt, ...) { if (g_nfc_log_enable) printf("\n[NFC LOG] " ##fmt, __VA_ARGS__); }
#define NFC_LOG1(fmt, ...) { if (g_nfc_log_enable) printf( ##fmt, __VA_ARGS__); }
#endif
#else 
#define NFC_LOG(...)
#define NFC_LOG1(...)
#endif
static uint_32 curr_ecc_mode;
uint_32 nfc_swap_addr1_with_metadata;

static uint_8 _ecc_mode_threshold[] = {
   0, /* No ECC*/ 
   3, /* 4-error */ 
   5, /* 6-error */
   7, /* 8-error */
   11, /* 12-error */
   14, /* 16-error */
   22, /* 24-error */
   30 /* 32-error */
};

#if NFC_DEBUG
void nfc_dump_buff( pointer buf, uint_32 size , char_ptr buff_name);
void nfc_dump_buff( pointer buf, uint_32 size , char_ptr buff_name)
{
   int i = 0;
   int j = 0;
   NFC_LOG1("\n NFC Buffer \"%s\" dump:\n", buff_name);

   for (i = 0; i < size; i++) {
      if  (  ((i != 0 ) && ((i % 16) == 0)) ) 
      {
         for (j =  (i-16)  ; j < i; j ++  )
         NFC_LOG1 (" %c", ((uint_8_ptr)buf)[j]);
      }
      if ((i % 16) == 0) NFC_LOG1 ("\n%04d: ", i);
      if ((i % 8) == 0) NFC_LOG1("%s"," | ");

      NFC_LOG1("%02x ", ((uint_8_ptr)buf)[i] );
   }
   
   if ((size < 16) ) {
      for (j =  0; j < size; j++)
      NFC_LOG1(" %c", ((uint_8_ptr) buf)[j]);
   }

   NFC_LOG1("%s","\n");
}
#else 
#define nfc_dump_buff(a, b, c)
#endif

extern uint_32 nfc_swap_addr1, nfc_swap_addr2;

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : nfc_init
* Returned Value   : NANDFLASHERR_NO_ERROR or
*                    NANDFLASHERR_IMPROPER_ECC_SIZE
*                    NANDFLASHERR_INFO_STRUC_MISSING
*                    MQX_OUT_OF_MEMORY
* Comments         :
*    This function initializes the driver.
*
*END*-------------------------------------------------------------------------*/

uint_32 nfc_wl_init
   (
      /* [IN] the NAND flash information */
      IO_NANDFLASH_STRUCT_PTR nandflash_ptr
   )
{ /* Body */
   uint_32 num_metadata_bytes;
   uint_32 num_ecc_bytes;
   uint_32 result = NANDFLASHERR_NO_ERROR;
   
   result = nfc_init(nandflash_ptr);
   switch(nandflash_ptr->ECC_SIZE)
   {
       case 0: 
          curr_ecc_mode = 0;
          break;
       case 4: 
          curr_ecc_mode = 1;
          break;
       case 6:
          curr_ecc_mode = 2;
          break;
       case 8: 
          curr_ecc_mode = 3;
          break;
       case 12: 
          curr_ecc_mode = 4;
          break;
       case 16: 
          curr_ecc_mode = 5;
          break;
       case 24: 
          curr_ecc_mode = 6;
          break;
       case 32: 
          curr_ecc_mode = 7;
          break;
       default:
          curr_ecc_mode = 0;
          break;
   }
   num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->ECC_SIZE);
   num_metadata_bytes = nandflash_ptr->NANDFLASH_INFO_PTR->SPARE_AREA_SIZE/nandflash_ptr->PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO 
                        - num_ecc_bytes;
   /* In case we arrange the page layout as Data-Metadata-ECC, we need to use another swap_addr1 */
   nfc_swap_addr1_with_metadata = ((nandflash_ptr->NANDFLASH_INFO_PTR->PHY_PAGE_SIZE)-
                                    ((nandflash_ptr->PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO - 1) * 
                                    (nandflash_ptr->VIRTUAL_PAGE_SIZE + num_ecc_bytes + num_metadata_bytes)))/8;
   NFC_LOG("NFC_INIT nfc_swap_addr1_with_metadata*8=%d\n", nfc_swap_addr1_with_metadata);
   
   return result;
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nfc_read_page_with_metadata
* Returned Value   : NANDFLASHERR_NO_ERROR or 
*                    NANDFLASHERR_ECC_FAILED or
*                    NANDFLASHERR_ECC_CORRECTED or
*                    NANDFLASHERR_TIMEOUT
* Comments         : 
*    This function reads one or several pages of NAND Flash 
*    using the NFC module.
* 
*END*----------------------------------------------------------------------*/

uint_32 nfc_read_page_with_metadata
   (  
      /* [IN] the NAND flash information */
      IO_NANDFLASH_WL_STRUCT_PTR nandflash_ptr,

      /* [OUT] where to copy data to */
      uchar_ptr               to_data_ptr,
      
      /* [OUT] where to copy metadata to */
      uchar_ptr               to_metadata_ptr,

      /* [IN] the page to read */
      uint_32                 page_number,

      /* [IN] the amount of pages to read */
      uint_32                 page_count
   )
{ /* Body */
   uint_32 result = NANDFLASHERR_TIMEOUT;
   uint_32 count1, count2, count3, real_virt_page_offset;
   uint_32 data_output_offset = 0, metadata_output_offset = 0;
   boolean ecc_corrected = FALSE, swap = FALSE;
   uint_32 row, col, num_ecc_bytes, num_metadata_bytes;
   uint_32 cfg_bck;
   
   NFC_MemMapPtr  nfc_ptr;
   uint_8 ecc_num_bit_correct;   

   num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->CORE_NANDFLASH.ECC_SIZE);
   num_metadata_bytes = nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO 
                        - num_ecc_bytes;
   /* Get the pointer to nfc registers structure */
   nfc_ptr = (NFC_MemMapPtr)_bsp_get_nfc_address();
   
   /* back up current nfc_ptr then change the ECC status position in output read buffer */
   cfg_bck = nfc_ptr->CFG;   
   nfc_ptr->CFG =  (nfc_ptr->CFG & (~NFC_CFG_ECCAD(0xFFFF))) 
                  | NFC_CFG_ECCAD(( (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE  +num_metadata_bytes) / 8) + 1); 
   
   for (count1 = page_number; count1 < (page_number + page_count); count1++)
   {     
      /* Calculate physical page address */
      row = count1/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO;
      real_virt_page_offset = nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO
      - 1 - (count1 - (row * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO));

      /* Calculate byte offset in this physical page */
      /* For preserve bad marking block address, we writes all pages in reserved order.
      * Virtual page 0 <-> write physically in Virtual page 3
      * Virtual page 1 <-> write physically in Virtual page 2 */

      col = real_virt_page_offset *(nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + num_ecc_bytes);

      /* If the last virtual page of the first or the second physical page
      is about to be written the swapping needs to be switched on due 
      to the bad block marking */
      count2 = count1 %
               ((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->BLOCK_SIZE / nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE)
               * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO);
      swap = ((count2 == (0)) ||
               (count2 == (nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO)));
      
      if(nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16)
         col = col/2;
      
      nfc_ptr->CMD1 = NFC_CMD1_BYTE2(NANDFLASH_CMD_PAGE_READ_CYCLE2);
      nfc_ptr->CMD2 = NFC_CMD2_BYTE1(NANDFLASH_CMD_PAGE_READ_CYCLE1) | 
                        NFC_CMD2_CODE(0x7EE0) |
                        NFC_CMD2_BUFNO(1);
      nfc_ptr->CAR  = col & 0xFFFF;
      nfc_ptr->RAR  = NFC_RAR_CS0_MASK |
                        NFC_RAR_RB0_MASK |
                        (row & 0xFFFFFF);
      nfc_ptr->SECSZ  = (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE)+ num_metadata_bytes +num_ecc_bytes ;
      
      /* For 16-bit data width flash devices, only odd SIZE is supported */
      if((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16) && !(nfc_ptr->SECSZ % 2))
         nfc_ptr->SECSZ  += 1;
      
      /* If the last virtual page of the first or the second physical page
      is about to be read the virtual page needs to be enlarged
      by 8 bytes and swapping switched on due to the bad block marking */
      if(swap)
      {
         nfc_ptr->SWAP  = NFC_SWAP_ADDR1(nfc_swap_addr1_with_metadata) | 
         NFC_SWAP_ADDR2(nfc_swap_addr2);
      }
      else
      {
         nfc_ptr->SWAP  = NFC_SWAP_ADDR1(0x7FF) | 
         NFC_SWAP_ADDR2(0x7FF);
      }
      NFC_LOG("nfc_read_page_with_metadata: issue a READ cmd w/ SECTZ=%d at COL*2=%d, page=%d\n", 
      nfc_ptr->SECSZ, col*2, count1);
      
      nfc_ptr->ISR  |= (NFC_ISR_DONECLR_MASK | 
      NFC_ISR_DONEEN_MASK | 
      NFC_ISR_IDLECLR_MASK |
      NFC_ISR_IDLEEN_MASK);
      
      /* Start command execution */
      nfc_ptr->CMD2 |= NFC_CMD2_BUSY_START_MASK;
      
      for (count2 = 0; count2 <= MAX_WAIT_COMMAND; count2++)
      {
         if (nfc_ptr->ISR & NFC_ISR_IDLE_MASK)
         {
            
            if ( nandflash_ptr->CORE_NANDFLASH.ECC_SIZE == 0)
            {
               if (nfc_ptr->SR2 & NANDFLASH_STATUS_ERR)
               {
                  result = NANDFLASHERR_ECC_FAILED;
                  break; /* break to out of for MAX_WAIT_COMMAND*/
               }
            }
            else  /* if nandflash_ptr->ECC_SIZE == 0)*/
            {      
               if((*(uint_32*)&NFC_SRAM_B1_REG(nfc_ptr, nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + (num_ecc_bytes + 4))) & NFC_ECC_STATUS_CORFAIL)
               {
                  /* ECC Fix FAILED */
                  result = NANDFLASHERR_ECC_FAILED;
                  break;
               }
               else
               {
                  if((*(uint_32*)&NFC_SRAM_B1_REG(nfc_ptr, nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + (num_ecc_bytes + 4))) & NFC_ECC_STATUS_CORFAIL)
                  {               
                     ecc_corrected = TRUE;
                     ecc_num_bit_correct = (*(uint_32*)&NFC_SRAM_B1_REG(nfc_ptr, nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + (num_ecc_bytes + 4))) & NFC_ECC_STATUS_ERROR_COUNT;
                     result = NANDFLASHERR_ECC_CORRECTED;
                  }
                  else /* ECC is success, we donot need to correct any bits */
                  {
                     result = NANDFLASHERR_NO_ERROR;
                  }
               }      
            } /* if nfc_ptr->ISR & NFC_ISR_IDLE*/

            /* In case we catch ECC_CORRECTED or NO_ERROR, we still copy data from SRAM -> requested buffer */
            
            /* Copy Page DATA from SRAM buffer to to_data_ptr */
            for(count3 = 0; count3 < (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE); count3++)
            {
               *(to_data_ptr + data_output_offset) = NFC_SRAM_B1_REG(nfc_ptr, count3);
               data_output_offset++;            
            }
            
            NFC_LOG("nfc_read_page_with_metadata: copy %d from page DATA to SRAM \n", count3);
            
            /* Copy Page METADATA from SRAM buffer to to_data_ptr */
            for(count3 = nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE; 
            count3 < (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes); count3++)
            {
               *(to_metadata_ptr + metadata_output_offset) = NFC_SRAM_B1_REG(nfc_ptr, count3);
               metadata_output_offset++;            
            }
            
            NFC_LOG("nfc_read_page_with_metadata: copy %d from page METADATA to SRAM \n", count3-nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE);                 
            
            
            break;
         } /* if nfc_ptr->ISR & IDLE */
      } /* for MAX_WAIT_COMMAND */

      /* Everytime we catch error even ECC_CORRECTED, just quit */
      if (NANDFLASHERR_NO_ERROR != result ) 
      {
         break; /* break out of for count1=page_number */
      }
   } 
   
   if (ecc_corrected)
   {
      if (ecc_num_bit_correct >= _ecc_mode_threshold[curr_ecc_mode])
         result = NANDFLASHERR_ECC_CORRECTED_EXCEED_THRESHOLD;
      else
         result = NANDFLASHERR_ECC_CORRECTED;
   }
   
   nfc_dump_buff(to_data_ptr, data_output_offset, "Read Page Buffer");
   nfc_dump_buff(to_metadata_ptr, metadata_output_offset, "Read Page Metadata Buffer");
   
   /* Restore nfc config */
   nfc_ptr->CFG = cfg_bck ;
   return(result);

} /* Endbody */




/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nfc_read_page_metadata
* Returned Value   : NANDFLASHERR_NO_ERROR or 
*                    NANDFLASHERR_ECC_FAILED or
*                    NANDFLASHERR_ECC_CORRECTED or
*                    NANDFLASHERR_TIMEOUT
* Comments         : 
*    This function reads one or several pages of NAND Flash 
*    using the NFC module.
* 
*END*----------------------------------------------------------------------*/

uint_32 nfc_read_page_metadata
   (  
      /* [IN] the NAND flash information */
      IO_NANDFLASH_WL_STRUCT_PTR nandflash_ptr,

      /* [OUT} where to copy data to */
      uchar_ptr               to_ptr,

      /* [IN] the page to read */
      uint_32                 page_number,

      /* [IN] the amount of pages to read */
      uint_32                 page_count
   )
{ /* Body */
   uint_32 result = NANDFLASHERR_TIMEOUT;
   uint_32 count1, count2, count3;
   uint_32 output_offset = 0;
   boolean  swap = FALSE;
   uint_32 row, col, num_ecc_bytes, real_virt_page_offset ;
   uint_32 num_metadata_bytes;
   
   uint_32 cfg_bck;

   NFC_MemMapPtr  nfc_ptr;   
   
   num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->CORE_NANDFLASH.ECC_SIZE);
   num_metadata_bytes = nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO 
                        - num_ecc_bytes;
   
   /* Get the pointer to nfc registers structure */
   nfc_ptr = (NFC_MemMapPtr)_bsp_get_nfc_address();
   
   /* Set the ECCMODE to 0 - ECC bypass */
   cfg_bck = nfc_ptr->CFG;
   nfc_ptr->CFG &= ~(NFC_CFG_ECCMODE(7));
   /* Do not write ECC Status to SRAM */
   nfc_ptr->CFG &= ~(NFC_CFG_ECCSRAM_MASK);
   
   /* Reset the swap register */
   nfc_ptr->SWAP = NFC_SWAP_ADDR1(0x7FF) | NFC_SWAP_ADDR2(0x7FF);

   for (count1 = page_number; count1 < (page_number + page_count); count1++) {
      row = count1/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO;
      real_virt_page_offset = nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO
                              - 1 - (count1 - (row * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO));

      /* Calculate byte offset in this physical page */
      /* For preserve bad marking block address, we writes all pages in reserved order.
      * Virtual page 0 <-> write physically in Virtual page 3
      * Virtual page 1 <-> write physically in Virtual page 2 */

      col = real_virt_page_offset *(nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + num_ecc_bytes);row = count1/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO;
      real_virt_page_offset = nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO
                              - 1 - (count1 - (row * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO));

      /* Calculate byte offset in this physical page */
      /* For preserve bad marking block address, we writes all pages in reserved order.
      * Virtual page 0 <-> write physically in Virtual page 3
      * Virtual page 1 <-> write physically in Virtual page 2 */

      col = real_virt_page_offset *(nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + num_ecc_bytes);
      col += nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE;

      /* Is the bad block byte(s) about to be re-written? */
      count2 = count1 %
               ((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->BLOCK_SIZE / nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE)
               * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO);
      swap = ((count2 == (0)) ||
               (count2 == (nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO)));
      /* If the last virtual page of the first or the second physical page
      is about to be read the virtual page needs to be enlarged
      by 8 bytes and swapping switched on due to the bad block marking */
      if(swap)
      {
         /* Here we are read MD from page MD swapped   *
         * -> we need to calculate to real MD of this * 
         * swapped page 
         */ 
         col -= ( nfc_swap_addr2 - nfc_swap_addr1_with_metadata  ) * 8;
      }

      if(nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16)
      col = col/2;
      
      nfc_ptr->CMD1 = NFC_CMD1_BYTE2(NANDFLASH_CMD_PAGE_READ_CYCLE2);
      nfc_ptr->CMD2 = NFC_CMD2_BYTE1(NANDFLASH_CMD_PAGE_READ_CYCLE1) | 
                        NFC_CMD2_CODE(0x7EE0) |
                        NFC_CMD2_BUFNO(1);
      nfc_ptr->CAR  = col & 0xFFFF;
      nfc_ptr->RAR  = NFC_RAR_CS0_MASK |
                        NFC_RAR_RB0_MASK |
                        (row & 0xFFFFFF);
      /* Calculate metadata size for each page (exclude ECC size), 
      * since SPARE_ARE_SIZE is total spare size for each Physical page */
      
      nfc_ptr->SECSZ = num_metadata_bytes +num_ecc_bytes;
      
      /* For 16-bit data width flash devices, only odd SIZE is supported */
      if((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16) && !(nfc_ptr->SECSZ % 2))
         nfc_ptr->SECSZ  += 1;

      nfc_ptr->ISR  |= (NFC_ISR_DONECLR_MASK | 
                        NFC_ISR_DONEEN_MASK | 
                        NFC_ISR_IDLECLR_MASK |
                        NFC_ISR_IDLEEN_MASK);
      NFC_LOG("nfc_read_page_metadata: issue a READ cmd w/ col =%d page %d\n", col, count1);
      /* Start command execution */
      nfc_ptr->CMD2 |= NFC_CMD2_BUSY_START_MASK;
      
      for (count2 = 0; count2 <= MAX_WAIT_COMMAND; count2++)
      {
         if (nfc_ptr->ISR & NFC_ISR_IDLE_MASK)
         {
            for(count3 = 0; count3 < (num_metadata_bytes); count3++)
            {
               *(to_ptr + output_offset) = NFC_SRAM_B1_REG(nfc_ptr, count3);
               output_offset++;            
            }
            result = NANDFLASHERR_NO_ERROR;
            break;
         }
      }
   }
   
   nfc_ptr->CFG = cfg_bck;
   return(result);

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nfc_write_page_with_metadata
* Returned Value   : NANDFLASHERR_NO_ERROR or 
*                    NANDFLASHERR_WRITE_FAILED or
*                    NANDFLASHERR_TIMEOUT
* Comments         : 
*    This function writes one or several pages of NAND Flash 
*    using the NFC module.
* 
*END*----------------------------------------------------------------------*/

uint_32 nfc_write_page_with_metadata
   (  
      /* [IN] the NAND flash information */
      IO_NANDFLASH_WL_STRUCT_PTR nandflash_ptr,

      /* [IN] where to copy data from */
      uchar_ptr               data_from_ptr,
      
      /* [IN] where to copy metadata from */
      uchar_ptr               metadata_from_ptr,

      /* [IN] the first page to write */
      uint_32                 page_number,

      /* [IN] the amount of pages to write */
      uint_32                 page_count
   )
{ /* Body */
   uint_32 result = NANDFLASHERR_TIMEOUT;
   uint_32 count1, count2;
   uint_32 row, col, num_ecc_bytes, num_metadata_bytes;
   boolean swap = FALSE;
   uint_32 output_offset = 0;
   uint_32 output_metadata_offset = 0;
   uint_32 real_virt_page_offset;
   NFC_MemMapPtr  nfc_ptr;

   if (nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT) {
      (*nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT)(&nandflash_ptr->CORE_NANDFLASH, FALSE);
   } /* Endif */
   
   num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->CORE_NANDFLASH.ECC_SIZE);
   num_metadata_bytes = nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO 
                        - num_ecc_bytes;
   
   /* Get the pointer to nfc registers structure */
   nfc_ptr = (NFC_MemMapPtr)_bsp_get_nfc_address();
   
   for (count1 = page_number; count1 < (page_number + page_count); count1++)
   {        
      /* Calculate physical page address */
      row = count1/nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO;
      real_virt_page_offset = nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO   
                              - 1 - (count1 - (row * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO));
      
      /* Calculate byte offset in this physical page */
      /* For preserve bad marking block address, we writes all pages in reserved order.
      * Virtual page 0 <-> write physically in Virtual page 3
      * Virtual page 1 <-> write physically in Virtual page 2 */

      col = real_virt_page_offset  *(nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes + num_ecc_bytes);
      
      /* If the last virtual page of the first or the second physical page
      is about to be written the swapping needs to be switched on due 
      to the bad block marking */
      count2 = count1 % 
               ((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->BLOCK_SIZE / nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE) 
               * nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO);
      swap = ((count2 == (0)) ||
               (count2 == (nandflash_ptr->CORE_NANDFLASH.PHY_PAGE_SIZE_TO_VIRTUAL_PAGE_SIZE_RATIO)));
      
      if(swap)
      {
         nfc_ptr->SWAP  = NFC_SWAP_ADDR1(nfc_swap_addr1_with_metadata) | 
         NFC_SWAP_ADDR2(nfc_swap_addr2);
      }
      else
      {
         nfc_ptr->SWAP  = NFC_SWAP_ADDR1(0x7FF) | 
         NFC_SWAP_ADDR2(0x7FF);
      }

      if(nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16)
         col = col/2;
      
      /* Copy one virtual page data into the SRAM buffer #0 */
      for ( count2 = 0; count2 < (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE); count2++ )
      {
         NFC_SRAM_B0_REG(nfc_ptr, count2) = *(data_from_ptr + output_offset);
         output_offset++;
      }
      
      NFC_LOG("nfc_write_page_with_metadata: copy %d from page DATA to SRAM \n", count2);      
      
      /* Copy one virtual page metadata into the SRAM buffer #0 */
      for ( count2 = nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE;
      count2 < (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE + num_metadata_bytes); count2++)
      {
         NFC_SRAM_B0_REG(nfc_ptr, count2) = *(metadata_from_ptr + output_metadata_offset);
         output_metadata_offset++;
      }
      
      NFC_LOG("nfc_write_page_with_metadata: copy %d from page METADATA to SRAM \n", count2 - nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE);
      
      nfc_ptr->CMD1 = NFC_CMD1_BYTE2(NANDFLASH_CMD_PAGE_PROGRAM_CYCLE2) | 
                        NFC_CMD1_BYTE3(NANDFLASH_CMD_READ_STATUS);
      nfc_ptr->CMD2 = NFC_CMD2_BYTE1(NANDFLASH_CMD_PAGE_PROGRAM_CYCLE1) |  
                        NFC_CMD2_CODE(0x7FD8) |
                        NFC_CMD2_BUFNO(0);
      nfc_ptr->CAR  = col & 0xFFFF;
      nfc_ptr->RAR  = NFC_RAR_CS0_MASK |
                        NFC_RAR_RB0_MASK |
                        (row & 0xFFFFFF);
      nfc_ptr->SECSZ  = (nandflash_ptr->CORE_NANDFLASH.VIRTUAL_PAGE_SIZE)+ num_metadata_bytes + num_ecc_bytes;
      
      /* For 16-bit data width flash devices, only odd SIZE is supported */
      if((nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16) && !(nfc_ptr->SECSZ % 2))
         nfc_ptr->SECSZ  += 1;

      NFC_LOG("nfc_write_page_with_metadata: issue a WRITE cmd w/ SECTZ=%d at COL*2=%d page %d\n", 
      nfc_ptr->SECSZ, col*2, count1);
      nfc_ptr->ISR  |= (NFC_ISR_DONECLR_MASK | 
                        NFC_ISR_DONEEN_MASK | 
                        NFC_ISR_IDLECLR_MASK |
                        NFC_ISR_IDLEEN_MASK);
      
      /* Start command execution */
      nfc_ptr->CMD2 |= NFC_CMD2_BUSY_START_MASK;

      for (count2 = 0; count2 <= MAX_WAIT_COMMAND; count2++)
      {
         if (nfc_ptr->ISR & NFC_ISR_IDLE_MASK)
         {
            if (nfc_ptr->SR2 & NANDFLASH_STATUS_ERR)
            {
               result = NANDFLASHERR_WRITE_FAILED;
               goto exit;
            }
            else
            {
               result = NANDFLASHERR_NO_ERROR;
            }
            break;
         }
      }
   }

exit:
   if (nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT) {
      (*nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT)(&nandflash_ptr->CORE_NANDFLASH, TRUE);
   }/* Endif */
   
   nfc_dump_buff(data_from_ptr, output_offset, "Write Page Buffer");
   nfc_dump_buff(metadata_from_ptr, output_metadata_offset,
   "Write Page Metadata Buffer");
   
   return(result);

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nfc_read_phy_page_raw
* Returned Value   : NANDFLASHERR_NO_ERROR or 
*                    NANDFLASHERR_ECC_FAILED or
*                    NANDFLASHERR_ECC_CORRECTED or
*                    NANDFLASHERR_TIMEOUT
* Comments         : 
*    This function reads one or several pages of NAND Flash 
*    using the NFC module.
* 
*END*----------------------------------------------------------------------*/

uint_32 nfc_read_phy_page_raw
   (  
      /* [IN] the NAND flash information */
      IO_NANDFLASH_WL_STRUCT_PTR nandflash_ptr,

      /* [OUT} where to copy data to */
      uchar_ptr               to_ptr,

      /* [IN] the page to read */
      uint_32                 phy_page_number,

      /* [IN] the amount of pages to read */
      uint_32                 page_count
   )
{ /* Body */
   uint_32 result = NANDFLASHERR_TIMEOUT;
   uint_32 count1, count2, count3;
   uint_32 output_offset = 0;   
   uint_32 row, col;
   uint_32 num_ecc_bytes;
   
   uint_32 count, cfg_bck;

   NFC_MemMapPtr  nfc_ptr;

   /* Get the pointer to nfc registers structure */
   nfc_ptr = (NFC_MemMapPtr)_bsp_get_nfc_address();
   
   
   /* Set the ECCMODE to 0 - ECC bypass */
   cfg_bck = nfc_ptr->CFG;
   nfc_ptr->CFG &= ~(NFC_CFG_ECCMODE(7));
   /* Do not write ECC Status to SRAM */
   nfc_ptr->CFG &= ~(NFC_CFG_ECCSRAM_MASK);
 
   /* Reset the swap register */
   nfc_ptr->SWAP  = NFC_SWAP_ADDR1(0x7FF) | 
                    NFC_SWAP_ADDR2(0x7FF);

   for (count1 = phy_page_number; count1 < (phy_page_number + page_count); count1++)
   {
      num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->CORE_NANDFLASH.ECC_SIZE);
      row = count1;
      /* Read from begin of physical page */
      col = 0;
      /* Is the bad block byte(s) about to be re-written? */

      nfc_ptr->CMD1 = NFC_CMD1_BYTE2(NANDFLASH_CMD_PAGE_READ_CYCLE2);
      nfc_ptr->CMD2 = NFC_CMD2_BYTE1(NANDFLASH_CMD_PAGE_READ_CYCLE1) | 
                     NFC_CMD2_CODE(0x7EE0) |
                     NFC_CMD2_BUFNO(1);
      nfc_ptr->CAR  = col & 0xFFFF;
      nfc_ptr->RAR  = NFC_RAR_CS0_MASK |
                     NFC_RAR_RB0_MASK |
                     (row & 0xFFFFFF);
      nfc_ptr->SECSZ  = nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE + nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE ;


      nfc_ptr->ISR  |= (NFC_ISR_DONECLR_MASK | 
                       NFC_ISR_DONEEN_MASK | 
                       NFC_ISR_IDLECLR_MASK |
                       NFC_ISR_IDLEEN_MASK);
    
      /* Start command execution */
      nfc_ptr->CMD2 |= NFC_CMD2_BUSY_START_MASK;

      for (count2 = 0; count2 <= MAX_WAIT_COMMAND; count2++)
      {
         if (nfc_ptr->ISR & NFC_ISR_IDLE_MASK)
         {
            for(count3 = 0; count3 < (nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE + nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE); count3++)
            {
               *(to_ptr + output_offset) = NFC_SRAM_B1_REG(nfc_ptr, count3);
               output_offset++;            
            }
            result = NANDFLASHERR_NO_ERROR;

            break; /* break for (count2 = 0;... */

         } /* if nfc_ptr->ISR & NFC_ISR_IDLE */ 
      }
   } /* for (count1 = phy_page_number */

   nfc_ptr->CFG = cfg_bck;

   return(result);

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nfc_write_phy_page_raw
* Returned Value   : NANDFLASHERR_NO_ERROR or 
*                    NANDFLASHERR_WRITE_FAILED or
*                    NANDFLASHERR_TIMEOUT
* Comments         : 
*    This function writes one or several pages of NAND Flash 
*    using the NFC module.
* 
*END*----------------------------------------------------------------------*/

uint_32 nfc_write_phy_page_raw
   (  
      /* [IN] the NAND flash information */
      IO_NANDFLASH_WL_STRUCT_PTR nandflash_ptr,

      /* [IN] where to copy data from */
      uchar_ptr               from_ptr,

      /* [IN] the first page to write */
      uint_32                 phy_page_number,

      /* [IN] the amount of pages to write */
      uint_32                 page_count
   )
{ /* Body */
   uint_32 result = NANDFLASHERR_TIMEOUT;
   uint_32 count1, count2;
   uint_32 row, col, num_ecc_bytes;
   boolean swap = FALSE;
   uint_32 cfg_bck;

   NFC_MemMapPtr  nfc_ptr;

   if (nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT) {
      (*nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT)(&nandflash_ptr->CORE_NANDFLASH, FALSE);
   }/* Endif */
   
   /* Get the pointer to nfc registers structure */
   nfc_ptr = (NFC_MemMapPtr)_bsp_get_nfc_address();
   
      /* Set the ECCMODE to 0 - ECC bypass */
   cfg_bck = nfc_ptr->CFG;
   nfc_ptr->CFG &= ~(NFC_CFG_ECCMODE(7));
   
   /* Reset the swap register */
   nfc_ptr->SWAP  = NFC_SWAP_ADDR1(0x7FF) | 
                    NFC_SWAP_ADDR2(0x7FF);


   for (count1 = phy_page_number; count1 < (phy_page_number + page_count); count1++)
   {
      num_ecc_bytes = NANDFLASH_ECC_SIZE_TO_NUM_BYTES_CONV(nandflash_ptr->CORE_NANDFLASH.ECC_SIZE);
      row = count1;
      col =0;
     

      if(nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->WIDTH == 16)
         col = col/2;
     
      /* Copy one virtual page into the SRAM buffer #0 */
      for ( count2 = 0; count2 < (nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE + nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE); count2++ )
      {
         // nfc_ptr->SRAM_B0[count2] = *(from_ptr + count2);
         NFC_SRAM_B0_REG(nfc_ptr, count2) = *(from_ptr + count2);
      }
      
      nfc_ptr->CMD1 = NFC_CMD1_BYTE2(NANDFLASH_CMD_PAGE_PROGRAM_CYCLE2) | 
                     NFC_CMD1_BYTE3(NANDFLASH_CMD_READ_STATUS);
      nfc_ptr->CMD2 = NFC_CMD2_BYTE1(NANDFLASH_CMD_PAGE_PROGRAM_CYCLE1) |  
                     NFC_CMD2_CODE(0x7FD8) |
                     NFC_CMD2_BUFNO(0);
      nfc_ptr->CAR  = col & 0xFFFF;
      nfc_ptr->RAR  = NFC_RAR_CS0_MASK |
                     NFC_RAR_RB0_MASK |
                     (row & 0xFFFFFF);
      nfc_ptr->SECSZ  = nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE + nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->SPARE_AREA_SIZE ;
      

      nfc_ptr->ISR  |= (NFC_ISR_DONECLR_MASK | 
                     NFC_ISR_DONEEN_MASK | 
                     NFC_ISR_IDLECLR_MASK |
                     NFC_ISR_IDLEEN_MASK);
      
      /* Start command execution */
      nfc_ptr->CMD2 |= NFC_CMD2_BUSY_START_MASK;

      for (count2 = 0; count2 <= MAX_WAIT_COMMAND; count2++)
      {
         if (nfc_ptr->ISR & NFC_ISR_IDLE_MASK)
         {
            if (nfc_ptr->SR2 & NANDFLASH_STATUS_ERR)
            {
               result = NANDFLASHERR_WRITE_FAILED;
               goto exit;
            }
            else
            {
               result = NANDFLASHERR_NO_ERROR;
            }
            break;
         }
      }
      from_ptr += nandflash_ptr->CORE_NANDFLASH.NANDFLASH_INFO_PTR->PHY_PAGE_SIZE;
   }

exit:
   if (nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT) {
      (*nandflash_ptr->CORE_NANDFLASH.WRITE_PROTECT)(&nandflash_ptr->CORE_NANDFLASH, TRUE);
   }/* Endif */
   
   nfc_ptr->CFG = cfg_bck;
   
   return(result);

} /* Endbody */

/* EOF */

