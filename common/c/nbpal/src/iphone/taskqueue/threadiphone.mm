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

    @file     threadiphone.cpp
    @date     09/12/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for iPhone thread.

    iPhone implementation for PAL thread.
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

#include "threadiphone.h"
#include "palstdlib.h"
#include "palimpl.h"
#include <Foundation/Foundation.h>

namespace nimpal
{
    namespace taskqueue
    {
      //  static void* Static_ThreadMain(void* arg);
        static void Static_NewTaskSourceCb(void* info);

        ThreadIphone::ThreadIphone(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener, const char* name)
            : Thread(pal, taskQueue, listener),
              m_thread(NULL),
              m_runLoop(nil),
              m_newTaskSource(nil),
              m_threadStartedEvent(NULL),
              m_name(name)
        {
            PAL_EventCreate(pal, &m_threadStartedEvent);
        }

        ThreadIphone::~ThreadIphone(void)
        {
            if (m_newTaskSource != NULL)
            {
                CFRunLoopRemoveSource(m_runLoop, m_newTaskSource, kCFRunLoopCommonModes);
                CFRelease(m_newTaskSource);
                m_newTaskSource = NULL;
            }
            
            if (m_runLoop != NULL)
            {
                CFRunLoopStop(m_runLoop);
                CFRelease(m_runLoop);
                m_runLoop = NULL;
            }
            
            if (m_thread != NULL)
            {
                pthread_join(m_thread, NULL);
            }

            PAL_EventDestroy(m_threadStartedEvent);
        }

        void ThreadIphone::NewTaskNotify(void)
        {
            if (m_newTaskSource != NULL && m_runLoop != NULL)
            {
                CFRunLoopSourceSignal(m_newTaskSource);
                CFRunLoopWakeUp(m_runLoop);
            }
        }

        void ThreadIphone::CancelRunningTaskNotify(void)
        {
            // TODO: figure out what to do
        }

        PAL_Error ThreadIphone::StartThread(void)
        {
            if (GetState() != Invalid)
            {
                // Thread was already created and started
                return PAL_Ok;
            }

            if (m_pal == NULL || m_threadStartedEvent == NULL)
            {
                return PAL_ErrNoInit;
            }
            
            PAL_Error err = PAL_Failed;
            
            // Initialize thread
            if (pthread_create(&m_thread, NULL, ThreadIphone::Static_ThreadMain, this) == 0)
            {
                PAL_EventWaitForEvent(m_threadStartedEvent);
                SetState(Created);
                err = PAL_Ok;
            }
            else
            {
                err = PAL_ErrQueueCannotStartNewThread;
            }
            
            return err;
        }
        
        nb_threadId ThreadIphone::ThreadId(void) const
        {
            return m_thread;
        }

        void* ThreadIphone::ThreadMain(void)
        {
            NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

            // Initialize the run loop new task source
            CFRunLoopSourceContext context = {0, this, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Static_NewTaskSourceCb};
            m_newTaskSource = CFRunLoopSourceCreate(NULL, 0, &context);
            if(m_newTaskSource == NULL)
            {
                [pool release];
                return reinterpret_cast<void*>(EXIT_FAILURE);
            }
            
            // Initialize the run loop
            m_runLoop = CFRunLoopGetCurrent();
            CFRetain(m_runLoop);
            
            CFRunLoopAddSource(m_runLoop, m_newTaskSource, kCFRunLoopCommonModes);

            PAL_EventSet(m_threadStartedEvent);
            CFRunLoopRun();
            
            [pool release];
            
            return reinterpret_cast<void*>(EXIT_SUCCESS);
        }

        void ThreadIphone::NewTaskSourceCb(void)
        {
            ExecuteTasks();
        }
        
        void* ThreadIphone::Static_ThreadMain(void* arg)
        {
            ThreadIphone* pMe = static_cast<ThreadIphone*>(arg);
            if (pMe != NULL)
            {
                pthread_setname_np(pMe->m_name.c_str());
                return pMe->ThreadMain();
            }
            else
            {
                return reinterpret_cast<void*>(EXIT_FAILURE);
            }
        }
        
        void Static_NewTaskSourceCb(void* info)
        {
            ThreadIphone* pMe = static_cast<ThreadIphone*>(info);
            if (pMe != NULL)
            {
                pMe->NewTaskSourceCb();
            }
        }
    }
}
