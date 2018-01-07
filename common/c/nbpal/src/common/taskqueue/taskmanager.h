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

    @file     taskmanager.h
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task manager.

    Common definition for PAL task manager.
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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "pal.h"
#include "base.h"
#include "palerror.h"
#include "pallock.h"
#include "paltaskqueue.h"
#include "taskqueue.h"
#include "thread.h"
#include "palqueue.h"

namespace nimpal
{
namespace taskqueue
{

class Task;

class ThreadStateListener
{
public:
            virtual void ThreadStateChanged(Thread* thread, Thread::State state) = 0;
            virtual ~ThreadStateListener(void){}
};

class TaskManager : public Base, public ThreadStateListener
{
public:
            virtual ~TaskManager(void);

            PAL_Error AddTask(TaskQueue::TaskQueueId queueId,
                              PAL_TaskQueueCallback callback,
                              void* userData, TaskId* taskId,
                              TaskPriority priority = DEFAULT_TASK_PRIORITY);
            PAL_Error RemoveTask(TaskQueue::TaskQueueId queueId, TaskId taskId, bool cancelIfRunning);
            PAL_Error RemoveAllTasks(TaskQueue::TaskQueueId queueId);

            virtual PAL_Error CreateWorkerQueue(const char* name, TaskQueue::TaskQueueId* queueId);
            virtual PAL_Error DestroyWorkerQueue(TaskQueue::TaskQueueId queueId);
            virtual TaskQueue::TaskQueueId GetWorkerQueueByName(const char* name) const;
            virtual nb_threadId GetWorkerThreadIdByName(const char* name) const;
            virtual nb_threadId GetEventThreadId(void) const = 0;
            virtual nb_threadId GetRenderThreadId(void) const = 0;
            virtual nb_threadId GetCurrentThreadId() const = 0;
            // this function returns false if TaskManager is busy
            virtual bool Destroy(void) = 0;

protected:
            class Lock : public Base
            {
            public:
                Lock(PAL_Lock* lock) : m_lock(lock) { PAL_LockLock(m_lock); }
        virtual ~Lock() { PAL_LockUnlock(m_lock); }
            private:
                PAL_Lock* m_lock;
            };

            // Hide constructor to force initialize object
            TaskManager(PAL_Instance* pal);

            // Forbid to copy class object
            TaskManager(const TaskManager&);
            TaskManager& operator=(const TaskManager&);

            virtual void ThreadStateChanged(Thread* thread, Thread::State state) = 0;
            virtual void FindThreadAndQueue(TaskQueue::TaskQueueId queueId, Thread** thread, TaskQueue** queue) const = 0;
            virtual PAL_Error Initialize(void) = 0;

protected:
            // PAL instance
            PAL_Instance* m_pal;

            // PAL lock object
            PAL_Lock* m_lock;

            bool       m_needToDestroy;
};
    
}
}

#endif
