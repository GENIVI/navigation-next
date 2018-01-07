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

    @file     uithreadqt.cpp
    @date     02/10/2012
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    Class for Qt QNX UI thread.

    Qt QNX implementation for PAL UI thread.
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "uithreadqt.h"
#include "palstdlib.h"
#include <QCoreApplication>

//#define DTHREAD
#ifdef DTHREAD
#include <QDebug>
#include <pthread.h>
#endif


UiThreadQt::UiThreadQt(PAL_Instance* pal, nimpal::taskqueue::TaskQueue* taskQueue, nimpal::taskqueue::ThreadStateListener* listener)
    : QObject(),
      Thread(pal, taskQueue, listener),
      m_uiThread(NULL)
{
    if (QCoreApplication::instance())
    {
        m_uiThread = QCoreApplication::instance()->thread();
    }
    connect(this, SIGNAL(NewTask()), this, SLOT(OnNewTask()), Qt::QueuedConnection);
}

UiThreadQt::~UiThreadQt(void)
{
}

void UiThreadQt::NewTaskNotify(void)
{
    if (m_uiThread != NULL)
    {
        emit NewTask();
    }
}

void UiThreadQt::OnNewTask()
{
    ExecuteTasks();
}

void UiThreadQt::CancelRunningTaskNotify(void)
{
    // TODO: figure out what to do
}

PAL_Error UiThreadQt::StartThread(void)
{
    PAL_Error err = PAL_Failed;
    if (GetState() != Invalid)
    {
        // UI thread class was already initialized
        return PAL_Ok;
    }
    if (m_pal == NULL)
    {
        return PAL_ErrNoInit;
    }

    // Initialize thread
    if (m_uiThread != NULL)
    {
        moveToThread(m_uiThread);
        m_uiThread->start();
        SetState(Created);
        err = PAL_Ok;
#ifdef DTHREAD
    qWarning() << "DTHREAD id:" << pthread_self() << " name: UI thread";
#endif
    }
    else
    {
        err = PAL_ErrQueueCannotStartNewThread;
    }

    return err;
}

nb_threadId UiThreadQt::ThreadId(void) const
{
    return(nb_threadId) m_uiThread;
}
