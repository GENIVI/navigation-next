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

    @file     threadwin32.h
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Win32 class for thread.

    Windows definition for PAL thread.
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

#ifndef THREADWIN32_H
#define THREADWIN32_H

extern "C" {
  #include <windows.h>
}

#include "thread.h"

namespace nimpal
{
    namespace taskqueue
    {
        class Task;
        class TaskQueue;

        class ThreadWin32 : public Thread
        {
        public:
            ThreadWin32(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener);
            virtual ~ThreadWin32(void);

            virtual void NewTaskNotify(void);
            virtual void CancelRunningTaskNotify(void);

            virtual PAL_Error StartThread(void);
            virtual nb_threadId ThreadId(void) const;
            void ThreadMain(void);

        private:
            enum ThreadEvents
            {
                EvtNewTask = 0,
                EvtCancelTask,
                EvtExit,
                // Do not add events after this one
                EvtLast
            };

            // Forbid to copy class object
            ThreadWin32(const ThreadWin32&);
            ThreadWin32& operator=(const ThreadWin32&);

        private:
            HANDLE   m_cbThread;
            DWORD    m_cbThreadId;
            HANDLE   m_eventArr[EvtLast];
            HANDLE   m_threadStartedEvent;
        };
    }
}

#endif
