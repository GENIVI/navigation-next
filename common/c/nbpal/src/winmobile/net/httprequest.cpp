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

    @file     HttpRequest.cpp
    @date     9/3/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Windows implementation for HttpRequest class.
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
#include "httpconnection.h"
#include "httprequest.h"
#include "palstdlib.h"
#include "paldebug.h"
#include "tchar.h"


// Disable "warning C4127: conditional expression is constant" because of the "while(1)" statement.
#pragma warning(disable:4127)


namespace nimpal
{
    namespace network
    {
        const char* DEFAULT_VERB = "GET";
        const char* DEFAULT_ACCEPT_TYPE = "*/*";
        const char* EMPTY_STRING = "";

        HttpRequest::HttpRequest(HttpConnection*    pConnection,
                                  HINTERNET         sessionHandle,
                                  const byte*&      bytes,
                                  int               count,
                                  const char*&      rVerb,
                                  const char*&      rObject,
                                  const char*&      rAcceptType,
                                  const char*&      rAdditionalHeaders,
                                  void*             pRequestData)
            : m_Connection(pConnection), m_SessionHandle(sessionHandle), m_RequestHandle(NULL), m_State(HRS_Created), m_pBytes(NULL), m_WinInetError(!ERROR_SUCCESS), m_Count(count), m_ReadOffset(0), m_requestData(pRequestData)

        {
            if (bytes != NULL && count > 0)
            {
                m_pBytes = new byte[count];
                memcpy(m_pBytes, bytes, count);
            }

            m_Verb = rVerb ? rVerb : EMPTY_STRING;
            m_Object = rObject ? rObject : EMPTY_STRING;
            m_AcceptType = rAcceptType ? rAcceptType : EMPTY_STRING;
            m_AdditionalHeaders = rAdditionalHeaders ? rAdditionalHeaders : EMPTY_STRING;

        }

        HttpRequest::~HttpRequest()
        {
            if (m_RequestHandle != NULL)
            {
                InternetCloseHandle(m_RequestHandle);
                m_RequestHandle = NULL;
            }
            delete [] m_pBytes;
        }

        HttpRequestState HttpRequest::GetRequestState()
        {
            return m_State;
        }

        void* HttpRequest::GetRequestData()
        {
            return m_requestData;
        }

        void HttpRequest::SetCallbackContextId(uint32 contextId)
        {
            m_CallbackContextId = contextId;
        }

        void HttpRequest::SetWinInetError(DWORD32 error)
        {
            m_WinInetError = error;
        }

        DWORD32 HttpRequest::GetWinInetError()
        {
            return m_WinInetError;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        //  SendRequest(): This function sends the HTTP request to server using wininet APIs     //

        void HttpRequest::SendRequest(void)
        {
             // STEP 1: Open a HTTP request
             const char* pVerb   = m_Verb.empty() ? DEFAULT_VERB : m_Verb.c_str();
             const char* pObject = m_Object.empty() ? NULL : m_Object.c_str();
             LPCSTR pAcceptTypes[] = { NULL, NULL };
             pAcceptTypes[0] = m_AcceptType.empty() ? DEFAULT_ACCEPT_TYPE : m_AcceptType.c_str();

             DWORD requestFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_DONT_CACHE;
             if (PNCP_HTTPS == m_Connection->GetHttpConnectionProtocol())
             {
                 requestFlags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
             }


             m_RequestHandle = HttpOpenRequestA( m_SessionHandle,
                                                 pVerb,
                                                 pObject,
                                                 "HTTP/1.1",                       // Default HTTP Version
                                                 NULL,                             // No Referrer
                                                 pAcceptTypes,
                                                 requestFlags,                     // Flags
                                                 (DWORD_PTR) m_CallbackContextId); // Context info

            // STEP 2: Add additional headers and Send request
            if (NULL != m_RequestHandle)
            {
               const char* pHeaders = m_AdditionalHeaders.empty() ? NULL : m_AdditionalHeaders.c_str();
               DWORD headersSize = m_AdditionalHeaders.empty() ? 0 : m_AdditionalHeaders.length();

              if (FALSE == HttpSendRequestA(m_RequestHandle,
                                            pHeaders,
                                            headersSize,
                                            m_pBytes,
                                            m_Count))
                {
                   if (ERROR_IO_PENDING != GetLastError())
                   {
                       // Request is in erroneous state
                        m_State = HRS_ErrorEncountered;
                   }
                }
            }
            else
            {
                // Request is in erroneous state
                m_State = HRS_ErrorEncountered;
            }

            // If in error state, Notify caller
            // TODO: Currently, we don't have any way to pass WinInet Errors
            //       For now passing all such errors as status code "400 Bad Request"
            if (HRS_ErrorEncountered == m_State)
            {
                m_Connection->NotifyHttpResponseStatus(400, m_requestData, (uint32)-1);
            }
            else
            {
                m_State = HRS_SendRequestInitiated;
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        //    FinishRequest(): This function processes the pending requests  which are not yet   //
        //                     finished to completion.

        void HttpRequest::FinishRequest()
        {
            if (ERROR_SUCCESS != m_WinInetError)
            {
                // Got error in Asynchronous response to request
                // Request is in erroneous state
                m_State = HRS_ErrorEncountered;

                // If in error state, Notify caller
                // TODO: Currently, we don't have any way to pass WinInet Errors
                //       For now passing all such errors as status code "400 Bad Request"
                m_Connection->NotifyHttpResponseStatus(400, m_requestData, (uint32)-1);
            }
            else if (HRS_SendRequestInitiated == m_State)
            {
                // Notify the client about HTTP Status code only if we have got response from server.
                uint32 contentLength = (uint32)-1;
                DWORD  contentLengthLen = sizeof(contentLength);
                if (!HttpQueryInfo(m_RequestHandle, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                                (LPDWORD)&contentLength, &contentLengthLen, NULL))
                {
                    contentLength  = (uint32)-1;
                }

                DWORD statusCode = GetResponseStatusCode();
                m_Connection->NotifyHttpResponseStatus(statusCode, m_requestData, contentLength);

                switch (statusCode)
                {
                case HTTP_STATUS_OK:
                case HTTP_STATUS_PARTIAL_CONTENT:
                    m_State = HRS_FileReadInProgess;
                    break;
                default:
                    m_State = HRS_ErrorEncountered;
                    break;
                }
            }

            if (m_State == HRS_FileReadInProgess)
            {
                // Read file
                INTERNET_BUFFERSA buffers;
                memset(&buffers, 0, sizeof(buffers));
                buffers.dwStructSize = sizeof(buffers);

                // Read data while it is available
                while(1)
                {
                    buffers.lpvBuffer = &m_ReadFileBuffer[m_ReadOffset];
                    buffers.dwBufferLength = READ_FILE_BUFFER_SIZE - m_ReadOffset;

                    if (TRUE == InternetReadFileExA(m_RequestHandle, &buffers, IRF_NO_WAIT, (DWORD_PTR) m_CallbackContextId))
                    {
                        if (0 == buffers.dwBufferLength)
                        {
                            // All data received. Request Complete.
                            // Check for pending notification data (if any)
                            if (0 != m_ReadOffset)
                            {
                                m_Connection->NotifyHttpDataReceived(m_requestData, PAL_Ok, m_ReadFileBuffer, m_ReadOffset);
                            }


                            char* responseHeader = GetResponseHeader();
                            m_Connection->NotifyHttpResponseHeaders(m_requestData, responseHeader);

                            // File download complete
                            m_State = HRS_FileReadComplete;
                            break;
                        }
                        // Data Received
                        else
                        {
                            m_ReadOffset += buffers.dwBufferLength;

                            if ( MINIMUM_DATA_NOTIFICATION_SIZE <=  m_ReadOffset)
                            {
                                // We have received the data with size atleast the minimum notification size
                                m_Connection->NotifyHttpDataReceived(m_requestData,
                                                                     PAL_Ok,
                                                                     m_ReadFileBuffer,
                                                                     m_ReadOffset);
                                // Reset offset
                                m_ReadOffset = 0;
                            }
                        }
                    }
                    else
                    {
                        if (ERROR_IO_PENDING != GetLastError())
                        {
                            // Request is in erroneous state
                             m_State = HRS_ErrorEncountered;
                             m_Connection->NotifyHttpDataReceived(m_requestData,
                                     PAL_ErrNoData,
                                     m_ReadFileBuffer,
                                     m_ReadOffset);
                        }
                        break;
                    }
                }
            }
        }

        DWORD HttpRequest::GetResponseStatusCode()
        {
          DWORD status = 0;
          DWORD statusSize = sizeof(status);

             if (HttpQueryInfo(m_RequestHandle, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &status, &statusSize, NULL) == FALSE)
             {
                    status = 0;
              }
          return status;
        }

        char*  HttpRequest::GetResponseHeader()
        {

            // Notify header available.
            char* headerString=NULL;
            DWORD dwSize = 0;
            BOOL  result = FALSE;
            do {
                result = HttpQueryInfoA(m_RequestHandle,
                                        HTTP_QUERY_RAW_HEADERS_CRLF,
                                        (LPVOID)headerString, &dwSize, NULL);
                if(!result)
                {
                    if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND)
                    {
                        // Check for an insufficient buffer.
                        if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
                        {
                            // Allocate the necessary buffer.
                            headerString = (char*)nsl_malloc(dwSize);
                            nsl_memset(headerString, 0, dwSize);
                            // Retry the call.
                            continue;
                        }
                        else
                        {
                            // Error handling code.
                            if (headerString)
                            {
                                nsl_free(headerString);
                                headerString = NULL;
                            }
                            break;
                        }
                    }
                }
            } while (!result);

            return headerString;
        }

#ifdef DEBUG
        boolean HttpRequest::GetResponseHeaderInfo()
        {
            boolean success = FALSE;

            TCHAR* pHeaderBuffer = NULL;
            DWORD size = 0;

            HttpQueryInfo(m_RequestHandle, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)pHeaderBuffer, &size, NULL);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                pHeaderBuffer = new TCHAR[size + 2];
                if (HttpQueryInfo(m_RequestHandle, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)pHeaderBuffer, &size, NULL) == TRUE)
                {
                    pHeaderBuffer[size] = '\n';
                    pHeaderBuffer[size +1 ] = '\0';
                    ::OutputDebugString(pHeaderBuffer);
                    success = TRUE;
                }
                delete pHeaderBuffer;
            }

            return success;
        }
#endif
    }
}
