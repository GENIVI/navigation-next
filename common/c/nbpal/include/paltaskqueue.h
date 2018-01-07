/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------
    @file paltaskqueue.h

    Interface for a simple platform independent task queue. The task queue
    provides a mechanism to call user callback in the context of particular thread.
    It can be UI, Event or Worker thread. Adding a task can be called from any
    thread.

    All UI function calls should be done in the context of UI thread. Such tasks should
    be added to UI task queue.

    All CCC API calls should be done in the context of CCC/PAL thread. Such tasks should
    be added to Event task queue.

    The internal CCC calls which require a lot of calculations or can be blocked should be
    done in the context of worker thread. Such tasks should be added to Worker task queue.

    The task queues implementation is highly platform dependent. Some platforms
    (e.g. iPhone) have already implementations of a task queue and should
    therefore use the platform specific implementation, while other platforms
    (e.g. Windows Mobile) need to implement the task queue using
    semaphores/critical sections in order to ensure proper synchronization.
*/

/*
    (C) Copyright 2010 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PAL_TASKQUEUE_H
#define	PAL_TASKQUEUE_H

#include "palerror.h"
#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* PAL_QueueHandle;
typedef uint32 TaskId;
typedef uint32 TaskPriority;

#define DEFAULT_TASK_PRIORITY  100
#define MAX_USER_TASK_PRIORITY 50
#define MIN_POSSIBLE_TASK_PRIORITY 0xffff

#define MAX_QUEUE_NAME_LENGTH 64

/*! Callback to execute a task.

    This callback is always called in the context of the thread related to the task queue
    which it added to.

    @see PAL_EventTaskQueueAdd
    @see PAL_UiTaskQueueAdd
    @see PAL_WorkerTaskQueueAdd
*/

typedef void (*PAL_TaskQueueCallback)(
       PAL_Instance* pal,                /*!< Caller-supplied reference to PAL structure */
       void* userData                    /*!< Caller-supplied data reference */
       );

/*! Add a task to the Event task queue.

    This function is thread safe and can be called from any thread. The added task will be
    called in the context of the CCC/PAL thread.

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_EventTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                      PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                      void* userData,                        /*!< Caller-supplied data reference */
                      TaskId* taskID                         /*!< On return a unique ID for the added task. */
                      );

/*! Add a task to the Event task queue.

    This function is thread safe and can be called from any thread. The added task will be
    called in the context of the CCC/PAL thread.

    Task with highest priority executes first. Smaller value indicates higher priority.
    If task's priority doesn't important - DEFAULT_TASK_PRIORITY can be used.
    Priority cannot be higher than MAX_USER_TASK_PRIORITY, the highest priorities are reserved for internal CCC tasks.

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_EventTaskQueueAddWithPriority(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                                  PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                                  void* userData,                        /*!< Caller-supplied data reference */
                                  TaskId* taskID,                        /*!< On return a unique ID for the added task. */
                                  TaskPriority priority                  /*!< Task priority */
                                );

/*! Remove a previously added task from Event task queue.

    Only queued tasks can be removed. Currently executing tasks can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a task).

    @see PAL_EventTaskQueueAdd
    @return 'PAL_ErrNoData' if task is already executed (or currently executing) and is no longer in the queue.
            'PAL_Ok' if the task got removed from the queue.
*/
PAL_DEC PAL_Error
PAL_EventTaskQueueRemove(PAL_Instance* pal,      /*!< Caller-supplied reference to PAL structure */
                         TaskId taskID,          /*!< Task ID returned by PAL_EventTaskQueueAdd(). */
                         nb_boolean cancelIfRunning /*!< If TRUE and task is running - try to cancel it. */
                         );

/*! Remove all tasks from Event task queue.

    Only queued tasks can be removed. Currently executing tasks can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a task).

    @see PAL_EventTaskQueueAdd
*/
PAL_DEC void
PAL_EventTaskQueueRemoveAll(PAL_Instance* pal  /*!< Caller-supplied reference to PAL structure */
                            );


/*! Create worker queue.

    The new worker thread will be created. It'll be valid until PAL_WorkerTaskQueueDestroy() is called.
    If worker queue with this name exists, the handle to the existed queue will be returned.

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueCreate(PAL_Instance* pal,              /*!< Caller-supplied reference to PAL structure */
                          const char* name,               /*!< Queue name(for named task queue). Can be NULL. */
                          PAL_QueueHandle* queueHandle    /*!< PAL task queue handle */
                          );

/*! Destroy worker queue.

    The associated worker thread will be destroyed. If worker task queue has pending tasks or task executed
    the PAL_ErrQueueNotEmpty will be returned. User can call PAL_WorkerTaskQueueRemoveAll() to remove
    pending tasks. In case of task executed, user should wait until task complete and then destroy the queue.

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueDestroy(PAL_Instance* pal,              /*!< Caller-supplied reference to PAL structure */
                           PAL_QueueHandle queueHandle     /*!< PAL task queue handle */
                           );

/*! Returns queue handle for given named queue.

    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueGetHandler(PAL_Instance* pal,             /*!< Caller-supplied reference to PAL structure */
                              const char* name,              /*!< Queue name */
                              PAL_QueueHandle* queueHandle   /*!< PAL task queue handle */
                              );

/*! Add a task to the worker queue.

    This function is thread safe and can be called from any thread. The added worker will be
    called in the context of the CCC/PAL thread (see explanation above).

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                       PAL_QueueHandle queueHandle,           /*!< PAL task queue handle */
                       PAL_TaskQueueCallback callback,        /*!< Callback to run given worker. */
                       void* userData,                        /*!< Caller-supplied data reference */
                       TaskId* taskID                         /*!< On return a unique ID for the added worker. */
                       );

/*! Add a task to the worker queue.

    This function is thread safe and can be called from any thread. The added worker will be
    called in the context of the CCC/PAL thread (see explanation above).

    Task with highest priority executes first. Smaller value indicates higher priority.
    If task's priority doesn't important - DEFAULT_TASK_PRIORITY can be used.
    Priority cannot be higher than MAX_USER_TASK_PRIORITY, the highest priorities are reserved for internal CCC tasks.

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueAddWithPriority(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                                   PAL_QueueHandle queueHandle,           /*!< PAL task queue handle */
                                   PAL_TaskQueueCallback callback,        /*!< Callback to run given worker. */
                                   void* userData,                        /*!< Caller-supplied data reference */
                                   TaskId* taskID,                        /*!< On return a unique ID for the added worker. */
                                   TaskPriority priority                  /*!< Task priority */
                       );

/*! Remove a previously added worker task.

    Only queued workers can be removed. Currently executing workers can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a worker)

    @see PAL_WorkerTaskQueueAdd
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_WorkerTaskQueueRemove(PAL_Instance* pal,            /*!< Caller-supplied reference to PAL structure */
                          PAL_QueueHandle queueHandle,  /*!< PAL task queue handle */
                          TaskId taskID,                /*!< Task ID returned by PAL_WorkerTaskQueueAdd(). */
                          nb_boolean cancelIfRunning    /*!< If TRUE and task is running - try to cancel it. */
                          );

/*! Remove all added worker tasks.

    Only queued workers can be removed. Currently executing workers can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a worker)

    @see PAL_WorkerTaskQueueAdd
*/
PAL_DEC void
PAL_WorkerTaskQueueRemoveAll(PAL_Instance* pal,               /*!< Caller-supplied reference to PAL structure */
                             PAL_QueueHandle queueHandle      /*!< PAL task queue handle */
                             );

/*! Check if the current thread is the running thread of work queue

    @return TRUE if the current thread is the running thread of work queue, FALSE otherwise.
*/
PAL_DEC nb_boolean
PAL_WorkerTaskQueueIsRunningThread(PAL_Instance* pal,       /*!< Caller-supplied reference to PAL structure */
                                   const char* name         /*!< Queue name */
                                   );

PAL_DEC nb_threadId
PAL_WorkerTaskQueueGetRunningThreadId(PAL_QueueHandle queueHandle      /*!< PAL task queue handle */
                                   );





/*! Add a task to the Event task queue.

    This function is thread safe and can be called from any thread. The added task will be
    called in the context of the CCC/PAL thread.

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_RenderTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                      PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                      void* userData,                        /*!< Caller-supplied data reference */
                      TaskId* taskID                         /*!< On return a unique ID for the added task. */
                      );

/*! Add a task to the Event task queue.

    This function is thread safe and can be called from any thread. The added task will be
    called in the context of the CCC/PAL thread.

    Task with highest priority executes first. Smaller value indicates higher priority.
    If task's priority doesn't important - DEFAULT_TASK_PRIORITY can be used.
    Priority cannot be higher than MAX_USER_TASK_PRIORITY, the highest priorities are reserved for internal CCC tasks.

    @see PAL_TaskQueueCallback
    @return PAL error code
*/
PAL_DEC PAL_Error
PAL_RenderTaskQueueAddWithPriority(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                                  PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                                  void* userData,                        /*!< Caller-supplied data reference */
                                  TaskId* taskID,                        /*!< On return a unique ID for the added task. */
                                  TaskPriority priority                  /*!< Task priority */
                                );

/*! Remove a previously added task from Event task queue.

    Only queued tasks can be removed. Currently executing tasks can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a task).

    @see PAL_EventTaskQueueAdd
    @return 'PAL_ErrNoData' if task is already executed (or currently executing) and is no longer in the queue.
            'PAL_Ok' if the task got removed from the queue.
*/
PAL_DEC PAL_Error
PAL_RenderTaskQueueRemove(PAL_Instance* pal,      /*!< Caller-supplied reference to PAL structure */
                         TaskId taskID,          /*!< Task ID returned by PAL_EventTaskQueueAdd(). */
                         nb_boolean cancelIfRunning /*!< If TRUE and task is running - try to cancel it. */
                         );

/*! Remove all tasks from Event task queue.

    Only queued tasks can be removed. Currently executing tasks can not be aborted and removed from the queue.
    (Immediately returning in the callback will effectively cancel/abort a task).

    @see PAL_EventTaskQueueAdd
*/
PAL_DEC void
PAL_RenderTaskQueueRemoveAll(PAL_Instance* pal  /*!< Caller-supplied reference to PAL structure */
                            );
/*! Check if the current thread is the running thread of work queue

    @return TRUE if the current thread is the running thread of work queue, FALSE otherwise.
*/
PAL_DEC nb_boolean
PAL_RenderTaskQueueIsRunningThread(PAL_Instance* pal       /*!< Caller-supplied reference to PAL structure */
                                   );

PAL_DEC nb_threadId
PAL_RenderTaskQueueGetRunningThreadId(PAL_Instance* pal);




#ifdef __cplusplus
}
#endif

#endif

/*! @} */
