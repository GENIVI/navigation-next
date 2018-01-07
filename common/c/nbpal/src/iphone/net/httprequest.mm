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

    @file     httprequest.cpp
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing a PAL HTTP network request.

    Windows implementation for storing a PAL HTTP network request.
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

#include <string>
#include <palstdlib.h>
#include "httprequest.h"

using namespace std;

namespace nimpal
{
    namespace network
    {
        const char* DEFAULT_USER_AGENT =
            "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3";
        const char* DEFAULT_VERB = "POST";
        const char* DEFAULT_OBJECT = "/";
        const char* DEFAULT_ACCEPT_TYPE = "*/*";
        const char* DEFAULT_ADDITIONAL_HEADER = "";

        HttpRequest::HttpRequest(const char* host,const byte* bytes,
                uint32 count, const char*& rVerb,
                const char*& rObject, const char*& rAcceptType,
                const char*& rAdditionalHeaders, void* pRequest)
            : m_pBytes(NULL), m_length(0),
            m_headerLength(0), m_bodyLength(0),
            m_sentCount(0), m_RequestData(pRequest)
        {
            // GET or POST line
            string requestHeader = nsl_strempty(rVerb)?DEFAULT_VERB: rVerb;
            requestHeader += " ";
            requestHeader += nsl_strempty(rObject)?DEFAULT_OBJECT:rObject;
            requestHeader += " HTTP/1.1\r\n";

            // Accept Header
            requestHeader += "Accept: ";
            requestHeader += nsl_strempty(rAcceptType)?DEFAULT_ACCEPT_TYPE:rAcceptType;
            requestHeader += "\r\n";

            // User Agent header
            requestHeader += "User-Agent: ";
            requestHeader += DEFAULT_USER_AGENT;
            requestHeader += "\r\n";

            // Host header
            requestHeader += "Host: ";
            requestHeader += host;
            requestHeader += "\r\n";

            // Cache-Control header
            requestHeader += "Cache-Control: no-cache\r\n";

            // additional header
            requestHeader += nsl_strempty(rAdditionalHeaders)?DEFAULT_ADDITIONAL_HEADER:rAdditionalHeaders;

            char tmpBuf[128] = {0};
            snprintf(tmpBuf, sizeof(tmpBuf)-1,
                    "Content-Length: %u\r\n", count);
            requestHeader += tmpBuf;

            requestHeader += "\r\n";

            m_headerLength = requestHeader.length();
            m_bodyLength = count;
            m_length = m_headerLength + m_bodyLength;

            m_pBytes = new byte[m_length];
            // copy header
            requestHeader.copy((char *)m_pBytes, m_headerLength);
            if (bytes != NULL && count > 0)
            {
                // copy body
                nsl_memcpy(m_pBytes+m_headerLength, bytes, m_bodyLength);
            }
        }

        HttpRequest::~HttpRequest()
        {
            delete [] m_pBytes;
        }

        void* HttpRequest::GetUserRequestData()
        {
          return m_RequestData;
        }
    }
}
