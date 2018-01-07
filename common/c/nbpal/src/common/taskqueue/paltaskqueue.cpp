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

    @file     paltaskqueue.cpp
    @date     2/10/2012
    @defgroup paltaskqueue in nbpal

    This file contains implementation of PAL Task Queue operations for QT QNX.
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palimpl.h"
#include "palstdlib.h"
#include "paltaskqueue.h"
#include "taskmanager.h"
#include "taskqueue.h"

typedef struct _PalQueueHandle
{
    TaskQueue::TaskQueueId queueId;
    nb_threadId runningThreadId;
} PalQueueHandle;

PAL_DEF PAL_Error
PAL_EventTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                      PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                      void* userData,                        /*!< Caller-supplied data reference */
                      TaskId* taskID                         /*!< On return a unique ID for the added task. */
                     )
{
    if (pal == NULL || callback == NULL || taskID == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(TaskQueue::EVENT_QUEUE_ID, callback, userData, taskID);
}

PAL_DEF PAL_Error
PAL_EventTaskQueueAddWithPriority(PAL_Instance* pal, PAL_TaskQueueCallback callback, void* userData, TaskId* taskID, TaskPriority priority)
{
    if (pal == NULL || callback == NULL || taskID == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(TaskQueue::EVENT_QUEUE_ID, callback, userData, taskID, priority);
}

PAL_DEF PAL_Error
PAL_EventTaskQueueRemove(PAL_Instance* pal,      /*!< Caller-supplied reference to PAL structure */
                         TaskId taskID,          /*!< Task ID returned by PAL_EventTaskQueueAdd(). */
                         nb_boolean cancelIfRunning /*!< If TRUE and task is running - try to cancel it. */
                        )
{
    if (pal == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->RemoveTask(TaskQueue::EVENT_QUEUE_ID, taskID, cancelIfRunning == TRUE);
}

PAL_DEF void
PAL_EventTaskQueueRemoveAll(PAL_Instance* pal  /*!< Caller-supplied reference to PAL structure */
                           )
{
    if (pal != NULL)
    {
        pal->taskManager->RemoveAllTasks(TaskQueue::EVENT_QUEUE_ID);
    }
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueCreate(PAL_Instance* pal,              /*!< Caller-supplied reference to PAL structure */
                          const char* name,               /*!< Queue name(for named task queue). Can be NULL. */
                          PAL_QueueHandle* queueHandle    /*!< PAL task queue handle */
                         )
{
    if (pal == NULL || queueHandle == NULL)
    {
        return PAL_ErrBadParam;
    }

    PalQueueHandle* handle = static_cast<PalQueueHandle*>( nsl_malloc(sizeof(PalQueueHandle)) );
    if (handle == NULL)
    {
        return PAL_ErrNoMem;
    }

    PAL_Error err = pal->taskManager->CreateWorkerQueue(name, &(handle->queueId));
    if (err == PAL_Ok)
    {
        handle->runningThreadId = pal->taskManager->GetWorkerThreadIdByName(name);
        *queueHandle = handle;
    }
    else
    {
        nsl_free(handle);
    }
    return err;
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueDestroy(PAL_Instance* pal,              /*!< Caller-supplied reference to PAL structure */
                           PAL_QueueHandle queueHandle     /*!< PAL task queue handle */
                          )
{
    if (pal == NULL || queueHandle == NULL)
    {
        return PAL_ErrBadParam;
    }

    TaskQueue::TaskQueueId queueId = static_cast<PalQueueHandle*>(queueHandle)->queueId;
    nsl_free(queueHandle);
    return pal->taskManager->DestroyWorkerQueue(queueId);
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueGetHandler(PAL_Instance* pal,             /*!< Caller-supplied reference to PAL structure */
                              const char* name,              /*!< Queue name */
                              PAL_QueueHandle* queueHandle   /*!< PAL task queue handle */
                             )
{
    if (pal == NULL || name == NULL || queueHandle == NULL)
    {
        return PAL_ErrBadParam;
    }

    PalQueueHandle* handle = static_cast<PalQueueHandle*>( nsl_malloc(sizeof(PalQueueHandle)) );
    if (handle == NULL)
    {
        return PAL_ErrNoMem;
    }

    handle->queueId = pal->taskManager->GetWorkerQueueByName(name);
    if (handle->queueId != TaskQueue::INVALID_QUEUE_ID)
    {
        *queueHandle = handle;
    }
    else
    {
        nsl_free(handle);
    }
    return (handle->queueId != TaskQueue::INVALID_QUEUE_ID) ? PAL_Ok : PAL_ErrQueueNotFound;
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                       PAL_QueueHandle queueHandle,           /*!< PAL task queue handle */
                       PAL_TaskQueueCallback callback,        /*!< Callback to run given worker. */
                       void* userData,                        /*!< Caller-supplied data reference */
                       TaskId* taskID                         /*!< On return a unique ID for the added worker. */
                      )
{
    if (pal == NULL || queueHandle == NULL || callback == NULL || taskID == 0)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(static_cast<PalQueueHandle*>(queueHandle)->queueId, callback, userData, taskID);
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueAddWithPriority(PAL_Instance* pal, PAL_QueueHandle queueHandle, PAL_TaskQueueCallback callback,
                       void* userData, TaskId* taskID, TaskPriority priority)
{
    if (pal == NULL || queueHandle == NULL || callback == NULL || taskID == 0)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(static_cast<PalQueueHandle*>(queueHandle)->queueId, callback, userData, taskID, priority);
}

PAL_DEF PAL_Error
PAL_WorkerTaskQueueRemove(PAL_Instance* pal,            /*!< Caller-supplied reference to PAL structure */
                          PAL_QueueHandle queueHandle,  /*!< PAL task queue handle */
                          TaskId taskID,                /*!< Task ID returned by PAL_WorkerTaskQueueAdd(). */
                          nb_boolean cancelIfRunning    /*!< If TRUE and task is running - try to cancel it. */
                         )
{
    if (pal == NULL || queueHandle == NULL || taskID == 0)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->RemoveTask(static_cast<PalQueueHandle*>(queueHandle)->queueId, taskID, cancelIfRunning == TRUE);
}

PAL_DEF void
PAL_WorkerTaskQueueRemoveAll(PAL_Instance* pal,               /*!< Caller-supplied reference to PAL structure */
                             PAL_QueueHandle queueHandle      /*!< PAL task queue handle */
                            )
{
    if (pal != NULL && queueHandle != NULL)
    {
        pal->taskManager->RemoveAllTasks(static_cast<PalQueueHandle*>(queueHandle)->queueId);
    }
}

PAL_DEF PAL_Error
PAL_EventTaskQueueAddLock(PAL_Instance* /*pal*/,
                          PAL_TaskQueueCallback /*callback*/,
                          void* /*userData*/,
                          TaskId* /*taskID*/,
                          nb_boolean /*lock*/)
{
    return PAL_ErrUnsupported;
}

PAL_DEF nb_boolean
PAL_WorkerTaskQueueIsRunningThread(PAL_Instance* pal,
                                   const char* name)
{
    return (nb_boolean) (pal &&
                         name &&
                         pal->taskManager &&
                         (pal->taskManager->GetWorkerThreadIdByName(name) == pal->taskManager->GetCurrentThreadId()));

}

PAL_DEF nb_threadId
PAL_WorkerTaskQueueGetRunningThreadId(PAL_QueueHandle queueHandle      /*!< PAL task queue handle */
                                   )
{
    if (queueHandle == NULL)
    {
        return NULL;
    }
    return static_cast<PalQueueHandle*>(queueHandle)->runningThreadId;
}






PAL_DEF PAL_Error
PAL_RenderTaskQueueAdd(PAL_Instance* pal,                     /*!< Caller-supplied reference to PAL structure */
                      PAL_TaskQueueCallback callback,        /*!< Callback to run given task. */
                      void* userData,                        /*!< Caller-supplied data reference */
                      TaskId* taskID                         /*!< On return a unique ID for the added task. */
                     )
{
    if (pal == NULL || callback == NULL || taskID == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(TaskQueue::RENDER_QUEUE_ID, callback, userData, taskID);
}

PAL_DEF PAL_Error
PAL_RenderTaskQueueAddWithPriority(PAL_Instance* pal, PAL_TaskQueueCallback callback, void* userData, TaskId* taskID, TaskPriority priority)
{
    if (pal == NULL || callback == NULL || taskID == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->AddTask(TaskQueue::RENDER_QUEUE_ID, callback, userData, taskID, priority);
}

PAL_DEF PAL_Error
PAL_RenderTaskQueueRemove(PAL_Instance* pal,      /*!< Caller-supplied reference to PAL structure */
                         TaskId taskID,          /*!< Task ID returned by PAL_RenderTaskQueueAdd(). */
                         nb_boolean cancelIfRunning /*!< If TRUE and task is running - try to cancel it. */
                        )
{
    if (pal == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pal->taskManager->RemoveTask(TaskQueue::RENDER_QUEUE_ID, taskID, cancelIfRunning == TRUE);
}

PAL_DEF void
PAL_RenderTaskQueueRemoveAll(PAL_Instance* pal  /*!< Caller-supplied reference to PAL structure */
                           )
{
    if (pal != NULL)
    {
        pal->taskManager->RemoveAllTasks(TaskQueue::RENDER_QUEUE_ID);
    }
}


PAL_DEF nb_boolean
PAL_RenderTaskQueueIsRunningThread(PAL_Instance* pal)
{
    return (nb_boolean) (pal &&
                         pal->taskManager &&
                         (pal->taskManager->GetRenderThreadId() == pal->taskManager->GetCurrentThreadId()));
}

PAL_DEF nb_threadId
PAL_RenderTaskQueueGetRunningThreadId(PAL_Instance* pal)
{
    return pal->taskManager->GetRenderThreadId();
}
/*! @} */
