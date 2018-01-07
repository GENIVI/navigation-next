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

    @file     httprequest.h
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing a PAL HTTP network request.

    Windows implementation for storing a PAL HTTP network request.
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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <wininet.h>
#include "paldebug.h"

using namespace std;

namespace nimpal
{
    namespace network
    {
        static const int READ_FILE_BUFFER_SIZE = 4096; // 4k. TODO: Maybe we should have the user decide the size
        static const int MINIMUM_DATA_NOTIFICATION_SIZE = 1024; // 1k

        typedef enum
        {
            HRS_ErrorEncountered = 0,
            HRS_Created,
            HRS_SendRequestInitiated,
            HRS_FileReadInProgess,
            HRS_FileReadComplete,
        } HttpRequestState;

        // HttpConnection class forward declaration
        class HttpConnection;

        class HttpRequest
        {
        public:
            HttpRequest(HttpConnection*   pConnection,
                        HINTERNET         sessionHandle,
                        const byte*&      bytes,
                        int               count,
                        const char*&      rVerb,
                        const char*&      rObject,
                        const char*&      rAcceptType,
                        const char*&      rAdditionalHeaders,
                        void*             pRequestData);
            ~HttpRequest();

            void    SendRequest();
            void    FinishRequest();
            void    SetCallbackContextId(uint32 contextId);
            void    SetWinInetError(DWORD32 error);
            DWORD32 GetWinInetError();
            void*   GetRequestData();
            HttpRequestState GetRequestState();

       private:
            DWORD GetResponseStatusCode();

#ifdef DEBUG
            boolean GetResponseHeaderInfo();
#endif

            /*! Get response header of this request.

                @note   caller should responsible for the release of allocated memory.

                @return response header string.
            */
            char*  GetResponseHeader();

            HttpConnection*  m_Connection;
            HINTERNET        m_SessionHandle;
            HINTERNET        m_RequestHandle;
            HttpRequestState m_State;
            byte*            m_pBytes;
            DWORD32          m_WinInetError;  // Error received from WinInet
            int              m_Count;
            string           m_Verb;
            string           m_Object;
            string           m_AcceptType;
            string           m_AdditionalHeaders;
            unsigned int     m_ReadOffset;
            byte             m_ReadFileBuffer[READ_FILE_BUFFER_SIZE];
            void*            m_requestData;   // User's reference pointer to request made. Can be NULL.
            DWORD            m_CallbackContextId;
        };
    }
}

#endif
