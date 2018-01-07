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

    @file     nbenhancedcontentmanagerprotected.h
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

#ifndef NBENHANCEDCONTENTMANAGERPROTECTED_H
#define NBENHANCEDCONTENTMANAGERPROTECTED_H

/*!
    @addtogroup nbenhancedcontentmanagerprotected
    @{
*/

#include "nbenhancedcontentmanager.h"

// Protected Constants ..............................................................................


// Protected Types ..................................................................................


// Protected Functions ..............................................................................

/*! Check if data item is available

Verifies that the specified data item has been downloaded (according to the persisted manifest).
Also verifies the file exists on the device storage as well.

@return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsDataAvailable(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to verify available */
    const char* dataset,                        /*!< Dataset of item to verify available */
    const char* dataId                          /*!< Data item to verify available */
    );

/*! Check if data item, and all the data items it is dependent on, are available

Verifies that the specified data item, and data items it is dependent on (such as textures),
have been downloaded (according to the persisted manifest).  Also verifies the file exists
on the device storage as well (except for enhanced city model tiles).

@return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsDataAvailableWithDependencies(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance  */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to verify available */
    const char* dataset,                        /*!< Dataset of item to verify available */
    const char* dataId                          /*!< Data item to verify available */
    );

/*! Check if all textures for a city are available

@return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean NB_EnhancedContentManagerIsCityTextureAvailable(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId                          /*!< City to check textures for */
    );

/*! Get the bounding box for one city

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCityBoundingBox(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId,                         /*!< City to get bounding box for */
    NB_CityBoundingBox* boundingBox             /*!< Buffer for city bounding box structure */
    );

/*! Get the tile bounding boxes for one city

Returns copy of array of bounding boxes for the requested city.  Returned array is allocated for caller and
should be freed with nsl_free() when no longer needed.

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCityTileBoundingBoxes(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId,                         /*!< City to get tile bounding boxes for */
    uint32* cityBoundingBoxesCount,             /*!< Buffer for count of bounding boxes */
    NB_CityBoundingBox** cityBoundingBoxes      /*!< Buffer for pointer to array of city bounding boxes */
    );

/*! Get projection parameters for a city

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetCityProjectionParameters(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId,                         /*!< City to get projection parameters for */
    NB_EnhancedContentProjectionType* projectionType,   /*!< Projection type */
    NB_EnhancedContentProjectionDatum* datum,   /*!< Projection datum */
    NB_LatitudeLongitude* origin,               /*!< Buffer for city origin */
    double* scaleFactor,                        /*!< Buffer for city scale factor */
    double* falseNorthing,                      /*!< Buffer for city false northing */
    double* falseEasting,                       /*!< Buffer for city false easting */
    double* zOffset                             /*!< Pointer to city z-offset */
    );

/*! Get downloaded data from storage for specified data item

Returns copy of requested data item data bytes.  Returned buffer is allocated for caller and
should be freed with nsl_free() when no longer needed.

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetDataItem(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to get */
    const char* dataset,                        /*!< Dataset of item to get */
    const char* dataId,                         /*!< Data item to get */
    const char* fileVersion,                    /*!< File version for MJOs and SARs. Only used when arguement allowDownload is TRUE */
    nb_boolean allowDownload,                   /*!< Indicates if data item should be queued for download if not present */
    uint32* dataSize,                           /*!< Buffer for copied data size */
    uint8** data                                /*!< Buffer for pointer to copy of requested data */
    );

/*! Get full file device path for data item

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetDataItemPath(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to get path of */
    const char* dataset,                        /*!< Dataset of item to get path of */
    const char* dataId,                         /*!< Data item to get path of */
    nb_boolean zipped,                          /*!< If TRUE build zipped file path or unzipped folder path
                                                     Only available for textures */
    uint32 dataItemPathSize,                    /*!< Size of provided buffer for path */
    char* dataItemPath                          /*!< Buffer to store full data item path at */
    );

/*! Request download for the data item.

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerRequestDataDownload(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to request */
    const char* dataset,                        /*!< Dataset of item to request */
    const char* dataId,                         /*!< Data item to request */
    const char* fileVersion                     /*!< File version. Only used for MJOs and SARs */
    );

/*! Clears all pending on demand data requests

@return NE_OK if success
*/
NB_DEC NB_Error NB_EnhancedContentManagerClearOnDemandRequests(
    NB_EnhancedContentManager* manager,          /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType          /*!< Data type to cancel */
    );

/*! Check manifest version

Server returns the latest manifest version for ECM along the route.

@return NE_OK if the manifest version is latest or return NE_NOENT
*/
NB_DEC NB_Error NB_EnhancedContentManagerCheckManifestVersion(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type to check manifest version of */
    const char* dataset,                        /*!< Dataset to check manifest version of */
    const char* manifestVersion                 /*!< Checked manifest version */
    );

/*! Check file version

Server returns the latest file version for MJO and SAR along the route.

@return NE_OK if the file version is latest
        NE_NOENT there is no entry of the file or the file version isn't latest
*/
NB_DEC NB_Error NB_EnhancedContentManagerCheckFileVersion(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,        /*!< Data type of item to check file version of */
    const char* dataset,                        /*!< Dataset of item to check file version of */
    const char* dataId,                         /*!< Data item to check file version of */
    const char* fileVersion                     /*!< Checked file version */
    );

/*! Get the dataset id for motorway junction objects

@return current dataset id used for  motorway junction objects
*/
NB_DEC const char* NB_EnhancedContentManagerGetMotorwayJunctionObjectsDatasetId(
    NB_EnhancedContentManager* manager           /*!< Enhanced content manager instance */
    );

/*! Get the dataset id for realistic signs

@return current dataset id used for realistic signs
*/
NB_DEC const char* NB_EnhancedContentManagerGetRealisticSignsDatasetId(
    NB_EnhancedContentManager* manager           /*!< Enhanced content manager instance */
    );

/*! Get the dataset id for speed limits signs.

    @return current dataset id used for speed limits signs
*/
NB_DEC const char*
NB_EnhancedContentManagerGetSpeedLimitsSignDatasetId(NB_EnhancedContentManager* manager /*!< Enhanced content manager instance */
                                                    );

/*! Get the data item size

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerGetDataItemSize(
    NB_EnhancedContentManager* manager,          /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,         /*!< Data type to get size of */
    const char* dataset,                         /*!< Dataset to get size of */
    const char* dataId,                          /*!< Data item to get size of */
    uint32* dataSize                             /*!< Buffer for data item size */
    );

/*! Update the data download status

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentManagerUpdateDataItemDownloadStatus(
    NB_EnhancedContentManager* manager,          /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,         /*!< Data type to update version of */
    const char* dataset,                         /*!< Dataset to update version of */
    const char* dataId,                          /*!< Data item to update version of */
    NB_EnhancedContentUpdateStatus updateStatus,        /*!< Update status */
    NB_EnhancedContentDownloadStatus downloadStatus,    /*!< Downloaded status */
    uint32 downloadedSize                        /*!< Downloaded size */
    );

/*! Remove persistent data from device storage

    If dataId is NULL remove the entire dataset.

    @return  NB_Error NE_NOENT if there is no such data
*/
NB_DEC NB_Error
NB_EnhancedContentManagerRemoveData(
    NB_EnhancedContentManager* manager,          /*!< Enhanced content manager instance */
    NB_EnhancedContentDataType dataType,         /*!< Data type to remove */
    const char* dataset,                         /*!< Dataset to remove */
    const char* dataId,                          /*!< Data item to remove */
    const char* fileVersion                      /*!< File version of file, set to NULL not cared */
    );

/*! Download next dataset

If result is NE_NOENT download complete.

@return NE_OK if success
*/
NB_DEC NB_Error
NB_EnhancedContentManagerDownloadNextDataset(
    NB_EnhancedContentManager* manager           /*!< Enhanced content manager instance */
     );

/*! Check if city is available

Verifies that the city related data has been downloaded (common textures, city textures and city model).

@return TRUE if available, FALSE otherwise
*/
NB_DEC nb_boolean
NB_EnhancedContentManagerIsCityAvailable(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId                          /*!< City identifier */
    );

/*! Check if city is selected

@return TRUE if selected, FALSE otherwise
*/
NB_DEC nb_boolean
NB_EnhancedContentManagerIsCitySelected(
    NB_EnhancedContentManager* manager,         /*!< Enhanced content manager instance */
    const char* cityId                          /*!< City identifier */
    );

/*! Get allowable download types.

    @return allowable download type.

    @see NB_EnhancedContentManagerSetAllowableDownload
*/

NB_DEC NB_EnhancedContentDownloadType
NB_EnhancedContentManagerGetAllowableDownload(NB_EnhancedContentManager* manager);


/*! @} */

#endif
