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
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Windows implementation for TcpConnection class.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.
    (C) Copyright 2010 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "tcpconnection.h"

namespace nimpal
{
    namespace network
    {
        //////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        //////////////////////////////////////////////////////////////////////////

        TcpConnection::TcpConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol) :
            WSAConnection(pal, protocol)
        {
        }

        //////////////////////////////////////////////////////////////////////////
        // protected functions
        //////////////////////////////////////////////////////////////////////////

        SOCKET TcpConnection::WCSocketCreate()
        {
            SOCKET sock = socket(m_Family, SOCK_STREAM, IPPROTO_TCP);
            if (sock != INVALID_SOCKET)
            {
                if (WSAEventSelect(sock, m_dataEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                {
                    closesocket(sock);
                    sock = INVALID_SOCKET;
                }
            }
            return (sock);
        }

        boolean TcpConnection::WCOnEvent(WSAEVENT ev)
        {
            if (m_Status == PNCS_Connecting)
            {
                OnConnect();
            }
            else
            {
                OnBytesToBeRead();
                OnBytesCanBeWritten();
            }
            return (TRUE);
        }

        //////////////////////////////////////////////////////////////////////////
        // private functions
        //////////////////////////////////////////////////////////////////////////

        void TcpConnection::OnBytesToBeRead()
        {
            char readBuffer[READ_BUFFER_SIZE];
            int count = ReadBytes(readBuffer, sizeof(readBuffer));

            if (count > 0)
            {
                NotifyDataReceived((byte*)readBuffer, count);
            }
        }

        void TcpConnection::OnBytesCanBeWritten()
        {
            Lock();
            {
                int countToWrite = m_SendQ.w - m_SendQ.r;
                if (countToWrite > 0)
                {
                    int countWritten = WriteBytes(m_SendQ.r, countToWrite);
                    if (countWritten > 0)
                    {
                        NotifyDataSent(0, countWritten);

                        // Move read pointer by number of bytes sent and shrink the queue
                        m_SendQ.r += countWritten;
                        if (countWritten > 0)
                        {
                            bqshrink(&m_SendQ);
                        }
                    }

                    // If there is anything left in the queue, set event so we'll get called again
                    if ((m_SendQ.w - m_SendQ.r) > 0)
                    {
                        SetEvent(m_dataEvent);
                    }
                }
            }
            Unlock();
        }

        int TcpConnection::ReadBytes(char* pBuffer, int count)
        {
            int ret = 0;

            ret = recv(m_Sock, pBuffer, count, 0);
            if (ret == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                {
                    ret = 0;
                }
                else
                {
                    OnError(err);
                }
            }

            return ret;
        }

        int TcpConnection::WriteBytes(char* pBuffer, int count)
        {
            int ret = 0;

            ret = send(m_Sock, pBuffer, count, 0);
            if (ret == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                {
                    ret = 0;
                }
                else
                {
                    OnError(err);
                }
            }

            return ret;
        }
    }
}
