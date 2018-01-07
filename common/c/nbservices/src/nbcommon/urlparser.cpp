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
   @file        urlparser.cpp

   Implementation of URL Parser.

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

#include "urlparser.h"
extern "C"
{
#include "palstdlib.h"
}
using namespace nbcommon;

const uint16 DEFAULT_HTTP_PORT = 80;
const uint16 DEFAULT_HTTPS_PORT = 443;

//static const char  URL_SEP      = '/';
static const char* URL_HEAD_SEP = "://";
static const int   HEAD_LENGTH  = nsl_strlen(URL_HEAD_SEP);

// Refer to header file for documents.
UrlParser::UrlParser(const char* url) : m_port(DEFAULT_HTTP_PORT)
{
    if (url && (nsl_strlen(url) != 0))
    {
        m_url = shared_ptr<std::string>(new std::string(url));
    }
    ParseUrl();
}

UrlParser::UrlParser(shared_ptr<std::string> url) :
    m_url(url),
    m_protocol(),
    m_hostname(),
    m_location(),
    m_port(DEFAULT_HTTP_PORT)
{
    ParseUrl();
}

UrlParser::~UrlParser()
{
}

void UrlParser::ParseUrl()
{
    if (!m_url)
        return;
    int length = m_url->length();
    if (length == 0)
        return;
    const char* ptr = m_url->c_str();
    if (ptr)
    {
        char* host = new char[length]; // We can not assume how long the hostname is.
        if (!host)
        {
            return;
        }
        nsl_memset(host, 0, length);

        char* protocol = new char[length];
        if (!protocol)
        {
            delete [] host;
            host = NULL;
            return;
        }
        nsl_memset(protocol, 0, length);

        int tmpPort = -1, length1 = -1, length2 = -1;
        int num = nsl_sscanf(ptr, "%[^://]://%[^:/]%n:%d%n", protocol, host, &length1, &tmpPort, &length2);
        if (num > 0)
        {
            if (tmpPort == -1)
            {
                // Set port to 80 as default value.
                m_port  = DEFAULT_HTTP_PORT;

                // Set default port according to the protocol (80 for HTTP and 443 for HTTPS).
                if (nsl_strlen(protocol) > 0)
                {
                    if (nsl_stricmp(protocol, HTTP_PROTOCOL_IDENTIFIER) == 0)
                    {
                        m_port = DEFAULT_HTTP_PORT;
                    }
                    else if (nsl_stricmp(protocol, HTTPS_PROTOCOL_IDENTIFIER) == 0)
                    {
                        m_port = DEFAULT_HTTPS_PORT;
                    }
                }

                ptr    += length1;
            }
            else
            {
                m_port  = (uint16) tmpPort;
                ptr    += length2;
            }
            m_protocol = shared_ptr<std::string>(new std::string(protocol));
            m_hostname = shared_ptr<std::string>(new std::string(host));
            m_location = shared_ptr<std::string>(new std::string(ptr));
        }
        delete[] protocol;
        delete[] host;
    }
}

shared_ptr<std::string> UrlParser::GetUrl()
{
    return m_url;
}

shared_ptr<std::string> UrlParser::GetProtocol()
{
    return m_protocol;
}

shared_ptr<std::string> UrlParser::GetHostname()
{
    return m_hostname;
}

shared_ptr<std::string> UrlParser::GetLocation()
{
    return m_location;
}

uint16 UrlParser::GetPort()
{
    return m_port;
}
