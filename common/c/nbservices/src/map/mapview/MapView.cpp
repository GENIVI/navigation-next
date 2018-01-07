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
    @file     MapView.cpp
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

extern "C"
{
#include "transformutility.h"
}

#include "NBGMViewController.h"
#include "MapView.h"
#include "MapNativeView.h"
#include "nbcontextprotected.h"
#include "NBUIConfig.h"
#include "nbspatial.h"
#include "palfile.h"
#include "commoncomponentconfiguration.h"
#include "PinBubbleResolver.h"
#include "PinManagerImpl.h"
#include "CustomLayerManagerImpl.h"
#include "MapViewTask.h"
#include "LayerFunctors.h"
#include "StringUtility.h"
#include <cmath>
#include <sstream>
#include <iterator>
#include <math.h>
#include <bitset>
#include "palmath.h"
#include "DebugFile.h"
#include "MapLegendInfo.h"
#include "paldisplay.h"
#include "DAMUtility.h"
#include "DAMConfiguration.h"
#include "MapServicesConfigurationImpl.h"
#include "MetadataConfiguration.h"
#include "nbsolarcalculator.h"
#include "paldebuglog.h"
#include "paltestlog.h"
#include "HybridManager.h"
#include "palclock.h"
#include <QDebug>
#include "pallog.h"

// Enable this define to see number of returned tiles from layermanager. Should be disabled for official releases
#define DEBUG_OUTPUT_TILE_COUNT

// Enable to have the map continuously render. This is useful for performance testing. Should be disabled for production builds.
//#define ENABLE_CONTINUOUS_RENDERING

//some configurational constants for gesture handling

#define MAX_PATH_LENGTH          512

/*! Min velocity of heading valid */
#define MIN_VELOCITY_OF_HEADING_VALID 1.527777777   // 5.5 km/h

// const for relative zoom level and to calculate relative camera height
#define DEFAULT_ZOOM_LEVEL 17

// #define LOG_TILE_REQUEST

using namespace nbcommon;
using namespace nbmap;

// @todo: Should we take this from a configuration?
//const int TILE_SIZE = 256;

// Filename of pin material config information
//const char PIN_MATERIAL_CONFIG_FILENAME[] = "pin_materials.xml";
//const char PIN_MATERIAL[]                 = "NBM_PIN_MATERIALS";

// MapKit3D internal folder
const char MAPKIT3D_FOLDER_NAME[]         = "mapKit3Dcontent";
const char METADATA_FOLDER_NAME[]         = "metadata";
// Traffic layer type name
const char TRAFFIC_LAYER_NAME[]           = "traffic";
// DVA layer type name
const char DVA_LAYER_NAME[]               = "DVA";
// Custom Pin Prefix
const char CUSTOM_MARKER_PREFIX[] = "CUSTOM_PIN/";


static const uint32 INVALID_LAYER_ID = 0xFFFFFFFF;  // This can't be '-1' since it is unsigned!

/*! A flag to indicate MapView is used at the first time.
  There is a requirement that asks us to record the time when the last tile to fill the
  first frame is rendered at the start up time of app. This flag is set to true when the
  first MapView instance is created, and set to false for all other instances.
*/
static bool firstUse = true;
static set<TileKey> requestedTiles;

#define WORLD_MAX_LON 180.0
#define WORLD_MIN_LON -180.0
#define WORLD_MAX_LAT 85.0
#define WORLD_MIN_LAT -85.0

static const uint32 NIGHT_MODE_CHECK_INTERVAL = 60 * 60 * 1000; // The interval to check night mode is one hour
static const uint32 FRAME_CHECK_INTERVAL = 300;
static const uint32 RENDER_INTERVAL = 18;
//static const uint32 MILLISECOND_PER_SECOND = 1000; // The interval to check night mode is one hour
static const uint32 DEBUG_UI_UPDATE_INTERVAL = 100;

/*! Status used to identify if rendering is allowed */
enum RenderingAllowedStatus
{
    RAS_Not_Allowed = 0,
    RAS_Common_Materials_Success = 0x1,
    RAS_Map_View_Initialized_Complete = 0x2,
    RAS_Allowed = RAS_Common_Materials_Success |
                  RAS_Map_View_Initialized_Complete
};

class ScreenRectangle : public Rectangle
{
public:
    ScreenRectangle(int screenWidth, int screenHeight)
        : Rectangle(0, 0, screenWidth, screenHeight)
    {}

    Point ViewCenter() const
    {
        /*
            when width or height is odd number, returns rounded number to a half,
            and when width or height is even number, returns (half -1).
        */
        int xPosition = (width > 0) ? ((width & 0x1) ? (width / 2) : (width / 2 - 1)) : 0;
        int yPosition = (height > 0) ? ((height & 0x1) ? (height / 2) : (height / 2 - 1)) : 0;

        return Point(xPosition, yPosition);
    }
};

class CancelRequestFunctor
{
public:
    CancelRequestFunctor(uint32 zoomLevel)
            : m_zoomLevel(zoomLevel)
    {
    }
    virtual ~CancelRequestFunctor() {}
    void operator() (const LayerPtr& layer) const
    {
        if (layer && (m_zoomLevel < layer->GetMinZoom() || m_zoomLevel > layer->GetMaxZoom()))
        {
            layer->RemoveAllTiles();
        }
    }

private:
    uint32 m_zoomLevel;
};

/*! Functor to organize tiles based on their TileKey. /*/
class TilesToReloadInserter
{
public:
    TilesToReloadInserter(map<TileKey, TilePtr>& tiles)
            : m_tiles(tiles) {}
    virtual ~TilesToReloadInserter(){}
    void operator() (const TilePtr& tile)
    {
        if (tile && tile->GetTileKey())
        {
            // Ignore return value.
            m_tiles.insert(make_pair(*(tile->GetTileKey()), tile));
        }
    }

private:
    map<TileKey, TilePtr>& m_tiles;
};

/*! Functor to remove animation layer. /*/
class AnimationLayerRemover
{
public:
    AnimationLayerRemover(MapView *view)
            : m_pMapView(view) {}
    virtual ~AnimationLayerRemover(){}

    void operator() (const map<LayerPtr, vector<uint32> >::value_type& it) const
    {
        if (it.first)
        {
            m_pMapView->AnimationLayerRemoved(it.first);
        }
    }

private:
    MapView *m_pMapView;
};

/*! Functor to take out value part of Map and store into a vector. */
template <class T1, class T2>
class MappedValuesToVectorFunctor
{
public:
    MappedValuesToVectorFunctor(vector<T2>& container)
            : m_container(container) {}
    virtual ~MappedValuesToVectorFunctor() {}

    void operator() (const pair<T1, T2>& iter)
    {
        m_container.push_back(iter.second);
    }

private:
    vector<T2>& m_container;
};

// MapViewRenderListener ......................................................................
/*! This class is used to calculate the bubble postion after frame rendering for now */
class MapViewRenderListener : public RenderListener
{
public:
    MapViewRenderListener(NBGMViewController* nbgmViewController, MapView* mapView)
 :  m_nbgmViewController(nbgmViewController),
    m_internalListener(NULL),
    m_mapView(mapView){}

    virtual ~MapViewRenderListener() { OnDiscard(); }
    // warppe the listerner passed in
    virtual void SetInternalListerner(RenderListener* listener);
    // called before frame rendering
    virtual void OnRenderFrameBegin();
    // called after frame rendering
    virtual void OnRenderFrameEnd();
    // called if useless
    virtual void OnDiscard();

private:
    NBGMViewController* m_nbgmViewController;
    RenderListener* m_internalListener;
    MapView* m_mapView;
};

void
MapViewRenderListener::SetInternalListerner(RenderListener* listener)
{
    nsl_assert(listener != this);
    if(listener != m_internalListener)
    {
        if(m_internalListener)
        {
            m_internalListener->OnDiscard();
        }
        m_internalListener = listener;
    }
}

void
MapViewRenderListener::OnRenderFrameBegin()
{
    if(m_internalListener)
    {
        m_internalListener->OnRenderFrameBegin();
    }
}

void
MapViewRenderListener::OnRenderFrameEnd()
{
    if(m_nbgmViewController)
    {
        m_nbgmViewController->OnRenderFrameEnd();
    }
    if(m_internalListener)
    {
        m_internalListener->OnRenderFrameEnd();
    }
    if (m_mapView)
    {
        m_mapView->UpdateDebugUI();
    }
}

void
MapViewRenderListener::OnDiscard()
{
    if(m_internalListener)
    {
        m_internalListener->OnDiscard();
        m_internalListener = NULL;
    }
}

// MapViewInitialize .......................................................................
MapViewInitialize::MapViewInitialize()
 :  m_lock(NULL),
    m_commonMaterialRequestCount(0),
    m_renderingAllowed(static_cast<int>(RAS_Not_Allowed)),
    m_done(false),
    m_shuttingDown(false)
{
}

MapViewInitialize::~MapViewInitialize()
{
}

/* See header for description */
void
MapViewInitialize::RequestCommonMaterialsIfNecessary(MapView* pMapView)
{
    Lock lock(m_lock);

    /* Trigger the common materials request if this function is called
       greater than twice, this function is called in the functions
       MapView::LayerRequestSuccess and MapView::RenderThread_Initialize.
       The common materials request may fail or the metadata may be
       updated (see the function MapView::LayersUpdated for details),
       so we need to check the request count greater than 2.
    */
    if (++m_commonMaterialRequestCount >= 2)
    {
        // Request common materials.
        switch (pMapView->m_nightMode)
        {
            case MVNM_DAY:
                pMapView->m_isNight = false;
                pMapView->m_eventQueue->AddTask(new MapViewTaskSetTheme(pMapView, &MapView::CCC_SetTheme, MVT_DAY));
                break;
            case MVNM_NIGHT:
                pMapView->m_isNight = true;
                pMapView->m_eventQueue->AddTask(new MapViewTaskSetTheme(pMapView, &MapView::CCC_SetTheme, MVT_NIGHT));
                break;
            case MVNM_AUTO:
                pMapView->m_UITaskQueue->AddTask(new MapViewTask(pMapView, &MapView::UI_CalculateNightMode));
                break;
            default:
                break;
        }
    }
}

/* See header for description */
bool
MapViewInitialize::CommonMaterialsReadyToRequest() const
{
    Lock lock(m_lock);
    return (m_commonMaterialRequestCount >= 2);
}

/* See header for description */
void
MapViewInitialize::SetRenderingAllowed(int status)
{
    m_renderingAllowed |= status;
}

/* See header for description */
bool
MapViewInitialize::IsRenderingAllowed() const
{
    return (m_renderingAllowed == static_cast<int>(RAS_Allowed));
}

/* See header for description */
void
MapViewInitialize::ResetRenderingState()
{
    m_renderingAllowed &= (~RAS_Common_Materials_Success);
}

/* See header for description */
void
MapViewInitialize::SetDone()
{
    m_done = true;
}

/* See header for description */
bool
MapViewInitialize::IsDone() const
{
    return m_done;
}

/* See header for description */
void
MapViewInitialize::SetShuttingDown()
{
    m_shuttingDown = true;
}

/* See header for description */
bool
MapViewInitialize::IsShuttingDown() const
{
    return m_shuttingDown;
}


// Map View ............................................................................................................

MapView::MapView(MapServicesConfigurationImpl* serviceConfiguration)
    :   m_initialize(),
        m_nbContext(NULL),
        m_palInstance(NULL),
        m_workFolder(),
        m_resourceFolder(),
        m_defaultLatitude(0.0),
        m_defaultLongitude(0.0),
        m_defaultTiltAngle(0.0),
        m_layerManager(),
        m_nativeView(NULL),
        m_lastZoom(0),
        m_frame(),
        m_showRoute(false),
        m_shouldInitRenderer(true),
        m_nbgmViewController(NULL),
        m_pTileCallback(),
        m_pCommonMaterialCallback(),
        m_pLayerCallback(),
        m_pRenderingQueue(),
        m_eventQueue(),
        m_UITaskQueue(),
        m_pinCushion(),
        m_pinManager(),
        m_damUtility(),
        m_maxRasterZoomLevel(0),
        m_enabledDebugInfo(false),
        m_nightMode(MVNM_AUTO),
        m_isNight(false),
        m_lastIsNight(false),
        m_isSatellite(false),
        m_needsRendering(false),
        m_background(false),
        m_viewSize(),
        m_gpsMode(NGM_INVALID),
        m_avatarMode(MVAM_MAP),
        m_layers(),
        m_pListener(NULL),
        m_info(),
        m_gpsFileName(),
        m_lastGpsMode(NGM_INVALID),
        m_droppedPinId(),
        m_CCCAnimationFrameLists(),
        m_currentTheme(MVT_MAX),
        m_trafficLayerAvailable(false),
        m_trafficLayerId(INVALID_LAYER_ID),
        m_minZoomOfTrafficLayer(0),
        m_lastTrafficTip(TTT_None),
        m_viewSizeWidth(0),
        m_viewSizeHeight(0),
        m_firstTilesLoaded(false),
        m_preferredLanguageCode(0),
        m_avatarRenderOneTimeOutofScreen(false),
        m_showLabelDebugInfo(false),
        m_serviceConfiguration(serviceConfiguration),
        m_pScreenshotInProgress(),
        m_frameUpdated(true),
        m_getTileWithFrame(true),
        m_defaultRenderListener(NULL),
        m_CombinedAtomicTaskExecuter(NULL),
        m_pPrefetchProcessor(),
        m_isNeedFrameUpdate(false),
        m_maxZoomOfAnimationLayer(0),
        m_lastAnimationTip(ATT_None),
        m_lastAnimationDisplay(false),
        m_lastAnimationTimestamp(0),
        m_cameraPrevLat(0),
        m_cameraPrevLon(0),
        m_cameraPrevHeading(0),
        m_cameraPrevTilt(0),
        m_cameraPrevZoomLevel(0),
        m_lastDebugUiUpdateTime(0),
        m_cameraUpdateGuard(false),
        m_isInAtomic(false),
        m_gestureHandler(NULL),
        m_cameraTimestamp(1),
        m_pinHandler(NULL),
        m_fontMagnifierLevel(MVFML_LARGE),
        m_refreshTilesAfterCommonMaterialUpdated(false),
        m_updatingLayers(false),
        m_renderPending(true)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::MapView", this);
    m_pTileCallback  = shared_ptr<MapViewTileCallback>(new MapViewTileCallback);
    m_pLayerCallback = shared_ptr<MapViewLayerCallback>(new MapViewLayerCallback);
    m_initialize.m_isValid = shared_ptr<bool> (new bool(true));
    m_pCommonMaterialCallback = shared_ptr<MapViewCommonMaterialRequestCallback>(new MapViewCommonMaterialRequestCallback);
    nsl_memset(&m_lastGpsLocation, 0, sizeof(m_lastGpsLocation));

    if (m_serviceConfiguration)
    {
        m_serviceConfiguration->RegisterConfigurationListener(this);
    }

    m_mapViewCamera = new MapViewCamera(this);
    m_gestureHandler = new MapViewGestureHandler();
}

MapView::~MapView()
{
    // Don't do anything here. All cleanup is done in MapView::Destroy()
    if (m_serviceConfiguration)
    {
        m_serviceConfiguration->UnregisterConfigurationListener(this);
    }

    if (m_gestureHandler)
    {
        delete m_gestureHandler;
    }

    if (m_mapViewCamera)
    {
        delete m_mapViewCamera;
    }

    if (m_pinHandler)
    {
        delete m_pinHandler;
    }
}

// Initialize the MapView instance
NB_Error
MapView::Initialize(NB_Context* nbContext,
                    shared_ptr<AsyncCallback<void*> > initializeCallback,
                    shared_ptr<AsyncCallback<void*> > destroyCallback,
                    const char* workFolder,
                    const char* resourceFolder,
                    double defaultLatitude,
                    double defaultLongitude,
                    double defaultTiltAngle,
                    shared_ptr<LayerManager> layerManager,
                    shared_ptr<PinCushion> pinCushion,
                    shared_ptr<PinManager> pinManager,
                    shared_ptr<CustomLayerManager> customLayerManager,
                    uint8 preferredLanguageCode,
                    int zorder_level,
                    void* mapViewContext,
                    shared_ptr<MetadataConfiguration> metadataConfig,
                    MapViewListener* pListener,
                    MapViewFontMagnifierLevel fontMagnifierLevel,
                    float scaleFactor)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::Initialize", this);
    // This has to be called in the context of the CCC thread!
    NB_ASSERT_CCC_THREAD(nbContext);

    m_nbContext = nbContext;
    m_palInstance = NB_ContextGetPal(nbContext);
    m_layerManager = layerManager;
    m_pinCushion = pinCushion;
    m_pinManager = pinManager;
    m_customLayerManager = customLayerManager;
    m_preferredLanguageCode = preferredLanguageCode;
    m_pMetadataConfig = metadataConfig;
    m_pListener = pListener;

    m_damUtility = DAMConfiguration::GetDAMUtility(nbContext, layerManager);

    m_defaultLatitude   = defaultLatitude;
    m_defaultLongitude  = defaultLongitude;
    m_defaultTiltAngle  = defaultTiltAngle;

    m_initialize.m_initializeCallback = initializeCallback;
    m_initialize.m_destroyCallback = destroyCallback;

    m_fontMagnifierLevel = fontMagnifierLevel;
    m_mapViewCamera->SetScaleFactor(scaleFactor);
    m_gestureHandler->Initiallize(this, m_mapViewCamera, &m_viewSizeWidth, &m_viewSizeHeight);
    if ((!workFolder) || (!resourceFolder))
    {
        return NE_BADDATA;
    }

    m_workFolder = workFolder;
    m_resourceFolder = resourceFolder;

    // Get CCC and UI task queue
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(m_palInstance));
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(m_palInstance));
    m_pRenderingQueue = shared_ptr<RenderTaskQueue>(new RenderTaskQueue(m_palInstance));

    /* Initialize CCC */
    NB_Error result = CCC_Initialize();
    if (result != NE_OK)
    {
        return result;
    }

    m_mapViewContext = mapViewContext;
    m_zorder = zorder_level;
    m_CombinedAtomicTaskExecuter = new CombinedAtomicTaskExecuter(this, m_palInstance);
    /*
        Initialize NativeView in UI thread. This will trigger NBGM initialize. Once NBGM is initialized, it will call the
        callback to the app level to indicate that the initialization is complete.
     */
    if (m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_InitializeNativeView)) != PAL_Ok)
    {
        return NE_NOTINIT;
    }

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"MAPVIEW_INIT_CALLED");
    m_pPrefetchProcessor = shared_ptr<PrefetchProcessor>(new PrefetchProcessor());
    PAL_TimerSet(m_palInstance, FRAME_CHECK_INTERVAL, OnFrameCheckTimerCallback, this);
    PAL_TimerSet(m_palInstance, RENDER_INTERVAL, OnRenderTimerCallback, this);
    return NE_OK;
}

// MapViewInterface functions ..........................................................................................

/* See MapViewInterface.h for description */
NB_Error
MapView::Destroy()
{
    // This function has to be called from the UI thread!

    //need to remove listener when mapview destroyed.
    m_pListener = NULL;

    if(m_CombinedAtomicTaskExecuter)
    {
        m_CombinedAtomicTaskExecuter->ClearAllTask();
        delete m_CombinedAtomicTaskExecuter;
        m_CombinedAtomicTaskExecuter = NULL;
    }

    // Prevent any further task handling
    m_initialize.SetShuttingDown();

    /*
        Don't call RemoveAllTasks() on the task queues. I think that could mess up the other map instances.
     */

    /*
        We perform the MapView shutdown asynchronously. The CCC shutdown triggers the render thread shutdown, which will
        then trigger the UI shutdown. After that the MapView instance will be deleted. We trigger the destroy callback
        once MapView is fully destroyed.
     */

    PAL_TimerCancel(m_palInstance, NightModeTimerCallback, this);
    PAL_TimerCancel(m_palInstance, OnFrameCheckTimerCallback, this);
    PAL_TimerCancel(m_palInstance, OnRenderTimerCallback, this);

    if ((!m_eventQueue) || (!m_pRenderingQueue) || (!m_UITaskQueue))
    {
        return NE_NOTINIT;
    }

    m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_Shutdown));

    m_pMetadataConfig.reset();
    m_pPrefetchProcessor.reset();

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"MAPVIEW_DESTROY_CALLED");

    return NE_OK;
}

/* See MapViewInterface.h for description */
void
MapView::InitializeRenderer()
{
    if (m_initialize.IsShuttingDown() || (!m_pRenderingQueue))
    {
        /* Do nothing if the map view is destroying, the map view
           initialized process should also initialize the renderer.
        */
        return;
    }

    // Check if this function is called in the render thread.
    if (m_pRenderingQueue->IsRuningThread())
    {
        /* Initialize the renderer directly in the render thread.

           TRICKY: Should not call the function
                   RenderThread_InitializeRenderer here, because the
                   flag 'm_shouldInitRenderer' needs to be set.
         */
        RenderThread_InitializeRendererWithTask(NULL);
    }
    else
    {
        // Switch to the render thread to initialize the renderer.
        AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_InitializeRendererWithTask));
    }
}

/* See MapViewInterface.h for description */
void
MapView::DestroyRenderer()
{
    if (m_initialize.IsShuttingDown() || (!m_pRenderingQueue))
    {
        /* User should not call this function if the function
           Destroy has already been called before.
        */
        return;
    }

    m_nativeView->DestroySurface();
    // Check if this function is called in the render thread.
    if (m_pRenderingQueue->IsRuningThread())
    {
        /* Destroy the renderer directly in the render thread.

           TRICKY: Should not call the function
                   RenderThread_DestroyRenderer here, because the
                   flag 'm_shouldInitRenderer' needs to be set.
         */
        RenderThread_DestroyRendererWithTask(NULL);
    }
    else
    {
        // Create the event to wait for destroying the renderer.
        PAL_Event* destroyingEvent = NULL;
        if (PAL_EventCreate(m_palInstance, &destroyingEvent) != PAL_Ok)
        {
            return;
        }

        // Switch to the render thread to destroy the renderer.
        AddRenderThreadTask(new RenderTaskDestroyRenderer(this, &MapView::RenderThread_DestroyRendererWithTask, destroyingEvent));

        // Wait for the event to destroy the renderer synchronously.
        PAL_EventWaitForEvent(destroyingEvent);
        PAL_EventDestroy(destroyingEvent);
        destroyingEvent = NULL;
    }
}

/* See MapViewInterface.h for description */
NB_Error
MapView::ShowView()
{
    if ((! m_nativeView) || (m_initialize.IsShuttingDown()))
    {
        return NE_INST;
    }

    // Forward to native view
    return m_nativeView->ShowView();
}

/* See MapViewInterface.h for description */
void*
MapView::GetNativeContext()
{
    if (m_initialize.IsShuttingDown() || (! m_nativeView))
    {
        return NULL;
    }

    return m_nativeView->GetNativeContext();
}

/* See MapViewInterface.h for description */
void
MapView::SetGpsMode(NB_GpsMode gpsMode)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    m_contextUI.lastGpsMode = gpsMode;
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"MAPVIEW_SETGPSMODE: GPSmode:%d", (int)gpsMode);

    AddRenderThreadTask(new RenderTaskSetGpsMode(this, &MapView::RenderThread_SetGpsMode, gpsMode));
    m_nativeView->SetGpsMode(gpsMode);
}

NB_GpsMode
MapView::GetGpsMode()const
{
    return m_contextUI.lastGpsMode;
}

/* See MapViewInterface.h for description */
void
MapView::SetAvatarState(bool /*headingValid*/)
{
    //if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    //{
    //    return;
    //}

    //AddRenderThreadTask(new RenderTaskSetAvatarState(this, &MapView::RenderThread_SetAvatarState, headingValid));
}

void
MapView::SetAvatarMode(MapViewAvatarMode mode)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetAvatarMode(this, &MapView::RenderThread_SetAvatarMode, mode));
}

/* See MapViewInterface.h for description */
void
MapView::SetAvatarLocation(const NB_GpsLocation& gpsLocation)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    // This function should be called in UI thread.
    uint32 gpsValid = gpsLocation.valid;
    // Check if the coordinate is valid.
    if (((gpsValid & NGV_Latitude) != 0) && ((gpsValid & NGV_Longitude) != 0))
    {
        m_contextUI.lastGpsLat = gpsLocation.latitude;
        m_contextUI.lastGpsLon = gpsLocation.longitude;
    }

    AddRenderThreadTask(new RenderTaskSetAvatarLocation(this, &MapView::RenderThread_SetAvatarLocation, gpsLocation));
}

void
MapView::SetCustomAvatar(shared_ptr<nbcommon::DataStream> directionalAvatar, shared_ptr<nbcommon::DataStream> directionlessAvatar,
                         int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                         int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetCustomAvatar(this,
                                                                &MapView::RenderThread_SetCustomAvatar,
                                                                directionalAvatar,
                                                                directionlessAvatar,
                                                                directionalCalloutOffsetX,
                                                                directionalCalloutOffsetY,
                                                                directionlessCalloutOffsetX,
                                                                directionlessCalloutOffsetY));
}

void
MapView::SetCustomAvatar(shared_ptr<std::string> directionalAvatarPath, shared_ptr<std::string> directionlessAvatarPath,
                         int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                         int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetCustomAvatar(this,
                                                                &MapView::RenderThread_SetCustomAvatar,
                                                                directionalAvatarPath,
                                                                directionlessAvatarPath,
                                                                directionalCalloutOffsetX,
                                                                directionalCalloutOffsetY,
                                                                directionlessCalloutOffsetX,
                                                                directionlessCalloutOffsetY));
}

void
MapView::SetCustomAvatarHaloStyle(shared_ptr<GeographyMaterial> haloStyle)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetCustomAvatarHaloStyle(this,
                                                                         &MapView::RenderThread_SetCustomAvatarHaloStyle,
                                                                         haloStyle));
}

/* See MapViewInterface.h for description */
void
MapView::SetBackgroundSynchronized(bool background)
{
    if(m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_mapViewCamera)
    {
        return;
    }

    //@TODO: need to ensure this event is processed when destroying MapView instance.
    PAL_Event* setBackgroundEvent = NULL;
    if (PAL_EventCreate(m_palInstance, &setBackgroundEvent) != PAL_Ok)
    {
        return;
    }

    // Switch to the render thread to get the background.
    MapViewTask* task = new RenderTaskSetBackground(this,
                                                    &MapView::RenderThread_SetBackground,
                                                    background,
                                                    setBackgroundEvent);
    AddRenderThreadTask(task);

    // Wait for the event to set background synchronously.
    PAL_EventWaitForEvent(setBackgroundEvent);
    PAL_EventDestroy(setBackgroundEvent);
}

/* See MapViewInterface.h for description */
void
MapView::SetBackground(bool background)
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }
    m_nbgmViewController->SetBackground(background);
    if(!background)
    {
        m_renderPending = true;
    }
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetViewSettings(MapViewSettings viewSetting, bool flag)
{
    return SetViewSettings(viewSetting, flag, 0, 0);
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetViewSettings(MapViewSettings viewSetting, bool flag, float portraitFOV, float landscapeFOV)
{
    /*
        This function has to be called in the UI thread
     */

    qDebug() << "Enter MapView::SetViewSettings";
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return NE_NOTINIT;
    }

    /*
        :TRICKY:
        ShowDebugView() has to be called in the UI thread. All other functions have to be called
        in the render thread!
     */
    qDebug() << "MapView::SetViewSettings is called from UI thread.";

    if (viewSetting == MVS_SHOW_DEBUG_VIEW)
    {
        ShowDebugView(flag);
        return NE_OK;
    }

    AddRenderThreadTask(new RenderTaskSetViewSettings(this, &MapView::RenderThread_SetViewSettings,
                                                                viewSetting, flag, portraitFOV, landscapeFOV));
    return NE_OK;
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetDisplayScreen(int screenIndex)
{
    /*
        This function has to be called in the UI thread
     */
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nativeView))
    {
        return NE_NOTINIT;
    }

    m_nativeView->SetDisplayScreen(screenIndex);
    return NE_OK;
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetCameraSettings(const MapViewCameraParameter& cameraParameter, bool animated, uint32 duration, MapViewCameraAnimationAccelertaionType type)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return NE_NOTINIT;
    }

    if(m_contextUI.gestrueProcessingFlag && m_contextUI.lastGpsMode == NGM_STAND_BY)
    {
        if(animated && cameraParameter.id != 0)
        {
            OnCameraAnimationDone(cameraParameter.id, MVAST_CANCELED);
        }
        return NE_CANCELLED;
    }

    if(m_isInAtomic)
    {
        AddRenderThreadTask(new RenderTaskSetCameraSettings(this, &MapView::RenderThread_SetCameraSettings,
                                                            cameraParameter.latitude, cameraParameter.longitude,
                                                            cameraParameter.heading, cameraParameter.zoomLevel,
                                                            cameraParameter.tilt, animated, duration, type, cameraParameter.id, m_cameraTimestamp++));
    }
    else
    {
        MapViewCamera::AnimationAccelerationType camera_type = MapViewCamera::AAT_LINEAR;
        switch(type)
        {
            case MVCAAT_LINEAR:
                camera_type = MapViewCamera::AAT_LINEAR;
                break;
            case MVCAAT_DECELERATION:
                camera_type = MapViewCamera::AAT_DECELERATION;
                break;
            case MVCAAT_ACCELERATION:
                camera_type = MapViewCamera::AAT_ACCELERATION;
                break;
            default:
                break;
        }

        m_mapViewCamera->SetCamera(cameraParameter.latitude, cameraParameter.longitude, cameraParameter.heading, cameraParameter.zoomLevel, cameraParameter.tilt, m_cameraTimestamp++, animated, duration, camera_type, cameraParameter.id);
        AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_SetFrameZoomAndCenter));
        m_renderPending = true;
    }

    return NE_OK;
}

/* See MapViewInterface.h for description */
void
MapView::SetGpsFileName(const char* gpsFileName)
{
    m_gpsFileName = gpsFileName;
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetTheme(MapViewTheme theme)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return NE_NOTINIT;
    }

    if (theme >= MVT_MAX)
    {
        return NE_BADDATA;
    }

    m_eventQueue->AddTask(new MapViewTaskSetTheme(this, &MapView::CCC_SetTheme, theme));

    return NE_OK;
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetAnimationLayerSettings(unsigned int totalAnimationTime, unsigned int loopPause, double minimumTilePercentage)
{
    AnimationLayerParameters parameters(totalAnimationTime, loopPause, minimumTilePercentage);

    // Switch to render thread
    AddRenderThreadTask(new RenderTaskUpdateAnimationParameters(this, &MapView::RenderThread_UpdateAnimationLayerParameters, parameters));
    return NE_OK;
}

/* See MapViewInterface.h for description */
NB_Error
MapView::SetAnimationLayerOpacity(unsigned int opacity)
{
    // Switch to render thread
    AddRenderThreadTask(new RenderTaskUpdateOpacity(this, &MapView::RenderThread_UpdateAnimationLayerOpacity, opacity));
    return NE_OK;
}

/* See MapViewInterface.h for description */
bool
MapView::GetRotateByGestureSupported()
{
    return NBUIConfig::GetRotateByGestureSupported();
}

/* See MapViewInterface.h for description */
bool
MapView::GetTiltByGestureSupported()
{
    return NBUIConfig::GetTiltByGestureSupported();
}

/* See MapViewInterface.h for description */
bool
MapView::GetInlineTrafficSupported()
{
    return NBUIConfig::GetInlineTrafficSupported();
}

/* See MapViewInterface.h for description */
void
MapView::ShowRouteLayer(uint32 /*layerID*/)
{
    // @todo
}

/* See MapViewInterface.h for description */
void
MapView::HideRouteLayer(uint32 /*layerID*/)
{
    // @todo

}
/* See MapViewInterface.h for description */
NB_Error
MapView::GenerateMapImage(shared_ptr<GenerateMapCallback> callback, int32 x, int32 y, uint32 width, uint32 height) const
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return NE_NOTINIT;
    }

    AddRenderThreadTask(new RenderTaskGenerateMapImage((void*)this, &MapView::RenderThread_GenerateMapImage, callback, x, y, width, height));

    return NE_OK;
}

/* See MapViewInterface.h for description */
void
MapView::SetNightMode(MapViewNightMode nightMode)
{
    if (m_nightMode == nightMode)
    {
        return;
    }

    m_nightMode = nightMode;
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView::SetNightMode :%d", m_nightMode);

    if (!m_initialize.IsDone() || m_initialize.IsShuttingDown())
    {
        return;
    }

    switch (m_nightMode)
        {
        case MVNM_DAY:
            m_isNight = false;
            m_eventQueue->AddTask(new MapViewTaskSetTheme(this, &MapView::CCC_SetTheme, MVT_DAY));
            break;
        case MVNM_NIGHT:
            m_isNight = true;
            m_eventQueue->AddTask(new MapViewTaskSetTheme(this, &MapView::CCC_SetTheme, MVT_NIGHT));
            break;
        case MVNM_AUTO:
            m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_CalculateNightMode));
            break;
        default:
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView::SetNightMode: invalid NightMode");
            break;
    }
}

/* See MapViewInterface.h for description */
bool
MapView::IsNightMode()
{
    return m_isNight;
}

void
MapView::BeginAtomicUpdate()
{
    if(m_CombinedAtomicTaskExecuter)
    {
        m_CombinedAtomicTaskExecuter->Enable(TRUE);
        m_isInAtomic = true;
    }
}

void
MapView::EndAtomicUpdate()
{
    if(m_CombinedAtomicTaskExecuter)
    {
        m_CombinedAtomicTaskExecuter->Enable(FALSE);
        m_isInAtomic = false;
    }
}

void
MapView::RenderThread_GenerateMapImage(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    const RenderTaskGenerateMapImage* task = static_cast<const RenderTaskGenerateMapImage*>(pTask);
    if (task)
    {
        DataStreamPtr dataStream = DataStream::Create();
        uint32 width, height;
        if(task->m_width == 0 || task->m_height == 0)
        {
            width = m_viewSizeWidth;
            height = m_viewSizeHeight;
        }
        else
        {
            width = task->m_width;
            height = task->m_height;
        }
        uint32 snapBufferLen = m_viewSizeWidth * m_viewSizeHeight * 4;
        uint8* snapBuffer = (uint8*)nsl_malloc(sizeof(uint8) * snapBufferLen);
        nsl_memset(snapBuffer, 0, snapBufferLen);

        if(m_nbgmViewController)
        {
            m_nbgmViewController->GenerateMapImage(snapBuffer, snapBufferLen, task->m_x, task->m_y, width, height);
        }

        dataStream->AppendData(snapBuffer, snapBufferLen);
        UItaskGenerateMapImage* uiTask = new UItaskGenerateMapImage(this, &MapView::UI_GenerateMapImage, task->m_callback, dataStream, width, height);
        m_UITaskQueue->AddTask(uiTask);

        if(snapBuffer)
        {
            nsl_free(snapBuffer);
            snapBuffer = NULL;
        }
    }
}


// MapViewUIInterface functions .....................................................................................................

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_InitializeMapView()
{
    // This function should only be called if the initialization wasn't completed
    if (m_initialize.IsDone())
    {
        return NE_BADDATA;
    }

    // Finish NBGM initialization
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_Initialize));
    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_Render(uint32 interval)
{
    /* This function is called from the render thread and needs to make a task switch if it calls any CCC functions */

    bool needRedraw = false;

    // Ignore any calls before we are fully initialized and during shutdown
    if (m_initialize.IsShuttingDown())
    {
        if (m_defaultRenderListener)
        {
            m_defaultRenderListener->OnDiscard();
        }
        return NE_NOTINIT;
    }

    // Check if we need to handle any animation tiles.
    if (m_frame.HasAnimationLayers() && m_lastAnimationDisplay)
    {
        if (m_frame.HasAnimationTiles())
        {
            bool nextFrame = false;
            bool beginningFrame = false;
            bool play = RenderThread_PlayDoppler(nextFrame, beginningFrame);

            if(play)
            {
                uint32 newInterval = interval;
                if(nextFrame)
                {
                    m_frame.SwitchAnimationToNextFrame();
                    newInterval = 0;
                }
                else if(beginningFrame)
                {
                    m_frame.SwitchAnimationToFirstFrame();
                    newInterval = 0;
                }

                map<uint32, ANIMATION_DATA> animationData;
                uint32 timestamp = 0;
                bool animationCanPlay = false;

                if (m_frame.GetAnimationFrameData(newInterval, animationData, timestamp, animationCanPlay))
                {
                    // Update the timestamp in the UI
                    if (timestamp != m_lastAnimationTimestamp || animationCanPlay != m_dopplerState.isPlaying)
                    {
                        m_UITaskQueue->AddTask(new MapViewTaskUIAnimationLayer(this, &MapView::UI_HandleAnimationLayer, false, false, timestamp, animationCanPlay));
                        m_lastAnimationTimestamp = timestamp;
                        m_dopplerState.isPlaying = animationCanPlay;
                    }

                    if (m_nbgmViewController && !animationData.empty())
                    {
                        // Update the data in NBGM
                        m_nbgmViewController->SetAnimationFrameData(animationData);
                    }
                }
                needRedraw = true;
                // I don't think we want to switch to "No Data" display if we have some tiles but it's not enought for the animation.
            }
        }
        else
        {
            //Keep the last timestamp and notify it couldn't play animation.
            if (m_dopplerState.isPlaying)
            {
                //Keep the last timestamp and notify it couldn't play animation.
                m_UITaskQueue->AddTask(new MapViewTaskUIAnimationLayer(this, &MapView::UI_HandleAnimationLayer, false, false, m_lastAnimationTimestamp, false));
            }
        }
    }

    // Reset rendering flag to indicate that new rendering tasks can be added to the queue.
    m_needsRendering = false;

    // interval parameter will be used for animation
    if (m_nbgmViewController)
    {
        needRedraw = m_nbgmViewController->Render(m_defaultRenderListener) ? true : needRedraw;
        double lat = 0;
        double lon = 0;
        float heading = 0;
        float tilt = 0;
        float zoomLevel = 0;
        m_mapViewCamera->GetCamera(lat, lon, zoomLevel, heading, tilt);
        if (m_pListener && (!m_cameraUpdateGuard))
        {
            if (m_cameraPrevLat != lat ||
                m_cameraPrevLon != lon ||
                m_cameraPrevZoomLevel != zoomLevel ||
                m_cameraPrevHeading != heading ||
                m_cameraPrevTilt != tilt)
            {
                m_cameraUpdateGuard = true;
                m_UITaskQueue->AddTask(new UITaskInvokeCameraUpdateListener(
                    this,
                    &MapView::UI_InvokeCameraUpdateListener,
                    lat, lon, zoomLevel, heading, tilt));
                m_cameraPrevLat = lat;
                m_cameraPrevLon = lon;
                m_cameraPrevZoomLevel = zoomLevel;
                m_cameraPrevHeading = heading;
                m_cameraPrevTilt = tilt;
            }
        }

        // Check animation status after this render.
        // AnimationCallbacks should be called after CameraUpdate callback.
        m_mapViewCamera->CheckAnimationStatus();
    }
    // Release render listener if it is not passed to NBGM
    if(m_defaultRenderListener)
    {
        m_defaultRenderListener->OnDiscard();
    }

#ifdef ENABLE_CONTINUOUS_RENDERING
    needRedraw = true;
#endif

    if (needRedraw)
    {
        m_renderPending = true;
    }

    /*
      Always update the frame zoom/center after a render call. It does nothing if
      zoom/center hasn't changed. The render call can modify zoom/tilt and other map
      parameters if an animation in is progress.
    */
    SetFrameZoomAndCenter();

    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_SetViewSize(int width, int height)
{
    m_viewSizeWidth = width;
    m_viewSizeHeight = height;

    // Ignore any calls during shutdown
    if ((!m_initialize.IsDone()) || m_initialize.IsShuttingDown() || (!m_nbgmViewController))
    {
        return NE_NOTINIT;
    }

    // this function need to take effect immediately, for it can effect WindowToWorld and WorldToWindow.
    m_mapViewCamera->SetFrameBufferSize(width, height);
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_SetFrameZoomAndCenter));
    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_SetScreenOrientation(bool portrait)
{
    // Ignore any calls during shutdown
    if ((!m_initialize.IsRenderingAllowed()) || m_initialize.IsShuttingDown())
    {
        return NE_NOTINIT;
    }

    // Switch to render thread
    AddRenderThreadTask(new RenderTaskSetScreenOrientation(this, &MapView::RenderThread_SetScreenOrientation, portrait));
    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_SetCurrentPosition(double latitude, double longitude)
{
    // Ignore any calls during shutdown
    if ((!m_initialize.IsRenderingAllowed()) || m_initialize.IsShuttingDown())
    {
        return NE_NOTINIT;
    }
    // Switch to render thread
    AddRenderThreadTask(new RenderTaskSetCurrentPosition(this, &MapView::RenderThread_SetCurrentPosition, latitude, longitude));
    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_SetViewPort(int width, int height)
{
    m_viewSizeWidth = width;
    m_viewSizeHeight = height;

    // Ignore any calls during shutdown
    if ((!m_initialize.IsDone()) || m_initialize.IsShuttingDown() || (!m_nbgmViewController))
    {
        return NE_NOTINIT;
    }

    // this function need to take effect immediately, for it can effect WindowToWorld and WorldToWindow.
    //m_nbgmViewController->SetFrameBufferSize(width, height);
    m_mapViewCamera->SetFrameBufferSize(width, height);
    // Switch to render thread
    //AddRenderThreadTask(new RenderTaskSetViewPort(this, &MapView::RenderThread_SetViewPort, width, height));
    return NE_OK;
}

/* See MapViewUIInterface.h for description */
NB_Error
MapView::UI_HandleViewGesture(const MapView_GestureParameters* parameters)
{
    /* This is currently the only UI_* call that is actually called from the UI thread! */

    // Ignore any calls before we are fully initialized and during shutdown
    if ((! m_initialize.IsRenderingAllowed()) || (m_initialize.IsShuttingDown()) || !m_initialize.IsDone())
    {
        return NE_NOTINIT;
    }

    if(parameters->state == MapView_GestureStateBegan &&
      (parameters->gestureType == MapViewTransformation_Move || parameters->gestureType == MapViewTransformation_RotateAngle))
    {
        m_contextUI.gestrueProcessingFlag = true;
    }
    if((parameters->state == MapView_GestureStateEnded || parameters->state == MapView_GestureStateCancelled) &&
       (parameters->gestureType == MapViewTransformation_Move || parameters->gestureType == MapViewTransformation_RotateAngle))
    {
        m_contextUI.gestrueProcessingFlag = false;
    }

    /* When we are in follow-me mode then we callback to the MapView user to check if we should allow the gesture or not. */
    if (m_gpsMode == NGM_FOLLOW_ME)
    {
        if (m_pListener)
        {
            // Convert gesture type
            MapViewGestureType type = MVGT_Invalid;
            switch (parameters->gestureType)
            {
                case MapViewTransformation_Move:            type = MVGT_Move;           break;
                case MapViewTransformation_TiltAngle:       type = MVGT_TiltAngle;      break;
                case MapViewTransformation_RotateAngle:     type = MVGT_RotateAngle;    break;
                case MapViewTransformation_Scale:           type = MVGT_Scale;          break;
                case MapViewTransformation_Tap:             type = MVGT_Tap;            break;
                case MapViewTransformation_DoubleTap:       type = MVGT_DoubleTap;      break;
                case MapViewTransformation_TwoFingerTap:    type = MVGT_TwoFingerTap;   break;
                case MapViewTransformation_LongPress:       type = MVGT_LongPress;      break;
                case MapViewTransformation_TwoFingerDoubleTap:  type = MVGT_TwoFingerDoubleTap;  break;
                default:
                    break;
            }

            // Callback to UI. Make sure to call this function in the UI thread.
            if (! m_pListener->GestureDuringFollowMeMode(type))
            {
                // Ignore the call
                return NE_OK;
            }
        }
    }

    m_gestureHandler->HandleGesture(parameters);
    uint32 time = PAL_ClockGetTimeMs();
    MapViewGestureState gs = (MapViewGestureState)parameters->state;
    if(gs == MVGS_Began || gs == MVGS_Ended || gs == MVGS_Cancelled || gs == MVGS_Failed)
    {
        m_pListener->OnGesture((MapViewGestureType)parameters->gestureType, (MapViewGestureState)parameters->state, time);
    }

    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_SetFrameZoomAndCenter));
    m_renderPending = true;

    return NE_OK;
}

/* See MapViewUIInterface.h for description */
void
MapView::UI_SetCamera(double lat, double lon, double zoomLevel, double heading, double tilt, bool animated, uint32 duration, MapViewUI_CameraAnimationAccelertaionType type)
{
    // Ignore any calls before we are fully initialized and during shutdown
    if ((! m_initialize.IsRenderingAllowed()) || (m_initialize.IsShuttingDown()) || (!m_mapViewCamera))
    {
        return;
    }

    MapViewCamera::AnimationAccelerationType camera_type = MapViewCamera::AAT_LINEAR;
    switch(type)
    {
        case MVUICAAT_LINEAR:
            camera_type = MapViewCamera::AAT_LINEAR;
            break;
        case MVUICAAT_DECELERATION:
            camera_type = MapViewCamera::AAT_DECELERATION;
            break;
        default:
            break;
    }

    m_mapViewCamera->SetCamera(lat, lon, heading, zoomLevel, tilt, m_cameraTimestamp++, animated, duration, camera_type);

    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_SetFrameZoomAndCenter));
    m_renderPending = true;
    // Update debug UI after camera is updated. Since CONTINUOUS_RENDERING is disabled, we
    // need to find proper places to update debug ui.
    UpdateDebugUI();
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"SET_CAMERA LAT:%d %d %d %d %d",lat, lon, zoomLevel, heading, tilt);
}

float
MapView::UI_GetCameraRotationAngle()
{
    return m_mapViewCamera->GetRotateAngle();
}

bool
MapView::IsFirstTilesLoaded(void)
{
    return m_firstTilesLoaded;
}

// LayerManagerListener functions ......................................................................................

/* See LayerManagerListener.h for description */
void
MapView::RefreshAllTiles()
{
    // Get tiles to unload and remove them from class Frame.
    vector<TilePtr> tilesToUnload;
    m_frame.UnloadAllTiles(tilesToUnload);

    /* Unload all tiles from NBGM containing NBGM cache.

       TRICKY: Do not check if the vector 'tilesToUnload' is empty here,
               because the NBGM cache needs to be cleared.
    */
    MapViewTaskUnloadTiles* taskToUnloadTiles = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadAllTilesFromNBGM);
    if (taskToUnloadTiles)
    {
        taskToUnloadTiles->m_tilesToUnload = tilesToUnload;
        AddRenderThreadTask(taskToUnloadTiles);
    }

    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);

    // @note: Although this is called in CCC
    // This should only be called in CCC thread, no need to switch thread.
    if (! tiles.empty())
    {
        // Re-request tiles for all layers
        vector<LayerPtr> emptyVector;
        m_layerManager->GetTiles(tiles, m_pTileCallback, emptyVector, emptyVector);
    }

    // Request animation tiles, if we have any animation layers
    if (! m_CCCAnimationFrameLists.empty())
    {
        for (map<LayerPtr, vector<uint32> >::iterator iterator = m_CCCAnimationFrameLists.begin(); iterator != m_CCCAnimationFrameLists.end(); ++iterator)
        {
            m_layerManager->GetAnimationTiles(iterator->first, tiles, iterator->second, m_pTileCallback);
        }
    }
}

void MapView::ReloadTilesOfLayer(LayerPtr layer)
{
    if (layer)
    {
        vector<TilePtr> tilesToReload;
        m_frame.GetLoadedTilesOfLayer(layer->GetID(), tilesToReload);
        if (!tilesToReload.empty())
        {
            AddRenderThreadTask(
                new RenderTaskReloadTiles(this, &MapView::RenderThread_ReloadTiles,
                                           tilesToReload));
        }
    }
}

/* See LayerManagerListener.h for description */
void
MapView::RefreshTilesOfLayer(LayerPtr layer)
{
    // Check if pointer to layer is valid.
    if (!layer)
    {
        return;
    }

    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);

    // @note: This should only be called in CCC thread, no need to switch thread.
    if (! tiles.empty())
    {
        RefreshTiles(tiles, layer);
    }
}

/* See LayerManagerListener.h for description */
void
MapView::RefreshTiles(const vector<TileKeyPtr>& tileKeys, LayerPtr layer)
{
    // Check if pointers to tile key and layer are valid.
    if ((tileKeys.empty()) || (!layer))
    {
        return;
    }

    //@todo: There is no better way to get layer type for now.
    //       We need to figure out a better way to handle refresh tiles, inlcuding traffic tiles.
    bool trafficLayer = layer->GetTileDataType()?(layer->GetTileDataType()->compare(TRAFFIC_LAYER_NAME) == 0):false;
    if(!trafficLayer)
    {
         m_tilesToReload.clear();
        // Get tiles to unload.
        vector<TilePtr> tilesToUnload = m_frame.UnloadTilesFromFrame(tileKeys, layer->GetID());
        if (!tilesToUnload.empty())
        {
            for_each(tilesToUnload.begin(), tilesToUnload.end(),
                TilesToReloadInserter(m_tilesToReload));
        }
    }

    /*
        Re-request the tiles for the given layer. This is potentially requesting too many
        tiles, but existing tiles are just ignored when they are received so I think it is
        ok.
     */

    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);
    if (!tiles.empty())
    {
        vector<LayerPtr> layerVector(1, layer);
        // Re-request tiles for the new layers
        vector<LayerPtr> emptyVector;
        m_layerManager->GetTiles(tiles, m_pTileCallback, layerVector, emptyVector);
    }

    // :TRICKY: For some kinds of layers(Pin, geographic), tiles are returned synchronously,
    //          and when code goes here, m_tilesToReload should have been modified in the
    //          callback, and only tiles that needs to be downloaded from DTS server are
    //          left. For those kinds of tiles, we need to unload them here, and load again
    //          when tiles are available.
    if(!trafficLayer)
    {
        if (!m_tilesToReload.empty())
        {
            vector<TilePtr> tilesToUnload;
            for_each(m_tilesToReload.begin(), m_tilesToReload.end(),
                MappedValuesToVectorFunctor<TileKey, TilePtr>(tilesToUnload));
            m_tilesToReload.clear();

            // Unload tiles from NBGM.
            MapViewTaskUnloadTiles* taskToUnloadTiles = new MapViewTaskUnloadTiles(
                this, &MapView::RenderThread_UnloadTilesToNBGM);
            if (taskToUnloadTiles)
            {
                taskToUnloadTiles->m_tilesToUnload = tilesToUnload;
                AddRenderThreadTask(taskToUnloadTiles);
            }
        }
    }
}

void
MapView::UpdateSatelliteMode()
{
    bool isSatellite = false;
    for (vector<LayerPtr>::const_iterator it = m_layers.begin();
        it != m_layers.end(); ++it)
    {
        LayerPtr l = *it;
        if (l->GetTileDataType())
        {
            if (l->GetTileDataType()->compare(TILE_TYPE_SATELLITE) == 0)
            {
                isSatellite = true;
                break;
            }
        }
    }

    if (isSatellite != m_isSatellite)
    {
        m_isSatellite = isSatellite;
        // update material
        SetTheme(m_isNight ? MVT_NIGHT : MVT_DAY);
    }
}

/* See LayerManagerListener.h for description */
void
MapView::LayersUpdated(const vector<LayerPtr>& layers)
{
    // this function get called in CCC thread.
    m_initialize.ResetRenderingState();
    CleanMaterials();
    LayerRequestSuccess(layers);

    vector<LayerPtr>::const_iterator endIter = layers.end();
    vector<LayerPtr>::const_iterator iter    =
            find_if(layers.begin(), endIter, LayerFindByDataTypeFunctor(TILE_TYPE_TRAFFIC));
    if (iter != endIter)
    {
        m_minZoomOfTrafficLayer = (*iter)->GetMinZoom();
        m_trafficLayerId        = (*iter)->GetID();
        m_trafficLayerAvailable      = true;
    }
    else
    {
        if (m_trafficLayerId != INVALID_LAYER_ID)
        {
            MapViewTask* pTask = new UITaskUpdateTrafficTip(this,
                                                            &MapView::UI_UpdateTrafficTip,
                                                            TTT_None);
            if (pTask)
            {
                m_UITaskQueue->AddTask(pTask);
            }
        }
        m_trafficLayerId   = INVALID_LAYER_ID;
        m_trafficLayerAvailable = false;
    }

    UpdateSatelliteMode();
}

/* See LayerManagerListener.h for description */
void
MapView::LayersAdded(const vector<LayerPtr>& layers)
{
    if (m_initialize.IsShuttingDown() || m_updatingLayers)
    {
        return;
    }

#ifdef DTRAFFIC
    qWarning("TRAFFIC: 9. MapView::LayersAdded(), addedLayers.size=%d", layers.size());
#endif

    // Check if traffic layer is newly added
    if (!m_trafficLayerAvailable)
    {
        vector<LayerPtr>::const_iterator endIter = layers.end();
        vector<LayerPtr>::const_iterator iter    =
                find_if(layers.begin(), endIter, LayerFindByDataTypeFunctor(TILE_TYPE_TRAFFIC));
        if (iter != endIter)
        {
            m_minZoomOfTrafficLayer = (*iter)->GetMinZoom();
            m_trafficLayerId        = (*iter)->GetID();
            m_trafficLayerAvailable      = true;

            // Update Traffic Tip if necessary
            MapViewTask* pTask = new MapViewTask(this, &MapView::CCC_CheckTrafficStatus);
            if (pTask)
            {
                m_eventQueue->AddTask(pTask);
            }
        }
    }

#ifdef DTRAFFIC
    qWarning("TRAFFIC: 9. MapView::LayersAdded(), m_trafficLayerAvailable=%s", m_trafficLayerAvailable?"true":"false");
#endif
 
    // filter out duplicated layers first.
    vector<LayerPtr> newLayers;
    vector<LayerPtr>::const_iterator iter    = layers.begin();
    vector<LayerPtr>::const_iterator end     = layers.end();
    vector<LayerPtr>::const_iterator endIter = m_layers.end();
    for (; iter != end; ++iter)
    {
        if (find_if(m_layers.begin(),
                    m_layers.end(),
                    LayerFindByPointerFunctor(*iter)) == endIter)
        {
            newLayers.push_back(*iter);
        }
    }

    if (newLayers.empty())
    {
        return;
    }

    m_layers.insert(m_layers.end(), newLayers.begin(), newLayers.end());
    UpdateFrameAndNBGM();

    // Get current tiles from added layers.
    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);
    if (! tiles.empty() && m_eventQueue)
    {
        // Re-request tiles for the new layers
        // @note: Need to switch to CCC thread to start downloading. In fact, this function
        // should only be invoked in CCC thread if CCI is using CCC correctly!
        vector<LayerPtr> emptyVector;
        MapViewTaskGetSpecifiedTiles* pTask =
                new MapViewTaskGetSpecifiedTiles(this, &MapView::CCC_GetSpecifiedTiles,
                                                 tiles, newLayers, emptyVector);
        if (pTask)
        {
            m_eventQueue->AddTask(pTask);
        }
    }

    UpdateSatelliteMode();
    m_renderPending = true;

    // @todo: If some common material layers were included in layers, then we may need to
    // update common materials here.
}

/* See LayerManagerListener.h for description */
void
MapView::LayersRemoved(const vector<LayerPtr>& layers)
{
    if (m_initialize.IsShuttingDown() || layers.empty() || m_updatingLayers)
    {
        return;
    }

    // Check if traffic layer is removed, and show Tips if so.
    if (m_trafficLayerAvailable)
    {
        vector<LayerPtr>::const_iterator endIter = layers.end();
        vector<LayerPtr>::const_iterator iter    =
                find_if(layers.begin(), endIter, LayerFindByDataTypeFunctor(TILE_TYPE_TRAFFIC));
        if (iter != endIter)
        {
            m_trafficLayerId        = INVALID_LAYER_ID;
            m_trafficLayerAvailable = false;
            // No need to switch thread here.
            CCC_CheckTrafficStatus(NULL);
            m_renderPending = true;
            vector<shared_ptr<string> > selectedIds = m_nbgmViewController->GetSelectedPinIds();
            for(int i = 0; i < selectedIds.size(); i++)
            {
                shared_ptr<string> selectedId = selectedIds[i];
                if ( selectedId && selectedId->find(TRAFFIC_LAYER_NAME) != string::npos)
                {
                    UnselectPin(selectedId);
                }
            }
        }
    }

    // Check and unload tile provided by these layers.
    vector<LayerPtr>::const_iterator iter = layers.begin();
    vector<LayerPtr>::const_iterator end  = layers.end();
    vector<TilePtr> tilesToUnload;

    for (; iter != end; ++iter)
    {
        if (*iter)
        {
            m_frame.UnloadTilesOfLayer((*iter)->GetID(), tilesToUnload);
            shared_ptr<string> layerType = (*iter)->GetTileDataType();
            if( layerType->find( "POI" ) != string::npos )
            {
                NotifyHideStaticPoiBubble();
            }
        }
    }

    vector<LayerPtr> newLayers(layers);

    // Sort layers for set_difference algorithm below
    sort(newLayers.begin(), newLayers.end());
    sort(m_layers.begin(), m_layers.end());

    // Remove all layers
    vector<LayerPtr> tempVector;
    m_layers.swap(tempVector);
    set_difference(tempVector.begin(), tempVector.end(), newLayers.begin(), newLayers.end(), back_inserter(m_layers));

    UpdateFrameAndNBGM();

    if (!tilesToUnload.empty())
    {
        // Unload tiles from NBGM.
        MapViewTaskUnloadTiles* pTask = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
        if (pTask)
        {
            // Add the tiles to the task object and add the task
            pTask->m_tilesToUnload = tilesToUnload;
            AddRenderThreadTask(pTask);
        }
    }

    UpdateSatelliteMode();
    m_renderPending = true;
}

/* See LayerManagerListener.h for description */
void
MapView::AnimationLayerAdded(shared_ptr<Layer> layer)
{
    // This is called in CCC thread.
    m_layers.push_back(layer);
    UpdateFrameAndNBGM();

    m_maxZoomOfAnimationLayer = layer->GetMaxZoom();

    // Insert the layer pointer and an empty frame list. The frame list gets updated in AnimationLayerUpdated()
    m_CCCAnimationFrameLists.insert(make_pair(layer, vector<uint32>()));

    // If this is the first animation layer then we create the timestamp label in the UI
    if (m_CCCAnimationFrameLists.size() == 1)
    {
        m_UITaskQueue->AddTask(new MapViewTaskUIAnimationLayer(this, &MapView::UI_HandleAnimationLayer, true, false, 0, false));
    }

    // Switch to render thread
    MapViewTaskAnimationLayer* pTask = new MapViewTaskAnimationLayer(this, &MapView::RenderThread_HandleAnimationLayer);
    if (pTask)
    {
        pTask->m_layerAdded = layer;
        AddRenderThreadTask(pTask);
    }
}

/* See LayerManagerListener.h for description */
void
MapView::AnimationLayerUpdated(shared_ptr<Layer> layer, const vector<uint32>& frameList)
{
    // This is called in CCC thread.
    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);

     map<LayerPtr, vector<uint32> >::iterator iterator = m_CCCAnimationFrameLists.find(layer);
    // Find the animation layer, we should always find it.
    if (iterator != m_CCCAnimationFrameLists.end())
    {
        // if framelist is empty, just keep current animation tiles and return. While zooming or panning map, current animation tiles will be unloaded.
        if (frameList.empty())
        {
            return;
        }

        // Sort frame list so that we can use set_difference() below
        vector<uint32> sortedFrameList(frameList);
        sort(sortedFrameList.begin(), sortedFrameList.end());

        // Calculate the difference (new - old) of the frame lists
        vector<uint32> deltaFrameList;
        set_difference(sortedFrameList.begin(), sortedFrameList.end(),
                       iterator->second.begin(), iterator->second.end(),
                       back_inserter(deltaFrameList));

        // Update frame list.
        iterator->second = sortedFrameList;

        // Request animation tiles for new timestamps
        m_layerManager->GetAnimationTiles(layer, tiles, deltaFrameList, m_pTileCallback);

        // Switch to render thread
        MapViewTaskAnimationLayer* pTask =
                new MapViewTaskAnimationLayer(this, &MapView::RenderThread_HandleAnimationLayer);
        if (pTask)
        {
            pTask->m_layerUpdated    = layer;
            pTask->m_sortedFrameList = sortedFrameList;
            AddRenderThreadTask(pTask);
        }

        // Get min/max timestamp
        uint32 minTimestamp = sortedFrameList.front();
        uint32 maxTimestamp = sortedFrameList.back();

        m_frame.RemoveInvalidAnimationTiles(layer->GetID(), minTimestamp, maxTimestamp);

        // Request animation tiles for new timestamps
        m_layerManager->GetAnimationTiles(layer, tiles, deltaFrameList, m_pTileCallback);
    }
}

/* See LayerManagerListener.h for description */
void
MapView::AnimationLayerRemoved(shared_ptr<Layer> layer)
{
    // This is called in CCC thread

    // Remove layer. Refer to "Erase-remove idiom on Wikipedia"
    LayerFindByIdFunctor functor(layer->GetID());

    std::vector<LayerPtr>::iterator iter = find_if(m_layers.begin(), m_layers.end(), functor);
    if(iter != m_layers.end())
    {
        m_layers.erase(iter);
    }

    UpdateFrameAndNBGM();

    // Unload all tiles for that layer from frame class
    vector<TilePtr> tilesToUnload;
    m_frame.UnloadTilesOfLayer(layer->GetID(), tilesToUnload);

    // Remove layer and frame list
    m_CCCAnimationFrameLists.erase(layer);

    // If this is the last animation layer then we remove the timestamp label in the UI
    if (m_CCCAnimationFrameLists.empty())
    {
        m_UITaskQueue->AddTask(new MapViewTaskUIAnimationLayer(this, &MapView::UI_HandleAnimationLayer, false, true, 0, false));
    }

    if (!tilesToUnload.empty())
    {
        // Unload tiles from NBGM.
        MapViewTaskUnloadTiles* pTask = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
        if (pTask)
        {
            // Add the tiles to the task object and add the task
            pTask->m_tilesToUnload = tilesToUnload;
            AddRenderThreadTask(pTask);
        }
    }

    // Remove animation layer from NBGM and Frame class
    MapViewTaskAnimationLayer* pTask = new MapViewTaskAnimationLayer(this, &MapView::RenderThread_HandleAnimationLayer);
    if (pTask)
    {
        pTask->m_layerRemoved = layer;
        AddRenderThreadTask(pTask);
    }
}

/* See LayerManagerListener.h for description */
void
MapView::PinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    if ((!pins) || pins->empty())
    {
        return;
    }

    // Add a render task to hide the pin bubble.
    if (m_pRenderingQueue)
    {
        AddRenderThreadTask(new MapViewTaskRemovePinBubble(this, &MapView::RenderThread_RemovePinBubble, pins));
    }
}

/* See LayerManagerListener.h for description */
//@todo: Change void* to proper data struct
void
MapView::EnableMapLegend(bool enabled, shared_ptr<MapLegendInfo> mapLegend)
{

    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    UpdateAnimationUI(enabled, mapLegend);
}

/* See header for description */
void
MapView::UpdateAnimationUI(bool enabled, shared_ptr<MapLegendInfo> mapLegend)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    if (mapLegend)
    {
        m_mapLegendInfo = mapLegend;
    }

    AnimationTipType tipType = ATT_None;
    bool animationDisplay = false;

    if (enabled && m_frame.HasAnimationLayers())
    {
        uint32 currentZoom = (uint32)floor(m_frame.GetZoom());
        if (currentZoom > m_maxZoomOfAnimationLayer)
        {
            animationDisplay = false;    //Hide animation bar
            tipType = ATT_ZoomOut;
        }
        else
        {
            animationDisplay = true;     //Show animation bar
            tipType = ATT_None;
        }
    }
    else
    {
        animationDisplay = false;
        tipType = ATT_None;
    }

    if (m_lastAnimationDisplay != animationDisplay)
    {
        MapViewTask* pTask = NULL;
        if (animationDisplay && m_mapLegendInfo)
        {
            pTask = new MapViewTaskShowMapLegend(this, &MapView::UI_ShowMapLegend, m_mapLegendInfo);
        }
        else
        {
            pTask = new MapViewTask(this, &MapView::UI_HideMapLegend);
        }
        if (pTask)
        {
            m_UITaskQueue->AddTask(pTask);
        }
    }

    if (m_lastAnimationTip != tipType)
    {
        MapViewTask* task = new UITaskUpdateAnimationTip(this,
                                                        &MapView::UI_UpdateAnimationTip,
                                                        tipType);
        if (task)
        {
            m_UITaskQueue->AddTask(task);
        }
    }

    m_lastAnimationDisplay = animationDisplay;
    m_lastAnimationTip = tipType;
}

/* See header for description */
void
MapView::LayerRequestSuccess(const vector<LayerPtr>& layers)
{
    /* This function gets called in the CCC thread */
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::LayerRequestSuccess", this);
    if (m_initialize.IsShuttingDown())
    {
        return;
    }
    m_updatingLayers = true;

    /*
        if the metadata does not change, the layers should be equal m_layers, otherwise if they are different , we should refresh tiles.
    */
    bool needfresh = false;
    if(layers.size() != m_layers.size())
    {
        needfresh = true;
    }
    else
    {
        vector<LayerPtr> templayer = layers;
        //the  shared_ptr has overload the operator < , so  sort the  the pointer of layer.
        sort (m_layers.begin(), m_layers.end());
        sort (templayer.begin(), templayer.end());
        vector<LayerPtr> layerdiff;
        set_symmetric_difference (m_layers.begin(), m_layers.end(), templayer.begin(), templayer.end(), back_inserter(layerdiff));
        if(layerdiff.size() > 0)
        {
            needfresh = true;
        }
    }

    m_layers = layers;
    UpdateFrameAndNBGM();

    // Request common materials only after both this function has succeeded and MapView::Initialize() has been called.
    // We don't know which one gets called first!
    m_initialize.RequestCommonMaterialsIfNecessary(this);

    if(needfresh)
    {
        RefreshAllTiles();
    }

    if (m_pListener)
    {
        m_pListener->LayersUpdated();
    }
    m_updatingLayers = false;
}

/* See header for description */
void
MapView::LayerRequestError(NB_Error error)
{
    /* TRICKY: This callback may be called synchronously in the function GetLayers,
               so the function GetLayers could not be called here directly.
    */
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "MapView(%#x)::LayerRequestError error = %d", this, error);
    if(error != NESERVERX_PROCESSING_ERROR &&
       error != NESERVERX_UNSUPPORTED_ERROR &&
       error != NESERVERX_INTERNAL_ERROR &&
       error != NE_UNEXPECTED)
    {
        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_GetLayers));
    }
}

/* See header for description */
bool
MapView::IsAllCommonMaterialDownloaded()
{
    bool baseReady = false;
    bool poiReady = false;
    bool dayThemeReady = false;
    bool nightThemeReady = false;
    bool satelliteThemeReady = false;
    for (uint32 i = 0; i < m_commonMaterialTiles.size(); ++i)
    {
        TilePtr t = m_commonMaterialTiles[i];
        if(!t->GetMaterialBaseType() && t->GetMaterialCategory() && t->GetMaterialCategory()->compare("PMAT") == 0)
        {
            // Server now returns PMAT  & PBMAT, which are both used for POI.
            // The content of the two materials are exactly same.
            // PBMAT will displace PMAT in future, but now, due to some compatibility reason, they are both downloaded.
            poiReady = true;
            continue;
        }

        if (t->GetMaterialBaseType()->compare("base") == 0)
        {
            if (t->GetMaterialCategory()->compare("PMAT") == 0)
            {
                poiReady = true;
            }
            else if(t->GetMaterialCategory()->compare("MAT") == 0)
            {
                baseReady = true;
            }
        }
        else if (t->GetMaterialBaseType()->compare("theme") == 0)
        {
            shared_ptr<const string> theme = t->GetMaterialThemeType();
            if (theme)
            {
                if (theme->compare("day") == 0)
                {
                    dayThemeReady = true;
                }
                else if (theme->compare("night") == 0)
                {
                    nightThemeReady = true;
                }
                else if (theme->compare("satellite") == 0)
                {
                    satelliteThemeReady = true;
                }
            }
        }
    }
    return baseReady && dayThemeReady && nightThemeReady && satelliteThemeReady && poiReady;
}

/* See header for description */
void
MapView::CommonMaterialRequestSuccess(TileKeyPtr /*request*/, TilePtr response)
{
    /* This function gets called in the CCC thread */
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CommonMaterialRequestSuccess", this);
    if ((! response) || (m_initialize.IsShuttingDown()) || !response->GetData() || !response->GetSize())
    {
        return;
    }

    shared_ptr<const string> baseType = response->GetMaterialBaseType();
    shared_ptr<const string> materialCategory = response->GetMaterialCategory();
    shared_ptr<const string> materialName(new string(*response->GetContentID()));
    if (materialCategory && *materialCategory != "MAT" && *materialCategory != "PMAT" && !baseType) //@todo: remove hard-coded value.
    {
        ExternalMaterialPtr material(new ExternalMaterial(materialCategory,
                                                          materialName,
                                                          response->GetData()));
        MaterialAdded(material);
        MaterialActived(material);
        return;
    }

    if (!baseType)//PMAT
    {
        m_commonMaterialTiles.push_back(response);
    }
    else
    {
        if (baseType->compare("base") == 0)
        {
            m_commonMaterialTiles.push_back(response);
        }
        else if (baseType->compare("theme") == 0)
        {
            shared_ptr<const string> theme = response->GetMaterialThemeType();
            if (!theme)
            {
                return;
            }
            m_commonMaterialTiles.push_back(response);
        }
    }

    if (!IsAllCommonMaterialDownloaded())
    {
        return;
    }

    m_firstTilesLoaded = true;

    MapViewTaskUpdateCommonMaterial* pTask = new MapViewTaskUpdateCommonMaterial(this, &MapView::RenderThread_UpdateCommonMaterials);

    if (pTask)
    {
        // Set the status of rendering allowed to common materials success.
        m_initialize.SetRenderingAllowed(static_cast<int>(RAS_Common_Materials_Success));

        // Add the TilePtr to the task object and add the task
        pTask->m_tileToLoad = m_commonMaterialTiles;
        m_commonMaterialTiles.clear();

        pTask->isSatellite = m_isSatellite;

        if (!m_isNight)
        {
            pTask->isDay = true;
        }
        else
        {
            pTask->isDay = false;
        }
        AddRenderThreadTask(pTask);

        if (m_initialize.IsRenderingAllowed())
        {
            m_nativeView->RenderingAllowedNotified();
            m_renderPending = true;
            AddUpdateFrameTask();
            if(m_serviceConfiguration)
            {
                m_serviceConfiguration->SetTheme(m_isNight?MVT_NIGHT:MVT_DAY);
            }
        }
    }
    m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_OnMapReady));
}

/* See header for description */
void
MapView::CommonMaterialRequestError(TileKeyPtr /*request*/, NB_Error error)
{
    m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_OnMapReady));
    //Not sure what to do here
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "MapView(%#x)::CommonMaterialRequestError error = %d", this, error);
}

/* See header for description */
void
MapView::TileRequestSuccess(TilePtr response)
{
    /* This function gets called in the CCC thread */
    if ((! response) || (m_initialize.IsShuttingDown()))
    {
        return;
    }

    // Filter out unexpected tiles under satellite mode.
    // This case is rare, we have to add a protection for now.
    // @todo: Check related layer state after each tile downloaded.
    if(m_isSatellite && response->GetDataType() && response->GetDataType()->compare(DVA_LAYER_NAME) == 0)
    {
        return;
    }

    vector<TilePtr> tilesToUnload;


    bool logRenderTime = false;
    // Check if this is the last tile of the first request when app starts up.
    // @todo: We checks only BR layers at present, this should be addressed.
    if (!requestedTiles.empty() && response->GetDataType()->find("BR") != string::npos)
    {
        requestedTiles.erase(*(response->GetTileKey()));
        if (requestedTiles.empty())
        {
            logRenderTime = true;
            firstUse = false;
        }
    }

    //@todo: There is no better way to get layer type for now.
    //       We need to figure out a better way to handle refresh tiles, inlcuding traffic tiles.
    if(response->GetDataType() && response->GetDataType()->compare(TRAFFIC_LAYER_NAME) == 0)
    {
        vector<TileKeyPtr> tilesToCheck(1, response->GetTileKey());
        vector<TilePtr> tilesUnload = m_frame.UnloadTilesFromFrame(tilesToCheck, response->GetLayerID());
        if(!tilesUnload.empty())
        {
            MapViewTaskUnloadTiles* pTask = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
            if (pTask)
            {
                // Add the tiles to the task object and add the task
                pTask->m_tilesToUnload = tilesUnload;
                AddRenderThreadTask(pTask);
            }
        }
    }

    // Check the Frame class if we need to load the tile to NBGM
    if (m_frame.TileReceived(response, tilesToUnload) || logRenderTime)
    {
        do
        {
            TileWrapperPtr tileWrapper(new TileWrapper(response, logRenderTime));
            if (!tileWrapper)
            {
                break;
            }

            MapViewTaskLoadTiles* task = new MapViewTaskLoadTiles(
                this, &MapView::RenderThread_LoadTilesToNBGM);
            if (!task)
            {
                break;
            }

            map<TileKey, TilePtr>::iterator iter =
                    m_tilesToReload.find(*(response->GetTileKey()));
            if (iter != m_tilesToReload.end()) // Reload tile
            {
                TilePtr& tileToUnload = iter->second;
                if (tileToUnload)
                {
                    tileWrapper->m_previousID = tileToUnload->GetContentID();
                }
                m_tilesToReload.erase(iter);
            }
            // Add the TilePtr to the task object and add the task
            task->m_tilesToLoad.push_back(tileWrapper);
            AddRenderThreadTask(task);
        } while (0);
    }

    if (!tilesToUnload.empty())
    {
        // Unload tiles from NBGM.
        MapViewTaskUnloadTiles* pTask = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
        if (pTask)
        {
            // Add the tiles to the task object and add the task
            pTask->m_tilesToUnload = tilesToUnload;
            AddRenderThreadTask(pTask);
        }
    }
}

/* See header for description */
void
MapView::TileRequestError(NB_Error error)
{
    switch (error)
    {
        /*
            Ignore these errors for now
         */

        // Ignore no-content and ignore errors
        case NE_HTTP_NO_CONTENT:
        case NE_IGNORED:

        // The pin layer returns No-entity errors when there are no pins in the tile requests. We ignore them for now.
        case NE_NOENT:

        // Did the tile get dropped of the request queue because the queue is full
        case NE_AGAIN:

        // Tiles get cancelled when we switch between raster and GVR/GVA
        case NE_CANCELLED:

            // Do nothing on all of those cases
            break;

        default:
            // Nothing we can do here
            break;
    }
}


// Private functions ..............................................................................................................


/* See header for description */
NB_Error
MapView::CCC_Initialize()
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CCC_Initialize", this);
    if (! m_frame.Initialize(m_palInstance))
    {
        return NE_NOTINIT;
    }

    m_pTileCallback->SetMapView(this);
    m_pLayerCallback->SetMapView(this);
    m_pCommonMaterialCallback->SetMapView(this);

//    shared_ptr<WorkerQueueManager> threadManager = CommonComponentConfiguration::GetWorkerQueueManagerComponent(m_nbContext);
//    if (threadManager)
//    {
//        /* get worker task queue for nbgm rendering tiles to nbgm in separate threads */
//        m_pRenderingQueue = threadManager->RetrieveNBGMRenderingTaskQueue();
//    }

    NB_Error result = CCC_InitializeLayerManager();
    if (result != NE_OK)
    {
        return result;
    }

    return NE_OK;
}

/*See header for description*/
NB_Error
MapView::CCC_InitializeLayerManager()
{
    NB_ASSERT_CCC_THREAD(m_nbContext);
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CCC_InitializeLayerManager", this);
    PAL_Error palError = PAL_Ok;
    char pathChar[MAX_PATH_LENGTH] = {0};

    // Append the caching subpath to the work folder and set this path as the caching path.
    nsl_memset(pathChar, 0, MAX_PATH_LENGTH);
    nsl_strlcpy(pathChar, m_workFolder.c_str(), MAX_PATH_LENGTH);
    palError = PAL_FileAppendPath(m_palInstance, pathChar, MAX_PATH_LENGTH, MAPKIT3D_FOLDER_NAME);
    if (palError != PAL_Ok)
    {
        return NE_NOMEM;
    }

    shared_ptr<string> cachePath(new string(pathChar));
    NB_Error result = m_layerManager->SetCachePath(cachePath);
    if (result != NE_OK)
    {
        return result;
    }

    // Append the metadata subpath to the work folder and set this path as the metadata path.
    nsl_memset(pathChar, 0, MAX_PATH_LENGTH);
    nsl_strlcpy(pathChar, m_workFolder.c_str(), MAX_PATH_LENGTH);
    palError = PAL_FileAppendPath(m_palInstance, pathChar, MAX_PATH_LENGTH, METADATA_FOLDER_NAME);
    if (palError != PAL_Ok)
    {
        return NE_NOMEM;
    }

    shared_ptr<string> metadataPath(new string(m_workFolder));
    result = m_layerManager->SetPersistentMetadataPath(metadataPath);
    if (result != NE_OK)
    {
        return result;
    }

    m_layerManager->RegisterListener(this);

    // Request layers. See callback for result. Only once it has succeeded can we start requesting tiles from the server.
    // The flag m_layerManagerInitialized will be set once we successfully received the layers.
    NB_Error getLayersError = m_layerManager->GetLayers(m_pLayerCallback, m_pMetadataConfig);
    if (getLayersError != NE_OK)
    {
        /* Add a CCC task to request metadata again. It could avoid infinite
           loop and cost CPU.
        */
        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_GetLayers));
    }

    return NE_OK;
}

/* See header file for description */
PinPtr
MapView::CCC_GetPin(shared_ptr<string> pinId)
{
    PinPtr foundPin;

    // Find the pin from the pin manager.
    if (m_pinManager && pinId && (!(pinId->empty())))
    {
        size_t position = pinId->find(ID_SEPERATOR);
        if (position != string::npos)
        {
            string layerId = pinId->substr(0, position);
            if (!(layerId.empty()))
            {
                PinManagerImpl* manager = static_cast<PinManagerImpl*>(m_pinManager.get());
                PinLayerPtr layer = manager->GetPinLayer(layerId);
                if (layer)
                {
                    foundPin = layer->GetPin(pinId);
                }
            }
        }
    }

    if (!foundPin && m_pinHandler)
    {
        //Find the pin which is not added by pin layer
        foundPin = m_pinHandler->GetPin(pinId);
    }

    return foundPin;
}

/* See header for description */
void
MapView::CCC_InitializeComplete(const MapViewTask* pTask)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CCC_InitializeComplete", this);
    const MapViewTaskInitialize* task = static_cast<const MapViewTaskInitialize*>(pTask);

    /*
        Call the application layer callback to indicate that MapView is fully initialized (or initialization has failed)
     */
    if (task->m_result == NE_OK)
    {
        m_initialize.m_initializeCallback->Success(NULL /* not used */);
        m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_OnMapInitialized));

        // Set the status of rendering allowed to map view initialized complete.
        m_initialize.SetRenderingAllowed(static_cast<int>(RAS_Map_View_Initialized_Complete));
        if (m_initialize.IsRenderingAllowed())
        {
            m_nativeView->RenderingAllowedNotified();
            m_renderPending = true;
        }
    }
    else
    {
        m_initialize.m_initializeCallback->Error(task->m_result);
    }
}

void
MapView::UI_OnMapInitialized(const MapViewTask* /*pTask*/)
{
    m_nativeView->OnMapInitialized();

    if (m_pListener)
    {
        m_pListener->OnMapCreate();
    }
}

/* See header for description */
void
MapView::CCC_Shutdown(const MapViewTask* /*pTask*/)
{
    if (m_pScreenshotInProgress)
    {
        *m_pScreenshotInProgress = false;
    }

    // Clear the pointers of the CCC callbacks.
    m_pTileCallback->SetMapView(NULL);
    m_pLayerCallback->SetMapView(NULL);
    m_pCommonMaterialCallback->SetMapView(NULL);

    m_layerManager->UnregisterListener(this);
    // @todo: Need to cancel all outstanding requests and signal shutdown event.
    //        Currently all MapView instances share the same UnifiedLayer instances, in this
    //        way, if we call UnifiedLayerManager::RemoveAllTiles() here, it may also cancel
    //        tile requests issued by other instance too. This should be addressed by
    //        updating logic of Layer/TileManager/TileRequest, and it needs to be discussed
    //        and approved by architecture and will take longer time. For now, just disable it.
    // m_layerManager->RemoveAllTiles();

    // Now trigger the render thead shutdown, this well trigger the UI shutdown
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_Shutdown));
}

/* See header for description */
void
MapView::CCC_ShutdownComplete(const MapViewTask* /*pTask*/)
{
    // Get the destroy callback from MapView so that we can still call it once MapView is completely destroyed.
    shared_ptr<AsyncCallback<void*> > destroyCallback(m_initialize.m_destroyCallback);

    // This is the only place that modifies m_isValid except constructor, so no lock is
    // needed. If necessary, use atomic operations here. This gets reset to ensure that any outstanding tasks don't call the destroyed MapView instance.
    *(m_initialize.m_isValid) = false;

    // Now we can delete the map instance itself
    delete this;

    // Callback to application to inform of complete destruction of MapView.
    destroyCallback->Success(NULL);
}

/* See header for description */
void
MapView::CCC_UpdateFrame(const MapViewTask* pTask)
{
    m_frameUpdated = true;

    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_initialize.IsRenderingAllowed())
    {
        return;
    }

    const MapViewTaskUpdateFrame* task = static_cast<const MapViewTaskUpdateFrame*>(pTask);
    if (!task)
    {
        return;
    }

    //@note: please pay attention to the sequence of calling Frame::UpdteTiles() and
    //       Frame::CheckForTilesToUnload(). A special flag is set in Frame::UpdteTiles()
    //       and this flag is used in Frame::CheckForTilesToUnload() to check if some tiles
    //       should be unloaded or not. So Frame::CheckForTilesToUnload() should be called
    //       after Frame::UpdteTiles();

    vector<TileKeyPtr> newTiles;
    vector<TileKeyPtr> newTilesNonOverlay;

    // Update the frame. This calculates (and returns) any new tiles needed.
    // For some cases, we may just need to calculate tiles, but don't need to download,
    // just return if so.
    if (!m_frame.UpdateTiles(newTiles, newTilesNonOverlay) || !m_getTileWithFrame)
    {
        return;
    }

    // Check if we need to unload any tiles from NBGM
    vector<TilePtr> tiles;
    if (m_frame.CheckForTilesToUnload(tiles) && m_getTileWithFrame)
    {
        /* Trigger task to unload the tile to NBGM */
        MapViewTaskUnloadTiles* pTaskUnload = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
        if (pTaskUnload)
        {
            // Add the tiles to the task object and add the task
            pTaskUnload->m_tilesToUnload = tiles;

            // TODO: The purpose of clearing tiles is to avoid a unexpected crash caused by shared ptr.
            // We need to find the root reason if possible.(Bug 185283)
            tiles.clear();

            AddRenderThreadTask(pTaskUnload);
        }
    }

    /*
      We have to cancel any outstanding tile requests, if we switch over from 3D vector view to raster, or vice versa.
      This allows us to load the current view better since layers have different download priorities.

      We also do this if we switch from one to the next raster layer
    */

    uint32 currentZoom = (uint32)(m_frame.GetZoom());

    if ((m_lastZoom != 0) && (m_lastZoom != currentZoom))
    {
        // If zoom level changed, Walk through all layers, and cancel request if
        // currentZoom is not in [minZoom, maxZoom] of that layer.
        CancelRequestFunctor functor(currentZoom);
        for_each (m_layers.begin(), m_layers.end(), functor);
    }
    m_lastZoom = currentZoom;

    if (firstUse && !newTiles.empty())
    {
        requestedTiles.clear();
        vector<TileKeyPtr>::const_iterator iter = newTiles.begin();
        vector<TileKeyPtr>::const_iterator end  = newTiles.end();
        for (; iter != end; ++iter)
        {
            if (*iter)
            {
                requestedTiles.insert(**iter);
            }
        }
    }

    if (!m_getTileWithFrame)
    {
        return;
    }

    /*
      In order to avoid flickering of the "higher zoom level tiles", in the case where
      we have the tile for the current zoom level cached, we retrieve any cached tiles
      from the LayerManager here. If we have any cached tiles then we pass them to NBGM
      before requesting the "higher zoom level tiles".

      This code does still request all the tiles, even if they are cached. The reason
      for this, is that we might just receive partial cached tiles. In that case it
      would require a lot more work to exclude these tiles from the request. Especially,
      since we don't know which layers we have and which we don't.

      We just ignores tiles, if it already have them, so the overhead shouldn't be too
      high.
    */
    NB_Error result = m_layerManager->GetCachedTiles(newTiles, m_pTileCallback);
    if (result != NE_OK)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "MapView::CCC_UpdateFrame: Failed to get cached tiles.");
    }

    /*
      Now request the tiles
    */
    vector<LayerPtr> emptyVector;

    if (! newTilesNonOverlay.empty())
    {
#ifdef LOG_TILE_REQUEST
        //Log Request tiles excluding any overlay layers
        vector<TileKeyPtr>::const_iterator iter = newTilesNonOverlay.begin();
        for (; iter < newTilesNonOverlay.end(); ++iter)
        {
            const TileKeyPtr& key = *iter;
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "[map_tile_perf] [new_tiles] x:%d,y:%d,z:%d", key->m_x, key->m_y, key->m_zoomLevel);
        }
#endif
        // Request tiles excluding any overlay layers
        NB_Error result = m_layerManager->GetTiles(newTilesNonOverlay, m_pTileCallback, emptyVector, emptyVector, false);
        if (result != NE_OK)
        {
            return;
        }
    }

    if (! newTiles.empty())
    {
#ifdef LOG_TILE_REQUEST
        //Log Request the new tiles
        vector<TileKeyPtr>::const_iterator iter = newTiles.begin();
        for (; iter < newTiles.end(); ++iter)
        {
            const TileKeyPtr& key = *iter;
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "[map_tile_perf] [new_tiles] x:%d,y:%d,z:%d", key->m_x, key->m_y, key->m_zoomLevel);
        }
#endif

        // Request the new tiles
        NB_Error result = m_layerManager->GetTiles(newTiles, m_pTileCallback, emptyVector, emptyVector, true);
        if (result != NE_OK)
        {
            return;
        }
        // Request animation tiles, if we have any animation layers
        if (! m_CCCAnimationFrameLists.empty())
        {
            for (map<LayerPtr, vector<uint32> >::iterator iterator = m_CCCAnimationFrameLists.begin(); iterator != m_CCCAnimationFrameLists.end(); ++iterator)
            {
                m_layerManager->GetAnimationTiles(iterator->first, newTiles, iterator->second, m_pTileCallback);
            }
        }

        // Update Traffic Tip if necessary
        if (m_trafficLayerAvailable)
        {
            CCC_CheckTrafficStatus(NULL);
        }
    }
}

/* See header for description */
void
MapView::CCC_GetSpecifiedTiles(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_layerManager)
    {
        return;
    }

    const MapViewTaskGetSpecifiedTiles* task =
            static_cast<const MapViewTaskGetSpecifiedTiles*>(pTask);
    if (task)
    {
        m_layerManager->GetTiles(task->m_tileKeys, m_pTileCallback,
                                 task->m_layerList, task->m_ignoreList);
    }
}

/* See header for description */
void MapView::CCC_DropPin(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskDropPin* task = static_cast<const MapViewTaskDropPin*>(pTask);
    if (task && m_pinCushion)
    {
        /*
            @todo: m_droppedPinId is never used, is this obsolete?
         */

        m_droppedPinId = m_pinCushion->DropPin(task->m_latitude, task->m_longitude);
    }
}

/* See header for description */
void MapView::CCC_ChangeStaticPoiBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskAddStaticPoiBubble* task = static_cast<const MapViewTaskAddStaticPoiBubble*>(pTask);
    if (task && m_pinCushion)
    {
        if (m_staticPoiBubble.get())
        {
            shared_ptr<vector<shared_ptr<BubbleInterface> > > bubbles(new vector<shared_ptr<BubbleInterface> >());
            bubbles->push_back(m_staticPoiBubble);
            m_UITaskQueue->AddTask(new MapViewTaskRemoveBubble(this,
                &MapView::UI_RemoveBubble,
                bubbles));
        }

        m_staticPoiVisible = false;
        // Note bubble resolver is thread-safe(by user)
        m_staticPoiBubble = m_pinCushion->GetPinBubbleResolver()->GetStaticPoiBubble(task->m_id, task->m_name, task->m_latitude, task->m_longtitude);
        // Show bubble. Bubble position can only get in render thread.
        m_renderPending = true;
    }
}

/* See header file for description */
void
MapView::CCC_CheckAndLoadTile(const MapViewTask* pTask)
{
    // This function get called in CCC thread.
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() )
    {
        return;
    }

    const MapViewTaskCheckAndLoadTiles* task =
            static_cast<const MapViewTaskCheckAndLoadTiles*>(pTask);
    if (!task || !task->m_tileToCheck)
    {
        return;
    }

    TileRequestSuccess(task->m_tileToCheck);
}

/* See header file for description */
void MapView::CCC_RefreshAllTiles(const MapViewTask* /*pTask*/)
{
    if (!m_initialize.IsShuttingDown() &&
        m_initialize.IsDone() &&
        m_initialize.IsRenderingAllowed())
    {
        RefreshAllTiles();
    }
}

void MapView::CCC_RefreshRasterTiles(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() ||
        !m_initialize.IsDone() ||
        !m_initialize.IsRenderingAllowed())
    {
        return;
    }

   // Get raster tiles to unload and remove them from class Frame.
    vector<TilePtr> tilesToUnload;
    m_frame.UnloadAllRasterTiles(tilesToUnload);
    if(!tilesToUnload.empty())
    {
        MapViewTaskUnloadTiles* taskToUnloadTiles = new MapViewTaskUnloadTiles(this, &MapView::RenderThread_UnloadTilesToNBGM);
        if (taskToUnloadTiles)
        {
            taskToUnloadTiles->m_tilesToUnload = tilesToUnload;
            AddRenderThreadTask(taskToUnloadTiles);
        }
    }

    vector<TileKeyPtr> tiles;
    m_frame.GetCurrentTiles(tiles);

    // This should only be called in CCC thread, no need to switch thread.
    if (! tiles.empty())
    {
        vector<LayerPtr> emptyVector;
        m_layerManager->GetTiles(tiles, m_pTileCallback, emptyVector, emptyVector);
    }
}

/* See header file for description */
void
MapView::CCC_GetLayers(const MapViewTask* /*pTask*/)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CCC_GetLayers", this);
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    // Request layers. See callback for result. Only once it has succeeded can we start requesting tiles from the server.
    // The flag m_layerManagerInitialized will be set once we successfully received the layers.
    NB_Error error = m_layerManager->GetLayers(m_pLayerCallback, m_pMetadataConfig);
    if (error != NE_OK)
    {
        /* Add a CCC task to request metadata again. It could avoid infinite
           loop and cost CPU.
        */
        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_GetLayers));
    }

}

/* See header file for description */
void
MapView::CCC_RequestCommonMaterials(const MapViewTask* /*pTask*/)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CCC_RequestCommonMaterials", this);
    if (m_initialize.IsShuttingDown() || !m_initialize.CommonMaterialsReadyToRequest())
    {
        return;
    }

    shared_ptr<string> type;

    if(m_isNight)
    {
        type = shared_ptr<string>(new string(MATERIAL_NIGHT_TIME));
    }
    else
    {
        type = shared_ptr<string>(new string(MATERIAL_DAY_TIME));
    }

    m_layerManager->GetCommonMaterials(m_pCommonMaterialCallback, type);
}

/* See header file for description */
void
MapView::CCC_SetTheme(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown()  || !m_initialize.IsDone() || !m_initialize.CommonMaterialsReadyToRequest())
    {
        return;
    }

    const MapViewTaskSetTheme *themeTask = static_cast<const MapViewTaskSetTheme*>(pTask);
    if(!themeTask)
    {
        return;
    }

    MapViewTheme theme = themeTask->m_theme;
    if(theme >= MVT_MAX)
    {
        return;
    }

    shared_ptr<string> type;
    switch (theme)
    {
        case MVT_DAY:
            type = shared_ptr<string>(new string(MATERIAL_DAY_TIME));
            m_isNight = false;
            break;
        case MVT_NIGHT:
            type = shared_ptr<string>(new string(MATERIAL_NIGHT_TIME));
            m_isNight = true;
            break;
        default:
            break;
    }

    if (type)
    {
        m_layerManager->GetCommonMaterials(m_pCommonMaterialCallback, type);
    }

    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskChangeNightMode(this, &MapView::UI_ChangeNightMode, m_isNight));
    }
}

/* See header file for description */
void
MapView::CCC_ShowPinBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskShowBubbleById* showBubbleByIdTask = static_cast<const MapViewTaskShowBubbleById*> (pTask);
    if (!showBubbleByIdTask)
    {
        return;
    }

    // Get the pin by pin ID.
    PinPtr pin = CCC_GetPin(showBubbleByIdTask->m_pinId);
    if (!pin)
    {
        return;
    }

    // Add an UI task to show the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskShowBubbleByPin(this,
                                                              &MapView::UI_ShowPinBubble,
                                                              pin,
                                                              showBubbleByIdTask->m_x,
                                                              showBubbleByIdTask->m_y,
                                                              showBubbleByIdTask->m_orientation));
    }
}

/* See header file for description */
void
MapView::CCC_HidePinBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskHideBubbleById* hideBubbleByIdTask = static_cast<const MapViewTaskHideBubbleById*> (pTask);
    if (!hideBubbleByIdTask)
    {
        return;
    }

    // Get the pin by pin ID.
    PinPtr pin = CCC_GetPin(hideBubbleByIdTask->m_pinId);
    if (!pin)
    {
        return;
    }

    // Add an UI task to hide the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskHideBubbleByPin(this,
                                                              &MapView::UI_HidePinBubble,
                                                              pin));
    }
}

/* See header file for description */
void
MapView::CCC_UpdatePinBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskShowBubbleById* updateBubbleByIdTask = static_cast<const MapViewTaskShowBubbleById*> (pTask);
    if (!updateBubbleByIdTask)
    {
        return;
    }

    // Get the pin by pin ID.
    PinPtr pin = CCC_GetPin(updateBubbleByIdTask->m_pinId);
    if (!pin)
    {
        return;
    }

    // Add an UI task to update the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskShowBubbleByPin(this,
                                                              &MapView::UI_UpdatePinBubble,
                                                              pin,
                                                              updateBubbleByIdTask->m_x,
                                                              updateBubbleByIdTask->m_y,
                                                              updateBubbleByIdTask->m_orientation));
    }
}

/* See header file for description */
void
MapView::CCC_ShowStaticPoiBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_staticPoiBubble)
    {
        return;
    }

    const MapViewTaskShowStaticPoiBubble* showBubbleByIdTask = static_cast<const MapViewTaskShowStaticPoiBubble*> (pTask);
    if (!showBubbleByIdTask)
    {
        return;
    }
    // Add an UI task to show the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskShowBubble(this,
            m_staticPoiVisible ? &MapView::UI_UpdateBubble : &MapView::UI_ShowBubble,
            m_staticPoiBubble,
            showBubbleByIdTask->m_x,
            showBubbleByIdTask->m_y,
            showBubbleByIdTask->m_orientation));
        m_staticPoiVisible = true;
    }
}

/* See header file for description */
void
MapView::CCC_HideStaticPoiBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_staticPoiBubble)
    {
        return;
    }

    const MapViewTaskHideStaticPoiBubble* hideBubbleByIdTask = static_cast<const MapViewTaskHideStaticPoiBubble*> (pTask);
    if (!hideBubbleByIdTask)
    {
        return;
    }

    // Add an UI task to hide the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskHideBubble(this,
            &MapView::UI_HideBubble,
            m_staticPoiBubble));
        m_staticPoiVisible = false;
    }
}

/* See header file for description */
void
MapView::CCC_UpdateStaticPoiBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_staticPoiBubble)
    {
        return;
    }
    const MapViewTaskShowStaticPoiBubble* updateBubbleByIdTask = static_cast<const MapViewTaskShowStaticPoiBubble*> (pTask);
    if (!updateBubbleByIdTask)
    {
        return;
    }

    // Add an UI task to update the pin bubble.
    if (m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new MapViewTaskShowBubble(this,
            &MapView::UI_UpdateBubble,
            m_staticPoiBubble,
            updateBubbleByIdTask->m_x,
            updateBubbleByIdTask->m_y,
            updateBubbleByIdTask->m_orientation));
    }
}

/* See header file for description */
void
MapView::CCC_CheckTrafficStatus(const MapViewTask* /*pTask*/)
{
    // this function get called in CCC thread.
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    TrafficTipType tipType = TTT_None;
    if (m_trafficLayerId != INVALID_LAYER_ID && m_trafficLayerAvailable)
    {
        uint32 currentZoom = (int)floor(m_frame.GetZoom());
        if (currentZoom < m_minZoomOfTrafficLayer)
        {
            tipType = TTT_ZoomIn;
        }
        else
        {
            vector<TileKeyPtr> tiles;
            m_frame.GetCurrentTiles(tiles);

            std::bitset<DI_TotalCount> damIndices;
            damIndices.set(DI_Realtime_Traffic_Partial);
            damIndices.set(DI_Realtime_Traffic_Full);
            if (m_damUtility->IsDataAvailable(tiles, damIndices) == NDV_False)
            {
                tipType = TTT_NoData;
            }
        }
    }

    if (m_lastTrafficTip != tipType)
    {
        MapViewTask* task = new UITaskUpdateTrafficTip(this,
                                                        &MapView::UI_UpdateTrafficTip,
                                                        tipType);
        if (task)
        {
            m_UITaskQueue->AddTask(task);
        }
    }

    m_lastTrafficTip = tipType;
}

// Function called in NBGM render thread ......................................................................................

/* See header for description */
void
MapView::RenderThread_Initialize(const MapViewTask* /*pTask*/)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::RenderThread_Initialize", this);
    NB_Error initResult = NE_OK;

    if (m_shouldInitRenderer)
    {
        // Initialize the 'NBGMViewController'.
        RenderThread_InitializeRenderer();

        // Check if the 'NBGMViewController' is NULL to set the initialized result.
        initResult = m_nbgmViewController ? NE_OK : NE_NOTINIT;
    }

    // Inform application level of success or failure.
    m_eventQueue->AddTask(new MapViewTaskInitialize(this, &MapView::CCC_InitializeComplete, initResult));
}

/* See header for description */
void
MapView::RenderThread_Shutdown(const MapViewTask* /*pTask*/)
{
    // Destroy the 'NBGMViewController'.
    RenderThread_DestroyRenderer();

    // Now trigger the UI shutdown
    m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_ShutdownNativeView));
}

/* See header for description */
void
MapView::RenderThread_InitializeRenderer()
{
    // Check if the 'NBGMViewController' has already been initialized.
    if (m_nbgmViewController)
    {
        return;
    }

    // Create the 'NBGMViewController'.
    m_nbgmViewController = new NBGMViewController(CreateNBGMMapView(), m_mapViewCamera);
    if (!m_nbgmViewController)
    {
        return;
    }

    m_defaultRenderListener = new MapViewRenderListener(m_nbgmViewController, this);

    // Initialize the 'NBGMViewController'.
    NB_Error result = m_nbgmViewController->Initiallize();
    if (result != NE_OK)
    {
        RenderThread_DestroyRenderer();
        return;
    }
    // Initialize to map view camera
    m_mapViewCamera->Initiallize(m_palInstance);

    // Get the full file path of the pin materials.

    m_nbgmViewController->SetBubblelistener(this);

    m_mapViewCamera->SetFrameBufferSize(m_viewSizeWidth, m_viewSizeHeight);

    // set default value into camera
    m_mapViewCamera->SetCamera(m_defaultLatitude, m_defaultLongitude, INVALID_CAMERA_HEADING, DEFAULT_ZOOM_LEVEL, m_defaultTiltAngle, m_cameraTimestamp++, false);
    RefreshFrame(m_defaultLatitude, m_defaultLongitude);

    m_initialize.SetDone();

    // Request common materials only after both this function has succeeded and LayerRequestSuccess() has been called.
    // We don't know which one gets called first!
    m_initialize.RequestCommonMaterialsIfNecessary(this);
}

/* See header for description */
void
MapView::RenderThread_DestroyRenderer()
{
    if (m_nbgmViewController)
    {
        delete m_defaultRenderListener;
        m_defaultRenderListener = NULL;
        m_nbgmViewController->Finalize();
        delete m_nbgmViewController;
        m_nbgmViewController = NULL;
    }
}

/* See header for description */
void
MapView::RenderThread_InitializeRendererWithTask(const MapViewTask* /*pTask*/)
{
    // Set the flag to true and initialize the renderer.
    m_shouldInitRenderer = true;

    if (!m_initialize.IsDone())
    {
        // Do nothing if the map view has not been initialized before.
        return;
    }

    RenderThread_InitializeRenderer();
}

/* See header for description */
void
MapView::RenderThread_DestroyRendererWithTask(const MapViewTask* pTask)
{
    // Set the flag to false and destroy the renderer.
    m_shouldInitRenderer = false;
    RenderThread_DestroyRenderer();

    const RenderTaskDestroyRenderer* task = static_cast<const RenderTaskDestroyRenderer*>(pTask);

    if (task && task->m_event)
    {
        /* Set the event to notify that the renderer has
           already been destroyed.
        */
        PAL_EventSet(task->m_event);
    }
}

/* See header for description */
void
MapView::RenderThread_NBGMRender(const MapViewTask* pTask)
{
    const MapViewTaskRender* task = static_cast<const MapViewTaskRender*>(pTask);
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        if (task)
        {
            if (task->m_pListener)
            {
                task->m_pListener->OnDiscard();
            }
        }
        return;
    }
    if (task)
    {
        MapViewRenderListener *defaultListener = dynamic_cast<MapViewRenderListener*>(m_defaultRenderListener);
        if(defaultListener)
        {
            // the default listener should wrappe the listener passed in,
            // just like a decorator
            defaultListener->SetInternalListerner(task->m_pListener);
        }

    }
    // This call calls back to MapView::UI_Render() which is circular calling. This is not very good. We should have
    // the function return a value which we respond on.
    m_nativeView->UpdateHostView();
}

/* See header file for description */
void
MapView::RenderThread_RemovePinBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() ||
        (!(m_initialize.IsDone())) ||
        (!m_nbgmViewController))
    {
        return;
    }


    const MapViewTaskRemovePinBubble* removePinBubbleTask = static_cast<const MapViewTaskRemovePinBubble*>(pTask);
    if (!removePinBubbleTask)
    {
        return;
    }

    // Get the pins to remove the bubble.
    shared_ptr<vector<PinPtr> > pinsToRemoveBubble = removePinBubbleTask->m_pins;
    if (!pinsToRemoveBubble)
    {
        return;
    }

    // Check if the current selected pin ID exists in the list of pins.
    vector<PinPtr>::const_iterator pinIterator = pinsToRemoveBubble->begin();
    vector<PinPtr>::const_iterator pinEnd = pinsToRemoveBubble->end();
    for (; pinIterator != pinEnd; ++pinIterator)
    {
        PinPtr pin = *pinIterator;
        if (pin)
        {
            shared_ptr<string> pinId = pin->GetPinID();

            if (pinId && (!(pinId->empty())))
            {
                // Get the selected pin ID from the NBGM view controller.
                shared_ptr<string> selectedPinId = m_nbgmViewController->GetSelectedPinId(*pinId);
                if (selectedPinId && !selectedPinId->empty() && (selectedPinId->compare(*pinId) == 0))
                {
                    vector<shared_ptr<string>> toRemovePins;
                    toRemovePins.push_back(selectedPinId);
                    // Remove the current selected pin.
                    m_nbgmViewController->RemovePins(toRemovePins);

                    // Add an UI task to hide the pin bubble.
                    if (m_UITaskQueue)
                    {
                        shared_ptr<vector<PinPtr> > pinsOfTask(new vector<PinPtr>(1, pin));
                        if (pinsOfTask)
                        {
                            m_UITaskQueue->AddTask(new MapViewTaskRemovePinBubble(this,
                                                                                  &MapView::UI_RemovePinBubble,
                                                                                  pinsOfTask));
                        }
                    }

                    break;
                }
            }
        }
    }
}

void
MapView::RenderThread_ResetScreen(const MapViewTask* /*pTask*/)
{
    m_nativeView->resetScreen();
    m_renderPending = true;
}

// Functions to control NBGM load and unload .....................................................................

/* See header for description */
void
MapView::RenderThread_LoadTilesToNBGM(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_initialize.IsRenderingAllowed() || (!m_nbgmViewController))
    {
        return;
    }

    const MapViewTaskLoadTiles* task = static_cast<const MapViewTaskLoadTiles*>(pTask);
    for (vector<TileWrapperPtr>::const_iterator tileInfo = task->m_tilesToLoad.begin(); tileInfo != task->m_tilesToLoad.end(); ++tileInfo)
    {
        TilePtr tile = (*tileInfo)? (*tileInfo)->m_tile : TilePtr();
        if (!tile)
        {
            continue;
        }
        // Is it an animation tile
        if (tile->GetTimeStamp() > 0)
        {
            m_frame.AnimationTileReceived(tile);
            m_renderPending = true; // Add a render task to trigger AnimationLayer playback.
        }

        // Load tile to NBGMViewController
        if (!m_nbgmViewController->LoadMapData(*tileInfo))
        {
            // Failed to load this tile, schedule a retry, it will load the tile again
            MapViewTask* checkTileTask = new MapViewTaskCheckAndLoadTiles(this, &MapView::CCC_CheckAndLoadTile, tile);
            if (checkTileTask)
            {
                m_eventQueue->AddTask(checkTileTask);
            }
        }
    }
    m_nbgmViewController->LoadAndUnloadTiles();
}

/* See header for description */
void
MapView::RenderThread_UnloadTilesToNBGM(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_initialize.IsRenderingAllowed() || (!m_nbgmViewController))
    {
        return;
    }

    const MapViewTaskUnloadTiles* task = static_cast<const MapViewTaskUnloadTiles*>(pTask);
    if (task && !task->m_tilesToUnload.empty())
    {
        // Inform any animation layers that tiles have been removed
        m_frame.AnimationTilesRemoved(task->m_tilesToUnload);

        m_nbgmViewController->RemoveMapData(task->m_tilesToUnload);
        m_nbgmViewController->LoadAndUnloadTiles();
    }
}

/* See header for description */
void
MapView::RenderThread_ReloadTiles(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() ||
        !m_initialize.IsRenderingAllowed() || (!m_nbgmViewController))
    {
        return;
    }

    const RenderTaskReloadTiles* task = static_cast<const RenderTaskReloadTiles*>(pTask);
    if (task && !task->m_tilesToReload.empty())
    {
        m_nbgmViewController->ReloadTiles(task->m_tilesToReload);
    }
}

/* See header for description */
void
MapView::RenderThread_UnloadAllTilesFromNBGM(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() ||
        (!(m_initialize.IsDone())) ||
        (!(m_initialize.IsRenderingAllowed())) ||
        (!m_nbgmViewController))
    {
        return;
    }

    const MapViewTaskUnloadTiles* taskToUnloadTiles = static_cast<const MapViewTaskUnloadTiles*>(pTask);
    if (taskToUnloadTiles &&
        (!(taskToUnloadTiles->m_tilesToUnload.empty())))
    {
        // Inform any animation layers that tiles have been removed.
        m_frame.AnimationTilesRemoved(taskToUnloadTiles->m_tilesToUnload);
    }

    // Unload all tiles from NBGM containing NBGM cache.
    m_nbgmViewController->UnloadAllTiles();
}

/* See header for description */
void
MapView::RenderThread_UpdateCommonMaterials(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || (!m_nbgmViewController))
    {
        return;
    }

    const MapViewTaskUpdateCommonMaterial* task = static_cast<const MapViewTaskUpdateCommonMaterial*>(pTask);
    if (task && !task->m_tileToLoad.empty())
    {
        m_nbgmViewController->UpdateCommonSettings(task->m_tileToLoad,task->isDay, task->isSatellite);
        if(m_refreshTilesAfterCommonMaterialUpdated)
        {
            m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_RefreshAllTiles));
            m_refreshTilesAfterCommonMaterialUpdated = false;
        }
    }
}

/* See header for description */
void
MapView::RenderThread_HandleAnimationLayer(const MapViewTask* pTask)
{
    const MapViewTaskAnimationLayer* task = static_cast<const MapViewTaskAnimationLayer*>(pTask);
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || (!m_nbgmViewController))
    {
        return;
    }

    if (task->m_layerAdded)
    {
        uint32 layerID = task->m_layerAdded->GetID();

        // Add the layer to the frame
        m_frame.AddAnimationLayer(layerID);

        // Create the layer in NBGM
        m_nbgmViewController->AddAnimationLayer(layerID);
    }
    else if (task->m_layerRemoved)
    {
        uint32 layerID = task->m_layerRemoved->GetID();

        // Remove layer from the frame class
        m_frame.RemoveAnimationLayer(layerID);

        // Delete the layer in NBGM
        m_nbgmViewController->RemoveAnimationLayer(layerID);
    }
    else if (task->m_layerUpdated)
    {
        vector<shared_ptr<string> > tilesToUnload;

        // Update frame list
        m_frame.UpdateAnimationFrameList(task->m_layerUpdated->GetID(), task->m_sortedFrameList, tilesToUnload);

        if (! tilesToUnload.empty())
        {
            // Unload tiles from NBGM
            m_nbgmViewController->UnloadAnimationTiles(task->m_layerUpdated->GetID(), tilesToUnload);
        }
    }
}

/* See header for description */
void
MapView::RenderThread_UpdateAnimationLayerParameters(const MapViewTask* pTask)
{
    const RenderTaskUpdateAnimationParameters* task = static_cast<const RenderTaskUpdateAnimationParameters*>(pTask);
    m_frame.SetAnimationParameters(task->m_parameters);
}

/* See header for description */
void
MapView::RenderThread_UpdateAnimationLayerOpacity(const MapViewTask* pTask)
{
    if (m_nbgmViewController)
    {
        const RenderTaskUpdateOpacity* task = static_cast<const RenderTaskUpdateOpacity*>(pTask);
        m_nbgmViewController->SetAnimationLayerOpacity(task->m_opacity);
        m_renderPending = true;
    }
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetGpsMode(const MapViewTask* pTask)
{
    const RenderTaskSetGpsMode* task = static_cast<const RenderTaskSetGpsMode*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    // Check if this GPS mode has already been set.
    if (task->m_gpsMode == m_gpsMode)
    {
        return;
    }

    // Set current GPS mode.
    m_gpsMode = task->m_gpsMode;

    // Set avatar mode to NBGM.
    switch (m_gpsMode)
    {
        case NGM_INVALID:
        {
            break;
        }
        case NGM_STAND_BY:
        {
            break;
        }

        case NGM_FOLLOW_ME:
        {
            m_nbgmViewController->EnableLocationBubble(false);
            break;
        }
        default:
        {
            break;
        }

    }
    m_renderPending = true;
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetAvatarState(const MapViewTask* /*pTask*/)
{
    //const RenderTaskSetAvatarState* task = static_cast<const RenderTaskSetAvatarState*>(pTask);

    //if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    //{
    //    return;
    //}

    //// Initialize the avatar state according to current GPS mode.
    //NBGM_AvatarState avatarState = NBGM_AS_DISABLED;
    //switch (m_gpsMode)
    //{
    //    case MVGM_INVALID:
    //        avatarState = NBGM_AS_DISABLED;
    //        break;

    //    case MVGM_STAND_BY:
    //    case MVGM_FOLLOW_ME:
    //        avatarState = NBGM_AS_MAP_STAND_BY;
    //        break;

    //    default:
    //        break;
    //}

    //if (task->m_headingValid)
    //{
    //    if (m_gpsMode == MVGM_FOLLOW_ME)
    //    {
    //        avatarState = NBGM_AS_MAP_FOLLOW_ME;
    //    }
    //}

    //m_nbgmViewController->SetAvatarState(avatarState);
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetAvatarMode(const MapViewTask* pTask)
{
    const RenderTaskSetAvatarMode* task = static_cast<const RenderTaskSetAvatarMode*>(pTask);


    if(task->m_avatarMode == m_avatarMode)
    {
        return;
    }

    m_avatarMode = task->m_avatarMode;

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    RefreshAvatarMode();

    m_renderPending = true;
}

void
MapView::RefreshAvatarMode()
{

    NBGM_AvatarState avatarState = NBGM_AS_DISABLED;
    NBGM_AvatarMode avatarMode = NBGM_AM_MAP_FOLLOW_ME;
    switch (m_avatarMode)
    {
        case MVAM_NONE:
            avatarState = NBGM_AS_DISABLED;
            break;
        case MVAM_MAP:
            if(m_lastGpsLocation.heading == INVALID_CAMERA_HEADING)
            {
                avatarState = NBGM_AS_MAP_STAND_BY;
            }
            else
            {
                avatarState = NBGM_AS_MAP_FOLLOW_ME;
            }
            avatarMode = NBGM_AM_MAP_FOLLOW_ME;
            break;

        case MVAM_ARROW:
            avatarState = NBGM_AS_NAV;
            avatarMode = NBGM_AM_NAV_BYCICLY;
            break;
        case MVAM_CAR:
            avatarState = NBGM_AS_NAV;
            avatarMode = NBGM_AM_NAV_CAR;
            break;

        default:
            break;
    }

    m_nbgmViewController->SetAvatarState(avatarState);
    m_nbgmViewController->SetAvatarMode(avatarMode);

}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetAvatarLocation(const MapViewTask* pTask)
{
    const RenderTaskSetAvatarLocation* task = static_cast<const RenderTaskSetAvatarLocation*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController) || (!m_UITaskQueue))
    {
        return;
    }

    const NB_GpsLocation& gpsLocation = task->m_gpsLocation;
    uint32 gpsValid = gpsLocation.valid;

    // Check if the coordinate is valid.
    if (((gpsValid & NGV_Latitude) == 0) || ((gpsValid & NGV_Longitude) == 0))
    {
        return;
    }

    if (((m_lastGpsLocation.valid & NGV_Latitude) == 0) || ((m_lastGpsLocation.valid & NGV_Longitude) == 0) || ((gpsLocation.gpsTime - m_lastGpsLocation.gpsTime) > NIGHT_MODE_CHECK_INTERVAL/1000))
    {
        m_lastGpsLocation = gpsLocation;
        if (m_nightMode == MVNM_AUTO)
        {
            m_UITaskQueue->AddTask(new MapViewTask(this, &MapView::UI_CalculateNightMode));
        }
    }
    else
    {
        m_lastGpsLocation = gpsLocation;
    }

    // Transfrom coordinate to mercator.
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(gpsLocation.latitude, gpsLocation.longitude, &mercatorX, &mercatorY);

    // Set mercator values to the NBGM location.
    NBGM_Location64 nbgmLocation;
    memset(&nbgmLocation, 0, sizeof(nbgmLocation));
    nbgmLocation.position.x = mercatorX;
    nbgmLocation.position.y = mercatorY;
    nbgmLocation.position.z = 0.0;

    // Set the GPS time.
    nbgmLocation.time = static_cast<uint64>(gpsLocation.gpsTime);

    // Set the speed if valid.
    if (gpsValid & NGV_HorizontalVelocity)
    {
        nbgmLocation.speed = gpsLocation.horizontalVelocity;
    }

    // Set the accuracy if valid.
    if (gpsValid & NGV_HorizontalUncertainty)
    {
        // Simply convert it to mercator and set to NBGM location.
        nbgmLocation.accuracy = gpsLocation.horizontalUncertaintyAlongAxis;
    }

    nbgmLocation.heading = gpsLocation.heading;

    RefreshAvatarMode();

    m_nbgmViewController->SetAvatarLocation(nbgmLocation);

    /*check if the avatar is in screen. If it is not in screen, it need render one more time.
    Because if stop render immediately, half avatar may show on the screen all the time.*/
    if ( !m_nbgmViewController->IsAvatarInScreen(static_cast<float>(mercatorX), static_cast<float>(mercatorY)))
    {
        if ( m_avatarRenderOneTimeOutofScreen )
        {
            return;
        }
        else
        {
            m_avatarRenderOneTimeOutofScreen = true;
        }
    }
    else
    {
        m_avatarRenderOneTimeOutofScreen = false;
    }

    m_renderPending = true;
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetCustomAvatar(const MapViewTask* pTask)
{
    const RenderTaskSetCustomAvatar* task = static_cast<const RenderTaskSetCustomAvatar*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    if (task && task->m_directionalAvatarPath && task->m_directionlessAvatarPath)
    {
        m_nbgmViewController->SetCustomAvatar(task->m_directionalAvatarPath,
                                              task->m_directionlessAvatarPath,
                                              task->m_directionalCalloutOffsetX,
                                              task->m_directionalCalloutOffsetY,
                                              task->m_directionlessCalloutOffsetX,
                                              task->m_directionlessCalloutOffsetY);
    }
    else if (task && task->m_directionalAvatar && task->m_directionlessAvatar)
    {
        m_nbgmViewController->SetCustomAvatar(task->m_directionalAvatar,
                                              task->m_directionlessAvatar,
                                              task->m_directionalCalloutOffsetX,
                                              task->m_directionalCalloutOffsetY,
                                              task->m_directionlessCalloutOffsetX,
                                              task->m_directionlessCalloutOffsetY);
    }
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetCustomAvatarHaloStyle(const MapViewTask* pTask)
{
    const RenderTaskSetCustomAvatarHaloStyle* task = static_cast<const RenderTaskSetCustomAvatarHaloStyle*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    if (task && task->m_haloStyle)
    {
        m_nbgmViewController->SetCustomAvatarHaloStyle((uint32)task->m_haloStyle->m_edgeColor, (uint32)task->m_haloStyle->m_fillColor,
                                                       task->m_haloStyle->m_edgeSize, (NBGM_HaloEdgeStyle)task->m_haloStyle->m_edgeStyle);
    }

    m_renderPending = true;
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetBackground(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    const RenderTaskSetBackground* task = static_cast<const RenderTaskSetBackground*>(pTask);


    m_nbgmViewController->SetBackground(task->m_background);

    PAL_EventSet(task->m_event);
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetViewSettings(const MapViewTask* pTask)
{
    const RenderTaskSetViewSettings* task = static_cast<const RenderTaskSetViewSettings*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    switch (task->m_viewSetting)
    {
        case MVS_FAR_NEAR_VISIBILITY:
            m_nbgmViewController->EnableFarNearVisibility(task->m_flag);
            break;

        case MVS_SHOW_DEBUG_VIEW:

            /* This is already handled in the UI thread. We should not handle this here! */
            nsl_assert(false);
            break;

        default:
            break;
    }
}

/*! See corresponding function in MapViewInterface.h */
void
MapView::RenderThread_SetCameraSettings(const MapViewTask* pTask)
{
    const RenderTaskSetCameraSettings* task = static_cast<const RenderTaskSetCameraSettings*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_mapViewCamera))
    {
        return;
    }

    MapViewCamera::AnimationAccelerationType type = MapViewCamera::AAT_LINEAR;
    switch(task->m_type)
    {
    case MVCAAT_LINEAR:
        type = MapViewCamera::AAT_LINEAR;
        break;
    case MVCAAT_DECELERATION:
        type = MapViewCamera::AAT_DECELERATION;
        break;
    default:
        break;
    }

    m_mapViewCamera->SetCamera(task->m_latitude, task->m_longitude, task->m_heading, task->m_zoomLevel, task->m_tilt, task->m_timestamp, task->m_animated, task->m_duration, type, task->m_id);
    SetFrameZoomAndCenter();
    m_renderPending = true;
}

/* See header for description */
void
MapView::RenderThread_SetCurrentPosition(const MapViewTask* pTask)
{
    if (m_mapViewCamera)
    {
        const RenderTaskSetCurrentPosition* task = static_cast<const RenderTaskSetCurrentPosition*>(pTask);

        double lat = task->m_latitude;
        double lon = task->m_longitude;
        m_mapViewCamera->SetCurrentPositon(lat, lon, false, false);
        SetFrameZoomAndCenter();
    }
}

/* See header for description */
void
MapView::RenderThread_SetViewPort(const MapViewTask* pTask)
{
    if (m_nbgmViewController && m_nativeView)
    {
        const RenderTaskSetViewPort* task = static_cast<const RenderTaskSetViewPort*>(pTask);
        //m_nbgmViewController->SetFrameBufferSize(task->m_width, task->m_height);
        m_nativeView->SetViewSize(task->m_width, task->m_height);
        SetFrameZoomAndCenter();
    }
}

/* See header for description */
void
MapView::RenderThread_SetScreenOrientation(const MapViewTask* /*pTask*/)
{
}

void
MapView::RenderThread_SetFrameZoomAndCenter(const MapViewTask* /*pTask*/)
{
    SetFrameZoomAndCenter();
}

void MapView::RenderThread_ModifyExternalMaterial(const MapViewTask* pTask)
{
    const RenderTaskModifyExternalMaterial* task =
            static_cast<const RenderTaskModifyExternalMaterial*>(pTask);

    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_nbgmViewController || !task)
    {
        return;
    }
    switch (task->m_type)
    {
        case RenderTaskModifyExternalMaterial::EMMT_Load:
        {
            m_nbgmViewController->LoadExternalMaterial(task->m_material);
            break;
        }

        case RenderTaskModifyExternalMaterial::EMMT_Unload:
        {
            m_nbgmViewController->UnloadExternalMaterial(task->m_material);
            break;
        }

        case RenderTaskModifyExternalMaterial::EMMT_Activate:
        {
            m_nbgmViewController->ActivateExternalMaterial(task->m_material);
            break;
        }
        default:
        {
            break;
        }
    }
}

void
MapView::RenderThread_RunCombinedAtomicTask(const MapViewTask* pTask)
{
    const CombinedAtomicTask* task = static_cast<const CombinedAtomicTask*>(pTask);
    task->ExcuteAllTasks();
}

// Function called in UI thread ............................................................

/* See header for description */
void
MapView::UI_InitializeNativeView(const MapViewTask* /*pTask*/)
{
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::UI_InitializeNativeView", this);
    /*
        Important:
        All return path' have to trigger the callback to the application level (with some exceptions, see below)!
     */

    m_nativeView = new MapNativeView(this, m_zorder, m_mapViewContext ? true : false, m_pRenderingQueue->GetRunningThreadId());
    if (!m_nativeView)
    {
        // Inform application level of failure
        m_eventQueue->AddTask(new MapViewTaskInitialize(this, &MapView::CCC_InitializeComplete, NE_NOMEM));
        return;
    }

    /*
        Important:
        The SetMapViewProtocol fucntion need invoke before Initialize called. For android project.
     */
    if (m_mapViewContext)
    {
        m_nativeView->SetMapViewProtocol(m_mapViewContext);
    }
}

void
MapView::UI_MapNativeViewCreated()
{
    nb_threadId renderingThreadId = this->m_pRenderingQueue->GetRunningThreadId();
    NB_Error result = m_nativeView->Initialize(m_viewSize.width,
                                               m_viewSize.height,
                                               renderingThreadId,
                                               m_palInstance);
    if (result != NE_OK)
    {
        // This function can fail with NE_AGAIN (e.g. on Android platforms) because the render context is not yet initialized.
        // If this happens then the function UI_InitializeMapView() needs to be called to finish initialization.
        if (result == NE_AGAIN)
        {
            // Don't trigger callback here, since we're waiting for the UI to callback
            return;
        }

        // Inform application level of failure
        m_eventQueue->AddTask(new MapViewTaskInitialize(this, &MapView::CCC_InitializeComplete, result));
        return;
    }

    /*
        Now that we have the UI initialized, we can now initialize NBGM. Once NBGM is initialized then we're done
        with the initialization.
        The second parameter meaning priority of task in the task queue. 0 is highest.
        When display the trip summary map view during the navigation, set high priority can
        speed up the function 'RenderThread_Initialize' invoke and display map view ASAP.
     */
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_Initialize), 0);


    // Don't trigger callback here, since RenderThread_Initialize() will trigger it
}

/* See header for description */
void
MapView::UI_ShutdownNativeView(const MapViewTask* /*pTask*/)
{
    if (m_nativeView)
    {
        m_nativeView->Finallize();
    }
    if (m_nativeView)
    {
        delete m_nativeView;
    }

    // Switch to CCC to trigger callback to Application level and destroy MapView
    m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_ShutdownComplete));
}

/* See header for description */
void
MapView::UI_OutputDebugUI(const MapViewTask* pTask)
{
    const MapViewTaskUpdateDebugUI* task = static_cast<const MapViewTaskUpdateDebugUI*>(pTask);

    // Update the debug UI
    m_nativeView->SetDebugInfo(task->m_debugStream.str().c_str());

    if (! task->m_fileHeader.str().empty())
    {
        // Write gps file name
        stringstream filestream;
        filestream << "GPS FileName: " << m_gpsFileName;
        WriteDebugToFile(filestream.str().c_str());

        // Write header
        WriteDebugToFile(task->m_fileHeader.str().c_str());
    }

    if (! task->m_fileStream.str().empty())
    {
        WriteDebugToFile(task->m_fileStream.str().c_str());
    }
}

/* See header for description */
void
MapView::UI_HandleAnimationLayer(const MapViewTask* pTask)
{
    const MapViewTaskUIAnimationLayer* task = static_cast<const MapViewTaskUIAnimationLayer*>(pTask);
    if ((! task) || (!m_nativeView) || m_initialize.IsShuttingDown())
    {
        return;
    }

    /*
        Create/Remove or update the timestamp in the UI.
     */

    if (task->m_createTimestampLabel)
    {
        m_nativeView->CreateTimestampLabel();
    }
    else if (task->m_removeTimestampLabel)
    {
        m_nativeView->RemoveTimestampLabel();
    }
    else
    {
        m_nativeView->UpdateTimestampLabel(task->m_timestamp);
    }

    m_nativeView->UpdateButtonState(task->m_animationCanPlay);
}

/* See header for description */
void
MapView::UI_ShowMapLegend(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_nativeView)
    {
        return;
    }

    const MapViewTaskShowMapLegend* task = static_cast<const MapViewTaskShowMapLegend*>(pTask);
    m_nativeView->ShowMapLegend(task->m_pMapLegend);
}

/* See header for description */
void
MapView::UI_HideMapLegend(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() || !m_nativeView)
    {
        return;
    }

    m_nativeView->HideMapLegend();
}

/* See header for description */
void
MapView::UI_UpdateTrafficTip(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_nativeView)
    {
        return;
    }

    const UITaskUpdateTrafficTip* task = static_cast<const UITaskUpdateTrafficTip*>(pTask);
    if (task)
    {
        m_nativeView->UpdateTrafficTip(task->m_tipType);
    }
}

/* See header for description */
void MapView::UI_UpdateLocationBubble(const MapViewTask* pTask)
{
    // This fucntion get called in UI thread.
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const UITaskUpdateLocationBubble* task = static_cast<const UITaskUpdateLocationBubble*>(pTask);
    if (task && m_nativeView)
    {
        m_nativeView->ShowLocationBubble(task->m_x, task->m_y, task->m_orientation,
                                         task->m_latitude, task->m_longitude);
    }
}

/* See header for description */
void MapView::UI_HideLocationBubble(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    if (pTask && m_nativeView)
    {
        m_nativeView->HideLocationBubble();
    }
}

/* See header file for description */
void
MapView::UI_ShowPinBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskShowBubbleByPin* showBubbleByPinTask = static_cast<const MapViewTaskShowBubbleByPin*> (pTask);
    if (!showBubbleByPinTask)
    {
        return;
    }

    // Show the pin bubble in the native map view.
    m_nativeView->ShowPinBubble(showBubbleByPinTask->m_pin,
                                showBubbleByPinTask->m_x,
                                showBubbleByPinTask->m_y,
                                showBubbleByPinTask->m_orientation);
}

/* See header file for description */
void
MapView::UI_HidePinBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskHideBubbleByPin* hideBubbleByPinTask = static_cast<const MapViewTaskHideBubbleByPin*> (pTask);
    if (!hideBubbleByPinTask)
    {
        return;
    }

    // Hide the pin bubble in the native map view.
    m_nativeView->HidePinBubble(hideBubbleByPinTask->m_pin);
}

/* See header file for description */
void
MapView::UI_UpdatePinBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskShowBubbleByPin* updateBubbleByPinTask = static_cast<const MapViewTaskShowBubbleByPin*> (pTask);
    if (!updateBubbleByPinTask)
    {
        return;
    }

    // Update the pin bubble in the native map view.
    m_nativeView->UpdatePinBubble(updateBubbleByPinTask->m_pin,
                                  updateBubbleByPinTask->m_x,
                                  updateBubbleByPinTask->m_y,
                                  updateBubbleByPinTask->m_orientation);
}

/* See header file for description */
void
MapView::UI_RemovePinBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskRemovePinBubble* removePinBubbleTask = static_cast<const MapViewTaskRemovePinBubble*> (pTask);
    if (!removePinBubbleTask)
    {
        return;
    }

    // Get the pins to remove the bubble.
    shared_ptr<vector<PinPtr> > pinsToRemoveBubble = removePinBubbleTask->m_pins;
    if (!pinsToRemoveBubble)
    {
        return;
    }

    vector<PinPtr>::const_iterator pinIterator = pinsToRemoveBubble->begin();
    vector<PinPtr>::const_iterator pinEnd = pinsToRemoveBubble->end();
    for (; pinIterator != pinEnd; ++pinIterator)
    {
        PinPtr pin = *pinIterator;
        if (pin)
        {
            // Remove the pin bubble in the native map view.
            m_nativeView->RemovePinBubble(pin);
        }
    }
}

/* See header file for description */
void
MapView::UI_ShowBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskShowBubble* task = static_cast<const MapViewTaskShowBubble*> (pTask);
    if (!task || task->m_bubble.get() == NULL)
    {
        return;
    }

    task->m_bubble->Show(task->m_x, task->m_y, task->m_orientation);
}

/* See header file for description */
void
MapView::UI_HideBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskHideBubble* task = static_cast<const MapViewTaskHideBubble*> (pTask);
    if (!task || task->m_bubble.get() == NULL)
    {
        return;
    }
    task->m_bubble->Hide();
}

/* See header file for description */
void
MapView::UI_UpdateBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskShowBubble* task = static_cast<const MapViewTaskShowBubble*> (pTask);
    if (!task || task->m_bubble.get() == NULL)
    {
        return;
    }

    task->m_bubble->Update(task->m_x, task->m_y, task->m_orientation);
}

/* See header file for description */
void
MapView::UI_RemoveBubble(const MapViewTask* pTask)
{
    if ((m_initialize.IsShuttingDown()) || (!m_nativeView))
    {
        return;
    }

    const MapViewTaskRemoveBubble* task = static_cast<const MapViewTaskRemoveBubble*> (pTask);
    if (!task)
    {
        return;
    }

    // Get the pins to remove the bubble.
    shared_ptr<vector<shared_ptr<BubbleInterface> > > bubbles = task->m_bubbles;
    if (!bubbles)
    {
        return;
    }

    vector<shared_ptr<BubbleInterface> >::const_iterator bubbleIterator = bubbles->begin();
    vector<shared_ptr<BubbleInterface> >::const_iterator bubbleEnd = bubbles->end();
    for (; bubbleIterator != bubbleEnd; ++bubbleIterator)
    {
        shared_ptr<BubbleInterface> bubble = *bubbleIterator;
        if (bubble)
        {
            bubble->Remove();
        }
    }
}

/* See header for description */
void
MapView::UI_UpdateAnimationTip(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_nativeView)
    {
        return;
    }

    const UITaskUpdateAnimationTip* task = static_cast<const UITaskUpdateAnimationTip*>(pTask);
    if (task)
    {
        m_nativeView->UpdateAnimationTip(task->m_tipType);
    }
}

/* See header file for description */
void
MapView::UI_CalculateNightMode(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    const MapViewTask* calculateNightModeTask = static_cast<const MapViewTask*> (pTask);
    if (!calculateNightModeTask)
    {
        return;
    }

    m_isNight = CalculateNightMode();

    SetTheme(m_isNight?MVT_NIGHT:MVT_DAY);
}

/* See header file for description */
void
MapView::UI_ChangeNightMode(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_pListener)
    {
        return;
    }

    const MapViewTaskChangeNightMode* task = static_cast<const MapViewTaskChangeNightMode*> (pTask);
    if (!task)
    {
        return;
    }

    if (task->m_isNightMode != m_lastIsNight)
    {
        m_pListener->OnNightModeChanged(task->m_isNightMode);
        m_lastIsNight = task->m_isNightMode;
    }
}

void
MapView::UI_OnTap(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_pListener)
    {
        return;
    }

    const UITaskOnTap* task = static_cast<const UITaskOnTap*> (pTask);
    if(!task)
    {
        return;
    }

    m_pListener->OnTap(task->m_latitude, task->m_longitude);
}

/* See header file for description */
bool
MapView::CalculateNightMode()
{
    if(m_contextUI.lastGpsLon == INVALID_LONGITUDE || m_contextUI.lastGpsLat == INVALID_LATITUDE)
    {
        return false;
    }
    PAL_ClockDateTime dateTime;
    memset(&dateTime, 0, sizeof(dateTime));
    if (PAL_ClockGetDateTime(&dateTime) != PAL_Ok)
    {
        return false;
    }

    double sunriseTime = 0.0;
    double sunsetTime = 0.0;
    if (NB_SolarCalculatorGetTime(m_contextUI.lastGpsLat, -m_contextUI.lastGpsLon, -dateTime.timezone, dateTime.daylightSaving, dateTime.year, dateTime.month, dateTime.day, &sunriseTime, &sunsetTime) != NE_OK)
    {
        return false;
    }

    // Log the time
    {
        int hour = static_cast<int>(sunriseTime / 60);
        int minu = static_cast<int>(sunriseTime - hour * 60);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView::CalculateNightMode sunrise %f = %02d:%02d.", sunriseTime, hour, minu);

        hour = static_cast<int>(sunsetTime / 60);
        minu = static_cast<int>(sunsetTime - hour * 60);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView::CalculateNightMode sunset %f = %02d:%02d.", sunsetTime, hour, minu);
    }

    bool isNight = false;
    int timerInterval = 0;
    int now = dateTime.hour * 60 + dateTime.minute;
    if (now < static_cast<int>(sunriseTime))
    {
        isNight = true;
        timerInterval = static_cast<int>((sunriseTime - now) * 60 * 1000);
    }
    else if (now >= static_cast<int>(sunriseTime) && now < static_cast<int>(sunsetTime))
    {
        isNight = false;
        timerInterval = static_cast<int>((sunsetTime - now) * 60 * 1000);
    }
    else if (now >= static_cast<int>(sunsetTime))
    {
        isNight = true;
        // Actually we need to recalculate sunriseTime of the next day.
        timerInterval = static_cast<int>((24 * 60 - now + sunriseTime + 1) * 60 * 1000);
    }

    timerInterval = (timerInterval > NIGHT_MODE_CHECK_INTERVAL) ? NIGHT_MODE_CHECK_INTERVAL : timerInterval;

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView::CalculateNightMode current mode: %s and check interval: %d seconds", isNight?"Night":"Day", timerInterval / 1000);
    PAL_TimerCancel(m_palInstance, NightModeTimerCallback, this);
    PAL_TimerSet(m_palInstance, timerInterval, NightModeTimerCallback, this);

    return isNight;
}

void
MapView::NightModeTimerCallback(PAL_Instance* /*pal*/, void* userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        MapView* pThis = (MapView*)userData;
        pThis->m_UITaskQueue->AddTask(new MapViewTask(pThis, &MapView::UI_CalculateNightMode));
    }
}

void
MapView::OnFrameCheckTimerCallback(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        MapView* pThis = (MapView*)userData;
        if(!pThis->m_isNeedFrameUpdate ||
           pThis->m_initialize.IsShuttingDown() ||
           !pThis->m_initialize.IsDone() ||
           !pThis->m_initialize.IsRenderingAllowed())
        {
            //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_CRITICAL, ("####OnFrameCheckTimerCallback, but map or render is not ready"));
        }else
        {
            //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_CRITICAL, ("####OnFrameCheckTimerCallback, trigger"));
            pThis->DoSetFrameZoomAndCenter();
            pThis->m_isNeedFrameUpdate = false;
        }

        PAL_TimerSet(pThis->m_palInstance, FRAME_CHECK_INTERVAL, OnFrameCheckTimerCallback, pThis);
    }
}

void
MapView::OnRenderTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        MapView* thiz = (MapView*)userData;
        if(thiz->m_renderPending)
        {
            thiz->AddRenderTask();
            thiz->m_renderPending = false;
        }
        PAL_TimerSet(pal, RENDER_INTERVAL, OnRenderTimerCallback, thiz);
    }
}

void
MapView::DoSetFrameZoomAndCenter()
{
    if (m_initialize.IsShuttingDown() || !m_mapViewCamera)
    {
        return;
    }

    Point newCenter;
    NBGM_Point3d64 nbgmPosition;
    memset(&nbgmPosition, 0, sizeof(nbgmPosition));

    double newZoom = m_mapViewCamera->GetZoomLevel();
    if(newZoom < m_mapViewCamera->GetMinZoomLevel())
    {
        newZoom = m_mapViewCamera->GetMinZoomLevel();
    }
    else if (newZoom > m_mapViewCamera->GetMaxZoomLevel())
    {
        newZoom = m_mapViewCamera->GetMaxZoomLevel();
    }
    m_mapViewCamera->GetCurrentPosition(nbgmPosition.x, nbgmPosition.y);

    NB_SpatialConvertMercatorToTile(nbgmPosition.x, nbgmPosition.y, (int)newZoom, &newCenter.x, &newCenter.y);
    //Todo: remove this line
    //yTile = yTile * TILE_SIZE + (int)(yOffset * (double)TILE_SIZE);


    // Update new center
    m_frame.SetCenter(newCenter);

    /*
      We have to get the frustum after all the values got updated in NBGM. This gets done
      in the render function.
    */
    vector<NBGM_Point2d64> frustum;
    vector<NBGM_Point2d64> frustumRasterNear;

    if (m_mapViewCamera->GetFrustum(frustum, frustumRasterNear))
    {
        if (m_frame.SetZoomLevelAndFrustum(newZoom, frustum, frustumRasterNear))
        {
            m_frame.SetFrustumValid(true);
            m_renderPending = true;
            AddUpdateFrameTask();
        }
    }
    else
    {
        m_frame.SetFrustumValid(false);
    }

    UpdateAnimationUI(true, shared_ptr<MapLegendInfo>());
}

/* See header for description */
void
MapView::SetFrameZoomAndCenter()
{
    m_isNeedFrameUpdate = true;
}

/* See header file for description */
uint32
MapView::GetNBGMVersion()
{
    return NBGMViewController::GetSupportedNBMFileVersion();
}

/* See header file for description */
shared_ptr<bool>
MapView::IsValid()
{
    return m_initialize.m_isValid;
}

/*See header file for description */
void
MapView::ShowDebugView(bool enabled)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    qDebug() << "MapView::ShowDebugView before checking nativeView";
    if (m_nativeView)
    {
        qDebug() << "MapView::ShowDebugView with valid nativeView";
        m_enabledDebugInfo = enabled;
        m_nativeView->EnableDebugInfo(enabled);
    }
}

/*See header file for description */
void
MapView::UpdateDebugUI()
{
    /*
        This function gets called in the context of the render thread!
     */

    if (! m_enabledDebugInfo)
    {
        return;
    }

    if ( m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->GetDebugInfo(m_info);

    /* Calculate average frame rate for NBGM */
    double frameRateNBGM = m_info.CalculateFrameRate(m_info.renderTime, m_info.lastRenderTimesNBGM);

    /*
     Calculate total frame rate
     */
    double frameRateTotal = 0.0;

    // We get the tick count here and measure the difference between the last time we were called. This only works
    // if the render function gets called continuously, which is the case at the time.
    uint32 newTickCount = MapViewProfiling::GetTickCount();
    if (m_info.lastRenderTickCount != 0)
    {
        uint32 newRenderTime = newTickCount - m_info.lastRenderTickCount;
        frameRateTotal = m_info.CalculateFrameRate(newRenderTime, m_info.lastRenderTimesTotal);
    }
    m_info.lastRenderTickCount = newTickCount;

    // Only update debug UI by a fixed frequency
    // Or it will add too many tasks and block the UI thread
    uint32 now = PAL_ClockGetTimeMs();
    if (DEBUG_UI_UPDATE_INTERVAL + m_lastDebugUiUpdateTime < now)
    {
        m_lastDebugUiUpdateTime = now;
    }
    else
    {
        return;
    }

    // Switch to UI thread to update the debug UI
    MapViewTaskUpdateDebugUI* pUITask = new MapViewTaskUpdateDebugUI(this, &MapView::UI_OutputDebugUI);
    if (pUITask)
    {
        pUITask->m_debugStream << "Zoom: " << floor(m_info.actualZoomLevel * 1000) / 1000 << endl;
        pUITask->m_debugStream << "Latitude: " << floor(m_info.latitude * 1000) / 1000 << endl;
        pUITask->m_debugStream << "Longitude: " << floor(m_info.longitude * 1000) / 1000 << endl;
        pUITask->m_debugStream << "Rotate Angle: " << floor(m_info.rotationAngle * 1000) / 1000 << endl;
        pUITask->m_debugStream << "Tilt Angle: " << floor(m_info.tiltAngle * 1000) / 1000 << endl;
        pUITask->m_debugStream << "Distance: " << floor(m_info.cameraHeight * 1000) / 1000 << endl;
        pUITask->m_debugStream << "FR: " << floor(frameRateTotal * 10) / 10 << " (" << floor(frameRateNBGM * 10) / 10 << ")" << endl;
        pUITask->m_debugStream << "Triangles: " << m_info.totalNumOfTriangles << endl;
        pUITask->m_debugStream << "Tri/sec: " << floor(frameRateNBGM * (double)m_info.totalNumOfTriangles) << endl;
        // pUITask->m_debugStream << "Loaded tiles: " << m_info.numberOfLoadedTiles;

        if (m_gpsMode == NGM_FOLLOW_ME)
        {
            /*
             We currently don't delete the old log file. Should we do that for each new follow-me session?
             */

            // Did we just start follow-me mode
            if (m_gpsMode != m_lastGpsMode)
            {
                // Output header. Make sure it matches our order below.
                pUITask->m_fileHeader << "Zoom, Latitude, Longitude, Rotation Angle, Tilt Angle, Distance, Frame Rate Total, Frame Rate NBGM, Triangles, Triangles/second Total, Triangles/second NBGM, Loaded Tiles";
            }

            // Only log every second
            if ((newTickCount - m_info.lastLoggingTickCount) > 1000)
            {
                // When we're in follow-me mode then we also output this information to a debug file. But we format it in CSV file format.
                pUITask->m_fileStream << m_info.actualZoomLevel << ",";
                pUITask->m_fileStream << m_info.latitude << ",";
                pUITask->m_fileStream << m_info.longitude << ",";
                pUITask->m_fileStream << m_info.rotationAngle << ",";
                pUITask->m_fileStream << m_info.tiltAngle << ",";
                pUITask->m_fileStream << m_info.cameraHeight << ",";
                pUITask->m_fileStream << frameRateTotal << ",";
                pUITask->m_fileStream << frameRateNBGM << ",";
                pUITask->m_fileStream << m_info.totalNumOfTriangles << ",";
                pUITask->m_fileStream << (frameRateTotal * (double)m_info.totalNumOfTriangles) << ",";
                pUITask->m_fileStream << (frameRateNBGM * (double)m_info.totalNumOfTriangles) << ",";
                pUITask->m_fileStream << m_info.numberOfLoadedTiles;

                m_info.lastLoggingTickCount = newTickCount;
            }
        }
        m_lastGpsMode = m_gpsMode;

        m_UITaskQueue->AddTask(pUITask);
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyShowBubble(shared_ptr<string> pinId, float x, float y, bool /*orientation*/)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to show the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskShowBubbleById(this,
                                                            &MapView::CCC_ShowPinBubble,
                                                            pinId,
                                                            x,
                                                            y,
                                                            true));
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyHideBubble(shared_ptr<std::string> pinId)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to hide the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskHideBubbleById(this,
                                                            &MapView::CCC_HidePinBubble,
                                                            pinId));
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyUpdateBubble(shared_ptr<string> pinId, float x, float y, bool orientation)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to hide the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskShowBubbleById(this,
                                                            &MapView::CCC_UpdatePinBubble,
                                                            pinId,
                                                            x,
                                                            y,
                                                            orientation));
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyShowLocationBubble(float x, float y, bool orientation,
                                  float latitude, float longitude)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_eventQueue)
    {
        return;
    }

    // Add a UI task to update the pin bubble. Bubble will be created if necessary, so just
    // update bubble here.
    UITaskUpdateLocationBubble* pTask =
            new UITaskUpdateLocationBubble(this, &MapView::UI_UpdateLocationBubble, x, y,
                                         orientation, latitude, longitude);
    if (pTask)
    {
        m_UITaskQueue->AddTask(pTask);
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyHideLocationBubble()
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_eventQueue)
    {
        return;
    }

    MapViewTask* pTask = new MapViewTask(this, &MapView::UI_HideLocationBubble);
    if (pTask)
    {
        m_UITaskQueue->AddTask(pTask);
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyUpdateLocationBubble(float x, float y, bool orientation,
                                    float latitude, float longitude)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_eventQueue)
    {
        return;
    }
    // Add a UI task to update the pin bubble.
    UITaskUpdateLocationBubble* pTask =
            new UITaskUpdateLocationBubble(this, &MapView::UI_UpdateLocationBubble, x, y,
                                         orientation, latitude, longitude);
    if (pTask)
    {
        m_UITaskQueue->AddTask(pTask);
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyShowStaticPoiBubble(float x, float y, bool /*orientation*/)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to show the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskShowStaticPoiBubble(this,
            &MapView::CCC_ShowStaticPoiBubble,
            x,
            y,
            true));
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyHideStaticPoiBubble()
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to hide the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskHideStaticPoiBubble(this,
            &MapView::CCC_HideStaticPoiBubble));
    }
}

/* See BubblelistenerInterface.h for description */
void
MapView::NotifyUpdateStaticPoiBubble(float x, float y, bool orientation)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    // Add a CCC task to hide the pin bubble.
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskShowStaticPoiBubble(this,
            &MapView::CCC_UpdateStaticPoiBubble,
            x,
            y,
            orientation));
    }
}

void
MapView::PreferredLanguageChanged(uint8 languageCode)
{
    if (m_preferredLanguageCode != languageCode)
    {
        m_preferredLanguageCode = languageCode;

        if (m_layerManager)
        {
            m_layerManager->SetPreferredLanguageCode(languageCode);
        }

        NBGM_SetPreferredLanguageCode(languageCode);
        m_layerManager->RemoveAllTiles();

        //TRICKY: Schedule a task in EventQueue to refresh all tiles. The reason is that
        //        when language code is changed, there maybe more than one MapView instances
        //        who are notified with this message. If we call RefreshAllTiles() here,
        //        then the tile requests issued by this instance may be removed by other
        //        instances as an side effect of m_layerManager->RemoveAllTiles(). Add a
        //        task in EventQueue will avoid this.

        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_RefreshAllTiles));
    }
}

void
MapView::MapThemeChanged(MapViewTheme theme)
{
    if (m_currentTheme != theme)
    {
        m_currentTheme = theme;

        if (m_layerManager)
        {
            m_layerManager->SetTheme(theme);
            m_layerManager->RemoveBackgroundRasterTiles();
        }

        //TRICKY: Schedule a task in EventQueue to refresh all tiles. The reason is that
        //        when mapview theme is changed, there maybe more than one MapView instances
        //        who are notified with this message. If we call RefreshAllTiles() here,
        //        then the tile requests issued by this instance may be removed by other
        //        instances as an side effect of m_layerManager->RemoveAllTiles(). Add a
        //        task in EventQueue will avoid this.
        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_RefreshRasterTiles));
    }
}

/* See header for description */
void
MapView::AddRenderTask(RenderListener* listener)
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone() || m_background)
    {
        if (listener)
        {
            listener->OnDiscard();
        }
        return;
    }

    // If the flag is still set then the render task wasn't called since our last call, ignore the call
    /*
        @todo: I think using the flag is a poor way to synchronize this. We can probably just get rid of the flag since we
               continuously render now anyway.
     */
    if (m_needsRendering && !listener)
    {
        return;
    }
    m_needsRendering = true;

    AddRenderThreadTask(new MapViewTaskRender(this, &MapView::RenderThread_NBGMRender ,listener));
}

void MapView::AddRenderThreadTask(MapViewTask* task) const
{
    AddRenderThreadTask(task, DEFAULT_TASK_PRIORITY);
}

void
MapView::AddRenderThreadTask(MapViewTask* task, uint32 priority) const
{
    if( !m_CombinedAtomicTaskExecuter || !m_CombinedAtomicTaskExecuter->AddTask(task)  )
    {
       m_pRenderingQueue->AddTask(task, priority);
    }
}

void
MapView::AddResetScreenTask()
{
    if (m_initialize.IsShuttingDown() || !m_initialize.IsDone())
    {
        return;
    }

    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_ResetScreen));
}

/* See header for description */
void
MapView::RefreshFrame(double latitude, double longitude)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    // Convert lat/lon to world coordinates (mercator times tile size)
    Point newCenter;
    double xMercator = 0.0;
    double yMercator = 0.0;

    if (!m_nbgmViewController)
    {
        return;
    }
    int currentZoom = (int) m_mapViewCamera->GetZoomLevel();

    NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
    NB_SpatialConvertMercatorToTile(xMercator, yMercator, currentZoom, &newCenter.x, &newCenter.y);

    m_frame.SetZoom(currentZoom);
    m_frame.SetCenter(newCenter);

    /*
        :TRICKY:
        Don't trigger a CCC_UpdateFrame (or any other) task here directly. This gets called during initialization and
        we don't want to trigger any tasks during the initialization.
     */
}

/* See header for description */
void MapView::UpdateFrameAndNBGM()
{
    /*
        This function gets called in the CCC thread!
     */

    // Get raster tile range and set it to frame and NBGMController
    uint32 minRaster = 0;
    m_layerManager->GetBackgroundRasterRange(minRaster, m_maxRasterZoomLevel);
    m_frame.SetRasterZoomRange(minRaster, m_maxRasterZoomLevel);

    // Save all the layers to the frame class
    m_frame.SetLayers(m_layers);
}

NB_Error
MapView::SetMapViewContext(void* context)
{
    if (m_initialize.IsShuttingDown())
    {
        return NE_NOTINIT;
    }

    m_nativeView->SetMapViewProtocol(context);
    return NE_OK;
}

void
MapView::SetLayerManager(LayerManagerPtr layerManager)
{
    m_layerManager = layerManager;
    if (m_nbgmViewController)
    {
        m_nbgmViewController->UseTileService();
    }
}

NB_Error
MapView::ShowRouteLayer(bool isShown)
{
    if (m_initialize.IsShuttingDown())
    {
        return NE_NOTINIT;
    }

    m_showRoute = isShown;

    // Swtich to CCC task queue
    m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_ShowRouteLayer));
    return NE_OK;
}

/* See header for description */
void
MapView::CCC_ShowRouteLayer(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    if (m_showRoute)
    {
        m_frame.ForceCleanCurrentFrame();
        AddUpdateFrameTask();
    }

    if (m_initialize.IsRenderingAllowed())
    {
        m_renderPending = true;
    }
}

NB_Error
MapView::WindowToWorld(int dX, int dY, double* lat, double* lon)
{
    if ((lat == NULL) || (lon == NULL))
    {
        return NE_INST;
    }
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    bool error = false;
    if (m_mapViewCamera)
    {
        error = m_mapViewCamera->ScreenToMapPosition(static_cast<float>(dX), static_cast<float>(dY), mercatorX, mercatorY);
    }
    if (!error)
    {
        return NE_INVAL;
    }
    mercatorReverse(mercatorX, mercatorY, lat, lon);
    return NE_OK;
}

NB_Error
MapView::SetCamera(const MapViewCameraParameter& cameraParameter)
{
    if(! m_mapViewCamera)
    {
        return NE_INST;
    }
    m_mapViewCamera->SetCamera(cameraParameter.latitude, cameraParameter.longitude, cameraParameter.heading, cameraParameter.zoomLevel, cameraParameter.tilt, m_cameraTimestamp++,false);
//    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "MapView::SetCamera alt=%f head=%f tilt=%f", altitude, heading, tilt);

    //Current tilt is smaller than the min tilt in this zoom level.
    //update the tilt to the min tilt defined in NBUI.
    if (cameraParameter.tilt != m_mapViewCamera->GetTiltAngle())
    {
        m_nativeView->UpdateTilt(90 -m_mapViewCamera->GetTiltAngle());
    }
    if (m_initialize.IsRenderingAllowed())
    {
        SetFrameZoomAndCenter();
    }

    return NE_OK;
}

PAL_Instance*
MapView::GetPalInstance()
{
    return this->m_palInstance;
}

shared_ptr<list<string> >
MapView::GetInteractedPois(float x, float y)
{
    shared_ptr<list<string> > poiList;
    if (m_nbgmViewController)
    {
        poiList = m_nbgmViewController->GetInteractedPois(x, y);
    }
    return poiList;
}

double
MapView::GetCurrentZoomLevel()
{
    double zoomLevel = 0.0;

    if (m_mapViewCamera)
    {
        zoomLevel = m_mapViewCamera->GetZoomLevel();
    }

    return zoomLevel;
}

NB_Error
MapView::WorldToWindow(double latitude, double longitude, int* dX, int* dY)
{
    if ((dX== NULL) || (dY == NULL))
    {
        return NE_INST;
    }

    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(latitude, longitude, &mercatorX, &mercatorY);
    float floatX = 0.0;
    float floatY = 0.0;
    bool error = false;

    if (m_mapViewCamera)
    {
        error = m_mapViewCamera->MapPositionToWindow(mercatorX, mercatorY, &floatX, &floatY);
    }

    if (!error)
    {
        return NE_INVAL;
    }
    *dX = (int)floatX;
    *dY = (int)floatY;

    return NE_OK;
}

NBGM_MapView*
MapView::CreateNBGMMapView()
{
    NBGM_Config nbgmConfig ;
    nsl_memset(&nbgmConfig, 0, sizeof(NBGM_Config));
    nbgmConfig.pal = m_palInstance;

    uint32 width=1;
    uint32 height=1;
    uint32 dpi=1;
    PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    nbgmConfig.dpi = (float)dpi;

    nbgmConfig.workSpace = m_resourceFolder.c_str();
    nbgmConfig.binFileTexturePath = m_resourceFolder.c_str();
    nbgmConfig.nbmOffset = 0;
    nbgmConfig.preferredLanguageCode = m_preferredLanguageCode;
    nbgmConfig.relativeCameraDistance = MapViewCamera::GetRelativeCameraHeight();
    nbgmConfig.metersPerPixelOnRelativeZoomLevel = MapViewCamera::GetMetersPerPixelOnRelativeZoomLevel();
    nbgmConfig.relativeZoomLevel = MapViewCamera::GetRelativeZoomLevel();
    nbgmConfig.useRoadBuildingVisibility = TRUE;
    if (m_fontMagnifierLevel == MVFML_LARGE)
    {
        nbgmConfig.fontMagnifierFactor = 2.0f;
    }
    else if (m_fontMagnifierLevel == MVFML_MEDIUM)
    {
        nbgmConfig.fontMagnifierFactor = 1.5f;
    }
    else
    {
        nbgmConfig.fontMagnifierFactor = 1.f;
    }
    NBGM_Initialize(&nbgmConfig);

    // Create the NBGM instance if it has not been created.
    NBGM_SetPreferredLanguageCode(m_preferredLanguageCode);
    
    // Disable all the NBGM logs
    NBGM_SetDebugLogSeverity((PAL_LogSeverity)0);

    NBGM_SetRelativeZoomLevel(MapViewCamera::GetRelativeZoomLevel());
    NBGM_SetRelativeCameraHeight(MapViewCamera::GetRelativeCameraHeight());
    NBGM_SetResolutionRelativeZoomLevel(MapViewCamera::GetMetersPerPixelOnRelativeZoomLevel());

    NBGM_MapViewConfig mapConfig;

    mapConfig.drawSky = FALSE;
    mapConfig.drawAvatar = TRUE;
    mapConfig.drawFlag = FALSE;
    mapConfig.renderContext = m_nativeView->GetRenderContext();
    mapConfig.renderTaskQueue = m_pRenderingQueue;
    mapConfig.renderInterval = MAX_RENDER_INTERVAL;

    NBGM_MapView* result = NULL;
    if(NBGM_CreateMapView(&mapConfig, &result) != PAL_Ok)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CreateNBGMMapView(), NBGM_CreateMapView Failed!", this);
        return NULL;
    }
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "MapView(%#x)::CreateNBGMMapView(), NBGM_CreateMapView successful!", this);
    return result;
}

void
MapView::SetViewPort(int width, int height)
{
    if (width != 0 && height != 0)
    {
        if (width != m_viewSizeWidth || height != m_viewSizeHeight)
        {
            UI_SetViewPort(width, height);
            m_nativeView->SetViewSize(width, height);
            m_renderPending = true;
        }
    }
}

void MapView::SetShowLabelDebugInfo(bool showLabelDebugInfo)
{
    if (m_showLabelDebugInfo != showLabelDebugInfo)
    {
        m_showLabelDebugInfo = showLabelDebugInfo;
        NBGM_ShowLabelDebugInfo(showLabelDebugInfo);
        RefreshAllTiles();
    }

}

void MapView::RefreshMetadataAndLayers(shared_ptr<MetadataConfiguration> metadataConfig)
{
    NB_ASSERT_CCC_THREAD(m_nbContext);
    if (m_eventQueue)
    {
        /* Add a CCC task to request metadata again.
           For the MetadataConfiguration changed.
        */
        m_pMetadataConfig = metadataConfig;
        m_eventQueue->AddTask(new MapViewTask(this, &MapView::CCC_GetLayers));
    }
}

void
MapView::AddUpdateFrameTask()
{
    if (!m_frameUpdated || m_initialize.IsShuttingDown() || !m_initialize.IsDone() || !m_initialize.IsRenderingAllowed())
    {
        return;
    }
    m_frameUpdated = false;

    m_eventQueue->AddTask(new MapViewTaskUpdateFrame(this, &MapView::CCC_UpdateFrame));
}

/* See MapViewInterface.h for description */
void MapView::CancelGenerateMapImage()
{
    if (m_pScreenshotInProgress)
    {
        *m_pScreenshotInProgress = false;
    }
}

/* See MapViewInterface.h for description */
NB_Error MapView::GenerateMapImage(shared_ptr<AsyncCallback<shared_ptr<DataStream> > >  callback)
{
    // This has to be called in the context of the CCC thread!
    NB_ASSERT_CCC_THREAD(m_nbContext);

    if (!callback)
    {
        return NE_INVAL;
    }

    if (m_initialize.IsShuttingDown() || !m_pRenderingQueue)
    {
        // MapView is being shutdown.
        return NE_INST;
    }

    if (m_pScreenshotInProgress && *m_pScreenshotInProgress)
    {
        return NE_BUSY;
    }

    m_pScreenshotInProgress = shared_ptr<bool> (new bool(true));
    shared_ptr<AsyncGenerateMapImageTileCallback> generateImageTileCallback = shared_ptr<AsyncGenerateMapImageTileCallback>(new AsyncGenerateMapImageTileCallback(this, m_pScreenshotInProgress, callback));

    /* This need to do a switch thread , ccc thread switch to render thread and render thread switch to ccc thread

       For guaranteed invoke m_frame.GetCurrentTiles(tiles) before the CCC_UpdateFrame invoke is finished.

    */

    // Add task switch to the render thread
    AddRenderThreadTask(new RenderTaskPrepareAsyncGenerateMapImage(this, &MapView::RenderThread_PrepareAsyncGenerateMapImage, generateImageTileCallback));

    return NE_OK;
}

void MapView::CCC_PrepareAsyncGenerateMapImage(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    const MapViewTaskPrepareAsyncGenerateMapImage* task = static_cast<const MapViewTaskPrepareAsyncGenerateMapImage*>(pTask);

    do
    {
        if (!task->m_generateImageTileCallback)
        {
            break;
        }

        const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > callback = task->m_generateImageTileCallback->m_callback;

        if (callback)
        {
            if (task->m_generateImageTileCallback->m_pScreenshotInProgress && *(task->m_generateImageTileCallback->m_pScreenshotInProgress))
            {
                vector<TileKeyPtr> tiles;
                m_frame.GetCurrentTiles(tiles);
                // This should only be called in CCC thread, no need to switch thread.
                if (! tiles.empty())
                {
                    // Re-request tiles for all layers
                    vector<LayerPtr> emptyVector;
                    m_layerManager->GetTiles(tiles, task->m_generateImageTileCallback, emptyVector, emptyVector);
                    return;
                }
                else
                {
                    /* should never happen, but just in case */
                    callback->Error(NE_INVAL);
                }
            }
        }

        *(task->m_generateImageTileCallback->m_pScreenshotInProgress) = false;

    }while(0);
}

void MapView::RenderThread_PrepareAsyncGenerateMapImage(const MapViewTask* pTask)
{
    if (!m_initialize.IsShuttingDown() && m_initialize.IsDone())
    {
        const RenderTaskPrepareAsyncGenerateMapImage* task = static_cast<const RenderTaskPrepareAsyncGenerateMapImage*>(pTask);

        // Add task switch to the CCC thread
        m_eventQueue->AddTask(new MapViewTaskPrepareAsyncGenerateMapImage(this, &MapView::CCC_PrepareAsyncGenerateMapImage, task->m_generateImageTileCallback));
    }
}

void MapView::RenderThread_AsyncGenerateMapImage(const MapViewTask* pTask)
{
    const MapViewTaskAsyncGenerateMapImage* task = static_cast<const MapViewTaskAsyncGenerateMapImage*>(pTask);

    if (m_initialize.IsShuttingDown() && !m_initialize.IsDone())
    {
        return;
    }

    if (task->m_pScreenshotInProgress && *(task->m_pScreenshotInProgress))
    {
        shared_ptr<AsyncGenerateMapImageCallback> callback(new AsyncGenerateMapImageCallback(this, task->m_pAsyncGenerateMapImageCallback, task->m_pScreenshotInProgress));
        if (callback && m_nbgmViewController)
        {
            if (PAL_Ok == m_nbgmViewController->AsyncGenerateMapImage(callback))
            {
               return;
            }
        }
        m_eventQueue->AddTask(new MapViewTaskNotifyAsyncGenerateMapImageCallback(this, &MapView::CCC_NotifyAsyncGenerateMapImageCallback, task->m_pAsyncGenerateMapImageCallback, shared_ptr<nbcommon::DataStream>(), NE_INVAL));
    }

    *(task->m_pScreenshotInProgress) = false;
}

void MapView::AllTilesDownloadFinished(const shared_ptr<AsyncCallback<shared_ptr<DataStream> > >  callback)
{
    if ((!m_initialize.IsShuttingDown()) && m_initialize.IsDone() && m_pScreenshotInProgress && *m_pScreenshotInProgress)
    {
        // Make task switch to the render thread
        AddRenderThreadTask(new MapViewTaskAsyncGenerateMapImage(this, &MapView::RenderThread_AsyncGenerateMapImage, callback, m_pScreenshotInProgress));
    }
    else
    {
        callback->Error(NE_INVAL);
        *m_pScreenshotInProgress = false;
    }
}

/* See header file for description */
void MapView::CCC_NotifyAsyncGenerateMapImageCallback(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    const MapViewTaskNotifyAsyncGenerateMapImageCallback* task = static_cast<const MapViewTaskNotifyAsyncGenerateMapImageCallback*>(pTask);

    if(task->m_error == NE_OK && task->m_dataStream)
    {
        task->m_pAsyncGenerateMapImageCallback->Success(task->m_dataStream);
    }
    else
    {
        task->m_pAsyncGenerateMapImageCallback->Error(task->m_error);
    }

}

void MapView::NotifyAsyncGenerateMapImageCallback(const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > pAsyncGenerateMapImageCallback,
                                                  shared_ptr<nbcommon::DataStream> dataStream,
                                                  NB_Error                         error)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new MapViewTaskNotifyAsyncGenerateMapImageCallback(this, &MapView::CCC_NotifyAsyncGenerateMapImageCallback, pAsyncGenerateMapImageCallback, dataStream, error));
    }
}

void MapView::EnableGetTileWithFrame(bool enabled)
{
    m_getTileWithFrame = enabled;
}

void
MapView::RenderThread_SelectPin(const MapViewTask* pTask)
{
    const MapViewTaskSelectPin* task = static_cast<const MapViewTaskSelectPin*>(pTask);

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    // Unselcet static poi bubble first.
    NotifyHideStaticPoiBubble();
    m_nbgmViewController->ResetPoiBubbleState();

    m_nbgmViewController->SelectPin(task->m_pinId, task->m_selected);
    m_renderPending = true;
}

// See MapView.h for description.
void
MapView::SelectPin(shared_ptr<string> pinId)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new MapViewTaskSelectPin(this, &MapView::RenderThread_SelectPin, pinId, true));
}

// See MapView.h for description.
void
MapView::UnselectPin(shared_ptr<string> pinId)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskSelectPin(this, &MapView::RenderThread_SelectPin, pinId, false));
}

void
MapView::ShowCompass(bool enabled)
{
    if (m_nbgmViewController)
    {
        m_nbgmViewController->ShowCompass(enabled);
        m_renderPending = true;
    }
}

void
MapView::SetCompassPosition(float screenX, float screenY)
{
    if (m_nbgmViewController)
    {
        m_nbgmViewController->SetCompassPosition(screenX, screenY);
        m_renderPending = true;
    }
}

void
MapView::SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath)
{
    if (m_nbgmViewController)
    {
        m_nbgmViewController->SetCompassIcons(dayModeIconPath, nightModeIconPath);
        m_renderPending = true;
    }
}

void MapView::GetElementsAt(float screenX, float screenY, set<string>& elements)
{
    PAL_Event* event = NULL;
    if (PAL_EventCreate(m_palInstance, &event) == PAL_Ok && m_nbgmViewController)
    {
        AddRenderThreadTask(
            new RenderTaskGetElementsAt(this, m_nbgmViewController, screenX, screenY,
                                        elements, event));
        PAL_EventWaitForEvent(event);
        PAL_EventDestroy(event);
    }
}

void MapView::EnableCustomLayerCollisionDetection(const std::string& layerId, bool enable)
{
    AddRenderThreadTask(
        new RenderTaskEnableCustomLayerCollisionDetection(this, m_nbgmViewController, layerId, enable));
}

void MapView::MaterialAdded(const ExternalMaterialPtr& material)
{
    if (material)
    {
        RenderTaskModifyExternalMaterial* task =
                new RenderTaskModifyExternalMaterial(this,
                                                     &MapView::RenderThread_ModifyExternalMaterial,
                                                     RenderTaskModifyExternalMaterial::EMMT_Load,
                                                     material);
        AddRenderThreadTask(task);
    }
}

void MapView::MaterialRemoved(const ExternalMaterialPtr& material)
{
    AddRenderThreadTask(
        new RenderTaskModifyExternalMaterial(this,
                                             &MapView::RenderThread_ModifyExternalMaterial,
                                             RenderTaskModifyExternalMaterial::EMMT_Unload,
                                             material));
}

void MapView::MaterialActived(const ExternalMaterialPtr& material)
{
    AddRenderThreadTask(
        new RenderTaskModifyExternalMaterial(this,
                                             &MapView::RenderThread_ModifyExternalMaterial,
                                             RenderTaskModifyExternalMaterial::EMMT_Activate,
                                             material));
}

void MapView::UI_InvokeSingleTapListener(const MapViewTask* pTask)
{
    const UITaskInvokeSingleTapListener* task =
            static_cast<const UITaskInvokeSingleTapListener*>(pTask);
    if (!m_initialize.IsShuttingDown() && m_initialize.IsDone() && task && m_pListener)
    {
        if (m_pListener)
        {
            //only invoke callback in ui thread, do not indicate tab gesture on our own.
            m_pListener->OnSingleTap(task->m_x, task->m_y, task->m_elements);
        }
    }
}

void MapView::UI_InvokeCameraUpdateListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }
    const UITaskInvokeCameraUpdateListener* task = static_cast<const UITaskInvokeCameraUpdateListener*>(pTask);
    if (m_pListener)
    {
        m_pListener->OnCameraUpdate(task->m_lat, task->m_lon, task->m_zoomLevel, task->m_heading, task->m_tilt);
    }

    m_cameraUpdateGuard = false;
}

void MapView::CCC_PinClickedListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }
    const CCCTaskInvokePinClickedListener* task = static_cast<const CCCTaskInvokePinClickedListener*>(pTask);
    if (m_pListener)
    {
        shared_ptr<Pin> pin = CCC_GetPin(shared_ptr<string>(new string(task->m_pinId)));
        if(pin)
        {
           if(pin->GetType() == PT_COMMON || pin->GetType() == PT_CUSTOM)
           {
                m_pListener->OnPinClicked(task->m_pinId);
           }
           // The left types should be incident pin types now.
           else
           {
               m_UITaskQueue->AddTask(new UITaskTrafficIncidentPinClickedListener(
                                      this,
                                      &MapView::UI_TrafficIncidentPinClickedListener,
                                      pin->GetLatitude(),
                                      pin->GetLongitude()));
           }
        }
    }
}

void MapView::CCC_MarkerClickedListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_customLayerManager)
    {
        return;
    }

    const CCCTaskInvokeMarkerClickedListener* task = static_cast<const CCCTaskInvokeMarkerClickedListener*>(pTask);
    if(task == NULL)
    {
        return;
    }

    int userId = 0;
    if(m_customLayerManager->GetMarkerID(task->m_markerId, userId))
    {
        m_UITaskQueue->AddTask(new UITaskInvokeMarkerClickedListener(
                              this,
                              &MapView::UI_InvokeMarkerClickListener,
                              userId));
    }
}

void MapView::UI_InvokeMarkerClickListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    const UITaskInvokeMarkerClickedListener* task = static_cast<const UITaskInvokeMarkerClickedListener*>(pTask);
    if(task == NULL)
    {
        return;
    }

    if (m_pListener)
    {
        m_pListener->OnMarkerClicked(task->m_markerId);
    }
}

void MapView::CCC_UnselectAllPinsListener(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    if (m_pListener)
    {
        m_pListener->OnUnselectAllPins();
    }
}

void MapView::RenderThread_SelectAvatarAndPin(const MapViewTask* pTask)
{
    if(!pTask)
    {
        return;
    }
    const RenderTaskSelectAvatarAndPinTask* task = static_cast<const RenderTaskSelectAvatarAndPinTask*>(pTask);
    float screenX = task->m_screenX;
    float screenY = task->m_screenY;
    shared_ptr<string> newSelectedPinId;
    bool isHit = false;
    bool hasTouchAnything = false;

    {
        // Try to select a pin
        shared_ptr<list<string> > poiList = m_nbgmViewController->GetInteractedPois(screenX, screenY);
        if (poiList && (!(poiList->empty())))
        {
            static const size_t LEN = nsl_strlen(CUSTOM_MARKER_PREFIX);
            // Filter out custom markers.
            list<string>::iterator iter = poiList->begin();
            for(; iter != poiList->end(); )
            {
                if(iter->find(CUSTOM_MARKER_PREFIX) != string::npos)
                {
                    if(iter->size() > LEN)
                    {
                        //@todo: Decide the condition if a pin and a marker are selected at the same time.
                        string markerId = iter->substr(LEN);
                        if(!markerId.empty())
                        {
                            //For now, we just need to send a single id outside.
                            m_eventQueue->AddTask(new CCCTaskInvokeMarkerClickedListener(
                                                  this,
                                                  &MapView::CCC_MarkerClickedListener,
                                                  markerId));
                            isHit = true;
                            break;
                        }
                    }
                    poiList->erase(iter++);
                }
                else
                {
                    ++iter;
                }
            }

            if(!isHit && !(poiList->empty()))
            {
                // Select the first one if there is no current selected pin ID.
                list<string>::const_iterator pinIdIterator = poiList->begin();
                list<string>::const_iterator pinIdEnd = poiList->end();
                // Find if the current selected pin ID exists in the list.
                for (; pinIdIterator != pinIdEnd; ++pinIdIterator)
                {
                    shared_ptr<string> currentSelectedPinId = m_nbgmViewController->GetSelectedPinId(*pinIdIterator);
                    if (currentSelectedPinId && !(currentSelectedPinId->empty()) && currentSelectedPinId->compare(*pinIdIterator) == 0)
                    {
                        // Get the next pin ID to select.
                        ++pinIdIterator;
                        break;
                    }
                }

                /* Use the first pin ID if the current pin ID is not found or the LAST
                   pin ID is the current selected. */
                if (pinIdIterator == pinIdEnd)
                {
                    newSelectedPinId = shared_ptr<string>(new string(poiList->front()));
                }
                else
                {
                    newSelectedPinId = shared_ptr<string>(new string(*pinIdIterator));
                }

                isHit = true;
            }
            hasTouchAnything = true;
        }
    }

    /* Select the new pin, NBGM view controller will notify to show or hide the pin by the
       listener.

       TRICKY: NBGM view controller checks if the pin is visible on the screen internal.
    */
    m_nbgmViewController->SelectPin(newSelectedPinId, newSelectedPinId?true:false);

    bool showStaticPoiBubble = false;
    if (!isHit)
    {
        // check static poi if no pin selected
        vector<string> poiList;
        m_nbgmViewController->GetStaticPois(screenX, screenY, poiList);
        if (poiList.size() > 0)
        {
            float bubbleOffsetX = 0;
            float bubbleOffsetY = 0;
            string id = poiList.front();
            string poiName;
            double latitude = 0;
            double longitude = 0;
            if (m_nbgmViewController->GetStaticPoiInfo(id, &bubbleOffsetX, &bubbleOffsetY, poiName, &latitude, &longitude))
            {
                isHit = true;
                hasTouchAnything = true;
                m_nbgmViewController->SetSelectedStaticPoiId(shared_ptr<string>(new string(id)));
                MapViewTask* task = new MapViewTaskAddStaticPoiBubble(this, &MapView::CCC_ChangeStaticPoiBubble, id, poiName, latitude, longitude);
                if (task)
                {
                    m_eventQueue->AddTask(task);
                    showStaticPoiBubble = true;
                }
                if(m_pListener)
                {
                    m_UITaskQueue->AddTask(new UITaskStaticPOIClickedListener(
                        this,
                        &MapView::UI_StaticPOIClickedListener,
                        id,
                        poiName,
                        latitude,
                        longitude));
                }
            }
        }
    }

    if (!showStaticPoiBubble)
    {
        NotifyHideStaticPoiBubble();
        m_nbgmViewController->ResetPoiBubbleState();
    }

    if (m_pListener)
    {
        if(newSelectedPinId.get() == NULL || newSelectedPinId->empty())
        {
            set<string> ids;
            m_nbgmViewController->GetElementsAt(screenX, screenY, ids);
            if (!ids.empty())
            {
                hasTouchAnything = true;
                // Need to switch to UI thread to invoke tap listener.
                m_UITaskQueue->AddTask(new UITaskInvokeSingleTapListener(
                                           this,
                                           &MapView::UI_InvokeSingleTapListener,
                                           screenX, screenY, ids));
            }
            m_eventQueue->AddTask(new CCCTaskInvokeUnselectAllPinsListener(this, &MapView::CCC_UnselectAllPinsListener));
        }
        else
        {
            m_eventQueue->AddTask(new CCCTaskInvokePinClickedListener(
                                       this,
                                       &MapView::CCC_PinClickedListener,
                                       *newSelectedPinId));
        }

    }

    if (!isHit)
    {
        // Check if avatar hitted under any GPS mode
        bool avatarHit = m_nbgmViewController->IsAvatarHit(screenX, screenY);
        // Avatar bubble will not be handled here for now.
        //m_nbgmViewController->EnableLocationBubble(avatarHit);

        if(avatarHit && m_pListener)
        {
            hasTouchAnything = true;
            m_UITaskQueue->AddTask(new UITaskAvatarClickedListener(
                this,
                &MapView::UI_AvatarClickedListener,
                m_lastGpsLocation.latitude,
                m_lastGpsLocation.longitude));
        }
    }

    if(!hasTouchAnything)
    {
        double mercatorX = 0.0;
        double mercatorY = 0.0;
        double lat = 0.0;
        double lon = 0.0;
        m_mapViewCamera->ScreenToMapPosition(screenX, screenY, mercatorX, mercatorY);
        mercatorReverse(mercatorX, mercatorY, &lat, &lon);
        m_UITaskQueue->AddTask(new UITaskOnTap(this, &MapView::UI_OnTap, lat, lon));
    }
}

// AsyncGenerateMapImageCallback ...........................................................

/* See MapView.h for description */
AsyncGenerateMapImageCallback::AsyncGenerateMapImageCallback(MapView* pMapView,
 const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > asyncGenerateMapImageCallback,
 shared_ptr<bool> screenshotInProgress)
    : m_pAsyncGenerateMapImageCallback(asyncGenerateMapImageCallback),
      m_pScreenshotInProgress(screenshotInProgress),
      m_pMapView(pMapView)
{
    if (m_pMapView)
    {
        m_isValid = m_pMapView->IsValid();
    }
}

/* See MapView.h for description */
void
AsyncGenerateMapImageCallback::OnScreenshotFinished(int /*width*/, int /*height*/, const uint8* buffer, uint32 bufferSize)
{
    do
    {
       if (!m_pScreenshotInProgress || !(*m_pScreenshotInProgress))
       {
           break;
       }

       if (!m_pMapView || !m_isValid || !(*m_isValid))
       {
           return;
       }

       if (m_pAsyncGenerateMapImageCallback)
       {
           if (bufferSize > 0 && buffer)
           {
               DataStreamPtr dataStream = DataStream::Create();
               if (dataStream)
               {
                  dataStream->AppendData(buffer, bufferSize);
                  m_pMapView->NotifyAsyncGenerateMapImageCallback(m_pAsyncGenerateMapImageCallback, dataStream, NE_OK);
                  break;
               }
           }

           m_pMapView->NotifyAsyncGenerateMapImageCallback(m_pAsyncGenerateMapImageCallback, shared_ptr<nbcommon::DataStream>(), NE_BADDATA);
       }
    }while(0);

    *m_pScreenshotInProgress = false;
}

NB_Error
MapView::SetCustomPinBubbleResolver(shared_ptr<PinBubbleResolver> bubbleResolver)
{
    if (m_initialize.IsShuttingDown() || !m_eventQueue)
    {
        return NE_NOTINIT;
    }

    if (!bubbleResolver)
    {
        return NE_INVAL;
    }

    MapViewTask* pTask = new MapViewTaskSetCustomPinBubbleResolver(this, &MapView::CCC_SetCustomPinBubbleResolver, bubbleResolver);
    if(!pTask)
    {
        return NE_NOMEM;
    }

    m_eventQueue->AddTask(pTask);
    return NE_OK;
}

void
MapView::CCC_Prefetch(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskPrefetch* task = static_cast<const MapViewTaskPrefetch*>(pTask);
    if (!task)
    {
        return;
    }

    vector<NBGM_Point2d64> frustum;
    vector<Point2d64> tmpFrustum;
    vector<NBGM_Point2d64> frustumRasterNear;
    m_mapViewCamera->GetFrustum(frustum, frustumRasterNear);
    // All frustums have to have 4 points/corners
    uint16 frustumSize = 4;

    // Validate input
    if (frustum.size() != frustumSize)
    {
        return;
    }

    std::vector<nbmap::TileKeyPtr> tilesToPrefetch;

    tmpFrustum.resize(4);
    for (size_t i = 0; i < 4; ++i)
    {
        tmpFrustum[i].x = frustum[i].x;
        tmpFrustum[i].y = frustum[i].y;
    }
    m_pPrefetchProcessor->Prefetch(task->m_zoomLevel, tmpFrustum, tilesToPrefetch);
    if(tilesToPrefetch.size() > 0)
    {
        m_layerManager->GetTiles(tilesToPrefetch, NULL, true, true);
    }
}

void
MapView::CCC_PrefetchForNAV(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown())
    {
        return;
    }

    const MapViewTaskPrefetchForNAV* task = static_cast<const MapViewTaskPrefetchForNAV*>(pTask);
    if (task && task->m_polylineList)
    {
        std::vector<nbmap::TileKeyPtr> tilesToPrefetch;
        m_pPrefetchProcessor->Prefetch(task->m_polylineList, task->m_prefetchExtensionLengthMeters,
                task->m_prefetchExtensionWidthMeters, task->m_zoomLevel, tilesToPrefetch);
        if(tilesToPrefetch.size() > 0)
        {
            m_layerManager->GetTiles(tilesToPrefetch, NULL, true, true);
        }
    }
}

void
MapView::Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
        double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters,
        float zoomLevel)
{
    //If the zoom level > maxRasterZoomLevel, then this will cause the huge calculation(check which tile should download...) in the prefetch logic.
    //Because of the all vector tile's reference grid level are 16, so we will convert the zoom level to 'm_maxRasterZoomLevel + 1' in this case.
    zoomLevel = (zoomLevel > m_maxRasterZoomLevel) ? (m_maxRasterZoomLevel + 1) : zoomLevel;
    MapViewTask* pTask = new MapViewTaskPrefetchForNAV(this, &MapView::CCC_PrefetchForNAV, polylineList, prefetchExtensionLengthMeters, prefetchExtensionWidthMeters, zoomLevel);
    if(!pTask)
    {
        return;
    }

    m_eventQueue->AddTask(pTask);
}

void
MapView::Prefetch(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    MapViewTask* pTask = new MapViewTaskPrefetch(this, &MapView::CCC_Prefetch, lat, lon, zoomLevel, heading, tilt);
    if(!pTask)
    {
        return;
    }

    m_eventQueue->AddTask(pTask);
}

void
MapView::CCC_SetCustomPinBubbleResolver(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || !m_pinCushion)
    {
        return;
    }

    const MapViewTaskSetCustomPinBubbleResolver* task = static_cast<const MapViewTaskSetCustomPinBubbleResolver*>(pTask);
    if (task && task->m_resolver)
    {
        m_pinCushion->SetCustomPinBubbleResolver(task->m_resolver);
    }
}

NB_Error
MapView::GetCamera(MapViewCameraParameter& cameraParameter)
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_mapViewCamera)
    {
        return NE_INST;
    }

    m_mapViewCamera->GetCamera(cameraParameter.latitude, cameraParameter.longitude, cameraParameter.zoomLevel, cameraParameter.heading, cameraParameter.tilt);

    return NE_OK;
}

void
MapView::GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }
    m_nbgmViewController->GetCompassBoundingBox(leftBottomX, leftBottomY, rightTopX, rightTopY);
}

void
MapView::StopAnimations(int animationType)
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_mapViewCamera)
    {
        return;
    }

    if(animationType == MVAT_NONE)
    {
        return;
    }
    // Add a render task to stop the animation
    if(m_mapViewCamera)
    {
        m_mapViewCamera->StopAnimations(animationType);
    }
}

void
MapView::UI_UpdateSurface()
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_nativeView)
    {
        return;
    }
    m_nativeView->UpdateSurface();
}

void
MapView::UI_DestroySurface()
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || !m_nativeView)
    {
        return;
    }
    m_nativeView->DestroySurface();
}

/* See description in header file. */
void
MapView::PlayDoppler()
{
    /* This function should be called in UI thread */

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    if (m_pRenderingQueue)
    {
        m_pRenderingQueue->AddTask(new RenderTaskUpdateDopplerState(this, &MapView::RenderThread_UpdateDopplerState, false, PAL_ClockGetTimeMs()));
    }
    m_renderPending = true;
}

/* See description in header file. */
void
MapView::PauseDoppler()
{
    /* This function should be called in UI thread */

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    if (m_pRenderingQueue)
    {
        m_pRenderingQueue->AddTask(new RenderTaskUpdateDopplerState(this, &MapView::RenderThread_UpdateDopplerState, true, PAL_ClockGetTimeMs()));
    }
}

/* See description in header file. */
void
MapView::SetReferenceCenter(float x, float y)
{
    AddRenderThreadTask(new RenderTaskSetReferenceCenter(this, &MapView::RenderThread_SetReferenceCenter, x, y));
}

/* See description in header file. */
void
MapView::EnableReferenceCenter(nb_boolean enable)
{
    AddRenderThreadTask(new RenderTaskEnableReferenceCenter(this, &MapView::RenderThread_EnableReferenceCenter, enable));
}

/* See description in header file. */
void
MapView::RenderThread_UpdateDopplerState(const MapViewTask* pTask)
{
    if(m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskUpdateDopplerState* task = static_cast<const RenderTaskUpdateDopplerState*>(pTask);
    if(task)
    {
        m_dopplerState.pauseByBar = task->m_pause;

        if(task->m_pause)
        {
            m_dopplerState.pauseTime = task->m_time;
        }
        else
        {
            m_dopplerState.playTime = task->m_time;
            m_dopplerState.checkPauseTimeInterval = true;
        }
        m_dopplerState.isPlaying = task->m_pause ? false : true;
    }
}

/* See description in header file. */
bool
MapView::RenderThread_PlayDoppler(bool& nextFrame, bool& beginningFrame)
{
    if(m_dopplerState.pauseByBar)
    {
        return false;
    }

    if(m_dopplerState.pauseByGesture)
    {
        return false;
    }

    // This value comes from //depot/Projects/756 - NexGen/Draft/Documents/UI/NexGen_Doppler.pdf
    const uint32 TIME_INTERVAL = 3 * 60 * 1000;
    if(m_dopplerState.checkPauseTimeInterval)
    {
        uint32 deltaTime = m_dopplerState.playTime - m_dopplerState.pauseTime;
        if(deltaTime > TIME_INTERVAL)
        {
            nextFrame = false;
            beginningFrame = true;
        }
    }
    else
    {
        nextFrame = false;
        beginningFrame = false;
    }

    m_dopplerState.checkPauseTimeInterval = false;

    return true;
}

void MapView::RenderThread_SetReferenceCenter(const MapViewTask* pTask)
{
    const RenderTaskSetReferenceCenter* task = static_cast<const RenderTaskSetReferenceCenter*>(pTask);
    m_mapViewCamera->SetReference(task->m_x, task->m_y);
}

void MapView::RenderThread_EnableReferenceCenter(const MapViewTask* pTask)
{
    const RenderTaskEnableReferenceCenter* task = static_cast<const RenderTaskEnableReferenceCenter*>(pTask);
    m_mapViewCamera->EnableReference(task->m_enable);
}

void MapView::UI_AvatarClickedListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    const UITaskAvatarClickedListener* task = static_cast<const UITaskAvatarClickedListener*>(pTask);
    if (task)
    {
        m_pListener->OnAvatarClicked(task->m_lat, task->m_lon);
    }
}

void MapView::AddCircle(int circleId,  double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || circleId == 0)
    {
        return;
    }

    NBGM_CircleParameters circlePara;
    memset(&circlePara, 0, sizeof(circlePara));
    // Transfrom coordinate to mercator.
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(lat, lon, &mercatorX, &mercatorY);
    circlePara.centerX = mercatorX;
    circlePara.centerY = mercatorY;
    circlePara.radius = radius;
    NBGM_Color color;
    color.red = ((fillColor >> 24) & 0xFF) / 255.0f;
    color.green = ((fillColor >> 16) & 0xFF) / 255.0f;
    color.blue = ((fillColor >> 8) & 0xFF) / 255.0f;
    color.alpha = (fillColor & 0xFF) / 255.0f;
    circlePara.fillColor = color;
    color.red = ((strokeColor >> 24) & 0xFF) / 255.0f;
    color.green = ((strokeColor >> 16) & 0xFF) / 255.0f;
    color.blue = ((strokeColor >> 8) & 0xFF) / 255.0f;
    color.alpha = (strokeColor & 0xFF) / 255.0f;
    circlePara.strokeColor = color;
    circlePara.zOrder = zOrder;
    circlePara.visible = visible;
    AddRenderThreadTask(new MapViewTaskAddCircle(this, &MapView::RenderThread_AddCircle, circleId, circlePara));
}


void MapView::RemoveCircle(int circleId)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || circleId == 0)
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskRemoveCircle(this, &MapView::RenderThread_RemoveCircle, circleId));
}

void MapView::RemoveAllCircles()
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) )
    {
        return;
    }
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_RemoveAllCircles));
}


void MapView::SetCircleCenter(int circleId, double lat, double lon)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || circleId == 0)
    {
        return;
    }

    AddRenderThreadTask(new MapViewTaskSetCircleCenter(this, &MapView::RenderThread_SetCircleCenter, circleId, lat, lon));
}

void MapView::SetCircleVisible(int circleId, bool visible)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || circleId == 0)
    {
        return;
    }

    AddRenderThreadTask(new MapViewTaskSetCircleVisible(this, &MapView::RenderThread_SetCircleVisible, circleId, visible));
}

void MapView::SetCircleStyle(int circleId, float radius, int fillClr, int outlineClr)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || circleId == 0)
    {
        return;
    }

    AddRenderThreadTask(new MapViewTaskSetCircleStyle(this, &MapView::RenderThread_SetCircleStyle, circleId, radius, fillClr, outlineClr));
}

void MapView::RenderThread_AddCircle(const MapViewTask* pTask)
{
    const MapViewTaskAddCircle* task = static_cast<const MapViewTaskAddCircle*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->AddCircle(task->m_circleId, task->m_circlePara);
    m_renderPending = true;
}

void MapView::RenderThread_RemoveCircle(const MapViewTask* pTask)
{
    const MapViewTaskRemoveCircle* task = static_cast<const MapViewTaskRemoveCircle*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveCircle(task->m_circleId);
    m_renderPending = true;
}

void MapView::RenderThread_RemoveAllCircles(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveAllCircles();
    m_renderPending = true;
}

void MapView::RenderThread_SetCircleCenter(const MapViewTask* pTask)
{
    const MapViewTaskSetCircleCenter* task = static_cast<const MapViewTaskSetCircleCenter*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(task->m_lat, task->m_lon, &mercatorX, &mercatorY);
    m_nbgmViewController->SetCircleCenter(task->m_circleId, mercatorX, mercatorY);
    m_renderPending = true;
}

void MapView::RenderThread_SetCircleVisible(const MapViewTask* pTask)
{
    const MapViewTaskSetCircleVisible* task = static_cast<const MapViewTaskSetCircleVisible*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->SetCircleVisible(task->m_circleId, task->m_visible);
    m_renderPending = true;
}

void MapView::RenderThread_SetCircleStyle(const MapViewTask* pTask)
{
    const MapViewTaskSetCircleStyle* task = static_cast<const MapViewTaskSetCircleStyle*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    int fillColor = task->m_fillColor;
    int outlineColor = task->m_outlineColor;
    NBGM_Color fillClr;
    fillClr.red = ((fillColor >> 24) & 0xFF) / 255.0f;
    fillClr.green = ((fillColor >> 16) & 0xFF) / 255.0f;
    fillClr.blue = ((fillColor >> 8) & 0xFF) / 255.0f;
    fillClr.alpha = (fillColor & 0xFF) / 255.0f;
    NBGM_Color outlineClr;
    outlineClr.red = ((outlineColor >> 24) & 0xFF) / 255.0f;
    outlineClr.green = ((outlineColor >> 16) & 0xFF) / 255.0f;
    outlineClr.blue = ((outlineColor >> 8) & 0xFF) / 255.0f;
    outlineClr.alpha = (outlineColor & 0xFF) / 255.0f;

    m_nbgmViewController->SetCircleStyle(task->m_circleId, task->m_radius, fillClr, outlineClr);
    m_renderPending = true;
}

void MapView::AddRect2d(int rectId,  double lat, double lon, float heading, float width, float height, int textureId, shared_ptr<nbcommon::DataStream> imageBuffer, bool visible)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || rectId == 0)
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskAddRect2d(this, &MapView::RenderThread_AddRect2d, rectId, lat, lon, heading, width, height, textureId, imageBuffer, visible));
}


void MapView::RemoveRect2d(int rectId)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskRemoveRect2d(this, &MapView::RenderThread_RemoveRect2d, rectId));
}

void MapView::SetRect2dVisible(int rectId, bool visible)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskSetRect2dVisible(this, &MapView::RenderThread_SetRect2dVisible, rectId, visible));
}

void MapView::SetRect2dSize(int rectId, float width, float height)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskSetRect2dSize(this, &MapView::RenderThread_SetRect2dSize, rectId, width, height));
}

void MapView::UpdateRect2d(int rectId, double lat, double lon, float heading)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskUpdateRect2d(this, &MapView::RenderThread_UpdateRect2d, rectId, lat, lon, heading));
}

void MapView::RemoveAllRect2ds()
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) )
    {
        return;
    }
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_RemoveAllRect2ds));
}

void MapView::RenderThread_AddRect2d(const MapViewTask* pTask)
{
    const MapViewTaskAddRect2d* task = static_cast<const MapViewTaskAddRect2d*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    NBGM_Rect2dParameters para;
    memset(&para, 0, sizeof(para));
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(task->m_lat, task->m_lon, &mercatorX, &mercatorY);
    para.centerX = mercatorX;
    para.centerY = mercatorY;
    para.heading = task->m_heading;
    para.width = task->m_width;
    para.height = task->m_height;
    para.textureId = task->m_textureId;
    para.visible = task->m_visible;
    if(task->m_textureData)
    {
        para.buffer.size = task->m_textureData->GetDataSize();
        if(para.buffer.size > 0)
        {
            para.buffer.addr = (uint8*)nsl_malloc(para.buffer.size);
        }
        if(para.buffer.addr)
        {
            nsl_memset(para.buffer.addr , 0, para.buffer.size);
            task->m_textureData->GetData(para.buffer.addr, 0, para.buffer.size);
        }
    }
    m_nbgmViewController->AddRect2d(task->m_rectId, para);
    m_renderPending = true;
    if(para.buffer.addr)
    {
        nsl_free(para.buffer.addr);
    }
}

void MapView::RenderThread_RemoveRect2d(const MapViewTask* pTask)
{
    const MapViewTaskRemoveRect2d* task = static_cast<const MapViewTaskRemoveRect2d*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveRect2d(task->m_rectId);
    m_renderPending = true;
}

void MapView::RenderThread_SetRect2dVisible(const MapViewTask* pTask)
{
    const MapViewTaskSetRect2dVisible* task = static_cast<const MapViewTaskSetRect2dVisible*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->SetRect2dVisible(task->m_rectId, task->m_visible);
    m_renderPending = true;
}

void MapView::RenderThread_SetRect2dSize(const MapViewTask* pTask)
{
    const MapViewTaskSetRect2dSize* task = static_cast<const MapViewTaskSetRect2dSize*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->SetRect2dSize(task->m_rectId, task->m_width, task->m_height);
    m_renderPending = true;
}

void MapView::RenderThread_UpdateRect2d(const MapViewTask* pTask)
{
    const MapViewTaskUpdateRect2d* task = static_cast<const MapViewTaskUpdateRect2d*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    double mercatorX = 0.0;
    double mercatorY = 0.0;
    mercatorForward(task->m_lat, task->m_lon, &mercatorX, &mercatorY);
    m_nbgmViewController->UpdateRect2d(task->m_rectId, mercatorX, mercatorY, task->m_heading);
    m_renderPending = true;
}

void MapView::RenderThread_RemoveAllRect2ds(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveAllRect2ds();
    m_renderPending = true;
}

void MapView::AddTexture(int textureId, nbcommon::DataStreamPtr textureData)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || textureId == 0 || !textureData)
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskAddTexture(this, &MapView::RenderThread_AddTexture, textureId, textureData));
}

void MapView::RemoveTexture(int textureId)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new MapViewTaskRemoveTexture(this, &MapView::RenderThread_RemoveTexture, textureId));
}

void MapView::RemoveAllTextures()
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) )
    {
        return;
    }
    AddRenderThreadTask(new MapViewTask(this, &MapView::RenderThread_RemoveAllTextures));
}

void MapView::RenderThread_AddTexture(const MapViewTask* pTask)
{
    const MapViewTaskAddTexture* task = static_cast<const MapViewTaskAddTexture*>(pTask);
    if(!task || task->m_textureId == 0 || !task->m_textureData)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    NBGM_BinaryBuffer buffer;
    buffer.size = task->m_textureData->GetDataSize();
    if(buffer.size > 0)
    {
        buffer.addr = (uint8*)nsl_malloc(buffer.size);
        if(buffer.addr)
        {
            nsl_memset(buffer.addr, 0, buffer.size);
            task->m_textureData->GetData(buffer.addr, 0, buffer.size);
            m_nbgmViewController->AddTexture(task->m_textureId, buffer);
            nsl_free(buffer.addr);
        }
    }
}

void MapView::RenderThread_RemoveTexture(const MapViewTask* pTask)
{
    const MapViewTaskRemoveTexture* task = static_cast<const MapViewTaskRemoveTexture*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveTexture(task->m_textureId);
}

void MapView::RenderThread_RemoveAllTextures(const MapViewTask* /*pTask*/)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    m_nbgmViewController->RemoveAllTextures();
}


void MapView::UI_OnMapReady(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (pTask == NULL))
    {
        return;
    }

    if (m_pListener)
    {
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_CRITICAL, ("####UI_OnMapReady-Notify UI"));
        m_pListener->OnMapReady();
    }
}

void MapView::UI_OnCameraAnimationDone(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const MapViewTaskCameraAnimationDone* pAnimationTask = static_cast<const MapViewTaskCameraAnimationDone*>(pTask);
    if(pAnimationTask == NULL)
    {
        return;
    }

    if (m_pListener)
    {
        m_pListener->OnCameraAnimationDone(pAnimationTask->m_animationId, pAnimationTask->m_animationStatus);
    }
}

void MapView::OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_UITaskQueue)
    {
        return;
    }

    m_UITaskQueue->AddTask(new MapViewTaskCameraAnimationDone(
                               this,
                               &MapView::UI_OnCameraAnimationDone,
                               animationId,
                               animationStatus));
}



void MapView::OnCameraLimited(MapViewCameraLimitedType type)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_UITaskQueue)
    {
        return;
    }
    m_UITaskQueue->AddTask(new UITaskCameraLimted(this, &MapView::UI_CameraLimited, type));
}

void MapView::UI_StaticPOIClickedListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    const UITaskStaticPOIClickedListener* task = static_cast<const UITaskStaticPOIClickedListener*>(pTask);
    if (task)
    {
        m_pListener->OnStaticPOIClicked(task->m_id, task->m_name, task->m_lat, task->m_lon);
    }
}

void MapView::UI_TrafficIncidentPinClickedListener(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    const UITaskTrafficIncidentPinClickedListener* task = static_cast<const UITaskTrafficIncidentPinClickedListener*>(pTask);
    if (task)
    {
        m_pListener->OnTrafficIncidentPinClicked(task->m_lat, task->m_lon);
    }
}

void MapView::SetFontScale(float scale)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetFontScale(this, &MapView::RenderThread_SetFontScale, scale));
}

void
MapView::RenderThread_SetFontScale(const MapViewTask* pTask)
{
    const RenderTaskSetFontScale* task = static_cast<const RenderTaskSetFontScale*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    m_nbgmViewController->SetFontScale(task->m_scale);
    m_renderPending = true;
}

void MapView::SetAvatarScale(float scale)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    AddRenderThreadTask(new RenderTaskSetAvatarScale(this, &MapView::RenderThread_SetAvatarScale, scale));
}

void
MapView::RenderThread_SetAvatarScale(const MapViewTask* pTask)
{
    const RenderTaskSetAvatarScale* task = static_cast<const RenderTaskSetAvatarScale*>(pTask);
    if(!task)
    {
        return;
    }

    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()) || (!m_nbgmViewController))
    {
        return;
    }

    m_nbgmViewController->SetAvatarScale(task->m_scale);
    m_renderPending = true;
}

void MapView::UI_CameraLimited(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    const UITaskCameraLimted* task = static_cast<const UITaskCameraLimted*> (pTask);
    if (task)
    {
        m_pListener->OnCameraLimited(task->m_type);
    }
}

void MapView::OnLongPressed(double lat, double lon)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || (m_pListener == NULL))
    {
        return;
    }

    if(m_pListener)
    {
        m_pListener->OnLongPressed(lat, lon);
    }
}

void MapView::SelectAvatarAndPin(float screenX, float screenY)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    RenderTaskSelectAvatarAndPinTask* task = new RenderTaskSelectAvatarAndPinTask(this, &MapView::RenderThread_SelectAvatarAndPin, screenX,screenY);
    AddRenderThreadTask(task);
}

void MapView::UpdateDopplerByGesture(bool isUpdate)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    RenderTaskUpdateDopplerByGesture* task = new RenderTaskUpdateDopplerByGesture(this, &MapView::RenderThread_UpdateDopplerStateByGesture, isUpdate);
    AddRenderThreadTask(task);
}

void MapView::RenderThread_UpdateDopplerStateByGesture(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskUpdateDopplerByGesture* task = static_cast<const RenderTaskUpdateDopplerByGesture*>(pTask);
    if (!task)
    {
        return;
    }

    if(task->m_needUpdate)
    {
        if(!m_dopplerState.pauseByBar)
        {
            m_dopplerState.pauseByGesture = true;
            m_dopplerState.pauseTime = PAL_ClockGetTimeMs();
            //Keep the last timestamp and notify it couldn't play animation.
            m_UITaskQueue->AddTask(new MapViewTaskUIAnimationLayer(this, &MapView::UI_HandleAnimationLayer, false, false, m_lastAnimationTimestamp, false));
        }
    }
    else
    {
        if(!m_dopplerState.pauseByBar)
        {
            m_dopplerState.pauseByGesture = false;
            m_dopplerState.checkPauseTimeInterval = true;
            m_dopplerState.playTime = PAL_ClockGetTimeMs();
        }
    }
}

void
MapView::SetGestureProcessing(bool isProcessing)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    RenderTaskSetGestureProcessing* task = new RenderTaskSetGestureProcessing(this, &MapView::RenderThread_RenderTaskSetGestureProcessing, isProcessing);
    AddRenderThreadTask(task);
}

void
MapView::RenderThread_RenderTaskSetGestureProcessing(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskSetGestureProcessing* task = static_cast<const RenderTaskSetGestureProcessing*>(pTask);
    if (!task)
    {
        return;
    }

    if(m_nbgmViewController)
    {
        m_nbgmViewController->SetGestureProcessingFlag(task->m_isProcessing);
    }
}

void
MapView::UI_GenerateMapImage(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const UItaskGenerateMapImage* task = static_cast<const UItaskGenerateMapImage*>(pTask);

    if (!task)
    {
        return;
    }

    if (task->m_dataStream)
    {
        task->m_callback->OnSnapShotSuccessed(task->m_dataStream, task->m_width, task->m_height);
    }
    else
    {
        task->m_callback->OnSnapShotError(NE_BADDATA);
    }
}

MapView::CombinedAtomicTaskExecuter::CombinedAtomicTaskExecuter(MapView* mapView, PAL_Instance* pal)
:m_mapView(mapView)
,m_CombinedAtomicTask(NULL)
,m_Lock(NULL)
{
    PAL_LockCreate(pal, &m_Lock);
}

MapView::CombinedAtomicTaskExecuter::~CombinedAtomicTaskExecuter()
{
    PAL_LockDestroy(m_Lock);
}

void MapView::CombinedAtomicTaskExecuter::Enable(nb_boolean enable)
{
    PAL_LockLock(m_Lock);
    if(enable)
    {
        if(!m_CombinedAtomicTask)
        {
            m_CombinedAtomicTask = new CombinedAtomicTask(m_mapView, &MapView::RenderThread_RunCombinedAtomicTask);
        }
    }
    else
    {
        if(m_CombinedAtomicTask)
        {
            m_mapView->m_pRenderingQueue->AddTask(m_CombinedAtomicTask);
            m_CombinedAtomicTask = NULL;
        }
    }
    PAL_LockUnlock(m_Lock);
}

nb_boolean MapView::CombinedAtomicTaskExecuter::AddTask(MapViewTask *task)
{
    PAL_LockLock(m_Lock);
    nb_boolean isAdded = FALSE;
    if(m_CombinedAtomicTask)
    {
        m_CombinedAtomicTask->AddTask(task);
        isAdded = TRUE;
    }
    PAL_LockUnlock(m_Lock);
    return isAdded;
}

void MapView::CombinedAtomicTaskExecuter::ClearAllTask()
{
    PAL_LockLock(m_Lock);
    if(m_CombinedAtomicTask)
    {
        m_CombinedAtomicTask->ClearAllTasks();
    }
    PAL_LockUnlock(m_Lock);
}

void MapView::UI_SetGpsMode(NB_GpsMode mode)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    m_contextUI.lastGpsMode = mode;

    // @todo hardcode just for lkt qt project
    switch (mode)
    {
    case NGM_STAND_BY:
        {
            EnableReferenceCenter(FALSE);
        }
    break;
    case NGM_FOLLOW_ME_ANY_HEADING:
        {
            SetReferenceCenter(m_viewSizeWidth / 2.0f, m_viewSizeHeight / 2.0f);
            EnableReferenceCenter(TRUE);
        }
    break;
    case NGM_FOLLOW_ME:
        {
            SetReferenceCenter(m_viewSizeWidth / 2.0f, m_viewSizeHeight / 2.0f);
            EnableReferenceCenter(TRUE);
        }
    break;
    default:
        {
            EnableReferenceCenter(FALSE);
        }
    break;
    }
}

void MapView::UI_SetDopplerOpacity(uint32 opacity)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    // Switch to render thread
    if(m_pRenderingQueue)
    {
        m_pRenderingQueue->AddTask(new RenderTaskUpdateOpacity(this, &MapView::RenderThread_UpdateAnimationLayerOpacity, opacity));
        m_renderPending = true;
    }
}

void MapView::UI_SetDopplerPlayState(bool play)
{
    if (m_initialize.IsShuttingDown() || (! m_initialize.IsDone()))
    {
        return;
    }

    // Switch to render thread
    if (m_pRenderingQueue)
    {
        m_pRenderingQueue->AddTask(new RenderTaskUpdateDopplerState(this, &MapView::RenderThread_UpdateDopplerState, !play, PAL_ClockGetTimeMs()));
        m_renderPending = true;
    }
}

void
MapView::AddPins(std::vector<PinParameters<nbmap::BubbleInterface> >& parametersVector)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController || parametersVector.empty())
    {
        return;
    }

    if (!m_pinHandler)
    {
        m_pinHandler = new MapViewPinHandler(m_pinManager->GetMaterialConfigPath());
    }

    if (m_pinHandler)
    {
        std::vector<NBGM_PinParameters> NBGMPinParametersVector;
        for (uint32 i = 0; i < parametersVector.size(); ++i)
        {
            NBGM_PinParameters pinParameters;
            m_pinHandler->AddPin(parametersVector[i], pinParameters);
            NBGMPinParametersVector.push_back(pinParameters);

        }
        AddRenderThreadTask(new RenderTaskAddPins(this, &MapView::RenderThread_AddPins, NBGMPinParametersVector));

        m_renderPending = true;
    }
}

void
MapView::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    if (m_pinHandler)
    {
        m_pinHandler->RemovePins(pinIDs);
        AddRenderThreadTask(new RenderTaskRemovePins(this, &MapView::RenderThread_RemovePins, pinIDs));
    }
}

void
MapView::RemoveAllPins()
{
    if (m_pinHandler)
    {
        m_pinHandler->RemoveAllPins();
        AddRenderThreadTask(new RenderTaskRemoveAllPins(this, &MapView::RenderThread_RemoveAllPins));
    }
}

void
MapView::UpdatePinPosition(shared_ptr<string> pinID, double lat, double lon)
{
    if (m_pinHandler)
    {
        m_pinHandler->UpdatePinPosition(pinID, lat, lon);
        AddRenderThreadTask(new RenderTaskUpdatePinPosition(this, &MapView::RenderThread_UpdataPinPosition, pinID, lat, lon));
    }
}


void
MapView::SetHBAOParameters(const HBAOParameters& parameters)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new RenderTaskSetHBAOParameters(this, &MapView::RenderThread_SetHBAOParameters, parameters));
    m_renderPending = true;
}

void
MapView::SetGlowParameters(const GlowParameters& parameters)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new RenderTaskSetGlowParameters(this, &MapView::RenderThread_SetGlowParameters, parameters));
    m_renderPending = true;
}


void
MapView::SetDPI(float dpi)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }
    AddRenderThreadTask(new RenderTaskSetDPI(this, &MapView::RenderThread_SetDPI, dpi));
    m_renderPending = true;
}

shared_ptr<nbmap::BubbleInterface>
MapView::UI_GetBubble(shared_ptr<std::string> pinID)
{
    if (m_pinHandler)
    {
        return m_pinHandler->GetBubble(pinID);
    }

    return shared_ptr<nbmap::BubbleInterface>();
}

void
MapView::RenderThread_AddPins(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskAddPins* task = static_cast<const RenderTaskAddPins*>(pTask);
    if (!task)
    {
        return;
    }

    std::vector<NBGM_PinParameters> NBGMPinParametersVector = task->m_pinParameters;

    m_nbgmViewController->AddPins(NBGMPinParametersVector);

    for (uint32 i = 0; i < NBGMPinParametersVector.size(); ++i)
    {
        NBGM_PinParameters pinParameters = NBGMPinParametersVector[i];
        if (pinParameters.unSelectedDataBinary.addr)
        {
            delete []pinParameters.unSelectedDataBinary.addr;
        }

        if (pinParameters.selectedDataBinary.addr)
        {
            delete []pinParameters.selectedDataBinary.addr;
        }
    }

    m_renderPending = true;

}

void
MapView::RenderThread_RemovePins(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskRemovePins* task = static_cast<const RenderTaskRemovePins*>(pTask);
    if (!task)
    {
        return;
    }

    m_nbgmViewController->RemovePins(task->m_pinIDs);
    m_renderPending = true;
}

void
MapView::RenderThread_RemoveAllPins(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskRemoveAllPins* task = static_cast<const RenderTaskRemoveAllPins*>(pTask);
    if (!task)
    {
        return;
    }

    m_nbgmViewController->RemoveAllPins();
    m_renderPending = true;
}

void
MapView::RenderThread_UpdataPinPosition(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskUpdatePinPosition* task = static_cast<const RenderTaskUpdatePinPosition*>(pTask);
    if (!task)
    {
        return;
    }

    double locationX = 0.0;
    double locationY = 0.0;
    mercatorForward(task->m_lat, task->m_lon, &locationX, &locationY);

    m_nbgmViewController->UpdatePinPosition(task->m_pinID, locationX, locationY);
    m_renderPending = true;
}

void
MapView::UI_GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const
{
    GetCompassBoundingBox(leftBottomX, leftBottomY, rightTopX, rightTopY);
}

void
MapView::RenderThread_SetHBAOParameters(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskSetHBAOParameters* task = static_cast<const RenderTaskSetHBAOParameters*>(pTask);

    if(task)
    {
        m_nbgmViewController->SetHBAOParameters(task->m_Parameters);
    }
}

void
MapView::RenderThread_SetGlowParameters(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskSetGlowParameters* task = static_cast<const RenderTaskSetGlowParameters*>(pTask);

    if(task)
    {
        m_nbgmViewController->SetGlowParameters(task->m_Parameters);
    }
}

void
MapView::RenderThread_SetDPI(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    const RenderTaskSetDPI* task = static_cast<const RenderTaskSetDPI*>(pTask);

    if(task)
    {
        m_nbgmViewController->SetDPI(task->m_dpi);
    }
}


void
MapView::RenderThread_CleanMaterilas(const MapViewTask* pTask)
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()) || !m_nbgmViewController)
    {
        return;
    }

    const RenderTaskUnloadMaterials *task = static_cast<const RenderTaskUnloadMaterials*> (pTask);
    if(!task)
    {
        return;
    }

    if (!(task->m_tilesToUnload.empty()))
    {
        // Inform any animation layers that tiles have been removed.
        m_frame.AnimationTilesRemoved(task->m_tilesToUnload);
        m_nbgmViewController->RemoveMapData(task->m_tilesToUnload);
    }

    // Must clean the loading/loaded tiles from NBGM cache first.
    m_nbgmViewController->UnloadAllTiles();
    // Unload existed external materials.
    m_nbgmViewController->UnloadCommonMaterials();
    m_refreshTilesAfterCommonMaterialUpdated = true;
}

void
MapView::CleanMaterials()
{
    if (m_initialize.IsShuttingDown() || (!m_initialize.IsDone()))
    {
        return;
    }

    // 1. Clean material record of CCC Mapview.
    m_commonMaterialTiles.clear();

    if(!m_nbgmViewController)
    {
        return;
    }

    // 2. Clean materials in NBGM.
    vector<TilePtr> tilesToUnload;
    m_frame.UnloadAllTiles(tilesToUnload);
    RenderTaskUnloadMaterials* UnloadMaterialsTask = new RenderTaskUnloadMaterials(this, &MapView::RenderThread_CleanMaterilas);
    if (!UnloadMaterialsTask)
    {
        return;
    }

    UnloadMaterialsTask->m_tilesToUnload = tilesToUnload;
    AddRenderThreadTask(UnloadMaterialsTask);
}

/*! @} */
