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
   @file        poiimagedownloadrequest.cpp

   Implementation of ImageDownloadRequest.
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

#include "poiimagedownloadrequest.h"
#include "poiimagemanagerimplementation.h"
#include "urlparser.h"

using namespace nbsearch;

static const int   RETRY_TIME   = 3;

ImageDownloadRequest::ImageDownloadRequest(PoiImageIdentifierPtr identifier,
                                           uint32 priority,
                                           PoiImageManager* manager) \
    : DownloadRequest(priority)

{
    m_identifier   = identifier;
    m_imageManager = manager;
    m_retryTimes   = RETRY_TIME;

    if (m_identifier)
    {
        nbcommon::UrlParser parser(m_identifier->GetUrl());
        m_hostname = parser.GetHostname();
        m_url      = parser.GetLocation();
        m_port     = parser.GetPort();

        // Set DOWNLOAD_PROTOCOL_HTTP as default value.
        m_protocol = nbcommon::DOWNLOAD_PROTOCOL_HTTP;

        //Parse the protocol.
        shared_ptr<string> protocol = parser.GetProtocol();
        if (protocol && (!(protocol->empty())))
        {
            const char* protocolChar = protocol->c_str();
            if (protocolChar && (nsl_strlen(protocolChar) > 0))
            {
                if (nsl_stricmp(protocolChar, HTTP_PROTOCOL_IDENTIFIER) == 0)
                {
                    m_protocol = nbcommon::DOWNLOAD_PROTOCOL_HTTP;
                }
                else if (nsl_stricmp(protocolChar, HTTPS_PROTOCOL_IDENTIFIER) == 0)
                {
                    m_protocol = nbcommon::DOWNLOAD_PROTOCOL_HTTPS;
                }
            }
        }
    }
}

ImageDownloadRequest::~ImageDownloadRequest()
{
}

nbcommon::DownloadProtocol ImageDownloadRequest::GetProtocol()
{
    return m_protocol;
}

shared_ptr<std::string> ImageDownloadRequest::GetHostname()
{
    return m_hostname;
}

uint16 ImageDownloadRequest::GetPort()
{
    return m_port;
}

shared_ptr<std::string> ImageDownloadRequest::GetUrl()
{
    return m_url;
}

void ImageDownloadRequest::DuplicatedRequestWillBeIgnored(nbcommon::DownloadRequestPtr duplicatedRequest)
{
    // Nothing to do here.
}

void ImageDownloadRequest::AddedToDownloadQueue(nbcommon::DownloadRequestPtr request)
{
    // Nothing to do here.
}

void ImageDownloadRequest::RemovedFromDownloadQueue(nbcommon::DownloadRequestPtr request)
{
    // Nothing to do here.
}

void ImageDownloadRequest::DataAvailable(uint32 size)
{
    if (size == 0)  // Download finished, Save to cache.
    {
        nbcommon::DataStreamPtr stream = GetDataStream();
        ((PoiImageManagerImplementation*)m_imageManager)->DownloadFinished(m_identifier, stream);
    }
    else
    {
        // Download is not finished, just wait until it is done.
        ;
    }
}

void ImageDownloadRequest::DataError(NB_Error error)
{
    ((PoiImageManagerImplementation*)m_imageManager)->DownloadFailed(m_identifier, error);
}

