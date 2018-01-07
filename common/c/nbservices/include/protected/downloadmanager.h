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

    @file       downloadmanager.h

    Class 'DownloadManager' is an interface. User could request or cancel
    downloading data by the functions of 'DownloadManager'. The level should
    be specified when user requests or cancels download data. The download
    sequence is from small download level to large. Each request has a download
    priority. The requests are sorted by the download priority of request
    in the download queue.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
}

#include <vector>
#include "downloadrequest.h"

namespace nbcommon
{

// Constants ....................................................................................

#define INVALID_DOWNLOAD_LEVEL ((uint32)-1)


// Types ........................................................................................

/*! Download levels for generic HTTP download manager */
typedef enum
{
    // High levels for extension.

    POI_IMAGE_DOWNLOAD_LEVEL = 1000,    /*!< Level to download POI images */
    TILE_DOWNLOAD_LEVEL_BASE = 2000     /*!< Base value to download tiles. This value is used to be
                                             added by 'download-priority' (0 - 255) from metadata
                                             protocol. The result of addition is the download level. */

    // Below value must be greater than 2255.

} GenericHttpDownloadLevel;

/*! Download manager interface */
class DownloadManager
{
public:
    // Interface Functions ......................................................................

    /*! DownloadManager destructor */
    virtual ~DownloadManager()
    {
        // Nothing to do here.
    }

    /*! Request downloading data

        This function creates a download level if the specified download level does not exist.
        Requests are added to the specified download level and sorted by the download priority
        of request.

        This function always removes duplicated requests in requested level (specified by
        parameter 'level'). User could specify download levels which may already contain these
        requests by parameter 'duplicatedLevels'.
        @todo: I think there is only one case to use this parameter. It is ECM downloading for
               preload and on demand. So I think it is useless to check duplicated requests for
               all download levels.

        There are some rules for adding request in this function:
        1. If the request is downloading in progress, the new request is ignored no matter
           the download level and priority of this request.
        2. If the request is found in high (small value) download level or same download level
           with high download priority, this request is ignored.
        3. If the request is found in low (large value) download level or same download level
           with low download priority and it is not downloading in progress, the found request
           is removed and new request is added.
        4. The request is added directly if this request does not exist in duplicated levels.

        @return NE_OK if success
    */
    virtual NB_Error
    RequestData(const std::vector<DownloadRequestPtr>& requests,            /*!< A vector of 'DownloadRequest' objects to request */
                uint32 level,                                               /*!< Download level to request */
                std::vector<uint32>* duplicatedLevels                       /*!< Download levels contained duplicated requests with
                                                                                 parameter 'requests'. If it is NULL there are no
                                                                                 duplicated levels. */
                ) = 0;

    /*! Cancel downloading request by the specified 'DownloadRequest' object

        There are no callback functions called for this cancelled operation.

        @return NE_OK if success, NE_NOENT if this request does not exist.
    */
    virtual NB_Error
    CancelRequest(DownloadRequestPtr request                                /*!< DownloadRequest' object to cancel request */
                  ) = 0;

    /*! Clear pending requests of download level

        There are no callback functions called for this cleared operation (or cancelled
        operation for each request).

        @return NE_OK if success, NE_NOENT if this download level does not exist.
    */
    virtual NB_Error
    ClearRequests(uint32 level,                                             /*!< Download level to clear requests */
                  bool inProgress                                           /*!< Whether to clear requests which are downloading
                                                                                 in progress */
                   ) = 0;

    /*! Remove the specified download level

        This function is called when the specified download level is useless. All requests
        of pending and downloading in progress are cancelled and cleared. There are no
        callback functions called for this operation.

        @return NE_OK if success, NE_NOENT if this download level does not exist.
    */
    virtual NB_Error
    RemoveLevel(uint32 level                                                /*!< Download level to remove */
                ) = 0;

    /*! Pause downloading of download level

        @return NE_OK if success
    */
    virtual NB_Error
    PauseDownloading(uint32 level,                                          /*!< Download level to pause downloading */
                     bool inProgress                                        /*!< Whether to cancel requests which are downloading
                                                                                 in progress */
                     ) = 0;

    /*! Resume downloading of download level

        @return NE_OK if success
    */
    virtual NB_Error
    ResumeDownloading(uint32 level                                          /*!< Download level to resume downloading */
                      ) = 0;

    /*! Checks if the request of the specified download level is empty

        @return TRUE if the request is empty, FALSE otherwise
    */
    virtual bool
    IsEmpty(uint32 level                                                    /*!< Download level to check if the request is empty */
            ) = 0;

    /*! Checks if the downloading for the specified download level is paused

        @return TRUE if the downloading is paused, FALSE otherwise
    */
    virtual bool
    IsPaused(uint32 level                                                   /*!< Download level to check if the downloading is paused */
             ) = 0;
};

typedef shared_ptr<DownloadManager> DownloadManagerSharedPointer;

};  // namespace nbmap

/*! @} */

#endif  // DOWNLOADMANAGER_H
