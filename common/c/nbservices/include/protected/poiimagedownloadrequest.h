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

/*!----------------------------------------------------------------------------
   @file        poiimagedownloadrequest.h

   Inherited from DownloadRequest, class ImageDownloadRequest is the requests
   processed by HttpDownloadManager. Several member-functions get called from
   HttpDownloadManager when this request has been finished.
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

#ifndef _POIIMAGEDOWNLOADREQUEST_H_
#define _POIIMAGEDOWNLOADREQUEST_H_

#include "downloadrequest.h"
#include "poiimageidentifier.h"
#include "poiimagemanager.h"

namespace nbsearch
{
class ImageDownloadRequest : public nbcommon::DownloadRequest {
  public:
    /*! Default constructor. */
    ImageDownloadRequest(PoiImageIdentifierPtr identifier,	/*!< Identifier of the image to be downloaded.*/
                         uint32 priority,                   /*!< Download priority of the request */
                         PoiImageManager* manager           /*!< PiiImageManager instance. */
                         );

    virtual ~ImageDownloadRequest();

    /* Refer to description in 'downloadrequest.h' */
    virtual nbcommon::DownloadProtocol GetProtocol();

    /* Refer to description in 'downloadrequest.h' */
    virtual shared_ptr<std::string> GetHostname();

    /* Refer to description in 'downloadrequest.h' */
    virtual shared_ptr<std::string> GetUrl();

    /* Refer to description in 'downloadrequest.h' */
    virtual uint16 GetPort();

    /* Refer to description in 'downloadrequest.h' */

    /* Refer to description in 'downloadrequest.h' */
    virtual void DuplicatedRequestWillBeIgnored(nbcommon::DownloadRequestPtr duplicatedRequest);

    /* Refer to description in 'downloadrequest.h' */
    virtual void AddedToDownloadQueue(nbcommon::DownloadRequestPtr request);

    /* Refer to description in 'downloadrequest.h' */
    virtual void RemovedFromDownloadQueue(nbcommon::DownloadRequestPtr request);

    /* Refer to description in 'downloadrequest.h' */
    virtual void DataAvailable(uint32 size);

    /* Refer to description in 'downloadrequest.h' */
    virtual void DataError(NB_Error error);

private:
    // Private functions .......................................................................
    // Copy constructor and assignment operator are not supported.
    ImageDownloadRequest(const ImageDownloadRequest& requst);
    ImageDownloadRequest& operator=(const ImageDownloadRequest& request);

    // Private members .........................................................................
    uint16 m_port;                                  /*!< Port for testing */
    uint32 m_retryTimes;                            /*!< Retry times for testing */
    nbcommon::DownloadProtocol m_protocol;          /*!< Protocol of the request */
    shared_ptr<std::string> m_hostname;             /*!< Hostname for testing */
    shared_ptr<std::string> m_url;                  /*!< URL for testing */
    PoiImageIdentifierPtr  m_identifier;            /*!< Identifier of image to be downloaded. */
    PoiImageManager* m_imageManager;                /*!< PoiImagemanager holding this request */
};

}; // namespace nbmap.

#endif /* _POIIMAGEDOWNLOADREQUEST_H_ */
