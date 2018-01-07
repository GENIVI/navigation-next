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

    @file     TcpConnection.cpp
    @date     4/23/2015
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Qt implementation for TcpConnection class.
*/
/*
    See file description in header file.

    (C) Copyright 2015 by TeleCommunication Systems, Inc.

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
#include "tcpconnection.h"
#include "tcpconnectionimpl.h"

Q_DECLARE_METATYPE(PAL_NetConnectionStatus)

TcpConnection::TcpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal)
                : QObject(NULL), NetConnection(protocol, pal)
{
    m_pConnection = new TCPConnectionImpl(protocol);

    qRegisterMetaType<PAL_NetConnectionStatus>();
    connect(m_pConnection, SIGNAL(ConnectionStatusChanged(PAL_NetConnectionStatus)), this, SLOT(ConnectionStatusChanged(PAL_NetConnectionStatus)));
    connect(m_pConnection, SIGNAL(DataSend(QByteArray)), this, SLOT(DataSend(QByteArray)));
    connect(m_pConnection, SIGNAL(DataReceived(const QByteArray&)), this, SLOT(DataReceived(const QByteArray&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(ConnectToHost(QString,quint16)), m_pConnection, SLOT(ConnectToHost(QString,quint16)));
    connect(this, SIGNAL(SendRequest(QByteArray)), m_pConnection, SLOT(SendRequest(QByteArray)));
}

TcpConnection::~TcpConnection(void)
{
    m_pConnection->wait();
    delete m_pConnection;
}

void TcpConnection::Destroy(void)
{
    disconnect(m_pConnection, SIGNAL(ConnectionStatusChanged(PAL_NetConnectionStatus)), this, SLOT(ConnectionStatusChanged(PAL_NetConnectionStatus)));
    disconnect(m_pConnection, SIGNAL(DataSend(QByteArray)), this, SLOT(DataSend(QByteArray)));
    disconnect(m_pConnection, SIGNAL(DataReceived(const QByteArray&)), this, SLOT(DataReceived(const QByteArray&)));
    disconnect(this, SIGNAL(ConnectToHost(QString,quint16)), m_pConnection, SLOT(ConnectToHost(QString,quint16)));
    disconnect(this, SIGNAL(SendRequest(QByteArray)), m_pConnection, SLOT(SendRequest(QByteArray)));

    m_pConnection->quit();
    deleteLater();
}

PAL_Error TcpConnection::Connect(const char* pHostName, uint16 nPort, uint32)
{
    emit ConnectToHost(pHostName, nPort);
    return PAL_Ok;
}

PAL_Error TcpConnection::Send(const byte* pBuffer, int nCount)
{
    QByteArray dataToSend((const char*)pBuffer, nCount);
    emit SendRequest(dataToSend);

    return PAL_Ok;
}

PAL_Error TcpConnection::Close(void)
{
    m_pConnection->quit();
    SetStatus(PNCS_Closed);
    return PAL_Ok;
}

PAL_Error TcpConnection::Send(const byte*, uint32, const char*, const char*, const char*, const char*, void*)
{
    return PAL_ErrUnsupported;
}

// Slots

void TcpConnection::ConnectionStatusChanged(PAL_NetConnectionStatus newStatus)
{
    SetStatus(newStatus);
}

void TcpConnection::DataSend(const QByteArray& data)
{
    NotifyDataSent((const byte*)data.constData(), data.size());
}

void TcpConnection::DataReceived(const QByteArray& data)
{
    NotifyDataReceived((const byte*)(data.data()), data.size());
}

/*! @} */
