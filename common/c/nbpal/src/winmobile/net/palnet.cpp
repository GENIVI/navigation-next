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
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    The following functions provide platform-independent network I/O support.

    See header file for function descriptions.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "palnet.h"

#include "tcpconnection.h"
#include "tlsconnection.h"
#include "httpconnection.h"
#include "connectionmanager.h"
#include "paldebug.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace nimpal::network;

/*! Internal implementation struct not visible externally; for PAL_NetConnection typedef. */
typedef struct PAL_NetConnection
{
    NetConnection* conn;
} PAL_NetConnection;

/*! Returns pointer to internal implementation socket class for given PAL network connection object.

    This function will return a pointer to the internal implementation socket class for the given
    PAL network connection object.

    @return pointer to instance of associated socket class, NULL if none exists
*/
static NetConnection*
GetNetConnection(PAL_NetConnection* pNetConn)
{
    return pNetConn == NULL ? NULL : pNetConn->conn;
}

PAL_Error
PAL_NetInitialize(PAL_Instance* pal)
{
    boolean success = TRUE;
#ifdef WINCE
    success = FALSE;
    if (ConnectionManager::InitializeConnectionManager())
    {
        if (ConnectionManager::EstablishConnection())
        {
            success = TRUE;
        }
    }
#endif
    if (success)
    {
        success = WSAConnection::InitializeWindowsSockets();
    }
    return success ? PAL_Ok : PAL_ErrNetGeneralFailure;
}

PAL_Error
PAL_NetShutdown(PAL_Instance* pal)
{
    WSAConnection::CleanupWindowsSockets();
    return PAL_Ok;
}

PAL_Error
PAL_NetCreateConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol, PAL_NetConnection** ppNetConn)
{
    PAL_Error err = PAL_ErrBadParam;

    if (pal != NULL && ppNetConn != NULL)
    {
        PAL_NetConnection* pNetConn = new PAL_NetConnection;
        if (pNetConn != NULL)
        {
            switch (protocol)
            {
                case PNCP_TCP:
                    pNetConn->conn = new TcpConnection(pal, protocol);
                    if (pNetConn->conn == NULL)
                    {
                        err = PAL_ErrNoMem;
                    }
                    break;
                case PNCP_TCPTLS:
#ifdef WINCE
                    // class TLSConnection is implemented for WM platform only.
                    pNetConn->conn = new TLSConnection(pal, protocol);
                    if (pNetConn->conn == NULL)
                    {
                        err = PAL_ErrNoMem;
                    }
#else // Win32
                    pNetConn->conn = NULL;
                    err = PAL_ErrUnsupported;
#endif
                    break;
                case PNCP_HTTP:
                case PNCP_HTTPS:
                    pNetConn->conn = new HttpConnection(pal, protocol);
                    if (pNetConn->conn == NULL)
                    {
                        err = PAL_ErrNoMem;
                    }
                    break;
                default:
                    err = PAL_ErrBadParam;
            }

            if (pNetConn->conn != NULL)
            {
                if (pNetConn->conn->InitializeConnection() == FALSE)
                {
                    pNetConn->conn->Release();
                    pNetConn->conn = NULL;
                    err = PAL_ErrNetGeneralFailure;
                }
            }

            if (pNetConn->conn != NULL)
            {
                *ppNetConn = pNetConn;
                err = PAL_Ok;
            }
            else
            {
                delete pNetConn;
                *ppNetConn = NULL;
            }
        }
        else
        {
            *ppNetConn = NULL;
            err = PAL_ErrNoMem;
        }
    }
    return err;
}

PAL_Error
PAL_NetOpenConnection(PAL_NetConnection* pNetConn, PAL_NetConnectionConfig* pNetConnConfig, const char* hostName, uint16 port)
{
    PAL_Error err = PAL_ErrBadParam;

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn != NULL && pNetConnConfig != NULL)
    {
        pConn->SetStatusCallback(pNetConnConfig->netStatusCallback,
                                 pNetConnConfig->userData);
        pConn->SetDataSentCallback(pNetConnConfig->netDataSentCallback,
                                   pNetConnConfig->userData);
        pConn->SetDataReceivedCallback(pNetConnConfig->netDataReceivedCallback,
                                       pNetConnConfig->userData);

        pConn->SetDnsResultCallback(pNetConnConfig->netDnsResultCallback,
                                    pNetConnConfig->userData);

        // Followings are HTTP only callbacks.

        pConn->SetHttpResponseStatusCallback(pNetConnConfig->netHttpResponseStatusCallback,
                                             pNetConnConfig->userData);

        pConn->SetHttpDataReceivedCallback(pNetConnConfig->netHttpDataReceivedCallback,
                                           pNetConnConfig->userData);

        pConn->SetHttpResponseHeadersCallback(pNetConnConfig->netHttpResponseHeadersCallback,
                                              pNetConnConfig->userData);

        pConn->SetTLSConfig(&pNetConnConfig->tlsConfig);

        if (pConn->Connect(hostName, port))
        {
            err = PAL_Ok;
        }

    }
    return err;
}

PAL_Error
PAL_NetSend(PAL_NetConnection* pNetConn, const byte* buffer, uint32 count)
{
    PAL_Error err = PAL_ErrNetGeneralFailure;

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn != NULL && pConn->Send(buffer, count))
    {
        err = PAL_Ok;
    }
    return err;
}

PAL_Error
PAL_NetHttpSend(
    PAL_NetConnection* pNetConn,
    const byte* buffer,
    uint32 count,
    const char* pVerb,
    const char* pObject,
    const char* pAcceptType,
    const char* pAdditionalHeaders,
    void*       requestData)
{
    PAL_Error err = PAL_ErrBadParam;
    HttpConnection* pConn = (HttpConnection*)GetNetConnection(pNetConn);

    if (pConn != NULL)
    {
        if (pConn->Send(buffer, count, pVerb, pObject, pAcceptType, pAdditionalHeaders, requestData))
        {
            err = PAL_Ok;
        }
    }
    return err;
}

PAL_NetConnectionStatus
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

PAL_Error
PAL_NetCloseConnection(PAL_NetConnection* pNetConn)
{
    PAL_Error err = PAL_ErrBadParam;

    NetConnection* pConn = GetNetConnection(pNetConn);
    if (pConn != NULL && pConn->Close())
    {
        err = PAL_Ok;
    }
    return err;
}

PAL_Error
PAL_NetDestroyConnection(PAL_NetConnection* pNetConn)
{
    PAL_Error err = PAL_ErrBadParam;

    if (pNetConn != NULL)
    {
        if (pNetConn->conn != NULL)
        {

            pNetConn->conn->SetStatusCallback(NULL, NULL);
            pNetConn->conn->SetDataSentCallback(NULL, NULL);
            pNetConn->conn->SetDataReceivedCallback(NULL, NULL);
            pNetConn->conn->SetDnsResultCallback(NULL, NULL);
            pNetConn->conn->SetHttpDataReceivedCallback(NULL, NULL);
            pNetConn->conn->SetHttpResponseHeadersCallback(NULL, NULL);
            pNetConn->conn->SetHttpResponseStatusCallback(NULL, NULL);

            if (pNetConn->conn->GetStatus() != PNCS_Closed)
            {
                pNetConn->conn->Close();
            }
            pNetConn->conn->Release();
            pNetConn->conn = NULL;
        }
        delete pNetConn;
        err = PAL_Ok;
    }
    return err;
}

PAL_Error
PAL_NetGetPppState(PAL_NetConnection* netConn,  PAL_NetPppState* state)
{
    return PAL_ErrUnsupported;
}

#ifdef __cplusplus
 }
#endif
