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

    @file     uithreadandroid.h
    @date     06/29/2012
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    UI Android Thread implementation.

    UI Android Thread definition for PAL thread.
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
#ifndef UITHREADANDROID_H_
#define UITHREADANDROID_H_

#include "thread.h"
#include <jni.h>

namespace nimpal
{
    namespace taskqueue
    {
        class Task;
        class TaskQueue;

        class UIThreadAndroid : public Thread
        {
        public:
            UIThreadAndroid(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener);
            virtual ~UIThreadAndroid(void);

            virtual void NewTaskNotify(void);
            virtual void CancelRunningTaskNotify(void);

            virtual PAL_Error StartThread(void);
            virtual PAL_Error StopThread(void);
            virtual nb_threadId ThreadId(void) const;
            void ExecuteTasks(void);

        private:
            // Forbid to copy class object
            UIThreadAndroid(const UIThreadAndroid&);
            UIThreadAndroid& operator=(const UIThreadAndroid&);

        private:
            pthread_t    m_cbThreadId;/*!< POSIX thread */
            jobject      m_nativeThread;
        };
    }
}


#endif /* UITHREADANDROID_H_ */
