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

    @file     httprequest.h
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing a PAL HTTP network request.

    Windows implementation for storing a PAL HTTP network request.
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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QtCore>
#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include "palnet.h"

class HttpRequest : public QObject
{
    Q_OBJECT

public:
    HttpRequest(QObject* pParent = NULL);
    HttpRequest(const HttpRequest& other);
    ~HttpRequest(void);

public:
    HttpRequest& operator=(const HttpRequest& other);

public:
    void SetHostname(const QString& hostname);
    const QString& Hostname(void) const;

    void SetPort(quint16 nPort);
    quint16 Port(void) const;

    void SetAction(const QString& action);
    const QString& Action(void) const;

    void SetPath(const QString& path);
    const QString& Path(void) const;

    void SetAccetpType(const QString& acceptType);
    const QString& AcceptType(void) const;

    void SetAdditionalHeaders(const QString& additionalHeaders);
    void SetAdditionalHeaders(const QList<QPair<QString, QString> >& additionalHeaders);
    const QList<QPair<QString, QString> >& AdditionalHeaders(void) const;

    void SetRequestData(const char* pBytes, quint32 nCount);
    void SetRequestData(const QByteArray& data);
    const QByteArray& RequestData(void) const;

    void SetUserData(void* pUserData);
    void* UserData(void) const;

    void SetHttps(bool b);
    bool isHttps() const;

    QString url() const;

private:
    QString                             m_hostname;
    quint16                             m_nPort;
    QString                             m_action;
    QString                             m_path;
    QString                             m_acceptType;
    QList<QPair<QString, QString> >     m_additionalHeaderList;
    QByteArray                          m_requestData;
    void*                               m_pUserData;
    bool                                m_https;
};

#endif
