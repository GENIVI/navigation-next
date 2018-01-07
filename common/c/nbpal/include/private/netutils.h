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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#ifndef _NETUTILS_H_
#define _NETUTILS_H_

extern "C"
{
#include "paltypes.h"
#include "palerror.h"
}

namespace nimpal
{

namespace network {

/* List the processed HTTP status codes

   Find the details in http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
*/

// Standard response for successful HTTP requests.
const unsigned int HTTP_SUCCESS = 200;
/* The server is delivering only part of the resource (byte serving)
   due to a range header sent by the client. The range header is used by
   tools like wget to enable resuming of interrupted downloads, or split
   a download into multiple simultaneous streams.*/
const unsigned int HTTP_PARTIAL_CONTENT = 206;

// The server successfully processed the request, but is not returning any content.
const unsigned int HTTP_NO_CONTENT = 204;

// Indicates the resource has not been modified since last requested.
const unsigned int HTTP_NOT_MODIFIED = 304;

// The request cannot be fulfilled due to bad syntax.
const unsigned int HTTP_BAD_REQUEST = 400;

// The requested resource could not be found but may be available again in the future.
const unsigned int HTTP_RESOURCE_NOT_FOUND = 404;

// The client must first authenticate itself with the proxy.
const unsigned int HTTP_PROXY_AUTH_REQUIRED = 407;

// A generic server error message, given when no more specific message is suitable.
const unsigned int HTTP_INTERNAL_SERVER_ERROR = 500;

// The server was acting as a gateway or proxy and received an invalid response from the upstream server.
const unsigned int HTTP_BAD_GATEWAY = 502;

PAL_Error
HttpStatusToPalError(unsigned int statusCode   /*!< A HTTP status code to be translated */
                    );

};

};



#endif /* _NETUTILS_H_ */
