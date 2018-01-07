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

    @file nbenhancedvectormaptypes.h
    @date 02/21/2012

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NB_ENHANCED_VECTOR_MAP_TYPES_H
#define NB_ENHANCED_VECTOR_MAP_TYPES_H

#include "palexp.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbenhancedcontenttypes.h"

#define MAX_ENHANCED_VECTOR_MAP_PATH_LENGTH         255
#define MAX_ENHANCED_VECTOR_MAP_STRING_LENGTH       127
#define ENHANCED_VECTOR_MAP_PREFETCH_LEVEL_COUNT    3

typedef struct NB_TileKey
{
    uint32 x;
    uint32 y;
    uint32 zoomLevel;
} NB_TileKey;

typedef enum
{
    NB_TileLayer_Areas = 0x1,               /*!< Detailed vector areas ("DVA") */
    NB_TileLayer_Roads = 0x2,               /*!< Detailed vector roads ("DVR") */
    NB_TileLayer_UntexBuilding = 0x4,       /*!< Untextured 3D buildings ("B3D") */
    NB_TileLayer_Landmarks = 0x8            /*!< 3D landmarks ("LM3D") */
} NB_TileLayerType;

typedef struct
{
    NB_TileKey tileKey;
    NB_TileLayerType layerType;
} NB_TileId;

typedef struct
{
    NB_TileId tileId;
    uint8* tileData;
    uint32 dataSize;
} NB_LoadTileCallbackData;

typedef struct
{
    NB_TileKey tileKey;
} NB_UnloadTileCallbackData;

typedef struct NB_EnhancedVectorMapManagerConfig
{
    uint32                          NBGMVersion;
    uint32                          screenWidth;        /*!< The screen width of the device, specified in pixels.*/
    uint32                          screenHeight;
    uint32                          maxCachingTileCount;
    uint32                          maximumTileRequestCountPerLayer;
    char                            language[MAX_ENHANCED_VECTOR_MAP_STRING_LENGTH + 1];
    char                            vectorTileDataPath[MAX_ENHANCED_VECTOR_MAP_PATH_LENGTH + 1];
    char                            clientGuid[MAX_CLIENT_GUID_LENGTH + 1];
} NB_EnhancedVectorMapManagerConfig;

typedef struct NB_EnhancedVectorMapManager NB_EnhancedVectorMapManager;

/*! Callback function for metadata request result

    This callback reports the results of a call to NB_EnhancedVectorMapManagerRequestMetadata().
    Check result to find out any error occured.
    If finished, the percent should be 100.

    @return None
    @see NB_EnhancedVectorMapManagerRequestMetadata
*/
typedef void (*NB_EnhancedVectorMapMetadataRequestCallbackFunction)(NB_EnhancedVectorMapManager* manager,               /*!< Enhanced vector map manager instance */
                                                                    NB_Error result,                                    /*!< Result of download */
                                                                    uint32 percent,                                     /*!< Progress in percent. Value from 0 - 100 */
                                                                    void* userData                                      /*!< User data specified in NB_EnhancedVectorMapManagerRequestMetadata() */
                                                                    );

/*! Structure for enhanced vector map metadata request callback data */
typedef struct
{
    NB_EnhancedVectorMapMetadataRequestCallbackFunction     callback;                   /*!< Callback function */
	void*                                                   callbackData;               /*!< Callback data */
} EnhancedVectorMapMetadataRequestCallback;

/*! Callback function for common material request result

    This callback reports the results of a call to NB_EnhancedVectorMapManagerRequestMetadata().
    Check result to find out any error occured.
    If finished, the percent should be 100.

    @return None
    @see NB_EnhancedVectorMapManagerRequestMetadata
*/
typedef void (*NB_EnhancedVectorMapCommonMaterialRequestCallbackFunction)(NB_EnhancedVectorMapManager* manager,             /*!< Enhanced vector map manager instance */
                                                                          NB_Error result,                                  /*!< Result of download */
                                                                          uint32 percent,                                   /*!< Progress in percent. Value from 0 - 100 */
                                                                          char* filename,                                   /*!< file name in local storage */
                                                                          void* userData                                    /*!< User data specified in NB_EnhancedVectorMapManagerRequestMetadata() */
                                                                          );

/*! Structure for enhanced vector map metadata request callback data */
typedef struct
{
    NB_EnhancedVectorMapCommonMaterialRequestCallbackFunction   callback;                           /*!< Callback function */
    void*                                                       callbackData;                       /*!< Callback data */
} EnhancedVectorMapCommonMaterialRequestCallback;

typedef struct NB_EnhancedVectorMapState NB_EnhancedVectorMapState;

/*! Callback function for loading map tile

    This callback give the tile data which should be load into NBGM context.
    NB_LoadTileCallbackData::tileData is pointing to a memory block containing tile data,
    it is allocated by nsl_malloc. User should call nsl_free to free this memory himself.

    @return None
    @see NB_LoadTileCallbackData
    @see NB_EnhancedVectorMapUnloadTileCallbackFunction
*/
typedef void (*NB_EnhancedVectorMapLoadTileCallbackFunction)(NB_EnhancedVectorMapState* state,                              /*!< Enhanced vector map state */
                                                             NB_LoadTileCallbackData callbackData,                          /*!< data for the tile */
                                                             void* userData                                                 /*!< User data specified in NB_EnhancedVectorMapConfiguration */
                                                             );

/*! Callback function for unloading map tile

    NB_UnloadTileCallbackData contains NB_TileKey only. User should unload all tiles with this tile key altogether.

    @return None
    @see NB_UnloadTileCallbackData
    @see NB_EnhancedVectorMapLoadTileCallbackFunction
*/
typedef void (*NB_EnhancedVectorMapUnloadTileCallbackFunction)(NB_EnhancedVectorMapState* state,                            /*!< Enhanced vector map state */
                                                               NB_UnloadTileCallbackData callbackData,                      /*!< data for the tile */
                                                               void* userData                                               /*!< User data specified in NB_EnhancedVectorMapConfiguration */
                                                               );

typedef struct
{
    NB_EnhancedVectorMapLoadTileCallbackFunction    loadTileCallback;
    NB_EnhancedVectorMapUnloadTileCallbackFunction  unloadTileCallback;
    void*                                           callbackData;
} NB_EnhancedVectorMapUpdateTileCallback;

typedef struct NB_EnhancedVectorMapPrefetchConfig
{
    float       prefetchWindow[ENHANCED_VECTOR_MAP_PREFETCH_LEVEL_COUNT];       /*!< meters along the route to be prefetched at one time */
    float       prefetchMinWindow[ENHANCED_VECTOR_MAP_PREFETCH_LEVEL_COUNT];    /*!< meters from the end of last prefetch window to start a new prefetch */
    uint32      zoomLevel;                                                      /*!< tile zoom level */
} NB_EnhancedVectorMapPrefetchConfig;

typedef struct NB_EnhancedVectorMapCameraConfig
{
    uint32 cameraHeightMeters;
    uint32 cameraToAvatarHorizontalMeters;
    uint32 avatarToHorizonHorizontalMeters;
    uint32 horizontalFieldOfViewDegrees;
    uint32 avatarToMapBottomPixel;
    uint32 mapFieldWidthPixel;
    uint32 mapFieldHeightPixel;
} NB_EnhancedVectorMapCameraConfig;

typedef struct NB_EnhancedVectorMapConfiguration
{
    NB_EnhancedVectorMapPrefetchConfig      prefetchConfig;
    NB_EnhancedVectorMapCameraConfig        cameraConfig;
    NB_EnhancedVectorMapUpdateTileCallback  updateTileCallback;
    nb_boolean                              exclude3DTilesInECM;                /*!< true if exclude untexbuilding and landmark tiles in ECM */
} NB_EnhancedVectorMapConfiguration;

typedef struct NB_EnhancedCityModelBoundingBoxParameters NB_EnhancedCityModelBoundingBoxParameters;

#endif

/*! @} */
