#if !defined(__deferred_task_h__)
#define __deferred_task_h__
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
 * $FileName: deferred_task.h$
 * $Version : 3.8.0.1$
 * $Date    : Aug-9-2012$
 *
 * Comments:
 *
 *   This file contains definition of the nand::DeferredTask class.
 *
 *END************************************************************************/

#include "wl_common.h"
#include "double_list.h"

/* 
** Class definations 
*/

namespace nand
{

    class DeferredTaskQueue;

    /*
    ** \brief Deferred task abstract base class.
    **
    ** Subclasses must implement the task() and getTaskTypeID() methods. They can optionally
    ** override the getShouldExamine(), examineOne(), and examine() methods to modify how the task
    ** looks at a queue prior to being inserted, to determine whether it should be inserted at all
    ** or perhaps perform some other operation.
    **
    ** Task priorities are inverted, in the sense that the highest priority is 0 and they go
    ** down in priority as the priority value increases. The priority is passed to the constructor
    ** and must not change over the lifetime of the task object.
    **
    ** To actually perform the task, call the run method(). It will internally invoke the pure
    ** abstract task() method that subclasses must provide. If you need more complex behaviour, then
    ** you may override run().
    **
    ** A completion callback is supported. When set, the default implementation of the run() method
    ** will call the completion callback after task() returns. If you override run(), then be sure
    ** to invoke the callback before returning.
    */
    class DeferredTask : public DoubleList::Node
    {
    public:

        /* Type for a completion callback function. */
        typedef void (*CompletionCallback_t)(DeferredTask * completedTask, void * data);
        
        /* Constructor. */
        DeferredTask(int priority);
        
        /* Destructor. */
        virtual ~DeferredTask();
        
        /* Properties */
        /* Return a unique ID for this task type. */
        virtual uint32_t getTaskTypeID() const = 0;
        
        virtual bool getShouldExamine() const;
        
        /* Return the task's priority. */
        int getPriority() const { return m_priority; }
        
        /* Operations */
        /* Execute the task. */
        virtual void run();
        
        virtual bool examine(DeferredTaskQueue & queue);
        
        virtual bool examineOne(DeferredTask * task);
        
        /* Completion callbacks */
        /* Set the completion callback. */
        void setCompletion(CompletionCallback_t callback, void * data);

    protected:
        /* The priority level for this task. */
        int m_priority;
        
        /* An optional completion callback function. */
        CompletionCallback_t m_callback;
        
        /* Arbitrary data passed to the callback. */
        void * m_callbackData;
        
        /* The task entry point provided by a concrete subclass. */
        virtual void task() = 0;
        
    };

} /* namespace nand */

#endif /* __deferred_task_h__ */

/* EOF */
