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

    @file     httpresponse.cpp
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

#include <string>
#include <palstdlib.h>
#include <cstdio>
#include "httpresponse.h"

using namespace std;

namespace nimpal
{
    namespace network
    {
        const char* HTTP_RESPONSE_BODY_START = "\r\n\r\n";
        const char* HTTP_NEW_LINE = "\r\n";
        const char* HTTP_CONTENT_LENGTH_HEADER = "Content-Length:";
        const char* HTTP_CONNECTION_CLOSE_HEADER = "Connection: close";

        HttpResponse::HttpResponse()
             : m_state(HTTP_RESPONSE_WAITING_STATUS),
              m_IsConnectionPersistent(TRUE), // Persistent(default) unless specified in headers.
              m_statusCodeFound(FALSE),
              m_statusCode(0),
              m_contentLengthHeaderFound(FALSE),
              m_contentLength(0),
              m_headersOffset(0),
              m_bodyOffset(0),
              m_bodyLength(0),
              m_currentLength(0),
              m_error(PAL_Ok),
              m_headers(NULL)
        {
            bqinit(&m_recvQ);
        }

        HttpResponse::~HttpResponse()
        {
            bqfree(&m_recvQ);

            // Free the saved string of response headers.
            if (m_headers)
            {
                nsl_free(m_headers);
                m_headers = NULL;
            }
        }

        int HttpResponse::FindString(const char* str, unsigned int offset)
        {
            int result = -1;

            if (nsl_strempty(str))
            {
                return -1;
            }

            int strLength = nsl_strlen(str);
            const byte* pData = (byte *)m_recvQ.r;
            int dataLength = m_recvQ.w - m_recvQ.r - offset;

            if (dataLength < strLength)
            {
                return -1;
            }

            for(int i=0; i<(dataLength-strLength); i++)
            {
                if(nsl_memcmp(pData+offset+i, str, strLength) == 0)
                {
                    result = i+offset;
                    break;
                }
            }

            return result;
        }

        HttpResponseState HttpResponse::ParseResponse()
        {
            HttpResponseState oldState;

            do
            {
                oldState = m_state;

                switch(m_state)
                {
                    case HTTP_RESPONSE_WAITING_STATUS:
                        {
                            int versionMajor = 0;
                            int versionMinor = 0;
                            int firstNewLine = FindString(HTTP_NEW_LINE);
                            if (firstNewLine < 0)
                            {
                                // not found;
                                break;
                            }

                            // found first new line, terminate it with NULL
                            byte* pData = (byte *)m_recvQ.r;
                            *(pData+firstNewLine) = '\0';
                            if (sscanf((const char*)pData, "HTTP/%d.%d %u ", &versionMajor,
                                        &versionMinor, &m_statusCode) == 3)
                            {
                                // found status code
                                m_statusCodeFound = TRUE;
                                m_state = HTTP_RESPONSE_WAITING_BODY;

                                /* Set the starting position of the header after the HTTP message
                                   types (For example: 'HTTP/1.1 200 OK\r\n').
                                */
                                m_headersOffset = firstNewLine + nsl_strlen(HTTP_NEW_LINE);
                            }
                        }
                        break;

                    case HTTP_RESPONSE_WAITING_BODY:
                        {
                            unsigned int headersLength = 0;
                            int bodyStart = FindString(HTTP_RESPONSE_BODY_START);
                            if (bodyStart < 0)
                            {
                                // not found;
                                break;
                            }

                            // found first new line, terminate it with NULL
                            m_state = HTTP_RESPONSE_RECEIVING_BODY;
                            byte* pData = (byte *)m_recvQ.r;
                            unsigned int length = m_recvQ.w - m_recvQ.r;
                            m_bodyOffset = bodyStart +
                                nsl_strlen(HTTP_RESPONSE_BODY_START);
                            m_bodyLength = length - m_bodyOffset;

                            // Copy and save the data of response headers to a string.
                            if (m_headers)
                            {
                                nsl_free(m_headers);
                                m_headers = NULL;
                            }
                            headersLength = m_bodyOffset - m_headersOffset;
                            if (headersLength > 0)
                            {
                                // There is a '\0' at the end of a string.
                                unsigned int stringLength = (headersLength + 1) * sizeof(char);

                                m_headers = (char*) nsl_malloc(stringLength);
                                if (m_headers)
                                {
                                    nsl_memset(m_headers, 0, stringLength);

                                    // Copy the data of response headers to a string.
                                    nsl_memcpy(m_headers, pData + m_headersOffset, headersLength);
                                }

                                // Clear the starting position of response headers.
                                m_headersOffset = 0;
                            }

                            // See if we have connection closure header
                            if (FindString(HTTP_CONNECTION_CLOSE_HEADER) >= 0)
                            {
                                m_IsConnectionPersistent = FALSE;
                            }

                            int contentLengthHeader = FindString(HTTP_CONTENT_LENGTH_HEADER);
                            if (contentLengthHeader >= 0)
                            {
                               int contentLengthHeaderEnd = FindString(HTTP_NEW_LINE, contentLengthHeader);
                               if (contentLengthHeaderEnd > contentLengthHeader)
                               {
                                  *(pData+contentLengthHeaderEnd) = '\0';
                                   if(sscanf((const char*)pData+contentLengthHeader+nsl_strlen(HTTP_CONTENT_LENGTH_HEADER), "%u", &m_contentLength))
                                    {
                                        m_contentLengthHeaderFound = TRUE;
                                    }
                                }
                            }
                        }
                        break;

                    case HTTP_RESPONSE_RECEIVING_BODY:
                         if (m_contentLengthHeaderFound)
                         {
                             // m_bodyOffset is 0 after user calling GetBody.
                             if ((m_currentLength - m_bodyOffset) >= m_contentLength)
                             {
                                 m_state = HTTP_RESPONSE_COMPLETE;
                             }
                         }
                         break;

                    case HTTP_RESPONSE_COMPLETE:
                        m_currentLength = 0;
                        break;
                }
            } while (oldState != m_state);  // parse multiple passes

            return m_state;
        }

        HttpResponseState HttpResponse::AddData(const byte* pBytes, unsigned int count)
        {
            if (m_state == HTTP_RESPONSE_RECEIVING_BODY)
            {
                return SetData(pBytes, count);
            }

            return AppendData(pBytes, count);
        }

        HttpResponseState HttpResponse::AppendData(const byte* pBytes, unsigned int count)
        {
            HttpResponseState newState = m_state;
            if (pBytes && (count > 0))
            {
                bqprep(&m_recvQ, count);
                nsl_memcpy(m_recvQ.w, pBytes, count);
                m_recvQ.w += count;

                m_currentLength += count;

                newState = ParseResponse();
            }
            return newState;
        }

        HttpResponseState HttpResponse::SetData(const byte* pBytes, unsigned int count)
        {
            // Previous data has been taken out.
            nsl_assert(m_state == HTTP_RESPONSE_RECEIVING_BODY && m_bodyOffset == 0);

            HttpResponseState newState = m_state;
            if (pBytes && (count > 0))
            {
                // CallbackHttpData copy data.
                bqresize(&m_recvQ, count);
                nsl_memcpy(m_recvQ.r, pBytes, count);
                m_recvQ.w = m_recvQ.r + count;

                m_currentLength += count;

                // Clear the offset of the header because the data has been reset.
                m_headersOffset = 0;

                newState = ParseResponse();
            }
            return newState;
        }

        /* See header file for description */
        char* HttpResponse::TakeHeaders()
        {
            char* headers = m_headers;

            /* Set m_headers NULL. Because this function takes ownership of the allocated
               string of response headers.
            */
            m_headers = NULL;

            return headers;
        }

        PAL_Error HttpResponse::GetBody(const byte** ppBytes, unsigned int* pCount)
        {
            const byte* pData = (byte *)m_recvQ.r;
            int length = m_recvQ.w - m_recvQ.r;

            if ((ppBytes == NULL) || (pCount == NULL))
            {
                return PAL_Failed;
            }

            if ((m_state == HTTP_RESPONSE_RECEIVING_BODY) ||
                    (m_state == HTTP_RESPONSE_COMPLETE))
            {
                *ppBytes = pData + m_bodyOffset;
                *pCount = length - m_bodyOffset;

                // Reset current length for comparing with content length of HTTP.
                m_currentLength -= m_bodyOffset;
                // Reset body offset for SetData.
                m_bodyOffset = 0;

                return PAL_Ok;
            }

            return PAL_Failed;
        }

        int HttpResponse::IsConnectionPersistent()
        {
            return m_IsConnectionPersistent;
        }

        int HttpResponse::IsResponseOk()
        {
            if (m_statusCodeFound)
            {
                if ((m_statusCode >= 200) &&
                    (m_statusCode < 299))
                {
                    return TRUE;
                }
            }

            return FALSE;
        }

        PAL_Error HttpResponse::GetResponseError()
        {
            return m_error;
        }

        void HttpResponse::SetResponseError(PAL_Error error)
        {
           m_error = error;
        }
    }
}

