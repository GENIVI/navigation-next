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

    @file     analyticsengineimpl.cpp
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "analyticsengineimpl.h"
#include "analyticsengine.h"
#include "analyticslibxmlparser.h"
#include "nbanalyticsprivate.h"
#include "nbanalyticsprotected.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "FileOperatingTaskManager.h"
#include "pal.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
#include "paltestlog.h"
#include "StringUtility.h"
#include "palclock.h"
#include "cslnetwork.h"
#include "nbmacros.h"
#include <deque>

namespace nbcommon
{

#define ANALYTICS_UPLOAD_TIMER_TIME     1000

#define ANALYTICS_SESSION_REFRESH_TIME  1000

/*! This is the local database max store count,
this is also the max count of one time read out.

calculated by 10MB/2000byte(max 10mb in local storage and per 2000bytes one event) = 5000
*/
#define ANALYTICS_MAX_STORE_COUNT       5000

#define ANALYTICS_UUID_LENGTH           48

/*! Most of analytics events' sizes are 1000-1500 bytes,
so 500 is defined to be the smallest size limitation of events.
*/
#define ANALYTICS_SMALLEST_XML_SIZE     500

#define ANALYTICS_GPS_EVENT_NAME            "gps-probes-event"

#define ANALYTICS_ROUTE_TRACKING_EVENT_NAME "route-state-event"
#define ANALYTICS_ROUTE_TRACKING_NAVIGATING "navigating"
#define ANALYTICS_ROUTE_TRACKING_NEW_ROUTE  "new-route"
#define ANALYTICS_ROUTE_TRACKING_RECALC     "recalc"

struct AnalyticsUserData
{
    AnalyticsEngineImpl*                impl;
    PAL_Instance*                       pal;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread contains analytics event structure and send information*/
struct AnalyticsEventCheckUploadParam
{
    AnalyticsEngineImpl::UploadReason   reason;
    AnalyticsEngineImpl*                impl;
    PAL_Instance*                       pal;
    shared_ptr<int>                     classStatus;
    int64                               oldestNonBillableTimeStamp;
    int64                               oldestBillableTimeStamp;
};

/*! the parameter passed from one thread to another thread contains analytics event structure and send information*/
struct AnalyticsEventSendParam
{
    AnalyticsEngineImpl::UploadReason   reason;
    AnalyticsEngineImpl*                impl;
    PAL_Instance*                       pal;
    std::vector<AnalyticsEvent>         eventsQueue;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread contains analytics event structure information*/
struct AnalyticsEventStructParam
{
    AnalyticsEngineImpl*                impl;
    PAL_Instance*                       pal;
    std::vector<AnalyticsEvent>         eventsQueue;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread contains analytics event string information*/
struct AnalyticsEventStringParam
{
    AnalyticsEngineImpl*                impl;
    std::string                         event;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread when clear/read task is over*/
struct AnalyticsEventFinishTaskParam
{
    AnalyticsEngineImpl*                impl;
    PAL_Instance*                       pal;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread when analytics network callbacks*/
struct AnalyticsCallBackParam
{
    AnalyticsEngineImpl*                impl;
    NB_AnalyticsActivity                activity;
    void*                               userdata;
    shared_ptr<int>                     classStatus;
};

/*! the parameter passed from one thread to another thread when a send request is began*/
struct AnalyticsBeginSendParam
{
    AnalyticsEngineImpl*                impl;
    shared_ptr<int>                     classStatus;
    AnalyticsEngineImpl::UploadReason   reason;
};

/*! the parameter passed from one thread to another thread when a master clear request is began*/
struct AnalyticsBeginClearParam
{
    AnalyticsEngineImpl*                impl;
    shared_ptr<int>                     classStatus;
};

class AnalyticsTask: public Task
{
public:
    enum AnalyticsTaskID
    {
        ATI_CLEAR_TASK,             /*! Clear all events in database*/
        ATI_STORE_TASK,             /*! Store an event in database*/
        ATI_READ_TASK,              /*! Read events from database*/
        ATI_REMOVEOBSOLETE_TASK,    /*! Remove obsolete events to database*/
        ATI_CHECKUPLOAD_TASK        /*! Check Upload conditions*/
    };

    typedef void (*AnalyticsTaskCallBack)(int id, NB_Error taskError, void* param);

public:
    AnalyticsTask():
        m_callback(NULL)
    {}
    ~AnalyticsTask(){}
public:
    void SetTaskFinishCallback(AnalyticsTaskCallBack callback)
    {
        m_callback = callback;
    }
protected:
    AnalyticsTaskCallBack m_callback;
};

/*! This is the task class used to clear all events in persistent storage*/
class ClearTask: public AnalyticsTask
{
public:
    ClearTask(shared_ptr<AnalyticsDBStorage> storage, shared_ptr<int> classStatus,
              AnalyticsEngineImpl* impl, PAL_Instance* pal):
        m_storage(storage),
        m_classStatus(classStatus),
        m_impl(impl),
        m_pal(pal)
    {
    }
    ~ClearTask(){}

public:
    void Execute(void)
    {
        NB_Error error = NE_NOTINIT;
        if(*m_classStatus == AnalyticsEngineImpl::CS_INITIALIZED)
        {
            error = m_storage->ClearTable();
        }
        if(m_callback)
        {
            AnalyticsEventFinishTaskParam* param = CCC_NEW AnalyticsEventFinishTaskParam;
            param->impl = m_impl;
            param->pal = m_pal;
            param->classStatus = m_classStatus;
            m_callback(ATI_CLEAR_TASK, error, param);
        }
        delete this;
    }
private:
    shared_ptr<AnalyticsDBStorage> m_storage;
    shared_ptr<int>                m_classStatus;
    AnalyticsEngineImpl*           m_impl;
    PAL_Instance*                  m_pal;
};

/*! This is the task class used to store events to persistent storage*/
class StoreTask: public AnalyticsTask
{
public:
    StoreTask(shared_ptr<AnalyticsDBStorage> storage, shared_ptr<int> classStatus,
              const AnalyticsEvent& event, const vector<uint32>& sortedCategory, int maxStorageSize):
        m_classStatus(classStatus),
        m_storage(storage),
        m_event(event),
        m_sortedCategory(sortedCategory)
    {
    }
    ~StoreTask()
    {
    }

public:
    void Execute(void)
    {
        NB_Error error = NE_NOTINIT;
        if(*m_classStatus == AnalyticsEngineImpl::CS_INITIALIZED)
        {
            error = NE_OK;
            std::vector<AnalyticsEvent> eventQueue;
            eventQueue.push_back(m_event);
            error = m_storage->AppendEvents(eventQueue);
        }
        if(m_callback)
        {
            m_callback(ATI_STORE_TASK, error, NULL);
        }
        delete this;
    }
private:
    shared_ptr<int>                m_classStatus;
    shared_ptr<AnalyticsDBStorage> m_storage;
    AnalyticsEvent                 m_event;
    std::vector<uint32>            m_sortedCategory;
};

/*! This class is the task class used to delete events that sent successfully.

*/
class RemoveObsoleteTask : public AnalyticsTask
{
public:
    RemoveObsoleteTask(shared_ptr<AnalyticsDBStorage> storage, AnalyticsEventStructParam* param):
        m_param(param),
        m_storage(storage)
    {
    }
    ~RemoveObsoleteTask()
    {
        delete m_param;
    }

public:
    void Execute(void)
    {
        NB_Error error = NE_NOTINIT;
        if(*m_param->classStatus == AnalyticsEngineImpl::CS_INITIALIZED)
        {
            error = NE_OK;
            if(!m_param->eventsQueue.empty())
            {
                error = m_storage->DeleteEvents(m_param->eventsQueue);
            }
        }
        if(m_callback)
        {
            AnalyticsEventFinishTaskParam* param = CCC_NEW AnalyticsEventFinishTaskParam;
            param->pal = m_param->pal;
            param->impl = m_param->impl;
            param->classStatus = m_param->classStatus;
            m_callback(ATI_REMOVEOBSOLETE_TASK, error, param);
        }
        delete this;
    }
private:
    AnalyticsEventStructParam*     m_param;
    shared_ptr<AnalyticsDBStorage> m_storage;
};
class CheckUploadTask: public AnalyticsTask
{
public:
    CheckUploadTask(AnalyticsEngineImpl* impl, PAL_Instance* pal,  uint32 maxAge,
                      shared_ptr<AnalyticsDBStorage> storage, shared_ptr<int> classStatus,
                      AnalyticsEngineImpl::UploadReason reason, const std::vector<uint32>& sortedCategory,
                      const std::vector<uint32>& categoryPriorities):
    m_impl(impl), m_pal(pal), m_storage(storage), m_classStatus(classStatus), m_reason(reason),
    m_maxAge(maxAge), m_sortedCategory(sortedCategory), m_priorities(categoryPriorities)
    {
    }
    ~CheckUploadTask(){}
public:
    void Execute(void)
    {
        NB_Error error = NE_NOTINIT;
        AnalyticsEventCheckUploadParam* param = CCC_NEW AnalyticsEventCheckUploadParam;
        if(*m_classStatus == AnalyticsEngineImpl::CS_INITIALIZED)
        {
            error = NE_OK;

            if (m_reason != AnalyticsEngineImpl::UR_FORCE && m_maxAge != 0)
            {
                error = m_storage->DeleteExpireEvents(PAL_ClockGetGPSTime()-m_maxAge);
            }
            uint32 category = 0;
            for (; category<NB_AEC_MAX && error == NE_OK; ++category)
            {
                if (m_priorities[category] == NB_AEP_Invalid)
                {
                    error = m_storage->DeleteEvents(category);
                }
            }

            param->oldestBillableTimeStamp = m_storage->GetOldestEventTimeStamp(NB_AEC_Gold);
            if (param->oldestBillableTimeStamp < 0)
            {
                error = NE_UNEXPECTED;
            }

            param->oldestNonBillableTimeStamp = 0;
            for (category = 0; category < NB_AEC_MAX && error == NE_OK; ++category)
            {
                if (category == NB_AEC_Gold)
                {
                    continue;
                }

                int64 timeStamp = m_storage->GetOldestEventTimeStamp(category);
                if (timeStamp < 0)
                {
                    error = NE_UNEXPECTED;
                }
                else if(timeStamp > 0)
                {
                    param->oldestNonBillableTimeStamp = (param->oldestNonBillableTimeStamp == 0 ||
                                                         param->oldestNonBillableTimeStamp > timeStamp)?
                                                         timeStamp:param->oldestNonBillableTimeStamp;
                }
            }
        }
        if (m_callback)
        {
            param->pal = m_pal;
            param->reason = m_reason;
            param->impl = m_impl;
            param->classStatus = m_classStatus;
            m_callback(ATI_CHECKUPLOAD_TASK, error, param);
        }
        else
        {
            delete param;
        }
        delete this;
    }
private:
    AnalyticsEngineImpl*               m_impl;
    PAL_Instance*                      m_pal;
    shared_ptr<AnalyticsDBStorage>     m_storage;
    shared_ptr<int>                    m_classStatus;
    AnalyticsEngineImpl::UploadReason  m_reason;
    uint32                             m_maxAge;
    std::vector<uint32>                m_sortedCategory;
    std::vector<uint32>                m_priorities;
};

/*! This is the task class used to read events from database and if conditions fit,
these events will be sent to the server
*/
class ReadTask: public AnalyticsTask
{
public:
    ReadTask(AnalyticsEngineImpl* impl, PAL_Instance* pal,
             shared_ptr<AnalyticsDBStorage> storage, shared_ptr<int> classStatus,
             AnalyticsEngineImpl::UploadReason reason, const std::vector<uint32>& sortedCategory,
             const std::vector<uint32>& categoryPriorities, uint32 maxUploadCount):
        m_impl(impl), m_pal(pal), m_storage(storage), m_classStatus(classStatus),
        m_reason(reason), m_sortedCategory(sortedCategory), m_priorities(categoryPriorities),
        m_maxUploadCount(maxUploadCount)
    {
    }
    ~ReadTask()
    {
    }
public:
    void Execute(void)
    {
        NB_Error error = NE_NOTINIT;
        AnalyticsEventSendParam* param = CCC_NEW AnalyticsEventSendParam;
        if(*m_classStatus == AnalyticsEngineImpl::CS_INITIALIZED)
        {
            error = NE_OK;
            uint32 category = 0;

            bool needClean = false;
            uint32 total = 0;
            for (; category < NB_AEC_MAX && error == NE_OK ; ++category)
            {
                int size = m_storage->EventCount(category);
                if (size < 0)
                {
                    error = NE_UNEXPECTED;
                }
                else
                {
                    total += size;
                }
            }

            if (error == NE_OK && total > ANALYTICS_MAX_STORE_COUNT)
            {
                needClean = true;
            }

            int readLeft = (!m_maxUploadCount || m_maxUploadCount > ANALYTICS_MAX_STORE_COUNT || needClean)?
                            ANALYTICS_MAX_STORE_COUNT:m_maxUploadCount;
            int readCount = 0;
            for (category = 0; category < NB_AEC_MAX && error == NE_OK && (readLeft -= readCount) > 0; ++category)
            {
                uint32 sorted = m_sortedCategory[category];
                if (m_priorities[sorted] == NB_AEP_Invalid)
                {
                    break;
                }
                readCount = 0;
                error = m_storage->ReadEvents(sorted, false, readLeft,
                                              param->eventsQueue, readCount);
            }
            if (error == NE_OK && needClean)
            {
                error = m_storage->ClearTable();
                error = (error == NE_OK)? m_storage->AppendEvents(param->eventsQueue) : error;
            }
        }
        if (m_callback)
        {
            param->impl = m_impl;
            param->classStatus = m_classStatus;
            param->pal = m_pal;
            param->reason = m_reason;
            m_callback(ATI_READ_TASK, error, param);
        }
        else
        {
            delete param;
        }
        delete this;
    }
private:
    AnalyticsEngineImpl*               m_impl;
    PAL_Instance*                      m_pal;
    shared_ptr<AnalyticsDBStorage>     m_storage;
    shared_ptr<int>                    m_classStatus;
    AnalyticsEngineImpl::UploadReason  m_reason;
    std::vector<uint32>                m_sortedCategory;
    std::vector<uint32>                m_priorities;
    uint32                             m_maxUploadCount;
};

}

using namespace nbcommon;

AnalyticsEngineImpl::~AnalyticsEngineImpl()
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        *m_classStatus = CS_DESTRUCTED;
        return;
    }

    *m_classStatus = CS_DESTRUCTED;
    PAL_TimerCancel(m_pal, SessionTimeUp, this);
    PAL_TimerCancel(m_pal, SendTimeUp, this);
    delete static_cast<AnalyticsUserData*>(NB_AnalyticsGetUserData(m_analytics));
    NB_AnalyticsSetUserData(m_analytics, NULL);
    NB_AnalyticsDestroy(m_analytics);
}

NB_Error AnalyticsEngineImpl::SetAnalyticsListener(AnalyticsListener* listener)
{
    if(!listener)
    {
        return NE_INVAL;
    }

    if(!m_sessionId.empty())
    {
        listener->OnSessionCreated("", m_sessionId);
    }
    m_sessionListener = listener;
    return NE_OK;
}

std::string AnalyticsEngineImpl::GetSessionId() const
{
    return m_sessionId;
}

NB_Error AnalyticsEngineImpl::SetDataRoaming(bool bRoaming)
{
    if (*m_classStatus == CS_INITIALIZED)
    {
        return NB_AnalyticsSetDataRoamingStatus(m_context, bRoaming);
    }
    return NE_NOTINIT;
}

const AnalyticsWifiConfig* AnalyticsEngineImpl::GetAnalyticsWifiConfig()
{
    const NB_AnalyticsWifiConfig* config = NULL;
    if (*m_classStatus == CS_INITIALIZED && (config = NB_AnalyticsGetWifiConfig(m_context)) != NULL)
    {
        m_wifiConfig.collection_interval = config->collection_interval;
        m_wifiConfig.max_horizontal_uncertainty = config->max_horizontal_uncertainty;
        m_wifiConfig.max_speed = config->max_speed;
        m_wifiConfig.minimum_distance_delta = config->minimum_distance_delta;
        return &m_wifiConfig;
    }
    return NULL;
}

void AnalyticsEngineImpl::SessionTimeUp(PAL_Instance */*pal*/, void *userData, PAL_TimerCBReason reason)
{
    AnalyticsEngineImpl* pthis = static_cast<AnalyticsEngineImpl*>(userData);
    nsl_assert(pthis);
    if(reason == PTCBR_TimerFired && *pthis->m_classStatus == CS_INITIALIZED)
    {
        pthis->UpdateSession();
        PAL_TimerSet(pthis->m_pal,
                     ANALYTICS_SESSION_REFRESH_TIME,
                     SessionTimeUp,
                     userData);
    }
}

void AnalyticsEngineImpl::UpdateSession()
{
    if (*m_classStatus != CS_INITIALIZED)
    {
        return;
    }

    NB_ASSERT_CCC_THREAD(m_context);
    uint32 maxSessionTime = NB_AnalyticsGetMaxSessionTime(m_analytics);
    if(m_sessionId.empty() || (maxSessionTime && m_refCount == 0 &&
       PAL_ClockGetGPSTime() - m_sessionTime > maxSessionTime))
    {
        m_sessionTime = PAL_ClockGetGPSTime();
        uint8 uuid[ANALYTICS_UUID_LENGTH] = {0};
        uint32 length = 0;
        if(PAL_GenerateUUID(m_pal, uuid, ANALYTICS_UUID_LENGTH, &length) == PAL_Ok)
        {
            std::string oldId = m_sessionId;
            m_sessionId = std::string((char*)uuid, length);
            NB_ContextUpdateActiveSession(m_context, m_sessionId.c_str(), m_sessionId.length());
            if (m_sessionListener)
            {
                m_sessionListener->OnSessionCreated(oldId, m_sessionId);
            }

            std::string strLog = StringUtility::BinaryBufferToString(reinterpret_cast<const unsigned char*>(m_sessionId.c_str()),
                                                                     m_sessionId.length());
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,"AnalyticsLog::New Session ID is (in binary):%s\n", strLog.c_str());
        }

    }
}

void AnalyticsEngineImpl::TaskCallback(int id, NB_Error error, void* param)
{
    switch (id)
    {
    case AnalyticsTask::ATI_CLEAR_TASK:
    {
        AnalyticsEventFinishTaskParam* finishTaskParam = static_cast<AnalyticsEventFinishTaskParam*>(param);
        nsl_assert(finishTaskParam);
        nsl_assert(finishTaskParam->pal);
        TaskId id;
        if(PAL_EventTaskQueueAdd(finishTaskParam->pal, AnalyticsEngineImpl::FinishMasterClearInCCCQueue,
                                 finishTaskParam, &id) != PAL_Ok)
        {
            delete finishTaskParam;
        }
        break;
    }
    case AnalyticsTask::ATI_STORE_TASK:
        break;
    case AnalyticsTask::ATI_READ_TASK:
    {
        AnalyticsEventSendParam* sendParam = static_cast<AnalyticsEventSendParam*>(param);
        nsl_assert(sendParam);
        nsl_assert(sendParam->pal);

        TaskId id;
        if(error == NE_OK)
        {
            if(PAL_EventTaskQueueAdd(sendParam->pal, AnalyticsEngineImpl::SendToServerInCCCQueue,
                                     param, &id) != PAL_Ok)
            {
                delete sendParam;
            }
        }
        else
        {
            AnalyticsEventFinishTaskParam* finishTaskParam = CCC_NEW AnalyticsEventFinishTaskParam;
            finishTaskParam->classStatus = sendParam->classStatus;
            finishTaskParam->pal = sendParam->pal;
            finishTaskParam->impl = sendParam->impl;
            if(PAL_EventTaskQueueAdd(sendParam->pal, AnalyticsEngineImpl::FinishQueryInCCCQueue,
                                     finishTaskParam, &id) != PAL_Ok)
            {
                delete finishTaskParam;
            }
            delete sendParam;
        }

        break;
    }
    case AnalyticsTask::ATI_REMOVEOBSOLETE_TASK:
    {
        AnalyticsEventFinishTaskParam* finishTaskParam = static_cast<AnalyticsEventFinishTaskParam*>(param);
        nsl_assert(finishTaskParam);
        nsl_assert(finishTaskParam->pal);
        TaskId id;
        if(PAL_EventTaskQueueAdd(finishTaskParam->pal, AnalyticsEngineImpl::FinishQueryInCCCQueue,
                                 finishTaskParam, &id) != PAL_Ok)
        {
            delete finishTaskParam;
        }
        break;
    }
    case AnalyticsTask::ATI_CHECKUPLOAD_TASK:
        {
            AnalyticsEventCheckUploadParam* checkParam = static_cast<AnalyticsEventCheckUploadParam*>(param);
            nsl_assert(checkParam);
            nsl_assert(checkParam->pal);
            TaskId id;
            if(error == NE_OK)
            {
                if(PAL_EventTaskQueueAdd(checkParam->pal, AnalyticsEngineImpl::CheckUploadInCCCQueue,
                                         param, &id) != PAL_Ok)
                {
                    delete checkParam;
                }
            }
            else
            {
                AnalyticsEventFinishTaskParam* finishTaskParam = CCC_NEW AnalyticsEventFinishTaskParam;
                finishTaskParam->classStatus = checkParam->classStatus;
                finishTaskParam->pal = checkParam->pal;
                finishTaskParam->impl = checkParam->impl;
                if(PAL_EventTaskQueueAdd(checkParam->pal, AnalyticsEngineImpl::FinishQueryInCCCQueue,
                                         finishTaskParam, &id) != PAL_Ok)
                {
                    delete finishTaskParam;
                }
                delete checkParam;
            }
        }
    default:
        break;
    }

}

NB_Error AnalyticsEngineImpl::StringToTps(const std::string& event, protocol::TpsElementPtr& ptr)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    return AnalyticsLibXmlParser::StringToTps(event, ptr);
}

void AnalyticsEngineImpl::BeginSendQueryInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsBeginSendParam* param = static_cast<AnalyticsBeginSendParam*>(userData);
    nsl_assert(param);

    if (*param->classStatus == CS_INITIALIZED && !param->impl->m_readTaskProcessing &&
        !param->impl->m_clearTaskProcessing)
    {
        param->impl->m_readTaskProcessing = true;

        uint32* categories = NB_AnalyticsGetSortedCategory(param->impl->m_analytics);
        uint32* priorities = NB_AnalyticsGetCategoryPriority(param->impl->m_analytics);
        std::vector<uint32> categoryVector(categories, categories+NB_AEC_MAX);
        std::vector<uint32> priorityVector(priorities, priorities+NB_AEC_MAX);

        AnalyticsTask* task = CCC_NEW CheckUploadTask(param->impl, param->impl->m_pal,
                                                      NB_AnalyticsGetMaxDataAge(param->impl->m_analytics),
                                                      param->impl->m_storage, param->classStatus,
                                                      param->reason, categoryVector, priorityVector);
        task->SetTaskFinishCallback((AnalyticsTask::AnalyticsTaskCallBack)TaskCallback);
        param->impl->m_manager->AddCustomizedTask(task);
    }
    delete param;
}

NB_Error AnalyticsEngineImpl::Upload(UploadReason reason)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    TaskId id;
    AnalyticsBeginSendParam* param = CCC_NEW AnalyticsBeginSendParam;
    param->impl = this;
    param->classStatus = m_classStatus;
    param->reason = reason;
    if(PAL_EventTaskQueueAdd(m_pal, BeginSendQueryInCCCQueue, param, &id) != PAL_Ok)
    {
        delete param;
        return NE_UNEXPECTED;
    }
    return NE_OK;
}

void AnalyticsEngineImpl::BeginClearQueryInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsBeginClearParam* param = static_cast<AnalyticsBeginClearParam*>(userData);
    nsl_assert(param);
    if (*param->classStatus == CS_INITIALIZED && !param->impl->m_clearTaskProcessing)
    {
        param->impl->m_clearTaskProcessing = true;
        AnalyticsTask* task = CCC_NEW ClearTask(param->impl->m_storage, param->classStatus,
                                                param->impl, param->impl->m_pal);
        task->SetTaskFinishCallback((AnalyticsTask::AnalyticsTaskCallBack)TaskCallback);
        param->impl->m_manager->AddCustomizedTask(task);
    }
    delete param;
}

NB_Error AnalyticsEngineImpl::RemoveAllEvents()
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    TaskId id;
    AnalyticsBeginClearParam* param = CCC_NEW AnalyticsBeginClearParam;
    param->impl = this;
    param->classStatus = m_classStatus;
    if(PAL_EventTaskQueueAdd(m_pal, BeginClearQueryInCCCQueue, param, &id) != PAL_Ok)
    {
        delete param;
        return NE_UNEXPECTED;
    }
    return NE_OK;
}

NB_Error AnalyticsEngineImpl::AddEvent(const std::string& event)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    TaskId id;
    AnalyticsEventStringParam* param = CCC_NEW AnalyticsEventStringParam;
    param->impl = this;
    param->classStatus = m_classStatus;
    param->event = event;
    if(PAL_EventTaskQueueAdd(m_pal, SeparateMetadataInCCCQueue, param, &id) != PAL_Ok)
    {
        delete param;
        return NE_UNEXPECTED;
    }
    return NE_OK;
}

NB_Error AnalyticsEngineImpl::Initialize(NB_Context* context, const std::string& dbPath,
                                         const std::string& schema)
{
    if(*m_classStatus == CS_INITIALIZED)
    {
        return NE_UNEXPECTED;
    }

    if(!context)
    {
        return NE_INVAL;
    }
    m_pal = NB_ContextGetPal(context);

    if (!m_pal)
    {
        return NE_UNEXPECTED;
    }

    m_context = context;
    m_parser = shared_ptr<AnalyticsLibXmlParser>(CCC_NEW AnalyticsLibXmlParser);
    m_storage = shared_ptr<AnalyticsDBStorage>(CCC_NEW AnalyticsDBStorage);

    if(!m_storage || !m_parser)
    {
        return NE_UNEXPECTED;
    }

    NB_Error error = m_storage->Initialize(context, dbPath);
    error = (error == NE_OK)?m_parser->Initialize(schema):error;
    if(error != NE_OK)
    {
        return error;
    }

    error = NB_AnalyticsCreate(context, NULL, &m_analytics);
    if(error != NE_OK)
    {
        return error;
    }

    m_manager = nbcommon::FileOperatingTaskManager::GetInstance(m_context);
    if(!m_manager)
    {
        return NE_UNEXPECTED;
    }
    if(PAL_TimerSet(m_pal, ANALYTICS_UPLOAD_TIMER_TIME, SendTimeUp, this) != PAL_Ok)
    {
        return NE_UNEXPECTED;
    }

    if(PAL_TimerSet(m_pal, ANALYTICS_SESSION_REFRESH_TIME, SessionTimeUp, this) != PAL_Ok)
    {
        return NE_UNEXPECTED;
    }

    AnalyticsUserData* data = new AnalyticsUserData;
    data->impl = this;
    data->classStatus = m_classStatus;
    data->pal = m_pal;
    NB_AnalyticsSetUserData(m_analytics, data);

    NB_ContextSetAnalyticsNoOwnershipTransfer(m_context, m_analytics);
    *m_classStatus = CS_INITIALIZED;
    UpdateSession();
    return NE_OK;
}

NB_Error AnalyticsEngineImpl::ValidateString(const std::string& event)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
         return NE_NOTINIT;
    }
    return m_parser->ValidateString(event);
}

void AnalyticsEngineImpl::AnalyticsCallBackInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsCallBackParam* data = static_cast<AnalyticsCallBackParam*>(userData);
    nsl_assert(data);
    switch (data->activity)
    {
    case NB_AA_PiggybackOpportunity:
        {
            if (*data->classStatus == CS_INITIALIZED)
            {
                data->impl->Upload(UR_PIGGYBACK);
            }
            break;
        }
    case NB_AA_QuerySent:
        break;
    case NB_AA_QueryTimeout:
        break;
    case NB_AA_QueryFailure:
        {
            AnalyticsEventStructParam* param = static_cast<AnalyticsEventStructParam*>(data->userdata);
            delete param;
            if(*data->classStatus == CS_INITIALIZED)
            {
                data->impl->m_lastSendFailTime = PAL_ClockGetGPSTime();
                data->impl->m_readTaskProcessing = false;
            }
            break;
        }
    case NB_AA_ReplyReceived:
        {
            AnalyticsEventStructParam* param = static_cast<AnalyticsEventStructParam*>(data->userdata);
            nsl_assert(param);
            if(*data->classStatus == CS_INITIALIZED)
            {
                AnalyticsTask* task = CCC_NEW RemoveObsoleteTask(data->impl->m_storage, param);
                task->SetTaskFinishCallback((AnalyticsTask::AnalyticsTaskCallBack)TaskCallback);
                data->impl->m_manager->AddCustomizedTask(task);
                data->impl->m_lastSendFailTime = 0;
            }
            else
            {
                delete param;
            }
            break;
        }
    case NB_AA_ConfigReceived:
        break;
    default:
        break;
    }
    delete data;
}

void AnalyticsEngineImpl::AnalyticsActivityCallback(NB_Analytics* analytics, void* userData,
                                                    NB_AnalyticsActivity activity)
{
    AnalyticsUserData* data = static_cast<AnalyticsUserData*>(NB_AnalyticsGetUserData(analytics));
    if(data)
    {
        TaskId id;
        AnalyticsCallBackParam* param = CCC_NEW AnalyticsCallBackParam;
        param->activity = activity;
        param->impl = data->impl;
        param->userdata = userData;
        param->classStatus = data->classStatus;
        if(PAL_EventTaskQueueAdd(data->pal, AnalyticsEngineImpl::AnalyticsCallBackInCCCQueue,
                                 param, &id) != PAL_Ok)
        {
            delete param;
        }
    }

}

NB_Error AnalyticsEngineImpl::SendToServer(std::vector<AnalyticsEvent>& eventsQueue,
                                           NB_AnalyticsCallbacks analyticsCallback,
                                           std::vector<AnalyticsEvent>& sendedQueue)
{
    uint32 maxSize = NB_AnalyticsGetMaxEventSize(m_analytics);
    uint32 nowSize = 0;

    CSL_Vector* eventVector = CSL_VectorAlloc(sizeof(tpselt));

    std::deque<AnalyticsEvent> eventQueue;

    if(!eventVector)
    {
        return NE_UNEXPECTED;
    }

    while (!eventsQueue.empty() && (!maxSize || nowSize<maxSize))
    {
        AnalyticsEvent& event = eventsQueue.front();
        if((event.eventBody.length() > maxSize) && maxSize)
        {
            eventsQueue.erase(eventsQueue.begin());
            continue;
        }
        else if (event.eventBody.length()+nowSize < maxSize || !maxSize)
        {
            if (event.name == ANALYTICS_GPS_EVENT_NAME)
            {
                eventQueue.push_back(event);
                nowSize += event.eventBody.length();
            }
            else
            {
                protocol::TpsElementPtr ptr;
                if(StringToTps(event.eventBody, ptr) == NE_OK && ptr->IsValid())
                {
                    tpselt elt = ptr->GetTPSElement();
                    CSL_VectorAppend(eventVector, &elt);
                    nowSize += event.eventBody.length();
                }
            }
            sendedQueue.push_back(event); //Clear this event no matter conversion succeed or not. or no other events can be sent.
            eventsQueue.erase(eventsQueue.begin());
        }
        else
        {
            break;
        }

    }

    NB_Error error = NE_OK;

    if (!eventQueue.empty())
    {
        AnalyticsEventXMLList analyticsEventXMLList;
        while (!eventQueue.empty())
        {
            AnalyticsEvent event(eventQueue.front());
            analyticsEventXMLList.Add(AnalyticsEventXML(event.eventBody));
            eventQueue.pop_front();
        }

        data_analytics_event* events = NULL;
        std::size_t numberOfEvent = 0;
        error = analyticsEventXMLList.ToAnalyticsEventList(NB_ContextGetDataState(m_context), events, numberOfEvent);
        if (error != NE_OK)
        {
            return error;
        }

        for (size_t i = 0; i < numberOfEvent; ++i)
        {
            data_analytics_event* event = events + i;
            tpselt te = data_analytics_event_to_tps(NB_ContextGetDataState(m_context), event);
            if (te != NULL)
            {
                CSL_VectorAppend(eventVector, &te);
            }
        }

        for (size_t i = 0; i < numberOfEvent; ++i)
        {
            DATA_FREE(NULL, events + i, data_analytics_event);
        }
        if(events)
        {
            nsl_free(events);
        }
    }

    error = NB_AnalyticsSetCallbacks(m_context, &analyticsCallback);
    if(error != NE_OK)
    {
        return error;
    }
    error = NB_AnalyticsSendEvents(m_context, eventVector);
    if(error != NE_OK)
    {
        NB_AnalyticsSetCallbacks(m_context, NULL);
    }
    return error;
}

void AnalyticsEngineImpl::SendTimeUp(PAL_Instance */*pal*/, void *userData, PAL_TimerCBReason reason)
{
    AnalyticsEngineImpl* impl = static_cast<AnalyticsEngineImpl*>(userData);
    nsl_assert(impl);
    if(reason == PTCBR_TimerFired && *impl->m_classStatus == CS_INITIALIZED)
    {
        impl->Upload(UR_TIMER);
        PAL_TimerSet(impl->m_pal, ANALYTICS_UPLOAD_TIMER_TIME, SendTimeUp, impl);
    }
}

NB_Error AnalyticsEngineImpl::SeparateMetadata(const std::string& event)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }
    std::string eventBody;
    AnalyticsLibXmlParser::AnalyticsMetadataContent content;
    NB_Error error = AnalyticsLibXmlParser::SeparateMetadata(event, content, eventBody);
    if(error != NE_OK)
    {
        return error;
    }

    if (content.name == ANALYTICS_GPS_EVENT_NAME)
    {
        if (PAL_ClockGetGPSTime() - m_collectLimitation.gpsLastSendTime <= NB_AnalyticsGetGPSSampleRate(m_analytics))
        {
            return NE_INVAL;
        }
        else
        {
            m_collectLimitation.gpsLastSendTime = PAL_ClockGetGPSTime();
        }
    }

    if (content.name == ANALYTICS_ROUTE_TRACKING_EVENT_NAME)
    {
        if(content.navigateState == ANALYTICS_ROUTE_TRACKING_NAVIGATING)
        {
            if (PAL_ClockGetGPSTime() - m_collectLimitation.routeLastSendTime <= NB_AnalyticsGetRouteCollectionInterval(m_analytics))
            {
                return NE_IGNORED;
            }
            else
            {
                m_collectLimitation.routeLastSendTime = PAL_ClockGetGPSTime();
            }
        }
        else if(content.navigateState == ANALYTICS_ROUTE_TRACKING_NEW_ROUTE||
                content.navigateState == ANALYTICS_ROUTE_TRACKING_RECALC)
        {
            m_collectLimitation.routeLastSendTime = 0;
        }
    }

    AnalyticsEvent eventStruct;
    eventStruct.timeStamp = content.timeStamp;
    eventStruct.eventBody = eventBody;
    eventStruct.name = content.name;
    eventStruct.category = (NB_AnalyticsEventsCategory)content.category;

    //If this event is not actually a golden event, it's category need to change.
    if(eventStruct.category == NB_AEC_Gold && !NB_AnalyticsIsGoldenEvent(m_analytics, content.name.c_str()))
    {
        /*! Only poi event can be golden event, so if event name not match server config, turn category to poi*/
        eventStruct.category = NB_AEC_Poi;
    }

    uint32* categories = NB_AnalyticsGetSortedCategory(m_analytics);
    std::vector<uint32> categoryVector(categories, categories+NB_AEC_MAX);
    AnalyticsTask* task = CCC_NEW StoreTask(m_storage, m_classStatus,
                                            eventStruct, categoryVector,
                                            NB_AnalyticsGetMaxDataSize(m_analytics));
    task->SetTaskFinishCallback((AnalyticsTask::AnalyticsTaskCallBack)TaskCallback);
    return m_manager->AddCustomizedTask(task);
}

void AnalyticsEngineImpl::FinishMasterClearInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsEventFinishTaskParam* finishTaskParam = static_cast<AnalyticsEventFinishTaskParam*>(userData);
    nsl_assert(finishTaskParam);
    if(*finishTaskParam->classStatus == CS_INITIALIZED)
    {
        NB_AnalyticsMasterClear(finishTaskParam->impl->m_context);
        finishTaskParam->impl->m_collectLimitation.routeLastSendTime = 0;
        finishTaskParam->impl->m_collectLimitation.gpsLastSendTime = 0;
        finishTaskParam->impl->m_clearTaskProcessing = false;
    }
    delete finishTaskParam;
}

void AnalyticsEngineImpl::FinishQueryInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsEventFinishTaskParam* finishTaskParam = static_cast<AnalyticsEventFinishTaskParam*>(userData);
    nsl_assert(finishTaskParam);
    if(*finishTaskParam->classStatus == CS_INITIALIZED)
    {
        finishTaskParam->impl->m_readTaskProcessing = false;
    }
    delete finishTaskParam;
}

void AnalyticsEngineImpl::SeparateMetadataInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsEventStringParam* stringParam = static_cast<AnalyticsEventStringParam*>(userData);
    nsl_assert(stringParam);
    nsl_assert(stringParam->impl);
    if(*stringParam->classStatus == CS_INITIALIZED && !stringParam->impl->m_clearTaskProcessing)
    {
        stringParam->impl->SeparateMetadata(stringParam->event);
    }
    delete stringParam;
}

bool AnalyticsEngineImpl::UploadAllowed(UploadReason reason, int64 oldestBillableTimeStamp,
                                        int64 oldestNonBillableTimeStamp)
{
    if(reason == UR_TIMER && NB_IsInitialQuerySent(m_analytics))
    {
        /*! The max hold time of all events except billable ones*/
        uint32 maxHoldTime = NB_AnalyticsGetMaxHoldTime(m_analytics);

        /*! The max hold time of billable events*/
        uint32 maxBillableHoldTime = NB_AnalyticsGetMaxBillableHoldTime(m_analytics);

        /*! The interval between oldest non-billable event timeStamp and current time*/
        uint32 billableTime = PAL_ClockGetGPSTime() - (uint32)oldestBillableTimeStamp;

        /*! The interval between oldtest billable event timestamp and current time*/
        uint32 nonBillableTime = PAL_ClockGetGPSTime() - (uint32)oldestNonBillableTimeStamp;

        /*! The interval between last fail upload and current time*/
        uint32 lastFailInterval = PAL_ClockGetGPSTime() - m_lastSendFailTime;
        if (((billableTime > maxBillableHoldTime && maxBillableHoldTime && oldestBillableTimeStamp) || /*! billable match*/
            (nonBillableTime > maxHoldTime && maxHoldTime && oldestNonBillableTimeStamp)) && /*! or non-billable match*/
            lastFailInterval > maxHoldTime) /*! last fail time must bigger than maxHoldTime*/
            /*! @TODO: This is to solve the problem that engine keep uploading when network is down. It follows Android old
            engine logic. But still: if maxHoldTime is 0 and maxBillableHoldTime is non-0, the problem this still there.
            */
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}
void AnalyticsEngineImpl::CheckUploadInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsEventCheckUploadParam* param = static_cast<AnalyticsEventCheckUploadParam*>(userData);
    nsl_assert(param);
    if(*param->classStatus == CS_INITIALIZED)
    {
        if(param->impl->UploadAllowed(param->reason, param->oldestBillableTimeStamp,
                                      param->oldestNonBillableTimeStamp))
        {
            uint32* categories = NB_AnalyticsGetSortedCategory(param->impl->m_analytics);
            uint32* priorities = NB_AnalyticsGetCategoryPriority(param->impl->m_analytics);
            std::vector<uint32> categoryVector(categories, categories+NB_AEC_MAX);
            std::vector<uint32> priorityVector(priorities, priorities+NB_AEC_MAX);
            uint32 maxSize = NB_AnalyticsGetMaxEventSize(param->impl->m_analytics);
            AnalyticsTask* task = CCC_NEW ReadTask(param->impl, param->impl->m_pal,
                                                   param->impl->m_storage, param->classStatus, param->reason,
                                                   categoryVector, priorityVector, maxSize/ANALYTICS_SMALLEST_XML_SIZE);
            task->SetTaskFinishCallback((AnalyticsTask::AnalyticsTaskCallBack)TaskCallback);
            param->impl->m_manager->AddCustomizedTask(task);
        }
        else
        {
            param->impl->m_readTaskProcessing = false;
        }
    }
    delete param;
}

void AnalyticsEngineImpl::SendToServerInCCCQueue(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsEventSendParam* param = static_cast<AnalyticsEventSendParam*>(userData);
    nsl_assert(param);

    if(*param->classStatus == CS_INITIALIZED)
    {
        if(param->eventsQueue.empty() && NB_IsInitialQuerySent(param->impl->m_analytics))
        {
            NB_AnalyticsSetForceUpload(param->impl->m_analytics, FALSE);
            param->impl->m_readTaskProcessing = false;
            delete param;
            return;
        }

            AnalyticsEventStructParam* callbackParam = CCC_NEW AnalyticsEventStructParam;
            callbackParam->impl = param->impl;
            callbackParam->pal = param->impl->m_pal;
            callbackParam->classStatus = param->classStatus;

            NB_AnalyticsCallbacks analyticsCallback;
            analyticsCallback.activityCallback = AnalyticsEngineImpl::AnalyticsActivityCallback;
            analyticsCallback.userData = callbackParam;

            if(param->reason == UR_FORCE)
            {
                NB_AnalyticsSetForceUpload(param->impl->m_analytics, TRUE);
            }

            if(param->impl->SendToServer(param->eventsQueue,
                                         analyticsCallback, callbackParam->eventsQueue) != NE_OK)
            {
                delete callbackParam;
                param->impl->m_readTaskProcessing = false;
            }

    }

    delete param;
}

NB_Error AnalyticsEngineImpl::NoticeAlive()
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    m_sessionTime = PAL_ClockGetGPSTime();
    return NE_OK;
}

NB_Error AnalyticsEngineImpl::KeepAlive(bool alive)
{
    if(*m_classStatus != CS_INITIALIZED)
    {
        return NE_NOTINIT;
    }

    m_sessionTime = PAL_ClockGetGPSTime();
    if (alive)
    {
        ++m_refCount;
    }
    else
    {
        --m_refCount;
    }

    nsl_assert(m_refCount >= 0);
    return NE_OK;
}

/*!This may be called in any thread, so NO CCC-SPECIFIC WORK SHOULD BE PUT HERE!!!*/
AnalyticsEngineImpl::AnalyticsEngineImpl():
    m_context(NULL),
    m_analytics(NULL),
    m_pal(NULL),
    m_readTaskProcessing(false),
    m_clearTaskProcessing(false),
    m_refCount(0),
    m_sessionTime(0),
    m_sessionListener(NULL),
    m_lastSendFailTime(0)
{
    m_classStatus = shared_ptr<int>(CCC_NEW int(CS_CONSTRUCTED));
}


/*!
    @}
*/
