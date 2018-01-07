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
 @file nbtaskqueue.cpp
 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

#include "nbtaskqueue.h"
#include "paltaskqueue.h"

using namespace std;

static void Static_TaskQueueCallback(PAL_Instance* /*pal*/, void* userData)
{
    Task* task = static_cast<Task*>(userData);
    if (task != NULL)
    {
        task->Execute();
    }
}

/////////////////////////////////////////////////////
// EventTaskQueue
/////////////////////////////////////////////////////

PAL_Error EventTaskQueue::AddTask(Task* task)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_EventTaskQueueAdd(m_pal, Static_TaskQueueCallback, task, &(task->m_taskId));
    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error EventTaskQueue::AddTask(Task* task, uint32 priority)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_EventTaskQueueAddWithPriority(m_pal, Static_TaskQueueCallback, task, &(task->m_taskId), priority);

    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error EventTaskQueue::RemoveTask(Task* task, bool cancelIfRunning)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_EventTaskQueueRemove(m_pal, task->m_taskId, (cancelIfRunning) ? TRUE : FALSE);

    return (err == PAL_Ok) ? PAL_Ok :
    ( (err == PAL_ErrNoData) ? PAL_ErrNoData : err );
}

void EventTaskQueue::RemoveAllTasks(void)
{
    PAL_EventTaskQueueRemoveAll(m_pal);
}

/////////////////////////////////////////////////////
// WorkerTaskQueue
/////////////////////////////////////////////////////

WorkerTaskQueue::WorkerTaskQueue(PAL_Instance* pal, shared_ptr<std::string> name)
: TaskQueue(pal),
m_name(name),
m_queueHandle(NULL)
{
    PAL_WorkerTaskQueueCreate(pal, name->c_str(), &m_queueHandle);
}

WorkerTaskQueue::~WorkerTaskQueue(void)
{
    if (m_queueHandle != NULL)
    {
        PAL_WorkerTaskQueueDestroy(m_pal, m_queueHandle);
    }
}

const shared_ptr<std::string> WorkerTaskQueue::GetName(void)
{
    return m_name;
}

bool WorkerTaskQueue::IsRuningThread()
{
    return (PAL_WorkerTaskQueueIsRunningThread(m_pal, m_name.get() ? m_name->c_str() : "")) == TRUE;
}

PAL_Error WorkerTaskQueue::AddTask(Task* task)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    if (m_queueHandle == NULL)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_WorkerTaskQueueAdd(m_pal, m_queueHandle, Static_TaskQueueCallback, task, &(task->m_taskId));

    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error WorkerTaskQueue::AddTask(Task* task, uint32 priority)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    if (m_queueHandle == NULL)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_WorkerTaskQueueAddWithPriority(m_pal, m_queueHandle, Static_TaskQueueCallback, task, &(task->m_taskId), priority);

    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error WorkerTaskQueue::RemoveTask(Task* task, bool cancelIfRunning)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    if (m_queueHandle == NULL)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_WorkerTaskQueueRemove(m_pal, m_queueHandle, task->m_taskId, (cancelIfRunning) ? TRUE : FALSE);

    return (err == PAL_Ok) ? PAL_Ok : err;
}

void WorkerTaskQueue::RemoveAllTasks(void)
{
    PAL_WorkerTaskQueueRemoveAll(m_pal, m_queueHandle);
}

void* WorkerTaskQueue::GetQueueHandle()
{
    return m_queueHandle;
}

nb_threadId WorkerTaskQueue::GetRunningThreadId() const
{
    return PAL_WorkerTaskQueueGetRunningThreadId(m_queueHandle);
}


/////////////////////////////////////////////////////
// RenderTaskQueue
/////////////////////////////////////////////////////

PAL_Error RenderTaskQueue::AddTask(Task* task)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_RenderTaskQueueAdd(m_pal, Static_TaskQueueCallback, task, &(task->m_taskId));
    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error RenderTaskQueue::AddTask(Task* task, uint32 priority)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_RenderTaskQueueAddWithPriority(m_pal, Static_TaskQueueCallback, task, &(task->m_taskId), priority);

    return (err == PAL_Ok) ? PAL_Ok : err;
}

PAL_Error RenderTaskQueue::RemoveTask(Task* task, bool cancelIfRunning)
{
    if (!task)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_RenderTaskQueueRemove(m_pal, task->m_taskId, (cancelIfRunning) ? TRUE : FALSE);

    return (err == PAL_Ok) ? PAL_Ok :
    ( (err == PAL_ErrNoData) ? PAL_ErrNoData : err );
}

void RenderTaskQueue::RemoveAllTasks(void)
{
    PAL_RenderTaskQueueRemoveAll(m_pal);
}

nb_threadId RenderTaskQueue::GetRunningThreadId() const
{
    return PAL_RenderTaskQueueGetRunningThreadId(m_pal);
}

bool RenderTaskQueue::IsRuningThread()
{
    return PAL_RenderTaskQueueIsRunningThread(m_pal) == TRUE;
}
