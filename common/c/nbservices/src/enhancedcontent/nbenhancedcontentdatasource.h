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

    @file     nbenhancedcontentdatasource.h
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

#ifndef NBENHANCEDCONTENTDATASOURCE_H
#define NBENHANCEDCONTENTDATASOURCE_H

/*!
    @addtogroup nbenhancedcontentdatasource
    @{
*/

#include "nbenhancedcontenttypes.h"
#include "nbenhancedcontentmanagerprivate.h"
#include "nbhandler.h"
#include "datautil.h"

// Public Constants ..............................................................................


// Public Types ..................................................................................

typedef struct NB_EnhancedContentDataSource NB_EnhancedContentDataSource;

/*! Callback function for metadata synchronization result

This callback reports the results of a call to NB_EnhancedContentDataSourceStartMetadataSynchronize().
If successful, the current array of cities will be returned here, with each cities current state
and current selection status.

    @return None
    @see NB_EnhancedContentDataSourceStartMetadataSynchronize
*/
typedef void (*NB_EnhancedContentDataSourceMetadataSynchronizeCallbackFunction)(
    NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
    NB_Error result,                                /*!< Result of metadata synchronize */
    NB_RequestStatus status,                        /*!< Status of metadata synchronize */
    uint32 percent,                                 /*!< Progress in percent. Value from 0 - 100 */
    CSL_Vector* deletedDatasetReplies,              /*!< Vector of NB_MetadataSourceReply. Valid if status is NB_NetworkRequestStatus_Success.
                                                         Should delete contained persistent datasets. */
    CSL_Vector* addedCities,                        /*!< Vector of NB_EnhancedContentCityData. Valid if status is NB_NetworkRequestStatus_Success.
                                                         Added cities only contain city id. */
    CSL_Vector* updatedCities,                      /*!< Vector of NB_EnhancedContentCityData. Valid if status is NB_NetworkRequestStatus_Success.
                                                         Updated cities only contain city id. */
    CSL_Vector* deletedCities,                      /*!< Vector of NB_EnhancedContentCityData. Valid if status is NB_NetworkRequestStatus_Success.
                                                         Deleted cities only contain city id. */
    void* userData                                  /*!< User data of NB_EnhancedContentDataSourceMetadataSynchronizeCallback */
    );

/*! Structure for metadata synchronization callback data */
typedef struct
{
    NB_EnhancedContentDataSourceMetadataSynchronizeCallbackFunction     callback;       /*!< Callback function */
    void*                                                               callbackData;   /*!< Callback data */
} NB_EnhancedContentDataSourceMetadataSynchronizeCallback;

/*! Callback function for manifest synchronization result

This callback reports the results of a call to NB_EnhancedContentDataSourceStartManifestSynchronize().

    @return None
    @see NB_EnhancedContentDataSourceStartManifestSynchronize
*/
typedef void (*NB_EnhancedContentDataSourceManifestSynchronizeCallbackFunction)(
            NB_EnhancedContentDataSource* dataSource,       /*!< Enhanced content data source instance */
            NB_Error result,                                /*!< Result of manifest synchronize */
            NB_RequestStatus status,                        /*!< Status of manifest synchronize */
            NB_EnhancedContentDataType contentType,         /*!< Content type of removed manifest replies */
            const char* datasetId,                          /*!< IDataset identifier of removed manifest replies */
            CSL_Vector* removedManifestReplies,             /*!< Vector of NB_MetadataManifestReply. Should delete contained persistent files. */
            void* userData                                  /*!< User data of NB_EnhancedContentDataSourceManifestSynchronizeCallback */
            );

/*! Structure for manifest synchronization callback data */
typedef struct
{
    NB_EnhancedContentDataSourceManifestSynchronizeCallbackFunction     callback;       /*!< Callback function */
    void*                                                               callbackData;   /*!< Callback data */
} NB_EnhancedContentDataSourceManifestSynchronizeCallback;


// Public Functions ..............................................................................

/*! Create a new instance of a NB_EnhancedContentDataSource object

    @return NB_OK if success
    @see NB_EnhancedContentDataSourceDestroy
*/
NB_Error NB_EnhancedContentDataSourceCreate(
    NB_Context* context,                                                                    /*!< Pointer to current context */
    const NB_EnhancedContentManagerConfiguration* managerConfiguration,                     /*!< Enhanced content manager configuration parameters */
    NB_EnhancedContentDataSourceMetadataSynchronizeCallback metadataSynchronizeCallback,    /*!< Callback for metadata synchronization */
    NB_EnhancedContentDataSourceManifestSynchronizeCallback manifestSynchronizeCallback,    /*!< Callback for manifest synchronization */
    NB_EnhancedContentDataSource** instance                                                 /*!< On success, returns pointer to enhanced content data source object */
    );

/*! Destroy an instance of a NB_EnhancedContentDataSource object

    @return NB_OK if success
    @see NB_EnhancedContentDataSourceCreate
*/
NB_Error NB_EnhancedContentDataSourceDestroy(
    NB_EnhancedContentDataSource* instance          /*!< Enhanced content data source instance */
    );

/*! Check if data source instance is valid for use

    @return NB_OK if success
*/
nb_boolean NB_EnhancedContentDataSourceIsValid(
    NB_EnhancedContentDataSource* instance          /*!< Enhanced content data source instance */
    );

/*! Start metadata query and synchronization with server

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceStartMetadataSynchronize(
    NB_EnhancedContentDataSource* instance          /*!< Enhanced content data source instance */
    );

/*! Start manifest query and synchronization with server

    @return NB_Error NE_NOENT if there is no need to update manifest
*/
NB_Error NB_EnhancedContentDataSourceStartManifestSynchronize(
    NB_EnhancedContentDataSource* instance         /*!< Enhanced content data source instance */
    );

/*! Cancel manifest query and synchronization with server

    @return NB_Error
*/
NB_Error NB_EnhancedContentDataSourceCancelManifestSynchronize(
    NB_EnhancedContentDataSource* instance         /*!< Enhanced content data source instance */
    );

/*! Check if metadata synchronization is currently in progress

    @return TRUE if metadata synchronization is currently in progress, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(
    NB_EnhancedContentDataSource* instance          /*!< Enhanced content data source instance */
    );

/*! Check if manifest synchronization is currently in progress

    @return TRUE if manifest synchronization is currently in progress, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(
    NB_EnhancedContentDataSource* instance          /*!< Enhanced content data source instance */
    );

/*! Get the list of all the cities received in the metadata reply

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetAvailableCities(
    NB_EnhancedContentDataSource* instance,          /*!< Enhanced content data source instance */
    CSL_Vector* vectorCities                         /*!< On return vector of NB_EnhancedContentCityData */
    );

/*! Get the list of all the cities selected by the user

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetSelectedCities(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    CSL_Vector* vectorCities                        /*!< On return vector of NB_EnhancedContentCityData */
    );

/*! Set the city selected

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceSelectCity(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* cityId,                             /*!< City identifier */
    nb_boolean selected                             /*!< Is city selected for download? */
    );

/*! Remove the city

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceRemoveCity(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* cityId                              /*!< City identifier */
    );

/*! Get the city information for the given data identifier

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetCityInformation(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* cityId,                             /*!< City identifier */
    NB_EnhancedContentCityData* city                /*!< Buffer for city info */
    );

/*! Get the list of all the entries received in the metadata reply

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetAvailableEntries(
    NB_EnhancedContentDataSource* instance,          /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType dataType,             /*!< data type */
    CSL_Vector* vectorEntries                        /*!< On return vector of Enetries */
    );

/*! Get the list of all the entries selected by the user

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetSelectedEntries(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    CSL_Vector* vectorEntries,                      /*!< On return vector of NB_ContentEntryData */
    NB_EnhancedContentDataType dataType             /*!< data type */
    );

/*! Get the list of all the existing entries on the disk

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetExistMapRegionFileIds(
    NB_EnhancedContentDataSource* instance,          /*!< Enhanced content data source instance */
    int32* count,                                    /*!< on return the count of file ids */
    char*** fileIds                                  /*!< on return the pointer to file ids */
    );

/*! Set the Entry selected

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceSelectEntry(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* id,                                 /*!< identifier */
    nb_boolean selected,                            /*!< Is city selected for download? */
    NB_EnhancedContentDataType dataType             /*!< data type */
    );

/*! Remove the entry

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceRemoveEntry(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* id,                                 /*!< identifier */
    NB_EnhancedContentDataType dataType             /*!< data type */
    );

/*! Get the region information for the given data identifier

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetRegionInformation(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* regionId,                           /*!< region identifier */
    NB_ContentEntryData* region                     /*!< Buffer for region info */
    );

/* Check if city data set is selected

    @return TRUE if city data set is selected, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsDatasetSelected(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,                /*!< Data type of data to check */
    const char* datasetId                           /*!< Dataset to check */
    );

/*! Get the size required for the given data item

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetDatasetSize(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,                /*!< Data type of data to get size of */
    const char* datasetId,                          /*!< Dataset to get size of */
    uint32* datasetSize                             /*!< Buffer for dataset size */
    );

/*! Build the url for a given data item identifier

    The returned URL does not contain the host name. The host name is retrieved using
    NB_EnhancedContentDataSourceGetHostname().

    @return NB_OK if success. NE_NOENT if couldn't find dataset. NE_RANGE if maxUrlSize isn't enough.
    @see NB_EnhancedContentDataSourceGetHostname
*/
NB_Error NB_EnhancedContentDataSourceBuildItemUrl(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId,     /*!< Data item to build url for */
    uint32 maxUrlSize,                              /*!< Maximum size of constructed url */
    char* url                                       /*!< Buffer to receive constructed url */
    );

/*! Return the host name for all enhanced content HTTP requests.

    @return NB_OK if success
    @see NB_EnhancedContentDataSourceBuildItemUrl
*/
NB_Error NB_EnhancedContentDataSourceGetHostname(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    uint32 maxHostnameSize,                         /*!< Maximum size of host name */
    char* hostname,                                 /*!< Buffer to receive host name */
    uint16* port                                    /*!< On return contains HTTP port */
    );
    
/*! Build the file system path for a given data item identifier

    The returned path does not include the base path for the data type only the sub-path for the
    specified item.

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceBuildItemFilePath(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId,     /*!< Data item to build file path for */
    nb_boolean zipped,                              /*!< If TRUE build zipped file path or unzipped folder path
                                                         Only available for textures */
    uint32 maxFilePathSize,                         /*!< Maximum size for constructed file path */
    char* filePath                                  /*!< Buffer to receive constructed file path */
    );

/*! Get item/file id from file path

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetItemIdFromFilePath(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* filePath,                           /*!< File path contained item id */
    char** itemId                                   /*!< On return item id. Has to be freed using nsl_free() when no longer used. */
    );

/*! Get the bounding box for one city

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetCityBoundingBox(
    NB_EnhancedContentDataSource* instance,         /*!< Enhanced content data source instance */
    const char* cityId,                             /*!< City to get bounding box for */
    NB_CityBoundingBox* boundingBox                 /*!< Buffer for bounding box */
    );

/*! Get projection parameters for a city

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetCityProjectionParameters(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    const char* cityId,                         /*!< City to get projection parameters for */
    NB_EnhancedContentProjectionType* projectionType,   /*!< Projection type */
    NB_EnhancedContentProjectionDatum* datum,   /*!< Projection datum */
    NB_LatitudeLongitude* origin,               /*!< Buffer for city origin */
    double* scaleFactor,                        /*!< Buffer for city scale factor */
    double* falseNorthing,                      /*!< Buffer for city false northing */
    double* falseEasting,                       /*!< Buffer for city false easting */
    double* zOffset                             /*!< Buffer for city z-offset */
    );

/*! Get the current enhanced data storage statistics

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetStorageStatistics(
    NB_EnhancedContentDataSource* instance,  /*!< Enhanced content data source instance */
    uint64* totalSize,                       /*!< Buffer for total bytes needed for current city selection, including
                                                  bytes for ECM tiles and all textures (city specific, common, and MJO) */
    uint64* downloadedSize,                  /*!< Buffer for total bytes currently downloaded for city selection */
    uint32* totalCommonTexturesSize,         /*!< Buffer for total bytes needed for common and MJO textures. */
    uint32* downloadedCommonTexturesSize,    /*!< Buffer for total bytes currently downloaded for common and MJO textures. */
    uint32* downloadedPercentage             /*!< Buffer for percentage of content that has been downloaded. */
    );

/*! Get the next data set to update based on the current location

    @return NE_OK for success, NE_NOENT if no dataset need to update.
*/
NB_Error NB_EnhancedContentDataSourceGetNextDatasetToUpdate(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    nb_boolean cityEnabled,                             /*!< Flag to indicate to enable city */
    NB_EnhancedContentIdentifier* nextDataset           /*!< Buffer for next data set to update */
    );

/*! Get the next city to update based on the current location

    @return NE_OK for success, NE_NOENT if no city need to update.
*/
NB_Error NB_EnhancedContentDataSourceGetNextCityToUpdate(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    NB_EnhancedContentIdentifier* nextCity              /*!< Buffer for next city to update */
    );

/*! Get the next entry to update

    @return NE_OK for success, NE_NOENT if no region need to update.
*/
NB_Error NB_EnhancedContentDataSourceGetNextEntryToUpdate(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    NB_EnhancedContentIdentifier* nextEntry,            /*!< Buffer for next entry to update */
    NB_EnhancedContentDataType dataType                 /*!< Data type of data to get */
    );

/*! Get the data item size

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetDataItemSize(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId,         /*!< Data item to get size of */
    uint32* dataSize                                    /*!< Buffer for data item size */
    );

/*! Get the data item size and downloaded size

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetDataItemSizeInfo(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId,         /*!< Data item to get size of */
    uint32* dataSize,                                   /*!< data item size */
    uint32* downloadedSize                              /*!< downloaded size */
    );

/*! Update the data download status

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId,         /*!< Data item to update downloaded size of */
    NB_EnhancedContentUpdateStatus updateStatus,        /*!< Update status */
    NB_EnhancedContentDownloadStatus downloadStatus,    /*!< Downloaded status */
    uint32 downloadedSize                               /*!< Downloaded size */
    );

/*! Gets a list of all the data items from a given dataset

Returns vector of data items for the requested dataset. Returned data items are created by NB_EnhancedContentIdentifierCreate
and should be freed with NB_EnhancedContentIdentifierDestroy one by one when no longer needed.

    @return NE_OK for success, NE_NOENT if no items exist (or all are downloaded already).
*/
NB_Error NB_EnhancedContentDataSourceGetDataItemIDs(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    nb_boolean nonDownloadedItemsOnly,                  /*!< Flag to indicate to return only items which are not downloaded yet or items which
                                                             have an outdated version. If this flag is FALSE then all items are returned for the
                                                             data set */
    NB_EnhancedContentDataType type,                    /*!< Data type of data to retrieve */
    const char* dataset,                                /*!< Pointer to dataset string of data to retrieve */
    CSL_Vector* vectorDataIds                           /*!< On return array of Data items. Has to be freed using nsl_free() when no longer used. */
    );

/*! Checks if manifest needs to be updated or not

    @return TRUE if manifest needs to be updated, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsManifestUpdateNeeded(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId          /*!< Data id to check */
    );

/*! Checks if given item from storage is consistent with metadata/manifest

    @return TRUE if item is consistent with metadata/manifest, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsDataItemConsistent(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId          /*!< Data item to check consistency of */
    );

/*! Checks if the given item is already downloaded or not based on the manifest status

    @return TRUE if the given item is already downloaded, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsDataItemAvailableForUse(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId          /*!< Data item to check */
    );

/*! Checks if the given item is enabled for download

    @return TRUE if data item is enabled for download, FALSE otherwise
*/
nb_boolean NB_EnhancedContentDataSourceIsDataItemEnabledForDownload(
    NB_EnhancedContentDataSource* instance,             /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId          /*!< Data item to check */
    );

/*! Get the tile bounding boxes for one city

Returns copy of array of bounding boxes for the requested city.  Returned array is allocated for caller and
should be freed with nsl_free() when no longer needed.

    @return NB_OK if success
*/
NB_Error NB_EnhancedContentDataSourceGetCityTileBoundingBoxes(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    const char* cityId,                         /*!< City to get tile bounding boxes for */
    uint32* cityBoundingBoxesCount,             /*!< Buffer for count of bounding boxes */
    NB_CityBoundingBox** cityBoundingBoxes      /*!< Buffer for pointer to array of city bounding boxes */
    );

/*! Get manifest version of the dataset.

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceGetManifestVersion(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataId, /*!< Data item to get manifest version */
    uint32 maxManifestVersionSize,              /*!< Maximum size of manifest version */
    char* manifestVersion                       /*!< Manifest version */
    );

/*! Clears the persistent store contents

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceMasterClear(
    NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
    );

/*! Get the dataset id for common textures

    @return current dataset id used for  motorway junction objects
*/
const char* NB_EnhancedContentDataSourceGetCommonTexturesDatasetId(
    NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
    );

/*! Get the dataset id for region common

    @return current dataset id used for  region common objects
*/
const char* NB_EnhancedContentDataSourceGetRegionCommonDatasetId(
    NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
    );

/*! Get the dataset id for motorway junction objects

    @return current dataset id used for  motorway junction objects
*/
const char* NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(
    NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
    );

/*! Get the dataset id for realistic signs

    @return current dataset id used for realistic signs
*/
const char* NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(
    NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
    );

/*! Get the dataset id for speed limits sign.

    @return current dataset id used for speed limits sign.
*/
const char*
NB_EnhancedContentDataSourceGetSpeedLimitsSignDatasetId(NB_EnhancedContentDataSource* instance      /*!< Enhanced content data source instance */
                                                        );

/*! Get the dataset id for special regions.

    @return current dataset id used for special region.
*/
const char*
NB_EnhancedContentDataSourceGetSpecialRegionsDatasetId(NB_EnhancedContentDataSource* instance       /*!< Enhanced content data source instance. */
                                                       );

/*! Get the city summary display

    @return NE_OK for success
    @see NB_MetadataInformationGetCitySummaryDisplay
*/
NB_Error NB_EnhancedContentDataSourceGetCitySummaryDisplay(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    const char** citySummaryDisplay             /*!< On success, returns pointer to the internal string */
    );

/*! Set current location

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceSetLocation(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    NB_LatitudeLongitude* location              /*!< Current location */
    );

/*! Remove data item from manifest table

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceRemoveDataItem(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,            /*!< Data type of item to remove */
    const char* datasetId,                      /*!< Dataset of item to remove */
    const char* itemId                          /*!< Data item to remove */
    );

/*! Update manifest version

Server returns the latest manifest version for ECM along the route.

    @return NB_OK if updated successful
        NE_EXIST if the manifest version is already latest
        NE_NOENT if the synchronization of metadata and manifest isn't complete
*/
NB_Error NB_EnhancedContentDataSourceUpdateManifestVersion(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,            /*!< Data type to update manifest version of */
    const char* datasetId,                      /*!< Dataset to update manifest version of */
    const char* manifestVersion                 /*!< Updated manifest version */
    );

/*! Check file version

Server returns the latest file version for MJO and SAR along the route.

    @return NE_OK if the file version is latest
        NE_NOENT there is no entry of the file or the file version isn't latest
*/
NB_Error NB_EnhancedContentDataSourceCheckFileVersion(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,            /*!< Data type of item to check file version of */
    const char* datasetId,                      /*!< Dataset of item to check file version of */
    const char* itemId,                         /*!< Data item to check file version of */
    const char* fileVersion                     /*!< Checked file version */
    );

/*! Add a new manifest entry

    @return NE_OK if added successful
            NE_EXIST if the entry is already latest
*/
NB_Error NB_EnhancedContentDataSourceAddManifestEntry(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    NB_EnhancedContentDataType type,            /*!< Data type of item to update file version of */
    const char* datasetId,                      /*!< Dataset of item to update file version of */
    const char* itemId,                         /*!< Data item to update file version of */
    const char* fileVersion                     /*!< Updated file version */
    );

/*! Calculate the download percentage of region.

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceCalculateRigonDownloadPercentage(
    NB_EnhancedContentDataSource* instance,     /*!< Enhanced content data source instance */
    uint32 percent,                             /*!< download percent of current download file */
    const NB_EnhancedContentIdentifier* item,   /*!< data item */
    uint32* percentOut                          /*!< download percentage of current region */
    );

/*! Update the data download status

    @return NE_OK for success
*/
NB_Error NB_EnhancedContentDataSourceUpdateDataSetDownloadStatus(
    NB_EnhancedContentDataSource* instance,                /*!< Enhanced content data source instance */
    const NB_EnhancedContentIdentifier* dataItem,          /*!< Data item to update downloaded size of */
    NB_EnhancedContentDownloadStatus downloadStatus        /*!< Downloaded status */
    );

/*! @} */

#endif
