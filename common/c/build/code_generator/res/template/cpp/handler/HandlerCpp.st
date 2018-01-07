HandlerCpp(handler) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     Tps<handler.name>Handler.cpp
    @defgroup protocol

*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

extern "C"
{
    #include "nbcontextprotected.h"
    #include "nbutilityprotected.h"
}
#include "Tps<handler.name>Handler.h"
#include "NBProtocol<handler.request.type.name>Serializer.h"
#include "NBProtocol<handler.response.type.name>Serializer.h"
#include "NBProtocolInvocationContext.h"
#include "NBProtocolInvocationContextSerializer.h"

using namespace protocol;


OffBoard<handler.name>Handler::OffBoard<handler.name>Handler(NB_Context* context):
                                              m_context(context)
{
    nsl_memset(&m_Query, 0, sizeof(m_Query));

    m_Query.networkQuery.action = &StaticNetworkCallback;
    m_Query.networkQuery.progress = &StaticNetworkProgressCallback;
    m_Query.networkQuery.qflags = TN_QF_DEFLATE;
    m_Query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strcpy(m_Query.networkQuery.target, NB_ContextGetTargetMapping(m_context, "<handler.request.type.targetName>"));
    m_Query.networkQuery.targetlen = -1;
    m_Query.pThis = this;
    m_inProgress = false;
    m_Query.networkQuery.invocation = NULL;
}

OffBoard<handler.name>Handler::~OffBoard<handler.name>Handler()
{
    if (m_Query.networkQuery.query)
    {
        te_dealloc(m_Query.networkQuery.query);
    }

    if (m_Query.networkQuery.invocation)
    {
        te_dealloc(m_Query.networkQuery.invocation);
    }
}

void OffBoard<handler.name>Handler::Request(shared_ptr\<<handler.request.type.name> > pParameters, <handler.name>CallbackPtr pCallback)
{
    TpsElementPtr tpsQueryPtr;
    if (!pCallback)
    {
        return;
    }
    m_callback = pCallback;

    if (!pParameters)
    {
        m_callback->Error(pParameters, NE_INVAL);
        return;
    }
    m_parameters = pParameters;

    tpsQueryPtr = <handler.request.type.name>Serializer::serialize(pParameters);
    if (!tpsQueryPtr)
    {
        m_callback->Error(m_parameters, NE_NOMEM);
        return;
    }
    if (!tpsQueryPtr->IsValid())
    {
        m_callback->Error(m_parameters, NE_INVAL);
        return;
    }
    m_Query.networkQuery.query = tpsQueryPtr->GetTPSElement();
    m_inProgress = true;
    CSL_NetworkRequest(NB_ContextGetNetwork(m_context), &m_Query.networkQuery);
}

bool OffBoard<handler.name>Handler::SetInvocationContext(InvocationContextSharedPtr invocationContext)
{
    bool success = false;
    if (invocationContext)
    {
        TpsElementPtr tpsInvocationPtr = InvocationContextSerializer::serialize(invocationContext);
        if (!tpsInvocationPtr)
        {
            return success;
        }

        if (!tpsInvocationPtr->IsValid())
        {
            return success;
        }

        m_Query.networkQuery.invocation = tpsInvocationPtr->GetTPSElement();
        success = true;
    }

    return success;
}

void OffBoard<handler.name>Handler::StaticNetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    if (!query)
    {
        return;
    }

    OffBoard<handler.name>Handler* pThis = NULL;
    pThis = reinterpret_cast\<OffBoard<handler.name>Handler*>((reinterpret_cast\<<handler.name>HandlerQuery*>(query))->pThis);

    if (!pThis)
    {
        return;
    }

    pThis->NetworkCallback(query, resp);
}

void OffBoard<handler.name>Handler::NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;
    NB_Error replyerr = NE_INVAL;
    m_inProgress = false;
    if (query)
    {
        query->query = NULL;
        query->invocation = NULL;
    }

    if (resp)
    {
        replyerr = static_cast\<NB_Error>(GetReplyErrorCode(resp));
        status = TranslateNetStatus(resp->status);
    }

    if (m_callback)
    {
        switch (status)
        {
            case NB_NetworkRequestStatus_Success:
            {
                TpsElementPtr tpsQueryPtr(new TpsElement(resp->reply));

                if (tpsQueryPtr && tpsQueryPtr->IsValid() && (replyerr == NE_OK))
                {
                    m_callback->Success(m_parameters, <handler.response.type.name>Serializer::deserialize(tpsQueryPtr));
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

        m_callback->Error(m_parameters, replyerr);
    }

}

void OffBoard<handler.name>Handler::StaticNetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!query)
    {
        return;
    }

    OffBoard<handler.name>Handler* pThis = NULL;
    pThis = reinterpret_cast\<OffBoard<handler.name>Handler*>((reinterpret_cast\<<handler.name>HandlerQuery*>(query))->pThis);

    if (!pThis)
    {
        return;
    }

    if (pThis->m_callback)
    {
        int percent = (cur * 100) / total;
        pThis->m_callback->Progress(percent);
    }

}

void OffBoard<handler.name>Handler::Cancel(void)
{
     if (m_inProgress == false)
     {
        return;
     }
         m_inProgress = false;
     CSL_NetworkCancel(NB_ContextGetNetwork(m_context), &m_Query.networkQuery);
}

bool OffBoard<handler.name>Handler::IsInProgress()
{
    return m_inProgress;
}

/*! @} */
>>
