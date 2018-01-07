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

    @file     palnet.cpp
    @date     3/15/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    The following functions provide platform-independent network I/O support.

    See header file for function descriptions.
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

/* all the file's code goes here */
#include "palimpl.h"
#include "palnet.h"
#include "palstdlib.h"
#include "paltaskqueue.h"
#include "tcpconnection.h"
#include "httpconnection.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace nimpal::network;

typedef struct PAL_NetConnection
{
    PAL_Instance* pal;
    NetConnection* conn;
} PAL_NetConnection;

PAL_DEF PAL_Error
PAL_NetInitialize(PAL_Instance* pal)
{
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_NetShutdown(PAL_Instance* pal)
{
    return PAL_Ok;
}

static inline NetConnection* GetNetConnection(PAL_NetConnection* pNetConn)
{
    return (pNetConn == NULL) ? NULL : pNetConn->conn;
}

PAL_DEF PAL_Error
PAL_NetCreateConnection(PAL_Instance* pal,
                        PAL_NetConnectionProtocol protocol, PAL_NetConnection** ppNetConn)
{
    if ((pal == NULL) || (ppNetConn == NULL))
    {
        return PAL_ErrBadParam;
    }

    PAL_NetConnection * pNetConn = new PAL_NetConnection();
    if (pNetConn == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_memset(pNetConn, 0, sizeof(*pNetConn));
    pNetConn->pal = pal;
    pNetConn->conn = NULL;

    switch (protocol)
    {
    case PNCP_TCPTLS:
    case PNCP_TCP:
        pNetConn->conn = new TcpConnection(protocol, pal);
        break;
    case PNCP_HTTPS:
    case PNCP_HTTP:
        pNetConn->conn = new HttpConnection(protocol, pal);
        break;
    default:
        delete pNetConn;
        return PAL_ErrBadParam;
    }

    if (pNetConn->conn == NULL)
    {
        delete pNetConn;
        return PAL_ErrNoMem;
    }

    *ppNetConn = pNetConn;
    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_NetOpenConnection(PAL_NetConnection* pNetConn,
                      PAL_NetConnectionConfig* pNetConnConfig,
                      const char* hostName,
                      uint16 port)
{
    if ((pNetConn == NULL) || (pNetConnConfig == NULL))
    {
        return PAL_ErrBadParam;
    }

    if ((hostName == NULL) || (nsl_strlen(hostName) == 0))
    {
        return PAL_ErrBadParam;
    }

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    pConn->SetStatusCallback(pNetConnConfig->netStatusCallback,
                             pNetConnConfig->userData);

    pConn->SetDataSentCallback(pNetConnConfig->netDataSentCallback,
                               pNetConnConfig->userData);

    pConn->SetDataReceivedCallback(pNetConnConfig->netDataReceivedCallback,
                                   pNetConnConfig->userData);

    pConn->SetHttpDataReceivedCallback(pNetConnConfig->netHttpDataReceivedCallback,
                                       pNetConnConfig->userData);

    pConn->SetHttpResponseStatusCallback(
                pNetConnConfig->netHttpResponseStatusCallback,
                pNetConnConfig->userData);

    pConn->SetDnsResultCallback(
                pNetConnConfig->netDnsResultCallback,
                pNetConnConfig->userData);

    pConn->SetHttpResponseHeadersCallback(
                pNetConnConfig->netHttpResponseHeadersCallback,
                pNetConnConfig->userData);

    return pConn->Connect(hostName, port, pNetConn->pal->config.localIpAddr);
}

PAL_DEF PAL_Error
PAL_NetSend(PAL_NetConnection* pNetConn, const byte* buffer,
            uint32 count)
{
    if ((pNetConn == NULL) || (buffer == NULL))
    {
        return PAL_ErrBadParam;
    }

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn == NULL)
    {
        return PAL_ErrBadParam;
    }
    return pConn->Send(buffer, count);
}

PAL_DEF PAL_Error
PAL_NetHttpSend(
        PAL_NetConnection* pNetConn,
        const byte* buffer,
        uint32 count,
        const char* pVerb,
        const char* pObject,
        const char* pAcceptType,
        const char* pAdditionalHeaders,
        void*       pRequestData)
{
    if (pNetConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    HttpConnection* pHttpConn =
            static_cast<HttpConnection*>(GetNetConnection(pNetConn));
    if (pHttpConn == NULL)
    {
        return PAL_ErrBadParam;
    }
    return pHttpConn->Send(buffer, count, pVerb, pObject, pAcceptType, pAdditionalHeaders, pRequestData);
}

PAL_DEF PAL_NetConnectionStatus
PAL_NetGetStatus(PAL_NetConnection *pNetConn)
{
    PAL_NetConnectionStatus status = PNCS_Undefined;

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn != NULL)
    {
        status = pConn->GetStatus();
    }
    return status;
}

PAL_DEF PAL_Error
PAL_NetCloseConnection(PAL_NetConnection* pNetConn)
{
    if (pNetConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    return pConn->Close();
}

static void DestroyNetConnection(void* user_data)
{
    if (user_data)
    {
        PAL_NetConnection* pNetConn = static_cast<PAL_NetConnection*>(user_data);
        NetConnection* pConn = GetNetConnection(pNetConn);
        if(pConn)
        {
            pConn->Destroy();
        }
        if(pNetConn)
        {
            delete pNetConn;
        }
    }
}

static void EventTaskQueueCallback(PAL_Instance* pal, void* userData)
{
    DestroyNetConnection(userData);
}

PAL_DEF PAL_Error
PAL_NetDestroyConnection(PAL_NetConnection* pNetConn)
{
    if (pNetConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn == NULL)
    {
        return PAL_ErrBadParam;
    }

    /* unregister callbacks */
    pConn->SetStatusCallback(NULL, NULL);
    pConn->SetDataSentCallback(NULL, NULL);
    pConn->SetDataReceivedCallback(NULL, NULL);
    pConn->SetHttpResponseStatusCallback(NULL, NULL);
    pConn->SetDnsResultCallback(NULL, NULL);
    pConn->SetHttpResponseHeadersCallback(NULL, NULL);

    if (pConn->GetStatus() != PNCS_Closed)
    {
        pConn->Close();
    }

    // @todo: I add same logic in HTTP connection. I think there is no conflict with here.

    // remove all pending callbacks
    pConn->StopAllCallbacks();

    /* the current function might be called inside a callback */
    if (pConn->IsInCallback())
    {
        TaskId tid = 0;
        PAL_EventTaskQueueAdd(pNetConn->pal, EventTaskQueueCallback, pNetConn, &tid);
    }
    else
    {
        DestroyNetConnection(pNetConn);
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error
PAL_NetGetPppState(PAL_NetConnection* netConn,  PAL_NetPppState* state)
{
    return PAL_ErrUnsupported;
}

PAL_DEF void
PAL_NetRemoveProxyCredentialsCallback(PAL_NetProxyCredentialsCallback* callback)
{
}

PAL_DEF void
PAL_NetAddProxyCredentialsCallback(PAL_NetProxyCredentialsCallback* callback)
{
}

PAL_DEF nb_boolean
PAL_NetSetProxyCredentials(const char* username,
                           const char* password)
{
    return false;
}

PAL_DEF PAL_Error
PAL_NetIsProxyNeeded(const char* hostname)
{
    return PAL_ErrUnsupported;
}

#ifdef __cplusplus
}
#endif
/*! @} */

