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
  @file        TestLayerManager.cpp
  @defgroup    test

  Description: Implementation of test case for LayerManger.

*/
/*
  (C) Copyright 2012 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  --------------------------------------------------------------------------*/

/*! @{ */

#include "CUnit.h"

extern "C"
{
#include "TestLayerManager.h"
#include "main.h"
#include "nbcontext.h"
#include "testnetwork.h"
#include "platformutil.h"
#include "palfile.h"
#include "palclock.h"
#include "paltimer.h"
}

#include <stdio.h>
#include <vector>
#include <set>
#include "TestInstance.h"
#include "LayerManager.h"
#include "TileServicesConfiguration.h"
#include "MetadataConfiguration.h"

#ifdef __GNUC__
#define FUNCTION(func) instance->func()
#else
#define FUNCTION(func) instance->##func()
#endif

#define PERFORM_TEST(CLASS, func, waitTimeout)        \
    do {                                              \
        CLASS* instance = new CLASS();                \
        CU_ASSERT_PTR_NOT_NULL(instance);             \
        if (instance != NULL)                         \
        {                                             \
            FUNCTION(func);                           \
            if (!instance->WaitEvent(waitTimeout))    \
            {                                         \
                CU_FAIL("Test is timed out!");        \
                instance->ScheduleFinishTest();       \
                instance->WaitEvent(DESTROY_TIMEOUT); \
            }                                         \
            delete instance;                          \
        }                                             \
    } while (0);

#define DO_REAL_TEST(TEST_CLASS, TEST_FUNCTION, CALLBACK_FUNCTION)  \
    do                                                              \
    {                                                               \
        m_functor = new TestTask<TEST_CLASS>                        \
                    (this,                                          \
                     TEST_FUNCTION,                                 \
                     CALLBACK_FUNCTION);                            \
        CallTestFunction();                                         \
    } while (0)


using namespace nbcommon;
using namespace nbmap;
using namespace std;

//  Local Constants
//  ...........................................................................

#define NBGM_VERSION       23
#define REPEAT_TIMES       4

#define MAX_FULL_PATH_LENGTH 256

/* There are B3D, DVA, DVR and LM3D layers in test tile keys(1 tile for each layer).
   There are 2 files of common materials(MAT) downloaded when
   metadata returns (No test callback) and 1 LAM tile is handled by CCC internally, no
   callback either).
*/
#define REQUEST_COUNT TEST_TILE_KEY_COUNT * TEST_LAYER_COUNT
#define REQUEST_RASTER_TILE_COUNT TEST_TILE_KEY_COUNT * TEST_RASTER_TILE_LAYER_COUNT

static const uint32 TEST_MAXIMUM_CACHING_TILE_COUNT          = 1000;
static const uint32 TEST_MAXIMUM_TILE_REQUEST_COUNT_PERLAYER = 6;

static const uint32 TEST_MAP_NBGM_VERSION                    = 23;
static const uint32 TEST_DATA_SIZE                           = 4096;
static const uint32 TEST_MAX_CACHING_TILE_COUNT              = 100;
// Set cache size for only 2 tile keys to test full cache
static const uint32 TEST_MAX_CACHING_TILE_COUNT_FULL         = 11;
static const uint32 TEST_MAX_TILE_REQUEST_PER_LAYER          = 3;

/* @todo: Add another zoom level to test raster tiles. Because Raster tiles display from
   zoom level 0 to 16 and vector tiles display from zoom level 17 to 20. */
static const int    TEST_REQUEST_ZOOM_LEVEL                  = 18;
static const int    TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL      = 7;

// Test values of min and max zoom levels of background raster layers, those variable may change.
static const uint32 TEST_MIN_BACKGROUND_RASTER_ZOOM_LEVEL = 0;
static const uint32 TEST_MAX_BACKGROUND_RASTER_ZOOM_LEVEL = 15;

//  Test tile keys contained tile x, y and zoom level
static const int    TEST_TILE_KEY_COUNT                      = 3;
static const int TEST_TILE_KEY_ARRAY[TEST_TILE_KEY_COUNT][3] =
{
    {5241, 12665, 15},
    {5242, 12662, 15},
    {5243, 12665, 15}
};

static const int TEST_LAYER_COUNT                         = 4; //  Four vector map layer
static const char TEST_LAYERS_ARRAY[TEST_LAYER_COUNT][10] = {"B3D", "DVA", "DVR", "LM3D"};
static const int TEST_RASTER_TILE_LAYER_COUNT             = 1; // Raster map
// @todo: add test for other layers: BR0 ~ BR8.
static const char TEST_RASTER_TILE_LAYERS_ARRAY[TEST_RASTER_TILE_LAYER_COUNT][10] =
{
    "BR7"
    // "BR0", "BR1", "BR2", "BR3", "BR4", "BR5", "BR6", "BR7", "BR8"
};

static const char TEST_PERSISTENT_METADATA_FILENAME[] = "testpersistentmetadata";
static const char TEST_TILE_CACHING_NAME[] = "NBM";
static const uint32 CALLBACK_TIMEOUT = 180 * 1000;
static const uint32 CALLBACK_TIMEOUT_GETTILES = 60 * 1000;

class TestLayerManagerInstance;

typedef  void (TestLayerManagerInstance::*MetadataSentinel)();


//  Local Classes  ................................................................
/*! Collection of all instances used for the unit tests */
class TestLayerManagerInstance : public TestInstance
{
public:
    /*! Default constructor. */
    TestLayerManagerInstance();

    /*! Default destructor. */
    virtual ~TestLayerManagerInstance();

    virtual void
    FinishTestPrivate();

    /*! Initializes the Test instance.

      @return NB_Error
    */
    NB_Error
    Initialize(bool testFullCache = false);

    /*! Start metadata request.

      @return NE_OK if succeeded.
    */
    NB_Error
    RequestMetadata(MetadataSentinel sentinel = NULL);

    void Reset();
    bool IsValid();

    LayerManagerPtr  m_layerManager;
    vector<LayerPtr> m_layers;
    shared_ptr<string> m_cachePath;
    void* m_event;

    bool m_testRaster;
    bool m_testIgnore;
    bool m_duplicateTileKey;

    // Real Test functions starts from here.
    void TestLayerManagerCreate();
    void TestLayerManagerSetCachePath();
    void TestLayerManagerSetPersistentMetadataPath();

    void TestLayerManagerGetRasterTilesWithCallback();
    void TestLayerManagerGetTilesWithCallback();
    void TestLayerManagerGetTilesWithCallbackMultiple();
    void TestLayerManagerGetTilesWithCallbackMultipleForRasterTile();
    void TestLayerManagerGetTilesWithIgnoredList();
    void TestLayerManagerGetTilesWithoutCallback();
    void TestLayerManagerGetTilesWithoutCallbackForRasterTile();
    void TestLayerManagerMetadataRequest();

    void TestMasterClear();


    // Helper functions to test.
    void Callback();

    void CreateNotCachedLayerManager();
    void GetRasterTilesWithCallback();
    void GetTilesWithCallback();
    void GetTilesWithCallbackMultiple();
    void GetTilesWithCallbackMultipleForRasterTile();
    void GetTilesWithIgnoredList();
    void GetTilesWithoutCallback();
    void GetTilesWithoutCallbackForRasterTile();
    void MasterClear();
    void MetadataRequest();
    void SetCachePath();
    void SetPersistentMetadataPath();

    void StartTileRequestsWithoutCallback();
    void StartTileRequestsWithCallback();
    void TileDownloadCallback();

    // Callbacks
    void MetadataRequestCallback();
};

class LayerAsyncCallback :
        public nbmap::AsyncCallback<const vector<shared_ptr<nbmap::Layer > >& >
{
public:
    LayerAsyncCallback(TestLayerManagerInstance* instance,
                       MetadataSentinel sentinel = NULL)
            : m_instance(instance),
              m_sentinel(sentinel)
    {
    }

    virtual ~LayerAsyncCallback()
    {
    }

    void Success(const vector<shared_ptr<nbmap::Layer > >& response)
    {
        for (unsigned int i = 0; i < response.size(); i++)
        {
            shared_ptr<string> type = response[i]->GetTileDataType();
            if ((!type) || (type->empty()))
            {
                CU_FAIL("Responsed layer is invalid");
            }
        }

        if (m_instance)
        {
            m_instance->m_layers = response;
            if (m_sentinel)
            {
                (m_instance->*m_sentinel)();
            }
            else
            {
                if (m_instance->m_functor)
                {
                    m_instance->m_functor->ExecuteCallback();
                }
                else
                {
                    SetCallbackCompletedEvent(m_instance->m_completeEvent);
                }
            }
        }
    }

    void Error(NB_Error error)
    {
        CU_FAIL("Error occurred.");
        if (m_instance->m_functor)
        {
            m_instance->m_functor->ExecuteCallback();
        }
    }

    bool Progress(int percentage)
    {
        return true;
    }

    TestLayerManagerInstance* m_instance;
    void (TestLayerManagerInstance::*m_sentinel)();
};

/*! Test callback object for getting a tile */
class TestTileCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>
{
public:
    /* See definition for description */

    TestTileCallback(TestLayerManagerInstance* instance,
                     int   requestCount,
                     bool  testIgnore = false)
    {
        m_instance = instance;
        m_requestCount = requestCount;
        m_testIgnore   = testIgnore;
    }

    virtual ~TestTileCallback()
    {
    }

    /* See description in AsyncCallback.h */
    virtual void Success(TileKeyPtr request,
                         TilePtr response)
    {
        if (!response)
        {
            CU_FAIL("Pointer to responsed tile is NULL");
            SetCallbackCompletedEvent(m_instance->m_completeEvent);
            return;
        }

        if (m_testIgnore)
        {
            uint32 id = response->GetLayerID();
            if (id >= 0x20 && id <= 0x3F)
            {
                CU_FAIL("Tiles received from ignored layers!");
            }
        }
        shared_ptr<string> dataType   = response->GetDataType();
        shared_ptr<string> contentId  = response->GetContentID();
        DataStreamPtr      dataStream = response->GetData();

        if ((!dataType) || (!contentId) || (!dataStream))
        {
            CU_FAIL("Responsed tile is invalid");
            SetCallbackCompletedEvent(m_instance->m_completeEvent);
            return;
        }

        if ((--m_requestCount) <= 0)
        {
            if (m_instance->m_functor)
            {
                m_instance->m_functor->ExecuteCallback();
            }
            else
            {
                SetCallbackCompletedEvent(m_instance->m_completeEvent);
            }
        }
    }

    virtual void Error(TileKeyPtr request,
                       NB_Error error)
    {
        //  @todo: Ignore the error for now. Because there is no logic for parsing LAM.
        if (m_instance->m_functor)
        {
            m_instance->m_functor->ExecuteCallback();
        }
        else
        {
            SetCallbackCompletedEvent(m_instance->m_completeEvent);
        }
    }

    virtual bool Progress(int percentage)
    {
        //  Nothing to do here.
        return false;
    }

    int   m_requestCount;
    TestLayerManagerInstance* m_instance;
    bool  m_testIgnore;
};

//  Local Classes Implementation ...........................................................
TestLayerManagerInstance::TestLayerManagerInstance()
        : TestInstance(),
          m_testRaster(false),
          m_duplicateTileKey(false),
          m_testIgnore(false)
{
}

TestLayerManagerInstance::~TestLayerManagerInstance()
{
    Reset();
}

NB_Error TestLayerManagerInstance::Initialize(bool testFullCache)
{
    NB_Error error = NE_OK;

    do
    {
        // This class should be initialized
        CU_ASSERT(m_initialized);
        if (!m_initialized)
        {
            error = NE_NOTINIT;
            break;
        }

        m_event = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(m_event);
        if (!m_event)
        {
            error = NE_NOTINIT;
            break;
        }

        uint32 maxCachingTileCount = testFullCache ?
                                     TEST_MAX_CACHING_TILE_COUNT_FULL :
                                     TEST_MAX_CACHING_TILE_COUNT;


        shared_ptr<MetadataConfiguration> metadataConfiguration =
                MetadataConfiguration::GetInstance(m_context);
        if (! metadataConfiguration.get())
        {
            error = NE_NOTINIT;
            CU_FAIL("Failed to retrieve metadata configuration");
            break;
        }

        metadataConfiguration->m_wantLableLayers = true;
        m_layerManager = TileServicesConfiguration::GetLayerManager(m_context,
                                                                    maxCachingTileCount,
                                                                    TEST_MAXIMUM_TILE_REQUEST_COUNT_PERLAYER,
                                                                    metadataConfiguration);

        if (!m_layerManager)
        {
            error = NE_NOTINIT;
            CU_FAIL("Failed to crate layerManager");
            break;
        }

        m_cachePath = shared_ptr<string>(new string(GetBasePath()));
        if (!m_cachePath)
        {
            error = NE_NOMEM;
            CU_FAIL("Failed to get BasePath");
            break;
        }

        //  Test setting the persistent metadata path.
        if (m_cachePath->size() >= MAX_FULL_PATH_LENGTH)
        {
            error = NE_NOTINIT;
            CU_FAIL("The length of path is out of range");
            break;
        }

    } while (0);

    return error;
}

void TestLayerManagerInstance::Reset()
{
    if (m_event)
    {
        DestroyCallbackCompletedEvent(m_event);
        m_event = NULL;
    }

    //  Make sure layerManager is deconstructed before Context and PAL.
    m_layers.clear();
    m_layerManager.reset();
}

bool TestLayerManagerInstance::IsValid()
{
    bool result = true;
    if (!m_context || !m_pal || !m_layerManager)
    {
        result = false;
    }
    return result;
}

NB_Error TestLayerManagerInstance::RequestMetadata(MetadataSentinel sentinel)
{
    NB_Error nbError = NE_OK;
    if (!IsValid())
    {
        return NE_INVAL;
    }

    shared_ptr<string> cachePath(new string(GetBasePath()));

    if (!cachePath)
    {
        CU_FAIL("Out of memory when allocated a string of cache path");
        return NE_NOMEM;
    }

    char testTilePath[MAX_FULL_PATH_LENGTH] = {0};
    nsl_strlcpy(testTilePath, GetBasePath(), nsl_strlen(GetBasePath()) + 1);

    PAL_Error palError = PAL_FileAppendPath(m_pal,
                                            testTilePath,
                                            MAX_FULL_PATH_LENGTH,
                                            TEST_TILE_CACHING_NAME);

    if (palError != PAL_Ok)
    {
        CU_FAIL("PAL_FileAppendPath failed");
        return NE_FSYS;
    }

    //  Remove all old data from tile cache directory
    PAL_FileRemoveDirectory(m_pal, testTilePath, TRUE);

    //  Test setting the cache path.
    nbError = m_layerManager->SetCachePath(cachePath);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetCachePath failed");
        return nbError;
    }

    //  Test setting the persistent metadata path.
    if (cachePath->size() >= MAX_FULL_PATH_LENGTH)
    {
        CU_FAIL("The length of path is out of range");
        return NE_RANGE;
    }

    char testMetadataPath[MAX_FULL_PATH_LENGTH] = {0};
    nsl_strlcpy(testMetadataPath, cachePath->c_str(), nsl_strlen(cachePath->c_str()) + 1);

    palError = PAL_FileAppendPath(m_pal,
                                  testMetadataPath,
                                  MAX_FULL_PATH_LENGTH,
                                  TEST_PERSISTENT_METADATA_FILENAME);

    if (palError != PAL_Ok)
    {
        CU_FAIL("PAL_FileAppendPath failed");
        return NE_FSYS;
    }

    nbError = m_layerManager->SetPersistentMetadataPath(shared_ptr<string>(new string(testMetadataPath)));

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetPersistentMetadataPath failed");
        return nbError;
    }

    shared_ptr<LayerAsyncCallback> layerCallback(new LayerAsyncCallback(this, sentinel));
    if (!layerCallback)
    {
        CU_FAIL("Out of memory when allocated LayerAsyncCallback");
        return NE_NOMEM;
    }

    shared_ptr<MetadataConfiguration> metadataConfiguration =  MetadataConfiguration::GetInstance(m_context);
    if (! metadataConfiguration.get())
    {
        CU_FAIL("Failed to retrieve metadata configuration");
        return NE_NOTINIT;
    }

    metadataConfiguration->m_wantLableLayers = true;
    m_layerManager->SetMetadataRetryTimes(3);

    nbError = m_layerManager->GetLayers(layerCallback, metadataConfiguration);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetLayers failed");
    }

    return nbError;
}


//  Local functions ........................................................................

/*! Create TileKey objects to test requesting tiles

  @return NE_OK if success
*/
NB_Error
CreateTestTileKeys(vector<TileKeyPtr>& tileKeys,                   /*!< Vector to hold TileKeys.*/
                   int specialZoomLevel = TEST_REQUEST_ZOOM_LEVEL, /*!< translate to speical test zoom level.*/
                   int repeatTimes = 1                             /*!< Repeat times of tilekeys, used to test
                                                                     whether layerManager can handle same
                                                                     tile multiple times.*/
                   )
{
    do
    {
        for (int i = 0; i < TEST_TILE_KEY_COUNT; ++i)
        {
            int tileX = TEST_TILE_KEY_ARRAY[i][0];
            int tileY = TEST_TILE_KEY_ARRAY[i][1];
            int zoomLevel = TEST_TILE_KEY_ARRAY[i][2];
            int distance = specialZoomLevel - zoomLevel;

            if (distance  > 0)
            {
                tileX = (tileX << distance);
                tileY = (tileY << distance);
            }
            else if (distance < 0)
            {
                tileX = (tileX >> (-distance));
                tileY = (tileY >> (-distance));
            }

            TileKeyPtr tileKey(new TileKey(tileX,
                                           tileY,
                                           specialZoomLevel));
            if (!tileKey)
            {
                CU_FAIL("Out of memory when allocated TileKey");
                return NE_NOMEM;
            }

            tileKeys.push_back(tileKey);
        }
    } while (--repeatTimes);

    return NE_OK;
}

int GetUniqueTileCount(vector<TileKeyPtr> tileKeys)
{
    set<TileKey> tileSet;
    size_t i = 0;
    for (i; i < tileKeys.size(); ++i)
    {
        tileSet.insert(*tileKeys[i]);
    }

    return tileSet.size();
}

/* See description in header file. */
void TestLayerManagerInstance::Callback()
{
    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerCreate()
{
    DO_REAL_TEST(TestLayerManagerInstance, &TestLayerManagerInstance::CreateNotCachedLayerManager, NULL);

    m_functor = new TestTask<TestLayerManagerInstance>(this,&TestLayerManagerInstance::CreateNotCachedLayerManager, NULL);
    CallTestFunction();
}


/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerSetCachePath()
{
    DO_REAL_TEST(TestLayerManagerInstance, &TestLayerManagerInstance::SetCachePath, NULL);
}

/* See description in header file. */
void TestLayerManagerInstance::SetCachePath()
{
    do
    {
        NB_Error nbError = Initialize(false);
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        shared_ptr<string> cachePath(new string(GetBasePath()));
        if (!cachePath)
        {
            CU_FAIL("Out of memory when allocated a string of cache path");
            break;
        }

        CU_ASSERT(!m_cachePath->empty());

        nbError = m_layerManager->SetCachePath(m_cachePath);
        if (nbError != NE_OK)
        {
            CU_FAIL("LayerManager::SetCachePath failed");
        }
    } while (0);

    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::CreateNotCachedLayerManager()
{
    NB_Error error = Initialize(false);
    if (error)
    {
        CU_FAIL("Failed to create LayerManager");
    }
    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerSetPersistentMetadataPath()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::SetPersistentMetadataPath, NULL);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithCallback()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithCallback,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetRasterTilesWithCallback()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetRasterTilesWithCallback,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithCallbackMultiple()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithCallbackMultiple,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithCallbackMultipleForRasterTile()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithCallbackMultipleForRasterTile,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithIgnoredList()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithIgnoredList,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithoutCallback()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithoutCallback,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerGetTilesWithoutCallbackForRasterTile()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::GetTilesWithoutCallbackForRasterTile,
                 &TestLayerManagerInstance::TileDownloadCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestLayerManagerMetadataRequest()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::MetadataRequest,
                 &TestLayerManagerInstance::MetadataRequestCallback);
}

/* See description in header file. */
void TestLayerManagerInstance::TestMasterClear()
{
    DO_REAL_TEST(TestLayerManagerInstance,
                 &TestLayerManagerInstance::MasterClear, NULL);
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithCallback()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }
        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }

    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetRasterTilesWithCallback()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }
        m_testRaster = true;
        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
        }
    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithCallbackMultiple()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
        }
    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithCallbackMultipleForRasterTile()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }
        m_testRaster = true;
        m_duplicateTileKey = true;
        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }

    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithoutCallback()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithoutCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }
    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithoutCallbackForRasterTile()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        m_testRaster = true;

        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithoutCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
        }
    } while (0);
    if (nbError)
    {
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::MetadataRequest()
{
    NB_Error nbError = Initialize(false);
    CU_ASSERT_EQUAL(nbError, NE_OK);
    if (nbError || !m_layerManager)
    {
        CU_FAIL("Failed to Initiate.");
        return;
    }

    nbError = RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::MasterClear()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }

        nbError = RequestMetadata();
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }

        nbError = NB_ContextSendMasterClear(m_context);

        if (nbError != NE_OK)
        {
            CU_FAIL("NB_ContextSendMasterClear failed");
        }
    } while (0);

    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::MetadataRequestCallback()
{
    //  Make it a little longer to give some time for retry.
    uint32 minZoomLevel = 0;
    uint32 maxZoomLevel = 0;
    NB_Error nbError = m_layerManager->GetBackgroundRasterRange(minZoomLevel,
                                                                maxZoomLevel);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetMinBackgroundRasterZoomLevel failed");
    }

    CU_ASSERT_EQUAL(minZoomLevel, TEST_MIN_BACKGROUND_RASTER_ZOOM_LEVEL);
    CU_ASSERT_EQUAL(maxZoomLevel, TEST_MAX_BACKGROUND_RASTER_ZOOM_LEVEL);

    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::FinishTestPrivate()
{
    Reset();
}


/* See description in header file. */
void TestLayerManagerInstance::StartTileRequestsWithoutCallback()
{
    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;

    NB_Error nbError =
            CreateTestTileKeys(tileKeys,
                               m_testRaster ? TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL : TEST_REQUEST_ZOOM_LEVEL,
                               m_duplicateTileKey ? 1 : REPEAT_TIMES);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        ScheduleFinishTest();
        return;
    }

    nbError = m_layerManager->GetTiles(tileKeys);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
    }
    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::SetPersistentMetadataPath()
{
    do
    {
        NB_Error nbError = Initialize(false);
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        nbError = m_layerManager->SetCachePath(m_cachePath);
        if (nbError != NE_OK)
        {
            CU_FAIL("LayerManager::SetCachePath failed");
            break;
        }

        char testMetadataPath[MAX_FULL_PATH_LENGTH] = {0};
        nsl_strlcpy(testMetadataPath, m_cachePath->c_str(), nsl_strlen(m_cachePath->c_str()) + 1);

        PAL_Error palError = PAL_FileAppendPath(m_pal,
                                                testMetadataPath,
                                                MAX_FULL_PATH_LENGTH,
                                                TEST_PERSISTENT_METADATA_FILENAME);

        if (palError != PAL_Ok)
        {
            nbError = NE_UNEXPECTED;
            CU_FAIL("PAL_FileAppendPath failed");
            break;
        }

        nbError = m_layerManager->SetPersistentMetadataPath(shared_ptr<string>(new string(testMetadataPath)));

        if (nbError != NE_OK)
        {
            CU_FAIL("LayerManager::SetPersistentMetadataPath failed");
        }
    } while (0);

    ScheduleFinishTest();
}

/* See description in header file. */
void TestLayerManagerInstance::StartTileRequestsWithCallback()
{
    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;

    NB_Error nbError =
            CreateTestTileKeys(tileKeys,
                               m_testRaster ? TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL : TEST_REQUEST_ZOOM_LEVEL,
                               m_duplicateTileKey ? REPEAT_TIMES : 1);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        ScheduleFinishTest();
        return;
    }

    // For vector tiles, we are not sure how many tiles would be downloaded without help of
    // LAM tile, so will treat as SUCCESS when at least tiles from one layer is downloaded.
    uint32 requestCount = 1;  // plus one for MAT;
    if (!m_testRaster)
    {
        requestCount ++;   // plus one LAM if not testing raster tile.
        requestCount += 3; // tiles in one layer.
    }
    else
    {
        requestCount ++; // Will map to the same raster tile.
    }

    // Create tile callback and get tiles
    shared_ptr<TestTileCallback> callback(new TestTileCallback(this, requestCount, m_testIgnore));
    shared_ptr<string> type(new string("day"));
    nbError = m_layerManager->GetCommonMaterials(callback, type);

    vector<LayerPtr> ignoredList;
    if (m_testIgnore)
    {
        for (int i = 0; i < (int)m_layers.size(); ++i)
        {
            LayerPtr layer = m_layers[i];
            if (layer)
            {
                uint32 id = layer->GetID();
                if (id >= 0x20 && id <= 0x3F) // Add into ignored list
                {
                    ignoredList.push_back(layer);
                }
            }
        }
    }
    nbError = m_layerManager->GetTiles(tileKeys,
                                       callback,
                                       &ignoredList);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        ScheduleFinishTest();
    }
}

/* See description in header file. */
void TestLayerManagerInstance::GetTilesWithIgnoredList()
{
    NB_Error nbError = Initialize(false);
    do
    {
        CU_ASSERT_EQUAL(nbError, NE_OK);
        if (nbError || !m_layerManager)
        {
            CU_FAIL("Failed to Initiate.");
            break;
        }

        m_testIgnore = true;
        nbError = RequestMetadata(&TestLayerManagerInstance::StartTileRequestsWithCallback);
        if (nbError != NE_OK)
        {
            CU_FAIL("RequestMetadata failed");
            break;
        }

        //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
        vector<TileKeyPtr> tileKeys;
        nbError = CreateTestTileKeys(tileKeys);
        if (nbError != NE_OK)
        {
            CU_FAIL("CreateTestTileKeys failed");
            break;
        }

    } while (0);

    if (nbError)
    {
        ScheduleFinishTest();
    }
}


/* See description in header file. */
void TestLayerManagerInstance::TileDownloadCallback()
{
    ScheduleFinishTest();
}

//  Real test functions ..............................................................

/*! Test initialization of layerManager.

  @return None.
*/
void
TestLayerManagerCreate(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerCreate, CALLBACK_TIMEOUT);
}

/*! Test set cache path for layer manager.

  @return None.
*/
void
TestLayerManagerSetCachePath(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerSetCachePath, CALLBACK_TIMEOUT);
}

/*! Test for setting persistent metadata path of layer manager.

  @return None.
*/
void
TestLayerManagerSetPersistentMetadataPath(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerSetPersistentMetadataPath,
                 CALLBACK_TIMEOUT);
}

/*! Test metadata requesting

  @return None.
*/
void
TestLayerManagerMetadataRequest(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerMetadataRequest,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting tiles without a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithoutCallback(void)
{

    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerGetTilesWithoutCallback,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting cached tiles with callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithCallback(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerGetTilesWithCallback,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting tiles multiple times with a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithCallbackMultiple(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerGetTilesWithCallbackMultiple,
                 CALLBACK_TIMEOUT);
}


/*! Test requesting raster tiles without a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithoutCallbackForRasterTile(void)
{
    PERFORM_TEST(TestLayerManagerInstance,
                 TestLayerManagerGetTilesWithoutCallbackForRasterTile,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting cached raster tiles with callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetRasterTilesWithCallback(void)
{

    PERFORM_TEST(TestLayerManagerInstance,
                 TestLayerManagerGetRasterTilesWithCallback,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting raster tiles multiple times with a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithCallbackMultipleForRasterTile(void)
{
    PERFORM_TEST(TestLayerManagerInstance,
                 TestLayerManagerGetTilesWithCallbackMultipleForRasterTile,
                 CALLBACK_TIMEOUT);
}

/*! Test requesting tiles with ignored list.

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithIgnoredList(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestLayerManagerGetTilesWithIgnoredList,
                 CALLBACK_TIMEOUT);
}


/*! Test master clear.

  @return None.
*/
void
TestMasterClear(void)
{
    PERFORM_TEST(TestLayerManagerInstance, TestMasterClear, CALLBACK_TIMEOUT);
}

//  Interfaces for CUnit ........................................................................

/*! Setup for test.

  @return 0 (succeed all the time.)
*/
int TestLayerManager_SuiteSetup(void)
{
    const char* basePath = NULL;

    basePath = GetBasePath();

    if (basePath)
    {
        //  @todo: Disable this code to test cache.
        //  Remove all test data
        //         PAL_FileRemoveDirectory(dummyPal, GetBasePath(), TRUE);
    }

    return 0;
}

/*! Cleans up the test.

  @return 0 (succeed all the time.)
*/
int TestLayerManager_SuiteCleanup(void)
{
    return 0;
}

/*! Add all test cases into test suite.

  @return None.
*/
// Lambda functions
void TestLayerManager_AddAllTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerCreate",
             TestLayerManagerCreate);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerSetCachePath",
             TestLayerManagerSetCachePath);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerSetPersistentMetadataPath",
             TestLayerManagerSetPersistentMetadataPath);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerMetadataRequest",
             TestLayerManagerMetadataRequest);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerMetadataRequest",
             TestLayerManagerMetadataRequest);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithoutCallback",
             TestLayerManagerGetTilesWithoutCallback);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithCallback",
             TestLayerManagerGetTilesWithCallback);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithCallbackMultiple",
             TestLayerManagerGetTilesWithCallbackMultiple);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithoutCallbackForRasterTile",
             TestLayerManagerGetTilesWithoutCallbackForRasterTile);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetRasterTilesWithCallback",
             TestLayerManagerGetRasterTilesWithCallback);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithCallbackMultipleForRasterTile",
             TestLayerManagerGetTilesWithCallbackMultipleForRasterTile);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestLayerManagerGetTilesWithIgnoredList",
             TestLayerManagerGetTilesWithIgnoredList);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite,
             "TestMasterClear",
             TestMasterClear);
}

/*! @} */
