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

    @file     threadposix.h
    @date     08/25/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    POSIX class for thread.

    POSIX definition for PAL thread.
*/
/*
    (C) Copyright 2012 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef THREADPOSIX_H
#define THREADPOSIX_H

#include "thread.h"
#include "palevent.h"
#include "signal.h"

namespace nimpal
{
    namespace taskqueue
    {
        class Task;
        class TaskQueue;

        class ThreadPosix : public Thread
        {
        public:
            ThreadPosix(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener, const char* name);
            virtual ~ThreadPosix(void);

            virtual void NewTaskNotify(void);
            virtual void CancelRunningTaskNotify(void);

            virtual PAL_Error StartThread(void);
            virtual nb_threadId ThreadId(void) const;
            void ThreadMain(void);

        private:
            // Forbid to copy class object
            ThreadPosix(const ThreadPosix&);
            ThreadPosix& operator=(const ThreadPosix&);

        private:
            pthread_t        m_cbThreadId;                   /*!< POSIX thread */
            PAL_Event*       m_NewTaskEvent;                 /* Thread New Task event */
            PAL_Event*       m_threadStartedEvent;           /*!< This event is signalled when thread is started. */
            bool             m_exit;                         /* set true when we need to exit from maint thread loop */
            static const int MAX_THREAD_NAME_LENGTH = 32;
            char             m_name[MAX_THREAD_NAME_LENGTH];
        };
    }
}

#endif //THREADPOSIX_H
