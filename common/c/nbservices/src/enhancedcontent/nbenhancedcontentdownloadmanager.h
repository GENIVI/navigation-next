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

    @file     nbenhancedcontentdownloadmanager.h
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

#ifndef NBENHANCEDCONTENTDOWNLOADMANAGER_H
#define NBENHANCEDCONTENTDOWNLOADMANAGER_H

/*!
    @addtogroup nbenhancedcontentdownloadmanager
    @{
*/

#include "nbenhancedcontentmanagerprivate.h"
#include "nbenhancedcontentdatasource.h"
#include "nbenhancedcontentmanager.h"


// Constants ....................................................................................


// Types ........................................................................................

typedef struct NB_EnhancedContentDownloadManager NB_EnhancedContentDownloadManager;

/*! Callback to notify the progress of downloaded items. */
typedef void (*NB_EnhancedContentDownloadManagerCallback)(NB_EnhancedContentDownloadManager* manager,
                        NB_Error result,                            /*!< Result of download request. */
                        uint32 percent,                             /*!< Progress in percent. Value from 0 - 100 */
                        const NB_EnhancedContentIdentifier* item,   /*!< Downloaded item. Can be NULL on general error (e.g. connection error). */
                        void* userData                              /*!< User data specified in NB_EnhancedContentDownloadManagerCreate() */
                        );


/*! Structure for callback data */
typedef struct 
{
    NB_EnhancedContentDownloadManagerCallback   callback;
	void*                                       userData;

} NB_EnhancedContentDownloadManagerCallbackData;

// Functions ....................................................................................

/*! Create a new instance of a NB_EnhancedContentDownloadManager object

    Use this function to create a new instance of a NB_EnhancedContentDownloadManager object.
    Call NB_EnhancedContentDownloadManagerDestroy() to destroy the object.

    @return NE_OK for success, NE_NOMEM if memory allocation failed
    @see NB_EnhancedContentManagerDestroy
*/
NB_Error 
NB_EnhancedContentDownloadManagerCreate(
    NB_Context* context,                                                /*!< Pointer to current context */
    const NB_EnhancedContentManagerConfiguration* configuration,
    NB_EnhancedContentDataSource* dataSource,                           /*!< Instance of data source. The data source has to be initialized before calling this function */
    const NB_EnhancedContentDownloadManagerCallbackData* callbackData,  /*!< Callback for download requests */
    NB_EnhancedContentDownloadManager** instance                        /*!< On success, returns pointer to enhanced content download manager */
    );

/*! Destroy an instance of a NB_EnhancedContentManager object

    Use this function to destroy an existing instance of a NB_EnhancedContentDownloadManager object created by
    NB_EnhancedContentDownloadManagerCreate().

    @return NE_OK for success
    @see NB_EnhancedContentDownloadManagerCreate
*/
NB_Error
NB_EnhancedContentDownloadManagerDestroy(
    NB_EnhancedContentDownloadManager* instance                 /*!< Enhanced content download manager instance */
    );

/*! Request a data item for downloading.

    Request either preload or on-demand data. To download entire datasets set the itemID to NULL in the passed in
    NB_EnhancedContentIdentifier. Only items which are not yet downloaded or out of sync (different version) are
    downloaded.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerRequestData(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* itemId,                 /*!< Item to request */
    const char* fileVersion,                                    /*!< File verion of MJO/SAR/SpeedLimit */
    NB_EnhancedContentDownloadType requestType                  /*!< Set to either preload or on-demand download type */
    );

/*! Remove persistent data from device storage

    If dataId is NULL remove the entire dataset.

    @return NB_Error NE_NOENT if there is no such data
*/
NB_Error
NB_EnhancedContentDownloadManagerRemoveData(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* itemId,                 /*!< Item to remove */
    nb_boolean zipped                                           /*!< If TRUE build zipped file path or unzipped folder path
                                                                     Only available for textures */
    );

/*! Cancel pending requests.

    This cancels either preload, on-demand or all requests.
    You can also request to cancel a specific data content type.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerCancelRequests(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType,                 /*!< Set to either preload, on-demand or all downloads */
    NB_EnhancedContentDataType dataType                         /*!< Data type to cancel. Set to NB_ECDT_All to cancel all requests */
    );

/*! Cancel pending request of item data

    @todo: Only support dataset now.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerCancelRequestData(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType,                 /*!< Set to either preload, on-demand or all downloads */
    const NB_EnhancedContentIdentifier* itemId                  /*!< Item to cancel */
);

/*! Pause download

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerPauseDownload(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType                  /*!< Specify which download to pause. */
    );

/*! Resume download

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerResumeDownload(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType                  /*!< Specify which download to resume */
    );

/*! Checks if download requests are currently pending

    @return TRUE if download requests are pending, FALSE otherwise
*/
nb_boolean
NB_EnhancedContentDownloadManagerIsPending(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType                  /*!< Download type to check */
    );

/*! Checks if downloads are currently paused

    @return TRUE if download requests are currently paused, FALSE otherwise
*/
nb_boolean
NB_EnhancedContentDownloadManagerIsPaused(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType                  /*!< Download type to check */
    );

/*! Disable a download queue of specific level.

    @return NB_OK if succeeded.
*/
NB_Error
NB_EnhancedContentDownloadManagerDisableLevel(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType,
    NB_EnhancedContentDownloadLevel    level                    /*!< Download level to level to be disabled */
    );


/*! Enable a download queue of specific level.

    @return NB_OK if succeeded.
*/
NB_Error
NB_EnhancedContentDownloadManagerEnableLevel(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    NB_EnhancedContentDownloadType requestType,                 /*!< Download type to to be operated. */
    NB_EnhancedContentDownloadLevel    level                    /*!< Download level to level to be enabled */
    );

/*! Check whether download queue of specified level is enabled or not.

    @return TRUE if enabled.
*/
nb_boolean
NB_EnhancedContentDownloadManagerIsLevelEnabled(NB_EnhancedContentDownloadManager* instance,    /*!< Enhanced content download manager instance */
                                                NB_EnhancedContentDownloadType requestType,     /*!< Download type to to be operated. */
                                                NB_EnhancedContentDownloadLevel    level        /*!< Download level to level to be enabled */
                                                );

/*! Checks if data item is available

    @return NB_OK if success
*/
nb_boolean
NB_EnhancedContentDownloadManagerIsDataItemAvailable(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* itemId                  /*!< Item to check if available */
    );

/*! Get data item data bytes

    Returns copy of requested data item data bytes.  Returned buffer is allocated for caller and
    should be freed with nsl_free() when no longer needed.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerGetDataItem(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* itemId,                 /*!< Data item to get data bytes for */
    uint32* dataSize,                                           /*!< On return buffer for copied data size */
    uint8** data                                                /*!< On return buffer for pointer to copy of requested data */
    );

/*! Check storage consistency

    Removes files stored on device that are not in the manifest.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerDoStorageConsistencyCheck(
    NB_EnhancedContentDownloadManager* instance                 /*!< Enhanced content download manager instance */
    );

/*! Clear all cached downloaded files on device.

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerMasterClear(
    NB_EnhancedContentDownloadManager* instance                 /*!< Enhanced content download manager instance */
    );

/*! Set the status of waitingMetaDate flag of download manager.

    @return NB_OK if succeeded.
*/
NB_Error
NB_EnhancedContentDownloadManagerSetWaitingMetadata(NB_EnhancedContentDownloadManager* manager, /*!< Enhanced content download manager instance */
                                                    nb_boolean waiting /*!< Flag to be set. */
                                                    );
/*! Check whether this download manager is waiting for the update of metadata.

    @return TRUE if so, or FALSE otherwise.
*/
nb_boolean
NB_EnhancedContentDownloadManagerIsWaitingMetadata( NB_EnhancedContentDownloadManager* instance  /*!< Enhanced content download manager instance */
                                                    );
/*! Pause download

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerPauseItemDownload(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* item,                   /*!< Download item. */
    NB_EnhancedContentDownloadType downloadType                 /*!< download type*/
    );

/*! Resume download

    @return NB_OK if success
*/
NB_Error
NB_EnhancedContentDownloadManagerResumeItemDownload(
    NB_EnhancedContentDownloadManager* instance,                /*!< Enhanced content download manager instance */
    const NB_EnhancedContentIdentifier* item,                   /*!< Download item. */
    NB_EnhancedContentDownloadType downloadType                 /*!< download type*/
    );

/*! @} */

#endif
