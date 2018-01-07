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

    @file     TestMap.c
    @date     01/23/2009
    @defgroup TEST_MAP_H System Tests for maps
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

#include "testrastermap.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "palfile.h"
#include "palclock.h"

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbrastermaphandler.h"
#include "nbrouteinformation.h"
#include "nbroutehandler.h"
#include "nbrouteparameters.h"
#include "nbguidanceinformation.h"
#include "testdirection.h"


// Defines/Constants .........................................................

// Comment this define if you want to keep the images after the tests have run
/// @todo Make this a command line option
#define CLEANUP_IMAGE_FILES


#define IMAGE_PREFIX            "TestImage"
#define IMAGE_NAME_BMP          IMAGE_PREFIX "%03d.bmp"
#define IMAGE_NAME_GIF          IMAGE_PREFIX "%03d.gif";
#define IMAGE_NAME_PNG          IMAGE_PREFIX "%03d.png";
#define IMAGE_NAME_BMP_DEFLATE  IMAGE_PREFIX "%03d.bmp";
#define IMAGE_NAME_JPG          IMAGE_PREFIX "%03d.jpg";

#define IMAGE_POI_PNG           "pinTestImg.png"

// Local Functions ...........................................................

// All tests. Add all tests to the TestRasterMap_AddTests function
static void TestRasterMapFormats(void);
static void TestRasterMapLegendsAndScalesLow(void);
static void TestRasterMapLegendsAndScalesHigh(void);
static void TestRasterMapRotate(void);
static void TestRasterMapSizes(void);
static void TestRasterMapRevGeocoded(void);
static void TestRasterMapWithPlaces(void);
static void TestRasterMapWithTraffic(void);
static void TestRasterMapWithPlacesAndTraffic(void);
static void TestRasterMapWithRoute(void);
static void TestRasterMapWithManeuver(void);
static void TestRasterMapWithDetour(void);
static void TestRasterMapWithPin(void);
static void TestRasterMapCancelRequest(void);

static void RasterMapRequest(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    double mapScale,
    double mapRotate,
    NB_RouteInformation* route,
    int index,
    NB_RouteInformation* detour);

static void
RasterMapPinRequest(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    uint8 zoom,
    NB_ImageOffset mapCenterOffset,
    NB_ImageOffset poiOffset);

static void TestRasterMapPin(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    uint8 zoom,
    NB_ImageSize poiSize);

static void DownloadCallbackNew(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);

static NB_RouteInformation* TestRouteRequest();
static void DownloadRouteCallbackNew(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void CleanupImageFiles();


// Variables .................................................................

// Event gets triggered from the download callback.
static void* g_CallbackEvent = NULL;

// Counter for all image files.
static int g_ImageNameCounter = 0;


static PAL_Instance* g_pal = 0;
static NB_Context* g_context = 0;

/*! Add all your test functions here

@return None
*/
void
TestRasterMap_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithPin", TestRasterMapWithPin);

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestRasterMapFormats", TestRasterMapFormats);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapLegendsAndScalesLow", TestRasterMapLegendsAndScalesLow);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapLegendsAndScalesHigh", TestRasterMapLegendsAndScalesHigh);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapRotate", TestRasterMapRotate);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapSizes", TestRasterMapSizes);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapRevGeocoded", TestRasterMapRevGeocoded);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithPlaces", TestRasterMapWithPlaces);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithTraffic", TestRasterMapWithTraffic);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithPlacesAndTraffic", TestRasterMapWithPlacesAndTraffic);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithRoute", TestRasterMapWithRoute);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithManeuver", TestRasterMapWithManeuver);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapWithDetour", TestRasterMapWithDetour);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestRasterMapCancelRequest", TestRasterMapCancelRequest);
};


/*! Add common initialization code here.

@return 0

@see TestRasterMap_SuiteCleanup
*/
int
TestRasterMap_SuiteSetup()
{
    // Get rid of any existing images before we run the test
    CleanupImageFiles();

    // Create event for callback synchronization
    g_CallbackEvent = CreateCallbackCompletedEvent();

    TestNetwork_SuiteSetup();
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestRasterMap_SuiteSetup
*/
int
TestRasterMap_SuiteCleanup()
{
#ifdef CLEANUP_IMAGE_FILES
    CleanupImageFiles();
#endif

    DestroyCallbackCompletedEvent(g_CallbackEvent);

    TestNetwork_SuiteCleanup();
    return 0;
}

void TestRasterMap_TestSetup()
{
    CU_ASSERT_PTR_NULL(g_pal);
    CU_ASSERT_PTR_NULL(g_context);

    CreatePalAndContext(&g_pal, &g_context);
}

void TestRasterMap_TestCleanup()
{
    if (g_context)
    {
        DestroyContext(g_context);
        g_context = 0;
    }

    if (g_pal)
    {
        PAL_DestroyInstance(g_pal);
        g_pal = 0;
    }
}

void TestRasterMapCancelRequest(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;
    NB_Error err = NE_OK;
    NB_RasterMapParameters* params = 0;
    NB_RasterMapHandler* handler = 0;
    nb_boolean  progress = FALSE;
    NB_ImageFormat mapFormat = NB_IF_BMP;
    double mapScale  = 1000.0;
    double mapRotate = 0.0;

    NB_RequestHandlerCallback callback = { 0, 0 };

    NB_LatitudeLongitude center;
    center.latitude = 36.10;
    center.longitude = -115.12;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // provide pal instance for callback file operations
    callback.callbackData = g_pal;

    err = NB_RasterMapParametersCreate(g_context, &center, mapFormat, &renderSize, mapScale, mapRotate, &mapConfig, &params);
    CU_ASSERT_EQUAL(err, NE_OK);

    LOGOUTPUT(LogLevelHigh, ("render size:%u x %u, format:%u scale %f\n", renderSize.width, renderSize.height, mapFormat, mapScale));

    err = NB_RasterMapHandlerCreate(g_context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapHandlerStartRequest(handler, params);
    CU_ASSERT_EQUAL(err, NE_OK);

    progress = NB_RasterMapHandlerIsRequestInProgress(handler);
    CU_ASSERT_EQUAL(progress, TRUE);

    err = NB_RasterMapHandlerCancelRequest(handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapHandlerDestroy(handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapParametersDestroy(params);
    CU_ASSERT_EQUAL(err, NE_OK);

    TestRasterMap_TestCleanup();
}


void
TestRasterMapFormats(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_BMP;

    double latitude                 = 36.10;
    double longitude                = -115.12;
    double mapScale                 = 1000.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // Get the map in all formats
    mapFormat = NB_IF_BMP;
    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    mapFormat = NB_IF_GIF;
    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    mapFormat = NB_IF_PNG;
    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    mapFormat = NB_IF_BMP_Deflate;
    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    TestRasterMap_TestCleanup();
}


/*! @brief Test the legend for map requests.

Test all available legend formats.

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapLegendsAndScalesLow(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat    = NB_IF_PNG;

    double latitude     = 36.10;
    double longitude    = -115.12;
    double mapScale     = 0.0;  // Gets set in the for loop
    double mapRotate    = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width    = 200;  // Width of the map
    renderSize.height   = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // Get maps for different scales
    for (mapScale = 1.0; mapScale <= 1500; mapScale = mapScale * 3.0)
    {
        // Get different legends
        mapConfig.legend = NB_RMLS_Imperial;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

        mapConfig.legend = NB_RMLS_Metric;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

        mapConfig.legend = NB_RMLS_Both;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);
    }

    TestRasterMap_TestCleanup();
}

void
TestRasterMapLegendsAndScalesHigh(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat    = NB_IF_PNG;

    double latitude     = 36.10;
    double longitude    = -115.12;
    double mapScale     = 0.0;  // Gets set in the for loop
    double mapRotate    = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width    = 200;  // Width of the map
    renderSize.height   = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // Get maps for different scales
    for (mapScale = 1501; mapScale <= 10000.0; mapScale = mapScale * 3.0)
    {
        // Get different legends
        mapConfig.legend = NB_RMLS_Imperial;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

        mapConfig.legend = NB_RMLS_Metric;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

        mapConfig.legend = NB_RMLS_Both;
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);
    }

    TestRasterMap_TestCleanup();
}

/*! @brief Test map rotations

Test rotations (offset to north) for maps.

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapRotate(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat    = NB_IF_PNG;

    // San Francisco
    double latitude     = 37.775239;
    double longitude    = -122.419170;
    double mapScale     = 50.0;
    double mapRotate    = 0.0;  // Gets set in the for loop

    TestRasterMap_TestSetup();

    renderSize.height   = 200;  // Width of the map
    renderSize.width    = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // Get maps for different rotation values
    for (mapRotate = 0.0; mapRotate <= 360.0; mapRotate += 18.0)
    {
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);
    }

    TestRasterMap_TestCleanup();
}


/*! @brief Test different map sizes

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapSizes(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat    = NB_IF_PNG;

    // Chicago
    double latitude     = 41.85;
    double longitude    = -87.65;

    double mapScale     = 35.0;
    double mapRotate    = 0.0;

    int i = 0;

    TestRasterMap_TestSetup();

    renderSize.width    = 10;
    renderSize.height   = 10;

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    // Get maps for different sizes
    for (i = 0; i < 20; ++i)
    {
        // Get the map in different sizes
        RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

        renderSize.width  += 28;
        renderSize.height += 32;
    }

    TestRasterMap_TestCleanup();
}


/*! @brief Test map reverse geocode

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapRevGeocoded(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    // White House
    double latitude                 = 38.89868;
    double longitude                = -77.03723;

    double mapScale                 = 10.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 1;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with places

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithPlaces(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    // White House
    double latitude                 = 38.89868;
    double longitude                = -77.03723 ;

    double mapScale                 = 10.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 1;
    mapConfig.wantTraffic = 0;

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with traffic

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithTraffic(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    // Downtown LA
    double latitude                 = 34.05;
    double longitude                = -118.23;
    double mapScale                 = 200.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 1;

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with traffic and places

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithPlacesAndTraffic(void)
{
    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    // Downtown LA
    double latitude                 = 34.05;
    double longitude                = -118.23;
    double mapScale                 = 15.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 1;
    mapConfig.wantTraffic = 1;

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, 0, -1, 0);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with a route

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithRoute(void)
{
    NB_RouteInformation* route = 0;

    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    double latitude                 = 33.73;
    double longitude                = -117.77;
    double mapScale                 = 100.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 1;

    // create test route to display
    route = TestRouteRequest();
    CU_ASSERT_NOT_EQUAL(route, 0);

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, route, -1, 0);
    NB_RouteInformationDestroy(route);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with a maneuver

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithManeuver(void)
{
    NB_RouteInformation* route = 0;

    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    double latitude                 = 33.73;
    double longitude                = -117.77;
    double mapScale                 = 100.0;
    double mapRotate                = 0.0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 1;

    // create test route to display
    route = TestRouteRequest();
    CU_ASSERT_NOT_EQUAL(route, 0);

    RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, route, 1, 0);
    NB_RouteInformationDestroy(route);

    TestRasterMap_TestCleanup();
}


/*! @brief Test map with a detour

@return None CUnit asserts are called on failures.
*/
void
TestRasterMapWithDetour(void)
{
    NB_RouteInformation* route = 0;
    NB_RouteInformation* detour = 0;

    NB_GpsHistory*            history = 0;
    NB_GpsLocation              fix;

    NB_ImageSize renderSize;
    NB_RasterMapConfiguration mapConfig;

    NB_ImageFormat mapFormat        = NB_IF_PNG;

    double latitude                 = 33.73;
    double longitude                = -117.77;
    double mapScale                 = 100.0;
    double mapRotate                = 0.0;

    NB_Error err = NE_OK;

    NB_DetourParameters* detourParameters = 0;
    NB_RouteParameters* routeParameters = 0;

    NB_RouteHandler* handler = 0;
    NB_RequestHandlerCallback callback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };

    uint32 maneuvers = 0;

    TestRasterMap_TestSetup();

    renderSize.width                = 200;  // Width of the map
    renderSize.height               = 320;  // Height of the map

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 0;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 1;

    // create test route to display
    route = TestRouteRequest();
    CU_ASSERT_NOT_EQUAL(route, 0);

    maneuvers = NB_RouteInformationGetManeuverCount(route);
    LOGOUTPUT(LogLevelHigh, ("Maneuver count: %u\n", maneuvers));
    CU_ASSERT(maneuvers >= 9);

    history = NB_ContextGetGpsHistory(g_context);
    CU_ASSERT_PTR_NOT_NULL(history);

    fix.status   = NE_OK;
    fix.valid    =  NGV_Latitude |
                    NGV_Longitude |
                    NGV_Altitude |
                    NGV_Heading |
                    NGV_HorizontalVelocity |
                    NGV_HorizontalUncertainty |
                    NGV_AxisUncertainty |
                    NGV_PerpendicularUncertainty;

    fix.gpsTime    = PAL_ClockGetGPSTime();
    fix.latitude   =  33.63435;
    fix.longitude  = -117.73638;
    fix.heading    = 0.0;
    fix.horizontalVelocity = 0.0;
    fix.altitude   = 0.0;
    fix.verticalVelocity = 0.0;
    fix.horizontalUncertaintyAngleOfAxis = 0.0;
    fix.horizontalUncertaintyAlongAxis = 25.0;
    fix.horizontalUncertaintyAlongPerpendicular = 25.0;
    fix.verticalUncertainty = 100.0;
    fix.utcOffset = 0;
    fix.numberOfSatellites = 4;

    err = NB_GpsHistoryAdd(history, &fix);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_DetourParametersCreate(g_context, &detourParameters);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_NOT_EQUAL(detourParameters, 0);

    if (detourParameters)
    {
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidManeuvers(detourParameters, "Maneuvers 5 thru 8", 5, 8), NE_OK);

        CU_ASSERT_EQUAL(NB_RouteParametersCreateDetour(g_context, route, NULL, detourParameters, history, &routeParameters), NE_OK);
        CU_ASSERT_NOT_EQUAL(routeParameters, 0);

        if (routeParameters)
        {
            callback.callbackData = &detour;
            err = NB_RouteHandlerCreate(g_context, &callback, &handler);
            if (!err && handler)
            {
                err = NB_RouteHandlerStartRequest(handler, routeParameters);
                if (!err)
                {
                   WaitForCallbackCompletedEvent(g_CallbackEvent, 60000);
                }

                NB_RouteHandlerDestroy(handler);
                err = NB_RouteParametersDestroy(routeParameters);
            }
            CU_ASSERT_NOT_EQUAL(detour, 0);

            RasterMapRequest(renderSize, mapFormat, &mapConfig, latitude, longitude, mapScale, mapRotate, route, 1, detour);
        }

        NB_DetourParametersDestroy(detourParameters);
    }

    NB_RouteInformationDestroy(detour);
    NB_RouteInformationDestroy(route);

    TestRasterMap_TestCleanup();
}


void
RasterMapRequest(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    double mapScale,
    double mapRotate,
    NB_RouteInformation* route,
    int index,
    NB_RouteInformation* detour
    )
{
    NB_Error err = NE_OK;
    NB_RasterMapParameters* params = 0;
    NB_RasterMapHandler* handler = 0;

    NB_RequestHandlerCallback callback = { DownloadCallbackNew, 0 };

    NB_LatitudeLongitude center;
    center.latitude = latitude;
    center.longitude = longitude;

    // provide pal instance for callback file operations
    callback.callbackData = g_pal;

    if (!route)
    {
        // no route specified... regular map
        err = NB_RasterMapParametersCreate(g_context, &center, mapFormat, &renderSize, mapScale, mapRotate, mapConfig, &params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    else if (detour)
    {
        // route & detour specified... detour map
        err = NB_RasterMapParametersCreateDetour(g_context, &center, mapFormat, &renderSize, mapScale, mapRotate, mapConfig, route, detour, &params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    else if (index < 0)
    {
        // route with no index... route map
        err = NB_RasterMapParametersCreateRoute(g_context, &center, mapFormat, &renderSize, mapScale, mapRotate, mapConfig, route, &params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }
    else
    {
        // route with index... maneuver map
        err = NB_RasterMapParametersCreateManeuver(g_context, &center, mapFormat, &renderSize, mapScale, mapRotate, mapConfig, route, index, &params);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    LOGOUTPUT(LogLevelHigh, ("render size:%u x %u, format:%u scale %f\n", renderSize.width, renderSize.height, mapFormat, mapScale));

    err = NB_RasterMapHandlerCreate(g_context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapHandlerStartRequest(handler, params);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
    {
        CU_ASSERT(FALSE);
    }

    err = NB_RasterMapHandlerDestroy(handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapParametersDestroy(params);
    CU_ASSERT_EQUAL(err, NE_OK);
}

void
RasterMapPinRequest(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    uint8 zoom,
    NB_ImageOffset mapCenterOffset,
    NB_ImageOffset poiOffset
    )
{
    NB_Error err = NE_OK;
    NB_RasterMapParameters* params = 0;
    NB_RasterMapHandler* handler = 0;
    NB_RequestHandlerCallback callback = { DownloadCallbackNew, 0 };

    NB_LatitudeLongitude pinLocation;
    NB_Image poiImage = {0};
    NB_ImageSize poiSize;

    PAL_FileLoadFile(g_pal, IMAGE_POI_PNG, &poiImage.data, &poiImage.dataLen);

    pinLocation.latitude = latitude;
    pinLocation.longitude = longitude;

    // provide pal instance for callback file operations
    callback.callbackData = g_pal;
    poiSize.width = 64;
    poiSize.height = 64;

    if( poiImage.data == NULL )
    {
         err = NB_RasterMapParametersCreateWithPoi(g_context, &pinLocation, mapFormat, &renderSize, zoom,  mapConfig,
                                         NULL, &poiSize, &mapCenterOffset, &poiOffset, &params);
    }
    else
    {
         err = NB_RasterMapParametersCreateWithPoi(g_context, &pinLocation, mapFormat, &renderSize, zoom, mapConfig,
                                         &poiImage, &poiSize, &mapCenterOffset, &poiOffset, &params);
         nsl_free(poiImage.data);
         poiImage.data = NULL;
    }

    CU_ASSERT_EQUAL(err, NE_OK);
    LOGOUTPUT(LogLevelHigh, ("render size:%u x %u, format:%u zoom: %d mapcenterOffset [%f, %f]\n",
              renderSize.width, renderSize.height, mapFormat, zoom,
              mapCenterOffset.x, mapCenterOffset.y));

    err = NB_RasterMapHandlerCreate(g_context, &callback, &handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapHandlerStartRequest(handler, params);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!WaitForCallbackCompletedEvent(g_CallbackEvent, 60000))
    {
        CU_ASSERT(FALSE);
    }

    err = NB_RasterMapHandlerDestroy(handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_RasterMapParametersDestroy(params);
    CU_ASSERT_EQUAL(err, NE_OK);
}

void TestRasterMapPin(
    NB_ImageSize renderSize,
    NB_ImageFormat mapFormat,
    NB_RasterMapConfiguration* mapConfig,
    double latitude,
    double longitude,
    uint8 zoom,
    NB_ImageSize poiSize
    )
{
    NB_ImageOffset mapCenterOffset;
    NB_ImageOffset poiOffset;
    double offsetX = (double)renderSize.width/4.0;
    double offsetY = (double)renderSize.height/4.0;

    poiOffset.x = -1.0*(double)poiSize.width;
    poiOffset.y = -1.0*(double)(poiSize.height-8);

    // poi in the center
    mapCenterOffset.x = 0.0;
    mapCenterOffset.y = 0.0;
    RasterMapPinRequest(renderSize, mapFormat, mapConfig, latitude, longitude, zoom, mapCenterOffset, poiOffset);

    // poi in the upper left quarter center
    mapCenterOffset.x = offsetX;
    mapCenterOffset.y = offsetY;
    RasterMapPinRequest(renderSize, mapFormat, mapConfig, latitude, longitude, zoom, mapCenterOffset, poiOffset);

    // poi in the upper right quarter center
    mapCenterOffset.x = -1.0*offsetX;
    mapCenterOffset.y = offsetY;
    RasterMapPinRequest(renderSize, mapFormat, mapConfig, latitude, longitude, zoom, mapCenterOffset, poiOffset);

    // poi in the lower left quarter center
    mapCenterOffset.x = offsetX;
    mapCenterOffset.y = -1.0*offsetY;
    RasterMapPinRequest(renderSize, mapFormat, mapConfig, latitude, longitude, zoom, mapCenterOffset, poiOffset);

    // poi in the lower right quarter center
    mapCenterOffset.x = -1.0*offsetX;
    mapCenterOffset.y = -1.0*offsetY;
    RasterMapPinRequest(renderSize, mapFormat, mapConfig, latitude, longitude, zoom, mapCenterOffset, poiOffset);
}

void
DownloadCallbackNew(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    if (!up)
    {
        if (err != NE_OK) {

            // Abort and trigger event
            CU_FAIL("Callback returned error");
            LOGOUTPUT(LogLevelHigh, ("Callback returned error %d\n", err));
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;

            case NB_NetworkRequestStatus_Success:
            {
                NB_Error err = NE_OK;
                NB_RasterMapInformation* mapInformation = 0;
                NB_SearchInformation* searchInformation = 0;

                NB_LatitudeLongitude latlong;
                double scale = 0.0;
                double rotation = 0.0;
                NB_ImageFormat format = NB_IF_BMP;
                NB_Location location = {{0}};

                nb_boolean hasIncidents = 0;
                int slice = 0;
                int index = 0;
                NB_SearchResultType type = NB_SRT_None;

                uint8* mapImage = 0;
                uint32 mapImageSize = 0;

                PAL_Instance* pal = (PAL_Instance*)userData;
                PAL_File* pImageFile = NULL;
                const char* pFormatString = 0;
                char fileName[1024];
                fileName[0] = 0;

                err = err ? err : NB_RasterMapHandlerGetMapInformation(handler, &mapInformation);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = err ? err : NB_RasterMapInformationGetMapInformation(mapInformation, &latlong, &scale, &rotation);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = err ? err : NB_RasterMapInformationGetImageData(mapInformation, &format, &mapImage, &mapImageSize);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = err ? err : NB_RasterMapInformationGetCenterLocation(mapInformation, &location);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = NB_RasterMapHandlerGetSearchInformation(handler, &hasIncidents, &searchInformation);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = NB_SearchInformationGetResultCount(searchInformation, &slice);
                CU_ASSERT_EQUAL(err, NE_OK);

                for (index = 0; index < slice; index++)
                {
                    NB_Place place = {{0}};
                    NB_ExtendedPlace* extendedPlace = 0;
                    NB_TrafficIncident incidentDetails = {0};
                    double distance = 0.0;

                    if (type & NB_SRT_Place)
                    {
                        err = NB_SearchInformationGetPlace(searchInformation, index, &place, &distance, &extendedPlace);
                        CU_ASSERT_EQUAL(err, NE_OK);
                    }
                    if (type & NB_SRT_TrafficIncident)
                    {
                        err = NB_SearchInformationGetTrafficIncident(searchInformation, index, &incidentDetails, &distance);
                        CU_ASSERT_EQUAL(err, NE_OK);
                    }
                }

                // Format file name
                switch (format)
                {
                    case NB_IF_BMP:
                        pFormatString = IMAGE_NAME_BMP;
                        break;
                    case NB_IF_GIF:
                        pFormatString = IMAGE_NAME_GIF;
                        break;
                    case NB_IF_PNG:
                        pFormatString = IMAGE_NAME_PNG;
                        break;
                    case NB_IF_BMP_Deflate:
                        pFormatString = IMAGE_NAME_BMP_DEFLATE;
                        break;
                    case NB_IF_JPG:
                        pFormatString = IMAGE_NAME_JPG;
                        break;
                }
                nsl_sprintf(fileName, pFormatString, ++g_ImageNameCounter);

                // Remove file if exists
                if (PAL_FileExists(pal, fileName) == PAL_Ok)
                {
                    PAL_FileRemove(pal, fileName);
                }

                // Create new file
                PAL_FileOpen(pal, fileName, PFM_Create, &pImageFile);
                if (pImageFile)
                {
                    uint32 bytesWritten = 0;

                    // Write the returned buffer to the file
                    PAL_FileWrite(pImageFile, mapImage, mapImageSize, &bytesWritten);
                    CU_ASSERT_EQUAL(bytesWritten, mapImageSize);

                    PAL_FileClose(pImageFile);
                    CU_ASSERT_EQUAL(PAL_FileExists(pal, fileName), PAL_Ok);
                }

                NB_SearchInformationDestroy(searchInformation);
                NB_RasterMapInformationDestroy(mapInformation);
                break;
            }
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
    }
}

NB_RouteInformation*
TestRouteRequest()
{
    NB_RouteInformation* route = 0;
    NB_Error err = NE_OK;
    NB_RouteParameters* parameters = 0;
    NB_RouteHandler* handler;
    NB_RouteOptions options;
    NB_RouteConfiguration config = {0};
    NB_RequestHandlerCallback callback = { (NB_RequestHandlerCallbackFunction) DownloadRouteCallbackNew, 0 };
    NB_Place destination = {{0}};
    NB_Place origin = {{0}};
    NB_GuidanceInformation* guidanceInfo = 0;

    CU_ASSERT_PTR_NOT_NULL(g_pal);
    CU_ASSERT_PTR_NOT_NULL(g_context);

    guidanceInfo = CreateGuidanceInformation (g_pal, g_context);

    options.avoid = 0;
    options.transport = NB_TransportationMode_Car;
    options.type = NB_RouteType_Fastest;
    options.traffic = NB_TrafficType_None;
    options.pronunStyle    = Test_OptionsGet()->voiceStyle;

    config.wantDestinationLocation = TRUE;
    config.wantManeuvers = TRUE;
    config.wantOriginLocation = TRUE;
    config.wantPolyline = TRUE;
    config.wantRouteMap = TRUE;
    config.routeMapAngleOfView = 10;
    config.routeMapDistanceOfView = 10;

    config.commandSetVersion = NB_GuidanceInformationGetCommandVersion(guidanceInfo);

    origin.location.type = NB_Location_Address;
    strcpy(origin.location.streetnum, "35");
    strcpy(origin.location.street1, "Tesla");
    strcpy(origin.location.city, "Irvine");
    strcpy(origin.location.state, "CA");
    strcpy(origin.location.postal, "92618");
    origin.location.latitude = 33.63435;
    origin.location.longitude = -117.73638;

    destination.location.type = NB_Location_Address;
    strcpy(destination.location.streetnum, "18872");
    strcpy(destination.location.street1, "Bardeen");
    strcpy(destination.location.city, "Irvine");
    strcpy(destination.location.state, "CA");
    strcpy(destination.location.postal, "92612");
    destination.location.latitude = 33.67035;
    destination.location.longitude = -117.85560;

    err = NB_RouteParametersCreateFromPlace(g_context, &origin, &destination, &options, &config, &parameters);

    if (!err && parameters)
    {
        callback.callbackData = &route;
        err = NB_RouteHandlerCreate(g_context, &callback, &handler);
        if (!err && handler)
        {
            err = NB_RouteHandlerStartRequest(handler, parameters);
            if (!err)
            {
               WaitForCallbackCompletedEvent(g_CallbackEvent, 60000);
            }

            NB_RouteHandlerDestroy(handler);
        }
        err = NB_RouteParametersDestroy(parameters);
    }

    NB_GuidanceInformationDestroy(guidanceInfo);

    // caller needs to destroy returned route
    return route;
}

void
TestRasterMapWithPin(void)
{
    NB_ImageSize renderSize;
    NB_ImageSize poiSize;
    NB_RasterMapConfiguration mapConfig;
    NB_ImageFormat mapFormat    = NB_IF_PNG;
    double latitude  = 33.55961648;
    double longitude = -117.7295304;
    uint8 zoom;

    mapConfig.dpi = 96;
    mapConfig.legend = NB_RMLS_None;
    mapConfig.wantCenterRevGeocoded = 1;
    mapConfig.wantPlaces = 0;
    mapConfig.wantTraffic = 0;

    poiSize.width    = 64;  // width of the poi
    poiSize.height   = 64;  // height of the poi

    TestRasterMap_TestSetup();

    zoom = 12;
    renderSize.width    = 288;   // width of the map
    renderSize.height   = 1232;  // height of the map
    TestRasterMapPin(renderSize, mapFormat, &mapConfig, latitude, longitude, zoom, poiSize);

    zoom = 13;
    renderSize.width    = 256;  // width of the map
    renderSize.height   = 256;  // height of the map
    TestRasterMapPin(renderSize, mapFormat, &mapConfig, latitude, longitude, zoom, poiSize);

    zoom = 14;
    renderSize.width    = 512;   // width of the map
    renderSize.height   = 512;   // height of the map
    TestRasterMapPin(renderSize, mapFormat, &mapConfig, latitude, longitude, zoom, poiSize);

    zoom = 15;
    renderSize.width    = 4296;   // width of the map
    renderSize.height   = 1676;   // height of the map
    TestRasterMapPin(renderSize, mapFormat, &mapConfig, latitude, longitude, zoom, poiSize);

    TestRasterMap_TestCleanup();
}

static void
DumpRouteManeuvers(NB_RouteInformation* route)
{
    uint32 numManeuvers= 0;
    uint32 nIndex = 0;

    CU_ASSERT_PTR_NOT_NULL(route);
    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT_NOT_EQUAL(numManeuvers, 0);

    for(nIndex = 0; nIndex < numManeuvers; nIndex++)
    {
#define TEXT_SIZE 128
        double                  latitude            = 0.0;
        double                  longitude           = 0.0;
        double                  heading             = 0.0;
        char                    primary[TEXT_SIZE]  = {0};
        char                    secondary[TEXT_SIZE]= {0};
        char                    current[TEXT_SIZE]  = {0};
        double                  distance            = 0.0;

        NB_RouteInformationGetTurnInformation(route, nIndex, NULL, primary, TEXT_SIZE, secondary, TEXT_SIZE, current, TEXT_SIZE, &distance, 0);

        LOGOUTPUT(LogLevelHigh, ("Maneuver %d\n", nIndex));
        LOGOUTPUT(LogLevelHigh, ("\tLat: %f, Long: %f, Heading: %f, Distance: %f\n", latitude, longitude, heading, distance));
        LOGOUTPUT(LogLevelHigh, ("\tCurrent: %s\n", current));
        LOGOUTPUT(LogLevelHigh, ("\tPrimary: %s\n", primary));
        LOGOUTPUT(LogLevelHigh, ("\tSecondary: %s\n", secondary));
    }
}

void
DownloadRouteCallbackNew(
                    void* handler,              /*!< Handler invoking the callback */
                    NB_NetworkRequestStatus status,    /*!< Status of download request */
                    NB_Error err,               /*!< error state */
                    uint8 up,                   /*!< Non-zero is query upload, zero is query download */
                    int percent,                /*!< Download progress */
                    void* pUserData)            /*!< Optional user data provided */
{
    if (!up)
    {
        NB_RouteInformation** route = (NB_RouteInformation**)pUserData;
        *route = 0;

        if (err != NE_OK)
        {
            // Abort and trigger event
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_CallbackEvent);
            return;
        }

        switch (status)
        {
        case NB_NetworkRequestStatus_Progress:
            // not complete yet... return
            return;

        case NB_NetworkRequestStatus_Success:
            err = NB_RouteHandlerGetRouteInformation(handler, route);
            CU_ASSERT_EQUAL(err, NE_OK);

            DumpRouteManeuvers(*route);

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
    }
}


void CleanupImageFiles()
{
    if (g_pal)
    {
        PAL_Error err = PAL_Ok;
        PAL_FileEnum* fileEnum;

        err = PAL_FileEnumerateCreate(g_pal, ".", TRUE, &fileEnum);
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
                        if (nsl_strncmp(info.filename, IMAGE_PREFIX, nsl_strlen(IMAGE_PREFIX)) == 0)
                        {
                            PAL_FileRemove(g_pal, info.filename);
                        }
                    }
                }
            } while (!err);

            PAL_FileEnumerateDestroy(fileEnum);
        }
    }
}


/*! @} */
