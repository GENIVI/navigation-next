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

    @file       testcachingtilemanager.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
    #include "platformutil.h"
    #include "networkutil.h"
}

#include "testcachingtilemanager.h"
#include "CachingTileManager.h"
#include "LeastRecentlyUsedEvictionStrategy.h"
#include "OffboardTileManager.h"
#include "OffboardConvertTileManager.h"
#include "OffboardTileType.h"
#include "base.h"

using namespace std;
using namespace nbmap;
using namespace nbcommon;

#define MAX_FULL_PATH_LENGTH 256

// Local Constants ..............................................................................

/* These two priorities are different. First is used to calculate download level in generic
   download manager. Second is priority of tile request in the download level (queue).
*/

static const uint32 TEST_LAYER_ID_DIGITAL = 0;
static const uint32 TEST_MIN_ZOOM         = 0;
static const uint32 TEST_MAX_ZOOM         = 24;

static const uint32 TEST_DEFAULT_MAX_REQUEST_COUNT = 16;
static const uint32 TEST_DOWNLOAD_PRIORITY         = 4;
static const uint32 TEST_MAX_CACHE_DISK_SPACE      = 1024; // 1M
static const uint32 TEST_TILE_DRAWORDER            = 15;
static const uint32 TEST_TILE_KEY_COUNT            = 3;
static const uint32 TEST_TILE_REQUEST_PRIORITY     = 0;
static const uint32 TEST_TILE_REQUEST_RETRY_TIMES  = 3;

static const char TEST_CACHING_NAME[] = "TEST_TILE_CACHE";

static const char TEST_CACHING_TYPE[] = "LAM";
static const char TEST_DATASET_ID1[] = "1355452862";
static const char TEST_DATASET_ID2[] = "0000000000";

static const char TEST_TILE_DATA_TYPE[] = "LAM";
static const char TEST_ENTIRE_URL_TEMPLATE[] = "http://dev14dts.navbuilder.nimlbs.net:8080/unified_map?type=LAM&gen=1755811585&fmt=$fmt&x=$x&y=$y&vfmt=$vfmt";
static const char TEST_CONTENT_ID_TEPLATE[] = "T_LAM_1_$fmt_$vfmt_$x_$y";

static const char TEST_METADATA_PARAMETER_LOCALE[] = "$loc";
static const char TEST_METADATA_PARAMETER_RESOLUTION[] = "$res";
static const char TEST_METADATA_PARAMETER_SIZE[] = "$sz";
static const char TEST_METADATA_PARAMETER_VERSION[] = "$v";

static const char TEST_METADATA_DEFAULT_LOCALE[] = "en-US";
static const char TEST_METADATA_DEFAULT_RESOLUTION[] = "192";
static const char TEST_METADATA_DEFAULT_TILE_SIZE[] = "256";
static const char TEST_METADATA_DEFAULT_VERSION[] = "1.0";

static const char TEST_KEY_FORMAT[] = "$fmt";
static const char TEST_VALUE_FORMAT[] = "PNG";

static const char TEST_KEY_FORMAT_VERSION[] = "$vfmt";
static const char TEST_VALUE_FORMAT_VERSION[] = "30";

static const char TEST_KEY_X[] = "$x";
static const char TEST_VALUE_X[TEST_TILE_KEY_COUNT][10] = {"35", "36", "37"};

static const char TEST_KEY_Y[] = "$y";
static const char TEST_VALUE_Y[TEST_TILE_KEY_COUNT][10] = {"54", "53", "52"};


// Local Types ..................................................................................

/*! Collection of all instances used for the unit tests */
class TestCachingTileInstances : public Base
{
public:
    /* See definition for description */

    TestCachingTileInstances();
    virtual ~TestCachingTileInstances();
    NB_Error Initialize(bool convert = FALSE);
    void Reset();
    NB_Error CheckCache(int indexes[], int count, bool shouldContain);

    NB_Error m_requestError;            /*!< Error during requesting tiles */
    int m_requestCount;                 /*!< Count of requests which are downloading in progress */
    PAL_Instance* m_pal;                /*!< Pointer to PAL instance */
    NB_Context* m_context;              /*!< Pointer to current context */
    void* m_event;                      /*!< Callback completed event */
    CachingTileManagerPtr m_tileManager;/*!< A CachingTileManager object */
    CachePtr m_cache;                   /*!< A Cache object */
    shared_ptr<string> m_contentId;     /*!< Content ID of the tile */
};

/*! Test callback object for getting data of tile */
class TestCachingCallback : public Base,
                            public AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr>
{
public:
    /* See definition for description */

    TestCachingCallback(TestCachingTileInstances& testInstances);
    virtual ~TestCachingCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<map<string, string> > request,
                         TilePtr response);
	virtual void Error(shared_ptr<map<string, string> > request,
                       NB_Error error);
	virtual bool Progress(int percentage);

    // Copy constructor and assignment operator are not supported.
    TestCachingCallback(const TestCachingCallback& callback);
    TestCachingCallback& operator=(const TestCachingCallback& callback);

    TestCachingTileInstances& m_testInstances;  /*!< A TestCachingTileInstances object */
};

typedef shared_ptr<TestCachingCallback> TestCachingCallbackPtr;


// Test Functions ...............................................................................

static void TestCachingTileManagerInitialization();
static void TestCachingTileManagerGetTileWithoutCallback();
static void TestCachingTileManagerGetTileWithCallback();
static void TestCachingTileManagerChangeDatasetId();
static void TestCachingTileManagerRemoveTile();
static void TestCachingTileManagerRemoveAllTiles();

static void TestCachingConvertTileManagerInitialization();
static void TestCachingConvertTileManagerGetTileWithoutCallback();
static void TestCachingConvertTileManagerGetTileWithCallback();
static void TestCachingConvertTileManagerChangeDatasetId();
static void TestCachingConvertTileManagerRemoveAllTiles();

static void TestCachingTileManagerGetTileWithoutCallbackCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerGetTileWithCallbackCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerChangeDatasetIdCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerRemoveAllTilesCommon(TestCachingTileInstances& testInstances);


static void TestCachingTileManagerGetTileWithoutCallbackCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerGetTileWithCallbackCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerChangeDatasetIdCommon(TestCachingTileInstances& testInstances);
static void TestCachingTileManagerRemoveAllTilesCommon(TestCachingTileInstances& testInstances);


// Helper functions .............................................................................

shared_ptr<map<string, string> > CreateCachingTileRequestParameters(const char valueX[], const char valueY[]);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestCachingTileManager_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestCachingTileManagerInitialization", &TestCachingTileManagerInitialization);
    CU_add_test(pTestSuite, "TestCachingTileManagerGetTileWithoutCallback", &TestCachingTileManagerGetTileWithoutCallback);
    CU_add_test(pTestSuite, "TestCachingTileManagerGetTileWithCallback", &TestCachingTileManagerGetTileWithCallback);
    CU_add_test(pTestSuite, "TestCachingTileManagerChangeDatasetId", &TestCachingTileManagerChangeDatasetId);
    CU_add_test(pTestSuite, "TestCachingTileManagerRemoveTile", &TestCachingTileManagerRemoveTile);
    CU_add_test(pTestSuite, "TestCachingTileManagerRemoveAllTiles", &TestCachingTileManagerRemoveAllTiles);
    CU_add_test(pTestSuite, "TestCachingConvertTileManagerInitialization", &TestCachingConvertTileManagerInitialization);
    CU_add_test(pTestSuite, "TestCachingConvertTileManagerGetTileWithoutCallback", &TestCachingConvertTileManagerGetTileWithoutCallback);
    CU_add_test(pTestSuite, "TestCachingConvertTileManagerGetTileWithCallback", &TestCachingConvertTileManagerGetTileWithCallback);
    CU_add_test(pTestSuite, "TestCachingConvertTileManagerChangeDatasetId", &TestCachingConvertTileManagerChangeDatasetId);
    CU_add_test(pTestSuite, "TestCachingConvertTileManagerRemoveAllTiles", &TestCachingConvertTileManagerRemoveAllTiles);
};

/*! Add common initialization code here

    @return 0
*/
int
TestCachingTileManager_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestCachingTileManager_SuiteCleanup()
{
    return 0;
}

TileLayerInfoPtr CreateDefaulltTileLayerInfo()
{
    TileLayerInfoPtr info = TileLayerInfoPtr(new TileLayerInfo);
    if (info)
    {
        info->drawOrder    = TEST_TILE_DRAWORDER;
        info->layerID      = TEST_LAYER_ID_DIGITAL;
        info->minZoom      = TEST_MIN_ZOOM;
        info->maxZoom      = TEST_MAX_ZOOM;
        info->tileDataType = shared_ptr<string> (new string(TEST_TILE_DATA_TYPE));
    }
    return info;
}

// Test Functions ...............................................................................

/*! Test initializing a CachingTileManager object

    @return None. CUnit Asserts get called on failures.
*/
void
TestCachingTileManagerInitialization()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }
}

void
TestCachingTileManagerGetTileWithoutCallback()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerGetTileWithoutCallbackCommon(testInstances);
}

void
TestCachingTileManagerGetTileWithCallback()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerGetTileWithCallbackCommon(testInstances);
}

void
TestCachingTileManagerChangeDatasetId()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerChangeDatasetIdCommon(testInstances);
}

void
TestCachingTileManagerRemoveAllTiles()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerRemoveAllTilesCommon(testInstances);
}

void
TestCachingConvertTileManagerInitialization()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize(TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }
}

void
TestCachingConvertTileManagerGetTileWithoutCallback()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize(TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerGetTileWithoutCallbackCommon(testInstances);
}

void
TestCachingConvertTileManagerGetTileWithCallback()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize(TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerGetTileWithCallbackCommon(testInstances);
}

void
TestCachingConvertTileManagerChangeDatasetId()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize(TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerChangeDatasetIdCommon(testInstances);
}

void
TestCachingConvertTileManagerRemoveAllTiles()
{
    NB_Error error = NE_OK;
    TestCachingTileInstances testInstances;

    // Initialize a CachingTileManager object in the function TestCachingTileInstances::Initialize.
    error = testInstances.Initialize(TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::Initialize() failed");
        return;
    }

    TestCachingTileManagerRemoveAllTilesCommon(testInstances);
}

/*! Test requesting a tile without a callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestCachingTileManagerGetTileWithoutCallbackCommon(TestCachingTileInstances& testInstances)
{
    NB_Error error = NE_OK;
    CachingTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;
    int index[1] = {0};

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateCachingTileRequestParameters(TEST_VALUE_X[0],
                                                            TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Request a tile.
    tileManager->GetTile(templateParameters,
                         TEST_TILE_REQUEST_PRIORITY);
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("CachingTileManager::GetTile failed");
        return;
    }

    //Wait some time and check that tile stored in cache
    WaitForCallbackCompletedEvent(testInstances.m_event, 10000);

    //Check tile stored in cache
    error = testInstances.CheckCache(index, 1, TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::CheckCache() failed");
        return;
    }
}

/*! Test requesting a tile with a callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestCachingTileManagerGetTileWithCallbackCommon(TestCachingTileInstances& testInstances)
{
    NB_Error error = NE_OK;
    TestCachingCallbackPtr callback;
    CachingTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;
    int index[1] = {0};

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateCachingTileRequestParameters(TEST_VALUE_X[0],
                                                            TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Create a callback object.
    callback.reset(new TestCachingCallback(testInstances));
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Request a tile with the flag 'fastLoadOnly' false.
    ++(testInstances.m_requestCount);
    tileManager->GetTile(shared_ptr<map<string, string> >(templateParameters),
                         (callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         false);
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("CachingTileManager::GetTile failed");
        return;
    }

    if (testInstances.m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 10000))
        {
            CU_FAIL("CachingTileManager::GetTile timeout");
            return;
        }
    }

    // Check if an error occurred during requesting tile.
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("An error occurred during requesting tile");
        return;
    }

    //Check tile stored in cache
    error = testInstances.CheckCache(index, 1, TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::CheckCache() failed");
        return;
    }

    // Request the tile with the flag 'fastLoadOnly' true.
    ++(testInstances.m_requestCount);
    tileManager->GetTile(shared_ptr<map<string, string> >(templateParameters),
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> >(callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         true);
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("CachingTileManager::GetTile failed");
        return;
    }

    // The count of request should be 0. Because the tile is already existing in the cache.
    CU_ASSERT_EQUAL(testInstances.m_requestCount, 0);
}

/*! Test changing dataset ID and comparing ETAG

    @return None. CUnit Asserts get called on failures.
*/
void
TestCachingTileManagerChangeDatasetIdCommon(TestCachingTileInstances& testInstances)
{
    TestCachingCallbackPtr callback;
    CachePtr cache;
    CachingTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    // Get the cache.
    cache = testInstances.m_cache;
    if (!cache)
    {
        CU_FAIL("Pointer to cache is NULL");
        return;
    }

    // Clear the cache.
    shared_ptr<string> emptyPointer;
    cache->RemoveData(emptyPointer, emptyPointer);

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateCachingTileRequestParameters(TEST_VALUE_X[0],
                                                            TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Create a callback object.
    callback.reset(new TestCachingCallback(testInstances));
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Remove the tile from cache.

    // Request a tile with the flag 'fastLoadOnly' false.
    ++(testInstances.m_requestCount);
    tileManager->GetTile(shared_ptr<map<string, string> >(templateParameters),
                         (callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         false);
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("CachingTileManager::GetTile failed");
        return;
    }

    if (testInstances.m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 10000))
        {
            CU_FAIL("CachingTileManager::GetTile timeout");
            return;
        }
    }

    // Check if an error occurred during requesting tile.
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("An error occurred during requesting tile");
        return;
    }

    // Create a string of caching type.
    shared_ptr<string> cachingType(new string(TEST_CACHING_TYPE));
    if (!cachingType)
    {
        CU_FAIL("Out of memory when allocated caching type");
        return;
    }

    // Get the content ID.
    shared_ptr<string> contentId = testInstances.m_contentId;
    if (!contentId)
    {
        CU_FAIL("Pointer to content ID is NULL");
        return;
    }

    // Get the additional data from cache.
    shared_ptr<map<string, shared_ptr<string> > > additionalData = cache->GetAdditionalData(cachingType, contentId);
    if (!additionalData)
    {
        CU_FAIL("Pointer to additional data is NULL");
        return;
    }

    // Get the dataset ID.
    shared_ptr<string> datasetId = (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID];
    if ((!datasetId) || ((*datasetId) != TEST_DATASET_ID1))
    {
        CU_FAIL("Dataset ID is wrong");
        return;
    }

    // Get the ETAG.
    shared_ptr<string> etag1 = (*additionalData)[TILE_ADDITIONAL_KEY_ETAG];
    if ((!etag1) || (etag1->empty()))
    {
        CU_FAIL("ETAG is wrong");
        return;
    }

    // Create another dataset ID.
    datasetId.reset();
    datasetId.reset(new string(TEST_DATASET_ID2));
    if (!datasetId)
    {
        CU_FAIL("Out of memory when allocated dataset ID");
        return;
    }

    // Set new dataset ID to the additional data and save it to cache.
    (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID] = datasetId;
    cache->SetAdditionalData(cachingType, contentId, additionalData);

    // Request the tile again.
    ++(testInstances.m_requestCount);
    tileManager->GetTile(shared_ptr<map<string, string> >(templateParameters),
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> >(callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         false);
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("CachingTileManager::GetTile failed");
        return;
    }

    if (testInstances.m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 10000))
        {
            CU_FAIL("CachingTileManager::GetTile timeout");
            return;
        }
    }

    // Get the updated additional data from cache.
    additionalData = cache->GetAdditionalData(cachingType, contentId);
    if (!additionalData)
    {
        CU_FAIL("Pointer to additional data is NULL");
        return;
    }

    // The dataset ID should be TEST_DATASET_ID1.
    datasetId = (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID];
    if ((!datasetId) || ((*datasetId) != TEST_DATASET_ID1))
    {
        CU_FAIL("Dataset ID is wrong");
        return;
    }

    shared_ptr<string> etag2 = (*additionalData)[TILE_ADDITIONAL_KEY_ETAG];
    if ((!etag2) || (etag2->empty()) || ((*etag1) != (*etag2)))
    {
        CU_FAIL("ETAG is wrong");
        return;
    }

    // The count of request should be 0. Because the tile is already existing in the cache.
    CU_ASSERT_EQUAL(testInstances.m_requestCount, 0);
}

/*! Test removing all tiles

    @return None. CUnit Asserts get called on failures.
*/
void
TestCachingTileManagerRemoveAllTilesCommon(TestCachingTileInstances& testInstances)
{
    NB_Error error = NE_OK;
    CachingTileManagerPtr tileManager;
    int index = 0;
    int indexes[3] = {0, 1, 2};

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Request several tiles and store them in cache
    for (index = 0; index < TEST_TILE_KEY_COUNT; index++)
    {
        // Create the parameters to request a tile.
        shared_ptr<map<string, string> > templateParameters = CreateCachingTileRequestParameters(TEST_VALUE_X[index],
                                                                                                 TEST_VALUE_Y[index]);
        if (!templateParameters)
        {
            CU_FAIL("Parameters to request a tile is NULL");
            return;
        }

        // Request a tile.
        tileManager->GetTile(templateParameters,
                             TEST_TILE_REQUEST_PRIORITY);
        if (testInstances.m_requestError != NE_OK)
        {
            CU_FAIL("CachingTileManager::GetTile failed");
            return;
        }
    }

    // Wait while tiles stored in cache
    WaitForCallbackCompletedEvent(testInstances.m_event, 10000);

    //Check tiles stored in cache
    error = testInstances.CheckCache(indexes, 3, TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::CheckCache() failed");
        return;
    }

    // Remove all tiles.
    tileManager->RemoveAllTiles();

    // TileManager::RemoveAllTiles() will not clear cache.
    //Check tiles removed from cache
    error = testInstances.CheckCache(indexes, 3, TRUE);
    if (error != NE_OK)
    {
        CU_FAIL("TestCachingTileInstances::CheckCache() failed");
        return;
    }

    // Request tiles again with flag 'fastLoadOnly' true and check tiles removed from cache
    for (index = 0; index < TEST_TILE_KEY_COUNT; index++)
    {
        TestCachingCallbackPtr callback;

        // Create the parameters to request a tile.
        shared_ptr<map<string, string> > templateParameters = CreateCachingTileRequestParameters(TEST_VALUE_X[index],
                                                                                                 TEST_VALUE_Y[index]);
        if (!templateParameters)
        {
            CU_FAIL("Parameters to request a tile is NULL");
            return;
        }

        // Create a callback object.
        callback.reset(new TestCachingCallback(testInstances));
        if (!callback)
        {
            CU_FAIL("Out of memory when allocated callback");
            return;
        }

        ++(testInstances.m_requestCount);
        tileManager->GetTile(shared_ptr<map<string, string> >(templateParameters),
                             shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> >(callback),
                             TEST_TILE_REQUEST_PRIORITY,
                             true);

        // Check requested tile removed from cache(request should fail)
        if (testInstances.m_requestError != NE_OK)
        {
            CU_FAIL("CachingTileManager::GetTile failed");
            return;
        }
    }

    CU_ASSERT_EQUAL(testInstances.m_requestCount, 0);
}


// Helper functions .............................................................................

/*! Create the parameters to request a tile

    @return A template parameter map to request a tile
*/
shared_ptr<map<string, string> >
CreateCachingTileRequestParameters(const char valueX[], const char valueY[])
{
    shared_ptr<map<string, string> > parameters = shared_ptr<map<string, string> >(new map<string, string>());

    if (!parameters)
    {
        return shared_ptr<map<string, string> >();
    }

    parameters->insert(pair<string, string>(TEST_KEY_X, valueX));
    parameters->insert(pair<string, string>(TEST_KEY_Y, valueY));

    return parameters;
}


// TestCachingTileInstances functions ...........................................................

/*! TestCachingTileInstances constructor */
TestCachingTileInstances::TestCachingTileInstances() : m_requestError(NE_OK),
                                                       m_requestCount(0),
                                                       m_pal(NULL),
                                                       m_context(NULL),
                                                       m_event(NULL),
                                                       m_tileManager(),
                                                       m_cache(),
                                                       m_contentId()
{
    // Nothing to do here.
}

/*! TestCachingTileInstances destructor */
TestCachingTileInstances::~TestCachingTileInstances()
{
    Reset();
}

/*! Initialize a TestCachingTileInstances object

    @return NE_OK if success,
            NE_EXIST if this object has already been initialized,
            NE_NOTINIT if this object is not properly initialized.
*/
NB_Error
TestCachingTileInstances::Initialize(bool convert)
{
    NB_Error error = NE_OK;

    // Check if this TestCachingTileInstances object has already been initialized.
    if (m_pal && m_context && m_event && m_tileManager && m_cache)
    {
        return NE_EXIST;
    }

    // Check if this TestCachingTileInstances object is not properly initialized.
    if (m_pal || m_context || m_event || m_tileManager || m_cache)
    {
        return NE_NOTINIT;
    }

    // Create a PAL and context.
    if (!CreatePalAndContext(&m_pal, &m_context))
    {
        Reset();
        return NE_NOTINIT;
    }
    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    // Create an event.
    m_event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(m_event);
    if (!m_event)
    {
        Reset();
        return NE_NOTINIT;
    }

    // Create and initialize a Cache object.
    shared_ptr<string> cachingPath(new string(GetBasePath()));

    if (cachingPath)
    {
        shared_ptr<string> cachingName(new string(TEST_CACHING_NAME));
        if (!cachingName)
        {
            Reset();
            return NE_NOMEM;
        }

        m_cache.reset(new Cache());
        if (!m_cache)
        {
            Reset();
            return NE_NOMEM;
        }

        error = m_cache->Initialize(m_context,
                                    TEST_MAX_CACHE_DISK_SPACE,
                                    cachingName,
                                    cachingPath);
        if (error != NE_OK)
        {
            Reset();
            return error;
        }
    }

    // Create a string of caching type for testing.
    shared_ptr<string> cachingType(new string(TEST_CACHING_TYPE));
    if (!cachingType)
    {
        Reset();
        return NE_NOMEM;
    }

    // Create a string of dataset ID for testing.
    shared_ptr<string> datasetId(new string(TEST_DATASET_ID1));
    if (!datasetId)
    {
        Reset();
        return NE_NOMEM;
    }

    // Create an OffboardTileType object to initialize the tile manager.
    OffboardTileTypePtr tileType(new OffboardTileType(shared_ptr<string>(new string(TEST_TILE_DATA_TYPE)),
                                                      shared_ptr<string>(new string(TEST_ENTIRE_URL_TEMPLATE)),
                                                      shared_ptr<string>(new string(TEST_CONTENT_ID_TEPLATE))));
    if (!tileType)
    {
        Reset();
        return NE_NOMEM;
    }

    TileLayerInfoPtr info = CreateDefaulltTileLayerInfo();
    TileManagerPtr tileManager;

    if (!convert)
    {
        // Create and initialize an OffboardTileManager object.
        OffboardTileManagerPtr offboardTileManager(new OffboardTileManager());

        if (!offboardTileManager)
        {
            Reset();
            return NE_NOMEM;
        }

        error = offboardTileManager->Initialize(m_context,
                                                true,
                                                TEST_DOWNLOAD_PRIORITY,
                                                TEST_TILE_REQUEST_RETRY_TIMES,
                                                0,
                                                TEST_DEFAULT_MAX_REQUEST_COUNT,
                                                tileType,
                                                info);
        if (error != NE_OK)
        {
            Reset();
            return error;
        }

        tileManager = TileManagerPtr(offboardTileManager);
    }
    else
    {
        TileConverterPtr tileConverter(new RasterTileConverter());

        if (!tileConverter)
        {
            Reset();
            return NE_NOMEM;
        }

        // Create and initialize an OffboardConvertTileManager object with the OffboardTileType object.
        OffboardConvertTileManagerPtr offboardConvertTileManager(new OffboardConvertTileManager());

        if (!offboardConvertTileManager)
        {
            Reset();
            return NE_NOMEM;
        }

        error = offboardConvertTileManager->Initialize(m_context,
                                                       false,
                                                       TEST_DOWNLOAD_PRIORITY,
                                                       TEST_TILE_REQUEST_RETRY_TIMES,
                                                       0,
                                                       TEST_DEFAULT_MAX_REQUEST_COUNT,
                                                       tileType,
                                                       tileConverter,
                                                       info);
        if (error != NE_OK)
        {
            Reset();
            return error;
        }

        tileManager = TileManagerPtr(offboardConvertTileManager);
    }

    // Create and initialize an CachingTileManager object.
    m_tileManager.reset(new CachingTileManager());
    if (!m_tileManager)
    {
        Reset();
        return NE_NOMEM;
    }

    error = m_tileManager->Initialize(m_context,
                                      tileType,
                                      tileManager,
                                      m_cache,
                                      cachingType,
                                      datasetId,
                                      info);
    if (error != NE_OK)
    {
        Reset();
        return error;
    }

    // Set the common parameters for the tile manager.
    {
        map<string, string>* commonParameters = NULL;

        commonParameters = new map<string, string>();
        if (!commonParameters)
        {
            Reset();
            return NE_NOMEM;
        }

        commonParameters->insert(pair<string, string>(TEST_KEY_FORMAT, TEST_VALUE_FORMAT));
        commonParameters->insert(pair<string, string>(TEST_KEY_FORMAT_VERSION, TEST_VALUE_FORMAT_VERSION));

        if (convert)
        {
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_VERSION, TEST_METADATA_DEFAULT_VERSION));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_LOCALE, TEST_METADATA_DEFAULT_LOCALE));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_RESOLUTION, TEST_METADATA_DEFAULT_RESOLUTION));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_SIZE, TEST_METADATA_DEFAULT_TILE_SIZE));
        }

        // Take ownership of the common parameter map object.
        error = m_tileManager->SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > (commonParameters));
        if (error != NE_OK)
        {
            Reset();
            return error;
        }
    }

    return error;
}

/*! Reset a TestOffboardTileInstances object to be uninitialized

    Destroy the PAL, context, event and OffboardTileManager object in the
    TestOffboardTileInstances object.

    @return None
*/
void
TestCachingTileInstances::Reset()
{
    m_contentId.reset();
    m_cache.reset();
    m_tileManager.reset();

    if (m_event)
    {
        DestroyCallbackCompletedEvent(m_event);
        m_event = NULL;
    }

    if (m_context)
    {
        NB_Error error = NE_OK;

        error = NB_ContextDestroy(m_context);
        CU_ASSERT_EQUAL(error, NE_OK);
        m_context = NULL;
    }

    if (m_pal)
    {
        PAL_Destroy(m_pal);
        m_pal = NULL;
    }

    m_requestCount = 0;
    m_requestError = NE_OK;
}

/*! Reset a TestOffboardTileInstances object to be uninitialized

    Destroy the PAL, context, event and OffboardTileManager object in the
    TestOffboardTileInstances object.

    @return None
*/
NB_Error
TestCachingTileInstances::CheckCache(int indexes[], int count, bool shouldContain)
{
    PAL_Error error = PAL_Ok;
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo fileInformation = {0};
    int tileIndex = 0;
    char tileCachePath[MAX_FULL_PATH_LENGTH] = {0};
    bool tileContains[TEST_TILE_KEY_COUNT] = {0};

    nsl_strncpy(tileCachePath, GetBasePath(), nsl_strlen(GetBasePath()) + 1);

    error = PAL_FileAppendPath(m_pal,
                               tileCachePath,
                               MAX_FULL_PATH_LENGTH,
                               TEST_CACHING_NAME);

    if (error != PAL_Ok)
    {
        CU_FAIL("PAL_FileAppendPath failed");
        return NE_INVAL;
    }

    error = PAL_FileEnumerateCreate(m_pal, tileCachePath, FALSE, &fileEnum);

    if (error != PAL_Ok)
    {
        CU_FAIL("PAL_FileEnumerateCreate failed");
        return NE_INVAL;
    }

    //Check data for all tiles  stored in cache
    while(PAL_Ok == PAL_FileEnumerateNext(fileEnum, &fileInformation))
    {
        char tmpCachePath[MAX_FULL_PATH_LENGTH] = {0};

        nsl_strncpy(tmpCachePath, tileCachePath, nsl_strlen(tileCachePath) + 1);

        error = PAL_FileAppendPath(m_pal,
                                   tmpCachePath,
                                   MAX_FULL_PATH_LENGTH,
                                   fileInformation.filename);

        if (error != PAL_Ok)
        {
            CU_FAIL("PAL_FileAppendPath failed");
            return NE_INVAL;
        }

        if (PAL_FileIsDirectory(m_pal, tmpCachePath))
        {
            if (!nsl_strncmp(fileInformation.filename, TEST_CACHING_TYPE, nsl_strlen(TEST_CACHING_TYPE)))
            {
                PAL_FileEnum* tmpFileEnum = NULL;
                PAL_FileEnumInfo tmpFileInformation = {0};

                error = PAL_FileEnumerateCreate(m_pal, tmpCachePath, TRUE, &tmpFileEnum);

                if (error != PAL_Ok)
                {
                    CU_FAIL("PAL_FileEnumerateCreate failed");
                    return NE_INVAL;
                }

                while(PAL_Ok == PAL_FileEnumerateNext(tmpFileEnum, &tmpFileInformation))
                {
                    for (tileIndex = 0; tileIndex < count; tileIndex++)
                    {
                        if (nsl_strstr(tmpFileInformation.filename, TEST_VALUE_X[indexes[tileIndex]]) != NULL &&
                            nsl_strstr(tmpFileInformation.filename, TEST_VALUE_Y[indexes[tileIndex]]) != NULL)
                        {
                            tileContains[tileIndex] = TRUE;
                            break;
                        }
                    }
                }

                error = PAL_FileEnumerateDestroy(tmpFileEnum);

                if (error != PAL_Ok)
                {
                    CU_FAIL("PAL_FileEnumerateDestroy failed");
                    return NE_INVAL;
                }

                break;
            }
            else
            {
                CU_FAIL("Wrong caching type");
                break;
            }
        }
    }

    for (tileIndex = 0; tileIndex < count; tileIndex++)
    {
        CU_ASSERT_EQUAL(tileContains[tileIndex], shouldContain);
    }

    PAL_FileEnumerateDestroy(fileEnum);

    if (error != PAL_Ok)
    {
        CU_FAIL("PAL_FileEnumerateDestroy failed");
        return NE_INVAL;
    }

    return NE_OK;
}

// TestCachingCallback functions ................................................................

/*! TestCachingCallback constructor */
TestCachingCallback::TestCachingCallback(TestCachingTileInstances& testInstances    /*!< A TestCachingTileInstances object */
                                         ) : m_testInstances(testInstances)
{
    // Nothing to do here.
}

/*! TestCachingCallback destructor */
TestCachingCallback::~TestCachingCallback()
{
    // Nothing to do here.
}

/* See description in AsyncCallback.h */
void
TestCachingCallback::Success(shared_ptr<map<string, string> > request,
                             TilePtr response)
{
    shared_ptr<string> contentId;
    DataStreamPtr dataStream;

    if (!response)
    {
        CU_FAIL("Pointer to responsed tile is NULL");
        SetCallbackCompletedEvent(m_testInstances.m_event);
        return;
    }

    contentId = response->GetContentID();
    dataStream = response->GetData();
    if ((!contentId) || (!dataStream))
    {
        CU_FAIL("Responsed tile is invalid");
        SetCallbackCompletedEvent(m_testInstances.m_event);
        return;
    }

    // The request succeeds.
    m_testInstances.m_contentId = contentId;
    --(m_testInstances.m_requestCount);
    SetCallbackCompletedEvent(m_testInstances.m_event);
}

/* See description in AsyncCallback.h */
void
TestCachingCallback::Error(shared_ptr<map<string, string> > request,
                           NB_Error error)
{
    // Save the request error in TestOffboardTileInstances object.
    m_testInstances.m_requestError = error;

    // The request fails.
    --(m_testInstances.m_requestCount);
    SetCallbackCompletedEvent(m_testInstances.m_event);
}

/* See description in AsyncCallback.h */
bool
TestCachingCallback::Progress(int percentage)
{
    // Nothing to do here.
    return false;
}

/*! @} */
