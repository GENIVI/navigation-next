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
   @file        urlparser.h

   A simple URL Parser to parse URL.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

--------------------------------------------------------------------------*/

#ifndef _URLPARSER_H_
#define _URLPARSER_H_

extern "C"
{
#include "nbexp.h"
#include "paltypes.h"
#include <stdio.h>
}

#include "smartpointer.h"
#include <string>

namespace  nbcommon
{

// Used to check with the string of protocol
#define HTTP_PROTOCOL_IDENTIFIER "http"
#define HTTPS_PROTOCOL_IDENTIFIER "https"

class UrlParser {

  public:
    /*! Default constructor.*/
    UrlParser(const char* url);

    /*! Default constructor.*/
    UrlParser(shared_ptr<std::string> url);

    /*! Default de-constructor.*/
    virtual ~UrlParser();

    /*! Get full URL.

      @return Full URL.
    */
    shared_ptr<std::string> GetUrl();

    /*! Get protocol of an URL.

        @return protocol.
    */
    shared_ptr<std::string> GetProtocol();

    /*! Get location part of an URL.

       @return location.
    */
    shared_ptr<std::string> GetLocation();

    /*! Return hostname of URL.

        @return hostname.
    */
    shared_ptr<std::string> GetHostname();

    /*! Return Port.

      @note If original url does not contain Port, the default value
            (80 for HTTP and 443 for HTTPS) will be returned.

      @return port.
    */
    uint16 GetPort();

private:

    /*! Parse Url to get hostname and port.  */
    void ParseUrl();

private:
    shared_ptr<std::string> m_url;
    shared_ptr<std::string> m_protocol;
    shared_ptr<std::string> m_hostname;
    shared_ptr<std::string> m_location;
    uint16 m_port;
};

};

#endif /* _URLPARSER_H_ */


