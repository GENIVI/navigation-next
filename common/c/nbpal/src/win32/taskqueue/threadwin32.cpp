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

    @file     threadwin32.cpp
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for Win32 thread.

    Windows implementation for PAL thread.
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

#include "threadwin32.h"
#include "palstdlib.h"

nb_threadId nimpal::taskqueue::Thread::GetCurrentThreadId()
{
	return (nb_threadId)::GetCurrentThreadId();
}

namespace nimpal
{
    namespace taskqueue
    {
        static const DWORD THREAD_EXIT_WAIT_TIMEOUT = 1000; // in milliseconds
        static const DWORD THREAD_RUN_LOOP_TIMEOUT = 1000; // in milliseconds

        static DWORD WINAPI StaticThreadMain(void* pThis);

        ThreadWin32::ThreadWin32(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener)
            : Thread(pal, taskQueue, listener),
              m_cbThread(NULL),
              m_cbThreadId(0),
              m_threadStartedEvent(NULL)
        {
            nsl_memset(m_eventArr, 0, sizeof(m_eventArr));
        }

        ThreadWin32::~ThreadWin32(void)
        {
            if (m_cbThread != NULL)
            {
                SetEvent(m_eventArr[EvtExit]);
                WaitForSingleObject(m_cbThread, THREAD_EXIT_WAIT_TIMEOUT);
                CloseHandle(m_cbThread);
                m_cbThread = NULL;
            }

            for (int i = 0; i < EvtLast; ++i)
            {
                if (m_eventArr[i] != NULL)
                {
                    CloseHandle(m_eventArr[i]);
                }
            }

            if (m_threadStartedEvent != NULL)
            {
                CloseHandle(m_threadStartedEvent);
            }
        }

        void ThreadWin32::NewTaskNotify(void)
        {
            SetEvent(m_eventArr[EvtNewTask]);
        }

        void ThreadWin32::CancelRunningTaskNotify(void)
        {
            SetEvent(m_eventArr[EvtCancelTask]);
        }

        PAL_Error ThreadWin32::StartThread(void)
        {
            if (GetState() != Invalid)
            {
                // Thread was already created and started
                return PAL_Ok;
            }

            m_threadStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (m_threadStartedEvent == NULL)
            {
                return PAL_Failed;
            }

            PAL_Error err = PAL_Failed;

            m_cbThread = CreateThread(NULL, 0, StaticThreadMain, this, 0, &m_cbThreadId);
            if (m_cbThread != NULL)
            {
                int i = 0;
                for (i = 0; i < EvtLast; ++i)
                {
                    m_eventArr[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
                    if (m_eventArr[i] == NULL)
                    {
                        break;
                    }
                }
                if (i == EvtLast)
                {
                    err = PAL_Ok;
                    SetState(Created);
                }

                // Wait until thread is actually started
                WaitForSingleObject(m_threadStartedEvent, INFINITE);
            }
            else
            {
                err = PAL_ErrQueueCannotStartNewThread;
            }
            return err;
        }

        nb_threadId ThreadWin32::ThreadId(void) const
        {
            return m_cbThreadId;
        }

        void ThreadWin32::ThreadMain(void)
        {
            bool exit = false;
            DWORD dwRet = 0;

            SetState(Pending);
            SetEvent(m_threadStartedEvent);

            // main thread loop
            while (!exit)
            {
                dwRet = WaitForMultipleObjects(EvtLast, m_eventArr, FALSE, THREAD_RUN_LOOP_TIMEOUT);
                if (EvtNewTask == dwRet - WAIT_OBJECT_0)
                {
                    ExecuteTasks();
                }
                else if (EvtCancelTask == dwRet - WAIT_OBJECT_0)
                {
                    ; //TODO: figure out what to do
                }
                else if (EvtExit == dwRet - WAIT_OBJECT_0)
                {
                    exit = true;
                    SetState(Invalid);
                }

                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        DWORD WINAPI StaticThreadMain(void* pThis)
        {
            if (pThis != NULL)
            {
                static_cast<ThreadWin32*>(pThis)->ThreadMain();
            }
            return 0;
        }
    }
}
