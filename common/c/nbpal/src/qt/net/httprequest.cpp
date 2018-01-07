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

    @file     httprequest.cpp
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

#include <string>
#include <palstdlib.h>
#include <QStringList>
#include "httprequest.h"

HttpRequest::HttpRequest(QObject* pParent) : QObject(pParent), m_https(false)
{
}

HttpRequest::HttpRequest(const HttpRequest& other) : QObject(NULL), m_https(false)
{
    *this = other;
}

HttpRequest::~HttpRequest(void)
{
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
    SetHostname(other.Hostname());
    SetPort(other.Port());
    SetAction(other.Action());
    SetPath(other.Path());
    SetAccetpType(other.AcceptType());
    SetAdditionalHeaders(other.AdditionalHeaders());
    SetRequestData(other.RequestData());
    SetUserData(other.UserData());
    SetHttps(other.isHttps());

    return *this;
}

void HttpRequest::SetHostname(const QString& hostname)
{
    m_hostname = hostname;
}

const QString& HttpRequest::Hostname(void) const
{
    return m_hostname;
}

void HttpRequest::SetPort(quint16 nPort)
{
    m_nPort = nPort;
}

quint16 HttpRequest::Port(void) const
{
    return m_nPort;
}

void HttpRequest::SetAction(const QString& action)
{
    m_action = action;
}

const QString& HttpRequest::Action(void) const
{
    return m_action;
}

void HttpRequest::SetPath(const QString& path)
{
    m_path = path;
}

const QString& HttpRequest::Path(void) const
{
    return m_path;
}

void HttpRequest::SetAccetpType(const QString& acceptType)
{
    m_acceptType = acceptType;
}

const QString& HttpRequest::AcceptType(void) const
{
    return m_acceptType;
}

void HttpRequest::SetAdditionalHeaders(const QString& additionalHeaders)
{
    if (!additionalHeaders.isEmpty())
    {
        QStringList list = additionalHeaders.split("\r\n");
        QStringList::const_iterator it = list.begin();
        for (; it != list.end(); ++it)
        {
            QString header = *it;
            QStringList list = header.split(": ");
            if (list.size() == 2)
            {
                QString& key = list[0];
                QString& value = list[1];
                m_additionalHeaderList.append(QPair<QString, QString>(key, value));
            }
        }
    }
}

void HttpRequest::SetAdditionalHeaders(const QList<QPair<QString, QString> >& additionalHeaders)
{
    m_additionalHeaderList = additionalHeaders;
}

const QList<QPair<QString, QString> >& HttpRequest::AdditionalHeaders(void) const
{
    return m_additionalHeaderList;
}

void HttpRequest::SetRequestData(const char* pBytes, quint32 nCount)
{
    // TODO: Should copy it.
    m_requestData.setRawData(pBytes, nCount);
}

void HttpRequest::SetRequestData(const QByteArray& data)
{
    m_requestData = data;
}

const QByteArray& HttpRequest::RequestData(void) const
{
    return m_requestData;
}

void HttpRequest::SetUserData(void* pUserData)
{
    m_pUserData = pUserData;
}

void* HttpRequest::UserData(void) const
{
    return m_pUserData;
}

void HttpRequest::SetHttps(bool b)
{
    m_https = b;
}

bool HttpRequest::isHttps() const
{
    return m_https;
}

QString HttpRequest::url() const
{
    QString url = QString("%1://%2:%3%4").arg(m_https ? "https" : "hhtp", m_hostname, QString::number(m_nPort), m_path);
    return url;
}

/*! @} */
