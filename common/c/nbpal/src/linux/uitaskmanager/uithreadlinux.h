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

    @file     uithreadlinux.h
    @date     02/10/2012
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Linux class for UI thread.

     Linux definition for PAL UI thread.

    Actually PAL UI thread is not thread. It is inherited from the Thread class in order to
    have the same interface as Event and Worker thread classes. From the user perspective it
    should behave as UI thread. Unlike Event and Worker thread classes, this class doesn't create
    the new thread - UI thread is created by user application.

    The responsibility of this class is to call user callbacks in the context of UI thread. In general CCC
    has no mechanism to find out what thread is UI thread, so we treat the thread where PAL was created
    as UI thread.

    On Win32 platform we create internal window object. User callbacks will be called from the WndProc() in
    the context of UI thread.
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef UITHREADQT_H
#define UITHREADQT_H

#include "thread.h"
#include "threadposix.h"
namespace nimpal
{
    namespace taskqueue
    {
        class Task;
        class TaskQueue;

        class UiThreadLinux :  public ThreadPosix
        {
        public:
            UiThreadLinux(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener);
            virtual ~UiThreadLinux(void);

            virtual void NewTaskNotify(void);
            virtual void CancelRunningTaskNotify(void);

            virtual PAL_Error StartThread(void);
            virtual nb_threadId ThreadId(void) const;

        private:
            // Forbid to copy class object
            UiThreadLinux(const UiThreadLinux&);
            UiThreadLinux& operator=(const UiThreadLinux&);

        private:
            ThreadPosix*                 m_uiThread;
        };
    }
}

#endif
