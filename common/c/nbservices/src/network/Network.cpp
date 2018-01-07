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

    @file     Network.cpp
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
extern "C"
{
#include "paltaskqueue.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
}

#include "Network.h"
#include "NetworkRequest.h"
#include "MutableTPSElement.h"
#include "NetworkTask.h"
#include "paldebuglog.h"
#include "nbmacros.h"

using namespace nbmap;

static inline NB_NetworkRequestStatus
NetworkTranslateNetStatus(NB_NetworkResponseStatus status)
{
    NB_NetworkRequestStatus reqstatus = NB_NetworkRequestStatus_Failed;

    switch (status) {

        case COMPLETE:
            reqstatus = NB_NetworkRequestStatus_Success;
            break;
        case CANCELLED:
            reqstatus = NB_NetworkRequestStatus_Canceled;
            break;
        case FAILED:
            reqstatus = NB_NetworkRequestStatus_Failed;
            break;
        case TIMEDOUT:
            reqstatus = NB_NetworkRequestStatus_TimedOut;
            break;
        case RESP_ERROR:
            reqstatus = NB_NetworkRequestStatus_Failed;
            break;
        default:
            break;
    }

    return reqstatus;
}

static inline uint32
NetworkGetReplyErrorCode(NB_NetworkResponse* resp)
{
    const char* code = NULL;
    if (resp->status == RESP_ERROR && resp->reply != NULL) {
        code = te_getattrc(resp->reply, "code");
        return nsl_strtoul((code != NULL) ? code : "0", NULL, 10);
    }
    return 0;
}

/* See description in header file. */
Network::Network(NB_Context* context)
    : m_context(context),
      m_eventQueue()
{
    // Get CCC and UI task queue
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(NB_ContextGetPal(m_context)));

    m_requestList.clear();

    PAL_LockCreate(NB_ContextGetPal(m_context), &m_lock);
}

/* See description in header file. */
Network::~Network()
{
    if (m_callback)
    {
        m_callback.reset();
    }

    CancelPendingRequests();
    if (m_lock)
    {
        PAL_LockDestroy(m_lock);
    }
}

/* See description in header file. */
void Network::Request(shared_ptr<NetworkRequest> request, RequestCallbackPtr callback)
{
    if (!m_eventQueue)
    {
        return;
    }

    m_eventQueue->AddTask(new NetworkTaskRequest(this, &Network::CCC_Request, request, callback));
}

/* See description in header file. */
bool Network::Cancel(uint32 requestID)
{
    if (!m_eventQueue)
    {
        return false;
    }

    m_eventQueue->AddTask(new NetworkTaskCancel(this, &Network::CCC_Cancel, requestID));

    return true;
}

/* See description in header file. */
void Network::CancelPendingRequests()
{
    if (!m_eventQueue)
    {
        return;
    }

    m_eventQueue->AddTask(new NetworkTaskCancelAll(this, &Network::CCC_CancelAll));
}

/* See description in header file. */
void Network::CCC_Request(const NetworkTask* pTask)
{
    const NetworkTaskRequest* task = static_cast<const NetworkTaskRequest*>(pTask);
    if (task)
    {
        if (!task->m_request)
        {
            if (task->m_callback)
            {
                task->m_callback->Error(task->m_request->GetID(), NE_INVAL);
            }
            return;
        }

        RequestQuery* nQuery = new (std::nothrow) RequestQuery();
        if (!nQuery)
        {
            if (task->m_callback)
            {
                task->m_callback->Error(task->m_request->GetID(), NE_NOMEM);
            }
            return;
        }

        m_callback = task->m_callback;

        nQuery->networkQuery.action = &StaticNetworkCallback;
        nQuery->networkQuery.progress = &StaticNetworkProgressCallback;
        nQuery->networkQuery.qflags = TN_QF_DEFLATE;
        nQuery->networkQuery.failFlags = TN_FF_CLOSE_ANY;
        nsl_strcpy(nQuery->networkQuery.target, NB_ContextGetTargetMapping(m_context, task->m_request->GetTarget()->c_str()));
        nQuery->networkQuery.targetlen = -1;

        nQuery->networkQuery.query = task->m_request->GetMutableTpsElement()->GetTPSElement();
        MutableTpsElement* invocationContextTpsElement = task->m_request->GetInvocationContextTpsElement();
        if(invocationContextTpsElement)
        {
            nQuery->networkQuery.invocation = invocationContextTpsElement->GetTPSElement();
        }
        nQuery->networkQuery.wantBinaryChunk = TRUE;
        nQuery->pThis = this;
        nQuery->requestID = task->m_request->GetID();

        nb::Lock lock(m_lock);
        m_requestList.push_back(nQuery);

        CSL_NetworkRequest(NB_ContextGetNetwork(m_context), &nQuery->networkQuery);
    }
}

/* See description in header file. */
void Network::CCC_Cancel(const NetworkTask* pTask)
{
    const NetworkTaskCancel* task = static_cast<const NetworkTaskCancel*>(pTask);
    if (task)
    {
        nb::Lock lock(m_lock);
        list<RequestQuery*>::iterator it = m_requestList.begin();
        for (; it != m_requestList.end(); ++it)
        {
            if ((*it)->requestID == task->m_id)
            {
                CSL_NetworkCancel(NB_ContextGetNetwork(m_context), &((*it)->networkQuery));
                RequestQuery* request = *it;
                m_requestList.erase(it);
                delete request;
                break;
            }
        }
    }
}

/* See description in header file. */
void Network::CCC_CancelAll(const NetworkTask* pTask)
{
    nb::Lock lock(m_lock);
    list<RequestQuery*>::iterator it = m_requestList.begin();
    for (; it != m_requestList.end(); ++it)
    {
        CSL_NetworkCancel(NB_ContextGetNetwork(m_context), &((*it)->networkQuery));
        delete *it;
    }

    m_requestList.clear();
}

bool Network::FindRequest(list<RequestQuery*>& requestList, uint32 requestID)
{
    nb::Lock lock(m_lock);
    bool find = false;
    list<RequestQuery*>::iterator it = requestList.begin();
    for (; it != requestList.end(); ++it)
    {
        if ((*it)->requestID == requestID)
        {
            find = true;
            break;
        }
    }

    return find;
}


bool Network::FindRequestAndRemove(uint32 requestID)
{
    nb::Lock lock(m_lock);
    bool find = false;
    list<RequestQuery*>::iterator it = m_requestList.begin();
    for (; it != m_requestList.end(); ++it)
    {
        if ((*it)->requestID == requestID)
        {
            find = true;
            RequestQuery* request = *it;
            m_requestList.erase(it);
            delete request;
            break;
        }
    }

    return find;
}


/* See description in header file. */
void Network::StaticNetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    if (!query)
    {
        return;
    }

    Network* pThis = NULL;
    uint32 requestID = -1;
    pThis = reinterpret_cast<Network*>((reinterpret_cast<RequestQuery*>(query))->pThis);
    requestID = (reinterpret_cast<RequestQuery*>(query))->requestID;

    if (!pThis)
    {
        return;
    }

    NB_NetworkRequestStatus status   = NB_NetworkRequestStatus_Failed;
    NB_Error                replyerr = NE_OK;

    if (resp)
    {
        replyerr = static_cast<NB_Error>(NetworkGetReplyErrorCode(resp));
        status = NetworkTranslateNetStatus(resp->status);
    }

    if (status == NB_NetworkRequestStatus_Canceled)
    {
        pThis->m_callback->Error(requestID, NE_CANCELLED);
        return;
    }

    if (query)
    {
        query->query = NULL;
    }

    if (!pThis->FindRequestAndRemove(requestID))
    {
        return;
    }

    // Binary data should be returned.
    if (!resp->binaryChunk || !resp->chunkLength)
    {
        status = NB_NetworkRequestStatus_Failed;
    }

    if (pThis->m_callback)
    {
        switch (status)
        {
            case NB_NetworkRequestStatus_Success:
            {
                BinaryDataPtr data(CCC_NEW BinaryData(resp->binaryChunk,
                                                            resp->chunkLength));
                if (data)
                {
                    pThis->m_callback->Success(requestID, data);
                    return;
                }
            }
            case NB_NetworkRequestStatus_Failed:
            {
                replyerr = (replyerr == NE_OK) ? NESERVER_FAILED : replyerr;
                break;
            }
            case NB_NetworkRequestStatus_TimedOut:
            {
                replyerr = NESERVER_TIMEDOUT;
                break;
            }
            default:
            {
                //@todo: Should response canceled and progress.
                return;
            }
        }
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: status = %d, replyerr: %d", __FUNCTION__, (int)status, (int)replyerr);
        pThis->m_callback->Error(requestID, replyerr);
    }
}

/* See description in header file. */
void Network::StaticNetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!query)
    {
        return;
    }

    Network* pThis = NULL;
    pThis = reinterpret_cast<Network*>((reinterpret_cast<RequestQuery*>(query))->pThis);
    uint32 requestID = (reinterpret_cast<RequestQuery*>(query))->requestID;
    if (!pThis)
    {
        return;
    }

    if (!pThis->FindRequest(pThis->m_requestList, requestID))
    {
        return;
    }

    if (pThis->m_callback)
    {
        pThis->m_callback->Progress(requestID, float(cur)/total);
    }
}

/* See description in header file. */
BinaryData::BinaryData(const char* data, uint32 size)
        : m_data(data),
          m_size(size)
{
}

/* See description in header file. */
BinaryData::~BinaryData()
{
}

/* See description in header file. */
const char* BinaryData::GetData() const
{
    return m_data;
}

/* See description in header file. */
uint32 BinaryData::GetSize() const
{
    return m_size;
}

/* See description in header file. */
bool BinaryData::IsValid() const
{
    return (m_data && m_size);
}


/*! @} */
