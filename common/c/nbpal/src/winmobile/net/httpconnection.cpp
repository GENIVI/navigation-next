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
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Windows implementation for HttpConnection class.
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

#include "httpconnection.h"
#include "palstdlib.h"
#include "palimpl.h"
#include "paldebug.h"
#include "tchar.h"

namespace nimpal
{
    namespace network
    {
        // Worker Thread events
        #define ERROR_EVENT             0x0
        #define REQUEST_EVENT           0x00000001
        #define RESPONSE_EVENT          0x00000002
        #define CLOSE_EVENT             0x00000004
        #define CONNECTION_ERROR_EVENT  0x00000008


        // Static members of class
        LONG HttpConnection::connectionIdCounter = 0;
        HttpConnection::ConnectionCache HttpConnection::m_ConnectionCache[MAX_CONNECTIONS] = {0};
        CRITICAL_SECTION HttpConnection::m_ConnRefLock;

        static const DWORD HTTP_RECEIVE_TIMEOUT = 15*1000;
        const TCHAR INTERNET_USER_AGENT[] = _T("Mozilla/4.0 (compatible; MSIE 6.0;Windows NT 5.1)");

        //////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        //////////////////////////////////////////////////////////////////////////
        HttpConnection::HttpConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol) :
            NetConnection(pal, protocol),
            m_RequestCompletedEvent(NULL),
            m_RequestWaitingEvent(NULL),
            m_CloseEvent(NULL),
            m_NetworkErrorEvent(NULL),
            m_InternetHandle(NULL),
            m_SessionRollingIndex(0),
            m_ConnectionId(0)
        {
            memset(m_SessionHandles, 0, sizeof(m_SessionHandles));
            memset(m_RequestCache, 0, sizeof(m_RequestCache));
        }

        HttpConnection::~HttpConnection()
        {
            StopNotifies();

            if (m_RequestWaitingEvent != NULL)
            {
                CloseHandle(m_RequestWaitingEvent);
                m_RequestWaitingEvent = NULL;
            }

            if (m_RequestCompletedEvent != NULL)
            {
                CloseHandle(m_RequestCompletedEvent);
                m_RequestCompletedEvent = NULL;
            }

            if (m_CloseEvent != NULL)
            {
                CloseHandle(m_CloseEvent);
                m_CloseEvent = NULL;
            }

            if (m_NetworkErrorEvent != NULL)
            {
                CloseHandle(m_NetworkErrorEvent);
                m_NetworkErrorEvent = NULL;
            }

            if (m_Thread)
            {
                CloseHandle(m_Thread);
            }
        }

        //////////////////////////////////////////////////////////////////////////
        // public functions
        //////////////////////////////////////////////////////////////////////////

        boolean HttpConnection::InitializeConnection()
        {
            if (PNCS_Created == GetStatus())
            {
                m_RequestCompletedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                m_RequestWaitingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                m_CloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                m_NetworkErrorEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            SetStatusImmediate(PNCS_Initialized);
            return TRUE;
        }

        boolean HttpConnection::Connect(const char* pHostName, uint16 port)
        {
            PAL_NetConnectionStatus status = GetStatus();
            if ((status == PNCS_Initialized || status == PNCS_Closed) && pHostName != NULL)
            {
                // Set status to "Connecting". Status gets changed to "Connected" in worker thread.
                SetStatusImmediate(PNCS_Connecting);
                strncpy(m_HostName, pHostName, sizeof(m_HostName));
                m_HostName[sizeof(m_HostName) - 1] = '\0';

                // if non-zero value given for port, use it regardless of protocol
                if (port)
                {
                    m_Port =  port;
                }
                // if zero value given for port, use default value based on protocol
                else
                {
                    switch (m_Protocol)
                    {
                        case PNCP_HTTP:
                            m_Port = INTERNET_DEFAULT_HTTP_PORT;
                            break;

                        case PNCP_HTTPS:
                            m_Port = INTERNET_DEFAULT_HTTPS_PORT;
                            break;

                        default:
                            return FALSE;
                    }
                }
                // Add reference, before creating thread
                AddRef();
                m_Thread = CreateThread(NULL,
                                        0,
                                        (LPTHREAD_START_ROUTINE)HttpConnection::WorkerThreadProc,
                                        this,
                                        0,
                                        &m_ThreadId);
                if (!m_Thread)
                {
                    SetStatus(PNCS_Failed);

                    // Releasing here, since worker thread won't be executing.
                    Release();
                }
                return (m_Thread != NULL);
            }
            return FALSE;
        }

        boolean HttpConnection::Send(const byte* pBuffer,
                                     uint32      count,
                                     const char* pVerb,
                                     const char* pObject,
                                     const char* pAcceptType,
                                     const char* pAdditionalHeaders,
                                     void*       pRequestData)
        {
            boolean success = FALSE;
            if (GetStatus() == PNCS_Connected)
            {
                m_SessionRollingIndex = m_SessionRollingIndex % MAX_SESSIONS_PER_CONNECTION;

                // We use rolling index to determine session handle for every request, so that all sessions are
                // equally balanced with requests.
                HttpRequest* pRequest = new HttpRequest(this,
                                                        m_SessionHandles[m_SessionRollingIndex],
                                                        pBuffer,
                                                        count,
                                                        pVerb,
                                                        pObject,
                                                        pAcceptType,
                                                        pAdditionalHeaders,
                                                        pRequestData);
                // Increment session index
                m_SessionRollingIndex++;

                // Add request reference before submitting request
                if(AddRequestToCache(pRequest))
                {
                    if (PushRequest(m_RequestReadyQueue, pRequest))
                    {
                        if (SetEvent(m_RequestWaitingEvent))
                        {
                            success = TRUE;
                        }
                    }
                }
            }
            return success;
        }

        boolean HttpConnection::Send(const byte* pBuffer, int count)
        {
            return Send(pBuffer, count, NULL, NULL, NULL, NULL, NULL);
        }

        boolean HttpConnection::Close()
        {
            boolean success = FALSE;
            PAL_NetConnectionStatus status = GetStatus();

            if (status != PNCS_Created && status != PNCS_Closed)
            {
                // Signal worker thread  ( only if it is created )
                if (m_Thread)
                {
                    // At this point all external operations on this HttpConnection object are ceased
                    // Signal the worker thread to stop processing and exit
                    m_StopThread = TRUE;
                    SetEvent(m_CloseEvent);

                    // Wait for worker thread to exit
                    WaitForSingleObject(m_Thread, INFINITE);

                    // Remove connection reference
                    RemoveConnectionFromCache(this);

                    // Clear all unfinished requests
                    ClearPendingRequests(HTC_MAIN_THREAD);
                    CloseHandle(m_Thread);
                    m_Thread = NULL;
                }
                success = TRUE;
            }
            SetStatusImmediate(PNCS_Closed);
            return success;
        }

        int32 HttpConnection::GetHttpConnectionProtocol()
        {
            return m_Protocol;
        }

        void HttpConnection::NotifyHttpDataSent(const byte* pBytes, int count)
        {
            NotifyDataSent(pBytes, count);
        }

        void HttpConnection::NotifyHttpDataReceived(void* pRequestData, PAL_Error errorCode, const byte* pBytes, int count)
        {
            NetConnection::NotifyHttpDataReceived(pRequestData, errorCode, pBytes, count);
        }

        void HttpConnection::NotifyHttpResponseHeaders(void* pRequestData, char* headerString)
        {
            NetConnection::NotifyHttpResponseHeaders(pRequestData, headerString);
        }

        void HttpConnection::NotifyHttpResponseStatus(const uint32 requestStatus, void* pRequestData, uint32 contentLength)
        {
            NetConnection::NotifyHttpResponseStatus(requestStatus, pRequestData, contentLength);
        }

        //////////////////////////////////////////////////////////////////////////
        // private functions
        //////////////////////////////////////////////////////////////////////////
        boolean HttpConnection::PushRequest(queue<HttpRequest*>& rQueue, HttpRequest*& rRequest)
        {
            Lock();
            {
                rQueue.push(rRequest);
            }
            Unlock();
            return TRUE;
        }

        boolean HttpConnection::PopRequest(queue<HttpRequest*>& rQueue, HttpRequest*& rRequest)
        {
            boolean success = FALSE;
            Lock();
            {
                if (rQueue.empty())
                {
                    success = FALSE;
                    rRequest = NULL;
                }
                else
                {
                    success = TRUE;
                    rRequest = rQueue.front();
                    rQueue.pop();
                }
            }
            Unlock();
            return success;
        }

        DWORD HttpConnection::GenerateContextId(uint16 connectionId, uint16 requestIndex)
        {
            // ContextId is comprised of 2 fields
            // MSB 16 bits represent 'Connection Id'
            // LSB 16 bits represent 'Request Index'
            // This contextId is used in Internet callback to find the request object, which
            // would be used to send the response back to the user.
            return ((DWORD)(connectionId) << 16 & 0xFFFF0000) | ((DWORD)(requestIndex) & 0xFFFF);
        }

        boolean HttpConnection::AddRequestToCache(HttpRequest* pRequest)
        {
            uint16 idx = 0;
            uint16 max_requests = MAX_SESSIONS_PER_CONNECTION * MAX_REQUESTS_PER_SESSION;
            Lock();
            {
                for(; idx < max_requests; idx++)
                {
                    if (NULL == m_RequestCache[idx])
                    {
                        m_RequestCache[idx] = pRequest;
                        break;
                    }
                }
            }
            Unlock();

            if (idx == max_requests)
            {
                // No request slot available for caching
                return FALSE;
            }

            // Set callback context id for request
            DWORD contextId = GenerateContextId(m_ConnectionId, idx);
            pRequest->SetCallbackContextId(contextId);
            return TRUE;
        }

        boolean HttpConnection::RemoveRequestFromCache(HttpRequest* pRequest)
        {
            uint16 idx = 0;
            uint16 max_requests = MAX_SESSIONS_PER_CONNECTION * MAX_REQUESTS_PER_SESSION;
            Lock();
            {
                 for(; idx < max_requests; idx++)
                 {
                     if (pRequest == m_RequestCache[idx])
                     {
                         m_RequestCache[idx] = NULL;
                         break;
                     }
                 }
            }
            Unlock();
            return TRUE;
        }

        boolean HttpConnection::RemoveFirstRequestFromCache(HttpRequest*& rRequest)
        {
            uint16 idx = 0;
            uint16 max_requests = MAX_SESSIONS_PER_CONNECTION * MAX_REQUESTS_PER_SESSION;
            Lock();
            {
                for(; idx < max_requests; idx++)
                {
                    if (NULL != m_RequestCache[idx])
                    {
                        rRequest = m_RequestCache[idx];
                        m_RequestCache[idx] = NULL;
                        break;
                    }
                }
            }
            Unlock();

            if (idx == max_requests)
            {
                //No more requests in cache
                rRequest = NULL;
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }

        boolean HttpConnection::StartConnection(void)
        {
            if (EnsureNetworkConnection())
            {
                m_InternetHandle = InternetOpen(INTERNET_USER_AGENT,
                                             INTERNET_OPEN_TYPE_PRECONFIG,
                                             NULL,
                                             NULL,
                                             INTERNET_FLAG_ASYNC);

                InternetSetStatusCallback(m_InternetHandle, (INTERNET_STATUS_CALLBACK)InternetCallbackProc);

                for (uint16 i = 0; i<MAX_SESSIONS_PER_CONNECTION; i++)
                {
                    // Information from MSDN:
                    //               InternetConnect() does not attempt to access or
                    //   establish a connection to the specified site. It just creates a session
                    //   locally and returns a handle for sending requests on the session.

                    if (NULL !=  (m_SessionHandles[i] = InternetConnectA( m_InternetHandle,
                                                                     m_HostName,
                                                                     m_Port,
                                                                     NULL,   // user name
                                                                     NULL,   // password
                                                                     INTERNET_SERVICE_HTTP,
                                                                      0,      // flags
                                                                     (DWORD_PTR)1 /* just a non-NULL*/)))
                    {
                        // Set timeout value on request
                        InternetSetOption(m_SessionHandles,
                                           INTERNET_OPTION_RECEIVE_TIMEOUT,
                                           (LPVOID) &HTTP_RECEIVE_TIMEOUT,
                                           sizeof(HTTP_RECEIVE_TIMEOUT));

                    }
                    else
                    {
                        return FALSE;
                    }
                }

                // Add connection reference
                return (AddConnectionToCache(this));
            }
            return FALSE;
        }

        void HttpConnection::StopConnection()
        {
            // Close sesssions
            for (uint16 i=0; i<MAX_SESSIONS_PER_CONNECTION; i++)
            {
                if (NULL != m_SessionHandles[i])
                {
                    InternetCloseHandle(m_SessionHandles[i]);
                    m_SessionHandles[i] = NULL;
                }
            }

            // Disable callback
            InternetSetStatusCallback(m_InternetHandle, (INTERNET_STATUS_CALLBACK)NULL);

            // Close internet handle
            if (!m_InternetHandle)
            {
                InternetCloseHandle(m_InternetHandle);
                m_InternetHandle = NULL;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        // worker thread
        //////////////////////////////////////////////////////////////////////////
        void HttpConnection::WorkerThread(void)
        {
            HttpRequest* pRequest = NULL;
            DWORD        event = ERROR_EVENT;

            if (StartConnection())
            {
                SetStatus(PNCS_Connected);
                m_StopThread = FALSE;
            }
            else
            {
                // Connection Failed
                SetStatus(PNCS_Failed);
                return;
            }


            while (FALSE == m_StopThread)
            {
                // Worker Thread has 4 things to do.
                // 1) Take requests from PAL_NET ( posted on queue) and send them to server address mentioned in request
                // 2) Process responses received from WinInet stack
                // 3) Handle Connection Closure
                // 4) Handle Network Error
                // In its idle state, Worker Thread waits on events that will lead it to do one of the above 4 jobs
                event = WorkerThreadWaitForEvent();

                // It is very likely that CLOSE_EVENT is queued behind REQUEST_EVENT AND/OR RESPONSE_EVENT
                // Check the m_StopThread and if it is TRUE, Worker thread should not waste
                // any time on processing requests and should immediately do cleanup and exit.
                if (m_StopThread)
                {
                    break;
                }

                switch (event)
                {
                    case REQUEST_EVENT:
                        while (TRUE == PopRequest(m_RequestReadyQueue, pRequest))
                        {
                            // Send the request to server
                            pRequest->SendRequest();

                            // Delete request, if in ERROR state Or there's a Network Connection error.
                            if ( HRS_ErrorEncountered == pRequest->GetRequestState())
                            {
                                // Notify data received as ZERO to notify end of request to user
                                ClearRequestAndNotify(pRequest, HTC_WORKER_THREAD);
                            }
                        }
                        break;

                    case RESPONSE_EVENT:
                        while (TRUE == PopRequest(m_RequestCompleteQueue, pRequest))
                        {
                            // Dispatch for completion
                            pRequest->FinishRequest();

                            // Delete request, if in ERROR state OR 'File Download Complete' state
                            if (HRS_FileReadComplete == pRequest->GetRequestState())
                            {
                                // Notify data received as ZERO to notify end of request to user
                                ClearRequestAndNotify(pRequest, HTC_WORKER_THREAD);
                            }
                            else if (HRS_ErrorEncountered == pRequest->GetRequestState())
                            {
                                // On error case just remove the request
                                RemoveRequestFromCache(pRequest);
                                delete pRequest;
                            }
                        }
                        break;

                    case CONNECTION_ERROR_EVENT:
                        // Remove connection reference
                        // After removal of connection reference, all responses in Internet callback for this
                        // connection will be rejected (meaning they won't be processed)
                        RemoveConnectionFromCache(this);

                        // Clear all unfinished requests
                        ClearPendingRequests(HTC_WORKER_THREAD);
                        SetStatus(PNCS_Error);
                        break;

                    case CLOSE_EVENT:
                        m_StopThread = TRUE;
                        break;

                    default:
                        nsl_assert(FALSE);
                        break;
                }
            }

            // Unregister with Connection manager for network error notification
            RegisterForNetworkErrorEvent(FALSE, m_NetworkErrorEvent);

            // Stop the connection
            StopConnection();
        }

        int HttpConnection::WorkerThreadWaitForEvent(void)
        {
             HANDLE waitObjects[] = {m_RequestWaitingEvent, m_RequestCompletedEvent, m_CloseEvent, m_NetworkErrorEvent};
             int    objectCount = sizeof(waitObjects)/sizeof(HANDLE);
             int    eventId;

              // Register for network error event
             if (TRUE != RegisterForNetworkErrorEvent(TRUE, m_NetworkErrorEvent))
             {
                 objectCount--; // There won't be any network error event on this connection (for Win32)
             }

             DWORD ret = WaitForMultipleObjects( objectCount, waitObjects, FALSE, INFINITE);
             switch(ret)
             {
                 case WAIT_OBJECT_0:
                     eventId = REQUEST_EVENT;
                     break;

                 case WAIT_OBJECT_0 + 1:
                     eventId = RESPONSE_EVENT;
                     break;

                 case WAIT_OBJECT_0 + 2:
                     eventId = CLOSE_EVENT;
                     break;

                 case WAIT_OBJECT_0 + 3:
                     eventId = CONNECTION_ERROR_EVENT;
                     break;

                 default:
                     // Shouldn't be here for any reason
                     nsl_assert(FALSE);
                     eventId = ERROR_EVENT;
                     break;
             }
             return eventId;
        }


        void HttpConnection::ClearRequestAndNotify(HttpRequest* pRequest, HttpThreadContext context)
        {
            PAL_Error errorCode;
            DWORD32   winInetError = pRequest->GetWinInetError();
            void*     pRequestData = pRequest->GetRequestData();
 
            // Associate WinInet errors to PAL errors
            switch(winInetError)
            {
                case ERROR_SUCCESS:
                    errorCode = PAL_Ok;
                    break; 

                case ERROR_INTERNET_TIMEOUT:
                    errorCode = PAL_ErrNetTimeout;
                    break;

                case ERROR_INTERNET_INVALID_URL:
                    errorCode = PAL_ErrNetDnsUnknownHostName;
                    break;

                default:
                    errorCode = PAL_ErrNetGeneralFailure;
                    break;
            } 

            if (HTC_WORKER_THREAD == context)
            {
                NotifyHttpDataReceived(pRequestData, errorCode, NULL, 0);
            }
            else
            {
                NotifyHttpDataReceivedImmediate(pRequestData, errorCode, NULL, 0);
            }
            RemoveRequestFromCache(pRequest);
            delete pRequest;
        }

        void HttpConnection::ClearPendingRequests(HttpThreadContext context)
        {
            // Clear all requests from cache
            HttpRequest *pRequest = NULL;
            while (RemoveFirstRequestFromCache(pRequest))
            {
                if (pRequest)
                {
                    // All pending requests are assumed to be in cancelled state.
                    pRequest->SetWinInetError(ERROR_INTERNET_OPERATION_CANCELLED);
                    ClearRequestAndNotify(pRequest, context);
                }
            }

            // Clear Ready queue
            while (PopRequest(m_RequestReadyQueue, pRequest));

            // Clear Complete queue
            while (PopRequest(m_RequestCompleteQueue, pRequest));
        }

        //////////////////////////////////////////////////////////////////////////
        // static functions
        //////////////////////////////////////////////////////////////////////////

        DWORD HttpConnection::WorkerThreadProc(LPVOID pParameter)
        {
            HttpConnection* pThis = static_cast<HttpConnection*>(pParameter);
            nsl_assert(pThis != NULL);

            pThis->WorkerThread();
            pThis->Release();

            return (DWORD)1;
        }


        boolean HttpConnection::GetRequestFromContext(DWORD            context,
                                                      HttpConnection*& rConnection,
                                                      HttpRequest*&    rRequest)
        {
            uint16  connectionId = GetConnectionId(context);
            uint16  requestIdx   = GetRequestIndex(context);
            uint16  i = 0;

            if (!connectionId)
            {
                // Invalid Context
                return FALSE;
            }

            ConnectionRefLock();
            for(; i < MAX_CONNECTIONS; i++)
            {
                if (connectionId == m_ConnectionCache[i].connectionId)
                {
                    rConnection = m_ConnectionCache[i].connection;
                    break;
                }
            }
            ConnectionRefUnLock();

            if (i == MAX_CONNECTIONS)
            {
                // Connection reference not found
                rConnection = NULL;
                rRequest = NULL;
                return FALSE;
            }

            // Find request reference
            nsl_assert(rConnection);
            rConnection->Lock();
            rRequest = rConnection->m_RequestCache[requestIdx];
            rConnection->Unlock();

            if (!rRequest)
            {
                // Invalid request Index
                return FALSE;
            }
            return TRUE;
        }


        boolean HttpConnection::AddConnectionToCache(HttpConnection* pConnection)
        {
            uint16 i = 0;

            // Allocate connection Id to this connection.
            if (1 == (pConnection->m_ConnectionId = (uint16) InterlockedIncrement(&connectionIdCounter)))
            {
                // Initialize the critical section (Only the first time, when connection id is 1))
                InitializeCriticalSection(&m_ConnRefLock);
            }

            ConnectionRefLock();
            for(; i < MAX_CONNECTIONS; i++)
            {
                if (0 == m_ConnectionCache[i].connectionId)
                {
                    m_ConnectionCache[i].connectionId = pConnection->m_ConnectionId;
                    m_ConnectionCache[i].connection = pConnection;
                    break;
                }
            }
            ConnectionRefUnLock();

            if ( i == MAX_CONNECTIONS)
            {
                //Empty slot not found
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }

        boolean HttpConnection::RemoveConnectionFromCache(HttpConnection* pConnection)
        {
            uint16 i = 0;
            ConnectionRefLock();
            for(; i < MAX_CONNECTIONS; i++)
            {
                if (pConnection->m_ConnectionId == m_ConnectionCache[i].connectionId)
                {
                    m_ConnectionCache[i].connectionId = 0;
                    m_ConnectionCache[i].connection = NULL;
                    break;
                }
            }
            ConnectionRefUnLock();

            if ( i == MAX_CONNECTIONS)
            {
                // Connection reference not found
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }

        //////////////////////////////////////////////////////////////////////////
        // internet callback
        //////////////////////////////////////////////////////////////////////////

        void HttpConnection::InternetCallbackProc(HINTERNET handle,
                                                  DWORD_PTR context,
                                                  DWORD status,
                                                  LPVOID pStatusInfo,
                                                  DWORD statusInfoLen)
        {
            HttpConnection*               pConnection = NULL;
            HttpRequest*                  pRequest    = NULL;

/*
    Only enable when debuggin HTTP. Otherwise it is just too much information
*/
//#ifdef DEBUG
//            DebugOutputInternetCallbackStatus(handle, status, pStatusInfo, statusInfoLen);
//#endif
            switch (status)
            {
                case INTERNET_STATUS_HANDLE_CREATED:
                    break;

                case INTERNET_STATUS_REQUEST_COMPLETE:
                    // Request completed
                    if (TRUE != GetRequestFromContext((DWORD)context, pConnection, pRequest))
                    {
                        // Invalid Context ( HTTP Connection might have been closed )
                        return;
                    }
                    nsl_assert(pConnection != NULL);
                    nsl_assert(pRequest != NULL);

                    // Set the WinInet error for request
                    pRequest->SetWinInetError(((INTERNET_ASYNC_RESULT *)pStatusInfo)->dwError);

                    // Push the 'Request Object' to 'Request Complete Queue'
                    pConnection->PushRequest(pConnection->m_RequestCompleteQueue, pRequest);

                    // Trigger the event so that Wait loop receives messages on 'Request Complete Queue'
                    SetEvent(pConnection->m_RequestCompletedEvent);
                    break;

                case INTERNET_STATUS_HANDLE_CLOSING:
                    break;

                default:
                    break;
            }
        }

#ifdef DEBUG
       void HttpConnection::DebugOutputInternetCallbackStatus(HINTERNET handle,
                                                              DWORD status,
                                                              LPVOID pStatusInfo,
                                                              DWORD statusInfoLen)
        {
            INTERNET_ASYNC_RESULT* pRes = (INTERNET_ASYNC_RESULT *)pStatusInfo;

            TCHAR debugMsg[80];
            TCHAR debugOut[80];

            switch (status)
            {
                case INTERNET_STATUS_HANDLE_CREATED:
                    _stprintf(debugMsg, _T("CONNECT HANDLE_CREATED (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_REQUEST_COMPLETE:
                    _stprintf(debugMsg, _T("REQUEST_COMPLETE (%d)"), pRes->dwResult);
                    break;
                case INTERNET_STATUS_RESPONSE_RECEIVED:
                    _stprintf(debugMsg, _T("RESPONSE_RECEIVED (%d)"), *(LPDWORD)pStatusInfo);
                    break;
                case INTERNET_STATUS_COOKIE_RECEIVED:
                    _stprintf(debugMsg, _T("COOKIE_RECEIVED (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_DETECTING_PROXY:
                    _stprintf(debugMsg, _T("DETECTING_PROXY (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_CLOSING_CONNECTION:
                    _stprintf(debugMsg, _T("CLOSING_CONNECTION (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_CONNECTED_TO_SERVER:
                    _stprintf(debugMsg, _T("CONNECTED_TO_SERVER (%s)"), (LPSTR) pStatusInfo);
                    break;
                case INTERNET_STATUS_CONNECTING_TO_SERVER:
                    _stprintf(debugMsg, _T("CONNECTING_TO_SERVER (%s)"), (LPSTR) pStatusInfo);
                    break;
                case INTERNET_STATUS_CONNECTION_CLOSED:
                    _stprintf(debugMsg, _T("CONNECTION_CLOSED (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_HANDLE_CLOSING:
                    _stprintf(debugMsg, _T("HANDLE_CLOSING (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
                    _stprintf(debugMsg, _T("INTERMEDIATE_RESPONSE (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_USER_INPUT_REQUIRED:
                    _stprintf(debugMsg, _T("USER_INPUT_REQUIRED (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_NAME_RESOLVED:
                    _stprintf(debugMsg, _T("NAME_RESOLVED (%s)"), (LPCSTR)pStatusInfo);
                    break;
                case INTERNET_STATUS_RECEIVING_RESPONSE:
                    _stprintf(debugMsg, _T("RECEIVING_RESPONSE (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
                    _stprintf(debugMsg, _T("CTL_RESPONSE_RECEIVED (%d)"), *(LPDWORD)pStatusInfo);
                    break;
                case INTERNET_STATUS_PREFETCH:
                    _stprintf(debugMsg, _T("PREFETCH (%d)"), *(LPDWORD)pStatusInfo);
                    break;
                case INTERNET_STATUS_REDIRECT:
                    _stprintf(debugMsg, _T("REDIRECT (%s)"), (LPCTSTR)pStatusInfo);
                    break;
                case INTERNET_STATUS_REQUEST_SENT:
                    _stprintf(debugMsg, _T("REQUEST_SENT (%d)"), *(LPDWORD)pStatusInfo);
                    break;
                case INTERNET_STATUS_RESOLVING_NAME:
                    _stprintf(debugMsg, _T("RESOLVING_NAME (%s)"), (LPCSTR)pStatusInfo);
                    break;
                case INTERNET_STATUS_SENDING_REQUEST:
                    _stprintf(debugMsg, _T("SENDING_REQUEST (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_STATE_CHANGE:
                    _stprintf(debugMsg, _T("STATE_CHANGE (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_COOKIE_SENT:
                    _stprintf(debugMsg, _T("COOKIE_SENT (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_PRIVACY_IMPACTED:
                    _stprintf(debugMsg, _T("PRIVACY_IMPACTED (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_P3P_HEADER:
                    _stprintf(debugMsg, _T("P3P_HEADER (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_P3P_POLICYREF:
                    _stprintf(debugMsg, _T("P3P_POLICYREF (%d)"), statusInfoLen);
                    break;
                case INTERNET_STATUS_COOKIE_HISTORY:
                    _stprintf(debugMsg, _T("COOKIE_HISTORY (%d)"), statusInfoLen);
                    break;
                default:
                    _stprintf(debugMsg, _T("Unknown: Status %d Given"), status);
                    break;
            }
            _stprintf(debugOut, _T("%s\n"), debugMsg);
            ::OutputDebugString(debugOut);
        }
#endif

    }
}
