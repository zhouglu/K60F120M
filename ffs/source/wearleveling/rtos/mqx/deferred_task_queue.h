#if !defined(__deferred_task_queue_h__)
#define __deferred_task_queue_h__
/**HEADER********************************************************************
** Copyright (c) 2013 Freescale Semiconductor, Inc. All rights reserved.
** 
** Freescale Semiconductor, Inc.
** Proprietary & Confidential
** 
** This source code and the algorithms implemented therein constitute
** confidential information and may comprise trade secrets of Freescale Semiconductor, Inc.
** or its associates, and any use thereof is subject to the terms and
** conditions of the Confidential Disclosure Agreement pursual to which this
** source code was originally received.
******************************************************************************
*
* $FileName: deferred_task_queue.h$
* $Version : 3.8.0.1$
* $Date    : Aug-9-2012$
*
* Comments: Definition of the nand::DeferredTask class.
*END************************************************************************/

#include "wl_common.h"
#include "double_list.h"
#include "simple_mutex.h"
#include "deferred_task.h"
/*
** Definitions
*/

namespace nand
{


/*
**  Priority queue of deferred task objects.
**
** This class is not only a priority queue but the manager for the thread that executes the
** tasks inserted into the queue.
**
** Users of a queue must ensure that the drain() method is called prior to destructing the queue
** if they want all tasks to be executed. Otherwise, the destructor will simply delete any
** tasks remaining on the queue.
*/
class DeferredTaskQueue
{
public:

    /* Constants for the task execution thread. */
    enum _task_thread_constants
    {
        /* kTaskThreadStackSize = 2048, */
        /* kTaskThreadPriority = 12, */
        kTaskThreadTimeoutMSecs = 500
    };

    /* Queue types */
    typedef uint16_t test12;
    typedef DoubleListT<DeferredTask> queue_t;    
    typedef DoubleListT<DeferredTask>::Iterator iterator_t;

    /* Constructor. */
    DeferredTaskQueue();
    
    /* Destructor. */
    ~DeferredTaskQueue();
    
    /* Initializer. */
    RtStatus_t init();
    
    /* Wait for all current tasks to complete. */
    RtStatus_t drain();
    
    /* Add a new task to the queue. */
    void post(DeferredTask * task);
    
    /* Returns whether the queue is empty. */
    bool isEmpty() const { return m_entries.isEmpty(); }

    iterator_t getBegin() {  return m_entries.getBegin(); }
    iterator_t getEnd()  { return m_entries.getEnd(); }
    
    /* Returns the task that is currently being executed. */
    DeferredTask * getCurrentTask() { return m_currentTask; }

protected:
    
    /* Mutex protecting the queue. */
    mutex_struct m_mutex;   
    
    /* List of queue entries. */
    queue_t m_entries;  
    
    /* Thread used to execute tasks. */
    _task_id m_thread;   
    
    /* Semaphore to signal availability of tasks to the thread. */
    LWSEM_STRUCT m_taskSem; 
    
    /* Task being executed. */
    DeferredTask * volatile m_currentTask;   
    TASK_TEMPLATE_STRUCT m_wl_task_template;
    
    /* Static entry point for the task thread. */
    static void taskThreadStub(uint32_t arg);
    
    /* The main entry point for the task thread. */
    void taskThread();
    
    /* Function to dispose of the task thread. */
    static void disposeTaskThread(uint32_t param);

};


} /*  namespace nand */

#endif /*  __deferred_task_queue_h__ */
/*  EOF */
