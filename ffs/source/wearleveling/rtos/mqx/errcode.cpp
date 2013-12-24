/**HEADER********************************************************************
* Copyright (c) 2013 Freescale Semiconductor, Inc. All rights reserved.
* 
* Freescale Semiconductor, Inc.
* Proprietary & Confidential
* 
* This source code and the algorithms implemented therein constitute
* confidential information and may comprise trade secrets of Freescale Semiconductor, Inc.
* or its associates, and any use thereof is subject to the terms and
* conditions of the Confidential Disclosure Agreement pursual to which this
* source code was originally received.
*****************************************************************************
* $FileName: errcode.c$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: 
*   Implementation error code convert between WL module and specific RTOS
*END************************************************************************/

#include "wl_common.h"
#include "ddi_media_errordefs.h"
#include "errcode.h"

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : os_err_code_to_wl
* Returned Value   : WearLevel's status
* Comments         :
*   Convert from MQX error -> WearLeveling status 
*
*END*--------------------------------------------------------------------*/
RtStatus_t os_err_code_to_wl
(
    /* [IN] MQX error/status */
    _mqx_uint  status,
    /* [IN] context for this MQX err/status */
    uint_32 context
)
{ /* Body */
    switch (status)
    {
        case NANDFLASHERR_NO_ERROR:
            return SUCCESS;
        case NANDFLASHERR_ECC_FAILED:
            return ERROR_DDI_NAND_HAL_ECC_FIX_FAILED;
        case NANDFLASHERR_ECC_CORRECTED:
            return ERROR_DDI_NAND_HAL_ECC_FIXED;
        case NANDFLASHERR_ECC_CORRECTED_EXCEED_THRESHOLD:
            return ERROR_DDI_NAND_HAL_ECC_FIXED_REWRITE_SECTOR;
        case NANDFLASHERR_ERASE_FAILED:
            return ERROR_DDI_NAND_ERASE_FAILED;
        case NANDFLASHERR_WRITE_FAILED:
            return ERROR_DDI_NAND_HAL_WRITE_FAILED;
        case NANDFLASHERR_TIMEOUT:
            if (kNandWritePage == context)
            {
                return ERROR_DDI_NAND_HAL_WRITE_FAILED;
            }
            else if (kNandReadMetadata == context || kNandReadPage == context)
            {
                // TODO: Reconsider it
                return ERROR_DDI_NAND_HAL_ECC_FIX_FAILED;
            }
            else
            {
                return ERROR_DDI_LDL_GENERAL;
            }/* Endif */  

        default:
            return ERROR_DDI_LDL_GENERAL;
    } /* Endswitch */
} /* Endbody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : wl_err_code_to_os
* Returned Value   : MQX error
* Comments         :
*   Convert from WearLeveling status -> MQX error
*
*END*--------------------------------------------------------------------*/
_mqx_uint wl_err_code_to_os
( 
    /* [IN] WL's status */
    RtStatus_t status, 
    /* [IN] context for this MQX err/status */
    uint_32 context
)
{/* Body*/
    return status;
}/* Endbody*/

/*FUNCTION*----------------------------------------------------------------
*
* Function Name    : wl_err_to_str
* Returned Value   : error string
* Comments         :
*   Convert from WearLeveling status -> WL error string
*
*END*--------------------------------------------------------------------*/
char_ptr wl_err_to_str
(
    /* [IN] WL's status */
    RtStatus_t  wl_status,
    /* [IN] WLcontext for this MQX err/status */ 
    uint_32 context
)
{ /* Body */
    switch (wl_status)
    {

        case ERROR_DDI_LDL_GENERAL:
            return ("ERROR_DDI_LDL_GENERAL");
        case ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_NUMBER:
            return ("ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_NUMBER");
        case ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED:
            return ("ERROR_DDI_LDL_LDRIVE_NOT_INITIALIZED");
        case ERROR_DDI_LDL_LDRIVE_HARDWARE_FAILURE:
            return ("ERROR_DDI_LDL_LDRIVE_HARDWARE_FAILURE");
        case ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TYPE:
            return ("ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TYPE");

        case ERROR_DDI_LDL_LDRIVE_INVALID_INFO_TYPE:
            return ("ERROR_DDI_LDL_LDRIVE_INVALID_INFO_TYPE");

        case ERROR_DDI_LDL_LDRIVE_SECTOR_OUT_OF_BOUNDS:
            return ("ERROR_DDI_LDL_LDRIVE_SECTOR_OUT_OF_BOUNDS");

        case ERROR_DDI_LDL_LDRIVE_WRITE_PROTECTED:
            return ("ERROR_DDI_LDL_LDRIVE_WRITE_PROTECTED");

        case ERROR_DDI_LDL_LDRIVE_WRITE_ABORT:
            return ("ERROR_DDI_LDL_LDRIVE_WRITE_ABORT");

        case ERROR_DDI_LDL_LDRIVE_MEDIA_NOT_ALLOCATED:
            return ("ERROR_DDI_LDL_LDRIVE_MEDIA_NOT_ALLOCATED");

        case ERROR_DDI_LDL_LDRIVE_LOW_LEVEL_MEDIA_FORMAT_REQUIRED:
            return ("ERROR_DDI_LDL_LDRIVE_LOW_LEVEL_MEDIA_FORMAT_REQUIRED");

        case ERROR_DDI_LDL_LMEDIA_HARDWARE_FAILURE:
            return ("ERROR_DDI_LDL_LMEDIA_HARDWARE_FAILURE");

        case ERROR_DDI_LDL_LMEDIA_INVALID_MEDIA_NUMBER:
            return ("ERROR_DDI_LDL_LMEDIA_INVALID_MEDIA_NUMBER");

        case ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED:
            return ("ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_INITIALIZED");

        case ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_DISCOVERED:
            return ("ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_DISCOVERED");

        case ERROR_DDI_LDL_LMEDIA_INVALID_MEDIA_INFO_TYPE:
            return ("ERROR_DDI_LDL_LMEDIA_INVALID_MEDIA_INFO_TYPE");

        case ERROR_DDI_LDL_LMEDIA_ALLOCATION_TOO_LARGE:
            return ("ERROR_DDI_LDL_LMEDIA_ALLOCATION_TOO_LARGE");

        case ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_ERASED:
            return ("ERROR_DDI_LDL_LMEDIA_MEDIA_NOT_ERASED");

        case ERROR_DDI_LDL_LMEDIA_MEDIA_ERASED:
            return ("ERROR_DDI_LDL_LMEDIA_MEDIA_ERASED");

        case ERROR_DDI_LDL_LMEDIA_MEDIA_ALLOCATE_BOUNDS_EXCEEDED:
            return ("ERROR_DDI_LDL_LMEDIA_MEDIA_ALLOCATE_BOUNDS_EXCEEDED");

        case ERROR_DDI_LDL_LDRIVE_DRIVE_NOT_RECOVERABLE:
            return ("ERROR_DDI_LDL_LDRIVE_DRIVE_NOT_RECOVERABLE");

        case ERROR_DDI_LDL_LDRIVE_INVALID_SECTOR_SIZE:
            return ("ERROR_DDI_LDL_LDRIVE_INVALID_SECTOR_SIZE");

        case ERROR_DDI_LDL_LDRIVE_ALREADY_INITIALIZED:
            return ("ERROR_DDI_LDL_LDRIVE_ALREADY_INITIALIZED");

        case ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TAG:
            return ("ERROR_DDI_LDL_LDRIVE_INVALID_DRIVE_TAG");

        case ERROR_DDI_LDL_ITERATOR_DONE:
            return ("ERROR_DDI_LDL_ITERATOR_DONE");

        case ERROR_DDI_LDL_UNIMPLEMENTED:
            return ("ERROR_DDI_LDL_UNIMPLEMENTED");

        case ERROR_DDI_LDL_LDRIVE_FS_FORMAT_REQUIRED:
            return ("ERROR_DDI_LDL_LDRIVE_FS_FORMAT_REQUIRED");

        case ERROR_DDI_MEDIABUFMGR_GENERAL:
            return ("ERROR_DDI_MEDIABUFMGR_GENERAL");

        case ERROR_DDI_MEDIABUFMGR_NO_ROOM:
            return ("ERROR_DDI_MEDIABUFMGR_NO_ROOM");

        case ERROR_DDI_MEDIABUFMGR_INVALID_BUFFER:
            return ("ERROR_DDI_MEDIABUFMGR_INVALID_BUFFER");

        case ERROR_DDI_MEDIABUFMGR_ALLOC_FAILED:
            return ("ERROR_DDI_MEDIABUFMGR_ALLOC_FAILED");

        case ERROR_DDI_MEDIABUFMGR_INVALID_PROPERTY:
            return ("ERROR_DDI_MEDIABUFMGR_INVALID_PROPERTY");
        case ERROR_DDI_LBA_NAND_GENERAL:
            return ("ERROR_DDI_LBA_NAND_GENERAL");

        case ERROR_DDI_LBA_NAND_UNKNOWN_DEVICE_TYPE:
            return ("ERROR_DDI_LBA_NAND_UNKNOWN_DEVICE_TYPE");

        case ERROR_DDI_LBA_NAND_VFP_SIZE_TOO_LARGE:
            return ("ERROR_DDI_LBA_NAND_VFP_SIZE_TOO_LARGE");

        case ERROR_DDI_LBA_NAND_MODE_NOT_SET:
            return ("ERROR_DDI_LBA_NAND_MODE_NOT_SET");

        case ERROR_DDI_LBA_NAND_ADDRESS_OUT_OF_RANGE:
            return ("ERROR_DDI_LBA_NAND_ADDRESS_OUT_OF_RANGE");

        case ERROR_DDI_LBA_NAND_SPARE_BLOCKS_EXHAUSTED:
            return ("ERROR_DDI_LBA_NAND_SPARE_BLOCKS_EXHAUSTED");

        case ERROR_DDI_LBA_NAND_UNKNOWN_VFP_CAPACITY_MODEL_TYPE:
            return ("ERROR_DDI_LBA_NAND_UNKNOWN_VFP_CAPACITY_MODEL_TYPE");

        case ERROR_DDI_LBA_NAND_VFP_SIZE_PARADOX:
            return ("ERROR_DDI_LBA_NAND_VFP_SIZE_PARADOX");

        case ERROR_DDI_LBA_NAND_WRITE_FAILED:
            return ("ERROR_DDI_LBA_NAND_WRITE_FAILED");

        case ERROR_DDI_LBA_NAND_SET_VFP_SIZE_FAILED:
            return ("ERROR_DDI_LBA_NAND_SET_VFP_SIZE_FAILED");

        case ERROR_DDI_NAND_GROUP_GENERAL:
            return ("ERROR_DDI_NAND_GROUP_GENERAL");

        case ERROR_DDI_NAND_LMEDIA_NO_REGIONS_IN_MEDIA:
            return ("ERROR_DDI_NAND_LMEDIA_NO_REGIONS_IN_MEDIA");

        case ERROR_DDI_NAND_LMEDIA_DRIVES_MAX_OUT:
            return ("ERROR_DDI_NAND_LMEDIA_DRIVES_MAX_OUT");

        case ERROR_DDI_NAND_LMEDIA_MEDIA_WRITE_PROTECTED:
            return ("ERROR_DDI_NAND_LMEDIA_MEDIA_WRITE_PROTECTED");

        case ERROR_DDI_NAND_LMEDIA_BAD_BLOCKS_MAX_OUT:
            return ("ERROR_DDI_NAND_LMEDIA_BAD_BLOCKS_MAX_OUT");

        case ERROR_DDI_NAND_LMEDIA_NOT_ALLOCATED:
            return ("ERROR_DDI_NAND_LMEDIA_NOT_ALLOCATED");

        case ERROR_DDI_NAND_CONFIG_BLOCK_NOT_FOUND:
            return ("ERROR_DDI_NAND_CONFIG_BLOCK_NOT_FOUND");

        case ERROR_DDI_NAND_DATA_DRIVE_CANT_RECYCLE_USECTOR_MAP:
            return ("ERROR_DDI_NAND_DATA_DRIVE_CANT_RECYCLE_USECTOR_MAP");

        case ERROR_DDI_NAND_DATA_DRIVE_UBLOCK_LBAS_INCONSISTENT:
            return ("ERROR_DDI_NAND_DATA_DRIVE_UBLOCK_LBAS_INCONSISTENT");

        case ERROR_DDI_NAND_DATA_DRIVE_UBLOCK_HSECTORIDX_OUT_OF_RANGE:
            return ("ERROR_DDI_NAND_DATA_DRIVE_UBLOCK_HSECTORIDX_OUT_OF_RANGE");

        case ERROR_DDI_NAND_DATA_DRIVE_CANT_RECYCLE_UBLOCK:
            return ("ERROR_DDI_NAND_DATA_DRIVE_CANT_RECYCLE_UBLOCK");

        case ERROR_DDI_NAND_DATA_DRIVE_BBTABLE_FULL:
            return ("ERROR_DDI_NAND_DATA_DRIVE_BBTABLE_FULL");

        case ERROR_DDI_NAND_DATA_DRIVE_CANT_ALLOCATE_USECTORS_MAPS:
            return ("ERROR_DDI_NAND_DATA_DRIVE_CANT_ALLOCATE_USECTORS_MAPS");

        case ERROR_DDI_NAND_DATA_DRIVE_INVALID_LOGICAL_SECTOR:
            return ("ERROR_DDI_NAND_DATA_DRIVE_INVALID_LOGICAL_SECTOR");

        case ERROR_DDI_NAND_HAL_NANDTYPE_MISMATCH:
            return ("ERROR_DDI_NAND_HAL_NANDTYPE_MISMATCH");

        case ERROR_DDI_NAND_HAL_LOOKUP_ID_FAILED:
            return ("ERROR_DDI_NAND_HAL_LOOKUP_ID_FAILED");

        case ERROR_DDI_NAND_HAL_WRITE_FAILED:
            return ("ERROR_DDI_NAND_HAL_WRITE_FAILED");

        case ERROR_DDI_NAND_HAL_ECC_FIXED:
            return ("ERROR_DDI_NAND_HAL_ECC_FIXED");

        case ERROR_DDI_NAND_HAL_ECC_FIXED_REWRITE_SECTOR:
            return ("ERROR_DDI_NAND_HAL_ECC_FIXED_REWRITE_SECTOR");

        case ERROR_DDI_NAND_HAL_ECC_FIX_FAILED:
            return ("ERROR_DDI_NAND_HAL_ECC_FIX_FAILED");

        case ERROR_DDI_NAND_DMA_TIMEOUT:
            return ("ERROR_DDI_NAND_DMA_TIMEOUT");

        case ERROR_DDI_NAND_ERASE_FAILED:
            return ("ERROR_DDI_NAND_ERASE_FAILED");

        case ERROR_DDI_NAND_RESET_FAILED:
            return ("ERROR_DDI_NAND_RESET_FAILED");

        case ERROR_DDI_NAND_PROGRAM_FAILED:
            return ("ERROR_DDI_NAND_PROGRAM_FAILED");

        case ERROR_DDI_NAND_D_BAD_BLOCK_TABLE_BLOCK_NOT_FOUND:
            return ("ERROR_DDI_NAND_D_BAD_BLOCK_TABLE_BLOCK_NOT_FOUND");

        case ERROR_DDI_NAND_CANT_ALLOCATE_DBBT_BLOCK:
            return ("ERROR_DDI_NAND_CANT_ALLOCATE_DBBT_BLOCK");

        case ERROR_DDI_NAND_MEDIA_FINDING_NEXT_VALID_BLOCK:
            return ("ERROR_DDI_NAND_MEDIA_FINDING_NEXT_VALID_BLOCK");

        case ERROR_DDI_NAND_MEDIA_CANT_ALLOCATE_BCB_BLOCK:
            return ("ERROR_DDI_NAND_MEDIA_CANT_ALLOCATE_BCB_BLOCK");

        case ERROR_DDI_NAND_DRIVER_NO_BCB:
            return ("ERROR_DDI_NAND_DRIVER_NO_BCB");

        case ERROR_DDI_NAND_BCB_SEARCH_FAILED:
            return ("ERROR_DDI_NAND_BCB_SEARCH_FAILED");

        case ERROR_DDI_NAND_INVALID_BOOT_IMAGE_FORMAT:
            return ("ERROR_DDI_NAND_INVALID_BOOT_IMAGE_FORMAT");

        case ERROR_DDI_NAND_NO_MATCHING_SECTION:
            return ("ERROR_DDI_NAND_NO_MATCHING_SECTION");

        case ERROR_DDI_NAND_MAPPER_UNITIALIZED:
            return ("ERROR_DDI_NAND_MAPPER_UNITIALIZED");

        case ERROR_DDI_NAND_MAPPER_INVALID_PHYADDR:
            return ("ERROR_DDI_NAND_MAPPER_INVALID_PHYADDR");

        case ERROR_DDI_NAND_MAPPER_GET_BLOCK_FAILED:
            return ("ERROR_DDI_NAND_MAPPER_GET_BLOCK_FAILED");

        case ERROR_DDR_NAND_MAPPER_PHYMAP_MAPFULL:
            return ("ERROR_DDR_NAND_MAPPER_PHYMAP_MAPFULL");

        case ERROR_DDI_NAND_MAPPER_FIND_LBAMAP_BLOCK_FAILED:
            return ("ERROR_DDI_NAND_MAPPER_FIND_LBAMAP_BLOCK_FAILED");

        case ERROR_DDI_NAND_MAPPER_LBA_OUTOFBOUND:
            return ("ERROR_DDI_NAND_MAPPER_LBA_OUTOFBOUND");

        case ERROR_DDI_NAND_MAPPER_PAGE_OUTOFBOUND:
            return ("ERROR_DDI_NAND_MAPPER_PAGE_OUTOFBOUND");

        case ERROR_DDI_NAND_MAPPER_LBA_CORRUPTED:
            return ("ERROR_DDI_NAND_MAPPER_LBA_CORRUPTED");

        case ERROR_DDI_NAND_MAPPER_ZONE_MAP_CACHE_INIT_FAILED:
            return ("ERROR_DDI_NAND_MAPPER_ZONE_MAP_CACHE_INIT_FAILED");

        case ERROR_DDI_NAND_MAPPER_ZONE_MAP_INSUFFICIENT_NUM_FREE_BLOCKS:
            return ("ERROR_DDI_NAND_MAPPER_ZONE_MAP_INSUFFICIENT_NUM_FREE_BLOCKS");

        case ERROR_DDI_NAND_DATA_DRIVE_SECTOR_OUT_OF_BOUNDS:
            return ("ERROR_DDI_NAND_DATA_DRIVE_SECTOR_OUT_OF_BOUNDS");

        case ERROR_DDI_NAND_HAL_CHECK_STATUS_FAILED:
            return ("ERROR_DDI_NAND_HAL_CHECK_STATUS_FAILED");

        case ERROR_DDI_NAND_FIRMWARE_REFRESH_BUSY:
            return ("ERROR_DDI_NAND_FIRMWARE_REFRESH_BUSY");

        case ERROR_DDI_NAND_CONFIG_BLOCK_VERSION_MISMATCH:
            return ("ERROR_DDI_NAND_CONFIG_BLOCK_VERSION_MISMATCH");

        case ERROR_DDI_NAND_DRIVER_NCB_TRIPLE_RED_CHK_FAILED:
            return ("ERROR_DDI_NAND_DRIVER_NCB_TRIPLE_RED_CHK_FAILED");

        case ERROR_DDI_NAND_DRIVER_NCB_SYNDROME_TABLE_MISMATCH:
            return ("ERROR_DDI_NAND_DRIVER_NCB_SYNDROME_TABLE_MISMATCH");

        case ERROR_DDI_NAND_DRIVER_NCB_HAMMING_DOUBLE_ERROR:
            return ("ERROR_DDI_NAND_DRIVER_NCB_HAMMING_DOUBLE_ERROR");

        case ERROR_DDI_NAND_HAL_INVALID_ONFI_PARAM_PAGE:
            return ("ERROR_DDI_NAND_HAL_INVALID_ONFI_PARAM_PAGE");

        default:
            return "NAND WL Unknow Error";
    } /* Endswitch */
} /* Endbody */
