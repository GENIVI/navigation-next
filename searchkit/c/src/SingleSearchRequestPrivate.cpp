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
 @file     SingleSearchRequestPrivate.cpp
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


#include "SingleSearchRequestPrivate.h"
#include "nbcontextprotected.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
#include "palevent.h"
#include "pallock.h"
#include <map>
#include "SingleSearchTask.h"
#include "SingleSearchHandlerPrivate.h"
#include "HybridManager.h"
#include "workerqueuemanager.h"
#include "commoncomponentconfiguration.h"

using namespace nbsearch;
using namespace protocol;
using namespace nbmap;

//todo: make a wrapper of the instance pool
typedef map<unsigned int, SingleSearchRequestPrivate*> SingleSearchRequestPrivateInstancePool;
static unsigned int instanceHandleBase = 1;
static SingleSearchRequestPrivateInstancePool instancePool;
static PAL_Lock* instancePoolLock = NULL;


bool SingleSearchRequestPrivate::IsRequestAlive()
{
    bool requestPrivAlive = false;
    {
        PAL_LockLock(instancePoolLock);
        for (SingleSearchRequestPrivateInstancePool::iterator iter = instancePool.begin(); iter != instancePool.end(); iter ++)
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

SingleSearchRequestPrivate* SingleSearchRequestPrivate::NewInstance(NB_Context* context,
                                                                    const SingleSearchRequest* user,
                                                                    const std::shared_ptr<SingleSearchListener>& listener,
                                                                    const std::shared_ptr<SingleSearchParameters>& param)
{
    if (!instancePoolLock)
    {
        PAL_LockCreate(NB_ContextGetPal(context), &instancePoolLock);
    }
    instanceHandleBase ++;

    SingleSearchRequestPrivate* instance = new SingleSearchRequestPrivate(context, user, listener, param);
    {
        PAL_LockLock(instancePoolLock);
        instancePool[instanceHandleBase] = instance;
        PAL_LockUnlock(instancePoolLock);
    }

    return instance;
}

SingleSearchRequestPrivate::SingleSearchRequestPrivate(NB_Context* context,
                                                       const SingleSearchRequest* user,
                                                       const std::shared_ptr<SingleSearchListener>& listener,
                                                       const std::shared_ptr<SingleSearchParameters>& param)
    : m_context(context),
      m_palInstance(NB_ContextGetPal(context)),
      m_inProgress(false),
      m_initSearch(false),
      m_state(new SingleSearchState())
{
    HybridMode mode = HybridManager::GetInstance(context)->GetMode();
    if (mode == HBM_ONBOARD_ONLY || param->onboardOnly()) {
        shared_ptr<WorkerQueueManager> manager = CommonComponentConfiguration::GetWorkerQueueManagerComponent(context);
        if (manager) {
            m_eventQueue = manager->RetrieveCommonWorkerTaskQueue();
        }
    } else {
        m_eventQueue = shared_ptr<TaskQueue>(new EventTaskQueue(m_palInstance));
    }
    // initial search takes the longest, hence is initiated by an "magic" word
    m_initSearch = param->m_name == "ytrebila6";
    if (m_initSearch) {
        // initial search needs to be a category code so only POI database is initialized
        param->setSearchSource(NB_SS_Explore);
        param->setCategoryCode("AE");
    }
    m_handler     = new SingleSearchHandlerPrivate(this, context, user, listener, param, m_state);
}

SingleSearchRequestPrivate::~SingleSearchRequestPrivate()
{
    m_state->setState(false);
    delete m_handler;
    m_handler = NULL;
    m_context = NULL;
    m_palInstance = NULL;
    //remove this pointer from instance pool

    PAL_LockLock(instancePoolLock);
    SingleSearchRequestPrivateInstancePool::iterator iter = instancePool.begin();
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
        //printf("error::%s %p not registered in instance pool\n", __FUNCTION__, this);
    }
    PAL_LockUnlock(instancePoolLock);
}

void SingleSearchRequestPrivate::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    if (m_handler)
    {
        m_handler->SetInvocationContext(inputSource, invocationMethod, screenId);
    }
}

void SingleSearchRequestPrivate::Request()
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new SingleSearchTask(this, &SingleSearchRequestPrivate::CCC_Request));
    }
}

void SingleSearchRequestPrivate::Cancel()
{
    m_state->setState(false);
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new SingleSearchTask(this, &SingleSearchRequestPrivate::CCC_Cancel));
    }
}

bool SingleSearchRequestPrivate::IsRequestInProgress()
{
    m_inProgress = false;
    PAL_Event* event;
    PAL_EventCreate(m_palInstance, &event);

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new SingleSearchTaskCheckInProgress(this, &SingleSearchRequestPrivate::CCC_CheckInProgress, event));
    }

    PAL_EventWaitForEvent(event);
    PAL_EventDestroy(event);
    return m_inProgress;
}

void SingleSearchRequestPrivate::Destroy()
{
    // callbacks need to check SingleSearchState::isValid() before proceed
    m_state->setState(false);

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new SingleSearchTask(this, &SingleSearchRequestPrivate::CCC_Destroy));
    }
}

//private menthods
void SingleSearchRequestPrivate::CCC_Request(const SingleSearchTask* pTask)
{
    m_handler->Request(m_context);
}

void SingleSearchRequestPrivate::CCC_Cancel(const SingleSearchTask* pTask)
{
    m_handler->Cancel();
}

void SingleSearchRequestPrivate::CCC_CheckInProgress(const SingleSearchTask* pTask)
{
    const SingleSearchTaskCheckInProgress* task = static_cast<const SingleSearchTaskCheckInProgress*>(pTask);

    m_inProgress = m_handler->IsInProgress();
    PAL_EventSet(task->m_event);
}


void SingleSearchRequestPrivate::CCC_Destroy(const SingleSearchTask* pTask)
{
    delete this;
}


/*! @} */
