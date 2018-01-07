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

    @file     thread.cpp
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for thread.

    Common implementation for PAL thread.
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

#include "thread.h"
#include "task.h"
#include "taskqueue.h"
#include "taskmanager.h"

namespace nimpal
{
    namespace taskqueue
    {
        Thread::Thread(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener)
            : m_pal(pal),
              m_taskQueue(taskQueue),
              m_stateListener(listener),
              m_runningTask(NULL),
              m_state(Invalid)
        {
            PAL_LockCreate(pal, &m_lock);
        }

        Thread::~Thread(void)
        {
            if (m_runningTask)
            {
                delete m_runningTask;
            }

            PAL_LockDestroy(m_lock);
        }

        void Thread::ExecuteTasks(void)
        {
            m_runningTask = m_taskQueue->GetTask();
            if (m_runningTask != NULL)
            {
                SetState(Executing);
                m_runningTask->Execute();
                SetState(Pending);

                delete m_runningTask;
                m_runningTask = NULL;

                if (!m_taskQueue->IsEmpty())
                {
                    // We execute one task at a time to not block the thread.
                    // This is critical for UI thread.
                    // It might be OK for Event and Worker threads, need to investigate this.
                    NewTaskNotify();
                }
            }
        }

        Thread::State Thread::GetState(void) const
        {
            PAL_LockLock(m_lock);
            State state = m_state;
            PAL_LockUnlock(m_lock);

            return state;
        }

        void Thread::SetState(State state)
        {
            PAL_LockLock(m_lock);
            bool equal = (m_state == state);
            m_state = state;
            PAL_LockUnlock(m_lock);

            if (!equal && m_stateListener != NULL)
            {
                m_stateListener->ThreadStateChanged(this, state);
            }
        }
    }
}
