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
* $FileName: simple_timer.cpp$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: Simple time operations .
*END************************************************************************/

#include "wl_common.h"
#include "simple_timer.h"
#include "timer.h"

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : nandwl_hw_get_microsecond
* Returned Value   : hw microsecond in uint_64 
* Comments         : 
*   This function retrieves the number of microseconds since
*   the processor started.  (without any time offset information)
*
*END*----------------------------------------------------------------------*/
uint_64 wl_hw_get_microsecond()
{ /* Body */
    MQX_TICK_STRUCT    tick;
    TIME_STRUCT_PTR time_ptr;
    
    _time_get_elapsed_ticks(&tick);
    return PSP_TICKS_TO_MICROSECONDS(&tick, &time_ptr );
} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : wl_msecs_to_ticks
* Returned Value   : ticks in uint_64 
* Comments         : 
*   Convert from given msecs to OS ticks 
*
*END*----------------------------------------------------------------------*/
uint_64 wl_msecs_to_ticks(uint_32 msecs)
{ /* Body */
    PSP_TICK_STRUCT tick;
    _psp_usecs_to_ticks(msecs, &tick);
    return tick.TICKS[0];
} /* Endbody */

/* EOF */
