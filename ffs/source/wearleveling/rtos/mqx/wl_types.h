#ifndef _WL_TYPES_H_
#define _WL_TYPES_H_
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
* $FileName: wl_types.h$
* $Version : 3.8.0.2$
* $Date    : Aug-23-2012$
*
* Comments: Type & define bridging between WL module and specific RTOS
*END************************************************************************/

/* NANDWL_USE_MM_LEAK_DETECTION uses to enable Memory Profiling */
#if NANDWL_USE_MM_LEAK_DETECTION 
#include "mem_management.h"
#endif /* NANDWL_USE_MM_LEAK_DETECTION */

/*--------------------------------------------------------------**
** ASSERT macros, assume _ASSERT_ is defined during development **
**    (perhaps in the make file), and undefined for production. **
** This macro will check for pointer values and other validations
** wherever appropriate.
**--------------------------------------------------------------*/
#define _ASSERT_
#ifndef _ASSERT_
#define ASSERT(cond)
#define assert(cond) ASSERT(cond)
#else /* _ASSERT_ */
#define ASSERT(cond) { \
        if (!(cond)) {          \
            printf("assertion failed: %s, file %s, line %d \n",#cond,__FILE__,__LINE__); \
            while (1){}; \
        } }

#define assert(cond) ASSERT(cond)
#endif /* _ASSERT_*/

/*--------------------------------------------------------------**
** Logging function                                             **
**--------------------------------------------------------------*/


#define WL_LOG_DISABLE              0
#define WL_LOG_ERROR                1
#define WL_LOG_WARNING              2
#define WL_LOG_INFO                 3
#define WL_LOG_DEBUG                4

#define WL_MODULE_GENERAL           (0x01<<4)
#define WL_MODULE_HAL               (0x02<<4)
#define WL_MODULE_MAPPER            (0x04<<4)
#define WL_MODULE_MEDIA             (0x08<<4)
#define WL_MODULE_DEFEERREDTASK     (0x20<<4)
#define WL_MODULE_MEDIABUFFER       (0x40<<4)
#define WL_MODULE_LOGICALDRIVE      (0x80<<4)
#define WL_MODULE_NANDWL            (0x100<<4)
#define WL_MODULE_NANDWL_TESTING    (0x200<<4)


typedef TIME_STRUCT WL_TIME_STRUCT;
#define WL_GET_TIME(time)            _time_get(time)

/* Loging configuration */
extern uint_32 g_wl_log_module_attributes;

/* NAND WL debug info level ( 0 - 4 ) */
extern uint_32 g_wl_log_level;

/** NAND WL debug macro
*/
#if  ( (WL_DEBUG))

#define WL_LOG(mdl, lev, ...)   {           \
        if ((g_wl_log_module_attributes & mdl )&& (lev <= g_wl_log_level))   {           \
            WL_TIME_STRUCT time;             \
            WL_GET_TIME(&time);              \
            printf("WL%u (%s:%d) %4d.%3d: ",lev, __FILE__, __LINE__, time.SECONDS, time.MILLISECONDS);          \
            printf(__VA_ARGS__);                \
        }                                       \
    }

#else /* WL_DEBUG */
#define WL_LOG(mdl, lev, ...) {}

#endif /* WL_DEBUG */


/*--------------------------------------------------------------------------*/
/*
**                  SPECIAL MEMORY ALLOCATOR OPTION
*/

#if NANDWL_USE_MM_LEAK_DETECTION

#define _wl_mem_alloc(s)            mm_alloc(s, __FILE__, __LINE__, FALSE, FALSE)
#define _wl_mem_alloc_zero(s)       mm_alloc(s, __FILE__, __LINE__, TRUE, FALSE)
#define _wl_mem_alloc_uncache       _mem_alloc


#define _wl_mem_free                mm_free
#define _wl_mem_set                 memset
#define _wl_mem_copy(d,s,len)       _mem_copy(s,d,len)

#ifndef _mem_management_internal_h_
#define new                         new(__FILE__, __LINE__)
#define delete                      delete
#endif /* _mem_management_internal_h_ */

#else
#define _wl_mem_alloc               _mem_alloc
#define _wl_mem_alloc_zero          _mem_alloc_zero
#define _wl_mem_alloc_uncache       _mem_alloc

#define _wl_mem_free                _mem_free
#define _wl_mem_set                 memset
#define _wl_mem_copy(d,s,len)       _mem_copy(s,d,len)
#endif /* NANDWL_USE_MM_LEAK_DETECTION */





/********************************************************************/
/*
* The basic data types
*/
// TODO: Declare basic data types for cases which there aren't these types in
#if !defined(BSP_TWRMCF54418) && !defined(BSP_TWRMPC5125) && !defined(BSP_TWR_K70F120M) && !defined(BSP_TWR_K60F120M) && !(MQX_CPU==PSP_CPU_VF65GS10_M4) && !(MQX_CPU==PSP_CPU_VF65GS10_A5) && !defined(__IAR_SYSTEMS_ICC__)
typedef int_8                   int8_t;
typedef int_16                  int16_t;
typedef int_32                  int32_t;
typedef int_64                  int64_t;

typedef uint_8                  uint8_t;
typedef uint_16                 uint16_t;
typedef uint_32                 uint32_t;
typedef uint_64                 uint64_t;
#endif
typedef unsigned long           ULONG;

typedef _mqx_uint               RtStatus_t;
typedef uint_8                  SECTOR_BUFFER;

#if defined(__GNUC__)
#define LLONG_MAX               0x7fffffffffffffffLL
#define ULLONG_MAX              0xffffffffffffffffULL
#define LLONG_MIN               (-LLONG_MAX - 1)
#endif
/********************************************************************/
/*
* The basic definitions
*/

#define SUCCESS                 MQX_OK
#define ERROR_OUT_OF_MEMORY     MQX_OUT_OF_MEMORY

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WL_TYPES_H_ */

/* EOF */
