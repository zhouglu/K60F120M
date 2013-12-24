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
 * $FileName: simple_mutex.cpp$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments: Defines a mutex helper class.
 *END************************************************************************/

#include "wl_common.h"
#include "simple_mutex.h"

MUTEX_ATTR_STRUCT SimpleMutex::m_mutexattr =
{   0};

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::CreateMutexAttr
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Initialise the Mutex Attributes
 *
 *END*--------------------------------------------------------------------*/
bool SimpleMutex::CreateMutexAttr()
{ /* Body */
    static bool bIsInit = FALSE;

    /* Init already */
    if (bIsInit)
    return TRUE;

    if (_mutatr_init(&SimpleMutex::m_mutexattr) != MQX_OK)
    {
        WL_LOG(WL_MODULE_GENERAL,WL_LOG_ERROR, "Initializing mutex attributes failed.\n");
        return FALSE;
    }/* Endif*/
    bIsInit = TRUE;
    return TRUE;
}/* Endbody */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::InitMutex
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Initialise the Mutex 
 *
 *END*--------------------------------------------------------------------*/
bool SimpleMutex::InitMutex
(
    /* [IN] pointer to Mutex  */
    WL_MUTEX _PTR_ pIncMutex
)
{/* Body */
    SimpleMutex::CreateMutexAttr();

    if (_mutex_init(pIncMutex, &SimpleMutex::m_mutexattr) != MQX_OK)
    {
        WL_LOG(WL_MODULE_GENERAL, WL_LOG_ERROR,"Initializing mutex failed.\n");
        return FALSE;
    }/* Endif*/
    return TRUE;
}/* Endbody */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::DestroyMutex
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Destroy the Mutex 
 *
 *END*--------------------------------------------------------------------*/
bool SimpleMutex::DestroyMutex
(
    /* [IN] pointer to Mutex  */
    WL_MUTEX _PTR_ pIncMutex
)
{/* Body */

    if (_mutex_destroy(pIncMutex) != MQX_OK)
    {
        WL_LOG(WL_MODULE_GENERAL, WL_LOG_ERROR,"Destroying mutex failed.\n");
        return FALSE;
    }/* Endif*/
    return TRUE;
}/* Endbody */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::InitMutex
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Constructor. Gets mutex. Takes a mutex pointer 
 *
 *END*--------------------------------------------------------------------*/
SimpleMutex::SimpleMutex
(
    /* [IN] pointer to Mutex  */
    WL_MUTEX _PTR_ pIncMutex
)
:
m_mutex(pIncMutex)
{/* Body */
    if (_mutex_lock(m_mutex) != MQX_OK)
    {
        WL_LOG(WL_MODULE_GENERAL, WL_LOG_ERROR, "Mutex lock failed.\n");
        assert(0);
    }/* Endif*/

}/* Endbody */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::InitMutex
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Constructor. Gets mutex. Takes a mutex reference 
 *
 *END*--------------------------------------------------------------------*/
SimpleMutex::SimpleMutex
(
    /* [IN] pointer to Mutex  */
    WL_MUTEX & incMutex
)
:
m_mutex(&incMutex)
{/* Body */
    if (_mutex_lock(m_mutex) != MQX_OK)
    {
        WL_LOG(WL_MODULE_GENERAL, WL_LOG_ERROR, "Mutex lock failed.\n");
        assert(0);
    }/* Endif*/
}/* Endbody */

/*FUNCTION*----------------------------------------------------------------
 *
 * Function Name    : SimpleMutex::InitMutex
 * Returned Value   : TRUE or FALSE
 * Comments         :
 *   Destructor. Puts mutex 
 *
 *END*--------------------------------------------------------------------*/
SimpleMutex::~SimpleMutex()
{ /* Body */
    _mutex_unlock(m_mutex);
}/* Endbody */

/* EOF */
