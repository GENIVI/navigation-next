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

    @file     TestVectorMap.c
    @date     03/18/2009
    @defgroup TEST_MAP_H System Tests for vector maps
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

#include "testvectormap.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "nbspatial.h"

#include "nbcontext.h"
#include "nbvectortilemanager.h"
#include "nbvectortilehandler.h"


// Local Types ..................................................................................


// Private Functions .............................................................................

// System test functions
// Handler way of getting tiles
static void TestVectorTileHandlerCreate(void);
static void TestVectorTileGetTile(void);
static void TestVectorTileGetMultipleTiles(void);
static void TestVectorTilePrecision(void);
static void TestVectorTileFromLatLong(void);
static void VectorTileCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void VerifyAndOutputVectorTileInfo(NB_VectorTile* tile);
static void GetVectorTileFromLatLong(double latitude, double longitude);


// Manager way of getting tiles
static void TestVectorTileManagerCreate(void);
static void TestVectorTileManagerGetTile(void);
static void TestVectorTileManagerMultipleDownload(void);
static void TestVectorTileManagerPrecision(void);
static void TestVectorTileManagerFromLatLong(void);
static void TestVectorTileManagerHandlerTest(void);
static void GetTileFromLatLong(double latitude, double longitude);
static void VectorTileManagerCallback(NB_VectorTileManager* vectorTileManager, NB_Error result, int percent, void* userData);

// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void 
TestVectorMap_AddTests(CU_pSuite pTestSuite, int level)
{
	// ! Test the lower level API first
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestVectorTileHandlerCreate", TestVectorTileHandlerCreate);
	ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileGetTile", TestVectorTileGetTile);
	ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileGetMultipleTiles", TestVectorTileGetMultipleTiles);
	ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTilePrecision", TestVectorTilePrecision);
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestVectorTileFromLatLong", TestVectorTileFromLatLong);

	// ! Test using the vector tile manager using the above API
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestVectorTileManagerCreate", TestVectorTileManagerCreate);
	ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileManagerGetTile", TestVectorTileManagerGetTile);
	ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileManagerMultipleDownload", TestVectorTileManagerMultipleDownload);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileManagerPrecision", TestVectorTileManagerPrecision);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestVectorTileManagerHandlerTest", TestVectorTileManagerHandlerTest);
    ADD_TEST(level, TestLevelSmoke, pTestSuite,"TestVectorTileManagerFromLatLong", TestVectorTileManagerFromLatLong);
};


/*! Add common initialization code here.

    @return 0

    @see TestVectorMap_SuiteCleanup
*/
int 
TestVectorMap_SuiteSetup()
{
    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();
	return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestVectorMap_SuiteSetup
*/
int 
TestVectorMap_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
	return 0;
}


// Private Functions .............................................................................

void 
TestVectorTileManagerCreate(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
	if (rc)
    {
        NB_VectorTileConfiguration tileConfig = {0};
		NB_VectorTileManager* vectorTileManager = 0;
		NB_Error result = NE_OK;

        // Create vector tile manager object
		tileConfig.mapPasses = 8;
		tileConfig.mapZoomLevel = 16;

        result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);
        if (result == NE_OK)
        {
            // Free vector tile manager object
            result = NB_VectorTileManagerDestroy(vectorTileManager);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


/*! Retrieve a vector tile

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileManagerGetTile(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
	if (rc)
    {
        NB_VectorTileConfiguration tileConfig = {0};
		NB_VectorTileManager* vectorTileManager = 0;
		NB_Error result = NE_OK;

        // Create vector tile manager object
		tileConfig.mapPasses = 8;
		tileConfig.mapZoomLevel = 16;

        result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_VectorTileManagerRegisterCallback(vectorTileManager, VectorTileManagerCallback, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (result == NE_OK)
        {
			NB_VectorTile* tile = 0;
			NB_VectorTileIndex tileToGet = {11342,26264};
			result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
            CU_ASSERT_EQUAL(result, NE_NOENT);
            CU_ASSERT_PTR_NULL(tile);

            // Now trigger the download
            result = NB_VectorTileManagerDownloadPendingTiles(vectorTileManager);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Wait for the callback to trigger the event
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
            {
                // Now try again. This time we should get it.
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                // Now we should have a valid tile
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile);

                if ((result == NE_OK) && tile)
                {
                    VerifyAndOutputVectorTileInfo(tile);

                    // Free the tile. The reference count will be one since the vector map instance retains a copy.
                    // It will be freed once the vector tile manager object is destroyed (below).
                    result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                    CU_ASSERT_EQUAL(result, NE_OK);
					tile = NULL;
                }

                // Get the tile one more time, the tile should be in the cache
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                // We better have a tile
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile);

                if ((result == NE_OK) && tile)
                {
                    VerifyAndOutputVectorTileInfo(tile);

                    // Unlock the tile. The vector tile manager instance retains a copy.
                    // It will be freed once the vector tile manager object is destroyed (below).
                    result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }
            }
            else
            {
                CU_FAIL("Wait for vector tile download timed out!");
            }
        }

        // Free vector tile manager object
        result = NB_VectorTileManagerDestroy(vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

	DestroyContext(context);
	PAL_DestroyInstance(pal);
}


/*! Test cueing up multiple maps/tiles for download

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileManagerMultipleDownload(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

	if (rc)
    {
        int zoom = 16;
        NB_VectorTileConfiguration tileConfig = {0};
		NB_VectorTileManager* vectorTileManager = 0;
		NB_Error result = NE_OK;

        // Initialize the config
        tileConfig.mapPasses    = 8;
        tileConfig.mapZoomLevel = zoom;

        // Create vector tile manager object
        result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_VectorTileManagerRegisterCallback(vectorTileManager, VectorTileManagerCallback, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (result == NE_OK)
        {
            // This is the number we request.
            const int MAX_CONCURRENT_REQUEST = 20;

            // Random values (in the U.S.)
            const int OFFSET = 37;
            const int X_START_VALUE = 11342;
            const int Y_START_VALUE = 26264;
			NB_VectorTileIndex tileToGet = {0};

            int concurrentIndex = 0;
            tileToGet.x = X_START_VALUE;
            tileToGet.y = Y_START_VALUE;

            LOGOUTPUT(LogLevelHigh, ("\n"));

            // Request a couple of tiles at the same time. Offset the x/y-coordinates each time.
            for (concurrentIndex = 0; concurrentIndex < MAX_CONCURRENT_REQUEST; ++concurrentIndex)
            {
				NB_VectorTile* tile = 0;

                tileToGet.x += OFFSET;
                tileToGet.y += OFFSET;

                // Request the tile. The first few should return NE_NOENT. If we are passed the maximum concurrent
                // number then we expect to get NE_BUSY
				result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                // We expect no entry
                CU_ASSERT_EQUAL(result, NE_NOENT);
                CU_ASSERT_PTR_NULL(tile);

                LOGOUTPUT(LogLevelHigh, ("Request tile[%d]: Result: %d, x: %d, y: %d, zoom: %d\n", concurrentIndex, result, tileToGet.x, tileToGet.y, zoom));
            }

            // Now trigger the download
            result = NB_VectorTileManagerDownloadPendingTiles(vectorTileManager);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Wait for the callback to trigger the event
            if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
            {
                CU_FAIL("Wait for vector tile download timed out!");
            }

            // Reset the values
            tileToGet.x = X_START_VALUE;
            tileToGet.y = Y_START_VALUE;

            // Now go through the same tiles again
            for (concurrentIndex = 0; concurrentIndex < MAX_CONCURRENT_REQUEST; ++concurrentIndex)
            {
                NB_VectorTile* tile = 0;

                tileToGet.x += OFFSET;
                tileToGet.y += OFFSET;

                // Now try again. This time we should get it
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                LOGOUTPUT(LogLevelHigh, ("Second request tile[%d]: Result: %d, x: %d, y: %d, zoom: %d, tile: %p\n", concurrentIndex, result, tileToGet.x, tileToGet.y, zoom, tile));

                // We expect success 
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile);

                if ((result == NE_OK) && tile)
                {
					VerifyAndOutputVectorTileInfo(tile);

                    // Unlock the tile. The vector tile manager instance retains a copy.
                    // It will be freed once the vector tile manager object is destroyed (below).
                    result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                    CU_ASSERT_EQUAL(result, NE_OK);
					tile = NULL;                   
                }
            }

			// Free vector tile manager object
			result = NB_VectorTileManagerDestroy(vectorTileManager);
			CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

	DestroyContext(context);
	PAL_DestroyInstance(pal);
}


/*! Test the precision of vector maps.

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileManagerPrecision(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

	if (rc)
    {
        const int zoom = 16;
        int precision = 0;

        for (precision = 1; precision <= 16; precision++)
        {
			NB_VectorTileConfiguration tileConfig = {0};
			NB_VectorTileManager* vectorTileManager = 0;
			NB_Error result = NE_OK;

            // Initialize the config
            tileConfig.mapPasses    = precision;
            tileConfig.mapZoomLevel = zoom;

            LOGOUTPUT(LogLevelHigh, ("\nPrecision: %d\n\n", precision));

			// Create vector tile manager object
			result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
			CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
                result = NB_VectorTileManagerRegisterCallback(vectorTileManager, VectorTileManagerCallback, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            if (result == NE_OK)
            {
                NB_VectorTile* tile = NULL;
				NB_VectorTileIndex tileToGet = {0};

                tileToGet.x = 11342;
                tileToGet.y = 26264;

                // Get a vector tile. The first call should return NE_NOENT since the tile is not yet downloaded.
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
                CU_ASSERT_PTR_NULL(tile);

                // Now trigger the download
                result = NB_VectorTileManagerDownloadPendingTiles(vectorTileManager);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    // Now try again. This time we should get it.
                    result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
                }
                else
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }

                // Now we should have a valid tile
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile);

                if ((result == NE_OK) && tile)
                {
                    VerifyAndOutputVectorTileInfo(tile);

                    // Unlock the tile. The vector tile manager instance retains a copy.
                    // It will be freed once the vector tile manager object is destroyed (below).
                    result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                    CU_ASSERT_EQUAL(result, NE_OK);
					tile = NULL;                   
                }

				// Free vector tile manager object
				result = NB_VectorTileManagerDestroy(vectorTileManager);
				CU_ASSERT_EQUAL(result, NE_OK);
            }
        }
    }

	DestroyContext(context);
	PAL_DestroyInstance(pal);

}


/*! Test vector tiles based on actual latitude/longitude values.

    This system test uses the spatial interface to do the conversions from latitude/longitde to tiles

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileManagerFromLatLong(void)
{
    // To verify the results enable the verbose flags.

    // New York. The vector tile should contain:
    // "Greenpoint Avenue", 38th to 45th street in Manhattan.
    GetTileFromLatLong(40.740494,-73.921745);
    
    // Seattle, WA. The vector tile should contain:
    // "E Madison Street", E Union Street", 12th - 15th street
    GetTileFromLatLong(47.612912,-122.315201);
}


/*! Test creation 

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileHandlerCreate(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        NB_VectorTileHandler* handler = NULL;
        NB_RequestHandlerCallback callback = {VectorTileCallback, 0};
               
        err = NB_VectorTileHandlerCreate(context, &callback, &handler);
        CU_ASSERT_EQUAL(err, NE_OK);
        
        NB_VectorTileHandlerDestroy(handler);
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
    
}


/*! Retrieve a vector tile

    @return None. CUnit asserts will be called on failures.
*/
void TestVectorTileGetTile(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
		NB_VectorTileParametersConfiguration config = {0};
		NB_VectorTileIndex tileToGet = {0};
		NB_VectorTileParameters* parameters = 0;
		NB_VectorTileInformation* info = NULL;

		// Specify parameter configuration
		NB_VectorTileParametersConfigurationSetDefaults(&config);
		config.passes = 8;
		config.zoomLevel = 16;

		// Create the request parameters
		tileToGet.x = 11342;
		tileToGet.y = 26264;
		err = NB_VectorTileParametersCreate(context, &config, &tileToGet, &parameters);
		CU_ASSERT_EQUAL(err, NE_OK);
		LOGOUTPUT(LogLevelHigh, ("Request tile[%d]: Result: %d, x: %d, y: %d, zoom: %d\n", 0, err, tileToGet.x, tileToGet.y, config.zoomLevel));

		if (err == NE_OK)
		{
			NB_VectorTileHandler* handler = NULL;
			NB_RequestHandlerCallback callback = {0};
	        
			callback.callback = VectorTileCallback;
			callback.callbackData = &info;
	        
			// Create the vector tile handler
			err = NB_VectorTileHandlerCreate(context, &callback, &handler);
			CU_ASSERT_EQUAL(err, NE_OK);
			if (err == NE_OK)
			{
				// Start the request
				err = NB_VectorTileHandlerStartRequest(handler, parameters);
				CU_ASSERT_EQUAL(err, NE_OK);

                // Wait for the callback to trigger the event
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    // Get information on the tile
					uint32 count = 0;
					if (info != NULL)
					{
						NB_VectorTile* tile = 0;
						err = NB_VectorTileInformationGetTileCount(info, &count);
						CU_ASSERT_EQUAL(err, NE_OK);

						err = NB_VectorTileInformationGetTile(info, 0, &tile);
						CU_ASSERT_EQUAL(err, NE_OK);
						VerifyAndOutputVectorTileInfo(tile);

						err = NB_VectorTileInformationDestroyTile(tile);
						CU_ASSERT_EQUAL(err, NE_OK);

						err = NB_VectorTileInformationDestroy(info);
						CU_ASSERT_EQUAL(err, NE_OK);
						info = NULL;
					}
                }
                else
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }

				NB_VectorTileHandlerDestroy(handler);
			}

			NB_VectorTileParametersDestroy(parameters);
		}
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);

}



/*! Test cueing up multiple tiles for download

    @return None. CUnit asserts will be called on failures.
*/
void TestVectorTileGetMultipleTiles(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
		NB_VectorTileParametersConfiguration config = {0};
		NB_VectorTileIndex tileToGet = {0};
		NB_VectorTileParameters* parameters = 0;
		NB_VectorTileInformation* info = NULL;
		int i = 0;

        // This is the number we request.
        const int NUMBER_OF_TILES_TO_REQUEST = 20;

        // Random values (in the U.S.)
        const int OFFSET = 37;
        const int X_START_VALUE = 11342;
        const int Y_START_VALUE = 26264;

        tileToGet.x = X_START_VALUE;
        tileToGet.y = Y_START_VALUE;

		// Specify parameter configuration
		NB_VectorTileParametersConfigurationSetDefaults(&config);
		config.passes = 8;
		config.zoomLevel = 16;

		// Create the request parameters
		tileToGet.x = X_START_VALUE;
		tileToGet.y = Y_START_VALUE;
		err = NB_VectorTileParametersCreate(context, &config, &tileToGet, &parameters);
		CU_ASSERT_EQUAL(err, NE_OK);

		// Now, add the rest of the tiles
		for (i = 1; i < NUMBER_OF_TILES_TO_REQUEST; i++)
		{
			tileToGet.x += OFFSET;
			tileToGet.y += OFFSET;
			err = NB_VectorTileParametersAddTile(parameters, &tileToGet);
			CU_ASSERT_EQUAL(err, NE_OK);
			LOGOUTPUT(LogLevelHigh, ("Request tile[%d]: Result: %d, x: %d, y: %d, zoom: %d\n", 0, err, tileToGet.x, tileToGet.y, config.zoomLevel));
		}

		if (err == NE_OK)
		{
			NB_VectorTileHandler* handler = NULL;
			NB_RequestHandlerCallback callback = {0};
	        
			callback.callback = VectorTileCallback;
			callback.callbackData = &info;
	        
			// Create the vector tile handler
			err = NB_VectorTileHandlerCreate(context, &callback, &handler);
			CU_ASSERT_EQUAL(err, NE_OK);
			if (err == NE_OK)
			{
				// Start the request
				err = NB_VectorTileHandlerStartRequest(handler, parameters);
				CU_ASSERT_EQUAL(err, NE_OK);

                // Wait for the callback to trigger the event
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    // Get information on the tile
					uint32 count = 0;
					if (info != NULL)
					{
						uint32 i = 0;
						NB_VectorTile* tile = 0;
						err = NB_VectorTileInformationGetTileCount(info, &count);
						CU_ASSERT_EQUAL(err, NE_OK);

						for (i = 0; i < count; i++, NB_VectorTileInformationGetTileCount(info, &count))
						{
							err = NB_VectorTileInformationGetTile(info, i, &tile);
							CU_ASSERT_EQUAL(err, NE_OK);
							VerifyAndOutputVectorTileInfo(tile);

							err = NB_VectorTileInformationDestroyTile(tile);
							CU_ASSERT_EQUAL(err, NE_OK);
						}

						err = NB_VectorTileInformationDestroy(info);
						CU_ASSERT_EQUAL(err, NE_OK);
						info = NULL;
					}
                }
                else
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }

				NB_VectorTileHandlerDestroy(handler);
			}
			NB_VectorTileParametersDestroy(parameters);
		}
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);

}


/*! Test the precision of vector tiles.

    @return None. CUnit asserts will be called on failures.
*/
void TestVectorTilePrecision()
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
		NB_VectorTileParametersConfiguration config = {0};
		NB_VectorTileIndex tileToGet = {0};
		NB_VectorTileParameters* parameters = 0;
		NB_VectorTileInformation* info = NULL;
		int precision = 0;

		// Specify parameter configuration
		NB_VectorTileParametersConfigurationSetDefaults(&config);
		config.zoomLevel = 16;

		for (precision = 1; precision < 16; precision++)
		{
			LOGOUTPUT(LogLevelHigh, ("\nPrecision: %d\n\n", precision));
			config.passes = precision;

			// Create the request parameters
			tileToGet.x = 11342;
			tileToGet.y = 26264;
			err = NB_VectorTileParametersCreate(context, &config, &tileToGet, &parameters);
			CU_ASSERT_EQUAL(err, NE_OK);

			if (err == NE_OK)
			{
				NB_VectorTileHandler* handler = NULL;
				NB_RequestHandlerCallback callback = {0};
		        
				callback.callback = VectorTileCallback;
				callback.callbackData = &info;
		        
				// Create the vector tile handler
				err = NB_VectorTileHandlerCreate(context, &callback, &handler);
				CU_ASSERT_EQUAL(err, NE_OK);
				if (err == NE_OK)
				{
					// Start the request
					err = NB_VectorTileHandlerStartRequest(handler, parameters);
					CU_ASSERT_EQUAL(err, NE_OK);

					// Wait for the callback to trigger the event
					if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
					{
						// Get information on the tile
						uint32 count = 0;
						if (info != NULL)
						{
							NB_VectorTile* tile = 0;
							err = NB_VectorTileInformationGetTileCount(info, &count);
							CU_ASSERT_EQUAL(err, NE_OK);

							err = NB_VectorTileInformationGetTile(info, 0, &tile);
							CU_ASSERT_EQUAL(err, NE_OK);
							VerifyAndOutputVectorTileInfo(tile);

							err = NB_VectorTileInformationDestroyTile(tile);
							CU_ASSERT_EQUAL(err, NE_OK);

							err = NB_VectorTileInformationDestroy(info);
							CU_ASSERT_EQUAL(err, NE_OK);
							info = NULL;
						}
					}
					else
					{
						CU_FAIL("Wait for vector tile download timed out!");
					}

					NB_VectorTileHandlerDestroy(handler);
				}
				NB_VectorTileParametersDestroy(parameters);
			}
		}
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}


/*! Test vector tiles based on actual latitude/longitude values.

    This system test uses the spatial interface to do the conversions from latitude/longitde to tiles

    @return None. CUnit asserts will be called on failures.
*/
void TestVectorTileFromLatLong()
{
    // To verify the results enable the verbose flags.

    // New York. The vector tile should contain:
    // "Greenpoint Avenue", 38th to 45th street in Manhattan.
    GetVectorTileFromLatLong(40.740494,-73.921745);
    
    // Seattle, WA. The vector tile should contain:
    // "E Madison Street", E Union Street", 12th - 15th street
    GetVectorTileFromLatLong(47.612912,-122.315201);
}

/*! Verify and Output all the vector tile information.

    In verbose mode this prints out all the vector tile information. If verbose mode is 
    off then it validates/verifies the information.

    @return None, CUnit asserts are called on failure.
*/
void
VerifyAndOutputVectorTileInfo(NB_VectorTile* tile)
{
    int i = 0;
    int k = 0;
    int l = 0;

    // We assign all values to dummy values. This ensures that all counters for the arrays are
    // correct and no crash occurs.
    double dummyDouble = 0;
    int dummyInt = 0;
    char* dummyChar = NULL;

    LOGOUTPUT(LogLevelHigh, ("\n\nVector Tile:\n\nAreas: %p, %d, Lines: %p, %d, Points: %p, %d, Roads: %p, %d\n",
           tile->areas,
           tile->areaCount,
           tile->lines,
           tile->lineCount,
           tile->points,
           tile->pointCount,
           tile->roads,
           tile->roadCount));

    // For all areas
    for (i = 0; i < tile->areaCount; i++)
    {
        // Assign to dummy values to ensure that the memory is not corrupt
        dummyChar   = tile->areas[i].label;
        dummyInt    = (int)(tile->areas[i].areaType);
        dummyInt    = (int)(tile->areas[i].priority);

        LOGOUTPUT(LogLevelHigh, ("Area[%d/%d]: %s, Type: %d, Priority: %d\n", 
               i, 
               tile->areaCount, 
               tile->areas[i].label, 
               tile->areas[i].areaType, 
               tile->areas[i].priority));

        // For all polygons
        for (k = 0; k < tile->areas[i].polygonCount; k++)
        {
            LOGOUTPUT(LogLevelHigh, ("   Polygon[%d]\n", k));
            // For all points in the polygon
            for (l = 0; l < tile->areas[i].polygons[k].count; l++)
            {
                // Assign to dummy values to ensure that the memory is not corrupt
                dummyDouble = tile->areas[i].polygons[k].points[l].mx;
                dummyDouble = tile->areas[i].polygons[k].points[l].my;

                LOGOUTPUT(LogLevelHigh, ("      [%d]: %f, %f\n", 
                       l,
                       tile->areas[i].polygons[k].points[l].mx,
                       tile->areas[i].polygons[k].points[l].my));
            }
        }
    }

    // For all lines
    for (i = 0; i < tile->lineCount; i++)
    {
        // Assign to dummy values to ensure that the memory is not corrupt
        dummyChar   = tile->lines[i].label;
        dummyInt    = (int)(tile->lines[i].priority);

        LOGOUTPUT(LogLevelHigh, ("Line[%d], %s, Priority: %d\n", i, tile->lines[i].label, tile->lines[i].priority));

        // For all polylines
        for (k = 0; k < tile->lines[i].polylineCount; k++)
        {
            LOGOUTPUT(LogLevelHigh, ("   Polyline[%d]\n", k));
            // For all points in the polyline
            for (l = 0; l < tile->lines[i].polylines[k].count; l++)
            {
                // Assign to dummy values to ensure that the memory is not corrupt
                dummyDouble = tile->lines[i].polylines[k].points[l].mx;
                dummyDouble = tile->lines[i].polylines[k].points[l].my;

                LOGOUTPUT(LogLevelHigh, ("      [%d]: %f, %f\n", 
                       l,
                       tile->lines[i].polylines[k].points[l].mx,
                       tile->lines[i].polylines[k].points[l].my));
            }
        }
    }

    // For all Points
    for (i = 0; i < tile->pointCount; i++)
    {
        // Assign to dummy values to ensure that the memory is not corrupt
        dummyDouble = tile->points[i].point.mx;
        dummyDouble = tile->points[i].point.my;
        dummyChar   = tile->points[i].label;
        dummyInt    = tile->points[i].priority;

        LOGOUTPUT(LogLevelHigh, ("Point[%d]: %f, %f, Label: %s, Priority: %d\n", 
               i, 
               tile->points[i].point.mx,
               tile->points[i].point.my,
               tile->points[i].label, 
               tile->points[i].priority));
    }

    // For all Roads
    for (i = 0; i < tile->roadCount; i++)
    {
        // Assign to dummy values to ensure that the memory is not corrupt
        dummyChar   = tile->roads[i].label;
        dummyInt    = tile->roads[i].lanes;
        dummyInt    = tile->roads[i].priority;
        dummyInt    = tile->roads[i].roadType;

        LOGOUTPUT(LogLevelHigh, ("Road[%d/%d]: %s, Lanes: %d, Priority: %d, Type: %d\n",
               i,
               tile->roadCount,
               tile->roads[i].label,
               tile->roads[i].lanes,
               tile->roads[i].priority,
               tile->roads[i].roadType));

        // For all points in the foreground tri-strip
        for (k = 0; k < tile->roads[i].foregroundTriStrip->count; k++)
        {
            // Assign to dummy values to ensure that the memory is not corrupt
            dummyDouble = tile->roads[i].foregroundTriStrip->points[k].mx;
            dummyDouble = tile->roads[i].foregroundTriStrip->points[k].my;

            if (k == 0)
            {
                LOGOUTPUT(LogLevelHigh, ("   Foreground Tri-Strip points: \n"));
            }

            LOGOUTPUT(LogLevelHigh, ("       [%d]: %f, %f\n", 
                   k,
                   tile->roads[i].foregroundTriStrip->points[k].mx,
                   tile->roads[i].foregroundTriStrip->points[k].my));
        }

        // For all points in the background tri-strip
        for (k = 0; k < tile->roads[i].backgroundTriStrip->count; k++)
        {
            // Assign to dummy values to ensure that the memory is not corrupt
            dummyDouble = tile->roads[i].backgroundTriStrip->points[k].mx;
            dummyDouble = tile->roads[i].backgroundTriStrip->points[k].my;

            if (k == 0)
            {
                LOGOUTPUT(LogLevelHigh, ("   Background Tri-Strip points: \n"));
            }

            LOGOUTPUT(LogLevelHigh, ("       [%d]: %f, %f\n", 
                   k,
                   tile->roads[i].backgroundTriStrip->points[k].mx,
                   tile->roads[i].backgroundTriStrip->points[k].my));
        }

        // For all polylines
        for (k = 0; k < tile->roads[i].polylineCount; k++)
        {
            LOGOUTPUT(LogLevelHigh, ("   Polyline[%d]\n", k));

            // For all points in the polyline
            for (l = 0; l < tile->roads[i].polylines[k].count; l++)
            {
                // Assign to dummy values to ensure that the memory is not corrupt
                dummyDouble = tile->roads[i].polylines[k].points[l].mx;
                dummyDouble = tile->roads[i].polylines[k].points[l].my;

                LOGOUTPUT(LogLevelHigh, ("      [%d]: %f, %f\n", 
                       l,
                       tile->roads[i].polylines[k].points[l].mx,
                       tile->roads[i].polylines[k].points[l].my));
            }
        }
    }
}


/*! Get a vector tile based on latitude/longitude values.

    Subfunction of the system test function(s).

    @return None, CUnit asserts are called on failure.

    @see TestVectorTileManagerFromLatLong
*/
void 
GetVectorTileFromLatLong(double latitude, double longitude)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
		NB_VectorTileParametersConfiguration config = {0};
		NB_VectorTileIndex tileToGet = {0};
		NB_VectorTileParameters* parameters = 0;
		NB_VectorTileInformation* info = NULL;
		double xMercator = 0.0;
		double yMercator = 0.0;

		// Specify parameter configuration
		NB_VectorTileParametersConfigurationSetDefaults(&config);
		config.passes = 8;
		config.zoomLevel = 16;

        // Convert latitude/longitude first to mercator projection and then get the tile indices.
        NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
        NB_SpatialConvertMercatorToTile(xMercator, yMercator, config.zoomLevel, &tileToGet.x, &tileToGet.y);

		// Create the request parameters
		err = NB_VectorTileParametersCreate(context, &config, &tileToGet, &parameters);
		CU_ASSERT_EQUAL(err, NE_OK);
		LOGOUTPUT(LogLevelHigh, ("Request tile[%d]: Result: %d, x: %d, y: %d, zoom: %d\n", 0, err, tileToGet.x, tileToGet.y, config.zoomLevel));

		if (err == NE_OK)
		{
			NB_VectorTileHandler* handler = NULL;
			NB_RequestHandlerCallback callback = {0};
	        
			callback.callback = VectorTileCallback;
			callback.callbackData = &info;
	        
			// Create the vector tile handler
			err = NB_VectorTileHandlerCreate(context, &callback, &handler);
			CU_ASSERT_EQUAL(err, NE_OK);
			if (err == NE_OK)
			{
				// Start the request
				err = NB_VectorTileHandlerStartRequest(handler, parameters);
				CU_ASSERT_EQUAL(err, NE_OK);

                // Wait for the callback to trigger the event
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    // Get information on the tile
					uint32 count = 0;
					if (info != NULL)
					{
						NB_VectorTile* tile = 0;
						err = NB_VectorTileInformationGetTileCount(info, &count);
						CU_ASSERT_EQUAL(err, NE_OK);

						err = NB_VectorTileInformationGetTile(info, 0, &tile);
						CU_ASSERT_EQUAL(err, NE_OK);
						VerifyAndOutputVectorTileInfo(tile);

						err = NB_VectorTileInformationDestroyTile(tile);
						CU_ASSERT_EQUAL(err, NE_OK);

						err = NB_VectorTileInformationDestroy(info);
						CU_ASSERT_EQUAL(err, NE_OK);
						info = NULL;
					}
                }
                else
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }

				NB_VectorTileHandlerDestroy(handler);
			}
			NB_VectorTileParametersDestroy(parameters);
		}
    }
    
    DestroyContext(context);
    PAL_DestroyInstance(pal);
}



void
VectorTileCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    LOGOUTPUT(LogLevelHigh, ("New VectorTile %s progress: %d\n", up ? "upload" : "download", percent));

    if (!up)
    {
        if (err)
        {
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
        else if (percent == 100)
        {
			NB_Error err = NE_OK;
			NB_VectorTileInformation** info = (NB_VectorTileInformation**)userData;
			*info = NULL;

			err = NB_VectorTileHandlerGetTileInformation(handler, info);
			CU_ASSERT_EQUAL(err, NE_OK);
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
}

void VectorTileManagerCallback(NB_VectorTileManager* vectorTileManager, NB_Error result, int percent, void* userData)
{
    CU_ASSERT_PTR_NOT_NULL(vectorTileManager);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NULL(userData);       // We set the user data to NULL

    LOGOUTPUT(LogLevelHigh, ("Tile Manager Callback: Result: %d, Percent: %d\n", result, percent));

    if (result == NE_OK)
    {
        // When the download is finished then we trigger the test function to continue
        if (percent == 100)
        {
            SetCallbackCompletedEvent(g_CallbackEvent);
        }
    }
    else
    {
        // Trigger the event on failure
        SetCallbackCompletedEvent(g_CallbackEvent);
    }
}




/*! Get a vector tile based on latitude/longitude values.

    Subfunction of the system test function(s).

    @return None, CUnit asserts are called on failure.

    @see TestVectorTileManagerFromLatLong
*/
void
GetTileFromLatLong(double latitude, double longitude)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);

	if (rc)
    {
        // Test zoom level 14 - 17. They should all return the same streets but the lower zoom levels should retrieve a 
        // much larger area.
        int zoom = 0;
        for (zoom = 14; zoom <= 17; zoom++)
        {
			NB_VectorTileConfiguration tileConfig = {0};
			NB_VectorTileManager* vectorTileManager = 0;
			NB_Error result = NE_OK;

            // Initialize the config
            tileConfig.mapPasses    = 8;
            tileConfig.mapZoomLevel = zoom;

            // Create vector tile manager object
			result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
			CU_ASSERT_EQUAL(result, NE_OK);

            if (result == NE_OK)
            {
				NB_VectorTile* tile = 0;
				NB_VectorTileIndex tileToGet = {0};

                double xMercator = 0.0;
                double yMercator = 0.0;

                result = NB_VectorTileManagerRegisterCallback(vectorTileManager, VectorTileManagerCallback, NULL);
    			CU_ASSERT_EQUAL(result, NE_OK);

                // Convert latitude/longitude first to mercator projection and then get the tile indices.
                NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
                NB_SpatialConvertMercatorToTile(xMercator, yMercator, zoom, &tileToGet.x, &tileToGet.y);

                // Queue up map. Should return NE_NOENT
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
				CU_ASSERT_PTR_NULL(tile);

                // Trigger the download
				result = NB_VectorTileManagerDownloadPendingTiles(vectorTileManager);
				CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event
                if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }

                // Get the downloaded map
                result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(tile);

                if ((result == NE_OK) && tile)
                {
                    VerifyAndOutputVectorTileInfo(tile);

                    result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

				// Free vector tile manager object
				result = NB_VectorTileManagerDestroy(vectorTileManager);
				CU_ASSERT_EQUAL(result, NE_OK);
            }
        }
    }

	DestroyContext(context);
	PAL_DestroyInstance(pal);
}


/*! Test to make sure that handlers are being freed after use

    @return None. CUnit asserts will be called on failures.
*/
void 
TestVectorTileManagerHandlerTest(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    NB_VectorTileIndex tileToGet = {11342,26264};
    const int OFFSET = 37;
    const int NUM_HANDLERS = 4;
    int i = 0;

	if (rc)
    {
        NB_VectorTileConfiguration tileConfig = {0};
		NB_VectorTileManager* vectorTileManager = 0;
		NB_Error result = NE_OK;

        // Create vector tile manager object
		tileConfig.mapPasses = 8;
		tileConfig.mapZoomLevel = 16;

        result = NB_VectorTileManagerCreate(context, &tileConfig, NULL, &vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (result == NE_OK)
        {
            result = NB_VectorTileManagerRegisterCallback(vectorTileManager, VectorTileManagerCallback, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (result == NE_OK)
        {
			NB_VectorTile* tile = 0;

            // This will test to make sure that the handlers are being freed after use.
            for (i = 0; i < NUM_HANDLERS * 5; i++)
            {	
                tileToGet.x += OFFSET;
                tileToGet.y += OFFSET;

			    result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
                CU_ASSERT_PTR_NULL(tile);

                // Now trigger the download
                result = NB_VectorTileManagerDownloadPendingTiles(vectorTileManager);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Wait for the callback to trigger the event
                if (WaitForCallbackCompletedEvent(g_CallbackEvent, 20 * 1000))
                {
                    // Now try again. This time we should get it.
                    result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                    // Now we should have a valid tile
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(tile);

                    if ((result == NE_OK) && tile)
                    {
                        VerifyAndOutputVectorTileInfo(tile);

                        // Free the tile. The reference count will be one since the vector map instance retains a copy.
                        // It will be freed once the vector tile manager object is destroyed (below).
                        result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                        CU_ASSERT_EQUAL(result, NE_OK);
					    tile = NULL;
                    }

                    // Get the tile one more time, the tile should be in the cache
                    result = NB_VectorTileManagerGetTile(vectorTileManager, &tileToGet, &tile, TRUE);

                    // We better have a tile
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(tile);

                    if ((result == NE_OK) && tile)
                    {
                        VerifyAndOutputVectorTileInfo(tile);

                        // Unlock the tile. The vector tile manager instance retains a copy.
                        // It will be freed once the vector tile manager object is destroyed (below).
                        result = NB_VectorTileManagerUnlockTile(vectorTileManager, tile);
                        CU_ASSERT_EQUAL(result, NE_OK);
                    }
                }
                else
                {
                    CU_FAIL("Wait for vector tile download timed out!");
                }
               
            } // for (..
        }

        // Free vector tile manager object
        result = NB_VectorTileManagerDestroy(vectorTileManager);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

	DestroyContext(context);
	PAL_DestroyInstance(pal);
}

/*! @} */
