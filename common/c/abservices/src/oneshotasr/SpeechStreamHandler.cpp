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

 @file     SpeechStreamHandler.cpp
 @date     02/28/2012
 @defgroup AB Speech Stream Handler API

 @brief    AB Speech Stream Handler API

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

/*! @{ */

#include "SpeechStreamHandlerInterface.h"
#include "nbcontextprotected.h"       // To get target mapping
#include "nbutilityprotected.h"
#include "cslnetwork.h"
#include <list>
#include "paltimer.h"
#include <algorithm>

using namespace std;


namespace aboneshotasr
{

static const int32   SMALL_TIMER_DELAY_FOR_ASYNC_CALL =  10;


// Forward declaration for speech stream request object
class SpeechStreamRequest;

/*! Speech stream request listener interface
*/
class SpeechStreamRequestListenerInterface
{
public:
    virtual void SpeechStreamRequestHandleEvent(SpeechStreamRequest* request, NB_NetworkRequestStatus status, NB_Error error) = 0;

protected:
    virtual ~SpeechStreamRequestListenerInterface(void) {}
};


/*! Speech stream request object
*/
class SpeechStreamRequest : public Base
{
public:
    SpeechStreamRequest(NB_Context* context);
    ~SpeechStreamRequest(void);

    NB_Error SendRequest(const SpeechStreamParametersInterface* parameters, SpeechStreamRequestListenerInterface* listener);
    NB_Error CancelRequest(void);
    SpeechStreamInformation* GetReplyInformation(void); // Client become owner of the returned data and should free it by self. NULL is valid value in error is occured case.

protected:
    static void NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
    static void NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);
    void HandleNetworkCallback(NB_NetworkResponse* resp);

    static void AsyncListenerNotify(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);
    void HandleAsyncListenerNotify(void);

    void DestroyReplyInformation(void);

private:
    NB_Context*                              m_context;               /*! NB Context. */
    SpeechStreamRequestListenerInterface*    m_listener;              /*! Speech stream request listener. */
    bool                                     m_inProgress;            /*! Flag to indicate that request in progress. */
    NB_NetworkRequestStatus                  m_replyStatus;           /*! Last reply status code. */
    NB_Error                                 m_replyError;            /*! Last reply error code. */
    SpeechStreamInformation*                 m_replyInformation;      /*! Last reply speech stream information. */

    /*! Useful struct to resolve cast from NB_NetworkQuery to SpeechStreamRequest*. */
    struct NetworkQuery
    {
        NB_NetworkQuery        networkQuery;
        SpeechStreamRequest*   self;
    } m_query;
};

SpeechStreamRequest::SpeechStreamRequest(NB_Context* context) :
    m_context(context),
    m_listener(NULL),
    m_inProgress(FALSE),
    m_replyError(NE_OK),
    m_replyStatus(NB_NetworkRequestStatus_Success),
    m_replyInformation(NULL)
{
    nsl_memset(&m_query, 0, sizeof(NetworkQuery));
}

SpeechStreamRequest::~SpeechStreamRequest(void)
{
    CancelRequest();

    if (m_query.networkQuery.query)
    {
        te_dealloc(m_query.networkQuery.query);
        m_query.networkQuery.query = NULL;
    }

    DestroyReplyInformation();
}

NB_Error
SpeechStreamRequest::SendRequest(const SpeechStreamParametersInterface* parameters, SpeechStreamRequestListenerInterface* listener)
{
    if (!parameters || !listener)
    {
        return NE_BADDATA;
    }

    if (m_inProgress)
    {
        return NE_UNEXPECTED;
    }

    // Extract tps from parameters
    tpselt te = parameters->CreateTPSQuery();
    if (!te)
    {
        return NE_BADDATA;
    }

    // Store listener
    m_listener = listener;

    // Setup network query
    m_query.self = this;
    m_query.networkQuery.action = NetworkCallback;
    m_query.networkQuery.progress = NetworkProgressCallback;
    m_query.networkQuery.qflags = TN_QF_DEFLATE;
    m_query.networkQuery.failFlags = TN_FF_CLOSE_ANY;

    nsl_strlcpy(m_query.networkQuery.target, NB_ContextGetTargetMapping(m_context, parameters->GetTargetName()), sizeof(m_query.networkQuery.target));
    m_query.networkQuery.targetlen = -1;

    // Free previous tpselt if it doesn't
    if (m_query.networkQuery.query)
    {
        te_dealloc(m_query.networkQuery.query);
        m_query.networkQuery.query = NULL;
    }

    // Free previous reply information
    DestroyReplyInformation();

    // QA log query
    parameters->QALogQuery();

    // Send network request with extracted tps
    m_inProgress = TRUE;
    m_query.networkQuery.query = te;
    CSL_NetworkRequest(NB_ContextGetNetwork(m_context), &(m_query.networkQuery));

    return NE_OK;
}

NB_Error
SpeechStreamRequest::CancelRequest(void)
{
    m_listener = NULL;
    PAL_TimerCancel(NB_ContextGetPal(m_context), AsyncListenerNotify, this);

    if (!m_inProgress)
    {
        return NE_OK;
    }

    m_inProgress = FALSE;

    CSL_NetworkCancel(NB_ContextGetNetwork(m_context), &(m_query.networkQuery));
    return NE_OK;
}

SpeechStreamInformation*
SpeechStreamRequest::GetReplyInformation(void)
{
    // Client become owner of the m_replyInformation and will free it by self, so we need just NULL m_replyInformation pointer to avoid memory corruption
    SpeechStreamInformation* information = m_replyInformation;
    m_replyInformation = NULL;
    return information;
}

void
SpeechStreamRequest::NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    if (query)
    {
        SpeechStreamRequest* self = ((struct NetworkQuery*)query)->self;
        if (self)
        {
            self->HandleNetworkCallback(resp);
        }
    }
}

void
SpeechStreamRequest::NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    // Do nothing
}

void
SpeechStreamRequest::HandleNetworkCallback(NB_NetworkResponse* resp)
{
    m_query.networkQuery.query = NULL;

    if (m_inProgress && m_listener && resp)
    {
        // Store reply data
        m_replyError = static_cast<NB_Error>(GetReplyErrorCode(resp));
        m_replyStatus = TranslateNetStatus(resp->status);

        DestroyReplyInformation();
        if (m_replyStatus == NB_NetworkRequestStatus_Success)
        {
            if (resp->reply)
            {
                m_replyInformation = new SpeechStreamInformation(m_context);
                m_replyInformation->ParseTPSReply(resp->reply);
                m_replyInformation->QALogReply();
            }
        }

        m_inProgress = FALSE;

        // Notify client in async way, since client may destroy SpeechStreamRequest object inside notification call
        PAL_TimerSet(NB_ContextGetPal(m_context), SMALL_TIMER_DELAY_FOR_ASYNC_CALL, AsyncListenerNotify, this);
    }
}

void
SpeechStreamRequest::AsyncListenerNotify(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        SpeechStreamRequest* self = static_cast<SpeechStreamRequest*>(userData);
        if (self)
        {
            self->HandleAsyncListenerNotify();
        }
    }
}

void
SpeechStreamRequest::HandleAsyncListenerNotify(void)
{
    if (m_listener)
    {
        // Notify client with network reply. Be sure to make this call last in context, since client can free this object inside call.
        m_listener->SpeechStreamRequestHandleEvent(this, m_replyStatus, m_replyError);
    }
}

void
SpeechStreamRequest::DestroyReplyInformation(void)
{
    if (m_replyInformation)
    {
        delete m_replyInformation;
        m_replyInformation = NULL;
    }
}

/*! Speech stream handler implementation
*/
class SpeechStreamHandler : public SpeechStreamHandlerInterface,
                            public SpeechStreamRequestListenerInterface
{
public:
    SpeechStreamHandler(void);
    virtual ~SpeechStreamHandler(void);

    NB_Error Init(NB_Context* context, SpeechStreamListenerInterface* listener);

    // Overrides SpeechStreamHandlerInterface
    virtual void Release(void);
    virtual NB_Error StartRequest(const SpeechStreamParametersInterface* parameters);
    virtual NB_Error CancelRequest(void);
    virtual NB_Error GetReplyInformation(SpeechStreamInformation** speechStreamInformation);

protected:
    // Overrides SpeechStreamRequestListenerInterface
    virtual void SpeechStreamRequestHandleEvent(SpeechStreamRequest* request, NB_NetworkRequestStatus status, NB_Error error);

    void FreeRequestList(void);
    void DestroyReplyInformation(void);

private:
    NB_Context*                        m_context;                    /*! NB Context. */
    SpeechStreamListenerInterface*     m_listener;                   /*! Speech stream listener. */
    bool                               m_inProgress;                 /*! Flag to indicate that request in progress. */
    list<SpeechStreamRequest*>         m_requestList;                /*! List to hold all active request objects. */
    bool                               m_lastRequestReceived;        /*! Flag to indicate that last request in sequence was received. */
    SpeechStreamInformation*           m_replyInformation;           /*! Last reply speech stream information. */
};

SpeechStreamHandler::SpeechStreamHandler() :
    m_context(NULL),
    m_listener(NULL),
    m_inProgress(FALSE),
    m_lastRequestReceived(FALSE),
    m_replyInformation(NULL)
{
}

SpeechStreamHandler::~SpeechStreamHandler(void)
{
    FreeRequestList();
    DestroyReplyInformation();
}

void
SpeechStreamHandler::Release(void)
{
    CancelRequest();
    delete this;
}

NB_Error
SpeechStreamHandler::Init(NB_Context* context, SpeechStreamListenerInterface* listener)
{
    m_context = context;
    m_listener = listener;

    return NE_OK;
}

NB_Error
SpeechStreamHandler::StartRequest(const SpeechStreamParametersInterface* parameters)
{
    if (!parameters)
    {
        return NE_BADDATA;
    }

    if (m_inProgress)
    {
        // Zero audio sequence must be first. And we should not handle any request if last buffer in audio sequence was already received.
        if ((parameters->GetSequenceID() == 0) || m_lastRequestReceived)
        {
            return NE_BUSY;
        }
    }

    // Remove all previous requests if it doesn't
    if (!m_inProgress)
    {
        FreeRequestList();
    }

    // Free previous reply information if it doesn't
    DestroyReplyInformation();

    // Create request object
    SpeechStreamRequest* request = new SpeechStreamRequest(m_context);

    // Send request
    NB_Error err = request->SendRequest(parameters, this);

    if (err == NE_OK)
    {
        // Store request object to list
        m_requestList.push_back(request);
        request = NULL;

        m_inProgress = TRUE;
        m_lastRequestReceived = (parameters->GetEndSpeech() > 0);
    }
    else
    {
        delete request;
        request = NULL;
    }

    return err;
}

NB_Error
SpeechStreamHandler::CancelRequest(void)
{
    if (!m_inProgress)
    {
        return NE_OK;
    }

    m_inProgress = FALSE;
    FreeRequestList();

    return NE_OK;
}

NB_Error
SpeechStreamHandler::GetReplyInformation(SpeechStreamInformation** speechStreamInformation)
{
    if (!speechStreamInformation)
    {
        return NE_BADDATA;
    }

    if (!m_replyInformation)
    {
        return NE_NOENT;
    }

    // Client become owner of the reply information
    *speechStreamInformation = m_replyInformation;
    m_replyInformation = NULL;

    return NE_OK;
}

void
SpeechStreamHandler::SpeechStreamRequestHandleEvent(SpeechStreamRequest* request, NB_NetworkRequestStatus status, NB_Error error)
{
    if (m_inProgress && request)
    {
        // Looking for request in actual list
        list<SpeechStreamRequest*>::iterator result = find(m_requestList.begin(), m_requestList.end(), request);

        if (result != m_requestList.end()) // Request is valid
        {
            if (error != NE_OK || status == NB_NetworkRequestStatus_Canceled || status == NB_NetworkRequestStatus_Failed || status == NB_NetworkRequestStatus_TimedOut)
            {
                // Handle error case

                NB_Error err = error;
                if (err == NE_OK)
                {
                    switch (status)
                    {
                        case NB_NetworkRequestStatus_Canceled:
                        {
                            err = NE_CANCELLED;
                            break;
                        }

                        case NB_NetworkRequestStatus_Failed:
                        {
                            err = NESERVER_FAILED;
                            break;
                        }

                        case NB_NetworkRequestStatus_TimedOut:
                        {
                            err = NESERVER_TIMEDOUT;
                            break;
                        }

                        default:
                        {
                            err = NESERVER_UNKNOWN;
                            break;
                        }
                    }
                }

                // Request become invalid pointer after CancelRequest call. Let's NULL it to avoid corruption.
                request = NULL;
                CancelRequest();
                DestroyReplyInformation();

                m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_GeneralError, static_cast<uint32>(err));
            }
            else if (status == NB_NetworkRequestStatus_Success)
            {
                // Get reply information
                DestroyReplyInformation();
                m_replyInformation = request->GetReplyInformation();

                // Remove request from the list
                m_requestList.erase(result);
                delete request;
                request = NULL;

                if (!m_replyInformation)
                {
                    if (m_requestList.empty() && m_lastRequestReceived)
                    {
                        CancelRequest();
                        m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_ServerError, static_cast<uint32>(ServerCompletionCode_NoValidResults));
                    }
                }
                else
                {
                    OneShotAsrResults oneShotAsrResults;
                    m_replyInformation->GetResults(&oneShotAsrResults);

                    if (oneShotAsrResults.results.size())
                    {
                        // Server returned some results. No matter what is completion code value. We suppose it's ServerCompletionCode_Success anywhere.
                        CancelRequest();
                        m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_ResultsAvailable, static_cast<uint32>(ServerCompletionCode_Success));

                        // Make information available only for duration of the callback
                        DestroyReplyInformation();
                    }
                    else if (oneShotAsrResults.completionCode == ServerCompletionCode_Success)
                    {
                        if (m_requestList.empty() && m_lastRequestReceived)
                        {
                            // Bad case when there are no more active request and there are still no results from server
                            CancelRequest();
                            m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_ServerError, static_cast<uint32>(ServerCompletionCode_NoValidResults));

                            // Make information available only for duration of the callback
                            DestroyReplyInformation();
                        }
                    }
                    else
                    {
                        // Server returned error
                        CancelRequest();
                        m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_ServerError, static_cast<uint32>(oneShotAsrResults.completionCode));

                        // Make information available only for duration of the callback
                        DestroyReplyInformation();
                    }
                }
            }
            else
            {
                // Remove request from list
                m_requestList.erase(result);
                delete request;
                request = NULL;

                if (m_requestList.empty() && m_lastRequestReceived)
                {
                    m_listener->SpeechStreamHandleEvent(SpeechStreamEvent_GeneralError, static_cast<uint32>(NESERVER_UNKNOWN));
                }
            }
        }
    }
}

void
SpeechStreamHandler::FreeRequestList(void)
{
    if (!m_requestList.empty())
    {
        for (list<SpeechStreamRequest*>::iterator it = m_requestList.begin(); it != m_requestList.end(); it++)
        {
            delete *it;
        }

        m_requestList.clear();
    }

    m_lastRequestReceived = FALSE;
}

void
SpeechStreamHandler::DestroyReplyInformation(void)
{
    if (m_replyInformation)
    {
        delete m_replyInformation;
        m_replyInformation = NULL;
    }
}

NB_Error
CreateSpeechStreamHandler(NB_Context* context,
                          SpeechStreamListenerInterface* listener,
                          SpeechStreamHandlerInterface** speechStreamHandler
                         )
{
    NB_Error err = NE_BADDATA;

    if (!context || !listener || !speechStreamHandler)
    {
        return err;
    }

    SpeechStreamHandler* handler = new SpeechStreamHandler;
    err = handler->Init(context, listener);

    if (err == NE_OK)
    {
        *speechStreamHandler = handler;
        handler = NULL;
    }
    else
    {
        delete handler;
        handler = NULL;
    }

    return err;
}

} //abspeechprocessor

/*! @} */
