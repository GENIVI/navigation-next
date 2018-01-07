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

 @file     TCPConnectionAdapter.mm
 @date     6/23/2014
 @defgroup PAL_NET PAL Network I/O Functions

 Platform-independent network I/O API.
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "TCPConnectionAdapter.h"

namespace nimpal
{
namespace network
{

TCPConnectionAdapter::TCPConnectionAdapter(PAL_NetConnectionProtocol protocol, PAL_Instance* pal): NetConnection(protocol, pal), connection(NULL), requestData(NULL)
{
    m_status = PNCS_Initialized;
}

TCPConnectionAdapter::~TCPConnectionAdapter()
{
    [connection release];
}

void TCPConnectionAdapter::Destroy()
{
    delete this;
}

PAL_Error TCPConnectionAdapter::Connect(const char* pHostName, uint16 port, struct sockaddr* /* localIpAddr */)
{
    PAL_Error result = PAL_ErrNetGeneralFailure;

    if (connection == NULL)
    {
        connection = [[TCPConnection alloc] init];

        connection.userData = this;
        connection.connectionStatusCallback = NetConnectionStatusCallback;
        connection.dataSentCallback = NetDataSentCallback;
        connection.dataReceivedCallback = NetDataReceivedCallback;
        connection.dnsResultCallback = NetDnsResultCallback;

        if (m_hostName != pHostName)
        {
            nsl_strncpy(m_hostName, pHostName, HOSTNAME_MAX_LEN);
            m_hostName[HOSTNAME_MAX_LEN] = '\0';
        }

        nsl_sprintf(m_portString, "%d", port);
        m_portNumber = port;

        NSString* host = nil;
        if (pHostName != NULL)
        {
            host = [NSString stringWithUTF8String:pHostName];
        }
        result = [connection connectToHost:host port:port];
    }

    return result;
}

PAL_Error TCPConnectionAdapter::Send(const byte* pBuffer, int count)
{
    if (pBuffer == NULL || count == 0)
    {
        return PAL_ErrBadParam;
    }

    return [connection sendData:[NSMutableData dataWithBytes:pBuffer length:count]];
}

PAL_Error TCPConnectionAdapter::Close()
{
    [connection close];
    return PAL_Ok;
}

PAL_Error TCPConnectionAdapter::Send(const byte* pBuffer,
                                     uint32      count,
                                     const char* pVerb,
                                     const char* pObject,
                                     const char* pAcceptType,
                                     const char* pAdditionalHeaders,
                                     void*       pRequestData)
{
    return PAL_ErrUnsupported;
}

void TCPConnectionAdapter::NetConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status)
{
    TCPConnectionAdapter* adapter = static_cast<TCPConnectionAdapter*>(userData);
    adapter->SetStatus(status);
}

void TCPConnectionAdapter::NetDataSentCallback(void* userData, const byte* bytes, uint32 count)
{
    TCPConnectionAdapter* adapter = static_cast<TCPConnectionAdapter*>(userData);
    adapter->NotifyDataSent(bytes, count);
}

void TCPConnectionAdapter::NetDataReceivedCallback(void* userData,
                                                   const byte* bytes,
                                                   uint32 count)
{
    TCPConnectionAdapter* adapter = static_cast<TCPConnectionAdapter*>(userData);
    adapter->NotifyDataReceived(bytes, count);
}

void TCPConnectionAdapter::NetDnsResultCallback(void* userData,
                                                PAL_Error errorCode,
                                                const char* hostName,
                                                uint32 const* const* addresses,
                                                uint32 addressCount)
{
    TCPConnectionAdapter* adapter = static_cast<TCPConnectionAdapter*>(userData);
    adapter->NotifyDnsResult(errorCode, (const char**)addresses, addressCount);
}

}
}

/*! @} */
