#ifndef _simple_mutex_h_
#define _simple_mutex_h_
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
* $FileName: simple_mutex.h$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: Defines a mutex helper class.
*END************************************************************************/

#include "wl_common.h"
#include "mutex.h"

typedef MUTEX_STRUCT WL_MUTEX ;
/*
** Types
*/

/*
** Mutex helper class.
**
** Automatically gets the mutex in the constructor and puts the mutex in the destructor.
** To use, allocate an instance on the stack so that it will put the mutex when it
** falls out of scope.
*/
class SimpleMutex {
public:

    static bool InitMutex(WL_MUTEX _PTR_ pIncMutex);
    static bool DestroyMutex(WL_MUTEX _PTR_ pIncMutex);
    /* Constructor. Gets mutex. Takes a mutex pointer. */
    SimpleMutex(WL_MUTEX _PTR_ pIncMutex);

    /* Constructor. Gets mutex. Takes a mutex reference. */
    SimpleMutex(WL_MUTEX & incMutex);
    /* Destructor. Puts mutex. */
    ~SimpleMutex();

protected:
    static bool CreateMutexAttr() ;

protected:
    /* Pointer to the mutex object. */
    WL_MUTEX _PTR_ m_mutex;
    static MUTEX_ATTR_STRUCT m_mutexattr;
};

#endif /* _simple_mutex_h_ */
/*  EOF */
