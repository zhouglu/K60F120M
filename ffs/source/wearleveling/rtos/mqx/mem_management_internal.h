#ifndef _mem_management_internal_h_
#define _mem_management_internal_h_
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
* $FileName: mem_management_internal.h$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: Memory management.
*END************************************************************************/

typedef struct _mem_entry_head_struct {
    // uint_8 file_name[15];
    const char* file_name;
    int         file_loc;
    uint_32     requested_size;
    boolean     is_from_operator;
    // TODO: 64bits signature does not support on IAR
    // uint_64   buf_head_sign; 
    uint_32     buf_head_sign;
} MEM_ENTRY_HEAD, _PTR_ MEM_ENTRY_HEAD_PTR;

typedef struct _mem_entry_tail {
    // TODO: 64bits signature does not support on IAR
    // uint_64 buf_tail_sign;
    uint_32 buf_tail_sign;
} MEM_ENTRY_TAIL, _PTR_ MEM_ENTRY_TAIL_PTR;

#define MEM_MAX_SAVE_BUF_PTR    200
typedef struct _mem_allocation_table_struct {
    uint_32 totalAllocSize;
    uint_32 totalFreedSize;
    uint_32 totalNewSize;
    uint_32 totalDeleteSize;
    MEM_ENTRY_HEAD_PTR buf_ptr[MEM_MAX_SAVE_BUF_PTR];
    uint_32 nextSaveIdx;
// TODO: Fix race-condition with a semaphore */
} MEM_ALLOCATION_TABLE, _PTR_ MEM_ALLOCATION_TABLE_PTR;

extern MEM_ALLOCATION_TABLE g_mem_allocation_table;

#endif //_mem_management_internal_h_

/* EOF */


