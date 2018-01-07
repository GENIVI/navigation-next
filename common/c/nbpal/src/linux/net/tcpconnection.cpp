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
    @date     3/16/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Linux implementation for TcpConnection class.
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
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "palstdlib.h"
#include "tcpconnection.h"
#include "paldebuglog.h"

namespace nimpal
{
    namespace network
    {
        TcpConnection::TcpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal) :
            NetConnection(protocol, pal),
            m_workerThread(NULL),
            m_workerThreadStop(false),
            m_workerThreadRunning(false),
            m_socket(INVALID_SOCKET)
        {
            m_pipe[0] = INVALID_SOCKET;
            m_pipe[1] = INVALID_SOCKET;
            bqinit(&m_sendQ);
            nsl_memset(&m_socketAddr, 0, sizeof(m_socketAddr));
        }

        TcpConnection::~TcpConnection()
        {
            Close();
            bqfree(&m_sendQ);
        }

        /* See description in netconnection.h */
        void TcpConnection::Destroy()
        {
            // @todo: I think same block issue exists in TCP. But there is not many TCP connection so far.
            delete this;
        }

        PAL_Error TcpConnection::Connect(const char* pHostName, uint16 port, uint32 localIpAddr)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Connect: %s %u\n", pHostName, port));
            if (nsl_strempty(pHostName))
            {
                return PAL_ErrNoData;
            }

            if (m_workerThread)
            {
                Close();    // close previous socket
            }

            bqfree(&m_sendQ);   // bq might not be empty because of reconnection
            m_workerThreadRunning = false;

            int created_status = pthread_mutex_init(&m_sendQMutex, NULL);
            if (created_status)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::Connect FAILED. Can't create mutex err=%u \n", created_status));
                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }

            if (pipe(m_pipe) < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::Connect FAILED. Can't create pipe\n"));

                pthread_mutex_destroy(&m_sendQMutex);

                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }

            if (m_hostName != pHostName) // prevent copying to itself
            {
                nsl_strncpy(m_hostName, pHostName, HOSTNAME_MAX_LEN);
                m_hostName[HOSTNAME_MAX_LEN-1] = '\0';  // always terminates
            }

            m_port = port;
            m_localIpAddr = localIpAddr;

            // start worker thread
            m_workerThreadStop = false;
            created_status = pthread_create(&m_workerThread, NULL, WorkerThreadFunc, this);

            if (created_status)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::Connect FAILED. Can't create worker thread error=%u\n", created_status));
                close(m_pipe[0]);
                close(m_pipe[1]);
                pthread_mutex_destroy(&m_sendQMutex);

                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }

            return PAL_Ok;
        }

        void TcpConnection::WakeWorkerThread(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WakeWorkerThread\n"));
            // write a dummy char to the pipe to wake up the listening worker thread
            write(m_pipe[1], "w", 1);
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WakeWorkerThread SUCCESS\n"));
        }

        PAL_Error TcpConnection::Send(const byte* pBuffer,
                                      uint32      count,
                                      const char* pVerb,
                                      const char* pObject,
                                      const char* pAcceptType,
                                      const char* pAdditionalHeaders,
                                      void*       pRequestData)
        {
            return PAL_ErrUnsupported;
        }

        PAL_Error TcpConnection::Close()
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Close\n"));
            if (m_workerThread)
            {
                // ask worker thread to quit
                m_workerThreadStop = true;
                // wake up worker thread
                WakeWorkerThread();
                // wait for the worker thread to quit
                pthread_join(m_workerThread, NULL);

                close(m_pipe[0]);
                close(m_pipe[1]);

                pthread_mutex_destroy(&m_sendQMutex);

                m_workerThread = NULL;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Close SUCCESS\n"));

            SetStatus(PNCS_Closed);
            return PAL_Ok;
        }


        PAL_Error TcpConnection::SetSocketNonBlocking(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetSocketNonBlocking\n"));

            int flag = fcntl(m_socket, F_GETFL, 0);

            if (flag < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::SetSocketNonBlocking FAILED flag=%d\n", flag));
                // connect error
                return PAL_Failed;
            }

            if (fcntl(m_socket, F_SETFL, (flag | O_NONBLOCK)) < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::SetSocketNonBlocking FAILED\n"));

                return PAL_Failed;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetSocketNonBlocking SUCCESS\n"));
            return PAL_Ok;
        }

        void TcpConnection::SetWorkerThreadDone(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetWorkerThreadDone\n"));

            m_workerThreadRunning = false;

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetWorkerThreadDone SUCCESS\n"));
        }

        PAL_Error TcpConnection::TranslateSocketError(int socketError)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetWorkerThreadDone socketError=%d\n", socketError));

            PAL_Error err = PAL_Ok;

            switch (socketError)
            {
                case HOST_NOT_FOUND:
                    err = PAL_ErrNetDnsMalformedHostName;
                    break;
                case NO_DATA:
                    err = PAL_ErrNetDnsUnknownHostName;
                    break;
                case TRY_AGAIN:
                    err = PAL_ErrNetTimeout;
                    break;
                default:
                    err = PAL_ErrNetGeneralFailure;
                    break;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::SetWorkerThreadDone err=%u\n", err));

            return err;
        }

        PAL_Error TcpConnection::ResolveHostName(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::ResolveHostName\n"));

            struct hostent hostBuf;
            struct hostent* pHostEnt = NULL;
            size_t tmpBufLen = 1024;    // temporary host buffer size
            char* tmpBuf = NULL;
            int hostErrno = 0;
            PAL_Error result = PAL_Failed;

            nsl_memset(&hostBuf, 0, sizeof(hostBuf));

            tmpBuf = (char *)nsl_malloc(tmpBufLen);
            if (tmpBuf == NULL)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::ResolveHostName FAILED. Can't create buffer\n"));
                return PAL_Failed;
            }
#ifdef __QNX__
            while ( (pHostEnt = gethostbyname_r(m_hostName, &hostBuf, tmpBuf,
                                    tmpBufLen, &hostErrno)) == NULL && hostErrno == ERANGE)

#else
            while ( gethostbyname_r(m_hostName, &hostBuf, tmpBuf,
                            tmpBufLen, &pHostEnt, &hostErrno) != 0 && hostErrno == ERANGE)
#endif
            {   //  buffer too small
                //  free previous buffer
                nsl_free(tmpBuf);

                //  and get a larger one
                tmpBufLen *= 2;
                tmpBuf = (char *)nsl_malloc(tmpBufLen);
                if (tmpBuf == NULL)
                {
                    return PAL_Failed;
                }
            }

            if ( pHostEnt )
            {
                // success
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::ResolveHostName pHostEnt=%s\n", pHostEnt->h_name));
                nsl_memset(&m_socketAddr, 0, sizeof(m_socketAddr));
                nsl_memcpy(&m_socketAddr.sin_addr, pHostEnt->h_addr,
                        pHostEnt->h_length);

                NotifyDnsResult(PAL_Ok, (const char**)pHostEnt->h_addr_list, pHostEnt->h_length);

                result = PAL_Ok;
            }
            else
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::ResolveHostName FAILED. pHostEnt is NULL\n"));
                NotifyDnsResult(TranslateSocketError(h_errno), 0, 0);
            }

            if(tmpBuf)
            {
                nsl_free(tmpBuf);
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::ResolveHostName Finished result=%u\n", result));

            return result;
        }

        int TcpConnection::CreateSocket(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocket\n"));

            m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocket m_socket=%d\n", socket));

            return m_socket;
        }

        void TcpConnection::DestroySocket(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::DestroySocket\n"));

            if ( m_socket != INVALID_SOCKET)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::DestroySocket socket is valid\n"));
                close(m_socket);
                m_socket = INVALID_SOCKET;
            }
        }

        PAL_Error TcpConnection::BindSocket(void)
        {
            int ret = 0;

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::BindSocket m_localIpAddr=%d\n", m_localIpAddr));

            if ((m_localIpAddr != 0) && (m_localIpAddr != INADDR_NONE))
            {
                // valid local IP address

                struct sockaddr_in localAddr;

                memset(&localAddr, 0, sizeof(localAddr));
                localAddr.sin_family = AF_INET;
                localAddr.sin_addr.s_addr = m_localIpAddr;

                ret = bind( m_socket, (struct sockaddr*)&localAddr, sizeof(localAddr) );
                if (ret)
                {
                    // bind failed
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::BindSocket FAILED ret=%u\n", ret));
                    return PAL_ErrNetGeneralFailure;
                }
            }

            return PAL_Ok;
        }

        PAL_Error TcpConnection::ConnectSocket(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::ConnectSocket\n"));

            int ret = 0;

            m_socketAddr.sin_family = AF_INET;
            m_socketAddr.sin_port = nsl_htons(m_port);

            ret = connect(m_socket, (struct sockaddr*)&m_socketAddr,
                    sizeof(m_socketAddr));
            if (ret)
            {
                if (errno != EINPROGRESS)
                {
                    // connect error
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::ConnectSocket FAILED errno=%d\n", errno));
                    return PAL_ErrNetGeneralFailure;
                }
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::ConnectSocket SUCCESS\n"));

            return PAL_Ok;
        }


        PAL_Error TcpConnection::CreateSocketConnection(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocketConnection\n"));

            if (CreateSocket() == INVALID_SOCKET)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't create.\n"));
                return PAL_ErrNetGeneralFailure;
            }

            if (BindSocket() != PAL_Ok)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't bind\n"));
                return PAL_ErrNetGeneralFailure;
            }

            if (SetSocketNonBlocking() != PAL_Ok)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't set Non blocking.\n"));
                return PAL_ErrNetGeneralFailure;
            }

            if (ConnectSocket() != PAL_Ok)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't connect\n"));
                return PAL_ErrNetGeneralFailure;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocketConnection SUCCESS\n"));
            return PAL_Ok;
        }

        void TcpConnection::DestroySocketConnection(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::DestroySocketConnection\n"));
            DestroySocket();
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::DestroySocketConnection SUCCESS\n"));
        }

        void* TcpConnection::WorkerThreadFunc(void* user_data)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WorkerThreadFunc\n"));

            TcpConnection* tcpConn = (TcpConnection*)user_data;
            int socketfd = INVALID_SOCKET;
            PAL_NetConnectionStatus netStatus = PNCS_Created;
            bool isSocketConnected = false;

            // Set meaningful name
            pthread_setname_np((pthread_t)pthread_self(), "TCS TcpConnection");

            tcpConn->m_workerThreadRunning = TRUE;
            tcpConn->SetStatus(PNCS_Resolving);
            if (tcpConn->ResolveHostName() != PAL_Ok)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc FAILED. Can't resolve host name\n"));
                tcpConn->SetStatus(PNCS_Failed);
                tcpConn->SetWorkerThreadDone();
                return (void*)PAL_ErrNetGeneralFailure;
            }

            // check whether need to stop
            if (tcpConn->m_workerThreadStop)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc FAILED. Need to stop\n"));
                tcpConn->SetWorkerThreadDone();
                return (void*)PAL_Ok;
            }

            if (PAL_Ok != tcpConn->CreateSocketConnection())
            {
                // Set Error
                tcpConn->DestroySocketConnection();
                tcpConn->SetStatus(PNCS_Failed);
                tcpConn->SetWorkerThreadDone();
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc FAILED. Can't create socket connection\n"));
                return (void*)PAL_ErrNetGeneralFailure;
            }

            netStatus = PNCS_Connecting;
            tcpConn->SetStatus(PNCS_Connecting);

            // main loop
            while (!tcpConn->m_workerThreadStop)
            {
                // If can't reach internet connection(e.g. by turn on airplane mode),
                // exit while loop immediatly and report error soon.
                // Sometimes our application will continue to request network even after
                // user turn on airplane mode, this will cause 30s timeout error happen later.
                if (!NetConnection::IsReachableForInternetConnection())
                {
                    netStatus = PNCS_Failed;
                    break;
                }

                fd_set readfds;
                fd_set writefds;
                int ret = 0;
                int maxfd = 0;

                socketfd = tcpConn->GetSocket();
                if (socketfd == INVALID_SOCKET)
                {
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc FAILED. INVALID_SOCKET\n"));
                    netStatus = PNCS_Error;
                    break;
                }

                // prepare fd_sets for initial connecting
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);

                FD_SET(tcpConn->m_pipe[0], &readfds);
                maxfd = tcpConn->m_pipe[0];

                if (tcpConn->IsSocketReadable())
                {
                    FD_SET(socketfd, &readfds);
                    if (maxfd < socketfd)
                    {
                        maxfd = socketfd;
                    }
                }

                if ((!isSocketConnected) || tcpConn->IsSocketWritable())
                {
                    FD_SET(socketfd, &writefds);
                    if (maxfd < socketfd)
                    {
                        maxfd = socketfd;
                    }
                }

                ret = select(maxfd+1, &readfds, &writefds, NULL, NULL);

                // check whether need to stop worker thread first
                if (tcpConn->m_workerThreadStop)
                {
                    // need to stop
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc. Stop worker thread\n"));
                    netStatus = PNCS_Closed;
                    break;
                }

                if (ret == 0)
                {
                    // timeout
                    continue;   // ingore timeout, back to select
                }
                else if (ret == SELECT_ERROR)
                {
                    // no fd is ready, ignore interruption
                    if (errno == EINTR)
                    {
                        continue;   // ingore interruption, back to select
                    }
                    else
                    {
                        // error happened
                        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WorkerThreadFunc BREAK\n"));
                        netStatus = PNCS_Error;
                        break;
                    }
                }

                //fd is ready
                if (FD_ISSET(tcpConn->m_pipe[0], &readfds))
                {
                    char tmpBuf[16];
                    // flush the notification pipe
                    read(tcpConn->m_pipe[0],tmpBuf, sizeof(tmpBuf));
                }

                if (FD_ISSET(socketfd, &readfds))
                {
                    netStatus = tcpConn->ReadSocket();
                    if (netStatus != PNCS_Connected)
                    {
                        // error encounted
                        break;
                    }
                }

                if (FD_ISSET(socketfd, &writefds))
                {
                    if (!isSocketConnected)
                    {
                        // async. socket connected
                        isSocketConnected = true;
                        tcpConn->SetStatus(PNCS_Connected);
                    }

                    netStatus = tcpConn->WriteSocket();
                    if (netStatus != PNCS_Connected)
                    {
                        // error encounted
                        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::WorkerThreadFunc BREAK. netStatus=%d\n", netStatus));
                        break;
                    }
                }
            }

            tcpConn->DestroySocket();
            tcpConn->SetStatus(netStatus);
            tcpConn->SetWorkerThreadDone();
            return (void*)PAL_Ok;
        }

        nb_boolean TcpConnection::IsSocketReadable(void)
        {
            return (m_status == PNCS_Connected);
        }

        nb_boolean TcpConnection::IsSocketWritable(void)
        {
            int byteCount = 0;

            if (m_status != PNCS_Connected)
            {
                return FALSE;
            }

            int lock_status = pthread_mutex_lock(&m_sendQMutex);
            if (lock_status == 0)
            {
                byteCount = m_sendQ.w - m_sendQ.r;
                pthread_mutex_unlock(&m_sendQMutex);
            }

            return (byteCount > 0);
        }

        PAL_NetConnectionStatus TcpConnection::WriteSocket(void)
        {
            unsigned int bytesToWrite = 0;
            unsigned int totalBytesWritten = 0;
            PAL_NetConnectionStatus status = PNCS_Connected;
            byte* pData = NULL;

            pthread_mutex_lock(&m_sendQMutex);
            pData = (byte *)m_sendQ.r;
            bytesToWrite = m_sendQ.w - m_sendQ.r;
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket w=%d\n", m_sendQ.w));
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket r=%d\n", m_sendQ.r));
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket bytesToWrite=%d\n", bytesToWrite));
            while (bytesToWrite > 0)
            {
                int bytesWritten = write(m_socket, m_sendQ.r, bytesToWrite);
                if (bytesWritten > 0)
                {
                    m_sendQ.r += bytesWritten;
                    bytesToWrite -= bytesWritten;
                    totalBytesWritten += bytesWritten;
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket r=%d", m_sendQ.r));
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket bytesToWrite=%d\n", bytesToWrite));
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::WriteSocket totalBytesWritten=%d\n", totalBytesWritten));
                }
                else if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    break;
                }
                else
                {
                    status = PNCS_Error;
                    break;
                }
            }

            if (totalBytesWritten > 0)
            {
                // notify data sent
                NotifyDataSent(pData, totalBytesWritten);
            }
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("---> TcpConnection::WriteSocket status=%d\n", status));
            pthread_mutex_unlock(&m_sendQMutex);

            return status;
        }

        PAL_NetConnectionStatus TcpConnection::ReadSocket(void)
        {
            // read data
            char readBuffer[READ_BUFFER_SIZE];
            ssize_t count = read(m_socket, readBuffer, sizeof(readBuffer));
            PAL_NetConnectionStatus status = PNCS_Connected;

            if (count > 0)
            {
                // data read
                NotifyDataReceived((const byte*)readBuffer, count);
                return status;
            }
            else if (count == 0)
            {
                // socket closed
                status = PNCS_Closed;
                return status;
            }
            else if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // signal caught or no data
                status = PNCS_Connected;
                return status;
            }
            else
            {
                // error encounted
                status = PNCS_Error;
                return status;
            }
        }

        PAL_Error TcpConnection::Send(const byte* pBuffer, int count)
        {
            // add the rest of the data to the buffer queue
            if (AddDataToSendQ(pBuffer, count) == PAL_Ok)
            {
                if (m_workerThreadRunning)
                {
                    // notify worker thread about data
                    WakeWorkerThread();
                }
                else
                {
                    // reconnect socket, and start the worker thread
                    return Connect(m_hostName, m_port, m_localIpAddr);
                }

                return PAL_Ok;
            }

            return PAL_Failed;
        }

        PAL_Error TcpConnection::AddDataToSendQ(const byte* pBuffer, int count)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::AddDataToSend pBuffer=%d, count=%d", pBuffer, count));
            if (pBuffer && (count > 0))
            {
                pthread_mutex_lock(&m_sendQMutex);
                bqprep(&m_sendQ, count);
                nsl_memcpy(m_sendQ.w, pBuffer, count);
                m_sendQ.w += count;
                pthread_mutex_unlock(&m_sendQMutex);
                return PAL_Ok;
            }

            return PAL_Failed;
        }

    }   //network
}   //nimpal
/*! @} */
