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
   @file        poiimagemanagerimplementation.h

   Inherited from PoiImageManager, provide real functions to download images.
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




#ifndef POIIMAGEMANAGERIMPLEMENTATION_H
#define POIIMAGEMANAGERIMPLEMENTATION_H

/*!
    @addtogroup poiimagemanager
    @{
*/

extern "C"
{
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "palfile.h"
}
#include "palmath.h"
#include "poiimagemanager.h"
#include "poiimagelistener.h"
#include "poiimagedownloadrequest.h"

#include "downloadmanager.h"
#include "PoiImageCache.h"
#include <vector>


// Constants ....................................................................................


// Types ........................................................................................
namespace nbsearch
{

typedef std::vector<PoiImageIdentifierPtr> VecPoiIdentifiers;

/*! The POI image manager. */
class PoiImageManagerImplementation : public PoiImageManager
{
public:
    // Public Functions ..............................................................................................

    /*! PoiImageManagerImplementation constructor */
    PoiImageManagerImplementation();

    /*! PoiImageManagerImplementation destructor */
    virtual
    ~PoiImageManagerImplementation();

    /*! Initialize the PoiImageManagerImplementation.

      @return NE_OK if succeeded.
    */
    NB_Error
    Initialize(NB_Context* context,                   /*!< Pointer to current context */
               nbcommon::DownloadManager* downloader, /*!< Downloader to be used while downloading. */
               PoiImageListenerPtr listener,          /*!< A PoiImageListener object */
               PoiImageManagerConfigPtr configuration /*!< Configuration used for POI image manager */
               );

    /* Refer to descriptions in 'nbpoiimagemanager.h' */
    virtual NB_Error
    DownloadSingleSearchThumbnailImages(NB_SingleSearchInformation* information);

    virtual NB_Error
    DownloadSingleSearchImagesForPoi(NB_SingleSearchInformation* information,
                                     int resultIndex);

    virtual NB_Error
    DownloadSearchThumbnailImages(NB_SearchInformation* information);

    virtual NB_Error
    DownloadSearchImagesForPoi(NB_SearchInformation* information,
                               int resultIndex);

    virtual nbcommon::DataStreamPtr
    GetImageData(PoiImageIdentifierPtr identifier);

    virtual NB_Error
    CancelDownloading();

    /*! Get number of pending download requests. */
    virtual int
    GetPendingItemNumber();

    /*! Clear all cached images. */
    virtual NB_Error
    ClearAllCache();

    /*! Callback for the ImageDownloadRequest

        This function will get called when download is finished normally. It stores downloaded
        data into corresponding cache, and tell listener about this.
     */
    void
    DownloadFinished(PoiImageIdentifierPtr identifier,          /*!< Identifier of this image */
                     nbcommon::DataStreamPtr   stream           /*!< DataStream containing image data. */
                     );

    /*! Callback for ImageDownloadRequest

        This function will be called when download encountered an error. It checks the
        arguments, and tell listener about this.

        @return None.
     */
    void
    DownloadFailed(PoiImageIdentifierPtr identifier, /*!< Identifier of image. */
                   NB_Error error                    /*!< Result returned from downloader .*/
                   );

    /*! Helper function to tell listener the remove of a cache item.
     */
    void


    ImageRemoved(PoiImageIdentifierPtr identifier);;

private:
    // Private Functions .......................................................................

    /*! Download images identified by PoiImageIdentifier;

        @return NE_OK if succeeded, or other value otherwise.
    */
    NB_Error
    DownloadIdentifiers(VecPoiIdentifiers& identifiers);

    /*! Convert an URL into a PoiImageIdentifier

        @note  Caller should make sure inputed url is not NULL;

        @return PoiImageIdentifierPtr represent this image.
    */
    PoiImageIdentifierPtr
    UrlToIdentifier(const char* url,   /*!< Poi to be converted. */
                    bool  isThumbnail  /*!< Flag to indicate whether this is thumbnail image. */
                    );

    /*! Get number of POIs in the inputed information.

        @note We want to download images for POIs, so return 0 if this information does not
              contains POIs (ie, StyleType of this information is neither NB_SRST_SingleSearch
              nor NB_SRST_Geocode).

        @return number of POIs.
    */
    uint32
    GetPoiCount(NB_SingleSearchInformation* information   /*!< Information to be checked. */
               );

    /*! Check whether the specified image exists in cache.

        @return TRUE if in cache, or FALSE otherwise.
    */
    bool
    ImageExistsInCache(PoiImageIdentifierPtr identifier   /*!< Identifier of image to be searched. */
                      );

    // Private variables .......................................................................
    NB_Context*               m_context;              /*!< Pointer to current context */
    PoiImageManagerConfigPtr  m_configuration;        /*!< Configuration used for POI image manager */
    PoiImageListenerPtr       m_listener;             /*!< Listener of this class. */
    nbcommon::DownloadManager* m_downloadManager;     /*!< Download manager instance */
    int                       m_pendingRequestNumber; /*!< Number of pending requests. */
    bool                      m_shouldStop;           /*!< A flag to indicate that the downloaded data
                                                        should not be written to cache for some
                                                        reason. */
    PoiImageCachePtr          m_thumbnailImageCache;  /*!< The CSL cache for thumbnail images */
    PoiImageCachePtr          m_largeImageCache;      /*!< The CSL cache for large images */};
};


/*! @} */

#endif
