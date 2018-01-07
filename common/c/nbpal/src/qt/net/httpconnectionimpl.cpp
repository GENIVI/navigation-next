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

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QDebug>
#include "httpconnectionimpl.h"
#include "httprequest.h"

#define DEBUG_LOG_ALL
#include "paldebuglog.h"

//#define DTHREAD
//#define DHTTPI
#if defined(DHTTPI) || defined(DTHREAD)
#include <QDebug>
#include <pthread.h>
#endif

HttpConnectionImpl::HttpConnectionImpl(PAL_NetConnectionProtocol protocol, QObject *parent) : QThread(parent)
{
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:HCI";
#endif
    m_protocol = protocol;
    m_pRequest = NULL;
    m_pNetworkAccessManager = NULL;
    m_pNetworkReply = NULL;

    moveToThread(this);
    start();
}

HttpConnectionImpl::~HttpConnectionImpl(void)
{
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:~HCI";
#endif
    delete m_pNetworkReply;
    delete m_pNetworkAccessManager;
}

void HttpConnectionImpl::SendRequest(const HttpRequest* pRequest)
{
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:SendRequest";
#endif
    if (m_pNetworkReply != NULL)
    {
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:SendRequest in progress:" << pRequest->url();
#endif
        // Request in progress.
        emit HttpDataReceived(pRequest, PAL_ErrNetInProgress, NULL);
        return;
    }

    m_pRequest = pRequest;

    if (m_pNetworkAccessManager == NULL)
    {
        m_pNetworkAccessManager = new QNetworkAccessManager(this);
    }

#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "Send : " << pRequest->url();
#endif

    QUrl url(pRequest->url());
    QNetworkRequest networkRequest(url);
    if (!m_pRequest->AcceptType().isNull())
    {
        networkRequest.setRawHeader("Accept", m_pRequest->AcceptType().toUtf8());
    }

    if (!m_pRequest->AdditionalHeaders().isEmpty())
    {
        const QList<QPair<QString, QString> >& headerList = m_pRequest->AdditionalHeaders();
        QList<QPair<QString, QString> >::const_iterator it = headerList.begin();
        for (; it != headerList.end(); ++it)
        {
            networkRequest.setRawHeader((*it).first.toUtf8(), (*it).second.toUtf8());
        }
    }

    QString verb(m_pRequest->Action().toUpper());
    if (verb.compare("GET") == 0)
    {
        m_pNetworkReply = m_pNetworkAccessManager->get(networkRequest);
    }
    else if (verb.compare("POST") == 0)
    {
        m_pNetworkReply = m_pNetworkAccessManager->post(networkRequest, m_pRequest->RequestData());
    }

    if (m_pNetworkReply != NULL)
    {
        connect(m_pNetworkReply, SIGNAL(finished()), this, SLOT(finished()));
        connect(m_pNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
        connect(m_pNetworkReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

        emit HttpDataSend(m_pRequest);
    }
    else
    {
        emit HttpConnectionStatusChanged(PNCS_Failed);
    }
}

void HttpConnectionImpl::run(void)
{
#ifdef DTHREAD
    qWarning() << "DTHREAD id:" << pthread_self() << "name: HttpConnectionImpl";
#endif
    exec();
}

void HttpConnectionImpl::finished(void)
{
    if (m_pNetworkReply->error() == QNetworkReply::NoError)
    {
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:finished no error";
#endif
        // Response HTTP status code.
        QVariant statusCode = m_pNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        emit HttpStatusResponse(m_pRequest, statusCode.toInt());

        // Response HTTP header fields.
        QString headerFields;
        const QList<QNetworkReply::RawHeaderPair>& rawHeaderPair = m_pNetworkReply->rawHeaderPairs();
        QList<QNetworkReply::RawHeaderPair>::const_iterator it = rawHeaderPair.begin();
        for (; it != rawHeaderPair.end(); ++it)
        {
            const QNetworkReply::RawHeaderPair& pair = *it;
            headerFields += QString("%1: %2\r\n").arg(pair.first.data()).arg(pair.second.data());
        }
        headerFields += "\r\n\r\n";
        emit HttpHeaderResponse(m_pRequest, headerFields);

        // Response HTTP received data.
        QNetworkReply* pReply = m_pNetworkReply;
        m_pNetworkReply = NULL;

        QByteArray received = pReply->readAll();
        emit HttpDataReceived(m_pRequest, PAL_Ok, &received);
        emit HttpDataReceived(m_pRequest, PAL_Ok, NULL);

        // Release
        pReply->deleteLater();
    }
    else
    {
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:finished ERROR";
#endif
        // Response HTTP received data.
        QNetworkReply* pReply = m_pNetworkReply;
        m_pNetworkReply = NULL;

        emit HttpDataReceived(m_pRequest, PAL_ErrNetGeneralFailure, NULL);

        // Release
        pReply->deleteLater();
    }
}

void HttpConnectionImpl::error(QNetworkReply::NetworkError /*code*/)
{
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:error";
#endif
}

void HttpConnectionImpl::sslErrors(const QList<QSslError> &errors)
{
#ifdef DHTTPI
    qWarning() << "DHTTPI: " << pthread_self() << "HCI:sslErrors";
#endif
    m_pNetworkReply->ignoreSslErrors(errors);
}
