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

    @file HttpsConnectionAdapter.cpp
    @date 11-1-30
    @defgroup HttpsConnectionAdapter.cpp in nbpal
*/
/*
    See file description in header file.

    (C) Copyright 2010 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

/* all the file’s code goes here */

#include "HttpsConnectionAdapter.h"
#import "HttpsConnection.h"

namespace nimpal
{
    namespace network
    {
        HttpsConnectionAdapter::HttpsConnectionAdapter(PAL_NetConnectionProtocol protocol,
                                                       PAL_Instance* pal)
                               : NetConnection(protocol, pal), connection(NULL), requestData(NULL)
        {
            m_status = PNCS_Initialized;
        }

        HttpsConnectionAdapter::~HttpsConnectionAdapter()
        {
            [connection release];
        }

        /* See description in netconnection.h */
        void HttpsConnectionAdapter::Destroy()
        {
            // @todo: I think same block issue exists in HTTPS. But there is not many HTTPS connection so far.
            delete this;
        }

        PAL_Error HttpsConnectionAdapter::Send(const byte* pBuffer, uint32 count,
                                               const char* pVerb, const char* pObject,
                                               const char* pAcceptType,
                                               const char* pAdditionalHeaders,
                                               void* pRequestData)
        {
            PAL_Error result = PAL_ErrNetGeneralFailure;
            if (connection != NULL)
            {
                NSString* verb = nil;
                NSString* object = nil;
                NSString* acceptType = nil;
                NSString* additionalHeader = nil;
                if (pVerb != NULL)
                {
                    verb = [NSString stringWithUTF8String:pVerb];
                }
                if (pObject != NULL)
                {
                    object = [NSString stringWithUTF8String:pObject];
                }
                if (pAcceptType != NULL)
                {
                    acceptType = [NSString stringWithUTF8String:pAcceptType];
                }
                if (pAdditionalHeaders != NULL)
                {
                    additionalHeader = [NSString stringWithUTF8String:pAdditionalHeaders];
                }
                requestData = pRequestData;
                result = [connection sendData:pBuffer withLength:count andVerb:verb andObject:object andAcceptType:acceptType andAdditionalHeaders:additionalHeader];
            }
            return result;
        }

        PAL_Error HttpsConnectionAdapter::Connect(const char* pHostName, uint16 port, struct sockaddr* localIpAddr)
        {
            PAL_Error result = PAL_ErrNetGeneralFailure;
            if (connection == NULL) {
                connection = [[HttpsConnection alloc] initWithUseHttps:YES];

                connection.userData = this;
                connection.connectionStatusCallback = NetConnectionStatusCallback;
                connection.dataSentCallback = NetDataSentCallback;
                connection.httpDataReceivedCallback = NetHttpDataReceivedCallback;
                connection.httpResponseStatusCallback = HttpResponseStatusCallback;

                NSString* host = nil;
                if (pHostName != NULL) {
                    host = [NSString stringWithUTF8String:pHostName];
                }
                result = [connection connectToHost:host andPort:port];
                if (result == PAL_Ok)
                {
                    SetStatus(PNCS_Connected);
                }
            }
            return result;
        }

        PAL_Error HttpsConnectionAdapter::Send(const byte* pBuffer, int count)
        {
            return PAL_ErrUnsupported;
        }

        PAL_Error HttpsConnectionAdapter::Close()
        {
            if (connection != NULL) {
                [connection close];
                connection = NULL;
                SetStatus(PNCS_Closed);
            }
            return PAL_Ok;
        }

        void HttpsConnectionAdapter::NetConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status)
        {
            HttpsConnectionAdapter* adapter = static_cast<HttpsConnectionAdapter*>(userData);
            adapter->NotifyStatusChange(status);
        }

        void HttpsConnectionAdapter::NetDataSentCallback(void* userData, const byte* bytes, uint32 count)
        {
            HttpsConnectionAdapter* adapter = static_cast<HttpsConnectionAdapter*>(userData);
            adapter->NotifyDataSent(bytes, count);
        }

        void HttpsConnectionAdapter::NetHttpDataReceivedCallback(void* userData, void* requestData,
                                                PAL_Error errorCode, const byte* bytes,
                                                uint32 count)
        {
            HttpsConnectionAdapter* adapter = static_cast<HttpsConnectionAdapter*>(userData);
            adapter->NotifyHttpDataReceived(adapter->requestData, errorCode, bytes, count);
        }

        void HttpsConnectionAdapter::HttpResponseStatusCallback(void* userData, unsigned int status)
        {
            HttpsConnectionAdapter* adapter = static_cast<HttpsConnectionAdapter*>(userData);
            adapter->NotifyHttpResponseStatus(adapter->requestData, status);
        }
    }
}

/*! @} */
