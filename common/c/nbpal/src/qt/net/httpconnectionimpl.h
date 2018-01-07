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

#ifndef HTTPCONNECTIONIMPL_H
#define HTTPCONNECTIONIMPL_H

#include <QString>
#include <QByteArray>
#include <QThread>
#include <QNetworkReply>
#include "palerror.h"
#include "palnet.h"

class HttpRequest;
class QNetworkAccessManager;

class HttpConnectionImpl : public QThread
{
    Q_OBJECT
public:
    explicit HttpConnectionImpl(PAL_NetConnectionProtocol protocol, QObject *parent = 0);
    virtual ~HttpConnectionImpl(void);

public slots:
    void SendRequest(const HttpRequest* pRequest);

signals:
    void HttpConnectionStatusChanged(PAL_NetConnectionStatus newStatus);
    void HttpDataSend(const HttpRequest* pRequest);
    void HttpStatusResponse(const HttpRequest* pRequest, int nStatusCode);
    void HttpHeaderResponse(const HttpRequest* pRequest, const QString& headerFields);
    void HttpDataReceived(const HttpRequest* pRequest, PAL_Error errorCode, const QByteArray* data);

protected:
    virtual void run(void);

protected slots:
    void finished(void);
    void error(QNetworkReply::NetworkError code);
    void sslErrors(const QList<QSslError> &errors);

private:
    PAL_NetConnectionProtocol   m_protocol;
    const HttpRequest*          m_pRequest;
    QNetworkAccessManager*      m_pNetworkAccessManager;
    QNetworkReply*              m_pNetworkReply;
};

#endif // HTTPCONNECTIONIMPL_H
