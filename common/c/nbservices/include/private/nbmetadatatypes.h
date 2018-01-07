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

    @file     nbmetadatatypes.h
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

#ifndef NBMETADATATYPES_H
#define NBMETADATATYPES_H

#include "nbenhancedcontenttypes.h"

/*! @{ */

// Public constants ..........................................................

#define DATASET_TYPE_CITY_MODEL                         "city models"
#define DATASET_TYPE_MOTORWAY_JUNCTION_OBJECT           "junctions"
#define DATASET_TYPE_COMMON_TEXTURE                     "textures-city-models-common"
#define DATASET_TYPE_SPECIFIC_TEXTURE                   "textures"
#define DATASET_TYPE_REALISTIC_SIGN                     "realistic signs"
#define DATASET_TYPE_RASTER_MAPS                        "raster maps"
#define DATASET_TYPE_UNIFIED_BINARY_MODEL               "unified binary model"
#define DATASET_TYPE_SPEED_SIGN                         "speed signs"
#define DATASET_TYPE_SPECIAL_REGION                     "special regions"
#define DATASET_TYPE_MAP_REGIONS_COMMON                 "map-regions-common"
#define DATASET_TYPE_MAP_REGIONS                        "map regions"

#define PROJECTION_TYPE_MERCATOR                        "mercator"
#define PROJECTION_TYPE_TRANSVERSE_MERCATOR             "transverse-mercator"
#define PROJECTION_TYPE_UNIVERSAL_TRANSVERSE_MERCATOR   "universal-transverse-mercator"

#define PROJECTION_DATUM_WGS84                          "WGS84"
#define PROJECTION_DATUM_D_NORTH_AMERICAN_1983          "D_North_American_1983"


// Public types ..............................................................

/*! Metadata source reply for persistence in database */
typedef struct
{
    int64 id;                                   /*!< Database-generated record id for referencing from Manifest */

    int contentType;                            /*!< NB_EnhancedContentDataType */
    int projectionType;                         /*!< NB_EnhancedContentProjectionType */
    int manifestState;                          /*!< NB_EnhancedContentManifestState */
    int updateStatus;                           /*!< NB_EnhancedContentUpdateStatus */
    int selected;                               /*!< Used for selected city.
                                                     This parameter is valid of content type NB_ECDT_CityModel and NB_ECDT_SpecificTexture. */
    int deleted;                                /*!< Used for force-delete.
                                                     If present, the client should delete this dataset from the device. */
    int datasetSize;                            /*!< Total size of the dataset */
    int downloadedSize;                         /*!< Downloaded size of the dataset */
    int failedSize;                             /*!< Failed size of the dataset TODO not support */

    int datum;                                  /*!< "WGS84"or "D_North_American_1983" */
    double boundingLatitude1;                   /*!< Latitude of top left */
    double boundingLongitude1;                  /*!< Longitude of top left */
    double boundingLatitude2;                   /*!< Latitude of bottom right */
    double boundingLongitude2;                  /*!< Longitude of bottom right */
    double originLatitude;                      /*!< Latitude of origin for the projection. */
    double originLongitude;                     /*!< Longitude of the origin of the projection. */
    double scaleFactor;                         /*!< Always 0.9996 for "universal-transverse-mercator". */
    double falseEasting;                        /*!< 500000.0 for "universal-transverse-mercator". */
    double falseNorthing;                       /*!< 10000000.0 for "universal-transverse-mercator" in the southern hemisphere. */
    double zOffset;                             /*!< A constant value to add to all z-coordinates in the model to obtain the
                                                     actual height off the referenced ellipsoid. */

    char datasetId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];           /*!< Data identifier */
    char manifestVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];     /*!< Version of the data */
    char urlBase[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                     /*!< Base URL for all url-args-template elements */
    char urlArgsTemplate[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];             /*!< This element specifies the URL template for
                                                                             a particular type of extended data. */
    char displayName[MAX_ENHANCED_CONTENT_CITY_DISPLAY_LENGTH + 1];         /*!< Name to be displayed to the user */
    char country[MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1];              /*!< Country code */
} NB_MetadataSourceReply;

/*! Metadata manifest reply for persistence in database */
typedef struct
{
    int64 id;                                   /*!< Database-generated record id */
    int64 metadataId;                           /*!< Index to Metadata record for dataset info */

    int downloadStatus;                         /*!< NB_EnhancedContentDownloadStatus
                                                     Only use NB_ECDS_DownloadNotStarted and NB_ECDS_DownloadComplete */
    int fileSize;                               /*!< File size in bytes */
    int downloadedSize;                         /*!< Downloaded size of the file */

    double boundingLatitude1;                   /*!< Latitude of top left */
    double boundingLongitude1;                  /*!< Longitude of top left */
    double boundingLatitude2;                   /*!< Latitude of bottom right */
    double boundingLongitude2;                  /*!< Longitude of bottom right */

    char fileId[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                      /*!< Data file identifier */
    char fileVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];         /*!< Current version of this file */
} NB_MetadataManifestReply;

/*! Manifest query city/map region for manifest synchronization one by one */
typedef struct
{
    int64 metadataId;                           /*!< Index to Metadata record for dataset info */

    char cityId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];              /*!< City id/map region id/dataset name */
} NB_ManifestQueryCity;

typedef NB_ManifestQueryCity NB_ManifestEntry;

/*! @} */

#endif
