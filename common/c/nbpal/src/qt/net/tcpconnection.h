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

  @file  tcpconnection.h
  @date  04/23/2015
  @defgroup PAL_NET PAL Network I/O Functions

  @brief  Class for PAL TCP network connections.

  Qt implementation for PAL TCP network functions.
  */
/*
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
#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QByteArray>
#include "netconnection.h"

class TCPConnectionImpl;

class TcpConnection : public QObject, public nimpal::network::NetConnection
{
    Q_OBJECT

public:
    TcpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal);

public:
    // From nimpal::network::NetConnection
    virtual void Destroy(void);
    virtual PAL_Error Connect(const char* pHostName, uint16 nPort, uint32 nLocalIpAddr=0);
    virtual PAL_Error Send(const byte* pBuffer, int nCount);
    virtual PAL_Error Close(void);
    virtual PAL_Error Send(const byte*, uint32, const char*, const char*, const char*, const char*, void*);

protected:
    virtual ~TcpConnection(void);

protected slots:
    void ConnectionStatusChanged(PAL_NetConnectionStatus newStatus);
    void DataSend(const QByteArray& data);
    void DataReceived(const QByteArray& data);

signals:
    void ConnectToHost(const QString& hostName, quint16 nPort);
    void SendRequest(QByteArray dataToSend);

private:
    TCPConnectionImpl*      m_pConnection;
};

#endif
/*! @} */
