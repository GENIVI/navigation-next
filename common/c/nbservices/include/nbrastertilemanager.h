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

    @file     nbrastertilemanager.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBRASTERTILEMANAGER_H
#define NBRASTERTILEMANAGER_H

#include "paltypes.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbrastertiledatasourcepublic.h"

/*!
    @addtogroup nbrastertilemanager
    @{
*/


// Public Types ..................................................................................

#define NB_RTM_MAX_CLIENT_GUID_LENGTH   40

typedef struct NB_RasterTileManager NB_RasterTileManager;
typedef struct NB_RasterTileCache NB_RasterTileCache;


/*! Raster tile command used for requesting tiles.

    @see NB_RasterTileManagerGetTile
*/
typedef enum
{
    NB_RTC_None = 0,

    NB_RTC_MemoryCache,         /*!< Return tile if it is in the memory cache. Don't download the tile. */
    NB_RTC_PersistentCache,     /*!< Return tile if it is in the memory or persistent cache. Don't download tile. */
    NB_RTC_Download,            /*!< Return tile if it is in the cache. Start download if it is not in the cache.  */

    NB_RTC_Force_Download       /*!< Force tile download. If the tile is already in the cache then it will be removed
                                     from the cache prior to the download. */

} NB_RasterTileCommand;

/*! Type for raster tiles. */
typedef enum
{
    NB_RTT_None     = 0,

    NB_RTT_Map      = 1,
    NB_RTT_BirdsEye,
    NB_RTT_Terrain,
    NB_RTT_Satellite,
    NB_RTT_Hybrid,

    NB_RTT_Traffic  = 10,
    NB_RTT_Route    = 20,
    NB_RTT_RouteSelector1  = 30,
    NB_RTT_RouteSelector2  = 40,
    NB_RTT_RouteSelector3  = 50
} NB_RasterTileType;

/*! Image format for tiles */
typedef enum
{
    NB_RTIF_None = 0,

    NB_RTIF_PNG,
    NB_RTIF_GIF,
    NB_RTIF_JPG

} NB_RasterTileImageFormat;

/*! Options used to clear cache. */
typedef enum
{
    NB_RTCC_All,                    /*!< Clear both the persistent and memory cache */
    NB_RTCC_MemoryOnly,             /*!< Clear only the memory cache including any locked tiles. */
    NB_RTCC_UnlockedOnly            /*!< Clear only the memory cache; locked tiles are kept and not deleted */

} NB_RasterTileClearCache;

/*! Information of a raster tile.

    @see NB_VectorTileManagerGetTile
*/
typedef struct
{
    uint32  xPosition;              /*!< See NB_VectorTileManagerGetTile() for a description of x/y and zoomLevel. */
    uint32  yPosition;
    uint32  zoomLevel;

    NB_RasterTileType type;

} NB_RasterTileInformation;

/*! Data buffer for a raster tile. */
typedef struct
{
    uint8*  buffer;
    uint32  size;

} NB_RasterTileBuffer;

/*! Raster tile. */
typedef struct
{
    NB_RasterTileInformation    information;
    NB_RasterTileBuffer         buffer;

} NB_RasterTile;

/*! Structure for multiple tile requests.

    @see NB_RasterTileManagerGetTiles
*/
typedef struct
{
    const NB_RasterTileInformation*     information;
    uint32                              priority;
    NB_RasterTileCommand                command;
    NB_RasterTileBuffer*                buffer;

} NB_RasterTileRequest;

/*! Configuration for each type/connection of tile.

    @see NB_RasterTileConfiguration
*/
typedef struct
{
    /*! Image format used for tiles */
    NB_RasterTileImageFormat imageFormat;

    /*! Maximum number of concurrent HTTP requests for tile downloads. Set to zero if this connection is not used. */
    uint16  maximumConcurrentRequests;

    /*! Maximum number of tiles allowed in the pending tile list. Set to zero if this connection is not used. */
    uint16  maximumPendingTiles;

} NB_RasterTileConnectionConfiguration;

/*! Configuration used for raster tile manager */
typedef struct
{
    /*! Tile size for all requested tiles from the server. All tiles are square, the number size
        represents the width and height. Currently only 64, 128 and 256 is supported. */
    uint32  tileSize;

    /*! Resolution in DPI for all requested tiles */
    uint32  tileResolution;

    /*! Configuration settings for all map tile types. */
    NB_RasterTileConnectionConfiguration mapConfiguration;
    NB_RasterTileConnectionConfiguration routeConfiguration;
    NB_RasterTileConnectionConfiguration trafficConfiguration;
    NB_RasterTileConnectionConfiguration satelliteConfiguration;
    NB_RasterTileConnectionConfiguration hybridConfiguration;
    NB_RasterTileConnectionConfiguration routeAlternate1Configuration;
    NB_RasterTileConnectionConfiguration routeAlternate2Configuration;
    NB_RasterTileConnectionConfiguration routeAlternate3Configuration;

    char    clientGuidData[NB_RTM_MAX_CLIENT_GUID_LENGTH];

} NB_RasterTileConfiguration;


/*! Callback function for raster tile download.

    The callback is called once a new tile is available and returns a tile buffer. NB_RasterTileManagerUnlockTile()
    has to be called once the tile buffer is no longer needed. See NB_RasterTileManagerGetTile() for
    additional information on caching.

    If the cache is full and all tiles are locked then the callback will return 'NE_RES' and discard the tile.

    If the tile doesn't exist on the server, the callback will return NE_INVAL, for example if a
    traffic tile or route tile doesn't exist at a particular location

    For valid data, the callback will return NE_OK

    This callback is also used to return the result of the data source initialization. If used for the data
    source initialization the 'tile' parameter is not used.

    @return None

    @see NB_RasterTileManagerGetTile
    @see NB_RasterTileManagerUnlockTile
*/
typedef void (*NB_RasterTileManagerCallbackFunction)(NB_RasterTileManager* manager,
                                                     NB_Error result,               /*!< Result of download request. 'NE_RES' if there are too many locked tiles. */
                                                     const NB_RasterTile* tile,     /*!< New available tile. NB_RasterTileManagerUnlockTile() once the data is no longer needed.
                                                                                         Can be NULL if a connection error occured. */
                                                     void* userData                 /*!< User data specified in NB_RasterTileManagerCreate() */
                                                     );


/*! Structure for callback data for raster tile download */
typedef struct
{
    NB_RasterTileManagerCallbackFunction    callback;
    void*                                   callbackData;
} NB_RasterTileManagerCallback;


// Public Functions ..............................................................................

/*! Create an raster tile manager instance.

    The data source needs to be retrieved using NB_RasterTileDataSourceHandler before calling this
    function.

    @returns NB_Error
    @see NB_RasterTileManagerDestroy
    @see NB_RasterTileDataSourceHandlerCreate
*/
NB_DEC NB_Error
NB_RasterTileManagerCreate(NB_Context* context,
                           const NB_RasterTileDataSourceData* dataSourceData,       /*!< Data source data */
                           const char* mapTileAccessToken,                          /*!< optional map tile access token, pass NULL if not used. */
                           const NB_RasterTileConfiguration* tileConfiguration,     /*!< Configuration parameters for raster tile manger. */
                           const NB_RasterTileManagerCallback* callback,            /*!< Callback data used for tile download. */
                           NB_RasterTileManager** instance                          /*!< On return the new raster tile manager instance. NULL on error. */
                           );

/*! Destroy a raster tile manager instance.

    @returns NB_Error
    @see NB_RasterTileManagerCreate
*/
NB_DEC NB_Error
NB_RasterTileManagerDestroy(NB_RasterTileManager* instance                          /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                            );

/*! Check to see if a particular data source type is available.

    Returns TRUE if the type is available, else FALSE

    @returns nb_boolean
*/
NB_DEC NB_Error
NB_RasterTileManagerTileTypeIsAvailable(NB_RasterTileManager* instance,
                            NB_RasterTileType type,
                            nb_boolean* result
                            );

/*! Get a raster tile.

    This function can be used to get a tile from the cache without triggering a download of the tile
    (see NB_RasterTileCommand for more details).

    If the command is set to 'NB_RTC_Download' and the tile is not in the cache then it will be added
    to the internal download queue. The callback given in the create function will be called once the tile
    is available. If the tile is available then it will be returned immediately.

    If the requested tile is already in the pending download queue then this function will behave the same way
    as NB_RasterTileManagerUpdateTilePriority(). It will update the priority (if different) but not add an
    additonal request to the internal queue.

    If a tile was returned by this function then NB_RasterTileManagerUnlockTile() has to be called once
    the tile is no longer needed. If the cache is full and all tiles are locked then the tile will be
    discarded and the callback will return an error.

    @returns NB_Error
    @see NB_RasterTileCommand
    @see NB_RasterTileManagerUnlockTile
    @see NB_RasterTileManagerCallbackFunction
*/
NB_DEC NB_Error
NB_RasterTileManagerGetTile(NB_RasterTileManager* instance,                         /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                            const NB_RasterTileInformation* information,            /*!< Information of tile to request */
                            uint32 priority,                                        /*!< Priority for tile download. Any positive value. '1' being the highest priority.
                                                                                         Only valid if command is set to 'NB_RTC_Download'. */
                            NB_RasterTileCommand command,                           /*!< Command for retrieving tiles */
                            NB_RasterTileBuffer* buffer                             /*!< If the tile is available immediately then this buffer will be filled on return. It will be
                                                                                         empty if the tile is not available. */
                            );

/*! Get multiple raster tiles.

    This function does a bulk request for tiles. It is faster to request multiple tiles at once. Otherwise this
    function works the same as NB_RasterTileManagerGetTile(). See descrption above.

    @returns NB_Error
    @see NB_RasterTileManagerGetTile
*/
NB_DEC NB_Error
NB_RasterTileManagerGetTiles(NB_RasterTileManager* instance,                        /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                             NB_RasterTileRequest* requestArray,                    /*!< Array of tiles to request. If the tiles are cached then NB_RasterTileBuffer
                                                                                         in the array get filled with the tiles and NB_RasterTileManagerUnlockTile()
                                                                                         has to be called for that particular tile. */
                             uint32 count                                           /*!< Number in request array */
                             );

/*! Unlock a tile from the memory cache.

    This function has to be called for each tile which is returned either by NB_RasterTileManagerGetTile()
    or the callback function. The tile buffer will become invalid after calling this function.

    @returns NB_Error

    @see NB_RasterTileManagerGetTile
    @see NB_RasterTileManagerCallbackFunction
*/
NB_DEC NB_Error
NB_RasterTileManagerUnlockTile(NB_RasterTileManager* instance,                      /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                               const NB_RasterTileInformation* information          /*!< Information of tile to unlock */
                               );


/*! Update the priority for a pending tile.

    Call this function to update the priority of a tile pending for download. This function has no effect
    if the tile was already downloaded.

    @returns NE_OK if the priority was updated, NE_INVAL if the tile is not in the pending list (anymore)
*/
NB_DEC NB_Error
NB_RasterTileManagerUpdateTilePriority(NB_RasterTileManager* instance,              /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                       const NB_RasterTileInformation* information, /*!< Information of tile for which to update the priority */
                                       uint32 priority                              /*!< See NB_RasterTileManagerGetTile() for description of priority parameter */
                                       );

/*! Remove all pending tiles from the internal download queue.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerFlushAllTiles(NB_RasterTileManager* instance                    /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                  );

/*! Remove tile from the internal download queue.

    @returns NE_OK if the tile was removed from the pending list, NE_INVAL if the
             tile is not (no longer) in the pending list.
*/
NB_DEC NB_Error
NB_RasterTileManagerFlushTile(NB_RasterTileManager* instance,                       /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                              const NB_RasterTileInformation* information           /*!< Information of tile to remove from the download queue */
                              );

/*! Clear memory and/or database cache.

    See  NB_RasterTileClearCache for clear options.

    @see NB_RasterTileClearCache
    @see NB_RasterTileManagerUnlockTile

    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerClearCache(NB_RasterTileManager* instance,                      /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                               NB_RasterTileClearCache options                      /*!< Clear options. See NB_RasterTileClearCache for description. */
                               );

/*! Save tiles from the memory cache to the persistent cache.

    When the raster tile manager receives tiles it puts it in the memory cache and not
    in the persistent cache. Call this function to synchronize the memory cache with the
    persistent cache (save any new tiles to the persistent cache).

    This function allows to save only a small number of tiles at a time to the persistent cache
    in order to not impede performance. Call this function periodically (e.g. from a timer) or
    when the system is idle.

    @see NB_RasterTileManagerSavePersistentCacheIndex
    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerSaveToPersistentCache(NB_RasterTileManager* instance,           /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                          int32 maximumEntries,                     /*!< Maximum entries to save to the persistent cache. Set to '-1' to save all entries. */
                                          int32* savedEntries                       /*!< On return the number of entries saved by this call. Can be less than the maximum
                                                                                         requested entries. Set to NULL if not needed. */
                                          );

/*! Save the persistent cache index.

    This function can to be called in order to bring the memory cache index and persistent
    cache index in synch. This is normally done during shutdown of the raster tile manager, but can also be achieved periodically
    (e.g. if the application goes in the background or the map becomes inactive) through this function.

    The advantage of calling this function is that if the application gets killed after calling this funciton
    the persistent cache will be in a valid state and can be reloaded again. Any new tile request will
    result in an out-of-synch state again. This is done for performance reasons. See the implementation of CSL_Cache for more detail.

    This function does not save any tiles to the persistent cache. Call NB_RasterTileManagerSaveToPersistentCache()
    prior to calling this function in order to save any unsaved tiles.

    @see NB_RasterTileManagerSaveToPersistentCache
    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerSavePersistentCacheIndex(NB_RasterTileManager* instance         /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                             );

/*! Set routes for route tile requests.

    The routes will be used when route-tiles are requested. The second route is optional.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerSetRoutes(NB_RasterTileManager* instance,                       /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                              NB_RouteId routeId1,                                  /*!< First Route-ID */
                              nb_color routeColor1,                                 /*!< Color to draw the first route overlay on the tile */
                              NB_RouteId routeId2,                                  /*!< Optional second route. Pass empty route-id if not needed */
                              nb_color routeColor2                                  /*!< Color to draw the second route overlay on the tile. Ignored if routeId2 is not set. */
                              );

/*! Set routes for route selector tile requests.

    The routes will be used when route selector tiles are requested.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerSetAlternateRoute(NB_RasterTileManager* instance,               /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                      NB_RouteId routeId1,                          /*!< First Route-ID */
                                      NB_RouteId routeId2,                          /*!< Second Route-ID. Pass empty route-id if not needed */
                                      NB_RouteId routeId3                           /*!< Third Route-ID. Pass empty route-id if not needed */
                                      );

/*! Get debug information.

    This function should only be called during testing/debugging and not in a production version.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_RasterTileManagerGetDebugInformation(NB_RasterTileManager* instance,             /*!< Raster tile manager instance returned by NB_RasterTileManagerCreate() */
                                        int* memoryCacheCount,                      /*!< On return the total number of tiles in the memory cache. Pass NULL if not needed. */
                                        int* persistentCacheCount,                  /*!< On return the total number of tiles in the persistent cache. Pass NULL if not needed. */
                                        int* protectedCount,                        /*!< On return the number of protected tiles (in the memory cache). Pass NULL if not needed. */
                                        int* pendingCount                           /*!< On return the total number of pending tiles. This includes base, traffic and route tiles.
                                                                                         Pass NULL if not needed. */
                                        );


/*! @} */

#endif  // NBRASTERTILEMANAGER_H

