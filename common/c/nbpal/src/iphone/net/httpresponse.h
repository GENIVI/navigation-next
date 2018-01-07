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

    @file     httpresponse.h
    @date     6/15/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing an HTTP response.

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

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include "paltypes.h"
#include "bq.h"

using namespace std;

namespace nimpal
{
    namespace network
    {
        const unsigned int HTTP_STATUS_OK = 200;
        typedef enum {
            HTTP_RESPONSE_WAITING_STATUS,
            HTTP_RESPONSE_WAITING_BODY,
            HTTP_RESPONSE_RECEIVING_BODY,
            HTTP_RESPONSE_COMPLETE
        } HttpResponseState;

        class HttpResponse
        {
        public:
            HttpResponse();
           ~HttpResponse();

            /*! Take ownership of the response headers if it is not NULL

                User should call the function nsl_free to free the returned pointer when
                it is usesless.

                @return Allocated pointer of the response headers by nsl_malloc
            */
            char* TakeHeaders();

            PAL_Error GetBody(const byte** ppBytes, unsigned int* pCount);
            HttpResponseState AddData(const byte* pBytes, unsigned int count);
            HttpResponseState GetState() { return m_state; };
            int IsResponseOk();
            int IsConnectionPersistent();
            int IsStatusCodeReceived() { return m_statusCodeFound; };
            unsigned int GetStatusCode() { return m_statusCode; };
            void SetResponseError(PAL_Error error);
            PAL_Error GetResponseError();

        private:
            HttpResponseState ParseResponse();
            int FindString( const char* str, unsigned int offset=0);

            HttpResponseState AppendData(const byte* pBytes, unsigned int count);
            HttpResponseState SetData(const byte* pBytes, unsigned int count);

            struct bq m_recvQ;
            HttpResponseState m_state;

            unsigned int m_IsConnectionPersistent;
            unsigned int m_statusCodeFound;
            unsigned int m_statusCode;
            unsigned int m_contentLengthHeaderFound;
            unsigned int m_contentLength;
            unsigned int m_headersOffset;   /*!< Starting position of response headers */
            unsigned int m_bodyOffset;
            unsigned int m_bodyLength;
            unsigned int m_currentLength;   /*!< Used to complete */
            PAL_Error    m_error;
            char*        m_headers;         /*!< Saved the string of response headers */
        };
    }
}

#endif

