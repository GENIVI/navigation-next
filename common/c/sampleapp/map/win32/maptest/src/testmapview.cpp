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
#include "CUnit.h"

extern "C"
{
#include "TestMapView.h"
#include "main.h"
#include "nbcontext.h"
#include "testnetwork.h"
#include "platformutil.h"
#include "palfile.h"
}

#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include <stdio.h>
#include <vector>

#include "MapServicesConfiguration.h"
#include "PinBubbleResolver.h"
#include "PinManager.h"
#include "TestInstance.h"
#include <functional>
#include <algorithm>
#include <fstream>

using namespace nbmap;
using namespace std;


//  Local Constants ...........................................................................

static const uint32 CALLBACK_TIMEOUT = 120 * 1000;

// Test data: Boston.
static const double TEST_DEFAULT_LATITUDE = 33.548834;
static const double TEST_DEFAULT_LONGITUDE = -117.688637;
static const double TEST_DEFAULT_TILT_ANGLE = 40.0;

static const uint32 TEST_MAXIMUM_CACHING_TILE_COUNT          = 1000;
static const uint32 TEST_MAXIMUM_TILE_REQUEST_COUNT_PERLAYER = 64;

// Local Functions

#define MAX_POINTS       100000

typedef pair<double,double>   pt;

bool get_points(const char* fn,vector<pt>& vp)
{
    ifstream f(fn);
    std::streambuf * pbuf = f.rdbuf();
    std::streamsize size = pbuf->pubseekoff(0,f.end);
    pbuf->pubseekoff(0,f.beg);       // rewind
    char* contents = new char [(uint32)size];
    pbuf->sgetn (contents,size);
    f.close();

    char* ptr = contents;
    vector<float> vf;
    char* ep=  NULL;
    int idx = 0;
    pair<double, double> p;
    while ((ep = strstr(ptr, ",")) != NULL) {
        *ep = '\0';
        if (idx++ % 2)
        {
            p.second = atof(ptr);
            vp.push_back(p);
        }
        else
        {
            p.first = atof(ptr);
        }
        ptr = ep + 1;
    }
    return !vp.empty();
}

void CreatePolylineOptions(vector<GeoPolylineOption>& options)
{
    GeoPoint p = {TEST_DEFAULT_LATITUDE, TEST_DEFAULT_LONGITUDE};
    double delta = 0.00001;

    vector<GeoPoint>* vp = new vector<GeoPoint>();
    vector<GeoPolylineAttribute>* va = new vector<GeoPolylineAttribute>();
    vp->reserve(MAX_POINTS);

    vector<pt> v;

    uint32 colors[] = {0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF,0xFF00FFFF,
                       0x00FFFFFF, 0xFFFFFFFF};
    int index = 0;
    for (int i = 0; i < MAX_POINTS; ++i)
    {
        p.longitude += delta;
        vp->push_back(p);
        if (i % 100 == 0)
        {
            va->push_back(GeoPolylineAttribute(colors[index++%7], i));
        }
    }
    //options.push_back(GeoPolylineOption(
    //                      shared_ptr<const vector<GeoPoint> >(vp),
    //                      shared_ptr<const vector<GeoPolylineAttribute> >(va),
    //                      10, GeoPolyline::POLYLINE_UNHIGHLIGHTED_COLOR, 0));
    vp = new vector<GeoPoint>();
    vp->reserve(MAX_POINTS);
    va = new vector<GeoPolylineAttribute>();
    index = 0;

    p.latitude = TEST_DEFAULT_LATITUDE;
	p.longitude = TEST_DEFAULT_LONGITUDE;
    vp->push_back(p);

    for (int i = 0; i < MAX_POINTS; ++i)
    {
        p.latitude += delta;
        vp->push_back(p);
        if (i % 200 == 0)
        {
            va->push_back(GeoPolylineAttribute(colors[index++%7], i));
        }
    }
    //options.push_back(GeoPolylineOption(
    //                      shared_ptr<const vector<GeoPoint> >(vp),
    //                      shared_ptr<const vector<GeoPolylineAttribute> >(va),
    //                      15, GeoPolyline::POLYLINE_UNHIGHLIGHTED_COLOR, 0));


    vp = new vector<GeoPoint>();
    vp->reserve(MAX_POINTS);
    va = new vector<GeoPolylineAttribute>();
    index = 0;

    p.latitude = TEST_DEFAULT_LATITUDE;
	p.longitude = TEST_DEFAULT_LONGITUDE;
    vp->push_back(p);

    for (int i = 0; i < MAX_POINTS; ++i)
    {
        p.latitude += delta;
        p.longitude += delta;
        vp->push_back(p);
        if (i % 300 == 0)
        {
            va->push_back(GeoPolylineAttribute(colors[index++%7], i));
        }
    }
    //options.push_back(GeoPolylineOption(
    //                      shared_ptr<const vector<GeoPoint> >(vp),
    //                      shared_ptr<const vector<GeoPolylineAttribute> >(va),
    //                      30, GeoPolyline::POLYLINE_UNHIGHLIGHTED_COLOR, 0));

    // Polyline with Cap
    vp = new vector<GeoPoint>();
    va = new vector<GeoPolylineAttribute>();
    index = 0;

    p.latitude  = TEST_DEFAULT_LATITUDE;
	p.longitude = TEST_DEFAULT_LONGITUDE - 0.001;
    vp->push_back(p);

    delta = 0.0001;

    for (int i = 0; i < 40; ++i)
    {
        if (i < 20)
        {
            p.latitude += delta;
        }
        else
        {
            p.longitude += delta;
        }
        vp->push_back(p);
    }

    //options.push_back(GeoPolylineOption(
    //                      shared_ptr<const vector<GeoPoint> >(vp),
    //                      shared_ptr<const vector<GeoPolylineAttribute> >(va),
    //                      30, 0xFF0000FF, 0, 
    //                      GeoPolylineCap::CreateArrowCap(45, 45),
    //                      GeoPolylineCap::CreateRoundCap(15)));

}

class TaskParameter_RemoveGeographics
{
public:
    TaskParameter_RemoveGeographics(OptionalLayersAgentPtr  optionalLayersAgent)
            : m_optionalLayersAgent(optionalLayersAgent){}
    virtual ~TaskParameter_RemoveGeographics(){}
    void AddGraphic(pair<string, Geographic*> p)
    {
        m_graphics.push_back(p.second);
    }

    vector<Geographic*>    m_graphics;
    OptionalLayersAgentPtr m_optionalLayersAgent;
    uint32 m_taskId;
};

void RemoveGeographics(PAL_Instance* pal, void* userData)
{
    TaskParameter_RemoveGeographics* param =
            static_cast<TaskParameter_RemoveGeographics*>(userData);
    if (param)
    {
        param->m_optionalLayersAgent->RemoveGeographic(param->m_graphics);
    }
	delete param;
}

//  Local Classes ..............................................................................


class GeographicFindById
{
public:
    GeographicFindById(const set<std::string>& ids)
            : m_ids(ids){}
    virtual ~GeographicFindById(){}
    bool operator() (const Geographic* graphic) const
    {
        return (graphic && m_ids.find(graphic->GetId()) != m_ids.end());
    }

private:
    GeographicFindById & operator=(const GeographicFindById&);
    const set<std::string>& m_ids;
};

class TestPinBubbleResolover : public PinBubbleResolver
{
public:
    TestPinBubbleResolover() : PinBubbleResolver(){}
    virtual ~TestPinBubbleResolover() {}

    virtual shared_ptr<BubbleInterface>
    GetPinBubble(double latitude, double longitude)
    {
        char tmp[256] = {'\0'};
        nsl_sprintf(tmp, "PinBubble --- latitude: %LF, longitude: %LF",
                   latitude, longitude);
        return shared_ptr<BubbleInterface>();
    }
};

/*! Collection of all instances used for the unit tests */
class TestMapViewInstance : public TestInstance,
                            public MapViewListener
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

    void TurnOffSatelliteLayer()
    {
        vector<LayerAgentPtr> layerAgents = m_optionalLayersAgent->GetOptionalLayerAgents();
        vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
        vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
        for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
        {
            LayerAgentPtr layerAgent = *layerAgentIterator;
            if (layerAgent)
            {
                shared_ptr<string> layerName = layerAgent->GetName();
                if (layerName && !layerName->compare("Satellite"))
                {
                    layerAgent->SetEnabled(false);
                    break;
                }
            }
        }

    }
    // Helper functions to test.
    void MapViewInstanceCreate();
	virtual void OnCameraUpdate(double,double,float,float,float){}
    virtual bool GestureDuringFollowMeMode(MapViewGestureType gesture) {return true;}
    virtual void LayersUpdated()
    {
        if (m_optionalLayersAgent)
        {
#if 0

            // Find the weather layer in the optional layers, according to ESD and SDS, ther
            // will be only one DWR layer, and this layer will handle weather of all regions
            // seamlessly. So just break the loop after finding the specified layer.
            vector<LayerAgentPtr> layerAgents = m_agent->GetOptionalLayerAgents();
            vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
            vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
            for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
            {
                LayerAgentPtr layerAgent = *layerAgentIterator;
                if (layerAgent)
                {
                    shared_ptr<string> layerName = layerAgent->GetName();
                    if (layerName && !layerName->compare("Weather"))
                    {
                        layerAgent->SetEnabled(true);
                        break;
                    }
                }
            }
#endif // End of #if 0


            // Find the weather layer in the optional layers, according to ESD and SDS, ther
            // will be only one DWR layer, and this layer will handle weather of all regions
            // seamlessly. So just break the loop after finding the specified layer.
            vector<LayerAgentPtr> layerAgents = m_optionalLayersAgent->GetOptionalLayerAgents();
            vector<LayerAgentPtr>::const_iterator layerAgentIterator = layerAgents.begin();
            vector<LayerAgentPtr>::const_iterator layerAgentEnd      = layerAgents.end();
            for (; layerAgentIterator != layerAgentEnd; ++layerAgentIterator)
            {
                LayerAgentPtr layerAgent = *layerAgentIterator;
                if (layerAgent)
                {
                    shared_ptr<string> layerName = layerAgent->GetName();
                    fprintf(stderr, "LayerName: %s\n", layerName->c_str());
                    if (layerName && !layerName->compare("Satellite"))
                    {
                        layerAgent->SetEnabled(true);
                        break;
                    }
                }
            }

            // Testing code: add Polylines...
            vector<GeoPolylineOption> options;
            CreatePolylineOptions(options);

            uint32 z = 0;
            vector<GeoPolylineOption>::const_iterator iter = options.begin();
            vector<GeoPolylineOption>::const_iterator end  = options.end();
            for (; iter != end; ++iter)
            {
                Geographic* graphic  = m_optionalLayersAgent->AddPolyline(*iter);
                if (graphic)
                {
                    graphic->SetDrawOrder(++z);
                    m_graphics.insert(map<string, Geographic*>::value_type(
                                          graphic->GetId(), graphic));
                }
            }

        }
    };

    virtual bool OnSingleTap(float screenX, float screenY, const set<string>& ids);

    void RemoveAllGraphics()
    {
    };

    MapViewInterface* m_pMapView;

    /*! CCC worker thread */
    shared_ptr<EventTaskQueue> m_eventQueue;

    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;

    map<string, Geographic*> m_graphics;
    OptionalLayersAgentPtr   m_optionalLayersAgent;
    Geographic*              m_highlightedObject;
    Geographic*              m_hidenObject;
};


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



//  Local Classes Implementation ...........................................................


/* See description in header file. */
TestMapViewInstance::TestMapViewInstance()
        : TestInstance(),
          m_highlightedObject(NULL),
          m_hidenObject(NULL)
{
    // Get CCC and UI task queue
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(m_pal));
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(m_pal));

    CU_ASSERT_PTR_NOT_NULL_FATAL(m_eventQueue.get());
    CU_ASSERT_PTR_NOT_NULL_FATAL(m_UITaskQueue.get());
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
    if (ids.empty())
    {
        return  false;
    }

    static int counter = 0;
    // Simple test, if clicked less than 5 times, highlight selected polyline,
    // or, remove all polylines.
    ++counter;
    if (counter != 4 && counter != 6 && counter != 8 && counter != 10) // switch color
    {
        vector<Geographic*> graphics;
        m_optionalLayersAgent->GetAllGeographics(graphics);

        vector<Geographic*>::iterator iter =
                find_if(graphics.begin(), graphics.end(),GeographicFindById(ids));
        if (iter == graphics.end())
        {
            return false;
        }

        if (m_highlightedObject)
        {
            m_highlightedObject->SetSelected(false);
            m_highlightedObject->SetDrawOrder(0);
        }
        m_highlightedObject = *iter;
        m_highlightedObject->SetSelected(true);
        m_highlightedObject->SetDrawOrder(counter + 100);
    }
    else if (counter == 4) // reset color of selected graphic.
    {
        if (m_highlightedObject)
        {
            GeoPolyline* polyline = static_cast<GeoPolyline*>(m_highlightedObject);
            if (polyline)
            {
                //polyline->SetColors(shared_ptr<const vector<GeoPolylineAttribute> >());
                polyline->SetWidth(15);
                polyline->Submit();
            }
        }
    }
    else if (counter == 6)
    {
        m_highlightedObject->Show(false);
        m_hidenObject = m_highlightedObject;
    }
    else if (counter == 8)
    {
        m_hidenObject->Show(true);
    }
    else if (counter == 10) // remove all graphic when counter reached 10.
    {
        RemoveAllGraphics();

        m_eventQueue->AddTask(
            new MapViewTestTask(this,
                                &TestMapViewInstance::TurnOffSatelliteLayer));
    }

    return true;
}
/* See description in header file. */
void TestMapViewInstance::TestMapViewInstanceCreate()
{
    CU_ASSERT(m_initialized);
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

    CU_ASSERT_PTR_NOT_NULL(mapConfiguration.get());
    CU_ASSERT_PTR_NOT_NULL(metadataConfiguration.get());

    metadataConfiguration->m_wantWeatherLayer    = true;
    metadataConfiguration->m_wantLableLayers     = false;
    metadataConfiguration->m_wantDAM             = false;
    metadataConfiguration->m_wantPoiLayers       = true;
    metadataConfiguration->m_wantSatelliteLayers = false;

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
                                          m_optionalLayersAgent,
                                          -5,       // What do we need to set z-order level to?
                                          NULL);    // MapViewContext: I think we need to pass in a window handle here for the real test?

    CU_ASSERT_PTR_NOT_NULL(m_pMapView);
    CU_ASSERT_PTR_NOT_NULL(pinManager.get());
    CU_ASSERT_PTR_NOT_NULL(m_optionalLayersAgent.get());

    m_pMapView->RegisterListener(this);
    // The completion event -- either error or success -- are signaled in the callback functions.
}


/*  Test Map creation. */
void TestMapViewInstanceCreate(void)
{
    TestMapViewInstance* instance = NULL; //new TestMapViewInstance();
    CU_ASSERT_PTR_NOT_NULL_FATAL(instance);

    // Schedule create map task in the CCC thread
    instance->m_eventQueue->AddTask(new MapViewTestTask(instance, &TestMapViewInstance::TestMapViewInstanceCreate));

    // Wait for it to finish. The event gets triggered in one of the callbacks
    CU_ASSERT_TRUE(instance->WaitEvent(30 * 60 * 1000));    // !!!! set to high value for testing

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
        CU_ASSERT_EQUAL(result, NE_OK);


        // !!!! TEST: Wait for now for testing!
        CU_ASSERT_TRUE(instance->WaitEvent(30 * 60 * 1000));    // !!!! set to high value for testing




        // Shutdown MapView: This has to be called in the UI thread I think
        instance->m_pMapView->Destroy();

        // Wait for it to finish. The event gets triggered in one of the callbacks
        CU_ASSERT_TRUE(instance->WaitEvent(10 * 60 * 1000)); // !!!! test
    }

    /* Now shutdown PAL/NB_Context */
    instance->ScheduleFinishTest();

    // Wait for shutdown to finish
    CU_ASSERT_TRUE(instance->WaitEvent(10 * 60 * 1000)); // !!!! test

    delete instance;
}


//  Interfaces for CUnit ........................................................................

/*! Setup for test.

  @return 0 (succeed all the time.)
*/
int TestMapView_SuiteSetup(void)
{
    return 0;
}

/*! Cleans up the test.

  @return 0 (succeed all the time.)
*/
int TestMapView_SuiteCleanup(void)
{
    return 0;
}

/*! Add all test cases into test suite.

  @return None.
*/
void TestMapView_AddAllTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestMapViewInstanceCreate", TestMapViewInstanceCreate);
}

/*! @} */
