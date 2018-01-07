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

    @file     nbtilemanager.c
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

#include "nbcontextprotected.h"
#include "nbtilemanager.h"
#include "palnet.h"
#include "csdict.h"
#include "cslcache.h"
#include "cslutil.h"
#include "cslquadkey.h"
#include "vec.h"
#include "nbqalog.h"
#include "palmath.h"
#include "nbcontextaccess.h"
#include "dynbuf.h"
#include "paltestlog.h"


/*! @{ */

// Constants .....................................................................................
#define MAXIMUM_REQUESTS_PER_CONNECTION  2
// Initial size of tile callbacks dictionary. It is initial count of requested tiles.
#define INITIAL_SIZE_OF_TILE_CALLBACKS_DICTIONARY 8
// Approximate maximum value for x,y,z in chars
#define XYZ_LENGTH                                30
// Length for tileSize ($sz) in chars
#define TILE_SIZE_LENGTH                          10

// following define should be updaed if some default placeholder will be expanded
#define DEFAULT_PLACEHOLDER_MAX_LENGTH            20
#define HTTP_VERB                             "GET"

// Remove this from the base url. The connection will fail if it is present.
#define BASE_HTTP_URL_REMOVE                  "http://"
#define BASE_HTTPS_URL_REMOVE                 "https://"
#define BASE_PROTOCOL_URL_LENGTH              10

// Remove this from the url.
#define URL_PARAMETER_USER                "$user"
#define URL_PARAMETER_TILE_SIZE            "$sz"

#define AMPERSAND                         "&"

// We use these values for our HTTP requests. I don't think there is a need to make them
// setable at this point. Most of them are currently ignored by the server anyway.
#define DEFAULT_LOCALE                 "en-us"
#define DEFAULT_VERSION                "1.0"

#define IMAGE_FORMAT_PNG               "png"
#define IMAGE_FORMAT_GIF               "gif"
#define IMAGE_FORMAT_JPG               "jpg"

#define TILE_CACHE_PARAMETER_X          "$x"
#define TILE_CACHE_PARAMETER_Y          "$y"
#define TILE_CACHE_PARAMETER_Z          "$z"
#define TILE_CACHE_PARAMETER_Q          "$q"
#define TILE_CACHE_PARAMETER_LANGUAGE   "$language"

#define TEMPLATE_PRINT_STRING            "%s"
#define TEMPLATE_PRINT_INT               "%d"

// TEMPLATE_TILE_CALLBACKS_KEY_MAX_LENGTH should reflect TEMPLATE_TILE_CALLBACKS_KEY's lenghth
#define TEMPLATE_TILE_CALLBACKS_KEY      "%d-%d-%d-%d-%d"
#define TEMPLATE_TILE_CALLBACKS_KEY_MAX_LENGTH  32
// Local types ...................................................................................

/*! Connection state used to determine if we need a new connection. */
typedef enum
{
    CS_Disconnected = 0,   /*!< Default state */
    CS_ConnectionError,    /*!< There's a error on network connection. */
    CS_ConnectionPending,  /*!< Connect request sent. Waiting for confirmation via connection status callback. */
    CS_Connected,          /*!< Connected and ready for tile download. */
    CS_ProxyAuthRequired   /*!< Wait to require the proxy credentials. */
} ConnectionState;

/*! Tile request data

    A pointer to this structure gets passed as request data to the PAL connection callbacks.
*/
typedef struct
{
    /*! Requested tile */
    NB_Tile        tile;

    /*! If we run out of memory during this tile download then we set this flag and report the error once the
        download has finished (instead of reporting multiple errors). */
    nb_boolean              allocationError;

    /*! Total number of bytes expected to be received in the response's body for the request,
        or -1 if unknown. */
    uint32                  contentLength;
} NB_TileRequestData;

/*! Connection data

    A pointer to this structure gets passed as user data to the PAL connection callbacks. It has
    all the information needed for this connection.
*/

typedef struct
{
    PAL_NetConnection*      connection;
    ConnectionState         state;

    /*! Pointer to the parent structure. */
    NB_TileManager*         manager;

    /*! Tile ready for request, when network is not connected */
    NB_Tile                 pendingConnectionTile;

    /*! Number of simultaneous requests allowed on this connection */
    uint32                  allowedRequests;

        /*! Array of outstanding tile requests */
    NB_TileRequestData**    tileRequests;

    /*! Number of on going requests on this connection which have not been finished to completion */
    uint32                  onGoingRequests;

    /*! Type of tiles for this connection. */
    uint32                  typeId;
} Connection;

/*! Structure including priority and tile data

    We use this structure for the pending tile vector.
*/
typedef struct
{
    /*! Priority of the tile. See NB_TileManagerGetTile for description of allowed values. */
    int priority;

    /*! Pending tile to download. */
    NB_TileInformation information;

} TileWithPriority;

/*! Configuration of type of tiles used for tile manager */
typedef struct
{
    uint32                          tileTypeId;
    /*! Tile size for all requested tiles from the server. All tiles are square, the number size
        represents the width and height. Currently only 64, 128 and 256 is supported. */

    /*! Array of connections tiles. */
    Connection*                     connections;

    /*! HTTP Ports for connections; get initialized based on the data source strings */
    uint16                          port;

    /*! Vector of pending tiles.

        We can't use a dictionary here, since we need the list to be sorted by priority. The current
        dictionary implementation uses a hash table which would not be sorted correctly. (What we
        really want is a binary tree map.) We don't want to use a linked list for better sorting performance.

        This vector contains TileWithPriority structures.
    */
    struct CSL_Vector*              pendingTiles;

    /*! Configuration for each type/connection of tile. */
    NB_TileConfiguration            configuration;

    /*! Data source template */
    NB_TileDataSourceTemplate       dataSource;

    /*! Structure for callback data for tile download */
    NB_TileManagerCallback          callbackData;

    /*! this is needed for get many tiles */
    nb_boolean                      allConnectionsBusy;

    /*! this is needed for get many tiles */
    nb_boolean                      isShouldBeDownload;

    /*! use Quad key to create url to download tiles*/
    nb_boolean                      useQuadKey;

    /*! additional part for each tile key(used for route tile)*/
    char*                           additionalTileKey;
} NB_TileTypeConfiguration;

/*! The  tile manager object. Gets created in NB_TileManagerCreate.

    @see NB_TileManagerCreate
*/
struct NB_TileManager
{
    NB_Context*                   context;

    NB_TileManagerCallback        callbackData;

    struct CSL_Vector*            tileTypes;

    /*! Cache object to cache all tiles. The cache object is owned by NB_Context. */
    struct CSL_Cache*             cache;

    /*! Vector to save non-cached tiles. */
    struct CSL_Vector*            nonCachedTilesVector;

    /*! Needs to specify number of type tile id. */
    uint32                        autoKey;

    struct CSL_Dictionary*        tileCallbacks;
};


// Local functions

// Callback function for network call
static void ConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status);
static void DataReceivedCallback(void* userData, void* requestData, PAL_Error errorCode, const byte* bytes, uint32 count);
static boolean TileRequestBegin(Connection* pConnection, NB_TileRequestData* pTileRequest);
static boolean TileRequestEnd(Connection* pConnection, NB_TileRequestData* pTileRequest);
static boolean TileRequestOutstanding(NB_TileManager* pThis, const NB_TileInformation* pTileInfo);
static boolean TileConnectionPending(NB_TileManager* pThis, const NB_TileInformation* pTileInfo);

static NB_Error OpenConnection(Connection* connection, const NB_TileInformation* information);
static NB_Error StartRequest(Connection* connection, const NB_TileInformation* information);
static boolean ProcessNextPendingTile(Connection* connection);
static void ResortAndLimitPendingVector(struct CSL_Vector* vector, int limit);
static boolean FindPendingEntry(NB_TileManager* instance, const NB_TileInformation* information, TileWithPriority** pEntry, int* pIndex);
static struct CSL_Vector* GetPendingVector(NB_TileManager* instance, uint32 typeId);
static void PendingConnectionTileClear(NB_TileManager* instance, Connection *connection, nb_boolean fullClear);

static int CallbackVectorCompare(const void* item1,const void* item2);
static int PriorityVectorCompare(const void* item1, const void* item2);
static NB_Error GenerateTileKey(NB_TileManager* instance, const NB_TileInformation* information, char** key, int* length);
static NB_Error InitializeConnections(NB_TileManager* pThis, uint32 connectionCount, Connection** connections, uint32 typeId, PAL_NetConnectionProtocol connectionProtocol);
static void CleanupConnections(NB_TileManager* pThis, uint32 connectionCount, Connection* connections);
static NB_Error CheckConnectionProtocol(NB_TileDataSourceTemplate *dataSource, PAL_NetConnectionProtocol *connectionProtocol);

static NB_Error FormatSource(NB_TileManager* pThis,
                             NB_TileTypeConfiguration* tileType,
                             NB_RouteId* routeId1,
                             nb_color* routeColor1,
                             NB_RouteId* routeId2,
                             nb_color* routeColor2);

static NB_Error BuildRouteString(char** routeString,
                                 NB_RouteId* routeId1,
                                 nb_color* routeColor1,
                                 NB_RouteId* routeId2,
                                 nb_color* routeColor2);

static boolean ProcessPlaceholder(char** currentPosition, const char* placeHolder);
static NB_Error ExtractPortsAndFormatBaseUrl(NB_TileManager* pThis, char** baseUrl, char** templateUrl, uint16* port, PAL_NetConnectionProtocol connectionProtocol);
static NB_Error ExtractTileStoreTemplate(char** tileStoreTemplate, const char* templateURL, const char* baseURL);
static NB_Error CloneTileDataSourceTemplate(NB_TileDataSourceTemplate* dest, const NB_TileDataSourceTemplate* src);
static void CleanTileDataSourceTemplate(NB_TileDataSourceTemplate* p);
static nb_boolean CompareTileDataSourceTemplate(const NB_TileDataSourceTemplate* p1, const NB_TileDataSourceTemplate* p2);
static nb_boolean CompareTileInformation(const NB_TileInformation* p1, const NB_TileInformation* p2);
static NB_Error InitTileType(NB_TileManager* instance, NB_TileTypeConfiguration* tileType, NB_TileDataSourceTemplate* tileDataSourceTemplate);

// Functions for logging
static NB_QaLogMapType GetQaLogMapType(uint32 type);
static void LogCacheStatistics(NB_TileManager* pThis);
static void GetCacheStatistics(NB_TileManager* pThis,
                               int* memoryCacheCount,
                               int* persistentCacheCount,
                               int* protectedCount,
                               int* pendingCount);

static void FreeBuffer(NB_TileBuffer* buffer);
static NB_TileTypeConfiguration* GetTileType(CSL_Vector* vec, uint32 typeId);
static NB_Error TileManagerUpdateTilePriority(NB_TileManager* instance,
                                    const NB_TileInformation* information,
                                    uint32 priority);
int CopyCallbacks(void* callbackData, void* dest, const void* src);
void InvokeCallback(NB_TileManager* pThis,
                    const NB_TileManagerCallback* tileTypeCallback,
                    NB_TileInformation* information,
                    NB_Error result,
                    const NB_Tile* tile);

static NB_Error strReplace( char** dest,
                     const char* src, 
                     const char* to_find, 
                     const char* replace_with);

static NB_Error AddCallbackToTile(NB_TileManager* pThis,
                                  const NB_TileInformation* information,
                                  const NB_TileManagerCallback* callback);


static void RemoveCallbackFromTile(NB_TileManager* pThis,
                                       const NB_TileInformation* information,
                                       const NB_TileManagerCallback* callback);

static void RemoveAllCallbacksFromTile(NB_TileManager* pThis,
                                          const NB_TileInformation* information);

static int GetTileCallbacksCount(NB_TileManager* pThis,
                                 const NB_TileInformation* information);

static CSL_Vector* GetTileCallbacks(NB_TileManager* pThis,
                                    const NB_TileInformation* information);
static NB_Error GenerateTileCallbackKey(NB_TileManager* pThis, const NB_TileInformation* information, char **key, int *length);
// Public Functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerCreate(NB_Context* context,
                     const NB_TileManagerCallback* callback,
                     NB_TileManager** instance)
{
    NB_Error result = NE_OK;
    NB_TileManager* pThis = NULL;

    // Validate parameters
    if (!context)
    {
        return NE_INVAL;
    }

    // Create  tile manager instance
    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    // Copy parameters to local instance
    pThis->context = context;

    if (callback)
    {
        nsl_memcpy(&pThis->callbackData, callback, sizeof(*callback));
    }

    // Get cache object from context and store it internally
    pThis->cache = NB_ContextGetRasterTileManagerCache(context);
    pThis->tileTypes = CSL_VectorAlloc(sizeof(NB_TileTypeConfiguration));
    pThis->nonCachedTilesVector = CSL_VectorAlloc(sizeof(NB_Tile));
    pThis->tileCallbacks = CSL_DictionaryAlloc(INITIAL_SIZE_OF_TILE_CALLBACKS_DICTIONARY);
    pThis->autoKey = 0;

    if (pThis->tileTypes == NULL || pThis->nonCachedTilesVector == NULL || pThis->tileCallbacks == NULL)
    {
        NB_TileManagerDestroy(pThis);
        return NE_NOMEM;
    }

    LogCacheStatistics(pThis);

    *instance = pThis;
    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerDestroy(NB_TileManager* instance)
{
    if (!instance)
    {
        return NE_OK;
    }

    if (instance->tileTypes)
    {
        uint32 i = 0;
        uint32 length = CSL_VectorGetLength(instance->tileTypes);
        NB_TileTypeConfiguration* tileType = NULL;

        for (i = 0; i < length; i++)
        {
            tileType = CSL_VectorGetPointer(instance->tileTypes, 0);
            if (tileType != NULL)
            {
                NB_TileManagerRemoveTileType(instance, tileType->tileTypeId);
            }
        }
        CSL_VectorDealloc(instance->tileTypes);
    }

    if (instance->nonCachedTilesVector)
    {
        uint32 i = 0;
        uint32 length = CSL_VectorGetLength(instance->nonCachedTilesVector);
        NB_Tile* tile = NULL;

        for (i = 0; i < length; i++)
        {
            tile = CSL_VectorGetPointer(instance->nonCachedTilesVector, 0);
            if (tile != NULL)
            {
                RemoveAllCallbacksFromTile(instance, (const NB_TileInformation*)&tile->information);
            }
            if (tile != NULL && tile->buffer.buffer != NULL)
            {
                nsl_free(tile->buffer.buffer);
                tile->buffer.buffer = NULL;
                tile->buffer.size = 0;
            }
            CSL_VectorRemove(instance->nonCachedTilesVector, 0);
        }
        CSL_VectorDealloc(instance->nonCachedTilesVector);
    }

    if (instance->tileCallbacks)
    {
        CSL_DictionaryDealloc(instance->tileCallbacks);
        instance->tileCallbacks = NULL;
    }

    // Don't destroy the cache object here, it is owned by NB_Context

    nsl_free(instance);
    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerAddTileType(NB_TileManager* instance,
                          NB_TileDataSourceTemplate* tileDataSourceTemplate,
                          NB_TileConfiguration* tileConfiguration,
                          uint32* tileTypeId,
                          const NB_TileManagerCallback* callback
                          )
{
    NB_TileTypeConfiguration* tileType = NULL;
    NB_Error err = NE_OK;
    PAL_NetConnectionProtocol connectionProtocol = PNCP_HTTP;

    if (!instance || !tileDataSourceTemplate || !tileConfiguration)
    {
        return NE_INVAL;
    }

    tileType = nsl_malloc(sizeof(NB_TileTypeConfiguration));

    if (!tileType)
    {
        err = NE_NOMEM;
        goto cleanup;
    }
    nsl_memset(tileType, 0, sizeof(NB_TileTypeConfiguration));

    tileType->tileTypeId = instance->autoKey++;

    err = InitTileType(instance, tileType, tileDataSourceTemplate);
    if (err != NE_OK)
    {
        goto cleanup;
    }

    nsl_memcpy(&tileType->configuration, tileConfiguration, sizeof(NB_TileConfiguration));

    if (callback)
    {
        tileType->callbackData.callback = callback->callback;
        tileType->callbackData.callbackData = callback->callbackData;
    }

    // Create vector for pending tiles
    tileType->pendingTiles = CSL_VectorAlloc(sizeof(TileWithPriority));

    if (!tileType->pendingTiles)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    (void)CheckConnectionProtocol(tileDataSourceTemplate, &connectionProtocol);

    /* Create all connections. The traffic and route connections are optional. */
    if (InitializeConnections(instance, tileType->configuration.maximumConcurrentRequests, &tileType->connections, tileType->tileTypeId, connectionProtocol) != NE_OK)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    if (!CSL_VectorAppend(instance->tileTypes, tileType))
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    if (tileTypeId)
    {
        *tileTypeId = tileType->tileTypeId;
    }

    nsl_free(tileType);

    return err;

cleanup:
    if (tileType)
    {
        if (tileType->pendingTiles)
        {
            CSL_VectorDealloc(tileType->pendingTiles);
            tileType->pendingTiles = NULL;
        }
        if (tileType->connections)
        {
            CleanupConnections(instance, tileType->configuration.maximumConcurrentRequests, tileType->connections);
            tileType->connections = NULL;
        }
        nsl_free(tileType);
    }

    return err;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerRemoveTileType(NB_TileManager* instance,
                             uint32 tileTypeId
                             )
{
    NB_TileTypeConfiguration* tileType = NULL;
    uint32 i = 0;
    uint32 length = 0;

    if (instance->tileTypes == NULL)
    {
        return NE_NOENT;
    }

    tileType = GetTileType(instance->tileTypes, tileTypeId);

    if (tileType == NULL)
    {
        return NE_NOENT;
    }

    if (tileType->pendingTiles)
    {
        TileWithPriority* tile = NULL;

        length = CSL_VectorGetLength(tileType->pendingTiles);
        for (i = 0; i < length; i++)
        {
            tile = CSL_VectorGetPointer(tileType->pendingTiles, i);
            if (tile != NULL)
            {
                RemoveAllCallbacksFromTile(instance, (const NB_TileInformation*)&tile->information);
                if(tile->information.additionalPartOfUrl)
                {
                    nsl_free(tile->information.additionalPartOfUrl);
                    tile->information.additionalPartOfUrl = NULL;
                }
            }
        }
        CSL_VectorDealloc(tileType->pendingTiles);
        tileType->pendingTiles = NULL;
    }
    if (tileType->connections)
    {
        CleanupConnections(instance, tileType->configuration.maximumConcurrentRequests, tileType->connections);
        tileType->connections = NULL;
    }

    if (instance->nonCachedTilesVector && !tileType->configuration.isCacheable)
    {
        int i = 0;
        int length = CSL_VectorGetLength(instance->nonCachedTilesVector);
        NB_Tile* tile = NULL;
        for (i = length - 1; i >= 0; i--)
        {
            tile = CSL_VectorGetPointer(instance->nonCachedTilesVector, i);

            if (tile != NULL && tile->information.tileTypeId == tileType->tileTypeId)
            {
                RemoveAllCallbacksFromTile(instance, (const NB_TileInformation*)&tile->information);

                if (tile->buffer.buffer != NULL)
                {
                    nsl_free(tile->buffer.buffer);
                    tile->buffer.buffer = NULL;
                    tile->buffer.size = 0;
                }
                CSL_VectorRemove(instance->nonCachedTilesVector, i);
            }
        }
    }

    CleanTileDataSourceTemplate(&tileType->dataSource);

    length = CSL_VectorGetLength(instance->tileTypes);
    for(i = 0; i < length; i++)
    {
        tileType = (NB_TileTypeConfiguration*)CSL_VectorGetPointer(instance->tileTypes, i);

        if(tileType->tileTypeId == tileTypeId)
        {
            if (tileType->additionalTileKey)
            {
                nsl_free(tileType->additionalTileKey);
                tileType->additionalTileKey = NULL;
            }
            CSL_VectorRemove(instance->tileTypes, i);
            break;
        }
    }

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerTileTypeIsAvailable(NB_TileManager* instance,
                                  uint32 typeId,
                                  nb_boolean* result
                                  )
{
    Connection* connectionArray = NULL;
    struct CSL_Vector* pendingTileVector = NULL;
    NB_TileTypeConfiguration* tileType = NULL;


    if (instance == NULL || result == NULL)
    {
        return NE_INVAL;
    }

    tileType = GetTileType(instance->tileTypes, typeId);

    if (tileType)
    {
        connectionArray     = tileType->connections;
        pendingTileVector   = tileType->pendingTiles;
    }

    *result = (!connectionArray || !pendingTileVector) ? FALSE : TRUE;

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerGetTile(NB_TileManager* instance,
                      const NB_TileInformation* information,
                      uint32 priority,
                      NB_TileCommand command,
                      NB_TileBuffer* buffer,
                      const NB_TileManagerCallback* callback
                      )
{
    /*
        This function is very similar to NB_TileManagerGetTiles(). When making changes here make sure to update
        NB_TileManagerGetTiles() as well.
    */
    NB_Error error = NE_OK;
    nb_boolean needFree = FALSE;
    NB_Error cacheResult = NE_OK;
    NB_TileTypeConfiguration* tileType = NULL;
    NB_TileInformation localInformation = {0};

    nsl_memcpy(&localInformation, information, sizeof(NB_TileInformation));
    if (information->additionalPartOfUrl != NULL)
    {
        localInformation.additionalPartOfUrl = nsl_strdup(information->additionalPartOfUrl);
        if (localInformation.additionalPartOfUrl == NULL)
        {
            return NE_NOMEM;
        }
    }

    // Validate input
    if (command == NB_TC_None)
    {
        error = NE_INVAL;
        goto exit;
    }

    buffer->buffer = NULL;
    buffer->size = 0;

    tileType = GetTileType(instance->tileTypes, localInformation.tileTypeId);
    if (tileType == NULL)
    {
        error = NE_INVAL;
        goto exit;
    }
    if (NE_OK != AddCallbackToTile(instance, &localInformation, callback))
    {
        error = NE_NOMEM;
        goto exit;
    }
    if (TileRequestOutstanding(instance, &localInformation) ||
        TileConnectionPending(instance, &localInformation))
    {
        // Outstanding request already issued.
        error = NE_OK;
        needFree = TRUE;
        goto exit;
    }

    if (tileType->configuration.isCacheable)
    {
        char* key = NULL;

        if ((NE_OK != GenerateTileKey(instance, &localInformation, &key, NULL)) || (!key))
        {
            RemoveCallbackFromTile(instance, (const NB_TileInformation*)&localInformation, callback);
            error = NE_NOMEM;
            goto exit;
        }
        if (command == NB_TC_Force_Download)
        {
            // If the force-download flag is set then we remove the item (if present) from the cache. Don't check the
            // result, in case the item is not in the cache.
            CSL_CacheClearOne(instance->cache, (byte*)key, nsl_strlen(key));

            LogCacheStatistics(instance);
        }
        else
        {
            // Check if the tile is in the cache. If it is in the cache then the entry will be locked and we
            // return the buffer right away.
            cacheResult = CSL_CacheFind(instance->cache,
                                        (byte*)key,
                                        nsl_strlen(key),
                                        &buffer->buffer,
                                        (size_t*)&buffer->size,
                                        TRUE,
                                        FALSE,
                                        (nb_boolean)(command == NB_TC_MemoryCache));

            LogCacheStatistics(instance);

            // If the tile was in the cache or the command is not set to download then we return
            if ((cacheResult == NE_OK) || (command != NB_TC_Download))
            {
                if(key)
                {
                    nsl_free(key);
                }
                RemoveCallbackFromTile(instance, (const NB_TileInformation*)&localInformation, callback);
                error = cacheResult;
                needFree = TRUE;
                goto exit;
            }
        }
        if(key)
        {
            nsl_free(key);
        }
    }
    {
        uint32 i = 0;
        Connection* connectionArray = NULL;
        uint16 connectionCount = 0;
        int maximumPendingTiles = 0;
        struct CSL_Vector* pendingTileVector = NULL;

        if (tileType)
        {
            connectionArray     = tileType->connections;
            connectionCount     = tileType->configuration.maximumConcurrentRequests;
            pendingTileVector   = tileType->pendingTiles;
            maximumPendingTiles = tileType->configuration.maximumPendingTiles;
        }

        if (!connectionArray || !pendingTileVector)
        {
            RemoveCallbackFromTile(instance, (const NB_TileInformation*)&localInformation, callback);
            error = NE_INVAL;
            goto exit;
        }

        /*
            Search pending tile vector for the requested tile. If tile is already pending,
            update the priority, re-sort the list, and exit without adding a new request.
        */
        if (TileManagerUpdateTilePriority(instance, &localInformation, priority) == NE_OK)
        {
            // Return out; no new request necessary
            error = NE_OK;
            needFree = TRUE;
            goto exit;
        }

        // For all network connections
        for (i = 0; i < connectionCount; i++)
        {
            Connection* connection = &(connectionArray[i]);
            /*
                If this connection is currently disconnected then we initiate the connect and make
                the HTTP request in the status callback once the connection is connected.
            */
            if (connection->state == CS_Disconnected)
            {
                // Open the connection. We make the HTTP request in the status callback once the connection
                // is connected.
                error = OpenConnection(connection, &localInformation);
                goto exit;
            }
            /*
                Start the request, if number of on going requests with CS_Connected connection are less than
                the allowed number of requests.
            */
            else if (connection->state == CS_Connected && connection->onGoingRequests < connection->allowedRequests)
            {
                // Start the http request. The results will be handled in the receive callback.
                error = StartRequest(connection, &localInformation);
                goto exit;
            }
        }

        /* All connections are currently serving requests to maximum allowed limit.
         Add to pending list. As soon as number of on going requests on a connection drop
         to less than the maximum allowed requests, it will process the next tile request.
         See receive callback. */
        {
            TileWithPriority tile = {0};
            tile.priority = priority;
            nsl_memcpy(&tile.information, &localInformation, sizeof(NB_TileInformation));

            if (!CSL_VectorAppend(pendingTileVector, &tile))
            {
                RemoveCallbackFromTile(instance, (const NB_TileInformation*)&localInformation, callback);
                error = NE_NOMEM;
                goto exit;
            }

            ResortAndLimitPendingVector(pendingTileVector, maximumPendingTiles);
        }

        error = NE_OK;
    }
exit:
    if (localInformation.additionalPartOfUrl && (needFree || error != NE_OK))
    {
        nsl_free(localInformation.additionalPartOfUrl);
        localInformation.additionalPartOfUrl = NULL;
    }

    return error;

}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerGetTiles(NB_TileManager* instance,
                       NB_TileRequest* requestArray,
                       uint32 count
                       )
{
    /*
        This function is very similar to NB_TileManagerGetTile(). When making changes here make sure to update
        NB_TileManagerGetTile() as well.
    */

    NB_Error result = NE_OK;
    uint32 i = 0;
    uint32 length = 0;
    NB_TileTypeConfiguration* tileType = NULL;


    // For all requested tiles
    for (i = 0; i < count; ++i)
    {
        NB_Error cacheResult = NE_OK;
        boolean needsDownload = TRUE;

        requestArray[i].buffer->buffer = NULL;
        requestArray[i].buffer->size = 0;

        tileType = GetTileType(instance->tileTypes, requestArray[i].information->tileTypeId);

        if (!tileType)
        {
            return NE_INVAL;
        }

        if (TileRequestOutstanding(instance, requestArray[i].information))
        {
            // Outstanding request already issued.
            needsDownload = FALSE;
        }
        else
        {
            if (tileType->configuration.isCacheable)
            {
                char* key = NULL;
                if ((NE_OK != GenerateTileKey(instance, requestArray[i].information, &key, NULL)) || (!key))
                {
                    return NE_NOMEM;
                }
                if (requestArray[i].command == NB_TC_Force_Download)
                {
                    // If the force-download flag is set then we remove the item (if present) from the cache. Don't check the
                    // result, in case the item is not in the cache.
                    CSL_CacheClearOne(instance->cache, (byte*)key, nsl_strlen(key));

                    LogCacheStatistics(instance);
                }
                else
                {
                    // Check first if the tile is in the cache. If it is in the cache then the entry will be locked and we
                    // return the buffer right away.
                    cacheResult = CSL_CacheFind(instance->cache,
                                                (byte*)key,
                                                nsl_strlen(key),
                                                &(requestArray[i].buffer->buffer),
                                                (size_t*)&(requestArray[i].buffer->size),
                                                TRUE,
                                                FALSE,
                                                (nb_boolean)(requestArray[i].command == NB_TC_MemoryCache));

                    LogCacheStatistics(instance);

                    // If the tile was in the cache or the command is not set to download then we're done
                    if ((cacheResult == NE_OK) || (requestArray[i].command != NB_TC_Download))
                    {
                        // Don't download the tile
                        needsDownload = FALSE;
                    }
                }
                if(key)
                {
                  nsl_free(key);
                }
            }
        } // if (TileRequestOutstanding(instance, information))

        if (needsDownload)
        {
            // Tile needs to be downloaded

            Connection* connectionArray = NULL;
            uint16 connectionCount = 0;
            int maximumPendingTiles = 0;
            struct CSL_Vector* pendingTileVector = NULL;

            TileWithPriority* pPendingTile = NULL;

            connectionArray         = tileType->connections;
            connectionCount         = tileType->configuration.maximumConcurrentRequests;
            pendingTileVector       = tileType->pendingTiles;
            maximumPendingTiles     = tileType->configuration.maximumPendingTiles;
            tileType->isShouldBeDownload  = TRUE;

            if (!connectionArray || !pendingTileVector)
            {
                return NE_INVAL;
            }

            /*
                Search pending tile vector for the requested tile. If tile is already pending,
                update the priority and don't add request to queue
            */
            if (FindPendingEntry(instance, requestArray[i].information, &pPendingTile, NULL))
            {
                // Update priority
                pPendingTile->priority = requestArray[i].priority;
            }
            else
            {
                if (tileType->allConnectionsBusy == FALSE)
                {
                    uint32 connectionIndex = 0;

                    // Assume everything is busy. If we find a non-busy connection then we reset the flag
                    tileType->allConnectionsBusy = TRUE;

                    // For all network connections
                    for (connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++)
                    {
                        Connection* connection = &(connectionArray[connectionIndex]);

                        /*
                            If this connection is currently disconnected then we initiate the connect and make
                            the HTTP request in the status callback once the connection is connected.
                        */
                        if (connection->state == CS_Disconnected)
                        {
                            // Open the connection. We make the HTTP request in the status callback once the connection
                            // is connected.
                            result = result ? result : OpenConnection(connection, requestArray[i].information);
                            tileType->allConnectionsBusy = FALSE;
                            break;
                        }
                        /*
                            Start the request, if number of on going requests with CS_Connected connection are less than
                            the allowed number of requests.
                        */
                        else if (connection->state == CS_Connected && connection->onGoingRequests < connection->allowedRequests)
                        {
                            // Start the http request. The results will be handled in the receive callback.
                            result = result ? result : StartRequest(connection, requestArray[i].information);
                            tileType->allConnectionsBusy = FALSE;
                            break;
                        }
                    }
                }

                if (tileType->allConnectionsBusy == TRUE)
                {
                    /* All connections are currently serving requests to maximum allowed limit.
                     Add to pending list. As soon as number of on going requests on a connection drop
                     to less than the maximum allowed requests, it will process the next tile request.
                     See receive callback. */

                    TileWithPriority tile = {0};
                    tile.priority = requestArray[i].priority;
                    nsl_memcpy(&tile.information, requestArray[i].information, sizeof(NB_TileInformation));

                    if (!CSL_VectorAppend(pendingTileVector, &tile))
                    {
                        return NE_NOMEM;
                    }
                }
            }
        }
    }

    length = CSL_VectorGetLength(instance->tileTypes);

    for (i = 0; i < length; ++i)
    {
        tileType = CSL_VectorGetPointer(instance->tileTypes, i);
        if (tileType)
        {
            // Resort any pending vector which needs resorting
            if (tileType->isShouldBeDownload == TRUE)
            {
                ResortAndLimitPendingVector(tileType->pendingTiles, tileType->configuration.maximumPendingTiles);
            }
            tileType->allConnectionsBusy = FALSE;
            tileType->isShouldBeDownload = FALSE;
        }
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerUnlockTile(NB_TileManager* instance,
                         const NB_TileInformation* information
                         )
{
    NB_TileTypeConfiguration* tileType = NULL;
    NB_Error err = NE_OK;

    tileType = GetTileType(instance->tileTypes, information->tileTypeId);

    if (tileType == NULL)
    {
        return NE_NOENT;
    }

    // Do not unlock tile if tile has any callbacks
    if (GetTileCallbacksCount(instance, information) > 1)
    {
        return NE_OK;
    }

    if (tileType->configuration.isCacheable)
    {
        char* key = NULL;

        if ((NE_OK != GenerateTileKey(instance, information, &key, NULL)) || (!key))
        {
            return NE_NOMEM;
        }

        // Unlock the tile from the cache
        err = CSL_CacheClearProtectOne(instance->cache, (byte*)key, nsl_strlen(key));

        nsl_free(key);

        return err;
    }

    if (instance->nonCachedTilesVector && !tileType->configuration.isCacheable)
    {
        uint32 i = 0;
        uint32 length = CSL_VectorGetLength(instance->nonCachedTilesVector);
        NB_Tile* tile = NULL;

        for (i = 0; i < length; i++)
        {
            tile = CSL_VectorGetPointer(instance->nonCachedTilesVector, i);
            if (tile != NULL && CompareTileInformation(information, &tile->information))
            {
                if (tile->buffer.buffer != NULL)
                {
                    nsl_free(tile->buffer.buffer);
                    tile->buffer.buffer = NULL;
                    tile->buffer.size = 0;
                }
                CSL_VectorRemove(instance->nonCachedTilesVector, i);
                break;
            }
        }
    }

    return NE_OK;
}


/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerFlushAllTiles(NB_TileManager* instance)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 length = 0;
    uint32 lengthPendingVector = 0;
    NB_TileTypeConfiguration* tileType = NULL;
    TileWithPriority* pendingTile = NULL;

    if (!instance || !instance->tileTypes)
    {
        return NE_INVAL;
    }

    length = CSL_VectorGetLength(instance->tileTypes);

    for (i = 0; i < length; i++)
    {
        tileType = CSL_VectorGetPointer(instance->tileTypes, i);

        if (tileType->pendingTiles)
        {
            lengthPendingVector = CSL_VectorGetLength(tileType->pendingTiles);

            for (j = 0; j < lengthPendingVector; j++)
            {
                pendingTile = CSL_VectorGetPointer(tileType->pendingTiles, j);
                RemoveAllCallbacksFromTile(instance, (const NB_TileInformation*)&pendingTile->information);
                if(pendingTile->information.additionalPartOfUrl)
                {
                    nsl_free(pendingTile->information.additionalPartOfUrl);
                    pendingTile->information.additionalPartOfUrl = NULL;
                }
            }
            CSL_VectorRemoveAll(tileType->pendingTiles);
        }
    }

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerFlushTile(NB_TileManager* instance,
                        const NB_TileInformation* information,
                        const NB_TileManagerCallback* callback
                        )
{
    int index = 0;
    TileWithPriority* pendingTile = NULL;
    if (FindPendingEntry(instance, information, &pendingTile, &index))
    {
        RemoveCallbackFromTile(instance, (const NB_TileInformation*)&pendingTile->information, callback);
        // Do not flush tile if tile has any callbacks
        if (GetTileCallbacksCount(instance, (const NB_TileInformation*)&pendingTile->information) == 0)
        {
            if(pendingTile->information.additionalPartOfUrl)
            {
                nsl_free(pendingTile->information.additionalPartOfUrl);
                pendingTile->information.additionalPartOfUrl = NULL;
            }
            // Remove from the pending list
            CSL_VectorRemove(GetPendingVector(instance, information->tileTypeId), index);
        }
        return NE_OK;
    }

    // Entry is not in the pending list
    return NE_INVAL;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerClearCache(NB_TileManager* instance, NB_TileClearCache options)
{
    CSL_ClearOption cslOptions = CSL_CF_All;
    switch (options)
    {
        case NB_TCC_All:           cslOptions = CSL_CF_All;                break;
        case NB_TCC_MemoryOnly:    cslOptions = CSL_CF_MemoryOnly;         break;
        case NB_TCC_UnlockedOnly:  cslOptions = CSL_CF_UnprotectedOnly;    break;
    }

    return CSL_CacheClearExtended(instance->cache, cslOptions);
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerSaveToPersistentCache(NB_TileManager* instance,
                                    int32 maximumEntries,
                                    int32* savedEntries
                                    )
{
    // Forward the call to the cache. Keep the tiles in the memory cache.
    NB_Error result = CSL_CacheSaveDirty(instance->cache, maximumEntries, savedEntries, FALSE);

    LogCacheStatistics(instance);

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerSavePersistentCacheIndex(NB_TileManager* instance)
{
    return CSL_CacheSaveIndex(instance->cache);
}


/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerGetDebugInformation(NB_TileManager* instance,
                                  int* memoryCacheCount,
                                  int* persistentCacheCount,
                                  int* protectedCount,
                                  int* pendingCount
                                  )
{
    GetCacheStatistics(instance, memoryCacheCount, persistentCacheCount, protectedCount, pendingCount);
    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerDoesTileTypeExist(NB_TileManager* instance,
                                NB_TileDataSourceTemplate* tileDataSourceTemplate,
                                uint32* tileTypeId
                                )
{
    NB_TileTypeConfiguration* tileType = NULL;
    NB_TileTypeConfiguration tempTileType = {0};

    uint32 length = 0;
    uint32 i = 0;
    NB_Error err = NE_OK;

    if (!instance || !instance->tileTypes || !tileTypeId)
    {
        return NE_INVAL;
    }

    *tileTypeId = (uint32)-1;

    err = InitTileType(instance, &tempTileType, tileDataSourceTemplate);
    if (err != NE_OK)
    {
        return err;
    }

    length = CSL_VectorGetLength(instance->tileTypes);

    for (i = 0; i < length; i++)
    {
        tileType = CSL_VectorGetPointer(instance->tileTypes, i);

        if (CompareTileDataSourceTemplate(&tempTileType.dataSource, &tileType->dataSource))
        {
            *tileTypeId = tileType->tileTypeId;
            break;
        }
    }

    CleanTileDataSourceTemplate(&tempTileType.dataSource);

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter( NB_TileManager* instance,
                            uint32 typeId,
                            nb_boolean* result )
{
    NB_TileTypeConfiguration* tileType = NULL;

    if ( !instance || !instance->tileTypes || !result )
    {
        return NE_INVAL;
    }

    *result = FALSE;
    tileType = GetTileType( instance->tileTypes, typeId );

    if ( tileType )
    {
        // Check if dataSource.templateUrl contains '$user'
        *result = NB_TileManagerDoesUrlHaveUserParameter(tileType->dataSource.templateUrl);
     }

    return NE_OK;
}


/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerDoesTileTypeUrlTemplateHaveTileSize( NB_TileManager* instance,
                            uint32 typeId,
                            nb_boolean* result )
{
    NB_TileTypeConfiguration* tileType = NULL;

    if ( !instance || !instance->tileTypes || !result )
    {
        return NE_INVAL;
    }

    *result = FALSE;
    tileType = GetTileType( instance->tileTypes, typeId );

    if ( tileType )
    {
        // Check if dataSource.templateUrl contains '$sz'
        *result = NB_TileManagerDoesUrlHaveTileSize(tileType->dataSource.templateUrl);
     }

    return NE_OK;
}

/* See header file for description. */
NB_DEF nb_boolean
NB_TileManagerDoesUrlHaveUserParameter( const char *URL )
{
    // Check if URL contains '$user'
    if ( URL && nsl_stristr( URL, URL_PARAMETER_USER ) != NULL )
    {
        return TRUE;
    }

    return FALSE;
}


/* See header file for description. */
NB_DEF nb_boolean
NB_TileManagerDoesUrlHaveTileSize( const char *URL )
{
    // Check if URL contains '$sz'
    if ( URL && nsl_stristr( URL, URL_PARAMETER_TILE_SIZE ) != NULL )
    {
        return TRUE;
    }

    return FALSE;
}

/* This is just temporary workaround
   Shall be reworked in the future*/
NB_DEF NB_Error
NB_TileManagerSetRoutes(NB_TileManager* instance,
                        NB_RouteId routeId1,
                        nb_color routeColor1,
                        NB_RouteId routeId2,
                        nb_color routeColor2
                        )
{
    int i = 0;
    int iTileTypesCount = 0;
    NB_Error err = NE_OK;
    NB_TileTypeConfiguration* pTileType = NULL;

    if ( !instance || !instance->tileTypes )
    {
        return NE_INVAL;
    }

    iTileTypesCount = CSL_VectorGetLength(instance->tileTypes);
    err = ( iTileTypesCount == 0 )? NE_INVAL : err;

    for ( i = 0; i < iTileTypesCount; i++ )
    {
        pTileType = CSL_VectorGetPointer(instance->tileTypes, i);

        if ( NULL != nsl_stristr(pTileType->dataSource.templateUrl, "route") )
        {
            err = err ? err : FormatSource(instance,
                                           pTileType,
                                           &routeId1,
                                           &routeColor1,
                                           &routeId2,
                                           &routeColor2);

            if (pTileType->additionalTileKey)
            {
                nsl_free(pTileType->additionalTileKey);
                pTileType->additionalTileKey = NULL;
            }

            err = err ? err : BuildRouteString(&pTileType->additionalTileKey,
                                               &routeId1,
                                               &routeColor1,
                                               &routeId2,
                                               &routeColor2);
        }
    }

    return err;
}

/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerGetTileKey(NB_TileManager* instance,
                         const NB_TileInformation* information,
                         char** key,
                         int* length)
{
    if (!instance || !information || !key || 0 == length)
    {
        return NE_INVAL;
    }

    return GenerateTileKey(instance, information, key, length);
}


/* See header file for description. */
NB_DEF NB_Error
NB_TileManagerClearTileFromCache(NB_TileManager* instance,
                         const NB_TileInformation* information)
{
    NB_Error err = NE_OK;
    char* key = NULL;

    if (!instance || !information)
    {
        return NE_INVAL;
    }

    err = GenerateTileKey(instance, information, &key, NULL);

    if (err == NE_OK && key != NULL)
    {
       err = CSL_CacheClearOne(instance->cache, (byte*)key, nsl_strlen(key));
       nsl_free(key);
    }

    return err;
}

/* See header file for description. */
NB_DEC NB_Error
NB_TileManagerFlushAllTilesByTileTypeId(NB_TileManager* instance,
                                        uint32 tileTypeId)
{
    NB_TileTypeConfiguration* tileType = NULL;
    uint32 i = 0;
    uint32 lengthPendingVector = 0;
    TileWithPriority* pendingTile = NULL;

    if (!instance || !instance->tileTypes)
    {
        return NE_INVAL;
    }

    tileType = GetTileType(instance->tileTypes, tileTypeId);
    if (tileType && tileType->pendingTiles)
    {
        lengthPendingVector = CSL_VectorGetLength(tileType->pendingTiles);

        for (i = 0; i < lengthPendingVector; ++i)
        {
            pendingTile = CSL_VectorGetPointer(tileType->pendingTiles, i);
            RemoveAllCallbacksFromTile(instance, (const NB_TileInformation*)&pendingTile->information);
            if(pendingTile->information.additionalPartOfUrl)
            {
                nsl_free(pendingTile->information.additionalPartOfUrl);
                pendingTile->information.additionalPartOfUrl = NULL;
            }
        }
        CSL_VectorRemoveAll(tileType->pendingTiles);
    }

    return NE_OK;
}

// Private functions .............................................................................

/*! Callback for network status.

    @return None
    @see NB_TileManagerGetTile
*/
void
ConnectionStatusCallback(void* userData,                   /*!< caller-supplied reference for this connection */
                         PAL_NetConnectionStatus status    /*!< current PAL network connection status */
                         )
{
    // The user data points to one of the entries in the connection array
    Connection* connection = (Connection*) userData;
    NB_TileManager* pThis = connection->manager;

    NB_TileTypeConfiguration* tileType = NULL;

    tileType = GetTileType(pThis->tileTypes, connection->typeId);

    if (!tileType)
    {
        return;
    }

    switch (status)
    {
        // The connection to the host has failed
        case PNCS_Failed:
            // Forward the network connection failure to the API users callback
            InvokeCallback(pThis, &tileType->callbackData, &connection->pendingConnectionTile.information, NE_NET, NULL);
            PendingConnectionTileClear(pThis, connection, TRUE);
            break;

        case PNCS_Error:
            // Connection is in error state, until it gets closed
            connection->state = CS_ConnectionError;
            // Close the connection.
            PAL_NetCloseConnection(connection->connection);
            PendingConnectionTileClear(pThis, connection, TRUE);
            break;

        case PNCS_Connected:
        {
            NB_Error result = NE_OK;

            // Update the connection state
            connection->state = CS_Connected;

            // Start the request for the tile which we specified when we opened the connection

            result = StartRequest(connection, &connection->pendingConnectionTile.information);

            if (result != NE_OK)
            {
                // Forward the failure to the API users callback.
                InvokeCallback(pThis, &tileType->callbackData, &connection->pendingConnectionTile.information, result, NULL);
            }

            PendingConnectionTileClear(pThis, connection, FALSE);

            // If there are any tiles in the pending list, and we still got additional slots  - fill them out
            while (ProcessNextPendingTile(connection));

            break;
        }

        case PNCS_Closed:
            if (connection->state == CS_ConnectionError)
            {
                // This connection is closed, because it got error.
                // There might be pending tiles we want to process. This connection can be reopenend.
                connection->state = CS_Disconnected;
                // Process next pending tile (if any)
                ProcessNextPendingTile(connection);
            }
            else
            {
                // No error on connection. It got closed on normal shutdown procedure.
                connection->state = CS_Disconnected;
            }
            PendingConnectionTileClear(pThis, connection, TRUE);
            break;

        case PNCS_ProxyAuthRequired:
        {
            // Set the state to CS_ProxyAuthRequired to block the requests.
            connection->state = CS_ProxyAuthRequired;
            break;
        }

        case PNCS_Created:
        case PNCS_Resolving:
        case PNCS_Connecting:
        case PNCS_Closing:
        case PNCS_Undefined:
        default:
            // Don't handle these
            break;
    }
}

/*! Starts tile request.

    @return TRUE if tile request is actually started, FALSE otherwise
*/
boolean
TileRequestBegin(Connection* pConnection, NB_TileRequestData* pTileRequest)
{
    uint32 i = 0;
    boolean result = FALSE;

    if (pConnection)
    {
        // Add to tileRequests
        for( i = 0; i < pConnection->allowedRequests; i++ )
        {
            if( pConnection->tileRequests[i] == NULL )
            {
                pConnection->tileRequests[i] = pTileRequest;
                result = TRUE;
                break;
            }
        }
    }

    if (!result)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "TileRequestBegin NO SLOT %d", (int)pTileRequest);
    }

    return result;
}

/*! Ends tile request.

    @return TRUE if tile request is actually ended, FALSE otherwise
*/
boolean
TileRequestEnd(Connection* pConnection, NB_TileRequestData* pTileRequest)
{
    uint32 i = 0;
    boolean ret = FALSE;

    if (pConnection)
    {
        // Clear from tileRequests
        for( i = 0; i < pConnection->allowedRequests; i++ )
        {
            if( pConnection->tileRequests[i] == pTileRequest )
            {
                nsl_free(pTileRequest);
                pConnection->tileRequests[i] = NULL;
                ret = TRUE;
                break;
            }
        }
    }

    if( !ret )
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "TileRequestEnd NOT FOUND %d", (int)pTileRequest);
    }

    return ret;
}

/*! Check whether tile request is outstanding.

    @return TRUE if tile request is outstanding, FALSE otherwise
*/
boolean
TileRequestOutstanding(NB_TileManager* pThis, const NB_TileInformation* pTileInfo)
{
    uint32 i = 0, j = 0;
    boolean ret = FALSE;
    NB_TileTypeConfiguration* tileType = NULL;

    tileType = GetTileType(pThis->tileTypes, pTileInfo->tileTypeId);

    if (tileType)
    {
        for (i = 0; i < tileType->configuration.maximumConcurrentRequests; i++)
        {
            for (j = 0; j < tileType->connections[i].allowedRequests; j++)
            {
                // Compare the tile information
                if ((tileType->connections[i].tileRequests[j]) &&
                     CompareTileInformation(&(tileType->connections[i].tileRequests[j]->tile.information), pTileInfo))
                {
                    ret = TRUE;
                    break;
                }
            }
        }
    }

    return ret;
}

/*! Check whether tile request is pending in any connection.

    @return TRUE if tile request is outstanding, FALSE otherwise
*/
boolean
TileConnectionPending(NB_TileManager* pThis, const NB_TileInformation* pTileInfo)
{
    uint32 i = 0;
    boolean ret = FALSE;
    NB_TileTypeConfiguration* tileType = NULL;

    tileType = GetTileType(pThis->tileTypes, pTileInfo->tileTypeId);

    if (tileType)
    {
        for (i = 0; i < tileType->configuration.maximumConcurrentRequests; i++)
        {
            Connection* connection = &(tileType->connections[i]);
            // Compare the tile information
            if (CompareTileInformation(&(connection->pendingConnectionTile.information), pTileInfo))
            {
                ret = TRUE;
                break;
            }
        }
    }

    return ret;
}


/*! Callback for receiving data from the network.

    @return None
    @see NB_TileManagerGetTile
*/
void
DataReceivedCallback(void* userData,             /*!< caller-supplied reference for this connection */
                     void* requestData,          /*!< request reference passed during call to PAL_NetHttpSend  */
                     PAL_Error errorCode,        /*!< PAL error code */
                     const byte* bytes,          /*!< bytes received */
                     uint32 count                /*!< count of bytes received */
                     )
{
    // The user data points to one of the entries in the connection array
    Connection*           connection = (Connection*) userData;
    NB_TileRequestData*   pTileRequest = (NB_TileRequestData*)requestData;
    NB_TileManager* pThis = connection->manager;
    NB_TileBuffer*  pBuffer = &(pTileRequest->tile.buffer);

    // Do we have any data
    if (errorCode == PAL_Ok && bytes && (count > 0))
    {
        // If we had a previous allocation error then we return. It will be handled once the download is complete.
        if (pTileRequest->allocationError)
        {
            return;
        }

        // Is it the first chunk of data
        if ((pBuffer->buffer == NULL) && (pBuffer->size == 0))
        {
            // Save data to newly allocated block
            pBuffer->buffer = nsl_malloc(count);
            if (!pBuffer->buffer)
            {
                pTileRequest->allocationError = TRUE;
                return;
            }

            pBuffer->size = count;
            nsl_memcpy(pBuffer->buffer, bytes, count);
        }
        // This is an additional chunk of data
        else
        {
            // Reallocate buffer
            void* tmp = nsl_realloc(pBuffer->buffer, pBuffer->size + count);

            // When the "nsl_realloc" fails the "pBuffer" is unchanged.
            // In this case, we have to release the allocated memory.
            if (!tmp)
            {
                FreeBuffer(pBuffer);
                pTileRequest->allocationError = TRUE;
                return;
            }
            else
            {
                pBuffer->buffer = tmp;
            }

            // Copy new data to end of buffer
            nsl_memcpy(pBuffer->buffer + pBuffer->size, bytes, count);
            pBuffer->size += count;
        }
    }
    // This must be the end of the receiving data
    else
    {
        // We need to make a copy of the tile since TileRequestEnd() frees the tile which we then need to pass into the callback.
        NB_Tile tempTile = {0};

        nsl_memcpy(&tempTile, &pTileRequest->tile, sizeof(NB_Tile));

        // Reallocation error OR Network error for this specific request
        if (pTileRequest->allocationError || errorCode != PAL_Ok)
        {
            // Reset buffer
            NB_Error result = NE_NOMEM;
            NB_TileTypeConfiguration* tileType = NULL;
            tileType = GetTileType(pThis->tileTypes, tempTile.information.tileTypeId);

            if (!pTileRequest->allocationError)
            {
                // Return NE_OK for no content
                result = (errorCode == PAL_ErrHttpNoContent) ? NE_OK : NE_NET;
            }
            FreeBuffer(pBuffer);

            if (!tileType)
            {
                return;
            }

            // Log the failed tile
            NB_QaLogRasterTileReply(pThis->context,
                                    GetQaLogMapType(tempTile.information.tileTypeId),
                                    tempTile.information.xPosition,
                                    tempTile.information.yPosition,
                                    tempTile.information.zoomLevel,
                                    0,
                                    tileType->dataSource.tileSize,
                                    tempTile.buffer.size,
                                    result);


            /* Tile request life cycle complete. Free it */
            TileRequestEnd(connection, pTileRequest);
            connection->onGoingRequests--;

            /*
                :TRICKY:
                Report an error to the user.
                This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
                The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
                the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
                so that we can re-request the tile right away.
            */

            InvokeCallback(pThis, &tileType->callbackData, &tempTile.information, result, &tempTile);
        }
        // We successfully downloaded the tile.
        else
        {
            NB_Error result = NE_OK;
            NB_TileTypeConfiguration* tileType = NULL;
            connection->state = CS_Connected;

            // Add the entry to cache
            tileType = GetTileType(pThis->tileTypes, tempTile.information.tileTypeId);

            if (!tileType)
            {
                return;
            }

            if (tileType->configuration.isCacheable == TRUE && !((pBuffer->buffer == NULL) && (pBuffer->size == 0)))
            {
                char* key = NULL;
                if ((NE_OK != GenerateTileKey(pThis, &tempTile.information, &key, NULL)) || (!key))
                {
                    return;
                }
                // Add the entry to the cache and protect/lock it. This will fail if too many entries are protected/locked.
                result = CSL_CacheAdd(pThis->cache, (byte*)key, nsl_strlen(key), pBuffer->buffer, pBuffer->size, TRUE, FALSE, tileType->configuration.expirationTime);

                nsl_free(key);

                if (result == NE_OK)
                {
                    // If the element was added successfully then the cache takes ownership of the tile data. We set the
                    // buffer to NULL in order to NOT free it below. We have to free the data in all other error cases.
                    pBuffer->buffer = NULL;
                    pBuffer->size = 0;
                }
            }
            else if (pBuffer->buffer != NULL && pBuffer->size > 0)
            {
                //this condition will reached if tile succesfully recieved, but not cacheable
                //so we will placed it in internal vector

                if (CSL_VectorAppend(pThis->nonCachedTilesVector, &pTileRequest->tile))
                {
                    //Now tile vector became owner of buffer, so we shouldn't free it below
                    pBuffer->buffer = NULL;
                    pBuffer->size = 0;
                }
                else
                {
                    result = NE_INVAL;
                }
            }
            else
            {
                //If we are here, result is PAL_OK, but buffer is empty
                //it is error
                //result should be NE_INVAL
                result = NE_INVAL;
            }

            // Log the received tile
            NB_QaLogRasterTileReply(pThis->context,
                                    GetQaLogMapType(tempTile.information.tileTypeId),
                                    tempTile.information.xPosition,
                                    tempTile.information.yPosition,
                                    tempTile.information.zoomLevel,
                                    0,
                                    tileType->dataSource.tileSize,
                                    tempTile.buffer.size,
                                    result);
            if (result == NE_OK)
            {
                // Indicate success for this tile.
                if (pTileRequest->contentLength == (uint32)(-1) || pTileRequest->contentLength == tempTile.buffer.size)
                {
                    result = NE_OK;
                }
                else
                {
                    // This is used for NBI because it uses an external cache. I still think this is bad code here since
                    // it would not return an error for code which uses the CCC cache.
                    result = NE_AGAIN;
                }
            }
            else
            {
                // Indicate that the cache is full and the tile was discarded.
                result = (pBuffer->size == 0) ? NE_INVAL : NE_RES;
            }

            // Free the buffer if an error occurred (the pointer was cleared if we were successful)
            FreeBuffer(pBuffer);

            /* Tile request life cycle complete. Free it */
            TileRequestEnd(connection, pTileRequest);
            connection->onGoingRequests--;

            /*
             :TRICKY:
             Report an error to the user.
             This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
             The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
             the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
             so that we can re-request the tile right away.
             */
            InvokeCallback(pThis, &tileType->callbackData, &tempTile.information, result, &tempTile);
            LogCacheStatistics(pThis);
        }

        // Process next pending tile (if any)
        ProcessNextPendingTile(connection);
    }
}

/*! Callback for response from the network.

    @return None
    @see NB_TileManagerGetTile
*/
static void
ResponseStatusCallback(PAL_Error errorCode,
                       void* userData,
                       void* requestData,
                       uint32  contentLength)
{
    Connection*           connection = (Connection*) userData;
    NB_TileRequestData*   pTileRequest = (NB_TileRequestData*)requestData;
    NB_TileManager* pThis = NULL;
    NB_TileBuffer*  pBuffer = NULL;

    if (!connection || !pTileRequest)
    {
        // Should never happen
        nsl_assert(FALSE);
        return;
    }

    pThis = connection->manager;
    pBuffer = &(pTileRequest->tile.buffer);

    if (!pThis || !pBuffer)
    {
        return;
    }

    if (errorCode == PAL_Ok)
    {
        // Capture contentLength
        pTileRequest->contentLength = contentLength;
    }
    // Don't check for particular error codes, we want to handle any error!
    else
    {
        // Return NE_OK for no content
        NB_Error result = (errorCode == PAL_ErrHttpNoContent) ? NE_OK : NE_NETSVC;

        /*
            :WARNING:
            In order for the code below to work, the PAL http connection code has to ensure that DataReceivedCallback()
            does NOT get called after ResponseStatusCallback() is called with an error. The code below frees the
            TileRequest structure and it will crash if DataReceivedCallback() gets called after this!

            We could add some special handling here to handle those cases but this would not be trivial at all. When would
            you throw out the TileRequest?
        */

        // We need to make a copy of the tile since TileRequestEnd() frees the tile which we then need to pass into the callback.
        NB_Tile tempTile = {0};
        NB_TileTypeConfiguration* tileType = NULL;

        nsl_memcpy(&tempTile, &pTileRequest->tile, sizeof(NB_Tile));

        /* Tile request life cycle complete. Free it */
        FreeBuffer(pBuffer);

        tileType = GetTileType(pThis->tileTypes, tempTile.information.tileTypeId);

        if (!tileType)
        {
            return;
        }

        // Log the received tile
        NB_QaLogRasterTileReply(pThis->context,
                                GetQaLogMapType(tempTile.information.tileTypeId),
                                tempTile.information.xPosition,
                                tempTile.information.yPosition,
                                tempTile.information.zoomLevel,
                                0,
                                tileType->dataSource.tileSize,
                                tempTile.buffer.size,
                                result);

        TileRequestEnd(connection, pTileRequest);
        connection->onGoingRequests--;

        /*
            :TRICKY:
            Report an error to the user.
            This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
            The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
            the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
            so that we can re-request the tile right away.

        */

        InvokeCallback(pThis, &tileType->callbackData, &tempTile.information, result, &tempTile);
        // Process next pending tile (if any)
        ProcessNextPendingTile(connection);
    }
}


/*! Open a HTTP connection.

    The call is asynchronous and the status callback gets called once the connection was successfully established.
    Once the connection is established it will make a http request with the specified tile information.

    @return NB_Error

    @see NB_TileManagerGetTile
    @see ConnectionStatusCallback
*/
NB_Error
OpenConnection(Connection* connection,                          /*!< Connection to open */
               const NB_TileInformation* information      /*!< Tile information to save in the connection object. This
                                                                     information will be used to make the send request once the
                                                                     connection is successfully established. */
               )
{
    PAL_Error palResult = PAL_Failed;
    NB_TileManager* manager = connection->manager;
    PAL_NetConnectionConfig networkConfiguration = {0};
    NB_TileTypeConfiguration* tileType = NULL;

    // This function should only be called in a disconnected state
    if (connection->state != CS_Disconnected)
    {
        return NE_UNEXPECTED;
    }

    networkConfiguration.netStatusCallback = ConnectionStatusCallback;
    networkConfiguration.netDataSentCallback = NULL;                            // Not needed
    networkConfiguration.netHttpDataReceivedCallback = DataReceivedCallback;
    networkConfiguration.netHttpResponseStatusCallback = ResponseStatusCallback;

    // Reset tile for this connection
    connection->pendingConnectionTile.information = *information;

    // Set the user data to this connection structure
    networkConfiguration.userData = connection;

    // Open the HTTP connection. The connection status gets updated in the status callback function.
    tileType = GetTileType(manager->tileTypes, connection->pendingConnectionTile.information.tileTypeId);
    if (tileType)
    {
        palResult = PAL_NetOpenConnection(connection->connection,
                                          &networkConfiguration,
                                          tileType->dataSource.baseUrl,
                                          tileType->port);
        if (palResult != PAL_Ok)
        {
            return NE_NET;
        }
        connection->state = CS_ConnectionPending;
    }
    else
    {
        return NE_INVAL;
    }

    return NE_OK;
}

/*! Make a new HTTP request to get a tile.

    The receive callback gets called once the data is received. Status callback gets called if an error occurrs.

    @return NB_Error

    @see NB_TileManagerGetTile
    @see ProcessNextPendingTile
    @see DataReceivedCallback
*/
NB_Error
StartRequest(Connection* connection,                       /*!< Connection to use for the request */
             const NB_TileInformation* information
             )
{
    PAL_Error palResult                         = PAL_Ok;
    NB_TileManager* manager                     = connection->manager;
    char* httpRequest                           = NULL;
    char* templateString                        = NULL;
    NB_TileRequestData* pTileRequest            = NULL;
    NB_TileTypeConfiguration* tileType          = NULL;
    int httpRequestLength                       = 0;
    NB_Error err                                = NE_OK;

    // This function should only be called in a idle state
    if (connection->state != CS_Connected)
    {
        return NE_UNEXPECTED;
    }

    // Create the request tile
    pTileRequest = nsl_malloc(sizeof(*pTileRequest));

    if (!pTileRequest)
    {
        return NE_NOMEM;
    }

    // Reset tile and error for this connection
    nsl_memset(pTileRequest, 0, sizeof(*pTileRequest));
    pTileRequest->allocationError = FALSE;
    pTileRequest->contentLength = (uint32)-1;

    // Set the information to the requested tile information
    pTileRequest->tile.information = *information;

    // Choose the correct template string.
    tileType = GetTileType(manager->tileTypes, information->tileTypeId);

    if (!tileType)
    {
        err = NE_INVAL;
        goto cleanup;
    }

    templateString = tileType->dataSource.templateUrl;

    if (tileType->useQuadKey)
    {
        char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};

        err = CSL_QuadkeyConvertFromXYZ(information->xPosition,
                                        information->yPosition,
                                        (uint8)(information->zoomLevel),
                                        quadkey);
        if (err != NE_OK)
        {
            goto cleanup;
        }
        httpRequestLength = nsl_strlen(templateString) +
                            nsl_strlen(quadkey) + 
                            ((information->additionalPartOfUrl)? nsl_strlen(information->additionalPartOfUrl) : 0) +
                            1;
        httpRequest = (char*)nsl_malloc(httpRequestLength);
        if(!httpRequest)
        {
            err = NE_NOMEM;
            goto cleanup;
        }
        // Format the http request using the quadkey
        nsl_sprintf(httpRequest, templateString, quadkey);
    }
    else
    {
        // Format the http request based on the requested tile information. The order of x/y/z is currently
        // assumed in this order. If it is not then we need to change the logic in FormatSource().
        httpRequestLength = nsl_strlen(templateString) +
                            XYZ_LENGTH +
                            ((information->additionalPartOfUrl)? nsl_strlen(information->additionalPartOfUrl) : 0) +
                            1;

        httpRequest = (char*)nsl_malloc(httpRequestLength);
        if(!httpRequest)
        {
            err = NE_NOMEM;
            goto cleanup;
        }
        nsl_sprintf(httpRequest,
                    templateString,
                    information->xPosition,
                    information->yPosition,
                    information->zoomLevel);
    }

    templateString = NULL;
    if ( NE_OK == strReplace( &templateString,
                    httpRequest,
                    URL_PARAMETER_USER,
                    TEMPLATE_PRINT_STRING) &&
         templateString &&
         information->additionalPartOfUrl)
    {
        nsl_sprintf( httpRequest, templateString, information->additionalPartOfUrl );
    }
    if (templateString)
    {
        nsl_free( templateString );
        templateString = NULL;
    }

    // Log the request
    NB_QaLogRasterTileRequest(manager->context,
                              GetQaLogMapType(information->tileTypeId),
                              information->xPosition,
                              information->yPosition,
                              information->zoomLevel,
                              0,
                              tileType->dataSource.tileSize,
                              httpRequest);

    if (TileRequestBegin(connection, pTileRequest))
    {
        // Make the asynchronous HTTP request. See receive callback for processing the result
        palResult = PAL_NetHttpSend(connection->connection, NULL, 0, HTTP_VERB, httpRequest, NULL, NULL, pTileRequest);
        if (palResult != PAL_Ok)
        {
            TileRequestEnd(connection, pTileRequest);
            err = NE_NET;
        }
        else
        {
            connection->onGoingRequests++;
        }
        pTileRequest = NULL;
    }
    else
    {
        err = NE_NOMEM;
    }

cleanup:
    if (httpRequest)
    {
        nsl_free(httpRequest);
    }
    if(pTileRequest)
    {
        nsl_free(pTileRequest);
    }
    return err;
}

/*! Process next tile from pending list (if any).

    This function checks if there are any pending tiles. If so it uses the given connection and
    either (re)connects the connection or starts the request immediately.

    This function should only be called if the connection is either disconnected or idle.

    This function gets called in the context of a callback. It therefore reports any failures
    back to the user by a callback.

    @return True if at least one pending tile was processed
*/
boolean
ProcessNextPendingTile(Connection* connection          /*!< Network connection to use for the request */
                       )
{
    NB_TileManager* pThis = connection->manager;
    struct CSL_Vector* pendingTilesVector = NULL;
    boolean retStatus = FALSE;

    // This function should only be called when onGoingRequests are within limit of allowedRequests.
    if (!(connection->onGoingRequests < connection->allowedRequests))
    {
        return retStatus;
    }

    // Get correct pending tile vector based on connection type
    pendingTilesVector = GetPendingVector(pThis, connection->typeId);
    if (!pendingTilesVector)
    {
        return retStatus;
    }

    // If there are any pending tiles, initiate download for the next tile.
    if (CSL_VectorGetLength(pendingTilesVector) > 0)
    {
        // Get the first item from the pending vector
        NB_Error result = NE_OK;
        TileWithPriority tile = {0};

        CSL_VectorGet(pendingTilesVector, 0, (void*)&tile);
        CSL_VectorRemove(pendingTilesVector, 0);

        // Check current connection state

        if (connection->state == CS_Disconnected)
        {
            // Open the connection. We make the HTTP request in the status callback once the connection
            // is connected.

            result = OpenConnection(connection, &tile.information);
        }
        else
        {
            // Start the http request. The results will be handled in the receive callback.
            result = StartRequest(connection, &tile.information);
        }

        if (result != NE_OK)
        {
            NB_Tile temporaryTile = {{0}};
            NB_TileTypeConfiguration* tileType = NULL;

            nsl_memcpy(&temporaryTile.information, &tile.information, sizeof(NB_TileInformation));

            tileType = GetTileType(pThis->tileTypes, temporaryTile.information.tileTypeId);
            if (!tileType)
            {
                return NE_INVAL;
            }

            InvokeCallback(pThis, &tileType->callbackData, &temporaryTile.information, result, &temporaryTile);
        }
        else
        {
            retStatus = TRUE;
        }
    }

    return retStatus;
}

/*! Resort the given pending tile and make sure it doesn't exceed the given limit.

    @return None.
*/
void
ResortAndLimitPendingVector(struct CSL_Vector* vector,      /*!< Pending tile vector to resort */
                            int limit                       /*!< Maximum allowed number in given pending tile vector */
                            )
{
    // Resort the vector based on priority. If we would have a binary tree then we would not have to do this.
    CSL_VectorSort(vector, &PriorityVectorCompare);

    // Check if we exceed the maximum number of pending tiles and if so discard the tiles with the lowest priority.
    if (CSL_VectorGetLength(vector) > limit)
    {
        CSL_VectorSetSize(vector, limit);
    }
}

/*! Find an entry in the pending tile vector.

    The function returns a pointer to the actual entry which can be modified directly. It also
    returns the index of the found entry (if any).

    @return 'TRUE' if entry was found, 'FALSE' otherwise
*/
boolean
FindPendingEntry(NB_TileManager* instance,                /*!< Instance */
                 const NB_TileInformation* information,   /*!< Entry to search for */
                 TileWithPriority** pEntry,                     /*!< On return the pointer to the entry. Set to NULL if not needed. */
                 int* pIndex                                    /*!< On return the index of the entry. Set to NULL if not needed */
                 )
{
    int i = 0;
    int length = 0;
    TileWithPriority* pPendingTile = NULL;

    struct CSL_Vector* pendingTileVector = GetPendingVector(instance, information->tileTypeId);
    if (!pendingTileVector)
    {
        return FALSE;
    }

    // For all pending tiles
    length = CSL_VectorGetLength(pendingTileVector);
    for (i = 0; i < length; i++)
    {
        // Does the tile match
        pPendingTile = (TileWithPriority*)CSL_VectorGetPointer(pendingTileVector, i);
        if (CompareTileInformation(&pPendingTile->information, information))
        {
            // Return pointer and index to existing entry
            if (pEntry)
            {
                *pEntry = pPendingTile;
            }
            if (pIndex)
            {
                *pIndex = i;
            }
            return TRUE;
        }
    }

    return FALSE;
}

/*! Get the pending vector based on the tile type.

    @return Pointer to pending list.
*/
struct CSL_Vector*
GetPendingVector(NB_TileManager* instance, uint32 typeId)
{
    NB_TileTypeConfiguration* tileType = NULL;

    tileType = GetTileType(instance->tileTypes, typeId);

    if(tileType)
    {
        return tileType->pendingTiles;
    }

    return NULL;
}

/*! Reset all pending tile data from connection

    @return allocated vector
*/
static void PendingConnectionTileClear(NB_TileManager* instance, Connection *connection, nb_boolean fullClear)
{
    if (fullClear)
    {
        RemoveAllCallbacksFromTile(instance, (const NB_TileInformation *)&(connection->pendingConnectionTile.information));
        if(connection->pendingConnectionTile.information.additionalPartOfUrl)
        {
            nsl_free(connection->pendingConnectionTile.information.additionalPartOfUrl);
        }
    }
    connection->pendingConnectionTile.information.additionalPartOfUrl = NULL;
    if(connection->pendingConnectionTile.buffer.buffer)
    {
        nsl_free(connection->pendingConnectionTile.buffer.buffer);
        connection->pendingConnectionTile.buffer.buffer = NULL;
    }
    connection->pendingConnectionTile.buffer.size = 0;

    connection->pendingConnectionTile.information.tileTypeId = (uint32)-1;
    connection->pendingConnectionTile.information.xPosition = (uint32)-1;
    connection->pendingConnectionTile.information.yPosition= (uint32)-1;
    connection->pendingConnectionTile.information.zoomLevel= (uint32)-1;
}

/*! Compare function for sorting of pending tile vector.

    @return Negative value if item1 is smaller than item2. Zero if they are identical. Positive otherwise.
*/
int
PriorityVectorCompare(const void* item1,    /*!< Left item to compare */
                      const void* item2     /*!< Right item to compare */
                      )
{
    const TileWithPriority* tile1 = (TileWithPriority*)item1;
    const TileWithPriority* tile2 = (TileWithPriority*)item2;

    if (tile1->priority < tile2->priority)
    {
        return -1;
    }

    if (tile1->priority == tile2->priority)
    {
        return 0;
    }

    return 1;
}

/*! Generate a hash string used as key to the cache entry.

    @return None
*/
NB_Error
GenerateTileKey(NB_TileManager* instance,                      /*!<  tile manager instance */
                const NB_TileInformation* information,         /*!< Tile information for which to generate the key */
                char** key,                                    /*!< Pointer to a buffer to receive the key. */
                int* length)
{
    NB_TileTypeConfiguration* tileType = NULL;
    char* str_template = NULL;
    NB_Error result = NE_OK;
    int keyLength = 0;
    char *localKey = NULL;

    if (!instance || !information || !key)
    {
        return NE_INVAL;
    }

    tileType = GetTileType(instance->tileTypes, information->tileTypeId);

    if (!tileType)
    {
        return NE_INVAL;
    }

    keyLength += (tileType->dataSource.generationInfo) ? nsl_strlen(tileType->dataSource.generationInfo) : 0;
    keyLength += (tileType->additionalTileKey)         ? nsl_strlen(tileType->additionalTileKey)         : 0;
    keyLength += (information->additionalPartOfUrl)    ? nsl_strlen(information->additionalPartOfUrl)    : 0;

    // Format key based on tile information
    if (tileType->useQuadKey)
    {
        char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};

        result = CSL_QuadkeyConvertFromXYZ(information->xPosition,
                                                    information->yPosition,
                                                    (uint8)(information->zoomLevel),
                                                    quadkey);
        if ( result != NE_OK )
        {
            goto cleanup;
        }
        if (NE_OK == strReplace(&str_template,
                                tileType->dataSource.tileStoreTemplate,
                                TILE_CACHE_PARAMETER_Q,
                                TEMPLATE_PRINT_STRING) &&
            str_template)
        {
            keyLength += nsl_strlen(str_template) + nsl_strlen(quadkey) + 1;
            localKey = nsl_malloc(keyLength);
            if(!localKey)
            {
                result = NE_NOMEM;
                goto cleanup;
            }
            nsl_sprintf(localKey, str_template, quadkey);
        }
        else
        {
            result = NE_NOMEM;
            goto cleanup;
        }
    }
    else
    {
        result = strReplace( &str_template,
                    tileType->dataSource.tileStoreTemplate,
                    TILE_CACHE_PARAMETER_X,
                    TEMPLATE_PRINT_INT);
        result = result? result : strReplace( &str_template,
                    str_template,
                    TILE_CACHE_PARAMETER_Y,
                    TEMPLATE_PRINT_INT);
        result = result? result : strReplace( &str_template,
                    str_template,
                    TILE_CACHE_PARAMETER_Z,
                    TEMPLATE_PRINT_INT);
        if ( result == NE_OK )
        {
            keyLength += nsl_strlen(str_template) + XYZ_LENGTH + 1;
            localKey = (char*)nsl_malloc(keyLength);
            if(!localKey)
            {
                result = NE_NOMEM;
                goto cleanup;
            }
            nsl_sprintf(localKey,
                        str_template,
                        information->zoomLevel,
                        information->xPosition,
                        information->yPosition);
        }
        else
        {
            goto cleanup;
        }
    }


    if(tileType->dataSource.generationInfo)
    {
        nsl_strlcat(localKey, tileType->dataSource.generationInfo, keyLength);
    }
    if(information->additionalPartOfUrl)
    {
        nsl_strlcat(localKey, information->additionalPartOfUrl, keyLength);
    }
    if(tileType->additionalTileKey)
    {
        nsl_strlcat(localKey, tileType->additionalTileKey, keyLength);
    }

    *key = nsl_strdup(localKey);

    if (length)
    {
        *length = nsl_strlen(*key);
    }

cleanup:
    if (str_template)
    {
        nsl_free( str_template );
    }
    if (localKey)
    {
        nsl_free(localKey);
    }
    return result;
}

/*! Initialize connections for a specific type.

    Create the array for the connections and open all the connections for this type

    @return TRUE on success, FALSE otherwise

    @see NB_TileManagerCreate
*/
NB_Error
InitializeConnections(NB_TileManager* pThis,      /*!<  tile manager instance */
                      uint32 connectionCount,     /*!< Number of connections to create */
                      Connection** connections,   /*!< Pointer to connections array to initialize */
                      uint32 typeId,              /*!< Tile type which is used for this connection */
                      PAL_NetConnectionProtocol connectionProtocol /*!< Connection protocol type */
)
{
    PAL_Error palResult = PAL_Ok;
    uint32 i = 0;
    uint32 size = sizeof(Connection) * connectionCount;
    if (connectionCount == 0)
    {
        // The traffic and route connection array is optional and zero is allowed.
        return NE_OK;
    }

    // Allocate array for connections array
    *connections = nsl_malloc(size);
    if (!*connections)
    {
        return NE_NOMEM;
    }
    nsl_memset(*connections, 0, size);

    // Create all network connections
    for (i = 0; i < connectionCount; i++)
    {
        palResult = PAL_NetCreateConnection(NB_ContextGetPal(pThis->context), connectionProtocol, &((*connections)[i].connection));
        if (palResult != PAL_Ok)
        {
            return NE_NET;
        }

        // Set the pointer in the connection structure to the parent structure so we can access it in the
        // callback functions.
        (*connections)[i].manager = pThis;

        // Set tile type
        (*connections)[i].typeId = typeId;

        // Set on going requests to 0
        (*connections)[i].onGoingRequests = 0;

        // Set number of allowed simultaneous requests on a connection
        (*connections)[i].allowedRequests = MAXIMUM_REQUESTS_PER_CONNECTION;

        (*connections)[i].tileRequests = nsl_malloc(sizeof(NB_TileRequestData*) * (*connections)[i].allowedRequests );
        nsl_memset((*connections)[i].tileRequests, 0, sizeof(NB_TileRequestData*) * (*connections)[i].allowedRequests);
    }

    return NE_OK;
}

/*! Disconnect any open connections and delete the connections array.

    @return None

    @see NB_TileManagerDestroy
*/
void
CleanupConnections(NB_TileManager* pThis,         /*!<  tile manager instance */
                   uint32 connectionCount,              /*!< Number of connections in connections array */
                   Connection* connections              /*!< Pointer to connections array to cleanup */
                   )
{
    uint32 i = 0;
    for (i = 0; i < connectionCount; i++)
    {
        if (connections[i].connection)
        {
            PAL_NetDestroyConnection(connections[i].connection);
        }

        if (connections[i].tileRequests)
        {
            uint32 j = 0;

            for(j = 0; j < connections[i].allowedRequests; j++)
            {
                if (NULL != connections[i].tileRequests[j])
                {
                    RemoveAllCallbacksFromTile(pThis, (const NB_TileInformation *)&(connections[i].tileRequests[j]->tile.information));
                    if(connections[i].tileRequests[j]->tile.information .additionalPartOfUrl)
                    {
                        nsl_free(connections[i].tileRequests[j]->tile.information.additionalPartOfUrl);
                        connections[i].tileRequests[j]->tile.information.additionalPartOfUrl = NULL;
                    }
                    FreeBuffer(&(connections[i].tileRequests[j]->tile.buffer));
                    nsl_free(connections[i].tileRequests[j]);
                }
            }

            nsl_free(connections[i].tileRequests);
            connections[i].tileRequests = NULL;

            PendingConnectionTileClear(pThis, &connections[i], TRUE);
        }
    }

    if (connections)
    {
        nsl_free(connections);
    }
}

/*! Check NB_TileDataSourceTemplate URLs to determine connection protocol (http/https)

    @return NE_OK on success
*/
NB_Error
CheckConnectionProtocol(NB_TileDataSourceTemplate *dataSource, PAL_NetConnectionProtocol *connectionProtocol)
{
    NB_Error retval = NE_INVAL;

    if (!dataSource || !connectionProtocol)
    {
        return retval;
    }

    // At first we check baseURL
    if (dataSource->baseUrl && nsl_strlen(dataSource->baseUrl) > 0)
    {
        if (nsl_stristr(dataSource->baseUrl, BASE_HTTPS_URL_REMOVE))
        {
            *connectionProtocol = PNCP_HTTPS;
            retval = NE_OK;
        }
        else if (nsl_stristr(dataSource->baseUrl, BASE_HTTP_URL_REMOVE))
        {
            *connectionProtocol = PNCP_HTTP;
            retval = NE_OK;
        }
    }
    // If baseURL is empty, we will check templateURL
    else if (dataSource->templateUrl && nsl_strlen(dataSource->templateUrl) > 0)
    {
        if (nsl_stristr(dataSource->templateUrl, BASE_HTTPS_URL_REMOVE))
        {
            *connectionProtocol = PNCP_HTTPS;
            retval = NE_OK;
        }
        else if (nsl_stristr(dataSource->templateUrl, BASE_HTTP_URL_REMOVE))
        {
            *connectionProtocol = PNCP_HTTP;
            retval = NE_OK;
        }
    }

    return retval;
}

/*! Format the data source URL so that it can be used by the HTTP connections.

    @return NE_OK on success
*/
NB_Error
FormatSource(NB_TileManager* pThis,
             NB_TileTypeConfiguration* tileType,    /*!< Tile Type */
             NB_RouteId* routeId1,
             nb_color* routeColor1,
             NB_RouteId* routeId2,
             nb_color* routeColor2
             )
{
    NB_Error err = NE_OK;
    // We need to replace all placeholders with appropriate values or '%d' so that we can format it later easily.
    //template "8080/route?loc=en-US&v=1&fmt=$fmt&res=192&x=$x&y=$y&z=$z&sz=128&rw=4"

    char* lastPosition = tileType->dataSource.templateUrl;
    char* currentPosition = NULL;

    // Temporary string for result
    char* resultString = NULL;
    char* tmp = NULL;
    int resultStringLength = 0;

    int placeholderStrLength = MAX((tileType->dataSource.generationInfo) ? (nsl_strlen(tileType->dataSource.generationInfo)):0,
                                    DEFAULT_PLACEHOLDER_MAX_LENGTH);
    char* placeholderString = (char*)nsl_malloc(placeholderStrLength + 1);

    // Cannot be longer then templateUrl
    char* iterationString = (char*)nsl_malloc(nsl_strlen(tileType->dataSource.templateUrl) + 1);

    nb_boolean initMemory = FALSE;
    int bytesCount = 0;

    if (!placeholderString || !iterationString)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    placeholderString[0] = '\0';
    iterationString[0] = '\0';
    // Search for all placeholder strings
    currentPosition = nsl_strchr(tileType->dataSource.templateUrl, NB_TDS_PLACEHOLDER_PREFIX);
    while (currentPosition)
    {
        nsl_strlcpy(iterationString, lastPosition, currentPosition - lastPosition + 1);
        // Don't compary prefix
        ++currentPosition;

        // Check which placeholder matches and advance position accordingly

        if (ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_VERSION))
        {
            // Add version
            if(tileType->dataSource.generationInfo && tileType->dataSource.generationInfo[0] != 0 )
            {
                nsl_strlcpy(placeholderString, tileType->dataSource.generationInfo, placeholderStrLength);
            }
            else
            {
                nsl_strlcpy(placeholderString, DEFAULT_VERSION, placeholderStrLength);
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_LOCALE))
        {
            // Add locale
            nsl_strlcpy(placeholderString, DEFAULT_VERSION, placeholderStrLength);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_FORMAT))
        {
            // Set image format based on the configuration
            if (nsl_strcmp(tileType->dataSource.mimeType, NB_MIME_TYPE_PNG) == 0)
            {
                nsl_strlcpy(placeholderString, IMAGE_FORMAT_PNG, placeholderStrLength);
            }
            else if (nsl_strcmp(tileType->dataSource.mimeType, NB_MIME_TYPE_GIF) == 0)
            {
                nsl_strlcpy(placeholderString, IMAGE_FORMAT_GIF, placeholderStrLength);
            }
            else if (nsl_strcmp(tileType->dataSource.mimeType, NB_MIME_TYPE_JPEG) == 0)
            {
                nsl_strlcpy(placeholderString, IMAGE_FORMAT_JPG, placeholderStrLength);
            }
            else
            {
                err = NE_NOTINIT;
                goto cleanup;
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_X_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_Y_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_ZOOM))
        {
            // The order of x/y/z is currently assumed in this order. If it changes then we
            // need a different logic here.

            // For x/y/zoom we just add '%d' so that we can add them later dynamically using the string format functions
            nsl_strlcpy(placeholderString, "%d", placeholderStrLength);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_QUADKEY))
        {
            // Add '%s' so we can later format the string dynamically using the quadkey.
            nsl_strlcpy(placeholderString, "%s", placeholderStrLength);
            tileType->useQuadKey = TRUE;
        }
        else if(ProcessPlaceholder(&currentPosition, NB_TDS_PLACEHOLDER_TILE_SIZE))
        {
            nsl_snprintf(placeholderString, placeholderStrLength, "%d", tileType->dataSource.tileSize);
        }
        // the '$' is needed to find this parameter
        // $user - this parameter will be filled in later

        lastPosition = currentPosition;
        if ( nsl_strncmp( currentPosition,
                          NB_TDS_PLACEHOLDER_USER_PARAMETERS,
                          nsl_strlen( NB_TDS_PLACEHOLDER_USER_PARAMETERS ) ) == 0 )
        {
            lastPosition--;
        }
        currentPosition = nsl_strchr(currentPosition, NB_TDS_PLACEHOLDER_PREFIX);

        resultStringLength = ((resultString)? nsl_strlen(resultString) : 0) +
                             ((iterationString)    ? nsl_strlen(iterationString)  : 0) + 
                             ((placeholderString)  ? nsl_strlen(placeholderString): 0) + 
                               resultStringLength + 1;

        tmp = (char*)nsl_malloc(resultStringLength);
        if (!tmp)
        {
            err = NE_NOMEM;
            goto cleanup;
        }
        nsl_memset(tmp, 0, resultStringLength);
        if (resultString)
        {
            nsl_strcpy(tmp, resultString);
            nsl_free(resultString);
        }
        resultString = tmp;
        tmp = NULL;
        nsl_strcat(resultString, iterationString);
        nsl_strcat(resultString, placeholderString);
        placeholderString[0] = '\0';
    }

    // Copy remaining string (if any)
    bytesCount = (resultString ? nsl_strlen(resultString) : 0) + nsl_strlen(lastPosition) + 1;

    if (!resultString)
    {
        initMemory = TRUE;
    }
    resultString = (char*)nsl_realloc((void*)resultString, bytesCount);
    if (initMemory)
    {
        nsl_memset(resultString, 0, bytesCount);
    }
    if (!resultString)
    {
        err = NE_NOMEM;
        goto cleanup;
    }
    nsl_strcat(resultString, lastPosition);

    // If the route-ID-1 is valid, encode it into base64 and add it to the string
    if (routeId1 && routeId1->data && (routeId1->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId1->data, routeId1->size, &encodedRouteId) == NE_OK)
        {
            resultString = (char*)nsl_realloc((void*)resultString,
                                              nsl_strlen(resultString) +
                                              nsl_strlen(NB_TDS_PARAMETER_ROUTE_ID1) +
                                              nsl_strlen(encodedRouteId) +
                                              1);
            if(!resultString)
            {
                err = NE_NOMEM;
                nsl_free(encodedRouteId);
                goto cleanup;
            }
            nsl_strcat(resultString, NB_TDS_PARAMETER_ROUTE_ID1);
            nsl_strcat(resultString, encodedRouteId);
            nsl_free(encodedRouteId);
        }

        // If route color 1 is set append it
        if (routeColor1)
        {
            // Format color and add to string
            char temp[64] = {0};
            nsl_sprintf(temp,
                        NB_TDS_COLOR_FORMAT,
                        NB_COLOR_GET_R(*routeColor1),
                        NB_COLOR_GET_G(*routeColor1),
                        NB_COLOR_GET_B(*routeColor1),
                        NB_COLOR_GET_A(*routeColor1));
            resultString = (char*)nsl_realloc((void*)resultString,
                                              nsl_strlen(resultString) +
                                              nsl_strlen(NB_TDS_PARAMETER_ROUTE_COLOR1) +
                                              nsl_strlen(temp) +
                                              1);
            if(!resultString)
            {
                err = NE_NOMEM;
                goto cleanup;
            }
            nsl_strcat(resultString, NB_TDS_PARAMETER_ROUTE_COLOR1);
            nsl_strcat(resultString, temp);
        }
    }

    // If the route-ID-2 is valid, encode it into base64 and add it to the string
    if (routeId2 && routeId2->data && (routeId2->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId2->data, routeId2->size, &encodedRouteId) == NE_OK)
        {
            resultString = (char*)nsl_realloc((void*)resultString,
                                              nsl_strlen(resultString) +
                                              nsl_strlen(NB_TDS_PARAMETER_ROUTE_ID2) +
                                              nsl_strlen(encodedRouteId) +
                                              1);
            if(!resultString)
            {
                nsl_free(encodedRouteId);
                err = NE_NOMEM;
                goto cleanup;
            }
            nsl_strcat(resultString, NB_TDS_PARAMETER_ROUTE_ID2);
            nsl_strcat(resultString, encodedRouteId);
            nsl_free(encodedRouteId);
        }

        // If route color 2 is set append it
        if (routeColor2)
        {
            // Format color and add to string
            char temp[64] = {0};
            nsl_sprintf(temp,
                        NB_TDS_COLOR_FORMAT,
                        NB_COLOR_GET_R(*routeColor2),
                        NB_COLOR_GET_G(*routeColor2),
                        NB_COLOR_GET_B(*routeColor2),
                        NB_COLOR_GET_A(*routeColor2));
            resultString = (char*)nsl_realloc((void*)resultString,
                                              nsl_strlen(resultString) +
                                              nsl_strlen(NB_TDS_PARAMETER_ROUTE_COLOR2) +
                                              nsl_strlen(temp) +
                                              1);
            if(!resultString)
            {
                err = NE_NOMEM;
                goto cleanup;
            }
            nsl_strcat(resultString, NB_TDS_PARAMETER_ROUTE_COLOR2);
            nsl_strcat(resultString, temp);
        }
    }

    // Replace old string with new formatted string
    nsl_free(tileType->dataSource.templateUrl);
    tileType->dataSource.templateUrl = resultString;
    resultString = NULL;

cleanup:
    if(placeholderString)
    {
        nsl_free(placeholderString);
    }
    if(iterationString)
    {
        nsl_free(iterationString);
    }
    if(resultString)
    {
        nsl_free(resultString);
    }
    return err;
}

/*! Build part of URL with RouteIDs.

    @return NE_OK on success
*/
static NB_Error BuildRouteString(char** routeString,
                                 NB_RouteId* routeId1,
                                 nb_color* routeColor1,
                                 NB_RouteId* routeId2,
                                 nb_color* routeColor2)
{
    // Temporary string for result
    char* resString = NULL;
    NB_Error retval = NE_OK;

    if(NULL == routeString)
    {
        return NE_INVAL;
    }

    // If the route-ID-1 is valid, encode it into base64 and add it to the string
    if (routeId1 && routeId1->data && (routeId1->size > 0))
    {
        char* encodedRouteId = NULL;

        retval = encode_base64((const char*)routeId1->data, routeId1->size, &encodedRouteId);

        if (retval == NE_OK)
        {
            int bytesCount = (resString ? nsl_strlen(resString) : 0) + nsl_strlen(NB_TDS_PARAMETER_ROUTE_ID1) + nsl_strlen(encodedRouteId) + 1;
            boolean initMemory = FALSE;
            if (!resString)
            {
                initMemory = TRUE;
            }
            resString = (char*)nsl_realloc((void*)resString, bytesCount);
            if(!resString)
            {
                retval = NE_NOMEM;
                nsl_free(encodedRouteId);
                goto exit;
            }
            if (initMemory)
            {
                nsl_memset(resString, 0, bytesCount);
            }
            nsl_strcat(resString, NB_TDS_PARAMETER_ROUTE_ID1);
            nsl_strcat(resString, encodedRouteId);
            nsl_free(encodedRouteId);

            // If route color 1 is set append it
            if (routeColor1)
            {
                // Format color and add to string
                char temp[64] = {0};
                nsl_sprintf(temp,
                            NB_TDS_COLOR_FORMAT,
                            NB_COLOR_GET_R(*routeColor1),
                            NB_COLOR_GET_G(*routeColor1),
                            NB_COLOR_GET_B(*routeColor1),
                            NB_COLOR_GET_A(*routeColor1));
                resString = (char*)nsl_realloc((void*)resString,
                                               nsl_strlen(resString) +
                                               nsl_strlen(NB_TDS_PARAMETER_ROUTE_COLOR1) +
                                               nsl_strlen(temp) +
                                               1);
                if(!resString)
                {
                    retval = NE_NOMEM;
                    goto exit;
                }
                nsl_strcat(resString, NB_TDS_PARAMETER_ROUTE_COLOR1);
                nsl_strcat(resString, temp);
            }
        }
    }

    // If the route-ID-2 is valid, encode it into base64 and add it to the string
    if (retval == NE_OK && routeId2 && routeId2->data && (routeId2->size > 0))
    {
        char* encodedRouteId = NULL;

        retval = encode_base64((const char*)routeId2->data, routeId2->size, &encodedRouteId);

        if (retval == NE_OK)
        {
            resString = (char*)nsl_realloc((void*)resString,
                                           (resString ? nsl_strlen(resString) : 0) +
                                            nsl_strlen(NB_TDS_PARAMETER_ROUTE_ID2) +
                                            nsl_strlen(encodedRouteId) +
                                            1);
            if(!resString)
            {
                nsl_free(encodedRouteId);
                retval = NE_NOMEM;
                goto exit;
            }
            nsl_strcat(resString, NB_TDS_PARAMETER_ROUTE_ID2);
            nsl_strcat(resString, encodedRouteId);
            nsl_free(encodedRouteId);

            // If route color 2 is set append it
            if (routeColor2)
            {
                // Format color and add to string
                char temp[64] = {0};
                nsl_sprintf(temp,
                            NB_TDS_COLOR_FORMAT,
                            NB_COLOR_GET_R(*routeColor2),
                            NB_COLOR_GET_G(*routeColor2),
                            NB_COLOR_GET_B(*routeColor2),
                            NB_COLOR_GET_A(*routeColor2));
                resString = (char*)nsl_realloc((void*)resString,
                                               nsl_strlen(resString) +
                                               nsl_strlen(NB_TDS_PARAMETER_ROUTE_COLOR2) +
                                               nsl_strlen(temp) +
                                               1);
                if(!resString)
                {
                    retval = NE_NOMEM;
                    goto exit;
                }
                nsl_strcat(resString, NB_TDS_PARAMETER_ROUTE_COLOR2);
                nsl_strcat(resString, temp);
            }
        }
    }

    *routeString = resString;
    resString = NULL;

exit:
    return retval;
}

/*! Checks if the current position matches the placeholder.

    If it matches the current position is advanced by the lenth of the placeholder.

    @return TRUE if it matches, FALSE otherwise.
*/
boolean
ProcessPlaceholder(char** currentPosition, const char* placeHolder)
{
    // Does it match
    if(nsl_strnicmp(*currentPosition, placeHolder, nsl_strlen(placeHolder)) == 0)
    {
        // Advance position accordingly
        *currentPosition += nsl_strlen(placeHolder);
        return TRUE;
    }

    return FALSE;
}

/*! Extract ports from template strings and format base url.

    Currently the server returns the ports in the template strings, but we need to extract
    them to make our HTTP connections. We also need to remove the "http://" and any
    colons from the base url.

    @return None
*/
NB_Error
ExtractPortsAndFormatBaseUrl(NB_TileManager* pThis,
                             char** baseUrl,                     /*!< Url to format */
                             char** templateUrl,                    /*!< Template to format */
                             uint16* port,                       /*!< On Return the extracted port */
                             PAL_NetConnectionProtocol connectionProtocol /*!< Protocol type, determines base url string */
                             )
{
    /*
        The code below makes a lot of assumptions on how the server returns the data.
        E.g. it is assumed that the colon (if any) is always in the base url. It is
        also assumed that the port is at the beginning of template string. If any of
        those assumptions are not valid anymore then the code needs to change to reflect this.
    */
    NB_Error err = NE_OK;
    char* tempUrl           = NULL;
    char* tempTemplate      = NULL;
    char* slashPosition     = NULL;
    char* removePosition    = NULL;
    char* colonPosition     = NULL;
    char* pointPosition     = NULL;
    char  base_url_remove[BASE_PROTOCOL_URL_LENGTH] = {0};
    nb_boolean bSingleUrl = FALSE;

    if(PNCP_HTTPS == connectionProtocol)
    {
        nsl_strlcpy(base_url_remove, BASE_HTTPS_URL_REMOVE, sizeof(base_url_remove));
    }
    else //if(PNCP_HTTP == connectionProtocol) /* we suppose that HTTP is default*/
    {
        nsl_strlcpy(base_url_remove, BASE_HTTP_URL_REMOVE, sizeof(base_url_remove));
    }

        // Remove "http://" if present. Our connect will fail if it is present.
    if (baseUrl && *baseUrl) // check - is it single URL. If baseURL present - not single;
    {
        tempUrl = nsl_strdup(*baseUrl);
    }
    else if(*templateUrl)
    {
        bSingleUrl = TRUE;
        tempUrl = nsl_strdup(*templateUrl);
    }
    else 
    {
        err = NE_INVAL;
        goto cleanup;
    }

    if(!tempUrl)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    removePosition = nsl_stristr(tempUrl, base_url_remove);
    if (removePosition)
    {
        nsl_free(*baseUrl);
        *baseUrl = nsl_strdup(removePosition + nsl_strlen(base_url_remove));
        if(!*baseUrl)
        {
            err = NE_NOMEM;
            goto cleanup;
        }

        if (bSingleUrl)
        {
            nsl_free(*templateUrl);
            *templateUrl = nsl_strdup(removePosition + nsl_strlen(base_url_remove));
            if(!*templateUrl)
            {
                err = NE_NOMEM;
                goto cleanup;
            }
        }
    }

    if((!baseUrl) || !(*baseUrl))
    {
        err = NE_INVAL;
        goto cleanup;
    }
    // Look for any colon
    colonPosition = nsl_strchr(*baseUrl, ':');
    if (colonPosition)
    {
        // Just remove the colon and everything after it.
        char *tmp = NULL;
        *colonPosition = '\0';
        tmp = nsl_strdup(*baseUrl);
        if (tmp)
        {
            nsl_free(*baseUrl);
            *baseUrl = tmp;
        }
    }

    // We can have no colon sometimes, in this case we should find first slash
    slashPosition = nsl_strchr(*baseUrl, '/');
    if (slashPosition)
    {
        // Just remove the slash and everything after it.
        char *tmp = NULL;
        *slashPosition = '\0';
        tmp = nsl_strdup(*baseUrl);
        if(tmp)
        {
            nsl_free(*baseUrl);
            *baseUrl = tmp;
        }
    }

    // Get the ports from the template strings. If there is no port then it is set to zero which
    // is ok since that's the value to use the default port.

    colonPosition = nsl_strchr(*templateUrl, ':');
    if (colonPosition)
    {
        colonPosition += 1;
        *port = (uint16)nsl_atoi(colonPosition);
    }
    else
    {
        pointPosition = nsl_strchr(*templateUrl, '.');
        slashPosition = nsl_strchr(*templateUrl, '/');
        // if symbol '.' is between the beginning of string and symbol '/' then it is not port
        if ((pointPosition == NULL) || (pointPosition != NULL && slashPosition != NULL  && (pointPosition > slashPosition)))
        {
            *port = (uint16)nsl_atoi(*templateUrl);
        }
    }

    //We should remove port if it present
    //If port not present in 'template' at all - we should remove all URL part, before first slash
    slashPosition = nsl_strchr(*templateUrl, '/');
    if (slashPosition)
    {
        tempTemplate = nsl_strdup(slashPosition);
        if(tempTemplate)
        {
            nsl_free(*templateUrl);
            *templateUrl = tempTemplate;
            tempTemplate = NULL;
        }
    }

cleanup:
    if(tempTemplate)
    {
        nsl_free(tempTemplate);
        tempTemplate = NULL;
    }
    if(tempUrl)
    {
        nsl_free(tempUrl);
        tempUrl = NULL;
    }
    return err;
}

/*! Extract part of TemplateURL which contain valuable variables for tile key generating

    tileStoreTemplate used for tileKey generating - so it should be no more than tileKey length - 128
    @return NE_OK on success
*/
NB_Error
ExtractTileStoreTemplate(char** tileStoreTemplate,
                         const char* templateURL,
                         const char* baseURL)
{
    int length = 0;

    if (!templateURL || !baseURL || !tileStoreTemplate)
    {
        return NE_INVAL;
    }

    length = nsl_strlen(baseURL) + nsl_strlen(templateURL) + 1; // +1 for terminator

    *tileStoreTemplate = (char*)nsl_malloc(length);

    if (NULL == *tileStoreTemplate)
    {
        return NE_NOMEM;
    }

    nsl_strcpy(*tileStoreTemplate, baseURL);
    nsl_strcat(*tileStoreTemplate, templateURL);

    return NE_OK;
}

/*! Allocate memory and make copy of specified NB_TileDataSourceTemplate

    @return NE_OK on success, NE_NOMEM if case of allocating error
*/
NB_Error
CloneTileDataSourceTemplate(NB_TileDataSourceTemplate *dest,
                            const NB_TileDataSourceTemplate *src)
{
    char **currentDestStr = NULL;
    char *currentSrcStr = NULL;
    char defaultMimeType[] = "image/png";
    int i = 0;

    if (!dest || !src)
    {
        return NE_INVAL;
    }

    // copy static fields
    *dest = *src;

    currentSrcStr  = src->generationInfo;
    currentDestStr = &dest->generationInfo;

    // copy strings
    while(currentDestStr)
    {
        if (currentSrcStr && nsl_strlen(currentSrcStr) > 0)
        {
            *currentDestStr = nsl_strdup(currentSrcStr);
            if (*currentDestStr == NULL)
            {
                // Client of this func should handle this error and release allocated memory
                return NE_NOMEM;
            }
        }
        else
        {
            *currentDestStr = NULL;
        }

        switch(++i)
        {
            case 1:
                currentSrcStr  = src->baseUrl;
                currentDestStr = &dest->baseUrl;
                break;
            case 2:
                currentSrcStr  = src->templateUrl;
                currentDestStr = &dest->templateUrl;
                break;
            case 3:
                currentSrcStr  = src->tileStoreTemplate;
                currentDestStr = &dest->tileStoreTemplate;
                break;
            case 4:
                currentSrcStr  = src->mimeType;
                currentDestStr = &dest->mimeType;
                if (!currentSrcStr || nsl_strlen(currentSrcStr) == 0)
                {
                    currentSrcStr = defaultMimeType;
                }
                break;
            default:
                currentSrcStr  = NULL;
                currentDestStr = NULL;
                break;
        }
    }

    return NE_OK;
}

static void CleanTileDataSourceTemplate(NB_TileDataSourceTemplate* p)
{
    if (!p)
    {
        return;
    }
    if (p->baseUrl)
    {
        nsl_free(p->baseUrl);
        p->baseUrl = NULL;
    }
    if (p->generationInfo)
    {
        nsl_free(p->generationInfo);
        p->generationInfo = NULL;
    }
    if (p->templateUrl)
    {
        nsl_free(p->templateUrl);
        p->templateUrl = NULL;
    }
    if (p->tileStoreTemplate)
    {
        nsl_free(p->tileStoreTemplate);
        p->tileStoreTemplate = NULL;
    }
    if (p->mimeType)
    {
        nsl_free(p->mimeType);
        p->mimeType = NULL;
    }
}

/*! Compare TileDataSourceTemplates

    @return TRUE if NB_TileDataSourceTemplates are equal, FALSE otherwise
*/
nb_boolean
CompareTileDataSourceTemplate(const NB_TileDataSourceTemplate* p1, const NB_TileDataSourceTemplate* p2)
{
    if (!p1 || !p2)
    {
        return FALSE;
    }

    return (nb_boolean)(
            p1->projection       == p2->projection              &&
            p1->isInternalSource == p2->isInternalSource        &&
            p1->tileSize         == p2->tileSize                &&
            p1->cacheId          == p2->cacheId                 &&

            ((p1->baseUrl && p2->baseUrl && 0 == nsl_strcmp(p1->baseUrl, p2->baseUrl)) ||
            ((!p1->baseUrl || !nsl_strlen(p1->baseUrl))&&(!p2->baseUrl || !nsl_strlen(p2->baseUrl)))) &&

            ((p1->generationInfo && p2->generationInfo && 0 == nsl_strcmp(p1->generationInfo, p2->generationInfo)) ||
            ((!p1->generationInfo || !nsl_strlen(p1->generationInfo))&&(!p2->generationInfo || !nsl_strlen(p2->generationInfo)))) &&

            ((p1->templateUrl && p2->templateUrl && 0 == nsl_strcmp(p1->templateUrl, p2->templateUrl)) ||
            ((!p1->templateUrl || !nsl_strlen(p1->templateUrl))&&(!p2->templateUrl || !nsl_strlen(p2->templateUrl)))) &&

            ((p1->tileStoreTemplate && p2->tileStoreTemplate && 0 == nsl_strcmp(p1->tileStoreTemplate, p2->tileStoreTemplate)) ||
            ((!p1->tileStoreTemplate || !nsl_strlen(p1->tileStoreTemplate))&&(!p2->tileStoreTemplate || !nsl_strlen(p2->tileStoreTemplate)))) &&

            ((p1->mimeType && p2->mimeType && 0 == nsl_strcmp(p1->mimeType, p2->mimeType)) ||
            ((!p1->mimeType || !nsl_strlen(p1->mimeType))&&(!p2->mimeType || !nsl_strlen(p2->mimeType))))
           );
}

/*! Compare tileInformation

    @return TRUE if NB_TileInformations are equal, FALSE otherwise
*/
nb_boolean
CompareTileInformation(const NB_TileInformation* p1, const NB_TileInformation* p2)
{
    if (!p1 || !p2)
    {
        return FALSE;
    }
    return (nb_boolean)(p1->tileTypeId == p2->tileTypeId &&
                        p1->xPosition== p2->xPosition &&
                        p1->yPosition== p2->yPosition &&
                        p1->zoomLevel== p2->zoomLevel &&
                        ((p1->additionalPartOfUrl && p2->additionalPartOfUrl && 0 == nsl_strcmp(p1->additionalPartOfUrl, p2->additionalPartOfUrl))
                         || ((!p1->additionalPartOfUrl || 0 == nsl_strlen(p1->additionalPartOfUrl))&&(!p2->additionalPartOfUrl || 0 == nsl_strlen(p2->additionalPartOfUrl)) ))
                        );
}

/*! Initialize NB_TileTypeConfiguration with information from NB_TileDataSourceTemplate

    @return nb error code
*/
NB_Error
InitTileType(NB_TileManager* instance, NB_TileTypeConfiguration* tileType, NB_TileDataSourceTemplate* tileDataSourceTemplate)
{
    NB_Error err = NE_OK;
    PAL_NetConnectionProtocol connectionProtocol = PNCP_HTTP;
    char *tmp = NULL;
    uint32 length = 0;

    if (!instance || !tileType || !tileDataSourceTemplate)
    {
        return NE_INVAL;
    }

    err = CloneTileDataSourceTemplate(&tileType->dataSource, tileDataSourceTemplate);
    if (err != NE_OK)
    {
        goto cleanup;
    }

    (void)CheckConnectionProtocol(&tileType->dataSource, &connectionProtocol);

    // Get ports from templates or base url string and remove any colons from base url
    err = ExtractPortsAndFormatBaseUrl(instance, &tileType->dataSource.baseUrl, &tileType->dataSource.templateUrl, &tileType->port, connectionProtocol);
    if (err != NE_OK)
    {
        goto cleanup;
    }

    if (NULL == tileType->dataSource.tileStoreTemplate)
    {
        if (NE_OK != ExtractTileStoreTemplate(&tileType->dataSource.tileStoreTemplate,
                                              tileType->dataSource.templateUrl,
                                              tileType->dataSource.baseUrl))
        {
            goto cleanup;
        }
    }
    length = nsl_strlen(tileType->dataSource.tileStoreTemplate) + TILE_SIZE_LENGTH + 1;
    tmp = (char*)nsl_malloc(length * sizeof(char));
    if (!tmp)
    {
        goto cleanup;
    }
    nsl_strlcpy(tmp, tileType->dataSource.tileStoreTemplate, length);
    nsl_sprintf(tileType->dataSource.tileStoreTemplate, "%d", tileType->dataSource.tileSize);
    nsl_strlcat(tmp, tileType->dataSource.tileStoreTemplate, length);
    nsl_free(tileType->dataSource.tileStoreTemplate);
    tileType->dataSource.tileStoreTemplate = NULL;
    tileType->dataSource.tileStoreTemplate = nsl_strdup(tmp);
    if (!tileType->dataSource.tileStoreTemplate)
    {
        goto cleanup;
    }
    nsl_free(tmp);
    tmp = NULL;

    // Format data source template strings so that we can use them later.
    err = FormatSource(instance,
                       tileType,
                       NULL,
                       NULL,
                       NULL,
                       NULL);
    if (err != NE_OK)
    {
        goto cleanup;
    }

    return err;

cleanup:
    if (tmp)
    {
        nsl_free(tmp);
        tmp = NULL;
    }
    CleanTileDataSourceTemplate(&tileType->dataSource);
    return err;
}



/*! Convert map type to QA compatible type.

    @return converted type
*/
NB_QaLogMapType
GetQaLogMapType(uint32 tileTypeId)
{
    //@TODO: return appropriate type

    return NB_QLMT_None;
}

/*! Log the cache statistics.

    Call this function each time the cache state changes.

    @return None
*/
void
LogCacheStatistics(NB_TileManager* pThis)
{
    // Only proceed if QA logging is enabled. We don't want to do the cache statistics calculation if we don't need it.
    if (NB_ContextGetQaLog(pThis->context))
    {
        int cachedMemoryItems       = 0;
        int cachedPersistentItems   = 0;
        int protectedItems          = 0;
        int pendingItems            = 0;

        GetCacheStatistics(pThis, &cachedMemoryItems, &cachedPersistentItems, &protectedItems, &pendingItems);

        // Log cache statistics
        NB_QaLogRasterCacheState(pThis->context,
                                 (unsigned)(cachedMemoryItems + cachedPersistentItems),
                                 (unsigned)cachedMemoryItems,
                                 (unsigned)cachedPersistentItems,
                                 (unsigned)pendingItems,
                                 0);
    }
}

/*! Get cache statistics.

    This is used for both LogCacheStatistics() and NB_TileManagerGetDebugInformation().

    @see LogCacheStatistics
    @see NB_TileManagerGetDebugInformation
*/
void
GetCacheStatistics(NB_TileManager* pThis,
                   int* memoryCacheCount,
                   int* persistentCacheCount,
                   int* protectedCount,
                   int* pendingCount)
{
    // Get all pending tiles
    if (pendingCount)
    {
        int length = 0;
        int i = 0;
        int j = 0;
        NB_TileTypeConfiguration* tileType = NULL;

        *pendingCount = 0;
        length = CSL_VectorGetLength(pThis->tileTypes);

        for (i = 0; i < length; i++)
        {
            tileType = CSL_VectorGetPointer(pThis->tileTypes, i);
            if (tileType)
            {
                // Add all pending tiles
                *pendingCount += CSL_VectorGetLength(tileType->pendingTiles);
                // We also want to include all ongoing requests. These are not included in the pending vectors.
                for (j = 0; j < tileType->configuration.maximumConcurrentRequests; ++j)
                {
                    *pendingCount += tileType->connections[j].onGoingRequests;
                }
            }
        }
    }

    CSL_CacheGetStats(pThis->cache, NULL, NULL, NULL, memoryCacheCount, persistentCacheCount, protectedCount);
}

/*! Free a tile buffer.

    @return None
*/
void
FreeBuffer(NB_TileBuffer* buffer)
{
    if (buffer && buffer->buffer)
    {
        nsl_free(buffer->buffer);
        buffer->buffer = NULL;
        buffer->size = 0;
    }
}

NB_TileTypeConfiguration*
GetTileType(CSL_Vector* vec, uint32 typeId)
{
    NB_TileTypeConfiguration* tileType = NULL;
    uint32 length = 0;
    uint32 i = 0;

    if (vec)
    {
        length = CSL_VectorGetLength(vec);

        for (i = 0; i < length; i++)
        {
            tileType = CSL_VectorGetPointer(vec, i);
            if (tileType && tileType->tileTypeId == typeId)
            {
                return tileType;
            }
        }
    }

    return NULL;
}


NB_Error
TileManagerUpdateTilePriority(NB_TileManager* instance,
                              const NB_TileInformation* information,
                              uint32 priority)
{
    TileWithPriority* pPendingTile = NULL;

    if (FindPendingEntry(instance, information, &pPendingTile, NULL))
    {
        // Update priority
        pPendingTile->priority = priority;

        // Sort
        CSL_VectorSort(GetPendingVector(instance, information->tileTypeId), &PriorityVectorCompare);

        // Return success
        return NE_OK;
    }

    // Tile is not in the pending list
    return NE_INVAL;
}
/*! @} */

int
CopyCallbacks(void* callbackData, void* dest, const void* src)
{
    const NB_TileManagerCallback* original = src;
    NB_TileManagerCallback* copy = dest;

    copy->callback = original->callback;
    copy->callbackData = original->callbackData;

    return TRUE;
}

void InvokeCallback(NB_TileManager* pThis,
                    const NB_TileManagerCallback* tileTypeCallback,
                    NB_TileInformation* information,
                    NB_Error result,
                    const NB_Tile* tile)
{
    nb_boolean resultCallback = FALSE;
    CSL_Vector* requestCallbacks = NULL;
    requestCallbacks = GetTileCallbacks(pThis, information);

    if (requestCallbacks)
    {
        nb_boolean res = FALSE;
        NB_TileManagerCallback* requestCallback = NULL;

        // if all of callback functions return TRUE we ignore the rest of callback functions
        // otherwise tila manager invokes the rest of callback functions
        resultCallback = TRUE;
        while (CSL_VectorGetLength(requestCallbacks) > 0)
        {
            requestCallback = CSL_VectorGetPointer(requestCallbacks, 0);
            if (requestCallback && requestCallback->callback)
            {
                res = requestCallback->callback(pThis, result, tile, requestCallback->callbackData);
                if (res == FALSE)
                {
                    resultCallback = FALSE;
                }
            }
            CSL_VectorRemove(requestCallbacks, 0);
        }
        RemoveAllCallbacksFromTile(pThis, information);
    }
    if (resultCallback == FALSE && tileTypeCallback && tileTypeCallback->callback)
    {
        tileTypeCallback->callback(pThis, result, tile, tileTypeCallback->callbackData);
    }
    if (resultCallback == FALSE && pThis->callbackData.callback)
    {
        pThis->callbackData.callback(pThis, result, tile, pThis->callbackData.callbackData);
    }
    if(information->additionalPartOfUrl)
    {
        nsl_free(information->additionalPartOfUrl);
        information->additionalPartOfUrl = NULL;
    }
}
/*
In src we try to find to_find string and replace it with replace_with.
Result will be save in dest.
If src is empty string do nothing.
If dest is NULL then we'll allocate memory.
If dest is not NULL, then we'll free it and allocate memory.
*/
NB_Error strReplace( char** dest,
                     const char* src, 
                     const char* to_find, 
                     const char* replace_with)
{

    char* srcDup = NULL;
    char* strPart = NULL;
    char* tmpString = NULL;
    uint32 length = 0;
    NB_Error result = NE_OK;

    if ( !src || !to_find || !replace_with || !dest)
    {
        return NE_INVAL;
    }

    length = nsl_strlen( src );
    if ( 0 == length )
    {
        return NE_OK;
    }
    length += 1; // for \0

    srcDup = nsl_strdup( src );
    if ( !srcDup )
    {
        return NE_NOMEM;
    }

    if ( *dest )
    {
        nsl_free( *dest );
        *dest = NULL;
    }

    tmpString = nsl_stristr( srcDup, to_find );
    if ( NULL != tmpString )
    {
        strPart = nsl_strdup( tmpString + nsl_strlen( to_find ) );
        if ( strPart )
        {
            nsl_strlcpy( tmpString, replace_with,
                        length - ( tmpString - srcDup ) );
            nsl_strlcat( tmpString + nsl_strlen( replace_with ),
                         strPart,
                         length - ( tmpString - srcDup + nsl_strlen( replace_with ) ) );
        }
        else
        {
            result = NE_NOMEM;
        }
    }

    if ( strPart )
    {
        nsl_free( strPart );
    }

    if ( result == NE_OK )
    {
        *dest = srcDup;
    }
    return result;
}
/*! Compare function for removing duplicates int tile callbacks vector.

    @return Zero if callbacks are identical. Positive otherwise.
*/
int
CallbackVectorCompare(const void* item1,    /*!< Left item to compare */
                      const void* item2     /*!< Right item to compare */
                      )
{
    const NB_TileManagerCallback* callback1 = (NB_TileManagerCallback*)item1;
    const NB_TileManagerCallback* callback2 = (NB_TileManagerCallback*)item2;

    if (callback1 && callback2 &&
       (callback1->callback == callback2->callback) &&
       (callback1->callbackData == callback2->callbackData))
    {
        return 0;
    }

    return 1;
}

NB_Error
AddCallbackToTile(NB_TileManager* pThis,
                  const NB_TileInformation* information,
                  const NB_TileManagerCallback* callback)
{
    char* key = NULL;
    CSL_Vector* vectorOfCallbacks = NULL;

    if (callback == NULL)
    {
        return NE_OK;
    }

    vectorOfCallbacks = GetTileCallbacks(pThis, information);

    if(vectorOfCallbacks == NULL)
    {
        vectorOfCallbacks = CSL_VectorAlloc(sizeof(NB_TileManagerCallback));
    }

    if(vectorOfCallbacks == NULL)
    {
        return NE_NOMEM;
    }

    if (!CSL_VectorAppend(vectorOfCallbacks, callback))
    {
        CSL_VectorDealloc(vectorOfCallbacks);
        return NE_NOMEM;
    }

    CSL_VectorRemoveDuplicates(vectorOfCallbacks, CallbackVectorCompare);

    if (NE_OK != GenerateTileCallbackKey(pThis, information, &key, NULL))
    {
        CSL_VectorDealloc(vectorOfCallbacks);
        return NE_NOMEM;
    }

    if (!CSL_DictionarySet(pThis->tileCallbacks, (const char*)key, (const char*)&vectorOfCallbacks, sizeof(vectorOfCallbacks)))
    {
        CSL_VectorDealloc(vectorOfCallbacks);
        if(key)
        {
            nsl_free(key);
        }
        return NE_NOMEM;
    }

    if(key)
    {
        nsl_free(key);
    }
    return NE_OK;
}

void
RemoveCallbackFromTile(NB_TileManager* pThis,
                       const NB_TileInformation* information,
                       const NB_TileManagerCallback* callback)
{
    CSL_Vector* vectorOfCallbacks = NULL;

    if (callback == NULL)
    {
        return;
    }

    vectorOfCallbacks = GetTileCallbacks(pThis, information);

    if(vectorOfCallbacks)
    {
        int i = 0;
        int length = CSL_VectorGetLength(vectorOfCallbacks);

        for (i = 0; i < length; i++)
        {
            if (CallbackVectorCompare(callback, CSL_VectorGetPointer(vectorOfCallbacks, i)) == 0)
            {
                CSL_VectorRemove(vectorOfCallbacks, i);
                break;
            }
        }

        if (CSL_VectorGetLength(vectorOfCallbacks) == 0)
        {
            char* key = NULL;

            if (NE_OK != GenerateTileCallbackKey(pThis, information, &key, NULL))
            {
                return;
            }

            CSL_VectorDealloc(vectorOfCallbacks);
            CSL_DictionaryDelete(pThis->tileCallbacks, (const char*) key);

            if(key)
            {
                nsl_free(key);
            }
        }
    }
}

void
RemoveAllCallbacksFromTile(NB_TileManager* pThis,
                           const NB_TileInformation* information)
{
    CSL_Vector* vectorOfCallbacks = NULL;

    vectorOfCallbacks = GetTileCallbacks(pThis, information);

    if(vectorOfCallbacks)
    {
        char* key = NULL;

        if (NE_OK == GenerateTileCallbackKey(pThis, information, &key, NULL))
        {
            CSL_VectorDealloc(vectorOfCallbacks);
            CSL_DictionaryDelete(pThis->tileCallbacks, (const char*) key);
            nsl_free(key);
        }
    }
}

int
GetTileCallbacksCount(NB_TileManager* pThis,
                      const NB_TileInformation* information)
{
    CSL_Vector* vectorOfCallbacks = NULL;

    vectorOfCallbacks = GetTileCallbacks(pThis, information);

    if(vectorOfCallbacks)
    {
        return CSL_VectorGetLength(vectorOfCallbacks);
    }
    return 0;
}

CSL_Vector*
GetTileCallbacks(NB_TileManager* pThis,
                 const NB_TileInformation* information)
{
    void* p = NULL;
    char* key = NULL;

    if (NE_OK != GenerateTileCallbackKey(pThis, information, &key, NULL))
    {
        return NULL;
    }

    p = (void*)CSL_DictionaryGet(pThis->tileCallbacks, key, NULL);

    nsl_free(key);

    if(p != NULL)
    {
        return *(CSL_Vector**)p;
    }

    return NULL;
}

NB_Error
GenerateTileCallbackKey(NB_TileManager* pThis, const NB_TileInformation* information, char **key, int *length)
{
    char *res = NULL;
    int len = 0;
    NB_TileTypeConfiguration* tileType = NULL;
    uint32 tileSize = 0;

    if (!information || !key || !pThis)
    {
        return NE_INVAL;
    }

    len = nsl_strlen(TEMPLATE_TILE_CALLBACKS_KEY) + XYZ_LENGTH + TILE_SIZE_LENGTH + 10 /*zoomLevel*/ + 1 +
          ((information->additionalPartOfUrl) ? (nsl_strlen(information->additionalPartOfUrl)) : 0);

    res = (char*)nsl_malloc(len);

    if (!res)
    {
        return NE_NOMEM;
    }

    if (pThis->tileTypes != NULL && NULL != (tileType = GetTileType(pThis->tileTypes, information->tileTypeId)))
    {
        tileSize = tileType->dataSource.tileSize;
    }

    nsl_sprintf(res, TEMPLATE_TILE_CALLBACKS_KEY,
                information->tileTypeId,
                information->xPosition,
                information->yPosition,
                information->zoomLevel,
                tileSize);

    if (information->additionalPartOfUrl)
    {
        nsl_strlcat(res, information->additionalPartOfUrl, len);
    }

    *key = res;

    if(length)
    {
        *length = len;
    }
    return NE_OK;
}

