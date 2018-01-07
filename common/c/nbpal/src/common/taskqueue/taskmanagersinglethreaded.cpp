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

    @file     taskmanagersinglethreaded.cpp
    @date     11/21/2013
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task manager in single-threaded configuration.

    Implementation for PAL task manager in single-threaded configuration.
*/
/*
    (C) Copyright 2013 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "taskmanagersinglethreaded.h"
#include "pal.h"
#include "palstdlib.h"
#include "palabstractfactory.h"

namespace nimpal
{
namespace taskqueue
{
TaskManagerSingleThreaded::TaskManagerSingleThreaded(PAL_Instance* pal)
    : TaskManager(pal),
      m_uiTaskQueue(NULL),
      m_uiThread(NULL)
{
}

TaskManagerSingleThreaded::~TaskManagerSingleThreaded(void)
{
    // Destroy the thread first because it can call the queue.
    if (m_uiThread)
    {
        delete m_uiThread;
        m_uiThread = NULL;
    }
    if (m_uiTaskQueue)
    {
        delete m_uiTaskQueue;
        m_uiTaskQueue = NULL;
    }
}

PAL_Error TaskManagerSingleThreaded::CreateTaskManager(PAL_Instance* pal, TaskManager** taskManager)
{
    if (pal == NULL || taskManager == NULL)
    {
        return PAL_ErrBadParam;
    }

    TaskManagerSingleThreaded* manager = new TaskManagerSingleThreaded(pal);
    if (manager == NULL)
    {
        return PAL_ErrNoMem;
    }

    PAL_Error err = manager->Initialize();
    if (err == PAL_Ok)
    {
        *taskManager = manager;
    }
    else
    {
        delete manager;
        *taskManager = NULL;
    }

    return err;
}

PAL_Error TaskManagerSingleThreaded::Initialize(void)
{
    PAL_Error err = PAL_ErrNoMem;

    m_uiTaskQueue = new TaskQueue(m_pal, TaskQueue::UI_QUEUE_ID);
    if (m_uiTaskQueue != NULL)
    {
        m_uiThread = PalAbstractFactory::CreateUiThread(m_pal, m_uiTaskQueue, this);
        if (m_uiThread != NULL)
        {
            err = m_uiThread->StartThread();
        }
    }

    if (err != PAL_Ok)
    {
        return err;
    }

    return err;
}

PAL_Error TaskManagerSingleThreaded::CreateWorkerQueue(const char* /*name*/, TaskQueue::TaskQueueId* /*queueId*/)
{
    PAL_Error err = PAL_ErrUnsupported;
    return err;
}

PAL_Error TaskManagerSingleThreaded::DestroyWorkerQueue(TaskQueue::TaskQueueId /*queueId*/)
{
    PAL_Error err = PAL_ErrUnsupported;
    return err;
}

TaskQueue::TaskQueueId TaskManagerSingleThreaded::GetWorkerQueueByName(const char* /*name*/) const
{
    return TaskQueue::INVALID_QUEUE_ID;
}

nb_threadId TaskManagerSingleThreaded::GetWorkerThreadIdByName(const char* /*name*/) const
{
    return 0;
}

// This utility function isn't thread-safe. We assume this function is always called inside the sync block.
// No need to use PAL_Lock.
void TaskManagerSingleThreaded::FindThreadAndQueue(TaskQueue::TaskQueueId /*queueId*/, Thread** thread, TaskQueue** queue) const
{
    *thread = m_uiThread;
    *queue  = m_uiTaskQueue;
}

nb_threadId TaskManagerSingleThreaded::GetEventThreadId(void) const
{
    // Since we have uiThread only, we return its thread ID as event thread ID
    return (m_uiThread != NULL) ? m_uiThread->ThreadId() : 0;
}

nb_threadId TaskManagerSingleThreaded::GetRenderThreadId(void) const
{
    // Since we have uiThread only, we return its thread ID as event thread ID
    return (m_uiThread != NULL) ? m_uiThread->ThreadId() : 0;
}

nb_threadId TaskManagerSingleThreaded::GetCurrentThreadId(void) const
{
    return (m_uiThread != NULL) ? m_uiThread->ThreadId() : 0;
}

void TaskManagerSingleThreaded::ThreadStateChanged(Thread* /*thread*/, Thread::State state)
{
    if (state != Thread::Executing && state != Thread::Invalid && m_needToDestroy && Destroy())
    {
        PAL_ScheduleDestroy(m_pal);
    }
}

bool TaskManagerSingleThreaded::Destroy(void)
{
    bool readyToDestroy = true;
    m_needToDestroy = true;

    readyToDestroy = readyToDestroy && (m_uiThread ? (m_uiThread->GetState() != Thread::Executing) : true);
    readyToDestroy = readyToDestroy && (m_uiTaskQueue ? m_uiTaskQueue->IsEmpty() : true);

    return readyToDestroy;
}
}
}
