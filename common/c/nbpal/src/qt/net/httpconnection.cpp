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

    @file     HttpConnection.cpp
    @date     3/18/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Linux implementation for HttpConnection class.
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
#include "palstdlib.h"
#include "httpconnection.h"

//#define DHTTP
#ifdef DHTTP
#include <QDebug>
#include <pthread.h>
#endif

Q_DECLARE_METATYPE(PAL_Error)
Q_DECLARE_METATYPE(PAL_NetConnectionStatus)

HttpConnection::HttpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal, QObject *parent)
                : QObject(parent), NetConnection(protocol, pal), m_protocol(protocol)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:HC";
#endif

    m_nPort = 0;
    m_pHttpConnectionImpl = new HttpConnectionImpl(protocol, NULL);

    qRegisterMetaType<PAL_Error>();
    qRegisterMetaType<PAL_NetConnectionStatus>();
    connect(m_pHttpConnectionImpl, SIGNAL(HttpConnectionStatusChanged(PAL_NetConnectionStatus)), this, SLOT(HttpConnectionStatusChanged(PAL_NetConnectionStatus)));
    connect(m_pHttpConnectionImpl, SIGNAL(HttpStatusResponse(const HttpRequest*,int)), this, SLOT(HttpStatusResponse(const HttpRequest*,int)));
    connect(m_pHttpConnectionImpl, SIGNAL(HttpHeaderResponse(const HttpRequest*,QString)), this, SLOT(HttpHeaderResponse(const HttpRequest*,QString)));
    connect(m_pHttpConnectionImpl, SIGNAL(HttpDataSend(const HttpRequest*)), this, SLOT(HttpDataSend(const HttpRequest*)));
    connect(m_pHttpConnectionImpl, SIGNAL(HttpDataReceived(const HttpRequest*,PAL_Error,const QByteArray*)), this, SLOT(HttpDataReceived(const HttpRequest*,PAL_Error,const QByteArray*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(SendRequest(const HttpRequest*)), m_pHttpConnectionImpl, SLOT(SendRequest(const HttpRequest*)));

    m_status = PNCS_Initialized;
}

HttpConnection::~HttpConnection(void)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:~HC";
#endif
    m_pHttpConnectionImpl->wait();
    delete m_pHttpConnectionImpl;
}

void HttpConnection::Destroy(void)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:Destroy";
#endif
    disconnect(m_pHttpConnectionImpl, SIGNAL(HttpConnectionStatusChanged(PAL_NetConnectionStatus)), this, SLOT(HttpConnectionStatusChanged(PAL_NetConnectionStatus)));
    disconnect(m_pHttpConnectionImpl, SIGNAL(HttpStatusResponse(const HttpRequest*,int)), this, SLOT(HttpStatusResponse(const HttpRequest*,int)));
    disconnect(m_pHttpConnectionImpl, SIGNAL(HttpHeaderResponse(const HttpRequest*,QString)), this, SLOT(HttpHeaderResponse(const HttpRequest*,QString)));
    disconnect(m_pHttpConnectionImpl, SIGNAL(HttpDataSend(const HttpRequest*)), this, SLOT(HttpDataSend(const HttpRequest*)));
    disconnect(m_pHttpConnectionImpl, SIGNAL(HttpDataReceived(const HttpRequest*,PAL_Error,const QByteArray*)), this, SLOT(HttpDataReceived(const HttpRequest*,PAL_Error,const QByteArray*)));
    disconnect(this, SIGNAL(SendRequest(const HttpRequest*)), m_pHttpConnectionImpl, SLOT(SendRequest(const HttpRequest*)));

    m_pHttpConnectionImpl->quit();
    deleteLater();
}

PAL_Error HttpConnection::Connect(const char* pHostName, uint16 nPort, uint32)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:Connect";
#endif
    m_hostName = pHostName;
    m_nPort = nPort;
    SetStatus(PNCS_Connected);

    return PAL_Ok;
}

PAL_Error HttpConnection::Send(const byte* pBuffer, int nCount)
{
    return Send(pBuffer, nCount, NULL, NULL, NULL, NULL, NULL);
}

PAL_Error HttpConnection::Close(void)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:Close";
#endif
    m_pHttpConnectionImpl->quit();
    SetStatus(PNCS_Closed);
    return PAL_Ok;
}

PAL_Error HttpConnection::Send(const byte* pBuffer, uint32 nCount, const char* pVerb,
                               const char* pObject, const char* pAcceptType,
                               const char* pAdditionalHeaders, void* pRequestData)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:Send";
#endif
    HttpRequest* request = new HttpRequest();
    request->SetHostname(m_hostName);
    request->SetPort(m_nPort);
    request->SetAction(pVerb);
    request->SetPath(pObject);
    request->SetAccetpType(pAcceptType);
    request->SetAdditionalHeaders(pAdditionalHeaders);
    request->SetRequestData((const char*)pBuffer, nCount);
    request->SetUserData(pRequestData);
    request->SetHttps(m_protocol == PNCP_HTTPS);

    {
        QMutexLocker locker(&m_sendQMutex);
        m_requestList.append(request);
    }

    emit SendRequest(request);

    return PAL_Ok;
}

void HttpConnection::HttpConnectionStatusChanged(PAL_NetConnectionStatus newStatus)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:StatusChanged newStatus = " << newStatus;
#endif
    SetStatus(newStatus);
}

void HttpConnection::HttpDataSend(const HttpRequest* pRequest)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:DataSent";
#endif
    const QByteArray& requestData = pRequest->RequestData();
    NotifyDataSent((const byte*)requestData.constData(), requestData.size());
}

void HttpConnection::HttpStatusResponse(const HttpRequest* pRequest, int nStatusCode)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:StatusResponse statusCode = " << nStatusCode;
#endif
    NotifyHttpResponseStatus(pRequest->UserData(), nStatusCode);
}

void HttpConnection::HttpHeaderResponse(const HttpRequest* pRequest, const QString& headerFields)
{
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:HeaderResponse";
#endif
    QByteArray headers = headerFields.toUtf8();
    const char* pSource = headers.constData();
    int nSize = headers.size();

    char* pDest = (char*)nsl_malloc(nSize + 1);
    nsl_memset(pDest, 0, nSize + 1);
    nsl_memcpy(pDest, pSource, nSize);

    NotifyHttpResponseHeaders(pRequest->UserData(), pDest);    // Take the ownership of the pDest.
}

void HttpConnection::HttpDataReceived(const HttpRequest* pRequest, PAL_Error errorCode, const QByteArray* data)
{
    if (data != NULL)
    {
#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:DataReceied  data not Null";
#endif
        NotifyHttpDataReceived(pRequest->UserData(), errorCode, (const byte*)data->constData(), data->size());
    }
    else
    {
        QMutexLocker locker(&m_sendQMutex);

#ifdef DHTTP
    qWarning() << "DHTTP: " << pthread_self() << "HC:DataReceived data is Null" << pRequest->url();
#endif

        // Remove the request from the list
        void* pUserData = pRequest->UserData();
        QList<HttpRequest*>::iterator it = m_requestList.begin();
        for (; it != m_requestList.end(); ++it)
        {
            if (*it == pRequest)
            {
                HttpRequest* request = *it;
                m_requestList.erase(it);
                delete request;
                // The pRequest was invalid now.
                break;
            }
        }

        NotifyHttpDataReceived(pUserData, errorCode, NULL, 0);

        // Send next request
        if (!m_requestList.empty())
        {
#ifdef DHTTP
            HttpRequest *request = m_requestList.first();
    qWarning() << "DHTTP: " << pthread_self() << "HC:DataReceived and m_RequestList not empty. Queue length = " << m_requestList.size() << request->url();
#endif
            // this will cause a crash due to race condition.  the request in the queue might be deleted
            // after the signal is sent.
            // there is no need to resend here, since the upper layer will do the resend
            // emit SendRequest(m_requestList.first());
        }
    }
}

/*! @} */
