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

    @file     TestRasterTile.c
*/
/*
    See description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "testrastertile.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "palfile.h"
#include "palstdlib.h"

#include "nbcontext.h"
#include "nbcontextaccess.h"

#include "nbrastertilemanager.h"
#include "nbrastertiledatasourcehandler.h"
#include "nbrastertiledatasourceparameters.h"
#include "nbrastertiledatasourceinformation.h"

#include "nbrouteinformation.h"
#include "nbroutehandler.h"
#include "nbrouteparameters.h"

#include "nbguidanceinformation.h"
#include "testdirection.h"


// Defines/Constants .............................................................................

// Comment this define if you want to keep the images after the tests have run
/// @todo Make this a command line option
#define CLEANUP_IMAGE_FILES

// Define to decide the stress test. Large number might not work on windows mobile (only on win32)
// because of the large amount of data.
#define PERFORM_LONG_STRESS_TEST

#ifdef PERFORM_LONG_STRESS_TEST
    const int STRESS_TEST_MULTIPLIER = 20;
#else
    const int STRESS_TEST_MULTIPLIER = 1;
#endif

#define TILE_PREFIX            "TestTile"
#define TILE_NAME_PNG          TILE_PREFIX "%03d.png"

const uint32 CALLBACK_TIMEOUT = 20 * 1000;
const uint32 DEFAULT_RESOLUTION = 96;


// Local types ...................................................................................

/*! Data about a tile. */
typedef struct
{
    int priority;
    NB_RasterTileInformation information;

} TileData;

/*! Test data passed to the receive callback. */
typedef struct
{
    PAL_Instance* pal;

    // Number of tiles requested for the current test. Gets set in the test and checked in the callback.
    int requestedTiles;
    int currentTileCount;

#ifdef PERFORM_LONG_STRESS_TEST
    /*! Array for tile information. We set this in the callback and check the results in the test functions. */
    TileData receivedTiles[10000];
#else
    TileData receivedTiles[500];
#endif

} TestData;

typedef struct TestRouteSelectorObj
{
    NB_RouteHandler* routeHandler;
    NB_RouteInformation* route;
}TestRouteSelectorObj;

// Variables .....................................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

// Counter for all image files. 
static int g_ImageNameCounter = 0;


// Private Functions .............................................................................

// Smoke tests
static void TestRasterTileCancelRequest(void);
static void TestRasterTileClearCache(void);
static void TestRasterTileInitializeDataSource(void);
static void TestRasterTileReceiveOne(void);
static void TestRasterTileReceiveMultiple(void);
static void TestRasterTileReceiveTrafficOne(void);
static void TestRasterTileReceiveTrafficMultiple(void);
static void TestRasterTileReceiveSatelliteOne(void);
static void TestRasterTileCacheOne(void);
static void TestRasterTileCacheMultiple(void);
static void TestRasterTileRouteTiles(void);
static void TestRasterTileRouteSelectorTiles(void);

// Intermediate tests
static void TestRasterTileBulkRequest(void);
static void TestRasterTileReuseConnection(void);
static void TestRasterTilePendingTiles(void);
// Tests have been removed for http://codereview.nimone.com/r/26379/
// ToDo: change these tests for new implementation from BR#93445
// static void TestRasterTilePendingTilesSorting(void);
// static void TestRasterTilePendingTilesResorting(void);
// static void TestRasterTileExceedLockedList(void);
// static void TestRasterTilePersistentCache(void);
// static void TestRasterTileFlushTiles(void);
static void TestRasterTileForceDownload(void);
static void TestRasterTileFailedTiles(void);
static void TestRasterTileAccessToken(void);

// Full tests
static void TestRasterTileStressTest(void);

// Callback functions for different tests
static void DataSourceHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static void DownloadCallback(NB_RasterTileManager* manager, NB_Error result, const NB_RasterTile* tile, void* userData);
static void LockedTilesDownloadCallback(NB_RasterTileManager* manager, NB_Error result, const NB_RasterTile* tile, void* userData);
static void FailedTilesDownloadCallback(NB_RasterTileManager* manager, NB_Error result, const NB_RasterTile* tile, void* userData);

// Helper functions
static NB_RasterTileDataSourceData* RetrieveDataSource(NB_Context* context);
static void CleanupImageFiles();
static void CleanupCacheFiles();
static void RequestTile(NB_RasterTileManager* manager, NB_RasterTile* tile, uint32 priority, int count);
static void VerifyTileFromPersistentCache(NB_RasterTileManager* manager, uint32 zoom, uint32 x, uint32 y);
static void VerifyTile(const NB_RasterTileInformation* information, uint32 zoom, uint32 x, uint32 y);

static void InitializeConfiguration(NB_RasterTileConfiguration* configuration,
                                    uint32 tileSize,
                                    uint16 maximumConcurrentRequestsMap,
                                    uint16 maximumConcurrentRequestsRoute,
                                    uint16 maximumConcurrentRequestsTraffic,
                                    uint16 maximumConcurrentRequestsSatellite,
                                    uint16 maximumpendingRequestsMap,
                                    uint16 maximumpendingRequestsRoute,
                                    uint16 maximumpendingRequestsTraffic,
                                    uint16 maximumpendingRequestsSatellite);

static void SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config);
static void SetupPlaces(NB_Place* origin, NB_Place* dest);
static void DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);

// Public Functions ..............................................................................

/*! Add all your test functions here

@return None
*/
void 
TestRasterTile_AddTests( CU_pSuite pTestSuite, int level )
{
    // Smoke tests
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileCancelRequest",				TestRasterTileCancelRequest);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileClearCache",					TestRasterTileClearCache);

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileInitializeDataSource",       TestRasterTileInitializeDataSource);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileReceiveOne",                 TestRasterTileReceiveOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileReceiveMultiple",            TestRasterTileReceiveMultiple);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileReceiveTrafficOne",          TestRasterTileReceiveTrafficOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileReceiveTrafficMultiple",     TestRasterTileReceiveTrafficMultiple);

/*
    Satellite tiles are currently not supported. Enable test again once supported by the server.

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileReceiveSatelliteOne",        TestRasterTileReceiveSatelliteOne);
*/

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileCacheOne",                   TestRasterTileCacheOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileCacheMultiple",              TestRasterTileCacheMultiple);

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileRouteTiles",                 TestRasterTileRouteTiles);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterTileRouteSelectorTiles",         TestRasterTileRouteSelectorTiles);


    // Intermediate tests
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileBulkRequest",         TestRasterTileBulkRequest);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileReuseConnection",     TestRasterTileReuseConnection);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTilePendingTiles",        TestRasterTilePendingTiles);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTilePendingTilesSorting", TestRasterTilePendingTilesSorting);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTilePendingTilesResorting",TestRasterTilePendingTilesResorting);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileExceedLockedList",    TestRasterTileExceedLockedList);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTilePersistentCache",     TestRasterTilePersistentCache);
//    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileFlushTiles",          TestRasterTileFlushTiles);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileForceDownload",       TestRasterTileForceDownload);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileFailedTiles",         TestRasterTileFailedTiles);

/*
    Access token is currently not supported for the product name used for system tests. Enable again when supported.

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterTileAccessToken",         TestRasterTileAccessToken);
*/

    // Full tests
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestRasterTileStressTest",  TestRasterTileStressTest);
};


/*! Add common initialization code here.

@return 0

@see TestRasterTile_SuiteCleanup
*/
int 
TestRasterTile_SuiteSetup()
{
    // Get rid of any existing images before we run the test
    CleanupImageFiles();

    CleanupCacheFiles();

    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();

    TestNetwork_SuiteSetup();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestRasterTile_SuiteSetup
*/
int 
TestRasterTile_SuiteCleanup()
{
#ifdef CLEANUP_IMAGE_FILES
    CleanupImageFiles();
#endif

    CleanupCacheFiles();

    DestroyCallbackCompletedEvent(g_CallbackEvent);

    TestNetwork_SuiteCleanup();
    return 0;
}


// Private Functions .............................................................................

/*! Test initialization of the data sources 

    @return None. Asserts are called on failure.
*/
void
TestRasterTileInitializeDataSource(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileDataSourceData* dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            nsl_free(dataSourceData);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

/*! Test raster tile cancel request.

    @return None. Asserts are called on failure.
*/
void
TestRasterTileCancelRequest(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_Error result = NE_OK;
        NB_RasterTileDataSourceParameters* parameters = NULL;
        NB_RasterTileDataSourceHandler* handler = NULL;
        NB_RequestHandlerCallback callbackData = {0};

        NB_RasterTileDataSourceData* dataSourceData = nsl_malloc(sizeof(NB_RasterTileDataSourceData));
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);

        if (dataSourceData)
        {
            nsl_memset(dataSourceData, 0, sizeof(NB_RasterTileDataSourceData));

            callbackData.callback = &DataSourceHandlerCallback;

            // Pass NB_RasterTileDataSourceData as callback data. It gets set in the callback
            callbackData.callbackData = dataSourceData;                       

            // Create handler to request data sources
            result = NB_RasterTileDataSourceHandlerCreate(context, &callbackData, &handler);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Create parameters used to make data source request
                result = NB_RasterTileDataSourceParametersCreate(context, &parameters);

                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    // Should get an error if cancel before starting request
                    result = NB_RasterTileDataSourceHandlerCancelRequest(handler);
                    CU_ASSERT_EQUAL(result, NE_UNEXPECTED);

                    // Make data source request. See DataSourceHandlerCallback() for handling of the result
                    result = NB_RasterTileDataSourceHandlerStartRequest(handler, parameters);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    if(result == NE_OK)
                    {
                        result = NB_RasterTileDataSourceHandlerIsRequestInProgress(handler);
                        CU_ASSERT_EQUAL(result, TRUE)

                        result = NB_RasterTileDataSourceHandlerCancelRequest(handler);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        if(result == NE_OK)
                        {
                            // Should get an error if cancel again
                            result = NB_RasterTileDataSourceHandlerCancelRequest(handler);
                            CU_ASSERT_EQUAL(result, NE_UNEXPECTED);
                        }
                    }
                }
            }
            nsl_free(dataSourceData);
        }

        if (parameters)
        {
            result = NB_RasterTileDataSourceParametersDestroy(parameters);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (handler)
        {
            result = NB_RasterTileDataSourceHandlerDestroy(handler);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

/*! Test adding one item to the cache, retrieve it and clear.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileClearCache(void)
{
    nb_boolean isTileAvailable = FALSE;
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
		CU_ASSERT_PTR_NOT_NULL(testData);
		if (!testData)
		{
			CU_FAIL("Free memory do not available");
			return;
		}
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("Raster tile download timed out!");
                    }
                }

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_None, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Map, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, TRUE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_BirdsEye, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Terrain, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Satellite, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Hybrid, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Traffic, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);    

                result = NB_RasterTileManagerTileTypeIsAvailable(manager, NB_RTT_Route, &isTileAvailable);
                CU_ASSERT_EQUAL(isTileAvailable, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Get the same tile again. It should now be in the memory-cache.
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_MemoryCache, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                result = NB_RasterTileManagerClearCache(manager, NB_RTCC_UnlockedOnly);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_PersistentCache, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_Download, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                result = NB_RasterTileManagerClearCache(manager, NB_RTCC_All);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerClearCache(manager, NB_RTCC_MemoryOnly);
                CU_ASSERT_EQUAL(result, NE_OK);

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test raster tile retrieval.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReceiveOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("Raster tile download timed out!");
                    }
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of multiple tiles at the same time.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReceiveMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 5 tiles. All 5 tiles should be downloaded simultaneoulsy.
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 10, 1);
                RequestTile(manager, &tile, 10, 2);
                RequestTile(manager, &tile, 10, 3);
                RequestTile(manager, &tile, 10, 4);
                RequestTile(manager, &tile, 10, 5);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of one traffic tiles.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReceiveTrafficOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 3, 3, 0, 10, 10, 10, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Downtown L.A. should always have traffic information
                const uint32 ZOOM = 12;
                const uint32 X_POSITION_START = 702;
                const uint32 Y_POSITION_START = 1635;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Traffic;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("Raster tile download timed out!");
                    }
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of multiple traffic tiles at the same time.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReceiveTrafficMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 20;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 3, 3, 0, 30, 30, 30, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Traffic;
                tile.information.zoomLevel = 12;

                // Request 20 tiles. Request traffic information from tiles which cover freeways in the L.A. area where
                // we know we should receive traffic.

                tile.information.xPosition = 700;
                tile.information.yPosition = 1632;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 700;
                tile.information.yPosition = 1633;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1632;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1633;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 702;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 702;
                tile.information.yPosition = 1635;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1635;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1636;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                // I-10 near Santa Monica/L.A.

                tile.information.zoomLevel = 13;

                tile.information.xPosition = 1400;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1401;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1402;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1403;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1404;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1405;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1404;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1405;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1406;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                tile.information.xPosition = 1406;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer), NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of one satellite map tile.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReceiveSatelliteOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 1, 20, 0, 0, 1);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            if (nsl_strempty(dataSourceData->templateSatellite.baseUrl) &&
                nsl_strempty(dataSourceData->templateSatellite.templateUrl))
            {
                nsl_free(dataSourceData);
                CU_FAIL("Satellite map data source URL empty, Server does not support satellite map?");
            }
            else
            {
                result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (result == NE_OK)
                {
                    const uint32 ZOOM = 16;
                    const uint32 X_POSITION_START = 11342;
                    const uint32 Y_POSITION_START = 26264;

                    NB_RasterTile tile = {{0}};
                    tile.information.type = NB_RTT_Satellite;
                    tile.information.zoomLevel = ZOOM;
                    tile.information.xPosition = X_POSITION_START;
                    tile.information.yPosition = Y_POSITION_START;

                    // Request one tile
                    RequestTile(manager, &tile, 10, 1);

                    // The first request should not return the buffer. It will be returned in the callback
                    CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                    CU_ASSERT_EQUAL(tile.buffer.size, 0);

                    if (result == NE_OK)
                    {
                        // Wait for the callback to trigger the event
                        if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                        {
                            // Validate the tile. x-position will be incremented by one
                            VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                            // Don't unlock tile here, it gets done in the callback.
                        }
                        else
                        {
                            CU_FAIL("Raster tile download timed out!");
                        }
                    }

                    result = NB_RasterTileManagerDestroy(manager);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

                nsl_free(dataSourceData);
            }
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test adding one item to the cache and retrieve it.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileCacheOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("Raster tile download timed out!");
                    }
                }

                // Get the same tile again. It should now be in the memory-cache.
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_MemoryCache, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_PersistentCache, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_Download, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test caching of multiple entries

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileCacheMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                int i = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 5 tiles. All 5 tiles should be downloaded simultaneoulsy.
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 10, 1);
                RequestTile(manager, &tile, 10, 2);
                RequestTile(manager, &tile, 10, 3);
                RequestTile(manager, &tile, 10, 4);
                RequestTile(manager, &tile, 10, 5);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                tile.information.xPosition = X_POSITION_START;

                // Get the same tiles again. It should now be in the memory-cache.
                for (i = 0; i < 5; ++i)
                {
                    tile.information.xPosition++;
                    tile.buffer.buffer = NULL;
                    tile.buffer.size = 0;

                    result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_MemoryCache, &tile.buffer);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                    CU_ASSERT(tile.buffer.size > 0);
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

void TestRasterTileRouteTiles(void)
{
    NB_Error                  result   = NE_OK;
    PAL_Instance*             pal      = NULL;
    NB_Context*               context  = NULL;

    TestRouteSelectorObj      routeObj = {0};

    NB_RouteParameters*       routeParameters = 0;
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};

    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart       = {{0}};
    NB_Place                  pDest        = {{0}};

    NB_RouteId*               routeId      = NULL;
    NB_RouteId                emptyRouteID = {0};

    NB_RasterTileManager* manager          = NULL;
    NB_RasterTileDataSourceData* dataSourceData = NULL;
    NB_RasterTileConfiguration configuration    = {0};
    NB_RasterTileManagerCallback callbackData   = {0};

    NB_RequestHandlerCallback routeHandlerCallback = {DownloadRouteCallback, 0 };
    TestData* testData = nsl_malloc(sizeof(*testData));

    nsl_memset(testData, 0, sizeof(*testData));

    testData->pal = pal;
    testData->requestedTiles = 5;
    callbackData.callback = &DownloadCallback;
    callbackData.callbackData = testData;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        InitializeConfiguration(&configuration, 256, 5, 5, 0, 0, 20, 20, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);

        if (NULL != routeParameters && NULL != routeObj.routeHandler && NULL != dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
                {
                    LOGOUTPUT(LogLevelHigh, ("TestRasterTileRouteTiles::Callback Successful\n"));
                    NB_RouteInformationGetRouteId(routeObj.route, &routeId);

                    result = NB_RasterTileManagerSetRoutes(manager,
                                                           *routeId,
                                                           MAKE_NB_COLOR(255,0,0),
                                                           emptyRouteID,
                                                           MAKE_NB_COLOR(0,0,0));
                    CU_ASSERT_EQUAL(result, NE_OK);

                    if (result == NE_OK)
                    {
                        const uint32 ZOOM = 16;
                        const uint32 X_POSITION_START = 11339;
                        const uint32 Y_POSITION_START = 26274;

                        NB_RasterTile tile = {{0}};
                        tile.information.type = NB_RTT_Route;
                        tile.information.zoomLevel = ZOOM;
                        tile.information.xPosition = X_POSITION_START;
                        tile.information.yPosition = Y_POSITION_START;

                        // The x-position get incremented by the function
                        RequestTile(manager, &tile, 10, 1);
                        RequestTile(manager, &tile, 10, 2);
                        RequestTile(manager, &tile, 10, 3);
                        RequestTile(manager, &tile, 10, 4);
                        RequestTile(manager, &tile, 10, 5);

                        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 40000))
                        {
                            // All tiles are downloaded successful.
                        }
                        else
                        {
                            CU_FAIL("Raster tile download timed out!");
                        }
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestRasterTileRouteTiles:: Failed to create instances\n"));
            }
        }
    }

    //Cleaning up
    result = NB_RasterTileManagerDestroy(manager);
    CU_ASSERT_EQUAL(result, NE_OK);

    nsl_free(dataSourceData);
    nsl_free(testData);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    NB_ContextDestroy(context);
    PAL_DestroyInstance(pal);

    CleanupCacheFiles();

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

void TestRasterTileRouteSelectorTiles(void)
{
    NB_Error                  result   = NE_OK;
    PAL_Instance*             pal      = NULL;
    NB_Context*               context  = NULL;

    TestRouteSelectorObj      routeObj = {0};

    NB_RouteParameters*       routeParameters = 0;
    NB_RouteOptions           options;
    NB_RouteConfiguration     config = {0};

    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_Place                  pStart       = {{0}};
    NB_Place                  pDest        = {{0}};

    NB_RasterTileManager* manager          = NULL;
    NB_RasterTileDataSourceData* dataSourceData = NULL;
    NB_RasterTileConfiguration configuration    = {0};
    NB_RasterTileManagerCallback callbackData   = {0};

    NB_RequestHandlerCallback routeHandlerCallback = {DownloadRouteCallback, 0 };
    TestData* testData = nsl_malloc(sizeof(*testData));

    nsl_memset(testData, 0, sizeof(*testData));

    testData->pal = pal;
    testData->requestedTiles = 15;
    callbackData.callback = &DownloadCallback;
    callbackData.callbackData = testData;

    routeHandlerCallback.callbackData = &routeObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        SetupPlaces(&pStart, &pDest);
        SetupConfigAndOptions(context, guidanceInfo, &options, &config);
        config.wantAlternateRoutes = TRUE;

        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);

        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &routeObj.routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeObj.routeHandler);

        InitializeConfiguration(&configuration, 256, 5, 5, 0, 0, 20, 20, 0, 0);

        configuration.routeAlternate1Configuration.maximumConcurrentRequests = 5;
        configuration.routeAlternate1Configuration.maximumPendingTiles = 20;
        configuration.routeAlternate1Configuration.imageFormat = NB_RTIF_PNG;

        configuration.routeAlternate2Configuration.maximumConcurrentRequests = 5;
        configuration.routeAlternate2Configuration.maximumPendingTiles = 20;
        configuration.routeAlternate2Configuration.imageFormat = NB_RTIF_PNG;

        configuration.routeAlternate3Configuration.maximumConcurrentRequests = 5;
        configuration.routeAlternate3Configuration.maximumPendingTiles = 20;
        configuration.routeAlternate3Configuration.imageFormat = NB_RTIF_PNG;

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);

        if (NULL != routeParameters && NULL != routeObj.routeHandler && NULL != dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_RouteHandlerStartRequest(routeObj.routeHandler, routeParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (NE_OK == result)
            {
                // Wait for event here from callback. Timeout 1 min.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
                {
                    int count = 0;
                    int i = 0;
                    NB_RouteId* routeid  = NULL;
                    NB_RouteId routeid1 = {0};
                    NB_RouteId routeid2 = {0};
                    NB_RouteId routeid3 = {0};

                    LOGOUTPUT(LogLevelHigh, ("TestRasterTileRouteSelectorTiles::Callback Successful\n"));

                    result = NB_RouteInformationGetRoutesCount(routeObj.route, (uint32*)&count);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    for (i = 0; i < count; i++)
                    {
                        result = NB_RouteInformationSetActiveRoute(routeObj.route, i + 1);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        result = NB_RouteInformationGetRouteId(routeObj.route, &routeid);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        switch(i)
                        {
                            case 0:
                                routeid1.data = (byte*)nsl_malloc(routeid->size);
                                routeid1.size = routeid->size;
                                nsl_memcpy(routeid1.data, routeid->data, routeid->size);
                                break;
                            case 1:
                                routeid2.data = (byte*)nsl_malloc(routeid->size);
                                routeid2.size = routeid->size;
                                nsl_memcpy(routeid2.data, routeid->data, routeid->size);
                                break;
                            case 2:
                                routeid3.data = (byte*)nsl_malloc(routeid->size);
                                routeid3.size = routeid->size;
                                nsl_memcpy(routeid3.data, routeid->data, routeid->size);
                                break;
                            default:
                                break;
                        }

                        routeid = NULL;
                    }

                    result = NB_RasterTileManagerSetAlternateRoute(manager,
                                                                   routeid1,
                                                                   routeid2,
                                                                   routeid3);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    if (routeid1.data != NULL)
                    {
                        nsl_free(routeid1.data);
                    }

                    if (routeid2.data != NULL)
                    {
                        nsl_free(routeid2.data);
                    }

                    if (routeid3.data != NULL)
                    {
                        nsl_free(routeid3.data);
                    }

                    if (result == NE_OK)
                    {
                        const uint32 ZOOM = 16;
                        const uint32 X_POSITION_START = 11339;
                        const uint32 Y_POSITION_START = 26275;

                        NB_RasterTile tile = {{0}};
                        tile.information.type = NB_RTT_RouteSelector1;
                        tile.information.zoomLevel = ZOOM;
                        tile.information.xPosition = X_POSITION_START;
                        tile.information.yPosition = Y_POSITION_START;

                        // The x-position get incremented by the function
                        RequestTile(manager, &tile, 10, 1);
                        RequestTile(manager, &tile, 10, 2);
                        RequestTile(manager, &tile, 10, 3);
                        RequestTile(manager, &tile, 10, 4);
                        RequestTile(manager, &tile, 10, 5);

                        tile.information.type = NB_RTT_RouteSelector2;
                        tile.information.xPosition = X_POSITION_START;

                        RequestTile(manager, &tile, 10, 1);
                        RequestTile(manager, &tile, 10, 2);
                        RequestTile(manager, &tile, 10, 3);
                        RequestTile(manager, &tile, 10, 4);
                        RequestTile(manager, &tile, 10, 5);

                        tile.information.type = NB_RTT_RouteSelector3;
                        tile.information.xPosition = X_POSITION_START;

                        RequestTile(manager, &tile, 10, 1);
                        RequestTile(manager, &tile, 10, 2);
                        RequestTile(manager, &tile, 10, 3);
                        RequestTile(manager, &tile, 10, 4);
                        RequestTile(manager, &tile, 10, 5);

                        if (WaitForCallbackCompletedEvent(g_CallbackEvent, 40000))
                        {
                            // All tiles are downloaded successful.
                        }
                        else
                        {
                            CU_FAIL("Raster tile download timed out!");
                        }
                    }
                }
            }
            else
            {
                LOGOUTPUT(LogLevelHigh, ("TestRasterTileRouteSelectorTiles:: Failed to create instances\n"));
            }
        }
    }

    //Cleaning up
    result = NB_RasterTileManagerDestroy(manager);
    CU_ASSERT_EQUAL(result, NE_OK);

    nsl_free(dataSourceData);
    nsl_free(testData);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteHandlerDestroy(routeObj.routeHandler);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_RouteInformationDestroy(routeObj.route);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    NB_ContextDestroy(context);
    PAL_DestroyInstance(pal);

    CleanupCacheFiles();

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Test requesting multiple tiles in one single call.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileBulkRequest(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;

        #define MAX_BULK_REQUEST 100
        testData->requestedTiles = MAX_BULK_REQUEST; // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 128, 2, 0, 0, 0, 100, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                NB_RasterTile tileArray[MAX_BULK_REQUEST] = {{{0}}};
                NB_RasterTileRequest requestArray[MAX_BULK_REQUEST] = {{0}};

                int i = 0;
                uint32 zoom = 15;
                uint32 x = 5670;
                uint32 y = 13141;
                uint32 priority = 50000;

                // Fill array for request
                for (i = 0; i < MAX_BULK_REQUEST; ++i)
                {
                    priority -= 10;
                    if ((i % 20) == 0)
                    {
                        y++;
                    }

                    tileArray[i].information.type       = NB_RTT_Map;
                    tileArray[i].information.xPosition  = x++;
                    tileArray[i].information.yPosition  = y;
                    tileArray[i].information.zoomLevel  = zoom;

                    requestArray[i].information         = &tileArray[i].information;
                    requestArray[i].priority            = priority;
                    requestArray[i].command             = NB_RTC_Download;
                    requestArray[i].buffer              = &tileArray[i].buffer;
                }

                result = NB_RasterTileManagerGetTiles(manager, requestArray, MAX_BULK_REQUEST);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 2 * 60 * 1000))
                {
                    // Done
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test reusing an already open HTTP connection.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileReuseConnection(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 5 tiles. All 5 tiles should be downloaded simultaneoulsy.
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 10, 1);
                RequestTile(manager, &tile, 10, 2);
                RequestTile(manager, &tile, 10, 3);
                RequestTile(manager, &tile, 10, 4);
                RequestTile(manager, &tile, 10, 5);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                testData->currentTileCount = 0;

                // Request 5 more tiles. The http connections should be idle and being reused.
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 10, 6);
                RequestTile(manager, &tile, 10, 7);
                RequestTile(manager, &tile, 10, 8);
                RequestTile(manager, &tile, 10, 9);
                RequestTile(manager, &tile, 10, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}


/*! Test processing pending tiles.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTilePendingTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 10;           // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        InitializeConfiguration(&configuration, 256, 1, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 10 tiles. Only the first one should start immediately, the other 9 should
                // go into the pending tile vector.
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 10, 1);
                RequestTile(manager, &tile, 10, 2);
                RequestTile(manager, &tile, 10, 3);
                RequestTile(manager, &tile, 10, 4);
                RequestTile(manager, &tile, 10, 5);
                RequestTile(manager, &tile, 10, 6);
                RequestTile(manager, &tile, 10, 7);
                RequestTile(manager, &tile, 10, 8);
                RequestTile(manager, &tile, 10, 9);
                RequestTile(manager, &tile, 10, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test the sorting of pending tiles based on priority.

    @return None. Asserts are called on failure.
*/
/*
void 
TestRasterTilePendingTilesSorting(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 10;           // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        InitializeConfiguration(&configuration, 256, 1, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 10 tiles. Only the first one should start immediately, the other 9 should
                // go into the pending tile vector. This time add them with different priority
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 1, 1);   // The first starts immediately, so the priority here doesn't matter
                RequestTile(manager, &tile, 8, 2);
                RequestTile(manager, &tile, 5, 3);
                RequestTile(manager, &tile, 6, 4);
                RequestTile(manager, &tile, 2, 5);
                RequestTile(manager, &tile, 3, 6);
                RequestTile(manager, &tile, 9, 7);
                RequestTile(manager, &tile, 7, 8);
                RequestTile(manager, &tile, 4, 9);
                RequestTile(manager, &tile, 10, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // This time the order should match exactly what me specified above

                    VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[1].information, ZOOM, X_POSITION_START + 5, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[2].information, ZOOM, X_POSITION_START + 6, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[3].information, ZOOM, X_POSITION_START + 9, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[4].information, ZOOM, X_POSITION_START + 3, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[5].information, ZOOM, X_POSITION_START + 4, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[6].information, ZOOM, X_POSITION_START + 8, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[7].information, ZOOM, X_POSITION_START + 2, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[8].information, ZOOM, X_POSITION_START + 7, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[9].information, ZOOM, X_POSITION_START + 10, Y_POSITION_START);

                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}
*/

/*! Test the re-sorting of pending tiles based on priority.

    @return None. Asserts are called on failure.
*/
/*
void 
TestRasterTilePendingTilesResorting(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 10;           // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        InitializeConfiguration(&configuration, 256, 1, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 10 tiles. Only the first one should start immediately, the other 9 should
                // go into the pending tile vector. This time add them with different priority
                // The x-position get incremented by the function
                RequestTile(manager, &tile, 1, 1);   // The first starts immediately, so the priority here doesn't matter
                RequestTile(manager, &tile, 102, 2);
                RequestTile(manager, &tile, 103, 3);
                RequestTile(manager, &tile, 104, 4);
                RequestTile(manager, &tile, 105, 5);
                RequestTile(manager, &tile, 106, 6);
                RequestTile(manager, &tile, 107, 7);
                RequestTile(manager, &tile, 108, 8);
                RequestTile(manager, &tile, 109, 9);
                RequestTile(manager, &tile, 110, 10);

                // Now add the same tiles (the last 9) again this time with different priority. This should resort them.

                tile.information.xPosition = X_POSITION_START + 1;
                tile.information.yPosition = Y_POSITION_START;

                // Use lower priority then above to ensure the sorting is correct
                RequestTile(manager, &tile, 155, 2);
                RequestTile(manager, &tile, 153, 3);
                RequestTile(manager, &tile, 157, 4);
                RequestTile(manager, &tile, 158, 5);
                RequestTile(manager, &tile, 159, 6);
                RequestTile(manager, &tile, 151, 7);
                RequestTile(manager, &tile, 152, 8);
                RequestTile(manager, &tile, 154, 9);
                RequestTile(manager, &tile, 156, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // This time the order should match exactly what me specified above

                    VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[1].information, ZOOM, X_POSITION_START + 7, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[2].information, ZOOM, X_POSITION_START + 8, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[3].information, ZOOM, X_POSITION_START + 3, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[4].information, ZOOM, X_POSITION_START + 9, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[5].information, ZOOM, X_POSITION_START + 2, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[6].information, ZOOM, X_POSITION_START + 10, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[7].information, ZOOM, X_POSITION_START + 4, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[8].information, ZOOM, X_POSITION_START + 5, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[9].information, ZOOM, X_POSITION_START + 6, Y_POSITION_START);

                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}
*/

/*! Test the discarding of the tiles once we exceed the number of allowed locked tiles.

    @return None. Asserts are called on failure.
*/
/*
void 
TestRasterTileExceedLockedList(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 20 + 10; // This has to match 'maximumConcurrentMapRequests' + 'maximumPendingMapTiles' below.
                                            // Requests will be ignored once all connections are busy and the pending tile list is full.

        // For this test we use a different download to verify the expected failures.
        callbackData.callback = &LockedTilesDownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 10, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                int i = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request tiles. After we exceed the allowed locked tiles then the subsequent requests will fail.
                // See LockedTilesDownloadCallback() for more details.
                for (i = 0; i < 100; i++)
                {
                    RequestTile(manager, &tile, 555, i + 1);
                }

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60 * 1000))
                {
                    // Done.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}
*/

/*! Test the persistent cache.

    @return None. Asserts are called on failure.
*/
/*
void 
TestRasterTilePersistentCache(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    CleanupCacheFiles();

    if (palResult)
    {
        int32 savedEntries = 0;
        const uint32 ZOOM = 16;
        const uint32 X_POSITION_START = 11342;
        const uint32 Y_POSITION_START = 26264;

        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 0;            // Set below!

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 128, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Download 5 tiles and put them in the memory cache
                testData->requestedTiles = 5;
                RequestTile(manager, &tile, 10, 1);
                RequestTile(manager, &tile, 10, 2);
                RequestTile(manager, &tile, 10, 3);
                RequestTile(manager, &tile, 10, 4);
                RequestTile(manager, &tile, 10, 5);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_RasterTileManagerSaveToPersistentCache(manager, -1, &savedEntries);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_EQUAL(savedEntries, 5);

                // Wait 1.5 seconds so that the next tiles get a higher timestamp.
                WaitForCallbackCompletedEvent(g_CallbackEvent, 1500);

                // Download 5 more tiles and put them in the memory cache
                testData->requestedTiles = 10;
                RequestTile(manager, &tile, 10, 6);
                RequestTile(manager, &tile, 10, 7);
                RequestTile(manager, &tile, 10, 8);
                RequestTile(manager, &tile, 10, 9);
                RequestTile(manager, &tile, 10, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_RasterTileManagerSaveToPersistentCache(manager, -1, &savedEntries);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_EQUAL(savedEntries, 5);

                // Get the tiles again from the cache. 5 should come from the memory cache and 5 from the persistent cache.
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 1, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 2, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 3, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 4, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 5, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 6, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 7, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 8, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 9, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 10, Y_POSITION_START);

                // Verify that all the cache-files exist. 
                // since we don't have a cross platform API to create full path, let's verify the
                // two likely platforms here: Windows and UNIX
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000001") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000002") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000003") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000004") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000005") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000006") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000007") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000008") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000009") == PAL_Ok)
                CU_ASSERT(PAL_FileExists(pal, RASTER_TILE_CACHE_NAME PATH_DELIMITER "00000010") == PAL_Ok)

                // Wait 1.5 seconds so that the next tiles get a higher timestamp.
                WaitForCallbackCompletedEvent(g_CallbackEvent, 1500);

                // Download 5 more tiles and put them in the memory cache
                testData->requestedTiles = 15;
                RequestTile(manager, &tile, 10, 11);
                RequestTile(manager, &tile, 10, 12);
                RequestTile(manager, &tile, 10, 13);
                RequestTile(manager, &tile, 10, 14);
                RequestTile(manager, &tile, 10, 15);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_RasterTileManagerSaveToPersistentCache(manager, -1, &savedEntries);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_EQUAL(savedEntries, 5);

                // Wait 1.5 seconds so that the next tiles get a higher timestamp.
                WaitForCallbackCompletedEvent(g_CallbackEvent, 1500);

                // Download 5 more tiles and put them in the memory cache
                testData->requestedTiles = 20;
                RequestTile(manager, &tile, 10, 16);
                RequestTile(manager, &tile, 10, 17);
                RequestTile(manager, &tile, 10, 18);
                RequestTile(manager, &tile, 10, 19);
                RequestTile(manager, &tile, 10, 20);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_RasterTileManagerSaveToPersistentCache(manager, -1, &savedEntries);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_EQUAL(savedEntries, 5);

                // Get the tiles again from the cache. 5 should come from the memory cache and 5 from the persistent cache.
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 11, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 12, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 13, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 14, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 15, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 16, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 17, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 18, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 19, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 20, Y_POSITION_START);

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                manager = NULL;
            }

            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                // Get the last 10 tiles again. They should now be read from the persistent cache.
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 11, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 12, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 13, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 14, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 15, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 16, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 17, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 18, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 19, Y_POSITION_START);
                VerifyTileFromPersistentCache(manager, ZOOM, X_POSITION_START + 20, Y_POSITION_START);

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}
*/

/*! Test flushing tiles from the pending list.

    @return None. Asserts are called on failure.
*/
/*
void 
TestRasterTileFlushTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 8;           // Has to match the number of RequestTile() calls below (after the flush calls)

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        InitializeConfiguration(&configuration, 256, 1, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // The first tile will be started immediately, all other once will be put in the pending tile list
                // The x-position gets incremented by the function
                RequestTile(manager, &tile, 100, 1);
                RequestTile(manager, &tile, 99, 2);
                RequestTile(manager, &tile, 98, 3);
                RequestTile(manager, &tile, 97, 4);
                RequestTile(manager, &tile, 96, 5);
                RequestTile(manager, &tile, 95, 6);
                RequestTile(manager, &tile, 94, 7);
                RequestTile(manager, &tile, 93, 8);
                RequestTile(manager, &tile, 92, 9);
                RequestTile(manager, &tile, 91, 10);

                // Flush the last 9 tiles
                result = NB_RasterTileManagerFlushAllTiles(manager);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Add 10 new tiles
                RequestTile(manager, &tile, 90, 11);
                RequestTile(manager, &tile, 89, 12);
                RequestTile(manager, &tile, 88, 13);
                RequestTile(manager, &tile, 87, 14);
                RequestTile(manager, &tile, 86, 15);
                RequestTile(manager, &tile, 85, 16);
                RequestTile(manager, &tile, 84, 17);
                RequestTile(manager, &tile, 83, 18);
                RequestTile(manager, &tile, 82, 19);
                RequestTile(manager, &tile, 81, 20);

                // Flush them again
                result = NB_RasterTileManagerFlushAllTiles(manager);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Add 10 more
                RequestTile(manager, &tile, 1, 21);
                RequestTile(manager, &tile, 2, 22);
                RequestTile(manager, &tile, 3, 23);
                RequestTile(manager, &tile, 4, 24);
                RequestTile(manager, &tile, 5, 25);
                RequestTile(manager, &tile, 6, 26);
                RequestTile(manager, &tile, 7, 27);
                RequestTile(manager, &tile, 8, 28);
                RequestTile(manager, &tile, 9, 29);
                RequestTile(manager, &tile, 10, 30);

                // Flush some tiles inbetween

                // Flush 23
                tile.information.xPosition = X_POSITION_START + 23;
                result = NB_RasterTileManagerFlushTile(manager, &tile.information);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Flush 25
                tile.information.xPosition = X_POSITION_START + 25;
                result = NB_RasterTileManagerFlushTile(manager, &tile.information);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Flush 28
                tile.information.xPosition = X_POSITION_START + 28;
                result = NB_RasterTileManagerFlushTile(manager, &tile.information);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                {
                    // These are the remaining 8 tiles received (the very first tile + the last 10 minus the three removed.)

                    VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[1].information, ZOOM, X_POSITION_START + 21, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[2].information, ZOOM, X_POSITION_START + 22, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[3].information, ZOOM, X_POSITION_START + 24, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[4].information, ZOOM, X_POSITION_START + 26, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[5].information, ZOOM, X_POSITION_START + 27, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[6].information, ZOOM, X_POSITION_START + 29, Y_POSITION_START);
                    VerifyTile(&testData->receivedTiles[7].information, ZOOM, X_POSITION_START + 30, Y_POSITION_START);

                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}
*/

/*! Force download of already downloaded tile.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileForceDownload(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request tile
                result = NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Download, &tile.buffer);
                CU_ASSERT_EQUAL(result, NE_OK);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. 
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.

                        testData->requestedTiles = 2;

                        // Request the same tile again. Set the command to force-download. This will remove it from the
                        // cache and force the download again.
                        result = NB_RasterTileManagerGetTile(manager, &tile.information, 100, NB_RTC_Force_Download, &tile.buffer);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        // Returned buffer should be NULL since we forced the download
                        CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                        CU_ASSERT_EQUAL(tile.buffer.size, 0);

                        if (result == NE_OK)
                        {
                            // Wait for the callback to trigger the event
                            if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                            {
                                // Validate the tile. 
                                VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START, Y_POSITION_START);

                                // Don't unlock tile here, it gets done in the callback.
                            }
                            else
                            {
                                CU_FAIL("Raster tile download timed out!");
                            }
                        }
                    }
                    else
                    {
                        CU_FAIL("Raster tile download timed out!");
                    }
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test handling of failed tiles.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileFailedTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 200;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &FailedTilesDownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 1000, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                int i = 0;

                for (i = 0; i < 10; ++i)
                {
                    /*
                        Compare the values with the callback handling in FailedTilesDownloadCallback()
                    */

                    // Invalid coordinates
                    uint32 ZOOM = 2;
                    uint32 X_POSITION_START = 10;
                    uint32 Y_POSITION_START = 10 + i;

                    NB_RasterTile tile = {{0}};
                    tile.information.type = NB_RTT_Map;
                    tile.information.zoomLevel = ZOOM;
                    tile.information.xPosition = X_POSITION_START;
                    tile.information.yPosition = Y_POSITION_START;

                    // Request tiles, they should fail
                    RequestTile(manager, &tile, 10, 1);
                    RequestTile(manager, &tile, 10, 2);
                    RequestTile(manager, &tile, 10, 3);
                    RequestTile(manager, &tile, 10, 4);
                    RequestTile(manager, &tile, 10, 5);
                    RequestTile(manager, &tile, 20, 6);
                    RequestTile(manager, &tile, 20, 7);
                    RequestTile(manager, &tile, 20, 8);
                    RequestTile(manager, &tile, 20, 9);
                    RequestTile(manager, &tile, 20, 10);

                    ZOOM = 16;
                    X_POSITION_START = 11111;
                    Y_POSITION_START = 23111 + i;

                    tile.information.type = NB_RTT_Map;
                    tile.information.zoomLevel = ZOOM;
                    tile.information.xPosition = X_POSITION_START;
                    tile.information.yPosition = Y_POSITION_START;

                    // Request more tiles, they should succeed
                    RequestTile(manager, &tile, 10, 1);
                    RequestTile(manager, &tile, 10, 2);
                    RequestTile(manager, &tile, 10, 3);
                    RequestTile(manager, &tile, 10, 4);
                    RequestTile(manager, &tile, 10, 5);
                    RequestTile(manager, &tile, 20, 6);
                    RequestTile(manager, &tile, 20, 7);
                    RequestTile(manager, &tile, 20, 8);
                    RequestTile(manager, &tile, 20, 9);
                    RequestTile(manager, &tile, 20, 10);
                }

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60 * 1000))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test raster tile access token.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileAccessToken(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 5, 0, 0, 0, 20, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            int zoomLevel = 0;
            for (zoomLevel = 12; zoomLevel < 20; zoomLevel++)
            {
                nb_boolean provideGlobalAccessToken = FALSE;

                do
                {
                    // Initialize test data buffer for request
                    nsl_memset(testData, 0, sizeof(*testData));
                    testData->pal = pal;
                    testData->requestedTiles = 1;

                    ResetCallbackCompletedEvent(g_CallbackEvent);

                    result = NB_RasterTileManagerCreate(
                        context,
                        dataSourceData,
                        provideGlobalAccessToken ? "st=g" : NULL,
                        &configuration,
                        &callbackData,
                        &manager);

                    CU_ASSERT_EQUAL(result, NE_OK);
                    if (result == NE_OK)
                    {
                        const uint32 X_POSITION_START = 32752;
                        const uint32 Y_POSITION_START = 21784;

                        NB_RasterTile tile = {{0}};
                        tile.information.type = NB_RTT_Map;
                        tile.information.zoomLevel = zoomLevel;
                        tile.information.xPosition = X_POSITION_START;
                        tile.information.yPosition = Y_POSITION_START;

                        // Request one tile
                        RequestTile(manager, &tile, 10, 1);

                        // The first request should not return the buffer. It will be returned in the callback
                        CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                        CU_ASSERT_EQUAL(tile.buffer.size, 0);

                        if (result == NE_OK)
                        {
                            // Wait for the callback to trigger the event
                            if (WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
                            {
                                // Validate the tile. x-position will be incremented by one
                                VerifyTile(&testData->receivedTiles[0].information, zoomLevel, X_POSITION_START + 1, Y_POSITION_START);

                                // Don't unlock tile here, it gets done in the callback.
                            }
                            else
                            {
                                CU_FAIL("Raster tile download timed out!");
                            }
                        }

                        result = NB_RasterTileManagerDestroy(manager);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        manager = NULL;
                    }

                    provideGlobalAccessToken = !provideGlobalAccessToken;

                } while (provideGlobalAccessToken);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Do an extensive test with lots of tiles.

    @return None. Asserts are called on failure.
*/
void 
TestRasterTileStressTest(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    
    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_RasterTileManager* manager = NULL;
        NB_RasterTileDataSourceData* dataSourceData = NULL;
        NB_RasterTileConfiguration configuration = {0};
        NB_RasterTileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;

        testData->requestedTiles = 100 * STRESS_TEST_MULTIPLIER; // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        // Fill out configuration for raster tile manager
        InitializeConfiguration(&configuration, 256, 10, 0, 0, 0, 10000, 0, 0, 0);

        dataSourceData = RetrieveDataSource(context);
        CU_ASSERT_PTR_NOT_NULL(dataSourceData);
        if (dataSourceData)
        {
            result = NB_RasterTileManagerCreate(context, dataSourceData, NULL, &configuration, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                int i = 0;
                int requestCount = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_RasterTile tile = {{0}};
                tile.information.type = NB_RTT_Map;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 100 tiles with different priorities with 10 concurrent connections. Pause inbetween
                // to simulate asynchronous requests.

                for (i = 0; i < 8 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 10);

                for (i = 0; i < 8 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 8 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 8 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 50 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 18 * STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }   

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                // Increase timeout to 5 minutes. The server is sometimes too slow to download in 2 minutes.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 5 * 60 * 1000))  
                {
                    // Impossible to verify results here.

                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_RasterTileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(dataSourceData);
        }

        nsl_free(testData);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Callback for data source handler.

    This function is used to retrieve the data sources.

    @return None
    @see TestRasterTileInitializeDataSource
*/
void
DataSourceHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Error localResult = NE_OK;
    NB_RasterTileDataSourceHandler* pHandler = (NB_RasterTileDataSourceHandler*)handler;
    NB_RasterTileDataSourceData* dataSourceData = (NB_RasterTileDataSourceData*)userData;   // User data points to data source data

    // Did the download complete successfully
    if ((status == NB_NetworkRequestStatus_Success) && (err == NE_OK) && (percent == 100))
    {
        // Get the information object from the handler
        NB_RasterTileDataSourceInformation* information = NULL;
        localResult = NB_RasterTileDataSourceHandlerGetInformation(pHandler, &information);
        CU_ASSERT_EQUAL(localResult, NE_OK);
        if (localResult == NE_OK)
        {
            // Get the data source from the information object
            localResult = NB_RasterTileDataSourceInformationGetDataSource(information, dataSourceData);
            CU_ASSERT_EQUAL(localResult, NE_OK);
            if (localResult == NE_OK)
            {
                // Trigger the test function
                SetCallbackCompletedEvent(g_CallbackEvent);
            }

            localResult = NB_RasterTileDataSourceInformationDestroy(information);
            CU_ASSERT_EQUAL(localResult, NE_OK);
        }
    }

    // Did an error occur
    if (((status != NB_NetworkRequestStatus_Success) && (status != NB_NetworkRequestStatus_Progress) &&
        (status != NB_NetworkRequestStatus_Canceled)) ||
        (err != NE_OK) ||
        (localResult != NE_OK))
    {
        CU_FAIL("Error in data source callback");
    }
}

/*! Callback for tile download.

    Used for most tests.

    @return None

    @see NB_RasterTileManagerCallbackFunction
*/
void 
DownloadCallback(NB_RasterTileManager* manager,  /* see NB_RasterTileManagerCallbackFunction() */
                 NB_Error result,
                 const NB_RasterTile* tile,
                 void* userData)
{
    TestData* testData = (TestData*)userData;

    ++testData->currentTileCount;

    // Check that we received a tile
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(tile);
    if (tile && (result == NE_OK))
    {
        NB_Error localResult = NE_OK;
        PAL_File* pFile = NULL;
        char fileName[1024];
        fileName[0] = 0;

        // The tile buffer has to be valid
        CU_ASSERT_PTR_NOT_NULL(tile->buffer.buffer);
        CU_ASSERT(tile->buffer.size > 0);

        LOGOUTPUT(LogLevelMedium, 
                  ("Tile received[%d]: %d, %d, %d\n", 
                   testData->currentTileCount,
                   tile->information.zoomLevel, 
                   tile->information.xPosition, 
                   tile->information.yPosition));

        // Format the png-filename
        nsl_sprintf(fileName, TILE_NAME_PNG, ++g_ImageNameCounter);

        // Create new file
        PAL_FileOpen(testData->pal, fileName, PFM_Create, &pFile);
        if (pFile)
        {
            uint32 bytesWritten = 0;
            
            // Write the returned buffer to the file
            PAL_FileWrite(pFile, tile->buffer.buffer, tile->buffer.size, &bytesWritten);
            CU_ASSERT_EQUAL(bytesWritten, tile->buffer.size);

            PAL_FileClose(pFile);
            CU_ASSERT_EQUAL(PAL_FileExists(testData->pal, fileName), PAL_Ok);
        }

        // Add the received tile to our test array.
        nsl_memcpy(&testData->receivedTiles[testData->currentTileCount - 1].information, &tile->information, sizeof(NB_RasterTileInformation));

        // Unlock the tile again.
        localResult = NB_RasterTileManagerUnlockTile(manager, &tile->information);
        CU_ASSERT_EQUAL(localResult, NE_OK);
    }
    else
    {
        if (tile)
        {
            LOGOUTPUT(LogLevelMedium, 
                      ("Tile FAILED[%d]: %d, %d, %d\n", 
                       testData->currentTileCount,
                       tile->information.zoomLevel, 
                       tile->information.xPosition, 
                       tile->information.yPosition));
        }
        else
        {
            LOGOUTPUT(LogLevelMedium, ("Tile FAILED[%d]. General error!\n", testData->currentTileCount));
        }
    }

    // Check if we received all tiles for this test
    if (testData->currentTileCount == testData->requestedTiles)
    {
        // Trigger the test function
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}

/*! Special download for locked tile test.

    @return None

    @see TestRasterTileExceedLockedList
    @see NB_RasterTileManagerCallbackFunction
*/
void 
LockedTilesDownloadCallback(NB_RasterTileManager* manager,  /* see NB_RasterTileManagerCallbackFunction() */
                            NB_Error result,
                            const NB_RasterTile* tile,
                            void* userData)
{
    TestData* testData = (TestData*)userData;

    ++testData->currentTileCount;

    CU_ASSERT_PTR_NOT_NULL(tile);
    if (tile)
    {
        if (testData->currentTileCount <= (signed)RASTERTILE_CACHE_MEMORY)
        {
            CU_ASSERT_EQUAL(result, NE_OK);

            LOGOUTPUT(LogLevelMedium, 
                      ("Tile received[%d]: %d, %d, %d\n", 
                       testData->currentTileCount,
                       tile->information.zoomLevel, 
                       tile->information.xPosition, 
                       tile->information.yPosition));
        }
        else
        {
            // The result code should be 'NE_RES'
            CU_ASSERT_EQUAL(result, NE_RES);
            if (result == NE_RES)
            {
                LOGOUTPUT(LogLevelMedium, 
                          ("Locked list is full. Tile failed as expected[%d]: %d, %d, %d\n", 
                           testData->currentTileCount,
                           tile->information.zoomLevel, 
                           tile->information.xPosition, 
                           tile->information.yPosition));
            }
            else
            {
                LOGOUTPUT(LogLevelMedium, 
                          ("Unexpected result[%d]: %d, %d, %d\n", 
                           testData->currentTileCount,
                           tile->information.zoomLevel, 
                           tile->information.xPosition, 
                           tile->information.yPosition));
            }
        }
    }

    // Check if we received all tiles for this test
    if (testData->currentTileCount == testData->requestedTiles)
    {
        // Trigger the test function
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}

/*! Download callback for failed-tile test.

    @return None

    @see TestRasterTileFailedTiles
    @see NB_RasterTileManagerCallbackFunction
*/
void 
FailedTilesDownloadCallback(NB_RasterTileManager* manager,  /* see NB_RasterTileManagerCallbackFunction() */
                            NB_Error result,
                            const NB_RasterTile* tile,
                            void* userData)
{
    TestData* testData = (TestData*)userData;

    ++testData->currentTileCount;

    // tile has to be valid even for failed tiles
    CU_ASSERT_PTR_NOT_NULL(tile);

    /*
        The tiles we request for zoom level 2 should fail, all others should succeed. See TestRasterTileFailedTiles()
    */

    if (tile)
    {
        // Is it a failed tile
        if (tile->information.zoomLevel == 2)
        {
            // Tile failed as expected
            LOGOUTPUT(LogLevelMedium, 
                      ("Tile failed as expected[%d]: %d, %d, %d\n", 
                       testData->currentTileCount,
                       tile->information.zoomLevel, 
                       tile->information.xPosition, 
                       tile->information.yPosition));

            // Failed tile
            CU_ASSERT(result != NE_OK);

            // The buffer should be NULL
            CU_ASSERT_PTR_NULL(tile->buffer.buffer);
            CU_ASSERT_EQUAL(tile->buffer.size, 0);
        }
        // all other tiles should succeed
        else
        {
            // Check that we received a tile
            CU_ASSERT_EQUAL(result, NE_OK);
            if (tile && (result == NE_OK))
            {
                // The tile buffer has to be valid
                CU_ASSERT_PTR_NOT_NULL(tile->buffer.buffer);
                CU_ASSERT(tile->buffer.size > 0);

                LOGOUTPUT(LogLevelMedium, 
                          ("Tile received[%d]: %d, %d, %d\n", 
                           testData->currentTileCount,
                           tile->information.zoomLevel, 
                           tile->information.xPosition, 
                           tile->information.yPosition));
            }
            else
            {
                if (tile)
                {
                    LOGOUTPUT(LogLevelMedium, 
                              ("Tile FAILED[%d]: %d, %d, %d\n", 
                               testData->currentTileCount,
                               tile->information.zoomLevel, 
                               tile->information.xPosition, 
                               tile->information.yPosition));
                }
                else
                {
                    LOGOUTPUT(LogLevelMedium, ("Tile FAILED[%d]. General error!\n", testData->currentTileCount));
                }
            }
        }
    }

    // Check if we received all tiles for this test
    if (testData->currentTileCount == testData->requestedTiles)
    {
        // Trigger the test function
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}

void
DownloadRouteCallback(void* handler,
                      NB_NetworkRequestStatus status,
                      NB_Error err,
                      uint8 up,
                      int percent,
                      void* pUserData)
{
    TestRouteSelectorObj* routeObj = (TestRouteSelectorObj*) pUserData;

    if (err != NE_OK)
    {
        CU_FAIL("Callback returned error");
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }

    switch (status)
    {
    case NB_NetworkRequestStatus_Progress:
        return;

    case NB_NetworkRequestStatus_Success:
        {
            NB_RouteHandlerGetRouteInformation(routeObj->routeHandler, &routeObj->route);
        }
        break;

    case NB_NetworkRequestStatus_TimedOut:
        CU_FAIL("NB_NetworkRequestStatus_TimedOut");
        break;

    case NB_NetworkRequestStatus_Canceled:
        CU_FAIL("NB_NetworkRequestStatus_Canceled");
        break;

    case NB_NetworkRequestStatus_Failed:
        CU_FAIL("NB_NetworkRequestStatus_Failed");
        break;
    }

    // Trigger main thread
    SetCallbackCompletedEvent(g_CallbackEvent);
    return;
}

/*! Retrieve the data source.

    The data source has to be retrieved before the raster tile manager is created. This function is used by all tests. 
    nsl_free() has to be called on the data source data once it is no longer needed.

    @return Data source on success, NULL on failure
*/
NB_RasterTileDataSourceData*
RetrieveDataSource(NB_Context* context                  /*!< Passed in context. Has to be valid */
                   )
{
    NB_Error result = NE_OK;
    NB_RasterTileDataSourceParameters* parameters = NULL;
    NB_RasterTileDataSourceHandler* handler = NULL;
    NB_RequestHandlerCallback callbackData = {0};

    NB_RasterTileDataSourceData* dataSourceData = nsl_malloc(sizeof(NB_RasterTileDataSourceData));
    if (!dataSourceData)
    {
        return NULL;
    }
    nsl_memset(dataSourceData, 0, sizeof(NB_RasterTileDataSourceData));

    callbackData.callback = &DataSourceHandlerCallback;

    // Pass NB_RasterTileDataSourceData as callback data. It gets set in the callback
    callbackData.callbackData = dataSourceData;                       

    // Create handler to request data sources
    result = NB_RasterTileDataSourceHandlerCreate(context, &callbackData, &handler);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result == NE_OK)
    {
        // Create parameters used to make data source request
        result = NB_RasterTileDataSourceParametersCreate(context, &parameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result == NE_OK)
        {
            // Make data source request. See DataSourceHandlerCallback() for handling of the result
            result = NB_RasterTileDataSourceHandlerStartRequest(handler, parameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            // We have to wait for the data source initialize to finish.
            if (!WaitForCallbackCompletedEvent(g_CallbackEvent, CALLBACK_TIMEOUT))
            {
                CU_FAIL("Data source retrieval timed out!");
                result = NE_BADDATA;
            }

            // Verify results

            CU_ASSERT_EQUAL(dataSourceData->templateMap.projection, NB_RTDSP_Mercator);
            CU_ASSERT_EQUAL(dataSourceData->templateRoute.projection, NB_RTDSP_Mercator);
            CU_ASSERT_EQUAL(dataSourceData->templateTraffic.projection, NB_RTDSP_Mercator);

            CU_ASSERT(nsl_strlen(dataSourceData->templateMap.baseUrl) > 0);
            CU_ASSERT(nsl_strlen(dataSourceData->templateRoute.baseUrl) > 0);
            CU_ASSERT(nsl_strlen(dataSourceData->templateTraffic.baseUrl) > 0);

            CU_ASSERT(nsl_strlen(dataSourceData->templateMap.templateUrl) > 0);
            CU_ASSERT(nsl_strlen(dataSourceData->templateRoute.templateUrl) > 0);
            CU_ASSERT(nsl_strlen(dataSourceData->templateTraffic.templateUrl) > 0);
        }
    }

    if (parameters)
    {
        result = result ? result : NB_RasterTileDataSourceParametersDestroy(parameters);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (handler)
    {
        result = result ? result : NB_RasterTileDataSourceHandlerDestroy(handler);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    if (result == NE_OK)
    {
        return dataSourceData;
    }
    else
    {
        nsl_free(dataSourceData);
        return NULL;
    }
}

/*! Delete all the temporary files from the tests.

    @return None
*/
void 
CleanupImageFiles()
{
    PAL_Instance* pal = PAL_CreateInstance();
    if (pal)
    {
        PAL_Error err = PAL_Ok;
        PAL_FileEnum* fileEnum;

        err = PAL_FileEnumerateCreate(pal, ".", TRUE, &fileEnum);
        if (!err)
        {
            PAL_FileEnumInfo info = { 0 };

            do
            {
                err = PAL_FileEnumerateNext(fileEnum, &info);
                if (!err)
                {
                    if ((info.attributes & PAL_FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        if (nsl_strncmp(info.filename, TILE_PREFIX, nsl_strlen(TILE_PREFIX)) == 0)
                        {
                            PAL_FileRemove(pal, info.filename);
                        }
                    }
                }
            } while (!err);

            PAL_FileEnumerateDestroy(fileEnum);
        }

        PAL_DestroyInstance(pal);
    }
}

/*! Cleanup any cache files.

    @return None
*/
void 
CleanupCacheFiles()
{
    PAL_Instance* pal = PAL_CreateInstance();
    if (pal)
    {
        PAL_FileEnum* enumerator = NULL;
        PAL_FileEnumInfo info = {0};

        nb_boolean dirExists = PAL_FileIsDirectory(pal, RASTER_TILE_CACHE_NAME);
        if (dirExists)
        {
            if (PAL_FileEnumerateCreate(pal, RASTER_TILE_CACHE_NAME, FALSE, &enumerator) == PAL_Ok)
            {
                while (PAL_FileEnumerateNext(enumerator, &info) == NE_OK)
                {
                    char path[1024] = {0};
                    nsl_snprintf(path, 1024, "%s/%s", RASTER_TILE_CACHE_NAME, info.filename);

                    PAL_FileRemove(pal, path);
                }

                PAL_FileEnumerateDestroy(enumerator);
            }

            PAL_FileRemoveDirectory(pal, RASTER_TILE_CACHE_NAME, FALSE);
        }

        PAL_DestroyInstance(pal);
    }
}

/*! Simple helper function to request tile for download.

    The xPosition of the passed in tile gets incremented by one.

    @return None. Asserts are called on failure.
*/
void 
RequestTile(NB_RasterTileManager* manager, NB_RasterTile* tile, uint32 priority, int count)
{
    NB_Error result = NE_OK;

    tile->information.xPosition++;
    LOGOUTPUT(LogLevelMedium, 
              ("Tile requested[%d]: %d, %d, %d\n", 
               count,
               tile->information.zoomLevel, 
               tile->information.xPosition, 
               tile->information.yPosition));

    result = NB_RasterTileManagerGetTile(manager, &tile->information, priority, NB_RTC_Download, &tile->buffer);
    CU_ASSERT_EQUAL(result, NE_OK);
}

/*! Simple helper function to request tile from persistent cache and verify the result

    The xPosition of the passed in tile gets incremented by one.

    @return None. Asserts are called on failure.
*/
void 
VerifyTileFromPersistentCache(NB_RasterTileManager* manager, uint32 zoom, uint32 x, uint32 y)
{
    NB_Error result = NE_OK;
    NB_RasterTile tile = {{0}};
    tile.information.type = NB_RTT_Map;
    tile.information.xPosition = x;
    tile.information.yPosition = y;
    tile.information.zoomLevel = zoom;

    // Get the tile from the (memory or persistent) cache
    result = NB_RasterTileManagerGetTile(manager, &tile.information, 10, NB_RTC_PersistentCache, &tile.buffer);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result == NE_OK)
    {
        CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
        CU_ASSERT(tile.buffer.size > 0);

        // Unlock the tile immediately
        result = NB_RasterTileManagerUnlockTile(manager, &tile.information);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

/*! Simple helper function to verify tile.

    @return None. Asserts are called on failure.
*/
void 
VerifyTile(const NB_RasterTileInformation* information,     /*!< Actual result */
           uint32 zoom,                                     /*!< Expected result */
           uint32 x,                                        /*!< Expected result */
           uint32 y                                         /*!< Expected result */
           )
{
    CU_ASSERT_EQUAL(information->zoomLevel, zoom);
    CU_ASSERT_EQUAL(information->xPosition, x);
    CU_ASSERT_EQUAL(information->yPosition, y);
}

/*! Set all the values for the raster tile manager configuration. */
void 
InitializeConfiguration(NB_RasterTileConfiguration* configuration,
                        uint32 tileSize,
                        uint16 maximumConcurrentRequestsMap,
                        uint16 maximumConcurrentRequestsRoute,
                        uint16 maximumConcurrentRequestsTraffic,
                        uint16 maximumConcurrentRequestsSatellite,
                        uint16 maximumpendingRequestsMap,
                        uint16 maximumpendingRequestsRoute,
                        uint16 maximumpendingRequestsTraffic,
                        uint16 maximumpendingRequestsSatellite)
{
    // Fill out configuration for raster tile manager
    configuration->mapConfiguration.imageFormat                      = NB_RTIF_PNG;
    configuration->routeConfiguration.imageFormat                    = NB_RTIF_PNG;
    configuration->trafficConfiguration.imageFormat                  = NB_RTIF_PNG;
    configuration->satelliteConfiguration.imageFormat                = NB_RTIF_JPG;

    configuration->tileSize                                          = tileSize;
    configuration->tileResolution                                    = DEFAULT_RESOLUTION;

    configuration->mapConfiguration.maximumConcurrentRequests        = maximumConcurrentRequestsMap;
    configuration->routeConfiguration.maximumConcurrentRequests      = maximumConcurrentRequestsRoute;
    configuration->trafficConfiguration.maximumConcurrentRequests    = maximumConcurrentRequestsTraffic;
    configuration->satelliteConfiguration.maximumConcurrentRequests  = maximumConcurrentRequestsSatellite;

    configuration->mapConfiguration.maximumPendingTiles              = maximumpendingRequestsMap;
    configuration->routeConfiguration.maximumPendingTiles            = maximumpendingRequestsRoute;
    configuration->trafficConfiguration.maximumPendingTiles          = maximumpendingRequestsTraffic;
    configuration->satelliteConfiguration.maximumPendingTiles        = maximumpendingRequestsSatellite;
}

void
SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config)
{
    uint32 command = 0;

    command = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
    // If this asserts, the command instructions sets (imageStyle, pronunStyle and stepsStyle) cannot be found
    // See CreateGuidanceInformation for the three directories that are expected to contain appropriate config.tps files
    CU_ASSERT_NOT_EQUAL_FATAL(command, 0);

    options->transport = NB_TransportationMode_Car;
    options->type = NB_RouteType_Fastest;
    options->traffic = NB_TrafficType_Avoid;
    options->pronunStyle = Test_OptionsGet()->voiceStyle;

    config->wantDestinationLocation = TRUE;
    config->wantManeuvers = TRUE;
    config->wantOriginLocation = TRUE;
    config->wantPolyline = FALSE;
    config->wantRouteMap = FALSE;
    config->wantCrossStreets = TRUE;
    config->commandSetVersion = command;
    config->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */
}

void
SetupPlaces (NB_Place* origin, NB_Place* dest)
{
    if (origin)
    {
        origin->location.type = NB_Location_Address;
        strcpy(origin->location.streetnum, "6");
        strcpy(origin->location.street1, "Liberty");
        strcpy(origin->location.city, "Aliso Viejo");
        strcpy(origin->location.state, "CA");
        strcpy(origin->location.postal, "92656");
        origin->location.latitude = 33.5598;
        origin->location.longitude = -117.7288;
    }

    if (dest)
    {
        dest->location.type = NB_Location_Address;
        strcpy(dest->location.streetnum, "18872");
        strcpy(dest->location.street1, "Bardeen");
        strcpy(dest->location.city, "Irvine");
        strcpy(dest->location.state, "CA");
        strcpy(dest->location.postal, "92612");
        dest->location.latitude = 33.67035;
        dest->location.longitude = -117.85560;
    }
}

/*! @} */
