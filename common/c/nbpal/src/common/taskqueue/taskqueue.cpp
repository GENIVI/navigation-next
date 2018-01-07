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

    @file     taskqueue.cpp
    @date     08/23/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task queue.

    Common implementation for PAL task queue.
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

#include "taskqueue.h"
#include "task.h"
#include "palstdlib.h"
#include "palatomic.h"
#include "paltestlog.h"

namespace nimpal
{
namespace taskqueue
{
volatile TaskQueue::TaskQueueId TaskQueue::m_nextId = WORKER_QUEUE_ID_BASE;

TaskQueue::TaskQueue(PAL_Instance* pal, TaskQueueId id, const char* name)
        : m_lock(NULL),
          m_queueId(id)
#ifndef NDEBUG
        ,m_queueSizePeak(0)
#endif
{
    if (id == WORKER_QUEUE_ID_BASE)
    {
        m_queueId = PAL_AtomicIncrement(reinterpret_cast<volatile int32*>(&m_nextId));
    }

    nsl_memset(m_name, 0, MAX_QUEUE_NAME_LENGTH * sizeof(char));
    if (name != NULL)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "create queue %d %s", m_queueId, name);
        nsl_strlcpy(m_name, name, MAX_QUEUE_NAME_LENGTH);
    }
    PAL_LockCreate(pal, &m_lock);
}

TaskQueue::~TaskQueue(void)
{
    Clear();
    PAL_LockDestroy(m_lock);
}

PAL_Error TaskQueue::Queue(Task* task)
{
    if (task == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_LockLock(m_lock);

    m_taskQueue.insert(GetInsertionIterator(task->Priority()), task);

#ifndef NDEBUG
    int curSize = (int)m_taskQueue.size();
    if (abs(curSize - m_queueSizePeak) > 50)
    {
        if (curSize == 1)
        {
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelWarning,
                        "queue:%d(%s): peakSize changed from %d to 1",
                        m_queueId, m_name, m_queueSizePeak);
            m_queueSizePeak = 0;
        }
        else
        {
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelError,
                        "queue:%d(%s) peakSize changed from: %d to: %d",
                        m_queueId, m_name, m_queueSizePeak, curSize);
            m_queueSizePeak = curSize;
        }
    }
#endif

    PAL_LockUnlock(m_lock);
    return PAL_Ok;
}

PAL_Error TaskQueue::Dequeue(TaskId taskId, bool /*cancelIfRunning*/)
{
    size_t size = 0;
    Task* task = NULL;
    PAL_Error err = PAL_ErrNoData;

    PAL_LockLock(m_lock);

    size = m_taskQueue.size();
    for (size_t i = 0; i < size; ++i)
    {
        task = m_taskQueue.at(i);
        if (task != NULL && task->Id() == taskId)
        {
            delete task;
            m_taskQueue.erase(m_taskQueue.begin() + i);
            err = PAL_Ok;
            break;
        }
    }

    PAL_LockUnlock(m_lock);

    return err;
}

PAL_Error TaskQueue::Clear(void)
{
    PAL_LockLock(m_lock);

    while (!m_taskQueue.empty())
    {
        delete m_taskQueue.front();
        m_taskQueue.pop_front();
    }

    PAL_LockUnlock(m_lock);

    return PAL_Ok;
}

Task* TaskQueue::GetTask(void)
{
    Task* task = NULL;

    PAL_LockLock(m_lock);

    if (!m_taskQueue.empty())
    {
        task = m_taskQueue.front();
        m_taskQueue.pop_front();
    }

    PAL_LockUnlock(m_lock);
    return task;
}

bool TaskQueue::IsEmpty(void) const
{
    bool isEmpty = false;

    PAL_LockLock(m_lock);

    isEmpty = m_taskQueue.empty();

    PAL_LockUnlock(m_lock);

    return isEmpty;
}

/* It's a binary search*/
std::deque<Task*>::iterator TaskQueue::GetInsertionIterator(uint32 newTaskPriority)
{
    if (m_taskQueue.size() == 0)
    {
        return m_taskQueue.end();
    }

    // Following statement added because most probably case - all tasks in queue have the same priority
    if ((*(m_taskQueue.end() - 1))->Priority() <= newTaskPriority)
    {
        return m_taskQueue.end();
    }

    uint32 leftBorder  = 0;
    uint32 rightBorder = m_taskQueue.size() - 1;
    uint32 currentIndex = 0;

    while (leftBorder < rightBorder)
    {
        currentIndex = (leftBorder + rightBorder) / 2;

        if (newTaskPriority < m_taskQueue.at(currentIndex)->Priority())
        {
            if (rightBorder != currentIndex)
            {
                rightBorder = currentIndex;
            }
            else
            {
                currentIndex--;
                break;
            }
        }
        else if (newTaskPriority > m_taskQueue.at(currentIndex)->Priority())
        {
            if (leftBorder != currentIndex)
            {
                leftBorder = currentIndex;
            }
            else
            {
                currentIndex++;
                break;
            }
        }
        else
        {
            while (m_taskQueue.at(++currentIndex)->Priority() == newTaskPriority);
            break;
        }
    }

    return (m_taskQueue.begin() + currentIndex);
}

} // namespace taskqueue
} // namespace nimpal
