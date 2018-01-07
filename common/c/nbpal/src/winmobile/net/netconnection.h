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

    @file     netconnection.h
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Base class for PAL network connections.

    Windows implementation for PAL base network function class.
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

#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include "bq.h"
#include "palnet.h"

#include "windows.h"
#include "nbutil.h"
#include "finishfunctionbase.h"

namespace nimpal
{
    namespace network
    {
        #define HOSTNAME_MAX_LEN 255

        class NetConnection
        {
        public:
            NetConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol);

            void AddRef();
            void Release();

            virtual boolean InitializeConnection();

            void SetStatusCallback(PAL_NetConnectionStatusCallback pStatusCallback, void* pData);
            void SetDataSentCallback(PAL_NetDataSentCallback pDataSentCallback, void* pData);
            void SetDataReceivedCallback(PAL_NetDataReceivedCallback pDataReceivedCallback, void* pData);
            void SetDnsResultCallback(PAL_NetDnsResultCallback pDnsResultCallback, void* pData);
            void SetHttpResponseStatusCallback(PAL_NetHttpResponseStatusCallback pResponseStatusCallback, void* pData);
            void SetHttpDataReceivedCallback(PAL_NetHttpDataReceivedCallback pDataReceivedCallback, void* pData);

            /*! Set the callback and user data for HTTP response headers

              @return None
            */
            void SetHttpResponseHeadersCallback(
                PAL_NetHttpResponseHeadersCallback pHttpResponseHeadersCallback,
                void* pData);

            void SetTLSConfig(PAL_NetTLSConfig *);

            virtual PAL_NetConnectionStatus GetStatus();

            virtual boolean Connect(const char* pHostName, uint16 port) = 0;
            virtual boolean Send(const byte* pBuffer, int count) = 0;
            virtual boolean Close() = 0;


        protected:
            virtual ~NetConnection();

            /*! Set the current connection status.  Call is marshaled back to PAL thread */
            void SetStatus(PAL_NetConnectionStatus status);
            /*! Set the current connection status by directly invoking the callback.  Must be used by PAL thread */
            void SetStatusImmediate(PAL_NetConnectionStatus status);
            /*! Callback that socket data was sent.  Call is marshaled back to PAL thread */
            void NotifyDataSent(const byte* pBytes, int count);
            /*! Callback that socket data was received.  Call is marshaled back to PAL thread */
            void NotifyDataReceived(const byte* pBytes, int count);
            /*! Callback to notify DNS result to PAL thread */
            void NotifyDnsResult(PAL_Error errorCode, const char** addressList, short addressLength);
            /*! HTTP network specific callback to notify http request status back to PAL thread */
            void NotifyHttpResponseStatus(const uint32 requestStatus, void* requestData, uint32 contentLength);
            /*! HTTP network specific callback that data was received.  Call is marshaled back to PAL thread */
            void NotifyHttpDataReceived(void* requestData, PAL_Error errorCode, const byte* pBytes, int count);
            /*! HTTP network specific callback that data was received.  This is immediate call in PAL thread  */
            void NotifyHttpDataReceivedImmediate(void* requestData, PAL_Error errorCode, const byte* pBytes, int count);

            /*! Notify the HTTP response headers

              This function takes ownership of the parameter responseHeaders.

              @return None
            */
            void NotifyHttpResponseHeaders(void* pRequestData, char* responseHeaders);
            /*! Disable PAL thread marshaling and remove any pending cross thread calls */
            void StopNotifies();

            /*! This function makes connection with connection manager, if already not connected */
            static  boolean EnsureNetworkConnection();
            boolean RegisterForNetworkErrorEvent(boolean enable, HANDLE errorEvent);

            void Lock() { EnterCriticalSection(&m_Lock); };
            void Unlock() { LeaveCriticalSection(&m_Lock); };

            PAL_NetConnectionStatus            m_Status;

            PAL_NetConnectionProtocol          m_Protocol;
            char                               m_HostName[HOSTNAME_MAX_LEN];
            uint16                             m_Port;

            HANDLE                             m_Thread;
            DWORD                              m_ThreadId;
            boolean                            m_StopThread;

            PAL_NetConnectionStatusCallback    m_pStatusCallback;
            void*                              m_pStatusCallbackData;

            PAL_NetDataSentCallback            m_pDataSentCallback;
            void*                              m_pDataSentCallbackData;

            PAL_NetDataReceivedCallback        m_pDataReceivedCallback;
            void*                              m_pDataReceivedCallbackData;

            PAL_NetDnsResultCallback           m_pDnsResultCallback;
            void*                              m_pDnsResultCallbackData;

            PAL_NetHttpResponseStatusCallback  m_pHttpResponseStatusCallback;
            void*                              m_pHttpResponseStatusCallbackData;

            PAL_NetHttpDataReceivedCallback    m_pHttpDataReceivedCallback;
            void*                              m_pHttpDataReceivedCallbackData;

            PAL_NetHttpResponseHeadersCallback m_pHttpResponseHeadersCallback;     /*!< Callback for HTTP response headers */
            void*                              m_pHttpResponseHeadersCallbackData; /*!< User data of callback for HTTP response headers */

            PAL_NetTLSConfig                   m_tlsConfig;


        private:
            /*! Schedule a PAL thread call as long as they are enabled */
            void                            ScheduleNotify(FinishFunctionBase* function);

            LONG                            m_referenceCount;
            uint32                          m_finishFunctionOwnerId;

            CRITICAL_SECTION                m_Lock;
            PAL_Instance*                   m_pal;
        };
    }
}

#endif
