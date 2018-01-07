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

    @file       poiimagemanager.h

    Class PoiImageManager is used to download thumbnail and large images
    for search result. Thumbnail images are displayed in the search result
    list. Large image is displayed in the search result detail. Images are
    saved in the cache. If user requests for images, try to get them from
    the cache first. If images do not exist in the cache, try to download
    them from the specified URL and save the data of images in the cache.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef POIIMAGEMANAGER_H
#define POIIMAGEMANAGER_H

/*!
    @addtogroup poiimagemanager
    @{
*/
#ifdef __cplusplus
extern "C" {
#endif

#include "nbcontext.h"
#include "nbsinglesearchinformation.h"
#include "nbsearchinformation.h"

#ifdef __cplusplus
}
#endif

#include "poiimagelistener.h"
#include "datastream.h"

namespace nbsearch
{

// Constants ...................................................................................


// Types .......................................................................................

/*! Configuration used for POI image manager, these parameters should be configurable by users. */
typedef struct _PoiImageManagerConfiguration
{
    uint32 m_thumbnailImageCacheSize;   /*!< Maximum cache size for thumbnail images. This is
                                             the cache entry/file count and NOT the size in
                                             bytes. */
    uint32 m_largeImageCacheSize;       /*!< Maximum cache size for large images. This is the
                                             cache entry/file count and NOT the size in bytes. */
    std::string      m_cachePath;       /*!< Path to save cache */
} PoiImageManagerConfiguration;

typedef shared_ptr<PoiImageManagerConfiguration>   PoiImageManagerConfigPtr;

/*! The POI image manager interface */
class NB_DLL PoiImageManager
{
public:
    // Public functions .........................................................................

    /*! PoiImageManager destructor */
    virtual ~PoiImageManager();

    /*! Create a new instance pointer to PoiImageManager

        Use this function to create a new instance pointer to PoiImageManager.
        Call operator delete to destroy the object.

        @return NE_OK for success, NE_NOMEM if memory allocation failed
    */
    static NB_Error
    CreateInstance(NB_Context* context,                    /*!< Pointer to current context */
                   PoiImageListenerPtr listener,           /*!< A listener of PoiImageManager */
                   PoiImageManagerConfigPtr configuration, /*!< Configuration used for PoiImageManager */
                   PoiImageManager** manager               /*!< On success, returns pointer to
                                                                PoiImageManager */
                   );

    /*! Destroy an PoiImageManager instance.

        @return NE_OK when succeeded.
     */
    static NB_Error
    DestroyInstance(PoiImageManager* manager /*!< PoiImageManager instance to be destroyed.*/
                   );

    /*! Download thumbnail images for the specified single search result

        @note   Caller should cancel the remained download requests due to new request from CCI.
        @return NE_OK if success
    */
    virtual NB_Error
    DownloadSingleSearchThumbnailImages(NB_SingleSearchInformation* information /*!< A single search result */
                                        ) = 0;

    /*! Download both large and thumbnail images for the specified POI of single search information

        @note   Caller should cancel the remained download requests due to new request from CCI.
        @return NE_OK if success
    */
    virtual NB_Error
    DownloadSingleSearchImagesForPoi(NB_SingleSearchInformation* information,   /*!< A single search result */
                                     int resultIndex                            /*!< The index of the result for downloading large and thumbnail images */
                                     ) = 0;

    /*! Download thumbnail images for the specified search results other than single search.

        @todo This function is designed to download movie posts, and is not supported by
        present.

        @return NE_OK if success
    */
    virtual NB_Error
    DownloadSearchThumbnailImages(NB_SearchInformation* information /*!< A single search result */
                                 ) = 0;

    /*! Download both large and thumbnail images for the specified POI of results other than
        single search.

        @todo This function is designed to download movie posts, and is not supported by
        present.

        @return NE_OK if success
    */
    virtual NB_Error
    DownloadSearchImagesForPoi(NB_SearchInformation* information,   /*!< A single search result */
                               int resultIndex                      /*!< The index of the result for downloading large and thumbnail images */
                               ) = 0;

    /*! Get data of large or thumbnail image for the specified POI

        @return Shared pointer to a BinaryData object
    */
    virtual nbcommon::DataStreamPtr
    GetImageData(PoiImageIdentifierPtr identifier /*!< Identifier of the image to get */
                 ) = 0;

    /*! Cancel POI images downloading

        @return NE_OK if success, or other value otherwise.
    */
    virtual NB_Error
    CancelDownloading() = 0;

    /*! Get number of pending download requests. */
    virtual int GetPendingItemNumber() = 0;

    /*! Clear all cached images. */
    virtual NB_Error ClearAllCache() = 0;
};

};
/*! @} */

#endif
