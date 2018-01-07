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

    @file     nbrastertilemanager.c
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

#include "nbcontextprotected.h"
#include "nbrastertilemanager.h"
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
static const uint32 MAXIMUM_REQUESTS_PER_CONNECTION   = 2;
static const char HTTP_VERB[]                      = "GET";

// Remove this from the base url. The connection will fail if it is present.
static const char BASE_URL_REMOVE[]                = "http://";

// The route-id is of variable length, this is the maximum data as string we use to generate the dictionary-key
// Make sure that this fits inside MAXIMUM_TILE_KEY_LENGTH with enough room for the remaining data
#define MAXIMUM_ROUTE_ID_STRING_LENGTH      32

/* maximum length of map tile access token */
#define MAXIMUM_MAP_TILE_ACCESS_TOKEN_LENGTH 32

// Values used to generate hash key for cache.
#define MAXIMUM_TILE_KEY_LENGTH             256
const char DICTIONARY_FORMAT_KEY[]          = "%d-%d-%d-%d-%s";  // Use "type, z, x, y, route-ids"

/*! We generate a checksum from the base URLs and generation info to detect if the cache is outdated.
    The data gets saved as user data in the cache.
    @see ValidateCache */
#define CACHE_USER_DATA_SIZE                4
const uint32 CRC_VALUE                      = 0xAEF0AEF0;   // Used to generate checksum

// We use these values for our HTTP requests. I don't think there is a need to make them
// setable at this point. Most of them are currently ignored by the server anyway.
const char DEFAULT_LOCALE[]                 = "en-us";
const char DEFAULT_VERSION[]                = "1.0";

const char IMAGE_FORMAT_PNG[]               = "png";
const char IMAGE_FORMAT_GIF[]               = "gif";
const char IMAGE_FORMAT_JPG[]               = "jpg";

// HTTP header required consts
#define MAX_HTTP_HEADER_LENGTH 256
static const char HTTP_HEADERCLIENT_GUID_HEADER[] = "X-Navbuilder-Clientid";
static const char COLON[] = ":";
static const char RETURN_LINEFEED[] = "\r\n";
static const char SPACE[] = " ";
static const char TAB[] = "\t";

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

/*! Connection data

    A pointer to this structure gets passed as user data to the PAL connection callbacks. It has
    all the information needed for this connection.
*/
typedef struct
{
    PAL_NetConnection*      connection;
    ConnectionState         state;

    /*! Pointer to the parent structure. */
    NB_RasterTileManager*   manager;

    /*! Tile ready for request, when network is not connected */
    NB_RasterTile           pendingConnectionTile;

    /*! Number of simultaneous requests allowed on this connection */
    uint32                  allowedRequests;

    /*! Number of on going requests on this connection which have not been finished to completion */
    uint32                  onGoingRequests;

    /*! Type of tiles for this connection. */
    NB_RasterTileType       type;

} Connection;

/*! Tile request data

    A pointer to this structure gets passed as request data to the PAL connection callbacks.
*/
typedef struct
{
    /*! Requested tile */
    NB_RasterTile           tile;

    /*! If we run out of memory during this tile download then we set this flag and report the error once the
        download has finished (instead of reporting multiple errors). */
    nb_boolean              allocationError;

    /*! Total number of bytes expected to be received in the response's body for the request,
        or -1 if unknown. */
    uint32                  contentLength;

} NB_TileRequestData;

/*! Structure including priority and tile data

    We use this structure for the pending tile vector.
*/
typedef struct
{
    /*! Priority of the tile. See NB_RasterTileManagerGetTile for description of allowed values. */
    int priority;

    /*! Pending tile to download. */
    NB_RasterTileInformation information;

} TileWithPriority;

/*! The raster tile manager object. Gets created in NB_RasterTileManagerCreate.

    @see NB_RasterTileManagerCreate
*/
struct NB_RasterTileManager
{
    NB_Context*                     context;
    NB_RasterTileConfiguration      tileConfiguration;
    NB_RasterTileManagerCallback    callbackData;

    /*! Array of connections for map/birds eye/satellite/terrain tiles. */
    Connection*                     mapConnections;

    /*! Array of connections for traffic tiles. */
    Connection*                     trafficConnections;

    /*! Array of connections for route tiles. */
    Connection*                     routeConnections;

    /*! Array of connections for satellite tiles. */
    Connection*                     satelliteConnections;

    /*! Array of connections for hybrid tiles. */
    Connection*                     hybridConnections;

    /*! Array of connections for route selector1 tiles. */
    Connection*                     routeSelectorConnections1;

    /*! Array of connections for route selector2 tiles. */
    Connection*                     routeSelectorConnections2;

    /*! Array of connections for route selector3 tiles. */
    Connection*                     routeSelectorConnections3;

    /*! Array of outstanding tile requests */
    NB_TileRequestData**            tileRequests;
    uint32                          outstandingTileRequestsCount;

    /*! HTTP Ports for connections; get initialized based on the data source strings */
    uint16                          portMap;
    uint16                          portTraffic;
    uint16                          portRoute;
    uint16                          portSatellite;
    uint16                          portHybrid;
    uint16                          portRouteSelector1;
    uint16                          portRouteSelector2;
    uint16                          portRouteSelector3;

    /*! Flags to indicate if the url templates use Quadkey coordinates instead of XYZ coordinates. */
    boolean                         useQuadKeyMap;
    boolean                         useQuadKeyTraffic;
    boolean                         useQuadKeyRoute;
    boolean                         useQuadKeySatellite;
    boolean                         useQuadKeyHybrid;
    boolean                         useQuadKeyRouteSelector1;
    boolean                         useQuadKeyRouteSelector2;
    boolean                         useQuadKeyRouteSelector3;

    /*! Vector of pending tiles.

        We can't use a dictionary here, since we need the list to be sorted by priority. The current
        dictionary implementation uses a hash table which would not be sorted correctly. (What we
        really want is a binary tree map.) We don't want to use a linked list for better sorting performance.

        This vector contains TileWithPriority structures.
    */
    struct CSL_Vector*              pendingMapTiles;
    struct CSL_Vector*              pendingTrafficTiles;
    struct CSL_Vector*              pendingRouteTiles;
    struct CSL_Vector*              pendingSatelliteTiles;
    struct CSL_Vector*              pendingHybridTiles;
    struct CSL_Vector*              pendingRouteSelectorTiles1;
    struct CSL_Vector*              pendingRouteSelectorTiles2;
    struct CSL_Vector*              pendingRouteSelectorTiles3;

    /*! Data source data. Gets passed in NB_RasterTileManagerCreate() */
    NB_RasterTileDataSourceData     dataSourceData;

    /*! Cache object to cache all tiles. The cache object is owned by NB_Context. */
    struct CSL_Cache*               cache;

    /*! We format the http-template with the route-id and route-color each time new route(s) get set */
    char                            semiFormattedRouteUrl[NB_RTDS_TEMPLATE_LENGTH];

    /*! We format the http-template with the route-id each time new route selector get set */
    char                            semiFormattedRouteSelector1Url[NB_RTDS_TEMPLATE_LENGTH];
    char                            semiFormattedRouteSelector2Url[NB_RTDS_TEMPLATE_LENGTH];
    char                            semiFormattedRouteSelector3Url[NB_RTDS_TEMPLATE_LENGTH];

    /*! Hexified route-ids used to generate dictionary-key. Only valid if routes are set.
        If we have two routes then we do a binary XOR and hexify the result here to save space
        (the dictionary key/cache name is limited to 64 bytes)
    */
    char                            routeString[MAXIMUM_ROUTE_ID_STRING_LENGTH + 1];

    /*! map tile access token that will be appended to map tile request URL */
    char                            mapTileAccessToken[MAXIMUM_MAP_TILE_ACCESS_TOKEN_LENGTH + 1];
};


// Local functions ...............................................................................

// Callback function for network call
static void ConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status);
static void DataReceivedCallback(void* userData, void* requestData, PAL_Error errorCode, const byte* bytes, uint32 count);
static boolean TileRequestBegin(NB_RasterTileManager* pThis, NB_TileRequestData* pTileRequest);
static boolean TileRequestEnd(NB_RasterTileManager* pThis, NB_TileRequestData* pTileRequest);
static boolean TileRequestOutstanding(NB_RasterTileManager* pThis, const NB_RasterTileInformation* pTileInfo);

static NB_Error OpenConnection(Connection* connection, const NB_RasterTileInformation* information);
static NB_Error StartRequest(Connection* connection, const NB_RasterTileInformation* information);
static boolean ProcessNextPendingTile(Connection* connection);
static void ResortAndLimitPendingVector(struct CSL_Vector* vector, int limit);
static boolean FindPendingEntry(NB_RasterTileManager* instance, const NB_RasterTileInformation* information, TileWithPriority** pEntry, int* pIndex);
struct CSL_Vector* GetPendingVector(NB_RasterTileManager* instance, NB_RasterTileType type);

static int PriorityVectorCompare(const void* item1, const void* item2);
static void GenerateTileKey(NB_RasterTileManager* instance, const NB_RasterTileInformation* information, char* key);
static NB_Error InitializeConnections(NB_RasterTileManager* pThis, uint32 connectionCount, Connection** connections, NB_RasterTileType type);
static void CleanupConnections(NB_RasterTileManager* pThis, uint32 connectionCount, Connection* connections);

static NB_Error FormatSource(NB_RasterTileManager* pThis,
                             char* templateString,
                             boolean appendToken,
                             NB_RasterTileImageFormat imageFormat,
                             char* version,
                             boolean* useQuadkey,
                             NB_RouteId* routeId1,
                             nb_color* routeColor1,
                             NB_RouteId* routeId2,
                             nb_color* routeColor2);

static NB_Error FormatSourceRouteSelector(NB_RasterTileManager* pThis,
                             char* templateString,
                             boolean appendToken,
                             NB_RasterTileImageFormat imageFormat,
                             char* version,
                             boolean* useQuadkey,
                             NB_RouteId* routeId1,
                             NB_RouteId* routeId2,
                             NB_RouteId* routeId3
                             );

static boolean ProcessPlaceholder(char** currentPosition, const char* placeHolder);
static void ExtractPortsAndFormatBaseUrl(NB_RasterTileManager* pThis, char* baseUrl, char* template, uint16* port);
static void ReplaceTileSizeString(NB_RasterTileManager* pThis, char* template);

// Functions to validate cache
static void ValidateCache(NB_RasterTileManager* pThis);
static uint32 GenerateChecksumFromDataSource(NB_RasterTileManager* pThis);
static void Concatenate(struct dynbuf* buffer, const char* pSource);

// Functions for logging
static NB_QaLogMapType GetQaLogMapType(NB_RasterTileType type);
static void LogCacheStatistics(NB_RasterTileManager* pThis);
static void GetCacheStatistics(NB_RasterTileManager* pThis,
                               int* memoryCacheCount,
                               int* persistentCacheCount,
                               int* protectedCount,
                               int* pendingCount);

static void FreeRoute(NB_RouteId* route);
static NB_Error SetRoute(NB_RouteId* destination, const NB_RouteId* source);
static void FreeBuffer(NB_RasterTileBuffer* buffer);


// Public Functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerCreate(NB_Context* context,
                           const NB_RasterTileDataSourceData* dataSourceData,
                           const char* mapTileAccessToken,
                           const NB_RasterTileConfiguration* tileConfiguration,
                           const NB_RasterTileManagerCallback* callback,
                           NB_RasterTileManager** instance)
{
    NB_Error result = NE_OK;
    NB_RasterTileManager* pThis = NULL;

    // Validate parameters. We have to have at least one map connection. Traffic/Route connections are optional
    if (!context || !dataSourceData || !tileConfiguration || !callback || (tileConfiguration->mapConfiguration.maximumConcurrentRequests == 0))
    {
        return NE_INVAL;
    }

    if (mapTileAccessToken && nsl_strlen(mapTileAccessToken) >= sizeof(pThis->mapTileAccessToken))
    {
        /* token too long */
        return NE_INVAL;
    }

    // Create raster tile manager instance
    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    if (!nsl_strempty(mapTileAccessToken))
    {
        nsl_strncpy(pThis->mapTileAccessToken, mapTileAccessToken, sizeof(pThis->mapTileAccessToken));
    }

    // Get cache object from context and store it internally
    pThis->cache = NB_ContextGetRasterTileManagerCache(context);

    // "maximumConcurrentRequests" means the number of connections for this tile type. Each connection
    // allows MAXIMUM_REQUESTS_PER_CONNECTION tile requests at once.
    pThis->outstandingTileRequestsCount = ( tileConfiguration->mapConfiguration.maximumConcurrentRequests +
         tileConfiguration->satelliteConfiguration.maximumConcurrentRequests +
         tileConfiguration->hybridConfiguration.maximumConcurrentRequests +
         tileConfiguration->trafficConfiguration.maximumConcurrentRequests +
         tileConfiguration->routeConfiguration.maximumConcurrentRequests +
         tileConfiguration->routeAlternate1Configuration.maximumConcurrentRequests +
         tileConfiguration->routeAlternate2Configuration.maximumConcurrentRequests +
         tileConfiguration->routeAlternate3Configuration.maximumConcurrentRequests) * MAXIMUM_REQUESTS_PER_CONNECTION;
    pThis->tileRequests = nsl_malloc(sizeof(NB_TileRequestData*) * pThis->outstandingTileRequestsCount);
    if (pThis->tileRequests == NULL)
    {
        NB_RasterTileManagerDestroy(pThis);
        return NE_NOMEM;
    }
    nsl_memset(pThis->tileRequests, 0, sizeof(NB_TileRequestData*) * pThis->outstandingTileRequestsCount);

    // Create vectors for pending tiles
    pThis->pendingMapTiles          = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingTrafficTiles      = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingRouteTiles        = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingSatelliteTiles    = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingHybridTiles       = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingRouteSelectorTiles1  = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingRouteSelectorTiles2  = CSL_VectorAlloc(sizeof(TileWithPriority));
    pThis->pendingRouteSelectorTiles3  = CSL_VectorAlloc(sizeof(TileWithPriority));
    if (!pThis->pendingMapTiles || !pThis->pendingTrafficTiles || !pThis->pendingRouteTiles ||
        !pThis->pendingSatelliteTiles || !pThis->pendingHybridTiles ||
        !pThis->pendingRouteSelectorTiles1 || !pThis->pendingRouteSelectorTiles2 || !pThis->pendingRouteSelectorTiles3)
    {
        NB_RasterTileManagerDestroy(pThis);
        return NE_NOMEM;
    }

    // Copy parameters to local instance
    pThis->context = context;
    nsl_memcpy(&pThis->dataSourceData, dataSourceData, sizeof(*dataSourceData));
    nsl_memcpy(&pThis->callbackData, callback, sizeof(*callback));
    nsl_memcpy(&pThis->tileConfiguration, tileConfiguration, sizeof(*tileConfiguration));

    LogCacheStatistics(pThis);

    // Format all data source template strings so that we can use them later.
    result = result ? result : FormatSource(pThis,
                                            pThis->dataSourceData.templateMap.templateUrl,
                                            pThis->dataSourceData.templateMap.isInternalSource,
                                            pThis->tileConfiguration.mapConfiguration.imageFormat,
                                            pThis->dataSourceData.templateMap.generationInfo,
                                            &pThis->useQuadKeyMap,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSource(pThis,
                                            pThis->dataSourceData.templateTraffic.templateUrl,
                                            pThis->dataSourceData.templateTraffic.isInternalSource,
                                            pThis->tileConfiguration.trafficConfiguration.imageFormat,
                                            pThis->dataSourceData.templateTraffic.generationInfo,
                                            &pThis->useQuadKeyTraffic,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSource(pThis,
                                            pThis->dataSourceData.templateSatellite.templateUrl,
                                            pThis->dataSourceData.templateSatellite.isInternalSource,
                                            pThis->tileConfiguration.satelliteConfiguration.imageFormat,
                                            pThis->dataSourceData.templateSatellite.generationInfo,
                                            &pThis->useQuadKeySatellite,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSource(pThis,
                                            pThis->dataSourceData.templateHybrid.templateUrl,
                                            pThis->dataSourceData.templateHybrid.isInternalSource,
                                            pThis->tileConfiguration.hybridConfiguration.imageFormat,
                                            pThis->dataSourceData.templateHybrid.generationInfo,
                                            &pThis->useQuadKeyHybrid,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSourceRouteSelector(pThis,
                                            pThis->dataSourceData.templateRouteSelector1.templateUrl,
                                            pThis->dataSourceData.templateRouteSelector1.isInternalSource,
                                            pThis->tileConfiguration.routeAlternate1Configuration.imageFormat,
                                            pThis->dataSourceData.templateRouteSelector1.generationInfo,
                                            &pThis->useQuadKeyRouteSelector1,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSourceRouteSelector(pThis,
                                            pThis->dataSourceData.templateRouteSelector2.templateUrl,
                                            pThis->dataSourceData.templateRouteSelector2.isInternalSource,
                                            pThis->tileConfiguration.routeAlternate2Configuration.imageFormat,
                                            pThis->dataSourceData.templateRouteSelector2.generationInfo,
                                            &pThis->useQuadKeyRouteSelector2,
                                            NULL,
                                            NULL,
                                            NULL);

    result = result ? result : FormatSourceRouteSelector(pThis,
                                            pThis->dataSourceData.templateRouteSelector3.templateUrl,
                                            pThis->dataSourceData.templateRouteSelector3.isInternalSource,
                                            pThis->tileConfiguration.routeAlternate3Configuration.imageFormat,
                                            pThis->dataSourceData.templateRouteSelector3.generationInfo,
                                            &pThis->useQuadKeyRouteSelector3,
                                            NULL,
                                            NULL,
                                            NULL);

    // Overwrite tile size for hybrid and satellite template.
    ReplaceTileSizeString(pThis, pThis->dataSourceData.templateSatellite.templateUrl);
    ReplaceTileSizeString(pThis, pThis->dataSourceData.templateHybrid.templateUrl);

    // Don't format route template here, it gets done in NB_RasterTileManagerSetRoutes().

    if (result != NE_OK)
    {
        NB_RasterTileManagerDestroy(pThis);
        return result;
    }

    // Get ports from templates or base url string and remove any colons from base url
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateMap.baseUrl, pThis->dataSourceData.templateMap.templateUrl, &pThis->portMap);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateTraffic.baseUrl, pThis->dataSourceData.templateTraffic.templateUrl, &pThis->portTraffic);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateRoute.baseUrl, pThis->dataSourceData.templateRoute.templateUrl, &pThis->portRoute);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateSatellite.baseUrl, pThis->dataSourceData.templateSatellite.templateUrl, &pThis->portSatellite);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateHybrid.baseUrl, pThis->dataSourceData.templateHybrid.templateUrl, &pThis->portHybrid);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateRouteSelector1.baseUrl, pThis->dataSourceData.templateRouteSelector1.templateUrl, &pThis->portRouteSelector1);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateRouteSelector2.baseUrl, pThis->dataSourceData.templateRouteSelector2.templateUrl, &pThis->portRouteSelector2);
    ExtractPortsAndFormatBaseUrl(pThis, pThis->dataSourceData.templateRouteSelector3.baseUrl, pThis->dataSourceData.templateRouteSelector3.templateUrl, &pThis->portRouteSelector3);

    /* Create all connections. The traffic and route connections are optional. */
    if ((InitializeConnections(pThis, tileConfiguration->mapConfiguration.maximumConcurrentRequests, &pThis->mapConnections, NB_RTT_Map) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->trafficConfiguration.maximumConcurrentRequests, &pThis->trafficConnections, NB_RTT_Traffic) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->routeConfiguration.maximumConcurrentRequests, &pThis->routeConnections, NB_RTT_Route) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->satelliteConfiguration.maximumConcurrentRequests, &pThis->satelliteConnections, NB_RTT_Satellite) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->hybridConfiguration.maximumConcurrentRequests, &pThis->hybridConnections, NB_RTT_Hybrid) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->routeAlternate1Configuration.maximumConcurrentRequests, &pThis->routeSelectorConnections1, NB_RTT_RouteSelector1) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->routeAlternate2Configuration.maximumConcurrentRequests, &pThis->routeSelectorConnections2, NB_RTT_RouteSelector2) != NE_OK) ||
        (InitializeConnections(pThis, tileConfiguration->routeAlternate3Configuration.maximumConcurrentRequests, &pThis->routeSelectorConnections3, NB_RTT_RouteSelector3) != NE_OK)
       )
    {
        NB_RasterTileManagerDestroy(pThis);
        return NE_NOMEM;
    }

    ValidateCache(pThis);

    *instance = pThis;
    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerDestroy(NB_RasterTileManager* instance)
{
    if (!instance)
    {
        return NE_OK;
    }

    // Destroy all network connections. Destroy them first before destroying the vector and
    // dictionaries since the close might still access them.

    CleanupConnections(instance, instance->tileConfiguration.mapConfiguration.maximumConcurrentRequests, instance->mapConnections);
    CleanupConnections(instance, instance->tileConfiguration.trafficConfiguration.maximumConcurrentRequests, instance->trafficConnections);
    CleanupConnections(instance, instance->tileConfiguration.routeConfiguration.maximumConcurrentRequests, instance->routeConnections);
    CleanupConnections(instance, instance->tileConfiguration.satelliteConfiguration.maximumConcurrentRequests, instance->satelliteConnections);
    CleanupConnections(instance, instance->tileConfiguration.hybridConfiguration.maximumConcurrentRequests, instance->hybridConnections);
    CleanupConnections(instance, instance->tileConfiguration.routeAlternate1Configuration.maximumConcurrentRequests, instance->routeSelectorConnections1);
    CleanupConnections(instance, instance->tileConfiguration.routeAlternate2Configuration.maximumConcurrentRequests, instance->routeSelectorConnections2);
    CleanupConnections(instance, instance->tileConfiguration.routeAlternate3Configuration.maximumConcurrentRequests, instance->routeSelectorConnections3);

    // Destroy all pending tile vectors
    if (instance->pendingMapTiles)
    {
        CSL_VectorDealloc(instance->pendingMapTiles);
    }

    if (instance->pendingTrafficTiles)
    {
        CSL_VectorDealloc(instance->pendingTrafficTiles);
    }

    if (instance->pendingRouteTiles)
    {
        CSL_VectorDealloc(instance->pendingRouteTiles);
    }

    if (instance->pendingSatelliteTiles)
    {
        CSL_VectorDealloc(instance->pendingSatelliteTiles);
    }

    if (instance->pendingHybridTiles)
    {
        CSL_VectorDealloc(instance->pendingHybridTiles);
    }

    if (instance->pendingRouteSelectorTiles1)
    {
        CSL_VectorDealloc(instance->pendingRouteSelectorTiles1);
    }

    if (instance->pendingRouteSelectorTiles2)
    {
        CSL_VectorDealloc(instance->pendingRouteSelectorTiles2);
    }

    if (instance->pendingRouteSelectorTiles3)
    {
        CSL_VectorDealloc(instance->pendingRouteSelectorTiles3);
    }

    if (instance->tileRequests)
    {
        uint32 i = 0;

        for (i = 0; i < instance->outstandingTileRequestsCount; i++)
        {
            if (instance->tileRequests[i] != NULL)
            {
                FreeBuffer(&(instance->tileRequests[i]->tile.buffer));
                nsl_free(instance->tileRequests[i]);
            }
        }
        nsl_free(instance->tileRequests);
    }

    // Don't destroy the cache object here, it is owned by NB_Context

    nsl_free(instance);
    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerTileTypeIsAvailable(NB_RasterTileManager* instance,
                            NB_RasterTileType type,
                            nb_boolean* result)
{
    Connection* connectionArray = NULL;
    struct CSL_Vector* pendingTileVector = NULL;

    if (instance == NULL || result == NULL)
    {
        return NE_INVAL;
    }

    switch (type)
    {
        case NB_RTT_Map:
            connectionArray     = instance->mapConnections;
            pendingTileVector   = instance->pendingMapTiles;
            break;

        case NB_RTT_Traffic:
            connectionArray     = instance->trafficConnections;
            pendingTileVector   = instance->pendingTrafficTiles;
            break;

        case NB_RTT_Route:
            connectionArray     = instance->routeConnections;
            pendingTileVector   = instance->pendingRouteTiles;
            break;

        case NB_RTT_Satellite:
            connectionArray     = instance->satelliteConnections;
            pendingTileVector   = instance->pendingSatelliteTiles;
            break;

        case NB_RTT_Hybrid:
            connectionArray     = instance->hybridConnections;
            pendingTileVector   = instance->pendingHybridTiles;
            break;

        case NB_RTT_RouteSelector1:
            connectionArray     = instance->routeSelectorConnections1;
            pendingTileVector   = instance->pendingRouteSelectorTiles1;
            break;

        case NB_RTT_RouteSelector2:
            connectionArray     = instance->routeSelectorConnections2;
            pendingTileVector   = instance->pendingRouteSelectorTiles2;
            break;

        case NB_RTT_RouteSelector3:
            connectionArray     = instance->routeSelectorConnections3;
            pendingTileVector   = instance->pendingRouteSelectorTiles3;
            break;

        case NB_RTT_None:
        case NB_RTT_BirdsEye:
        case NB_RTT_Terrain:
        default:
            // None of these are currently supported
            break;
    }

    *result = (!connectionArray || !pendingTileVector) ? FALSE : TRUE;
    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerGetTile(NB_RasterTileManager* instance,
                            const NB_RasterTileInformation* information,
                            uint32 priority,
                            NB_RasterTileCommand command,
                            NB_RasterTileBuffer* buffer)
{
    /*
        This function is very similar to NB_RasterTileManagerGetTiles(). When making changes here make sure to update
        NB_RasterTileManagerGetTiles() as well.
    */

    NB_Error cacheResult = NE_OK;
    char key[MAXIMUM_TILE_KEY_LENGTH] = {0};

    // Validate input
    if (command == NB_RTC_None)
    {
        return NE_INVAL;
    }

    buffer->buffer = NULL;
    buffer->size = 0;

    if (TileRequestOutstanding(instance, information))
    {
        // Outstanding request already issued.
        return NE_OK;
    }

    GenerateTileKey(instance, information, key);

    if (command == NB_RTC_Force_Download)
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
                                    (nb_boolean)(command == NB_RTC_MemoryCache));

        LogCacheStatistics(instance);

        // If the tile was in the cache or the command is not set to download then we return
        if ((cacheResult == NE_OK) || (command != NB_RTC_Download))
        {
            return cacheResult;
        }
    }

    {
        uint32 i = 0;
        Connection* connectionArray = NULL;
        uint16 connectionCount = 0;
        int maximumPendingTiles = 0;
        struct CSL_Vector* pendingTileVector = NULL;

        // Check requested tile type and set connection array and pending tile vector accordingly
        switch (information->type)
        {
            case NB_RTT_Map:
                connectionArray     = instance->mapConnections;
                connectionCount     = instance->tileConfiguration.mapConfiguration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingMapTiles;
                maximumPendingTiles = instance->tileConfiguration.mapConfiguration.maximumPendingTiles;
                break;

            case NB_RTT_Traffic:
                connectionArray     = instance->trafficConnections;
                connectionCount     = instance->tileConfiguration.trafficConfiguration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingTrafficTiles;
                maximumPendingTiles = instance->tileConfiguration.trafficConfiguration.maximumPendingTiles;
                break;

            case NB_RTT_Route:
                connectionArray     = instance->routeConnections;
                connectionCount     = instance->tileConfiguration.routeConfiguration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingRouteTiles;
                maximumPendingTiles = instance->tileConfiguration.routeConfiguration.maximumPendingTiles;
                break;

            case NB_RTT_Satellite:
                connectionArray     = instance->satelliteConnections;
                connectionCount     = instance->tileConfiguration.satelliteConfiguration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingSatelliteTiles;
                maximumPendingTiles = instance->tileConfiguration.satelliteConfiguration.maximumPendingTiles;
                break;

            case NB_RTT_Hybrid:
                connectionArray     = instance->hybridConnections;
                connectionCount     = instance->tileConfiguration.hybridConfiguration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingHybridTiles;
                maximumPendingTiles = instance->tileConfiguration.hybridConfiguration.maximumPendingTiles;
                break;

            case NB_RTT_RouteSelector1:
                connectionArray     = instance->routeSelectorConnections1;
                connectionCount     = instance->tileConfiguration.routeAlternate1Configuration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingRouteSelectorTiles1;
                maximumPendingTiles = instance->tileConfiguration.routeAlternate1Configuration.maximumPendingTiles;
                break;

            case NB_RTT_RouteSelector2:
                connectionArray     = instance->routeSelectorConnections2;
                connectionCount     = instance->tileConfiguration.routeAlternate2Configuration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingRouteSelectorTiles2;
                maximumPendingTiles = instance->tileConfiguration.routeAlternate2Configuration.maximumPendingTiles;
                break;

            case NB_RTT_RouteSelector3:
                connectionArray     = instance->routeSelectorConnections3;
                connectionCount     = instance->tileConfiguration.routeAlternate3Configuration.maximumConcurrentRequests;
                pendingTileVector   = instance->pendingRouteSelectorTiles3;
                maximumPendingTiles = instance->tileConfiguration.routeAlternate3Configuration.maximumPendingTiles;
                break;

            case NB_RTT_None:
            case NB_RTT_BirdsEye:
            case NB_RTT_Terrain:
            default:
                // None of these are currently supported
                break;
        }

        if (!connectionArray || !pendingTileVector)
        {
            return NE_INVAL;
        }

        /*
            Search pending tile vector for the requested tile. If tile is already pending,
            update the priority, re-sort the list, and exit without adding a new request.
        */
        if (NB_RasterTileManagerUpdateTilePriority(instance, information, priority) == NE_OK)
        {
            // Return out; no new request necessary
            return NE_OK;
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
                return OpenConnection(connection, information);
            }
            /*
                Start the request, if number of on going requests with CS_Connected connection are less than
                the allowed number of requests.
            */
            else if (connection->state == CS_Connected && connection->onGoingRequests < connection->allowedRequests)
            {
                // Start the http request. The results will be handled in the receive callback.
                return StartRequest(connection, information);
            }
        }

        /* All connections are currently serving requests to maximum allowed limit.
         Add to pending list. As soon as number of on going requests on a connection drop
         to less than the maximum allowed requests, it will process the next tile request.
         See receive callback. */
        {
            TileWithPriority tile = {0};
            tile.priority = priority;
            nsl_memcpy(&tile.information, information, sizeof(NB_RasterTileInformation));

            if (!CSL_VectorAppend(pendingTileVector, &tile))
            {
                return NE_NOMEM;
            }

            ResortAndLimitPendingVector(pendingTileVector, maximumPendingTiles);
        }

        return NE_OK;
    }
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerGetTiles(NB_RasterTileManager* instance,
                             NB_RasterTileRequest* requestArray,
                             uint32 count)
{
    /*
        This function is very similar to NB_RasterTileManagerGetTile(). When making changes here make sure to update
        NB_RasterTileManagerGetTile() as well.
    */

    NB_Error result = NE_OK;
    char key[MAXIMUM_TILE_KEY_LENGTH] = {0};
    uint32 i = 0;

    nb_boolean anyMapTiles          = FALSE;
    nb_boolean anyTrafficTiles      = FALSE;
    nb_boolean anyRouteTiles        = FALSE;
    nb_boolean anySatelliteTiles    = FALSE;
    nb_boolean anyHybridTiles       = FALSE;
    nb_boolean anyRouteSelectorTiles1       = FALSE;
    nb_boolean anyRouteSelectorTiles2       = FALSE;
    nb_boolean anyRouteSelectorTiles3       = FALSE;

    nb_boolean allMapConnectionsBusy        = FALSE;
    nb_boolean allTrafficConnectionsBusy    = FALSE;
    nb_boolean allRouteConnectionsBusy      = FALSE;
    nb_boolean allSatelliteConnectionsBusy  = FALSE;
    nb_boolean allHybridConnectionsBusy     = FALSE;
    nb_boolean allRouteSelectorConnectionsBusy  = FALSE;

    // For all requested tiles
    for (i = 0; i < count; ++i)
    {
        NB_Error cacheResult = NE_OK;
        boolean needsDownload = TRUE;

        requestArray[i].buffer->buffer = NULL;
        requestArray[i].buffer->size = 0;

        if (TileRequestOutstanding(instance, requestArray[i].information))
        {
            // Outstanding request already issued.
            needsDownload = FALSE;
        }
        else
        {
            GenerateTileKey(instance, requestArray[i].information, key);

            if (requestArray[i].command == NB_RTC_Force_Download)
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
                                            (nb_boolean)(requestArray[i].command == NB_RTC_MemoryCache));

                LogCacheStatistics(instance);

                // If the tile was in the cache or the command is not set to download then we're done
                if ((cacheResult == NE_OK) || (requestArray[i].command != NB_RTC_Download))
                {
                    // Don't download the tile
                    needsDownload = FALSE;
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
            boolean* pAllConnectionsAreBusy = NULL;

            // Check requested tile type and set connection array and pending tile vector accordingly
            switch (requestArray[i].information->type)
            {
                case NB_RTT_Map:
                    connectionArray         = instance->mapConnections;
                    connectionCount         = instance->tileConfiguration.mapConfiguration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingMapTiles;
                    maximumPendingTiles     = instance->tileConfiguration.mapConfiguration.maximumPendingTiles;
                    anyMapTiles             = TRUE;
                    pAllConnectionsAreBusy  = &allMapConnectionsBusy;
                    break;

                case NB_RTT_Traffic:
                    connectionArray         = instance->trafficConnections;
                    connectionCount         = instance->tileConfiguration.trafficConfiguration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingTrafficTiles;
                    maximumPendingTiles     = instance->tileConfiguration.trafficConfiguration.maximumPendingTiles;
                    anyTrafficTiles         = TRUE;
                    pAllConnectionsAreBusy  = &allTrafficConnectionsBusy;
                    break;

                case NB_RTT_Route:
                    connectionArray         = instance->routeConnections;
                    connectionCount         = instance->tileConfiguration.routeConfiguration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingRouteTiles;
                    maximumPendingTiles     = instance->tileConfiguration.routeConfiguration.maximumPendingTiles;
                    anyRouteTiles           = TRUE;
                    pAllConnectionsAreBusy  = &allRouteConnectionsBusy;
                    break;

                case NB_RTT_Satellite:
                    connectionArray         = instance->satelliteConnections;
                    connectionCount         = instance->tileConfiguration.satelliteConfiguration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingSatelliteTiles;
                    maximumPendingTiles     = instance->tileConfiguration.satelliteConfiguration.maximumPendingTiles;
                    anySatelliteTiles       = TRUE;
                    pAllConnectionsAreBusy  = &allSatelliteConnectionsBusy;
                    break;

                case NB_RTT_Hybrid:
                    connectionArray         = instance->hybridConnections;
                    connectionCount         = instance->tileConfiguration.hybridConfiguration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingHybridTiles;
                    maximumPendingTiles     = instance->tileConfiguration.hybridConfiguration.maximumPendingTiles;
                    anyHybridTiles          = TRUE;
                    pAllConnectionsAreBusy  = &allHybridConnectionsBusy;
                    break;

                case NB_RTT_RouteSelector1:
                    connectionArray         = instance->routeSelectorConnections1;
                    connectionCount         = instance->tileConfiguration.routeAlternate1Configuration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingRouteSelectorTiles1;
                    maximumPendingTiles     = instance->tileConfiguration.routeAlternate1Configuration.maximumPendingTiles;
                    anyRouteSelectorTiles1  = TRUE;
                    pAllConnectionsAreBusy  = &allRouteSelectorConnectionsBusy;
                    break;

                case NB_RTT_RouteSelector2:
                    connectionArray         = instance->routeSelectorConnections2;
                    connectionCount         = instance->tileConfiguration.routeAlternate2Configuration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingRouteSelectorTiles2;
                    maximumPendingTiles     = instance->tileConfiguration.routeAlternate2Configuration.maximumPendingTiles;
                    anyRouteSelectorTiles2  = TRUE;
                    pAllConnectionsAreBusy  = &allRouteSelectorConnectionsBusy;
                    break;

                case NB_RTT_RouteSelector3:
                    connectionArray         = instance->routeSelectorConnections3;
                    connectionCount         = instance->tileConfiguration.routeAlternate3Configuration.maximumConcurrentRequests;
                    pendingTileVector       = instance->pendingRouteSelectorTiles3;
                    maximumPendingTiles     = instance->tileConfiguration.routeAlternate3Configuration.maximumPendingTiles;
                    anyRouteSelectorTiles3  = TRUE;
                    pAllConnectionsAreBusy  = &allRouteSelectorConnectionsBusy;
                    break;

                case NB_RTT_None:
                case NB_RTT_BirdsEye:
                case NB_RTT_Terrain:
                default:
                    // None of these are currently supported
                    break;
            }

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
                if (!(*pAllConnectionsAreBusy))
                {
                    uint32 connectionIndex = 0;

                    // Assume everything is busy. If we find a non-busy connection then we reset the flag
                    *pAllConnectionsAreBusy = TRUE;

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
                            if (result == NE_BADDATA) // it's not fatal errror, it just means that baseUrl is invalid
                            {
                                result = NE_OK;
                            }
                            // Open the connection. We make the HTTP request in the status callback once the connection
                            // is connected.
                            result = result ? result : OpenConnection(connection, requestArray[i].information);
                            *pAllConnectionsAreBusy = FALSE;
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
                            *pAllConnectionsAreBusy = FALSE;
                            break;
                        }
                    }
                }

                if (*pAllConnectionsAreBusy)
                {
                    /* All connections are currently serving requests to maximum allowed limit.
                     Add to pending list. As soon as number of on going requests on a connection drop
                     to less than the maximum allowed requests, it will process the next tile request.
                     See receive callback. */

                    TileWithPriority tile = {0};
                    tile.priority = requestArray[i].priority;
                    nsl_memcpy(&tile.information, requestArray[i].information, sizeof(NB_RasterTileInformation));

                    if (!CSL_VectorAppend(pendingTileVector, &tile))
                    {
                        return NE_NOMEM;
                    }
                }
            }
        }
    }

    // Resort any pending vector which needs resorting

    if (anyMapTiles)
    {
        ResortAndLimitPendingVector(instance->pendingMapTiles, instance->tileConfiguration.mapConfiguration.maximumPendingTiles);
    }
    if (anyTrafficTiles)
    {
        ResortAndLimitPendingVector(instance->pendingTrafficTiles, instance->tileConfiguration.trafficConfiguration.maximumPendingTiles);
    }
    if (anyRouteTiles)
    {
        ResortAndLimitPendingVector(instance->pendingRouteTiles, instance->tileConfiguration.routeConfiguration.maximumPendingTiles);
    }
    if (anySatelliteTiles)
    {
        ResortAndLimitPendingVector(instance->pendingSatelliteTiles, instance->tileConfiguration.satelliteConfiguration.maximumPendingTiles);
    }
    if (anyHybridTiles)
    {
        ResortAndLimitPendingVector(instance->pendingHybridTiles, instance->tileConfiguration.hybridConfiguration.maximumPendingTiles);
    }
    if (anyRouteSelectorTiles1)
    {
        ResortAndLimitPendingVector(instance->pendingRouteSelectorTiles1, instance->tileConfiguration.routeAlternate1Configuration.maximumPendingTiles);
    }
    if (anyRouteSelectorTiles2)
    {
        ResortAndLimitPendingVector(instance->pendingRouteSelectorTiles2, instance->tileConfiguration.routeAlternate2Configuration.maximumPendingTiles);
    }
    if (anyRouteSelectorTiles3)
    {
        ResortAndLimitPendingVector(instance->pendingRouteSelectorTiles3, instance->tileConfiguration.routeAlternate3Configuration.maximumPendingTiles);
    }

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerUnlockTile(NB_RasterTileManager* instance,
                               const NB_RasterTileInformation* information)
{
    char key[MAXIMUM_TILE_KEY_LENGTH] = {0};
    GenerateTileKey(instance, information, key);

    // Unlock the tile from the cache
    return CSL_CacheClearProtectOne(instance->cache, (byte*)key, nsl_strlen(key));
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerUpdateTilePriority(NB_RasterTileManager* instance,
                                       const NB_RasterTileInformation* information,
                                       uint32 priority)
{
    TileWithPriority* pPendingTile = NULL;

    if (FindPendingEntry(instance, information, &pPendingTile, NULL))
    {
        // Update priority
        pPendingTile->priority = priority;

        // Sort
        CSL_VectorSort(GetPendingVector(instance, information->type), &PriorityVectorCompare);

        // Return success
        return NE_OK;
    }

    // Tile is not in the pending list
    return NE_INVAL;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerFlushAllTiles(NB_RasterTileManager* instance)
{
    CSL_VectorRemoveAll(instance->pendingMapTiles);
    CSL_VectorRemoveAll(instance->pendingTrafficTiles);
    CSL_VectorRemoveAll(instance->pendingRouteTiles);
    CSL_VectorRemoveAll(instance->pendingSatelliteTiles);
    CSL_VectorRemoveAll(instance->pendingHybridTiles);
    CSL_VectorRemoveAll(instance->pendingRouteSelectorTiles1);
    CSL_VectorRemoveAll(instance->pendingRouteSelectorTiles2);
    CSL_VectorRemoveAll(instance->pendingRouteSelectorTiles3);

    return NE_OK;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerFlushTile(NB_RasterTileManager* instance,
                              const NB_RasterTileInformation* information)
{
    int index = 0;
    if (FindPendingEntry(instance, information, NULL, &index))
    {
        // Remove from the pending list
        CSL_VectorRemove(GetPendingVector(instance, information->type), index);
        return NE_OK;
    }

    // Entry is not in the pending list
    return NE_INVAL;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerClearCache(NB_RasterTileManager* instance, NB_RasterTileClearCache options)
{
    CSL_ClearOption cslOptions = CSL_CF_All;
    switch (options)
    {
        case NB_RTCC_All:           cslOptions = CSL_CF_All;                break;
        case NB_RTCC_MemoryOnly:    cslOptions = CSL_CF_MemoryOnly;         break;
        case NB_RTCC_UnlockedOnly:  cslOptions = CSL_CF_UnprotectedOnly;    break;
    }

    return CSL_CacheClearExtended(instance->cache, cslOptions);
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerSaveToPersistentCache(NB_RasterTileManager* instance,
                                          int32 maximumEntries,
                                          int32* savedEntries)
{
    // Forward the call to the cache. Keep the tiles in the memory cache.
    NB_Error result = CSL_CacheSaveDirty(instance->cache, maximumEntries, savedEntries, FALSE);

    LogCacheStatistics(instance);

    return result;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerSavePersistentCacheIndex(NB_RasterTileManager* instance)
{
    return CSL_CacheSaveIndex(instance->cache);
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerSetRoutes(NB_RasterTileManager* instance,
                              NB_RouteId routeId1,
                              nb_color routeColor1,
                              NB_RouteId routeId2,
                              nb_color routeColor2)
{
    nsl_memset(instance->routeString, 0, sizeof(instance->routeString));

    // Do we have two route ids
    if ((routeId1.data && (routeId1.size > 0)) && (routeId2.data && (routeId2.size > 0)))
    {
        // The maximum number of bytes we use from the route-ids is half the size which we have for
        // the hexified characters.
        #define MAXIMUM_COMBINED_SIZE   (MAXIMUM_ROUTE_ID_STRING_LENGTH / 2)
        int maximumCombinedSize = MAXIMUM_COMBINED_SIZE;
        uint8 combinedData[MAXIMUM_COMBINED_SIZE] = {0};

        int dataSizeUsed = MIN(routeId1.size, maximumCombinedSize);
        int index = 0;

        // Copy first route-id
        nsl_memcpy(combinedData, routeId1.data, dataSizeUsed);

        // XOR second route-id to save space
        dataSizeUsed = MIN(routeId2.size, maximumCombinedSize);
        for (index = 0; index < dataSizeUsed; ++index)
        {
            combinedData[index] ^= routeId2.data[index];
        }

        // Now hexify the result in our route-string. This value will be used to generate the key for the cache
        hexlify(dataSizeUsed, (const char*)combinedData, instance->routeString);
    }
    // Do we only have one route-id
    else if (routeId1.data && (routeId1.size > 0))
    {
        // Use the first route-id as hex-string
        hexlify(MIN(routeId1.size, MAXIMUM_ROUTE_ID_STRING_LENGTH / 2), (const char*)routeId1.data, instance->routeString);
    }

    // Make a copy of the route template and format the copy with the route-id(s) and route-color(s)
    nsl_strlcpy(instance->semiFormattedRouteUrl, instance->dataSourceData.templateRoute.templateUrl, sizeof(instance->semiFormattedRouteUrl));
    return FormatSource(instance,
                        instance->semiFormattedRouteUrl,
                        instance->dataSourceData.templateRoute.isInternalSource,
                        instance->tileConfiguration.routeConfiguration.imageFormat,
                        NULL,
                        &instance->useQuadKeyRoute,
                        &routeId1,
                        &routeColor1,
                        &routeId2,
                        &routeColor2);
}

/* See header file for description. */
NB_DEC NB_Error
NB_RasterTileManagerSetAlternateRoute(NB_RasterTileManager* instance,
                              NB_RouteId routeId1,
                              NB_RouteId routeId2,
                              NB_RouteId routeId3
                              )
{
    NB_Error err = NE_OK;

    nsl_memset(instance->routeString, 0, sizeof(instance->routeString));

    // Do we have three route ids
    if( routeId1.data && (routeId1.size > 0) )
    {
        // The maximum number of bytes we use from the route-ids is half the size which we have for
        // the hexified characters.
        #define MAXIMUM_COMBINED_SIZE   (MAXIMUM_ROUTE_ID_STRING_LENGTH / 2)
        int maximumCombinedSize = MAXIMUM_COMBINED_SIZE;
        uint8 combinedData[MAXIMUM_COMBINED_SIZE] = {0};

        int dataSizeUsed = MIN(routeId1.size, maximumCombinedSize);
        int index = 0;

        // Copy first route-id
        nsl_memcpy(combinedData, routeId1.data, dataSizeUsed);

        if( routeId2.data && (routeId2.size > 0) )
        {
            // XOR second route-id to save space
            dataSizeUsed = MIN(routeId2.size, maximumCombinedSize);
            for (index = 0; index < dataSizeUsed; ++index)
            {
                combinedData[index] ^= routeId2.data[index];
            }

            if( routeId3.data && (routeId3.size > 0) )
            {
                // XOR third route-id to save space
                dataSizeUsed = MIN(routeId3.size, maximumCombinedSize);
                for (index = 0; index < dataSizeUsed; ++index)
                {
                    combinedData[index] ^= routeId3.data[index];
                }
            }
        }

        // Now hexify the result in our route-string. This value will be used to generate the key for the cache
        hexlify(dataSizeUsed, (const char*)combinedData, instance->routeString);
    }
    else
    {
        return NE_INVAL;
    }

    // Make a copy of the route template and format the copy with the route-id(s)

    nsl_strlcpy(instance->semiFormattedRouteSelector1Url, instance->dataSourceData.templateRouteSelector1.templateUrl, sizeof(instance->semiFormattedRouteSelector1Url));
    err = FormatSourceRouteSelector(instance,
                            instance->semiFormattedRouteSelector1Url,
                            instance->dataSourceData.templateRouteSelector1.isInternalSource,
                            instance->tileConfiguration.routeAlternate1Configuration.imageFormat,
                            NULL,
                            &instance->useQuadKeyRouteSelector1,
                            &routeId1,
                            &routeId2,
                            &routeId3
                            );

    nsl_strlcpy(instance->semiFormattedRouteSelector2Url, instance->dataSourceData.templateRouteSelector2.templateUrl, sizeof(instance->semiFormattedRouteSelector2Url));
    err = err ? err : FormatSourceRouteSelector(instance,
                            instance->semiFormattedRouteSelector2Url,
                            instance->dataSourceData.templateRouteSelector2.isInternalSource,
                            instance->tileConfiguration.routeAlternate2Configuration.imageFormat,
                            NULL,
                            &instance->useQuadKeyRouteSelector2,
                            &routeId1,
                            &routeId2,
                            &routeId3
                            );

    nsl_strlcpy(instance->semiFormattedRouteSelector3Url, instance->dataSourceData.templateRouteSelector3.templateUrl, sizeof(instance->semiFormattedRouteSelector3Url));
    err = err ? err : FormatSourceRouteSelector(instance,
                            instance->semiFormattedRouteSelector3Url,
                            instance->dataSourceData.templateRouteSelector3.isInternalSource,
                            instance->tileConfiguration.routeAlternate3Configuration.imageFormat,
                            NULL,
                            &instance->useQuadKeyRouteSelector3,
                            &routeId1,
                            &routeId2,
                            &routeId3
                            );
    return err;
}

/* See header file for description. */
NB_DEF NB_Error
NB_RasterTileManagerGetDebugInformation(NB_RasterTileManager* instance,
                                        int* memoryCacheCount,
                                        int* persistentCacheCount,
                                        int* protectedCount,
                                        int* pendingCount)
{
    GetCacheStatistics(instance, memoryCacheCount, persistentCacheCount, protectedCount, pendingCount);
    return NE_OK;
}


// Private functions .............................................................................

/*! Callback for network status.

    @return None
    @see NB_RasterTileManagerGetTile
*/
void
ConnectionStatusCallback(void* userData,                   /*!< caller-supplied reference for this connection */
                         PAL_NetConnectionStatus status    /*!< current PAL network connection status */
                         )
{
    // The user data points to one of the entries in the connection array
    Connection* connection = (Connection*) userData;
    NB_RasterTileManager* pThis = connection->manager;

    switch (status)
    {
        // The connection to the host has failed
        case PNCS_Failed:
            // Forward the network connection failure to the API users callback
            pThis->callbackData.callback(pThis, NE_NET, NULL, pThis->callbackData.callbackData);
            break;

        case PNCS_Error:
            // Connection is in error state, until it gets closed
            connection->state = CS_ConnectionError;
            // Close the connection.
            PAL_NetCloseConnection(connection->connection);
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
                pThis->callbackData.callback(pThis, result, &connection->pendingConnectionTile, pThis->callbackData.callbackData);
            }

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
TileRequestBegin(NB_RasterTileManager* pThis, NB_TileRequestData* pTileRequest)
{
    uint32 i = 0;
    boolean result = FALSE;

    // Add to tileRequests
    for( i = 0; i < pThis->outstandingTileRequestsCount; i++ )
    {
        if( pThis->tileRequests[i] == NULL )
        {
            pThis->tileRequests[i] = pTileRequest;
            result = TRUE;
            break;
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
TileRequestEnd(NB_RasterTileManager* pThis, NB_TileRequestData* pTileRequest)
{
    uint32 i = 0;
    boolean ret = FALSE;

    // Clear from tileRequests
    for( i = 0; i < pThis->outstandingTileRequestsCount; i++ )
    {
        if( pThis->tileRequests[i] == pTileRequest )
        {
            nsl_free(pTileRequest);
            pThis->tileRequests[i] = NULL;
            ret = TRUE;
            break;
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
TileRequestOutstanding(NB_RasterTileManager* pThis, const NB_RasterTileInformation* pTileInfo)
{
    uint32 i = 0;
    boolean ret = FALSE;

    for (i = 0; i < pThis->outstandingTileRequestsCount; i++)
    {
        // Compare the tile information
        if ((pThis->tileRequests[i]) && (nsl_memcmp(&(pThis->tileRequests[i]->tile.information), pTileInfo, sizeof(NB_RasterTileInformation)) == 0))
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

/*! Callback for receiving data from the network.

    @return None
    @see NB_RasterTileManagerGetTile
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
    NB_RasterTileManager* pThis = connection->manager;
    NB_RasterTileBuffer*  pBuffer = &(pTileRequest->tile.buffer);

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
        NB_RasterTile tempTile = {0};
        nsl_memcpy(&tempTile, &pTileRequest->tile, sizeof(NB_RasterTile));

        // Reallocation error OR Network error for this specific request
        if (pTileRequest->allocationError || errorCode != PAL_Ok)
        {
            // Allocation error
            NB_Error result = NE_NOMEM;

            if (!pTileRequest->allocationError)
            {
                // Handle special error code for traffic/route tile failure
                result = (errorCode == PAL_ErrHttpNoContent) ? NE_HTTP_NO_CONTENT : NE_NET;
            }

            FreeBuffer(pBuffer);

            // Log the failed tile
            NB_QaLogRasterTileReply(pThis->context,
                                    GetQaLogMapType(tempTile.information.type),
                                    tempTile.information.xPosition,
                                    tempTile.information.yPosition,
                                    tempTile.information.zoomLevel,
                                    pThis->tileConfiguration.tileResolution,
                                    pThis->tileConfiguration.tileSize,
                                    tempTile.buffer.size,
                                    result);

            /* RasterTile request life cycle complete. Free it */
            TileRequestEnd(pThis, pTileRequest);
            connection->onGoingRequests--;

            /*
                :TRICKY:
                Report an error to the user.
                This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
                The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
                the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
                so that we can re-request the tile right away.
            */
            pThis->callbackData.callback(pThis, result, &tempTile, pThis->callbackData.callbackData);
        }
        // We successfully downloaded the tile.
        else
        {
            NB_Error result = NE_OK;
            char key[MAXIMUM_TILE_KEY_LENGTH] = {0};
            GenerateTileKey(pThis, &tempTile.information, key);
            connection->state = CS_Connected;

            // Add the entry to cache, except for the case of overlay tiles with no data
            if (!
                ( ((tempTile.information.type == NB_RTT_Traffic)        ||
                   (tempTile.information.type == NB_RTT_Route)          ||
                   (tempTile.information.type == NB_RTT_RouteSelector1) ||
                   (tempTile.information.type == NB_RTT_RouteSelector2) ||
                   (tempTile.information.type == NB_RTT_RouteSelector3)
                  ) &&
                  ((pBuffer->buffer == NULL) && (pBuffer->size == 0)))
               )
            {
                // Add the entry to the cache and protect/lock it. This will fail if too many entries are protected/locked.
                result = CSL_CacheAdd(pThis->cache, (byte*)key, nsl_strlen(key), pBuffer->buffer, pBuffer->size, TRUE, FALSE, 0);
                if (result == NE_OK)
                {
                    // If the element was added successfully then the cache takes ownership of the tile data. We set the
                    // buffer to NULL in order to NOT free it below. We have to free the data in all other error cases.
                    pBuffer->buffer = NULL;
                    pBuffer->size = 0;
                }
            }

            // Log the received tile
            NB_QaLogRasterTileReply(pThis->context,
                                    GetQaLogMapType(tempTile.information.type),
                                    tempTile.information.xPosition,
                                    tempTile.information.yPosition,
                                    tempTile.information.zoomLevel,
                                    pThis->tileConfiguration.tileResolution,
                                    pThis->tileConfiguration.tileSize,
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

            /* RasterTile request life cycle complete. Free it */
            TileRequestEnd(pThis, pTileRequest);
            connection->onGoingRequests--;

            /*
             :TRICKY:
             Report an error to the user.
             This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
             The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
             the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
             so that we can re-request the tile right away.
             */
            pThis->callbackData.callback(pThis, result, &tempTile, pThis->callbackData.callbackData);

            LogCacheStatistics(pThis);
        }

        // Process next pending tile (if any)
        ProcessNextPendingTile(connection);
    }
}

/*! Callback for response from the network.

    @return None
    @see NB_RasterTileManagerGetTile
*/
void
ResponseStatusCallback(PAL_Error errorCode,
                       void* userData,
                       void* requestData,
                       uint32  contentLength)
{
    Connection*           connection = (Connection*) userData;
    NB_TileRequestData*   pTileRequest = (NB_TileRequestData*)requestData;
    NB_RasterTileManager* pThis = NULL;
    NB_RasterTileBuffer*  pBuffer = NULL;

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

    //NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "ResponseStatusCallback %d %d %d %d", (int)userData, (int)requestData, errorCode, contentLength);

    if (errorCode == PAL_Ok)
    {
        // Capture contentLength
        pTileRequest->contentLength = contentLength;
    }
    // Don't check for particular error codes, we want to handle any error!
    else
    {
        // Special handling for no traffic/route info
        NB_Error result = (errorCode == PAL_ErrHttpNoContent) ? NE_HTTP_NO_CONTENT : NE_NETSVC;

        /*
            :WARNING:
            In order for the code below to work, the PAL http connection code has to ensure that DataReceivedCallback()
            does NOT get called after ResponseStatusCallback() is called with an error. The code below frees the
            TileRequest structure and it will crash if DataReceivedCallback() gets called after this!

            We could add some special handling here to handle those cases but this would not be trivial at all. When would
            you throw out the TileRequest?
        */

        // We need to make a copy of the tile since TileRequestEnd() frees the tile which we then need to pass into the callback.
        NB_RasterTile tempTile = {0};
        nsl_memcpy(&tempTile, &pTileRequest->tile, sizeof(NB_RasterTile));

        /* RasterTile request life cycle complete. Free it */
        FreeBuffer(pBuffer);

        // Log the received tile
        NB_QaLogRasterTileReply(pThis->context,
            GetQaLogMapType(tempTile.information.type),
            tempTile.information.xPosition,
            tempTile.information.yPosition,
            tempTile.information.zoomLevel,
            pThis->tileConfiguration.tileResolution,
            pThis->tileConfiguration.tileSize,
            tempTile.buffer.size,
            result);

        TileRequestEnd(pThis, pTileRequest);
        connection->onGoingRequests--;

        /*
            :TRICKY:
            Report an error to the user.
            This has to be done AFTER calling TileRequestEnd() and BEFORE calling ProcessNextPendingTile()
            The reason for this is that the map control will retry the tile. If it is called before TileRequestEnd() then
            the tile will still be in the outgoing tile request vector. We also want to do it before calling ProcessNextPendingTile()
            so that we can re-request the tile right away.

        */
        pThis->callbackData.callback(pThis, result, &tempTile, pThis->callbackData.callbackData);

        // Process next pending tile (if any)
        ProcessNextPendingTile(connection);
    }
}


/*! Open a HTTP connection.

    The call is asynchronous and the status callback gets called once the connection was successfully established.
    Once the connection is established it will make a http request with the specified tile information.

    @return NB_Error

    @see NB_RasterTileManagerGetTile
    @see ConnectionStatusCallback
*/
NB_Error
OpenConnection(Connection* connection,                          /*!< Connection to open */
               const NB_RasterTileInformation* information      /*!< Tile information to save in the connection object. This
                                                                     information will be used to make the send request once the
                                                                     connection is successfully established. */
               )
{
    PAL_Error palResult = PAL_Failed;
    NB_RasterTileManager* manager = connection->manager;
    PAL_NetConnectionConfig networkConfiguration = {0};
    uint16 port = 0;
    const char* baseUrl = NULL;

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
    switch (connection->type)
    {
        case NB_RTT_Map:
            baseUrl = manager->dataSourceData.templateMap.baseUrl,
            port = manager->portMap;
            break;

        case NB_RTT_Traffic:
            baseUrl = manager->dataSourceData.templateTraffic.baseUrl,
            port = manager->portTraffic;
            break;

        case NB_RTT_Route:
            baseUrl = manager->dataSourceData.templateRoute.baseUrl,
            port = manager->portRoute;
            break;

        case NB_RTT_Satellite:
            baseUrl = manager->dataSourceData.templateSatellite.baseUrl,
            port = manager->portSatellite;
            break;

        case NB_RTT_Hybrid:
            baseUrl = manager->dataSourceData.templateHybrid.baseUrl,
            port = manager->portHybrid;
            break;

        case NB_RTT_RouteSelector1:
            baseUrl = manager->dataSourceData.templateRouteSelector1.baseUrl,
            port = manager->portRouteSelector1;
            break;

        case NB_RTT_RouteSelector2:
            baseUrl = manager->dataSourceData.templateRouteSelector2.baseUrl,
            port = manager->portRouteSelector2;
            break;

        case NB_RTT_RouteSelector3:
            baseUrl = manager->dataSourceData.templateRouteSelector3.baseUrl,
            port = manager->portRouteSelector3;
            break;

        default:
            return NE_UNEXPECTED;
    }

    if (nsl_strlen(baseUrl) == 0)
    {
        return NE_BADDATA;
    }

    palResult = PAL_NetOpenConnection(connection->connection,
                                      &networkConfiguration,
                                      baseUrl,
                                      port);

    if (palResult != PAL_Ok)
    {
        return NE_NET;
    }

    connection->state = CS_ConnectionPending;

    return NE_OK;
}

/*! Make a new HTTP request to get a tile.

    The receive callback gets called once the data is received. Status callback gets called if an error occurrs.

    @return NB_Error

    @see NB_RasterTileManagerGetTile
    @see ProcessNextPendingTile
    @see DataReceivedCallback
*/
NB_Error
StartRequest(Connection* connection,                            /*!< Connection to use for the request */
             const NB_RasterTileInformation* information        /*!< Tile to request */
             )
{
    PAL_Error palResult                         = PAL_Ok;
    NB_RasterTileManager* manager               = connection->manager;
    char httpRequest[NB_RTDS_TEMPLATE_LENGTH]   = {0};
    const char* templateString                  = NULL;
    boolean* useQuadkey                         = NULL;
    NB_TileRequestData* pTileRequest            = NULL;

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
    switch (connection->type)
    {
        case NB_RTT_Map:
            templateString = manager->dataSourceData.templateMap.templateUrl;
            useQuadkey = &manager->useQuadKeyMap;
            break;

        case NB_RTT_Traffic:
            templateString = manager->dataSourceData.templateTraffic.templateUrl;
            useQuadkey = &manager->useQuadKeyTraffic;
            break;

        case NB_RTT_Route:
            templateString = manager->semiFormattedRouteUrl;
            useQuadkey = &manager->useQuadKeyRoute;
            break;

        case NB_RTT_Satellite:
            templateString = manager->dataSourceData.templateSatellite.templateUrl;
            useQuadkey = &manager->useQuadKeySatellite;
            break;

        case NB_RTT_Hybrid:
            templateString = manager->dataSourceData.templateHybrid.templateUrl;
            useQuadkey = &manager->useQuadKeyHybrid;
            break;

        case NB_RTT_RouteSelector1:
            templateString = manager->semiFormattedRouteSelector1Url;
            useQuadkey = &manager->useQuadKeyRouteSelector1;
            break;

        case NB_RTT_RouteSelector2:
            templateString = manager->semiFormattedRouteSelector2Url;
            useQuadkey = &manager->useQuadKeyRouteSelector2;
            break;

        case NB_RTT_RouteSelector3:
            templateString = manager->semiFormattedRouteSelector3Url;
            useQuadkey = &manager->useQuadKeyRouteSelector3;
            break;

        default:
            nsl_free(pTileRequest);
            return NE_UNEXPECTED;
    }

    if (*useQuadkey)
    {
        char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};

        NB_Error result = CSL_QuadkeyConvertFromXYZ(information->xPosition,
                                                    information->yPosition,
                                                    (uint8)(information->zoomLevel),
                                                    quadkey);
        if (result != NE_OK)
        {
            nsl_free(pTileRequest);
            return result;
        }

        // Format the http request using the quadkey
        nsl_sprintf(httpRequest, templateString, quadkey);
    }
    else
    {
        // Format the http request based on the requested tile information. The order of x/y/z is currently
        // assumed in this order. If it is not then we need to change the logic in FormatSource().
        nsl_sprintf(httpRequest,
                    templateString,
                    information->xPosition,
                    information->yPosition,
                    information->zoomLevel);
    }

    // Log the request
    NB_QaLogRasterTileRequest(manager->context,
                              GetQaLogMapType(information->type),
                              information->xPosition,
                              information->yPosition,
                              information->zoomLevel,
                              manager->tileConfiguration.tileResolution,
                              manager->tileConfiguration.tileSize,
                              httpRequest);

    if (TileRequestBegin(manager, pTileRequest))
    {
        char httpHeader[MAX_HTTP_HEADER_LENGTH] = {0};

        if (nsl_strlen(manager->tileConfiguration.clientGuidData) > 0)
        {
            // Fill http header
            nsl_strlcpy(httpHeader, HTTP_HEADERCLIENT_GUID_HEADER, MAX_HTTP_HEADER_LENGTH);
            nsl_strlcat(httpHeader, COLON, MAX_HTTP_HEADER_LENGTH);
            nsl_strlcat(httpHeader, SPACE, MAX_HTTP_HEADER_LENGTH);
            nsl_strlcat(httpHeader, manager->tileConfiguration.clientGuidData, MAX_HTTP_HEADER_LENGTH);
            nsl_strlcat(httpHeader, RETURN_LINEFEED, MAX_HTTP_HEADER_LENGTH);
        }

        // Make the asynchronous HTTP request. See receive callback for processing the result
        palResult = PAL_NetHttpSend(connection->connection, NULL, 0, HTTP_VERB, httpRequest, NULL, ((nsl_strlen(manager->tileConfiguration.clientGuidData) > 0) ? httpHeader : NULL), pTileRequest);
        if (palResult != PAL_Ok)
        {
            TileRequestEnd(manager, pTileRequest);
            return NE_NET;
        }
        connection->onGoingRequests++;
    }
    else
    {
        nsl_free(pTileRequest);
        return NE_NOMEM;
    }

    return NE_OK;
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
    NB_RasterTileManager* pThis = connection->manager;
    struct CSL_Vector* pendingTilesVector = NULL;
    boolean retStatus = FALSE;

    // This function should only be called when onGoingRequests are within limit of allowedRequests.
    if (!(connection->onGoingRequests < connection->allowedRequests))
    {
        return retStatus;
    }

    // Get correct pending tile vector based on connection type
    pendingTilesVector = GetPendingVector(pThis, connection->type);
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
            NB_RasterTile temporaryTile = {{0}};
            nsl_memcpy(&temporaryTile.information, &tile.information, sizeof(NB_RasterTileInformation));

            // Inform user of failure
            pThis->callbackData.callback(pThis, result, &temporaryTile, pThis->callbackData.callbackData);
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
FindPendingEntry(NB_RasterTileManager* instance,                /*!< Instance */
                 const NB_RasterTileInformation* information,   /*!< Entry to search for */
                 TileWithPriority** pEntry,                     /*!< On return the pointer to the entry. Set to NULL if not needed. */
                 int* pIndex                                    /*!< On return the index of the entry. Set to NULL if not needed */
                 )
{
    int i = 0;
    int length = 0;
    TileWithPriority* pPendingTile = NULL;

    struct CSL_Vector* pendingTileVector = GetPendingVector(instance, information->type);
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
        if (nsl_memcmp(&pPendingTile->information, information, sizeof(NB_RasterTileInformation)) == 0)
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
GetPendingVector(NB_RasterTileManager* instance, NB_RasterTileType type)
{
    switch (type)
    {
        case NB_RTT_Map:        return instance->pendingMapTiles;
        case NB_RTT_Traffic:    return instance->pendingTrafficTiles;
        case NB_RTT_Route:      return instance->pendingRouteTiles;
        case NB_RTT_Satellite:  return instance->pendingSatelliteTiles;
        case NB_RTT_Hybrid:     return instance->pendingHybridTiles;
        case NB_RTT_RouteSelector1:     return instance->pendingRouteSelectorTiles1;
        case NB_RTT_RouteSelector2:     return instance->pendingRouteSelectorTiles2;
        case NB_RTT_RouteSelector3:     return instance->pendingRouteSelectorTiles3;
    }

    return NULL;
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
void
GenerateTileKey(NB_RasterTileManager* instance,                 /*!< Raster tile manager instance */
                const NB_RasterTileInformation* information,    /*!< Tile information for which to generate the key */
                char* key                                       /*!< Pointer to a buffer to receive the key. Has to be at least of size MAXIMUM_TILE_KEY_LENGTH. */
                )
{
    // Format key based on tile information
    nsl_sprintf(key,
                DICTIONARY_FORMAT_KEY,
                information->type,
                information->zoomLevel,
                information->xPosition,
                information->yPosition,
                (information->type == NB_RTT_Route || information->type == NB_RTT_RouteSelector1 ||
                 information->type == NB_RTT_RouteSelector2 || information->type == NB_RTT_RouteSelector3) ? instance->routeString : "");
}

/*! Initialize connections for a specific type.

    Create the array for the connections and open all the connections for this type

    @return TRUE on success, FALSE otherwise

    @see NB_RasterTileManagerCreate
*/
NB_Error
InitializeConnections(NB_RasterTileManager* pThis,      /*!< Raster tile manager instance */
                      uint32 connectionCount,           /*!< Number of connections to create */
                      Connection** connections,         /*!< Pointer to connections array to initialize */
                      NB_RasterTileType type            /*!< Tile type which is used for this connection */
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
        palResult = PAL_NetCreateConnection(NB_ContextGetPal(pThis->context), PNCP_HTTP, &((*connections)[i].connection));
        if (palResult != PAL_Ok)
        {
            return NE_NET;
        }

        // Set the pointer in the connection structure to the parent structure so we can access it in the
        // callback functions.
        (*connections)[i].manager = pThis;

        // Set tile type
        (*connections)[i].type = type;

        // Set on going requests to 0
        (*connections)[i].onGoingRequests = 0;

        // Set number of allowed simultaneous requests on a connection
        (*connections)[i].allowedRequests = MAXIMUM_REQUESTS_PER_CONNECTION;
    }

    return NE_OK;
}

/*! Disconnect any open connections and delete the connections array.

    @return None

    @see NB_RasterTileManagerDestroy
*/
void
CleanupConnections(NB_RasterTileManager* pThis,         /*!< Raster tile manager instance */
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
    }

    if (connections)
    {
        nsl_free(connections);
    }
}

/*! Format the data source URL so that it can be used by the HTTP connections.

    @return NE_OK on success
*/
NB_Error
FormatSource(NB_RasterTileManager* pThis,
             char* templateString,                  /*!< Template string to format, replaced with the formated source template */
             boolean appendToken,                   /*!< map tile access token need to be appended to url */
             NB_RasterTileImageFormat imageFormat,  /*!< Image format used for the tile downloads */
             char* version,                         /*!< Version string, if empty then DEFAULT_VERSION is used */
             boolean* useQuadkey,                   /*!< This flag gets set by this function if the template contains quadkey coordinates */
             NB_RouteId* routeId1,                  /*!< Optional route-ID. If set route-ID parameter will be appended */
             nb_color* routeColor1,                 /*!< Optional route-color. If set route-color parameter will be appended */
             NB_RouteId* routeId2,                  /*!< Optional route-ID. If set route-ID parameter will be appended */
             nb_color* routeColor2                  /*!< Optional route-color. If set route-color parameter will be appended */
             )
{
    // We need to replace all placeholders with appropriate values or '%d' so that we can format it later easily.

    char* lastPosition = templateString;
    char* currentPosition = NULL;

    // Temporary string for result
    char newString[NB_RTDS_TEMPLATE_LENGTH];
    newString[0] = '\0';

    // Search for all placeholder strings
    currentPosition = nsl_strchr(templateString, NB_RTDS_PLACEHOLDER_PREFIX);
    while (currentPosition)
    {
        // Copy everything up to the new placeholder to the new string
        nsl_strlcpy(newString + nsl_strlen(newString), lastPosition, currentPosition - lastPosition + 1);

        // Don't compary prefix
        ++currentPosition;

        // Check which placeholder matches and advance position accordingly

        if (ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_VERSION))
        {
            // Add version
            if( version && version[0] != 0 )
            {
                nsl_strcat(newString, version);
            }
            else
            {
                nsl_strcat(newString, DEFAULT_VERSION);
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_LOCALE))
        {
            // Add locale
            nsl_strcat(newString, DEFAULT_LOCALE);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_FORMAT))
        {
            // Set image format based on the configuration
            switch (imageFormat)
            {
                case NB_RTIF_PNG:   nsl_strcat(newString, IMAGE_FORMAT_PNG);    break;
                case NB_RTIF_GIF:   nsl_strcat(newString, IMAGE_FORMAT_GIF);    break;
                case NB_RTIF_JPG:   nsl_strcat(newString, IMAGE_FORMAT_JPG);    break;

                default:
                    return NE_NOTINIT;
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_X_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_Y_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_ZOOM))
        {
            // The order of x/y/z is currently assumed in this order. If it changes then we
            // need a different logic here.

            // For x/y/zoom we just add '%d' so that we can add them later dynamically using the string format functions
            nsl_strcat(newString, "%d");
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_QUADKEY))
        {
            // Add '%s' so we can later format the string dynamically using the quadkey.
            nsl_strcat(newString, "%s");
            *useQuadkey = TRUE;
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_TILE_SIZE))
        {
            // Take tile size from configuration and add it to formatted string
            char temp[32] = {0};
            nsl_sprintf(temp, "%d", pThis->tileConfiguration.tileSize);
            nsl_strcat(newString, temp);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_RESOLUTION))
        {
            // Add resolution to string
            char temp[32] = {0};
            nsl_sprintf(temp, "%d", pThis->tileConfiguration.tileResolution);
            nsl_strcat(newString, temp);
        }

        lastPosition = currentPosition;
        currentPosition = nsl_strchr(currentPosition, NB_RTDS_PLACEHOLDER_PREFIX);
    }

    // Copy remaining string (if any)
    nsl_strcat(newString, lastPosition);

    // If the route-ID-1 is valid, encode it into base64 and add it to the string
    if (routeId1 && routeId1->data && (routeId1->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId1->data, routeId1->size, &encodedRouteId) == NE_OK)
        {
            nsl_strlcat(newString, NB_RTDS_PARAMETER_ROUTE_ID1, NB_RTDS_TEMPLATE_LENGTH);
            nsl_strlcat(newString, encodedRouteId, NB_RTDS_TEMPLATE_LENGTH);
            nsl_free(encodedRouteId);
        }

        // If route color 1 is set append it
        if (routeColor1)
        {
            // Format color and add to string
            char temp[64] = {0};
            nsl_sprintf(temp,
                        NB_RTDS_COLOR_FORMAT,
                        NB_COLOR_GET_R(*routeColor1),
                        NB_COLOR_GET_G(*routeColor1),
                        NB_COLOR_GET_B(*routeColor1),
                        NB_COLOR_GET_A(*routeColor1));

            nsl_strcat(newString, NB_RTDS_PARAMETER_ROUTE_COLOR1);
            nsl_strcat(newString, temp);
        }
    }


    // If the route-ID-2 is valid, encode it into base64 and add it to the string
    if (routeId2 && routeId2->data && (routeId2->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId2->data, routeId2->size, &encodedRouteId) == NE_OK)
        {
            nsl_strcat(newString, NB_RTDS_PARAMETER_ROUTE_ID2);
            nsl_strcat(newString, encodedRouteId);
            nsl_free(encodedRouteId);
        }

        // If route color 2 is set append it
        if (routeColor2)
        {
            // Format color and add to string
            char temp[64] = {0};
            nsl_sprintf(temp,
                        NB_RTDS_COLOR_FORMAT,
                        NB_COLOR_GET_R(*routeColor2),
                        NB_COLOR_GET_G(*routeColor2),
                        NB_COLOR_GET_B(*routeColor2),
                        NB_COLOR_GET_A(*routeColor2));

            nsl_strcat(newString, NB_RTDS_PARAMETER_ROUTE_COLOR2);
            nsl_strcat(newString, temp);
        }
    }

    if (appendToken && !nsl_strempty(pThis->mapTileAccessToken))
    {
        if (nsl_strlen(newString) + nsl_strlen(pThis->mapTileAccessToken) < (sizeof(newString)-1) )
        {
            /* there is enough space to append '&' and token */
            nsl_strcat(newString, "&");
            nsl_strcat(newString, pThis->mapTileAccessToken);
        }
        else
        {
            /* there is not enough space to append '&' and token */
            return NE_INVAL;
        }
    }

    // Replace old string with new formatted string
    templateString[0] = '\0';
    nsl_strcpy(templateString, newString);

    return NE_OK;
}


/*! Format the data source URL so that it can be used by the HTTP connections.

    @return NE_OK on success
*/
static NB_Error FormatSourceRouteSelector(NB_RasterTileManager* pThis,
                             char* templateString,
                             boolean appendToken,
                             NB_RasterTileImageFormat imageFormat,
                             char* version,
                             boolean* useQuadkey,
                             NB_RouteId* routeId1,
                             NB_RouteId* routeId2,
                             NB_RouteId* routeId3
                             )
{
    // We need to replace all placeholders with appropriate values or '%d' so that we can format it later easily.

    char* lastPosition = templateString;
    char* currentPosition = NULL;

    // Temporary string for result
    char newString[NB_RTDS_TEMPLATE_LENGTH];
    newString[0] = '\0';

    // Search for all placeholder strings
    currentPosition = nsl_strchr(templateString, NB_RTDS_PLACEHOLDER_PREFIX);
    while (currentPosition)
    {
        // Copy everything up to the new placeholder to the new string
        nsl_strlcpy(newString + nsl_strlen(newString), lastPosition, currentPosition - lastPosition + 1);

        // Don't compary prefix
        ++currentPosition;

        // Check which placeholder matches and advance position accordingly

        if (ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_VERSION))
        {
            // Add version
            if( version && version[0] != 0 )
            {
                nsl_strcat(newString, version);
            }
            else
            {
                nsl_strcat(newString, DEFAULT_VERSION);
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_LOCALE))
        {
            // Add locale
            nsl_strcat(newString, DEFAULT_LOCALE);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_FORMAT))
        {
            // Set image format based on the configuration
            switch (imageFormat)
            {
                case NB_RTIF_PNG:   nsl_strcat(newString, IMAGE_FORMAT_PNG);    break;
                case NB_RTIF_GIF:   nsl_strcat(newString, IMAGE_FORMAT_GIF);    break;
                case NB_RTIF_JPG:   nsl_strcat(newString, IMAGE_FORMAT_JPG);    break;

                default:
                    return NE_NOTINIT;
            }
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_X_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_Y_COORDINATE) ||
                ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_ZOOM))
        {
            // The order of x/y/z is currently assumed in this order. If it changes then we
            // need a different logic here.

            // For x/y/zoom we just add '%d' so that we can add them later dynamically using the string format functions
            nsl_strcat(newString, "%d");
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_QUADKEY))
        {
            // Add '%s' so we can later format the string dynamically using the quadkey.
            nsl_strcat(newString, "%s");
            *useQuadkey = TRUE;
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_TILE_SIZE))
        {
            // Take tile size from configuration and add it to formatted string
            char temp[32] = {0};
            nsl_sprintf(temp, "%d", pThis->tileConfiguration.tileSize);
            nsl_strcat(newString, temp);
        }
        else if(ProcessPlaceholder(&currentPosition, NB_RTDS_PLACEHOLDER_RESOLUTION))
        {
            // Add resolution to string
            char temp[32] = {0};
            nsl_sprintf(temp, "%d", pThis->tileConfiguration.tileResolution);
            nsl_strcat(newString, temp);
        }

        lastPosition = currentPosition;
        currentPosition = nsl_strchr(currentPosition, NB_RTDS_PLACEHOLDER_PREFIX);
    }

    // Copy remaining string (if any)
    nsl_strcat(newString, lastPosition);

    // If the route-ID-1 is valid, encode it into base64 and add it to the string
    if (routeId1 && routeId1->data && (routeId1->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId1->data, routeId1->size, &encodedRouteId) == NE_OK)
        {
            nsl_strlcat(newString, NB_RTDS_PARAMETER_ROUTE_ID1, NB_RTDS_TEMPLATE_LENGTH);
            nsl_strlcat(newString, encodedRouteId, NB_RTDS_TEMPLATE_LENGTH);
            nsl_free(encodedRouteId);
        }
    }

    // If the route-ID-2 is valid, encode it into base64 and add it to the string
    if (routeId2 && routeId2->data && (routeId2->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId2->data, routeId2->size, &encodedRouteId) == NE_OK)
        {
            nsl_strcat(newString, NB_RTDS_PARAMETER_ROUTE_ID2);
            nsl_strcat(newString, encodedRouteId);
            nsl_free(encodedRouteId);
        }
    }

    // If the route-ID-3 is valid, encode it into base64 and add it to the string
    if (routeId3 && routeId3->data && (routeId3->size > 0))
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)routeId3->data, routeId3->size, &encodedRouteId) == NE_OK)
        {
            nsl_strcat(newString, NB_RTDS_PARAMETER_ROUTE_ID3);
            nsl_strcat(newString, encodedRouteId);
            nsl_free(encodedRouteId);
        }
    }

    if (appendToken && !nsl_strempty(pThis->mapTileAccessToken))
    {
        if (nsl_strlen(newString) + nsl_strlen(pThis->mapTileAccessToken) < (sizeof(newString)-1) )
        {
            /* there is enough space to append '&' and token */
            nsl_strcat(newString, "&");
            nsl_strcat(newString, pThis->mapTileAccessToken);
        }
        else
        {
            /* there is not enough space to append '&' and token */
            return NE_INVAL;
        }
    }

    // Replace old string with new formatted string
    templateString[0] = '\0';
    nsl_strcpy(templateString, newString);

    return NE_OK;
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
void
ExtractPortsAndFormatBaseUrl(NB_RasterTileManager* pThis,
                             char* baseUrl,                     /*!< Url to format */
                             char* template,                    /*!< Template to format */
                             uint16* port                       /*!< On Return the extracted port */
                             )
{
    /*
        The code below makes a lot of assumptions on how the server returns the data.
        E.g. it is assumed that the colon (if any) is always in the base url. It is
        also assumed that the port is at the beginning of template string. If any of
        those assumptions are not valid anymore then the code needs to change to reflect this.
    */

    char tempUrl[NB_RTDS_BASE_URL_LENGTH]       = {0};
    char tempTemplate[NB_RTDS_TEMPLATE_LENGTH]  = {0};
    char* slashPosition     = NULL;
    char* removePosition    = NULL;
    char* colonPosition     = NULL;

    // Remove "http://" if present. Our connect will fail if it is present.
    nsl_strlcpy(tempUrl, baseUrl, sizeof(tempUrl));
    removePosition = nsl_stristr(tempUrl, BASE_URL_REMOVE);
    if (removePosition)
    {
        nsl_strcpy(baseUrl, removePosition + nsl_strlen(BASE_URL_REMOVE));
    }

    // Look for any colon
    colonPosition = nsl_strchr(baseUrl, ':');
    if (colonPosition)
    {
        // Just remove the colon and everything after it.
        *colonPosition = '\0';
    }

    // Get the ports from the template strings. If there is no port then it is set to zero which
    // is ok since that's the value to use the default port.
    *port = (uint16)nsl_atoi(template);

    // Remove port if present
    if (*port != 0)
    {
        nsl_strlcpy(tempTemplate, template, NB_RTDS_TEMPLATE_LENGTH);
        slashPosition = nsl_strchr(tempTemplate, '/');
        if (slashPosition)
        {
            nsl_strcpy(template, slashPosition);
        }
    }
}

/*! Replace tile size in given template string.

    :KLUDGE:
    This function is a kludge to fix the discrepancy between the hybrid/satellite and map/traffic/route templates.
    The map/traffic/route templates use the tile size given in the rastertile configuration. The hybrid/satellite
    template have the tile size set by the server (dependent on the given device resolution in the data source
    request). This does not work with the current interface. We would have to take the tile size from the data
    source and remove it from the raster tile configuration. For now we just overwrite the tile size for
    hybrid and satellite templates.

    @return None
*/
void
ReplaceTileSizeString(NB_RasterTileManager* pThis,
                      char* template                    /*!< Template string to modify */
                      )
{
    char* pSize = nsl_strstr(template, NB_RTDS_PARAMETER_HYBRID_SIZE);
    if (pSize)
    {
        int oldSize = 0;
        pSize += nsl_strlen(NB_RTDS_PARAMETER_HYBRID_SIZE);

        oldSize = nsl_atoi(pSize);
        if ((uint32)oldSize != pThis->tileConfiguration.tileSize)
        {
            int newSizeLength = 0;
            int oldSizeLength = 0;
            char newSizeString[32] = {0};
            char oldSizeString[32] = {0};

            nsl_sprintf(newSizeString, "%d", pThis->tileConfiguration.tileSize);
            nsl_sprintf(oldSizeString, "%d", oldSize);
            newSizeLength = nsl_strlen(newSizeString);
            oldSizeLength = nsl_strlen(oldSizeString);

            // If the string length of the new and old are different then we need to copy the string following our
            // replace string
            if (newSizeLength != oldSizeLength)
            {
                // Make sure to use memmove and not strcpy since the memory overlaps. Include NULL termination
                nsl_memmove(pSize + newSizeLength, pSize + oldSizeLength, nsl_strlen(pSize + oldSizeLength) + 1);
            }

            // Replace old with new tile size
            nsl_strncpy(pSize, newSizeString, nsl_strlen(newSizeString));
        }
    }
}

/*! Ensure that the current cache is not "out-of-date".

    If we switch to a different server for any of the data sources or the generation info for a data source
    changes then we discard the existing cache to avoid having mismatched tiles from seperate sources.

    This function should be called during startup to validate the cache.

    @return None
*/
void
ValidateCache(NB_RasterTileManager* pThis)
{
    /*
        We save the checksum of the generation info and base URLs as user-date in the cache.
    */

    nb_boolean deleteCache = TRUE;      // Delete the cache unless we verify that it is valid.
    byte* userData = NULL;
    size_t userSize = 0;

    if (CSL_CacheGetUserData(pThis->cache, &userData, &userSize) == NE_OK)
    {
        // Double check the length, it should always match
        if (userData && (userSize == CACHE_USER_DATA_SIZE))
        {
            // Compare checksum, if it matches then we keep the cache and don't delete it
            uint32 newChecksum = GenerateChecksumFromDataSource(pThis);
            if (nsl_memcmp(&newChecksum, userData, sizeof(newChecksum)) == 0)
            {
                deleteCache = FALSE;
            }
        }
    }

    if (deleteCache)
    {
        uint32 checksum = 0;

        // Delete the existing cache
        CSL_CacheClear(pThis->cache);

        // Set the new checksum as user data. The cache will save it to the index-file.
        checksum = GenerateChecksumFromDataSource(pThis);
        CSL_CacheSetUserData(pThis->cache, (byte*)&checksum, CACHE_USER_DATA_SIZE);
    }
}

/*! Generate a checksum value from the data sources and generation info.

    @see ValidateCache
    @return checksum
*/
uint32
GenerateChecksumFromDataSource(NB_RasterTileManager* pThis)
{
    uint32 checksum = 0;

    // Allocate dynamic buffer. The size is not important, it will grow if needed.
    struct dynbuf buffer = {0};
    if (dbufnew(&buffer, 512) == NE_OK)
    {
        // Concatenate all base URLs and all generation infos together.
        Concatenate(&buffer, pThis->dataSourceData.templateMap.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateMap.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateRoute.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateRoute.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateTraffic.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateTraffic.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateSatellite.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateSatellite.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector1.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector1.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector2.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector2.generationInfo);

        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector3.baseUrl);
        Concatenate(&buffer, pThis->dataSourceData.templateRouteSelector3.generationInfo);

        /* include map tile access token in check-sum calculation */
        Concatenate(&buffer, pThis->mapTileAccessToken);

        // When the tile size changes then we need to clear the cache
        dbufcat(&buffer, (const byte*)&pThis->tileConfiguration.tileSize, sizeof(pThis->tileConfiguration.tileSize));

        // Generate a check-sum from the data
        checksum = crc32(CRC_VALUE, dbufget(&buffer), dbuflen(&buffer));

        dbufdel(&buffer);
    }

    return checksum;
}

/*! Simple helper function to concatenate a string to a dynamic buffer.

    @see CompareWithNull
    @return None
*/
void
Concatenate(struct dynbuf* buffer,      /*!< Buffer to which to add the data. */
            const char* pSource         /*!< String to concatenate. */
            )
{
    dbufcat(buffer, (const byte*)pSource, nsl_strlen(pSource));
}

/*! Convert map type to QA compatible type.

    @return converted type
*/
NB_QaLogMapType
GetQaLogMapType(NB_RasterTileType type)
{
    switch (type)
    {
        case NB_RTT_Map:        return NB_QLMT_Map;
        case NB_RTT_Traffic:    return NB_QLMT_Traffic;
        case NB_RTT_Route:      return NB_QLMT_Route;
        case NB_RTT_Satellite:  return NB_QLMT_Satellite;
        case NB_RTT_Hybrid:     return NB_QLMT_Hybrid;
        case NB_RTT_RouteSelector1:     return NB_QLMT_PTRouteSelector1;
        case NB_RTT_RouteSelector2:     return NB_QLMT_PTRouteSelector2;
        case NB_RTT_RouteSelector3:     return NB_QLMT_PTRouteSelector3;
    }

    return NB_QLMT_None;
}

/*! Log the cache statistics.

    Call this function each time the cache state changes.

    @return None
*/
void
LogCacheStatistics(NB_RasterTileManager* pThis)
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
                                 0);                        // @todo (BUG 56064): We currently don't have a count in the cache for discarded items.
    }
}

/*! Get cache statistics.

    This is used for both LogCacheStatistics() and NB_RasterTileManagerGetDebugInformation().

    @see LogCacheStatistics
    @see NB_RasterTileManagerGetDebugInformation
*/
void
GetCacheStatistics(NB_RasterTileManager* pThis,
                   int* memoryCacheCount,
                   int* persistentCacheCount,
                   int* protectedCount,
                   int* pendingCount)
{
    // Get all pending tiles
    if (pendingCount)
    {
        int i = 0;

        // Add all pending tiles
        *pendingCount = CSL_VectorGetLength(pThis->pendingMapTiles)
                      + CSL_VectorGetLength(pThis->pendingTrafficTiles)
                      + CSL_VectorGetLength(pThis->pendingRouteTiles)
                      + CSL_VectorGetLength(pThis->pendingSatelliteTiles);

        /*
            We also want to include all ongoing requests. These are not included in the pending vectors.
        */
        if (pThis->mapConnections)
        {
            for (i = 0; i < pThis->tileConfiguration.mapConfiguration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->mapConnections[i].onGoingRequests;
            }
        }
        if (pThis->trafficConnections)
        {
            for (i = 0; i < pThis->tileConfiguration.trafficConfiguration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->trafficConnections[i].onGoingRequests;
            }
        }
        if (pThis->routeConnections)
        {
            for (i = 0; i < pThis->tileConfiguration.routeConfiguration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->routeConnections[i].onGoingRequests;
            }
        }
        if (pThis->satelliteConnections)
        {
            for (i = 0; i < pThis->tileConfiguration.satelliteConfiguration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->satelliteConnections[i].onGoingRequests;
            }
        }
        if (pThis->routeSelectorConnections1)
        {
            for (i = 0; i < pThis->tileConfiguration.routeAlternate1Configuration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->routeSelectorConnections1[i].onGoingRequests;
            }
        }
        if (pThis->routeSelectorConnections2)
        {
            for (i = 0; i < pThis->tileConfiguration.routeAlternate2Configuration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->routeSelectorConnections2[i].onGoingRequests;
            }
        }
        if (pThis->routeSelectorConnections3)
        {
            for (i = 0; i < pThis->tileConfiguration.routeAlternate3Configuration.maximumConcurrentRequests; ++i)
            {
                *pendingCount += pThis->routeSelectorConnections3[i].onGoingRequests;
            }
        }
    }

    CSL_CacheGetStats(pThis->cache, NULL, NULL, NULL, memoryCacheCount, persistentCacheCount, protectedCount);
}

/*! Free a route if it is set.

    @return None
*/
void
FreeRoute(NB_RouteId* route)
{
    if (route->data)
    {
        nsl_free(route->data);
        route->data = NULL;
        route->size = 0;
    }
}

/*! Copy a route if the source is valid.

    @return NB_Error
*/
NB_Error
SetRoute(NB_RouteId* destination,       /*!< Has to be valid */
         const NB_RouteId* source       /*!< Can point to an empty route */
         )
{
    if (source->data && (source->size > 0))
    {
        destination->data = nsl_malloc(source->size);
        if (!destination->data)
        {
            return NE_NOMEM;
        }
        nsl_memcpy(destination->data, source->data, source->size);
        destination->size = source->size;
    }

    return NE_OK;
}

/*! Free a tile buffer.

    @return None
*/
void
FreeBuffer(NB_RasterTileBuffer* buffer)
{
    if (buffer->buffer)
    {
        nsl_free(buffer->buffer);
        buffer->buffer = NULL;
        buffer->size = 0;
    }
}

/*! @} */



