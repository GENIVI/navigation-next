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

    @file     threadqt.cpp
    @date     02/15/2015
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for Qt thread.

    Qt implementation for PAL thread.
*/
/*
    (C) Copyright 2015 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "threadqt.h"
#include "palstdlib.h"
#include "taskqueue.h"
#include "task.h"
#include <QThread>

//#define DTHREAD
#ifdef DTHREAD
#include <QDebug>
#include <pthread.h>
#endif


nb_threadId nimpal::taskqueue::Thread::GetCurrentThreadId()
{
    return (nb_threadId)QThread::currentThread();
}


ThreadQt::ThreadQt(PAL_Instance* pal, nimpal::taskqueue::TaskQueue* taskQueue, nimpal::taskqueue::ThreadStateListener* listener, const char* name)
    : Thread(pal, taskQueue, listener)
{
    this->setObjectName(name);
}

ThreadQt::~ThreadQt(void)
{
    quit();
    wait();

    // Flag the thread to exit the loop
    if (GetState() != Invalid)
    {
        m_taskQueue->Clear();
    }
}

void ThreadQt::CancelRunningTaskNotify(void)
{
    // Do nothing for this
}

PAL_Error ThreadQt::StartThread(void)
{
    if (GetState() != Invalid)
    {
        // Thread was already created and started
        return PAL_Ok;
    }

    if (m_pal == NULL)
    {
        return PAL_ErrNoInit;
    }

    SetState(Created);

    moveToThread(this);
    connect(this,SIGNAL(NewTask()),this, SLOT(OnNewTask()), Qt::QueuedConnection);

    start();
    return PAL_Ok;
}

void ThreadQt::NewTaskNotify(void)
{
    emit NewTask();
}

void ThreadQt::OnNewTask()
{
    ExecuteTasks();
}

void ThreadQt::run()
{
#ifdef DTHREAD
    qWarning() << "DTHREAD id:" << pthread_self() << " name: " << objectName();
#endif
    exec();
}

nb_threadId ThreadQt::ThreadId(void) const
{
    return (nb_threadId)this;
}
