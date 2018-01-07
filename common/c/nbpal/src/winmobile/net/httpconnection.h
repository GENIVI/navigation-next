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

    @file     httpconnection.h
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for PAL HTTP network connections.

    Windows implementation for PAL HTTP network functions.
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

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "netconnection.h"
#include "httprequest.h"

#include <wininet.h>

#include <string>
#include <queue>

using namespace std;


namespace nimpal
{
    namespace network
    {
        static const uint16 MAX_CONNECTIONS = 40;
        static const uint16 MAX_SESSIONS_PER_CONNECTION = 10;
        static const uint16 MAX_REQUESTS_PER_SESSION = 1;

        class HttpConnection : public NetConnection
        {
        public:
            HttpConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol);
            virtual ~HttpConnection();

            virtual boolean InitializeConnection();

            virtual boolean Connect(const char* pHostName, uint16 port);
            virtual boolean Send(const byte* pBuffer, int count);
            virtual boolean Close();

            boolean Send(const byte* pBuffer,
                         uint32      count,
                         const char* pVerb,
                         const char* pObject,
                         const char* pAcceptType,
                         const char* pAdditionalHeaders,
                         void*       pRequestData);

            int32 GetHttpConnectionProtocol();

            void NotifyHttpDataSent(const byte* pBytes, int count);
            void NotifyHttpDataReceived(void* pRequestData, PAL_Error errorCode, const byte* pBytes, int count);
            void NotifyHttpResponseStatus(const uint32 requestStatus, void* pRequestData, uint32 contentLength);
            void NotifyHttpResponseHeaders(void* pRequestData, char* headerString);

         private:
            typedef enum{
                HTC_MAIN_THREAD = 0,
                HTC_WORKER_THREAD,
            }HttpThreadContext;

            boolean StartConnection();
            void    StopConnection();
            boolean PushRequest(queue<HttpRequest*>& rQueue, HttpRequest*& rRequest);
            boolean PopRequest(queue<HttpRequest*>& rQueue, HttpRequest*& rRequest);
            void    ClearRequestAndNotify(HttpRequest* pRequest, HttpThreadContext context);
            void    ClearPendingRequests(HttpThreadContext context);
            boolean AddRequestToCache(HttpRequest*    pRequest);
            boolean RemoveRequestFromCache(HttpRequest* pRequest);
            boolean RemoveFirstRequestFromCache(HttpRequest*& rRequest);

            void WorkerThread(void);
            int  WorkerThreadWaitForEvent(void);

            HINTERNET           m_InternetHandle;
            HINTERNET           m_SessionHandles[MAX_SESSIONS_PER_CONNECTION];
            uint16              m_SessionRollingIndex;

            HANDLE              m_RequestWaitingEvent;
            HANDLE              m_RequestCompletedEvent;
            HANDLE              m_CloseEvent;
            HANDLE              m_NetworkErrorEvent;

            queue<HttpRequest*> m_RequestReadyQueue;
            queue<HttpRequest*> m_RequestCompleteQueue;
            uint16              m_ConnectionId;
            HttpRequest*        m_RequestCache[MAX_SESSIONS_PER_CONNECTION*MAX_REQUESTS_PER_SESSION];

            // Static members
            class ConnectionCache
            {
            public:
                uint16          connectionId;
                HttpConnection* connection;
            };

            static CRITICAL_SECTION                m_ConnRefLock;
            static LONG            connectionIdCounter;
            static ConnectionCache m_ConnectionCache[MAX_CONNECTIONS];

            static void    ConnectionRefLock() { EnterCriticalSection(&m_ConnRefLock); };
            static void    ConnectionRefUnLock() { LeaveCriticalSection(&m_ConnRefLock); };
            static DWORD   GenerateContextId(uint16 connectionId, uint16 requestIndex);
            static uint16  GetRequestIndex(DWORD contextId) { return LOWORD(contextId);};
            static uint16  GetConnectionId(DWORD contextId) { return HIWORD(contextId);};
            static boolean AddConnectionToCache(HttpConnection* pConnection);
            static boolean RemoveConnectionFromCache(HttpConnection* pConnection);
            static boolean GetRequestFromContext(DWORD             context,
                                                  HttpConnection*&  rConnection,
                                                  HttpRequest*&     rRequest);

            static void __stdcall InternetCallbackProc(HINTERNET handle, DWORD_PTR context, DWORD status, LPVOID pStatusInfo, DWORD statusInfoLen);
            static DWORD __stdcall WorkerThreadProc(LPVOID pParameter);
#ifdef DEBUG
            static void DebugOutputInternetCallbackStatus(HINTERNET handle, DWORD status, LPVOID pStatusInfo, DWORD statusInfoLen);
#endif
        };
    }
}

#endif
