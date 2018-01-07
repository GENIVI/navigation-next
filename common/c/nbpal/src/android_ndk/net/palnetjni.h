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

    @file     palnetjni.h
    @date     9/27/2013

    @brief    Invoke JAVA implement network function through JNI.
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef PAL_NET_JNI_H_
#define PAL_NET_JNI_H_

#include <jni.h>
#include <string>
#include <map>
#include "paltypes.h"
#include "palimpl.h"
#include "pallock.h"

namespace nimpal
{
    namespace network
    {
        class NetworkInstancesMappingTable
        {
            public:
                NetworkInstancesMappingTable(){};
                virtual ~NetworkInstancesMappingTable(){PAL_LockDestroy(m_lock);};

                static void  SetPalInstance(PAL_Instance* pal);
                static bool  AddNetworkInstance(void* networkInstance, uint32& handleID);
                static void* GetNetworkInstance(uint32 handleID);
                static bool  RemoveNetworkInstance(uint32 handleID);
                static uint32 Size();

            private:
                static uint32 GenNetworkHandleID();
                static std::map<uint32, void*> m_networkInstanceMap;
                static uint32 handleID;
                static PAL_Instance* m_pal;
                static PAL_Lock* m_lock;
        };

        class PalNetJNI
        {
            public:
                PalNetJNI(){};
                ~PalNetJNI(){};

                //The interface is applied for common and security network connection(TCP or TLS).

                /*! Create a TCP/TLS connection object in JAVA side.
                    @param userData the instance pointer of connection in CCC side.

                    @return the pointer of java tcp connection object. this java object pointer
                    will be save in the tcp connection object in CCC side.
                */
                static void* CreateSocket(void* userData, uint32& handleID, bool isSecurityTcpNetwork = false);
                static bool Connect(void* jInstance, std::string host, int port);
                static bool Send(void* jInstance, char* buf, int len);
                static void Close(void* jInstance, uint32 handleID);

                //The interface is applied for HTTP/HTTPS network connection.

                /*! Create a HTTP/HTTPS connection object in JAVA side.
                    @param userData the instance pointer of connection in CCC side.

                    @return the pointer of java http connection object. this java object pointer
                    will be save in the http connection object in CCC side.
                */
                static void* HttpCreate(void* userData, uint32& handleID, bool isSecurityHttpNetwork = false);

                /*! Connection to http server in JAVA side.
                    @param jInstance the pointer of java http connection object, return by function HttpCreate,
                */
                static bool HttpConnect(void* jInstance, std::string host, int port, bool isSecurityConnection);
                //Please see netconnection.h
                static bool HttpSend(void* jInstance,
                                     const char* pBuffer,
                                     int         count,
                                     const char* pVerb,
                                     const char* pObject,
                                     const char* pAcceptType,
                                     const char* pAdditionalHeaders,
                                     void*       pRequestData);
                static void HttpClose(void* jInstance, uint32 handleID);
                static void SetPalInstance(PAL_Instance* pal);
        };
    }
}

#endif /* PAL_NET_JNI_H_ */
