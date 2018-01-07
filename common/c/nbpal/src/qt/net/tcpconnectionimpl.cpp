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

#include "tcpconnectionimpl.h"
#include "paldebuglog.h"
#include <QtCore/QtGlobal>

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QAbstractSocket::SocketError)
Q_DECLARE_METATYPE(QAbstractSocket::SocketState)
#endif

TCPConnectionImpl::TCPConnectionImpl(PAL_NetConnectionProtocol protocol, QObject *parent) : QThread(parent)
{
    m_protocol = protocol;
    m_pSocket = NULL;

    moveToThread(this);
    start();
}

TCPConnectionImpl::~TCPConnectionImpl(void)
{
    DestroySocketConnection();
    delete m_pSocket;
}

void TCPConnectionImpl::ConnectToHost(const QString& hostName, quint16 nPort)
{
    if (m_pSocket->state() == QAbstractSocket::UnconnectedState)
    {
        m_pSocket->connectToHost(hostName, nPort, QIODevice::ReadWrite);
    }
    else
    {
        emit ConnectionStatusChanged(PNCS_Error);
    }
}

void TCPConnectionImpl::SendRequest(QByteArray data)
{
    if (m_pSocket->state() == QAbstractSocket::ConnectedState)
    {
        qint64 result = m_pSocket->write(data);
        if (result == -1)
        {
            emit ConnectionStatusChanged(PNCS_Error);
        }
        else
        {
            emit DataSend(data);
        }
    }
    else
    {
        emit ConnectionStatusChanged(PNCS_Error);
    }
}

void TCPConnectionImpl::run(void)
{
    m_pSocket = new QTcpSocket();

    qRegisterMetaType<QAbstractSocket::SocketError>();
    qRegisterMetaType<QAbstractSocket::SocketState>();
    connect(m_pSocket, SIGNAL(connected()), SLOT(connected()));
    connect(m_pSocket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
    connect(m_pSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(m_pSocket, SIGNAL(readyRead()), SLOT(readyRead()));

    exec();
}

void TCPConnectionImpl::DestroySocketConnection(void)
{
    m_pSocket->close();
}

void TCPConnectionImpl::connected(void)
{
    emit ConnectionStatusChanged(PNCS_Connected);
}

void TCPConnectionImpl::disconnected(void)
{
    DestroySocketConnection();
    emit ConnectionStatusChanged(PNCS_Closed);
}

void TCPConnectionImpl::error(QAbstractSocket::SocketError nErrorCode)
{
    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::error nErrorCode = %d\n", nErrorCode));

    switch (nErrorCode)
    {
        case QAbstractSocket::RemoteHostClosedError:
        {
            // Do nothing here. This error will be handled by disconnected slot
            break;
        }
        default:
        {
            DestroySocketConnection();
            emit ConnectionStatusChanged(PNCS_Failed);
            break;
        }
    }
}

void TCPConnectionImpl::stateChanged(QAbstractSocket::SocketState socketState)
{
    switch(socketState)
    {
        case QAbstractSocket::HostLookupState:
            emit ConnectionStatusChanged(PNCS_Resolving);
            break;
        case QAbstractSocket::ConnectingState:
            emit ConnectionStatusChanged(PNCS_Connecting);
            break;
        default:
            break;
    }
}

void TCPConnectionImpl::readyRead(void)
{
    QByteArray array = m_pSocket->readAll();
    emit DataReceived(array);
}
