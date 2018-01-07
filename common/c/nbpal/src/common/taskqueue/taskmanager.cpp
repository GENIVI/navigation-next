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

    @file     taskmanager.cpp
    @date     08/23/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task manager.

    Common implementation for PAL task manager.
*/
/*
    (C) Copyright 2011 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "taskmanager.h"
#include "task.h"
#include "palimpl.h"

namespace nimpal
{
namespace taskqueue
{
TaskManager::TaskManager(PAL_Instance* pal)
        : m_pal(pal),
          m_lock(NULL),
          m_needToDestroy(false)
{
    PAL_LockCreate(pal, &m_lock);
}

TaskManager::~TaskManager(void)
{
    PAL_LockDestroy(m_lock);
}

PAL_Error TaskManager::AddTask(TaskQueue::TaskQueueId queueId, PAL_TaskQueueCallback callback,
                               void* userData, TaskId* taskId, TaskPriority priority)
{
    Thread* thread = NULL;
    TaskQueue* queue = NULL;

    Lock lock(m_lock);

    FindThreadAndQueue(queueId, &thread, &queue);
    if (thread == NULL || queue == NULL)
    {
        return PAL_ErrQueueNotFound;
    }

    Task* task = new Task(m_pal, callback, userData, priority);
    if (task == NULL)
    {
        return PAL_ErrNoMem;
    }

    if (taskId != NULL)
    {
        *taskId = task->Id();
    }

    PAL_Error err = queue->Queue(task);
    if (err == PAL_Ok)
    {
        // Notify thread about new task
        thread->NewTaskNotify();
    }
    else
    {
        delete task;
    }

    return err;
}

PAL_Error TaskManager::RemoveTask(TaskQueue::TaskQueueId queueId, TaskId taskId, bool cancelIfRunning)
{
    Thread* thread = NULL;
    TaskQueue* queue = NULL;

    Lock lock(m_lock);

    FindThreadAndQueue(queueId, &thread, &queue);
    if (thread == NULL && queue == NULL)
    {
        return PAL_ErrQueueNotFound;
    }

    return queue->Dequeue(taskId, cancelIfRunning);
}

PAL_Error TaskManager::RemoveAllTasks(TaskQueue::TaskQueueId queueId)
{
    Thread* thread = NULL;
    TaskQueue* queue = NULL;

    Lock lock(m_lock);

    FindThreadAndQueue(queueId, &thread, &queue);
    if (thread == NULL && queue == NULL)
    {
        return PAL_ErrQueueNotFound;
    }

    return queue->Clear();
}

PAL_Error TaskManager::CreateWorkerQueue(const char* /*name*/, TaskQueue::TaskQueueId* /*queueId*/)
{
    return PAL_ErrUnsupported;
}

PAL_Error TaskManager::DestroyWorkerQueue(TaskQueue::TaskQueueId /*queueId*/)
{
    return PAL_ErrUnsupported;
}

TaskQueue::TaskQueueId TaskManager::GetWorkerQueueByName(const char* /*name*/) const
{
    return TaskQueue::INVALID_QUEUE_ID;
}
nb_threadId TaskManager::GetWorkerThreadIdByName(const char* /*name*/) const
{
    return 0;
}

}
}

