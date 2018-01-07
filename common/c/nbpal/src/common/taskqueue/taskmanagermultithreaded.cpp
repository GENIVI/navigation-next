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

    @file     taskmanagermultithreaded.cpp
    @date     09/16/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task manager in multi-threaded configuration.

    Implementation for PAL task manager in multi-threaded configuration.
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

#include "taskmanagermultithreaded.h"
#include "pal.h"
#include "palstdlib.h"
#include "thread.h"
#include "palabstractfactory.h"

namespace nimpal
{
    namespace taskqueue
    {
        TaskManagerMultiThreaded::TaskManagerMultiThreaded(PAL_Instance* pal)
            : TaskManager(pal),
              m_eventTaskQueue(NULL),
              m_eventThread(NULL),
              m_uiTaskQueue(NULL),
              m_uiThread(NULL),
              m_renderTaskQueue(NULL),
              m_renderThread(NULL)
        {
        }

        TaskManagerMultiThreaded::~TaskManagerMultiThreaded(void)
        {
            QueueNode node;
            nsl_memset(&node, 0, sizeof(node));
            size_t size = m_workerQueues.size();
            for (size_t i = 0; i < size; ++i)
            {
                node = m_workerQueues.at(i);
                // Destroy the thread first because it can call the queue.
                delete node.thread;
                delete node.taskQueue;
            }

            if (m_eventThread)
            {
                delete m_eventThread;
                m_eventThread = NULL;
            }
            if (m_eventTaskQueue)
            {
                delete m_eventTaskQueue;
                m_eventTaskQueue = NULL;
            }

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

            if (m_renderThread)
            {
                delete m_renderThread;
                m_renderThread = NULL;
            }

            if (m_renderTaskQueue)
            {
                delete m_renderTaskQueue;
                m_renderTaskQueue = NULL;
            }
        }

        PAL_Error TaskManagerMultiThreaded::CreateTaskManager(PAL_Instance* pal, TaskManager** taskManager)
        {
            if (pal == NULL || taskManager == NULL)
            {
                return PAL_ErrBadParam;
            }

            TaskManagerMultiThreaded* manager = new TaskManagerMultiThreaded(pal);
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

        PAL_Error TaskManagerMultiThreaded::Initialize(void)
        {
            PAL_Error err = PAL_ErrNoMem;

            err = PAL_ErrNoMem;
            m_eventTaskQueue = new TaskQueue(m_pal, TaskQueue::EVENT_QUEUE_ID, "pal event task queue");
            if (m_eventTaskQueue != NULL)
            {
                m_eventThread = PalAbstractFactory::CreateThread(m_pal, m_eventTaskQueue, this, m_eventTaskQueue->GetName());
                if (m_eventThread != NULL)
                {
                    err = m_eventThread->StartThread();
                }
            }

            m_uiTaskQueue = new TaskQueue(m_pal, TaskQueue::UI_QUEUE_ID, "ui task queue");
            if (m_uiTaskQueue != NULL)
            {
                m_uiThread = PalAbstractFactory::CreateUiThread(m_pal, m_uiTaskQueue, this);
                if (m_uiThread != NULL)
                {
                    err = m_uiThread->StartThread();
                }
            }

            m_renderTaskQueue = new TaskQueue(m_pal, TaskQueue::UI_QUEUE_ID, "render task queue");
            if (m_renderTaskQueue != NULL)
            {
                m_renderThread = PalAbstractFactory::CreateRenderThread(m_pal, m_renderTaskQueue, this, "renderThread");
                if (m_renderThread != NULL)
                {
                    //err = m_renderThread->StartThread();
                }
            }

            return err;
        }

        PAL_Error TaskManagerMultiThreaded::CreateWorkerQueue(const char* name, TaskQueue::TaskQueueId* queueId)
        {
            Thread* thread = NULL;
            TaskQueue* queue = NULL;

            Lock lock(m_lock);

            if (name != NULL)
            {
                FindThreadAndQueue(name, &thread, &queue);
                if (queue != NULL)
                {
                    // Queue with this name exists. Return the queue id.
                    *queueId = queue->Id();
                    return PAL_Ok;
                }
            }

            PAL_Error err = PAL_ErrNoMem;
            queue = new TaskQueue(m_pal, TaskQueue::WORKER_QUEUE_ID_BASE, name);
            if (queue != NULL)
            {
                thread = PalAbstractFactory::CreateThread(m_pal, queue, this, queue->GetName());
                if (thread != NULL)
                {
                    err = thread->StartThread();
                    if (err == PAL_Ok)
                    {
                        QueueNode node;
                        nsl_memset(&node, 0, sizeof(node));
                        node.taskQueue = queue;
                        node.thread = thread;

                        m_workerQueues.push_back(node);

                        if (queueId)
                        {
                            *queueId = queue->Id();
                        }
                    }
                }
            }

            if (err != PAL_Ok)
            {
                if (queue != NULL)
                {
                    delete queue;
                }
                if (thread != NULL)
                {
                    delete thread;
                }
            }
            return err;
        }

        PAL_Error TaskManagerMultiThreaded::DestroyWorkerQueue(TaskQueue::TaskQueueId queueId)
        {
            if (queueId < TaskQueue::WORKER_QUEUE_ID_BASE)
            {
                // Only worker queue can be destroyed by user.
                return PAL_ErrBadParam;
            }

            Thread* thread = NULL;
            TaskQueue* queue = NULL;

            Lock lock(m_lock);

            FindThreadAndQueue(queueId, &thread, &queue);
            if (thread == NULL || queue == NULL)
            {
                return PAL_ErrQueueNotFound;
            }

            if (!queue->IsEmpty() || thread->GetState() == Thread::Executing)
            {
                // Queue has pending tasks or task is executed by the thread right now.
                return PAL_ErrQueueNotEmpty;
            }

            RemoveWorkerThreadAndQueue(thread, queue);
            return PAL_Ok;
        }

        nb_threadId TaskManagerMultiThreaded::GetWorkerThreadIdByName(const char* name) const
        {
            if (name == NULL)
            {
                return 0;
            }

            Thread* thread = NULL;
            TaskQueue* queue = NULL;

            Lock lock(m_lock);

            FindThreadAndQueue(name, &thread, &queue);

            return (thread != NULL) ? thread->ThreadId() : 0;
        }

        TaskQueue::TaskQueueId TaskManagerMultiThreaded::GetWorkerQueueByName(const char* name) const
        {
            if (name == NULL)
            {
                return TaskQueue::INVALID_QUEUE_ID;
            }

            Thread* thread = NULL;
            TaskQueue* queue = NULL;

            Lock lock(m_lock);

            FindThreadAndQueue(name, &thread, &queue);
            if (queue != NULL)
            {
                return queue->Id();
            }
            return TaskQueue::INVALID_QUEUE_ID;
        }

        // This utility function isn't thread-safe. We assume this function is always called inside the sync block.
        // No need to use PAL_Lock.
        void TaskManagerMultiThreaded::FindThreadAndQueue(TaskQueue::TaskQueueId queueId, Thread** thread, TaskQueue** queue) const
        {
            if (queueId == TaskQueue::EVENT_QUEUE_ID)
            {
                *thread = m_eventThread;
                *queue  = m_eventTaskQueue;
            }
            else if (queueId == TaskQueue::UI_QUEUE_ID)
            {
                *thread = m_uiThread;
                *queue = m_uiTaskQueue;
            }
            else if (queueId == TaskQueue::RENDER_QUEUE_ID)
            {
                *thread = m_renderThread;
                *queue = m_renderTaskQueue;
            }
            else
            {
                QueueNode node = {0};
                size_t size = m_workerQueues.size();
                *thread = NULL; *queue = NULL;

                for (size_t i = 0; i < size; ++i)
                {
                    node = m_workerQueues.at(i);
                    if (node.taskQueue->Id() == queueId)
                    {
                        *thread = node.thread;
                        *queue  = node.taskQueue;
                        break;
                    }
                }
            }
        }

        // This utility function isn't thread-safe. We assume this function is always called inside the sync block.
        // No need to use PAL_Lock.
        // The "name" is checked for NULL on the higher level.
        void TaskManagerMultiThreaded::FindThreadAndQueue(const char* name, Thread** thread, TaskQueue** queue) const
        {
            QueueNode node;
            nsl_memset(&node, 0, sizeof(node));
            size_t size = m_workerQueues.size();
            *thread = NULL; *queue = NULL;

            for (size_t i = 0; i < size; ++i)
            {
                node = m_workerQueues.at(i);
                if (nsl_strcmp(node.taskQueue->GetName(), name) == 0)
                {
                    *thread = node.thread;
                    *queue  = node.taskQueue;
                    break;
                }
            }
        }

        // This utility function isn't thread-safe. We assume this function is always called inside the sync block.
        // No need to use PAL_Lock.
        void TaskManagerMultiThreaded::RemoveWorkerThreadAndQueue(Thread* thread, TaskQueue* queue)
        {
            QueueNode node;
            nsl_memset(&node, 0, sizeof(node));
            size_t size = m_workerQueues.size();

            for (size_t i = 0; i < size; ++i)
            {
                node = m_workerQueues.at(i);
                if (node.taskQueue->Id() == queue->Id())
                {
                    m_workerQueues.erase(m_workerQueues.begin() + i);
                    break;
                }
            }

            delete thread;
            delete queue;
        }

        nb_threadId TaskManagerMultiThreaded::GetCurrentThreadId(void) const
        {
            return Thread::GetCurrentThreadId();
        }

        nb_threadId TaskManagerMultiThreaded::GetEventThreadId(void) const
        {
            return (m_eventThread != NULL) ? m_eventThread->ThreadId() : 0;
        }

        nb_threadId TaskManagerMultiThreaded::GetRenderThreadId(void) const
        {
            return (m_renderThread != NULL) ? m_renderThread->ThreadId() : 0;
        }

        void TaskManagerMultiThreaded::ThreadStateChanged(Thread* /*thread*/, Thread::State state)
        {
            if (state != Thread::Executing && state != Thread::Invalid && m_needToDestroy && Destroy())
            {
                PAL_ScheduleDestroy(m_pal);
            }
        }

        bool TaskManagerMultiThreaded::Destroy(void)
        {
            bool readyToDestroy = true;
            m_needToDestroy = true;

            readyToDestroy = readyToDestroy && (m_uiThread ? (m_uiThread->GetState() != Thread::Executing) : true);
            readyToDestroy = readyToDestroy && (m_uiTaskQueue ? m_uiTaskQueue->IsEmpty() : true);

            readyToDestroy = readyToDestroy && (m_eventThread ? (m_eventThread->GetState() != Thread::Executing) : true);
            readyToDestroy = readyToDestroy && (m_eventTaskQueue ? m_eventTaskQueue->IsEmpty() : true);

            readyToDestroy = readyToDestroy && (m_renderThread ? (m_renderThread->GetState() != Thread::Executing) : true);
            readyToDestroy = readyToDestroy && (m_renderTaskQueue ? m_renderTaskQueue->IsEmpty() : true);

            QueueNode node;
            nsl_memset(&node, 0, sizeof(node));
            size_t size = m_workerQueues.size();

            for (size_t i = size; i > 0 && readyToDestroy; --i)
            {
                node = m_workerQueues.at(i - 1);
                readyToDestroy = readyToDestroy && (node.thread ? (node.thread->GetState() != Thread::Executing) : true);
                readyToDestroy = readyToDestroy && (node.taskQueue ? node.taskQueue->IsEmpty() : true);
            }
            return readyToDestroy;
        }
    }
}
