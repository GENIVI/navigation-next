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

    @file     taskmanagermultithreaded.h
    @date     09/16/2011
    @defgroup PAL_TASKQUEUE PAL task queue component
    @brief    Class for task manager in multi-threaded configuration.

    Common definition for PAL task manager in multi-threaded configuration.

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


#ifndef TASKMANAGERMULTITHREADED_H
#define TASKMANAGERMULTITHREADED_H

#include "taskmanager.h"

namespace nimpal
{
    namespace taskqueue
    {
        class TaskManagerMultiThreaded : public TaskManager
        {
        public:
            virtual ~TaskManagerMultiThreaded(void);
            static PAL_Error CreateTaskManager(PAL_Instance* pal, TaskManager** taskManager);
            virtual PAL_Error CreateWorkerQueue(const char* name, TaskQueue::TaskQueueId* queueId);
            virtual PAL_Error DestroyWorkerQueue(TaskQueue::TaskQueueId queueId);
            virtual TaskQueue::TaskQueueId GetWorkerQueueByName(const char* name) const;
            virtual nb_threadId GetWorkerThreadIdByName(const char* name) const;
            virtual nb_threadId GetCurrentThreadId(void) const;
            virtual nb_threadId GetEventThreadId(void) const;
            virtual nb_threadId GetRenderThreadId(void) const;
            virtual bool Destroy(void);

        private:
            typedef struct _QueueNode
            {
                TaskQueue* taskQueue;
                Thread*    thread;
            } QueueNode;

            // Hide constructor to force initialize object
            TaskManagerMultiThreaded(PAL_Instance* pal);
            // Forbid to copy class object
            TaskManagerMultiThreaded(const TaskManagerMultiThreaded&);
            TaskManagerMultiThreaded& operator=(const TaskManagerMultiThreaded&);
            virtual void ThreadStateChanged(Thread* thread, Thread::State state);
            virtual void FindThreadAndQueue(TaskQueue::TaskQueueId queueId, Thread** thread, TaskQueue** queue) const;
            void FindThreadAndQueue(const char* name, Thread** thread, TaskQueue** queue) const;
            void RemoveWorkerThreadAndQueue(Thread* thread, TaskQueue* queue);
            virtual PAL_Error Initialize(void);

        private:
            // Queue with the pairs "task queue-thread"
            std::deque<QueueNode> m_workerQueues;
            // Event task queue(CCC thread tasks). It can be only one such queue
            TaskQueue* m_eventTaskQueue;
            // Event thread. It can be only one such thread
            Thread* m_eventThread;
            // UI task queue. It can be only one such queue
            TaskQueue* m_uiTaskQueue;
            // UI thread. It can be only one such thread
            Thread* m_uiThread;

            TaskQueue* m_renderTaskQueue;
            Thread* m_renderThread;
        };
    }
}

#endif
