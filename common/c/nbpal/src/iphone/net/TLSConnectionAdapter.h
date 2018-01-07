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

 @file  TLSConnectionAdapter.h
 @date  5/14/2014
 @defgroup PAL_NET PAL Network I/O Functions

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

/*! @{ */

#ifndef __TLS_CONNECTION_ADAPTER_H__
#define __TLS_CONNECTION_ADAPTER_H__

#include <string>
#include "netconnection.h"
#import "TLSConnection.h"

namespace nimpal {
namespace network {

class TLSConnectionAdapter : public NetConnection
{
public:
    TLSConnectionAdapter(PAL_NetConnectionProtocol protocol, PAL_Instance* pal);

    /* See description in netconnection.h */
    virtual void Destroy();
    virtual PAL_Error Connect(const char* pHostName, uint16 port, struct sockaddr* localIpAddr = NULL);
    virtual PAL_Error Send(const byte* pBuffer, int count);
    virtual PAL_Error Close();
    virtual PAL_Error Send(const byte* pBuffer,
                           uint32      count,
                           const char* pVerb,
                           const char* pObject,
                           const char* pAcceptType,
                           const char* pAdditionalHeaders,
                           void*       pRequestData);

protected:
    virtual ~TLSConnectionAdapter();

private:
    static void NetConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status);
    static void NetDataSentCallback(void* userData, const byte* bytes, uint32 count);
    static void NetDataReceivedCallback(void* userData,
                                        const byte* bytes,
                                        uint32 count);
    static void NetDnsResultCallback(void* userData,
                                     PAL_Error errorCode,
                                     const char* hostName,
                                     uint32 const* const* addresses,
                                     uint32 addressCount);

    TLSConnectionAdapter(const TLSConnectionAdapter&);
    TLSConnectionAdapter& operator= (const TLSConnectionAdapter&);

    TLSConnection*              connection;
    void*                       requestData;
};

}
}

#endif /* __TLS_CONNECTION_ADAPTER_H__ */

/*! @} */
