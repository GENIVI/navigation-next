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

    @file     HttpConnection.cpp
    @date     3/18/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Linux implementation for HttpConnection class.
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
#include <signal.h>
#include <unistd.h>
#include "palstdlib.h"
#include "paltaskqueue.h"
#include "httpconnection.h"
#include "paldebuglog.h"

#define MAX_RETRY_BEFORE_NOTIFY       3

namespace nimpal
{
    namespace network
    {
        // Action flags for main WAIT loop
        #define  HTTP_CONNECTION_CLOSE     0x00000001
        #define  HTTP_NEW_REQUEST          0x00000002
        #define  HTTP_SOCKET_READ          0x00000004
        #define  HTTP_SOCKET_WRITE         0x00000008

        const int FLUSH_BUFFER_SIZE = 128;

        /*! This is the time (in seconds) between retries when the socket connect() call fails.
                    This is also used for the select() timeout.
                */
        const int CONNECT_RETRY_TIMEOUT = 15;

        const unsigned int DEFAULT_HTTP_PORT = 80;
        const unsigned int DEFAULT_HTTPS_PORT = 443;
        const char* ACCEPT_HEADER = "Accept";
        const char *CACHE_CONTROL_HEADER = "Cache-Control";
        const char *DEFAULT_CACHE_CONTROL= "no-cache";

        const int HTTP_INVALID_SOCKET = -1;
        const int HTTP_SELECT_ERROR = -1;

        HttpConnection::HttpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal) :
            NetConnection(protocol, pal),
            m_requestQueue(),
            m_pal(pal),
            m_currentRequest(NULL),
            m_currentResponse(NULL),
            m_statusNotified(false),
            m_workerThread(0),
            m_workerThreadStop(false),
            m_willBeDestroyed(false),
            m_socket(HTTP_INVALID_SOCKET)
        {
            nsl_memset(&m_socketAddr, 0, sizeof(m_socketAddr));

            // Prevent the SIGPIPE when send data to disconnected socket.
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigaction(SIGPIPE, &sa, NULL);

            pthread_mutex_init(&m_requestQueueMutex, NULL);

            m_status = PNCS_Initialized;
        }

        HttpConnection::~HttpConnection()
        {

            pthread_mutex_lock( &m_requestQueueMutex );
            while ( m_requestQueue.size() >0 )
            {
                delete m_requestQueue.front();
                m_requestQueue.pop();
            }

            // Discard current request and response
            if (m_currentRequest)
            {
                delete m_currentRequest;
                m_currentRequest = NULL;
            }

            if (m_currentResponse)
            {
                delete m_currentResponse;
                m_currentResponse = NULL;
            }

            pthread_mutex_unlock(&m_requestQueueMutex);
            pthread_mutex_destroy(&m_requestQueueMutex);
        }

        PAL_Error HttpConnection::Close()
        {

            if (m_workerThread)
            {
                // Ask worker thread to quit
                m_workerThreadStop = TRUE;
                // Wake up worker thread
                WakeWorkerThread();

                /* Set current status to PNCS_Closing to avoid reconnecting when closing
                           connection has not been finished. User should ignore PNCS_Closing status
                           and reconnect when receiving PNCS_Closed status. If user reconnects when
                           closing connection has not been finished, the function 'Connect' blocks
                           the thread for closing connection and then reconnect. */
                SetStatus(PNCS_Closing);

                /* function WorkerThreadExitCallback is called and sets status to closed
                           when worker thread exits. */
            }
            else
            {
                SetStatus(PNCS_Closed);
            }

            return PAL_Ok;
        }

        /* See description in netconnection.h */
        void HttpConnection::Destroy()
        {

            // Remove all pending callbacks of this connection.
            StopAllCallbacks();

            if (m_workerThread == NULL)
            {
                // Current function may be called in a callback function of this connection.
                if (IsInCallback())
                {
                    TaskId taskId = 0;
                    PAL_EventTaskQueueAdd(m_pal,
                                          (PAL_TaskQueueCallback) WorkerThreadExitCallback,
                                          (void*) this,
                                          &taskId);
                }
                else
                {
                    delete this;
                }
            }
            // Check if destroyed flag is set.
            else if (!m_willBeDestroyed)
            {
                m_willBeDestroyed = TRUE;
                Close();
            }
        }

        PAL_Error HttpConnection::Connect(const char* pHostName, uint16 port, uint32 /*localIpAddr*/)
        {
            if (nsl_strempty(pHostName))
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::Connect pHostName is empty! \n"));
                return PAL_ErrNoData;
            }

            /* @todo: I check with current code of CCC using HTTP connection. This function is always called
                              after receiving closed flag. So cannot enter this check so far.
                    */
            if (m_workerThread)
            {
                // Ask worker thread to quit
                m_workerThreadStop = TRUE;
                // Wake up worker thread
                WakeWorkerThread();

                // Wait for worker thread to exit.
                pthread_join(m_workerThread, NULL);

                close(m_pipe[0]);
                close(m_pipe[1]);

                m_workerThread = NULL;

                SetStatus(PNCS_Closed);
            }

            if (pipe(m_pipe) < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::Connect pipe error \n"));
                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }

            // Set pipe fds as nonblocking
            SetFdNonBlocking(m_pipe[0]);
            SetFdNonBlocking(m_pipe[1]);

            if (m_hostName != pHostName) // prevent copying to itself
            {
                nsl_strncpy(m_hostName, pHostName, HOSTNAME_MAX_LEN);
                m_hostName[HOSTNAME_MAX_LEN-1] = '\0';  // always terminates
            }
            m_port = DEFAULT_HTTP_PORT;//port

            // start worker thread
            m_workerThreadStop = false;
            int created_status = pthread_create(
                        &m_workerThread,
                        NULL,
                        WorkerThreadFunc,
                        this
                        );

            if (created_status)
            {
                close(m_pipe[0]);
                close(m_pipe[1]);

                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }
            SetStatus(PNCS_Connected);
            return PAL_Ok;
        }


        PAL_Error HttpConnection::ResolveHostName(void)
        {
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

            if ( pHostEnt != NULL )
            {
                // success
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ResolveHostName pHostEnt=%s\n", pHostEnt->h_name));
                nsl_memset(&m_socketAddr, 0, sizeof(m_socketAddr));
                nsl_memcpy(&m_socketAddr.sin_addr, pHostEnt->h_addr,
                           pHostEnt->h_length);

                result = PAL_Ok;
            }

            if (tmpBuf)
            {
                nsl_free(tmpBuf);
            }
            return result;
        }

        PAL_Error HttpConnection::SetFdNonBlocking(int fd)
        {

            int flag = fcntl(fd, F_GETFL, 0);
            if (flag < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::SetFdNonBlocking invalid flag \n"));
                return PAL_ErrNetGeneralFailure;
            }

            if (fcntl(fd, F_SETFL, (flag | O_NONBLOCK)) < 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::SetFdNonBlocking unable to set flag \n"));
                return PAL_ErrNetGeneralFailure;
            }
            return PAL_Ok;
        }

        PAL_Error HttpConnection::EstablishSocketConnection(void)
        {
            // STEP 1: Create socket
            m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (-1 == m_socket)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::EstablishSocketConnection socket creation error! \n"));
                return PAL_ErrNetGeneralFailure;
            }

            // STEP 2: Set socket as non-blocking
            if (PAL_Ok != SetFdNonBlocking(m_socket))
            {
                return PAL_ErrNetGeneralFailure;
            }

            // STEP 3: Connect
            int result = 0;
            m_socketAddr.sin_family = AF_INET;
            m_socketAddr.sin_port = nsl_htons(m_port);

            result = connect(m_socket, (struct sockaddr*)&m_socketAddr, sizeof(m_socketAddr));
            if ((result == 0) || (errno == EISCONN))
            {
                return PAL_Ok;
            }
            else if (errno != EINPROGRESS)
            {
                // Connect failed.
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::EstablishSocketConnection connect error! \n"));
                return PAL_ErrNetGeneralFailure;
            }

            // Step 4: Wait for connect success.
            int selectResult = 0;
            // Maximum value of descriptor to check
            int maxFd = 0;
            timeval timeout = {0};
            // Descriptors to check in 'select' call
            fd_set readFds;
            fd_set writeFds;
            fd_set exceptionFds;

            timeout.tv_sec = CONNECT_RETRY_TIMEOUT;
            timeout.tv_usec = 0;

            // Initialize all descriptor sets to null sets.
            FD_ZERO(&readFds);
            FD_ZERO(&writeFds);
            FD_ZERO(&exceptionFds);

            // Add socket to read and write decriptor sets to monitor connect.
            FD_SET(m_socket, &readFds);
            FD_SET(m_socket, &writeFds);
            FD_SET(m_socket, &exceptionFds);

            // Add pipe to read descriptor set to monitor exit.
            FD_SET(m_pipe[0], &readFds);
            FD_SET(m_pipe[0], &exceptionFds);

            maxFd = std::max(m_socket, m_pipe[0]);

            // The function 'WakeWorkerThread' could interrupt this 'select'
            selectResult = select(maxFd + 1, &readFds, &writeFds, &exceptionFds, &timeout);

            // Check if function select failed or timeout. Retry logic is out of this function.
            if (selectResult <= 0)
            {
                return PAL_ErrNetGeneralFailure;
            }

            // Check if pipe is readable and return failed. Because user wakes this thread.
            if (FD_ISSET(m_pipe[0], &readFds))
            {
                byte pipeFlushBuffer[FLUSH_BUFFER_SIZE] = {0};
                unsigned int pipeFlushBufferLength = sizeof(pipeFlushBuffer);

                // Clear data in pipe and return failed.
                ReadData(m_pipe[0], pipeFlushBuffer, &pipeFlushBufferLength);
                return PAL_ErrNetGeneralFailure;
            }

            // Check if socket descriptor is readable or writable.
            if (!(FD_ISSET(m_socket, &readFds) || FD_ISSET(m_socket, &writeFds)))
            {
                return PAL_ErrNetGeneralFailure;
            }

            byte socketFlushBuffer[FLUSH_BUFFER_SIZE] = {0};
            unsigned int socketFlushBufferLength = sizeof(socketFlushBuffer);

            // Clear data in socket to send a request.
            ReadData(m_socket,socketFlushBuffer, &socketFlushBufferLength);

            int socketError = 0;
            socklen_t socketErrorLength = sizeof(socketError);

            // Ignore returned value and just check socketError.
            getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &socketError, &socketErrorLength);
            if (socketError == 0)
            {
                return PAL_Ok;
            }

            return PAL_ErrNetGeneralFailure;
        }

        void HttpConnection::DestroySocketConnection(void)
        {
            if (HTTP_INVALID_SOCKET != m_socket)
            {
                shutdown(m_socket, SHUT_WR);
                close(m_socket);
                m_socket = HTTP_INVALID_SOCKET;
            }
        }

        void HttpConnection::WakeWorkerThread()
        {
            // Write a dummy char to the pipe to wake up the listening worker thread
            unsigned int count = 1;
            WriteData(m_pipe[1], "w", &count);
        }

        void HttpConnection::WaitForNextAction(int* flag        /*!< !Important:
                                                                                                                                                                                                                                         Input: can be combination of flags (I guess)
                                                                                                                                                                                                                                         Ouput: The calling function 'assumes' that no flags get combined.
                                                                                                                                                                                                                                         This is very misleading but that's the way it is implemented */
                                               )
        {
            while(1)
            {
                int ret = 0;

                // Descriptors to check in select call
                fd_set  readFds;
                fd_set  writeFds;
                fd_set  exceptionFds;

                // Maximum value of descriptor to check
                int     max_fd = 0;

                FD_ZERO(&readFds);
                FD_ZERO(&writeFds);
                FD_ZERO(&exceptionFds);

                if (*flag & (HTTP_CONNECTION_CLOSE | HTTP_NEW_REQUEST))
                {
                    FD_SET(m_pipe[0], &readFds);
                }

                if (*flag & HTTP_SOCKET_READ)
                {
                    FD_SET(m_socket, &readFds);
                }

                if (*flag & HTTP_SOCKET_WRITE)
                {
                    FD_SET(m_socket, &writeFds);
                }

                // Add pipe and socket to exception descriptor so that we get any error on those sockets/pipes
                FD_SET(m_pipe[0], &exceptionFds);
                max_fd = std::max(max_fd, m_pipe[0]);

                if (m_socket != HTTP_INVALID_SOCKET)
                {
                    FD_SET(m_socket, &exceptionFds);
                    max_fd = std::max(max_fd, m_socket);
                }

                // Set timeout
                timeval timeout = {0};
                timeout.tv_sec = CONNECT_RETRY_TIMEOUT;
                timeout.tv_usec = 0;

                // Wait until anything happens on the socket/pipe
                ret = select(max_fd + 1, &readFds, &writeFds, &exceptionFds, &timeout);

                // timeout
                if (ret == 0)
                {
                    // timeout
                    if (TRUE == m_workerThreadStop)
                    {
                        // Connection closure
                        *flag = HTTP_CONNECTION_CLOSE;
                        return;
                    }

                    if (*flag & HTTP_NEW_REQUEST)
                    {
                        // Continue to wait on select
                        continue;
                    }

                    // If we are waiting on anything other then a new request and the select() call times out then
                    // we probably lost connection. We close the connection and try to reestablis a new connection
                    *flag = HTTP_CONNECTION_CLOSE;
                    return;
                }
                // Success
                else if (ret > 0)
                {
                    // Did an error occur on the socket or pipe (e.g. closure of the socket)
                    if (FD_ISSET(m_pipe[0], &exceptionFds) || ((m_socket != HTTP_INVALID_SOCKET) && (FD_ISSET(m_socket, &exceptionFds))))
                    {
                        // Close connection
                        *flag = HTTP_CONNECTION_CLOSE;
                        return;
                    }
                    else if (FD_ISSET(m_pipe[0], &readFds))
                    {
                        if (TRUE == m_workerThreadStop)
                        {
                            // Connection closure
                            *flag = HTTP_CONNECTION_CLOSE;
                        }
                        else
                        {
                            // We have new request
                            *flag = HTTP_NEW_REQUEST;
                            // Flush pipe
                            unsigned int count = sizeof(m_readBuffer);
                            ReadData(m_pipe[0], m_readBuffer, &count);
                        }
                    }
                    else if ((*flag & HTTP_SOCKET_READ) && FD_ISSET(m_socket, &readFds))
                    {
                        // Socket ready for read
                        *flag = HTTP_SOCKET_READ;
                    }
                    else if ((*flag & HTTP_SOCKET_WRITE) && FD_ISSET(m_socket, &writeFds))
                    {
                        // Socket ready for write
                        *flag = HTTP_SOCKET_WRITE;
                    }
                    else
                    {
                        // No desciptor is set. This should never happen.
                        *flag = HTTP_CONNECTION_CLOSE;
                    }
                    return;
                }
                // Failure
                else
                {
                    if (errno == EINTR)
                    {
                        continue; // Ignore interruption, back to select.
                    }
                    else
                    {
                        // Connection closure
                        *flag = HTTP_CONNECTION_CLOSE;
                        return;
                    }
                }
            }
        }

        HTTP_Error HttpConnection::ReadData(int fd, void *buf, unsigned int* count)
        {
            HTTP_Error err = HTTP_Ok;
            int bytesRead = 0;
            while(1)
            {

                bytesRead = read(fd, buf, *count);
                if (-1 == bytesRead)
                {
                    // Read error
                    if (EINTR == errno)
                    {
                        // Read interrupted by a signal. Continue. Read again
                        continue;
                    }
                    else
                    {
                        if (EAGAIN == errno)
                        {
                            // No data available for read
                            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReadData No data available for read \n"));
                            err = HTTP_ErrReadAgain;
                        }
                        else
                        {
                            // Maybe a I/O error or some other error
                            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReadData I/O error \n"));
                            err = HTTP_ErrReadFailure;
                        }
                        // Set read count to ZERO
                        *count = 0;
                        break;
                    }
                }
                else
                {
                    // Read succeeded
                    *count = bytesRead;
                    break;
                }
            }
            return err;
        }

        HTTP_Error HttpConnection::WriteData(int fd, const void* buf, unsigned int* count)
        {
            HTTP_Error err = HTTP_Ok;
            int bytesWritten = 0;
            uint32 bytesToWrite = *count;

            while (bytesToWrite > 0)
            {

                bytesWritten = write(fd,
                                     (const void *)((char *)buf + bytesWritten),
                                     bytesToWrite);

                if (bytesWritten > 0)
                {
                    bytesToWrite -= bytesWritten;
                }
                else if (errno == EINTR)
                {
                    // Interrupted by a signal. Ignore it
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    // Not yet ready for write
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::WriteData Not yet ready for write \n"));
                    err = HTTP_ErrWriteAgain;
                    break;
                }
                else
                {
                    // Maybe a EIO error or some other error
                    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::WriteData EIO error \n"));
                    err = HTTP_ErrWriteFailure;
                    break;
                }
            }
            *count -= bytesToWrite;
            return err;
        }

        //  This function establishes a new socket connection with server, if previous one
        //  is terminated by server.
        //  Call this function to ensure that socket is active, before making request.
        //  Note: DO NOT call this function when there is a response pending from server.
        //  i.e. Call it only for fresh requests.
        PAL_Error HttpConnection::EnsureSocketConnection()
        {
            PAL_Error error = PAL_Ok;
            if (HTTP_INVALID_SOCKET == m_socket)
            {
                /*
                            @todo:
                            We currently create a new socket connection for every single http request. If we would use
                            persistent HTTP connections then we could improve the performance.
                        */
                error = EstablishSocketConnection();
            }
            else
            {
                unsigned int numBytes = HTTP_READ_BUFFER_SIZE;
                HTTP_Error err = ReadData(m_socket, m_readBuffer, &numBytes);
                if (HTTP_Ok == err)
                {
                    if (!numBytes)
                    {
                        // Zero bytes on Idle socket means that Server has closed the connection
                        DestroySocketConnection();
                        error = EstablishSocketConnection();
                    }
                    else
                    {
                        // There shouldn't be any data on socket at this stage, but it does happen quite a lot.
                        // I think this is ok. Return success
                        error = PAL_Ok;
                    }
                }
                else if (HTTP_ErrReadAgain == err)
                {
                    // Socket is just fine and connected and has no data for READ
                    // Use the same socket
                    error = PAL_Ok;
                }
                else
                {
                    DestroySocketConnection();
                    error = EstablishSocketConnection();
                }
            }
            return error;
        }

        void* HttpConnection::WorkerThreadFunc(void* user_data)
        {
            HttpConnection* httpConn = (HttpConnection*)user_data;
            int default_flag = HTTP_CONNECTION_CLOSE;
            int flag = 0;

            // Set meaningful name
            pthread_setname_np((pthread_t)pthread_self(), "TCS HttpConnection");

            // Don't call ResolveHostName() and EstablishSocketConnection() here since we don't have a
            // way to recover. The functions get called below in the while loop

            // Initially, Worker thread waits for new request or connection closure
            flag = default_flag | HTTP_NEW_REQUEST;
            while (!httpConn->m_workerThreadStop)
            {
                // Make sure we have socket connection, before writing
                if (flag & HTTP_SOCKET_WRITE)
                {
                    PAL_Error ensureResult = PAL_Ok;
                    bool firstRun = true;

                    /*
                                Try to reconnect until we get a connection or the thread is closed.
                            */
                    do
                    {
                        // Only do the web-call if it has already failed at least once. We don't want to make this call in
                        // normal operation
                        // Resolve host name if it is not resolved, yet
                        ensureResult = PAL_Ok;
                        if (httpConn->m_socketAddr.sin_addr.s_addr == 0)
                        {
                            ensureResult = httpConn->ResolveHostName();
                        }

                        if (ensureResult == PAL_Ok)
                        {
                            ensureResult = httpConn->EnsureSocketConnection();
                            if (ensureResult == PAL_Ok)
                            {
                                // Success
                                break;
                            }
                        }

                        // Check the thread stop flag. Because function EstablishSocketConnection calls select.
                        if (httpConn->m_workerThreadStop)
                        {
                            break;
                        }

                        /*
                                    Fix to simply use 'select' to replace 'sleep' here. Using 'sleep' cause the CCC thread block by
                                    below reproduced steps:
                                    1. Disable the network
                                    2. Create a HttpConnection instance and call HttpConnection::Connect
                                    3. The function 'Connect' create a network thread and block at here because the network is disable
                                    4. User try to call HttpConnection::Close() in CCC thread
                                    5. The CCC thread blocks at the line 'pthread_join(m_workerThread, NULL);' to wait the 'sleep'
                                */
                        // Don't sleep on the first run so that we can recover immediately from airplane mode
                        if (!firstRun)
                        {
                            int selectResult = 0;
                            // Maximum value of descriptor to check
                            int maxFd = 0;
                            timeval timeout = {0};
                            // Descriptors to check in 'select' call
                            fd_set  readFds;
                            fd_set  exceptionFds;

                            timeout.tv_sec = CONNECT_RETRY_TIMEOUT;
                            timeout.tv_usec = 0;

                            FD_ZERO(&readFds);
                            FD_ZERO(&exceptionFds);
                            FD_SET(httpConn->m_pipe[0], &readFds);
                            FD_SET(httpConn->m_pipe[0], &exceptionFds);
                            maxFd = std::max(maxFd, httpConn->m_pipe[0]);

                            // The function 'WakeWorkerThread' could interrupt this 'select'
                            selectResult = select(maxFd + 1, &readFds, (fd_set*)NULL, &exceptionFds, &timeout);

                            // Success of 'readFds' must flush 'm_pipe'. Otherwise it causes dead loop.
                            if ((selectResult > 0) && (FD_ISSET(httpConn->m_pipe[0], &readFds)))
                            {
                                byte pipeFlushBuffer[FLUSH_BUFFER_SIZE] = {0};
                                unsigned int pipeFlushBufferLength = sizeof(pipeFlushBuffer);

                                httpConn->ReadData(httpConn->m_pipe[0], pipeFlushBuffer, &pipeFlushBufferLength);
                            }
                        }

                        firstRun = false;

                        // Do not need to handle for retry counters, the HttpDownloadManager
                        // should do this.
                        httpConn->NotifyErrorForAllRequests(PAL_ErrNetGeneralFailure);
                    } while (!httpConn->m_workerThreadStop && (ensureResult != PAL_Ok));
                }

                if (httpConn->m_workerThreadStop)
                {
                    break;
                }

                // Worker thread remains blocked until it has new things to do
                httpConn->WaitForNextAction(&flag);

                /*
                            This assumse that WaitForNextAction() does NOT combine any flags. Otherwise we would always go into
                            the default case!
                        */
                switch(flag)
                {
                case HTTP_NEW_REQUEST:
                    // We have new requests. Wait for Socket to be ready for WRITE
                    flag = default_flag | HTTP_SOCKET_WRITE;
                    break;

                case HTTP_SOCKET_WRITE:
                    // Send the request to server
                    httpConn->SendRequest(&flag);
                    // Set flags  (default flags to stay anyway)
                    flag |= default_flag;
                    break;

                case HTTP_SOCKET_READ:
                    // Send the request to server
                    httpConn->ReceiveResponse(&flag);
                    // Set flags  (default flags to stay anyway)
                    flag |= default_flag;
                    break;

                case HTTP_CONNECTION_CLOSE:

                    if (httpConn->m_currentResponse)
                    {
                        httpConn->m_currentResponse->SetResponseError(PAL_ErrNetGeneralFailure);
                    }
                    httpConn->DestroySocketConnection();
                    httpConn->FinishRequest(&flag);
                    break;

                default:
                    break;
                }
            }

            // Check if this connection need to destroy.
            if (httpConn->m_willBeDestroyed)
            {
                close(httpConn->m_pipe[0]);
                close(httpConn->m_pipe[1]);
                httpConn->m_workerThread = 0;
                delete httpConn;
                httpConn = NULL;
            }
            else
            {
                // Add a task to notify worker thead to exit.
                TaskId taskId = 0;
                PAL_EventTaskQueueAdd(httpConn->m_pal,
                                      (PAL_TaskQueueCallback) WorkerThreadExitCallback,
                                      (void*) httpConn,
                                      &taskId);
            }

            return (void*)PAL_Ok;
        }

        PAL_Error HttpConnection::Send(const byte* pBuffer, int count)
        {
            return Send(pBuffer, count, NULL, NULL, NULL, NULL, NULL);
        }

        PAL_Error HttpConnection::Send(const byte* pBuffer,
                                       uint32      count,
                                       const char* pVerb,
                                       const char* pObject,
                                       const char* pAcceptType,
                                       const char* pAdditionalHeaders,
                                       void*       pRequestData)
        {

            // Create a new request
            HttpRequest* pRequest = new HttpRequest(m_hostName, pBuffer, count,
                                                    pVerb, pObject, pAcceptType, pAdditionalHeaders, pRequestData);

            // add the rest of the data to the buffer queue
            if (AddDataToSendQ(pRequest) == PAL_Ok)
            {
                // notify worker thread about data
                WakeWorkerThread();
                return PAL_Ok;
            }
            return PAL_Failed;
        }

        PAL_Error HttpConnection::AddDataToSendQ(HttpRequest* pRequest)
        {
            pthread_mutex_lock(&m_requestQueueMutex);
            m_requestQueue.push(pRequest);
            pthread_mutex_unlock(&m_requestQueueMutex);

            return PAL_Ok;
        }

        HttpRequest* HttpConnection::GetNextRequest(void)
        {
            HttpRequest* pRequest = NULL;

            pthread_mutex_lock(&m_requestQueueMutex);
            if (!m_requestQueue.empty())
            {
                pRequest = m_requestQueue.front();
                m_requestQueue.pop();
            }
            pthread_mutex_unlock(&m_requestQueueMutex);

            return pRequest;
        }

        //
        // Note: SendRequest() MUST NOT be called when there's a response pending from server
        //
        void HttpConnection::SendRequest(int* nextActionFlag)
        {
            if (NULL == m_currentRequest)
            {
                m_currentRequest = GetNextRequest();
                if (NULL == m_currentRequest)
                {
                    // No more requests in queue.
                    *nextActionFlag = HTTP_NEW_REQUEST;
                    return;
                }
            }
            HTTP_Error   err    = HTTP_Ok;
            unsigned int length = m_currentRequest->m_length - m_currentRequest->m_sentCount;
            const void*  bytes  = m_currentRequest->m_pBytes + m_currentRequest->m_sentCount;

            // Write data to socket
            err = WriteData(m_socket, bytes, &length);
            m_currentRequest->m_sentCount += length;
            if (HTTP_Ok == err)
            {
                // notify data sent
                if (m_currentRequest->m_bodyLength)
                {
                    NotifyDataSent(m_currentRequest->GetRequestBody(),
                                   m_currentRequest->m_bodyLength);
                }
                // All request data sent to server
                // Next: Wait for READ Ready on socket for response
                *nextActionFlag = HTTP_SOCKET_READ;
                return;
            }
            else if (err == HTTP_ErrWriteAgain)
            {
                // Request data is not completely written to socket
                *nextActionFlag = HTTP_SOCKET_WRITE;
                return;
            }
            else
            {
                // Bad Socket
                DestroySocketConnection();
                FinishRequest(nextActionFlag);
            }
        }

        void HttpConnection::ReceiveResponse(int* nextActionFlag)
        {
            HTTP_Error         err = HTTP_Ok;
            unsigned int       count = HTTP_READ_BUFFER_SIZE;
            HttpResponseState  responseState = HTTP_RESPONSE_WAITING_STATUS;
            if (NULL == m_currentResponse)
            {
                m_currentResponse = new HttpResponse();
            }

            // STEP1: Read data
            err = ReadData(m_socket, m_readBuffer, &count);
            if (HTTP_Ok == err)
            {
                if (!count)
                {
                    m_currentResponse->SetResponseError(PAL_ErrNetGeneralFailure);
                }
                else
                {
                    // STEP2: Process received data
                    responseState = m_currentResponse->AddData(m_readBuffer, count);

                    // STEP3: Notify Status code (if not done before)
                    if (FALSE == m_statusNotified && m_currentResponse->IsStatusCodeReceived())
                    {
                        unsigned int statusCode = m_currentResponse->GetStatusCode();

                        NotifyHttpResponseStatus(m_currentRequest->GetUserRequestData(),
                                                 statusCode);
                        m_statusNotified = TRUE;
                        if (HTTP_STATUS_OK != statusCode)
                        {
                            // TODO: To handle cases other than OK.
                            // Close the socket connection
                            m_currentResponse->SetResponseError(PAL_ErrNetGeneralFailure);
                        }
                    }

                    if (PAL_Ok == m_currentResponse->GetResponseError())
                    {
                        switch (responseState)
                        {
                        case HTTP_RESPONSE_WAITING_STATUS:
                        case HTTP_RESPONSE_WAITING_BODY:
                        case HTTP_RESPONSE_RECEIVING_BODY:
                        {
                            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse - HTTP_RESPONSE_RECEIVING_BODY\n"));
                            const byte* pData = NULL;
                            unsigned int count = 0;
                            if (PAL_Ok == m_currentResponse->GetBody(&pData, &count))
                            {
                                // STEP4: Notify DATA received to application
                                NotifyHttpDataReceived(m_currentRequest->GetUserRequestData(),
                                                       PAL_Ok, pData, count);
                            }

                            *nextActionFlag = HTTP_SOCKET_READ;
                            break;
                        }

                        case HTTP_RESPONSE_COMPLETE:
                        {
                            // Done with this request

                            char* responseHeaders = NULL;
                            const byte* pData = NULL;
                            unsigned int count = 0;
                            if(m_currentResponse == NULL)
                            {
                                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("HttpConnection::ReceiveResponse - m_currentResponse == NULL"));
                            }
                            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse - HTTP_RESPONSE_COMPLETE\n"));

                            // Take ownership of the response headers
                            responseHeaders = m_currentResponse->TakeHeaders();
                            if (responseHeaders)
                            {
                                /* Notify the response headers to application. The function NotifyHttpResponseHeaders
                                               takes ownership of the string of response headers.
                                            */
                                NotifyHttpResponseHeaders(m_currentRequest->GetUserRequestData(),
                                                          responseHeaders);
                            }

                            if (PAL_Ok == m_currentResponse->GetBody(&pData, &count))
                            {
                                // STEP4: Notify DATA received to application
                                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse - Notify received data dataLen=%u\n", count));
                                NotifyHttpDataReceived(m_currentRequest->GetUserRequestData(),
                                                       PAL_Ok, pData, count);
                                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse - After notify received data\n"));
                            }
                            break;
                        }

                        default:
                            // Shouldn't be here for any reason
                            break;
                        }
                    }

                }
            }
            else if (HTTP_ErrReadAgain == err)
            {
                // Socket not yet ready for read.
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse Socket not yet ready for read \n"));
                *nextActionFlag = HTTP_SOCKET_READ;
                return;
            }
            else
            {
                // BAD socket
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::ReceiveResponse BAD socket \n"));
                m_currentResponse->SetResponseError(PAL_ErrNetGeneralFailure);
            }

            if (HTTP_RESPONSE_COMPLETE == m_currentResponse->GetState()
                    || PAL_Ok != m_currentResponse->GetResponseError())
            {
                // Does this socket need closure
                if (!m_currentResponse->IsConnectionPersistent()
                        || PAL_Ok != m_currentResponse->GetResponseError())
                {
                    // Close the socket connection
                    DestroySocketConnection();
                }
                FinishRequest(nextActionFlag);
            }
        }

        void HttpConnection::FinishRequest(int* nextActionFlag)
        {
            PAL_Error error = PAL_ErrNetGeneralFailure;
            if (m_currentResponse)
            {
                error = m_currentResponse->GetResponseError();
            }
            if (m_currentRequest)
            {
                /*
                            :WARNING:
                            Check if we should call the http data receive callback or not! If we already called the NotifyHttpResponseStatus()
                            with an error the the user of this connection must NOT receive another callback on the same connection. The
                            user might already freed any data associated with the connection and making another callback might result in
                            a crash!
                        */
                bool makeCallback = true;
                if (m_currentResponse && (m_currentResponse->GetStatusCode() != HTTP_STATUS_OK) && m_statusNotified)
                {
                    makeCallback = false;
                }

                if (makeCallback)
                {
                    // Notify data received as ZERO to notify end of request to user
                    NotifyHttpDataReceived(m_currentRequest->GetUserRequestData(), error, NULL, 0);
                }
            }

            // Discard current request and response
            if (m_currentRequest)
            {
                delete m_currentRequest;
                m_currentRequest = NULL;
            }

            if (m_currentResponse)
            {
                delete m_currentResponse;
                m_currentResponse = NULL;
            }

            // See if there are more requests on request queue
            if (NULL != (m_currentRequest = GetNextRequest()))
            {
                *nextActionFlag = HTTP_SOCKET_WRITE;
            }
            else
            {
                // No more requests in queue.
                *nextActionFlag = HTTP_NEW_REQUEST;
            }
            // Reset status
            m_statusNotified = FALSE;
        }

        /*! Notify the user of some error specified in parameter.

                    This will also clean up some previously allocated resources.

                    @return None
                */
        void  HttpConnection::NotifyErrorForAllRequests(PAL_Error error)
        {
            if (m_currentRequest)
            {
                NotifyHttpDataReceived(m_currentRequest->GetUserRequestData(),
                                       error, NULL, 0);
                delete m_currentRequest;
                m_currentRequest = NULL;
            }

            deque<HttpRequest*> requestQueue;
            pthread_mutex_lock(&m_requestQueueMutex);
            m_requestQueue.swap(requestQueue);
            pthread_mutex_unlock(&m_requestQueueMutex);

            std::deque<HttpRequest*>::iterator it = requestQueue.begin();
            for (; it != requestQueue.end(); it++)
            {
                NotifyHttpDataReceived((*it)->GetUserRequestData(),
                                       error, NULL, 0);
                delete *it;
            }

            if (m_currentResponse)
            {
                delete m_currentResponse;
                m_currentResponse = NULL;
            }
        }

        /* See header file for description */
        void HttpConnection::WorkerThreadExitCallback(PAL_Instance* pal,
                                                      HttpConnection* connection)
        {
            if (!connection)
            {
                return;
            }

            if (connection->m_workerThread)
            {
                // Wait for worker thread to exit. This call is quick. Because this function is fired when worker thread exits.
                pthread_join(connection->m_workerThread, NULL);

                close(connection->m_pipe[0]);
                close(connection->m_pipe[1]);

                connection->m_workerThread = NULL;

                connection->SetStatus(PNCS_Closed);
            }

            // Check if this connection will be destroyed.
            if (connection->m_willBeDestroyed)
            {
                connection->Destroy();
            }
        }
    }
}

/*! @} */
