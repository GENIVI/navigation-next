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

    @file     wsaconnection.cpp
    @date     2010/06/10
    @defgroup PAL_NET PAL Network I/O Functions

    Intermediate support class for protocols based on Windows Mobile
    WSA sockets
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.
    (C) Copyright 2010 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/


#include "wsaconnection.h"


#include <ws2tcpip.h>

namespace nimpal
{
    namespace network
    {
        #define MAX_WORKER_EVENTS       10
        static const long SOCKET_WAIT_TIMEOUT = 5; // in seconds

        boolean WSAConnection::m_WinsockInitialized = FALSE;

        //////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        //////////////////////////////////////////////////////////////////////////

        WSAConnection::WSAConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol) :
            NetConnection(pal, protocol),
            m_Sock(INVALID_SOCKET),
            m_Family(AF_UNSPEC),
            m_dataEvent(INVALID_HANDLE_VALUE),
            m_exitEvent(INVALID_HANDLE_VALUE),
            m_networkErrorEvent(INVALID_HANDLE_VALUE)
        {
            m_dataEvent = CreateEvent(0, FALSE, FALSE, 0);
            m_exitEvent = CreateEvent(0, FALSE, FALSE, 0);
            m_networkErrorEvent = CreateEvent(0, FALSE, FALSE, 0);
            bqinit(&m_SendQ);
        }

        WSAConnection::~WSAConnection()
        {
            if (m_dataEvent != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_dataEvent);
            }

            if (m_exitEvent != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_exitEvent);
            }

            if (m_networkErrorEvent != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_networkErrorEvent);
            }


            if (m_Thread)
            {
                CloseHandle(m_Thread);

            }

            bqfree(&m_SendQ);
        }

        //////////////////////////////////////////////////////////////////////////
        // public functions
        //////////////////////////////////////////////////////////////////////////

        boolean WSAConnection::Connect(const char* pHostName, uint16 port)
        {
            if (m_dataEvent == INVALID_HANDLE_VALUE || m_exitEvent == INVALID_HANDLE_VALUE)
            {
                return FALSE;
            }

            boolean success = FALSE;
            if (!m_Thread && m_WinsockInitialized)
            {
                strncpy(m_HostName, pHostName, sizeof(m_HostName));
                m_HostName[sizeof(m_HostName) - 1] = '\0';
                m_Port = port;
                bqinit(&m_SendQ);

                /*
                    The AddRef for the new thread must be called on this thread.  This is because it is impossible to know when
                    the threadproc will actually start executing, making it possible for the owner to call this function and then
                    Release before the thread has a chance to call AddRef.  In this case, the object is gone when the thread
                    starts executing and it uses an invalid pointer

                    The AddRef for the new thread must also be called before the thread is created.  This is because the
                    threadproc could start and finish before the call to CreateThread returns and the subsequent code gets the
                    opportunity to call AddRef.  In this scenario, the threadproc calls Release, killing the object, while this
                    code is still executing
                */
                AddRef();
                m_Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WSAConnection::WorkerThreadProc, this, 0, &m_ThreadId);
                success = (m_Thread != NULL);
                if (!success)
                {
                    SetStatus(PNCS_Failed);
                    m_HostName[0] = 0;
                    m_Port = 0;
                    m_Thread = NULL;
                    m_ThreadId = 0;

                    /*
                        Call Release because the threadproc never got a chance to do it
                    */
                    Release();
                }
            }

            return success;
        }

        boolean WSAConnection::Send(const byte* pBuffer, int count)
        {
            boolean success = FALSE;

            if (GetStatus() == PNCS_Connected)
            {
                Lock();
                {
                    bqprep(&m_SendQ, count);
                    memcpy(m_SendQ.w, pBuffer, count);
                    m_SendQ.w += count;
                    success = TRUE;
                }
                Unlock();

                // signal thread that there is something to write
                SetEvent(WCSendEvent());
            }

            return success;
        }

        boolean WSAConnection::Close()
        {
            StopNotifies();
            SetStatusImmediate(PNCS_Closed);
            SetEvent(m_exitEvent);

            return TRUE;
        }

        //////////////////////////////////////////////////////////////////////////
        // protected functions
        //////////////////////////////////////////////////////////////////////////

        void WSAConnection::OnConnect()
        {
            SetStatus(PNCS_Connected);
        }

        void WSAConnection::OnError(int error)
        {
            if (GetStatus() == PNCS_Connected)  // got error while connected
            {
                SetStatus(PNCS_Error);
            }
            else                                // got error before connected
            {
                SetStatus(PNCS_Failed);
            }

            SetEvent(m_exitEvent);
        }

        void WSAConnection::WCWorkerEvents(WSAEVENT *events, int *countp)
        {
            events[(*countp)++] = m_dataEvent;
        }

        HANDLE WSAConnection::WCSendEvent()
        {
            return (m_dataEvent);
        }

        //////////////////////////////////////////////////////////////////////////
        // worker thread
        //////////////////////////////////////////////////////////////////////////

        int WSAConnection::SocketCreate()
        {
            int result = SOCKET_ERROR;
            struct addrinfo *addrResult = NULL;
            struct addrinfo hints = {0};

            SetStatus(PNCS_Resolving);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            if ( 0 != getaddrinfo( m_HostName, NULL, &hints, &addrResult ) || !addrResult )
            {
                NotifyDnsResult(TranslateWinsockError(WSAGetLastError()), NULL, 0);
            }
            else
            {
                short ipv4AddressCount = 0, ipv6AddressCount = 0;
                const char** addressList = NULL;
                struct sockaddr_in* ipv4Address = NULL;
                struct sockaddr_in6* ipv6Address = NULL;
                struct sockaddr* address = NULL;
                int addressLen = 0;

                // need to perform addr list array 
                addressList = static_cast<const char**>( malloc( sizeof( const char**) * MAX_DNS_RESULT_ADDRESSES ) );
                memset(addressList, 0, sizeof( const char**) * MAX_DNS_RESULT_ADDRESSES);
                if (addressList != NULL)
                {
                  for ( struct addrinfo* pAddr = addrResult;
                        pAddr != NULL;
                        pAddr = pAddr->ai_next )
                  {
                      if( pAddr->ai_family == AF_INET && ipv4AddressCount < IPV4_DNS_RESULT_ADDRESSES )   // IPv4 address
                      {
                         ipv4Address = reinterpret_cast< sockaddr_in* >( pAddr->ai_addr );
                         addressList[ipv4AddressCount++] = reinterpret_cast< const char* >( &ipv4Address->sin_addr.S_un );
                      }
                      else if( pAddr->ai_family == AF_INET6 && ipv6AddressCount < IPV6_DNS_RESULT_ADDRESSES )  // IPv6 address
                      {
                         ipv6Address = reinterpret_cast< sockaddr_in6* >( pAddr->ai_addr );
                         addressList[IPV4_DNS_RESULT_ADDRESSES + ipv6AddressCount++] = 
                              reinterpret_cast< const char* >( &ipv6Address->sin6_addr.u );
                      }
                  }
                  NotifyDnsResult( PAL_Ok, addressList, -1 );
                  free(addressList);
                  addressList = NULL;
                }
                else
                {
                    NotifyDnsResult(PAL_ErrNoMem, NULL, 0);
                }

                // we need to pass address family into socket create function
                if ( ipv4Address != NULL )
                {
                    m_Family = AF_INET;
                    ipv4Address->sin_port = htons(m_Port);
                    addressLen = sizeof( sockaddr_in );
                    address = reinterpret_cast< struct sockaddr* >( ipv4Address );
                }
                else
                {
                    m_Family = AF_INET6;
                    ipv6Address->sin6_port = htons(m_Port);
                    addressLen = sizeof( sockaddr_in6 );
                    address = reinterpret_cast< struct sockaddr* >( ipv6Address );
                }

                m_Sock = WCSocketCreate();

                // If socket opened initiate connection
                if (m_Sock != INVALID_SOCKET)
                {
                  SetStatus(PNCS_Connecting);
                  result = connect( m_Sock, address, addressLen );

                  if ( result != SOCKET_ERROR )
                  {
                      OnConnect(); 
                  }
                }
                else
                {
                  result = SOCKET_ERROR;
                }
                freeaddrinfo( addrResult );
                addrResult = NULL;
            }

            if (result == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                {
                    // Socket isn't ready now, we need to wait.
                    fd_set writer = {0};
                    struct timeval timeout = {0};

                    FD_SET(m_Sock, &writer);
                    timeout.tv_sec  = SOCKET_WAIT_TIMEOUT;
                    timeout.tv_usec = 0;

                    // Wait until socket is ready for writing. This call blocks the thread,
                    // but it is OK since socket is created in the separate thread.
                    err = select(0, NULL, &writer, NULL, &timeout);
                    if (err == 1)
                    {   // Socket is ready
                        result = 0;
                    }
                    else if (err == 0 || err == SOCKET_ERROR)
                    {   // Timeout or socket error
                        SetStatus(PNCS_Failed);
                    }
                }
                else
                {
                    SetStatus(PNCS_Failed);
                }
            }

            return result;
        }

        void WSAConnection::SocketDestroy()
        {
            if (m_Sock != INVALID_SOCKET)
            {
                closesocket(m_Sock);
                SetStatus(PNCS_Closed);
            }

            m_Sock = INVALID_SOCKET;
            m_Family = AF_UNSPEC;

            m_HostName[0] = 0;
            m_Port = 0;

            Lock();
            bqfree(&m_SendQ);
            Unlock();

            m_Thread = NULL;
            m_ThreadId = 0;
        }

        void WSAConnection::WorkerThread(void)
        {
            if (TRUE != EnsureNetworkConnection())
            {
                SetStatus(PNCS_Failed);
                return;
            }

            int result = SocketCreate();
            if (result == SOCKET_ERROR)
            {
                SocketDestroy();
                return;
            }

            WSAEVENT events[MAX_WORKER_EVENTS];
            int eventCount;

            events[0] = m_exitEvent;
            eventCount = 1;

            WCWorkerEvents(events, &eventCount);
            if (RegisterForNetworkErrorEvent(TRUE, m_networkErrorEvent))
            {
                events[eventCount++] = m_networkErrorEvent;
            }

            m_StopThread = FALSE;
            // Loop forever
            while (m_StopThread == FALSE)
            {
                DWORD result = WSAWaitForMultipleEvents(eventCount, events, FALSE, WSA_INFINITE, FALSE);
                if (result >= WSA_WAIT_EVENT_0 && result < WSA_WAIT_EVENT_0 + (unsigned)eventCount)
                {
                    WSAEVENT ev = events[result - WSA_WAIT_EVENT_0];

                    if (ev == m_exitEvent)
                    {
                    // Exit event signaled - break out of loop
stop:
                        m_StopThread = TRUE;
                        break;
                    }
                    else if (ev == m_networkErrorEvent)
                    {
                        // Network connection error
                        OnError(PAL_ErrNetGeneralFailure);
         
                        // Throw all queued data (if any)
                        Lock();
                        {
                            // Set read pointer same as write pointer
                            m_SendQ.r = m_SendQ.w;
                            bqshrink(&m_SendQ);
                        }
                        Unlock();
                        goto stop;
                    }
                    else if (!WCOnEvent(ev))
                    {
                        goto stop;
                    }
                }
                else
                {
                    // Unrecognized return code, so break out of the loop
                    goto stop;
                }
            }

            // Unregister for network error event
            RegisterForNetworkErrorEvent(FALSE, m_networkErrorEvent);
            SocketDestroy();
        }

        PAL_Error WSAConnection::TranslateWinsockError(int wsaError)
        {
            PAL_Error err = PAL_Ok;

            switch (wsaError)
            {
                case WSATRY_AGAIN:
                    err = PAL_ErrNetTimeout;
                    break;
                case WSAEFAULT:
                    err = PAL_ErrNetDnsMalformedHostName;
                    break;
                case WSAHOST_NOT_FOUND:
                case WSANO_DATA:
                    err = PAL_ErrNetDnsUnknownHostName;
                    break;
                case WSANOTINITIALISED:
                default:
                    err = PAL_ErrNetGeneralFailure;
                    break;
            }
            return err;
        }

        void WSAConnection::WCOnError(int wsaError)
        {
            OnError(TranslateWinsockError(wsaError));
        }

        //////////////////////////////////////////////////////////////////////////
        // static class members & functions
        //////////////////////////////////////////////////////////////////////////

        boolean WSAConnection::InitializeWindowsSockets()
        {
            if( TRUE != EnsureNetworkConnection())
            {
               return FALSE;
            }

            if (m_WinsockInitialized == FALSE)
            {
                m_WinsockInitialized = FALSE;

                int err = 0;
                WSADATA wsaData;
                memset(&wsaData, 0, sizeof(WSADATA));

                err = WSAStartup(MAKEWORD(2, 2), &wsaData);
                if (err == 0)
                {
                      m_WinsockInitialized = TRUE;
                }
            }
            return m_WinsockInitialized;
        }

        void WSAConnection::CleanupWindowsSockets()
        {
            // Clean up Winsock
            if (m_WinsockInitialized)
            {
                WSACleanup();
                m_WinsockInitialized = FALSE;
            }
        }

        DWORD WSAConnection::WorkerThreadProc(LPVOID pParameter)
        {
            WSAConnection* pThis = static_cast<WSAConnection*>(pParameter);

            pThis->WorkerThread();
            pThis->Release();

            return (DWORD)1;
        }
    }
}
