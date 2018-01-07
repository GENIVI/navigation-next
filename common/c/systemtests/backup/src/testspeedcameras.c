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
    @file     TestCpeedCameras.c
    @date     11/08/2011
    @defgroup TestSpeedCameras_h System tests for testing SpeedCameras functions

    System tests for SpeedCameras functions

    This file contains all system tests for the SpeedCameras functionality in 
    NAVBuilder Services SDK.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunications Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunications Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "testsearchbuilder.h"
#include "testspeedcameras.h"
#include "testnavigation.h"
#include "nbsearchhandler.h"
#include "nbcamerainformation.h"
#include "nbqalog.h"

/* Destination is located in UK, north of London about 32 km */
const NB_LatitudeLongitude SpeedCamerasTestDestination = {51.6571, -0.49309};

typedef struct  
{
    NB_SearchHandler*       pSearchHandler;
    NB_SearchInformation*   pSearchInformation;
} SearchTestState;

static SearchTestState g_SearchState = {0};

const uint32 StartNavTime = 908814147;
const uint32 EndNavTime = 908814258;
//=================================================

static void TestNavigationUKCameras(void);
static void TestSpeedCamerasGetCameraNullParam(void);
static void TestSpeedCamerasGetCameraInvalidParam(void);
static void TestSpeedCamerasGetCameras(void);
static void TestSpeedCamerasGetCamerasCountNullInput1(void);
static void TestSpeedCamerasGetCamerasCountNullInput2(void);
static void TestSpeedCamerasGetCamerasCount(void);
static void TestSpeedCamerasGetCamerasState(void);
static void TestnSpeedCamerasGetCameraStateNullInput(void);
static void TestSpeedCamerasGetManeuverCameras(void);
static void TestSpeedCamerasGetManeuverCamerasNullInput1(void);
static void TestSpeedCamerasGetManeuverCamerasNullInput2(void);
static void TestSpeedCamerasGetManeuverCamerasState(void);
static void TestSpeedCamerasGetManeuverCamerasStateNullInput1(void);
static void TestSpeedCamerasGetManeuverCamerasStateNullInput2(void);
static void TestSpeedCamerasInfoDestroyNullInput(void);
static void TestSpeedCamerasGetNextCamera(void);
static void TestSpeedCamerasGetNextCameraNullInput(void);
static void TestSpeedCamerasGetAnoncementPending(void);
static void TestSpeedCamerasGetAnoncementPendingNullInput(void);
static void TestSpeedCamerasUpdateCameraInformationNullInput(void);
static void TestSpeedCamerasQALogCamera(void);
static void TestSpeedCamerasQALogCameraWarning(void);
static void TestSpeedCamerasCreateCameraFromSearch(void);
static void TestSpeedCameras2DNavigation(void);

// Event gets triggered from the download callback.
void* g_CallbackEvent ;
void* g_TimeoutCallbackEvent ;

static void
MainSpeedCamerasTest(NB_NavigationSessionTestCallback TestNavigationValidatorCB)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions sessionOptions = { FALSE, TRUE };

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Unknown UK Location");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude = SpeedCamerasTestDestination.latitude;
    destination.location.longitude = SpeedCamerasTestDestination.longitude;

    routeOptions.type = NB_RouteType_Easiest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "32GallowHillLane_AbbotsLangley.gps", 100, StartNavTime, EndNavTime, TestNavigationValidatorCB, NULL,
        "UK Speed Cameras", "UK_Cameras", &sessionOptions);
}

void 
TestSpeedCameras_AddTests( CU_pSuite pTestSuite, int level )
{
    // Speed Cameras API testing overall time of testrun Speed Cameras API about 13 min
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCameraNullParamFirst", TestSpeedCamerasGetCameraNullParam);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCameraInvalidParam", TestSpeedCamerasGetCameraInvalidParam);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCameras", TestSpeedCamerasGetCameras);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCamerasCountNullInput1", TestSpeedCamerasGetCamerasCountNullInput1);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCamerasCountNullInput2", TestSpeedCamerasGetCamerasCountNullInput2);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCamerasCount", TestSpeedCamerasGetCamerasCount);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestnSpeedCamerasGetCameraStateNullInput", TestnSpeedCamerasGetCameraStateNullInput);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetCamerasState", TestSpeedCamerasGetCamerasState);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCamerasNullInput1", TestSpeedCamerasGetManeuverCamerasNullInput1);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCamerasNullInput2", TestSpeedCamerasGetManeuverCamerasNullInput2);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCameras", TestSpeedCamerasGetManeuverCameras);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCamerasStateNullInput1", TestSpeedCamerasGetManeuverCamerasStateNullInput1);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCamerasStateNullInput2", TestSpeedCamerasGetManeuverCamerasStateNullInput2);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetManeuverCamerasState", TestSpeedCamerasGetManeuverCamerasState);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasInfoDestroyNullInput", TestSpeedCamerasInfoDestroyNullInput);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetNextCameraNullInput", TestSpeedCamerasGetNextCameraNullInput);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetNextCamera", TestSpeedCamerasGetNextCamera);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetAnoncementPendingNullInput", TestSpeedCamerasGetAnoncementPendingNullInput);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasGetAnoncementPending", TestSpeedCamerasGetAnoncementPending);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasUpdateCameraInformationNullInput", TestSpeedCamerasUpdateCameraInformationNullInput);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasQALogCamera", TestSpeedCamerasQALogCamera);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasQALogCameraWarning", TestSpeedCamerasQALogCameraWarning);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationUKCameras", TestNavigationUKCameras);
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpeedCamerasCreateCameraFromSearch", TestSpeedCamerasCreateCameraFromSearch); // in progress
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationSpeedCameras2DNavigations", TestSpeedCameras2DNavigation); // in progress
}

int 
TestSpeedCameras_SuiteSetup()
{
    g_CallbackEvent = CreateCallbackCompletedEvent(); /*! Create event for callback synchronization */
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNavigation_SuiteSetup
*/
int 
TestSpeedCameras_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

/* Speed Cameras Testing. All test-cases describe in:
//depot/Projects/466 - Telefonica Country 1/Drafts/QA/test-scenarios for SpeedCameras.xlsx
*/
static void 
TestNavigationSpeedCamerasGetCamCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_CameraInformationGetCamera(cameraInfo, cameraCount-1, &camera);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_CameraInformationGetCamera(cameraInfo, cameraCount+1, &camera);
    CU_ASSERT_EQUAL(result, NE_INVAL);

    LOGOUTPUT(LogLevelMedium, ("      speed=%f m/s, heading=%f isBidirectional:%d, status=%d, description:\"%s\"\n", camera.speedLimit, camera.heading, camera.isBidirectional, camera.status, camera.description));
}

static void
TestSpeedCamerasGetCameraNullParam(void)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    uint32 nextCameraIndex = 0;
    NB_CameraInformation* cameraInfo = NULL;

    result = NB_CameraInformationGetCamera(cameraInfo, nextCameraIndex, &camera);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_EQUAL(cameraInfo, NULL);

    result = NB_CameraInformationGetCamera(cameraInfo, nextCameraIndex+1, &camera);
    CU_ASSERT_EQUAL(result, NE_INVAL);
    CU_ASSERT_EQUAL(cameraInfo, NULL);
}

static void
TestSpeedCamerasGetCameraInvalidParam(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetCamCB);
}

static void
TestSpeedCamerasGetCameras(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetCamCB);
}

static void
TestSpeedCamerasGetCamerasCountNullInput1(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void
TestSpeedCamerasGetCamerasCountNullInput2(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;

    result = NB_CameraInformationGetCameraCount(cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void 
TestNavigationSpeedCamerasGetCamCountCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelMedium, ("      speed=%f m/s, heading=%f isBidirectional:%d, status=%d, description:\"%s\"\n", camera.speedLimit, camera.heading, camera.isBidirectional, camera.status, camera.description));
}

static void
TestSpeedCamerasGetCamerasCount(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetCamCountCB);
}

static void 
TestnSpeedCamerasGetCameraStateNullInput(void)
{
    NB_Error result = NE_OK;
    NB_CameraState* cameraState = NULL;

    result =  NB_NavigationGetCameraState(NULL, &cameraState);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void 
TestNavigationSpeedCamerasGetCamStateCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    double distanceToNextCamera = 0;
    uint32 nextCameraIndex = 0;
    NB_CameraState* cameraState = NULL;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    result =  NB_NavigationGetCameraState(navigation, &cameraState);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(cameraState);

    if (nextCameraIndex < cameraCount)
    {
        result = NB_CameraProcessorGetNextCamera(cameraState, &nextCameraIndex, &distanceToNextCamera);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT(distanceToNextCamera > 0);
    }
    LOGOUTPUT(LogLevelMedium, ("      speed=%f m/s, heading=%f isBidirectional:%d, status=%d, description:\"%s\"\n", camera.speedLimit, camera.heading, camera.isBidirectional, camera.status, camera.description));
}

static void
TestSpeedCamerasGetCamerasState(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetCamStateCB);
}

static void
TestSpeedCamerasGetManeuverCamerasNullInput1(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    uint32 maneuver = 0;

    result =  NB_CameraInformationGetManeuverCameraCount(cameraInfo, maneuver, &cameraCount);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void
TestSpeedCamerasGetManeuverCamerasNullInput2(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32* cameraCount = NULL;
    uint32 maneuver = 0;

    result =  NB_CameraInformationGetManeuverCameraCount(cameraInfo, maneuver, cameraCount);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void 
TestNavigationSpeedCamerasGetManeuverCamCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    uint32 maneuver = 0;
    uint32 numManeuver = 0;
    NB_RouteInformation* route = NULL;
    nb_boolean control = FALSE;
    uint32 manCameraCount = 0;

    result = NB_NavigationGetInformation(navigation, &route, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if ((cameraInfo == NULL)||(route == NULL))
    {
        return; /* camera and route not yet downloaded */
    }

    maneuver =  NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraCount > 0)
    {
        control = FALSE;
        for(numManeuver = 0; numManeuver < maneuver; numManeuver++)
        {
            result = NB_CameraInformationGetManeuverCameraCount(cameraInfo, numManeuver, &manCameraCount);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (manCameraCount > 0)
            {
                control = TRUE;
            } 
        }
        if (!control)
        {
            CU_FAIL("Camera Count on all maneuver is nil");
        }
    }
}

static void
TestSpeedCamerasGetManeuverCameras(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetManeuverCamCB);
}

static void
TestSpeedCamerasGetManeuverCamerasStateNullInput1(void)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    uint32 maneuver = 0;
    uint32 numCamIndex = 0;

    result = NB_CameraInformationGetManeuverCamera(cameraInfo, maneuver, numCamIndex, &camera);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void
TestSpeedCamerasGetManeuverCamerasStateNullInput2(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 maneuver = 0;
    uint32 numCamIndex = 0;

    result = NB_CameraInformationGetManeuverCamera(cameraInfo, maneuver, numCamIndex, NULL);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}
static void 
TestNavigationSpeedCamerasGetManeuverCamStateCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    uint32 maneuver = 0;
    uint32 numManeuver = 0;
    uint32 numCamIndex = 0;
    NB_RouteInformation* route = NULL;
    nb_boolean control = FALSE;
    uint32 manCameraCount = 0;

    result = NB_NavigationGetInformation(navigation, &route, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if ((cameraInfo == NULL)||(route == NULL))
    {
        return; /* camera and route not yet downloaded */
    }

    maneuver =  NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraCount > 0)
    {
        control = FALSE;
        for(numManeuver = 0; numManeuver < maneuver; numManeuver++)
        {
            result = NB_CameraInformationGetManeuverCameraCount(cameraInfo, numManeuver, &manCameraCount);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (manCameraCount > 0)
            {
                for(numCamIndex = 0; numCamIndex < manCameraCount; numCamIndex++)
                {
                    result = NB_CameraInformationGetManeuverCamera(cameraInfo, numManeuver, numCamIndex, &camera);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT(camera.distance > 0.0);
                }
                control = TRUE;
            } 
        }
        if (!control)
        {
            CU_FAIL("Camera Count on all maneuver is nil");
        }
    }
}

static void
TestSpeedCamerasGetManeuverCamerasState(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetManeuverCamStateCB);
}

static void
TestSpeedCamerasGetNextCameraNullInput(void)
{
    NB_Error result = NE_OK;
    uint32 nextCameraIndex = 0;
    NB_CameraState* cameraState = NULL;
    double distanceToNextCamera = -1.0;

    result = NB_CameraProcessorGetNextCamera(cameraState, &nextCameraIndex, &distanceToNextCamera);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void 
TestNavigationSpeedCamerasGetNextCamCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    uint32 numCamIndex = 0;
    NB_CameraState* cameraState = NULL;
    double distanceToNextCamera = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera and route not yet downloaded */
    }

    result =  NB_NavigationGetCameraState(navigation, &cameraState);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(cameraState);

    if (cameraState == NULL)
    {
        return;
    }

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_CameraProcessorGetNextCamera(cameraState, &numCamIndex, &distanceToNextCamera);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT(distanceToNextCamera > 0);

}

static void
TestSpeedCamerasGetNextCamera(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetNextCamCB);
}

static void 
TestSpeedCamerasGetAnoncementPendingNullInput(void)
{
    nb_boolean control = TRUE;
    NB_CameraState* cameraState = NULL;

    control = NB_CameraProcessorIsAnnouncementPending(cameraState);
    CU_ASSERT_EQUAL(control, FALSE);
    CU_ASSERT_EQUAL(cameraState, NULL);
}

static void 
TestNavigationSpeedCamerasGetAnoncPendingCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;
    nb_boolean control = FALSE;
    NB_CameraState* cameraState = NULL;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera and route not yet downloaded */
    }

    result =  NB_NavigationGetCameraState(navigation, &cameraState);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(cameraState);

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraCount > 0)
    {
        result =  NB_NavigationGetCameraState(navigation, &cameraState);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(cameraState);
        control = NB_CameraProcessorIsAnnouncementPending(cameraState);
        CU_ASSERT_EQUAL(control, FALSE);
        /* @todo: Will add new checks for another conditions when NB_CameraProcessorUpdate() will allow use
            (now cannot get NB_NavigationState* for this function) or this check will add to unit-tests for NB_Services
        */
    }
}

static void
TestSpeedCamerasGetAnoncementPending(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasGetAnoncPendingCB);
}

static void
TestSpeedCamerasUpdateCameraInformationNullInput(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    NB_Navigation* nav = NULL;

    result = NB_NavigationUpdateCameraInformation(nav, cameraInfo);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void
TestSpeedCamerasInfoDestroyNullInput(void)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;

    result =NB_CameraInformationDestroy(cameraInfo);
    CU_ASSERT_EQUAL(result, NE_INVAL);
}

static void 
TestNavigationSpeedCamerasQALogCamCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Camera camera = {0};
    PAL_Error error = PAL_Ok;
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    const char QALogFile[] = "SystemTests.qa";
    uint32 fsize = 0;
    uint32 prevFsize = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCamera(cameraInfo, 0, &camera);
    CU_ASSERT_EQUAL(result, NE_OK);

    error = PAL_FileGetSize(pal, QALogFile, &prevFsize);
    CU_ASSERT_EQUAL(error, PAL_Ok);
    CU_ASSERT(prevFsize > 0);

    NB_QaLogCamera(context, &camera); 
    error = PAL_FileGetSize(pal, QALogFile, &fsize);
    CU_ASSERT_EQUAL(error, PAL_Ok);
    CU_ASSERT(fsize > prevFsize);
}

static void
TestSpeedCamerasQALogCamera(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasQALogCamCB);
}

static void 
TestNavigationSpeedCamerasQALogCamWarningCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Camera camera = {0};
    NB_Error result = NE_OK;
    PAL_Error error = PAL_Ok;
    NB_CameraInformation* cameraInfo = NULL;
    const char QALogFile[] = "SystemTests.qa";
    uint32 fsize = 0;
    uint32 prevFsize = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCamera(cameraInfo, 0, &camera);
    CU_ASSERT_EQUAL(result, NE_OK);

    error = PAL_FileGetSize(pal, QALogFile, &prevFsize);
    CU_ASSERT_EQUAL(error, PAL_Ok);
    CU_ASSERT(prevFsize > 0);

    NB_QaLogCameraWarning(context, camera.distance, &camera); 
    error = PAL_FileGetSize(pal, QALogFile, &fsize);
    CU_ASSERT_EQUAL(error, PAL_Ok);
    CU_ASSERT(fsize > prevFsize);
}

static void
TestSpeedCamerasQALogCameraWarning(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasQALogCamWarningCB);
}
// in progress
static void 
TestNavigationSpeedCamerasCreateCamInfoFromSearchCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_CameraInformation* cameraInfo = NULL;
    NB_CameraInformation* searchCameraInfo = NULL;
    NB_SearchHandler* handler = NULL;
    NB_RequestHandlerCallback callback = {&RequestHandlerCallback, 0};
    NB_SearchRegion region = {0};
    NB_RouteInformation* routeInformation = NULL;
    NB_SearchParameters* parameters = NULL;

    callback.callbackData = 0;
    result = NB_NavigationGetInformation(navigation, &routeInformation, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeInformation);

    region.type = NB_ST_RouteAndCenter;
    region.route = routeInformation;
    region.center.latitude = gpsFix->latitude;
    region.center.longitude = gpsFix->longitude;
    region.distance = 3000;
    region.width = 0;

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_SearchHandlerCreate(context, &callback, &g_SearchState.pSearchHandler);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(handler);

    result = NB_SearchParametersCreatePOI(context, &region, NULL, "safety-cameras", NULL, 0, 100, NB_EXT_WantFormattedText, NULL, &parameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(parameters);

    result = NB_SearchHandlerStartRequest(g_SearchState.pSearchHandler, parameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 30000)) 
    {
        CU_FAIL("Download timed out!");
      //  return;
    }

    result = NB_CameraInformationCreateFromSearch(context, g_SearchState.pSearchInformation, &searchCameraInfo);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchCameraInfo);
}
static void
TestSpeedCamerasCreateCameraFromSearch(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCamerasCreateCamInfoFromSearchCB);
}

static void 
TestNavigationUKCamerasCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_CameraState* cameraState = NULL;
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    uint32 nextCameraIndex = 0;
    double distanceToNextCamera = -1.0;
    NB_CameraInformation* cameraInfo = NULL;
    uint32 cameraCount = 0;

    result = NB_NavigationGetInformation(navigation, NULL, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    LOGOUTPUT(LogLevelMedium, ("   cameraCount = %u\n", cameraCount));
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_NavigationGetCameraState(navigation, &cameraState);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(cameraState);

    result = NB_CameraProcessorGetNextCamera(cameraState, &nextCameraIndex, &distanceToNextCamera);
    LOGOUTPUT(LogLevelMedium, ("   result = %d\n", result));
    LOGOUTPUT(LogLevelMedium, ("   current lat=%f long=%f\n", gpsFix->latitude, gpsFix->longitude));
    LOGOUTPUT(LogLevelMedium, ("   next camera index: %u distance to next camera:%f\n", nextCameraIndex, distanceToNextCamera));
    if (result == NE_NOENT)
    {
        LOGOUTPUT(LogLevelMedium, ("   NO more camera\n"));
        return;
    }

    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_FATAL(nextCameraIndex < cameraCount);

    result = NB_CameraInformationGetCamera(cameraInfo, nextCameraIndex, &camera);
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelMedium, ("      speed=%f m/s, heading=%f isBidirectional:%d, status=%d, description:\"%s\"\n", camera.speedLimit, camera.heading, camera.isBidirectional, camera.status, camera.description));
}
static void
TestNavigationUKCameras(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions sessionOptions = { FALSE, TRUE };

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Unknown UK Location");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude = 51.6571;
    destination.location.longitude =  -0.49309;

    routeOptions.type = NB_RouteType_Easiest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "32GallowHillLane_AbbotsLangley.gps", 100, 908814147, 908814948, TestNavigationUKCamerasCB, NULL,
        "UK Speed Cameras", "UK_Cameras", &sessionOptions);
}

//in progress
static void 
TestNavigationSpeedCameras2DNavigationCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error result = NE_OK;
    NB_Camera camera = {0};
    NB_CameraInformation* cameraInfo = NULL;
    NB_CameraInformation* searchCameraInfo = NULL;
    NB_SearchHandler* handler = NULL;
    uint32 index = 0;
    uint32 camCount = 0;
    NB_RequestHandlerCallback callback = {&RequestHandlerCallback, 0};
    NB_SearchRegion region = {0};
    NB_RouteInformation* routeInformation = NULL;
    NB_SearchParameters* parameters = NULL;

    callback.callbackData = 0;
    result = NB_NavigationGetInformation(navigation, &routeInformation, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeInformation);

    region.type = NB_ST_BoundingBox;
    region.route = routeInformation;
    region.center.latitude = gpsFix->latitude;
    region.center.longitude = gpsFix->longitude;
    region.distance = 3000;
    region.width = 0;

    if (cameraInfo == NULL)
    {
        return; /* camera not yet downloaded */
    }

    result = NB_SearchHandlerCreate(context, &callback, &g_SearchState.pSearchHandler);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(handler);

    result = NB_SearchParametersCreatePOI(context, &region, NULL, "safety-cameras", NULL, 0, 100, NB_EXT_WantFormattedText, NULL, &parameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(parameters);

    result = NB_SearchHandlerStartRequest(g_SearchState.pSearchHandler, parameters);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (! WaitForCallbackCompletedEvent(g_CallbackEvent, 30000)) 
    {
        CU_FAIL(L"Download timed out!");
      //  return;
    }

    result = NB_CameraInformationCreateFromSearch(context, g_SearchState.pSearchInformation, &searchCameraInfo);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(searchCameraInfo);

    result = NB_CameraInformationGetCameraCount(searchCameraInfo, &camCount);
    CU_ASSERT_EQUAL(result, NE_OK);

    for(index = 0; index < camCount; index++)
    {
        result = NB_CameraInformationGetCamera(searchCameraInfo, index, &camera);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(searchCameraInfo);
    }
}
static void
TestSpeedCameras2DNavigation(void)
{
    MainSpeedCamerasTest(TestNavigationSpeedCameras2DNavigationCB);
}
//-----------------------------------------------------------------------------------
/*! @} */
