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

  @file       poiimagemanagerimplementation.cpp

  Inherited from PoiImageManager, provide real functions to download images.
*/
/*
  (C) Copyright 2011 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary
  to TeleCommunication Systems, Inc., and considered a trade secret as
  defined in section 499C of the penal code of the State of
  California. Use of this information by anyone other than
  authorized employees of Networks In Motion is granted only
  under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  ---------------------------------------------------------------------------*/

#include "poiimagemanagerimplementation.h"

using namespace nbsearch;

/*! @{ */

const int    DEFAULT_PORT               = 80;
const uint32 POI_IMAGE_REQUEST_PRIORITY = 1;

typedef std::vector<nbcommon::DownloadRequestPtr> VecDownloadRequests;

// Refer to header file for descriptions.

PoiImageManagerImplementation::PoiImageManagerImplementation()
{
    m_listener             = PoiImageListenerPtr ();
    m_context              = NULL;
    m_downloadManager      = NULL;
    m_pendingRequestNumber = 0;
    m_shouldStop           = false;
    nsl_memset(&m_configuration, 0, sizeof(m_configuration));
}

PoiImageManagerImplementation::~PoiImageManagerImplementation()
{
    NB_Error error = NE_OK;
    int counter = 3; // Max times to retry.
    do {
        error = m_downloadManager->ClearRequests(nbcommon::POI_IMAGE_DOWNLOAD_LEVEL, true);
        counter --;
    } while (counter > 0 && error != NE_OK && error != NE_NOENT);
}

NB_Error
PoiImageManagerImplementation::Initialize(NB_Context* context,
                                          nbcommon::DownloadManager* downloader,
                                          PoiImageListenerPtr listener,
                                          PoiImageManagerConfigPtr configuration)
{
    m_listener        = listener;
    m_context         = context;
    m_downloadManager = downloader;
    m_configuration   = configuration;

    PAL_Instance *pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return NE_INVAL;
    }

    PoiImageCacheFactory cacheFactory(pal, this, m_configuration);

    m_thumbnailImageCache = cacheFactory.CreateInstance(true);
    m_largeImageCache     = cacheFactory.CreateInstance(false);

    if (!m_largeImageCache->IsOk() || !m_thumbnailImageCache->IsOk())
    {
        return NE_NOTINIT;
    }
    return NE_OK;
}

NB_Error
PoiImageManagerImplementation::DownloadSingleSearchThumbnailImages(NB_SingleSearchInformation* information)
{
    if (information == NULL)
    {
        return NE_INVAL;
    }

    uint32 count = GetPoiCount(information);
    uint32 index = 0;
    VecPoiIdentifiers identifiers;
    for (; index < count; index++) {
        const char *url = NB_SingleSearchInformationGetPoiImageUrl(information, index, TRUE);
        if (url)
        {
            PoiImageIdentifierPtr identifier = UrlToIdentifier(url, true);
            identifiers.push_back(identifier);
        }
    }
    return DownloadIdentifiers(identifiers);
}


NB_Error
PoiImageManagerImplementation::DownloadSingleSearchImagesForPoi(NB_SingleSearchInformation* information,
                                                                int index)
{
    if (information == NULL)
    {
        return NE_INVAL;
    }
    NB_Error error = NE_INVAL;
    uint32 count = GetPoiCount(information);
    if ((uint32)index < count && index >= 0)
    {
        VecPoiIdentifiers identifiers;
        const char* url = NB_SingleSearchInformationGetPoiImageUrl(information, index, FALSE);
        if (url)
        {
            PoiImageIdentifierPtr identifier = UrlToIdentifier(url, false);
            identifiers.push_back(identifier);
        }

        url = NB_SingleSearchInformationGetPoiImageUrl(information, index, TRUE);
        if (url)
        {
            PoiImageIdentifierPtr identifier = UrlToIdentifier(url, true);
            identifiers.push_back(identifier);
        }

        error = DownloadIdentifiers(identifiers);
    }

    return error;
}

/*! Not implemented. */
NB_Error
PoiImageManagerImplementation::DownloadSearchThumbnailImages(NB_SearchInformation* information)
{
    return NE_NOSUPPORT;
}

/*! Not implemented. */
NB_Error
PoiImageManagerImplementation::DownloadSearchImagesForPoi(NB_SearchInformation* information,
                                                          int resultIndex)
{
    return NE_NOSUPPORT;
}

NB_Error
PoiImageManagerImplementation::CancelDownloading()
{
    m_pendingRequestNumber = 0;
    NB_Error error = m_downloadManager->ClearRequests(nbcommon::POI_IMAGE_DOWNLOAD_LEVEL, false);

    if (error == NE_NOENT) // Return NE_OK if there's no requests to cancel.
    {
        error = NE_OK;
    }
    return error;
}

NB_Error
PoiImageManagerImplementation::ClearAllCache()
{
    NB_Error error = m_thumbnailImageCache->ClearCache();
    error = error ? error : m_largeImageCache->ClearCache();
    return error;
}

nbcommon::DataStreamPtr
PoiImageManagerImplementation::GetImageData(PoiImageIdentifierPtr identifier)
{
    return identifier->IsThumbnail() ? m_thumbnailImageCache->GetImageData(identifier) :
            m_largeImageCache->GetImageData(identifier);
}


PoiImageIdentifierPtr
PoiImageManagerImplementation::UrlToIdentifier(const char* url,
                                               bool isThumbnail
                                               )
{
    PoiImageIdentifier* identifier = NULL;
    if (url && nsl_strlen(url) != 0)
    {
        shared_ptr<std::string> utfUrl = shared_ptr<std::string>(new std::string(url));
        identifier = new PoiImageIdentifierImplementation(utfUrl, isThumbnail);
    }
    return PoiImageIdentifierPtr(identifier);
}

NB_Error
PoiImageManagerImplementation::DownloadIdentifiers(VecPoiIdentifiers& identifiers)
{
    NB_Error err = NE_OK;
    m_pendingRequestNumber += identifiers.size();

    // Check cache before download, if image exists in cache, do following steps:
    //        1. Remove it from requests,
    //        2. Tell the Listener,
    //        3. Check next request.
    // At last, call download manager to process the remained requests.
    VecPoiIdentifiers::iterator iter;
    VecDownloadRequests requests;
    for (iter = identifiers.begin(); iter < identifiers.end(); iter++)
    {
        if (ImageExistsInCache(*iter)) // Image existed, tell listener and process next one.
        {
            m_pendingRequestNumber --;
            m_listener->ImageDownloaded(NE_OK, (*iter));
        }
        else
        {
            nbcommon::DownloadRequestPtr request =
                    nbcommon::DownloadRequestPtr(new ImageDownloadRequest(*iter, POI_IMAGE_REQUEST_PRIORITY, this));
            requests.push_back(request);
        }
    }

    if (!requests.empty())
    {
        err = m_downloadManager->RequestData(requests, nbcommon::POI_IMAGE_DOWNLOAD_LEVEL, NULL);
    }

    return err;
}

bool
PoiImageManagerImplementation::ImageExistsInCache(PoiImageIdentifierPtr identifier)
{
    return identifier->IsThumbnail() ? m_thumbnailImageCache->ImageExistsInCache(identifier) :
            m_largeImageCache->ImageExistsInCache(identifier);
}

void
PoiImageManagerImplementation::DownloadFinished(PoiImageIdentifierPtr identifier,
                                                nbcommon::DataStreamPtr stream
                                                )
{
    NB_Error error = NE_OK;
    if ((!identifier) || (!stream) || m_shouldStop)
    {
        error = NE_INVAL;
    }
    else if (identifier->IsThumbnail())
    {
        error = m_thumbnailImageCache->SaveImageToCache(identifier, stream);
    }
    else
    {
        error = m_largeImageCache->SaveImageToCache(identifier, stream);
    }

    if (error != NE_OK)
    {
        DownloadFailed(identifier, error);
    }

    m_pendingRequestNumber--;
    m_listener->ImageDownloaded(NE_OK, identifier);
}

void
PoiImageManagerImplementation::DownloadFailed(PoiImageIdentifierPtr identifier,
                                              NB_Error error)
{
    if (identifier)
    {
        m_pendingRequestNumber--;
        m_listener->ImageDownloaded(error, identifier);
    }
}

int PoiImageManagerImplementation::GetPendingItemNumber()
{
    return m_pendingRequestNumber;
}

void PoiImageManagerImplementation::ImageRemoved(PoiImageIdentifierPtr identifier)
{
    if (identifier)
    {
        m_listener->ImageRemoved(identifier);
    }
}

uint32
PoiImageManagerImplementation::GetPoiCount(NB_SingleSearchInformation* information)
{
    uint32 count = 0;
    NB_SearchResultStyleType type = NB_SRST_None;
    NB_Error error = NB_SingleSearchInformationGetResultStyleType(information, &type);
    if ((error == NE_OK) && (type == NB_SRST_Geocode || type == NB_SRST_SingleSearch))
    {
        error = NB_SingleSearchInformationGetResultCount(information, &count);
    }
    return error == NE_OK ? count : 0;
}
