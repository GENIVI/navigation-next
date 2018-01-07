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

    @file     netconnection.cpp
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Windows implementation of NetConnection base class.
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

#ifdef WINCE
#include "connectionmanager.h"
#endif

#include "palstdlib.h"
#include "palimpl.h"
#include "netconnection.h"
#include "finishfunctionnetworkdata.h"
#include "finishfunctionnetworkstatus.h"
#include "finishfunctionhttpstatus.h"
#include "finishfunctionhttpdata.h"
#include "finishfunctiondnsresult.h"
#include "finishfunctionhttpresponseheaders.h"
#include "paldebug.h"

namespace nimpal
{
    namespace network
    {
        //////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        //////////////////////////////////////////////////////////////////////////

        NetConnection::NetConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol) :
            m_Status(PNCS_Created),
            m_Protocol(protocol),
            m_Port(0),
            m_Thread(NULL),
            m_ThreadId(0),
            m_StopThread(TRUE),
            m_pStatusCallback(NULL),
            m_pStatusCallbackData(NULL),
            m_pDataSentCallback(NULL),
            m_pDataSentCallbackData(NULL),
            m_pDataReceivedCallback(NULL),
            m_pDataReceivedCallbackData(NULL),
            m_pDnsResultCallback(NULL),
            m_pHttpResponseStatusCallback(NULL),
            m_pHttpResponseStatusCallbackData(NULL),
            m_pHttpDataReceivedCallback(NULL),
            m_pHttpDataReceivedCallbackData(NULL),
            m_finishFunctionOwnerId(0),
            m_referenceCount(1),
            m_pal(pal)
        {
            m_HostName[0] = 0;
            m_finishFunctionOwnerId = m_pal->functionManager->RegisterOwner();
            InitializeCriticalSection(&m_Lock);
        }

        NetConnection::~NetConnection()
        {
            DeleteCriticalSection(&m_Lock);
        }

        //////////////////////////////////////////////////////////////////////////
        // public functions
        //////////////////////////////////////////////////////////////////////////
        boolean NetConnection::EnsureNetworkConnection()
        {
#ifdef WINCE
            return ConnectionManager::EstablishConnection();
#else
            return TRUE;
#endif
        }

        boolean NetConnection::RegisterForNetworkErrorEvent(boolean enable, HANDLE errorEvent)
        {
#ifdef WINCE
            return ConnectionManager::RegisterForConnectionErrorEvent(enable, errorEvent);
#else
            return FALSE; 
#endif
        }

        void NetConnection::AddRef()
        {
            InterlockedIncrement(&m_referenceCount);
        }

        void NetConnection::Release()
        {
            if ( 0 == InterlockedDecrement(&m_referenceCount))
            {
                delete this;
            }
        }

        boolean NetConnection::InitializeConnection()
        {
            return TRUE;
        }

        void NetConnection::SetStatusCallback(PAL_NetConnectionStatusCallback pStatusCallback, void* pData)
        {
            m_pStatusCallback = pStatusCallback;
            m_pStatusCallbackData = pStatusCallback == NULL ? NULL : pData;
        }

        void NetConnection::SetDataSentCallback(PAL_NetDataSentCallback pSentCallback, void* pData)
        {
            m_pDataSentCallback = pSentCallback;
            m_pDataSentCallbackData = pSentCallback == NULL ? NULL : pData;
        }

        void NetConnection::SetDataReceivedCallback(PAL_NetDataReceivedCallback pReceivedCallback, void* pData)
        {
            m_pDataReceivedCallback = pReceivedCallback;
            m_pDataReceivedCallbackData = pReceivedCallback == NULL ? NULL : pData;
        }

        void NetConnection:: SetDnsResultCallback(PAL_NetDnsResultCallback pDnsResultCallback, void* pData)
        {
            m_pDnsResultCallback = pDnsResultCallback;
            m_pDnsResultCallbackData = pDnsResultCallback == NULL ? NULL : pData;
        }

        void NetConnection:: SetHttpResponseStatusCallback(PAL_NetHttpResponseStatusCallback pResponseStatusCallback, void* pData)
        {
            m_pHttpResponseStatusCallback = pResponseStatusCallback;
            m_pHttpResponseStatusCallbackData = pResponseStatusCallback == NULL ? NULL : pData;
        }

        void NetConnection:: SetHttpDataReceivedCallback(PAL_NetHttpDataReceivedCallback pDataReceivedCallback, void* pData)
        {
            m_pHttpDataReceivedCallback = pDataReceivedCallback;
            m_pHttpDataReceivedCallbackData = pDataReceivedCallback == NULL ? NULL : pData;
        }

        /* See header file for description */
        void NetConnection::SetHttpResponseHeadersCallback(
            PAL_NetHttpResponseHeadersCallback pHttpResponseHeadersCallback, void* pData)
        {
            m_pHttpResponseHeadersCallback     = pHttpResponseHeadersCallback;
            m_pHttpResponseHeadersCallbackData = pData;
        }

        void NetConnection::SetTLSConfig(PAL_NetTLSConfig *tconfp)
        {
            COPY_TLSCONFIG(m_tlsConfig, *tconfp);
        }

        PAL_NetConnectionStatus NetConnection::GetStatus()
        {
            return m_Status;
        }

        //////////////////////////////////////////////////////////////////////////
        // protected functions
        //////////////////////////////////////////////////////////////////////////

        void NetConnection::SetStatus(PAL_NetConnectionStatus status)
        {
            if (m_Status != status)
            {
                m_Status = status;

                if (m_pStatusCallback != NULL )
                {
                    /// @todo What should we do if the new fails?  Crashing app is probably best since the
                    /// network is essentially non functional at this point
                    FinishFunctionNetworkStatus* function = new FinishFunctionNetworkStatus(m_pStatusCallback, m_Status, m_pStatusCallbackData);
                    ScheduleNotify(function);
                }
            }
        }

        void NetConnection::SetStatusImmediate(PAL_NetConnectionStatus status)
        {
            if (m_Status != status)
            {
                m_Status = status;

                if (m_pStatusCallback != NULL )
                {
                    (m_pStatusCallback)(m_pStatusCallbackData, m_Status);
                }
            }
        }

        void NetConnection::NotifyDataSent(const byte* pBytes, int count)
        {
            if (m_pDataSentCallback != NULL)
            {
                FinishFunctionNetworkData* function = new FinishFunctionNetworkData(m_pDataSentCallback, pBytes, count, m_pDataSentCallbackData);
                ScheduleNotify(function);
            }
        }

        void NetConnection::NotifyDataReceived(const byte* pBytes, int count)
        {
            if (m_pDataReceivedCallback != NULL)
            {
                FinishFunctionNetworkData* function = new FinishFunctionNetworkData(m_pDataReceivedCallback, pBytes, count, m_pDataReceivedCallbackData);
                ScheduleNotify(function);
            }
        }

        void NetConnection::NotifyHttpResponseStatus(const uint32 status, void* requestData, uint32 contentLength)
        {
            if (m_pHttpResponseStatusCallback != NULL)
            {
                FinishFunctionHttpStatus* function = new FinishFunctionHttpStatus(m_pHttpResponseStatusCallback, status, m_pHttpResponseStatusCallbackData, requestData, contentLength);
                ScheduleNotify(function);
            }
        }

        void NetConnection::NotifyHttpDataReceived(void* requestData, PAL_Error errorCode, const byte* pBytes, int count)
        {
            if (m_pHttpDataReceivedCallback != NULL)
            {
                FinishFunctionHttpData* function = new FinishFunctionHttpData(m_pHttpDataReceivedCallback, requestData, errorCode, pBytes, count, m_pHttpDataReceivedCallbackData);
                ScheduleNotify(function);
            }
        }

        /* See header file for description */
        void NetConnection::NotifyHttpResponseHeaders(void* pRequestData, char* responseHeaders)
        {
            if (m_pHttpResponseHeadersCallback != NULL)
            {
                // Take ownership of the string of response headers in the constructor.
                FinishFunctionHttpResponseHeaders *function =
                    new FinishFunctionHttpResponseHeaders(m_pHttpResponseHeadersCallback,
                                                          (void*) m_pHttpResponseHeadersCallbackData,
                                                          pRequestData,
                                                          responseHeaders);

                if (function)
                {
                    /* The string of response headers has been taken ownership of the created
                       CallbackHttpResponseHeaders object.
                    */
                    responseHeaders = NULL;

                    ScheduleNotify(function);
                }
            }

            // Free the string of response headers if it is not taken ownership.
            if (responseHeaders)
            {
                nsl_free(responseHeaders);
                responseHeaders = NULL;
            }
        }

        void NetConnection::NotifyHttpDataReceivedImmediate(void* requestData, PAL_Error errorCode, const byte* pBytes, int count)
        {
            if (m_pHttpDataReceivedCallback != NULL)
            {
                (m_pHttpDataReceivedCallback) (m_pHttpDataReceivedCallbackData, requestData, errorCode, pBytes, count);
            }
        }

        void NetConnection::NotifyDnsResult(PAL_Error errorCode, const char** addressList, short addressLength)
        {
            if (m_pDnsResultCallback != NULL)
            {
                FinishFunctionDnsResult* function = new FinishFunctionDnsResult(m_pDnsResultCallback, errorCode, m_HostName, addressList, addressLength, m_pDnsResultCallbackData);
                ScheduleNotify(function);
            }
        }

        void NetConnection::ScheduleNotify(FinishFunctionBase* function)
        {
            Lock();
            if (m_finishFunctionOwnerId != 0)
            {
                m_pal->functionManager->Schedule(m_finishFunctionOwnerId, function);
            }
            else
            {
                delete function;
            }
            Unlock();
        }

        void NetConnection::StopNotifies()
        {
            Lock();
            uint32 ownerId = m_finishFunctionOwnerId;
            m_finishFunctionOwnerId = 0;
            Unlock();

            m_pal->functionManager->DeleteScheduledFunctions(ownerId);
        }
    }
}
