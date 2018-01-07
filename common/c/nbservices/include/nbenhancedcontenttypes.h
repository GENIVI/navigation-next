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

    @file     nbenchancedcontenttypes.h
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

#ifndef NBENHANCEDCONTENTTYPES_H
#define NBENHANCEDCONTENTTYPES_H

/*!
 @addtogroup nbenhancedcontenttypes
 @{
 */

#include "navpublictypes.h"

// Public Constants ..............................................................................

#define MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH     16
#define MAX_ENHANCED_CONTENT_PATH_LENGTH            256
#define MAX_ENHANCED_CONTENT_FILE_TYPE_LENGTH       8

#define MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH    16
#define MAX_ENHANCED_CONTENT_CITY_DISPLAY_LENGTH    80

#define MAX_ENHANCED_CONTENT_BOX_NAME_LENGTH        256

#define MAX_CLIENT_GUID_LENGTH                      40

// Public Types ..................................................................................

/*! Wanted content type */
typedef enum
{
    NB_ECDT_None = 0,

    NB_ECDT_CityModel               = 1,        /*!< City model data */
    NB_ECDT_MotorwayJunctionObject  = 1 << 1,   /*!< Motorway junction data */
    NB_ECDT_CommonTexture           = 1 << 2,   /*!< Common texture shared across cities */
    NB_ECDT_SpecificTexture         = 1 << 3,   /*!< City specific texture */
    NB_ECDT_RealisticSign           = 1 << 4,   /*!< Realistic sign */
    NB_ECDT_RasterMaps              = 1 << 5,   /*!< All raster map layers */
    NB_ECDT_UnifiedBinaryModel      = 1 << 6,   /*!< All nified map
                                                     types/layers other than
                                                     the raster map layers */
    NB_ECDT_SpeedLimitsSign         = 1 << 7,   /*!< Speed signs */
    NB_ECDT_SpecialRegions          = 1 << 8,   /*!< Special regions.*/
    NB_ECDT_MapRegionsCommon        = 1 << 9,   /*!< common files shared across map regions.*/
    NB_ECDT_MapRegions              = 1 << 10,  /*!< Map regions.*/
    NB_ECDT_All =                               /*!< All data types */
    NB_ECDT_CityModel | NB_ECDT_MotorwayJunctionObject | \
    NB_ECDT_CommonTexture | NB_ECDT_SpecificTexture |    \
    NB_ECDT_RealisticSign | NB_ECDT_RasterMaps |         \
    NB_ECDT_UnifiedBinaryModel | NB_ECDT_SpeedLimitsSign | \
    NB_ECDT_SpecialRegions | NB_ECDT_MapRegionsCommon |    \
    NB_ECDT_MapRegions
} NB_EnhancedContentDataType;

/*! Map types the client can handle. */
typedef enum _NB_SupportedMapType
{
    NB_ECDT_MAP_NONE     = 0,
    NB_ECDT_MAP_EXTENDED = 1,       /*!< If present, the client can handle extended map tile
                                         congurations in the reply.
                                         Note: This element is required if the client is Vx or
                                         newer. */
    NB_ECDT_MAP_SHARED   = 1 << 1,  /*!< If present, the client can handle extended maps
                                         congurations that include want-extended-maps
                                         functionality as well as the new tile-store-template in
                                         the url-args-template in-cluded with the reply.
                                         Note: Not required if want-unied-maps is specied (i.e.
                                         want-unied-maps is a superset of the want-shared-maps
                                         functionality). */
    NB_ECDT_MAP_UNIFIED  = 1 << 2,  /*!< If present, the client can handle the unied map tile
                                         congurations in the reply.
                                         Note: This element is required if the client is Vx3 or
                                         newer.). */
    NB_ECDT_MAP_ALL      =          /*!< All types. */
    NB_ECDT_MAP_EXTENDED | NB_ECDT_MAP_SHARED | NB_ECDT_MAP_UNIFIED,
} NB_SupportedMapType;

/*! DownloadLevls used by enhanced content download manager.*/
typedef enum _NB_EnhancedContentDownloadLevel
{
    NB_ECDL_Heigh = 0,                  /*!< Highest level, for speed limit signs.*/
    NB_ECDL_Middle,                     /*!< Middle level, for SARs. */
    NB_ECDL_Low,                        /*!< Lowest level, for all the others. */
    NB_ECDL_All,
} NB_EnhancedContentDownloadLevel;

/*! Allowed download setting */
typedef enum
{
    NB_ECDO_None = 0,                       /*!< Allow no download requests to be processed */

    NB_ECD0_OnDemand = 1,                   /*!< On-demand downloads */
    NB_ECD0_Preload = 1 << 1,               /*!< Pre-load downloads */

    NB_ECDO_All =                           /*!< All downloads */
    NB_ECD0_OnDemand | NB_ECD0_Preload

} NB_EnhancedContentDownloadType;

/*! City manifest state */
typedef enum
{
    NB_ECMS_ManifestNotSynchronized = 0,    /*!< Manifest of dataset hasn't be synchronized */
    NB_ECMS_ManifestLatest                  /*!< Manifest of dataset is latest */
} NB_EnhancedContentManifestState;

/*! City update status */
typedef enum
{
    NB_ECUS_UpdateStatusNew = 0,            /*!< New city */
    NB_ECUS_UpdateStatusUsed                /*!< City was used/downloaded */
} NB_EnhancedContentUpdateStatus;

/*! City download status */
typedef enum
{
    NB_ECDS_DownloadNotStarted = 0,         /*!< Download has not started */
    NB_ECDS_DownloadInProgress,             /*!< Download is in progress */
    NB_ECDS_DownloadPaused,                 /*!< Download is paused */
    NB_ECDS_DownloadComplete                /*!< Download has completed */
} NB_EnhancedContentDownloadStatus;

/*! Enhanced Content Projection Type */
typedef enum {
    NB_ECPT_Invalid = 0,
    NB_ECPT_Mercator,                       /*!< Mercator */
    NB_ECPT_TransverseMercator,             /*!< Transverse Mercator */
    NB_ECPT_UniversalTransverseMercator     /*!< Universal Transverse Mercator */

} NB_EnhancedContentProjectionType;

/*! Enhanced Content Projection Datum */
typedef enum
{
    NB_ECPD_Invalid = 0,
    NB_ECPD_WGS84,                          /*!< WGS84 */
    NB_ECPD_DNorthAmerican1983,             /*!< D North American 1983 */

} NB_EnhancedContentProjectionDatum;

/*! City bounding box  */
typedef struct
{
    char name[MAX_ENHANCED_CONTENT_BOX_NAME_LENGTH + 1];            /*!< Tile bounding box name */
    NB_LatitudeLongitude point1;                                    /*!< One corner point */
    NB_LatitudeLongitude point2;                                    /*!< The other corner point */

} NB_CityBoundingBox;

/*! Enhanced content city data */   // TODO: some of this data can be protected
typedef struct
{
    char cityId[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];           /*!< City id/dataset name */
    char version[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];          /*!< Current city version */
    char textureVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];   /*!< Current texture version */
    char displayName[MAX_ENHANCED_CONTENT_CITY_DISPLAY_LENGTH + 1];      /*!< Display name */

    nb_boolean selected;                                            /*!< Is city selected? */
    nb_boolean deleted;                                             /*!< Is city deleted? */

    NB_EnhancedContentUpdateStatus updateStatus;                    /*!< City update status */

    uint32 cityModelDataSize;                                       /*!< City model total size */
    uint32 downloadedCityModelDataSize;                             /*!< City model downloaded size */
    uint32 failedCityModelDataSize;                                 /*!< City model failed size */

    uint32 textureDataSize;                                         /*!< Texture total size */
    uint32 downloadedTextureDataSize;                               /*!< Texture downloaded size */
    uint32 failedTextureDataSize;                                   /*!< Texture failed size */

    NB_CityBoundingBox box;                                         /*!< Bounding box for entire city */
    NB_LatitudeLongitude center;                                    /*!< Center point for city */

} NB_EnhancedContentCityData;

typedef NB_EnhancedContentCityData NB_ContentEntryData;


/*! @} */

#endif
