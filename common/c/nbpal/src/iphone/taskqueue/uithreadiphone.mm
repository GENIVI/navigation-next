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

    @file     uithreadiphone.cpp
    @date     09/14/2011
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for iPhone UI thread.

    iPhone implementation for PAL UI thread.
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

#include "uithreadiphone.h"
#include "palstdlib.h"

namespace nimpal
{
    namespace taskqueue
    {
        static void Static_NewTaskSourceCb(void* info);

        UiThreadIphone::UiThreadIphone(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener)
            : Thread(pal, taskQueue, listener),
              m_uiThreadId( pthread_self() ),
              m_newTaskSource(nil)
        {
        }

        UiThreadIphone::~UiThreadIphone(void)
        {
            if (m_newTaskSource != NULL)
            {
                CFRunLoopRemoveSource(CFRunLoopGetMain(), m_newTaskSource, kCFRunLoopCommonModes);
                CFRelease(m_newTaskSource);
                m_newTaskSource = NULL;
            }
        }

        void UiThreadIphone::NewTaskNotify(void)
        {
            if (m_newTaskSource != NULL)
            {
                CFRunLoopSourceSignal(m_newTaskSource);
                CFRunLoopWakeUp(CFRunLoopGetMain());
            }
        }

        void UiThreadIphone::CancelRunningTaskNotify(void)
        {
            // TODO: figure out what to do
        }

        PAL_Error UiThreadIphone::StartThread(void)
        {
            if (GetState() != Invalid)
            {
                // UI thread class was already initialized
                return PAL_Ok;
            }

            // Initialize the run loop new task source
            CFRunLoopSourceContext context = {0, this, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Static_NewTaskSourceCb};
            m_newTaskSource = CFRunLoopSourceCreate(NULL, 0, &context);
            if(m_newTaskSource == NULL)
            {
                return PAL_ErrNoMem;
            }
            
            CFRunLoopAddSource(CFRunLoopGetMain(), m_newTaskSource, kCFRunLoopCommonModes);
            
            SetState(Pending);

            return PAL_Ok;
        }

        nb_threadId UiThreadIphone::ThreadId(void) const
        {
            return m_uiThreadId;
        }
        
        void UiThreadIphone::NewTaskSourceCb(void)
        {
            ExecuteTasks();
        }

        void Static_NewTaskSourceCb(void* info)
        {
            UiThreadIphone* pMe = static_cast<UiThreadIphone*>(info);
            if (pMe != NULL)
            {
                pthread_setname_np("UiThreadIphone");
                pMe->NewTaskSourceCb();
            }
        }
    }
}
