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

    @file     taskqueue.h
    @date     08/23/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for task queue.

    Common definition for PAL task queue.
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

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "palerror.h"
#include "pal.h"
#include "base.h"
#include "pallock.h"
#include "paltaskqueue.h"
#include <deque>

namespace nimpal
{
namespace taskqueue
{
class Task;

class TaskQueue : public Base
{
public:
    typedef uint32 TaskQueueId;

    static const TaskQueueId INVALID_QUEUE_ID = 0;
    static const TaskQueueId UI_QUEUE_ID = 1;
    static const TaskQueueId EVENT_QUEUE_ID = 2;
    static const TaskQueueId RENDER_QUEUE_ID = 3;
    static const TaskQueueId WORKER_QUEUE_ID_BASE = 10;

    TaskQueue(PAL_Instance* pal, TaskQueueId id, const char* name = NULL);
    ~TaskQueue(void);

    PAL_Error Queue(Task* task);
    PAL_Error Dequeue(TaskId taskId, bool cancelIfRunning);
    PAL_Error Clear(void);

    Task* GetTask(void);
    bool IsEmpty(void) const;
    const char* GetName(void) const { return m_name; }
    TaskQueueId Id(void) const { return m_queueId; }

private:
    // Forbid to copy class object
    TaskQueue(const TaskQueue&);
    TaskQueue& operator=(const TaskQueue&);

    std::deque<Task*>::iterator GetInsertionIterator(uint32 newTaskPriority);

private:
    // PAL lock object
    PAL_Lock* m_lock;

    // Unique queue id.
    TaskQueueId m_queueId;

    // Task queue.
    std::deque<Task*> m_taskQueue;

    // Queue name, optional
    char m_name[MAX_QUEUE_NAME_LENGTH];

#ifndef NDEBUG
    int m_queueSizePeak;
#endif

    static volatile TaskQueueId m_nextId;
};
}
}

#endif
