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
 @file     ReverseGeocodeRequestPrivate.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "ReverseGeocodeRequestPrivate.h"
#include "ReverseGeocodeInformationImpl.h"
#include "nbcontextprotected.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
#include "palevent.h"
#include "pallock.h"
#include "smartpointer.h"
#include <map>
#include "ReverseGeocodeTask.h"
#include "HybridManager.h"
#include "OnboardReverseGeocodeHandler.h"
#include "workerqueuemanager.h"
#include "commoncomponentconfiguration.h"

using namespace nbcommon;
using namespace protocol;
using namespace nbmap;

//todo: make a wrapper of the instance pool
typedef map<unsigned int, ReverseGeocodeRequestPrivate*> ReverseGeocodeRequestPrivateInstancePool;
static unsigned int instanceHandleBase = 1;
static ReverseGeocodeRequestPrivateInstancePool instancePool;
static PAL_Lock* instancePoolLock = NULL;


bool ReverseGeocodeRequestPrivate::IsRequestAlive()
{
    bool requestPrivAlive = false;
    {
        PAL_LockLock(instancePoolLock);
        for (ReverseGeocodeRequestPrivateInstancePool::iterator iter = instancePool.begin(); iter != instancePool.end(); iter ++)
        {
            if (iter->second == this)
            {
                requestPrivAlive = true;
                break;
            };
        }
        PAL_LockUnlock(instancePoolLock);
    }

    return requestPrivAlive;
}

ReverseGeocodeRequestPrivate* ReverseGeocodeRequestPrivate::NewInstance(NB_Context* context,
                                                                        const ReverseGeocodeRequest* user,
                                                                        const std::shared_ptr<ReverseGeocodeListener>& listener,
                                                                        const std::shared_ptr<ReverseGeocodeParameter>& sparam)
{
    if (!instancePoolLock)
    {
        PAL_LockCreate(NB_ContextGetPal(context), &instancePoolLock);
    }
    instanceHandleBase ++;

    ReverseGeocodeRequestPrivate* instance = new ReverseGeocodeRequestPrivate(context, user, listener, sparam);
    {
        PAL_LockLock(instancePoolLock);
        instancePool[instanceHandleBase] = instance;
        PAL_LockUnlock(instancePoolLock);
    }

    return instance;
}

ReverseGeocodeRequestPrivate::ReverseGeocodeRequestPrivate(NB_Context* context,
                                                           const ReverseGeocodeRequest* user,
                                                           const std::shared_ptr<ReverseGeocodeListener>& listener,
                                                           const std::shared_ptr<ReverseGeocodeParameter>& sparam)
    : m_context(context),
      m_palInstance(NB_ContextGetPal(context)),
      m_listener(listener),
      m_userRequest(user),
      m_inProgress(false)
{
    HybridMode mode = HybridManager::GetInstance(context)->GetMode();
    if (mode == HBM_ONBOARD_ONLY || sparam->config().onboardOnly()) {
        m_handler = new OnboardReverseGeocodeHandler(this);
        shared_ptr<WorkerQueueManager> manager = CommonComponentConfiguration::GetWorkerQueueManagerComponent(context);
        if (manager) {
            m_eventQueue = manager->RetrieveCommonWorkerTaskQueue();
        }
    } else {
        m_handler = new ReverseGeocodeHandler(shared_ptr<ReverseGeocodeAsyncCallback>(new ReverseGeocodeAsyncCallback(this)));
        m_eventQueue = shared_ptr<TaskQueue>(new EventTaskQueue(m_palInstance));
    }

    m_handler->SetParameter(sparam);
    // Get CCC and UI task queue
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(m_palInstance));
}

ReverseGeocodeRequestPrivate::~ReverseGeocodeRequestPrivate()
{
    delete m_handler;
    m_handler = NULL;
    m_context = NULL;
    m_palInstance = NULL;
    m_listener = NULL;
    //remove this pointer from instance pool

    PAL_LockLock(instancePoolLock);
    ReverseGeocodeRequestPrivateInstancePool::iterator iter = instancePool.begin();
    do
    {
        if (iter->second == this)
        {
            instancePool.erase(iter);
            break;
        }
        iter++;
    }
    while (iter != instancePool.end());
    if (iter == instancePool.end())
    {
        //printf("error::%s %p not registered in instance pool", __FUNCTION__, this);
    }
    PAL_LockUnlock(instancePoolLock);
}

void ReverseGeocodeRequestPrivate::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    if (m_handler)
    {
        m_handler->SetInvocationContext(m_context, inputSource, invocationMethod, screenId);
    }
}

void ReverseGeocodeRequestPrivate::Request()
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new ReverseGeocodeTask(this, &ReverseGeocodeRequestPrivate::CCC_Request));
    }
}

void ReverseGeocodeRequestPrivate::Cancel()
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new ReverseGeocodeTask(this, &ReverseGeocodeRequestPrivate::CCC_Cancel));
    }
}

bool ReverseGeocodeRequestPrivate::IsRequestInProgress()
{
    m_inProgress = false;
    PAL_Event* event;
    PAL_EventCreate(m_palInstance, &event);

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new ReverseGeocodeTaskCheckInProgress(this, &ReverseGeocodeRequestPrivate::CCC_CheckInProgress, event));
    }

    PAL_EventWaitForEvent(event);
    PAL_EventDestroy(event);
    return m_inProgress;
}

void ReverseGeocodeRequestPrivate::NotifySuccess(ReverseGeocodeInformation* info)
{
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new ReverseGeocodeUITaskSuccess(this, &ReverseGeocodeRequestPrivate::UI_NotifySuccess, info));
    }
}

void ReverseGeocodeRequestPrivate::NotifyError(NB_Error error)
{
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new ReverseGeocodeUITaskError(this, &ReverseGeocodeRequestPrivate::UI_NotifyError, error));
    }
}

void ReverseGeocodeRequestPrivate::NotifyProgress(int percentage)
{
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new ReverseGeocodeUITaskProgress(this, &ReverseGeocodeRequestPrivate::UI_NotifyProgress, percentage));
    }
}

//private menthods
void ReverseGeocodeRequestPrivate::CCC_Request(const ReverseGeocodeTask* pTask)
{
    m_handler->Request(m_context);
}

void ReverseGeocodeRequestPrivate::CCC_Cancel(const ReverseGeocodeTask* pTask)
{
    m_handler->Cancel();
}

void ReverseGeocodeRequestPrivate::CCC_CheckInProgress(const ReverseGeocodeTask* pTask)
{
    const ReverseGeocodeTaskCheckInProgress* task = static_cast<const ReverseGeocodeTaskCheckInProgress*>(pTask);

    m_inProgress = m_handler->IsInProgress();
    PAL_EventSet(task->m_event);
}

void ReverseGeocodeRequestPrivate::UI_NotifySuccess(const ReverseGeocodeTask* pTask)
{
    const ReverseGeocodeUITaskSuccess* task = static_cast<const ReverseGeocodeUITaskSuccess*>(pTask);
    if (m_listener && IsRequestAlive())
    {
        m_listener->OnSuccess(*this->m_userRequest, *task->m_information);
    }
    delete task->m_information;
}

void ReverseGeocodeRequestPrivate::UI_NotifyError(const ReverseGeocodeTask* pTask)
{
    const ReverseGeocodeUITaskError* task = static_cast<const ReverseGeocodeUITaskError*>(pTask);
    if (m_listener && IsRequestAlive())
    {
        m_listener->OnError(*this->m_userRequest, task->m_error);
    }
}

void ReverseGeocodeRequestPrivate::UI_NotifyProgress(const ReverseGeocodeTask* pTask)
{
    const ReverseGeocodeUITaskProgress* task = static_cast<const ReverseGeocodeUITaskProgress*>(pTask);
    if (m_listener && IsRequestAlive())
    {
        m_listener->OnProgressUpdated(*this->m_userRequest, task->m_percentage);
    }
}

void ReverseGeocodeRequestPrivate::Destroy()
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new ReverseGeocodeTask(this, &ReverseGeocodeRequestPrivate::CCC_Destroy));
    }
}

void ReverseGeocodeRequestPrivate::CCC_Destroy(const ReverseGeocodeTask* pTask)
{
    delete this;
}

//SingleSearchAsyncCallback
ReverseGeocodeAsyncCallback::ReverseGeocodeAsyncCallback(ReverseGeocodeRequestPrivate* requestPrivate)
    : m_requestPrivate(requestPrivate)
{
}

void ReverseGeocodeAsyncCallback::Success(ReverseGeocodeParametersSharedPtr request,
                                          ReverseGeocodeInformationSharedPtr response)
{
    if (m_requestPrivate)
    {
        m_requestPrivate->NotifySuccess(new ReverseGeocodeInformationImpl(response));
    }
}
void ReverseGeocodeAsyncCallback::Error(ReverseGeocodeParametersSharedPtr request, NB_Error error)
{
    if (m_requestPrivate)
    {
        m_requestPrivate->NotifyError(error);
    }
}

bool ReverseGeocodeAsyncCallback::Progress(int percentage)
{
    if (m_requestPrivate)
    {
        m_requestPrivate->NotifyProgress(percentage);
    }
    return true;
}

void ReverseGeocodeAsyncCallback::Reset()
{

}
/*! @} */
