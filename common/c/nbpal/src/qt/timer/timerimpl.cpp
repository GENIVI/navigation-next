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

#include "timerimpl.h"

QtTimerManager::QtTimerManager(PAL_Instance *pal):
    mPal(pal)
{
    mTimerHandlers = new QMap<QString, QtTimerHandler*>();
    mMutex = new QMutex(QMutex::Recursive);
}


QtTimerManager::~QtTimerManager()
{
    delete mMutex;
    delete mTimerHandlers;
}

PAL_Error QtTimerManager::SetTimer(PAL_QueueHandle handle, int32 milliseconds, PAL_TimerCallback *callback, void *userData)
{
    QtTimerHandler* timerHandler = new QtTimerHandler(this, mPal, handle, callback, userData);
    timerHandler->Set(milliseconds);
    QString key = QtTimerHandler::CreateKey(callback, userData);

    mMutex->lock();
    mTimerHandlers->insert(key, timerHandler);
    mMutex->unlock();

    return PAL_Ok;
}

PAL_Error QtTimerManager::CancelTimer(PAL_TimerCallback *callback, void *userData)
{
    QString key = QtTimerHandler::CreateKey(callback, userData);

    RemoveTimerHandler(key);

    return PAL_Ok;
}

uint32 QtTimerManager::ExpirationTime(PAL_TimerCallback *callback, void *userData)
{
    QString key = QtTimerHandler::CreateKey(callback, userData);

    mMutex->lock();
    QtTimerHandler* handler = mTimerHandlers->value(key);
    mMutex->unlock();

    if(handler)
    {
        return handler->ExpirationTime();
    }
    return 0;
}

void QtTimerManager::RemoveTimerHandler(const QString& key)
{
    mMutex->lock();
    QtTimerHandler* handler = mTimerHandlers->take(key);
    mMutex->unlock();
    delete handler;
}

QtTimerHandler::QtTimerHandler(QtTimerManager* timerManager,
                               PAL_Instance *pal,
                               PAL_QueueHandle handle,
                               PAL_TimerCallback *callback,
                               void *userData):
    mTimerManager(timerManager),
    mPal(pal),
    mHandle(handle),
    mCallback(callback),
    mUserData(userData),
    mExpirationTime(0),
    mActive(false)
{

}

QtTimerHandler::~QtTimerHandler()
{
    if(mActive)
    {
        struct TimerCallback_t* t = new struct TimerCallback_t();
        t->callback = mCallback;
        t->userData = mUserData;
        if (mHandle)
        {
            PAL_WorkerTaskQueueAddWithPriority(mPal,
                                               mHandle,
                                               QtTimerHandler::TaskQueueCallbackTimerCancel,
                                               t,
                                               &mTaskQueueId,
                                               MAX_USER_TASK_PRIORITY);
        }
        else
        {
            PAL_EventTaskQueueAddWithPriority(mPal,
                                              QtTimerHandler::TaskQueueCallbackTimerCancel,
                                              t,
                                              &mTaskQueueId,
                                              MAX_USER_TASK_PRIORITY);
        }
    }
}

QString QtTimerHandler::CreateKey(PAL_TimerCallback *callback, void *userData)
{
    QString key;
    key.sprintf("%p%p", callback, userData);
    return key;
}

void QtTimerHandler::Set(int32 milliseconds)
{
    mActive = true;
    mExpirationTime = milliseconds;
    QTimer::singleShot(milliseconds, this, SLOT(OnTimeout()));
}

uint32 QtTimerHandler::ExpirationTime() const
{
    return mExpirationTime;
}

QString QtTimerHandler::Key() const
{
    return QtTimerHandler::CreateKey(mCallback, mUserData);
}

void QtTimerHandler::TaskQueueCallback(PAL_Instance* pal, void* self)
{
    QtTimerHandler* thiz = static_cast<QtTimerHandler*>(self);

    PAL_TimerCallback* callback = thiz->mCallback;
    void* userData = thiz->mUserData;

    thiz->mTimerManager->RemoveTimerHandler(thiz->Key());

    callback(pal, userData, PTCBR_TimerFired);
}

void QtTimerHandler::TaskQueueCallbackTimerCancel(PAL_Instance* pal, void* cbt)
{
    struct TimerCallback_t* t = (TimerCallback_t*)cbt;
    t->callback(pal, t->userData, PTCBR_TimerCanceled);
    delete t;
}

void QtTimerHandler::OnTimeout()
{
    if (mHandle)
    {
        PAL_WorkerTaskQueueAddWithPriority(mPal,
                                           mHandle,
                                           QtTimerHandler::TaskQueueCallback,
                                           this,
                                           &mTaskQueueId,
                                           MAX_USER_TASK_PRIORITY);
    }
    else
    {
        PAL_EventTaskQueueAddWithPriority(mPal,
                                          QtTimerHandler::TaskQueueCallback,
                                          this,
                                          &mTaskQueueId,
                                          MAX_USER_TASK_PRIORITY);
    }
    mActive = false;
}
