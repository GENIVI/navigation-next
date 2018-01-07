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
    @date     3/15/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    iPhone implementation of NetConnection base class.
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

/* all the file's code goes here */
#include <cstddef>  /* NULL */
#include "palstdlib.h"
#include "netconnection.h"
#include "callbacknetworkstatus.h"
#include "callbacknetworkdata.h"
#include "callbackhttpdata.h"
#include "callbackhttpresponsestatus.h"
#include "callbackdnsresult.h"
#include "callbackhttpresponseheaders.h"
#include <cstdio>
#include <netinet/in.h>
#include <SystemConfiguration/SystemConfiguration.h>

namespace nimpal
{
    namespace network
    {
        NetConnection::NetConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal) :
            CallbackManager(pal),
            m_status(PNCS_Created),
            m_protocol(protocol),
            m_portNumber(0),
            m_localIpAddr(NULL),
            m_pStatusCallback(NULL),
            m_pStatusCallbackData(NULL),
            m_pDataSentCallback(NULL),
            m_pDataSentCallbackData(NULL),
            m_pDataReceivedCallback(NULL),
            m_pDataReceivedCallbackData(NULL),
            m_pHttpDataReceivedCallback(NULL),
            m_pHttpDataReceivedCallbackData(NULL),
            m_pHttpResponseStatusCallback(NULL),
            m_pHttpResponseStatusCallbackData(NULL),
            m_pDnsResultCallback(NULL),
            m_pDnsResultCallbackData(NULL),
            m_pHttpResponseHeadersCallback(NULL),
            m_pHttpResponseHeadersCallbackData(NULL)
        {
            m_hostName[0] = 0;
            m_portString[0] = 0;

            m_tlsConfig.name = NULL;
            m_tlsConfig.override = 0;
        }

        NetConnection::~NetConnection()
        {
            if(m_localIpAddr)
            {
                nsl_free(m_localIpAddr);
                m_localIpAddr = NULL;
            }

            if (m_tlsConfig.name)
            {
                nsl_free((void*)m_tlsConfig.name);
                m_tlsConfig.name = NULL;
            }
        }

        void NetConnection::SetStatusCallback(
                PAL_NetConnectionStatusCallback pStatusCallback, void* pData)
        {
            m_pStatusCallback = pStatusCallback;
            m_pStatusCallbackData = pData;
        }

        void NetConnection::SetDataSentCallback(
                PAL_NetDataSentCallback pSentCallback, void* pData)
        {
            m_pDataSentCallback = pSentCallback;
            m_pDataSentCallbackData = pData;
        }

        void NetConnection::SetDataReceivedCallback(
                PAL_NetDataReceivedCallback pReceivedCallback, void* pData)
        {
            m_pDataReceivedCallback = pReceivedCallback;
            m_pDataReceivedCallbackData = pData;
        }

        void NetConnection::SetHttpDataReceivedCallback(
                PAL_NetHttpDataReceivedCallback pHttpReceivedCallback, void* pData)
        {
            m_pHttpDataReceivedCallback = pHttpReceivedCallback;
            m_pHttpDataReceivedCallbackData = pData;
        }

        void NetConnection::SetHttpResponseStatusCallback(
                PAL_NetHttpResponseStatusCallback pHttpResponseStatusCallback, void* pData)
        {
            m_pHttpResponseStatusCallback = pHttpResponseStatusCallback;
            m_pHttpResponseStatusCallbackData = pData;
        }

        void NetConnection::SetDnsResultCallback(
                PAL_NetDnsResultCallback pDnsResultCallback, void* pData)
        {
            m_pDnsResultCallback = pDnsResultCallback;
            m_pDnsResultCallbackData = pData;
        }

        /* See header file for description */
        void NetConnection::SetHttpResponseHeadersCallback(
                PAL_NetHttpResponseHeadersCallback pHttpResponseHeadersCallback, void* pData)
        {
            m_pHttpResponseHeadersCallback = pHttpResponseHeadersCallback;
            m_pHttpResponseHeadersCallbackData = pData;
        }

        void NetConnection::SetTLSConfig(
                const PAL_NetTLSConfig& tlsConfig)
        {
            if (tlsConfig.name)
            {
                if (m_tlsConfig.name)
                {
                    nsl_free((void*)m_tlsConfig.name);
                    m_tlsConfig.name = NULL;
                }

                m_tlsConfig.name = nsl_strdup(tlsConfig.name);
            }

            m_tlsConfig.override = tlsConfig.override;
        }

        bool NetConnection::IsReachableForInternetConnection()
        {
            bool result = false;
            struct sockaddr_in address;
            bzero(&address, sizeof(address));
            address.sin_len = sizeof(address);
            address.sin_family = AF_INET;
            SCNetworkReachabilityRef reachabilityRef = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)&address);
            if (reachabilityRef)
            {
                SCNetworkReachabilityFlags flags = 0;
                if (SCNetworkReachabilityGetFlags(reachabilityRef, &flags))
                {
                    result = (flags & kSCNetworkReachabilityFlagsReachable);
                }
                CFRelease(reachabilityRef);
            }
            return result;
        }

        PAL_NetConnectionStatus NetConnection::GetStatus() const
        {
            return m_status;
        }

        void NetConnection::SetStatus(PAL_NetConnectionStatus status)
        {
            if (m_status != status)
            {
                m_status = status;
                NotifyStatusChange(status);
            }
        }

        void NetConnection::SetStatusImmediate(PAL_NetConnectionStatus status)
        {
            if (m_status != status)
            {
                m_status = status;
                if (m_pStatusCallback != NULL )
                {
                    m_pStatusCallback(m_pStatusCallbackData, status);
                }
            }
        }

        void NetConnection::NotifyStatusChange(PAL_NetConnectionStatus status)
        {
            if (m_pStatusCallback != NULL )
            {
                CallbackNetworkStatus* callback = new CallbackNetworkStatus(
                        (void*)m_pStatusCallback,
                        (void*)m_pStatusCallbackData,
                        status);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        void NetConnection::NotifyDataSent(const byte* pBytes, unsigned int count)
        {
            if (m_pDataSentCallback != NULL)
            {
                CallbackNetworkData* callback = new CallbackNetworkData(
                        (void*)m_pDataSentCallback,
                        (void*)m_pDataSentCallbackData,
                        pBytes, count);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        void NetConnection::NotifyDataReceived(const byte* pBytes, unsigned int count)
        {
            if (m_pDataReceivedCallback != NULL)
            {
                CallbackNetworkData* callback = new CallbackNetworkData(
                        (void*)m_pDataReceivedCallback,
                        (void*)m_pDataReceivedCallbackData,
                        pBytes, count);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        void NetConnection::NotifyHttpDataReceived(void* pRequestData, PAL_Error errorCode, const byte* pBytes, unsigned int count)
        {
            if (m_pHttpDataReceivedCallback != NULL)
            {
                CallbackHttpData* callback = new CallbackHttpData(
                        (void*)m_pHttpDataReceivedCallback,
                        (void*)m_pHttpDataReceivedCallbackData,
                        pRequestData, errorCode, pBytes, count);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        void NetConnection::NotifyHttpResponseStatus(void* pRequestData, unsigned int status)
        {
            if (m_pHttpResponseStatusCallback != NULL )
            {
                CallbackHttpResponseStatus* callback = new CallbackHttpResponseStatus(
                        (void*)m_pHttpResponseStatusCallback,
                         (void*)m_pHttpResponseStatusCallbackData,
                        pRequestData, status);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        void NetConnection::NotifyDnsResult(PAL_Error errorCode,
                                            const char** addressList,
                                            short addressLength)
        {
            if (m_pDnsResultCallback != NULL )
            {
                CallbackDnsResult* callback = new CallbackDnsResult(
                        (void*)m_pDnsResultCallback,
                        (void*)m_pDnsResultCallbackData,
                        errorCode, m_hostName, addressList, addressLength);
                if (Schedule(callback) != PAL_Ok)
                {
                    delete callback;
                }
            }
        }

        /* See header file for description */
        void NetConnection::NotifyHttpResponseHeaders(void* pRequestData, char* responseHeaders)
        {
            if (m_pHttpResponseHeadersCallback != NULL)
            {
                // Take ownership of the string of response headers in the constructor.
                CallbackHttpResponseHeaders* callback = new CallbackHttpResponseHeaders(
                        m_pHttpResponseHeadersCallback,
                        (void*) m_pHttpResponseHeadersCallbackData,
                        pRequestData,
                        responseHeaders);

                if (callback)
                {
                    /* The string of response headers has been taken ownership of the created
                       CallbackHttpResponseHeaders object.
                    */
                    responseHeaders = NULL;

                    if (Schedule(callback) != PAL_Ok)
                    {
                        delete callback;
                        callback = NULL;
                    }
                }
            }

            // Free the string of response headers if it is not taken ownership.
            if (responseHeaders)
            {
                nsl_free(responseHeaders);
                responseHeaders = NULL;
            }
        }

    }   /* network */
}   /* nimpal */
/*! @} */
