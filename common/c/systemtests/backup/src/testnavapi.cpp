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

    @file       testnavapi.cpp

    See header file for description.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palmath.h"

extern "C"
{
    #include "testnavapi.h"
    #include "main.h"
    #include "platformutil.h"
    #include "testnetwork.h"
    #include "nbgpstypes.h"
    #include "palclock.h"
    #include "utility.h"
    #include "nbgpshistory.h"
    #include "nbcontextaccess.h"
    #include "palfile.h"
}

#include "NavApiNavigation.h"
#include "NavApiListeners.h"
#include "NavApiTypes.h"
#include "NavApiRouteRequest.h"
#include "NavApiRouteSummaryRequest.h"

using namespace nbnav;
static const char WORK_DESTINATION_GPS_FILE[] = "work_destination.gps";
static const double WORK_DESTINATION_GPS_ARRIVED_LAT = 33.673418;
static const double WORK_DESTINATION_GPS_ARRIVED_LON = -117.847016;
static const uint32 WORK_DESTINATION_GPS_BEGIN_OFFSET = 793729896;
static const uint32 WORK_DESTINATION_GPS_END_OFFSET = 793731139;
static const double WORK_ORIGIN_GPS_ARRIVED_LAT = 33.603841;
static const double WORK_ORIGIN_GPS_ARRIVED_LON = -117.690530;

class TestLocation : public Location
{
public:
    TestLocation(NB_GpsLocation* gpsLocation)
    {
        nsl_memcpy(&m_gpsLocation, gpsLocation, sizeof(NB_GpsLocation));
    }
    virtual ~TestLocation() {};
    virtual double Altitude() const { return m_gpsLocation.altitude; }
    virtual unsigned int GpsTime() const { return m_gpsLocation.gpsTime; }
    virtual double Heading() const { return m_gpsLocation.heading; }
    virtual double HorizontalUncertaintyAlongAxis() const { return m_gpsLocation.horizontalUncertaintyAlongAxis; }
    virtual double HorizontalUncertaintyAlongPerpendicular() const { return m_gpsLocation.horizontalUncertaintyAlongPerpendicular; }
    virtual double HorizontalUncertaintyAngleOfAxis() const { return m_gpsLocation.horizontalUncertaintyAngleOfAxis; }
    virtual double HorizontalVelocity() const { return m_gpsLocation.horizontalVelocity; }
    virtual double Latitude() const { return m_gpsLocation.latitude; }
    virtual double Longitude() const { return m_gpsLocation.longitude; }
    virtual int NumberOfSatellites() const { return m_gpsLocation.numberOfSatellites; }
    virtual int UtcOffset() const { return 0; }
    virtual unsigned int Valid() const { return m_gpsLocation.valid; }
    virtual double VerticalUncertainty() const { return m_gpsLocation.verticalUncertainty; }
    virtual double VerticalVelocity() const { return m_gpsLocation.verticalVelocity; }
    virtual bool IsGpsFix() const { return m_gpsLocation.horizontalUncertaintyAlongAxis <= 50; }
private:
    NB_GpsLocation m_gpsLocation;
};

// Listeners

class TestRoutePositionListener : public RoutePositionListener
{
public:
    bool isLaneGuidanceEnabledComming;
    bool isLaneGuidanceDisabledComming;
    bool isRealisticSignEnabledComming;
    bool isRealisticSignDisabledComming;

    TestRoutePositionListener() : isLaneGuidanceEnabledComming(false),
                                  isLaneGuidanceDisabledComming(false),
                                  isRealisticSignEnabledComming(false),
                                  isRealisticSignDisabledComming(false)
    {
    }

    void TripRemainingTime(int time);
    void TripRemainingDelay(int delay);
    void TripRemainingDistance(double distance);
    void PositionUpdated(const Coordinates& coordinates, double speed, double heading);
    void ManeuverRemainingTime(int time);
    void ManeuverRemainingDelay(int delay);
    void ManeuverRemainingDistance(double distance);
    void CurrentRoadName(string primaryName, string secondaryName);
    void ManeuverExitNumber(string exitNumber);
    void NextRoadName(string primaryName, string secondaryName);
    void ManeuverType(string type);
    void ManeuverImageId(string imageId);
    void StackTurnImageTTF(string stackImageTTF);
    void ManeuverPoint(const Coordinates& point);
    void SpeedLimit(const SpeedLimitInformation& speedLimitInfo);
    void DisableSpeedLimit();
    void RoadSign(const nbnav::RoadSign& signInfo);
    void DisableRoadSign();
    void LaneInformation(const nbnav::LaneInformation& laneInfo);
    void DisableLaneInformation();
    void UpdateManeuverList(const ManeuverList& maneuvers);

    void TestLaneGetHighlighted(const nbnav::LaneInformation& laneInfo);
    void TestLaneGetNonHighlighted(const nbnav::LaneInformation& laneInfo);
    void TestLaneInfoLanes(const nbnav::LaneInformation& laneInfo);
    void TestLaneInfoSelectedLanePosition(const nbnav::LaneInformation& laneInfo);
    void TestLaneInfoSelectedLanes(const nbnav::LaneInformation& laneInfo);

    void TestRoadSignGetImageData(const nbnav::RoadSign& signInfo);

    void TestPreferencesSpeedLimitAlert(const SpeedLimitInformation& speedLimitInfo);
    void TestPreferencesSpeedLimitSign(const SpeedLimitInformation& speedLimitInfo);

    void TestSpeedLimitInfoCurrentSpeed(const SpeedLimitInformation& speedLimitInfo);
    void TestSpeedLimitInfoInformation(const SpeedLimitInformation& speedLimitInfo);
    void TestSpeedLimitInfoIsWarning(const SpeedLimitInformation& speedLimitInfo);
    void TestSpeedLimitInfoSpeedLimit(const SpeedLimitInformation& speedLimitInfo);
    void TestSpeedLimitInfoType(const SpeedLimitInformation& speedLimitInfo);
};

class TestAnnouncementListener : public AnnouncementListener
{
public:
    void Announce(shared_ptr<Announcement> announcement);

    void TestPreferencesSchoolZoneAlert(shared_ptr<Announcement> announcement);
    void TestAnnouncementGetAudio(shared_ptr<Announcement> announcement);
    void TestAnnouncementGetText(shared_ptr<Announcement> announcement);
    void TestPreferencesTrafficAnncEnabled(shared_ptr<Announcement> announcement);
};


class TestSessionListener : public SessionListener
{
public:
    TestSessionListener(Navigation* _navigation):navigation(_navigation) {}
    void OffRoute();
    void OnRoute();
    void RouteReceived(SessionListener::RouteRequestReason reason, vector<RouteInformation>& routes);
    void RouteRequested(RouteRequestReason reason);
    void RouteProgress(int progress);
    void RouteError(NavigateRouteError error);
    void ArrivingToDestination(DestinationStreetSide direction);
    void RouteFinish();
    void OffRoutePositionUpdate(double headingToRoute);

    void TestPreferencesOffRouteIgnoreCount(int reason, vector<RouteInformation>& routes);
    void TestNavigationPauseSession();
    void TestNavigationResumeSession();
private:
    Navigation* navigation;
};


class TestTrafficListener : public TrafficListener
{
public:
    void TrafficAlerted(const TrafficEvent& trafficEvent);
    void TrafficChanged(const TrafficInformation& trafficInfo);
    void DisableTrafficAlerted();
};

class TestPositionUpdateListener : public PositionUpdateListener
{
public:
    void PositionUpdate(int maneuverRemainingDelay, double maneuverRemainingDistance,
            int maneuverRemainingTime, Coordinates coordinates,
            int speed, int heading, int tripRemainingDelay,
            double tripRemainingDistance, int tripRemainingTime);
};

class TestManeuverUpdateListener : public ManeuverUpdateListener
{
public:
    void ManeuverUpdate(int maneuverID, string currentRoadPrimaryName, string currentRoadSecondaryName,
            string maneuverExitNumber, string maneuverImageId,
            Coordinates maneuverPoint, string maneuverType,
            string nextRoadPrimaryName, string nextRoadSecondaryName,
            string maneuverStackTurnImageTTF);

    void TestPreferencesNeedNC(string maneuverType);
    void TestPreferencesBridgeManeuverEnabled(string maneuverType);
};

class TestNavEventListener : public NavEventListener
{
public:
    bool isLaneGuidanceEnabledComming;
    bool isLaneGuidanceDisabledComming;
    bool isRealisticSignEnabledComming;
    bool isRealisticSignDisabledComming;

    TestNavEventListener() : isLaneGuidanceEnabledComming(false),
                             isLaneGuidanceDisabledComming(false),
                             isRealisticSignEnabledComming(false),
                             isRealisticSignDisabledComming(false)
    {
    }

    void NavEvent(NavEventTypeMask navEventTypeMask, const LaneInformation& laneInfo,
            const RoadSign& signInfo, const SpeedLimitInformation& speedLimitInfo,
            const TrafficEvent& trafficEvent, const TrafficInformation& trafficInfo);
};

class TestRouteListener : public RouteListener
{
public:
    virtual void OnRoute(vector<RouteInformation>& information, RouteRequest* request);

    void TestRouteInfoGetRouteProperties(vector<RouteInformation>& information);
    void TestRouteInfoGetBoundingBox(vector<RouteInformation>& information);
    void TestRouteInfoGetDelay(vector<RouteInformation>& information);
    void TestRouteInfoGetDestination(vector<RouteInformation>& information);
    void TestRouteInfoGetDistance(vector<RouteInformation>& information);
    void TestRouteInfoGetInitialGuidanceText(vector<RouteInformation>& information);
    void TestRouteInfoGetManeuverList(vector<RouteInformation>& information);
    void TestRouteInfoGetOrigin(vector<RouteInformation>& information);
    void TestRouteInfoGetPolyline(vector<RouteInformation>& information);
    void TestRouteInfoGetRouteDescription(vector<RouteInformation>& information);
    void TestRouteInfoGetRouteError(vector<RouteInformation>& information);
    void TestRouteInfoGetRouteID(vector<RouteInformation>& information);
    void TestRouteInfoGetTime(vector<RouteInformation>& information);
    void TestRouteInfoGetVoiceFile(vector<RouteInformation>& information);

    void TestManeueverGetRoutingIcon(vector<RouteInformation>& information);
    void TestManeuverGetBbx(vector<RouteInformation>& information);
    void TestManeuverGetCommand(vector<RouteInformation>& information);
    void TestManeuverGetDescription(vector<RouteInformation>& information);
    void TestManeuverGetDistance(vector<RouteInformation>& information);
    void TestManeuverGetFormattedDesc(vector<RouteInformation>& information);
    void TestManeuverGetManeuverCode(vector<RouteInformation>& information);
    void TestManeuverGetManeuverID(vector<RouteInformation>& information);
    void TestManeuverGetPoint(vector<RouteInformation>& information);
    void TestManeuverGetPolyline(vector<RouteInformation>& information);
    void TestManeuverGetRoutingTTF(vector<RouteInformation>& information);
    void TestManeuverGetTime(vector<RouteInformation>& information);
    void TestManeuverGetTrafficDelay(vector<RouteInformation>& information);
    void TestManeuverListGetDestination(vector<RouteInformation>& information);
    void TestManeuverListGetManeuver(vector<RouteInformation>& information);
    void TestManeuverListGetNumberOfMans(vector<RouteInformation>& information);
    void TestManeuverListGetOrigin(vector<RouteInformation>& information);
    void TestManeuverListGetRouteBbx(vector<RouteInformation>& information);
    void TestManeuverListGetTotalDelay(vector<RouteInformation>& information);
    void TestManeuverListGetTotalDistance(vector<RouteInformation>& information);
    void TestManeuverListGetTotalTripTime(vector<RouteInformation>& information);
};

class TestRouteSummaryListener : public RouteSummaryListener
{
public:
    virtual void OnRouteSummary(RouteSummaryInformation& information, RouteSummaryRequest* request);
};

class TestSpecialRegionListener : public SpecialRegionListener
{
public:
    virtual void SpecialRegion(const SpecialRegionInformation& specialRegionInfo);
    virtual void DisableSpecialRegion(const SpecialRegionInformation& specialRegionInfo);

    void TestSpecialRegionInfoGetDescription(const SpecialRegionInformation& specialRegionInfo);
    void TestSpecialRegionInfoGetImage(const SpecialRegionInformation& specialRegionInfo);
    void TestSpecialRegionInfoGetNextOccurrenceEndTime(const SpecialRegionInformation& specialRegionInfo);
    void TestSpecialRegionInfoGetNextOccurrenceStartTime(const SpecialRegionInformation& specialRegionInfo);
    void TestSpecialRegionInfoGetRegionId(const SpecialRegionInformation& specialRegionInfo);
    void TestSpecialRegionInfoGetType(const SpecialRegionInformation& specialRegionInfo);
};

typedef enum ListenerTypes
{
    lRouteRequested = 0,
    lAnnounceGettingRoute,
    lRouteProgress,
    lRouteReceived,
    lTrafficChanged,
    lRoadSign,
    lCurrentRoadName,
    lManeuverExitNumber,
    lManeuverImageId,
    lManeuverType,
    lManeuverPoint,
    lNextRoadName,
    lStackTurnImageTTF,
    lAnnounce,
    lPositionUpdated,
    lTripRemainingTime,
    lTripRemainingDelay,
    lTripRemainingDistance,
    lManeuverRemainingTime,
    lManeuverRemainingDelay,
    lManeuverRemainingDistance,
    lOnOffRoute,
    lFinishTest
} ListenerTypes;


// Local Constants ..............................................................................


/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;
    NB_Context* context;
    void* event;
    bool routeFinish;
    ListenerTypes listenerType; // for TestListenersInvocationOrder
} Instances;

// Variables .....................................................................................

static Instances g_instances = {0};

static bool bifNKGPS = false;
static bool bifNK_SARGPS = false;
static bool bifNK_SPDLGPS = false;
static bool bifNK_RECALCGPS = false;
static bool bifNK_CRIGPS = false;
static bool bifNK_SECGPS = false;
static bool bifNK_NGGPS =false;
static bool bifNK_trafficGPS = false;
static bool bifNK_BEGPS = false;

// System test functions..........................................................................
static void TestNavigationCreate(void);
static void TestNavigationSessionCreate(void);
static void TestListenersInvocationOrder(void);
static void TestAdditionalListeners(void);
static void TestNavigationSessionWithSingleRoute(void);
static void TestNavigationSessionWithMultipleRoute(void);
static void TestRouteRequest(void);
static void TestRouteSummaryRequest(void);
static void TestSpecialRegion(void);
static void TestLaneGuidance(void);
static void TestRealisticSign(void);

static void TestNKGPS(void);
static void TestNK_SARGPS(void);
static void TestNK_SPDLGPS(void);
static void TestNK_RECALCGPS(void);
static void TestNK_CRIGPS(void);
static void TestNK_SECGPS(void);
static void TestNK_NGGPS(void);
static void TestNK_TrafficGPS(void);
static void TestNK_BEGPS(void);

// Local Functions ...............................................................................
static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);
static NB_GpsLocation* TestGPSFileGetNextFix(TestGPSFile* gps, int delayMilliseconds);
static TestGPSFile* TestGPSFileLoad(PAL_Instance* pal, const char* gpsFileName);
static void TestGPSFileDestroy(TestGPSFile* gps);
static Preferences GetDefaultPreferences();
static RouteOptions GetDefaultRouteOptions();
static Place GetDefaultDestination(int index, const char** gpsFileName, uint32* gpsBeginOffset, uint32* gpsEndOffset);
static Place GetDefaultOrigin();
static TestGPSFile* PrepareGps(const char* gpsFileName, uint32 gpsBeginOffset, uint32 gpsEndOffset);
static void CheckAndIncrementListener(ListenerTypes current);
static void WaitForRelease();

static bool valHighlighted(vector<nbnav::Lane> lanes);
static bool valNonHighlighted(vector<nbnav::Lane> lanes);

static Place GetDefaultDestination(int index, const char** gpsFileName, uint32* gpsBeginOffset, uint32* gpsEndOffset)
{
    MapLocation mapLocation;
    Place destination;
    if (index == 0)
    {
        mapLocation.airport          = "LAX";
        mapLocation.number           = "1";
        mapLocation.street           = "World Way";
        mapLocation.city             = "Los Angeles";
        mapLocation.state            = "CA";
        mapLocation.postal           = "90045";
        mapLocation.center.latitude  = WORK_DESTINATION_GPS_ARRIVED_LAT;
        mapLocation.center.longitude = WORK_DESTINATION_GPS_ARRIVED_LON;
        *gpsFileName                  = WORK_DESTINATION_GPS_FILE;
        *gpsBeginOffset              = WORK_DESTINATION_GPS_BEGIN_OFFSET;
        *gpsEndOffset                = WORK_DESTINATION_GPS_END_OFFSET;

        destination.SetLocation(mapLocation);
    }
    else if (index == 1)
    {
        // quick destination
        mapLocation.center.latitude  = 33.644359;
        mapLocation.center.longitude = -117.734385;
        *gpsFileName                  = WORK_DESTINATION_GPS_FILE;
        *gpsBeginOffset              = WORK_DESTINATION_GPS_BEGIN_OFFSET;
        *gpsEndOffset                = WORK_DESTINATION_GPS_END_OFFSET;
    }
    //else if (index == 2) //You could add the different destinations here
    return destination;
}

Place GetDefaultOrigin()
{
    MapLocation mapLocation;
    Place origin;
    mapLocation.center.latitude  = WORK_ORIGIN_GPS_ARRIVED_LAT;
    mapLocation.center.longitude = WORK_ORIGIN_GPS_ARRIVED_LON;
    origin.SetLocation(mapLocation);
    return origin;
}

static Preferences GetDefaultPreferences(uint32* gpsUpdateTime)
{
    Preferences preferences;
    preferences.SetOffRouteIgnoreCount(3);
    uint32 updateRate = 0; // if it is not 0 then gpsUpdateTime should be 1-3 sec
    *gpsUpdateTime = updateRate ? 1000 : 10;

    preferences.SetPositionUpdateRate(updateRate);
    preferences.SetRecalcCriteria(Preferences::Relaxed);
    preferences.SetDownloadableAudioEnabled(true);

    return preferences;
}
static RouteOptions GetDefaultRouteOptions()
{
   RouteOptions routeOptions(Fastest, Car, 0, "female-6-aac-v5");
   return routeOptions;
}

static TestGPSFile* PrepareGps(const char* gpsFileName, uint32 gpsBeginOffset, uint32 gpsEndOffset)
{
    NB_GpsHistory* history = NB_ContextGetGpsHistory(g_instances.context);
    TestGPSFile* gpsSource = TestGPSFileLoad(g_instances.pal, gpsFileName);
    NB_GpsLocation* gpsFix = NULL;

    // Add the fixes up to the start time into the GPS history
    while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, 0)) != NULL) && (gpsFix->gpsTime <= gpsBeginOffset))
    {
        if (gpsFix->status == NE_OK)
        {
            NB_Error result = NB_GpsHistoryAdd(history, gpsFix);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }
    return gpsSource;
}

extern "C" void TestNavApi_AddTests(CU_pSuite pTestSuite, int level)
{
    //TODO: Add TestNavigationSessionWithTTS to test Preferences.SetDownloadableAudioEnabled(false)
    //CU_add_test(pTestSuite, "TestNavigationSessionWithTTS", TestNavigationSessionWithTTS);

    //TODO: Add TestNavigationSessionWithDownloadableAudio to test Preferences.SetDownloadableAudioEnabled(true)
    //CU_add_test(pTestSuite, "TestNavigationSessionWithDownloadableAudio", TestNavigationSessionWithDownloadableAudio);

    //TODO: Add TestNavigationSessionWithOffRoute to test SessionListener.OffRoute() and SessionListener.OnRoute()
    //CU_add_test(pTestSuite, "TestNavigationSessionWithOffRoute", TestNavigationSessionWithOffRoute);

    //TODO: Add TestNavigationSessionWithDetour to test Navigation.SetActiveRoute()
    //CU_add_test(pTestSuite, "TestNavigationSessionWithDetour", TestNavigationSessionWithDetour);

    //TODO: Add TestNavigationSessionWithInvalidDestination
    //CU_add_test(pTestSuite, "TestNavigationSessionWithInvalidDestination", TestNavigationSessionWithInvalidDestination);

    //TODO: Add TestNavigationSessionWithInvalidGpsFixes
    //CU_add_test(pTestSuite, "TestNavigationSessionWithInvalidGpsFixes", TestNavigationSessionWithInvalidGPSFixes);

    //TODO: Add TestNavigationSessionWithInvalidPreferences
    //CU_add_test(pTestSuite, "TestNavigationSessionWithInvalidPreferences", TestNavigationSessionWithInvalidPreferences);

    //TODO: Add TestNavigationSessionWithSuspendAndResume
    //CU_add_test(pTestSuite, "TestNavigationSessionWithSuspendAndResume", TestNavigationSessionWithSuspendAndResume);

    CU_add_test(pTestSuite, "TestNavigationCreate", TestNavigationCreate);
    CU_add_test(pTestSuite, "TestListenersInvocationOrder", TestListenersInvocationOrder);
    CU_add_test(pTestSuite, "TestNavigationSessionCreate", TestNavigationSessionCreate);
    CU_add_test(pTestSuite, "TestNavigationSessionWithSingleRoute", TestNavigationSessionWithSingleRoute);
    CU_add_test(pTestSuite, "TestNavigationSessionWithMultipleRoute", TestNavigationSessionWithMultipleRoute);
    CU_add_test(pTestSuite, "TestAdditionalListeners", TestAdditionalListeners);
    CU_add_test(pTestSuite, "TestLaneGuidance", TestLaneGuidance);
    CU_add_test(pTestSuite, "TestRealisticSign", TestRealisticSign);
    CU_add_test(pTestSuite, "TestRouteRequest", TestRouteRequest);
    CU_add_test(pTestSuite, "TestRouteSummaryRequest", TestRouteSummaryRequest);
    CU_add_test(pTestSuite, "TestSpecialRegion", TestSpecialRegion);

    CU_add_test(pTestSuite, "TestNKGPS", TestNKGPS);
    CU_add_test(pTestSuite, "TestNK_SARGPS", TestNK_SARGPS);
    CU_add_test(pTestSuite, "TestNK_SPDLGPS", TestNK_SPDLGPS);
    CU_add_test(pTestSuite, "TestNK_RECALCGPS", TestNK_RECALCGPS);
    CU_add_test(pTestSuite, "TestNK_CRIGPS", TestNK_CRIGPS);
    CU_add_test(pTestSuite, "TestNK_SECGPS", TestNK_SECGPS);
    CU_add_test(pTestSuite, "TestNK_NGGPS", TestNK_NGGPS);
    CU_add_test(pTestSuite, "TestNK_TrafficGPS", TestNK_TrafficGPS);
    CU_add_test(pTestSuite, "TestNK_BEGPS", TestNK_BEGPS);
}

// Cleanup for suite
extern "C" int TestNavApi_SuiteSetup(void)
{
    return 0;
}

extern "C" int TestNavApi_SuiteCleanup(void)
{
    return 0;
}


void
TestNavigationCreate(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        CU_ASSERT(g_instances.pal != NULL && g_instances.context != NULL);

        Place destination;
        RouteOptions routeOptions = GetDefaultRouteOptions();
        Preferences preferences;

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, routeOptions, preferences);

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }
        delete navigation;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestNavigationSessionCreate(void)
{
/*
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        TestGPSFile* gpsSource = PrepareGps(gpsFileName, gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* rpl = new TestRoutePositionListener();
        TestTrafficListener* trl = new TestTrafficListener();
        TestAnnouncementListener* tal = new TestAnnouncementListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(rpl);
        navigation->AddTrafficListener(trl);
        navigation->AddAnnouncementListener(tal);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            int loopCount = 0;

            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                // Reset the event so we can properly capture the next status change
                ResetCallbackCompletedEvent(g_instances.event);

                // Allow async processing (pump messages)
                if ((++loopCount % 50) == 0)
                {
                    WaitForCallbackCompletedEvent(g_instances.event, 1);
                }
                if (loopCount == 50)
                {
                    navigation->DoDetour();
                }
                else if (loopCount == 100)
                {
                    navigation->PauseSession();
                }
                else if (loopCount == 110)
                {
                    navigation->ResumeSession();
                }
                else if (loopCount == 120)
                {
                    navigation->StopSession();
                }
                if (g_instances.routeFinish)
                {
                    break;
                }
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        delete navigation;
        delete tsl;
        delete rpl;
        delete trl;
        delete tal;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }*/
}

void
TestListenersInvocationOrder(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        TestGPSFile* gpsSource = PrepareGps(gpsFileName, gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;

        Navigation* navigation =
            Navigation::GetNavigation(g_instances.context, destination,
                                      GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* rpl = new TestRoutePositionListener();
        TestTrafficListener* trl = new TestTrafficListener();
        TestAnnouncementListener* tal = new TestAnnouncementListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(rpl);
        navigation->AddTrafficListener(trl);
        navigation->AddAnnouncementListener(tal);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        g_instances.listenerType = lRouteRequested;
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);

                if (g_instances.listenerType == lFinishTest)
                {
                    break;
                }
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveRoutePositionListener(rpl);
        navigation->RemoveTrafficListener(trl);
        navigation->RemoveAnnouncementListener(tal);
        delete navigation;
        delete tsl;
        delete rpl;
        delete trl;
        delete tal;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestAdditionalListeners(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        TestGPSFile* gpsSource = PrepareGps(gpsFileName, gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestPositionUpdateListener* tpul = new TestPositionUpdateListener();
        TestNavEventListener* tnel = new TestNavEventListener();
        TestManeuverUpdateListener* tmul = new TestManeuverUpdateListener();

        navigation->AddSessionListener(tsl);
        navigation->AddPositionUpdateListener(tpul);
        navigation->AddNavEventListener(tnel);
        navigation->AddManeuverUpdateListener(tmul);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        navigation->UpdatePosition(location);
        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemovePositionUpdateListener(tpul);
        navigation->RemoveNavEventListener(tnel);
        navigation->RemoveManeuverUpdateListener(tmul);

        delete navigation;
        delete tsl;
        delete tpul;
        delete tnel;
        delete tmul;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestNavigationSessionWithSingleRoute(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        TestGPSFile* gpsSource = PrepareGps(gpsFileName, gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;
        Preferences preferences = GetDefaultPreferences(&gpsUpdateTime);
        preferences.SetMultipleRoutes(false);

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), preferences);

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* rpl = new TestRoutePositionListener();
        TestTrafficListener* trl = new TestTrafficListener();
        TestAnnouncementListener* tal = new TestAnnouncementListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(rpl);
        navigation->AddTrafficListener(trl);
        navigation->AddAnnouncementListener(tal);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        g_instances.listenerType = lRouteRequested;
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            int loop = 0;
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);

                if (loop == 50)
                {
                    break;
                }
                loop++;
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveRoutePositionListener(rpl);
        navigation->RemoveTrafficListener(trl);
        navigation->RemoveAnnouncementListener(tal);
        delete navigation;
        delete tsl;
        delete rpl;
        delete trl;
        delete tal;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestNavigationSessionWithMultipleRoute(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        TestGPSFile* gpsSource = PrepareGps(gpsFileName, gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;
        Preferences preferences = GetDefaultPreferences(&gpsUpdateTime);
        preferences.SetMultipleRoutes(true);

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), preferences);

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* rpl = new TestRoutePositionListener();
        TestTrafficListener* trl = new TestTrafficListener();
        TestAnnouncementListener* tal = new TestAnnouncementListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(rpl);
        navigation->AddTrafficListener(trl);
        navigation->AddAnnouncementListener(tal);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        g_instances.listenerType = lRouteRequested;
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            int loop = 0;
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);

                if (loop == 50)
                {
                    break;
                }
                loop++;
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveRoutePositionListener(rpl);
        navigation->RemoveTrafficListener(trl);
        navigation->RemoveAnnouncementListener(tal);
        delete navigation;
        delete tsl;
        delete rpl;
        delete trl;
        delete tal;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestRouteRequest(void)
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        Place origin = GetDefaultOrigin();
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        RouteOptions options = GetDefaultRouteOptions();
        TestRouteListener* trl = new TestRouteListener();

        // Test single route request
        RouteRequest* routeRequestSingle = new RouteRequest(g_instances.context, origin, destination, options, trl, false);
        routeRequestSingle->StartRequest();
        ResetCallbackCompletedEvent(g_instances.event);
        CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE);

        // Test multiple routes request
        RouteRequest* routeRequestMulti = new RouteRequest(g_instances.context, origin, destination, GetDefaultRouteOptions(), trl, true);
        routeRequestMulti->StartRequest();
        ResetCallbackCompletedEvent(g_instances.event);
        CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE);

        // Test cancel route request
        RouteRequest* routeRequestCancel = new RouteRequest(g_instances.context, origin, destination, GetDefaultRouteOptions(), trl, true);
        routeRequestCancel->StartRequest();
        CU_ASSERT_TRUE(routeRequestCancel->IsRequestInProgress());
        // Wait several seconds to give the change that route request is really started.
        ResetCallbackCompletedEvent(g_instances.event);
        WaitForCallbackCompletedEvent(g_instances.event, 1000);
        routeRequestCancel->CancelRequest();
        CU_ASSERT_FALSE(routeRequestCancel->IsRequestInProgress());

        delete routeRequestSingle;
        delete routeRequestMulti;
        delete routeRequestCancel;
        delete trl;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestRouteSummaryRequest()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        Place origin = GetDefaultOrigin();
        const char* gpsFileName = NULL;
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 0;
        Place destination = GetDefaultDestination(0, &gpsFileName, &gpsBeginOffset, &gpsEndOffset);
        RouteOptions options = GetDefaultRouteOptions();
        TestRouteSummaryListener* trl = new TestRouteSummaryListener();

        // Test route summary request
        RouteSummaryRequest* request = new RouteSummaryRequest(g_instances.context, origin, destination, trl);
        request->StartRequest();
        ResetCallbackCompletedEvent(g_instances.event);
        CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE);

        // Test cancel route summary request
        RouteSummaryRequest* requestCancel = new RouteSummaryRequest(g_instances.context, origin, destination, trl);
        requestCancel->StartRequest();
        CU_ASSERT_TRUE(requestCancel->IsRequestInProgress());
        // Wait several seconds to give the change that route summary request is really started.
        ResetCallbackCompletedEvent(g_instances.event);
        WaitForCallbackCompletedEvent(g_instances.event, 1000);
        requestCancel->CancelRequest();
        CU_ASSERT_FALSE(requestCancel->IsRequestInProgress());

        delete request;
        delete requestCancel;
        delete trl;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestSpecialRegion()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 20;
        Place destination;
        MapLocation destinationLocation;
        destinationLocation.center.latitude = 45.18651962;
        destinationLocation.center.longitude = -109.246994;
        destination.SetLocation(destinationLocation);

        TestGPSFile* gpsSource = PrepareGps("specialregion.gps", gpsBeginOffset, gpsEndOffset);
        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestSpecialRegionListener* tsrl = new TestSpecialRegionListener();

        navigation->AddSessionListener(tsl);
        navigation->AddSpecialRegionListener(tsrl);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);
            }
        }
        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveSpecialRegionListener(tsrl);
        delete navigation;
        delete tsl;
        delete tsrl;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestLaneGuidance()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 10000;
        Place destination;
        MapLocation destinationLocation;
        destinationLocation.center.latitude = 34.063566;
        destinationLocation.center.longitude = -118.366731;
        destination.SetLocation(destinationLocation);
        TestGPSFile* gpsSource = PrepareGps("LA_6345_WilshireBlvd.gps", gpsBeginOffset, gpsEndOffset);

        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;
        Preferences preference = GetDefaultPreferences(&gpsUpdateTime);
        preference.SetLaneGuidance(true);

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), preference);

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* tspl = new TestRoutePositionListener();
        TestNavEventListener* tnel = new TestNavEventListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(tspl);
        navigation->AddNavEventListener(tnel);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);
            }
        }

        CU_ASSERT_TRUE(tspl->isLaneGuidanceEnabledComming);
        CU_ASSERT_TRUE(tspl->isLaneGuidanceDisabledComming);
        CU_ASSERT_TRUE(tnel->isLaneGuidanceEnabledComming);
        CU_ASSERT_TRUE(tnel->isLaneGuidanceDisabledComming);

        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveRoutePositionListener(tspl);
        navigation->RemoveNavEventListener(tnel);
        delete navigation;
        delete tsl;
        delete tspl;
        delete tnel;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestRealisticSign()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    // Release all shared pointers before destroying the test instances.
    {
        uint32 gpsBeginOffset = 0;
        uint32 gpsEndOffset = 10000;
        Place destination;
        MapLocation destinationLocation;
        destinationLocation.center.latitude = 32.880600;
        destinationLocation.center.longitude = -117.107386;
        destination.SetLocation(destinationLocation);
        TestGPSFile* gpsSource = PrepareGps("poway.gps", gpsBeginOffset, gpsEndOffset);

        NB_GpsLocation* gpsFix = NULL;
        uint32 gpsUpdateTime = 0;
        Preferences preference = GetDefaultPreferences(&gpsUpdateTime);
        preference.SetLaneGuidance(true);

        Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), preference);

        if (!navigation)
        {
            CU_FAIL("Navigation creation failed");
            DestroyInstances(&g_instances);
            return;
        }

        TestSessionListener* tsl = new TestSessionListener(navigation);
        TestRoutePositionListener* tspl = new TestRoutePositionListener();
        TestNavEventListener* tnel = new TestNavEventListener();

        navigation->AddSessionListener(tsl);
        navigation->AddRoutePositionListener(tspl);
        navigation->AddNavEventListener(tnel);

        ResetCallbackCompletedEvent(g_instances.event);

        gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
        TestLocation location(gpsFix);
        navigation->UpdatePosition(location);

        //@todo - potential race condition here if the route comes back REALLY fast.
        ResetCallbackCompletedEvent(g_instances.event);
        if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
        {
            while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
            {
                TestLocation location(gpsFix);
                navigation->UpdatePosition(location);

                /* Reset the event so we can properly capture the next status change */
                ResetCallbackCompletedEvent(g_instances.event);
            }
        }

        CU_ASSERT_TRUE(tspl->isRealisticSignEnabledComming);
        CU_ASSERT_TRUE(tspl->isRealisticSignDisabledComming);
        CU_ASSERT_TRUE(tnel->isRealisticSignEnabledComming);
        CU_ASSERT_TRUE(tnel->isRealisticSignDisabledComming);

        if (gpsSource != NULL)
        {
            TestGPSFileDestroy(gpsSource);
        }

        navigation->RemoveSessionListener(tsl);
        navigation->RemoveRoutePositionListener(tspl);
        navigation->RemoveNavEventListener(tnel);
        delete navigation;
        delete tsl;
        delete tspl;
        delete tnel;

        WaitForRelease();
        DestroyInstances(&g_instances);
    }
}

void
TestNKGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNKGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNKGPS = false;
}

void
TestNK_SARGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_SARGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk_sar.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_SARGPS = false;
}

void
TestNK_SPDLGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_SPDLGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk_spdl.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);
    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_SPDLGPS = false;
}

void
TestNK_RECALCGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_RECALCGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk_recalc.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_RECALCGPS = false;
}

void
TestNK_CRIGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_CRIGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk_cri.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_CRIGPS = false;
}

void
TestNK_SECGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_SECGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk_sec.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_SECGPS = false;
}

void
TestNK_NGGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_NGGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("SEuclidAve2EMissionBlvd_4j.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_NGGPS = false;
}

void
TestNK_TrafficGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_trafficGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("10054 Airport Blvd Los Angeles CA 90045.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_trafficGPS = false;
}

void
TestNK_BEGPS()
{
    if(!CreateInstances(&g_instances))
    {
        CU_FAIL("NB Context and PAL creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    bifNK_BEGPS = true;
    uint32 gpsBeginOffset = 0;
    uint32 gpsEndOffset = 20;
    Place destination;
    MapLocation destinationLocation;
    destinationLocation.center.latitude = 45.18651962;
    destinationLocation.center.longitude = -109.246994;
    destination.SetLocation(destinationLocation);

    TestGPSFile* gpsSource = PrepareGps("nk.gps", gpsBeginOffset, gpsEndOffset);
    NB_GpsLocation* gpsFix = NULL;
    uint32 gpsUpdateTime = 0;

    Navigation* navigation = Navigation::GetNavigation(g_instances.context, destination, GetDefaultRouteOptions(), GetDefaultPreferences(&gpsUpdateTime));

    if (!navigation)
    {
        CU_FAIL("Navigation creation failed");
        DestroyInstances(&g_instances);
        return;
    }

    TestSessionListener* tsl = new TestSessionListener(navigation);
    TestRoutePositionListener* rpl = new TestRoutePositionListener();
    TestTrafficListener* trl = new TestTrafficListener();
    TestAnnouncementListener* tal = new TestAnnouncementListener();

    navigation->AddSessionListener(tsl);
    navigation->AddRoutePositionListener(rpl);
    navigation->AddTrafficListener(trl);
    navigation->AddAnnouncementListener(tal);

    ResetCallbackCompletedEvent(g_instances.event);

    gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime);
    TestLocation location(gpsFix);
    navigation->UpdatePosition(location);

    //@todo - potential race condition here if the route comes back REALLY fast.
    ResetCallbackCompletedEvent(g_instances.event);
    if (WaitForCallbackCompletedEvent(g_instances.event, 60000) == TRUE)
    {
        while ((gpsSource != NULL) && ((gpsFix = TestGPSFileGetNextFix(gpsSource, gpsUpdateTime)) != NULL) && (gpsFix->gpsTime <= gpsEndOffset))
        {
            TestLocation location(gpsFix);
            navigation->UpdatePosition(location);

            /* Reset the event so we can properly capture the next status change */
            ResetCallbackCompletedEvent(g_instances.event);
        }
    }
    if (gpsSource != NULL)
    {
        TestGPSFileDestroy(gpsSource);
    }

    navigation->RemoveSessionListener(tsl);
    navigation->RemoveRoutePositionListener(rpl);
    navigation->RemoveTrafficListener(trl);
    navigation->RemoveAnnouncementListener(tal);

    delete navigation;
    delete tsl;
    delete rpl;
    delete trl;
    delete tal;

    WaitForRelease();
    DestroyInstances(&g_instances);
    bifNK_BEGPS = false;
}

void TestSessionListener::OffRoute()
{
    LOGOUTPUT(LogLevelMedium, ("OffRoute \n"));
    CheckAndIncrementListener(lOnOffRoute);
}

void TestSessionListener::OnRoute()
{
    LOGOUTPUT(LogLevelMedium, ("OnRoute \n"));
    CheckAndIncrementListener(lOnOffRoute);
}

void TestSessionListener::RouteReceived(SessionListener::RouteRequestReason reason, vector<RouteInformation>& routes)
{
    LOGOUTPUT(LogLevelMedium, ("RouteReceived reason=%d \n", reason));

    CheckAndIncrementListener(lRouteReceived);

    vector<RouteInformation>::iterator iter = routes.begin();
    for ( ; iter != routes.end(); ++iter)
    {
        LOGOUTPUT(LogLevelMedium, ("    route description=%s \n", iter->GetRouteDescriptions().c_str()));
    }

    if (nsl_strcmp(CU_get_current_test()->pName, "TestNavigationSessionWithMultipleRoute") == 0 && reason == SessionListener::RouteSelector)
    {
        CU_ASSERT(routes.size() > 1);
        navigation->SetActiveRoute(routes.back());
    }
    else
    {
        // for detours as well
        navigation->SetActiveRoute(routes[0]);
    }

    if (reason == SessionListener::Calculate || reason == SessionListener::RouteSelector)
    {
        SetCallbackCompletedEvent(g_instances.event);
    }
}

void TestSessionListener::TestPreferencesOffRouteIgnoreCount(int reason, vector<RouteInformation>& routes)
{
    if (bifNK_RECALCGPS)
    {

    }
}

void TestSessionListener::TestNavigationPauseSession()
{

}

void TestSessionListener::TestNavigationResumeSession()
{

}

void TestRoutePositionListener::TripRemainingTime(int time)
{
    LOGOUTPUT(LogLevelHigh, ("TripRemainingTime time=%d \n", time));
    CheckAndIncrementListener(lTripRemainingTime);
}

void TestRoutePositionListener::TripRemainingDelay(int delay)
{
    LOGOUTPUT(LogLevelHigh, ("TripRemainingDelay delay=%d \n", delay));
    CheckAndIncrementListener(lTripRemainingDelay);
}

void TestRoutePositionListener::TripRemainingDistance(double distance)
{
    LOGOUTPUT(LogLevelHigh, ("TripRemainingDistance distance=%f \n", distance));
    CheckAndIncrementListener(lTripRemainingDistance);
}

void TestRoutePositionListener::PositionUpdated(const Coordinates& coordinates, double speed, double heading)
{
    LOGOUTPUT(LogLevelHigh, ("PositionUpdated coordinates.latitude=%f, coordinates.longitude=%f \n", coordinates.latitude, coordinates.longitude));
    LOGOUTPUT(LogLevelHigh, ("PositionUpdated speed=%f, heading=%f \n", speed, heading));
    CheckAndIncrementListener(lPositionUpdated);
}

void TestRoutePositionListener::ManeuverRemainingTime(int time)
{
    LOGOUTPUT(LogLevelHigh, ("ManeuverRemainingTime time=%d \n", time));
    CheckAndIncrementListener(lManeuverRemainingTime);
}

void TestRoutePositionListener::ManeuverRemainingDelay(int delay)
{
    LOGOUTPUT(LogLevelHigh, ("ManeuverRemainingDelay delay=%d \n", delay));
    CheckAndIncrementListener(lManeuverRemainingDelay);
}

void TestRoutePositionListener::ManeuverRemainingDistance(double distance)
{
    LOGOUTPUT(LogLevelHigh, ("ManeuverRemainingDistance distance=%f \n", distance));
    CheckAndIncrementListener(lManeuverRemainingDistance);
}

void TestRoutePositionListener::CurrentRoadName(string primaryName, string secondaryName)
{
    LOGOUTPUT(LogLevelMedium, ("CurrentRoadName primaryName=%s secondaryName=%s \n", primaryName.c_str(), secondaryName.c_str()));
    CheckAndIncrementListener(lCurrentRoadName);
}

void TestRoutePositionListener::ManeuverExitNumber(string exitNumber)
{
    LOGOUTPUT(LogLevelMedium, ("ManeuverExitNumber exitNumber=%s \n", exitNumber.c_str()));
    CheckAndIncrementListener(lManeuverExitNumber);
}

void TestRoutePositionListener::NextRoadName(string primaryName, string secondaryName)
{
    LOGOUTPUT(LogLevelMedium, ("NextRoadName primaryName=%s secondaryName=%s \n", primaryName.c_str(), secondaryName.c_str()));
    CheckAndIncrementListener(lNextRoadName);
}

void TestRoutePositionListener::ManeuverType(string type)
{
    LOGOUTPUT(LogLevelMedium, ("ManeuverType type=%s  \n", type.c_str()));
    CheckAndIncrementListener(lManeuverType);
}

void TestRoutePositionListener::ManeuverImageId(string imageId)
{
    LOGOUTPUT(LogLevelMedium, ("ManeuverImageId type=%s  \n", imageId.c_str()));
    CheckAndIncrementListener(lManeuverImageId);
}

void TestRoutePositionListener::StackTurnImageTTF(string stackImageTTF)
{
    LOGOUTPUT(LogLevelMedium, ("StackTurnImageTTF type=%s  \n", stackImageTTF.c_str()));
    CheckAndIncrementListener(lStackTurnImageTTF);
}

void TestRoutePositionListener::ManeuverPoint(const Coordinates& point)
{
    LOGOUTPUT(LogLevelMedium, ("ManeuverPoint coordinates.latitude=%f, coordinates.longitude=%f \n", point.latitude, point.longitude));
    CheckAndIncrementListener(lManeuverPoint);
}

void TestRoutePositionListener::SpeedLimit(const SpeedLimitInformation& speedLimitInfo)
{
    LOGOUTPUT(LogLevelMedium, ("SpeedLimit \n"));
    TestPreferencesSpeedLimitAlert(speedLimitInfo);
    TestSpeedLimitInfoCurrentSpeed(speedLimitInfo);
    TestSpeedLimitInfoInformation(speedLimitInfo);
    TestSpeedLimitInfoIsWarning(speedLimitInfo);
    TestSpeedLimitInfoSpeedLimit(speedLimitInfo);
    TestSpeedLimitInfoType(speedLimitInfo);
}

void TestRoutePositionListener::DisableSpeedLimit()
{
    LOGOUTPUT(LogLevelHigh, ("DisableSpeedLimit \n"));
}

void TestRoutePositionListener::RoadSign(const nbnav::RoadSign& signInfo)
{
    isRealisticSignEnabledComming = true;
    CheckAndIncrementListener(lRoadSign);
    LOGOUTPUT(LogLevelMedium, ("RoadSign \n"));
    TestRoadSignGetImageData(signInfo);
}

void TestRoutePositionListener::DisableRoadSign()
{
    isRealisticSignDisabledComming = true;
    CheckAndIncrementListener(lRoadSign);
    LOGOUTPUT(LogLevelHigh, ("DisableRoadSign \n"));
}

void TestRoutePositionListener::LaneInformation(const nbnav::LaneInformation& laneInfo)
{
    isLaneGuidanceEnabledComming = true;
    LOGOUTPUT(LogLevelMedium, ("LaneInformation \n"));
    TestLaneGetHighlighted(laneInfo);
    TestLaneGetNonHighlighted(laneInfo);
    TestLaneInfoLanes(laneInfo);
    TestLaneInfoSelectedLanePosition(laneInfo);
    TestLaneInfoSelectedLanes(laneInfo);
}

void TestRoutePositionListener::DisableLaneInformation()
{
    isLaneGuidanceDisabledComming = true;
    LOGOUTPUT(LogLevelHigh, ("DisableLaneInformation \n"));
}

void TestRoutePositionListener::UpdateManeuverList(const ManeuverList& maneuvers)
{
    CU_ASSERT_TRUE(maneuvers.GetNumberOfManeuvers() > 0);
    LOGOUTPUT(LogLevelHigh, ("UpdateManeuverList \n"));
}

void TestRoutePositionListener::TestLaneGetHighlighted(const nbnav::LaneInformation& laneInfo)
{
    if(bifNKGPS)
    {
        if(laneInfo.Lanes().size() > 0)
        {
            CU_ASSERT_TRUE(valHighlighted(laneInfo.Lanes()));
        }
    }
}

void TestRoutePositionListener::TestLaneGetNonHighlighted(const nbnav::LaneInformation& laneInfo)
{
    if(bifNKGPS)
    {
        if(laneInfo.Lanes().size() > 0)
        {
            CU_ASSERT_TRUE(valNonHighlighted(laneInfo.Lanes()));
        }
    }
}

void TestRoutePositionListener::TestLaneInfoLanes(const nbnav::LaneInformation& laneInfo)
{
     if(bifNKGPS)
     {
         CU_ASSERT_TRUE(laneInfo.Lanes().size() > 0);
     }
}

void TestRoutePositionListener::TestLaneInfoSelectedLanePosition(const nbnav::LaneInformation& laneInfo)
{
    if (bifNKGPS)
    {
        CU_ASSERT_TRUE(laneInfo.SelectedLanePosition() > 0);
    }
}

void TestRoutePositionListener::TestLaneInfoSelectedLanes(const nbnav::LaneInformation& laneInfo)
{
    if (bifNKGPS)
    {
        CU_ASSERT_TRUE(laneInfo.SelectedLanes() > 0);
    }

}

void TestRoutePositionListener::TestRoadSignGetImageData(const nbnav::RoadSign& signInfo)
{
    if (bifNK_SARGPS)
    {
        CU_ASSERT_TRUE(signInfo.GetImageData().GetData().size() > 0);
    }
}

void TestRoutePositionListener::TestPreferencesSpeedLimitAlert(const SpeedLimitInformation& speedLimitInfo)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.IsWarning());
    }
}

void TestRoutePositionListener::TestPreferencesSpeedLimitSign(const SpeedLimitInformation& speedLimitInfo)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.Information().GetData().size() > 0);
    }
}

void TestRoutePositionListener::TestSpeedLimitInfoCurrentSpeed(const SpeedLimitInformation& speedLimitInfo)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.CurrentSpeed() > 0);
    }
}

void TestRoutePositionListener::TestSpeedLimitInfoInformation(const SpeedLimitInformation& speedLimitInfo)
{
    if(bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.SpeedLimit() > 0);
    }
}

void TestRoutePositionListener::TestSpeedLimitInfoIsWarning(const SpeedLimitInformation& speedLimitInfo)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.IsWarning());
    }
}
void TestRoutePositionListener::TestSpeedLimitInfoSpeedLimit(const SpeedLimitInformation& speedLimitInfo)
{
    if(bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.SpeedLimit() > 0);
    }
}

void TestRoutePositionListener::TestSpeedLimitInfoType(const SpeedLimitInformation& speedLimitInfo)
{
    if(bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(speedLimitInfo.Type() != "");
    }
}

// TestSessionListener;
void TestSessionListener::RouteRequested(RouteRequestReason reason)
{
    LOGOUTPUT(LogLevelMedium, ("RouteRequested reason=%d \n", reason));
    CheckAndIncrementListener(lRouteRequested);
}

void TestSessionListener::RouteProgress(int progress)
{
    LOGOUTPUT(LogLevelHigh, ("RouteProgress progress=%d \n", progress));
    if (progress == 100)
    {
        CheckAndIncrementListener(lRouteProgress);
    }
}

void TestSessionListener::RouteError(NavigateRouteError error)
{
    LOGOUTPUT(LogLevelMedium, ("RouteError error=%d \n", (int)error));
}

void TestSessionListener::RouteFinish()
{
    g_instances.routeFinish = true;
    LOGOUTPUT(LogLevelMedium, ("RouteFinish \n"));
}

void TestSessionListener::OffRoutePositionUpdate(double headingToRoute)
{
    LOGOUTPUT(LogLevelMedium, ("OffRoutePositionUpdate: %f \n", headingToRoute));
}

void TestSessionListener::ArrivingToDestination(DestinationStreetSide direction)
{
    LOGOUTPUT(LogLevelMedium, ("ArrivingToDestination \n"));
}

void TestAnnouncementListener::Announce(shared_ptr<Announcement> announcement)
{
    LOGOUTPUT(LogLevelMedium, ("Announce %s \n", announcement->GetText().c_str()));
    if (g_instances.listenerType == lAnnounceGettingRoute)
    {
        CheckAndIncrementListener(lAnnounceGettingRoute);
    }
    else
    {
        CheckAndIncrementListener(lAnnounce);
    }

    TestPreferencesSchoolZoneAlert(announcement);
    TestAnnouncementGetAudio(announcement);
    TestAnnouncementGetText(announcement);

    announcement->OnAnnouncementComplete();
}

void TestAnnouncementListener::TestPreferencesSchoolZoneAlert(shared_ptr<Announcement> announcement)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(announcement->GetText() != "");
    }
}

void TestAnnouncementListener::TestAnnouncementGetAudio(shared_ptr<Announcement> announcement)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(announcement->GetAudio().GetData().size() > 0);
    }
}

void TestAnnouncementListener::TestAnnouncementGetText(shared_ptr<Announcement> announcement)
{
    if (bifNK_SPDLGPS)
    {
        CU_ASSERT_TRUE(announcement->GetText() != "");
    }
}

void TestAnnouncementListener::TestPreferencesTrafficAnncEnabled(shared_ptr<Announcement> announcement)
{
    if (bifNK_trafficGPS)
    {
        string audioText = announcement->GetText();
        CU_ASSERT_TRUE((audioText.find("traffic congestion") != -1) && (audioText.find("half a mile") != -1));
    }
}

void TestTrafficListener::TrafficAlerted(const TrafficEvent& trafficEvent)
{
    LOGOUTPUT(LogLevelMedium, ("TrafficAlerted \n"));
}

void TestTrafficListener::DisableTrafficAlerted()
{
    LOGOUTPUT(LogLevelMedium, ("DisableTrafficAlerted \n"));
}

void TestTrafficListener::TrafficChanged(const TrafficInformation& trafficInfo)
{
    CheckAndIncrementListener(lTrafficChanged);
    LOGOUTPUT(LogLevelMedium, ("TrafficChanged \n"));
    vector<TrafficEvent> events = trafficInfo.GetTrafficEvents();
    vector<TrafficEvent>::iterator it = events.begin();
    for(;it != events.end(); ++it)
    {
        LOGOUTPUT(LogLevelMedium, ("Traffic Event description %s \n", (*it).GetDescription().c_str()));
    }
}

void TestPositionUpdateListener::PositionUpdate(int maneuverRemainingDelay, double maneuverRemainingDistance,
            int maneuverRemainingTime, Coordinates coordinates,
            int speed, int heading, int tripRemainingDelay,
            double tripRemainingDistance, int tripRemainingTime)
{
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("PositionUpdate \n"));
}

void TestManeuverUpdateListener::ManeuverUpdate(int maneuverID,
            string currentRoadPrimaryName, string currentRoadSecondaryName,
            string maneuverExitNumber, string maneuverImageId,
            Coordinates maneuverPoint, string maneuverType,
            string nextRoadPrimaryName, string nextRoadSecondaryName,
            string maneuverStackTurnImageTTF)
{
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("ManeuverUpdate \n"));
}

void TestManeuverUpdateListener::TestPreferencesNeedNC(string maneuverType)
{
    if (bifNK_trafficGPS)
    {
        CU_ASSERT_TRUE(maneuverType == "CO.");
    }
}


void TestManeuverUpdateListener::TestPreferencesBridgeManeuverEnabled(string maneuverType)
{
    if (bifNK_BEGPS)
    {
        CU_ASSERT_TRUE(maneuverType == "BE.");
    }
}

void TestNavEventListener::NavEvent(NavEventTypeMask navEventTypeMask, const LaneInformation& laneInfo,
                                    const RoadSign& signInfo, const SpeedLimitInformation& speedLimitInfo,
                                    const TrafficEvent& trafficEvent, const TrafficInformation& trafficInfo)
{
    switch (navEventTypeMask)
    {
    case NETP_RoadSignEnable:
        isRealisticSignEnabledComming = true;
        break;
    case NETP_RoadSignDisable:
        isRealisticSignDisabledComming = true;
        break;
    case NETP_LaneInformationEnable:
        isLaneGuidanceEnabledComming = true;
        break;
    case NETP_LaneInformationDisable:
        isLaneGuidanceDisabledComming = true;
        break;
    default:
        break;
    }
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("NavEvent navEventTypeMask=%d \n", (int)navEventTypeMask));
}

void TestRouteListener::TestManeueverGetRoutingIcon(vector<RouteInformation>& information)
{
}

void TestRouteListener::TestManeuverGetBbx(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        Coordinates minPnt = maneuverlist.GetRouteBoundingBox().point1;
        Coordinates maxPnt = maneuverlist.GetRouteBoundingBox().point2;
        vector<Coordinates> ply = (*it).GetPolyline();

        for (vector<Coordinates>::iterator itor = ply.begin(); itor != ply.end(); ++itor)
        {
            // left bottom/right top point on boundingbox
            double minLat = minPnt.latitude;
            double minLon = minPnt.longitude;
            double maxLat = maxPnt.latitude;
            double maxLon = maxPnt.longitude;
            // start/end point for each polyline segment
            double minLat4Ply = itor->latitude;
            double minLon4Ply = itor->longitude;
            CU_ASSERT_TRUE(minLat4Ply > minLat && minLat4Ply < maxLat);
            CU_ASSERT_TRUE(minLon4Ply > minLon && minLon4Ply < maxLon);
        }
    }
}

void TestRouteListener::TestManeuverGetCommand(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetCommand().length() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetDescription(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetDescription(true).length() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetDistance(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetDistance() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetFormattedDesc(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetManeuverText(true).GetFormattedTextCount() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetManeuverCode(vector<RouteInformation>& information)
{
}

void TestRouteListener::TestManeuverGetManeuverID(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetManeuverID() >= 0);
        }
    }
}

void TestRouteListener::TestManeuverGetPoint(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestManeuverGetPolyline(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetPolyline().size() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetRoutingTTF(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetRoutingTTF().length() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetTime(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetTime() > 0);
        }
    }
}

void TestRouteListener::TestManeuverGetTrafficDelay(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i)->GetTrafficDelay() > 0);
        }
    }
}

void TestRouteListener::TestManeuverListGetDestination(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestManeuverListGetManeuver(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        ManeuverList maneuverlist = (*it).GetManeuverList();
        for (uint32 i = 0; i<maneuverlist.GetNumberOfManeuvers(); ++i)
        {
            CU_ASSERT_TRUE(maneuverlist.GetManeuver(i) != NULL);
        }
    }
}

void TestRouteListener::TestManeuverListGetNumberOfMans(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetManeuverList()).GetNumberOfManeuvers() > 0);
    }
}

void TestRouteListener::TestManeuverListGetOrigin(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestManeuverListGetRouteBbx(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestManeuverListGetTotalDelay(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetManeuverList()).GetTotalDelay() > 0);
    }
}

void TestRouteListener::TestManeuverListGetTotalDistance(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetManeuverList()).GetTotalDistance() > 0);
    }
}

void TestRouteListener::TestManeuverListGetTotalTripTime(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetManeuverList()).GetTotalTripTime() > 0);
    }
}

void TestRouteListener::OnRoute(vector<RouteInformation>& information, RouteRequest* request)
{
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("OnRoute routeInformationNumber=%d \n", information.size()));
    SetCallbackCompletedEvent(g_instances.event);

    TestRouteInfoGetRouteProperties(information);
    TestRouteInfoGetBoundingBox(information);
    TestRouteInfoGetDelay(information);
    TestRouteInfoGetDestination(information);
    TestRouteInfoGetDistance(information);
    TestRouteInfoGetInitialGuidanceText(information);
    TestRouteInfoGetManeuverList(information);
    TestRouteInfoGetOrigin(information);
    TestRouteInfoGetPolyline(information);
    TestRouteInfoGetRouteDescription(information);
    TestRouteInfoGetRouteError(information);
    TestRouteInfoGetRouteID(information);
    TestRouteInfoGetTime(information);
    TestRouteInfoGetVoiceFile(information);

    TestManeueverGetRoutingIcon(information);
    TestManeuverGetBbx(information);
    TestManeuverGetCommand(information);
    TestManeuverGetDescription(information);
    TestManeuverGetDistance(information);
    TestManeuverGetFormattedDesc(information);
    TestManeuverGetManeuverCode(information);
    TestManeuverGetManeuverID(information);
    TestManeuverGetPoint(information);
    TestManeuverGetPolyline(information);
    TestManeuverGetRoutingTTF(information);
    TestManeuverGetTime(information);
    TestManeuverGetTrafficDelay(information);
    TestManeuverListGetDestination(information);
    TestManeuverListGetManeuver(information);
    TestManeuverListGetNumberOfMans(information);
    TestManeuverListGetOrigin(information);
    TestManeuverListGetRouteBbx(information);
    TestManeuverListGetTotalDelay(information);
    TestManeuverListGetTotalDistance(information);
    TestManeuverListGetTotalTripTime(information);
}

void TestRouteListener::TestRouteInfoGetRouteProperties(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE((*it).GetRouteProperties().size() > 0);
    }
}

void TestRouteListener::TestRouteInfoGetBoundingBox(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
       //CU_ASSERT_TRUE((*it).GetBoundingBox());
    }
}

void TestRouteListener::TestRouteInfoGetDelay(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE((*it).GetDelay() >= 0);
    }
}

void TestRouteListener::TestRouteInfoGetDestination(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestRouteInfoGetDistance(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE((*it).GetDistance() >= 0);
    }
}

void TestRouteListener::TestRouteInfoGetInitialGuidanceText(vector<RouteInformation>& information)
{
    if (bifNK_NGGPS)
    {
        vector<RouteInformation>::iterator it = information.begin();
        for (; it != information.end(); ++it)
        {
            string text = (*it).GetInitialGuidanceText();
            CU_ASSERT_TRUE(text.find("<span class=\"road-name\">South Euclid Avenue</span>") != -1
                           && text.find("<span class=\"road-name\">E Mission Blvd</span>") != -1);
        }
    }
}

void TestRouteListener::TestRouteInfoGetManeuverList(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetManeuverList()).GetNumberOfManeuvers() > 0);
    }

}

void TestRouteListener::TestRouteInfoGetOrigin(vector<RouteInformation>& information)
{

}

void TestRouteListener::TestRouteInfoGetPolyline(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetPolyline()).size() > 0);
    }
}

void TestRouteListener::TestRouteInfoGetRouteDescription(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetRouteDescriptions()).length() > 0);
    }

}

void TestRouteListener::TestRouteInfoGetRouteError(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetRouteError()) != 0);
    }
}

void TestRouteListener::TestRouteInfoGetRouteID(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE(((*it).GetRouteID()).size() > 0);
    }

}

void TestRouteListener::TestRouteInfoGetTime(vector<RouteInformation>& information)
{
    vector<RouteInformation>::iterator it = information.begin();
    for (; it != information.end(); ++it)
    {
        CU_ASSERT_TRUE((*it).GetTime() > 0);
    }
}

void TestRouteListener::TestRouteInfoGetVoiceFile(vector<RouteInformation>& information)
{

}

void TestRouteSummaryListener::OnRouteSummary(RouteSummaryInformation& information, RouteSummaryRequest* request)
{
    SetCallbackCompletedEvent(g_instances.event);
}

void TestSpecialRegionListener::SpecialRegion(const SpecialRegionInformation& specialRegionInfo)
{
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("SpecialRegion \n"));
    TestSpecialRegionInfoGetDescription(specialRegionInfo);
    TestSpecialRegionInfoGetImage(specialRegionInfo);
    TestSpecialRegionInfoGetNextOccurrenceEndTime(specialRegionInfo);
    TestSpecialRegionInfoGetNextOccurrenceStartTime(specialRegionInfo);
    TestSpecialRegionInfoGetRegionId(specialRegionInfo);
    TestSpecialRegionInfoGetType(specialRegionInfo);
}

void TestSpecialRegionListener::DisableSpecialRegion(const SpecialRegionInformation& specialRegionInfo)
{
    // TODO: Add more logging
    LOGOUTPUT(LogLevelMedium, ("DisableSpecialRegion \n"));
}
void TestSpecialRegionListener::TestSpecialRegionInfoGetDescription(const SpecialRegionInformation& specialRegionInfo)
{
    CU_ASSERT_TRUE(specialRegionInfo.GetDescription() == "seasonal closure");
}

void TestSpecialRegionListener::TestSpecialRegionInfoGetImage(const SpecialRegionInformation& specialRegionInfo)
{
    CU_ASSERT_TRUE((specialRegionInfo.GetImage()).GetData().size() > 0);
}

void TestSpecialRegionListener::TestSpecialRegionInfoGetNextOccurrenceEndTime(const SpecialRegionInformation& specialRegionInfo)
{

}

void TestSpecialRegionListener::TestSpecialRegionInfoGetNextOccurrenceStartTime(const SpecialRegionInformation& specialRegionInfo)
{

}

void TestSpecialRegionListener::TestSpecialRegionInfoGetRegionId(const SpecialRegionInformation& specialRegionInfo)
{
    CU_ASSERT_TRUE((specialRegionInfo.GetDescription()).length() > 0);
}

void TestSpecialRegionListener::TestSpecialRegionInfoGetType(const SpecialRegionInformation& specialRegionInfo)
{
    CU_ASSERT_TRUE(specialRegionInfo.GetType() == "seasonal-closure");
}

static nb_boolean
CreateInstances(Instances* instances)
{
    // Create PAL instance and context
    uint8 createResult = CreatePalAndContext(&(instances->pal), &(instances->context));
    CU_ASSERT(createResult);
    CU_ASSERT_PTR_NOT_NULL(instances->pal);
    CU_ASSERT_PTR_NOT_NULL(instances->context);

    if (!createResult)
    {
        DestroyInstances(instances);
        return FALSE;
    }

    instances->event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(instances->event);

    return TRUE;
}

/*! Destroy PAL, context

@return None
*/
static void
DestroyInstances(Instances* instances)
{
    if (instances->context)
    {
        DestroyContext(instances->context);
    }

    if (instances->pal)
    {
        PAL_Destroy(instances->pal);
    }

    if (instances->event)
    {
        DestroyCallbackCompletedEvent(instances->event);
    }
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
        ResetCallbackCompletedEvent(g_instances.event);
        WaitForCallbackCompletedEvent(g_instances.event, delayMilliseconds);
    }

    gps->nextFix++;
    gps->curFix++;

    return &gps->curGPSLocation;
}

static TestGPSFile*
TestGPSFileLoad(PAL_Instance* pal, const char* gpsFileName)
{
    PAL_Error       palError = PAL_Ok;
    TestGPSFile*    pThis = static_cast<TestGPSFile*>(nsl_malloc(sizeof(TestGPSFile)));

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

static void CheckAndIncrementListener(ListenerTypes current)
{
    if (nsl_strcmp(CU_get_current_test()->pName, "TestListenersInvocationOrder") == 0)
    {
        if (g_instances.listenerType != current && g_instances.listenerType != lFinishTest)
        {
            char msg[100] = {0};
            nsl_sprintf(msg, "Incorrect listener invocation order for %d \n listener is %d", current, g_instances.listenerType);
            CU_FAIL(msg);
        }
        if (g_instances.listenerType < lFinishTest)
        {
            g_instances.listenerType = static_cast<ListenerTypes>(g_instances.listenerType + 1);
        }
    }
}

void WaitForRelease()
{
    if (g_instances.event)
    {
        // wait several seconds for release timing.
        ResetCallbackCompletedEvent(g_instances.event);
        WaitForCallbackCompletedEvent(g_instances.event, 1000);
    }
}

bool valHighlighted(vector<nbnav::Lane> lanes) {
    bool flag = true;
    for (uint32 i = 0; i < lanes.size(); ++i) {
        switch (i) {
            case 0:
            case 1:
                if (lanes[i].GetHighlighted().length() != 0) {
                    flag = false;
                }
                continue;
            case 2:
            case 3:
            case 4:
                if (lanes[i].GetHighlighted().length() != 0) {
                    flag = false;
                }
                continue;
            case 5:
                if (lanes[i].GetHighlighted().length() == 0
                    || lanes[i].GetHighlighted()[0] != 'E') {
                    flag = false;
                }
                continue;
            default:
                continue;
        }
    }
    return flag;
}

bool valNonHighlighted(vector<nbnav::Lane> lanes) {
    bool flag = true;
    for (uint32 i = 0; i < lanes.size(); i++) {
        switch (i) {
            case 0:
            case 1:
                if (lanes[i].GetNonHighlighted().length() == 0
                    || lanes[i].GetNonHighlighted()[0] != 'A') {
                    flag = false;
                }
                continue;
            case 2:
            case 3:
            case 4:
                if (lanes[i].GetNonHighlighted().length() == 0
                    || lanes[i].GetNonHighlighted()[0] != 'C') {
                    flag = false;
                }
                continue;
            case 5:
                if (lanes[i].GetNonHighlighted().length() != 0) {
                    flag = false;
                }
                continue;
            default:
                continue;
        }
    }
    return flag;
}

/*! @} */
