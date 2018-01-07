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

    @file     Main.c
    @date     01/01/09
    @defgroup SYSTEMTEST_H System Tests for NIM NAVBuilder Services

    This is the main file for all the system test.

    All test suites are loaded here and run from here.
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

/*! @{ */



#include <windows.h>
#include <winbase.h>
#include <tchar.h>

#include "CUnit.h"

extern "C"
{
#include "nbcontext.h"
#include "palfile.h"
#include "platformutil.h"
#include "testnetwork.h"
}

#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include "MapServicesConfiguration.h"
#include "PinBubbleResolver.h"
#include "PinManager.h"
#include "TestInstance.h"
#include "CustomLayerManagerImpl.h"

using namespace nbmap;
using namespace std;

static const double TEST_DEFAULT_LATITUDE = 33.548834;
static const double TEST_DEFAULT_LONGITUDE = -117.688637;
static const double TEST_DEFAULT_TILT_ANGLE = 40.0;

static const uint32 TEST_MAXIMUM_CACHING_TILE_COUNT          = 1000;
static const uint32 TEST_MAXIMUM_TILE_REQUEST_COUNT_PERLAYER = 64;

/*! Collection of all instances used for the unit tests */
class TestMapViewInstance : public TestInstance, public MapViewListener
{
public:
    /*! Default constructor. */
    TestMapViewInstance();

    /*! Default destructor. */
    virtual ~TestMapViewInstance();

    // Test Functions
    void TestMapViewInstanceCreate();

    /* See TestInstance.h */
    virtual void FinishTestPrivate();
    virtual void OnPinClicked(const std::string& id);
    virtual void OnAvatarClicked(double lat, double lon) {}

    //void TurnOffSatelliteLayer()
    //{
    //    vector<LayerAgentPtr> layerAgents = m_optionalLayersAgent->GetOptionalLayerAgents();
    //    vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
    //    vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
    //    for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
    //    {
    //        LayerAgentPtr layerAgent = *layerAgentIterator;
    //        if (layerAgent)
    //        {
    //            shared_ptr<string> layerName = layerAgent->GetName();
    //            if (layerName && !layerName->compare("Satellite"))
    //            {
    //                layerAgent->SetEnabled(false);
    //                break;
    //            }
    //        }
    //    }

    //}
    // Helper functions to test.
    void MapViewInstanceCreate();
	virtual void OnCameraUpdate(double,double,float,float,float){}
    virtual bool GestureDuringFollowMeMode(MapViewGestureType gesture) {return true;}
    virtual void LayersUpdated()
    {
//        if (m_optionalLayersAgent)
//        {
//#if 0
//
//            // Find the weather layer in the optional layers, according to ESD and SDS, ther
//            // will be only one DWR layer, and this layer will handle weather of all regions
//            // seamlessly. So just break the loop after finding the specified layer.
//            vector<LayerAgentPtr> layerAgents = m_agent->GetOptionalLayerAgents();
//            vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
//            vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
//            for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
//            {
//                LayerAgentPtr layerAgent = *layerAgentIterator;
//                if (layerAgent)
//                {
//                    shared_ptr<string> layerName = layerAgent->GetName();
//                    if (layerName && !layerName->compare("Weather"))
//                    {
//                        layerAgent->SetEnabled(true);
//                        break;
//                    }
//                }
//            }
//#endif // End of #if 0
//
//
//            // Find the weather layer in the optional layers, according to ESD and SDS, ther
//            // will be only one DWR layer, and this layer will handle weather of all regions
//            // seamlessly. So just break the loop after finding the specified layer.
//            vector<LayerAgentPtr> layerAgents = m_optionalLayersAgent->GetOptionalLayerAgents();
//            vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
//            vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
//            for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
//            {
//                LayerAgentPtr layerAgent = *layerAgentIterator;
//                if (layerAgent)
//                {
//                    shared_ptr<string> layerName = layerAgent->GetName();
//                    fprintf(stderr, "LayerName: %s\n", layerName->c_str());
//                    if (layerName && !layerName->compare("Satellite"))
//                    {
//                        layerAgent->SetEnabled(true);
//                        break;
//                    }
//                }
//            }
//
//            // Testing code: add Polylines...
//            vector<GeoPolylineOption> options;
//            CreatePolylineOptions(options);
//
//            uint32 z = 0;
//            vector<GeoPolylineOption>::const_iterator iter = options.begin();
//            vector<GeoPolylineOption>::const_iterator end  = options.end();
//            for (; iter != end; ++iter)
//            {
//                Geographic* graphic  = m_optionalLayersAgent->AddPolyline(*iter);
//                if (graphic)
//                {
//                    graphic->SetDrawOrder(++z);
//                    m_graphics.insert(map<string, Geographic*>::value_type(
//                                          graphic->GetId(), graphic));
//                }
//            }
//
//        }
    };

    virtual bool OnSingleTap(float screenX, float screenY, const set<string>& ids);

    void RemoveAllGraphics()
    {
    };

    virtual void OnMapReady() {}
    virtual void OnCameraAnimationDone(int,MapViewAnimationStatusType) {}
    virtual void OnNightModeChanged(bool isNightMode) {};
    virtual void OnUnselectAllPins() {};
    virtual void OnCameraLimited(MapViewCameraLimitedType type) {};

    virtual void OnTrafficIncidentPinClicked(double latitude, double longitude) {};

    virtual void OnStaticPOIClicked(const std::string& id, const std::string& name, double latitude, double longitude) {};
    virtual void OnMarkerClicked(int id) {};
    virtual void OnTap(double lat, double lon) {};


    MapViewInterface* m_pMapView;

    /*! CCC worker thread */
    shared_ptr<EventTaskQueue> m_eventQueue;

    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;

    //map<string, Geographic*> m_graphics;
    OptionalLayersAgentPtr m_optionalLayersAgent;
};

/* See description in header file. */
TestMapViewInstance::TestMapViewInstance()
        : TestInstance()
{
    // Get CCC and UI task queue
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(m_pal));
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(m_pal));

}

TestMapViewInstance::~TestMapViewInstance()
{
}

/* See TestInstance.h */
void TestMapViewInstance::FinishTestPrivate()
{
}

void TestMapViewInstance::OnPinClicked(const std::string& id)
{
}

bool TestMapViewInstance::OnSingleTap(float screenX, float screenY, const set<string>& ids)
{

    return true;
}

/*! Asynchronous callback used for MapView initialization.

    We have to wait before pushing the view to the forground until we receive this callback from CCC.
 */
class MapInitializeCallback : public AsyncCallback<void*>
{
public:
    MapInitializeCallback(TestMapViewInstance* testInstance) : m_testInstance(testInstance){};
    virtual ~MapInitializeCallback(){};

    /* Overwrites from AsyncCallback. This gets called once the map is fully initialized (or the initialization failed) */
    virtual void Success(void*)
    {
        // Trigger test function
        SetCallbackCompletedEvent(m_testInstance->m_completeEvent);
    };

    virtual void Error(NB_Error error)
    {
        CU_ASSERT_TRUE(error == NE_OK);
        CU_FAIL("Map Init failed");

        // Trigger test function
        SetCallbackCompletedEvent(m_testInstance->m_completeEvent);
    };

private:
    TestMapViewInstance* m_testInstance;
};


/*! Asynchronous callback used for MapView shutdown. */
class MapDestroyCallback : public AsyncCallback<void*>
{
public:
    MapDestroyCallback(TestMapViewInstance* testInstance) : m_testInstance(testInstance){};
    virtual ~MapDestroyCallback(){};

    /* Overwrites from AsyncCallback. This gets called once the map is fully destroyed */
    virtual void Success(void*)
    {
        // Trigger test function
        SetCallbackCompletedEvent(m_testInstance->m_completeEvent);
    };

    virtual void Error(NB_Error error)
    {
        CU_ASSERT_TRUE(error == NE_OK);
        CU_FAIL("Map Destroy failed");

        // Trigger test function
        SetCallbackCompletedEvent(m_testInstance->m_completeEvent);
    };

private:
    TestMapViewInstance* m_testInstance;
};


/* See description in header file. */
void TestMapViewInstance::TestMapViewInstanceCreate()
{
    if (!m_initialized)
    {
        return;
    }

    shared_ptr<AsyncCallback<void*> > initializeCallback(new MapInitializeCallback(this));
    shared_ptr<AsyncCallback<void*> > destroyCallback(new MapDestroyCallback(this));

    shared_ptr<PinManager> pinManager;
    shared_ptr<OptionalLayersAgent> optionalLayersAgent;
    shared_ptr<MapServicesConfiguration> mapConfiguration = MapServicesConfiguration::GetInstance();
    shared_ptr<MetadataConfiguration> metadataConfiguration = MetadataConfiguration::GetInstance(m_context);

    shared_ptr<nbmap::CustomLayerManager> customLayerManager = shared_ptr<CustomLayerManagerImpl>(new CustomLayerManagerImpl(m_context));


    metadataConfiguration->m_wantWeatherLayer    = true;
    metadataConfiguration->m_wantLableLayers     = false;
    metadataConfiguration->m_wantDAM             = false;
    metadataConfiguration->m_wantPoiLayers       = true;
    metadataConfiguration->m_wantSatelliteLayers = false;

    //mMapView = mapService->GetMap(mNBContext,
    //                              initializeCallback,
    //                              destroyCallback,
    //                              config.defaultLatitude,
    //                              config.defaultLongitude,
    //                              config.defaultTiltAngle,
    //                              config.maximumCachingTileCount,
    //                              config.maximumTileRequestCountPerLayer,
    //                              config.workFolder.c_str(),
    //                              config.resourceFolder.c_str(),
    //                              metadataConfigPtr,
    //                              mPinManager,
    //                              mCustomLayerManager,
    //                              mOptionalLayersAgent,
    //                              config.zorderLevel,
    //                              mapViewContext,
    //                              mDefaultMapViewListener);

    m_pMapView = mapConfiguration->GetMap(m_context,
                                          initializeCallback,
                                          destroyCallback,
                                          TEST_DEFAULT_LATITUDE,
                                          TEST_DEFAULT_LONGITUDE,
                                          TEST_DEFAULT_TILT_ANGLE,
                                          TEST_MAXIMUM_CACHING_TILE_COUNT,
                                          TEST_MAXIMUM_TILE_REQUEST_COUNT_PERLAYER,
                                          GetBasePath(),
                                          GetBasePath(),
                                          metadataConfiguration,
                                          pinManager,
                                          customLayerManager,
                                          m_optionalLayersAgent,
                                          -5,       // What do we need to set z-order level to?
                                          NULL,
                                          NULL
                                          );    // MapViewContext: I think we need to pass in a window handle here for the real test?


    //m_pMapView->RegisterListener(this);
    // The completion event -- either error or success -- are signaled in the callback functions.
}


// Define member callback function for task
typedef void (TestMapViewInstance::*MapViewTestTaskFunction)();

/*! Task to schedule CCC/UI tasks */
class MapViewTestTask : public Task
{
public:

    MapViewTestTask(TestMapViewInstance* pHandler, MapViewTestTaskFunction pFunction) : m_pHandler(pHandler), m_pFunction(pFunction){};
    virtual ~MapViewTestTask(){};

    virtual void Execute(void)
    {
        // Call task function
        (m_pHandler->*m_pFunction)();
    };

    TestMapViewInstance* m_pHandler;
    MapViewTestTaskFunction m_pFunction;
};


int _tmain(int argc, _TCHAR* argv[])
{
    TestMapViewInstance* instance = new TestMapViewInstance();

    // Schedule create map task in the CCC thread
    instance->m_eventQueue->AddTask(new MapViewTestTask(instance, &TestMapViewInstance::TestMapViewInstanceCreate));

    // Wait for it to finish. The event gets triggered in one of the callbacks
    (instance->WaitEvent(30 * 60 * 1000));    // !!!! set to high value for testing

    if (instance->m_pMapView)
    {
        // Set map to some test data

        // Aliso Viejo
//        instance->m_pMapView->SetPosition(33.548834,-117.688637);
//        instance->m_pMapView->SetZoomLevel(21.0, false);            // @todo: I don't think this works correctly.

        MapViewCameraParameter para;
        para.latitude = TEST_DEFAULT_LATITUDE;
        para.longitude = TEST_DEFAULT_LONGITUDE;
        para.zoomLevel = 17;
        para.heading = 0;
        para.tilt = 90;
        NB_Error result = instance->m_pMapView->SetCamera(para);


        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }




        // Shutdown MapView: This has to be called in the UI thread I think
        instance->m_pMapView->Destroy();

    }


    delete instance;

    return 0;
}


/*! @} */
