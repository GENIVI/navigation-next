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

    @file     nbtilemanager.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBTILEMANAGER_H
#define NBTILEMANAGER_H

#include "paltypes.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbtiledatasource.h"

/*!
    @addtogroup nbtilemanager
    @{
*/

// Public Types ..................................................................................

typedef struct NB_TileManager NB_TileManager;
typedef struct NB_TileCache   NB_TileCache;

/*! Ivalid tile type id value. */
#define INVALID_TILE_TYPE_ID  ((uint32)-1)

/*!  tile command used for requesting tiles.

    @see NB_TileManagerGetTile
*/
typedef enum
{
    NB_TC_None = 0,

    NB_TC_MemoryCache,         /*!< Return tile if it is in the memory cache. Don't download the tile. */
    NB_TC_PersistentCache,     /*!< Return tile if it is in the memory or persistent cache. Don't download tile. */
    NB_TC_Download,            /*!< Return tile if it is in the cache. Start download if it is not in the cache.  */
    NB_TC_Force_Download       /*!< Force tile download. If the tile is already in the cache then it will be removed
                                    from the cache prior to the download. */
} NB_TileCommand;


/*! Options used to clear cache. */
typedef enum
{
    NB_TCC_All,                    /*!< Clear both the persistent and memory cache */
    NB_TCC_MemoryOnly,             /*!< Clear only the memory cache including any locked tiles. */
    NB_TCC_UnlockedOnly            /*!< Clear only the memory cache; locked tiles are kept and not deleted */
} NB_TileClearCache;

/*! Information of a  tile.

    @see NB_VectorTileManagerGetTile
*/
typedef struct
{
    uint32  xPosition;              /*!< See NB_VectorTileManagerGetTile() for a description of x/y and zoomLevel. */
    uint32  yPosition;
    uint32  zoomLevel;
    uint32  tileTypeId;
    char*   additionalPartOfUrl;    /*!< Additional part of http request */
} NB_TileInformation;

/*! Data buffer for a  tile. */
typedef struct
{
    uint8*  buffer;
    uint32  size;

} NB_TileBuffer;

/*!  tile. */
typedef struct
{
    NB_TileInformation    information;
    NB_TileBuffer         buffer;
} NB_Tile;

/*! Structure for multiple tile requests.

    @see NB_TileManagerGetTiles
*/
typedef struct
{
    const NB_TileInformation*     information;
    uint32                        priority;
    NB_TileCommand                command;
    NB_TileBuffer*                buffer;

} NB_TileRequest;

/*! Configuration for each type/connection of tile.

    @see NB_TileConfiguration
*/
typedef struct
{
    /*! Maximum number of concurrent HTTP requests for tile downloads. Set to zero if this connection is not used. */
    uint16      maximumConcurrentRequests;

    /*! Maximum number of tiles allowed in the pending tile list. Set to zero if this connection is not used. */
    uint16      maximumPendingTiles;

    /*! Validity period of tiles, placed in cache (in seconds). Zero - valid forever*/
    uint32      expirationTime;

    /*! If set to TRUE - tiles will be saved in cache*/
    nb_boolean  isCacheable;

} NB_TileConfiguration;


/*! Callback function for tile download.

    The callback is called once a new tile is available and returns a tile buffer. NB_TileManagerUnlockTile()
    has to be called once the tile buffer is no longer needed. See NB_TileManagerGetTile() for
    additional information on caching.

    If the cache is full and all tiles are locked then the callback will return 'NE_RES' and discard the tile.

    If the tile doesn't exist on the server, the callback will return NE_INVAL, for example if a
    traffic tile or route tile doesn't exist at a particular location

    For valid data, the callback will return NE_OK

    This callback is also used to return the result of the data source initialization. If used for the data
    source initialization the 'tile' parameter is not used.

    @return TRUE - if it needs to invoke common callback

    @see NB_TileManagerGetTile
    @see NB_TileManagerUnlockTile
*/
typedef nb_boolean (*NB_TileManagerCallbackFunction)(NB_TileManager* manager,
                                                     NB_Error result,               /*!< Result of download request. 'NE_RES' if there are too many locked tiles. */
                                                     const NB_Tile* tile,           /*!< New available tile. NB_TileManagerUnlockTile() once the data is no longer needed.
                                                                                         Can be NULL if a connection error occured. */
                                                     void* userData                 /*!< User data specified in NB_TileManagerCreate() */
                                                     );

/*! Structure for callback data for tile download */
typedef struct
{
    NB_TileManagerCallbackFunction          callback;
    void*                                   callbackData;
} NB_TileManagerCallback;


// Public Functions ..............................................................................

/*! Create an  tile manager instance.

    @returns NB_Error
    @see NB_TileManagerDestroy
*/
NB_DEC NB_Error
NB_TileManagerCreate(NB_Context* context,
                     const NB_TileManagerCallback* callback,            /*!< Callback data used for tile download. */
                     NB_TileManager** instance                          /*!< On return the new  tile manager instance. NULL on error. */
                     );

/*! Destroy a  tile manager instance.

    @returns NB_Error
    @see NB_TileManagerCreate
*/
NB_DEC NB_Error
NB_TileManagerDestroy(NB_TileManager* instance                          /*!<  tile manager instance returned by NB_TileManagerCreate() */
                      );

/*! Add type of tiles

    @see NB_TileDataSourceTemplate
    @see NB_TileConfiguration
    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerAddTileType(NB_TileManager* instance,
                          NB_TileDataSourceTemplate* tileDataSourceTemplate, /*!< Tile Data Source Template. */
                          NB_TileConfiguration* tileConfiguration, /*!< Configuration parameters for type of tiles */
                          uint32* tileTypeId,                                          /*!< Tile Manager returns number of type of tiles */
                          const NB_TileManagerCallback* callback                 /*!< Callback data used for tile download. */
                          );


/*! Remove type of tiles

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerRemoveTileType(NB_TileManager* instance,
                             uint32 tileTypeId
                             );

/*! Check to see if type of tile is presented for a data source template

    Returns tileTypeId if the type is available

    @see NB_TileDataSourceTemplate
    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerDoesTileTypeExist(NB_TileManager* instance,
                                NB_TileDataSourceTemplate* tileDataSourceTemplate,
                                uint32* tileTypeId
                                );

/*! Check to see if URL template of specified tile has user parameter

    Returns TRUE if the parameter is available, else FALSE

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter( NB_TileManager* instance,
                            uint32 tileTypeId,
                            nb_boolean* result );

/*! Check to see if URL template of specified tile has tile size parameter

    Returns TRUE if the parameter is available, else FALSE

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerDoesTileTypeUrlTemplateHaveTileSize( NB_TileManager* instance,
                            uint32 tileTypeId,
                            nb_boolean* result );

/*! Check to see if URL has user parameter

    @returns TRUE if the parameter is available, else FALSE
*/
NB_DEC nb_boolean
NB_TileManagerDoesUrlHaveUserParameter( const char *URL );

/*! Check to see if URL has tile size parameter

    @returns TRUE if the parameter is available, else FALSE
*/

NB_DEC nb_boolean
NB_TileManagerDoesUrlHaveTileSize( const char *URL );

/*! Check to see if a particular data source type is available.

    Returns TRUE if the type is available, else FALSE

    @returns nb_boolean
*/
NB_DEC NB_Error
NB_TileManagerTileTypeIsAvailable(NB_TileManager* instance,
                                  uint32 tileTypeId,
                                  nb_boolean* result
                                  );

/*! Get a  tile.

    This function can be used to get a tile from the cache without triggering a download of the tile
    (see NB_TileCommand for more details).

    If the command is set to 'NB_TC_Download' and the tile is not in the cache then it will be added
    to the internal download queue. The callback given in the create function will be called once the tile
    is available. If the tile is available then it will be returned immediately.

    If the requested tile is already in the pending download queue then this function will behave the same way
    as NB_TileManagerUpdateTilePriority(). It will update the priority (if different) but not add an
    additonal request to the internal queue.

    If recieved tile isn't cacheable, you SHOULD invoke NB_TileManagerUnlockTile() before request it again.

    If a tile was returned by this function then NB_TileManagerUnlockTile() has to be called once
    the tile is no longer needed. If the cache is full and all tiles are locked then the tile will be
    discarded and the callback will return an error.

    Also the additional callback can be provided into the function for one request.
    If all of callback functions return TRUE we ignore the rest of callbacks - callback of tile type and common callback

    @returns NB_Error
    @see NB_TileCommand
    @see NB_TileManagerUnlockTile
    @see NB_TileManagerCallbackFunction
*/
NB_DEC NB_Error
NB_TileManagerGetTile(NB_TileManager* instance,                         /*!<  tile manager instance returned by NB_TileManagerCreate() */
                      const NB_TileInformation* information,            /*!< Information of tile to request */
                      uint32 priority,                                  /*!< Priority for tile download. Any positive value. '1' being the highest priority.
                                                                             Only valid if command is set to 'NB_TC_Download'. */
                      NB_TileCommand command,                           /*!< Command for retrieving tiles */
                      NB_TileBuffer* buffer,                            /*!< If the tile is available immediately then this buffer will be filled on return. It will be
                                                                             empty if the tile is not available. */
                      const NB_TileManagerCallback* callback
                      );

/*! Get multiple  tiles.

    This function does a bulk request for tiles. It is faster to request multiple tiles at once. Otherwise this
    function works the same as NB_TileManagerGetTile(). See descrption above.

    @returns NB_Error
    @see NB_TileManagerGetTile
*/
NB_DEC NB_Error
NB_TileManagerGetTiles(NB_TileManager* instance,                        /*!<  tile manager instance returned by NB_TileManagerCreate() */
                       NB_TileRequest* requestArray,                    /*!< Array of tiles to request. If the tiles are cached then NB_TileBuffer
                                                                             in the array get filled with the tiles and NB_TileManagerUnlockTile()
                                                                             has to be called for that particular tile. */
                       uint32 count                                     /*!< Number in request array */
                       );

/*! Unlock a tile from the memory cache.

    This function has to be called for each tile which is returned either by NB_TileManagerGetTile()
    or the callback function. The tile buffer will become invalid after calling this function.

    @returns NB_Error

    @see NB_TileManagerGetTile
    @see NB_TileManagerCallbackFunction
*/
NB_DEC NB_Error
NB_TileManagerUnlockTile(NB_TileManager* instance,                      /*!<  tile manager instance returned by NB_TileManagerCreate() */
                         const NB_TileInformation* information          /*!< Information of tile to unlock */
                         );



/*! Remove all pending tiles from the internal download queue.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerFlushAllTiles(NB_TileManager* instance                    /*!<  tile manager instance returned by NB_TileManagerCreate() */
                            );

/*! Remove tile from the internal download queue.

    Also the additional callback can be provided into the function.
    Tile Manager will remove callback by using internal properties
    NB_TileManagerCallbackFunction callback and callbackData
    from the vector of callbacks for the flushed tile

    @returns NE_OK if the tile was removed from the pending list, NE_INVAL if the
             tile is not (no longer) in the pending list.
*/
NB_DEC NB_Error
NB_TileManagerFlushTile(NB_TileManager* instance,                       /*!<  tile manager instance returned by NB_TileManagerCreate() */
                        const NB_TileInformation* information,           /*!< Information of tile to remove from the download queue */
                        const NB_TileManagerCallback* callback
                        );

/*! Clear memory and/or database cache.

    See  NB_TileClearCache for clear options.

    @see NB_TileClearCache
    @see NB_TileManagerUnlockTile

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerClearCache(NB_TileManager* instance,                      /*!<  tile manager instance returned by NB_TileManagerCreate() */
                         NB_TileClearCache options                      /*!< Clear options. See NB_TileClearCache for description. */
                         );

/*! Save tiles from the memory cache to the persistent cache.

    When the  tile manager receives tiles it puts it in the memory cache and not
    in the persistent cache. Call this function to synchronize the memory cache with the
    persistent cache (save any new tiles to the persistent cache).

    This function allows to save only a small number of tiles at a time to the persistent cache
    in order to not impede performance. Call this function periodically (e.g. from a timer) or
    when the system is idle.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerSaveToPersistentCache(NB_TileManager* instance,           /*!<  tile manager instance returned by NB_TileManagerCreate() */
                                    int32 maximumEntries,               /*!< Maximum entries to save to the persistent cache. Set to '-1' to save all entries. */
                                    int32* savedEntries                 /*!< On return the number of entries saved by this call. Can be less than the maximum
                                                                             requested entries. Set to NULL if not needed. */
                                     );

/*! Set routes for route tile requests.

    The routes will be used when route-tiles are requested. The second route is optional.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerSetRoutes(NB_TileManager* instance,                       /*!< Tile manager instance returned by NB_RasterTileManagerCreate() */
                        NB_RouteId routeId1,                            /*!< First Route-ID */
                        nb_color routeColor1,                           /*!< Color to draw the first route overlay on the tile */
                        NB_RouteId routeId2,                            /*!< Optional second route. Pass empty route-id if not needed */
                        nb_color routeColor2                            /*!< Color to draw the second route overlay on the tile. Ignored if routeId2 is not set. */
                        );

/*! Generate tile cache key for specified tile information

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerGetTileKey(NB_TileManager* instance,                      /*!< Tile manager instance */
                         const NB_TileInformation* information,         /*!< Tile information for which to generate the key */
                         char** key,                                     /*!< Pointer to a buffer to receive the key. Has to be mo more than MAXIMUM_TILE_KEY_LENGTH. */
                         int* length);                                   /*!< Length of a buffer */

/*! Clear tile from cache for specified tile information

    @returns NB_Error
*/

NB_DEC NB_Error
NB_TileManagerClearTileFromCache(NB_TileManager* instance,
                         const NB_TileInformation* information);

/*! Remove all pending tiles of specified tileTypeId from the internal download queue.

    @returns NB_Error
*/
NB_DEC NB_Error
NB_TileManagerFlushAllTilesByTileTypeId(NB_TileManager* instance,
                            uint32 tileTypeId);

/*! @} */

#endif  // NBTILEMANAGER_H

