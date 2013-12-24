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
* $FileName: mem_management.h$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: Memory management and profiling for WL Module 
*END************************************************************************/
#ifndef _mem_management_h_
#define _mem_management_h_

#define _mm_internal_alloc              _mem_alloc_system
#define _mm_internal_alloc_zero         _mem_alloc_system_zero
#define _mm_internal_free               _mem_free
#define _mm_internal_set                memset
#define _mm_internal_zero               _mem_zero
#define _mm_internal_memcpy             _mem_copy


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern pointer mm_alloc(_mem_size request_size, const char * file_name, 
    int file_loc, boolean isZero, boolean is_from_operator);
    extern _mqx_uint mm_free(pointer buf);
    extern void mm_display(void);
    extern void mm_display_unfree(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#ifdef __cplusplus
#if defined(__IAR_SYSTEMS_ICC__) || defined(__CC_ARM) || defined (__GNUG__) // TODO: IAR or KEIL
typedef unsigned int size_t;
#else
typedef _mem_size size_t;
#endif

void * operator new (size_t size, const char *fname, int floc);
void * operator new[] (size_t size, const char *fname, int floc);
void operator delete (void *p);
void operator delete[] (void *p);

#endif /* __cplusplus */

#endif /* _mem_management_h_ */

/* EOF */
