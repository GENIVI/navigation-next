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

    @file     callbackhttpresponsestatus.cpp
    @date     03/02/2012
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing a PAL HTTP network request.

    Qt QNX implementation for storing a PAL HTTP network request.
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palnet.h"
#include "callbackhttpresponsestatus.h"

/* List the processed HTTP status codes

   Find the details in http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
*/

// Standard response for successful HTTP requests.

const unsigned int HTTP_SUCCESS = 200;

// The server successfully processed the request, but is not returning any content.
const unsigned int HTTP_NO_CONTENT = 204;

// Indicates the resource has not been modified since last requested.
const unsigned int HTTP_NOT_MODIFIED = 304;

// The request cannot be fulfilled due to bad syntax.
const unsigned int HTTP_BAD_REQUEST = 400;

// The requested resource could not be found but may be available again in the future.
const unsigned int HTTP_RESOURCE_NOT_FOUND = 404;

// A generic server error message, given when no more specific message is suitable.
const unsigned int HTTP_INTERNAL_SERVER_ERROR = 500;

// The server was acting as a gateway or proxy and received an invalid response from the upstream server.
const unsigned int HTTP_BAD_GATEWAY = 502;


namespace nimpal
{
    namespace network
    {
        CallbackHttpResponseStatus::CallbackHttpResponseStatus(void* function, void* userData,
                void* requestData, unsigned int status) :
            Callback(function, userData),
            m_requestData(requestData),
            m_httpResponseStatus(status)
        {
        }

        CallbackHttpResponseStatus::~CallbackHttpResponseStatus()
        {
        }

        void CallbackHttpResponseStatus::InvokeFunction()
        {
            if (m_function)
            {
                // Translate this HTTP status code to a PAL error returned in callback function.
                PAL_Error palError = TranslateHttpStatusCodeToPalError(m_httpResponseStatus);

                (PAL_NetHttpResponseStatusCallback(m_function))(palError, m_userData, m_requestData, -1);
            }
        }

        /*! Translate a HTTP status code to a PAL error

            @return A PAL error translated from the HTTP status code
        */
        PAL_Error
        CallbackHttpResponseStatus::TranslateHttpStatusCodeToPalError(unsigned int statusCode   /*!< A HTTP status code to be translated */
                                                                      )
        {
            PAL_Error error = PAL_Ok;

            switch (statusCode)
            {
                case HTTP_SUCCESS:
                {
                    error = PAL_Ok;
                    break;
                }
                case HTTP_NO_CONTENT:
                {
                    error = PAL_ErrHttpNoContent;
                    break;
                }
                case HTTP_NOT_MODIFIED:
                {
                    error = PAL_ErrHttpNotModified;
                    break;
                }
                case HTTP_BAD_REQUEST:
                {
                    error = PAL_ErrHttpBadRequest;
                    break;
                }
                case HTTP_RESOURCE_NOT_FOUND:
                {
                    error = PAL_ErrHttpResourceNotFound;
                    break;
                }
                case HTTP_INTERNAL_SERVER_ERROR:
                {
                    error = PAL_ErrHttpInternalServerError;
                    break;
                }
                case HTTP_BAD_GATEWAY:
                {
                    error = PAL_ErrHttpBadGateway;
                    break;
                }
                default:
                {
                    // Set the error to general net failure for default.
                    error = PAL_ErrNetGeneralFailure;
                    break;
                }
            }

            return error;
        }
    }
}

/*! @} */
