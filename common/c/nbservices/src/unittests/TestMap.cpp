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

  @file       testmap.cpp

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

//@todo: As we are exposing the LayerManager to public, this test can be moved into system tests.

#include "CUnit.h"

extern "C"
{
#include "testmap.h"
#include "nbcontext.h"
#include "networkutil.h"
#include "platformutil.h"
#include "palfile.h"
#include "nbqalog.h"
#include "palclock.h"
}

#include <stdio.h>
#include <vector>
#include <set>

#include "MetadataConfiguration.h"

#include "contextbasedsingleton.h"
#include "OnboardLayerProvider.h"
#include "OffboardLayerProvider.h"
#include "LocalLayerProvider.h"
#include "UnifiedLayerManager.h"
#include "AggregateLayerProvider.h"
#include "RouteManagerImpl.h"
#include "TrafficManager.h"
#include "PinLayer.h"
#include <sstream>

using namespace nbcommon;
using namespace nbmap;
using namespace std;

#define REPEAT_TIMES       4

//  Local Constants ...........................................................................
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
static const uint32 TEST_MAX_BACKGROUND_RASTER_ZOOM_LEVEL = 13;

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

#define MAX_FULL_PATH_LENGTH 256

/* There are B3D, DVA, DVR and LM3D layers in test tile keys(1 tile for each layer).
   There are 2 files of common materials(MAT) downloaded when
   metadata returns (No test callback) and 1 LAM tile is handled by CCC internally, no
   callback either).
*/
#define REQUEST_COUNT TEST_TILE_KEY_COUNT * TEST_LAYER_COUNT
#define REQUEST_RASTER_TILE_COUNT TEST_TILE_KEY_COUNT * TEST_RASTER_TILE_LAYER_COUNT
//  Local Classes ..............................................................................

class LayerAsyncCallback :
        public nbmap::AsyncCallback<const vector<shared_ptr<nbmap::Layer > >& >
{
public:
    LayerAsyncCallback(UnifiedLayerManager* manager, void* event)
            : m_manager(manager),
              m_event(event)
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

        shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback;

        NB_Error error = m_manager->GetCommonMaterials(callback,
                                                     shared_ptr<string>(new string("day")));
        error = error ? error :
                m_manager->GetCommonMaterials(callback, shared_ptr<string>(new string("night")));
        CU_ASSERT_EQUAL(error, NE_OK);

        m_lastReceivedTime = PAL_ClockGetUnixTime();
        if (m_event)
        {
            SetCallbackCompletedEvent(m_event);
        }
    }

    void Error(NB_Error error)
    {
        CU_FAIL("Error occurred.");
        if (m_event)
        {
            SetCallbackCompletedEvent(m_event);
        }
    }

    bool Progress(int percentage)
    {
        return true;
    }
    nb_unixTime m_lastReceivedTime;

    UnifiedLayerManager* m_manager;
    void* m_event;
};

/*! Test callback object for getting a tile */
class TestTileCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>
{
public:
    /* See definition for description */

    TestTileCallback(void* event,
                     int requestCount);
    virtual ~TestTileCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(TileKeyPtr request,
                         TilePtr response);
    virtual void Error(TileKeyPtr request,
                       NB_Error error);
    virtual bool Progress(int percentage);

    //  Copy constructor and assignment operator are not supported.
    TestTileCallback(const TestTileCallback& callback);
    TestTileCallback& operator=(const TestTileCallback& callback);

    int m_requestCount;         /*!< Count of requests which are downloading in progress */
    void* m_event;
};

/*! Listener for layer manager. */
class LayerManagerListenerInstance : public LayerManagerListener
{
public:
    LayerManagerListenerInstance(void* event);
    virtual ~LayerManagerListenerInstance();
    virtual void RefreshAllTiles();
    virtual void RefreshTilesOfLayer(shared_ptr<Layer> layer);
    virtual void RefreshTiles(const vector<TileKeyPtr>& tileKeys,
                              LayerPtr layer);
    virtual void LayersUpdated(const vector<LayerPtr>& layers);
    virtual void LayersAdded(const vector<LayerPtr>& layers);
    virtual void LayersRemoved(const vector<LayerPtr>& layers);
    virtual void AnimationLayerAdded(shared_ptr<Layer> layer);
    virtual void AnimationLayerUpdated(shared_ptr<Layer> layer,
                                       const vector<uint32>& frameList);
    virtual void AnimationLayerRemoved(shared_ptr<Layer> layer);
    virtual void PinsRemoved(shared_ptr<vector<PinPtr> > pins);
    virtual void EnableMapLegend(bool enabled,
                                 shared_ptr <MapLegendInfo> mapLegend);

private:
    void SetEvent();
    void* m_event;
};

/*! Collection of all instances used for the unit tests */
class TestMapInstance
{
public:
    /*! Default constructor. */
    TestMapInstance();

    /*! Default destructor. */
    virtual ~TestMapInstance();

    /*! Initializes the Test instance.

        @return NB_Error
    */
    NB_Error Initialize(bool testFullCache = false);

    /*! Register a listener into layerManager.

        We can choose to not to register it if we do not care about the listener.

        @return NB_Error
    */
    NB_Error RegisterListener();

    /*! Return layer manager of this instance.

        @return pointer of LayerManager.
    */
    LayerManagerPtr GetLayerManager();

    /*! Return RouteManager in this test instance.

        @return RouteManagerPtr
    */
    RouteManagerPtr GetRouteManager();

    /*! Return TrafficManager in this test instance.

        @return TrafficManagerPtr
    */
    TrafficManagerPtr GetTrafficManager();

    /*! Start metadata request.

        @return NE_OK if succeeded.
    */
    NB_Error RequestMetadata(bool multiple=false);

    /*! Checks the status of cache.

        @return NE_OK if succeeded.
    */
    NB_Error CheckMetadataCache();

    /*! Checks if caching tiles existing.

        @return NE_OK if succeeded.
    */
    NB_Error CheckTileCache();
    NB_Error CheckTileCacheForRasterTile(int specialZoomLevel = TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);

    /*! Checks count of caching tiles if cache is full.

      @todo: I just check count of caching tiles here. Because I cannot simply ensure which
      tiles exists in cache if cache is full. Reasons are:
      1. There are multiple HTTP connections to download tiles and tiles have different
      data size. So I cannot know which tiles are downloaded and saved in cache
      quickly (even if there are priorities for different types of tiles). Current
      cache uses a strategy of 'least recently used removing'.
      2. Application of unittests can run for several times. If caching tiles are not
      removed when unittests stops (normally or broken) at last time, there are
      different tiles existing in cache.
      I think testing case of full cache in 'testcache.cpp' is better.

      @return NE_OK if succeeded.
    */
    NB_Error CheckCachingTileCount(uint32 expectedCount     /*< Expected count of caching tiles */
                                   );

    NB_Error InitiateQaLog();

    void Reset();
    bool IsValid();

    void* m_event;
    NB_Context*   m_context;
    PAL_Instance* m_pal;

    LayerManagerListenerInstance* m_listener;
    shared_ptr<AggregateLayerProvider> m_pAggregateLayerProvider;
    NB_RouteId* m_routeId;
    nb_color    m_routeColor;

    LayerManagerPtr m_layerManager;
    LayerManagerPtr m_secondLayerManager;
};

//  Local Classes Implementation ..................................................................

TestMapInstance::TestMapInstance()
{
    m_context       = NULL;
    m_event         = NULL;
    m_pal           = NULL;
    m_listener      = NULL;
    m_routeId       = NULL;
}

TestMapInstance::~TestMapInstance()
{
    Reset();
}

NB_Error TestMapInstance::Initialize(bool testFullCache)
{
    NB_Error error = NE_OK;
    do
    {
        if (m_pal || m_context || m_event || m_layerManager || m_listener || m_routeId)
        {
            error = NE_NOTINIT;
            break;
        }

        //  Create PAL instance and context
        uint8 createResult = CreatePalAndContext(&(m_pal), &(m_context));
        CU_ASSERT(createResult);
        CU_ASSERT_PTR_NOT_NULL(m_pal);
        CU_ASSERT_PTR_NOT_NULL(m_context);

        if (!createResult)
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
                                     TEST_MAX_CACHING_TILE_COUNT_FULL : TEST_MAX_CACHING_TILE_COUNT;
        nbcommon::Tuple<TYPELIST_3(uint32, uint32, uint32)> otherParameters =
                     nbcommon::MakeTuple(TEST_MAP_NBGM_VERSION,
                                         TEST_MAX_TILE_REQUEST_PER_LAYER,
                                         maxCachingTileCount);
        OffboardLayerProviderPtr provider =
        ContextBasedSingleton<OffboardLayerProvider>::getInstance(m_context,
                                                                  otherParameters);

        maxCachingTileCount = testFullCache ?
        TEST_MAX_CACHING_TILE_COUNT_FULL : TEST_MAX_CACHING_TILE_COUNT;
        otherParameters =
        nbcommon::MakeTuple(TEST_MAP_NBGM_VERSION,
                            TEST_MAX_TILE_REQUEST_PER_LAYER,
                            maxCachingTileCount);
        provider =
        ContextBasedSingleton<OffboardLayerProvider>::getInstance(m_context,
                                                                  otherParameters);


        LayerIdGeneratorPtr idGenerator =
                ContextBasedSingleton<LayerIdGenerator>::getInstance(m_context);

        provider->SetLayerIdGenerator(idGenerator);

        m_pAggregateLayerProvider =
                shared_ptr<AggregateLayerProvider>(new AggregateLayerProvider(m_context,
                                                                              TEST_MAP_NBGM_VERSION));

        m_pAggregateLayerProvider->Initialize();


        //Add the OffboardLayerProvider here.
        m_pAggregateLayerProvider->AddLayerProvider(provider);

        m_layerManager =
                LayerManagerPtr(new UnifiedLayerManager(m_pAggregateLayerProvider,
                                                        m_context));

        shared_ptr<AggregateLayerProvider> aggregateLayerProvider(
            new AggregateLayerProvider(m_context,
                                       TEST_MAP_NBGM_VERSION));
        aggregateLayerProvider->Initialize();
        aggregateLayerProvider->AddLayerProvider(provider);

        m_secondLayerManager =
                LayerManagerPtr(new UnifiedLayerManager(aggregateLayerProvider,
                                                        m_context));

        m_routeId = new NB_RouteId;
        m_routeId->data = new byte[64];
        SPRINTF((char*)(m_routeId->data), "Random test code");
        m_routeId->size = 64;
        m_routeColor = MAKE_NB_COLOR_ALPHA(128, 0, 128, 32);
    } while (0);

    return error;
}

NB_Error TestMapInstance::InitiateQaLog()
{
    NB_Error error = NE_OK;
    //  CSL_QaLogHeaderData header = {0};
    //  CSL_QaLog* log = NULL;
    //  nb_version productVersion = { 4, 9, 15, 10 };
    //  header.productName = "NBServices";
    //  header.productVersion = productVersion;
    //  header.platformId = 0;
    //  header.mobileDirectoryNumber = GetMobileDirectoryNumber(m_pal);
    //  header.mobileInformationNumber = GetMobileInformationNumber(m_pal);

    //  error = CSL_QaLogCreate(m_pal, &header, "TestMetadata.qa", FALSE, &log);
    //  CU_ASSERT_EQUAL(error, NE_OK);

    //  error = NB_ContextSetQaLog(m_context, log);
    //  CU_ASSERT_EQUAL(error, NE_OK);
    return error;
}

void TestMapInstance::Reset()
{

    //  Make sure layerManager is deconstructed before Context and PAL.
    m_pAggregateLayerProvider.reset();
    m_layerManager.reset();
    m_secondLayerManager.reset();

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
    if (m_listener)
    {
        delete m_listener;
        m_listener = NULL;
    }
    if (m_routeId)
    {
        if (m_routeId->data)
        {
            delete m_routeId->data;
        }
        delete m_routeId;
        m_routeId = NULL;
    }
}

NB_Error TestMapInstance::RegisterListener()
{
    NB_Error error = NE_NOTINIT;
    if (m_event && m_layerManager)
    {
        if (!m_listener)
        {
            m_listener = new LayerManagerListenerInstance(m_event);
        }
        error = m_layerManager->RegisterListener(m_listener);
    }
    return error;
}

LayerManagerPtr TestMapInstance::GetLayerManager()
{
    return m_layerManager;
}

RouteManagerPtr TestMapInstance::GetRouteManager()
{
    RouteManagerPtr routeManager;
    if (m_layerManager)
    {
        LocalLayerProviderPtr routeLayerProvider(new LocalLayerProvider(m_context, m_pAggregateLayerProvider.get()));
        m_pAggregateLayerProvider->AddLayerProvider(routeLayerProvider);
        LayerIdGeneratorPtr idGenerator =
                ContextBasedSingleton<LayerIdGenerator>::getInstance(m_context);

        routeManager = shared_ptr<RouteManager>(new RouteManagerImpl(m_context,
                                                                     m_layerManager,
                                                                     routeLayerProvider,
                                                                     idGenerator));
    }
    return routeManager;
}

TrafficManagerPtr TestMapInstance::GetTrafficManager()
{
    TrafficManagerPtr trafficManager;
    //@todo: Enable following lines when TrafficManager is ready.
#if 0
    if (m_layerManager)
    {
        LocalLayerProviderPtr trafficLayerProvider(new LocalLayerProvider(m_context, m_pAggregateLayerProvider.get()));

        LayerIdGeneratorPtr idGenerator =
                ContextBasedSingleton<LayerIdGenerator>::getInstance(m_context);
        trafficManager = TrafficManagerPtr(new TrafficManager(m_context));
        if (trafficManager)
        {
            trafficManager->Initialize(m_layerManager,
                                       PinManagerPtr(),
                                       trafficLayerProvider,
                                       idGenerator,
                                       shared_ptr<string>(new string(TEST_LANGUAGE)));
        }
    }
#endif // End of #if 0

    return trafficManager;
}

bool TestMapInstance::IsValid()
{
    bool result = true;
    if (!m_context || !m_pal || !m_layerManager)
    {
        result = false;
    }
    return result;
}

NB_Error TestMapInstance::RequestMetadata(bool multiple)
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
    nbError = nbError ? nbError : m_secondLayerManager->SetCachePath(cachePath);

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

    shared_ptr<string> metadataPath(new string(testMetadataPath));
    nbError = m_layerManager->SetPersistentMetadataPath(metadataPath);
    nbError = nbError ? nbError : m_secondLayerManager->SetPersistentMetadataPath(metadataPath);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetPersistentMetadataPath failed");
        return nbError;
    }

    shared_ptr<LayerAsyncCallback> layerCallback(
        new LayerAsyncCallback(static_cast<UnifiedLayerManager*>(m_layerManager.get()),
                               m_event));

    if (!layerCallback)
    {
        CU_FAIL("Out of memory when allocated LayerAsyncCallback");
        return NE_NOMEM;
    }

    shared_ptr<string> language(new string("en-US"));

    shared_ptr<MetadataConfiguration> metadataConfiguration = shared_ptr<MetadataConfiguration>(new MetadataConfiguration(language, false, true, false, false, 780, 585));
    nbError = m_layerManager->GetLayers(layerCallback, metadataConfiguration);
    if (multiple)
    {
        layerCallback.reset(
            new LayerAsyncCallback(static_cast<UnifiedLayerManager*>(m_secondLayerManager.get()),
                                   m_event));

        if (!layerCallback)
        {
            CU_FAIL("Out of memory when allocated LayerAsyncCallback");
            return NE_NOMEM;
        }

        nbError = nbError ? nbError :
                  m_secondLayerManager->GetLayers(layerCallback, metadataConfiguration);
    }

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetLayers failed");
        return nbError;
    }

    m_layerManager->SetMetadataRetryTimes(3);
    //  Make it a little longer to give some time for retry.
    if (!WaitForCallbackCompletedEvent(m_event, 600000))
    {
        CU_FAIL("LayerManager::GetLayers timed out!");
        return NE_NET;
    }

    // Server is keep changing.

    // CU_ASSERT_EQUAL(minZoomLevel, TEST_MIN_BACKGROUND_RASTER_ZOOM_LEVEL);
    // CU_ASSERT_EQUAL(maxZoomLevel, TEST_MAX_BACKGROUND_RASTER_ZOOM_LEVEL);
    return NE_OK;
}

NB_Error TestMapInstance::CheckMetadataCache()
{
    if (!IsValid())
    {
        CU_FAIL("Invalid state.");
        return NE_INVAL;
    }
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo fileInformation = {0};
    bool metadataFound = FALSE;

    PAL_FileEnumerateCreate(m_pal, GetBasePath(), TRUE, &fileEnum);
    while(PAL_Ok == PAL_FileEnumerateNext(fileEnum, &fileInformation))
    {
        if (nsl_strstr(fileInformation.filename, TEST_PERSISTENT_METADATA_FILENAME) != NULL)
        {
            metadataFound = TRUE;
            break;
        }
    }

    PAL_FileEnumerateDestroy(fileEnum);

    CU_ASSERT(metadataFound == TRUE);
    if (metadataFound)
    {
        return NE_OK;
    }
    else
    {
        return NE_INVAL;
    }
}

NB_Error TestMapInstance::CheckTileCache()
{
    PAL_Error error = PAL_Ok;
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo fileInformation = {0};
    int layerIndex = 0;
    int tileIndex = 0;
    int startTileIndex = 0;
    int tileCount = 0;
    char tileCachePath[MAX_FULL_PATH_LENGTH] = {0};
    bool tileContains[TEST_LAYER_COUNT][TEST_TILE_KEY_COUNT] = {0};

    nsl_strncpy(tileCachePath, GetBasePath(), nsl_strlen(GetBasePath()) + 1);

    error = PAL_FileAppendPath(m_pal,
                               tileCachePath,
                               MAX_FULL_PATH_LENGTH,
                               TEST_TILE_CACHING_NAME);

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

    int maxTileCount = TEST_MAX_CACHING_TILE_COUNT;

    if (tileCount > maxTileCount)
    {
        CU_FAIL("Count of stored tiles larger than cache limit");
    }


    // Check data for all tile keys for each layer in request stored in cache
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
            PAL_FileEnumerateDestroy(fileEnum);
            CU_FAIL("PAL_FileAppendPath failed");
            return NE_INVAL;
        }

        if (PAL_FileIsDirectory(m_pal, tmpCachePath))
        {
            for (layerIndex = 0; layerIndex < TEST_LAYER_COUNT; layerIndex++)
            {
                if (!nsl_strncmp(fileInformation.filename, TEST_LAYERS_ARRAY[layerIndex], nsl_strlen(TEST_LAYERS_ARRAY[layerIndex])))
                {
                    PAL_FileEnum* tmpFileEnum = NULL;
                    PAL_FileEnumInfo tmpFileInformation = {0};

                    error = PAL_FileEnumerateCreate(m_pal, tmpCachePath, TRUE, &tmpFileEnum);

                    if (error != PAL_Ok)
                    {
                        PAL_FileEnumerateDestroy(fileEnum);
                        CU_FAIL("PAL_FileEnumerateCreate failed");
                        return NE_INVAL;
                    }

                    while(PAL_Ok == PAL_FileEnumerateNext(tmpFileEnum, &tmpFileInformation))
                    {
                        for (tileIndex = 0; tileIndex < TEST_TILE_KEY_COUNT; tileIndex++)
                        {
                            char buffer_x[10] = {0};
                            char buffer_y[10] = {0};

                            nsl_int64tostr(TEST_TILE_KEY_ARRAY[tileIndex][0], buffer_x, 10);
                            nsl_int64tostr(TEST_TILE_KEY_ARRAY[tileIndex][1], buffer_y, 10);

                            if (nsl_strstr(tmpFileInformation.filename, buffer_x) != NULL &&
                                nsl_strstr(tmpFileInformation.filename, buffer_y) != NULL)
                            {
                                tileContains[layerIndex][tileIndex] = TRUE;
                                // break;
                            }

                            tileCount++;
                        }
                    }

                    error = PAL_FileEnumerateDestroy(tmpFileEnum);

                    if (error != PAL_Ok)
                    {
                        PAL_FileEnumerateDestroy(fileEnum);
                        CU_FAIL("PAL_FileEnumerateDestroy failed");
                        return NE_INVAL;
                    }

                    break;
                }
            }
        }
    }

    for (layerIndex = 0; layerIndex < TEST_LAYER_COUNT; layerIndex++)
    {
        char msg[256];
        for (tileIndex = startTileIndex; tileIndex < TEST_TILE_KEY_COUNT; tileIndex++)
        {
            if (!tileContains[layerIndex][tileIndex])
            {
                nsl_memset(msg, 0, 256);
                printf("\tTile key: %u not stored in cache\n", tileIndex);
                CU_FAIL("Check cache failed.");
            }
        }
    }

    PAL_FileEnumerateDestroy(fileEnum);

    if (error != PAL_Ok)
    {
        CU_FAIL("PAL_FileEnumerateDestroy failed");
        return NE_INVAL;
    }

    return NE_OK;
}

NB_Error TestMapInstance::CheckTileCacheForRasterTile(int specialZoomLevel)
{
    PAL_Error error = PAL_Ok;
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo fileInformation = {0};
    int layerIndex = 0;
    int tileIndex = 0;
    int startTileIndex = 0;
    int tileCount = 0;
    char tileCachePath[MAX_FULL_PATH_LENGTH] = {0};
    bool tileContains[TEST_RASTER_TILE_LAYER_COUNT][TEST_TILE_KEY_COUNT] = {0};

    nsl_strlcpy(tileCachePath, GetBasePath(), nsl_strlen(GetBasePath()) + 1);

    error = PAL_FileAppendPath(m_pal,
                               tileCachePath,
                               MAX_FULL_PATH_LENGTH,
                               TEST_TILE_CACHING_NAME);

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

    int maxTileCount = TEST_MAX_CACHING_TILE_COUNT;

    // Check data for all tile keys for each layer in request stored in cache
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
            PAL_FileEnumerateDestroy(fileEnum);
            CU_FAIL("PAL_FileAppendPath failed");
            return NE_INVAL;
        }

        if (PAL_FileIsDirectory(m_pal, tmpCachePath))
        {
            for (layerIndex = 0; layerIndex < TEST_RASTER_TILE_LAYER_COUNT; layerIndex++)
            {
                if (!nsl_strncmp(fileInformation.filename, TEST_RASTER_TILE_LAYERS_ARRAY[layerIndex], nsl_strlen(TEST_RASTER_TILE_LAYERS_ARRAY[layerIndex])))
                {
                    PAL_FileEnum* tmpFileEnum = NULL;
                    PAL_FileEnumInfo tmpFileInformation = {0};

                    error = PAL_FileEnumerateCreate(m_pal, tmpCachePath, TRUE, &tmpFileEnum);

                    if (error != PAL_Ok)
                    {
                        PAL_FileEnumerateDestroy(fileEnum);
                        CU_FAIL("PAL_FileEnumerateCreate failed");
                        return NE_INVAL;
                    }

                    while(PAL_Ok == PAL_FileEnumerateNext(tmpFileEnum, &tmpFileInformation))
                    {
                        for (tileIndex = 0; tileIndex < TEST_TILE_KEY_COUNT; tileIndex++)
                        {
                            char buffer_x[10] = {0};
                            char buffer_y[10] = {0};
                            int tileX = TEST_TILE_KEY_ARRAY[tileIndex][0];
                            int tileY = TEST_TILE_KEY_ARRAY[tileIndex][1];
                            int zoomLevel = TEST_TILE_KEY_ARRAY[tileIndex][2];
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


                            nsl_int64tostr(tileX, buffer_x, 10);
                            nsl_int64tostr(tileY, buffer_y, 10);

                            if (nsl_strstr(tmpFileInformation.filename, buffer_x) != NULL &&
                                nsl_strstr(tmpFileInformation.filename, buffer_y) != NULL)
                            {
                                tileContains[layerIndex][tileIndex] = TRUE;
                            }

                            tileCount++;
                        }
                    }

                    error = PAL_FileEnumerateDestroy(tmpFileEnum);

                    if (error != PAL_Ok)
                    {
                        PAL_FileEnumerateDestroy(fileEnum);
                        CU_FAIL("PAL_FileEnumerateDestroy failed");
                        return NE_INVAL;
                    }

                    break;
                }
            }
        }
    }

    for (layerIndex = 0; layerIndex < TEST_RASTER_TILE_LAYER_COUNT; layerIndex++)
    {
        char msg[256] = {0};
        for (tileIndex = startTileIndex; tileIndex < TEST_TILE_KEY_COUNT; tileIndex++)
        {
            if (!tileContains[layerIndex][tileIndex])
            {
                nsl_memset(msg, 0, 256);
                printf("\tTile key: %u not stored in cache\n", tileIndex);
                CU_FAIL("Tile key: %i not stored in cache" tileIndex);
            }
        }
    }

    PAL_FileEnumerateDestroy(fileEnum);

    if (error != PAL_Ok)
    {
        CU_FAIL("PAL_FileEnumerateDestroy failed");
        return NE_INVAL;
    }

    if (tileCount > maxTileCount)
    {
        CU_FAIL("Count of stored tiles larger than cache limit");
        return NE_UNEXPECTED;
    }

    return NE_OK;
}


NB_Error
TestMapInstance::CheckCachingTileCount(uint32 expectedCount)
{
    //  @todo: Similar code in function CheckTileCache.

    PAL_Error error = PAL_Ok;
    uint32 existingTileCount = 0;
    PAL_FileEnum* fileEnum = NULL;
    PAL_FileEnumInfo fileInformation = {0};
    char tileCachePath[MAX_FULL_PATH_LENGTH] = {0};

    nsl_strncpy(tileCachePath, GetBasePath(), nsl_strlen(GetBasePath()) + 1);
    error = PAL_FileAppendPath(m_pal,
                               tileCachePath,
                               MAX_FULL_PATH_LENGTH,
                               TEST_TILE_CACHING_NAME);
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

    //  Enumerate tiles in caching path.
    while(PAL_FileEnumerateNext(fileEnum, &fileInformation) == PAL_Ok)
    {
        char tmpCachePath[MAX_FULL_PATH_LENGTH] = {0};

        nsl_strncpy(tmpCachePath, tileCachePath, nsl_strlen(tileCachePath) + 1);
        error = PAL_FileAppendPath(m_pal,
                                   tmpCachePath,
                                   MAX_FULL_PATH_LENGTH,
                                   fileInformation.filename);
        if (error != PAL_Ok)
        {
            PAL_FileEnumerateDestroy(fileEnum);
            CU_FAIL("PAL_FileAppendPath failed");
            return NE_INVAL;
        }

        if (PAL_FileIsDirectory(m_pal, tmpCachePath))
        {
            PAL_FileEnum* tmpFileEnum = NULL;
            PAL_FileEnumInfo tmpFileInformation = {0};

            error = PAL_FileEnumerateCreate(m_pal, tmpCachePath, TRUE, &tmpFileEnum);
            if (error != PAL_Ok)
            {
                PAL_FileEnumerateDestroy(fileEnum);
                CU_FAIL("PAL_FileEnumerateCreate failed");
                return NE_INVAL;
            }

            while(PAL_FileEnumerateNext(tmpFileEnum, &tmpFileInformation) == PAL_Ok)
            {
                ++existingTileCount;
            }
            PAL_FileEnumerateDestroy(tmpFileEnum);
        }
    }
    PAL_FileEnumerateDestroy(fileEnum);

    if (expectedCount != existingTileCount)
    {
        CU_FAIL("Count of caching tiles is wrong");
        return NE_UNEXPECTED;
    }

    return NE_OK;
}

//  Implementation of TestTileCallback

/*! TestTileCallback constructor */
TestTileCallback::TestTileCallback(void* event,         /*!< event to wait */
                                   int requestCount     /*!< Count of requests which are downloading in progress */
                                   )
{
    m_event = event;
    m_requestCount = requestCount;
}

/*! TestTileCallback destructor */
TestTileCallback::~TestTileCallback()
{
    //  Nothing to do here.
}

/*! See description in AsyncCallback.h */
void
TestTileCallback::Success(TileKeyPtr request,
                          TilePtr response)
{
    if (!response)
    {
        CU_FAIL("Pointer to responsed tile is NULL");
        SetCallbackCompletedEvent(m_event);
        return;
    }

    shared_ptr<string> dataType   = response->GetDataType();
    shared_ptr<string> contentId  = response->GetContentID();
    DataStreamPtr      dataStream = response->GetData();

    if ((!dataType) || (!contentId) || (!dataStream))
    {
        CU_FAIL("Responsed tile is invalid");
        SetCallbackCompletedEvent(m_event);
        return;
    }

    uint32 oldSize = response->GetSize();
    CU_ASSERT(oldSize > 0);
    CU_ASSERT(oldSize == dataStream->GetDataSize());

    // Executing following functions will cause tile to load data from cache.
    uint32 newSize = response->GetSize();
    dataStream = response->GetData();
    CU_ASSERT(newSize > 0);
    CU_ASSERT(newSize == dataStream->GetDataSize());

    CU_ASSERT(newSize == oldSize);

    //  Do not modify counter if it is common material, because we don't take it into account
    //  when starting test case.
    if ((*dataType) == TILE_TYPE_COMMON_MATERIALS)
    {
        // We are requesting only DMAT.
        if (contentId->find("NMAT") != string::npos)
        {
            CU_FAIL("Material type (night) received!");
        }
    }

    if ((--m_requestCount) == 0)
    {
        //  All requests succeed.
        SetCallbackCompletedEvent(m_event);
    }
}

/*! See description in AsyncCallback.h */
void
TestTileCallback::Error(TileKeyPtr request,
                        NB_Error error)
{
    //  @todo: Ignore the error for now. Because there is no logic for parsing LAM.
}

/*! See description in AsyncCallback.h */
bool
TestTileCallback::Progress(int percentage)
{
    //  Nothing to do here.
    return false;
}

// Implementation of LayerManagerListenerInstance.
// @todo: Fill these implementations if needed.

LayerManagerListenerInstance::LayerManagerListenerInstance(void* event)
{
    m_event = event;
}

LayerManagerListenerInstance::~LayerManagerListenerInstance()
{
}

void LayerManagerListenerInstance:: RefreshAllTiles()
{
    SetEvent();
}
void LayerManagerListenerInstance:: RefreshTilesOfLayer(shared_ptr<Layer> layer)
{
    if (!layer)
    {
        CU_FAIL("Refreshing empty layer!");
        return;
    }
    uint32 layerId = layer->GetID();
    shared_ptr<string> type = layer->GetTileDataType();
    if (!type || type->empty())
    {
        CU_FAIL("Unknown layer type!");
        return;
    }
    printf("\n\t\tTrying to refresh layer, LayerID: %u, LayerType: %s\n",
           layerId, type->c_str());
    SetEvent();
}

void LayerManagerListenerInstance:: RefreshTiles(const vector<TileKeyPtr>& tileKeys,
                                                 LayerPtr layer)
{
    SetEvent();
}

void LayerManagerListenerInstance::LayersUpdated(const vector <LayerPtr>& layers)
{
    SetEvent();
}

void LayerManagerListenerInstance::LayersAdded(const vector <LayerPtr>& layers)
{
    if (layers.empty())
    {
        CU_FAIL("empty layer!");
        return;
    }
    for (size_t i= 0; i < layers.size(); ++i)
    {
        printf("\n\t\tNew layer added:, LayerID: %u, LayerType: %s\n",
               layers[i]->GetID(), layers[i]->GetTileDataType()->c_str());

    }
    SetEvent();
}

void LayerManagerListenerInstance::LayersRemoved(const vector <LayerPtr>& layers)
{
    if (layers.empty())
    {
        CU_FAIL("empty layer!");
        return;
    }
    for (size_t i= 0; i < layers.size(); ++i)
    {
        printf("\n\t\tlayer removed.:, LayerID: %u, LayerType: %s\n",
               layers[i]->GetID(), layers[i]->GetTileDataType()->c_str());

    }
    SetEvent();
}

void
LayerManagerListenerInstance::AnimationLayerAdded(shared_ptr<Layer> layer)
{
}

void
LayerManagerListenerInstance::AnimationLayerUpdated(shared_ptr<Layer> layer,
                                                    const vector<uint32>& frameList)
{
}

void
LayerManagerListenerInstance::AnimationLayerRemoved(shared_ptr<Layer> layer)
{
}

void LayerManagerListenerInstance::PinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    // @todo: Test the notification of pins removed.
}

void LayerManagerListenerInstance::EnableMapLegend(bool enabled,
                                                   shared_ptr <MapLegendInfo> mapLegend)
{
    // @todo: Test map legend.
}

void LayerManagerListenerInstance:: SetEvent()
{
    if (m_event)
    {
        SetCallbackCompletedEvent(m_event);
    }
}

//  Local functions ...............................................................................

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

//  Real test functions .........................................................................

/*! Test initialization of layerManager.

  @return None.
*/
void
TestLayerManagerCreate(void)
{
    TestMapInstance instance;
    NB_Error error = instance.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Test set cache path for layer manager.

  @return None.
*/
void
TestLayerManagerSetCachePath(void)
{
    NB_Error nbError = NE_OK;
    TestMapInstance instance;
    nbError = instance.Initialize();
    CU_ASSERT_EQUAL(nbError, NE_OK);
    if (nbError)
    {
        CU_FAIL("Failed to Initiate.")
                return;
    }

    shared_ptr<string> cachePath(new string(GetBasePath()));

    if (!cachePath)
    {

        CU_FAIL("Out of memory when allocated a string of cache path");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    if (!layerManager)
    {
        CU_FAIL("Failed to get layer manager.");
        return;
    }
    nbError = layerManager->SetCachePath(cachePath);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetCachePath failed");
    }
}

/*! Test for setting persistent metadata path of layer manager.

  @return None.
*/
void
TestLayerManagerSetPersistentMetadataPath(void)
{
    char testMetadataPath[MAX_FULL_PATH_LENGTH] = {0};

    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();
    CU_ASSERT_EQUAL(nbError, NE_OK);
    if (nbError)
    {
        CU_FAIL("Failed to Initiate.")
                return;
    }

    shared_ptr<string> cachePath(new string(GetBasePath()));

    if (!cachePath)
    {

        CU_FAIL("Out of memory when allocated a string of cache path");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    if (!layerManager)
    {
        CU_FAIL("Failed to get layer manager.");
        return;
    }

    nbError = layerManager->SetCachePath(cachePath);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetCachePath failed");
    }

    //  Test setting the persistent metadata path.
    if (cachePath->size() >= MAX_FULL_PATH_LENGTH)
    {
        CU_FAIL("The length of path is out of range");
        return;
    }

    nsl_strlcpy(testMetadataPath, cachePath->c_str(), nsl_strlen(cachePath->c_str()) + 1);

    PAL_Error palError = PAL_FileAppendPath(instance.m_pal,
                                            testMetadataPath,
                                            MAX_FULL_PATH_LENGTH,
                                            TEST_PERSISTENT_METADATA_FILENAME);

    if (palError != PAL_Ok)
    {
        CU_FAIL("PAL_FileAppendPath failed");
    }

    nbError = layerManager->SetPersistentMetadataPath(shared_ptr<string>(new string(testMetadataPath)));

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::SetPersistentMetadataPath failed");
    }
}

/*! Test metadata requesting

  @return None.
*/
void
TestLayerManagerMetadataRequest(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();
    if (nbError)
    {
        CU_FAIL("Failed to initialize test instance.");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    if (!layerManager)
    {
        CU_FAIL("Failed to get layerManager");
        return;
    }

    nbError = instance.RequestMetadata(true);
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
    }
}

/*! Test clearing layers

    @return None
*/
void
TestLayerManagerClearLayers(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();
    if (nbError)
    {
        CU_FAIL("Failed to initialize test instance");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    if (!layerManager)
    {
        CU_FAIL("Failed to get layerManager");
        return;
    }

    // Test requesting metadata.
    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("Metadata request failed");
        return;
    }

    // Clear all layers, ClearLayers() should be used by UnifiedLayerManager internally,
    // here a type cast is used to test.
    UnifiedLayerManager* uLayerManager = static_cast<UnifiedLayerManager*>(layerManager.get());
    if (uLayerManager)
    {
        uLayerManager->ClearLayers();
    }

    // Requesting metadata again.
    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("Metadata request failed");
        return;
    }

    // Test to request tiles after clearing layers.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    // Create tile callback and get tiles
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event, REQUEST_COUNT));
    nbError = layerManager->GetTiles(tileKeys, callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    if (callback->m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(instance.m_event, 600000))
        {
            CU_FAIL("LayerManager::GetTiles timeout");
            return;
        }
    }
}

/*! Request metadata and check the cache.

  @return None.
*/
void
TestLayerManagerMetadataRequestAndCheckCache(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.CheckMetadataCache();
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}

/*! Test requesting tiles without a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithoutCallback(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    instance.InitiateQaLog();

    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
    }
}

/*! Test requesting tiles without a callback and check cache

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithoutCallbackAndCheckCache(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    instance.InitiateQaLog();
    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    // Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    // Check cache folder contains all requested tiles
    nbError = instance.CheckTileCache();

    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}

/*! Test requesting cached tiles with callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetCachedTilesWithCallback(void)
{

    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    // Check cache folder contains all requested tiles
    nbError = instance.CheckTileCache();

    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
        return;
    }

    // Create tile callback and get tiles again
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event,
                                                               REQUEST_COUNT));
    nbError = layerManager->GetCommonMaterials(callback, shared_ptr<string>(new string("day")));
    nbError = layerManager->GetTiles(tileKeys, callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return synchronously because the data is already existing in the cache.
    if (callback->m_requestCount > 0)
    {
        CU_FAIL("LayerManager::GetTiles should return synchronously but don't");
    }
}

/*! Test requesting tiles with a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetNotCachedTilesWithCallback(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    instance.InitiateQaLog();

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event, REQUEST_COUNT));
    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys,
                                     callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return asynchronously because the data not existing in the cache.
    if (callback->m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(instance.m_event, 600000))
        {
            CU_FAIL("LayerManager::GetTiles timeout");
            return;
        }
    }
    else
    {
        CU_FAIL("LayerManager::GetTiles should not return synchronously");
    }

    nbError = instance.CheckTileCache();
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}

/*! Test requesting tiles without a callback and check cache(count of tiles is more than cache limit)

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithoutCallbackFullCache(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize(TRUE);

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    // Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    nbError = instance.CheckCachingTileCount(TEST_MAX_CACHING_TILE_COUNT_FULL);
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCachingTileCount failed");
    }
}

/*! Test requesting tiles multiple times with a callback

  @return None. CUnit Asserts get called on failures.
*/
void
TestLayerManagerGetTilesWithCallbackMultiple(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys,TEST_REQUEST_ZOOM_LEVEL ,REPEAT_TIMES);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    // Duplicated requests are filtered out, so callback should be called REQUEST_COUNT times.
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event, REQUEST_COUNT));
    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys,
                                     callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return asynchronously because the data not existing in the cache.
    if (callback->m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(instance.m_event, 60000))
        {
            CU_FAIL("LayerManager::GetTiles timeout");
            return;
        }
    }
    else
    {
        CU_FAIL("LayerManager::GetTiles should not return synchronously");
    }

    nbError = instance.CheckTileCache();
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}


/*! Test requesting raster tiles without a callback

 @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetTilesWithoutCallbackForRasterTile(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    instance.InitiateQaLog();

    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys, TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
    }
}

/*! Test requesting raster tiles without a callback and check cache

 @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetTilesWithoutCallbackAndCheckCacheForRasterTile(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    instance.InitiateQaLog();
    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys,TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    // Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    // Check cache folder contains all requested tiles
    nbError = instance.CheckTileCacheForRasterTile();

    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}

/*! Test requesting cached raster tiles with callback

 @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetCachedTilesWithCallbackForRasterTile(void)
{

    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys, TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    // Check cache folder contains all requested tiles
    nbError = instance.CheckTileCacheForRasterTile();

    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
        return;
    }

    // Create tile callback and get tiles again
    int number = GetUniqueTileCount(tileKeys);
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event, number));

    nbError = layerManager->GetTiles(tileKeys, callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return synchronously because the data is already existing in the cache.
    if (callback->m_requestCount > 0)
    {
        CU_FAIL("LayerManager::GetTiles should return synchronously but don't");
    }
}

/*! Test requesting raster tiles with a callback

    @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetNotCachedTilesWithCallbackForRasterTile(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    instance.InitiateQaLog();

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys, TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    int number = GetUniqueTileCount(tileKeys);
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event,
                                                               number));
    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys,
                                     callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return asynchronously because the data not existing in the cache.
    if (callback->m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(instance.m_event, 600000))
        {
            CU_FAIL("LayerManager::GetTiles timeout");
            return;
        }
    }
    else
    {
        CU_FAIL("LayerManager::GetTiles should not return synchronously");
    }
}

/*! Test requesting raster tiles without a callback and check cache(count of tiles is more than cache limit)

 @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetTilesWithoutCallbackFullCacheForRasterTile(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize(true);

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys,TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    // Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    nbError = instance.CheckCachingTileCount(TEST_MAX_CACHING_TILE_COUNT_FULL);
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCachingTileCount failed");
    }
}

/*! Test requesting raster tiles multiple times with a callback

 @return None. CUnit Asserts get called on failures.
 */
void
TestLayerManagerGetTilesWithCallbackMultipleForRasterTile(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("Initialize failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;
    nbError = CreateTestTileKeys(tileKeys, TEST_REQUEST_RASTER_TILE_ZOOM_LEVEL ,REPEAT_TIMES);
    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    // Duplicated requests are filtered out, so callback should be called REQUEST_COUNT times.
    int number = GetUniqueTileCount(tileKeys);
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event,
                                                               number));
    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys,
                                     callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests should return asynchronously because the data not existing in the cache.
    if (callback->m_requestCount > 0)
    {
        if (!WaitForCallbackCompletedEvent(instance.m_event, 60000))
        {
            CU_FAIL("LayerManager::GetTiles timeout");
            return;
        }
    }
    else
    {
        CU_FAIL("LayerManager::GetTiles should not return synchronously");
    }

    nbError = instance.CheckTileCacheForRasterTile();
    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
    }
}


/*! Test master clear.

    @return None.
*/
void
TestMasterClear(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = NB_ContextSendMasterClear(instance.m_context);

    if (nbError != NE_OK)
    {
        CU_FAIL("NB_ContextSendMasterClear failed");
    }

    //  After masterclear we request metadata and get tile again to test masterclear
    nbError = instance.RequestMetadata();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata after master clear failed");
        return;
    }

    //  Requesting metadata succeeds. Test to request tiles and check tiles stored in cache.
    vector<TileKeyPtr> tileKeys;

    nbError = CreateTestTileKeys(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("CreateTestTileKeys failed");
        return;
    }

    LayerManagerPtr layerManager = instance.GetLayerManager();
    nbError = layerManager->GetTiles(tileKeys);

    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  Wait some time while data downloaded and stored in cache
    WaitForCallbackCompletedEvent(instance.m_event, 20000);

    // Check cache folder contains all requested tiles
    nbError = instance.CheckTileCache();

    if (nbError != NE_OK)
    {
        CU_FAIL("CheckCache failed");
        return;
    }

    // Create tile callback and get tiles again
    shared_ptr<TestTileCallback> callback(new TestTileCallback(instance.m_event, 8));

    nbError = layerManager->GetTiles(tileKeys, callback);
    if (nbError != NE_OK)
    {
        CU_FAIL("LayerManager::GetTiles failed");
        return;
    }

    //  The requests return synchronously if the data is already existing in the cache.
    if (callback->m_requestCount > 0)
    {
        CU_FAIL("LayerManager::GetTiles should return synchronously but don't");
    }

    //  and then masterclear again
    nbError = NB_ContextSendMasterClear(instance.m_context);

    if (nbError != NE_OK)
    {
        CU_FAIL("NB_ContextSendMasterClear again failed");
    }
}

/*! Test whether RouteManager works.

    @todo: If this test is moved into system test, RouteManager should be created along with
           MapView.
    @return None.
*/
void TestRouteManager(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RegisterListener();
    if (nbError != NE_OK)
    {
        CU_FAIL("Failed to register listener.");
        return;
    }

    RouteManagerPtr routeManager = instance.GetRouteManager();
    if (!routeManager)
    {
        CU_FAIL("Failed to create RouteManager.");
        return;
    }

    // Enable RouteLayer
    RouteInfo info;
    info.routeId = instance.m_routeId;
    info.routeColor = instance.m_routeColor;
    vector<RouteInfo*> routeInfos;
    routeInfos.push_back(&info);

    // Add a layer, should succeed.
    shared_ptr<string> routeLayersId = routeManager->AddRouteLayer(routeInfos);
    if (routeLayersId)
    {
        WaitForCallbackCompletedEvent(instance.m_event, 1000);
    }
    else
    {
        CU_FAIL("Failed to add route layer.");
        return;
    }

    // Add layer with different route id, should succeed.
    routeInfos.push_back(&info);
    routeInfos.push_back(&info);

    shared_ptr<string> routeLayersId2 = routeManager->AddRouteLayer(routeInfos);
    if (routeLayersId2)
    {
        CU_ASSERT(*routeLayersId2 != *routeLayersId)
        WaitForCallbackCompletedEvent(instance.m_event, 1000);
    }
    else
    {
        CU_FAIL("Failed to add route layer.");
        return;
    }

    // Add route layer with same id again, should fail.
    shared_ptr<string> routeLayerId3 = routeManager->AddRouteLayer(routeInfos);
    if (routeLayerId3)
    {
        CU_FAIL("Add route layers with same ID again, should fail, but not");
    }

    // Remove some route layres.
    nbError = routeManager->RemoveRouteLayer(routeLayersId);
    if (nbError == NE_OK)
    {
        WaitForCallbackCompletedEvent(instance.m_event, 1000);
    }
    else
    {
        CU_FAIL("Failed to remove route layer.\n");
    }

    nbError = routeManager->RemoveRouteLayer(routeLayersId2);
    if (nbError == NE_OK)
    {
        WaitForCallbackCompletedEvent(instance.m_event, 1000);
    }
    else
    {
        CU_FAIL("Failed to remove route layer.\n");
    }

    // Remove invalid is, should fail.
    shared_ptr<string> testId(new string("TEST_ROUTE_LAYERS_ID"));
    nbError = routeManager->RemoveRouteLayer(testId);
    CU_ASSERT_NOT_EQUAL(nbError, NE_OK);

    routeManager->RemoveAllRouteLayers();
}

/*! Test whether TrafficManager works.

  @todo: If this test is moved into system test, TrafficManager should be created along with
  MapView.
  @return None.
*/
void TestTrafficManager(void)
{
    TestMapInstance instance;
    NB_Error nbError = instance.Initialize();

    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RequestMetadata();
    if (nbError != NE_OK)
    {
        CU_FAIL("RequestMetadata failed");
        return;
    }

    nbError = instance.RegisterListener();
    if (nbError != NE_OK)
    {
        CU_FAIL("Failed to register listener.");
        return;
    }


    TrafficManagerPtr trafficManager = instance.GetTrafficManager();
    if (!trafficManager)
    {
        CU_FAIL("Failed to create TrafficManager.");
        return;
    }

    // Enable traffic.
    trafficManager->EnableTraffic();
    WaitForCallbackCompletedEvent(instance.m_event, 1000);

    // Disable traffic.
    trafficManager->DisableTraffic();
    WaitForCallbackCompletedEvent(instance.m_event, 1000);
}

//  Interfaces for CUnit ........................................................................

/*! Setup for test.

  @return 0 (succeed all the time.)
*/
int TestMap_SuiteSetup(void)
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
int TestMap_SuiteCleanup(void)
{
    return 0;
}

/*! Add all test cases into test suite.

  @return None.
*/
void TestMap_AddAllTests(CU_pSuite pTestSuite)
{
    CU_add_test(pTestSuite, "TestLayerManagerCreate",
                &TestLayerManagerCreate);
    CU_add_test(pTestSuite, "TestLayerManagerSetCachePath",
                &TestLayerManagerSetCachePath);
    CU_add_test(pTestSuite, "TestLayerManagerSetPersistentMetadataPath",
                &TestLayerManagerSetPersistentMetadataPath);
    CU_add_test(pTestSuite, "TestLayerManagerMetadataRequest",
                &TestLayerManagerMetadataRequest);
    CU_add_test(pTestSuite, "TestLayerManagerClearLayers",
                &TestLayerManagerClearLayers);
    CU_add_test(pTestSuite, "TestLayerManagerMetadataRequestAndCheckCache",
                &TestLayerManagerMetadataRequestAndCheckCache);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithoutCallback",
                &TestLayerManagerGetTilesWithoutCallback);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithoutCallbackAndCheckCache",
                &TestLayerManagerGetTilesWithoutCallbackAndCheckCache);
    CU_add_test(pTestSuite, "TestLayerManagerGetCachedTilesWithCallback",
                &TestLayerManagerGetCachedTilesWithCallback);
    CU_add_test(pTestSuite, "TestLayerManagerGetNotCachedTilesWithCallback",
                &TestLayerManagerGetNotCachedTilesWithCallback);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithoutCallbackFullCache",
                &TestLayerManagerGetTilesWithoutCallbackFullCache);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithCallbackMultiple",
                &TestLayerManagerGetTilesWithCallbackMultiple);
    CU_add_test(pTestSuite, "TestLayerManagerMetadataRequest",
                &TestLayerManagerMetadataRequest);
    CU_add_test(pTestSuite, "TestLayerManagerMetadataRequestAndCheckCache",
                &TestLayerManagerMetadataRequestAndCheckCache);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithoutCallbackForRasterTile",
                &TestLayerManagerGetTilesWithoutCallbackForRasterTile);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithoutCallbackAndCheckCacheForRasterTile",
                &TestLayerManagerGetTilesWithoutCallbackAndCheckCacheForRasterTile);
    CU_add_test(pTestSuite, "TestLayerManagerGetCachedTilesWithCallbackForRasterTile",
                &TestLayerManagerGetCachedTilesWithCallbackForRasterTile);
    CU_add_test(pTestSuite, "TestLayerManagerGetNotCachedTilesWithCallbackForRasterTile",
                &TestLayerManagerGetNotCachedTilesWithCallbackForRasterTile);
    CU_add_test(pTestSuite, "TestLayerManagerGetTilesWithCallbackMultipleForRasterTile",
                &TestLayerManagerGetTilesWithCallbackMultipleForRasterTile);
    CU_add_test(pTestSuite, "TestMasterClear", &TestMasterClear);
    CU_add_test(pTestSuite, "TestRouteManager", &TestRouteManager);
    CU_add_test(pTestSuite, "TestTrafficManager", &TestTrafficManager);

}

/*! @} */
