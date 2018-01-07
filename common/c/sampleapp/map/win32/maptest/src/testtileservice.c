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

    @file     TestTileService.c
*/
/*
    See description in header file.

    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

 /*! @{ */

#include "testtileservice.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "palfile.h"
#include "palstdlib.h"

#include "nbcontext.h"
#include "nbtilemanager.h"
#include "nbrastertiledatasourcehandler.h"
#include "nbrastertiledatasourceparameters.h"
#include "nbrastertiledatasourceinformation.h"


// Defines/Constants .............................................................................

// Comment this define if you want to keep the images after the tests have run
#define CLEANUP_IMAGE_FILES

// Define to decide the stress test. Large number might not work on windows mobile (only on win32)
// because of the large amount of data.
#define PERFORM_LONG_STRESS_TEST

#ifdef PERFORM_LONG_STRESS_TEST
    const int TM_STRESS_TEST_MULTIPLIER = 20;
#else
    const int TM_STRESS_TEST_MULTIPLIER = 1;
#endif

#define TILE_PREFIX            "TestTMTile"
#define TILE_NAME_PNG          TILE_PREFIX "%03d.png"
#define TESTREQUESTARRAYCOUNT 16

const uint32 TM_CALLBACK_TIMEOUT = 20 * 1000;


// Local types ...................................................................................

/*! Type for raster tiles. */
typedef enum
{
    RTT_None     = 0,
    RTT_Map,
    RTT_Traffic,
    RTT_Satellite

} RasterTileType;

/*! Data about a tile. */
typedef struct
{
    int priority;
    NB_TileInformation information;

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

/*! Test data passed to the receive callback. */
typedef struct
{
    PAL_Instance* pal;

    // Number of tiles requested for the current test. Gets set in the test and checked in the callback.
    int* requestedTiles;
    int* currentTileCount;
    TileData receivedTiles[500];
} TestDataWithSharedValues;

// Variables .....................................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

// Counter for all image files.
static int g_ImageNameCounter = 0;


// Private Functions .............................................................................

// Smoke tests
static void TestTileServiceInitializeDataSource(void);
static void TestTileServiceReceiveOne(void);
static void TestTileServiceReceiveMultiple(void);
static void TestTileServiceReceiveTrafficOne(void);
static void TestTileServiceReceiveSatelliteMultiple(void);
static void TestTileServiceReceiveMultipleWithDifferentTypes(void);
static void TestTileServiceReceiveMultipleWithDifferentTypesAndCallbacks(void);
static void TestTileServiceCacheOne(void);
static void TestTileServiceCacheMultiple(void);

// Intermediate tests
static void TestTileServiceBulkRequest(void);
static void TestTileServiceBulkRequestWithDifferentTypes(void);
static void TestTileServiceReuseConnection(void);
static void TestTileServicePendingTiles(void);
static void TestTileServiceExceedLockedList(void);
static void TestTileServicePersistentCache(void);
static void TestTileServiceFlushTiles(void);
static void TestTileServiceForceDownload(void);
static void TestTileServiceFailedTiles(void);
static void TestTileServiceDoesExistTypeOfTiles(void);
static void TestUserParametersAtEnd(void);
static void TestUserParametersAtMiddle(void);
static void TestPOITiles(void);
static void TestSingleURL(void);
static void TestPOISingleURL(void);

// for NBI phase 2 CCC part
static void TestTileServiceTileManagerCreateNullContext(void);
static void TestTileServiceTileManagerDestroyNullInput(void);
static void TestTileServiceTileManagerDoesTileTypeExistNullInput(void);
static void TestTileServiceTileManagerDoesTileTypeExistNullInputTypeId(void);
static void TestTileServiceTileManagerDoesTileTypeNullInput(void);
static void TestTileServiceTileManagerDoesTileTypeNullResult(void);
static void TestTileServiceTileManagerGetTileNullCommand(void);
static void TestTileServiceTileManagerFlushAllTilesNullInput(void);
static void TestTileServiceTileManagerFlushAllTilesSpecialParam(void);
static void TestTileServiceTileManagerTileTypeIsAvailableNullInput(void);
static void TestTileServiceTileManagerTileTypeIsAvailableNullResult(void);
static void TestTileServiceTileManagerTileTypeIsAvailable(void);
static void TestTileServiceTileManagerClearCacheNbTccAllOption(void);
static void TestTileServiceTileManagerClearCacheNbTccMemoryOnlyOption(void);
static void TestTileServiceTileManagerClearCacheNbTccUnlockedOnlyOption(void);
static void TestTileServiceTileManagerRemoveTileType(void);
static void TestTileServiceTileManagerRemoveTileType2(void);
static void TestTileServiceTileManagerSetRoutesNullInput(void);
static void TestTileServiceTileManagerSetRoutesNonTilesType(void);
static void TestTileServiceTileManagerSetRoutes(void);
static void TestTileServiceTileManagerGetTileKeyNullInput(void);
static void TestTileServiceTileManagerGetTileKeyNullInfo(void);
static void TestTileServiceTileManagerGetTileKeyNullKey(void);
static void TestTileServiceTileManagerGetTileKeyNullLength(void);
static void TestTileServiceTileManagerGetTileKey(void);
static void TestTileServiceTileManagerGetTileSpecialParam(void);
static void TestTileServiceTileManagerGetTilesSpecialParam1(void);
static void TestTileServiceTileManagerGetTilesSpecialParam2(void);
static void TestTileServiceTileManagerClearCache1(void);
static void TestTileServiceTileManagerGetTilesCheck(void);

// Full tests
static void TestTileServiceStressTest(void);

// Callback functions for different tests
static void DataSourceHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static nb_boolean DownloadCallback(NB_TileManager* manager, NB_Error result, const NB_Tile* tile, void* userData);
static nb_boolean DownloadCallbackWithSharedCounters(NB_TileManager* manager, NB_Error result, const NB_Tile* tile, void* userData);
static nb_boolean LockedTilesDownloadCallback(NB_TileManager* manager, NB_Error result, const NB_Tile* tile, void* userData);
static nb_boolean FailedTilesDownloadCallback(NB_TileManager* manager, NB_Error result, const NB_Tile* tile, void* userData);

// Helper functions
static NB_RasterTileDataSourceData* RetrieveDataSource(NB_Context* context);
static void CleanupImageFiles();
static void CleanupCacheFiles();
static void RequestTile(NB_TileManager* manager, NB_Tile* tile, uint32 priority, int count);
static void RequestTileEx(NB_TileManager* manager, NB_Tile* tile, uint32 priority, int count, const NB_TileManagerCallback* callback);
static void VerifyTileFromPersistentCache(NB_TileManager* manager, uint32 zoom, uint32 x, uint32 y);
static void VerifyTile(const NB_TileInformation* information, uint32 zoom, uint32 x, uint32 y);

static NB_Error InitializeConfigurationForRasterTiles(NB_Context* context,
                                      NB_TileDataSourceTemplate* dataSource,
                                      NB_TileConfiguration* connectionConfiguration,
                                      RasterTileType type);
static NB_Error CloneTileDataSourceTemplate(NB_TileDataSourceTemplate *dest,
                                            const NB_TileDataSourceTemplate *src);
static void CleanupTileDataSource(NB_TileDataSourceTemplate* p);



// Public Functions ..............................................................................

/*! Add all your test functions here

@return None
*/
void
TestTileService_AddTests( CU_pSuite pTestSuite, int level )
{
    // Smoke tests
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceInitializeDataSource",       TestTileServiceInitializeDataSource);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveOne",                 TestTileServiceReceiveOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveMultiple",            TestTileServiceReceiveMultiple);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveMultipleWithDifferentTypes", TestTileServiceReceiveMultipleWithDifferentTypes);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveMultipleWithDifferentTypesAndCallbacks", TestTileServiceReceiveMultipleWithDifferentTypesAndCallbacks);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveTrafficOne",          TestTileServiceReceiveTrafficOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceReceiveSatelliteMultiple",   TestTileServiceReceiveSatelliteMultiple);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceCacheOne",                   TestTileServiceCacheOne);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceCacheMultiple",              TestTileServiceCacheMultiple);

    // NBI ph2 CCC tests (tilemanager)
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerCreateNullContext", TestTileServiceTileManagerCreateNullContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerDestroyNullInput", TestTileServiceTileManagerDestroyNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerDoesTileTypeExistNullInput", TestTileServiceTileManagerDoesTileTypeExistNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerDoesTileTypeExistNullInputTypeId", TestTileServiceTileManagerDoesTileTypeExistNullInputTypeId);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerDoesTileTypeNullInput", TestTileServiceTileManagerDoesTileTypeNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerDoesTileTypeNullResult", TestTileServiceTileManagerDoesTileTypeNullResult);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileNullCommand", TestTileServiceTileManagerGetTileNullCommand);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerFlushAllTilesNullInput", TestTileServiceTileManagerFlushAllTilesNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerFlushAllTilesSpecialParam", TestTileServiceTileManagerFlushAllTilesSpecialParam);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerTileTypeIsAvailableNullInput", TestTileServiceTileManagerTileTypeIsAvailableNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerTileTypeIsAvailableNullResult", TestTileServiceTileManagerTileTypeIsAvailableNullResult);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerTileTypeIsAvailable", TestTileServiceTileManagerTileTypeIsAvailable);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerClearCacheNbTccAllOption", TestTileServiceTileManagerClearCacheNbTccAllOption);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerClearCacheNbTccMemoryOnlyOption", TestTileServiceTileManagerClearCacheNbTccMemoryOnlyOption);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerClearCacheNbTccUnlockedOnlyOption", TestTileServiceTileManagerClearCacheNbTccUnlockedOnlyOption);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerClearCache1", TestTileServiceTileManagerClearCache1);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerRemoveTileType", TestTileServiceTileManagerRemoveTileType);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerRemoveTileType2", TestTileServiceTileManagerRemoveTileType2);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerSetRoutesNullInput", TestTileServiceTileManagerSetRoutesNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerSetRoutesNonTilesType", TestTileServiceTileManagerSetRoutesNonTilesType);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerSetRoutes", TestTileServiceTileManagerSetRoutes);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileKeyNullInput", TestTileServiceTileManagerGetTileKeyNullInput);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileKeyNullInfo", TestTileServiceTileManagerGetTileKeyNullInfo);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileKeyNullKey", TestTileServiceTileManagerGetTileKeyNullKey);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileKeyNullLength", TestTileServiceTileManagerGetTileKeyNullLength);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileKey", TestTileServiceTileManagerGetTileKey);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTileSpecialParam", TestTileServiceTileManagerGetTileSpecialParam);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTilesSpecialParam1", TestTileServiceTileManagerGetTilesSpecialParam1);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTilesSpecialParam2", TestTileServiceTileManagerGetTilesSpecialParam2);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTileServiceTileManagerGetTilesCheck", TestTileServiceTileManagerGetTilesCheck);

    // Intermediate tests
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceBulkRequest",         TestTileServiceBulkRequest);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceBulkRequestWithDifferentTypes", TestTileServiceBulkRequestWithDifferentTypes);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceReuseConnection",     TestTileServiceReuseConnection);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServicePendingTiles",        TestTileServicePendingTiles);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceExceedLockedList",    TestTileServiceExceedLockedList);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServicePersistentCache",     TestTileServicePersistentCache);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceFlushTiles",          TestTileServiceFlushTiles);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceForceDownload",       TestTileServiceForceDownload);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceFailedTiles",         TestTileServiceFailedTiles);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestTileServiceDoesExistTypeOfTiles",TestTileServiceDoesExistTypeOfTiles);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestUserParametersAtEnd",            TestUserParametersAtEnd );
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestUserParametersAtMiddle",         TestUserParametersAtMiddle );
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPOITiles",                       TestPOITiles );
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSingleURL",                      TestSingleURL );
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestPOISingleURL",                   TestPOISingleURL );

    // Full tests
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestTileServiceStressTest",  TestTileServiceStressTest);
};


/*! Add common initialization code here.

@return 0

@see TestTileService_SuiteCleanup
*/
int
TestTileService_SuiteSetup()
{
    // Get rid of any existing images before we run the test
    CleanupImageFiles();

    CleanupCacheFiles();

    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestTileService_SuiteSetup
*/
int
TestTileService_SuiteCleanup()
{
#ifdef CLEANUP_IMAGE_FILES
    CleanupImageFiles();
#endif

    CleanupCacheFiles();

    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}


// Private Functions .............................................................................

/*! Test initialization of the data sources

    @return None. Asserts are called on failure.
*/
void
TestTileServiceInitializeDataSource(void)
{
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

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

/*! Test raster tile retrieval.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};


        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START + 1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Test retrieval of multiple tiles at the same time.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }
        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of multiple tiles at the same time.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveMultipleWithDifferentTypesAndCallbacks(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTileMap = {0};
        NB_TileDataSourceTemplate dataSourceTileSatellite = {0};
        NB_TileConfiguration configuration = {0};

        NB_TileManagerCallback callbackDataMap = {0};
        NB_TileManagerCallback callbackDataMap2 = {0};
        NB_TileManagerCallback callbackDataMap3 = {0};
        TestDataWithSharedValues* testData = nsl_malloc(sizeof(*testData));
        TestDataWithSharedValues* testData2 = nsl_malloc(sizeof(*testData2));
        TestDataWithSharedValues* testData3 = nsl_malloc(sizeof(*testData3));

        testData->pal = pal;
        testData2->pal = pal;
        testData3->pal = pal;

        // use shared values to verify the test
        testData->currentTileCount = nsl_malloc(sizeof(int));
        testData->requestedTiles = nsl_malloc(sizeof(int));
        testData2->currentTileCount = testData->currentTileCount;
        testData2->requestedTiles = testData->requestedTiles;
        testData3->currentTileCount = testData->currentTileCount;
        testData3->requestedTiles = testData->requestedTiles;

        (*testData->requestedTiles) = 0;
        (*testData->currentTileCount) = 0;

        callbackDataMap.callback = &DownloadCallbackWithSharedCounters;
        callbackDataMap.callbackData = testData;
        callbackDataMap2.callback = &DownloadCallbackWithSharedCounters;
        callbackDataMap2.callbackData = testData2;
        callbackDataMap3.callback = &DownloadCallbackWithSharedCounters;
        callbackDataMap3.callbackData = testData3;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileMap, &configuration, RTT_Map);
        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileSatellite, &configuration, RTT_Satellite);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, NULL, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeMapId = 0;
            uint32 tileTypeTrafficId = 0;

            result = NB_TileManagerAddTileType(manager, &dataSourceTileMap, &configuration, &tileTypeMapId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_TileManagerAddTileType(manager, &dataSourceTileSatellite, &configuration, &tileTypeTrafficId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11322;
                const uint32 Y_POSITION_START = 26224;

                NB_Tile tileMap = {{0}};
                NB_Tile tileTraffic = {{0}};

                tileMap.information.tileTypeId = tileTypeMapId;
                tileMap.information.zoomLevel = ZOOM;
                tileMap.information.xPosition = X_POSITION_START;
                tileMap.information.yPosition = Y_POSITION_START;

                tileTraffic.information.tileTypeId = tileTypeTrafficId;
                tileTraffic.information.zoomLevel = ZOOM;
                tileTraffic.information.xPosition = X_POSITION_START;
                tileTraffic.information.yPosition = Y_POSITION_START;
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap2);
                RequestTileEx(manager, &tileTraffic, 10, (*testData->requestedTiles)++, &callbackDataMap3);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap2);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap2);
                RequestTileEx(manager, &tileTraffic, 10, (*testData->requestedTiles)++, &callbackDataMap3);
                RequestTileEx(manager, &tileMap, 10, (*testData->requestedTiles)++, &callbackDataMap2);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }
        }

        nsl_free(testData->currentTileCount);
        nsl_free(testData->requestedTiles);
        testData2->currentTileCount = NULL;
        testData2->requestedTiles = NULL;
        testData3->currentTileCount = NULL;
        testData3->requestedTiles = NULL;
        nsl_free(testData);
        nsl_free(testData2);
        nsl_free(testData3);
        CleanupTileDataSource(&dataSourceTileMap);
        CleanupTileDataSource(&dataSourceTileSatellite);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of multiple tiles at the same time.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveMultipleWithDifferentTypes(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTileMap = {0};
        NB_TileDataSourceTemplate dataSourceTileTraffic = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 10;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileMap, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileTraffic, &configuration, RTT_Traffic);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeMapId = 0;
            uint32 tileTypeTrafficId = 0;

            result = NB_TileManagerAddTileType(manager, &dataSourceTileMap, &configuration, &tileTypeMapId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_TileManagerAddTileType(manager, &dataSourceTileTraffic, &configuration, &tileTypeTrafficId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeMapId;
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

                tile.information.tileTypeId = tileTypeTrafficId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 5 tiles. All 5 tiles should be downloaded simultaneoulsy.
                // The y-position get incremented by the function
                RequestTile(manager, &tile, 10, 6);
                RequestTile(manager, &tile, 10, 7);
                RequestTile(manager, &tile, 10, 8);
                RequestTile(manager, &tile, 10, 9);
                RequestTile(manager, &tile, 10, 10);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }
        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTileMap);
        CleanupTileDataSource(&dataSourceTileTraffic);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of one traffic tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveTrafficOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Traffic);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;

            configuration.isCacheable = FALSE;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                // Downtown L.A. should always have traffic information
                const uint32 ZOOM = 12;
                const uint32 X_POSITION_START = 702;
                const uint32 Y_POSITION_START = 1635;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test retrieval of multiple traffic tiles at the same time.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReceiveSatelliteMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 20;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Traffic);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;

            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = 12;

                // Request 20 tiles. Request traffic information from tiles which cover freeways in the L.A. area where
                // we know we should receive traffic.

                tile.information.xPosition = 700;
                tile.information.yPosition = 1632;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 700;
                tile.information.yPosition = 1633;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1632;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1633;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 701;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 702;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 702;
                tile.information.yPosition = 1635;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1634;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1635;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 703;
                tile.information.yPosition = 1636;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                // I-10 near Santa Monica/L.A.

                tile.information.zoomLevel = 13;

                tile.information.xPosition = 1400;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1401;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1402;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1403;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1404;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1405;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1404;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1405;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1406;
                tile.information.yPosition = 3270;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                tile.information.xPosition = 1406;
                tile.information.yPosition = 3271;
                CU_ASSERT_EQUAL(NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL), NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT*5))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test adding one item to the cache and retrieve it.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceCacheOne(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;

            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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
                result = NB_TileManagerGetTile(manager, &tile.information, 10, NB_TC_MemoryCache, &tile.buffer, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_TileManagerGetTile(manager, &tile.information, 10, NB_TC_PersistentCache, &tile.buffer, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                // Get again with different command, it still should return right away
                tile.buffer.buffer = NULL;
                tile.buffer.size = 0;
                result = NB_TileManagerGetTile(manager, &tile.information, 10, NB_TC_Download, &tile.buffer, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                CU_ASSERT(tile.buffer.size > 0);

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }


        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test caching of multiple entries

    @return None. Asserts are called on failure.
*/
void
TestTileServiceCacheMultiple(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                int i = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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

                    result = NB_TileManagerGetTile(manager, &tile.information, 10, NB_TC_MemoryCache, &tile.buffer, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
                    CU_ASSERT(tile.buffer.size > 0);
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test requesting multiple tiles in one single call.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceBulkRequest(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;

        #define MAX_BULK_REQUEST 50
        testData->requestedTiles = MAX_BULK_REQUEST; // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                NB_Tile tileArray[MAX_BULK_REQUEST] = {{{0}}};
                NB_TileRequest requestArray[MAX_BULK_REQUEST] = {{0}};

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

                    tileArray[i].information.tileTypeId = tileTypeId;
                    tileArray[i].information.xPosition  = x++;
                    tileArray[i].information.yPosition  = y;
                    tileArray[i].information.zoomLevel  = zoom;

                    requestArray[i].information         = &tileArray[i].information;
                    requestArray[i].priority            = priority;
                    requestArray[i].command             = NB_TC_Download;
                    requestArray[i].buffer              = &tileArray[i].buffer;
                }

                result = NB_TileManagerGetTiles(manager, requestArray, MAX_BULK_REQUEST);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 100000))
                {
                    // Done
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}


/*! Test requesting multiple tiles in one single call with different types of tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceBulkRequestWithDifferentTypes(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTileMap = {0};
        NB_TileDataSourceTemplate dataSourceTileSatellite = {0};

        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;

        testData->requestedTiles = MAX_BULK_REQUEST; // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileMap, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileSatellite, &configuration, RTT_Satellite);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeMapId = 0;
            uint32 tileTypeSatelliteId = 0;

            result = NB_TileManagerAddTileType(manager, &dataSourceTileMap, &configuration, &tileTypeMapId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_TileManagerAddTileType(manager, &dataSourceTileSatellite, &configuration, &tileTypeSatelliteId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                NB_Tile tileArray[MAX_BULK_REQUEST] = {{{0}}};
                NB_TileRequest requestArray[MAX_BULK_REQUEST] = {{0}};

                int i = 0;
                uint32 zoom = 15;
                uint32 x = 5670;
                uint32 y = 13141;
                uint32 priority = 50000;
                int count = MAX_BULK_REQUEST / 2;

                // Fill array for request
                for (i = 0; i < count; ++i)
                {
                    priority -= 10;
                    if ((i % 20) == 0)
                    {
                        y++;
                    }

                    tileArray[i].information.tileTypeId = tileTypeMapId;
                    tileArray[i].information.xPosition  = x++;
                    tileArray[i].information.yPosition  = y;
                    tileArray[i].information.zoomLevel  = zoom;

                    requestArray[i].information         = &tileArray[i].information;
                    requestArray[i].priority            = priority;
                    requestArray[i].command             = NB_TC_Download;
                    requestArray[i].buffer              = &tileArray[i].buffer;
                }
                // Fill array for request
                for (i = count; i < MAX_BULK_REQUEST; ++i)
                {
                    priority -= 10;
                    if ((i % 20) == 0)
                    {
                        y++;
                    }

                    tileArray[i].information.tileTypeId = tileTypeSatelliteId;
                    tileArray[i].information.xPosition  = x++;
                    tileArray[i].information.yPosition  = y;
                    tileArray[i].information.zoomLevel  = zoom;

                    requestArray[i].information         = &tileArray[i].information;
                    requestArray[i].priority            = priority;
                    requestArray[i].command             = NB_TC_Download;
                    requestArray[i].buffer              = &tileArray[i].buffer;
                }

                result = NB_TileManagerGetTiles(manager, requestArray, MAX_BULK_REQUEST);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
                {
                    // Done
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTileMap);
        CleanupTileDataSource(&dataSourceTileSatellite);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}


/*! Test reusing an already open HTTP connection.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceReuseConnection(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 5;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.

                    // We can't verify the received tiles here (easily) since the order might not be the
                    // same as the order in which we requested them.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}


/*! Test processing pending tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServicePendingTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 10;           // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        configuration.maximumConcurrentRequests = 1;
        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // All tiles are downloaded successful.
                }
                else
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test the discarding of the tiles once we exceed the number of allowed locked tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceExceedLockedList(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 20 + 10; // This has to match 'maximumConcurrentMapRequests' + 'maximumPendingMapTiles' below.
                                            // Requests will be ignored once all connections are busy and the pending tile list is full.

        // For this test we use a different download to verify the expected failures.
        callbackData.callback = &LockedTilesDownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        configuration.maximumConcurrentRequests = 10;
        configuration.maximumPendingTiles = 20;

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                int i = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test the persistent cache.

    @return None. Asserts are called on failure.
*/
void
TestTileServicePersistentCache(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

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

        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 0;            // Set below!

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_TileManagerSaveToPersistentCache(manager, -1, &savedEntries);
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
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_TileManagerSaveToPersistentCache(manager, -1, &savedEntries);
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
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_TileManagerSaveToPersistentCache(manager, -1, &savedEntries);
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
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    CU_FAIL("Raster tile download timed out!");
                }

                // Save the 5 tiles to the persistent cache
                result = NB_TileManagerSaveToPersistentCache(manager, -1, &savedEntries);
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                manager = NULL;
            }
            CleanupTileDataSource(&dataSourceTile);
            result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_TileManagerCreate(context, &callbackData, &manager);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                uint32 tileTypeId = 0;
                result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
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

                    result = NB_TileManagerDestroy(manager);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

            }
        }
        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}


/*! Test flushing tiles from the pending list.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceFlushTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 8;           // Has to match the number of RequestTile() calls below (after the flush calls)

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);
        // We use only one concurrent connection for this test so that subsequent calls go
        // immediately into the pending tile vector.
        configuration.maximumConcurrentRequests = 1;

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11211;
                const uint32 Y_POSITION_START = 26222;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                result = NB_TileManagerFlushAllTiles(manager);
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
                result = NB_TileManagerFlushAllTiles(manager);
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
                result = NB_TileManagerFlushTile(manager, &tile.information, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Flush 25
                tile.information.xPosition = X_POSITION_START + 25;
                result = NB_TileManagerFlushTile(manager, &tile.information, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Flush 28
                tile.information.xPosition = X_POSITION_START + 28;
                result = NB_TileManagerFlushTile(manager, &tile.information, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event. The callback will trigger once all tiles are downloaded.
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                {
                    // These are the remaining 8 tiles received (the very first tile + the last 10 minus the three removed.)

                      //We cannot check tiles with such way - callbacks can be called in any order,
                      //So, we should sort received tiles array by x_position
                    int i = 0, j = 0;

                    for(i = 0; i < 8; i++)
                    {
                        for(j = 0; j < 8 - i - 1; j++)
                        {
                            if ( testData->receivedTiles[j].information.xPosition > testData->receivedTiles[j+1].information.xPosition)
                            {
                                TileData tmp = testData->receivedTiles[j];
                                testData->receivedTiles[j] = testData->receivedTiles[j+1];
                                testData->receivedTiles[j+1] = tmp;
                            }
                        }
                    }

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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Force download of already downloaded tile.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceForceDownload(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request tile
                result = NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Download, &tile.buffer, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile.
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.

                        testData->requestedTiles = 2;

                        // Request the same tile again. Set the command to force-download. This will remove it from the
                        // cache and force the download again.
                        result = NB_TileManagerGetTile(manager, &tile.information, 100, NB_TC_Force_Download, &tile.buffer, NULL);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        // Returned buffer should be NULL since we forced the download
                        CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                        CU_ASSERT_EQUAL(tile.buffer.size, 0);

                        if (result == NE_OK)
                        {
                            // Wait for the callback to trigger the event
                            if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Test handling of failed tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceFailedTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 200;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &FailedTilesDownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        configuration.maximumConcurrentRequests = 5;
        configuration.maximumPendingTiles = 1000;

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
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

                    NB_Tile tile = {{0}};
                    tile.information.tileTypeId = tileTypeId;
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

                    tile.information.tileTypeId = tileTypeId;
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Do an extensive test with lots of tiles.

    @return None. Asserts are called on failure.
*/
void
TestTileServiceStressTest(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        NB_TileManagerCallback callbackData = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;

        testData->requestedTiles = 100 * TM_STRESS_TEST_MULTIPLIER; // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            uint32 tileTypeId = 0;
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                int i = 0;
                int requestCount = 0;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                // Request 100 tiles with different priorities with 10 concurrent connections. Pause inbetween
                // to simulate asynchronous requests.

                for (i = 0; i < 8 * TM_STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 10);

                for (i = 0; i < 8 * TM_STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 8 * TM_STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 8 * TM_STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 50 * TM_STRESS_TEST_MULTIPLIER; i++)
                {
                    RequestTile(manager, &tile, 20 - i, ++requestCount);
                }

                // Wait just a few milliseconds to start (continue) the other requests
                WaitForCallbackCompletedEvent(g_CallbackEvent, 50);

                for (i = 0; i < 18 * TM_STRESS_TEST_MULTIPLIER; i++)
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

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

        }

        nsl_free(testData);
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }

    CleanupCacheFiles();
}

/*! Verify NB_TileManagerDoesTileTypeExist.

    @return None. Asserts are called on failure.
*/
static void TestTileServiceDoesExistTypeOfTiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 i = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileDataSourceTemplate dataSourceTileArr[4] = {{0}};
        uint32                          tileTypeIds[4] = {0};
        NB_TileConfiguration configuration = {0};

        //Configure first dataSource
        result = InitializeConfigurationForRasterTiles(context, &dataSourceTileArr[0], &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        if(result != NE_OK)
        {
            return;
        }
        //Filling all dataStoreTemplates

        //Next one with single URL
        CloneTileDataSourceTemplate(&dataSourceTileArr[1], &dataSourceTileArr[0]);
        nsl_free(dataSourceTileArr[1].baseUrl);
        dataSourceTileArr[1].baseUrl = NULL;
        nsl_free(dataSourceTileArr[1].templateUrl);
        dataSourceTileArr[1].templateUrl = NULL;
        nsl_free(dataSourceTileArr[1].mimeType);
        dataSourceTileArr[1].mimeType = NULL;

        dataSourceTileArr[1].mimeType = nsl_strdup( "image/gif" );
        dataSourceTileArr[1].templateUrl = nsl_malloc(nsl_strlen(dataSourceTileArr[0].baseUrl) + nsl_strlen(dataSourceTileArr[0].templateUrl) + 1);
        nsl_strcpy(dataSourceTileArr[1].templateUrl, dataSourceTileArr[0].baseUrl);
        nsl_strcat(dataSourceTileArr[1].templateUrl, dataSourceTileArr[0].templateUrl);

        //One more with(out) tileStoreTemplate
        CloneTileDataSourceTemplate(&dataSourceTileArr[2], &dataSourceTileArr[0]);
        if(!dataSourceTileArr[2].tileStoreTemplate)
        {
            dataSourceTileArr[2].tileStoreTemplate = nsl_strdup( "bing-1-$x-$y-$z-$q" );
        }
        else
        {
            nsl_free(dataSourceTileArr[2].tileStoreTemplate);
            dataSourceTileArr[2].tileStoreTemplate = NULL;
        }

        //Last one with SingleURL and with(out) tileStoreTemplate
        CloneTileDataSourceTemplate(&dataSourceTileArr[3], &dataSourceTileArr[1]);
        if(!dataSourceTileArr[3].tileStoreTemplate)
        {
            dataSourceTileArr[3].tileStoreTemplate = nsl_strdup( "bing-2-$x-$y-$z-$q" );
        }
        else
        {
            nsl_free(dataSourceTileArr[3].tileStoreTemplate);
            dataSourceTileArr[3].tileStoreTemplate = NULL;
        }

        result = NB_TileManagerCreate(context, NULL, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            for (i = 0; i < sizeof(dataSourceTileArr)/sizeof(dataSourceTileArr[0]); i++)
            {
                result = NB_TileManagerAddTileType(manager,
                                                   &dataSourceTileArr[i],
                                                   &configuration,
                                                   &tileTypeIds[i],
                                                   NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_NOT_EQUAL(tileTypeIds[i], (uint32)-1);
            }

            for (i = 0; i < sizeof(dataSourceTileArr)/sizeof(dataSourceTileArr[0]); i++)
            {
                uint32 tileTypeId = 0;

                result = NB_TileManagerDoesTileTypeExist(manager, &dataSourceTileArr[i], &tileTypeId);

                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_EQUAL(tileTypeId, tileTypeIds[i]);
            }

            result = NB_TileManagerDestroy(manager);
        }

        for (i = 0; i < sizeof(dataSourceTileArr)/sizeof(dataSourceTileArr[0]); i++)
        {
            CleanupTileDataSource(&dataSourceTileArr[i]);
        }

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

/*! Test user parameters in single URL.

    @return None. Asserts are called on failure.
*/
void
TestUserParametersAtEnd(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;
    char* strTmp = NULL;
    char tmp[256] = {0};

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};


        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (! dataSourceTile.templateUrl ||
            ! dataSourceTile.baseUrl
            )
        {
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.templateUrl);
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.baseUrl);
            return  ;
        }
        
        strTmp = nsl_stristr(dataSourceTile.templateUrl, "sz");
        if ( strTmp )
        {
          nsl_strlcpy(strTmp, "$user", nsl_strlen(strTmp));
        }

        // perform single URL
        nsl_strlcpy(tmp,dataSourceTile.baseUrl, sizeof(tmp));
        nsl_strlcat(tmp,dataSourceTile.templateUrl, sizeof(tmp));
        nsl_free(dataSourceTile.templateUrl);
        nsl_free(dataSourceTile.baseUrl);
        dataSourceTile.baseUrl = NULL;
        dataSourceTile.templateUrl = nsl_strdup(tmp);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                nb_boolean needAdditionalURL = FALSE;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11342;
                const uint32 Y_POSITION_START = 26264;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;
                (void)NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter( manager,
                            tileTypeId,
                            &needAdditionalURL );
                if ( needAdditionalURL )
                {
                    tile.information.additionalPartOfUrl = nsl_strdup("sz=256");
                }
                else
                {
                    tile.information.additionalPartOfUrl = NULL;
                }

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START+1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (tile.information.additionalPartOfUrl)
                {
                    nsl_free(tile.information.additionalPartOfUrl);
                }
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Test user parameters in single URL.

    @return None. Asserts are called on failure.
*/
void
TestUserParametersAtMiddle(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;
    char* strTmp = NULL;
    char tmp[256] = {0};

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};


        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (! dataSourceTile.templateUrl ||
            ! dataSourceTile.baseUrl
            )
        {
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.templateUrl);
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.baseUrl);
            return  ;
        }

        // Set up $user in URL
        strTmp = nsl_stristr( dataSourceTile.templateUrl, "$x" );
        if ( strTmp )
        {
            strTmp[0] = '%';
            strTmp[1] = 's';
            strTmp = nsl_strdup( dataSourceTile.templateUrl );
            if ( strTmp )
            {
              nsl_free(dataSourceTile.templateUrl);
              dataSourceTile.templateUrl = (char*)nsl_malloc(nsl_strlen(strTmp) + 10);
              nsl_sprintf(dataSourceTile.templateUrl, strTmp, "$x&$user" );
              nsl_free( strTmp );
              strTmp = NULL;
            }
        }
        strTmp = nsl_stristr( dataSourceTile.templateUrl, "&sz" );
        *strTmp = 0;

        // perform single URL
        nsl_strlcpy(tmp,dataSourceTile.baseUrl, sizeof(tmp));
        nsl_strlcat(tmp,dataSourceTile.templateUrl, sizeof(tmp));
        nsl_free(dataSourceTile.templateUrl);
        nsl_free(dataSourceTile.baseUrl);
        dataSourceTile.baseUrl = NULL;
        dataSourceTile.templateUrl = nsl_strdup(tmp);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                nb_boolean needAdditionalURL = FALSE;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11345;
                const uint32 Y_POSITION_START = 26267;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;
                (void)NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter( manager,
                            tileTypeId,
                            &needAdditionalURL );
                if ( needAdditionalURL )
                {
                    tile.information.additionalPartOfUrl = nsl_strdup("sz=256");
                }
                else
                {
                    tile.information.additionalPartOfUrl = NULL;
                }

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START+1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                if(tile.information.additionalPartOfUrl)
                {
                    nsl_free(tile.information.additionalPartOfUrl);
                }
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Test POI tiles handling.

    @return None. Asserts are called on failure.
*/
static void TestPOITiles(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        //Set valid POI URLs
        nsl_free(dataSourceTile.templateUrl);
        nsl_free(dataSourceTile.baseUrl);
        dataSourceTile.templateUrl = nsl_strdup("/maps/api/place/search/json?$user");
        dataSourceTile.baseUrl = nsl_strdup("https://maps.googleapis.com");

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                nb_boolean needAdditionalURL = FALSE;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11345;
                const uint32 Y_POSITION_START = 26267;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;
                result = NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter(manager,
                            tileTypeId,
                            &needAdditionalURL );
                CU_ASSERT_EQUAL(result, NE_OK);
                if ( needAdditionalURL )
                {
                    tile.information.additionalPartOfUrl = nsl_strdup("location=-33.8670522,151.1957362&radius=500&types=food&name=harbour&sensor=true&key=AIzaSyAiFpFd85eMtfbvmVNEYuNds5TEF9FjIPI");
                }
                else
                {
                    tile.information.additionalPartOfUrl = NULL;
                }

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START+1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (tile.information.additionalPartOfUrl)
                {
                    nsl_free(tile.information.additionalPartOfUrl);
                }
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Test single URL handling.

    @return None. Asserts are called on failure.
*/
static void TestSingleURL(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};
        char tmp[256] = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (! dataSourceTile.templateUrl ||
            ! dataSourceTile.baseUrl
            )
        {
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.templateUrl);
            CU_ASSERT_PTR_NOT_NULL(dataSourceTile.baseUrl);
            return  ;
        }

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Set single URL
        nsl_strlcpy(tmp,dataSourceTile.baseUrl, sizeof(tmp));
        nsl_strlcat(tmp,dataSourceTile.templateUrl, sizeof(tmp));
        nsl_free(dataSourceTile.templateUrl);
        nsl_free(dataSourceTile.baseUrl);
        dataSourceTile.baseUrl = NULL;
        dataSourceTile.templateUrl = nsl_strdup(tmp);

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11345;
                const uint32 Y_POSITION_START = 26267;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
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
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START+1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Test POI tiles with single URL handling.

    @return None. Asserts are called on failure.
*/
static void TestPOISingleURL(void)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_Context* context = NULL;
    uint32 tileTypeId = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if (palResult)
    {
        NB_TileManager* manager = NULL;
        NB_TileManagerCallback callbackData = {0};
        NB_TileDataSourceTemplate dataSourceTile = {0};
        NB_TileConfiguration configuration = {0};

        // Test data passed to the callback
        TestData* testData = nsl_malloc(sizeof(*testData));
        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->requestedTiles = 1;            // Has to match the number of RequestTile() calls below

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = InitializeConfigurationForRasterTiles(context, &dataSourceTile, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        // Set single POI URLs
        nsl_free(dataSourceTile.templateUrl);
        nsl_free(dataSourceTile.baseUrl);
        dataSourceTile.baseUrl = NULL;
        dataSourceTile.templateUrl = nsl_strdup("https://maps.googleapis.com/maps/api/place/search/json?$user");

        if (result == NE_OK)
        {
            result = NB_TileManagerAddTileType(manager, &dataSourceTile, &configuration, &tileTypeId, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                nb_boolean needAdditionalURL = FALSE;
                const uint32 ZOOM = 16;
                const uint32 X_POSITION_START = 11345;
                const uint32 Y_POSITION_START = 26267;

                NB_Tile tile = {{0}};
                tile.information.tileTypeId = tileTypeId;
                tile.information.zoomLevel = ZOOM;
                tile.information.xPosition = X_POSITION_START;
                tile.information.yPosition = Y_POSITION_START;

                result = NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter(manager,
                            tileTypeId,
                            &needAdditionalURL );
                CU_ASSERT_EQUAL(result, NE_OK);

                if (needAdditionalURL)
                {
                    tile.information.additionalPartOfUrl = nsl_strdup("location=-33.8670522,151.1957362&radius=500&types=food&name=harbour&sensor=true&key=AIzaSyAiFpFd85eMtfbvmVNEYuNds5TEF9FjIPI");
                }
                else
                {
                    tile.information.additionalPartOfUrl = NULL;
                }

                // Request one tile
                RequestTile(manager, &tile, 10, 1);

                // The first request should not return the buffer. It will be returned in the callback
                CU_ASSERT_PTR_NULL(tile.buffer.buffer);
                CU_ASSERT_EQUAL(tile.buffer.size, 0);

                if (result == NE_OK)
                {
                    // Wait for the callback to trigger the event
                    if (WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
                    {
                        // Validate the tile. x-position will be incremented by one
                        VerifyTile(&testData->receivedTiles[0].information, ZOOM, X_POSITION_START+1, Y_POSITION_START);

                        // Don't unlock tile here, it gets done in the callback.
                    }
                    else
                    {
                        CU_FAIL("tile download timed out!");
                    }
                }

                result = NB_TileManagerDestroy(manager);
                CU_ASSERT_EQUAL(result, NE_OK);
                if (tile.information.additionalPartOfUrl)
                {
                    nsl_free(tile.information.additionalPartOfUrl);
                }
            }

            nsl_free(testData);
        }
        CleanupTileDataSource(&dataSourceTile);
        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
    CleanupCacheFiles();
}

/*! Callback for data source handler.

    This function is used to retrieve the data sources.

    @return None
    @see TestTileServiceInitializeDataSource
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
    if (((status != NB_NetworkRequestStatus_Success) && (status != NB_NetworkRequestStatus_Progress)) ||
        (err != NE_OK) ||
        (localResult != NE_OK))
    {
        CU_FAIL("Error in data source callback");
    }
}

/*! Callback for tile download.

    Used for most tests.

    @return None

    @see NB_TileManagerCallbackFunction
*/

nb_boolean
DownloadCallback(NB_TileManager* manager,  /* see NB_TileManagerCallbackFunction() */
                 NB_Error result,
                 const NB_Tile* tile,
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
        nsl_memcpy(&testData->receivedTiles[testData->currentTileCount - 1].information, &tile->information, sizeof(NB_TileInformation));

        // Unlock the tile again.
        localResult = NB_TileManagerUnlockTile(manager, &tile->information);
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

    return TRUE;
}

/*! Callback for tile download.

    Used for most tests.

    @return None

    @see NB_TileManagerCallbackFunction
*/

nb_boolean
DownloadCallbackWithSharedCounters(NB_TileManager* manager,  /* see NB_TileManagerCallbackFunction() */
                 NB_Error result,
                 const NB_Tile* tile,
                 void* userData)
{
    TestDataWithSharedValues* testData = (TestDataWithSharedValues*)userData;

    
    ++(*testData->currentTileCount);

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
        nsl_memcpy(&testData->receivedTiles[*(testData->currentTileCount) - 1].information, &tile->information, sizeof(NB_TileInformation));

        // Unlock the tile again.
        localResult = NB_TileManagerUnlockTile(manager, &tile->information);
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
    if ((*testData->currentTileCount) == (*testData->requestedTiles))
    {
        // Trigger the test function
        SetCallbackCompletedEvent(g_CallbackEvent);
    }

    return TRUE;
}


/*! Special download for locked tile test.

    @return None

    @see TestTileServiceExceedLockedList
    @see NB_TileManagerCallbackFunction
*/
nb_boolean
LockedTilesDownloadCallback(NB_TileManager* manager,  /* see NB_TileManagerCallbackFunction() */
                            NB_Error result,
                            const NB_Tile* tile,
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

    return TRUE;
}

/*! Download callback for failed-tile test.

    @return None

    @see TestTileServiceFailedTiles
    @see NB_TileManagerCallbackFunction
*/
nb_boolean
FailedTilesDownloadCallback(NB_TileManager* manager,  /* see NB_TileManagerCallbackFunction() */
                            NB_Error result,
                            const NB_Tile* tile,
                            void* userData)
{
    TestData* testData = (TestData*)userData;

    ++testData->currentTileCount;

    // tile has to be valid even for failed tiles
    CU_ASSERT_PTR_NOT_NULL(tile);

    /*
        The tiles we request for zoom level 2 should fail, all others should succeed. See TestTileServiceFailedTiles()
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

    return TRUE;
}

/*! Retrieve the data source.

    The data source has to be retrieved before the tile manager is created. This function is used by all tests.
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
            if (!WaitForCallbackCompletedEvent(g_CallbackEvent, TM_CALLBACK_TIMEOUT))
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
RequestTile(NB_TileManager* manager, NB_Tile* tile, uint32 priority, int count)
{
    NB_Error result = NE_OK;

    tile->information.xPosition++;
    LOGOUTPUT(LogLevelMedium,
              ("Tile requested[%d]: %d, %d, %d\n",
               count,
               tile->information.zoomLevel,
               tile->information.xPosition,
               tile->information.yPosition));

    result = NB_TileManagerGetTile(manager, &tile->information, priority, NB_TC_Download, &tile->buffer, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
}

/*! Simple helper function to request tile for download with callback data.

    The xPosition of the passed in tile gets incremented by one.

    @return None. Asserts are called on failure.
*/
void
RequestTileEx(NB_TileManager* manager, NB_Tile* tile, uint32 priority, int count, const NB_TileManagerCallback* callback)
{
    NB_Error result = NE_OK;

    tile->information.xPosition++;
    LOGOUTPUT(LogLevelMedium,
              ("Tile requested[%d]: %d, %d, %d\n",
               count,
               tile->information.zoomLevel,
               tile->information.xPosition,
               tile->information.yPosition));

    result = NB_TileManagerGetTile(manager, &tile->information, priority, NB_TC_Download, &tile->buffer, callback);
    CU_ASSERT_EQUAL(result, NE_OK);
}

/*! Simple helper function to request tile from persistent cache and verify the result

    The xPosition of the passed in tile gets incremented by one.

    @return None. Asserts are called on failure.
*/
void
VerifyTileFromPersistentCache(NB_TileManager* manager, uint32 zoom, uint32 x, uint32 y)
{
    NB_Error result = NE_OK;
    NB_Tile tile = {{0}};
    tile.information.tileTypeId = 0;
    tile.information.xPosition = x;
    tile.information.yPosition = y;
    tile.information.zoomLevel = zoom;

    // Get the tile from the (memory or persistent) cache
    result = NB_TileManagerGetTile(manager, &tile.information, 10, NB_TC_PersistentCache, &tile.buffer, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result == NE_OK)
    {
        CU_ASSERT_PTR_NOT_NULL(tile.buffer.buffer);
        CU_ASSERT(tile.buffer.size > 0);

        // Unlock the tile immediately
        result = NB_TileManagerUnlockTile(manager, &tile.information);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

/*! Simple helper function to verify tile.

    @return None. Asserts are called on failure.
*/
void
VerifyTile(const NB_TileInformation* information,     /*!< Actual result */
           uint32 zoom,                                     /*!< Expected result */
           uint32 x,                                        /*!< Expected result */
           uint32 y                                         /*!< Expected result */
           )
{
    CU_ASSERT_EQUAL(information->zoomLevel, zoom);
    CU_ASSERT_EQUAL(information->xPosition, x);
    CU_ASSERT_EQUAL(information->yPosition, y);
}

/*! Set all the values for the tile manager configuration. */
NB_Error
InitializeConfigurationForRasterTiles(NB_Context* context,
                                      NB_TileDataSourceTemplate* dataSource,
                                      NB_TileConfiguration* configuration,
                                      RasterTileType type)
{
    NB_RasterTileDataSourceData * dataSourceData = NULL;

    dataSourceData = RetrieveDataSource(context);
    CU_ASSERT_PTR_NOT_NULL(dataSourceData);

    if (dataSourceData)
    {
        nsl_memset(dataSource, 0, sizeof(NB_TileDataSourceTemplate));

        dataSource->generationInfo = nsl_strdup(dataSourceData->templateMap.generationInfo);
        dataSource->baseUrl        = nsl_strdup(dataSourceData->templateMap.baseUrl);
        dataSource->templateUrl    = nsl_strdup(dataSourceData->templateMap.templateUrl);
        dataSource->mimeType       = nsl_strdup(NB_MIME_TYPE_PNG);
        dataSource->tileSize = dataSourceData->templateMap.tileSize;
        dataSource->cacheId = dataSourceData->templateMap.cacheId;
        dataSource->projection = dataSourceData->templateMap.projection;
        dataSource->isInternalSource = dataSourceData->templateMap.isInternalSource;

        configuration->maximumConcurrentRequests = 2;
        configuration->maximumPendingTiles = 50;
        configuration->isCacheable = TRUE;
        configuration->expirationTime = 0;

        nsl_free(dataSourceData);
        return NE_OK;
    }
    else
    {
        return NE_NOMEM;
    }
}

void CleanupTileDataSource(NB_TileDataSourceTemplate* p)
{
    if(!p)
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

/*! Make copy of TileDataSourceTemplate */
NB_Error
CloneTileDataSourceTemplate(NB_TileDataSourceTemplate *dest,
                            const NB_TileDataSourceTemplate *src)
{
    if (!dest || !src)
    {
        return NE_INVAL;
    }

    //copy static fields
    *dest = *src;

    //copy strings
    (src->generationInfo && nsl_strlen(src->generationInfo) > 0)       ? (dest->generationInfo = nsl_strdup(src->generationInfo))       : (dest->generationInfo = NULL);
    (src->baseUrl && nsl_strlen(src->baseUrl) > 0)                     ? (dest->baseUrl = nsl_strdup(src->baseUrl))                     : (dest->baseUrl = NULL);
    (src->templateUrl && nsl_strlen(src->templateUrl) > 0)             ? (dest->templateUrl = nsl_strdup(src->templateUrl))             : (dest->templateUrl = NULL);
    (src->tileStoreTemplate && nsl_strlen(src->tileStoreTemplate) > 0) ? (dest->tileStoreTemplate = nsl_strdup(src->tileStoreTemplate)) : (dest->tileStoreTemplate = NULL);
    (src->mimeType && nsl_strlen(src->mimeType) > 0)                   ? (dest->mimeType = nsl_strdup(src->mimeType))                   : (dest->mimeType = NULL);

    return NE_OK;
}

/*
NBI phase 2 testing
*/
static nb_boolean TestTileServiceInitFunction(PAL_Instance** p_pal,
                                              NB_Context** p_context, NB_TileManager** p_manager,
                                              NB_TileDataSourceTemplate* p_tileDataSourceTemplate,
                                              NB_TileManagerCallback* p_callbackData,
                                              nb_boolean avaliableTyleTypesId)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileConfiguration configuration = {4, 64, 0, TRUE};
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    NB_TileManagerCallback callbackData = {0};
    TestData* testData = NULL;
    uint32 typeId = 0;

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if ((nb_boolean)palResult)
    {
        if (avaliableTyleTypesId)
        {
            configuration.maximumPendingTiles = 0;
        }

        result = InitializeConfigurationForRasterTiles(context, &tileDataSourceTemplate, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        testData = nsl_malloc(sizeof(*testData));
        CU_ASSERT_PTR_NOT_NULL(testData);

        if (testData == NULL)
        {
            CU_FAIL("Cannot allocate memory!");
            return FALSE;
        }

        nsl_memset(testData, 0, sizeof(*testData));
        testData->pal = pal;
        testData->currentTileCount = 0;
        testData->requestedTiles = 1;

        callbackData.callback = &DownloadCallback;
        callbackData.callbackData = testData;

        result = NB_TileManagerCreate(context, &callbackData, &manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (manager == NULL)
        {
            CU_FAIL("Error of initialization!");
            return FALSE;
        }

        if (avaliableTyleTypesId)
        {
           result = NB_TileManagerAddTileType(manager, &tileDataSourceTemplate, &configuration, &typeId, &callbackData);
           CU_ASSERT_EQUAL(result, NE_OK);
        }

        *p_pal = pal;
        *p_context = context;
        *p_manager = manager;
        *p_tileDataSourceTemplate = tileDataSourceTemplate;
        if (p_callbackData != NULL)
        {
            *p_callbackData = callbackData;
        }
        return TRUE;
    } else
    {
        CU_FAIL("PAL/Context creating error!");
        return FALSE;
    }
}

static void TestTileServiceTileManagerCreateNullContext(void)
{
    NB_Error result = NE_OK;
    NB_TileManager* manager = NULL;
    NB_TileManagerCallback callbackData = {0};

    callbackData.callback = &DownloadCallback;
    callbackData.callbackData = 0;

    result = NB_TileManagerCreate(NULL, &callbackData, &manager);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    if (result == NE_OK)
    {
        NB_TileManagerDestroy(manager);
    }
}

static void TestTileServiceTileManagerDestroyNullInput(void)
{
    NB_Error result = NE_OK;

    result = NB_TileManagerDestroy(NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
}

static void TestTileServiceTileManagerDoesTileTypeExistNullInput(void)
{
    NB_Error result = NE_OK;
    uint32 tileTypeId = 0;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileConfiguration configuration = {0};
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};

    uint8 palResult = CreatePalAndContext(&pal, &context);
    CU_ASSERT(palResult);

    LOGOUTPUT(LogLevelMedium, ("\n"));

    if ((nb_boolean)palResult)
    {
        result = InitializeConfigurationForRasterTiles(context, &tileDataSourceTemplate, &configuration, RTT_Map);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerDoesTileTypeExist(NULL, &tileDataSourceTemplate, &tileTypeId);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    } else
    {
        CU_FAIL("PAL/Context creating error!");
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerDoesTileTypeExistNullInputTypeId(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerDoesTileTypeExist(manager, &tileDataSourceTemplate, NULL);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerDoesTileTypeNullInput(void)
{
    NB_Error result = NE_OK;
    nb_boolean userParamStatus = FALSE;

    result = NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter(NULL, 0, &userParamStatus);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void TestTileServiceTileManagerDoesTileTypeNullResult(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerDoesTileTypeUrlTemplateHaveUserParameter(manager, 0, NULL);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileNullCommand(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    NB_TileInformation information = {0};
    NB_TileBuffer buffer = {0};
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerGetTile(manager, &information, 1, NB_TC_None, &buffer, &callbackData);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerFlushAllTilesNullInput(void)
{
    NB_Error result = NE_OK;

    result = NB_TileManagerFlushAllTiles(NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void TestTileServiceTileManagerTileTypeIsAvailableNullInput(void)
{
    NB_Error result = NE_OK;
    nb_boolean avaliableStatus = FALSE;

    result = NB_TileManagerTileTypeIsAvailable(NULL, 0, &avaliableStatus);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void TestTileServiceTileManagerTileTypeIsAvailableNullResult(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerTileTypeIsAvailable(manager, 0, NULL);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerTileTypeIsAvailable(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    nb_boolean avaliableStatus = TRUE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerTileTypeIsAvailable(manager, 0, &avaliableStatus);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerClearCacheNbTccAllOption(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerClearCache(manager, NB_TCC_All);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerClearCacheNbTccMemoryOnlyOption(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerClearCache(manager, NB_TCC_MemoryOnly);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerClearCacheNbTccUnlockedOnlyOption(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerClearCache(manager, NB_TCC_UnlockedOnly);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerClearCache1(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation info = {701, 1632, 12, 0, NULL};
    NB_TileBuffer buffer = {0};
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerGetTile(manager, &info, 10000, NB_TC_Download, &buffer, &callbackData);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerClearCache(manager, NB_TCC_All);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    CleanupTileDataSource(&tileDataSourceTemplate);
    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    CleanupTileDataSource(&tileDataSourceTemplate);
    nsl_free(callbackData.callbackData);
}

static void TestTileServiceTileManagerRemoveTileType(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    uint32 typeId = 0; // typeId = 0 was added in TestTileServiceInitFunction(...., TRUE)
    nb_boolean avaliableOfType = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerTileTypeIsAvailable(manager, typeId, &avaliableOfType);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(avaliableOfType, TRUE);

        result = NB_TileManagerRemoveTileType(manager, typeId);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerTileTypeIsAvailable(manager, typeId, &avaliableOfType);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(avaliableOfType, FALSE);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerRemoveTileType2(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    uint32 typeId = 0; // typeId = 0 was added in TestTileServiceInitFunction(...., TRUE)
    nb_boolean avaliableOfType = FALSE;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, FALSE);
    if (boolResult)
    {
        result = NB_TileManagerTileTypeIsAvailable(manager, typeId, &avaliableOfType);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(avaliableOfType, FALSE);

        result = NB_TileManagerRemoveTileType(manager, typeId);
        CU_ASSERT_EQUAL(result, NE_NOENT);

        result = NB_TileManagerTileTypeIsAvailable(manager, typeId, &avaliableOfType);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_EQUAL(avaliableOfType, FALSE);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerSetRoutesNullInput(void)
{
    NB_Error result = NE_OK;
    NB_RouteId routeId1 = {0};
    NB_RouteId routeId2 = {0};
    nb_color routeColor1 = 1;
    nb_color routeColor2 = 2;


    result = NB_TileManagerSetRoutes(NULL, routeId1, routeColor1, routeId2, routeColor2);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void TestTileServiceTileManagerSetRoutesNonTilesType(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_RouteId routeId1 = {0};
    NB_RouteId routeId2 = {0};
    nb_color routeColor1 = 1;
    nb_color routeColor2 = 2;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, FALSE);
    if (boolResult)
    {
        result = NB_TileManagerSetRoutes(manager, routeId1, routeColor1, routeId2, routeColor2);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerSetRoutes(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_RouteId routeId1 = {0};
    NB_RouteId routeId2 = {0};
    nb_color routeColor1 = 1;
    nb_color routeColor2 = 2;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
        result = NB_TileManagerSetRoutes(manager, routeId1, routeColor1, routeId2, routeColor2);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileKeyNullInput(void)
{
    NB_Error result = NE_OK;
    NB_TileInformation information = {0};
    char* key = NULL;
    int length = 1;

    result = NB_TileManagerGetTileKey(NULL, &information, &key, &length);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void TestTileServiceTileManagerGetTileKeyNullInfo(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    char* key = NULL;
    int length = 1;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
       result = NB_TileManagerGetTileKey(manager, NULL, &key, &length);
       CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileKeyNullKey(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation information = {0};
    int length = 1;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
       result = NB_TileManagerGetTileKey(manager, &information, NULL, &length);
       CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileKeyNullLength(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation information = {0};
    char* key = NULL;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
       result = NB_TileManagerGetTileKey(manager, &information, &key, NULL);
       CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileKey(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation information = {0};
    char* key = NULL;
    int length = 1;
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, TRUE);
    if (boolResult)
    {
       result = NB_TileManagerGetTileKey(manager, &information, &key, &length);
       CU_ASSERT_EQUAL(result, NE_OK);
    }

    nsl_free(key);
    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerFlushAllTilesSpecialParam(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation info = {16, 0, 0, FALSE};
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, FALSE);
    if (boolResult)
    {
       result = NB_TileManagerFlushTile(manager, &info, &callbackData);
       CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}

static void TestTileServiceTileManagerGetTileSpecialParam(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileInformation info = {0};
    NB_TileBuffer buffer = {0};
    NB_TileManagerCallback callbackData = {0};

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, FALSE);
    if (boolResult)
    {
       result = NB_TileManagerGetTile(manager, &info, 100, NB_TC_Download, &buffer, &callbackData);
       CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}
/* This test checking condition for tileType = GetTileType(manager->tileTypes, information->tileTypeId) = NULL
than NB_TileManagerGetTiles() result = NE_INVAL
*/
static void TestTileServiceTileManagerGetTilesSpecialParam1(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileRequest requestArray[TESTREQUESTARRAYCOUNT] = {{{0}}};
    uint32 zoom = 1;
    uint32 x = 0;
    uint32 y = 0;
    uint32 priority = 100;
    NB_Tile tileArray[TESTREQUESTARRAYCOUNT]= {{0}};
    uint32 i = 0;
    NB_TileManagerCallback callbackData = {0};

    for (i = 0; i < TESTREQUESTARRAYCOUNT; ++i)
    {
        if ((i % 2) == 0)
        {
            y++;
        }

        tileArray[i].information.tileTypeId = 0;
        tileArray[i].information.xPosition = x++;
        tileArray[i].information.yPosition = y;
        tileArray[i].information.zoomLevel = zoom;

        requestArray[i].information = &tileArray[i].information;
        requestArray[i].priority = priority;
        requestArray[i].command = NB_TC_Download;
        requestArray[i].buffer = &tileArray[i].buffer;
    }

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate, &callbackData, FALSE);
    if (boolResult)
    {
        result = NB_TileManagerGetTiles(manager, requestArray, TESTREQUESTARRAYCOUNT);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}
/* This test checking condition for tileType = GetTileType(manager->tileTypes, information->tileTypeId) =NOT_ NULL
and tileType->connections = NULL than NB_TileManagerGetTiles() result = NE_INVAL
*/
static void TestTileServiceTileManagerGetTilesSpecialParam2(void)
{
    NB_Error result = NE_OK;
    NB_Context* context = NULL;
    PAL_Instance* pal = NULL;
    NB_TileDataSourceTemplate tileDataSourceTemplate = {0};
    NB_TileManager* manager = NULL;
    nb_boolean boolResult = FALSE;
    NB_TileRequest requestArray[TESTREQUESTARRAYCOUNT] = {{{0}}};
    uint32 zoom = 1;
    uint32 x = 0;
    uint32 y = 0;
    uint32 priority = 100;
    NB_Tile tileArray[TESTREQUESTARRAYCOUNT]= {{0}};
    uint32 i = 0;
    NB_TileConfiguration config = {0, 0 ,0, FALSE};
    uint32 TypeId = 1;
    NB_TileManagerCallback callbackData = {0};

    for (i = 0; i < TESTREQUESTARRAYCOUNT; ++i)
    {
        if ((i % 2) == 0)
        {
            y++;
        }

        tileArray[i].information.tileTypeId = 0;
        tileArray[i].information.xPosition = x++;
        tileArray[i].information.yPosition = y;
        tileArray[i].information.zoomLevel = zoom;

        requestArray[i].information = &tileArray[i].information;
        requestArray[i].priority = priority;
        requestArray[i].command = NB_TC_Download;
        requestArray[i].buffer = &tileArray[i].buffer;
    }

    boolResult = TestTileServiceInitFunction(&pal, &context, &manager, &tileDataSourceTemplate , &callbackData, FALSE);
    if (boolResult)
    {
        result = NB_TileManagerAddTileType(manager, &tileDataSourceTemplate, &config, &TypeId, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_TileManagerGetTiles(manager, requestArray, TESTREQUESTARRAYCOUNT);
        CU_ASSERT_EQUAL(result, NE_INVAL);
    }

    NB_TileManagerDestroy(manager);
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    nsl_free(callbackData.callbackData);
    CleanupTileDataSource(&tileDataSourceTemplate);
}
/*
This test will request for many tiles with NB_TileManagerGetTiles() function and will check recieved tiles
*/
static void TestTileServiceTileManagerGetTilesCheck(void)
{
    // in progress
}
/*! @} */
