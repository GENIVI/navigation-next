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

@file     TestNavigation.c
@date     02/03/2009
@defgroup TestNavigation_h System tests for Navigation functions
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

#include "testnavigation.h"
#include "testnetwork.h"
#include "testenhancedcontent.h"

#include "platformutil.h"
#include "palstdlib.h"
#include "stdlib.h"
#include "nbcontextaccess.h"
#include "nbrouteinformation.h"
#include "nbroutehandler.h"
#include "nbtrafficinformation.h"
#include "nbcamerainformation.h"
#include "navpublictypes.h"
#include "nbtrafficprocessor.h"
#include "nbdetourparameters.h"
#include "nbspatial.h"
#include "nbtrafficparameters.h"
#include "nbtraffichandler.h"
#include "nbanalytics.h"
#include "nbenhancedcontentmanager.h"
#include "testdirection.h"
#include "palfile.h"

//const char LA_CSV_FILE[] = "LA_Followme.csv";
const char LAS_VEGAS_GPS_FILE[] = "LasVegas.gps";
const char SAN_FRANCISCO_GPS_FILE[] = "SanFrancisco1.gps";
const char LAS_VEGAS_CSV_FILE[] = "LasVegas1.csv";
const char WORK_DESTINATION_GPS_FILE[] = "work_destination.gps";

typedef struct TestRouteObj
{
    NB_RouteHandler* routeHandler;  
    NB_RouteInformation* route;
    NB_TrafficInformation* trafficInfo;
    NB_CameraInformation* cameraInfo;
    NB_PointsOfInterestInformation* poiInfo;
    NB_NavigateRouteError expectedRouteError; /* expected route error during navigation */
} TestRouteObj;

#define MAX_TEST_POSITION_FIXES 10000

typedef struct TestPositionSummary
{
    int num;
    NB_LatitudeLongitude coords[MAX_TEST_POSITION_FIXES]; 
    NB_LatitudeLongitude lastPos;
} TestPositionSummary;

static char* routingErrors[] = {
    "NB_NRE_None",
    "NB_NRE_TimedOut",
    "NB_NRE_BadDestination",
    "NB_NRE_BadOrigin",
    "NB_NRE_CannotRoute",
    "NB_NRE_EmptyRoute",
    "NB_NRE_NetError",
    "NB_NRE_UnknownError",
    "NB_NRE_NoMatch",
    "NB_NRE_ServerError",
    "NB_NRE_NoDetour",
    "NB_NRE_PedRouteTooLong"
};

/************************************************************************/
/*                  Private Function declaration                        */
/************************************************************************/ 

// System test functions
static void TestNavigationCreateObject(void);
static void TestNavigationNavRequest(void);
static void TestNavigationNavRequest2(void);
static void TestNavigationNavRequest2WithAvoidHighway(void);
static void TestNavigationManeuvers(void);
static void TestNavigationManeuverCrossStreets(void);
static void TestNavigationRouteSelector(void);
static void TestNavigationRequestFromFile(void);
static void TestNavigationRequestWithTrafficNotify(void);
static void TestNavigationRequestIncidents(void);

static void TestNavigationWorkDestination(void);

static void TestNavigationIrvineLAX(void);
static void TestNavigationLibertyToPanera(void);
static void TestNavigationWrongWay(void);
static void TestNavigationStartupCaseToRoad(void);
static void TestNavigationKfcToWork(void);
static void TestNavigationSwedenFerries(void);
static void TestNavigationUKCameras(void);
static void TestNavigationUKCamerasSR(void);
static void TestNavigationFastPOILowes(void);
static void TestNavigationFastPOI5Fwy(void);
static void TestNavigationFastPOILasVegas(void);
static void TestNavigationWorkDestinationDetour(void);
static void TestNavigationWorkDestinationArrival(void);
static void TestNavigationRemainInStartup(void);
static void TestNavigationNoDetour(void);
static void TestNavigationGermanyTunnel(void);

static void TestNavigationDetourParameters(void);

static void TestNavigationVectorPrefetchRealTime(void);
static void TestNavigationTrafficRealTime(void);
static void TestNavigationSeattleTraffic(void);
static void TestNavigationSeattleFloatingBridge(void);
static void TestNavigationAlisoCreekToOrangeCrush(void);
static void TestNavigationLiberty6ToSNA(void);
static void TestNavigationLiberty6ToSNASR(void);
static void TestNavigationTrafficOutsideRouteWindow(void);
static void TestNavigationInitialGuidanceUnnamedRoad(void);
static void TestNavigationInitialGuidanceShortRoute(void);
static void TestNavigationOffRouteRecalcConfirm(void);
static void TestNavigationTripDelay(void);
static void TestNavigationLaCostaToBestBuy(void);
static void TestNavigationTrafficDisabled(void);
static void TestNavigationTrafficManualPolling(void);
static void TestNavigationConfusedHeading(void);
static void TestNavigationStackedManeuvers(void);
static void TestNavigationStaticRouting(void);

//-------------------------------------------------------------------------
// Test by country: Car
//-------------------------------------------------------------------------
static void TestNavigationRouteCarAustria(void);
static void TestNavigationRouteCarBrazil(void);
static void TestNavigationRouteCarBritainSR(void);
static void TestNavigationRouteCarBritain(void);
static void TestNavigationRouteCarBrussels(void);
static void TestNavigationRouteCarCanada(void);
static void TestNavigationRouteCarCzechRepublic(void);
static void TestNavigationRouteCarCroatia(void);
static void TestNavigationRouteCarDenmark(void);
static void TestNavigationRouteCarEstonia(void);
static void TestNavigationRouteCarFinland(void);
static void TestNavigationRouteCarFrance(void);
static void TestNavigationRouteCarGermany(void);
static void TestNavigationRouteCarGreece(void);
static void TestNavigationRouteCarHungary(void);
static void TestNavigationRouteCarIreland(void);
static void TestNavigationRouteCarItaly(void);
static void TestNavigationRouteCarLatvia(void);
static void TestNavigationRouteCarLithuania(void);
static void TestNavigationRouteCarLuxembourg(void);     // Not in PRD
static void TestNavigationRouteCarMexico(void);
static void TestNavigationRouteCarNetherlands(void);
static void TestNavigationRouteCarPoland(void);         // Not in PRD 
static void TestNavigationRouteCarPortugal(void);
static void TestNavigationRouteCarRomania(void);        // Not in PRD 
static void TestNavigationRouteCarSpain(void);
static void TestNavigationRouteCarSweden(void);
static void TestNavigationRouteCarSwitzerland(void);
static void TestNavigationRouteCarTurkey(void);


//-------------------------------------------------------------------------
// Test by country: Pedestrian
//-------------------------------------------------------------------------
static void TestNavigationPedestrianAustria(void);
static void TestNavigationPedestrianBrazil(void);
static void TestNavigationPedestrianBrussels(void);
static void TestNavigationPedestrianCanada(void);
static void TestNavigationPedestrianCzechRepublic(void);
static void TestNavigationPedestrianCroatia(void);
static void TestNavigationPedestrianDenmark(void);
static void TestNavigationPedestrianEstonia(void);
static void TestNavigationPedestrianFinland(void);
static void TestNavigationPedestrianFrance(void);
static void TestNavigationPedestrianIreland(void);
static void TestNavigationPedestrianItaly(void);
static void TestNavigationPedestrianLatvia(void);
static void TestNavigationPedestrianLithuania(void);
static void TestNavigationPedestrianGermany(void);
// TODO: static void TestNavigationPedestrianGreece(void);
static void TestNavigationPedestrianHungary(void);
static void TestNavigationPedestrianMexico(void);
static void TestNavigationPedestrianNetherlands(void);
static void TestNavigationPedestrianNorway(void);
static void TestNavigationPedestrianPoland(void);
static void TestNavigationPedestrianPortugal(void);
static void TestNavigationPedestrianSpain(void);
static void TestNavigationPedestrianSweden(void);
static void TestNavigationPedestrianSwitzerland(void);
static void TestNavigationPedestrianTurkey(void);
static void TestNavigationPedestrianUnitedKingdom(void);
static void TestNavigationPedestrianUSA(void);
static void TestNavigationPedestrianRouteTooLong(void);
static void TestNavigationPedestrianNoRecalc(void);
static void TestNavigationPedestrianGuidance(void);
static void TestNavigationPedestrianManeuvers(void);

static void TestNavigation73ToLiberty(void);
static void TestNavigationMultipleRecalc(void);
static void TestNavigationRecalcAfterReachingArriving(void);
static void TestNavigationSeattleTraffic(void);

static void TestNavigationEnhancedRealisticSigns(void);
static void TestNavigationEnhancedJunctionsWithSigns(void);
static void TestNavigationEnhancedCityModels(void);

static void 
NavigationTestNavigationRouteError(NB_Place* destination, NB_RouteOptions* routeOptions, 
    NB_RouteConfiguration* routeConfiguration, const char* gpsFileName, int fixDelayMilliseconds, 
    uint32 startTime, uint32 endTime, 
    NB_NavigationSessionTestCallback* testCallback, void* testCallbackUserData, 
    const char* testName, const char* logBaseName, TestNavigationSessionOptions* sessionOptions,
    NB_NavigateRouteError expectedRouteError);
static void TestNavigationPronunDataForTTS(void);

// Sub-Tests
static void Test_ValidateRouteInformation(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, NB_Navigation* navigation);
static void Test_ValidateRoutePolyline(NB_RouteInformation* route);
static void Test_ValidateManeuvers(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, NB_Navigation* navigation);
static void Test_ValidateTurnImages(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance);
static void Test_ValidateTurnAnnouncements(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance, NB_Navigation* navigation);
static void Test_ValidateManeuverText(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance);
static nb_boolean Test_ValidatePronunCode(PAL_Instance* pal, NB_Context* context, const char* code, nb_size size);
static void Test_ValidateGuidanceMessage(PAL_Instance* pal, NB_Context* context, NB_GuidanceMessage* message, NB_Navigation* navigation);
static void Test_ValidateElevators(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance, uint32 *numElevators);
static void Test_ValidateNavigationStatus(NB_Navigation* navigation, nb_gpsTime currentTime, nb_gpsTime startTime, nb_gpsTime endTime, NB_NavigateStatus status);


static void Test_DumpRouteToKML(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, TestPositionSummary* navigationSummary,  TestPositionSummary* trafficSummary, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max, const char* displayName, const char* kmlFileName, uint32 routeColor);


// Subfunctions of system test functions
static char* GetStatusString(NB_NavigateStatus status);
static const char* GetManeuverPostionString(NB_NavigateManeuverPos position);

static void EnumerateTextCallback(NB_Font font, nb_color color, const char* txt, nb_boolean newline, void* pUser);

void SetupRouteConfigurationVehicle(NB_RouteConfiguration* routeConfiguration);
static void SetupRouteConfigurationPedestrian(NB_RouteConfiguration* routeConfiguration);
static void SetupPlaces (NB_Place* defaultOrigin, NB_Place* defaultDest, NB_Place* origin, NB_Place* destination);
static void SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config);

static void SetupNavigationConfiguration(NB_NavigationConfiguration* configuration);

static void RequestRoute(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj, NB_RouteParameters* inParams, NB_RouteInformation* recalcRoute, NB_Place* origin, NB_Place* destination, NB_RouteAvoid avoid);
static void RequestRouteSelector(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj);
static void RequestPedestrianRoute(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj, NB_RouteParameters* inParams, NB_RouteInformation* recalcRoute, NB_Place* origin, NB_Place* destination);

static NB_Error DestroyRouteObject(TestRouteObj* routeObj, nb_boolean usedForNavigation);
static void ClearInformationRouteObject(TestRouteObj* routeObj);
static void DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void NavigationModeChangeCallback(NB_Navigation* navigation, NB_NavigationMode mode, void* userData);

static void TestNavigationWorkDestinationCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData);

static void TestNavigationSessionGuidance(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData);

static void DumpTrafficInformation(NB_Navigation* navigation, const char* source);
static void LogRouteError(NB_Navigation* navigation, char filename[], unsigned int line);

static void DumpPointsOfInterestAlongRoute(NB_Navigation* navigation);

static void DumpCameraInformation(NB_Navigation* navigation);

static void DumpNextUpcomingTrafficEvent(NB_Navigation* navigation);

static void DumpTrafficEventInformationForEachManeuver(NB_Navigation* navigation, const char* source);
static void DumpTrafficEventList(NB_Navigation* navigation, const char* source);


#define LOG_ROUTE_ERROR(nav) LogRouteError(nav, __FILE__, __LINE__)

// Variables .................................................................

// Event gets triggered from the download callback.
void* g_CallbackEvent = NULL;
void* g_TimeoutCallbackEvent = NULL;

// Status gets updated in the navigation callback function
static NB_NavigateStatus g_oldStatus = NB_NS_Invalid;

static nb_boolean g_wantExtendedPronunData = FALSE;

void 
SetupRouteConfigurationVehicle(NB_RouteConfiguration* routeConfiguration)
{
    nsl_memset(routeConfiguration, 0, sizeof(*routeConfiguration));

    routeConfiguration->wantOriginLocation = TRUE;
    routeConfiguration->wantDestinationLocation = TRUE;
    routeConfiguration->wantRouteMap = FALSE;
    routeConfiguration->wantCrossStreets = TRUE;
    routeConfiguration->wantPolyline = FALSE;
    routeConfiguration->wantManeuvers = TRUE;
    routeConfiguration->wantFerryManeuvers = TRUE;
    routeConfiguration->wantTowardsRoadInformation = FALSE;
    routeConfiguration->wantIntersectionRoadInformation = FALSE;
    routeConfiguration->wantCountryInformation = TRUE;
   	routeConfiguration->wantEnterRoundaboutManeuvers = TRUE;
	routeConfiguration->wantEnterCountryManeuvers = TRUE;
	routeConfiguration->wantBridgeManeuvers = TRUE;
	routeConfiguration->wantTunnelManeuvers = TRUE;
    routeConfiguration->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */
    routeConfiguration->wantRouteExtents = TRUE;

    // enhanced content disabled by default
    routeConfiguration->wantCityModels = FALSE;
    routeConfiguration->wantJunctionModels = FALSE;
    routeConfiguration->wantRealisticSigns = FALSE;

    // want speed camera request disabled by default
    routeConfiguration->wantSpeedCameras = FALSE;

    // want non guidance maneuvers disabled by default
    routeConfiguration->wantNonGuidanceManeuvers = FALSE;
}

static void 
SetupRouteConfigurationPedestrian(NB_RouteConfiguration* routeConfiguration)
{
    nsl_memset(routeConfiguration, 0, sizeof(*routeConfiguration));

    routeConfiguration->wantOriginLocation = TRUE;
    routeConfiguration->wantDestinationLocation = TRUE;
    routeConfiguration->wantRouteMap = FALSE;
    routeConfiguration->wantCrossStreets = FALSE;
    routeConfiguration->wantPolyline = FALSE;
    routeConfiguration->wantManeuvers = TRUE;
    routeConfiguration->wantFerryManeuvers = TRUE;
    routeConfiguration->wantTowardsRoadInformation = FALSE;
    routeConfiguration->wantIntersectionRoadInformation = FALSE;
    routeConfiguration->wantCountryInformation = TRUE;
   	routeConfiguration->wantEnterRoundaboutManeuvers = TRUE;
	routeConfiguration->wantEnterCountryManeuvers = TRUE;
	routeConfiguration->wantBridgeManeuvers = TRUE;
	routeConfiguration->wantTunnelManeuvers = TRUE;
    routeConfiguration->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */

    // enhanced content disabled by default
    routeConfiguration->wantCityModels = FALSE;
    routeConfiguration->wantJunctionModels = FALSE;
    routeConfiguration->wantRealisticSigns = FALSE;
}

/*! Add all your test functions here

@return None
*/

void
SetupPlaces (NB_Place* defaultOrigin, NB_Place* defaultDest, NB_Place* origin, NB_Place* destination)
{
    if (defaultOrigin && origin)
    {   
        defaultOrigin->location.type = NB_Location_Address;
        strcpy(defaultOrigin->location.streetnum, origin->location.streetnum);
        strcpy(defaultOrigin->location.street1, origin->location.street1);
        strcpy(defaultOrigin->location.city, origin->location.postal);
        strcpy(defaultOrigin->location.state, origin->location.postal);
        strcpy(defaultOrigin->location.postal, origin->location.postal);
        defaultOrigin->location.latitude = origin->location.latitude;
        defaultOrigin->location.longitude = origin->location.longitude;
    }
    else if (defaultOrigin)
    {   
        defaultOrigin->location.type = NB_Location_Address;
        strcpy(defaultOrigin->location.streetnum, "35");
        strcpy(defaultOrigin->location.street1, "Tesla");
        strcpy(defaultOrigin->location.city, "Irvine");
        strcpy(defaultOrigin->location.state, "CA");
        strcpy(defaultOrigin->location.postal, "92618");
        defaultOrigin->location.latitude = 33.63435;
        defaultOrigin->location.longitude = -117.73638;
    }
    
    if ( defaultDest && destination )
    {
        defaultDest->location.type = NB_Location_Address;
        strcpy(defaultDest->location.streetnum, destination->location.streetnum);
        strcpy(defaultDest->location.street1, destination->location.street1);
        strcpy(defaultDest->location.city, destination->location.postal);
        strcpy(defaultDest->location.state, destination->location.postal);
        strcpy(defaultDest->location.postal, destination->location.postal);
        defaultDest->location.latitude = destination->location.latitude;
        defaultDest->location.longitude = destination->location.longitude;
    }
    else if (defaultDest)
    {    
        defaultDest->location.type = NB_Location_Address;
        strcpy(defaultDest->location.streetnum, "18872");
        strcpy(defaultDest->location.street1, "Bardeen");
        strcpy(defaultDest->location.city, "Irvine");
        strcpy(defaultDest->location.state, "CA");
        strcpy(defaultDest->location.postal, "92612");
        defaultDest->location.latitude = 33.67035;
        defaultDest->location.longitude = -117.85560;      
    }
}

static void
SetupVectorMapPrefetchConfiguration(NB_VectorMapPrefetchConfiguration* config)
{
    config->segmentExtensionLengthMeters = 1000.0;
    config->prefetchWindowWidthMeters = 750.0;    
    config->minPrefetchDistance = 1000.0;

    config->numberOfPrefetchTiles = 75;

    config->tz = 16;
    config->secondsBeforeFirstPrefetch = 5;
    config->secondsBetweenPrefetch = 15;
}

void 
SetupConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config)
{        
    uint32 command = 0;

    command = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
    // If this asserts, the command instructions sets (imageStyle, pronunStyle and stepsStyle) cannot be found
    // See CreateGuidanceInformation for the three directories that are expected to contain appropriate config.tps files
    CU_ASSERT_NOT_EQUAL_FATAL(command, 0);

    options->transport = NB_TransportationMode_Car;
    options->type = NB_RouteType_Fastest;
    options->traffic = NB_TrafficType_Avoid;
    options->pronunStyle = Test_OptionsGet()->voiceStyle;

    config->wantDestinationLocation = TRUE;
    config->wantManeuvers = TRUE;
    config->wantOriginLocation = TRUE;
    config->wantPolyline = FALSE;    
    config->wantRouteMap = FALSE;
    config->wantCrossStreets = TRUE;
    config->commandSetVersion = command;    
    config->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */
}


void 
SetupPedestrianConfigAndOptions(NB_Context* context, NB_GuidanceInformation* guidanceInfo, NB_RouteOptions* options, NB_RouteConfiguration* config)
{        
    uint32 command = 0;

    command = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
    // If this asserts, the command instructions sets (imageStyle, pronunStyle and stepsStyle) cannot be found
    // See CreateGuidanceInformation for the three directories that are expected to contain appropriate config.tps files
    CU_ASSERT_NOT_EQUAL_FATAL(command, 0);

    options->type = NB_RouteType_Fastest;
    options->avoid = NB_RouteAvoid_HOV;    
    options->transport = NB_TransportationMode_Pedestrian;
    options->traffic = NB_TrafficType_None;
    options->pronunStyle = Test_OptionsGet()->voiceStyle;

    nsl_memset(config, 0, sizeof(*config));

    config->wantOriginLocation = TRUE;
    config->wantDestinationLocation = TRUE;
    config->wantRouteMap = FALSE;
    config->wantCrossStreets = FALSE;
    config->wantPolyline = FALSE;
    config->wantManeuvers = TRUE;
    config->wantFerryManeuvers = TRUE;
    config->wantTowardsRoadInformation = FALSE;
    config->wantIntersectionRoadInformation = FALSE;
    config->wantCountryInformation = TRUE;
   	config->wantEnterRoundaboutManeuvers = TRUE;
	config->wantEnterCountryManeuvers = TRUE;
	config->wantBridgeManeuvers = TRUE;
	config->wantTunnelManeuvers = TRUE;
    config->maxPronunFiles = 64; /* 64 for speedup GPS file playback, 3 for normal GPS */
    config->commandSetVersion = NB_GuidanceInformationGetCommandVersion(guidanceInfo);
}

static void 
RequestRoute(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj, NB_RouteParameters* inParams, NB_RouteInformation* recalcRoute, NB_Place* origin, NB_Place* destination, NB_RouteAvoid inAvoid)
{    
    NB_RouteParameters*       routeParameters = 0;
    NB_Place                  pStart      = {{0}};  
    NB_Place                  pDest       = {{0}};     
    NB_RouteOptions           options = {0};
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { 0 };    
    NB_Error                  result      = NE_OK;    

    routeHandlerCallback.callback = DownloadRouteCallback;
    routeHandlerCallback.callbackData = RouteObj;
   
    LOGOUTPUT(LogLevelHigh, ("\n\n"));
       
    SetupPlaces(&pStart, &pDest, origin, destination);
    SetupConfigAndOptions(context, guidanceInfo, &options, &config);
    
    options.avoid = inAvoid;

    if (inParams)
    {
        routeParameters = inParams;
    }
    else if (recalcRoute)
    {
        result = NB_RouteParametersCreateOffRoute(context, NB_ContextGetGpsHistory(context), recalcRoute, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
    }    
    else
    {
        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
    }        

    if (!RouteObj->routeHandler)
    {
        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &RouteObj->routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(RouteObj->routeHandler);
    }
    
    if (NULL != routeParameters && NULL != RouteObj->routeHandler)
    {
        ResetCallbackCompletedEvent(g_CallbackEvent);
        
        result = NB_RouteHandlerStartRequest(RouteObj->routeHandler, routeParameters);

        CU_ASSERT_EQUAL(result, NE_OK);

        if (NE_OK == result)
        {                
            // Wait for event here from callback. Timeout 1 min.
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
            {
                
            }                
        }
        else
        {
            LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
        }
    }
                     
    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);    
}

static void 
RequestRouteSelector(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj)
{
    NB_RouteParameters*       routeParameters = 0;
    NB_Place                  pStart      = {{0}};
    NB_Place                  pDest       = {{0}};
    NB_RouteOptions           options = {0};
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { 0 };
    NB_Error                  result      = NE_OK;

    routeHandlerCallback.callback = DownloadRouteCallback;
    routeHandlerCallback.callbackData = RouteObj;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    SetupPlaces(&pStart, &pDest, NULL, NULL);
    SetupConfigAndOptions(context, guidanceInfo, &options, &config);

    config.wantAlternateRoutes = TRUE;

    result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeParameters);

    if (!RouteObj->routeHandler)
    {
        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &RouteObj->routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(RouteObj->routeHandler);
    }

    if (NULL != routeParameters && NULL != RouteObj->routeHandler)
    {
        result = NB_RouteHandlerStartRequest(RouteObj->routeHandler, routeParameters);

        CU_ASSERT_EQUAL(result, NE_OK);

        if (NE_OK == result)
        {
            ResetCallbackCompletedEvent(g_CallbackEvent);
            // Wait for event here from callback. Timeout 1 min.
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
            {

            }
        }
        else
        {
            LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
        }
    }

    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);
}

static void
RequestPedestrianRoute(PAL_Instance* pal, NB_Context* context, NB_GuidanceInformation* guidanceInfo, TestRouteObj* RouteObj, NB_RouteParameters* inParams, NB_RouteInformation* recalcRoute, NB_Place* origin, NB_Place* destination)
{    
    NB_RouteParameters*       routeParameters = 0;
    NB_Place                  pStart      = {{0}};  
    NB_Place                  pDest       = {{0}};     
    NB_RouteOptions           options = {0};
    NB_RouteConfiguration     config = {0};
    NB_RequestHandlerCallback routeHandlerCallback = { 0 };    
    NB_Error                  result      = NE_OK;    

    routeHandlerCallback.callback = DownloadRouteCallback;
    routeHandlerCallback.callbackData = RouteObj;
   
    LOGOUTPUT(LogLevelHigh, ("\n\n"));
       
    SetupPlaces(&pStart, &pDest, origin, destination);
    SetupPedestrianConfigAndOptions(context, guidanceInfo, &options, &config);
    
    if (inParams)
    {
        routeParameters = inParams;
    }
    else if (recalcRoute)
    {
        result = NB_RouteParametersCreateOffRoute(context, NB_ContextGetGpsHistory(context), recalcRoute, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
    }    
    else
    {
        result = NB_RouteParametersCreateFromPlace(context, &pStart, &pDest, &options, &config, &routeParameters);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(routeParameters);
    }        

    if (!RouteObj->routeHandler)
    {
        result = NB_RouteHandlerCreate(context, &routeHandlerCallback, &RouteObj->routeHandler);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(RouteObj->routeHandler);
    }
    
    if (NULL != routeParameters && NULL != RouteObj->routeHandler)
    {
        ResetCallbackCompletedEvent(g_CallbackEvent);
        
        result = NB_RouteHandlerStartRequest(RouteObj->routeHandler, routeParameters);

        CU_ASSERT_EQUAL(result, NE_OK);

        if (NE_OK == result)
        {                
            // Wait for event here from callback. Timeout 1 min.
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
            {
                
            }                
        }
        else
        {
            LOGOUTPUT(LogLevelHigh, ("TestDirectionsRequest2:: NBI_DirectionsSetup Failure\n"));
        }
    }
                     
    result = NB_RouteParametersDestroy(routeParameters);
    CU_ASSERT_EQUAL(result, NE_OK);    
}

static NB_Error
DestroyRouteObject(TestRouteObj* routeObj, nb_boolean usedForNavigation)
{
    NB_Error result = NE_OK;

    if (routeObj->routeHandler && result == NE_OK)
    {
        result = NB_RouteHandlerDestroy(routeObj->routeHandler);
    }

    if (!usedForNavigation)
    {
        if (routeObj->route && result == NE_OK)
        {
            result = NB_RouteInformationDestroy(routeObj->route);
        }

        if (routeObj->trafficInfo && result == NE_OK)
        {
            result = NB_TrafficInformationDestroy(routeObj->trafficInfo);
        }
    }

    if (routeObj->cameraInfo && result == NE_OK)
    {
        result = NB_CameraInformationDestroy(routeObj->cameraInfo);
    }

    if (routeObj->poiInfo && result == NE_OK)
    {
        result = NB_PointsOfInterestInformationDestroy(routeObj->poiInfo);
    }

    return result;
}

static void 
ClearInformationRouteObject(TestRouteObj* routeObj)
{
    routeObj->route = NULL;
    routeObj->trafficInfo = NULL;
    routeObj->cameraInfo = NULL;
    routeObj->poiInfo = NULL;
    routeObj->expectedRouteError = NB_NRE_None;
}

static void 
DownloadRouteCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData)
{          
    NB_Error result = NE_OK;
    TestRouteObj* routeObj = (TestRouteObj*) pUserData;
    if (err != NE_OK)
    {
        // Abort and trigger event
        CU_FAIL("Callback returned error");
        SetCallbackCompletedEvent(g_CallbackEvent);
        return;
    }
    LOGOUTPUT(LogLevelHigh, ("route download callback status = %d, err = %d, percent = %d\n", status, err, percent));

    switch (status)
    {
    case NB_NetworkRequestStatus_Progress:
        // not complete yet... return
        return;

    case NB_NetworkRequestStatus_Success:
        {
            result = NB_RouteHandlerGetRouteInformation(routeObj->routeHandler, &routeObj->route);  
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_RouteHandlerGetTrafficInformation(routeObj->routeHandler, &routeObj->trafficInfo);  
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        break;
    case NB_NetworkRequestStatus_TimedOut:
        CU_FAIL("NB_NetworkRequestStatus_TimedOut");
        break;
    case NB_NetworkRequestStatus_Canceled:
        LOGOUTPUT(LogLevelHigh, ("NB_NetworkRequestStatus_Canceled"));
        CU_FAIL("NB_NetworkRequestStatus_Canceled");
        break;
    case NB_NetworkRequestStatus_Failed:
        CU_FAIL("NB_NetworkRequestStatus_Failed");
        break;
    }

    // Trigger main thread
    SetCallbackCompletedEvent(g_CallbackEvent);
    return;
}

void 
TestNavigation_AddTests( CU_pSuite pTestSuite, int level )
{
    // Basic Functional Tests  
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationCreateObject", TestNavigationCreateObject);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationNavRequest", TestNavigationNavRequest);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationNavRequest2", TestNavigationNavRequest2);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationNavRequest2WithAvoidHighway", TestNavigationNavRequest2WithAvoidHighway);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationManeuvers", TestNavigationManeuvers);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationManeuverCrossStreets", TestNavigationManeuverCrossStreets);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationRouteSelector", TestNavigationRouteSelector);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRequestFromFile", TestNavigationRequestFromFile);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRequestWithTrafficNotify", TestNavigationRequestWithTrafficNotify);

    // Navigation Session Tests -- May run for a considerable length of time
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationWorkDestination", TestNavigationWorkDestination);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationIrvineLAX", TestNavigationIrvineLAX);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationLibertyToPanera", TestNavigationLibertyToPanera);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigation73ToLiberty", TestNavigation73ToLiberty);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationStartupCaseToRoad", TestNavigationStartupCaseToRoad);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationMultipleRecalc", TestNavigationMultipleRecalc);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationRecalcAfterReachingArriving", TestNavigationRecalcAfterReachingArriving);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationKfcToWork", TestNavigationKfcToWork);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationSwedenFerries", TestNavigationSwedenFerries);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationWorkDestinationDetour", TestNavigationWorkDestinationDetour);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationGermanyTunnel", TestNavigationGermanyTunnel);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationUKCameras", TestNavigationUKCameras);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationFastPOILowes", TestNavigationFastPOILowes);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationFastPOI5Fwy", TestNavigationFastPOI5Fwy);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationFastPOILasVegas", TestNavigationFastPOILasVegas);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationWorkDestinationArrival", TestNavigationWorkDestinationArrival);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRemainInStartup", TestNavigationRemainInStartup);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationNoDetour", TestNavigationNoDetour);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationWrongWay", TestNavigationWrongWay);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationDetourParameters", TestNavigationDetourParameters);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationVectorPrefetchRealTime", TestNavigationVectorPrefetchRealTime);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationTrafficRealTime", TestNavigationTrafficRealTime);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationSeattleTraffic", TestNavigationSeattleTraffic);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationAlisoCreekToOrangeCrush", TestNavigationAlisoCreekToOrangeCrush);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationLiberty6ToSNA", TestNavigationLiberty6ToSNA);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationTrafficOutsideRouteWindow", TestNavigationTrafficOutsideRouteWindow);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationInitialGuidanceUnnamedRoad", TestNavigationInitialGuidanceUnnamedRoad);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationInitialGuidanceShortRoute", TestNavigationInitialGuidanceShortRoute);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationOffRouteRecalcConfirm", TestNavigationOffRouteRecalcConfirm);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationSeattleFloatingBridge", TestNavigationSeattleFloatingBridge);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationSeattleFloatingBridge", TestNavigationSeattleFloatingBridge);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationConfusedHeading", TestNavigationConfusedHeading);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationStackedManeuvers", TestNavigationStackedManeuvers);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationTripDelay", TestNavigationTripDelay);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationLaCostaToBestBuy", TestNavigationLaCostaToBestBuy);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationTrafficDisabled", TestNavigationTrafficDisabled);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationTrafficManualPolling", TestNavigationTrafficManualPolling);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNavigationStaticRouting", TestNavigationStaticRouting);
    //ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarBritainSR", TestNavigationRouteCarBritainSR);
    //ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationLiberty6ToSNASR", TestNavigationLiberty6ToSNASR);
    //ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationUKCamerasSR", TestNavigationUKCamerasSR);

    // Test by country: CAR
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarAustria",        TestNavigationRouteCarAustria);
	//server don't have map config for Brazil
	//ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarBrazil",         TestNavigationRouteCarBrazil);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarBritain",        TestNavigationRouteCarBritain);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarBrussels",       TestNavigationRouteCarBrussels);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationRouteCarCanada",         TestNavigationRouteCarCanada);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarCzechRepublic",  TestNavigationRouteCarCzechRepublic);   
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarCroatia",        TestNavigationRouteCarCroatia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarDenmark",        TestNavigationRouteCarDenmark);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarEstonia",        TestNavigationRouteCarEstonia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarFinland",        TestNavigationRouteCarFinland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarFrance",         TestNavigationRouteCarFrance);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarGermany",        TestNavigationRouteCarGermany);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarGreece",         TestNavigationRouteCarGreece);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarHungary",        TestNavigationRouteCarHungary);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarIreland",        TestNavigationRouteCarIreland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarItaly",          TestNavigationRouteCarItaly);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarLatvia",         TestNavigationRouteCarLatvia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarLithuania",      TestNavigationRouteCarLithuania);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarLuxembourg",     TestNavigationRouteCarLuxembourg);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarMexico",         TestNavigationRouteCarMexico);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarNetherlands",    TestNavigationRouteCarNetherlands);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarPoland",         TestNavigationRouteCarPoland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarPortugal",       TestNavigationRouteCarPortugal);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarRomania",        TestNavigationRouteCarRomania);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarSpain",          TestNavigationRouteCarSpain);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarSweden",         TestNavigationRouteCarSweden);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarSwitzerland",    TestNavigationRouteCarSwitzerland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationRouteCarTurkey",         TestNavigationRouteCarTurkey);   

    // Test by country: Pedestrian
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationPedestrianUSA",           TestNavigationPedestrianUSA);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianAustria",       TestNavigationPedestrianAustria);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianBrazil",        TestNavigationPedestrianBrazil);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianBrussels",      TestNavigationPedestrianBrussels);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianCanada",        TestNavigationPedestrianCanada);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianCroatia",       TestNavigationPedestrianCroatia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianCzechRepublic", TestNavigationPedestrianCzechRepublic);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianDenmark",       TestNavigationPedestrianDenmark);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianEstonia",       TestNavigationPedestrianEstonia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianFinland",       TestNavigationPedestrianFinland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianFrance",        TestNavigationPedestrianFrance);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianItaly",         TestNavigationPedestrianItaly);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianGermany",       TestNavigationPedestrianGermany);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianHungary",       TestNavigationPedestrianHungary);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianIreland",       TestNavigationPedestrianIreland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianLatvia",        TestNavigationPedestrianLatvia);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianLithuania",     TestNavigationPedestrianLithuania);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianMexico",        TestNavigationPedestrianMexico);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianNetherlands",   TestNavigationPedestrianNetherlands);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianNorway",        TestNavigationPedestrianNorway);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianPoland",        TestNavigationPedestrianPoland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianPortugal",      TestNavigationPedestrianPortugal);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianSpain",         TestNavigationPedestrianSpain);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianSweden",        TestNavigationPedestrianSweden);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianSwitzerland",   TestNavigationPedestrianSwitzerland);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianTurkey",        TestNavigationPedestrianTurkey);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNavigationPedestrianUnitedKingdom", TestNavigationPedestrianUnitedKingdom);

    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationPedestrianRouteTooLong",  TestNavigationPedestrianRouteTooLong);   
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationPedestrianNoRecalc",      TestNavigationPedestrianNoRecalc);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationPedestrianGuidance",      TestNavigationPedestrianGuidance);

    // Enhanced Content Tests
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationEnhancedJunctionsWithSigns", TestNavigationEnhancedJunctionsWithSigns);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationEnhancedCityModels", TestNavigationEnhancedCityModels);
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationEnhancedRealisticSigns", TestNavigationEnhancedRealisticSigns);

    // TTS pronun
    ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestNavigationPronunDataForTTS", TestNavigationPronunDataForTTS);
}


/*! Add common initialization code here.

@return 0

@see TestNavigation_SuiteCleanup
*/
int 
TestNavigation_SuiteSetup()
{
    g_CallbackEvent = CreateCallbackCompletedEvent(); /*! Create event for callback synchronization */
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNavigation_SuiteSetup
*/
int 
TestNavigation_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_CallbackEvent);
    return 0;
}

/*! This system test is used to test the network object creation.

Before creating a navigation object we need NB_Network, NB_State and Pal_Instance objects.
This method tests the creation of these objects

@return None. CUnit Asserts get called on failures.

@see NavigationRequest
*/
void 
TestNavigationCreateObject(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_Navigation*            navigation = 0;
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_NavigationConfiguration  configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);
        
        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, NULL, 0);
        
        result = NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(navigation);
               
        /* Don't actually do anything */
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, FALSE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Make a Navigation request.

This system test connects to a server. Makes a Navigation request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see NavigationRequest
*/
void 
TestNavigationNavRequest(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, NULL, 0);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);
        }
        else
        {
            CU_FAIL("No route returned");
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


/*! Make a Navigation request.

This system test connects to a server. Makes a Navigation request and then
outputs the results on the standard output.

@return None. CUnit Asserts get called on failures.

@see NavigationRequest
*/
void 
TestNavigationNavRequest2(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_Place                  destination = {{0}};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    // LAX
    destination.location.type = NB_Location_Address;
    strcpy(destination.location.streetnum, "1");
    strcpy(destination.location.street1, "World Way");
    strcpy(destination.location.city, "Los Angeles");
    strcpy(destination.location.state, "CA");
    strcpy(destination.location.postal, "90045");
    destination.location.latitude = 34.0522222;
    destination.location.longitude = -118.2427778;

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, &destination, 0);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
            CU_ASSERT_EQUAL(result, NE_OK);

            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);
        }
        else
        {
            CU_FAIL("No Route Returned");
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}



/*! Make a Navigation request with the avoid highway option.

This system test connects to a server. Makes a Navigation request and then
outputs the results on the standard output.

This test was added to check BUG #62342.

@return None. CUnit Asserts get called on failures.

@see NavigationRequest
*/
void 
TestNavigationNavRequest2WithAvoidHighway(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_Place                  destination = {{0}};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    // LAX
    destination.location.type = NB_Location_Address;
    strcpy(destination.location.streetnum, "1");
    strcpy(destination.location.street1, "World Way");
    strcpy(destination.location.city, "Los Angeles");
    strcpy(destination.location.state, "CA");
    strcpy(destination.location.postal, "90045");
    destination.location.latitude = 34.0522222;
    destination.location.longitude = -118.2427778;

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, &destination, NB_RouteAvoid_Highway);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
            CU_ASSERT_EQUAL(result, NE_OK);

            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);
        }
        else
        {
            CU_FAIL("No Route Returned");
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

void
TestNavigationManeuverCrossStreets(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    uint32                    numManeuvers = 0;
    NB_GuidanceInformation*   guidanceInfo = 0;    
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, NULL, 0);
        
        if (routeObj.route != NULL)
        {
            numManeuvers = NB_RouteInformationGetManeuverCount(routeObj.route);
            CU_ASSERT(numManeuvers > 0);

            if (numManeuvers > 0)
            {
                uint32 crossStreetCount = 0;
                uint32 nman = 0;
                uint32 ncsc = 0;
                char crossStreet[NB_ROUTEINFORMATION_STREET_NAME_MAX_LEN];
                
                for (nman=0; nman<numManeuvers; nman++)
                {
                    result = NB_RouteInformationGetManeuverCrossStreetsCount(routeObj.route, nman, &crossStreetCount);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    for (ncsc=0; ncsc<crossStreetCount; ncsc++)
                    {
                        result = NB_RouteInformationGetManeuverCrossStreets(routeObj.route, nman, ncsc, crossStreet);
                        CU_ASSERT_EQUAL(result, NE_OK);
                    }
                }                
            }
        }
        else 
        {
            CU_FAIL("No Route Returned");
        }
    }

    result = DestroyRouteObject(&routeObj, FALSE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void
TestNavigationRouteSelector(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    uint32 routeCount = 0;
    uint32 i = 0;

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context))
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRouteSelector(pal, context, guidanceInfo, &routeObj);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_RouteInformationGetRoutesCount(routeObj.route, &routeCount);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_NOT_EQUAL(routeCount, 0);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);
        }
        else
        {
            CU_FAIL("No route returned");
        }

        routeObj.cameraInfo = NULL;
        routeObj.poiInfo = NULL;

        for (i = 0; i < routeCount; i++)
        {
            NB_RouteInformationSetActiveRoute(routeObj.route, i + 1);
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! This system test is used to test maneuver turn Images

@return none

@see NBI_NavigateSetup
@see NBI_NavigateStart
*/

/*! This system test is used to test maneuver APIS

@return none

@see NBI_NavigateSetup
@see NBI_NavigateStart
*/
void 
TestNavigationManeuvers(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    uint32                    numManeuvers = 0;
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);
    
    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 33.58;
    gpsFix.longitude  = -117.73;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, NULL, NULL, 0);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            numManeuvers = NB_RouteInformationGetManeuverCount(route);
            CU_ASSERT(numManeuvers > 0);

            if (numManeuvers > 0)
            {
                uint32 resultManeuvers = 0;
                uint32 currManeuver = 1;
                nb_boolean bRet = FALSE;

                bRet = NB_RouteInformationIsManeuverSignificant(route, 0);
                CU_ASSERT_EQUAL(bRet, TRUE);
                LOGOUTPUT(LogLevelHigh, ("IsSignificantManeuver = %d\n", bRet));

                resultManeuvers = NB_RouteInformationGetNextSignificantManeuver(route, currManeuver);
                CU_ASSERT_EQUAL(resultManeuvers, currManeuver+1);
                LOGOUTPUT(LogLevelHigh, ("currManeuver: %d, NextSignificantManeuver = %d\n", currManeuver, resultManeuvers));

                resultManeuvers = NB_RouteInformationGetPreviousManeuver(route, currManeuver);
                CU_ASSERT_EQUAL(resultManeuvers, currManeuver-1);
                LOGOUTPUT(LogLevelHigh, ("currManeuver: %d, PreviousSignificantManeuver = %d\n", currManeuver, resultManeuvers));

                currManeuver = 2;    
                resultManeuvers = NB_RouteInformationGetNextManeuver(route, currManeuver);
                CU_ASSERT_EQUAL(resultManeuvers, currManeuver+1);
                LOGOUTPUT(LogLevelHigh, ("currManeuver: %d, NextManeuver = %d\n", currManeuver, resultManeuvers));

                resultManeuvers = NB_RouteInformationGetPreviousManeuver(route, currManeuver);
                CU_ASSERT_EQUAL(resultManeuvers, currManeuver-1);
                LOGOUTPUT(LogLevelHigh, ("currManeuver: %d, PreviousManeuver = %d\n", currManeuver, resultManeuvers));
            }
        }
        else 
        {
            CU_FAIL("No Route Returned");
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! This system test is used to test maneuver text

@return none

@see NBI_NavigateSetup
@see NBI_NavigateStart
*/

/*! Do a navigation based on a CSV file

This system test reads position data from a CSV file and performs the navigation.

@return None. CUnit Asserts get called on failures.
*/
void 
TestNavigationRequestFromFile(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };    
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_Place           destination     = {{0}};
    NB_Place           origin     = {{0}};
    PAL_Error     palError    = PAL_Ok;
    uint32          fileSize    = 0;
    unsigned char*  fileBuffer  = NULL;
    double distance = 0.0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};
    NB_Analytics* analytics = NULL;

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);

    origin.location.type = NB_Location_Address;
    origin.location.latitude = 33.623308;
    origin.location.longitude = -117.608846;

    destination.location.type = NB_Location_Address;
    destination.location.latitude = 36.12040758;
    destination.location.longitude = -115.154888;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        palError = PAL_FileLoadFile(pal, LAS_VEGAS_CSV_FILE, &fileBuffer, &fileSize);
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        CU_ASSERT_PTR_NOT_NULL(fileBuffer);
        CU_ASSERT(fileSize > 0);

        result = NB_AnalyticsCreate(context, NULL, &analytics);
        CU_ASSERT_EQUAL(result, NE_OK);
        (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, &origin, &destination, 0);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        if (result == NE_OK) 
        {
            int     gpsTime     = 0;
            double  latitude    = 0;
            double  longitude   = 0;
            char*   position    = (char*)fileBuffer;    
            int counter = 0;

            result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);

            // For all GPS fixes from the file
            while (GetNextGpsFix(&position, &gpsTime, &latitude, &longitude) && counter <= 50)
            {
                static nb_unixTime timeOffset   = 0;
                int currentManeuver          = 0;
                uint32 maneuverIndex            = 0;
                counter++;
                                
                if (timeOffset == 0)
                {
                    timeOffset = PAL_ClockGetGPSTime() - gpsTime;
                }

                {
                    NB_GpsLocation gpsFix = {0};

                    gpsFix.status   = PAL_Ok;
                    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
                    gpsFix.gpsTime    = gpsTime + timeOffset;
                    gpsFix.latitude     = latitude;
                    gpsFix.longitude    = longitude;
                    
                    LOGOUTPUT(LogLevelHigh, ("GPS Fix: %d, %f, %f\n", gpsFix.gpsTime, gpsFix.latitude, gpsFix.longitude));
                
                    // Update GPS.                    
                    result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }
                                                           
                // We should always get some maneuvers. Otherwise something went wrong.
                result = NB_NavigationGetManeuverProgress(navigation, &currentManeuver);    
                
                CU_ASSERT_EQUAL(result, NE_OK);
                
                {
                   #define _SIZE 1024
                   char primaryManeuver[_SIZE] = {0};
                   char secondaryManeuver[_SIZE] = {0};
                   char currentRoad[_SIZE] = {0};

                   result = NB_RouteInformationGetTurnInformation(route, 
                            currentManeuver, 
                            NULL,
                            primaryManeuver, 
                            _SIZE, 
                            secondaryManeuver, 
                            _SIZE, 
                            currentRoad, 
                            _SIZE, 
                            &distance, 
                            FALSE);

                   if (result != NE_OK)
                   {
                       int b=0;
                       b++;
                   }

                    CU_ASSERT_EQUAL(result, NE_OK);                            
                    LOGOUTPUT(LogLevelHigh, ("Maneuver[%d]: Primary: %s, Secondary: %s, Current Road: %s, Distance: %1.1f\n",
                            maneuverIndex,
                            primaryManeuver,
                            secondaryManeuver,
                            currentRoad,
                            distance)); 
                }
            }
        }

        nsl_free(fileBuffer);
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }


    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

/*! Do a navigation based on a GPS file

This system test reads position data from a GPS file and performs the navigation.

@return None. CUnit Asserts get called on failures.
*/

void 
TestNavigationRequestWithTrafficNotify(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "SNA/John Wayne");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude = 33.6703222;
    destination.location.longitude = -117.85320;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731140, TestNavigationWorkDestinationCB, NULL, 
        "Alicia_To_SNA", "Alicia_To_SNA", NULL);
}

/*! This system test is used for testing the incidences on the route

@return none

@see NBI_NavigateSetup
@see NBI_NavigateStart
*/
void 
TestNavigationRequestIncidents(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };    
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_Place           destination     = {{0}};
    NB_Place           origin     = {{0}};
    PAL_Error     palError    = PAL_Ok;
    uint32          fileSize    = 0;
    unsigned char*  fileBuffer  = NULL;
    GPSFileRecord*      pGPSFileRecord  = NULL;
    uint32 endManeuver = 0;
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);
    
    origin.location.type = NB_Location_Address;
    origin.location.latitude = 33.623308;
    origin.location.longitude = -117.608846;

    destination.location.type = NB_Location_Address;
    destination.location.latitude = 37.807882;
    destination.location.longitude = -122.411932;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        palError = PAL_FileLoadFile(pal, LAS_VEGAS_GPS_FILE, &fileBuffer, &fileSize);
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        CU_ASSERT_PTR_NOT_NULL(fileBuffer);
        CU_ASSERT(fileSize > 0);

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, &origin, &destination, 0);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        if (result == NE_OK) 
        {
            int     index   = 0;                        
            unsigned char*  tempBuffer  = NULL;
            int             nFixes      = 0;

            result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
            CU_ASSERT_EQUAL(result, NE_OK);

            Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);

            if (fileBuffer != NULL && fileSize > 0)
            {
                tempBuffer = fileBuffer; //Store buff in temp variable
                nFixes = fileSize/sizeof(GPSFileRecord);
            }

            // Take only a limit number of fixes to avoid long runs. Can be modified for more thorough testing.
            for(index = 0; (index < GetMaxFixes()) && (index < nFixes); index++) 
            {
                static uint32       counter       = 0;
                NB_GpsLocation     gpsFix        = {0};
                uint32              incidentsCount = 0;
                NB_TrafficState* trafficState = NULL;
                NB_Place incident = {{0}};

                ++counter;

                pGPSFileRecord = (GPSFileRecord*)tempBuffer;

                GPSInfo2GPSFix(pGPSFileRecord, &gpsFix);

                // Update GPS.                    
                result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
                CU_ASSERT_EQUAL(result, NE_OK);

                endManeuver = NB_RouteInformationGetManeuverCount(routeObj.route);

                NB_TrafficInformationGetSummary(routeObj.trafficInfo, routeObj.route, 0,0.0, endManeuver, NULL, &incidentsCount, NULL, NULL);

                if (incidentsCount > 0)
                {
                    result = NB_NavigationGetTrafficState(navigation, &trafficState);
                    CU_ASSERT_EQUAL(result, NE_OK);

                    result = NB_TrafficProcessorGetNextIncident(trafficState, &incident, NULL, NULL, NULL);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }
                          
                // Advance temporary pointer
                tempBuffer += sizeof(GPSFileRecord);    
            }

            CU_ASSERT_EQUAL(result, NE_OK);
        }

        nsl_free(fileBuffer);    

        if (navigation)
        {
            result = NB_NavigationDestroy(navigation);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        result = DestroyRouteObject(&routeObj, TRUE);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_GuidanceInformationDestroy(guidanceInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        //Cleanup pal and context
        DestroyContext(context);
        PAL_DestroyInstance(pal);    

        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
}


static void 
TestNavigationWorkDestinationCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationWorkDestination(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "John Wayne-Orange County Airport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.68129;
    destination.location.longitude = -117.85883;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731140, TestNavigationWorkDestinationCB, NULL, 
        "Alicia to SNA (Car)", "Alicia_To_SNA_Car", NULL);
}

static void 
TestNavigationIrvineLAXCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationIrvineLAX(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "LAX");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.945249;
    destination.location.longitude = -118.400329;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "irvine-to-lax.gps", 100, 943229170, 943234885, TestNavigationIrvineLAXCB, NULL, 
        "Irvine to LAX (Car)", "Irvine_to_LAX_Car", NULL);
}

static void 
TestNavigationLibertyToPaneraCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationLibertyToPanera(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Panera");
    destination.location.type = NB_Location_AddressFreeForm;
    nsl_strcpy(destination.location.freeform, "26419 aliso creek rd, aliso viejo, ca");
    destination.location.latitude  = 33.575635;
    destination.location.longitude = -117.723508;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
                                     "liberty-to-panera.gps", 100, 961869117, 961869713, TestNavigationLibertyToPaneraCB, NULL,
                                     "liberty-to-panera (Car)", "liberty-to-panera_Car", NULL);
}

static void
TestNavigationWrongWayCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationWrongWay(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Sea World");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 32.762927;
    destination.location.longitude = -117.231237;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 0, 793729907, 793731140, TestNavigationWrongWayCB, NULL, 
        "Wrong Way", "WrongWay", NULL);
}

static void 
TestNavigationVectorPrefetchRealTimeCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationVectorPrefetchRealTime()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "KnockAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 53.9146596193;
    destination.location.longitude = -8.81799280643;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "irl_Knock_Airport_fastest.gps", 100, 0, 8588, TestNavigationVectorPrefetchRealTimeCB, 
        NULL, "Ireland Vector Prefetch RealTime", "Ireland_Vector_Prefetch_RealTime", NULL);
}

static void 
TestNavigationTrafficRealTimeCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationTrafficRealTime()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "LAX");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.945580;
    destination.location.longitude = -118.391098;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "worktowardslax.gps", 100, 936680849, 936681822, TestNavigationTrafficRealTimeCB, 
        NULL, "LAX Traffic RealTime", "LAX_Traffic_RealTime", NULL);
}

static void 
TestNavigationSeattleTrafficCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationSeattleTraffic()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Woodinville, WA");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 47.75600433;
    destination.location.longitude = -122.1555099;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "SeattleTraffic.gps", 1000, 971538269, 971540000, TestNavigationSeattleTrafficCB,
        NULL, "Seattle Traffic", "Seattle_Traffic", NULL);
}

static void 
TestNavigationSeattleFloatingBridgeCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationSeattleFloatingBridge(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Kirkland");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 47.679272;
    destination.location.longitude = -122.178438;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "seattle-kirkland.gps", 100, 958231217, 958232605, TestNavigationSeattleFloatingBridgeCB, 
        NULL, "Seattle to Kirkland", "Seattle_to_Kirkland", NULL);
}

static void 
TestNavigationAlisoCreekToOrangeCrushCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationAlisoCreekToOrangeCrush(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Santa Ana");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.775818944;
    destination.location.longitude = -117.872706056;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "alisocreek-orangecrush.gps", 100, 935897487, 935898986, TestNavigationAlisoCreekToOrangeCrushCB, 
        NULL, "Aliso Creek to Orange Crush", "Aliso_Creek_to_Orange_Crush", NULL);
}

static void 
TestNavigationLiberty6ToSNACB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationLiberty6ToSNA(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "SNA");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.67561;
    destination.location.longitude =   -117.863358;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "liberty6ToSNA.gps", 100, 968962296, 968963575, TestNavigationLiberty6ToSNACB,
        NULL, "Liberty 6 to SNA", "Liberty_6_To_SNA", NULL);
}

void TestNavigationLiberty6ToSNASR(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "SNA");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.67561;
    destination.location.longitude = -117.863358;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantNonGuidanceManeuvers = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "liberty6ToSNA.gps", 100, 968962296, 968963575, TestNavigationLiberty6ToSNACB,
        NULL, "Liberty 6 to SNA", "Liberty_6_To_SNA", NULL);
}

static void 
TestNavigationTrafficOutsideRouteWindowCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationTrafficOutsideRouteWindow(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Fremont CA");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 37.555629;
    destination.location.longitude =   -122.051402;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);


    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "liberty6ToSNA.gps", 100, 968962296, 968963575, TestNavigationTrafficOutsideRouteWindowCB,
        NULL, "Traffic Outside Route Windows", "Traffic_Outside_Route_Window", NULL);
}
static void 
TestNavigationInitialGuidanceUnnamedRoadCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationInitialGuidanceUnnamedRoad(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "City Lights");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.572064;
    destination.location.longitude =   -117.726361;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);


    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "InitialGuidanceUnnamedRoad.gps", 1000, 34, 120, TestNavigationInitialGuidanceUnnamedRoadCB,
        NULL, "Initial Announcenment Unnamed Road", "Initial_Announcement_UnnamedRoad", NULL);
}

static void 
TestNavigationInitialGuidanceShortRouteCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationInitialGuidanceShortRoute(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Liberty 65");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.559316;
    destination.location.longitude = -117.729172;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "Liberty6ToLiberty65.gps",1000, 928123242, 0xffffffff, TestNavigationInitialGuidanceShortRouteCB,
        NULL, "Initial Announcenment Short Route", "Initial_Announcement_Short_Route", NULL);
}

static void 
TestNavigationOffRouteRecalcConfirmCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationOffRouteRecalcConfirm(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "NIM");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.55961648;
    destination.location.longitude = -117.7295304;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "recalc-at-del-mar.gps", 100, 967327195, 967327495, TestNavigationOffRouteRecalcConfirmCB, 
        NULL, "Recalc At Del Mar", "Recalc_At_Del_Mar", NULL);
}

static void 
TestNavigationTripDelayCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_TrafficState* trafficState = NULL;
    uint32 tripRemainingTime = 0;
    uint32 tripRemainingDelay = 0;
    double tripRemainingMeter = 0.0;

    NB_Error result = NE_OK;

    result = NB_NavigationGetTrafficState(navigation, &trafficState);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_NavigationGetTripRemainingInformation(navigation,
            &tripRemainingTime, &tripRemainingDelay, &tripRemainingMeter);
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelMedium, ("trip remain time: %u, delay: %u, meter : %f\n", tripRemainingTime, tripRemainingDelay, tripRemainingMeter));
}


void 
TestNavigationTripDelay()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "SNA");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.675609;
    destination.location.longitude = -117.86336;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Warn;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "liberty6.gps", 10, 957241993, 957242686, TestNavigationTripDelayCB,
        NULL, "NIM to SNA trip delay", "NIM_TO_SNA_TRIP_DELAY", NULL);
}

void 
TestNavigationLaCostaToBestBuy()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Best Buy");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.066807;
    destination.location.longitude = -117.262404;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Warn;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "lacosta_to_bestbuy.gps", 500, 962069109, 962069409, TestNavigationTripDelayCB,
        NULL, "La Costa To Best Buy delay", "LA_COSTA_TO_BEST_BUY_DELAY", NULL);
}

void
TestNavigationTrafficDisabled()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Best Buy");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.066807;
    destination.location.longitude = -117.262404;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "lacosta_to_bestbuy.gps", 500, 962069109, 962069409, TestNavigationTripDelayCB,
        NULL, "Traffic Disabled", "TRAFFIC_DISABLED", NULL);
}

static void 
TestNavigationTrafficManualPollingCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationTrafficManualPolling(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    TestNavigationSessionOptions options = {0};
    options.manualTrafficPollIntervalMinutes = 1;

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "John Wayne-Orange County Airport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.68129;
    destination.location.longitude = -117.85883;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731140, TestNavigationTrafficManualPollingCB, NULL, 
        "Work to SNA (Manual Traffic)", "Work_To_SNA_Manual_Traffic", &options);
}

static void 
TestNavigationStartupCaseToRoadCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    /* These points have been validated to exit startup on the 3rd point to match the road with a valid heading - timestamp 928123281 */

    /* Startup Case*/
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 928123240, 928123280, NB_NS_NavigatingStartup);
    
    /* Arriving */
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 928123281, 928123317, NB_NS_NavigatingArriving);

    /* Arriving */
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 928123318, 928123325, NB_NS_Arrived);
}

static void 
TestNavigationStartupCaseToRoad(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Point on Liberty");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.5587853193;
    destination.location.longitude = -117.726316452;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "startup_to_road.gps", 0, 928123240, 928123325, TestNavigationStartupCaseToRoadCB, NULL, 
        "Startup Case to Road (Car)", "Startup_To_Road_Car", NULL);
}


static void 
TestNavigationKfcToWorkCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 933968335, 933968501, NB_NS_Navigating);
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 933968505, 933968525, NB_NS_NavigatingStartup);
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 933968526, 933968866, NB_NS_Navigating);
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 933968867, 933968868, NB_NS_NavigatingArriving);
}

static void 
TestNavigationKfcToWork(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "6 Liberty");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.55961648;
    destination.location.longitude = -117.7295304;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    /*
        This test is from the KFC at Alicia and Crown Valley to work.  The origin fix
        is on the side road, making the first maneuver a u-turn to get back to Alicia.
        Was actually in the parking lot and able to turn left onto the side road, making
        the u-turn unnecessary and getting right onto the route
    */
    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "kfctowork.gps", 0, 933968318, 933968868, TestNavigationKfcToWorkCB, NULL, 
        "KFC to Work", "KFC_to_Work", NULL);
}


static void 
TestNavigationSwedenFerriesCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    /* First Ferry - These time stamps could be fine tuned to the exact start/end points */
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884004409, 884004866, NB_NS_NavigatingFerry);
    
	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884004982, 884005103, NB_NS_Navigating);

	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005104, 884005104, NB_NS_UpdatingRoute);

	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005105, 884005449, NB_NS_Navigating);

	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005450, 884005450, NB_NS_UpdatingRoute);

	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005451, 884005637, NB_NS_Navigating);

	Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005638, 884005638, NB_NS_UpdatingRoute);

    /* Rest of Route - These time stamps could be fine tuned to the exact start/end points */
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 884005639, 884005899, NB_NS_Navigating);
}

static void
TestNavigationSwedenFerries(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Unknown Swedish Location");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  =  59.338789;
    destination.location.longitude =  17.937631;

    routeOptions.type = NB_RouteType_Easiest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "ferry.gps", 0, 884004023, 884005890, TestNavigationSwedenFerriesCB, NULL,
        "Sweden Ferry", "Sweden_Ferry", NULL);
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

static void 
TestNavigationUKCamerasSR(void)
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
    routeConfiguration.wantNonGuidanceManeuvers = TRUE;
    routeConfiguration.wantSpeedCameras = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "32GallowHillLane_AbbotsLangley.gps", 100, 908814147, 908814948, TestNavigationUKCamerasCB, NULL,
        "UK Speed Cameras", "UK_Cameras", &sessionOptions);
}

static void
TestNavigationFastPOILowesCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationFastPOILowes(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions sessionOptions = { TRUE, FALSE };

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Lowes");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.5780221224;
    destination.location.longitude = -117.722851038;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "FastPOI_NIM_Lowes.gps", 100, 0, 148, TestNavigationFastPOILowesCB, NULL, 
        "Fast POI - Lowes", "Fast_POI_Lowes", &sessionOptions);
}

static void 
TestNavigationFastPOI5FwyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationFastPOI5Fwy(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions sessionOptions = { TRUE, FALSE };

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "5 Fwy");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.6037498713;
    destination.location.longitude = -117.690632343;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "FastPOI_NIM_Alicia5Fwy.gps", 100, 0, 366, TestNavigationFastPOI5FwyCB, NULL, 
        "Fast POI - 5 Fwy", "Fast_POI_5 Fwy", &sessionOptions);
}

static void 
TestNavigationFastPOILasVegasCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void TestNavigationFastPOILasVegas(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions sessionOptions = { TRUE, FALSE };

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Ontario");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 34.04800415;
    destination.location.longitude = -117.547853;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "LasVegas.gps", 100, 805398232, 805399459, TestNavigationFastPOILasVegasCB, NULL, 
        "Fast POI - Las Vegas", "Fast_POI_Las Vegas", &sessionOptions);
}

static void 
TestNavigationBugCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationConfusedHeading(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Bug 51477");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.945249;
    destination.location.longitude = -118.400329;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "LagunaNiguel-RSM-MV.gps", 100, 940860009, 940860876, TestNavigationBugCB, NULL, 
        "Confused Heading - 51477", "ConfusedHeading_51477", NULL);
}

static void 
TestNavigationStackedManeuvers(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Bug 52308");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.6299443245;
    destination.location.longitude = -117.597908378;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "Oso-241-Antonio.gps", 100, 941469446, 941469796, TestNavigationBugCB, NULL, 
        "StackedManeuvers - 52308", "StackedManeuvers_52308", NULL);
}

static NB_Error
RequestDetour(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_DetourParameters* detourParameters, const char* detourRouteName, const char* detourKMLFileName, uint32 routeColor, nb_boolean expectNoDetour)
{
    NB_Error error = NE_OK;
    NB_RouteParameters* parameters = NULL;
    NB_GpsHistory* history = NULL;
    NB_RouteInformation* route = NULL;
    NB_TrafficInformation* traffic = NULL;
    NB_RequestHandlerCallback callbacks = { 0 };    
    TestRouteObj    routeObj = {0};

    callbacks.callback = DownloadRouteCallback;
    callbacks.callbackData = &routeObj;

    history = NB_ContextGetGpsHistory(context);
    CU_ASSERT_PTR_NOT_NULL(history);

    error = error ? error : NB_NavigationGetInformation(navigation, &route, &traffic, NULL, NULL);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_RouteParametersCreateDetour(context, route, traffic, detourParameters, history, &parameters);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_RouteHandlerCreate(context, &callbacks, &routeObj.routeHandler);
    CU_ASSERT_EQUAL(error, NE_OK);

    if (NULL != parameters && NULL != routeObj.routeHandler)
    {
        ResetCallbackCompletedEvent(g_CallbackEvent);
        
        error = NB_RouteHandlerStartRequest(routeObj.routeHandler, parameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        if (NE_OK == error)
        {                
            // Wait for event here from callback. Timeout 1 min.
            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
            {
            }
        }
        else
        {
            LOGOUTPUT(LogLevelHigh, ("RequestDetour: Failure\n"));
        }
    }

    if (expectNoDetour)
    {
        NB_NavigateRouteError routeError = NB_NRE_None;
        if (NB_RouteHandlerGetRouteError(routeObj.routeHandler, &routeError) == NE_OK && routeError == NB_NRE_None)
        {
            CU_FAIL("Detour route returned -- expected detour request to fail.")
        }
    }

    else if (routeObj.route)
    {   
        if (!NB_RouteInformationIsRouteComplete(routeObj.route))
        {
            CU_FAIL("Detour route incomplete.");
        }

        Test_ValidateRouteInformation(pal, context, routeObj.route, guidance, navigation);

        Test_DumpRouteToKML(pal, context, routeObj.route, guidance, NULL, NULL, NULL, NULL, detourRouteName, detourKMLFileName, routeColor);

        error = NB_NavigationNewRoute(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo);
        CU_ASSERT_EQUAL(error, NE_OK);

        if (error == NE_OK)
        {
            ClearInformationRouteObject(&routeObj);
        }
    }
    else
    {
        CU_FAIL("No detour route received.");
    }

    DestroyRouteObject(&routeObj, FALSE);

    if (parameters)
    {
        (void)NB_RouteParametersDestroy(parameters);
    }

    return error;
}


static NB_Error
RequestFullRouteDetour(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance)
{
    NB_Error error = NE_OK;
    NB_DetourParameters* detourParameters = NULL;

    error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_DetourParametersAddAvoidEntireRoute(detourParameters, "Entire Route");

    error = RequestDetour(pal, context, navigation, guidance, detourParameters, "Work Destination Detour - Full Route Detour", "Work_Destination_Detour_Full_Route.kml", 0x7fff0000, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    if (detourParameters)
    {
        (void)NB_DetourParametersDestroy(detourParameters);
    }

    return error;
}

static NB_Error
RequestClearDetour(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance)
{
    NB_Error error = NE_OK;

    error = RequestDetour(pal, context, navigation, guidance, NULL, "Work Destination Detour - Clear Detour", "Work_Destination_Detour_Clear.kml", 0x7f00ff00, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    return error;
}

static NB_Error
RequestDetourByDistance(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, double start, double length)
{
    NB_Error error = NE_OK;
    NB_DetourParameters* detourParameters = NULL;

    error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_DetourParametersAddAvoidPartOfRoute(detourParameters, "Distance", start, length);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = RequestDetour(pal, context, navigation, guidance, detourParameters, "Work Destination Detour - Distance Detour", "Work_Destination_Detour_Distance.kml", 0x7f0000ff, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    if (detourParameters)
    {
        (void)NB_DetourParametersDestroy(detourParameters);
    }

    return error;
}

static NB_Error
RequestDetourByManeuver(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, uint32 startIndex, uint32 endIndex)
{
    NB_Error error = NE_OK;
    NB_DetourParameters* detourParameters = NULL;

    error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_DetourParametersAddAvoidManeuvers(detourParameters, "Maneuvers", startIndex, endIndex);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = RequestDetour(pal, context, navigation, guidance, detourParameters, "Work Destination Detour - Maneuvers Detour", "Work_Destination_Detour_Maneuvers.kml", 0x7f0000ff, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    if (detourParameters)
    {
        (void)NB_DetourParametersDestroy(detourParameters);
    }

    return error;
}

static NB_Error
RequestDetourByTrafficIncident(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, uint32 index)
{
    NB_Error error = NE_OK;
    NB_DetourParameters* detourParameters = NULL;

    error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : NB_DetourParametersAddAvoidTrafficIncident(detourParameters, "Traffic Incident", index);
    CU_ASSERT_EQUAL(error, NE_OK);

    error = RequestDetour(pal, context, navigation, guidance, detourParameters, "Work Destination Detour - Incident Detour", "Work_Destination_Detour_Incident.kml", 0x7f0000ff, FALSE);
    CU_ASSERT_EQUAL(error, NE_OK);

    if (detourParameters)
    {
        (void)NB_DetourParametersDestroy(detourParameters);
    }

    return error;
}

static void 
TestNavigationWorkDestinationDetourCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error error = NE_OK;

    if (gpsFix->gpsTime == 793729950)
    {
        error = RequestFullRouteDetour(pal, context, navigation, guidance);
        LOGOUTPUT(LogLevelHigh, ("request full router detour, error = %d\n", error));
        CU_ASSERT_EQUAL(error, NE_OK);
    }
    else if (gpsFix->gpsTime == 793730321)
    {
        error = RequestClearDetour(pal, context, navigation, guidance);        
        LOGOUTPUT(LogLevelHigh, ("request clear detour, error = %d\n", error));
        CU_ASSERT_EQUAL(error, NE_OK);
    }
    else if (gpsFix->gpsTime == 793730553) 
    {
        NB_NavigationPublicState state = {{0}};

        error = error ? error : NB_NavigationGetPublicState(navigation, &state);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = error ? error : RequestDetourByDistance(pal, context, navigation, guidance, state.routeLength - state.tripRemainDistance + 1000.0, 1500.0);
        LOGOUTPUT(LogLevelHigh, ("request detour by distance, error = %d\n", error));
        CU_ASSERT_EQUAL(error, NE_OK);
    }
    else if (gpsFix->gpsTime == 793730840)
    {
        error = RequestDetourByManeuver(pal, context, navigation, guidance, 1, 3);
        LOGOUTPUT(LogLevelHigh, ("request detour by maneuver, error = %d\n", error));
        CU_ASSERT_EQUAL(error, NE_OK);
    }
}

static void 
TestNavigationWorkDestinationDetour(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "John Wayne-Orange County Airport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.68129;
    destination.location.longitude = -117.85883;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731133, TestNavigationWorkDestinationDetourCB,
        NULL, "Alicia to SNA (Car) Detour", "Alicia_To_SNA_Car_Detour", NULL);
}

static void 
TestNavigationWorkDestinationArrivalCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 793730988, 793731102, NB_NS_Navigating);
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 793731104, 793731120, NB_NS_NavigatingArriving);
    Test_ValidateNavigationStatus(navigation, gpsFix->gpsTime, 793731122, 793731140, NB_NS_Arrived);
}

static void 
TestNavigationWorkDestinationArrival(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Near Old NIM Office -- Bardeen");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.670708537;
    destination.location.longitude =  -117.85368919;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 0, 793730988, 793731140, TestNavigationWorkDestinationArrivalCB, 
        NULL, "Arrival At Bardeen", "Arrival_At_Bardeen", NULL);
}

static void 
TestNavigationRemainInStartupCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error error = NE_OK;

    if (gpsFix->gpsTime == 966013593)
    {
        NB_DetourParameters* detourParameters = NULL;
        NB_TrafficEvent* trafficEvents = NULL;
        uint32 trafficEventCount = 0;
        uint32 index = 0;

        error = error ? error : NB_NavigationGetUpcomingTrafficEvents(navigation, &trafficEventCount, &trafficEvents);
        CU_ASSERT_EQUAL(error, NE_OK);

        for (index = 0; index < trafficEventCount; index++)
        {
            if (trafficEvents[index].type == NB_TET_Incident)
            {
                error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
                CU_ASSERT_EQUAL(error, NE_OK);

                error = error ? error : NB_DetourParametersAddAvoidTrafficEvent(detourParameters, "Traffic Incident Event", index);
                CU_ASSERT_EQUAL(error, NE_OK);

                error = RequestDetour(pal, context, navigation, guidance, detourParameters, "Remain In Startup - Detour Incident Event", "Remain_In_Startup_Detour_Incident_Event.kml", 0x7f0000ff, FALSE);
                CU_ASSERT_EQUAL(error, NE_OK);

                if (detourParameters)
                {
                    (void)NB_DetourParametersDestroy(detourParameters);
                }
                break;
            }
        }
        if (trafficEvents)
        {
            NB_NavigationFreeTrafficEvents(trafficEvents);
        }
    }
}

static void 
TestNavigationRemainInStartup(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Holiday Inn");
    destination.location.type = NB_Location_Address;
    destination.location.latitude  = 33.626552;
    destination.location.longitude = -117.716521;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "startup_only.gps", 100, 966013276, 966014618, TestNavigationRemainInStartupCB, NULL, 
        "Remain In Startup", "Remain_In_Startup", NULL);
}

static void 
TestNavigationNoDetourCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    NB_Error error = NE_OK;

    if (gpsFix->gpsTime == 935426031)
    {
        NB_DetourParameters* detourParameters = NULL;

        error = error ? error : NB_DetourParametersCreate(context, &detourParameters);
        CU_ASSERT_EQUAL(error, NE_OK);

        error = error ? error : NB_DetourParametersAddAvoidEntireRoute(detourParameters, "Entire Route");
        // error = error ? error : NB_DetourParametersAddAvoidCongestion(detourParameters, "Avoid Congestion");
        CU_ASSERT_EQUAL(error, NE_OK);

        error = RequestDetour(pal, context, navigation, guidance, detourParameters, "PCH No Detour - Congestion", "PCH_No_Detour_Congestion.kml", 0x7f0000ff, TRUE);
        CU_ASSERT_EQUAL(error, NE_OK);

        if (detourParameters)
        {
            (void)NB_DetourParametersDestroy(detourParameters);
        }
    }
}

static void 
TestNavigationNoDetour(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Topanga Canyon");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 34.0393996239;
    destination.location.longitude = -118.582177162;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "PCH-no-detour.gps", 100, 935425904, 935426254, TestNavigationNoDetourCB,
        NULL, "PCH No Detour", "PCH_No_Detour", NULL);
}

// Peter-Welter-Platz Germany.gps
static void 
TestNavigationGermanyTunnelCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationGermanyTunnel(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Peter-Welter-Platz Köln Germany");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 50.933990;
    destination.location.longitude =  6.96179688;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    // Note - the original file name for this GPS file contains an "ö" character.  Since this
    // code file is saved with the windows-1252 code page, it does not work properly
    // I have changed it to a normal "o" to work around.
    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "Peter-Welter-Platz Koln Germany.gps", 0, 910513103, 910516421, TestNavigationGermanyTunnelCB, 
        NULL, "Peter-Welter-Platz Köln Germany", "Peter-Welter-Platz_Köln_Germany", NULL);
}

static void
TestNavigationStaticRoutingCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void TestNavigationStaticRouting(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));

    destination.location.type = NB_Location_LatLon;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV | NB_RouteAvoid_Toll;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantNonGuidanceManeuvers = TRUE;

    nsl_strcpy(destination.name, "Niagara Falls");
    destination.location.latitude  = 43.0897906;
    destination.location.longitude = -79.0620937;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "Canada_USA1.gps", 0, 0, 100000, TestNavigationStaticRoutingCB,
        NULL, "Canada_USA1", "Canada_USA1", NULL);

    nsl_strcpy(destination.name, "Togos");
    destination.location.latitude  = 33.57083;
    destination.location.longitude = -117.71436;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "tcs_togos.gps", 0, 0, 100000, TestNavigationStaticRoutingCB,
        NULL, "Tcs_to_Togos", "Tcs_to_Togos", NULL);

    nsl_strcpy(destination.name, "Coyote Grill");
    destination.location.latitude  = 33.50317;
    destination.location.longitude = -117.74494;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "tcs_coyotegrill.gps", 0, 0, 100000, TestNavigationStaticRoutingCB,
        NULL, "Tcs_to_Coyote_Grill", "Tcs_to_Coyote_Grill", NULL);
    
    nsl_strcpy(destination.name, "SNA");
    destination.location.latitude  = 33.67561;
    destination.location.longitude =   -117.863358;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "alisocreek-orangecrush.gps", 0, 935897487, 935898986, TestNavigationStaticRoutingCB,
        NULL, "Aliso Creek to Orange Crush", "Aliso_Creek_to_Orange_Crush", NULL);
}

/*****************************************************************************/

//-------------------------------------------------------------------------
// Test by country: Car
//-------------------------------------------------------------------------


static void 
TestNavigationRouteCarAustriaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void
TestNavigationRouteCarAustria()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "SparkasseBank");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 47.8119689226;
    destination.location.longitude =  13.0045884848;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "austria_SparkasseBank_car.gps", 0, 0, 1608, TestNavigationRouteCarAustriaCB, 
        NULL, "Arrival At SparkasseBank", "Arrival_At_SparkasseBank", NULL);
}


static void 
TestNavigationRouteCarBrusselsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarBrussels()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "BRU");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 50.8967024088;
    destination.location.longitude =  4.48391854763;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "belgium_BRU_fastest.gps", 0, 0, 870, TestNavigationRouteCarBrusselsCB, 
        NULL, "Arrival At BRU", "Arrival_At_BRU", NULL);
}


static void 
TestNavigationRouteCarBrazilCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarBrazil()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Supermercado Brazil");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = -25.4880720377;
    destination.location.longitude = -49.2690736055;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "brazil_Supermercado_Borssato_shortest.gps", 0, 0, 604, TestNavigationRouteCarBrazilCB, 
        NULL, "Arrival At Supermercado", "Arrival_At_Supermercado", NULL);
}


static void 
TestNavigationRouteCarCanadaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarCanada()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "BMO");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 50.1443696022;
    destination.location.longitude = -96.8708056211;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "canada_BMO_fastest.gps", 0, 0, 2670, TestNavigationRouteCarCanadaCB, 
        NULL, "Arrival At BMO", "Arrival_At_BMO", NULL);
}


static void 
TestNavigationRouteCarSwitzerlandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarSwitzerland()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Migrol Tankstelle");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 46.9589352608;
    destination.location.longitude = 7.45595097542;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "che_Migrol_Tankstelle_fastest.gps", 0, 0, 984, TestNavigationRouteCarSwitzerlandCB, 
        NULL, "Arrival At Migrol", "Arrival_At_Migrol", NULL);
}


static void 
TestNavigationRouteCarGermanyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarGermany()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Commerzbank");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.430781126;
    destination.location.longitude = 13.752399087;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "deu_Commerzbank_shortest.gps", 0, 0, 2256, TestNavigationRouteCarGermanyCB, 
        NULL, "Arrival At Commerzbank", "Arrival_At_Commerzbank", NULL);
}


static void 
TestNavigationRouteCarDenmarkCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarDenmark()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Statoil");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 55.6177443266;
    destination.location.longitude = 8.48451912403;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "dnk_Statoil_shortest.gps", 0, 0, 1236, TestNavigationRouteCarDenmarkCB, 
        NULL, "Arrival At Statoil", "Arrival_At_Statoil", NULL);
}


static void 
TestNavigationRouteCarSpainCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarSpain()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "BancoPopular");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 38.3788007498;
    destination.location.longitude = -0.512838363647;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "esp_Banco_Popular_fastest.gps", 0, 0, 2606, TestNavigationRouteCarSpainCB, 
        NULL, "Arrival At BancoPopular", "Arrival_At_BancoPopular", NULL);
}


static void 
TestNavigationRouteCarFinlandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarFinland()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "HelsinkiAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 60.2966701984;
    destination.location.longitude = 24.9592316151;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "fin_Helsinki_Aiport_fastest.gps", 0, 0, 1134, TestNavigationRouteCarFinlandCB, 
        NULL, "Arrival At HelsinkiAirport", "Arrival_At_HelsinkiAirport", NULL);
}

static void 
TestNavigationRouteCarFranceCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarFrance()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "PetitCasino");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 43.2375043631;
    destination.location.longitude = 6.07182383537;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "fra_Petit_Casino_fastest.gps", 0, 0, 1058, TestNavigationRouteCarFranceCB, 
        NULL, "Arrival At PetitCasino", "Arrival_At_PetitCasino", NULL);
}

static void 
TestNavigationRouteCarBritainCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarBritain()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "PrestatynHospital");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 53.3261615038;
    destination.location.longitude = -3.40192079544;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "gbr_Prestatyn_Hospital_fastest.gps", 0, 0, 2332, TestNavigationRouteCarBritainCB, 
        NULL, "Arrival At PrestatynHospital", "Arrival_At_PrestatynHospital", NULL);
}

void
TestNavigationRouteCarBritainSR()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "PrestatynHospital");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 53.3261615038;
    destination.location.longitude = -3.40192079544;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantNonGuidanceManeuvers = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "gbr_Prestatyn_Hospital_fastest.gps", 0, 0, 2332, TestNavigationRouteCarBritainCB,
        NULL, "Arrival At PrestatynHospital", "Arrival_At_PrestatynHospital", NULL);
}

static void 
TestNavigationRouteCarGreeceCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarGreece()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "TatoiAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 38.100913167;
    destination.location.longitude = 23.7769031525;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "grc_Tatoi_Airport_fastest.gps", 0, 0, 1642, TestNavigationRouteCarGreeceCB, 
        NULL, "Arrival At TatoiAirport", "Arrival_At_TatoiAirport", NULL);
}

static void 
TestNavigationRouteCarCroatiaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarCroatia()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "ZagrebAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 45.1535886526;
    destination.location.longitude = 15.2722996473;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "hrv_Zagreb_airport_fastest.gps", 0, 0, 6336, TestNavigationRouteCarCroatiaCB, 
        NULL, "Arrival At ZagrebAirport", "Arrival_At_ZagrebAirport", NULL);
}

static void 
TestNavigationRouteCarIrelandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarIreland()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "KnockAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 53.9146596193;
    destination.location.longitude = -8.81799280643;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "irl_Knock_Airport_fastest.gps", 0, 0, 8588, TestNavigationRouteCarIrelandCB, 
        NULL, "Arrival At KnockAirport", "Arrival_At_KnockAirport", NULL);
}

static void 
TestNavigationRouteCarItalyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarItaly()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "IstitutoMarta");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 44.4158685207;
    destination.location.longitude = 8.93986165524;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "ita_Istituto_Marta_shortest.gps", 0, 0, 346, TestNavigationRouteCarItalyCB, 
        NULL, "Arrival At IstitutoMarta", "Arrival_At_IstitutoMarta", NULL);
}


static void 
TestNavigationRouteCarLuxembourgCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarLuxembourg()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "NoertrangeAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 49.9685293436;
    destination.location.longitude = 5.92387855053;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "lux_Noertrange_Airport_fastest.gps", 0, 0, 3654, TestNavigationRouteCarLuxembourgCB, 
        NULL, "Arrival At NoertrangeAirport", "Arrival_At_NoertrangeAirport", NULL);
}

static void 
TestNavigationRouteCarMexicoCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarMexico()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "CiudadJuarez");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 31.6364085674;
    destination.location.longitude = -106.436421275;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "mex_Ciudad_Juarez_Int_fastest.gps", 0, 0, 1278, TestNavigationRouteCarMexicoCB, 
        NULL, "Arrival At CiudadJuarez", "Arrival_At_CiudadJuarez", NULL);
}

static void 
TestNavigationRouteCarNetherlandsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarNetherlands()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "CafeRoba");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.3593378067;
    destination.location.longitude = 4.91410195827;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "nld_Cafe_Roba_fastest.gps", 0, 0, 728, TestNavigationRouteCarNetherlandsCB, 
        NULL, "Arrival At CafeRoba", "Arrival_At_CafeRoba", NULL);
}


static void 
TestNavigationRouteCarNorwayCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarNorway()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "StatoilService");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 68.4595012665;
    destination.location.longitude = 15.0828391314;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "nor_Statoil_Service_shortest.gps", 0, 0, 5944, TestNavigationRouteCarNorwayCB, 
        NULL, "Arrival At StatoilService", "Arrival_At_StatoilService", NULL);
}

static void 
TestNavigationRouteCarPolandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarPoland()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "BarPrzyRynku");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 54.4128102064;
    destination.location.longitude = 18.5909217596;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "pol_Bar_przy_Rynku_fastest.gps", 0, 0, 1076, TestNavigationRouteCarPolandCB, 
        NULL, "Arrival At BarPrzyRynku", "Arrival_At_BarPrzyRynku", NULL);
}


static void 
TestNavigationRouteCarPortugalCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarPortugal()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "LisboaAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 39.8379224539;
    destination.location.longitude = -8.7207198143;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "prt_Lisboa_Airport_fastest.gps", 0, 0, 5676, TestNavigationRouteCarPortugalCB, 
        NULL, "Arrival At LisboaAirport", "Arrival_At_LisboaAirport", NULL);
}

static void
TestNavigationRouteCarRomaniaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarRomania()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "ArtMuseum");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 45.6517231464;
    destination.location.longitude = 25.6110996008;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "rou_Art_Museum_shortest.gps", 0, 0, 5766, TestNavigationRouteCarRomaniaCB, 
        NULL, "Arrival At ArtMuseum", "Arrival_At_ArtMuseum", NULL);
}


static void 
TestNavigationRouteCarSwedenCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarSweden()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "GulaboSkola");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 56.4535689354;
    destination.location.longitude = 15.8169221878;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "swe_Gullabo_Skola_fastest.gps", 0, 0, 2862, TestNavigationRouteCarSwedenCB, 
        NULL, "Arrival At GulaboSkola", "Arrival_At_GulaboSkola", NULL);
}

static void 
TestNavigationRouteCarTurkeyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarTurkey()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "AntalyaAirport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 36.9066381454;
    destination.location.longitude = 30.8005142212;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "tur_Antalya_Airport_fastest.gps", 0, 0, 1610, TestNavigationRouteCarTurkeyCB, 
        NULL, "Arrival At AntalyaAirport", "Arrival_At_AntalyaAirport", NULL);
}


static void 
TestNavigationRouteCarCzechRepublicCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarCzechRepublic()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "RozvojvaBanka");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 49.8369562626;
    destination.location.longitude = 18.2908523083;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "cze_RozvojvaBanka.gps", 0, 0, 684, TestNavigationRouteCarCzechRepublicCB, 
        NULL, "Arrival At RozvojvaBanka", "Arrival_At_RozvojvaBanka", NULL);
}


static void 
TestNavigationRouteCarEstoniaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}


void 
TestNavigationRouteCarEstonia()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "RoccaAlMareMuseum");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 59.4313949347;
    destination.location.longitude = 24.636400938;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "est_Rocca_al_Mare_Museum.gps", 0, 0, 948, TestNavigationRouteCarEstoniaCB, 
        NULL, "Arrival At RoccaAlMareMuseum", "Arrival_At_RoccaAlMareMuseum", NULL);
}


static void 
TestNavigationRouteCarHungaryCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void 
TestNavigationRouteCarHungary()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Budo Sport Budapest");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 47.500788;
    destination.location.longitude = 19.069210;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "hun_Budo-SportBudapestHungary_fastest.gps", 0, 0, 482, TestNavigationRouteCarHungaryCB, 
        NULL, "Arrival At Budo Sport Budapest", "Arrival_At_BudoSportBudapest", NULL);
}


static void 
TestNavigationRouteCarLatviaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void 
TestNavigationRouteCarLatvia()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Liepaja");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 56.513635;
    destination.location.longitude = 21.014586;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "lat_fastest_Liepaja.gps", 0, 0, 11352, TestNavigationRouteCarLatviaCB, 
        NULL, "Arrival At Liepaja", "Arrival_At_Liepaja", NULL);
}


static void 
TestNavigationRouteCarLithuaniaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void 
TestNavigationRouteCarLithuania()
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "War Museum");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 54.899902;
    destination.location.longitude = 23.912692;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "lat_fastest_Liepaja.gps", 0, 0, 1182, TestNavigationRouteCarLithuaniaCB, 
        NULL, "Arrival At War Museum", "Arrival_At_WarMuseum", NULL);
}



//-------------------------------------------------------------------------
// Begin - Test by country: Pedestrian
//-------------------------------------------------------------------------

static void 
TestNavigationPedestrianUSACB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianUSA(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};
    TestNavigationSessionOptions    testOptions = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Walgreens Pharmacy");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.6127406359;
    destination.location.longitude = -117.712760568;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    testOptions.showPlaces = TRUE;
    
    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_USA.gps", 0, 0, 3172, TestNavigationPedestrianUSACB, 
    NULL, "Arrival At Walgreens", "Arrival_At_Walgreens", &testOptions);
}


static void 
TestNavigationPedestrianAustriaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianAustria(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Cafe Schonbrunn");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 48.2332098484;
    destination.location.longitude = 16.3533693552;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_AUT.gps", 0, 0, 2652, TestNavigationPedestrianAustriaCB, 
    NULL, "Arrival At CafeSchonbrunn", "Arrival_At_CafeSchonbrunn", NULL);
}



static void 
TestNavigationPedestrianBrusselsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianBrussels(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Citibank");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 50.873131156;
    destination.location.longitude = 4.3255931139;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_BEL.gps", 0, 0, 3946, TestNavigationPedestrianBrusselsCB, 
    NULL, "Arrival At Citibank", "Arrival_At_Citibank", NULL);
}


static void 
TestNavigationPedestrianBrazilCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianBrazil(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Clinica Medica");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = -15.7960438728;
    destination.location.longitude = -48.1207931042;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_BRA.gps", 0, 0, 1512, TestNavigationPedestrianBrazilCB, 
    NULL, "Arrival At Clinica Medica", "Arrival_At_ClinicaMedica", NULL);
}

static void 
TestNavigationPedestrianCanadaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianCanada(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "MacEwen Petroleum");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 45.4101204872;
    destination.location.longitude = -75.6924533844;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_CAN.gps", 0, 0, 8340, TestNavigationPedestrianCanadaCB, 
    NULL, "Arrival At MacEwen Petroleum", "Arrival_At_MacEwenPetroleum", NULL);
}


static void 
TestNavigationPedestrianSwitzerlandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianSwitzerland(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Klinik");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 46.9602549076;
    destination.location.longitude = 7.43748128414;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_CHE.gps", 0, 0, 3612, TestNavigationPedestrianSwitzerlandCB, 
    NULL, "Arrival At Klinik", "Arrival_At_Klinik", NULL);
}


static void 
TestNavigationPedestrianCzechRepublicCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianCzechRepublic(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Palladium");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 50.0891000032;
    destination.location.longitude = 14.4283479452;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_CZE.gps", 0, 0, 1880, TestNavigationPedestrianCzechRepublicCB, 
    NULL, "Arrival At Palladium", "Arrival_At_Palladium", NULL);
}

static void 
TestNavigationPedestrianGermanyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianGermany(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Danckelmannstr");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.5108987093;
    destination.location.longitude = 13.2973515987;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_DEU.gps", 0, 0, 3386, TestNavigationPedestrianGermanyCB, 
    NULL, "Arrival At Danckelmannstr", "Arrival_At_Danckelmannstr", NULL);
}


static void 
TestNavigationPedestrianDenmarkCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianDenmark(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Christensen");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 55.686762929;
    destination.location.longitude = 12.505890727;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_DNK.gps", 0, 0, 3134, TestNavigationPedestrianDenmarkCB, 
    NULL, "Arrival At Christensen", "Arrival_At_Christensen", NULL);
}


static void 
TestNavigationPedestrianSpainCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianSpain(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Galan Ruiz");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 40.4363393784;
    destination.location.longitude = -3.71212899685;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_ESP.gps", 0, 0, 1910, TestNavigationPedestrianSpainCB, 
    NULL, "Arrival At Galan Ruiz", "Arrival_At_GalanRuiz", NULL);
}


static void 
TestNavigationPedestrianFranceCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianFrance(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Gt Il Fiore");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 48.8794398308;
    destination.location.longitude = 2.34331190586;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_FRA.gps", 0, 0, 3032, TestNavigationPedestrianFranceCB, 
    NULL, "Arrival At Gt Il Fiore", "Arrival_At_GtIlFiore", NULL);
}


static void 
TestNavigationPedestrianItalyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianItaly(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Viale");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 41.908839941;
    destination.location.longitude = 12.497189641;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_ITA.gps", 0, 0, 1638, TestNavigationPedestrianItalyCB,
    NULL, "Arrival At Viale", "Arrival_At_Viale", NULL);
}


static void 
TestNavigationPedestrianNetherlandsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianNetherlands(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Bilderdijk Apotheek");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.3681890965;
    destination.location.longitude = 4.87143874168;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_NLD.gps", 0, 0, 2840, TestNavigationPedestrianNetherlandsCB,
    NULL, "Arrival At Bilderdijk Apotheek", "Arrival_At_BilderdijkApotheek", NULL);
}


static void 
TestNavigationPedestrianNorwayCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianNorway(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Radisson");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 59.9489861727;
    destination.location.longitude = 10.7674330473;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_NOR.gps", 0, 0, 3722, TestNavigationPedestrianNorwayCB,
    NULL, "Arrival At Radisson", "Arrival_At_Radisson", NULL);
}


static void 
TestNavigationPedestrianPolandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianPoland(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Invest-Bank");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.2688025236;
    destination.location.longitude = 20.9828299284;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_POL.gps", 0, 0, 6052, TestNavigationPedestrianPolandCB,
    NULL, "Arrival At Invest-Bank", "Arrival_At_Invest_Bank", NULL);
}


static void 
TestNavigationPedestrianPortugalCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianPortugal(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Farmacia");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 52.2688025236;
    destination.location.longitude = 20.9828299284;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_PRT.gps", 0, 0, 6052, TestNavigationPedestrianPortugalCB,
    NULL, "Arrival At Farmacia", "Arrival_At_Farmacia", NULL);
}


static void 
TestNavigationPedestrianSwedenCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianSweden(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "GtBensin");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 59.3727135658;
    destination.location.longitude = 17.8991907835;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_SWE.gps", 0, 0, 7272, TestNavigationPedestrianSwedenCB,
    NULL, "Arrival At GtBensin", "Arrival_At_GtBensin", NULL);
}


static void 
TestNavigationPedestrianTurkeyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianTurkey(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Akpet");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 38.7619382143;
    destination.location.longitude = 35.4655086994;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_TUR.gps", 0, 0, 4858, TestNavigationPedestrianTurkeyCB,
    NULL, "Arrival At Akpet", "Arrival_At_Akpet", NULL);
}


static void 
TestNavigationPedestrianUnitedKingdomCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianUnitedKingdom(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Bank Of Cyprus");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 51.5137714148;
    destination.location.longitude = -0.13945877552;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_UK.gps", 0, 0, 2392, TestNavigationPedestrianUnitedKingdomCB,
    NULL, "Arrival At Bank Of Cyprus", "Arrival_At_BankOfCyprus", NULL);
}


static void 
TestNavigationPedestrianCroatiaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianCroatia(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Dobranska");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 45.800381;
    destination.location.longitude = 15.966499;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_CRO.gps", 0, 0, 2540, TestNavigationPedestrianCroatiaCB, 
    NULL, "Arrival At Dobranska", "Arrival_At_Dobranska", NULL);
}


static void 
TestNavigationPedestrianEstoniaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianEstonia(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Hotel_St_Petersburg");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 59.437122;
    destination.location.longitude = 24.74267;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_EST.gps", 0, 0, 288, TestNavigationPedestrianEstoniaCB, 
    NULL, "Arrival At Hotel_St_Petersburg", "Arrival_At_Hotel_St_Petersburg", NULL);
}


static void 
TestNavigationPedestrianFinlandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianFinland(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Kymintle");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 60.209426;
    destination.location.longitude = 24.962399;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_FIN.gps", 0, 0, 2822, TestNavigationPedestrianFinlandCB, 
    NULL, "Arrival At Kymintle", "Arrival_At_Kymintle", NULL);
}


static void 
TestNavigationPedestrianHungaryCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianHungary(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "PestiAlsoRakpart");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 47.497123;
    destination.location.longitude = 19.047329;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_HUN.gps", 0, 0, 1162, TestNavigationPedestrianHungaryCB, 
    NULL, "Arrival At PestiAlsoRakpart", "Arrival_At_PestiAlsoRakpart", NULL);
}


static void 
TestNavigationPedestrianIrelandCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianIreland(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Butterfield");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 53.296146;
    destination.location.longitude = -6.288708;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_IRE.gps", 0, 0, 2950, TestNavigationPedestrianIrelandCB, 
    NULL, "Arrival At Butterfield", "Arrival_At_Butterfield", NULL);
}


static void 
TestNavigationPedestrianLatviaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianLatvia(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "FreedomMonumentRiga");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 56.951254;
    destination.location.longitude = 24.113034;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_LAT.gps", 0, 0, 2950, TestNavigationPedestrianLatviaCB, 
    NULL, "Arrival At FreedomMonumentRiga", "Arrival_At_FreedomMonumentRiga", NULL);
}


static void 
TestNavigationPedestrianLithuaniaCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianLithuania(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "GatesofDawn");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 54.674474;
    destination.location.longitude = 25.289485;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_LIT.gps", 0, 0, 954, TestNavigationPedestrianLithuaniaCB, 
    NULL, "Arrival At GatesofDawn", "Arrival_At_GatesofDawn", NULL);
}


static void 
TestNavigationPedestrianMexicoCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianMexico(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Articulo");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 29.120193;
    destination.location.longitude = -110.976014;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_MEX.gps", 0, 0, 2312, TestNavigationPedestrianMexicoCB, 
    NULL, "Arrival At Articulo", "Arrival_At_Articulo", NULL);
}

//-------------------------------------------------------------------------
// End - Test by country: Pedestrian
//-------------------------------------------------------------------------


static void 
TestNavigationPedestrianRouteTooLongCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianRouteTooLong(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "LAX");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 34.042276;
    destination.location.longitude = -118.405666;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationRouteError(&destination, &routeOptions, &routeConfiguration,
    "Route_Ped_USA.gps", 0, 0, 3172, TestNavigationPedestrianRouteTooLongCB, 
    NULL, "Pedestrian Nav Route Too Long", "ped_nav_error", NULL, NB_NRE_PedRouteTooLong);
}


static void 
TestNavigationPedestrianNoRecalcCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
    CU_ASSERT_NOT_EQUAL(publicState->status, NB_NS_UpdatingRoute);
}

void
TestNavigationPedestrianNoRecalc(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Walgreens");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude = 33.67035; // Bardeen
    destination.location.longitude = -117.85560; 

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;    
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;    

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
    "Route_Ped_USA.gps", 0, 0, 3172, TestNavigationPedestrianNoRecalcCB, 
    NULL, "Pedestrian Nav Verify no recalc", "verify_no_recalc", NULL);
}

static void 
TestNavigationPedestrianGuidanceCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{
}

void
TestNavigationPedestrianGuidance(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));

    destination.location.type = NB_Location_Address;
    strcpy(destination.location.streetnum, "2");
    strcpy(destination.location.street1, "Orion");
    strcpy(destination.location.city, "Aliso Viejo");
    strcpy(destination.location.state, "CA");
    strcpy(destination.location.postal, "92656");
    destination.location.latitude = 33.55764;
    destination.location.longitude = -117.726569;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Pedestrian;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationPedestrian(&routeConfiguration);
    routeConfiguration.wantTowardsRoadInformation = FALSE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
                                     "Route_Ped_USA_2.gps", 0, 939488203, 0xffffffff, TestNavigationPedestrianGuidanceCB,
                                     NULL, "Pedestrian Nav Guidance", "pedestrian_nav_guidance", NULL);
}

static void 
TestNavigation73ToLibertyCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigation73ToLiberty(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "NIM");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  =  33.559736;	
    destination.location.longitude = -117.72893;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "73-to-liberty.gps", 100, 940350883, 940351208, TestNavigation73ToLibertyCB, NULL, 
        "73 To Liberty (Car)", "73_To_Liberty", NULL);
}

static void
TestNavigationMultipleRecalcCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void
TestNavigationMultipleRecalc(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "LAX");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.9455986023;
    destination.location.longitude = -118.392303467;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "multiple_recalc.gps", 1000, 943130047, 943130328, TestNavigationMultipleRecalcCB, NULL,
        "Multiple Recalc (Car)", "multile_recalc", NULL);
}


static void
TestNavigationRecalcAfterReachingArrivingCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void
TestNavigationRecalcAfterReachingArriving(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "159th & Gauger");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 41.601053;
    destination.location.longitude = -87.653367;

    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_None;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "broadway ave harvey.gps", 100, 1, 111, TestNavigationRecalcAfterReachingArrivingCB, NULL,
        "Recalc after reaching arriving", "recalc_arriving", NULL);
}


void 
TestNavigationPedestrianManeuvers(void)
{
    PAL_Instance*             pal = 0;
    NB_Context*               context = 0;
    NB_Error                  result = NE_OK;
    TestRouteObj              routeObj = { 0 };
    NB_GpsLocation            gpsFix = {0};
    NB_Navigation*            navigation = 0;
    NB_NavigationConfiguration configuration = { 0 };
    NB_NavigationCallbacks    callbacks = {0};
    uint32                    numManeuvers = 0;
    NB_GuidanceInformation*   guidanceInfo = 0;
    NB_RouteInformation*      route = 0;
    NB_Place                  origin      = {{0}};  
    NB_Place                  destination = {{0}};     
    uint32                    maneuverIndex = 0;
    uint32                    numElevators = 0;

    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};

    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);
    
    gpsFix.status   = PAL_Ok;
    gpsFix.valid    = NGV_Latitude | NGV_Longitude;
    gpsFix.gpsTime    = PAL_ClockGetGPSTime();
    gpsFix.latitude   = 54.981817;
    gpsFix.longitude  = -1.490704;

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    if (CreatePalAndContext(&pal, &context)) 
    {
        CU_ASSERT_PTR_NOT_NULL(pal);
        CU_ASSERT_PTR_NOT_NULL(context);

        // Setup route that contains pedestrian escalators
        // Pizza Uno
        // 74 Ellison St
        // Jarrow, Tyne and Wear, NE32 3HX, United Kingdom
        origin.location.type = NB_Location_Address;
        strcpy(origin.location.streetnum, "74");
        strcpy(origin.location.street1, "Ellison St");
        strcpy(origin.location.city, "Jarrow");
        strcpy(origin.location.state, "Tyne And Wear");
        strcpy(origin.location.postal, "NE32 3HX");
        origin.location.latitude = 54.9816841015253;
        origin.location.longitude = -1.49096146031355;
        

        // Lynn's Fish & Chips
        // 122B Bewicke Rd
        // Wallsend, Tyne and Wear, NE28 6NJ, United Kingdom
        destination.location.type = NB_Location_Address;
        strcpy(destination.location.streetnum, "122");
        strcpy(destination.location.street1, "Bewicke Rd");
        strcpy(destination.location.city, "Wallsend");
        strcpy(destination.location.state, "Tyne and Wear");
        strcpy(destination.location.postal, "NE28 6NJ");
        destination.location.latitude = 54.9914341965655;
        destination.location.longitude = -1.49492126248487;

        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        RequestPedestrianRoute(pal, context, guidanceInfo, &routeObj, NULL, NULL, &origin, &destination);

        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;

        NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);

        result = NB_NavigationStartWithRouteInformation(navigation, routeObj.route, routeObj.trafficInfo, routeObj.cameraInfo, routeObj.poiInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        /* If the previous operation succeeded, then the navigation session now owns the memory, so clear our references */
        if (result == NE_OK) 
        {
            ClearInformationRouteObject(&routeObj);
        }

        result = NB_NavigationUpdateGPSLocation(navigation, &gpsFix);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (route != NULL)
        {
            numManeuvers = NB_RouteInformationGetManeuverCount(route);
            CU_ASSERT(numManeuvers > 0);

            if (numManeuvers > 0)
            {
                for (maneuverIndex = 0; maneuverIndex < numManeuvers; maneuverIndex++)
                {
                    Test_ValidateElevators(route, maneuverIndex, guidanceInfo, &numElevators);
                }

                CU_ASSERT(numElevators == 0);
            }
        }
        else 
        {
            CU_FAIL("No Route Returned");
        }
    }

    if (navigation)
    {
        result = NB_NavigationDestroy(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);
    }

    result = DestroyRouteObject(&routeObj, TRUE);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_GuidanceInformationDestroy(guidanceInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}


//-------------------------------------------------------------------------
// Begin  - Test Navigating with Enhanced Content
//-------------------------------------------------------------------------

static void 
TestNavigationEnhancedRealisticSignsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationEnhancedRealisticSigns(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "John Wayne-Orange County Airport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.68129;
    destination.location.longitude = -117.85883;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantRealisticSigns = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731140, TestNavigationEnhancedRealisticSignsCB, NULL, 
        "Alicia to SNA (SAR)", "Alicia_To_SNA_SAR", NULL);
}

static void 
TestNavigationEnhancedJunctionsWithSignsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationEnhancedJunctionsWithSigns(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "John Wayne-Orange County Airport");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 33.68129;
    destination.location.longitude = -117.85883;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantRealisticSigns = TRUE;
    routeConfiguration.wantJunctionModels = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "work_destination.gps", 100, 793729907, 793731140, TestNavigationEnhancedJunctionsWithSignsCB, NULL, 
        "Alicia to SNA (MJO)", "Alicia_To_SNA_MJO", NULL);
}

static void 
TestNavigationEnhancedCityModelsCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void 
TestNavigationEnhancedCityModels(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Staples Center");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 34.043546319;
    destination.location.longitude = -118.26533854;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantCityModels = TRUE;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration, 
        "LA_staples_center.gps", 100, 100, 239, TestNavigationEnhancedCityModelsCB, NULL, 
        "LA Staples Center", "LA_Staples_Center", NULL);
}

//-------------------------------------------------------------------------
// End - Test Navigating with Enhanced Content
//-------------------------------------------------------------------------

static void
TestNavigationPronunDataForTTSCB(PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation,
                                 NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData)
{

}

static void
TestNavigationPronunDataForTTS(void)
{
    NB_Place                destination = {{0}};
    NB_RouteOptions         routeOptions = {0};
    NB_RouteConfiguration   routeConfiguration = {0};

    nsl_memset(&destination, 0, sizeof(destination));
    nsl_strcpy(destination.name, "Staples Center");
    destination.location.type = NB_Location_LatLon;
    destination.location.latitude  = 34.043546319;
    destination.location.longitude = -118.26533854;


    routeOptions.type = NB_RouteType_Fastest;
    routeOptions.avoid = NB_RouteAvoid_HOV;
    routeOptions.transport = NB_TransportationMode_Car;
    routeOptions.traffic = NB_TrafficType_Default;
    routeOptions.pronunStyle = Test_OptionsGet()->voiceStyle;

    SetupRouteConfigurationVehicle(&routeConfiguration);
    routeConfiguration.wantCompleteRoute = TRUE;
    routeConfiguration.wantManeuvers = FALSE;
    g_wantExtendedPronunData = TRUE;
    nsl_strlcpy(routeConfiguration.supportedPhoneticsFormats, "IPA", sizeof(routeConfiguration.supportedPhoneticsFormats));
    routeConfiguration.maxPronunFiles = 0;

    NavigationTestNavigationSession(&destination, &routeOptions, &routeConfiguration,
        "LA_staples_center.gps", 100, 100, 150, TestNavigationPronunDataForTTSCB, NULL,
        "LA Staples Center", "LA_Staples_Center", NULL);
}

static TestGPSFile*
TestGPSFileLoad(PAL_Instance* pal, const char* gpsFileName)
{
    PAL_Error       palError = PAL_Ok;
    TestGPSFile*    pThis = nsl_malloc(sizeof(TestGPSFile));

    CU_ASSERT_PTR_NOT_NULL(pThis);
    
    if (pThis == NULL)
    {
        return pThis;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    palError = PAL_FileLoadFile(pal, gpsFileName, &pThis->fileBuffer, &pThis->fileSize);
    
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(pThis->fileBuffer);
    CU_ASSERT(pThis->fileSize > 0);

    pThis->nextFix = (GPSFileRecord*) pThis->fileBuffer;
    pThis->numFixes = pThis->fileSize / sizeof(GPSFileRecord);
    pThis->curFix = 0;

    /* Make sure the file makes sense for the record size */
    CU_ASSERT_EQUAL(pThis->fileSize % sizeof(GPSFileRecord), 0);

    return pThis;
}

static void
TestGPSFileDestroy(TestGPSFile* gps)
{
    CU_ASSERT_PTR_NOT_NULL(gps);

    if (gps == NULL)
    {
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(gps->fileBuffer);

    if (gps->fileBuffer != NULL)
    {
        nsl_free(gps->fileBuffer);
    }

    nsl_free(gps);
}

static NB_GpsLocation*
TestGPSFileGetNextFix(TestGPSFile* gps, int delayMilliseconds)
{
    if (gps->curFix >= gps->numFixes)
    {
        /* We are out of fixes */
        return NULL;
    }

    GPSInfo2GPSFix(gps->nextFix, &gps->curGPSLocation);

    if (delayMilliseconds)
    {
        // Wait for an event that will never be signaled.  This is an easy way to sleep (and pump messages).
        ResetCallbackCompletedEvent(g_CallbackEvent);
        WaitForCallbackCompletedEvent(g_CallbackEvent, delayMilliseconds);
    }

    gps->nextFix++;
    gps->curFix++;

    return &gps->curGPSLocation;
}

static void
TestGPSFileReset(TestGPSFile* gps)
{
    if (gps)
    {
        gps->nextFix = (GPSFileRecord*) gps->fileBuffer;        
        gps->curFix = 0;   
    }        
}

static void
TestNavigationSessionStatusChange(NB_Navigation* navigation, NB_NavigateStatus status, void* userData)
{
    SetCallbackCompletedEvent(g_CallbackEvent);
}

static void
ManeuverPositionChangeCallback(NB_Navigation* navigation, uint32 maneuver, NB_NavigateManeuverPos maneuvPosition, void* userData)
{
    LOGOUTPUT(LogLevelMedium, ("navigation maneuver position changed: maneuver = %d, maneuverPosition = %s\n",
        maneuver, GetManeuverPostionString(maneuvPosition)));
}

static nb_boolean g_completeRouteDownloaded = FALSE;

void AddPositionToSummary(TestPositionSummary* summary, double lat, double lon)
{
    int cur = 0;
    if (summary->num >= MAX_TEST_POSITION_FIXES)
    {
        return;
    }
    cur = summary->num;
    summary->coords[cur].latitude = lat; 
    summary->coords[cur].longitude = lon; 
    summary->num++;
}

void TrafficNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    if (status == NB_NetworkRequestStatus_Success)
    {
        DumpTrafficInformation((NB_Navigation*)userData, "traffic notify");

        DumpTrafficEventInformationForEachManeuver((NB_Navigation*)userData, "traffic notify");
        DumpTrafficEventList((NB_Navigation*)userData, "traffic notify");
    }
}

void CameraNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    if (status == NB_NetworkRequestStatus_Success)
    {
        DumpCameraInformation((NB_Navigation*)userData);
    }
}

static void
TestNavigationCompleteRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    g_completeRouteDownloaded = TRUE;
    DumpTrafficInformation((NB_Navigation*)userData, "route");

    DumpTrafficEventInformationForEachManeuver((NB_Navigation*)userData, "route");
    DumpTrafficEventList((NB_Navigation*)userData, "route");
    SetCallbackCompletedEvent(g_CallbackEvent);
}

static void
TestNavigationRouteDownload(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;
    NB_NavigationPublicState    publicNavigationState;
    
    NB_NavigationGetPublicState(navigation, &publicNavigationState);
    
    LOGOUTPUT(LogLevelHigh, ("navigation route download callback status = %d, err = %d, percent = %d\n", status, err, percent));
    switch (status)
    {
    case NB_NetworkRequestStatus_Progress:
        // not complete yet... return
        return;
    case NB_NetworkRequestStatus_Success:
        break;
    case NB_NetworkRequestStatus_TimedOut:
        CU_FAIL("NB_NetworkRequestStatus_TimedOut");
        break;
    case NB_NetworkRequestStatus_Canceled:
        LOGOUTPUT(LogLevelHigh, ("NB_NetworkRequestStatus_Canceled\n"));
        CU_FAIL("NB_NetworkRequestStatus_Canceled");
        break;
    case NB_NetworkRequestStatus_Failed:
        CU_FAIL("NB_NetworkRequestStatus_Failed");
        break;
    }
}

static void
TestNavigationRouteDownloadExpectError(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;
    NB_NavigationPublicState    publicNavigationState;
    
    NB_NavigationGetPublicState(navigation, &publicNavigationState);
    
    LOGOUTPUT(LogLevelHigh, ("navigation route download callback status = %d, err = %d, percent = %d\n", status, err, percent));
    switch (status)
    {
    case NB_NetworkRequestStatus_Progress:
        // not complete yet... return
        return;

    case NB_NetworkRequestStatus_Success:
        CU_FAIL("Should fail but did not!");
        break;
    case NB_NetworkRequestStatus_TimedOut:
        LOGOUTPUT(LogLevelHigh, ("NB_NetworkRequestStatus_TimedOut\n"));
        break;
    case NB_NetworkRequestStatus_Canceled:
        LOGOUTPUT(LogLevelHigh, ("NB_NetworkRequestStatus_Canceled\n"));
        break;
    case NB_NetworkRequestStatus_Failed:
        LOGOUTPUT(LogLevelHigh, ("NB_NetworkRequestStatus_Failed\n"));
        break;
    }
}
static void 
TestNavigationSessionGuidance(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData)
{
    int count;
    int index;
    const char* code;

    CU_ASSERT_PTR_NOT_NULL(navigation);
    CU_ASSERT_PTR_NOT_NULL(message);

    count = NB_GuidanceMessageGetCodeCount(message);

    CU_ASSERT_TRUE(count > 0);
    
    LOGOUTPUT(LogLevelMedium, ("\nGuidance Message\n"));

    for (index = 0; index < count; index++)
    {
        code = NB_GuidanceMessageGetCode(message, index);

        CU_ASSERT_PTR_NOT_NULL(code);
        LOGOUTPUT(LogLevelMedium, ("%s\n", code));
    }

    LOGOUTPUT(LogLevelMedium, ("\n"));

    NB_GuidanceMessagePlayed(message);

    NB_GuidanceMessageDestroy(message);
}

static void 
TestNavigationPointsOfInterestNotifcationCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Navigation* navigation = (NB_Navigation*)userData;

    if (status == NB_NetworkRequestStatus_Success)
    {
        DumpPointsOfInterestAlongRoute(navigation);
    }
    else
    {
        // Fail on any other status except progress
        CU_ASSERT_EQUAL(status, NB_NetworkRequestStatus_Progress);
    }
}


/*! Callback function for enhanced content data synchronization result
*/
static void
EnhancedContentSynchronizationCallback(NB_EnhancedContentManager* manager, NB_Error result, uint16 cityCount, void* userData)
{
    // TODO:
}


/*! Callback function for enhanced content data download result
*/
static void
EnhancedContentDownloadCallback(NB_EnhancedContentManager* manager, NB_Error result, void* data, void* userData)
{
    // TODO:
}

/*! Callback function for enhanced content state change notification during navigation
*/
static void
EnhancedContentStateNotificationCallback(NB_Navigation* navigation, NB_EnhancedContentState* state, NB_EnhancedContentStateData available, void* userData)
{
    NB_Error result = NE_OK;
    
    LOGOUTPUT(LogLevelMedium, ("\nenhanced content available:\n"));
    if (available == NB_ECSD_None)
    {
        LOGOUTPUT(LogLevelMedium, ("NONE\n"));
    }
    else 
    {
        // Check if map frame data available
        if ((available & NB_ECSD_JunctionMapFrame) || (available & NB_ECSD_CityModelMapFrame))
        {
            uint32 tileCount = 0;
            const char** tileIds = NULL;
            
            uint32 splineCount = 0;
            NB_MapFrameSpline* splines = NULL;
            
            NB_MapFrameProjectionParameters projectionParameters = { 0 };

            uint32 index = 0;

            result = NB_EnhancedContentStateGetMapFrameTileIds(state, &tileCount, &tileIds);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_NOT_EQUAL(tileCount, 0);

            result = NB_EnhancedContentStateGetMapFrameSplines(state, &splineCount, &splines);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_NOT_EQUAL(splineCount, 0);

            LOGOUTPUT(LogLevelMedium, ("%s: tile count: %d spline count: %d\n",
                ((available & NB_ECSD_JunctionMapFrame) ? "MJO" : "ECM"),
                tileCount, splineCount));

            LOGOUTPUT(LogLevelMedium, ("  Tile IDs:  "));
            for (index = 0; index < tileCount; index++)
            {
                LOGOUTPUT(LogLevelMedium, ("%s ", tileIds[index]));
            }
            LOGOUTPUT(LogLevelMedium, ("\n"));

            LOGOUTPUT(LogLevelMedium, ("  Splines:  "));
            for (index = 0; index < splineCount; index++)
            {
                LOGOUTPUT(LogLevelMedium, ("%s ", splines[index].pathId));
            }
            LOGOUTPUT(LogLevelMedium, ("\n"));
            
            result = NB_EnhancedContentStateGetMapFrameProjectionParameters(state, &projectionParameters);
            CU_ASSERT_EQUAL(result, NE_OK);

            LOGOUTPUT(LogLevelMedium, ("  Proj params: proj %d, datum %d, origin %f, %f, scale %f, falseE %f, falseN %f, zoffset %f\n",
                projectionParameters.projection, projectionParameters.datum,
                projectionParameters.originLatitude, projectionParameters.originLongitude, projectionParameters.scaleFactor,
                projectionParameters.falseEasting, projectionParameters.falseNorthing, projectionParameters.zOffset));
        }
        
        // Check if realistic sign available
        if (available & NB_ECSD_RealisticSign)
        {
            NB_RealisticSign sign = { 0 };
            
            result = NB_EnhancedContentStateGetRealisticSign(state, &sign);
            CU_ASSERT_EQUAL(result, NE_OK);

            LOGOUTPUT(LogLevelMedium, ("SAR: id '%s'\n", sign.signId));

            result = NB_EnhancedContentStateFreeRealisticSign(state, &sign);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }
    LOGOUTPUT(LogLevelMedium, ("\n"));
}

void NavigationModeChangeCallback(NB_Navigation* navigation, NB_NavigationMode mode, void* userData)
{
    LOGOUTPUT(LogLevelMedium, ("Navigation Mode Changed mode=%s\n", (mode == NB_NM_Automatic) ? "Automatic" : "StaticRoute"));
}

static void
CreateEnhancedContentManager(NB_Context* context, NB_RouteConfiguration* routeConfiguration, NB_EnhancedContentManager** manager)
{
    if (routeConfiguration->wantCityModels || routeConfiguration->wantJunctionModels || routeConfiguration->wantRealisticSigns)
    {
        NB_Error result = NE_OK;

        NB_EnhancedContentManagerConfiguration config = { 0 };

        NB_EnhancedContentSynchronizationCallback synchronizationCallback = {0};
        NB_EnhancedContentDownloadCallback downloadCallback = {0};

        // TODO: enhanced content not working with default nav,global servlet
        NB_TargetMapping targets[] = {{ "nav", "nav,usa" }};
        result = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));

        // @todo: Support enhanced content suite for iPhone
        SetupEnhancedContentManagerConfiguration(&config);
        config.enhancedCityModelEnabled = routeConfiguration->wantCityModels;
        config.motorwayJunctionObjectsEnabled = routeConfiguration->wantJunctionModels;
        config.realisticSignsEnabled = routeConfiguration->wantRealisticSigns;

        result = NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, manager);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(context, *manager);
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

static void
SetupEnhancedContentMapConfiguration(NB_EnhancedContentMapConfiguration* enhancedConfiguration)
{
    /* From SDS

        NavData-MapPerspective-vx-1.3
        
        Map perspective Z2, used for rendering the ECM, shall be defined as follows:
        
        Portrait Orientation:
        Camera Height (AB): 150 meters
        Camera to Avatar (BF): 12 meters
        Avatar to Horizon (FG): 750 meters
        Bottom to Avatar (DH):  70 pixels
        Horizontal Field-of-View: 55 degrees
        Avatar Scale: 4

        Landscape Orientation:
        Camera Height (AB): 150 meters
        Camera to Avatar (BF): 12 meters
        Avatar to Horizon (FG): 380 meters
        Bottom to Avatar (DH):  70 pixels
        Horizontal Field-of-View: 90 degrees
        Avatar Scale: 4
    */
    
    // Initialize configuration parameters to default values
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraHeightMeters                = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraToAvatarHorizontalMeters    = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToHorizonHorizontalMeters   = 750;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToMapBottomPixel            = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].horizontalFieldOfViewDegrees      = 55;
    
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraHeightMeters               = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraToAvatarHorizontalMeters   = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToHorizonHorizontalMeters  = 380;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToMapBottomPixel           = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].horizontalFieldOfViewDegrees     = 90;

    enhancedConfiguration->waitBetweenCityModelDisplaySeconds = 15;
    enhancedConfiguration->maximumJunctionViewDistanceMeters  = 200;

    enhancedConfiguration->mapFieldWidthPixel     = 480;
    enhancedConfiguration->mapFieldHeightPixel    = 758;
}


/*! Common test function for a navigation session. 

Common test function for a navigation session. This function is intended as the driver function for a variety of navigation session tests.

@param destination Where the route ends
@param routeOptions User options that influence the route
@param routeConfiguration Application configuration that specifies data options
@param gpsFileName File name for the GPS file containing the fixes for the route
@param startTime time stamp for the GPS fix to start the navigation session with
@param endTime time stamp for the last GPS fix to use for the test
@param testName name of this test for logging purposes
@param kmlFileName name of the KML file to log the route to.
@returns NB_Error
*/

void 
NavigationTestNavigationSession(NB_Place* destination, NB_RouteOptions* routeOptions, 
    NB_RouteConfiguration* routeConfiguration, const char* gpsFileName, int fixDelayMilliseconds, 
    uint32 startTime, uint32 endTime, 
    NB_NavigationSessionTestCallback* testCallback, void* testCallbackUserData, 
    const char* testName, const char* logBaseName, TestNavigationSessionOptions* sessionOptions)
{
    PAL_Instance*               pal = NULL;
    NB_Context*                 context = NULL;
    NB_GpsHistory*              history = NULL;
    NB_Error                    result = NE_OK;

    NB_Navigation*              navigation = NULL;
    NB_NavigationConfiguration  configuration = { 0 };
    NB_NavigationCallbacks      callbacks = {0};
    NB_NavigateStatus           status;
    NB_NavigationPublicState    publicNavigationState;

    NB_GuidanceInformation*     guidanceInfo = NULL;
    NB_GuidanceMessage*         message = NULL;

    NB_RouteParameters*         parameters = NULL;
    NB_RouteInformation*        route = NULL;
    NB_TrafficInformation*      trafficInfo = 0;

    NB_EnhancedContentManager*  ECManager = NULL;

    TestGPSFile*                gpsSource = NULL;
    NB_GpsLocation*             gpsFix = NULL;

    char                        kmlFileName[255];
    int                         routeIndex = 0;

    NB_LatitudeLongitude        maxLatLon;
    NB_LatitudeLongitude        minLatLon;
    uint32                      routePolyColor = 0xff00ff00;
                            
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};
    NB_PointsOfInterestPrefetchConfiguration poiConfig = {{0}};
    NB_EnhancedContentMapConfiguration enhancedConfig = {{0}};
    
    NB_Analytics* analytics = NULL;

    NB_RouteId*         routeID = NULL;
    
    TestPositionSummary* navPositionSummary;

    uint32 endManeuver = 0;
    double meter = 0;
    uint32 incidents = 0;
    uint32 time = 0;
    uint32 delay = 0;
    
    navPositionSummary = nsl_malloc(sizeof(TestPositionSummary));
    if(!navPositionSummary)
    {
        CU_ASSERT_PTR_NOT_NULL(navPositionSummary);
        return;
    }
    navPositionSummary->num = 0;

    minLatLon.latitude = INVALID_LATLON;
    minLatLon.longitude = INVALID_LATLON;
    maxLatLon.latitude = INVALID_LATLON;
    maxLatLon.longitude = INVALID_LATLON;
   
    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);
    configuration.cameraSearchEnabled = (nb_boolean)(sessionOptions && sessionOptions->showCameras);

    if (sessionOptions && sessionOptions->showPlaces)
    {
        nsl_strcpy(poiConfig.searchScheme,"atlasbook-bing");
        // set up POI prefetch configuration - Matches the Vehicle Settings from ABV5
        poiConfig.categoryCount = 9;
        nsl_strcpy(poiConfig.categories[0], "AE");   // Restaurant
        nsl_strcpy(poiConfig.categories[1], "ACC");  // Gas Stations
        nsl_strcpy(poiConfig.categories[2], "AA");   // Banks & ATMs
        nsl_strcpy(poiConfig.categories[3], "ACA");   // Lodging
        nsl_strcpy(poiConfig.categories[4], "AIC");   // Dentists
        nsl_strcpy(poiConfig.categories[5], "AID");   // Emergency Medical Service
        nsl_strcpy(poiConfig.categories[6], "AIE");   // Hospitals
        nsl_strcpy(poiConfig.categories[7], "AKE");   // Pharmacies
        nsl_strcpy(poiConfig.categories[8], "AFC");   // Police

        poiConfig.densityFactor = 5;
        poiConfig.searchWidthMeters = 400;
    }

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    CreatePalAndContext(&pal, &context);
    
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);
    
    if (pal != NULL && context != NULL) 
    {
        result = NB_AnalyticsCreate(context, NULL, &analytics);
        CU_ASSERT_EQUAL(result, NE_OK);
        (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);

        CreateEnhancedContentManager(context, routeConfiguration, &ECManager);

        history = NB_ContextGetGpsHistory(context);
        CU_ASSERT_PTR_NOT_NULL(history);

        gpsSource = TestGPSFileLoad(pal, gpsFileName);
        CU_ASSERT_PTR_NOT_NULL(gpsSource);

        /* Add the fixes up to the start time into the GPS history */
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, 0)) != NULL) && (gpsFix->gpsTime <= startTime))
        {       
            if (gpsFix->status == NE_OK)
            {
                result = NB_GpsHistoryAdd(history, gpsFix);
                CU_ASSERT_EQUAL(result, NE_OK);
            }                                    
        }
        
        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        routeConfiguration->commandSetVersion = NB_GuidanceInformationGetCommandVersion(guidanceInfo);

        callbacks.statusChangeCallback = TestNavigationSessionStatusChange;
        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;
        callbacks.routeCompleteDownloadCallback = TestNavigationCompleteRouteDownload;
        callbacks.routeDownloadCallback = TestNavigationRouteDownload;
        callbacks.pointsOfInterestNotificationCallback = TestNavigationPointsOfInterestNotifcationCallback;
        callbacks.trafficNotificationCallback = TrafficNotifcationCallback;
        callbacks.cameraNotificationCallback = CameraNotifcationCallback;
        callbacks.maneuverPositionChangeCallback = ManeuverPositionChangeCallback;
        callbacks.enhancedContentNotificationCallback = EnhancedContentStateNotificationCallback;
        callbacks.modeChangeCallback = NavigationModeChangeCallback;
        callbacks.userData = NULL;

        result = NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);

        callbacks.userData = navigation;
        NB_NavigationSetCallbacks(navigation, &callbacks);

        // Set enhanced content configuration parameters for navigating with enhanced content
        if (routeConfiguration->wantCityModels || routeConfiguration->wantJunctionModels || routeConfiguration->wantRealisticSigns)
        {
            SetupEnhancedContentMapConfiguration(&enhancedConfig);

            result = NB_NavigationSetEnhancedContentMapConfiguration(navigation, &enhancedConfig);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_NavigationSetMapOrientation(navigation, NB_MO_Portrait);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
        
        if (sessionOptions && sessionOptions->manualTrafficPollIntervalMinutes > 0)
        {
            result = NB_NavigationSetTrafficPollingInterval(navigation, sessionOptions->manualTrafficPollIntervalMinutes);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        result = NB_RouteParametersCreateFromGpsHistory(context, history, destination, 
                    routeOptions, routeConfiguration, &parameters);
        CU_ASSERT_PTR_NOT_NULL(parameters);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_RouteParametersSetLanguage(parameters, Test_OptionsGet()->currentLanguage);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (guidanceInfo != NULL && navigation != NULL && parameters != NULL)
        {
            result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (sessionOptions && sessionOptions->showPlaces)
            {
                result = NB_NavigationSetPointsOfInterestConfiguration(navigation, &poiConfig);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            result = NB_NavigationGetStatus(navigation, &status);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(status, NB_NS_Created);

            ResetCallbackCompletedEvent(g_CallbackEvent);

            result = NB_NavigationStartWithRouteParameters(navigation, parameters, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_NavigationGetStatus(navigation, &status);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(status, NB_NS_InitialRoute);

            //@todo - potential race condition here if the route comes back REALLY fast.
            ResetCallbackCompletedEvent(g_CallbackEvent);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
            {
                int loopCount = 0;
                gpsFix = TestGPSFileGetNextFix(gpsSource, 0);
                NB_NavigationUpdateGPSLocation(navigation,gpsFix);
                result = NB_NavigationGetStatus(navigation, &status);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT((status == NB_NS_Navigating || status == NB_NS_NavigatingStartup || status == NB_NS_NavigatingFerry || status == NB_NS_NavigatingArriving));
                LOGOUTPUT(LogLevelHigh, ("navigation status = %d\n", status));

                if (status == NB_NS_Error)
                {
                    LOG_ROUTE_ERROR(navigation);
                }

                result = NB_NavigationGetInformation(navigation, &route, &trafficInfo, NULL, NULL);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(route);

                result = NB_RouteInformationGetRouteId(route, &routeID);
                CU_ASSERT_EQUAL(result, NE_OK);                
                CU_ASSERT_PTR_NOT_NULL(routeID);

                endManeuver = NB_RouteInformationGetManeuverCount(route);
                result = NB_TrafficInformationGetSummary(trafficInfo, route, 0, 0, endManeuver, &meter, &incidents, &time, &delay);
                LOGOUTPUT(LogLevelMedium, ("time = %u, delay = %u meter = %f, incidents = %d\n", time, delay, meter, incidents));
                
                if (route != NULL)
                {
                    Test_ValidateRouteInformation(pal,context, route, guidanceInfo, navigation);
                    
                    if (!NB_RouteInformationIsRouteComplete(route))
                    {
                        nsl_sprintf(kmlFileName, "%s-%4.4d-partial.kml", logBaseName, routeIndex); 
                        routePolyColor = 0xff0000ff;
                    }
                    else
                    {
                        nsl_sprintf(kmlFileName, "%s-%4.4d-full.kml", logBaseName, routeIndex);   
                        routePolyColor = 0xff00ff00;
                    }
                    routeIndex++;
                                                                                
                    NB_RouteInformationGetRouteExtent(route, 0, NB_RouteInformationGetManeuverCount(route), &minLatLon, &maxLatLon );
                    Test_DumpRouteToKML(pal, context, route, guidanceInfo, NULL, NULL, &minLatLon, &maxLatLon, testName, kmlFileName, routePolyColor);

                    if (NB_RouteInformationIsRouteComplete(route))
                    {
                        g_completeRouteDownloaded = FALSE;
                    }

                    while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, fixDelayMilliseconds)) != NULL) && (gpsFix->gpsTime <= endTime))
                    {                   
                        NB_PositionRoutePositionInformation routePositionInfo = {0};
                        result = NB_NavigationUpdateGPSLocation(navigation, gpsFix);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        /* Reset the event so we can properly capture the next status change */
                        ResetCallbackCompletedEvent(g_CallbackEvent);
                        
                        /* Allow async processing (pump messages) */                 
                        if ((++loopCount % 100) == 0)
                        {
                            WaitForCallbackCompletedEvent(g_CallbackEvent, 1); 
                        }                        

                        result = NB_NavigationGetStatus(navigation, &status);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        if (status == NB_NS_Error)
                        {
                            LOG_ROUTE_ERROR(navigation);
                        }

                        result = NB_NavigationGetRoutePositionInformation(navigation, &routePositionInfo);
                        CU_ASSERT_EQUAL(result, NE_OK);
                        LOGOUTPUT(LogLevelHigh, ("nav average speed : %f\n", routePositionInfo.averageSpeed));

                        result = NB_NavigationGetPublicState(navigation, &publicNavigationState);
                        CU_ASSERT_EQUAL(result, NE_OK);

                        AddPositionToSummary(navPositionSummary, publicNavigationState.latitude, publicNavigationState.longitude); 

                        if (testCallback != NULL)
                        {
                            testCallback(pal, context, navigation, guidanceInfo, gpsFix, &publicNavigationState, testCallbackUserData);
                        }

                        if (status == NB_NS_Navigating)
                        {
                            /* Retrieve and validate the current manual guidance message */
                            /* if this function returns NE_INVAL, it means we don't have a valid route position */
                            result = NB_NavigationGetManualGuidanceMessage(navigation, NB_NAT_Street, NB_NAU_Miles, FALSE, &message);

                            if (result != NE_OK) 
                            {
                                CU_FAIL("Invalid Manual Guidance Message");
                            }

                            CU_ASSERT_EQUAL(result, NE_OK);
                            CU_ASSERT_PTR_NOT_NULL(message);

                            if (message != NULL)
                            {
                                NB_GuidanceMessagePlayed(message);

                                Test_ValidateGuidanceMessage(pal, context, message, navigation);

                                NB_GuidanceMessageDestroy(message);
                            }

                            DumpNextUpcomingTrafficEvent(navigation);

                            if ( g_completeRouteDownloaded )
                            {                            
                                result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);                                
                                if (!NB_RouteInformationIsRouteComplete(route))
                                {
                                    nsl_sprintf(kmlFileName, "%s-%4.4d-partial.kml", logBaseName, routeIndex); 
                                    routePolyColor = 0xff0000ff;
                                }
                                else
                                {
                                    nsl_sprintf(kmlFileName, "%s-%4.4d-full.kml", logBaseName, routeIndex);   
                                    routePolyColor = 0xff00ff00;
                                }
                                routeIndex++;
                                
                                NB_RouteInformationGetRouteExtent(route, 0, NB_RouteInformationGetManeuverCount(route), &minLatLon, &maxLatLon );
                                Test_DumpRouteToKML(pal, context, route, guidanceInfo, NULL, NULL, &minLatLon, &maxLatLon, testName, kmlFileName, routePolyColor);
                                if (NB_RouteInformationIsRouteComplete(route))
                                {
                                    g_completeRouteDownloaded = FALSE;
                                }
                            }
                        }
                        else if (status == NB_NS_UpdatingRoute)
                        {
                            /* Recalculating */
                            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE) 
                            {                                                 
                                result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
                                CU_ASSERT_EQUAL(result, NE_OK);
                                CU_ASSERT_PTR_NOT_NULL(route);

                                Test_ValidateRouteInformation(pal, context, route, guidanceInfo, navigation);
                                
                                if (!NB_RouteInformationIsRouteComplete(route))
                                {
                                    nsl_sprintf(kmlFileName, "%s-%4.4d-partial-recalc.kml", logBaseName, routeIndex); 
                                    routePolyColor = 0xff0000ff;
                                }
                                else
                                {
                                    nsl_sprintf(kmlFileName, "%s-%4.4d-full-recalc.kml", logBaseName, routeIndex);   
                                    routePolyColor = 0xff00ff00;
                                }
                                routeIndex++;

                                NB_RouteInformationGetRouteExtent(route, 0, NB_RouteInformationGetManeuverCount(route), &minLatLon, &maxLatLon );
                                Test_DumpRouteToKML(pal, context, route, guidanceInfo, NULL, NULL, &minLatLon, &maxLatLon, testName, kmlFileName, routePolyColor);
                                if (NB_RouteInformationIsRouteComplete(route))
                                {
                                    g_completeRouteDownloaded = FALSE;
                                }

                            }
                            else
                            {
                                CU_FAIL_FATAL("Failed to get updated route before timeout");
                            }
                        }
                        else if (status == NB_NS_NavigatingConfirmRecalc)
                        {
                            result = NB_NavigationRecalculate(navigation);// TODO: need wait for event

                            CU_ASSERT_EQUAL(result, NE_OK);
                            ResetCallbackCompletedEvent(g_CallbackEvent);
                            WaitForCallbackCompletedEvent(g_CallbackEvent, 60000); 
                        }
                        else if (status == NB_NS_Error)
                        {
                            LOG_ROUTE_ERROR(navigation);

                            /* Update the route as whatever caused the failure may have invalidated the old route */ 
                            result = NB_NavigationGetInformation(navigation, &route, NULL, NULL, NULL);
                            CU_ASSERT_EQUAL(result, NE_OK);
                            CU_ASSERT_PTR_NOT_NULL(route);
                            
                            if (!NB_RouteInformationIsRouteComplete(route))
                            {
                                nsl_sprintf(kmlFileName, "%s-%4.4d-partial-error.kml", logBaseName, routeIndex); 
                                routePolyColor = 0xff0000ff;
                            }
                            else
                            {
                                nsl_sprintf(kmlFileName, "%s-%4.4d-full-error.kml", logBaseName, routeIndex);   
                                routePolyColor = 0xff00ff00;
                            }
                            routeIndex++;     
                            
                            NB_RouteInformationGetRouteExtent(route, 0, NB_RouteInformationGetManeuverCount(route), &minLatLon, &maxLatLon );
                            Test_DumpRouteToKML(pal, context, route, guidanceInfo, NULL, NULL, &minLatLon, &maxLatLon, testName, kmlFileName, routePolyColor);
                            if (NB_RouteInformationIsRouteComplete(route))
                            {
                                g_completeRouteDownloaded = FALSE;
                            }


                            break;
                        }
                    }                                                            
                }
                else
                {
                    CU_FAIL("No Route Returned");
                }                
            }
            else
            {
                CU_FAIL("Timeout waiting for route");
            } 
        }
        
        if (navPositionSummary)
        {
            nsl_free(navPositionSummary);
        }

        if (parameters != NULL)
        {
            result = NB_RouteParametersDestroy(parameters);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (navigation != NULL)
        {
            result = NB_NavigationDestroy(navigation);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (guidanceInfo != NULL)
        {
            result = NB_GuidanceInformationDestroy(guidanceInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        if (ECManager != NULL)
        {
            NB_EnhancedContentManagerDestroy(ECManager);
        }
    }

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);    

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void 
NavigationTestNavigationRouteError(NB_Place* destination, NB_RouteOptions* routeOptions, 
    NB_RouteConfiguration* routeConfiguration, const char* gpsFileName, int fixDelayMilliseconds, 
    uint32 startTime, uint32 endTime, 
    NB_NavigationSessionTestCallback* testCallback, void* testCallbackUserData, 
    const char* testName, const char* logBaseName, TestNavigationSessionOptions* sessionOptions,
    NB_NavigateRouteError expectedRouteError)
{
    PAL_Instance*               pal = NULL;
    NB_Context*                 context = NULL;
    NB_GpsHistory*              history = NULL;
    NB_Error                    result = NE_OK;

    NB_Navigation*              navigation = NULL;
    NB_NavigationConfiguration  configuration = { 0 };
    NB_NavigationCallbacks      callbacks = {0};
    NB_NavigateStatus           status;

    NB_GuidanceInformation*     guidanceInfo = NULL;

    NB_RouteParameters*         parameters = NULL;

    TestGPSFile*                gpsSource = NULL;
    NB_GpsLocation*             gpsFix = NULL;

    NB_LatitudeLongitude        maxLatLon;
    NB_LatitudeLongitude        minLatLon;
                            
    NB_VectorMapPrefetchConfiguration vectorPrefetchConfig = {0};
    NB_PointsOfInterestPrefetchConfiguration poiConfig = {{0}};

    NB_Analytics* analytics = NULL;

    minLatLon.latitude = INVALID_LATLON;
    minLatLon.longitude = INVALID_LATLON;
    maxLatLon.latitude = INVALID_LATLON;
    maxLatLon.longitude = INVALID_LATLON;
   
    SetupVectorMapPrefetchConfiguration(&vectorPrefetchConfig);
    SetupNavigationConfiguration(&configuration);
    configuration.cameraSearchEnabled = (nb_boolean)(sessionOptions && sessionOptions->showCameras);

    if (sessionOptions && sessionOptions->showPlaces)
    {
        nsl_strcpy(poiConfig.searchScheme,"atlasbook-standard");
        // set up POI prefetch configuration - Matches the Vehicle Settings from ABV5
        poiConfig.categoryCount = 9;
        nsl_strcpy(poiConfig.categories[0], "AE");   // Restaurant
        nsl_strcpy(poiConfig.categories[1], "ACC");  // Gas Stations
        nsl_strcpy(poiConfig.categories[2], "AA");   // Banks & ATMs
        nsl_strcpy(poiConfig.categories[3], "ACA");   // Lodging
        nsl_strcpy(poiConfig.categories[4], "AIC");   // Dentists
        nsl_strcpy(poiConfig.categories[5], "AID");   // Emergency Medical Service
        nsl_strcpy(poiConfig.categories[6], "AIE");   // Hospitals
        nsl_strcpy(poiConfig.categories[7], "AKE");   // Pharmacies
        nsl_strcpy(poiConfig.categories[8], "AFC");   // Police

        poiConfig.densityFactor = 5;
        poiConfig.searchWidthMeters = 400;
    }

    LOGOUTPUT(LogLevelHigh, ("\n\n"));

    // Initialize Pal, NIMState and NB_Network
    CreatePalAndContext(&pal, &context);
    
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);
    
    if (pal != NULL && context != NULL)
    {
        result = NB_AnalyticsCreate(context, NULL, &analytics);
        CU_ASSERT_EQUAL(result, NE_OK);
        (void)NB_ContextSetAnalyticsNoOwnershipTransfer(context, analytics);

        history = NB_ContextGetGpsHistory(context);
        CU_ASSERT_PTR_NOT_NULL(history);

        gpsSource = TestGPSFileLoad(pal, gpsFileName);
        CU_ASSERT_PTR_NOT_NULL(gpsSource);

        /* Add the fixes up to the start time into the GPS history */
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, 0)) != NULL) && (gpsFix->gpsTime <= startTime))
        {
            if (gpsFix->status == NE_OK)
            {
                result = NB_GpsHistoryAdd(history, gpsFix);
                CU_ASSERT_EQUAL(result, NE_OK);
            }
        }
        
        guidanceInfo = CreateGuidanceInformation(pal, context);
        CU_ASSERT_PTR_NOT_NULL(guidanceInfo);

        routeConfiguration->commandSetVersion = NB_GuidanceInformationGetCommandVersion(guidanceInfo);

        callbacks.statusChangeCallback = TestNavigationSessionStatusChange;
        callbacks.guidanceMessageCallback = TestNavigationSessionGuidance;
        callbacks.routeCompleteDownloadCallback = TestNavigationCompleteRouteDownload;
        callbacks.routeDownloadCallback = TestNavigationRouteDownloadExpectError;
        callbacks.pointsOfInterestNotificationCallback = TestNavigationPointsOfInterestNotifcationCallback;
        callbacks.trafficNotificationCallback = TrafficNotifcationCallback;
        callbacks.cameraNotificationCallback = CameraNotifcationCallback;
        callbacks.userData = NULL;

        result = NB_NavigationCreate(context, &configuration, &callbacks, &vectorPrefetchConfig, &navigation);
        CU_ASSERT_PTR_NOT_NULL(navigation);
        CU_ASSERT_EQUAL(result, NE_OK);

        callbacks.userData = navigation;
        NB_NavigationSetCallbacks(navigation, &callbacks);

        result = NB_RouteParametersCreateFromGpsHistory(context, history, destination,
                routeOptions, routeConfiguration, &parameters);
        CU_ASSERT_PTR_NOT_NULL(parameters);
        CU_ASSERT_EQUAL(result, NE_OK);

        result = NB_RouteParametersSetLanguage(parameters, Test_OptionsGet()->currentLanguage);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (guidanceInfo != NULL && navigation != NULL && parameters != NULL)
        {
            result = NB_NavigationSetGuidanceInformation(navigation, guidanceInfo);
            CU_ASSERT_EQUAL(result, NE_OK);

            if (sessionOptions && sessionOptions->showPlaces)
            {
                result = NB_NavigationSetPointsOfInterestConfiguration(navigation, &poiConfig);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            result = NB_NavigationGetStatus(navigation, &status);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(status, NB_NS_Created);

            ResetCallbackCompletedEvent(g_CallbackEvent);

            result = NB_NavigationStartWithRouteParameters(navigation, parameters, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_NavigationGetStatus(navigation, &status);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(status, NB_NS_InitialRoute);

            //@todo - potential race condition here if the route comes back REALLY fast.
            ResetCallbackCompletedEvent(g_CallbackEvent);

            if (WaitForCallbackCompletedEvent(g_CallbackEvent, 60000) == TRUE)
            {
                NB_NavigateRouteError routeError = NB_NRE_None;
                NB_Error navError = NE_OK;

                CU_ASSERT_EQUAL(NB_NavigationGetErrors(navigation, &routeError, &navError), NE_OK);
                CU_ASSERT_EQUAL(routeError, expectedRouteError);
            }
            else
            {
                CU_FAIL("Timeout waiting for route");
            }
        }

        if (parameters != NULL)
        {
            result = NB_RouteParametersDestroy(parameters);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (navigation != NULL)
        {
            result = NB_NavigationDestroy(navigation);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (guidanceInfo != NULL)
        {
            result = NB_GuidanceInformationDestroy(guidanceInfo);
            CU_ASSERT_EQUAL(result, NE_OK);
        }

        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }
    }

    //Cleanup pal and context
    DestroyContext(context);
    PAL_DestroyInstance(pal);

    LOGOUTPUT(LogLevelHigh, ("\n"));
}
/*****************************************************************************/

// Sub-Tests

static void 
Test_ValidateRouteInformation(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, NB_Navigation* navigation)
{
    CU_ASSERT_PTR_NOT_NULL(route);

    if (route == NULL) 
    {
        return;
    }

    Test_ValidateManeuvers(pal, context, route, guidance, navigation);
    Test_ValidateRoutePolyline(route);
}

static void
Test_ValidateRoutePolyline(NB_RouteInformation* route)
{
    NB_Error result = NE_OK;
    uint32 numManeuvers = 0;
    NB_MercatorPolyline* polyline = NULL;

    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT(numManeuvers > 0);

    result = NB_RouteInformationGetRoutePolyline(route, 0, numManeuvers, &polyline);
    CU_ASSERT_EQUAL(result, NE_OK);

    CU_ASSERT_PTR_NOT_NULL(polyline)
    CU_ASSERT_PTR_NOT_NULL(polyline->points)

    /* This is not a really good test, but it isn't clear how to validate this. */
    CU_ASSERT_TRUE(polyline->count >= (int) numManeuvers);

    result = NB_MercatorPolylineDestroy(polyline);
    CU_ASSERT_EQUAL(result, NE_OK);
}

static void 
Test_ValidateManeuvers(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, NB_Navigation* navigation)
{
    NB_Error result = NE_OK;
    uint32 numManeuvers = 0;
    uint32 maneuverIndex = 0;
    double distance = 0.0;

    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT(numManeuvers > 0);
    
    for (maneuverIndex = 0; maneuverIndex < numManeuvers; maneuverIndex++)
    {
        #define _SIZE 1024
        char primaryManeuver[_SIZE] = {0};
        char secondaryManeuver[_SIZE] = {0};
        char currentRoad[_SIZE] = {0};

        result = NB_RouteInformationGetTurnInformation(route, 
            maneuverIndex, 
            NULL,
            primaryManeuver, 
            _SIZE, 
            secondaryManeuver, 
            _SIZE, 
            currentRoad, 
            _SIZE, 
            &distance, 
            FALSE);
        
        CU_ASSERT_EQUAL(result, NE_OK);                            
        LOGOUTPUT(LogLevelHigh, ("Maneuver[%d]: Primary: %s, Secondary: %s, Current Road: %s, Distance: %1.1f\n",
            maneuverIndex,
            primaryManeuver,
            secondaryManeuver,
            currentRoad,
            distance));               

        Test_ValidateTurnImages(route, maneuverIndex, guidance);
        Test_ValidateTurnAnnouncements(pal, context, route, maneuverIndex, guidance, navigation);
        Test_ValidateManeuverText(route, maneuverIndex, guidance);
    }                    
}

static void
Test_ValidateTurnImages(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance)
{
    NB_Error result = NE_OK;
    char imageCode[40] = {0};
    NB_NavigateManeuverPos position = NB_NMP_Current;
    nb_boolean required = FALSE;

    // For all maneuver positions. 'NBI_NMP_ConfirmDetour' is the last one.
    while ((int)position <= (int)NB_NMP_ConfirmDetour)
    {
        required = FALSE;

        if (position == NB_NMP_Continue ||
            position == NB_NMP_ShowTurn ||
            position == NB_NMP_Prepare ||
            position == NB_NMP_Turn ||
            position == NB_NMP_Past)
        {
            required = TRUE;
        }
        

        // Get turn image code
        // @TODO NB_RouteInformationGetTurnImage deprecated. use NB_GuidanceInformationGetTurnCharacter
        result = NB_RouteInformationGetTurnImage(route, guidance, maneuverIndex, position, NB_NAU_Miles, imageCode, sizeof(imageCode));

        // The result can either be NE_OK or NE_NOENT (NE_NOENT is not allowed if the position is one that is required to have a turn icon)
        CU_ASSERT((result == NE_OK) || ((!required) && result == NE_NOENT));

        if (result == NE_OK)
        {
            CU_ASSERT(nsl_strlen(imageCode) > 0);

            LOGOUTPUT(LogLevelHigh, ("Image code: %s\n", imageCode));
        }
        else if (result == NE_NOENT)
        {
            LOGOUTPUT(LogLevelHigh, ("No Image code.\n"));
        }
        
        ++position;
    }          
}


static void
Test_ValidateElevators(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance, uint32 *numElevators)
{
    NB_Error result = NE_OK;
    char imageCode[40] = {0};
    NB_NavigateManeuverPos position = NB_NMP_Current;
    nb_boolean required = FALSE;

    // For all maneuver positions. 'NBI_NMP_ConfirmDetour' is the last one.
    while ((int)position <= (int)NB_NMP_ConfirmDetour)
    {
        required = FALSE;

        if (position == NB_NMP_Continue ||
            position == NB_NMP_ShowTurn ||
            position == NB_NMP_Prepare ||
            position == NB_NMP_Turn ||
            position == NB_NMP_Past)
        {
            required = TRUE;
        }
        
        // Get turn image code
        // @TODO NB_RouteInformationGetTurnImage deprecated. use NB_GuidanceInformationGetTurnCharacter
        result = NB_RouteInformationGetTurnImage(route, guidance, maneuverIndex, position, NB_NAU_Miles, imageCode, sizeof(imageCode));

        // The result can either be NE_OK or NE_NOENT (NE_NOENT is not allowed if the position is one that is required to have a turn icon)
        CU_ASSERT((result == NE_OK) || ((!required) && result == NE_NOENT));

        if (result == NE_OK)
        {
            CU_ASSERT(nsl_strlen(imageCode) > 0);

            // Look for elevator instruction
            if (nsl_strcmp("ee-r", imageCode) == 0 ||
                nsl_strcmp("ee-l", imageCode) == 0)
            {
                *numElevators++;
                LOGOUTPUT(LogLevelHigh, ("Elevator instruction\n"));
            }
        }
        else if (result == NE_NOENT)
        {
            LOGOUTPUT(LogLevelHigh, ("No Image code.\n"));
        }
        
        ++position;
    }          
}



static void
Test_ValidateTurnAnnouncements(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance, NB_Navigation* navigation)
{
    NB_Error result = NE_OK;
    NB_NavigateAnnouncementType type;
    NB_GuidanceMessage* message = NULL;
    //only NB_NAT_Street uses now
    for (type = NB_NAT_Street; type <= NB_NAT_Street; type++) 
    {
        result = NB_RouteInformationGetTurnAnnouncement(route, guidance, maneuverIndex, NB_NMP_Turn, NB_NAS_Lookahead, type, NB_NAU_Miles, &message);

        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(message);

        if (result == NE_OK && message)
        {
            Test_ValidateGuidanceMessage(pal, context, message, navigation);

            NB_GuidanceMessageDestroy(message);
            message = NULL;
        }                   
    }
}

static void
Test_ValidateManeuverText(NB_RouteInformation* route, uint32 maneuverIndex, NB_GuidanceInformation* guidance)
{
// directions.sexp is deprecated
    /*    NB_Error result = NE_OK;

    LOGOUTPUT(LogLevelHigh, ("NBI_NavigateEnumManeuverText[%d]\n", maneuverIndex));
    result = NB_RouteInformationEnumerateManeuverText(route, guidance, NB_NTT_Primary, NB_NAU_Miles, maneuverIndex, EnumerateTextCallback, route);

    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelHigh, ("NBI_NavigateEnumStackText[%d]\n", maneuverIndex));
    // Only returns text for stack turns. Otherwise no text is returned.
    result = NB_RouteInformationEnumerateStackedTurnText(route, guidance, NB_NTT_Primary, NB_NAU_Miles, maneuverIndex, EnumerateTextCallback, route);
    CU_ASSERT_EQUAL(result, NE_OK);
*/
}

static nb_boolean 
Test_ValidatePronunCode(PAL_Instance* pal, NB_Context* context, const char* code, nb_size size)
{
    NB_Error result =  NE_OK;
    int i;
    nb_boolean found = FALSE;
    byte* audio_data;
    size_t audio_size;
    char formatFile[100] = "voices%s%s%s";
    char file[100];
    char* extensions[] = {
    ".aac",
    ".amr",
    ".ipa"};

    CSL_Cache* voiceCache = NB_ContextGetVoiceCache(context);

    CU_ASSERT_PTR_NOT_NULL(voiceCache);

    // check all extenstions
    for (i = 0; !found && i < (sizeof(extensions)/sizeof(extensions[0])); i++)
    {
        // TODO: merge PATH_DELIMETER into palstdlib.h from any branch
        nsl_sprintf(file, formatFile, /* PATH_DELIMETER*/ "\\", code, extensions[i]);
        if (PAL_Ok == PAL_FileExists(pal, file))
        {
            found = TRUE;
        }
    }

    if (!found)
    { 
        result = CSL_CacheFind(voiceCache, (byte*) code, (size_t) size, &audio_data, &audio_size, FALSE, FALSE, FALSE);

        if (result == NE_OK)
        {
            found = TRUE;
        }
    }

    return found;
}


static void 
Test_ValidateGuidanceMessage(PAL_Instance* pal, NB_Context* context, NB_GuidanceMessage* message, NB_Navigation* navigation)
{
    int num = 0;
    const char* code = NULL;

    CU_ASSERT_PTR_NOT_NULL(message);

    if (message)
    {
        int count = NB_GuidanceMessageGetCodeCount(message);
        LOGOUTPUT(LogLevelHigh, ("VOICE (%d): ", count));
        for (num = 0; num < count; num++)
        {
            code = NB_GuidanceMessageGetCode(message, num);
            CU_ASSERT_PTR_NOT_NULL(code);

            if (code)
            {
                CU_ASSERT(nsl_strlen(code) > 0);
                LOGOUTPUT(LogLevelHigh, ("%s ", code));

                if (!Test_ValidatePronunCode(pal, context, code, nsl_strlen(code)))
                {
                    LOGOUTPUT(LogLevelLow, ("\nMissing Data for Voice Code[%d]: %s\n", num, code));
                    CU_FAIL(("Missing Data for Voice Code"));
                }
                else
                {
                    if (g_wantExtendedPronunData)
                    {
                        char textBuffer[200] = {0};
                        uint32 textBufferSize = 200;
                        byte* phoneticsData = nsl_malloc(200);
                        uint32 phoneticsDataSize = 200 * sizeof(byte);
                        double duration = 0.0;

                        nsl_memset(phoneticsData, 0, phoneticsDataSize);
                        if (NE_OK != NB_NavigationGetPronunInformation(navigation, code,
                                                          textBuffer, &textBufferSize,
                                                          phoneticsData, &phoneticsDataSize,
                                                          &duration))
                        {
                            LOGOUTPUT(LogLevelLow, ("\nMissing Pronun Data for Voice Code[%d]: %s\n", num, code));
                            CU_FAIL(("Missing Data for Voice Code"));
                        }

                        if (phoneticsData)
                        {
                            nsl_free(phoneticsData);
                        }
                    }
                }
            }
        }
        LOGOUTPUT(LogLevelHigh, ("\n"));
    }
}

static void 
Test_ValidateNavigationStatus(NB_Navigation* navigation, nb_gpsTime currentTime, nb_gpsTime startTime, nb_gpsTime endTime, NB_NavigateStatus status)
{
    NB_Error result;
    NB_NavigateStatus currentStatus;

    if (currentTime >= startTime && currentTime <= endTime)
    {
        result = NB_NavigationGetStatus(navigation, &currentStatus);
        CU_ASSERT_EQUAL(result, NE_OK);

        if (status != currentStatus)
        {
            LOGOUTPUT(LogLevelHigh, ("Status Mismatch - Time=%d status=%d current=%d\n", currentTime, status, currentStatus));
        }
    }
}

static PAL_Error
Test_WriteToKML(PAL_File* kmlFile, const char* data)
{
    uint32 size = nsl_strlen(data);
    uint32 written = 0;
    return PAL_FileWrite(kmlFile, (const uint8*) data, size, &written);
}

static PAL_Error
Test_WriteRoutePolylineToKML(PAL_File* kmlFile, NB_RouteInformation* route, uint32 color)
{
    NB_Error error = NE_OK;
    PAL_Error result = PAL_Ok;
    NB_MercatorPolyline* polyline;
    int index;
    double latitude;
    double longitude;
    char buffer[64];
    uint32 numManeuvers = 0;
    
    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT(numManeuvers > 0);

    result = NB_RouteInformationGetRoutePolyline(route, 0, numManeuvers, &polyline);    
    CU_ASSERT_EQUAL(error, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(polyline);

    if (polyline)
    {
        result = result ? result : Test_WriteToKML(kmlFile, "<Style id=\"routePolylineStyle\">\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <LineStyle>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <color>");
        nsl_sprintf(buffer, "%8.8X", color); 
        result = result ? result : Test_WriteToKML(kmlFile, buffer);
        result = result ? result : Test_WriteToKML(kmlFile, "</color>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <width>10</width>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   </LineStyle>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "</Style>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "<Placemark>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <name>Route Polyline</name>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <styleUrl>#routePolylineStyle</styleUrl>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   <LineString>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <extrude>1</extrude>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <tessellate>1</tessellate>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <altitudeMode>clampToGround</altitudeMode>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "       <coordinates>\n");

        for (index = 0; index < polyline->count; index++)
        {
            NB_SpatialConvertMercatorToLatLong(polyline->points[index].mx, polyline->points[index].my, &latitude, &longitude);

            nsl_doubletostr(longitude, buffer, sizeof(buffer));
            result = result ? result : Test_WriteToKML(kmlFile, buffer);
            result = result ? result : Test_WriteToKML(kmlFile, ",");
            nsl_doubletostr(latitude, buffer, sizeof(buffer));
            result = result ? result : Test_WriteToKML(kmlFile, buffer);
            result = result ? result : Test_WriteToKML(kmlFile, " ");
        }

        result = result ? result : Test_WriteToKML(kmlFile, "       </coordinates>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "   </LineString>\n");
        result = result ? result : Test_WriteToKML(kmlFile, "</Placemark>\n");
             
        error = NB_MercatorPolylineDestroy(polyline);
        CU_ASSERT_EQUAL(error, NE_OK);
    }

    return result;
}

static PAL_Error
Test_DumpManeuversToKML(PAL_File* kmlFile, NB_RouteInformation* route, NB_GuidanceInformation* guidance)
{
    PAL_Error error = PAL_Ok;
    NB_Error result = NE_OK;
    uint32 numManeuvers = 0;
    uint32 maneuverIndex = 0;
    double distance = 0.0;
    NB_LatitudeLongitude turnPoint = {0};

    numManeuvers = NB_RouteInformationGetManeuverCount(route);
    CU_ASSERT(numManeuvers > 0);
    
    for (maneuverIndex = 0; maneuverIndex < numManeuvers; maneuverIndex++)
    {
        #define _SIZE 1024
        char primaryManeuver[_SIZE] = {0};
        char secondaryManeuver[_SIZE] = {0};
        char currentRoad[_SIZE] = {0};
        char turnImageCode[64] = {0};
        char buffer[_SIZE] = {0};

        result = NB_RouteInformationGetTurnInformation(route, 
            maneuverIndex, 
            &turnPoint,
            primaryManeuver, 
            _SIZE, 
            secondaryManeuver, 
            _SIZE, 
            currentRoad, 
            _SIZE, 
            &distance, 
            FALSE);
        
        CU_ASSERT_EQUAL(result, NE_OK);            

        // @TODO NB_RouteInformationGetTurnImage deprecated. use NB_GuidanceInformationGetTurnCharacter
        result = NB_RouteInformationGetTurnImage(route, guidance, maneuverIndex, NB_NMP_Turn, NB_NAU_Miles, turnImageCode, sizeof(turnImageCode));

        CU_ASSERT_EQUAL(result, NE_OK);  

        error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "  <name>\n");
        nsl_sprintf(buffer, "Maneuver #%d - %s\n", maneuverIndex, turnImageCode);
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, "  </name>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "  <Point>\n");
        result = result ? result : Test_WriteToKML(kmlFile, " <color>ff0000ff</color>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "    <coordinates>");
        nsl_doubletostr(turnPoint.longitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, ",");
        nsl_doubletostr(turnPoint.latitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, ",0");
        error = error ? error : Test_WriteToKML(kmlFile, "</coordinates>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "  </Point>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");
    }      

    return error;
}


static PAL_Error
Test_DumpNavigationPositionToKML(PAL_File* kmlFile, TestPositionSummary* navigationSummary)
{
    PAL_Error error = PAL_Ok;
    int index;    
    char buffer[64];

    error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"routePolylineStyle\">\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   <LineStyle>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <color>ff0000ff</color>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <width>10</width>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   </LineStyle>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "</Style>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   <name>GPS Polyline</name>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   <styleUrl>#routePolylineStyle</styleUrl>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   <LineString>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <extrude>1</extrude>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <tessellate>1</tessellate>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <altitudeMode>clampToGround</altitudeMode>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "       <coordinates>\n");

    for (index = 0; index < navigationSummary->num; index++)
    {        
        nsl_doubletostr(navigationSummary->coords[index].longitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, ",");
        nsl_doubletostr(navigationSummary->coords[index].latitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, " ");
    }

    error = error ? error : Test_WriteToKML(kmlFile, "       </coordinates>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "   </LineString>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");

    CU_ASSERT_EQUAL(error, NE_OK);

    return error;
}

static PAL_Error
Test_DumpTrafficNotifyRepliesToKML(PAL_File* kmlFile, TestPositionSummary* trafficSummary)
{
    PAL_Error error = PAL_Ok;
    int index;    
    char buffer[64];

    //dump traffic notify reply successes
    for (index = 0; index < trafficSummary->num; index++)
    {            
        error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");        
        error = error ? error : Test_WriteToKML(kmlFile, "  <name>\n");        
        nsl_sprintf(buffer, "Traffic notify reply: %d", index);
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, "  </name>\n");   

        error = error ? error : Test_WriteToKML(kmlFile, "<StyleMap id=\"msn_cabs\">");
        error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <key>normal</key>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sn_cabs</styleUrl>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <key>highlight</key>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sh_cabs</styleUrl>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
        error = error ? error : Test_WriteToKML(kmlFile, "</StyleMap>");
        error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sn_cabs\">");
        error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <scale>0.4</scale>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <Icon>");
        error = error ? error : Test_WriteToKML(kmlFile, "          <href>http://maps.google.com/mapfiles/kml/shapes/cabs.png</href>");
        error = error ? error : Test_WriteToKML(kmlFile, "      </Icon>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
        error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
        error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sh_cabs\">");
        error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <scale>0.6</scale>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <Icon>");
        error = error ? error : Test_WriteToKML(kmlFile, "      <href>http://maps.google.com/mapfiles/kml/shapes/cabs.png</href>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </Icon>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
        error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
        error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
        error = error ? error : Test_WriteToKML(kmlFile, "  <Point>\n");        
        error = error ? error : Test_WriteToKML(kmlFile, "    <coordinates>");
        nsl_doubletostr(trafficSummary->coords[index].longitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, ",");
        nsl_doubletostr(trafficSummary->coords[index].latitude, buffer, sizeof(buffer));
        error = error ? error : Test_WriteToKML(kmlFile, buffer);
        error = error ? error : Test_WriteToKML(kmlFile, ",0");
        error = error ? error : Test_WriteToKML(kmlFile, "</coordinates>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "  </Point>\n");
        error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");
    }

    return error;
}

static PAL_Error
Test_DumpRouteBounds(PAL_File* kmlFile, NB_LatitudeLongitude *min, NB_LatitudeLongitude *max)
{
    PAL_Error error = PAL_Ok;    
    char buffer[64];

    error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <name>\n");        
    nsl_sprintf(buffer, "Route MIN Bound Point");
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, "  </name>\n");   

    error = error ? error : Test_WriteToKML(kmlFile, "<StyleMap id=\"msn_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <key>normal</key>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sn_cabs</styleUrl>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <key>highlight</key>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sh_cabs</styleUrl>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "</StyleMap>");
    error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sn_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <scale>0.4</scale>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "          <href>http://maps.google.com/mapfiles/ms/micons/orange.png</href>");
    error = error ? error : Test_WriteToKML(kmlFile, "      </Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
    error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sh_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <scale>0.6</scale>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <href>http://maps.google.com/mapfiles/ms/micons/orange.png</href>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Point>\n");        
    error = error ? error : Test_WriteToKML(kmlFile, "    <coordinates>");
    nsl_doubletostr(min->longitude, buffer, sizeof(buffer));
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, ",");
    nsl_doubletostr(min->latitude, buffer, sizeof(buffer));
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, ",0");
    error = error ? error : Test_WriteToKML(kmlFile, "</coordinates>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Point>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");    


    error = error ? error : Test_WriteToKML(kmlFile, "<Placemark>\n");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <name>\n");        
    nsl_sprintf(buffer, "Route MAX Bound Point");
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, "  </name>\n");   

    error = error ? error : Test_WriteToKML(kmlFile, "<StyleMap id=\"msn_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <key>normal</key>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sn_cabs</styleUrl>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <key>highlight</key>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <styleUrl>#sh_cabs</styleUrl>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Pair>");
    error = error ? error : Test_WriteToKML(kmlFile, "</StyleMap>");
    error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sn_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <scale>0.4</scale>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "          <href>http://maps.google.com/mapfiles/ms/micons/orange.png</href>");
    error = error ? error : Test_WriteToKML(kmlFile, "      </Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
    error = error ? error : Test_WriteToKML(kmlFile, "<Style id=\"sh_cabs\">");
    error = error ? error : Test_WriteToKML(kmlFile, "  <IconStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <scale>0.6</scale>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "      <href>http://maps.google.com/mapfiles/ms/micons/orange.png</href>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Icon>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </IconStyle>");        
    error = error ? error : Test_WriteToKML(kmlFile, "  <ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "  </ListStyle>");
    error = error ? error : Test_WriteToKML(kmlFile, "</Style>");
    error = error ? error : Test_WriteToKML(kmlFile, "  <Point>\n");        
    error = error ? error : Test_WriteToKML(kmlFile, "    <coordinates>");
    nsl_doubletostr(max->longitude, buffer, sizeof(buffer));
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, ",");
    nsl_doubletostr(max->latitude, buffer, sizeof(buffer));
    error = error ? error : Test_WriteToKML(kmlFile, buffer);
    error = error ? error : Test_WriteToKML(kmlFile, ",0");
    error = error ? error : Test_WriteToKML(kmlFile, "</coordinates>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  </Point>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "</Placemark>\n");    
      
    

    return error;
}

static void 
Test_DumpRouteToKML(PAL_Instance* pal, NB_Context* context, NB_RouteInformation* route, NB_GuidanceInformation* guidance, TestPositionSummary* navigationSummary, TestPositionSummary* trafficSummary, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max, const char* displayName, const char* kmlFileName, uint32 routeColor)
{
    PAL_Error error = PAL_Ok;
    PAL_File* kmlFile = NULL;
    
    error = error ? error : PAL_FileOpen(pal, kmlFileName, PFM_Create, &kmlFile);
    error = error ? error : Test_WriteToKML(kmlFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    error = error ? error : Test_WriteToKML(kmlFile, "<Document>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  <name>");
    error = error ? error : Test_WriteToKML(kmlFile, displayName);
    error = error ? error : Test_WriteToKML(kmlFile, "</name>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "  <open>1</open>\n");
    if (route)
    {
        error = error ? error : Test_WriteRoutePolylineToKML(kmlFile, route, routeColor);
        error = error ? error : Test_DumpManeuversToKML(kmlFile, route, guidance);
    }
    if (navigationSummary)
    {
        error = error ? error : Test_DumpNavigationPositionToKML(kmlFile, navigationSummary);
    }
    if (trafficSummary)
    {
        error = error ? error : Test_DumpTrafficNotifyRepliesToKML(kmlFile, trafficSummary);
    }
    if ( min && max )
    {
        error = error ? error : Test_DumpRouteBounds(kmlFile, min, max);
    }

    error = error ? error : Test_WriteToKML(kmlFile, "</Document>\n");
    error = error ? error : Test_WriteToKML(kmlFile, "</kml>\n");
    error = error ? error : PAL_FileClose(kmlFile);
}

/*! Return string for the current status.

This method is responsible for returning the appropriate string for the current status

@return String for the current status.

*/
char* GetStatusString(NB_NavigateStatus status)
{
    switch(status) 
    {
    case NB_NS_Invalid:
        return "NB_NS_Invalid";
    case NB_NS_Created:
        return "NB_NS_Created";
//    case NB_NS_InitialGPS:
//        return "NB_NS_InitialGPS";
    case NB_NS_InitialRoute:
        return "NB_NS_InitialRoute";
    case NB_NS_NavigatingStartup:
        return "NB_NS_NavigatingStartup";
    case NB_NS_Navigating:
        return "NB_NS_Navigating";
    case NB_NS_NavigatingArriving:
        return "NB_NS_NavigatingArriving";
    case NB_NS_Arrived:
        return "NB_NS_Arrived";
    case NB_NS_UpdatingRoute:
        return "NB_NS_UpdatingRoute";
//    case NB_NS_GettingDetour:
//        return "NB_NS_GettingDetour";
//    case NB_NS_RestartGPS:
//        return "NB_NS_RestartGPS";
//    case NB_NS_Paused:
//        return "NB_NS_Paused";
    case NB_NS_Error:
        return "NB_NS_Error";
//    case NB_NS_ConfirmRecalculate:
//        return "NB_NS_ConfirmRecalc";
//    case NB_NS_ConfirmDetour:
//        return "NB_NS_ConfirmDetour";
//    case NB_NS_NoDetour:
//        return "NB_NS_NoDetour";
    default:
        return "Unknown NB_NavigateStatus";
    }
}

/*! Return string for current maneuver position

@return String for the current maneuver position

*/
const char*
GetManeuverPostionString(NB_NavigateManeuverPos position)
{
    switch (position)
    {
    case NB_NMP_Invalid:
        return "NB_NMP_Invalid";
    case NB_NMP_Current:
        return "NB_NMP_Current";
    case NB_NMP_Continue:
        return "NB_NMP_Continue";
    case NB_NMP_ShowTurn:
        return "NB_NMP_ShowTurn";
    case NB_NMP_Prepare:
        return "NB_NMP_Prepare";
    case NB_NMP_Turn:
        return "NB_NMP_Turn";
    case NB_NMP_Past:
        return "NB_NMP_Past";
    case NB_NMP_Recalculate:
        return "NB_NMP_Recalculate";
    case NB_NMP_SoftRecalculate:
        return "NB_NMP_SoftRecalculate";
    case NB_NMP_RecalculateConfirm:
        return "NB_NMP_RecalculateConfirm";
    case NB_NMP_RecalculateTraffic:
        return "NB_NMP_RecalculateTraffic";
    case NB_NMP_TrafficCongestion:
        return "NB_NMP_TrafficCongestion";
    case NB_NMP_TrafficIncident:
        return "NB_NMP_TrafficIncident";
    case NB_NMP_TrafficAlert:
        return "NB_NMP_TrafficAlert";
    case NB_NMP_TrafficDelay:
        return "NB_NMP_TrafficDelay";
    case NB_NMP_Calc:
        return "NB_NMP_Calc";
    case NB_NMP_ConfirmDetour:
        return "NB_NMP_ConfirmDetour";
    default:
        return "Unknown NB_NavigateManeuverPos";
    }
}

/*! Callback called for enumerate text functions.

@return NE_OK if succesful

@see NavManeuversTextRerquest
*/
static void 
EnumerateTextCallback(NB_Font font, 
                      nb_color color, 
                      const char* txt, 
                      nb_boolean newline, 
                      void* pUser)
{
    if (txt && (nsl_strlen(txt) > 0))
    {
        //LOGOUTPUT(LogLevelHigh, ("EnumerateTextCallback: NBI_Font = %d, nb_color = %d, newline = %d, text: = %s \n", font, color, newline, txt));    
    }
    else
    {
        //LOGOUTPUT(LogLevelHigh, ("EnumerateTextCallback: No text supplied, NBI_Font = %d, nb_color = %d, newline = %d\n", font, color, newline));
    }

//    return NE_OK;
}


/*! Test to exercise detour parameters to ensure that there
are no memory leaks or invalid memory copies */
void TestNavigationDetourParameters(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    if (CreatePalAndContext(&pal, &context)) 
    {
        NB_DetourParameters* params = 0;
        NB_DetourParameters* clone = 0;

        CU_ASSERT_EQUAL(NB_DetourParametersCreate(context, NULL), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersCreate(NULL, NULL), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersCreate(NULL, &params), NE_INVAL);
        CU_ASSERT_PTR_NULL(params);

        CU_ASSERT_EQUAL(NB_DetourParametersCreate(context, &params), NE_OK);
        CU_ASSERT_PTR_NOT_NULL(params);

        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidEntireRoute(NULL, "Avoid Entire Route"), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidManeuvers(NULL, "Avoid Maneuvers 2 - 4", 2, 4), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidPartOfRoute(NULL, "Avoid Part of Route", 1200.21, 3345.98), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidTrafficIncident(NULL, "Avoid Incident", 1), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidCongestion(NULL, "Avoid Congestion"), NE_INVAL);

        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidEntireRoute(params, "Avoid Entire Route"), NE_OK);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidManeuvers(params, "Avoid Maneuvers 2 - 4", 2, 4), NE_OK);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidManeuvers(params, "Avoid Maneuvers 5 - 7", 5, 7), NE_OK);

        CU_ASSERT_EQUAL(NB_DetourParametersClone(params, NULL), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersClone(NULL, NULL), NE_INVAL);
        CU_ASSERT_EQUAL(NB_DetourParametersClone(NULL, &clone), NE_INVAL);
        CU_ASSERT_PTR_NULL(clone);

        CU_ASSERT_EQUAL(NB_DetourParametersClone(params, &clone), NE_OK);
        CU_ASSERT_PTR_NOT_NULL(clone);

        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidPartOfRoute(clone, "Avoid Part of Route", 1200.21, 3345.98), NE_OK);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidTrafficIncident(clone, "Avoid Incident", 1), NE_OK);
        CU_ASSERT_EQUAL(NB_DetourParametersAddAvoidCongestion(clone, "Avoid Congestion"), NE_OK);

        CU_ASSERT_EQUAL(NB_DetourParametersDestroy(NULL), NE_INVAL);

        CU_ASSERT_EQUAL(NB_DetourParametersDestroy(params), NE_OK);
        CU_ASSERT_EQUAL(NB_DetourParametersDestroy(clone), NE_OK);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void DumpTrafficInformation(NB_Navigation* navigation, const char* source)
{
    NB_TrafficInformation* trafficInfo = 0;
    NB_RouteInformation*  route = 0;
    uint32 endManeuver = 0;
    double meter = 0;
    uint32 incidents = 0;
    uint32 time = 0;
    uint32 delay = 0;
    uint32 maneuverIndex = 0;
    uint32 incidentIndex = 0;
    NB_Place incidentPlace = { { 0 } };
    NB_TrafficIncident incidentInfo = { 0 };
    double distance = 0;
    NB_Error result = NE_OK;

    LOGOUTPUT(LogLevelLow, ("\n----- Received traffic from %s -----\n", source));

    result = NB_NavigationGetInformation(navigation, &route, &trafficInfo, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelLow, ("  %d total incidents\n", NB_TrafficInformationGetIncidentCount(trafficInfo)));

    endManeuver = NB_RouteInformationGetManeuverCount(route);
    LOGOUTPUT(LogLevelLow, ("  %d maneuvers:\n", endManeuver));
    for (maneuverIndex = 0; maneuverIndex < endManeuver; maneuverIndex++)
    {
        result = NB_TrafficInformationGetSummary(trafficInfo, route, maneuverIndex, 0, maneuverIndex, &meter, &incidents, &time, &delay);
        LOGOUTPUT(LogLevelMedium, ("    maneuver: %d, meter: %f, incidents: %d, time: %d, delay: %d\n", maneuverIndex, meter, incidents, time, delay));
        CU_ASSERT_EQUAL(result, NE_OK);

        for (incidentIndex = 0; incidentIndex < incidents; incidentIndex++)
        {
            NB_TrafficInformationGetRouteManeuverIncident(trafficInfo, route, maneuverIndex, incidentIndex, &incidentPlace, &incidentInfo, &distance);
            CU_ASSERT_EQUAL(result, NE_OK);
            LOGOUTPUT(LogLevelMedium, ("      incident: %d - %s\n", incidentIndex, incidentInfo.description));
        }
    }

    incidents = NB_TrafficInformationGetIncidentCount(trafficInfo);
    LOGOUTPUT(LogLevelMedium, ("   %d incidents:\n", incidents));
    for (incidentIndex = 0; incidentIndex < incidents; incidentIndex++)
    {
        NB_TrafficInformationGetIncident(trafficInfo, incidentIndex, &incidentPlace, &incidentInfo, &distance);
        LOGOUTPUT(LogLevelMedium, ("    incident: %d, type: %d, severity: %d - %s\n", incidentIndex, incidentInfo.type, incidentInfo.severity, incidentInfo.description));
        LOGOUTPUT(LogLevelHigh, ("        Lat: %f, Lon: %f\n", incidentPlace.location.latitude, incidentPlace.location.longitude));
        LOGOUTPUT(LogLevelHigh, ("        Entered: %d, start: %d, end: %d, utz offset: %d\n", incidentInfo.entry_time, incidentInfo.start_time, incidentInfo.end_time, incidentInfo.utc_offset));
        LOGOUTPUT(LogLevelHigh, ("        Road: %s\n", incidentInfo.road));
        CU_ASSERT_EQUAL(result, NE_OK);
    }
}

void DumpCameraInformation(NB_Navigation* navigation)
{
    NB_CameraInformation* cameraInfo = 0;
    NB_RouteInformation*  route = 0;
    uint32 endManeuver = 0;
    uint32 maneuverIndex = 0;
    NB_Error result = NE_OK;
    uint32 cameraCount = 0;
    uint32 i=0;
    uint32 maneuverCameraCount = 0;

    LOGOUTPUT(LogLevelLow, ("\n----- Received speed camera info. -----\n"));

    result = NB_NavigationGetInformation(navigation, &route, NULL, &cameraInfo, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL_FATAL(cameraInfo);

    result = NB_CameraInformationGetCameraCount(cameraInfo, &cameraCount);
    CU_ASSERT_EQUAL(result, NE_OK);
    LOGOUTPUT(LogLevelLow, ("  %d total cameras \n", cameraCount));

    for (i=0; i < cameraCount; i++)
    {
        NB_Camera camera = {0};
        result = NB_CameraInformationGetCamera(cameraInfo, i, &camera);
        CU_ASSERT_EQUAL(result, NE_OK);
        LOGOUTPUT(LogLevelLow, ("   camera %d, distance=%f, speed=%f m/s, heading=%f isBidirectional:%d, status=%d, description:\"%s\"\n", i, camera.distance, camera.speedLimit, camera.heading, camera.isBidirectional, camera.status, camera.description));
    }

    endManeuver = NB_RouteInformationGetManeuverCount(route);
    LOGOUTPUT(LogLevelLow, ("  %d maneuvers:\n", endManeuver));
    for (maneuverIndex = 0; maneuverIndex < endManeuver; maneuverIndex++)
    {
        result = NB_CameraInformationGetManeuverCameraCount(cameraInfo, maneuverIndex, &maneuverCameraCount);
        CU_ASSERT_EQUAL(result, NE_OK);
        LOGOUTPUT(LogLevelMedium, ("    maneuver: %d, maneuverCameraCount: %d\n", maneuverIndex, maneuverCameraCount));

        for (i = 0; i < maneuverCameraCount; i++)
        {
            NB_Camera camera = {0};
            result = NB_CameraInformationGetManeuverCamera(cameraInfo, maneuverIndex, i, &camera);
            CU_ASSERT_EQUAL(result, NE_OK);
            LOGOUTPUT(LogLevelMedium, ("      camera: %d - distance %f, lat=%f, lon=%f \"%s\"\n", i, camera.routeDistance, camera.place.location.latitude, camera.place.location.longitude, camera.description));
        }
    }
}

void DumpPointsOfInterestAlongRoute(NB_Navigation* navigation)
{
    NB_Error result = NE_OK;
    NB_PointsOfInterestInformation* poiInfo = 0;

    LOGOUTPUT(LogLevelLow, ("\n----- Received POIs along route -----\n"));

    result = NB_NavigationGetInformation(navigation, NULL, NULL, NULL, &poiInfo);
    CU_ASSERT_EQUAL(result, NE_OK);

    if (!result && poiInfo)
    {
        NB_PositionRoutePositionInformation routePositionInfo = {0};
        int poiIndex = 0;
        int count = 0;
        double saveLatitude  = 0.0; 
        double saveLongitude = 0.0;

        result = NB_PointsOfInterestInformationGetPointOfInterestCount(poiInfo, &count);
        CU_ASSERT_EQUAL(result, NE_OK);
        
        result = NB_NavigationGetRoutePositionInformation(navigation, &routePositionInfo);
        CU_ASSERT_EQUAL(result, NE_OK);

        LOGOUTPUT(LogLevelLow, ("  %d total POIs along route  %f meters remaining in route\n", count, routePositionInfo.remainingRouteDist));
        for (poiIndex = 0; poiIndex < count; poiIndex++)
        {
            NB_PointOfInterest poi = {{{0}}};
            result = NB_PointsOfInterestInformationGetPointOfInterest(poiInfo, poiIndex, &poi);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (!result)
            {                
                if( poiIndex == 0 ) 
                {
                    LOGOUTPUT(LogLevelMedium, ("%d %f %s - %s\n", poiIndex, poi.routeDistanceRemaining, poi.place.name, poi.place.location.street1));
                }
                else
                {
                    double deltaDistance = 0.0;
                    double latitude      = poi.place.location.latitude; 
                    double longitude     = poi.place.location.longitude;
                    // Calculate distance between two points
                    deltaDistance = NB_SpatialGetLineOfSightDistance(latitude, longitude, saveLatitude, saveLongitude, NULL);
                    LOGOUTPUT(LogLevelMedium, ("%d %f %s - %s (spacing %f)\n", poiIndex, poi.routeDistanceRemaining, poi.place.name, poi.place.location.street1, deltaDistance ));
                    CU_ASSERT_TRUE(deltaDistance > POIS_SPACING_VALUE);
                }

                saveLatitude  = poi.place.location.latitude;
                saveLongitude = poi.place.location.longitude;
            }
        }
    }
}

static void
LogTrafficEvent(int logLevel, NB_TrafficEvent* trafficEvent)
{
    if (trafficEvent)
    {
        switch (trafficEvent->type)
        {
        case NB_TET_Congestion: LOGOUTPUT(logLevel, ("CONGESTION")); break;
        case NB_TET_Incident:   LOGOUTPUT(logLevel, ("INCIDENT"));   break;
        default:
        case NB_TET_None:       LOGOUTPUT(logLevel, ("NONE"));       break;
        }

        if (trafficEvent->type == NB_TET_Congestion || trafficEvent->type == NB_TET_Incident)
        {
            LOGOUTPUT(logLevel, (": man %d, to %f, after %f\n",
                trafficEvent->maneuverIndex, trafficEvent->distanceToEvent, trafficEvent->routeRemainAfterEvent));

            switch (trafficEvent->type)
            {
            case NB_TET_Congestion:
                LOGOUTPUT(logLevel, ("  severity "));
                switch (trafficEvent->detail.congestion.severity)
                {
                case NB_NTC_None:       LOGOUTPUT(logLevel, ("NONE"));      break;
                case NB_NTC_Moderate:   LOGOUTPUT(logLevel, ("MODERATE"));  break;
                case NB_NTC_Severe:     LOGOUTPUT(logLevel, ("SEVERE"));    break;
                default:
                case NB_NTC_Unknown:    LOGOUTPUT(logLevel, ("UNKNOWN"));   break;
                }
                LOGOUTPUT(logLevel, (", length %f, speed %f, delay %d\n  in congestion %d, reported time %d, road '%s'\n",
                    trafficEvent->detail.congestion.length, trafficEvent->detail.congestion.speed, trafficEvent->detail.congestion.delaySeconds,
                    trafficEvent->detail.congestion.inCongestion, trafficEvent->detail.congestion.updateGpsTimeSeconds,
                    trafficEvent->detail.congestion.road));
                break;

            case NB_TET_Incident:
                LOGOUTPUT(logLevel, ("  severity "));
                switch (trafficEvent->detail.incident.severity)
                {
                case NB_TrafficIncident_Severe:     LOGOUTPUT(logLevel, ("SEVERE"));        break;
                case NB_TrafficIncident_Major:      LOGOUTPUT(logLevel, ("MAJOR"));         break;
                case NB_TrafficIncident_Minor:      LOGOUTPUT(logLevel, ("MINOR"));         break;
                case NB_TrafficIncident_LowImpact:  LOGOUTPUT(logLevel, ("LOW IMPACT"));    break;
                }
                LOGOUTPUT(logLevel, (", type %d,  road '%s'\n  desc '%s'\n  lat %f, lon %f\n  entered %d, start %d, end %d utz offset: %d\n",
                    trafficEvent->detail.incident.type, trafficEvent->detail.incident.road, trafficEvent->detail.incident.description,
                    trafficEvent->place.location.latitude, trafficEvent->place.location.longitude,
                    trafficEvent->detail.incident.entry_time,  trafficEvent->detail.incident.start_time,
                    trafficEvent->detail.incident.end_time,  trafficEvent->detail.incident.utc_offset));
                break;
            }
        }
        else
        {
            LOGOUTPUT(logLevel, ("\n"));
        }
    }
}

static void
DumpNextUpcomingTrafficEvent(NB_Navigation* navigation)
{
    NB_Error result = NE_OK;
    NB_TrafficEvent trafficEvent = { 0 };
    NB_PositionRoutePositionInformation positionInformation = { 0 };
    NB_TrafficState* trafficState = NULL;
    double incidentRouteRemain = 0.0;
    double distToIncident = 0.0;

    result = NB_NavigationGetTrafficState(navigation, &trafficState);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(trafficState);
    result = NB_TrafficProcessorGetNextIncident(trafficState, NULL, NULL, &incidentRouteRemain, &distToIncident);
    CU_ASSERT_EQUAL(result, NE_OK);

    result = NB_NavigationGetRoutePositionInformation(navigation, &positionInformation);
    CU_ASSERT_EQUAL(result, NE_OK);

    LOGOUTPUT(LogLevelHigh, ("ROUTE: dist remain %f\n", positionInformation.remainingRouteDist));
    LOGOUTPUT(LogLevelHigh, ("Next Incident: dist to: %f route remain : %f\n", distToIncident, incidentRouteRemain));

    result = NB_NavigationGetNextTrafficEvent(navigation, &trafficEvent);
    CU_ASSERT(result == NE_OK || result == NE_NOENT);

    LogTrafficEvent(LogLevelHigh, &trafficEvent);
}

static void
DumpTrafficEventInformationForEachManeuver(NB_Navigation* navigation, const char* source)
{
    NB_Error result = NE_OK;
    NB_RouteInformation* routeInformation = 0;
    NB_TrafficInformation* trafficInformation = 0;

    uint32 maneuverCount = 0;
    uint32 maneuverIndex = 0;

    result = NB_NavigationGetInformation(navigation, &routeInformation, &trafficInformation, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeInformation);
    CU_ASSERT_PTR_NOT_NULL(trafficInformation);

    LOGOUTPUT(LogLevelMedium, ("\n===== Traffic event by maneuver (%s) =====\n", source));

    maneuverCount = NB_RouteInformationGetManeuverCount(routeInformation);
    for (maneuverIndex = 0; maneuverIndex < maneuverCount; maneuverIndex++)
    {
        #define _SIZE 1024
        char primaryManeuver[_SIZE] = {0};
        char secondaryManeuver[_SIZE] = {0};
        char currentRoad[_SIZE] = {0};
        double distance = 0.0;

        NB_TrafficEvent trafficEvent = { 0 };
        double meter = 0.0;
        uint32 time = 0;
        uint32 delay = 0;

        result = NB_RouteInformationGetTurnInformation(
            routeInformation,
            maneuverIndex, 
            NULL,
            primaryManeuver, 
            _SIZE, 
            secondaryManeuver, 
            _SIZE, 
            currentRoad, 
            _SIZE, 
            &distance, 
            FALSE);
        CU_ASSERT(result == NE_OK);

        result = NB_NavigationGetManeuverTrafficSummary(
            navigation,
            maneuverIndex,
            0,
            maneuverIndex + 1,
            &trafficEvent,
            &meter,
            &time,
            &delay);
        CU_ASSERT(result ==  NE_OK || result == NE_NOENT);

        LOGOUTPUT(LogLevelMedium, ("maneuver: %d, \"%s\", \"%s\", \"%s\" distance %f",
                  maneuverIndex,
                  primaryManeuver,
                  secondaryManeuver,
                  currentRoad,
                  distance));
        LOGOUTPUT(LogLevelMedium, (" meter: %f, time: %d, delay: %d\n  next event: ",
            meter, time, delay));
        LogTrafficEvent(LogLevelMedium, &trafficEvent);
    }
    LOGOUTPUT(LogLevelHigh, ("\n"));
}

static void
DumpTrafficEventList(NB_Navigation* navigation, const char* source)
{
    NB_Error result = NE_OK;
    NB_TrafficInformation* trafficInformation = 0;
    NB_RouteInformation* routeInformation = 0;

    NB_TrafficEvent* trafficEventList = 0;
    uint32 trafficEventCount = 0;
    uint32 eventIndex = 0;

    result = NB_NavigationGetInformation(navigation, &routeInformation, &trafficInformation, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(routeInformation);
    CU_ASSERT_PTR_NOT_NULL(trafficInformation);

    LOGOUTPUT(LogLevelMedium, ("\n===== Traffic event list (%s) =====\n", source));

    result = NB_NavigationGetUpcomingTrafficEvents(navigation, &trafficEventCount, &trafficEventList);
    CU_ASSERT(result == NE_OK || result == NE_NOENT);

    if (trafficEventCount)
    {
        for (eventIndex = 0; eventIndex < trafficEventCount; eventIndex++)
        {
            LOGOUTPUT(LogLevelMedium, ("event %d: ", eventIndex));
            LogTrafficEvent(LogLevelMedium, &trafficEventList[eventIndex]);
        }

        NB_NavigationFreeTrafficEvents(trafficEventList);
    }
    else
    {
        LOGOUTPUT(LogLevelMedium, ("No upcoming events\n"));
        CU_ASSERT_EQUAL(trafficEventList, 0);
    }

    LOGOUTPUT(LogLevelHigh, ("\n"));
}

void SetupNavigationConfiguration(NB_NavigationConfiguration* configuration)
{
    configuration->gpsFixMinimumSpeed = 1.0;
    configuration->startupRegionSizeFactor = 1.5;
    configuration->filteredGpsMinimumSpeed = 5.5;
    configuration->filteredGpsConstant = 0.97;
    configuration->trafficNotifyMaximumRetries = 3;
    configuration->trafficNotifyDelay = 15 * 1000;
    configuration->completeRouteMaximumAttempts = 2;
    configuration->completeRouteRequestDelay = 5 * 1000;
    configuration->pronunRequestDelay = 15 * 1000;
    configuration->cameraSearchDelay = 15 * 1000;
    configuration->cameraSearchDistance = 99999999;
    configuration->startupRegionMinimumSize = 150;
    configuration->excessiveRecalcMaximumCount = 3;
    configuration->excessiveRecalcTimePeriod = 120;
    configuration->filteredGpsEnabled = FALSE;
    configuration->cameraSearchEnabled = TRUE;
}

void LogRouteError(NB_Navigation* navigation, char filename[], unsigned int line)
{
    NB_NavigateRouteError routeError = NB_NRE_None;
    NB_Error navError = NE_OK;
    char temp[200] = { 0 };

    CU_ASSERT_EQUAL(NB_NavigationGetErrors(navigation, &routeError, &navError), NE_OK);

    if (routeError > 0 && routeError < (sizeof(routingErrors) / sizeof(routingErrors[0])))
    {
        nsl_sprintf(temp, "Route error: %s, nav error: %d", routingErrors[routeError], navError);
    }
    else
    {
        nsl_sprintf(temp, "Route error: %d, nav error: %d", routeError, navError);
    }

    CU_assertImplementation(CU_FALSE, line, temp, filename, "", CU_FALSE);
}

/*! @} */
