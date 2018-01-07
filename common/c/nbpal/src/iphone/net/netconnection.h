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
    @date     3/15/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Base class for PAL network connections.

    Windows implementation for PAL base network function class.
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
#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include "palnet.h"
#include "callbackmanager.h"

namespace nimpal
{
    namespace network
    {
        const unsigned int HOSTNAME_MAX_LEN = 255;
        const unsigned int PORT_STRING_MAX_LEN = 11;

        class NetConnection : public CallbackManager
        {
        public:
            NetConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal);

            void SetStatusCallback(
                    PAL_NetConnectionStatusCallback pStatusCallback,
                    void* pData);

            void SetDataSentCallback(
                    PAL_NetDataSentCallback pDataSentCallback,
                    void* pData);

            void SetDataReceivedCallback(
                    PAL_NetDataReceivedCallback pDataReceivedCallback,
                    void* pData);

            void SetHttpDataReceivedCallback(
                    PAL_NetHttpDataReceivedCallback pHttpDataReceivedCallback,
                    void* pData);

            void SetHttpResponseStatusCallback(
                    PAL_NetHttpResponseStatusCallback pHttpResponseStatusCallback,
                    void* pData);

            void SetDnsResultCallback(
                    PAL_NetDnsResultCallback pDnsResultCallback,
                    void* pData);

            /*! Set the callback and user data for HTTP response headers

                @return None
            */
            void SetHttpResponseHeadersCallback(
                    PAL_NetHttpResponseHeadersCallback pHttpResponseHeadersCallback,
                    void* pData);

            void SetTLSConfig(
                    const PAL_NetTLSConfig& tlsConfig);

            // Check whether internet connection is reachable
            static bool IsReachableForInternetConnection();

            virtual PAL_NetConnectionStatus GetStatus() const;

            /*! Destroy a HTTP connection object

                User cannot destroy this object by delete. Because destructor is private.
                User should call this function to destroy this object.

                @return None
            */
            virtual void Destroy() = 0;

            virtual PAL_Error Connect(const char* pHostName, uint16 port, struct sockaddr* localIpAddr = NULL) = 0;
            virtual PAL_Error Send(const byte* pBuffer, int count) = 0;
            virtual PAL_Error Close() = 0;
            virtual PAL_Error Send(const byte* pBuffer,
                                   uint32      count,
                                   const char* pVerb,
                                   const char* pObject,
                                   const char* pAcceptType,
                                   const char* pAdditionalHeaders,
                                   void*       pRequestData) = 0;

        protected:

            /*! Destructor of HTTP connection

                This function is protected. User should call function Destroy to destroy this object.
            */
            virtual ~NetConnection();

            void SetStatus(PAL_NetConnectionStatus status);
            void SetStatusImmediate(PAL_NetConnectionStatus status);
            void NotifyStatusChange(PAL_NetConnectionStatus status);
            void NotifyDataSent(const byte* pBytes, unsigned int count);
            void NotifyDataReceived(const byte* pBytes, unsigned int count);
            void NotifyHttpDataReceived(void* pRequestData, PAL_Error errorCode, const byte* pBytes, unsigned int count);
            void NotifyHttpResponseStatus(void* pRequestData, unsigned int status);
            void NotifyDnsResult(PAL_Error errorCode,
                    const char** addressList, short addressLength);

            /*! Notify the HTTP response headers

                This function takes ownership of the parameter responseHeaders.

                @return None
            */
            void NotifyHttpResponseHeaders(void* pRequestData, char* responseHeaders);

            PAL_NetConnectionStatus         m_status;
            PAL_NetConnectionProtocol       m_protocol;
            char                            m_hostName[HOSTNAME_MAX_LEN+1];
            char                            m_portString[PORT_STRING_MAX_LEN+1];
            uint16                          m_portNumber;
            struct sockaddr_storage*        m_localIpAddr;

            // Common Name(CN) in PAL_NetTLSConfig is typically composed of Host + Domain name and will look like "www.yoursite.com" or "yoursite.com".
            // SSL Server Certificates are specific to the Common Name that they have been issued to at the Host Level.
            // The Common Name must be the same as the Web address you will be accessing when connecting to a secure site.
            PAL_NetTLSConfig                m_tlsConfig;

            PAL_NetConnectionStatusCallback m_pStatusCallback;
            void*                           m_pStatusCallbackData;

            PAL_NetDataSentCallback         m_pDataSentCallback;
            void*                           m_pDataSentCallbackData;

            PAL_NetDataReceivedCallback     m_pDataReceivedCallback;
            void*                           m_pDataReceivedCallbackData;

            PAL_NetHttpDataReceivedCallback m_pHttpDataReceivedCallback;
            void*                           m_pHttpDataReceivedCallbackData;

            PAL_NetHttpResponseStatusCallback m_pHttpResponseStatusCallback;
            void*                           m_pHttpResponseStatusCallbackData;

            PAL_NetDnsResultCallback        m_pDnsResultCallback;
            void*                           m_pDnsResultCallbackData;

            PAL_NetHttpResponseHeadersCallback  m_pHttpResponseHeadersCallback; /*!< Callback for HTTP response headers */
            void*                           m_pHttpResponseHeadersCallbackData; /*!< User data of callback for HTTP response headers */
        };
    }
}

#endif
/*! @} */
