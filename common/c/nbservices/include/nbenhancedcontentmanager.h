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

    @file     nbenchancedcontentmanager.h
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

#ifndef NBENHANCEDCONTENTMANAGER_H
#define NBENHANCEDCONTENTMANAGER_H

/*!
    @addtogroup nbenhancedcontentmanager
    @{
*/

#include "nbexp.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "navpublictypes.h"
#include "nbenhancedcontenttypes.h"

// Public Constants ..............................................................................


// Public Types ..................................................................................

typedef struct NB_EnhancedContentManager NB_EnhancedContentManager;

/*! Configuration used for enhanced content manager */
typedef struct
{
    uint32 screenWidth;                                                                 /*!< The screen width of the device, specified in pixels.*/
    uint32 screenHeight;                                                                /*!< The screen height of the device, specified in pixels.*/
    uint32 screenResolution;                                                            /*!< The resolution of the screen, specified in DPI */

    uint32 maxMotorwayJunctionsCacheSize;                                               /*!< maximum cache size for MJO objects. This is the cache entry/file count and NOT the size in bytes! */
    uint32 maxRealisticSignsCacheSize;                                                  /*!< maximum cache size for SAR objects. This is the cache entry/file count and NOT the size in bytes! */
    uint32 maxSpeedLimitImagesCacheSize;                                                /*!< maximum cache size for speed limits images objects. This is the cache entry/file count and NOT the size in bytes! */
    uint32 maxSpecialRegionsCacheSize;                                                  /*!< maximum cache size for special regions image objects. This is the cache entry/file count and NOT the size in bytes! */
    nb_boolean enhancedCityModelEnabled;                                                /*!< Enable Enhanced City Model (ECM) */
    nb_boolean motorwayJunctionObjectsEnabled;                                          /*!< Enable Motorway Junction Objects (MJO) */
    nb_boolean realisticSignsEnabled;                                                   /*!< Enable realistic signs (SAR) */
    nb_boolean speedLimitsSignEnabled;                                                  /*!< Enable speed limit signs (SAR) */
    nb_boolean specialRegionsEnabled;                                                   /*!< Enable special regions signs. */
    nb_boolean mapRegionsEnabled;                                                       /*!< Enable map regions. */

    NB_SupportedMapType supportedTypes;                                                 /*!< Types of maps this client can handle. */

    char country[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                          /*!< Country */
    char language[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                         /*!< Language */
    char nbgmVersion[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];                      /*!< NBGM version */

    char persistentMetadataPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                  /*!< Persistent metadata path */
    char mapRegionsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                          /*!< map regions data path */

    char enhancedCityModelPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                   /*!< ECM file path */
    char motorwayJunctionObjectPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];              /*!< MJO file path */
    char texturesPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                            /*!< textures path */
    char realisticSignsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                      /*!< SAR path */
    char speedLimitsSignsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                    /*!< speed limits images path */
    char specialRegionsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                      /*!< special region images path */

    char enhancedCityModelFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];        /*!< ECM file format */
    char motorwayJunctionObjectFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];   /*!< MJO file format */
    char realisticSignsFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];           /*!< SAR file format */
    char speedLimitsSignFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];          /*!< Speed limit image file format */
    char specialRegionsFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];           /*!< Special region image file format */
    char textureFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];                  /*!< Textures file format */
    char compressedTextureFileFormat[MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH + 1];        /*!< Textures compressed file format */

    char clientGuid[MAX_CLIENT_GUID_LENGTH + 1];                                        /*!< Client guid to be sent with http requests */

} NB_EnhancedContentManagerConfiguration;

/*! Callback function for enhanced content data synchronization result

    This callback reports the results of a call to NB_EnhancedContentManagerStartSynchronization().
    If successful, the current array of cities will be returned here, with each cities current state
    and current selection and deletion status.

    @return None
    @see NB_EnhancedContentManagerStartSynchronization
*/
typedef void (*NB_EnhancedContentSynchronizationCallbackFunction)(
                    NB_EnhancedContentManager* manager,                 /*!< Enhanced content manager instance */
                    NB_Error result,                                    /*!< Result of synchronization */
                    NB_RequestStatus status,                            /*!< Status of synchronization */
                    uint32 percent,                                     /*!< Progress in percent. Value from 0 - 100 */
                    uint32 addedCityCount,                              /*!< Returns number in array of added cities */
                    const NB_EnhancedContentCityData* addedCities,      /*!< Returns pointer to array of added cities */
                    uint32 updatedCityCount,                            /*!< Returns number in array of updated cities */
                    const NB_EnhancedContentCityData* updatedCities,    /*!< Returns pointer to array of updated cities */
                    uint32 deletedCityCount,                            /*!< Returns number in array of deleted cities */
                    const NB_EnhancedContentCityData* deletedCities,    /*!< Returns pointer to array of deleted cities */
                    void* userData                                      /*!< User data specified in NB_EnhancedContentManagerCreate() */
                    );

/*! Structure for enhanced content synchronization callback data */
typedef struct
{
    NB_EnhancedContentSynchronizationCallbackFunction   callback;       /*!< Callback function */
    void*                                               callbackData;   /*!< Callback data */
} NB_EnhancedContentSynchronizationCallback;

/*! Callback function for enhanced content data download result

    TODO: fix callback signature/status info returned for errors etc.

    This callback returns status updates for background preload downloading that is taking place.
    Called for notification when download starts, pauses, resumes, finishes, and stops due to errors.

    TODO: Are periodic progress calls needed for client update as well?

    @return None
    @see NB_EnhancedContentManagerSelectCity
*/
typedef void (*NB_EnhancedContentDownloadCallbackFunction)(
                    NB_EnhancedContentManager* manager,                 /*!< Enhanced content manager instance */
                    NB_Error result,                                    /*!< Result of download */
                    uint32 percent,                                     /*!< Progress in percent. Value from 0 - 100 */
                    NB_EnhancedContentDataType dataType,                /*!< Current downloading data type */
                    const char* datasetId,                              /*!< Current downloading dataset id */
                    void* userData                                      /*!< User data specified in NB_EnhancedContentManagerCreate() */
                    );

/*! Structure for enhanced content download callback data */
typedef struct
{
    NB_EnhancedContentDownloadCallbackFunction   callback;              /*!< Callback function */
    void*                                        callbackData;          /*!< Callback data */
} NB_EnhancedContentDownloadCallback;


// Public Functions ..............................................................................

/*! Create a new instance of a NB_EnhancedContentManager object

Use this function to create a new instance of a NB_EnhancedContentManager object.
Call NB_EnhancedContentManagerDestroy() to destroy the object.

@return NE_OK for success, NE_NOMEM if memory allocation failed
@see NB_EnhancedContentManagerDestroy
*/
NB_DEC NB_Error NB_EnhancedContentManagerCreate(
    NB_Context* context,                                                /*!< Pointer to current context */
    const NB_EnhancedContentManagerConfiguration* configuration,        /*!< Enhanced content manager configuration parameters */
    NB_EnhancedContentSynchronizationCallback* synchronizationCallback, /*!< Callback for content synchronization results */
    NB_EnhancedContentDownloadCallback* downloadCallback,               /*!< Callback for download progress */
    NB_EnhancedContentManager** manager                                 /*!< On success, returns pointer to enhanced content manager object */
    );

/*! Destroy an instance of a NB_EnhancedContentManager object
 
Use this function to destroy an existing instance of a NB_EnhancedContentManager object created by
NB_EnhancedContentManagerCreate().

@return NE_OK for success
@see NB_EnhancedContentManagerCreate
*/
NB_DEC NB_Error NB_EnhancedContentManagerDestroy(
    NB_EnhancedContentManager* manager              /*!< Enhanced content manager to destroy */
    );

/*! Start enhanced content metadata synchronization with server

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerStartMetadataSynchronization(
    NB_EnhancedContentManager* manager              /*!< Enhanced content manager instance */
    );

/*! Set current location

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerSetLocation(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_LatitudeLongitude* location                  /*!< Current location */
    );

/*! Select city for enhanced content download

Remove stored enhanced content data of city unselected.

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerSelectCity(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* cityId,                             /*!< City identifier */
    nb_boolean selected                             /*!< Is city selected for download? */
    );

/*! Select region for content download

Remove stored enhanced content data of region if unselected.

@return NE_OK for success
*/
NB_DEC NB_Error NB_ContentManagerSelectMapRegion(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* regionId,                           /*!< region identifier */
    nb_boolean selected                             /*!< Is region selected for download? */
    );

/*! Remove stored enhanced content data of city

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerRemoveCity(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* cityId                              /*!< City identifier */
    );

/*! Remove stored content data of region

@return NE_OK for success
*/
NB_DEC NB_Error NB_ContentManagerRemoveRegion(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* regionId                            /*!< region identifier */
    );

/*! Start enhanced content manifest synchronization with server

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerStartManifestSynchronization(
    NB_EnhancedContentManager* manager              /*!< Enhanced content manager instance */
    );

/*! Set enhanced content manifest synchronization allowable

@return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedContentManagerSetManifestSynchronizationAllowable(
    NB_EnhancedContentManager* manager,              /*!< Enhanced content manager instance */
    nb_boolean manifestSynchronizationAllowable      /*!< Is manifest synchronization allowable? */
    );

/*! Pause item downloading by download type

@return NE_OK for success
@see NB_EnhancedContentManagerResumeDownload
*/
NB_DEC NB_Error NB_EnhancedContentManagerPauseDownload(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType     /*!< Specify which download to pause */
    );

/*! Resume item downloading by download type

@return NE_OK for success
@see NB_EnhancedContentManagerPauseDownload
*/
NB_DEC NB_Error NB_EnhancedContentManagerResumeDownload(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType     /*!< Specify which download to resume */
    );

/*! Disable a download level of enhanced content manager.

    @return NE_OK if succeeded.
*/
NB_DEC NB_Error
NB_EnhancedContentManagerDisableDownloadLevel(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType,    /*!< Specify which download to resume */
    NB_EnhancedContentDownloadLevel level           /*!< Download level to be disabled */
    );

/*! Enable a download level of enhanced content manager.

    @return NE_OK if succeeded.
*/
NB_DEC NB_Error
NB_EnhancedContentManagerEnableDownloadLevel(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType,    /*!< Specify which download to resume */
    NB_EnhancedContentDownloadLevel level           /*!< Download level to be disabled */
    );

/*! Report whether the download queue of special level is enabled or not.

    @return TRUE if it is enabled.
*/
NB_DEC nb_boolean
NB_EnhancedContentManagerIsDownloadLevelEnabled(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType,    /*!< Specify which download to resume */
    NB_EnhancedContentDownloadLevel level           /*!< Download level to be disabled */
    );

/*! Report if there is metadata synchronization in progress

@return TRUE if metadata synchronization in progress, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsMetadataSynchronizationInProgress(
    NB_EnhancedContentManager* manager              /*!< Enhanced content manager instance */
    );

/*! Report if there is manifest synchronization in progress

@return TRUE if manifest synchronization in progress, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsManifestSynchronizationInProgress(
    NB_EnhancedContentManager* manager              /*!< Enhanced content manager instance */
    );

/*! Report if there is preload or on demand download in progress

Set argument 'downloadType' NB_ECDO_All if check both preload and on demand are in progress

@return TRUE if preload or on demand download in progress, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsDownloadInProgress(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType     /*!< Download type */
    );

/*! Stop downloading and remove all pending download requests

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerCancelDataDownload(
    NB_EnhancedContentManager* manager       /*!< Enhanced content manager instance */
    );

/*! Remove all stored enhanced content data from the device

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerRemoveAllStoredData(
    NB_EnhancedContentManager* manager       /*!< Enhanced content manager instance */
    );

/*! Get the current enhanced data storage statistics

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetStorageStatistics(
    NB_EnhancedContentManager* manager,      /*!< Enhanced content manager instance */
    uint64* totalSize,                       /*!< Buffer for total bytes needed for current city selection, including
                                                  bytes for ECM tiles and all textures (city specific, common, and MJO) */
    uint64* downloadedSize,                  /*!< Buffer for total bytes currently downloaded for city selection */
    uint32* totalCommonTexturesSize,         /*!< Buffer for total bytes needed for common and MJO textures */
    uint32* downloadedCommonTexturesSize,    /*!< Buffer for total bytes currently downloaded for common and MJO textures */
    uint32* downloadedPercentage             /*!< Buffer for percentage of content that has been downloaded */
    );

/*! Get City download status

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCityDownloadStatus(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadStatus* downloadStatus            /*!< Download status */
    );

/*! Get the current available city list

Returns copy of array of cities. Returned array is allocated for caller and
should be freed with nsl_free() when no longer needed.

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCityList(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    uint32* cityCount,                                          /*!< Returns number in array of cities */
    NB_EnhancedContentCityData** cities                         /*!< Returns pointer to array of cities */
    );

/*! Get the current available region list

Returns copy of array of regions. Returned array is allocated for caller and
should be freed with nsl_free() when no longer needed.

@return NE_OK for success
*/
NB_DEC NB_Error NB_ContentManagerGetRegionList(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    uint32* count,                                              /*!< Returns number in array of regions */
    NB_ContentEntryData** regions                               /*!< Returns pointer to array of regions */
    );

/*! Get the current exist region file Ids on disk

Returns copy of array of file ids. Returned array is allocated for caller and
should be freed with nsl_free() when no longer needed.

@return NE_OK for success
*/
NB_DEC NB_Error NB_ContentManagerGetExistMapRegionFileIds(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    int32* count,                                               /*!< Returns number in array of regions */
    char*** fileIds                                             /*!< Returns pointer to array of regions */
    );

/*! Get the city summary display

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCitySummaryDisplay(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    const char** citySummaryDisplay                             /*!< City summary string from server */
    );

/*! Set allowable download type for city model

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerSetAllowableDownload(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    NB_EnhancedContentDownloadType downloadType                 /*!< Allowable download type for city model */
    );

/*! Get the next city to update based on the current location

@return NE_OK for success, NE_NOENT if no city need to update.
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetNextCityToUpdate(
    NB_EnhancedContentManager* manager,                         /*!< Enhanced content manager instance */
    uint32 maxCityIdSize,                                       /*!< Maximum size of city id */
    char* cityId                                                /*!< On success, returns city id */
    );

/*! Check if common textures available

    @return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsCommonTextureAvailable(
    NB_EnhancedContentManager* manager                          /*!< Enhanced content manager instance */
    );

/*! Check if motorway junction textures available

    @return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(
    NB_EnhancedContentManager* manager                          /*!< Enhanced content manager instance */
    );

/*! Check whether the download requests of ContentManager is waiting for the finish of metadata update.

    @return TRUE if so, or FALSE otherwise.
*/
NB_DEC nb_boolean
NB_EnhancedContentManagerDownloadIsWaitingForMetadata(NB_EnhancedContentManager* manager /*!< Enhanced content manager instance */
                                                      );

/*! Pause region downloading

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerPauseRegionDownload(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* regionId,                           /*!< Specify which download to pause */
    NB_EnhancedContentDownloadType downloadType     /*!< download type*/
    );

/*! Resume region downloading

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerResumeRegionDownload(
    NB_EnhancedContentManager* manager,             /*!< Enhanced content manager instance */
    const char* regionId,                           /*!< Specify which download to resume */
    NB_EnhancedContentDownloadType downloadType     /*!< download type*/
    );
/*! @} */

#endif
