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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "NavigationImpl.h"
#include "NavigationHelper.h"
#include "Detour.h"
#include "NavTask.h"
#include "Lock.h"
#include "NavApiNavUtils.h"
extern "C"
{
#include "nbgpstypes.h"
#include "nbrouteinformation.h"
#include "nbnavigation.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "palfile.h"
#include "abpalaudio.h"
#include "nbspatial.h"
#include "NBProtocolInvocationContextSerializer.h"
}
#include <algorithm>

#define SPAN_ROAD_NAME                          "span-road-name"
#define SPAN_CLOSE                              "span-close"
#define DEFAULT_LOCAL                           "en-US"
#define GPS_PROBE_INTERVAL                      10000

namespace nbnav
{
const int    extrapolatePathMaxLength   = 30;
//const int    extrapolateMinDuration     = 1000;
const int    extrapolateDuration        = 3000;
const double minimumDistanceBeforeIncidentAnnouncement = 10000;  //10 KM /6.3 miles
const int    bufferSize                 = 200;
const int    announceLengthLimit        = 15;
const double estimateOfShorterAnnounce  = 1.0f;    // the estimation playtim when announce text length is shorter than the announceLengthLimit
const double estimateofLongerAnnounce   = 2.0f;    // the estimation playtim when announce text length is longer than the announceLengthLimit
const uint32 BADINITIAL_STARTUP_TIMEOFF = 5;       // if no available fix is received in this timeoff, nav-query will be with sigle fix.
const uint32 ENHANCED_STARTUP_TIMEOFF   = 15;      // if no available fix is received in this timeoff, navigation will enter enhanced startup process.
const uint32 STARTUP_PROCESS_INTERVAL   = 1000;    // the time interval of startup process, 1000 milliseconds as default
const uint32 GPSHISTORY_MAX_FIXES       = 100;     // Max fixes
const char*  PAUSE_VOICE_CODE           = "pause"; // the pause voice code
const double LOCATION_THRESHHOLD        = 50;      // the thresh hold of location available check
const uint32 ETA_STATE_INTERVAL         = 5 * 60;  // the interval of add navigation state analytics of ETA information

static void LocationToNbGpsLocation(const Location& location, NB_GpsLocation* nbGpsLocation);

#define  NAV_WRITE_LOG(X, L, ...)                       \
    do                                                  \
    {                                                   \
        if (X)                                          \
        {                                               \
            (X)->WriteLog((L), "NavKit",  __VA_ARGS__); \
        }                                               \
    } while (0)

#define NAV_DEBUG_LOG(X, ...) NAV_WRITE_LOG(X, Logger::LL_Debug, __VA_ARGS__)
#define NAV_INFO_LOG(X, ...) NAV_WRITE_LOG(X, Logger::LL_Info, __VA_ARGS__)
#define NAV_WARN_LOG(X, ...) NAV_WRITE_LOG(X, Logger::LL_Warn, __VA_ARGS__)
#define NAV_FATAL_LOG(X, ...) NAV_WRITE_LOG(X, Logger::LL_Fatal, __VA_ARGS__)

static inline bool CompareStringWithRouteId(const string& id1, const NB_RouteId* id2)
{
    bool ret = false;
    char* encodedRouteId = NULL;
    if ((id2 && id2->data && id2->size && !id1.empty()) && (encode_base64((const char*)id2->data, id2->size,
                      &encodedRouteId) == NE_OK))
    {
        ret =  !nsl_memcmp(encodedRouteId, id1.data(), id1.size());
        nsl_free(encodedRouteId);
    }

    // simply return false if either id is empty, even if they are both empty.
    return ret;
}

NavigationImpl::NavigationImpl(NB_Context* nbContext, const std::vector<Place> destinations, const RouteOptions& routeOptions, const Preferences& preferences, const vector<WayPoint>* wayPoint)
    : m_context(nbContext),
      m_routeOptions(routeOptions),
      m_preferences(preferences),
      m_nbNavigation(NULL),
      m_nbEnhancedManager(NULL),
      m_nbGuidanceInfo(NULL),
      m_nbGuidanceInfoForUI(NULL),
      m_destinations(destinations),
      m_detour(NULL),
      m_activeRoute(NULL),
      m_currentManeuver(NAV_MANEUVER_NONE),
      m_closestManeuver(NAV_MANEUVER_NONE),
      m_onOffRoute(rtNone),
      m_extrapolatePosition(extrapolatePathMaxLength),
      m_extrapolatePathLen(0),
      m_extrapolatePointsInterval(0),
      m_extrapolateCurrentPosition(0),
      m_horizontalVelocity(0),
      m_methodLock(NULL),
      m_requestedPercent(0),
      m_tripRemainingTime(0),
      m_tripRemainingDistance(0),
      m_tripRemainingDelay(0),
      m_tripTrafficColor('\0'),
      m_maneuverRemainingTime(0),
      m_maneuverRemainingDistance(0),
      m_maneuverRemainingDelay(0),
      m_maneuverTraveledDistance(0),
      m_trafficDelay(0),
      m_currentSpeed(0),
      m_currentHeading(0),
      m_currentMask(NETP_None),
      m_currentTotalMask(NETP_None),
      m_currentSignInfo(NULL),
      m_currentLaneInfo(NULL),
      m_currentSpeedLimitInfo(NULL),
      m_currentTrafficEvent(NULL),
      m_currentTrafficInfo(NULL),
      m_currentManeuverList(NULL),
      m_isRouteArrived(false),
      m_startupState(SU_GoodInitial),
      m_startupTimeoff(0),
      m_isStartupRunning(false),
      m_gpsHistory(NULL),
      m_lastNoGpsFix(NULL),
      m_isRouteActive(false),
      m_logger(NULL),
      m_lastETATime(0),
      m_needToRecalcForPlanTrip(false),
      m_isNewRouteForPlanTripReceived(true),
      m_planningTrip(false),
      m_mockProvider(NULL)
{
    m_isValid = shared_ptr<bool>(new bool(true));
    m_isDeleteValid = shared_ptr<bool>(new bool(true));
    nsl_memset(&m_lastPosition, 0, sizeof(NB_GpsLocation));
    m_pal = NB_ContextGetPal(m_context);
    if (preferences.GetPositionUpdateRate() > 0)
    {
        m_extrapolatePointsInterval = 1000 / preferences.GetPositionUpdateRate();
        if (m_extrapolatePointsInterval < extrapolateDuration / extrapolatePathMaxLength)
        {
            m_extrapolatePointsInterval = extrapolateDuration / extrapolatePathMaxLength;
        }
    }
    m_analytics.Initialize(m_pal, m_context, m_isValid);

    if(m_preferences.GetLocal().empty())
    {
        m_locale = NB_ContextGetLocale(m_context);
    }
    else
    {
        m_locale = m_preferences.GetLocal();
    }
    m_voicesStyle = m_routeOptions.GetPronunciationStyle();

    m_isStartupRunning = true;
    PAL_TimerSet(m_pal, STARTUP_PROCESS_INTERVAL, Static_StartupCb, this);

    // Construct path for m_logger
    const char* path = PAL_FileGetCachePath();
    char fullPath[PAL_FILE_MAX_FULL_PATH_LENGTH] = {'\0'};
    if (path && sprintf(fullPath, "%s", path) &&
        PAL_FileAppendPath(NB_ContextGetPal(m_context), fullPath,
                           PAL_FILE_MAX_NAME_LENGTH, "Navkit.log") == PAL_Ok)
    {
        path = fullPath;
    }
    else
    {
        path = NULL;
    }

    m_logger = Logger::GetInstance(m_context, path, Logger::LL_Debug);

    //@todo: This is actually a test to SetLogLevel(), should be called out of Navkit.
    if (m_logger)
    {
        m_logger->SetLogLevel(Logger::LL_Verbose);
    }

    m_lastExtrapolatePosition.latitude = INVALID_LATLON;
    m_lastExtrapolatePosition.longitude = INVALID_LATLON;
    m_lastExtrapolatePosition.heading = 0;
    m_wayPoints.clear();
    if(wayPoint)
    {
        m_wayPoints = *wayPoint;
    }
}

NavigationImpl::~NavigationImpl()
{
    delete m_currentSignInfo;
    delete m_currentLaneInfo;
    delete m_currentSpeedLimitInfo;
    delete m_currentTrafficEvent;
    delete m_currentTrafficInfo;
    delete m_currentManeuverList;
    delete m_mockProvider;
    delete m_activeRoute;

    if (m_lastNoGpsFix)
    {
        delete m_lastNoGpsFix;
    }

    (void)PAL_LockDestroy(m_methodLock);
    m_methodLock = NULL;
    (void)PAL_LockDestroy(m_memberLock);
    m_memberLock = NULL;
}

void NavigationImpl::Release()
{
    m_notifier.ClearListeners();

    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this, NULL,
                                                                          &NavigationImpl::DeleteThisScheduled,
                                                                          m_isDeleteValid, true);
    task->Execute();
}

void NavigationImpl::DeleteThisScheduled(int* data)
{
    *m_isValid = false;
    *m_isDeleteValid = false;
    DestroyNbNavSession();
    delete this;
}

void NavigationImpl::ClearNbRoutes()
{
    nb::Lock lock(m_memberLock);
    vector<NB_RouteInformation*>::iterator itRoute = m_nbRoutes.begin();
    for (; itRoute != m_nbRoutes.end(); ++itRoute)
    {
        (void)NB_RouteInformationDestroy((*itRoute));
    }
    m_nbRoutes.clear();
}

void NavigationImpl::ClearNbDetours()
{
    nb::Lock lock(m_memberLock);
    vector<NB_RouteInformation*>::iterator itRoute = m_nbDetours.begin();
    for (; itRoute != m_nbDetours.end(); ++itRoute)
    {
        (void)NB_RouteInformationDestroy((*itRoute));
    }
    m_nbDetours.clear();
}

void NavigationImpl::ClearActiveRoute()
{
    nb::Lock lock(m_memberLock);
    if (m_activeRoute)
    {
        delete m_activeRoute;
        m_activeRoute = NULL;
    }
}

NB_RouteInformation* NavigationImpl::GetNbRoute(const RouteInformation& route) const
{
    return GetNbRoute(route.m_index);
}

NB_RouteInformation* NavigationImpl::GetNbRoute(unsigned int routeIndex) const
{
    if (routeIndex >= Detour::detourOffset)
    {
        return m_nbDetours[routeIndex - Detour::detourOffset];
    }
    if(m_nbRoutes.size() <= routeIndex)
    {
        return NULL;
    }
    return m_nbRoutes[routeIndex];
}

bool NavigationImpl::IsRouteValid(const RouteInformation& route) const
{
    vector<NB_RouteInformation*>::const_iterator itRoute = m_nbRoutes.begin();
    for (; itRoute != m_nbRoutes.end(); ++itRoute)
    {
        NB_RouteId* routeId = NULL;

        NB_RouteInformationGetRouteId((*itRoute), &routeId);
        if (CompareStringWithRouteId(route.GetRouteID(), routeId))
        {
            return true;
        }
    }
    return false;
}

bool NavigationImpl::IsDetourValid(const RouteInformation& route) const
{
    vector<NB_RouteInformation*>::const_iterator itRoute = m_nbDetours.begin();
    for (; itRoute != m_nbDetours.end(); ++itRoute)
    {
        NB_RouteId* routeId = NULL;
        NB_RouteInformationGetRouteId((*itRoute), &routeId);
        if (CompareStringWithRouteId(route.GetRouteID(), routeId))
        {
            return true;
        }
    }
    return false;
}

unsigned int NavigationImpl::GetNbRouteIndex(const RouteInformation& route)
{
    if (route.m_index >= Detour::detourOffset)
    {
        return route.m_index - Detour::detourOffset;
    }
    return route.m_index;
}

NB_Error NavigationImpl::Init()
{
    NB_Error err = m_notifier.Init(m_pal, m_isValid);
    err = ((PAL_LockCreate(m_pal, &m_methodLock) == PAL_Ok) ? NE_OK : NE_INVAL);
    err = err ? err : ((PAL_LockCreate(m_pal, &m_memberLock) == PAL_Ok) ? NE_OK : NE_INVAL);
    return err;
}

void NavigationImpl::UpdatePosition(const Location& location)
{
    nb::Lock lock(m_methodLock);
    //revoke locations from outside when navigating and mock gps enabled
    if (m_activeRoute && m_preferences.GetMockGpsEnabled())
    {
        return;
    }

    NB_GpsLocation* gpsLocation = new NB_GpsLocation();
    nsl_memset(gpsLocation, 0, sizeof(NB_GpsLocation));
    LocationToNbGpsLocation(location, gpsLocation);

    NavTask<NavigationImpl, NB_GpsLocation>* task = new NavTask<NavigationImpl, NB_GpsLocation>(
        m_pal, this, gpsLocation, &NavigationImpl::UpdatePositionScheduled, m_isValid);
    task->Execute();
}

void NavigationImpl::Lock()
{
    PAL_LockLock(m_memberLock);
}
void NavigationImpl::UnLock()
{
    PAL_LockUnlock(m_memberLock);
}

void NavigationImpl::UpdatePositionScheduled(NB_GpsLocation* gpsLocation)
{
	NB_NavigateStatus status = NB_NS_Invalid;
    NB_NavigationPublicState publicState;
    if (gpsLocation == NULL)
    {
        return;
    }
    Lock();
    nsl_memcpy(&m_lastPosition, gpsLocation, sizeof(NB_GpsLocation));

    NB_GpsHistory* sharedHistroy = NB_ContextGetGpsHistory(m_context);
    if (!m_gpsHistory)
    {
        if (NB_GpsHistoryCreate(m_context, GPSHISTORY_MAX_FIXES, m_preferences.GetSpeedThresholdForGpsHeading(), &m_gpsHistory) == NE_OK)
        {
            NB_GpsHistorySetLocationThreshhold(m_gpsHistory, LOCATION_THRESHHOLD, TRUE);
        }

        // clear the shared gps history when start a new navigation session
        if (sharedHistroy)
        {
            NB_GpsHistoryClear(sharedHistroy);
        }
    }

    // Add gps fix into the GPS history
    if (m_gpsHistory && m_lastPosition.status == NE_OK)
    {
        if (m_nbNavigation == NULL || m_startupState == SU_EnterEnhancedStartup)
        {
            // if this fix is no gps don't add it into history and hold this to last no gps pointer
            if (NB_GpsHistoryAdd(m_gpsHistory, &m_lastPosition) == NE_GPS)
            {
                if (m_lastNoGpsFix)
                {
                    delete m_lastNoGpsFix;
                }
                m_lastNoGpsFix = new NB_GpsLocation(m_lastPosition);
            }

            if (sharedHistroy)
            {
                NB_GpsHistoryAdd(sharedHistroy, &m_lastPosition);
            }
        }
    }

    if (m_nbNavigation == NULL)
    {
        NB_Error error = NE_OK;
        // we need to create and start navigation session
        error = CreateNbNavSession();
        if (error != NE_OK)
        {
            if (error == NE_GPS_TIMEOUT)
            {
                SetRouteError(NRE_GPSTimeout);
            }
            else
            {
                SetRouteError(NRE_Internal);
            }
            goto errexit;
        }
        else
        {
            /*! we should start analytics session if navigation started
                and end the session if navigation ended. */
            m_analytics.StartAnalyticsSession(true);
        }
    }
    else
    {
        uint32 time = PAL_ClockGetTimeMs();
        if((time - m_lastGpsEventTime) >=  GPS_PROBE_INTERVAL)
        {
            uint32 sessionID = 0;
            NB_NavigationGetSessionId(m_nbNavigation, &sessionID);
            m_analytics.AddGpsProbesEvent(sessionID, m_lastPosition);
            m_lastGpsEventTime = time;
        }
    }
    // just return and waiting another fix
    if (m_nbNavigation == NULL)
    {
        goto errexit;
    }

    m_extrapolatePathLen = 0;
    (void)PAL_TimerCancel(m_pal, Static_ExtraPolationPositionChangeCb, this);

    if (!GetNbNavRouteInfo())
    {
        goto errexit;
    }

    if (!m_activeRoute)
    {
        goto errexit;
    }

    // recalculation when the first gps in from plan trip.
    if (m_needToRecalcForPlanTrip)
    {
        m_needToRecalcForPlanTrip = false;

        // reset the callbacks when start navigation at plan route.
        NB_NavigationCallbacks callbacks = {0};
        callbacks.trafficNotificationCallback         = NbTrafficNotifcationCb;
        callbacks.maneuverPositionChangeCallback      = NbManeuverPositionChangeCb;
        callbacks.enhancedContentNotificationCallback = NbEnhancedContentStateNotificationCb;
        callbacks.speedLimitCallback                  = NbSpeedLimitCb;
        callbacks.visualLaneGuidanceCallback          = NbVisualLaneGuidanceCb;
        callbacks.specialRegionCallback               = NbSpecialRegionCb;
        callbacks.getAnnounceSnippetLengthCallback    = NbAnnounceSnippetLength;
        callbacks.userData                            = this;
        NB_NavigationSetCallbacks(m_nbNavigation, &callbacks);

        if (sharedHistroy)
        {
            NB_GpsHistoryClear(sharedHistroy);
            NB_GpsHistoryAdd(sharedHistroy, &m_lastPosition);
        }
        ConfigNBNavigation(m_nbNavigation, m_preferences, GetMeasurementUnits());
        m_isNewRouteForPlanTripReceived = false;
        Recalculate(m_routeOptions, m_preferences, false);
    }

    // if no route received do not update position of nbservices.
    if (!m_isNewRouteForPlanTripReceived)
    {
        goto errexit;
    }

    if (m_preferences.GetEnableEnhancedNavigationStartup() && m_startupState == SU_EnterEnhancedStartup)
    {
        // if in enter enhanced start up state and received one gps fix then exit the enhanced start up state.
        int gpsCount = NB_GpsHistoryGetCount(m_gpsHistory);
        if (gpsCount > 0)
        {
            m_startupState = SU_ExitEnhancedStartup;
            m_notifier.ExitEnhancedStartup();
        }
        else // if in enter enhanced start up state then do nothing.
        {
            goto errexit;
        }
    }
    if(m_nbNavigation)
    {
        NB_NavigationGetPublicState(GetNbNavigation(), &publicState);
        status = publicState.status;
    }
    (void)NB_NavigationUpdateGPSLocation(m_nbNavigation, gpsLocation);
	if(m_nbNavigation)
    {
        NB_NavigationGetPublicState(GetNbNavigation(), &publicState);
        if(publicState.status != NB_NS_UpdatingRoute && ((status == NB_NS_UpdatingRoute)||(status == NB_NS_Invalid)))
        {
            nsl_memset(&m_lastExtrapolatePosition, 0, sizeof(m_lastExtrapolatePosition));
            m_lastExtrapolatePosition.latitude = INVALID_LATLON;
            m_lastExtrapolatePosition.longitude = INVALID_LATLON;
            NB_NavigationSetLastExtrapolatePosition(m_nbNavigation, &m_lastExtrapolatePosition);
        }
    }
    if(m_status == NB_NS_Arrived)
    {
        m_notifier.RouteFinish();
    }
    if (m_lastETATime == 0 || gpsLocation->gpsTime > m_lastETATime + ETA_STATE_INTERVAL)
    {
        AddAnalyticsNavigationState("navigating");
        m_lastETATime = gpsLocation->gpsTime;
    }

     // call user listeners asynchronously
    (void)PAL_TimerSet(m_pal, 0, Static_PositionChangeCb, this);

    if (IsExtrapolateEnabled())
    {
        NB_NavigationSetLastExtrapolatePosition(m_nbNavigation, &m_lastExtrapolatePosition);
        NB_NavigationGetExtrapolatePath(m_nbNavigation, extrapolateDuration,
                                        m_extrapolatePointsInterval, &m_extrapolatePosition.front(),
                                        extrapolatePathMaxLength, &m_extrapolatePathLen, m_preferences.GetMinSpeedExtrapolationThreshold());

        if (m_extrapolatePathLen == 0)
        {
            NB_NavigationPublicState publicState;
            nsl_memset(&publicState, 0, sizeof(publicState));
            NB_NavigationGetPublicState(GetNbNavigation(), &publicState);
            m_extrapolatePosition[0].heading = publicState.onRoute ? publicState.currentRoutePosition.segmentHeading : m_lastPosition.heading;
            if (publicState.onRoute && publicState.status != NB_NS_NavigatingStartup)
            {
                m_extrapolatePosition[0].latitude = publicState.currentRoutePosition.projLat;
                m_extrapolatePosition[0].longitude = publicState.currentRoutePosition.projLon;
            }
            else
            {
                m_extrapolatePosition[0].latitude = m_lastPosition.latitude;
                m_extrapolatePosition[0].longitude = m_lastPosition.longitude;
            }
            m_extrapolatePosition[0].maneuver = publicState.currentRoutePosition.closestManeuver;
            m_extrapolatePosition[0].segment = publicState.currentRoutePosition.closestSegment;
            m_extrapolatePathLen = 1;
        }
        m_extrapolateCurrentPosition = 0;
        m_horizontalVelocity = m_lastPosition.horizontalVelocity;
        (void)PAL_TimerSet(m_pal, 0, Static_ExtraPolationPositionChangeCb, this);
    }
    else
    {
        m_lastExtrapolatePosition.latitude = INVALID_LATLON;
        m_lastExtrapolatePosition.longitude = INVALID_LATLON;
    }
errexit:
    UnLock();
}

NB_Error NavigationImpl::CreateNbNavSession()
{
    NAV_DEBUG_LOG(m_logger, "Creating NavSession\n");

    NB_Error result = NE_OK;
    int gpsCount = NB_GpsHistoryGetCount(m_gpsHistory);
    nb_boolean isGpsValid = TRUE;
    switch (m_startupState)
    {
        case SU_GoodInitial:
        {
            if (gpsCount == 0)
            {
                return NE_OK;
            }
            else if ((result = NB_GpsHistoryIfLastHeadingValid(m_gpsHistory, &isGpsValid)) == NE_OK)
            {
                // if the last gps fix has heading then start nav-query with the last gps fix
                if (isGpsValid)
                {
                    break;
                }
                else if (gpsCount < 2)
                {
                    // if there is not more that two gps fixes then wait another fix
                    return NE_OK;
                }
                // else start nav-query with two gps fixes.
            }
            break;
        }
        case SU_BadInitial:
        {
            if (gpsCount == 0)
            {
                // if there not any gps fixes than wait another one.
                return NE_OK;
            }
            break;
        }
        case SU_EnterEnhancedStartup:
        {
            if (m_preferences.GetEnableEnhancedNavigationStartup())
            {
                if (gpsCount > 0)
                {
                    // if the last fix is GPS fix then change the startup state into susBadInitial
                    m_startupState = SU_BadInitial;
                    break;
                }
                else if (m_lastNoGpsFix) // if there is not any gps fixes and has at least one no-gps
                                       // fix then use this no-gps fix to startup and enter enhanced startup state
                {
                    NB_GpsHistorySetLocationThreshhold(m_gpsHistory, LOCATION_THRESHHOLD, FALSE);
                    NB_GpsHistoryAdd(m_gpsHistory, m_lastNoGpsFix);

                    m_enhancedStartupPosition.latitude = m_lastNoGpsFix->latitude;
                    m_enhancedStartupPosition.longitude = m_lastNoGpsFix->longitude;
                    m_enhancedStartupPosition.accuracy = m_lastNoGpsFix->horizontalUncertaintyAlongAxis;
                    Coordinates* coordinate = new Coordinates(m_enhancedStartupPosition);
                    m_notifier.EnterEnhancedStartup(coordinate);
                    break;
                }
            }
            return NE_GPS_TIMEOUT;
        }
        case SU_ExitEnhancedStartup:
            break;
    }

    // if some error occur on gps history or gps timeout then return it
    if (result != NE_OK)
    {
        return result;
    }

    m_isStartupRunning = false;
    PAL_TimerCancel(m_pal, Static_StartupCb, this);

    NB_NavigationCallbacks callbacks = {0};
    callbacks.statusChangeCallback                = NbSessionStatusChangeCb;
    callbacks.guidanceMessageCallback             = NbSessionGuidanceCb;
    callbacks.routeCompleteDownloadCallback       = NbCompleteRouteDownloadCb;
    callbacks.routeDownloadCallback               = NbRouteDownloadCb;
    callbacks.trafficNotificationCallback         = NbTrafficNotifcationCb;
    callbacks.maneuverPositionChangeCallback      = NbManeuverPositionChangeCb;
    callbacks.enhancedContentNotificationCallback = NbEnhancedContentStateNotificationCb;
    callbacks.speedLimitCallback                  = NbSpeedLimitCb;
    callbacks.visualLaneGuidanceCallback          = NbVisualLaneGuidanceCb;
    callbacks.routeRequestedCallback              = NbRouteRequestedCb;
    callbacks.specialRegionCallback               = NbSpecialRegionCb;
    callbacks.getAnnounceSnippetLengthCallback    = NbAnnounceSnippetLength;
    callbacks.reachViaPointCallback               = NbReachViaPointCB;
    callbacks.userData = this;

    NB_RouteConfiguration routeConfiguration;
    SetupRouteConfiguration(&routeConfiguration, &m_preferences, m_preferences.GetEnableEnhancedNavigationStartup());

    NB_RouteOptions nbRouteOptions;
    SetupRouteOptions(&nbRouteOptions, &m_routeOptions, &m_preferences, m_voicesStyle.c_str());

    NB_NavigationConfiguration  configuration;
    SetupNavigationConfiguration(&configuration, &m_preferences);

    SetupForPedestrianAndBike(&nbRouteOptions, &routeConfiguration, &m_routeOptions);

    m_nbEnhancedManager = CreateEnhancedContentManager(m_context, &routeConfiguration, PAL_FileGetCachePath());

    // Start metadata synchronization.
    NB_EnhancedContentManagerStartMetadataSynchronization(m_nbEnhancedManager);

    NB_Place dest;
    if(m_destinations.size() > 0)
    {
        PlaceToNbPlace(m_destinations[0],&dest);
    }

    string baseVoicePath = GetBaseVoicesPath();
    m_nbGuidanceInfo = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                 baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                 m_locale.c_str(), IsTtsEnabled());
    if (m_nbGuidanceInfo == NULL)
    {
        return NE_INVAL;
    }

    m_nbGuidanceInfoForUI = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                      baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                      m_locale.c_str(), IsTtsEnabled());
    if (m_nbGuidanceInfoForUI == NULL)
    {
        return NE_INVAL;
    }

    routeConfiguration.commandSetVersion = NB_GuidanceInformationGetCommandVersion(m_nbGuidanceInfo);

    result = NB_NavigationCreate(m_context, &configuration, &callbacks, NULL, &m_nbNavigation);
    if (result == NE_OK)
    {
        ConfigNBNavigation(m_nbNavigation, m_preferences, GetMeasurementUnits());

        NB_RouteParameters* parameters = NULL;
        result = NB_RouteParametersCreateFromGpsHistory(m_context, m_gpsHistory, &dest,
                                                        &nbRouteOptions, &routeConfiguration, &parameters);
        NB_GpsHistorySetLocationThreshhold(m_gpsHistory, LOCATION_THRESHHOLD, TRUE);
        NB_GpsHistoryClear(m_gpsHistory);

        if (result == NE_OK)
        {
            (void)NB_RouteParametersSetLanguage(parameters, m_locale.c_str());

            (void)NB_NavigationSetGuidanceInformation(m_nbNavigation, m_nbGuidanceInfo);
            for(int i=0; i< m_wayPoints.size(); i++)
            {
                NB_WayPoint point;
                NB_Place place;
                PlaceToNbPlace(m_wayPoints[i].location,&place);
                point.location = place.location;
                point.isStopPoint = m_wayPoints[i].isStopPoint;
                NB_RouteParametersSetViaPoint(parameters, &point);
            }
            SetupLaneGuidanceConfiguration(m_context, parameters, GetPrivateWorkFolder());
            AddToneIntoVoiceCache(m_pal, m_context , GetPrivateWorkFolder());

            result = NB_NavigationStartWithRouteParameters(m_nbNavigation, parameters, FALSE);
        }

        (void)NB_RouteParametersDestroy(parameters);
    }
    return result;
}

const char* NavigationImpl::GetPrivateWorkFolder() const
{
    if (m_workPath.size())
    {
        return m_workPath.c_str();
    }
    return PAL_FileGetWorkPath();
}

NB_Error NavigationImpl::CreateStaticNbNavSession(const Place& origin, NB_NavigationCallbacks* callbacks)
{
    m_isStartupRunning = false;
    PAL_TimerCancel(m_pal, Static_StartupCb, this);

    NB_RouteConfiguration routeConfiguration;
    SetupStaticRouteConfiguration(&routeConfiguration, &m_preferences);

    NB_RouteOptions nbRouteOptions;
    SetupRouteOptions(&nbRouteOptions, &m_routeOptions, &m_preferences, m_voicesStyle.c_str());
    nbRouteOptions.traffic = (NB_TrafficType)(NB_TrafficType_HistoricalSpeed | NB_TrafficType_RealTimeSpeed);

    NB_NavigationConfiguration  configuration;
    SetupNavigationConfiguration(&configuration, &m_preferences);

    SetupForPedestrianAndBike(&nbRouteOptions, &routeConfiguration, &m_routeOptions);

    NB_Place destPlace;
    if(m_destinations.size() > 0)
    {
        PlaceToNbPlace(m_destinations[0],&destPlace);
    }

    NB_Place originPlace;
    PlaceToNbPlace(origin, &originPlace);

    string baseVoicePath = GetBaseVoicesPath();
    m_nbGuidanceInfo = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                 baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                 m_locale.c_str(), IsTtsEnabled());
    if (m_nbGuidanceInfo == NULL)
    {
        return NE_INVAL;
    }

    m_nbGuidanceInfoForUI = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                      baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                      m_locale.c_str(), IsTtsEnabled());
    if (m_nbGuidanceInfoForUI == NULL)
    {
        return NE_INVAL;
    }

    routeConfiguration.commandSetVersion = NB_GuidanceInformationGetCommandVersion(m_nbGuidanceInfo);

    NB_Error result = NB_NavigationCreate(m_context, &configuration, callbacks, NULL, &m_nbNavigation);
    if (result == NE_OK)
    {

        NB_RouteParameters* parameters = NULL;
        result = NB_RouteParametersCreateFromPlace(m_context, &originPlace, &destPlace,
                                                   &nbRouteOptions, &routeConfiguration, &parameters);
        if (result == NE_OK)
        {
            (void)NB_RouteParametersSetLanguage(parameters, m_locale.c_str());
            (void)NB_NavigationSetGuidanceInformation(m_nbNavigation, m_nbGuidanceInfo);
            SetupLaneGuidanceConfiguration(m_context, parameters, GetPrivateWorkFolder());
            for(int i=0; i< m_wayPoints.size(); i++)
            {
                NB_WayPoint point;
                NB_Place place;
                PlaceToNbPlace(m_wayPoints[i].location,&place);
                point.location = place.location;
                point.isStopPoint = m_wayPoints[i].isStopPoint;
                NB_RouteParametersSetViaPoint(parameters, &point);
            }
            SetupLaneGuidanceConfiguration(m_context, parameters, GetPrivateWorkFolder());
            result = NB_NavigationStartWithRouteParameters(m_nbNavigation, parameters, FALSE);
        }

        (void)NB_RouteParametersDestroy(parameters);
        m_analytics.StartAnalyticsSession(true);
    }
    return result;
}

NB_Error NavigationImpl::CreateSummaryNbNavSession(const Place& origin, NB_NavigationCallbacks* callbacks)
{
    PAL_TimerCancel(m_pal, Static_StartupCb, this);
    NB_RouteConfiguration routeConfiguration;
    SetupRouteSummaryConfiguration(&routeConfiguration);

    NB_RouteOptions nbRouteOptions;
    SetupRouteOptions(&nbRouteOptions, &m_routeOptions, &m_preferences, m_voicesStyle.c_str());

    NB_NavigationConfiguration  configuration;
    SetupNavigationConfiguration(&configuration, &m_preferences);

    NB_Place destPlace;
    if(m_destinations.size() > 0)
    {
        PlaceToNbPlace(m_destinations[0],&destPlace);
    }

    NB_Place originPlace;
    PlaceToNbPlace(origin, &originPlace);

    NB_Error result = NB_NavigationCreate(m_context, &configuration, callbacks, NULL, &m_nbNavigation);
    if (result == NE_OK)
    {
        NB_RouteParameters* parameters = NULL;
        result = NB_RouteParametersCreateFromPlace(m_context, &originPlace, &destPlace,
                                                   &nbRouteOptions, &routeConfiguration, &parameters);
        if (result == NE_OK)
        {
            (void)NB_RouteParametersSetLanguage(parameters, m_locale.c_str());
            if(m_destinations.size() > 1)
            {
                for(int i=1; i< m_destinations.size(); i++)
                {
                    NB_Place place = {{0}};
                    PlaceToNbPlace(m_destinations[i], &place);
                    NB_RouteParametersSetRouteSummaryDestination(parameters, &place);
                }
            }
            result = NB_NavigationStartWithRouteParameters(m_nbNavigation, parameters, FALSE);
        }

        (void)NB_RouteParametersDestroy(parameters);
        m_analytics.StartAnalyticsSession(true);
    }
    return result;
}

void NavigationImpl::DestroyNbNavSession()
{
    NAV_DEBUG_LOG(m_logger, "%p Destroying NavSession...", this);

    PAL_TimerCancel(m_pal, Static_PositionChangeCb, this);
    PAL_TimerCancel(m_pal, Static_ExtraPolationPositionChangeCb, this);
    PAL_TimerCancel(m_pal, Static_StartupCb, this);
    if (m_currentAnnouncement)
    {
        m_currentAnnouncement->OnAnnouncementComplete();
        m_currentAnnouncement.reset();
    }
    if (m_detour)
    {
        delete m_detour;
        m_detour = NULL;
    }

    if (m_nbNavigation)
    {
        NB_NavigationDestroy(m_nbNavigation);
        m_nbNavigation = NULL;
    }

    if (m_nbGuidanceInfo)
    {
        NB_GuidanceInformationDestroy(m_nbGuidanceInfo);
        m_nbGuidanceInfo = NULL;
    }
    if (m_nbGuidanceInfoForUI)
    {
        NB_GuidanceInformationDestroy(m_nbGuidanceInfoForUI);
        m_nbGuidanceInfoForUI = NULL;
    }

    if (m_nbEnhancedManager)
    {
        NB_EnhancedContentManagerDestroy(m_nbEnhancedManager);
        m_nbEnhancedManager = NULL;
        NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(m_context, NULL);
    }
    ClearActiveRoute();
    ClearNbRoutes();
    ClearNbDetours();

    if (m_gpsHistory)
    {
        NB_GpsHistoryDestroy(m_gpsHistory);
        m_gpsHistory = NULL;
    }
}

void NavigationImpl::PauseSession()
{
    nb::Lock lock(m_methodLock);

    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(
        m_pal, this, NULL, &NavigationImpl::PauseSessionScheduled, m_isValid);
    task->Execute();
}

void NavigationImpl::PauseSessionScheduled(int* data)
{
    (void)NB_NavigationSuspendSession(m_nbNavigation);
    AddAnalyticsNavigationState("pause-route");
}

void NavigationImpl::ResumeSession()
{
    nb::Lock lock(m_methodLock);

    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(
        m_pal, this, NULL, &NavigationImpl::ResumeSessionScheduled, m_isValid);
    task->Execute();
}

void NavigationImpl::ResumeSessionScheduled(int* data)
{
    (void)NB_NavigationResumeSession(m_nbNavigation);
    AddAnalyticsNavigationState("resume-route");
}

void NavigationImpl::StopSession()
{
    nb::Lock lock(m_methodLock);

    AddAnalyticsNavigationState("stop");

    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(
        m_pal, this, NULL, &NavigationImpl::StopSessionScheduled, m_isValid);
    task->Execute();
    NAV_DEBUG_LOG(m_logger, "StopSession called.");
}

void NavigationImpl::PlanTrip(const Place& originPlace)
{
    if (!m_nbNavigation)
    {
        m_planningTrip = true;
        StartStaticRouteRequest(originPlace);
    }
}

void NavigationImpl::StopSessionScheduled(int* data)
{
    *m_isValid = false;
    DestroyNbNavSession();
    /*! we should start analytics session if navigation started
     and end the session if navigation ended. */
    m_analytics.StartAnalyticsSession(false);
}

void NavigationImpl::AddSessionListener(SessionListener* listener)
{
    NB_NavigationPublicState publicState;
    nsl_memset(&publicState, 0, sizeof(publicState));
    Lock();
    (void)NB_NavigationGetPublicState(GetNbNavigation(), &publicState);
    UnLock();

    if (publicState.status != NB_NS_Invalid)
    {
        if (publicState.onRoute)
        {
            m_onOffRoute = rtOnRoute;
        }
        else
        {
            m_onOffRoute = rtOffRoute;
        }
    }
    m_notifier.AddSessionListener(listener, publicState.status == NB_NS_Invalid ? rtNone :  m_onOffRoute);
}

void NavigationImpl::RemoveSessionListener(SessionListener* listener)
{
    m_notifier.RemoveSessionListener(listener);
}

void NavigationImpl::AddRoutePositionListener(RoutePositionListener* listener)
{
    PositionChangeData data;
    ManeuverPositionChangeData maneuverPositionData;
    Lock();
    NavEventData navEventData(m_currentTotalMask,
                              m_currentLaneInfo ? new LaneInformation(*m_currentLaneInfo) : NULL,
                              m_currentSignInfo ? new RoadSign(*m_currentSignInfo) : NULL,
                              m_currentSpeedLimitInfo ? new SpeedLimitInformation(*m_currentSpeedLimitInfo) : NULL,
                              m_currentTrafficEvent ? new TrafficEvent(*m_currentTrafficEvent) : NULL,
                              m_currentTrafficInfo ? new TrafficInformation(*m_currentTrafficInfo) : NULL);

    data.currentHeading = m_currentHeading;
    data.currentPosition = m_currentPosition;
    data.currentSpeed = m_currentSpeed;
    data.maneuverRemainingDelay = m_maneuverRemainingDelay;
    data.maneuverRemainingDistance = m_maneuverRemainingDistance;
    data.maneuverRemainingTime = m_maneuverRemainingTime;
    data.trafficDelay = m_trafficDelay;
    data.tripRemainingDelay = m_tripRemainingDelay;
    data.tripRemainingDistance = m_tripRemainingDistance;
    data.tripRemainingTime = m_tripRemainingTime;
    data.tripTrafficColor = m_tripTrafficColor;
    data.maneuverImageStr = m_imageIdStr;
    data.stackTurnImageStr = m_stackTurnImageStr;

    maneuverPositionData.maneuverID = (int)m_currentManeuver;
    maneuverPositionData.currentPrimaryRoadNameStr = m_currentPrimaryRoadNameStr;
    maneuverPositionData.currentSecondaryRoadNameStr = m_currentSecondaryRoadNameStr;
    maneuverPositionData.exitNumberStr = m_exitNumberStr;
    maneuverPositionData.imageIdStr = m_imageIdStr;
    maneuverPositionData.commandStr = m_commandStr;
    maneuverPositionData.maneuverTurnPoint = m_maneuverTurnPoint;
    maneuverPositionData.nextPrimaryRoadNameStr = m_nextPrimaryRoadNameStr;
    maneuverPositionData.nextSecondaryRoadNameStr = m_nextSecondaryRoadNameStr;
    maneuverPositionData.stackTurnImageStr = m_stackTurnImageStr;
    maneuverPositionData.maneuvers = m_currentManeuverList ? new ManeuverList(*m_currentManeuverList) : NULL;
    UnLock();
    m_notifier.AddRoutePositionListener(listener, &data, &maneuverPositionData, &navEventData);
}

void NavigationImpl::RemoveRoutePositionListener(RoutePositionListener* listener)
{
    m_notifier.RemoveRoutePositionListener(listener);
}

void NavigationImpl::AddAnnouncementListener(AnnouncementListener* listener)
{
    m_notifier.AddAnnouncementListener(listener);
}

void NavigationImpl::RemoveAnnouncementListener(AnnouncementListener* listener)
{
    m_notifier.RemoveAnnouncementListener(listener);
}

void NavigationImpl::AddTrafficListener(TrafficListener* listener)
{
    Lock();
    NavEventData data(m_currentTotalMask, NULL, NULL, NULL,
                      m_currentTrafficEvent ? new TrafficEvent(*m_currentTrafficEvent) : NULL,
                      m_currentTrafficInfo ? new TrafficInformation(*m_currentTrafficInfo) : NULL);
    UnLock();
    m_notifier.AddTrafficListener(listener, &data);
}

void NavigationImpl::RemoveTrafficListener(TrafficListener* listener)
{
    m_notifier.RemoveTrafficListener(listener);
}

void NavigationImpl::AddPositionUpdateListener(PositionUpdateListener* listener)
{
    Lock();
    PositionChangeData data;
    data.currentHeading = m_currentHeading;
    data.currentPosition = m_currentPosition;
    data.currentSpeed = m_currentSpeed;
    data.maneuverRemainingDelay = m_maneuverRemainingDelay;
    data.maneuverRemainingDistance = m_maneuverRemainingDistance;
    data.maneuverRemainingTime = m_maneuverRemainingTime;
    data.trafficDelay = m_trafficDelay;
    data.tripRemainingDelay = m_tripRemainingDelay;
    data.tripRemainingDistance = m_tripRemainingDistance;
    data.tripRemainingTime = m_tripRemainingTime;
    data.tripTrafficColor = m_tripTrafficColor;
    UnLock();
    m_notifier.AddPositionUpdateListener(listener, &data);
}

void NavigationImpl::RemovePositionUpdateListener(PositionUpdateListener* listener)
{
    m_notifier.RemovePositionUpdateListener(listener);
}

void NavigationImpl::AddManeuverUpdateListener(ManeuverUpdateListener* listener)
{
    Lock();
    ManeuverPositionChangeData data;
    data.maneuverID = (int)m_currentManeuver;
    data.currentPrimaryRoadNameStr = m_currentPrimaryRoadNameStr;
    data.currentSecondaryRoadNameStr = m_currentSecondaryRoadNameStr;
    data.exitNumberStr = m_exitNumberStr;
    data.imageIdStr = m_imageIdStr;
    data.commandStr = m_commandStr;
    data.maneuverTurnPoint = m_maneuverTurnPoint;
    data.nextPrimaryRoadNameStr = m_nextPrimaryRoadNameStr;
    data.nextSecondaryRoadNameStr = m_nextSecondaryRoadNameStr;
    data.stackTurnImageStr = m_stackTurnImageStr;
    UnLock();
    m_notifier.AddManeuverUpdateListener(listener, &data);
}

void NavigationImpl::RemoveManeuverUpdateListener(ManeuverUpdateListener* listener)
{
    m_notifier.RemoveManeuverUpdateListener(listener);
}

void NavigationImpl::AddNavEventListener(NavEventListener* listener)
{
    Lock();
    NavEventData data(m_currentTotalMask,
                      m_currentLaneInfo && (m_currentTotalMask & NETP_LaneInformationEnable) ?
                      new LaneInformation(*m_currentLaneInfo) : new LaneInformation(),
                      m_currentSignInfo && (m_currentTotalMask & NETP_RoadSignEnable) ?
                      new RoadSign(*m_currentSignInfo) : new RoadSign(),
                      m_currentSpeedLimitInfo && (m_currentTotalMask & NETP_SpeedLimitEnable) ?
                      new SpeedLimitInformation(*m_currentSpeedLimitInfo) : new SpeedLimitInformation(),
                      m_currentTrafficEvent && (m_currentTotalMask & NETP_TrafficEventNotification) ?
                      new TrafficEvent(*m_currentTrafficEvent) : new TrafficEvent(),
                      m_currentTrafficInfo ? new TrafficInformation(*m_currentTrafficInfo) : new TrafficInformation());
    UnLock();
    m_notifier.AddNavEventListener(listener, &data);
}

void NavigationImpl::RemoveNavEventListener(NavEventListener* listener)
{
    m_notifier.RemoveNavEventListener(listener);
}

void NavigationImpl::AddSpecialRegionListener(SpecialRegionListener* listener)
{
    m_notifier.AddSpecialRegionListener(listener);
}

void NavigationImpl::RemoveSpecialRegionListener(SpecialRegionListener* listener)
{
    m_notifier.RemoveSpecialRegionListener(listener);
}

void NavigationImpl::AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener)
{
    m_notifier.AddEnhancedNavigationStartupListener(listener, m_preferences.GetEnableEnhancedNavigationStartup(),
                                                    m_startupState, &m_enhancedStartupPosition);
}

void NavigationImpl::RemoveEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener)
{
    m_notifier.RemoveEnhancedNavigationStartupListener(listener);
}

void NavigationImpl::Announce()
{
    nb::Lock lock(m_methodLock);
    Announce(NAV_MANEUVER_CURRENT);
}

void NavigationImpl::Announce(int manueverNumber)
{
    nb::Lock lock(m_methodLock);
    if (m_activeRoute)
    {
        Announce(manueverNumber, *m_activeRoute);
    }
}

void NavigationImpl::Announce(int manueverNumber, const RouteInformation& route)
{
    nb::Lock lock(m_methodLock);
    AnnounceData* data = new AnnounceData();
    data->maneuver = manueverNumber;
    data->route = &route;
    data->units = GetMeasurementUnits();

    NavTask<NavigationImpl, AnnounceData>* task = new NavTask<NavigationImpl, AnnounceData>(
        m_pal, this, data, &NavigationImpl::AnnounceScheduled, m_isValid);
    task->Execute();
}

void NavigationImpl::AnnounceScheduled(AnnounceData* data)
{
    if (data)
    {
        AnnounceData* announceData = static_cast<AnnounceData*>(data);
        if (!announceData || !announceData->route)
        {
            return;
        }

        NB_RouteInformation* routeFromNbNav = GetNbNavRouteInfo();
        NB_RouteInformation* routeFromNbRoute = GetNbRoute(*announceData->route);
        NB_RouteId* routeIDFromNbNav = NULL;
        NB_RouteId* routeIDFromNbRoute = NULL;

        if (routeFromNbNav)
        {
            NB_RouteInformationGetRouteId(routeFromNbNav, &routeIDFromNbNav);
        }
        if (routeFromNbRoute)
        {
            NB_RouteInformationGetRouteId(routeFromNbRoute, &routeIDFromNbRoute);
        }

        if (routeIDFromNbNav && routeIDFromNbRoute &&
            routeIDFromNbNav->size == routeIDFromNbRoute->size &&
            nsl_memcmp(routeIDFromNbNav->data, routeIDFromNbRoute->data, routeIDFromNbRoute->size) == 0)
        {
            NB_NavigationPlayManualGuidanceMessage(m_nbNavigation, announceData->maneuver, NB_NAT_Street, announceData->units);
        }
        else
        {
            NB_NavigationPlayManualGuidanceMessageForManeuverList(m_nbNavigation,
                                                                  GetNbRoute(*announceData->route),
                                                                  announceData->maneuver,
                                                                  NB_NAT_Street, announceData->units,
                                                                  m_lastPosition.latitude, m_lastPosition.longitude);
        }
    }
}

void NavigationImpl::DoDetour()
{
    nb::Lock lock(m_methodLock);

    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this, NULL,
                                                                          &NavigationImpl::DoDetourScheduled,
                                                                          m_isValid);
    task->Execute();
}

void NavigationImpl::DoDetourScheduled(int* data)
{
    if (m_detour)
    {
        delete m_detour;
    }
    m_detour = new Detour(this);
}

void NavigationImpl::CancelDetour()
{
    nb::Lock lock(m_methodLock);
    NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this, NULL,
                                                                          &NavigationImpl::DoCancelDetourScheduled,
                                                                          m_isValid);
    task->Execute();
}

void NavigationImpl::DoCancelDetourScheduled(int* data)
{
    if (m_detour && m_detour->IsRequesting())
    {
        delete m_detour;
        m_detour = NULL;
    }
}

bool NavigationImpl::SetActiveRoute(const RouteInformation& route)
{
    nb::Lock lock(m_methodLock);
    // here one place where nbRoute could be used in any thread
    // it is because of SetActiveRoute returns bool value
    nb::Lock nbRouteLock(m_memberLock);
    if (!IsRouteValid(route) && !IsDetourValid(route))
    {
        return false;
    }
    RouteInformation* routeData = new RouteInformation(route);
    NavTask<NavigationImpl, RouteInformation>* task = new NavTask<NavigationImpl, RouteInformation>(m_pal, this,
        routeData, &NavigationImpl::SetActiveRouteScheduled, m_isValid, false, false, false);
    task->Execute();
    return true;
}

void NavigationImpl::SetActiveRouteScheduled(RouteInformation* data)
{
    if (data)
    {
        RouteInformation* route = static_cast<RouteInformation*>(data);

        if (IsRouteValid(*route))
        {
            if (m_nbRoutes.size() == 1 ||
                NE_OK == NB_RouteInformationSetActiveRoute(GetNbNavRouteInfo(), GetNbRouteIndex(*route) + 1))
            {
                if (m_preferences.GetMultipleRoutesEnabled())
                {
                    NB_TrafficInformation* trafficInformation = NULL;
                    NB_RouteInformationGetTrafficInformation(GetNbNavRouteInfo(), &trafficInformation);
                    NB_NavigationUpdateTrafficInformation(m_nbNavigation, trafficInformation);
                }

                ClearActiveRoute();
                m_activeRoute = new RouteInformation(*route);
                m_isRouteActive = true;

                // auto recalc when current is plan route.
                if (route->m_isPlanRoute)
                {
                    m_planningTrip = false; //exit the plan trip state here
                    m_needToRecalcForPlanTrip = true;
                    delete route;
                    return;
                }

                /* Update call back data related with maneuver position changed event.
                 It would occur at adding listener when first get into navigation session and on route,
                 but ccc maneuver position changed callback has not invoked yet.
                 */
                NB_NavigationPublicState publicState;
                nsl_memset(&publicState, 0, sizeof(publicState));
                NB_NavigationGetPublicState(GetNbNavigation(), &publicState);
                if (m_currentManeuver == NAV_MANEUVER_NONE)
                {
                    if (publicState.onRoute)
                    {
                        UpdateManeuverPositionInformation(0);
                    }
                    else //set route name from public state
                    {
                        UpdateManeuverPositionInformation(publicState.currentManeuver);
                    }
                }

                // When upper want to start navigation with one selected route, and startup process
                // is in enter enhanced startup state, then play enhanced startup announcement
                if (m_startupState == SU_EnterEnhancedStartup)
                {
                    NB_GuidanceMessage* pMessage = NULL;
                    NB_RouteInformationGetTurnAnnouncementExt(GetNbNavRouteInfo(), GetNbGuidanceInfo(),
                                                              NAV_MANEUVER_ENHANCED_STARTUP, NB_NMP_Turn,
                                                              NB_NAS_Lookahead, NB_NAT_Street,
                                                              GetMeasurementUnits(), &pMessage,
                                                              m_lastPosition.latitude, m_lastPosition.longitude);
                    if (pMessage)
                    {
                        Announce(pMessage);
                    }
                }

                /* Auto update position of ccc with last gps position, when the case is multiple route. */
                 NB_GpsLocation* gpsLocation = new NB_GpsLocation();
                nsl_memset(gpsLocation, 0, sizeof(NB_GpsLocation));
                nsl_memcpy(gpsLocation, &m_lastPosition, sizeof(NB_GpsLocation));
                NavTask<NavigationImpl, NB_GpsLocation>* task = new NavTask<NavigationImpl, NB_GpsLocation>(
                    m_pal, this, gpsLocation, &NavigationImpl::UpdatePositionScheduled, m_isValid, true);
                task->Execute();

                if (m_preferences.GetMockGpsEnabled())
                {

                    if(m_mockProvider == NULL)
                    {
                    m_mockProvider = new NavMockGPSProvider(this, m_pal, m_nbNavigation);
                    m_mockProvider->SetSpeed(m_preferences.GetMockGpsSpeed());
                    m_mockProvider->Start();
                    }
                }

                delete route;
                return;
            }
        }
        else if (IsDetourValid(*route))
        {
            if(m_detour->ApplyDetour(*route))
            {
                m_lastETATime = 0;

                // reset m_currentManeuver so that we can update maneuver information in NbManeuverPositionChangeCb
                m_currentManeuver = NAV_MANEUVER_NONE;
                m_closestManeuver = NAV_MANEUVER_NONE;
                delete m_detour;
                m_detour = NULL;
                delete route;

                if (m_mockProvider)
                {
                    m_mockProvider->Reset();
                }
                return;
            }
        }
        delete route;

        // send error if we are here
        SetRouteError(NRE_Internal);
    }
}

void NavigationImpl::NbSessionStatusChangeCb(NB_Navigation* navigation, NB_NavigateStatus status, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    
    if (me)
    {
        me->m_status = status;
        switch (status)
        {
        case NB_NS_Arrived:
            {
                me->m_isRouteArrived = true;

                if (me->m_mockProvider)
                {
                    me->m_mockProvider->Stop();
                }

                /* Add arrival m_analytics when route finished. */
                uint32 sessionID = 0;
                NB_NavigationGetSessionId(me->m_nbNavigation, &sessionID);
                me->m_analytics.AddArrivalEvent(sessionID);
            }
            break;
        case NB_NS_NavigatingArriving:
            {
                char commandBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
                NB_NavigationPublicState publicState;
                nsl_memset(&publicState, 0, sizeof(publicState));
                NB_NavigationGetPublicState(me->m_nbNavigation, &publicState);
                NB_RouteInformationGetTurnInformation2(me->GetNbNavRouteInfo(), publicState.currentManeuver,
                                                       NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL,
                                                       NULL, commandBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN, TRUE);

                if (me->m_activeRoute)
                {
                    DestinationStreetSide* streetSide = new DestinationStreetSide(Center);
                    *streetSide = me->m_activeRoute->GetDestinationStreetSide();
                    me->m_notifier.RouteArriving(streetSide);
                }
            }
            break;
        case NB_NS_NavigatingConfirmRecalc:
            {
                NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(
                    me->m_pal, me, NULL, &NavigationImpl::RecalculateScheduled, me->m_isValid, true);
                task->Execute();
            }
            break;
        default:
            break;
        }
    }
}

void NavigationImpl::NbSessionGuidanceCb(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (NB_GuidanceMessageGetType(message) == NB_GMT_HeadsUp)
        {
            me->m_notifier.AnnouncementHeadsUp();
        }
        else
        {
            NB_NavigateManeuverPos pos = NB_GuidanceMessageGetManeuverPos(message);
            if(pos == NB_NMP_Continue && !me->m_preferences.GetContinueAnnouncementEnable())
            {
                NB_GuidanceMessagePlayed(message);
                NB_GuidanceMessageDestroy(message);
            }
            else
            {
            me->Announce(message);
        }
    }
}
}

void NavigationImpl::NbManeuverPositionChangeCb(NB_Navigation* navigation, uint32 maneuver,
                                                NB_NavigateManeuverPos maneuverPosition, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);

    if (me && me->m_currentManeuver != maneuver)
    {
        me->m_currentManeuver = maneuver;
        if (me->UpdateManeuverPositionInformation(me->m_currentManeuver) == NE_OK)
        {
            ManeuverPositionChangeData* data = new ManeuverPositionChangeData();
            data->maneuverID = (int)me->m_currentManeuver;
            data->commandStr = me->m_commandStr;
            data->currentPrimaryRoadNameStr = me->m_currentPrimaryRoadNameStr;
            data->currentSecondaryRoadNameStr = me->m_currentSecondaryRoadNameStr;
            data->exitNumberStr = me->m_exitNumberStr;
            data->imageIdStr = me->m_imageIdStr;
            data->maneuverTurnPoint = me->m_maneuverTurnPoint;
            data->nextPrimaryRoadNameStr = me->m_nextPrimaryRoadNameStr;
            data->nextSecondaryRoadNameStr = me->m_nextSecondaryRoadNameStr;
            data->stackTurnImageStr = me->m_stackTurnImageStr;

            PAL_LockLock(me->m_memberLock);
            if (me->m_currentManeuverList)
            {
                delete me->m_currentManeuverList;
                me->m_currentManeuverList = NULL;
            }
            me->m_currentManeuverList = new ManeuverList(me, me->m_activeRoute->m_index, true);
            PAL_LockUnlock(me->m_memberLock);
            data->maneuvers = new ManeuverList(*me->m_currentManeuverList);

            me->m_notifier.ManeuverPositionChange(data);
        }
    }
}
    
void NavigationImpl::NbReachViaPointCB(NB_Location* location, void* userData)
{
    if (!location || !userData)
    {
        return;
    }
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        NB_Place nbPlace;
        nsl_memset(&nbPlace, 0, sizeof(nbPlace));
        nbPlace.location = *location;
        Place* place = new Place;
        *place = NbPlaceToPlace(nbPlace);
        me->m_notifier.ReachViaPoint(place);
    }
}

void NavigationImpl::NbEnhancedContentStateNotificationCb(NB_Navigation* navigation, NB_EnhancedContentState* state,
                                                          NB_EnhancedContentStateData available, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (me->m_currentSignInfo)
        {
            delete me->m_currentSignInfo;
            me->m_currentSignInfo = NULL;
        }

        if (available)
        {
            me->m_currentSignInfo = new RoadSign(*me, static_cast<void*>(state));
        }
        if (me->m_currentSignInfo && me->m_currentSignInfo->GetImageData().GetData().size() > 0)
        {
            me->m_currentMask = NETP_RoadSignEnable;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_RoadSignDisable);
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_RoadSignEnable);
        }
        else
        {
            me->m_currentMask = NETP_RoadSignDisable;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_RoadSignEnable);
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_RoadSignDisable);
        }

        NavEventData* data = new NavEventData(me->m_currentMask, new LaneInformation(),
                                              me->m_currentMask == NETP_RoadSignEnable ?
                                              new RoadSign(*me->m_currentSignInfo) : new RoadSign(),
                                              new SpeedLimitInformation(), new TrafficEvent(), new TrafficInformation());
        me->m_notifier.NavEvent(data);
    }
}

void NavigationImpl::NbSpeedLimitCb(NB_SpeedLimitStateData* speedlimitData, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (me->m_currentSpeedLimitInfo)
        {
            delete me->m_currentSpeedLimitInfo;
            me->m_currentSpeedLimitInfo = NULL;
        }

        if (speedlimitData && (speedlimitData->showSchoolZone || speedlimitData->showSpeedLimit))
        {
            me->m_currentMask = NETP_SpeedLimitEnable;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_SpeedLimitDisable);
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_SpeedLimitEnable);
            me->m_currentSpeedLimitInfo = new SpeedLimitInformation(*me, static_cast<void*>(speedlimitData));
        }
        else
        {
            me->m_currentMask = NETP_SpeedLimitDisable;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_SpeedLimitEnable);
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_SpeedLimitDisable);
        }

        NavEventData* data = new NavEventData(me->m_currentMask, new LaneInformation(), new RoadSign(),
                                              me->m_currentMask == NETP_SpeedLimitEnable ?
                                              new SpeedLimitInformation(*me->m_currentSpeedLimitInfo)
                                              : new SpeedLimitInformation(), new TrafficEvent(), new TrafficInformation());
        me->m_notifier.NavEvent(data);
    }
}

void NavigationImpl::NbVisualLaneGuidanceCb(NB_Navigation* navigation, unsigned long manueverIndex,
                                            unsigned long laneCount, NB_Lane * laneItems, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (me->m_currentLaneInfo)
        {
            delete me->m_currentLaneInfo;
            me->m_currentLaneInfo = NULL;
        }

        if (laneCount > 0)
        {
            me->m_currentLaneInfo = new LaneInformation(*me, manueverIndex, laneCount, static_cast<void*>(laneItems));
            me->m_currentMask = NETP_LaneInformationEnable;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_LaneInformationDisable);
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_LaneInformationEnable);
        }
        else
        {
            if (me->m_currentTotalMask & NETP_LaneInformationEnable)
            {
                me->m_currentMask = NETP_LaneInformationDisable;
                me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_LaneInformationEnable);
                me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_LaneInformationDisable);
            }
            else
            {
                return;
            }
        }

        NavEventData* data = new NavEventData(me->m_currentMask, me->m_currentMask == NETP_LaneInformationEnable ?
                                              new LaneInformation(*me->m_currentLaneInfo) : new LaneInformation(),
                                              new RoadSign(), new SpeedLimitInformation(),
                                              new TrafficEvent(), new TrafficInformation());
        me->m_notifier.NavEvent(data);
    }
}

void NavigationImpl::NbCompleteRouteDownloadCb(void* handler, NB_RequestStatus status, NB_Error err,
                                               nb_boolean up, int percent, void* userData)
{
    if (up || userData == NULL)
    {
        return;
    }

    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (status == NB_NetworkRequestStatus_Success && err == NE_OK)
        {
            NAV_DEBUG_LOG(me->m_logger, "Route received");

            me->m_currentManeuver = NAV_MANEUVER_NONE;
            me->m_closestManeuver = NAV_MANEUVER_NONE;
            me->m_notifier.SetRouteReceived(true);
            /* Add navigation reply and navigation state m_analytics when route downloaded. */
            uint32 sessionID = 0;
            NB_NavigationPublicState publicState;
            NB_NavigationGetSessionId(me->m_nbNavigation, &sessionID);
            NB_NavigationGetPublicState(me->m_nbNavigation, &publicState);
            me->m_analytics.AddRouteReplyEvent(sessionID, publicState.tripRemainTime, publicState.tripRemainDistance);
            me->AddAnalyticsNavigationState(me->m_routeRequestedReason == SessionListener::Recalculate ? "recalc" : "new-route");
            if (me->m_routeRequestedReason == SessionListener::Recalculate)
            {
                me->m_lastETATime = 0;
            }

            me->UpdateNbRoutes(me->GetNbNavRouteInfo(), false);

            me->m_isNewRouteForPlanTripReceived = true;

            /* set m_activeRoute with the received new route, in the case of recalculate and after
               one route is selected to start navigation session. */
            if (me->m_routeRequestedReason == SessionListener::Recalculate && me->m_isRouteActive && !me->m_activeRoute)
            {
                me->m_activeRoute = new RouteInformation(me, me->m_nbRoutes[0], 0);
            }

            me->m_updateTrafficInternal = true;
            NavigationImpl::NbTrafficNotifcationCb(handler, status, err, up, percent, userData);
            me->m_updateTrafficInternal = false;

            /* Auto update position of ccc with last gps position, when the case is single route. */
            if (!me->m_preferences.GetMultipleRoutesEnabled())
            {
                NB_GpsLocation* gpsLocation = new NB_GpsLocation();
                nsl_memset(gpsLocation, 0, sizeof(NB_GpsLocation));
                nsl_memcpy(gpsLocation, &me->m_lastPosition, sizeof(NB_GpsLocation));
                NavTask<NavigationImpl, NB_GpsLocation>* task = new NavTask<NavigationImpl, NB_GpsLocation>(
                    me->m_pal, me, gpsLocation, &NavigationImpl::UpdatePositionScheduled, me->m_isValid, true);
                task->Execute();
            }
        }
    }
}

void NavigationImpl::NbRouteDownloadCb(void* handler, NB_RequestStatus status, NB_Error err,
                                       nb_boolean up, int percent, void* userData)
{
    if (up || userData == NULL)
    {
        return;
    }

    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (status == NB_NetworkRequestStatus_Failed || status == NB_NetworkRequestStatus_TimedOut || err != NE_OK)
        {
            NB_NavigateRouteError nbRouteError = NB_NRE_None;
            NB_RouteHandler* routeHandler = static_cast<NB_RouteHandler*>(handler);

            NavigateRouteError error = me->NbErrorToRouteError(err);
            if (error == NRE_None)
            {
                err = NB_RouteHandlerGetRouteError(routeHandler, &nbRouteError);
                if(err == NE_OK && nbRouteError != NB_NRE_None)
                {
                    error = me->NbRouteErrorToRouteError(nbRouteError);
                }
            }

            me->SetRouteError(error);
        }
        else
        {
            me->m_routeError = NRE_None;
            me->m_requestedPercent = percent;
            int* data = new int(percent);
            me->m_notifier.RouteProgress(data);
        }
    }
}

void NavigationImpl::NbTrafficNotifcationCb(void* handler, NB_RequestStatus status, NB_Error err,
                                            nb_boolean up, int percent, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (me->m_currentTrafficInfo)
        {
            delete me->m_currentTrafficInfo;
            me->m_currentTrafficInfo = NULL;
        }

        if (status == NB_NetworkRequestStatus_Success)
        {
            me->m_currentTrafficInfo = new TrafficInformation(me, static_cast<void*>(me->GetNbNavRouteInfo()));
            me->m_currentMask = NETP_TrafficUpdate;
            me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_TrafficUpdate);
            
            if(me->m_activeRoute && !me->m_updateTrafficInternal)
            {
                me->m_activeRoute->UpdateTraffic(true);
                RouteReceivedData* data = new RouteReceivedData();
                data->routeRequestedReason = SessionListener::TrafficUpdate;
                data->receivedRoutes.push_back(*(me->m_activeRoute));
                me->m_notifier.RouteReceived(data);
            }

            NavEventData* data = new NavEventData(me->m_currentMask, new LaneInformation(),
                                              new RoadSign(), new SpeedLimitInformation(),
                                              new TrafficEvent(), new TrafficInformation(*me->m_currentTrafficInfo));
            me->m_notifier.NavEvent(data);
        }
    }
}

void NavigationImpl::NotifyPosition(const Coordinates& coordinates, double speed, double heading, bool isExtrapolateFix)
{
    m_currentPosition = coordinates;
    m_currentSpeed = speed;
    m_currentHeading = heading;

    PositionUpdatedData* positionData = new PositionUpdatedData();
    positionData->currentHeading = m_currentHeading;
    positionData->currentPosition = m_currentPosition;
    positionData->currentSpeed = m_currentSpeed;
    if ((IsExtrapolateEnabled() && isExtrapolateFix && IsSuitablePosition()))
    {
        m_lastExtrapolatePosition.latitude = m_extrapolatePosition.at(m_extrapolateCurrentPosition).latitude;
        m_lastExtrapolatePosition.longitude = m_extrapolatePosition.at(m_extrapolateCurrentPosition).longitude;
        m_lastExtrapolatePosition.heading = m_extrapolatePosition.at(m_extrapolateCurrentPosition).heading;
        m_lastExtrapolatePosition.maneuver = m_extrapolatePosition.at(m_extrapolateCurrentPosition).maneuver;
        m_lastExtrapolatePosition.segment = m_extrapolatePosition.at(m_extrapolateCurrentPosition).segment;
        m_notifier.PositionUpdated(positionData);
    }
    else if (!IsExtrapolateEnabled() && !isExtrapolateFix)
    {
        m_notifier.PositionUpdated(positionData);
    }


    if (!isExtrapolateFix)
    {
        NB_NavigationPublicState publicState;
        nsl_memset(&publicState, 0, sizeof(publicState));
        NB_NavigationGetPublicState(GetNbNavigation(), &publicState);

        NB_NavigationGetTripRemainingInformation(m_nbNavigation, &m_tripRemainingTime, &m_tripRemainingDelay, NULL);
        NB_NavigationGetTrafficColor(m_nbNavigation, &m_tripTrafficColor);
        NB_NavigationGetTraveledDistance(m_nbNavigation, &m_maneuverTraveledDistance);

        NB_TrafficInformationGetSummary(GetNbNavTrafficInfo(), GetNbNavRouteInfo(), m_closestManeuver,
                                        m_maneuverTraveledDistance, m_closestManeuver, NULL, NULL, NULL, &m_trafficDelay);

        NB_CommandFont maneuverTurnCharacter = {0};
        NB_CommandFont stackTurnCharacter = {0};

        if (publicState.stackNman != NAV_MANEUVER_NONE)
        {
            NB_RouteInformationGetTurnCommandCharacter(GetNbNavRouteInfo(), GetNbGuidanceInfo(), publicState.stackNman,
                NB_NMP_Turn, GetMeasurementUnits(), &stackTurnCharacter);
        }

        NB_RouteInformationGetTurnCommandCharacter(GetNbNavRouteInfo(), GetNbGuidanceInfo(),
                                                   publicState.currentManeuver, NB_NMP_Turn, GetMeasurementUnits(),
                                                   &maneuverTurnCharacter);

        m_tripRemainingDistance = publicState.tripRemainDistance;
        m_maneuverRemainingTime = publicState.turnRemainTime;
        m_maneuverRemainingDelay = m_trafficDelay;

        //start up offroute use the distance from orgin
        if ((m_currentManeuver == NAV_MANEUVER_NONE || m_currentManeuver == NAV_MANEUVER_START) && !publicState.onRoute)
        {
            m_maneuverRemainingDistance = publicState.distToOrigin;
        }
        else
        {
            m_maneuverRemainingDistance = publicState.turnRemainDistance;
        }

        bool ifNeedUpdateStackImage = false;
        bool ifNeedUpdateManeuverImage = false;
        if (maneuverTurnCharacter.character)
        {
            if (m_imageIdStr.compare((char*)(&maneuverTurnCharacter.character)) != 0)
            {
                ifNeedUpdateManeuverImage = true;
                m_imageIdStr.assign((char*)(&maneuverTurnCharacter.character));
            }
        }
        if (stackTurnCharacter.character)
        {
            if (m_stackTurnImageStr.compare((char*)(&stackTurnCharacter.character)) != 0)
            {
                ifNeedUpdateStackImage = true;
                m_stackTurnImageStr.assign((char*)(&stackTurnCharacter.character));
            }
        }
        else if (!m_stackTurnImageStr.empty())
        {
            ifNeedUpdateStackImage = true;
            m_stackTurnImageStr.clear();
        }

        PositionChangeData* data = new PositionChangeData();
        data->currentHeading = m_currentHeading;
        data->currentPosition = m_currentPosition;
        data->currentSpeed = m_currentSpeed;
        data->maneuverRemainingDistance = m_maneuverRemainingDistance;
        data->maneuverRemainingTime = m_maneuverRemainingTime;
        data->trafficDelay = m_trafficDelay;
        data->tripRemainingDelay = m_tripRemainingDelay;
        data->tripRemainingDistance = m_tripRemainingDistance;
        data->tripRemainingTime = m_tripRemainingTime;
        data->tripTrafficColor = m_tripTrafficColor;
        data->ifNeedUpdateManeuverImage = ifNeedUpdateManeuverImage;
        data->maneuverImageStr = m_imageIdStr;
        data->ifNeedUpdateStackImage = ifNeedUpdateStackImage;
        data->stackTurnImageStr = m_stackTurnImageStr;
        m_notifier.PositionChange(data);
    }
}

void NavigationImpl::Static_ExtraPolationPositionChangeCb(PAL_Instance *m_pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        NavigationImpl* me = static_cast<NavigationImpl*>(userData);
        if (me)
        {
            Coordinates coord;
            coord.latitude = me->m_extrapolatePosition.at(me->m_extrapolateCurrentPosition).latitude;
            coord.longitude = me->m_extrapolatePosition.at(me->m_extrapolateCurrentPosition).longitude;
            coord.accuracy = me->m_lastPosition.horizontalUncertaintyAlongAxis;
            double heading = me->m_extrapolatePosition.at(me->m_extrapolateCurrentPosition).heading;
            me->NotifyPosition(coord, me->m_horizontalVelocity, heading, true);
            if (++me->m_extrapolateCurrentPosition < me->m_extrapolatePathLen)
            {
                (void)PAL_TimerSet(m_pal, me->m_extrapolatePointsInterval, Static_ExtraPolationPositionChangeCb, me);
            }
        }
    }
}

void NavigationImpl::Static_StartupCb(PAL_Instance* m_pal, void* userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        NavigationImpl* me = static_cast<NavigationImpl*>(userData);
        if (!me)
        {
            return;
        }

        if(me->m_startupTimeoff <= me->m_preferences.GetEnableEnhancedNavigationStartupTimeout())
        {
            if (me->m_startupTimeoff < BADINITIAL_STARTUP_TIMEOFF)
            {
                me->m_startupState = SU_GoodInitial;
            }
            else if (me->m_startupTimeoff < ENHANCED_STARTUP_TIMEOFF)
            {
                me->m_startupState = SU_BadInitial;
            }
        }
        else
        {
            me->m_startupState = SU_EnterEnhancedStartup;
            me->m_isStartupRunning = false;
            // there is not any gps fixes and no-gps fixes then return gps timeout error
            if (NB_GpsHistoryGetCount(me->m_gpsHistory) == 0 && !me->m_lastNoGpsFix)
            {
                me->SetRouteError(NRE_GPSTimeout);
            }
        }
        me->m_startupTimeoff++;
        if (me->m_isStartupRunning)
        {
            PAL_TimerSet(m_pal, STARTUP_PROCESS_INTERVAL, Static_StartupCb, me);
        }
    }
}

void NavigationImpl::Static_PositionChangeCb(PAL_Instance *m_pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        NavigationImpl* me = static_cast<NavigationImpl*>(userData);
        if (me)
        {
            Coordinates coord = {0};
            NB_NavigationPublicState publicState;
            nsl_memset(&publicState, 0, sizeof(publicState));
            (void)NB_NavigationGetPublicState(me->GetNbNavigation(), &publicState);

            // update route type and heading based on current state.
            OnOffRouteType routeType = publicState.onRoute ? rtOnRoute : rtOffRoute;
            double heading = publicState.onRoute ? publicState.currentRoutePosition.segmentHeading : me->m_lastPosition.heading;

            if (publicState.onRoute && publicState.status != NB_NS_NavigatingStartup)
            {
                coord.latitude = publicState.currentRoutePosition.projLat;
                coord.longitude = publicState.currentRoutePosition.projLon;
            }
            else
            {
                coord.latitude = me->m_lastPosition.latitude;
                coord.longitude = me->m_lastPosition.longitude;
            }

            coord.accuracy = me->m_lastPosition.horizontalUncertaintyAlongAxis;

            if (me->m_onOffRoute != routeType)
            {
                me->m_onOffRoute = routeType;
                if (me->m_onOffRoute == rtOffRoute)
                {
                    NAV_DEBUG_LOG(me->m_logger, "Status changed: OnRoute --> OffRoute");
                    me->m_notifier.OffRoute();
                }
                else if (me->m_onOffRoute == rtOnRoute)
                {
                    NAV_DEBUG_LOG(me->m_logger, "Status changed: OffRoute --> OnRoute");
                    me->m_notifier.OnRoute();
                }
            }

            me->NotifyPosition(coord, me->m_lastPosition.horizontalVelocity, heading, false);

            // at startup off route state.
            if (publicState.status == NB_NS_NavigatingStartup && !publicState.onRoute &&
                (me->m_routeRequestedReason == SessionListener::Calculate ||
                 me->m_routeRequestedReason == SessionListener::RouteSelector ||
                 me->m_routeRequestedReason == SessionListener::Recalculate))
            {
                double* headingToOrigin = new double;
                if (NB_RouteInformationGetDirectionToRoute(me->GetNbNavRouteInfo(),
                                                           me->m_currentPosition.latitude,
                                                           me->m_currentPosition.longitude, NULL,
                                                           headingToOrigin) == NE_OK)
                {
                    me->m_notifier.OffRoutePositionUpdate(headingToOrigin);
                }
                else
                {
                    delete headingToOrigin;
                }
            }

            NB_TrafficEvent nbTrafficEvent;
            bool needUpdateForDisabled = false;
            if (NB_NavigationGetNextTrafficEvent(me->m_nbNavigation, &nbTrafficEvent) == NE_OK)
            {
                if (me->m_currentTrafficEvent)
                {
                    delete me->m_currentTrafficEvent;
                    me->m_currentTrafficEvent = NULL;
                }
                if (nbTrafficEvent.type != NB_TET_None && nbTrafficEvent.distanceToEvent <= minimumDistanceBeforeIncidentAnnouncement)
                {
                    me->m_currentTrafficEvent = new TrafficEvent(me, static_cast<void*>(&nbTrafficEvent),
                                                               static_cast<void*>(me->GetNbNavRouteInfo()));
                    me->m_currentMask = NETP_TrafficEventNotification;
                    me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_TrafficEventDisable);
                    me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_TrafficEventNotification);
                }
                else
                {
                    if (me->m_currentTotalMask & NETP_TrafficEventNotification)
                    {
                        needUpdateForDisabled = true;
                    }
                    me->m_currentMask = NETP_TrafficEventDisable;
                    me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask & ~NETP_TrafficEventNotification);
                    me->m_currentTotalMask = (NavEventTypeMask)(me->m_currentTotalMask | NETP_TrafficEventDisable);
                }

                if (me->m_currentMask == NETP_TrafficEventNotification ||
                    (me->m_currentMask == NETP_TrafficEventDisable && needUpdateForDisabled))
                {
                    NavEventData* data = new NavEventData(me->m_currentMask, new LaneInformation(),
                                                          new RoadSign(), new SpeedLimitInformation(),
                                                          me->m_currentMask == NETP_TrafficEventNotification ?
                                                          new TrafficEvent(*me->m_currentTrafficEvent) : new TrafficEvent(),
                                                          new TrafficInformation());
                    me->m_notifier.NavEvent(data);
                }
            }
        }
    }
}

void NavigationImpl::NbRouteRequestedCb(NB_Navigation* navigation, NB_NavigateRouteRequestReason reason, void* userData)
{
    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        uint32 sessionId = 0;
        NB_NavigationGetSessionId(navigation, &sessionId);

        switch (reason)
        {
            case NB_NRRR_Recalculation:
            {
                me->m_routeRequestedReason = SessionListener::Recalculate;

                NB_NavigationPublicState publicState;
                nsl_memset(&publicState, 0, sizeof(publicState));
                NB_Error error = NB_NavigationGetPublicState(me->m_nbNavigation, &publicState);
                if (error == NE_OK && publicState.currentRoutePosition.wrongWay)
                {
                    me->m_analytics.AddRouteRequestEvent("wrong-way", sessionId);
                }
                else
                {
                    me->m_analytics.AddRouteRequestEvent("off-route", sessionId);
                }
                break;
            }
            case NB_NRRR_RouteSelector:
            {
                me->m_routeRequestedReason = SessionListener::RouteSelector;

                me->m_analytics.AddRouteRequestEvent("init", sessionId);
                break;
            }
            case NB_NRRR_Detour:
            {
                me->m_routeRequestedReason = SessionListener::Detour;

                me->m_analytics.AddRouteRequestEvent("detour", sessionId);
                break;
            }
            case NB_NRRR_InitialRoute:
            default:
            {
                me->m_routeRequestedReason = SessionListener::Calculate;

                me->m_analytics.AddRouteRequestEvent("init", sessionId);
                break;
            }
        }

        SessionListener::RouteRequestReason* data = new SessionListener::RouteRequestReason(me->m_routeRequestedReason);
        me->m_notifier.RouteRequested(data);
    }
}

void NavigationImpl::NbSpecialRegionCb(NB_SpecialRegionStateData* specialRegionData, void* userData)
{
    if (!specialRegionData)
    {
        return;
    }

    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        SpecialRegionInformation* specialRegion = new SpecialRegionInformation(specialRegionData);
        if (specialRegionData->showSpecialRegion)
        {
            me->m_notifier.SpecialRegion(specialRegion);
        }
        else
        {
            me->m_notifier.DisableSpecialRegion(specialRegion);
        }
    }
}

double NavigationImpl::NbAnnounceSnippetLength(const char* AnnounceCode, void* userData)
{
    if (!AnnounceCode)
    {
        return 0;
    }

    NavigationImpl* me = static_cast<NavigationImpl*>(userData);
    if (me)
    {
        if (me->IsTtsEnabled())
        {
            char textBuffer[bufferSize] = {0};
            byte phoneticsData[bufferSize] = {0};
            uint32 textBufferSize = bufferSize;
            uint32 phoneticsDataSize = bufferSize;
            if (NB_NavigationGetPronunInformation(me->GetNbNavigation(), AnnounceCode, textBuffer,
                                                  &textBufferSize, phoneticsData, &phoneticsDataSize,NULL,NULL,NULL,NULL,NULL,NULL) == NE_OK)
            {
                if (textBufferSize > announceLengthLimit)
                {
                    return estimateofLongerAnnounce;
                }
                else
                {
                    return estimateOfShorterAnnounce;
                }
            }
        }
        else
        {
            CSL_Cache* voiceCache = NB_ContextGetVoiceCache(me->GetNbContext());
            if (voiceCache)
            {
                byte* pData = NULL;
                size_t dataSize = 0;
                double duration = 0;
                if(CSL_CacheFind(voiceCache, (byte*)AnnounceCode, nsl_strlen(AnnounceCode),
                                 &pData, &dataSize, TRUE, TRUE, FALSE) == NE_OK)
                {
                    ABPAL_AudioGetEstimatedDuration(pData, (uint32)dataSize, &duration);
                }
                if (pData && dataSize > 0)
                {
                    nsl_free(pData);
                }
                return duration;
            }
        }
    }
    return 0;
}

void NavigationImpl::Announce(NB_GuidanceMessage* message)
{
    if (message)
    {
        shared_ptr<Announcement>* announcement = new shared_ptr<Announcement>(new AnnouncementImpl(*this, static_cast<void*>(message)));

        if (announcement)
        {
            // add traffic announcement analytics event
            double playTimes = 0;
            nb_boolean isOverAll = FALSE;
            nb_boolean isCongestion = FALSE;
            nb_boolean isInCongestion = FALSE;
            char trafficColor = '\0';
            if (NB_NavigationGetTrafficGuidanceInformation(m_nbNavigation, message, &playTimes, &isOverAll,
                                                           &isCongestion, &isInCongestion, &trafficColor) == NE_OK)
            {
                uint32 sessionID = 0;
                NB_NavigationGetSessionId(m_nbNavigation, &sessionID);
                TrafficAnnouncementType trafficAnnounceType = TAT_OverAll;
                TrafficColor trafficAnnounceColor = TC_Incident;
                if (!isOverAll)
                {
                    if (isCongestion)
                    {
                        if (isInCongestion)
                        {
                            trafficAnnounceType = TAT_In_Congestion;
                        }
                        else
                        {
                            trafficAnnounceType = TAT_Approaching_Congestion;
                        }
                    }
                    else
                    {
                        trafficAnnounceType = TAT_Approaching_Incident;
                    }
                }
                switch (trafficColor)
                {
                    case 'R':
                        trafficAnnounceColor = TC_Red;
                        break;
                    case 'Y':
                        trafficAnnounceColor = TC_Yellow;
                        break;
                    case 'G':
                        trafficAnnounceColor = TC_Green;
                        break;
                    default:
                        trafficAnnounceColor = TC_Incident;
                        break;
                }
                m_analytics.AddAnnouncementEvent(sessionID, (*announcement)->GetText(), (float)m_tripRemainingDistance, (uint32)playTimes,
                                                 trafficAnnounceType, trafficAnnounceColor, &m_lastPosition);

            }

            // avoid memory leak
            if (m_notifier.IsNoneAnnounceListener() || (m_commandStr == "CO." && !m_preferences.IsNeedCOManeuver()))
            {
                (*announcement)->OnAnnouncementComplete();
            }
            else
            {
                m_currentAnnouncement = *announcement;
                m_notifier.Announce(announcement);
            }
        }
    }
}

bool NavigationImpl::UpdateNbRoutes(NB_RouteInformation* route, bool detour)
{
    NB_RouteInformation* activceNbRoute = NULL;

    if (detour)
    {
        ClearNbDetours();
    }
    else
    {
        ClearNbRoutes();
        ClearActiveRoute();
    }
    nb::Lock lock(m_memberLock);
    if (route)
    {
        uint32 count = 0;
        NB_RouteInformationGetRoutesCount(route, &count);
        if (detour && m_activeRoute)
        {
            activceNbRoute = GetNbRoute(*m_activeRoute);
            NB_RouteInformationUpdateDescriptionWithDetour(activceNbRoute, route);
            ClearActiveRoute();
            m_activeRoute = new RouteInformation(this, activceNbRoute, 0);
        }

        if (NE_OK == NB_RouteInformationGetRoutesCount(route, &count))
        {
            m_receivedRoutes.clear();
            for(uint32 i = 0; i < count; ++i)
            {
                NB_RouteInformation* singleRoute = NULL;
                if (NE_OK == NB_RouteInformationGetSingleRouteInformation(route,
                    i + 1,
                    &singleRoute))
                {
                    if (!detour)
                    {
                        m_nbRoutes.push_back(singleRoute);
                    }
                    else
                    {
                        m_nbDetours.push_back(singleRoute);
                    }
                    RouteInformation routeInfo(this, singleRoute, detour ? Detour::detourOffset + i : i);
                    std::vector<RouteSummaryInformation>& routeSummary = routeInfo.GetRouteSummary();
                    if(m_destinations.size() >= routeSummary.size())
                    {
                        for(int i = 0; i< routeSummary.size(); i++)
                        {
                            routeSummary[i].setOrigin(m_origin);
                            routeSummary[i].setDestination(m_destinations[i]);
                        }
                    }
                    routeInfo.m_isPlanRoute = m_planningTrip;
                    m_receivedRoutes.push_back(routeInfo);
                }
            }

            if (!m_preferences.GetMultipleRoutesEnabled() && !m_activeRoute && count == 1)
            {
                m_activeRoute = new RouteInformation(m_receivedRoutes[0]);
                if (m_preferences.GetMockGpsEnabled())
                {

                    if(m_mockProvider == NULL)
                    {
                    m_mockProvider = new NavMockGPSProvider(this, m_pal, m_nbNavigation);
                    m_mockProvider->SetSpeed(m_preferences.GetMockGpsSpeed());
                    m_mockProvider->Start();
                    }
                }
            }

            if (detour)
            {
                m_receivedRoutes.push_back(*m_activeRoute);
            }
            RouteReceivedData* data = new RouteReceivedData();
            data->routeRequestedReason = detour ? SessionListener::Detour : m_routeRequestedReason;
            std::swap(data->receivedRoutes, m_receivedRoutes);

            m_notifier.RouteReceived(data);

            NAV_DEBUG_LOG(m_logger, "%p: Invoke callback: %s, total routes: %lu\n",
                          this, __FUNCTION__, data->receivedRoutes.size());
        }
    }
    return true;
}

NB_Navigation* NavigationImpl::GetNbNavigation() const
{
    return m_nbNavigation;
}

NB_RouteInformation* NavigationImpl::GetNbNavRouteInfo() const
{
    NB_RouteInformation* route = NULL;
    NB_NavigationGetInformation(m_nbNavigation, &route, NULL, NULL, NULL);

    return route;
}

NB_TrafficInformation* NavigationImpl::GetNbNavTrafficInfo() const
{
    NB_TrafficInformation* traffic = NULL;
    NB_NavigationGetInformation(m_nbNavigation, NULL, &traffic, NULL, NULL);

    return traffic;
}

NB_Context* NavigationImpl::GetNbContext() const
{
    return m_context;
}

PAL_Instance* NavigationImpl::GetPal() const
{
    return m_pal;
}

NB_GuidanceInformation* NavigationImpl::GetNbGuidanceInfo() const
{
    return m_nbGuidanceInfo;
}

NB_GuidanceInformation* NavigationImpl::GetNbGuidanceInfoForUI() const
{
    return m_nbGuidanceInfoForUI;
}

void LocationToNbGpsLocation(const Location& location, NB_GpsLocation* nbGpsLocation)
{
    nsl_memset(nbGpsLocation, 0, sizeof(NB_GpsLocation));

    nbGpsLocation->status    = (location.Valid() != None) ? NE_OK : NE_INVAL;
    nbGpsLocation->gpsTime   = location.GpsTime();
    nbGpsLocation->valid     = location.Valid();
    nbGpsLocation->latitude  = location.Latitude();
    nbGpsLocation->longitude = location.Longitude();
    nbGpsLocation->heading   = location.Heading();
    nbGpsLocation->altitude  = location.Altitude();

    nbGpsLocation->horizontalVelocity = location.HorizontalVelocity();
    nbGpsLocation->verticalVelocity   = location.VerticalVelocity();

    nbGpsLocation->horizontalUncertaintyAngleOfAxis        = location.HorizontalUncertaintyAngleOfAxis();
    nbGpsLocation->horizontalUncertaintyAlongAxis          = location.IsGpsFix() ?
        location.HorizontalUncertaintyAlongAxis() : LOCATION_THRESHHOLD + 1;
    nbGpsLocation->horizontalUncertaintyAlongPerpendicular = location.HorizontalUncertaintyAlongPerpendicular();
    nbGpsLocation->verticalUncertainty                     = location.VerticalUncertainty();

    nbGpsLocation->utcOffset          = (int16)location.UtcOffset();
    nbGpsLocation->numberOfSatellites = (int16)location.NumberOfSatellites();
}

bool NavigationImpl::IsTtsEnabled() const
{
    return !m_preferences.GetDownloadableAudioEnabled();
}

bool NavigationImpl::IsNCEnabled() const
{
    return m_preferences.IsNeedNCManeuver();
}

NB_NavigateAnnouncementUnits NavigationImpl::GetMeasurementUnits() const
{
    if (m_preferences.GetMeasurement() == Preferences::Metric)
    {
        return NB_NAU_Kilometers;
    }
    else if(m_preferences.GetMeasurement() == Preferences::NonMetric)
    {
        return NB_NAU_Miles;
    }
    else
    {
        return NB_NAU_MilesYards;
    }
}

NavigateRouteError NavigationImpl::NbRouteErrorToRouteError(NB_NavigateRouteError nbRouteError)
{
    NavigateRouteError routeError = NRE_None;

    switch (nbRouteError)
    {
        case NB_NRE_None:
        {
            routeError = NRE_None;
            break;
        }
        case NB_NRE_TimedOut:
        {
            routeError = NRE_TimedOut;
            break;
        }
        case NB_NRE_BadDestination:
        {
            routeError = NRE_BadDestination;
            break;
        }
        case NB_NRE_BadOrigin:
        {
            routeError = NRE_BadOrigin;
            break;
        }
        case NB_NRE_CannotRoute:
        {
            routeError = NRE_CannotRoute;
            break;
        }
        case NB_NRE_EmptyRoute:
        {
            routeError = NRE_EmptyRoute;
            break;
        }
        case NB_NRE_NetError:
        {
            routeError = NRE_NetError;
            break;
        }
        case NB_NRE_NoMatch:
        {
            routeError = NRE_NoMatch;
            break;
        }
        case NB_NRE_ServerError:
        {
            routeError = NRE_ServerError;
            break;
        }
        case NB_NRE_NoDetour:
        {
            routeError = NRE_NoDetour;
            break;
        }
        case NB_NRE_PedRouteTooLong:
        {
            routeError = NRE_PedRouteTooLong;
            break;
        }
        case NB_NRE_OriginCountryUnsupported:
        {
            routeError = NRE_OriginCountryUnsuppoted;
            break;
        }
        case NB_NRE_DestinationCountryUnsupported:
        {
            routeError = NRE_DestinationCountryUnsupported;
            break;
        }
        case NB_NRE_BicycleRouteTooLong:
        {
            routeError = NRE_BicycleRouteTooLong;
            break;
        }
        case NB_NRE_RouteTooLong:
        {
            routeError = NRE_RouteTooLong;
            break;
        }
        default:
        {
            routeError = NRE_None;
            break;
        }
    }
    return routeError;
}

NavigateRouteError NavigationImpl::NbErrorToRouteError(NB_Error nbError)
{
    NavigateRouteError routeError = NRE_None;
    switch (nbError)
    {
        case NESERVERX_UNAUTHORIZED_ACCESS:
            routeError = NRE_Unauthorized;
            break;
        case NESERVERX_INVALID_COORD_ERROR:
            routeError = NRE_CannotRoute;
            break;
        default:
            break;
    }
    return routeError;
}

void NavigationImpl::ConvertGuidanceMessageToString(NB_GuidanceMessage* pMessage, string& plainText) const
{
    if (!m_nbNavigation || !pMessage)
    {
        return;
    }
    int codeCount = NB_GuidanceMessageGetCodeCount(pMessage);
    const char* code = NULL;
    char textBuffer[bufferSize] = {0};
    uint32 textBufferSize = 0;
    byte phoneticBuffer[bufferSize] = {0};
    uint32 phoneticBufferSize = 0;
    std::string preCode;
    for (int i = 0; i < codeCount; i++)
    {
        nsl_memset(textBuffer, 0, sizeof(textBuffer));
        nsl_memset(phoneticBuffer, 0, sizeof(phoneticBuffer));
        textBufferSize = bufferSize;
        phoneticBufferSize = bufferSize;
        code = NB_GuidanceMessageGetCode(pMessage, i);
        if (nsl_strcmp(code, PAUSE_VOICE_CODE) == 0)
        {
            continue;
        }
        nb_boolean isBase;
        NB_NavigationGetDescriptionInformation(m_nbNavigation, code, textBuffer, &textBufferSize, phoneticBuffer,
                                          &phoneticBufferSize,NULL,NULL,NULL,NULL, NULL, &isBase);
        if (textBufferSize)
        {
            std::string tmp = textBuffer;
            if(!isBase)
            {
                tmp = NavUtils::ConvertFormatStringToPlainString(tmp);
            }
            if (i > 0 && preCode != SPAN_ROAD_NAME && nsl_strcmp(code,SPAN_CLOSE))
            {
                plainText += " ";
            }
            if(i == 0 && !tmp.empty())
            {
                tmp[0] = toupper(tmp[0]);
            }
            plainText.append(tmp);
            preCode = code;
        }
    }
}

void NavigationImpl::AddAnalyticsNavigationState(const char* state)
{
    uint32 sessionID = 0;
    NB_NavigationPublicState publicState;
    nsl_memset(&publicState, 0, sizeof(publicState));
    double traveledDistance = 0;

    NB_NavigationGetSessionId(m_nbNavigation, &sessionID);
    NB_NavigationGetPublicState(m_nbNavigation, &publicState);
    NB_NavigationGetTraveledDistance(m_nbNavigation, &traveledDistance);

    m_analytics.AddRouteStateEvent(sessionID, state, publicState.tripRemainTime,
                                   publicState.tripRemainDistance,
                                   traveledDistance, &m_lastPosition);
}

void NavigationImpl::AddAnalyticsDetourRequest()
{
    uint32 sessionId = 0;
    NB_NavigationGetSessionId(m_nbNavigation, &sessionId);
    m_analytics.AddRouteRequestEvent("detour", sessionId);
}

void NavigationImpl::AddAnalyticsDetourReply()
{
    uint32 sessionID = 0;
    NB_NavigationPublicState publicState;
    NB_NavigationGetSessionId(m_nbNavigation, &sessionID);
    NB_NavigationGetPublicState(m_nbNavigation, &publicState);
    m_analytics.AddRouteReplyEvent(sessionID, publicState.tripRemainTime, publicState.tripRemainDistance);
}

void NavigationImpl::StartStaticRouteRequest(const Place& originPlace)
{
    if (!m_nbNavigation)
    {
        m_origin = originPlace;
        MapLocation originLocation = originPlace.GetLocation();
        m_lastPosition.latitude = originLocation.center.latitude;
        m_lastPosition.longitude = originLocation.center.longitude;
        m_lastPosition.horizontalUncertaintyAlongAxis = originLocation.center.accuracy;
        m_lastPosition.valid = NGV_Latitude & NGV_Longitude & NGV_HorizontalUncertainty;

        NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this,
            NULL, &NavigationImpl::StartStaticRouteRequestScheduled, m_isValid);
        task->Execute();
    }
}

void NavigationImpl::StartRouteSummaryRequest(const Place& originPlace)
{
    if (!m_nbNavigation)
    {
        m_origin = originPlace;
        NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this,
            NULL, &NavigationImpl::StartRouteSummaryRequestScheduled, m_isValid);
        task->Execute();
    }
}

void NavigationImpl::StartStaticRouteRequestScheduled(int* data)
{
    NB_NavigationCallbacks callbacks = {0};
    callbacks.statusChangeCallback                = NbSessionStatusChangeCb;
    callbacks.routeCompleteDownloadCallback       = NbCompleteRouteDownloadCb;
    callbacks.routeDownloadCallback               = NbRouteDownloadCb;
    callbacks.routeRequestedCallback              = NbRouteRequestedCb;
    callbacks.guidanceMessageCallback             = NbSessionGuidanceCb;
    callbacks.userData = this;

    NB_GpsHistory* sharedHistory = NB_ContextGetGpsHistory(m_context);
    if (sharedHistory)
    {
        NB_GpsHistoryClear(sharedHistory);
        NB_GpsHistoryAdd(sharedHistory, &m_lastPosition);
    }

    if (!m_nbNavigation && CreateStaticNbNavSession(m_origin, &callbacks)!= NE_OK)
    {
        SetRouteError(NRE_Internal);
    }
}

void NavigationImpl::StartRouteSummaryRequestScheduled(int* data)
{
    NB_NavigationCallbacks callbacks = {0};
    callbacks.statusChangeCallback                = NbSessionStatusChangeCb;
    callbacks.routeCompleteDownloadCallback       = NbCompleteRouteDownloadCb;
    callbacks.routeDownloadCallback               = NbRouteDownloadCb;
    callbacks.routeRequestedCallback              = NbRouteRequestedCb;
    callbacks.userData = this;

    if (!m_nbNavigation && CreateSummaryNbNavSession(m_origin, &callbacks) != NE_OK)
    {
        SetRouteError(NRE_Internal);
    }
}

void NavigationImpl::SetWorkFolder(const string& path)
{
    m_workPath = path;
    if(!IsSupportedLocal(m_locale))
    {
        m_locale = DEFAULT_LOCAL;
    }
}

string NavigationImpl::GetBaseVoicesPath() const
{
    string workPath(GetPrivateWorkFolder());
    string path (workPath + "NK_" + m_locale + "_audio" + PATH_DELIMITER + m_voicesStyle + PATH_DELIMITER);
    return path;
}

void NavigationImpl::Recalculate()
{
    if (m_nbNavigation && !m_isRouteArrived)
    {
        NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(
            m_pal, this, NULL, &NavigationImpl::RecalculateScheduled, m_isValid);
        task->Execute();
    }
}
    
void NavigationImpl::Recalculate(const vector<WayPoint>* wayPoint)
{
    if(wayPoint)
    {
        m_wayPoints = *wayPoint;
    }
    else
    {
        m_wayPoints.clear();
    }
    if (m_nbNavigation && !m_isRouteArrived)
    {
        NavTask<NavigationImpl, int>* task = new NavTask<NavigationImpl, int>(m_pal, this, NULL, &NavigationImpl::RecalculateWithWayPointScheduled, m_isValid);
        task->Execute();
    }
}

void NavigationImpl::Recalculate(const RouteOptions& newRouteOptions, const Preferences& newPreferences,
                                 bool wantAlternateRoute)
{
    if (m_nbNavigation && !m_isRouteArrived)
    {
        RouteOptionsData* options = new RouteOptionsData();
        options->options = newRouteOptions;
        options->wantAlternateRoute = wantAlternateRoute;
        options->preference = newPreferences;
        NavTask<NavigationImpl, RouteOptionsData>* task = new NavTask<NavigationImpl, RouteOptionsData>(m_pal, this,
            options, &NavigationImpl::RecalculateWithRouteOptionScheduled, m_isValid);
        task->Execute();
    }
}

void NavigationImpl::RecalculateScheduled(int* data)
{
    if (m_nbNavigation)
    {
        NB_NavigationRecalculate(m_nbNavigation);
    }
}

void NavigationImpl::RecalculateWithWayPointScheduled(int* data)
{
    if (m_nbNavigation && m_wayPoints.size()>0)
    {
        NB_WayPoint* pWayPoint = new NB_WayPoint[m_wayPoints.size()];
        for(int i = 0; i < m_wayPoints.size(); i++)
        {
            NB_Place place;
            PlaceToNbPlace(m_wayPoints[i].location,&place);
            (pWayPoint+i)->location = place.location;
            (pWayPoint+i)->isStopPoint = m_wayPoints[i].isStopPoint;
        }
        NB_NavigationRecalculateWithWayPoint(m_nbNavigation, pWayPoint, m_wayPoints.size());
        delete pWayPoint;
        
        m_currentMask = NETP_None;
        m_currentTotalMask = NETP_None;
        m_notifier.DisableNavEvents();
    }
}

void NavigationImpl::RecalculateWithRouteOptionScheduled(RouteOptionsData* options)
{
    if (options && m_nbNavigation)
    {
        string oldPronunciationStyle = m_routeOptions.GetPronunciationStyle();
        string newPronunciationStyle = options->options.GetPronunciationStyle();

        m_routeOptions = options->options;
        NB_RouteOptions nbOptions;
        SetupRouteOptions(&nbOptions, &m_routeOptions, &m_preferences, m_voicesStyle.c_str());

        m_preferences = options->preference;
        NB_RouteConfiguration nbConfig;
        SetupRouteConfiguration(&nbConfig, &m_preferences, false);
        SetupForPedestrianAndBike(&nbOptions, &nbConfig, &m_routeOptions);

        nbConfig.startToNavigate = !m_isNewRouteForPlanTripReceived;
        nbConfig.disableNavigation = m_planningTrip;
        nbConfig.commandSetVersion = NB_GuidanceInformationGetCommandVersion(m_nbGuidanceInfo);

        if (!m_nbEnhancedManager)
        {
            m_nbEnhancedManager = CreateEnhancedContentManager(m_context, &nbConfig, PAL_FileGetCachePath());

            // Start metadata synchronization.
            NB_EnhancedContentManagerStartMetadataSynchronization(m_nbEnhancedManager);
        }

        //recreate the guidance information when pronunciation style changed
        if (oldPronunciationStyle.compare(newPronunciationStyle) != 0)
        {
            if (m_nbGuidanceInfo)
            {
                NB_GuidanceInformationDestroy(m_nbGuidanceInfo);
                m_nbGuidanceInfo = NULL;
            }

            m_voicesStyle = newPronunciationStyle;
            string baseVoicePath = GetBaseVoicesPath();
            m_nbGuidanceInfo = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                         baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                         m_locale.c_str(), IsTtsEnabled());

            if (m_nbGuidanceInfoForUI)
            {
                NB_GuidanceInformationDestroy(m_nbGuidanceInfoForUI);
                m_nbGuidanceInfoForUI = NULL;
            }
            m_nbGuidanceInfoForUI = CreateGuidanceInformation(m_context, m_pal, GetPrivateWorkFolder(),
                                                              baseVoicePath.c_str(), m_voicesStyle.c_str(),
                                                              m_locale.c_str(), IsTtsEnabled());
            if (m_nbGuidanceInfo)
            {
                NB_NavigationSetGuidanceInformation(m_nbNavigation, m_nbGuidanceInfo);
            }
            else
            {
                SetRouteError(NRE_Internal);
                return;
            }
        }

        NB_NavigationRecalculateWithOptionsAndConfig(m_nbNavigation, &nbOptions, &nbConfig, options->wantAlternateRoute);

        m_currentMask = NETP_None;
        m_currentTotalMask = NETP_None;
        m_notifier.DisableNavEvents();
    }
}

NB_Error NavigationImpl::UpdateManeuverPositionInformation(uint32 maneuver)
{
    nb::Lock lock(m_methodLock);
    char currentPrimaryRoadName[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char currentSecondaryRoadName[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char nextPrimaryRoadName[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char nextSecondaryRoadName[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    char commandBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    NB_CommandFont nextTurnCharacter = {0};
    NB_CommandFont stackTurnCharacter = {0};
    NB_LatitudeLongitude turnPoint = {0};
    char exitNumber[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};

    NB_Error error = NB_RouteInformationGetTurnInformation2(GetNbNavRouteInfo(), maneuver, &turnPoint,
                                                            nextPrimaryRoadName, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                            nextSecondaryRoadName, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                            currentPrimaryRoadName, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                            currentSecondaryRoadName, NB_NAVIGATION_STREET_NAME_MAX_LEN,
                                                            NULL, NULL, NULL,
                                                            commandBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN, TRUE);

    // Turn Character
    if (error == NE_OK)
    {
        error = NB_RouteInformationGetTurnCommandCharacter(GetNbNavRouteInfo(), GetNbGuidanceInfo(),
                                                           maneuver, NB_NMP_Turn, GetMeasurementUnits(), &nextTurnCharacter);
    }

    NB_NavigationPublicState publicState;
    nsl_memset(&publicState, 0, sizeof(publicState));
    (void)NB_NavigationGetPublicState(GetNbNavigation(), &publicState);

    m_closestManeuver = publicState.currentRoutePosition.closestManeuver;

    // Stack Information
    if (error == NE_OK && publicState.stackNman != NAV_MANEUVER_NONE)
    {
        error = NB_RouteInformationGetTurnCommandCharacter(GetNbNavRouteInfo(), GetNbGuidanceInfo(), publicState.stackNman,
                                                           NB_NMP_Turn, GetMeasurementUnits(), &stackTurnCharacter);
    }

    if (error == NE_OK)
    {
        (void)NB_RouteInformationGetExitNumber(GetNbNavRouteInfo(), publicState.currentManeuver,
                                               NULL, exitNumber, NB_NAVIGATION_STREET_NAME_MAX_LEN);
    }

    if (error == NE_OK)
    {
        m_currentPrimaryRoadNameStr.assign(currentPrimaryRoadName);
        m_currentSecondaryRoadNameStr.assign(currentSecondaryRoadName);
        m_nextPrimaryRoadNameStr.clear();
        if (publicState.status == NB_NS_NavigatingStartup)
        {
            m_nextPrimaryRoadNameStr.assign(publicState.destinationStreet);
        }
        else if (nextPrimaryRoadName)
        {
            m_nextPrimaryRoadNameStr.assign(nextPrimaryRoadName);
        }
        m_nextSecondaryRoadNameStr.clear();
        if (publicState.status == NB_NS_NavigatingStartup)
        {
            m_nextSecondaryRoadNameStr.assign(publicState.destinationSecondary);
        }
        else if (nextSecondaryRoadName)
        {
            m_nextSecondaryRoadNameStr.assign(nextSecondaryRoadName);
        }
        m_commandStr.assign(commandBuffer);
        m_imageIdStr.clear();
        if (nextTurnCharacter.character)
        {
            m_imageIdStr.assign((char*)(&nextTurnCharacter.character));
        }
        m_stackTurnImageStr.clear();
        if (stackTurnCharacter.character)
        {
            m_stackTurnImageStr.assign((char*)(&stackTurnCharacter.character));
        }
        m_exitNumberStr.assign(exitNumber);
        m_maneuverTurnPoint.latitude = turnPoint.latitude;
        m_maneuverTurnPoint.longitude = turnPoint.longitude;
    }
    return error;
}

NavigateRouteError NavigationImpl::GetRouteError() const
{
    return m_routeError;
}

const NB_GpsLocation* NavigationImpl::GetLastLocation() const
{
    return &m_lastPosition;
}

unsigned int NavigationImpl::GetSignificantManeuverIndex(unsigned int maneuverIndex) const
{
    uint32 maneuverCount = NB_RouteInformationGetManeuverCount(GetNbNavRouteInfo());
    for (uint32 i = maneuverIndex; i < maneuverCount; i++)
    {
        if (NB_RouteInformationIsManeuverSignificantForTurnLists(GetNbNavRouteInfo(), i, i == m_closestManeuver))
        {
            return i;
        }
    }
    return maneuverIndex;
}

unsigned int NavigationImpl::GetClosestManeuver() const
{
    return m_closestManeuver;
}

shared_ptr<bool> NavigationImpl::GetValidPointer() const
{
    return m_isValid;
}

void NavigationImpl::SetRouteError(NavigateRouteError error)
{
    m_routeError = error;
    NAV_DEBUG_LOG(m_logger, "RouteError: %d", (int)error);
    NavigateRouteError* data = new NavigateRouteError(m_routeError);
    m_notifier.RouteError(data);
}

void NavigationImpl::UpdateManeuverList()
{
    if (m_currentManeuverList)
    {
        delete m_currentManeuverList;
        m_currentManeuverList = NULL;
    }
    m_currentManeuverList = new ManeuverList(this, m_activeRoute->m_index, true);
    m_notifier.UpdateManeuverList(new ManeuverList(*m_currentManeuverList));
}

void NavigationImpl::OnMockGPSUpdate(NB_GpsLocation* location)
{
    UpdatePositionScheduled(location);
}

bool NavigationImpl::IsExtrapolateEnabled() const
{
    return m_extrapolatePointsInterval > 0;
}

bool NavigationImpl::IsSuitablePosition() const
{
    bool result = true;
    double distance = 0;

    if (m_lastExtrapolatePosition.latitude == INVALID_LATLON || m_onOffRoute == rtOffRoute)
    {
        return result;
    }

    distance = NB_SpatialGetLineOfSightDistance(m_lastExtrapolatePosition.latitude,
                                                m_lastExtrapolatePosition.longitude,
                                                m_extrapolatePosition.at(m_extrapolateCurrentPosition).latitude,
                                                m_extrapolatePosition.at(m_extrapolateCurrentPosition).longitude,
                                                NULL);

    if (distance > 50)
    {
        return result;
    }

    if (m_extrapolatePosition.at(m_extrapolateCurrentPosition).maneuver < m_lastExtrapolatePosition.maneuver)
    {
        result = false;
    }
    else if (m_extrapolatePosition.at(m_extrapolateCurrentPosition).maneuver == m_lastExtrapolatePosition.maneuver)
    {
        if (m_extrapolatePosition.at(m_extrapolateCurrentPosition).segment < m_lastExtrapolatePosition.segment)
        {
            result = false;
        }
    }
    return result;
}

const std::string NavigationImpl::GetLocal() const
{
    return m_locale;
}
    
const std::set<std::string>& NavigationImpl::GetSupportLocal()
{
    const char* localPrefix = "NK_";
    const char* common = "NK_common";
    string local;
    if(m_supportLocal.size() == 0)
    {
        string workPath = GetPrivateWorkFolder();
        if(workPath[workPath.size()-1] == PATH_DELIMITER)
        {
            workPath = workPath.substr(0, workPath.size()-1);
        }
        PAL_FileEnum* fileEnum = NULL;
        PAL_FileEnumInfo info = {0};
        if (PAL_Ok == PAL_FileEnumerateCreate(m_pal, workPath.c_str(), FALSE, &fileEnum))
        {
            while(PAL_FileEnumerateNext(fileEnum, &info) != PAL_ErrNoData)
            {
                if (nsl_strncmp(info.filename, localPrefix, nsl_strlen(localPrefix)) == 0)
                {
                    if(nsl_strcmp(info.filename, common) != 0)
                    {
                        int length = nsl_strlen(localPrefix);
                        local = info.filename;
                        local = local.substr(length, local.size()-length);
                        m_supportLocal.insert(local);
                    }
                }
            }
            PAL_FileEnumerateDestroy(fileEnum);
        }
    }
    return m_supportLocal;
}
    
bool NavigationImpl::IsSupportedLocal(std::string local)
{
    const std::set<std::string>& supportLocal = GetSupportLocal();
    if(supportLocal.find(local) != supportLocal.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}
    

}
