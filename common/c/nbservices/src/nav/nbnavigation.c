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
    @file     nbnavigate.c
    @defgroup nbnavigate nbnavigate
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstateprivate.h"
#include "nbgpshistory.h"
#include "nbrouteinformation.h"
#include "nbrouteinformationprivate.h"
#include "palstdlib.h"
#include "palclock.h"
#include "navpublictypes.h"
#include "navtypes.h"
#include "nav_avg_speed.h"
#include "navutil.h"
#include "nbcameraprocessor.h"
#include "nbcameraprocessorprivate.h"
#include "nbcamerainformationprivate.h"
#include "nbpositionprocessor.h"
#include "nbtrafficprocessor.h"
#include "nbtrafficprocessorprivate.h"
#include "nbtrafficparametersprivate.h"
#include "nbguidanceprocessor.h"
#include "nbguidanceprocessorprivate.h"
#include "nbguidanceinformation.h"
#include "nbguidanceinformationprivate.h"
#include "nbenhancedcontentprocessorprivate.h"
#include "nbenhancedvectormapprocessorprivate.h"
#include "nav_avg_speed.h"
#include "gpsutil.h"
#include "nbqalog.h"
#include "nbnavqalog.h"
#include "nbspatial.h"
#include "cslnetwork.h"
#include "paltimer.h"
#include "palmath.h"
#include "nbanalytics.h"
#include "nbsinglesearchparameters.h"
#include "nbsinglesearchhandler.h"
#include "nbsinglesearchinformation.h"
#include "nbsearchparameters.h"
#include "cslqarecorddefinitions.h"
#include "nbpronunparameters.h"
#include "nbpronunhandler.h"
#include "nbspeedlimitinformationprivate.h"
#include "nbnaturallaneguidanceprocessor.h"
#include "cslutil.h"
#include "data_via_point.h"
#include "jsonstylelogger.h"
#include "palfile.h"

/*! @{ */

#define DEFAULT_POI_SEARCH_WIDTH_METERS         400
#define DEFAULT_POI_SEARCH_LENGTH_METERS        5000
#define DEFAULT_POI_PRE_SEARCH_METERS           500
#define DEFAULT_POI_SCHEDULE_TIMEOUT_MSECS      5000
#define PRONUN_MAXIMUM_RETRIES                  3
#define MIN_SPEED                               4.47
#define GPS_UPDATING_TIMEOUT  10
#define OUT_TUNNEL_GPS_SIFTER_ERROR_RADIUS 20

#define LOG_NAV_ERROR(navigation,status) NB_QaLogNavigationError(navigation->context, navigation->navState->routeError, navigation->navState->error, status, __FUNCTION__, __FILE__, __LINE__);

static NB_Error SetStatus(NB_Navigation* navigation, NB_NavigateStatus status);
static nb_boolean IsNavigatingStatus(NB_NavigateStatus status);

static NB_Error   SetPoiSearchTimerTimeOutStatus(NB_Navigation* navigation, nb_boolean status);
static NB_Error   GetPoiSearchTimerTimeOutStatus(const NB_Navigation* navigation, nb_boolean* poiTimerStatus);

static NB_Error PlayGuidanceMessage(NB_Navigation* navigation, NB_GuidanceMessage* message, nb_boolean invokeClientCallback);

static NB_Navigation* AllocateNavigation(NB_Context* context);
static NB_NavigationState* AllocateNavigationState(NB_Navigation* navigation, NB_RoutePrivateConfiguration* routePrivateConfig, NB_NavigateTransportMode transportMode);
static NB_NavigationPublicState* AllocateNavigationPublicState(NB_Context* context);
static NB_Error NB_NavigationStateNewRoute(NB_NavigationState* navigationState);
static NB_Error NB_NavigationStateDestroy(NB_NavigationState* navigationState);
static NB_Error NB_NavigationPublicStateDestroy(NB_NavigationPublicState* publicState);
static void ResetNavigationPublicState(NB_NavigationPublicState* publicState);

static NB_Error NB_NavigationCheckForRecalc(NB_Navigation* navigation, const NB_GpsLocation* location, nb_boolean* recalcRequired, enum nav_update_reason* recalcReason);
static NB_Error NB_NavigationDoRecalc(NB_Navigation* navigation, NB_GpsHistory* history, NB_RouteOptions* routeOptions, NB_RouteConfiguration* routeConfig, nb_boolean wantAlternateRoute, NB_WayPoint* pWaypoint, uint32 wayPointSize);
static void NB_NavigationNetworkCallback(NB_NetworkNotifyEvent event, void* data, void* userData);

static void NB_TrafficNotifyTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static NB_Error NB_NavigationDoTrafficNotify(NB_Navigation* navigation, NB_TrafficParameters* tp);
static NB_Error NB_NavigationCancelTrafficNotify(NB_Navigation* navigation);
static void NB_ScheduleTrafficNotify(NB_Navigation* navigation, uint32 millisecs, nb_boolean initial);
static NB_Error NB_NavigationTrafficDownloadSucceeded(NB_Navigation* navigation, NB_TrafficHandler* handler);
static void NB_NavigationTrafficDownloadCanceled(NB_Navigation* navigation);
static void NB_NavigationTrafficDownloadFailed(NB_Navigation* navigation);
static void NB_NavigationTrafficHandlerCallback(void* handler,NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);

static void NB_NavigationRouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);

static NB_Error NB_NavigationPlayMessage(NB_Navigation* navigation);

static void NB_NavigationCameraHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void NB_NavigationProcessCameras(NB_Navigation* navigation, NB_SearchHandler* handler);
static NB_Error NB_NavigationSearchCameras(NB_Navigation* navigation, NB_RouteInformation* route);

static void NB_NavigationPointsOfInterestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static void NB_NavigationProcessPointsOfInterest(NB_Navigation* navigation, NB_SingleSearchHandler* pointsOfInterestHandler);
static NB_Error NB_NavigationSearchPointsOfInterest(NB_Navigation* navigation, NB_RouteInformation* route);
static nb_boolean IsPointsOfInterestPreSearchPending(NB_Navigation* navigation);

static void NB_ScheduleCompleteRouteRequest(NB_Navigation* navigation, uint32 millisecs);
static void NB_CompleteRouteRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static NB_Error NB_NavigationDoCompleteRouteRequest(NB_Navigation* navigation);

static void NB_SchedulePronunDownloadRequest(NB_Navigation* navigation, uint32 millisecs);
static void NB_PronunRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void NB_NavigationPronunHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static NB_Error NB_NavigationDoPronunRequest(NB_Navigation* navigation);
static void NB_NavigationUpdateVoices(NB_Navigation* navigation, NB_PronunHandler* pronunHandler);

static void SchedulePoiSearchRequest(NB_Navigation* navigation, uint32 delay);
static void PoiSearchRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

static void ScheduleCameraDownloadRequest(NB_Navigation* navigation, uint32 delay);
static void CameraRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void CameraRequestFailed(NB_Navigation* navigation);

static void CancelPendingRequests(NB_Navigation* navigation);
static NB_Error PlayRouteDownloadMessage(NB_Navigation* navigation, NB_NavigateRouteCalculationType routeCalculationType);
static void CalculateStartupRegion(NB_Navigation* navigation);
static boolean InStartupRegion(NB_Navigation* navigation, const NB_GpsLocation* location);
static void CalculateArrivingRegion(NB_Navigation* navigation, double latitude, double longitude);
static boolean InArrivingRegion(NB_Navigation* navigation, const NB_GpsLocation* location);
static void RecalcTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static NB_Error RecalcCheckLimit(NB_Navigation* navigation, nb_boolean immediate);
static void NavigationSpeedLimitCallbackFunctionEx(NB_SpeedLimitStateData* speedlimitData, const char * sign_id, void* userData);
static void NB_RouteUpdatedMessagePlayedCallback(NB_GuidanceMessage* message, void* userData);
static void NB_GuidanceMessageQaLogPlayedCallback(NB_GuidanceMessage* message, void* userData);
// Play voice announcement when we switch from MDO(map data off) to TBT(turn-by-turn) maneuver
static NB_Error PlaySwitchToTbtMessage(NB_Navigation* navigation);
static NB_Error NB_NavigationTheFirstFixProcess(NB_Navigation* navigation);
static void NavigationSpecialRegionCallbackFunction(NB_SpecialRegionStateData* specialRegionStateData, void* userData);

static void LogNavigationConfigurationAsJsonFormat(PAL_Instance* pal, NB_NavigationConfiguration* configuration);

NB_DEF NB_Error
NB_NavigationCreate(NB_Context* context, NB_NavigationConfiguration* configuration, NB_NavigationCallbacks* callbacks, NB_VectorMapPrefetchConfiguration* vectorPrefetchConfig, NB_Navigation** navigation)
{
    NB_Navigation* pThis = 0;

    LogNavigationConfigurationAsJsonFormat(NB_ContextGetPal(context), configuration);
    if (!context)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    if (!callbacks)
    {
        return NE_INVAL;
    }

    pThis = AllocateNavigation(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }
    pThis->configuration = *configuration;

    pThis->navPublicState = AllocateNavigationPublicState(context);
    if (!pThis->navPublicState)
    {
        NB_NavigationDestroy(pThis);
        return NE_NOMEM;
    }
    nsl_memset(pThis->navPublicState, 0, sizeof(*pThis->navPublicState));

    pThis->guidanceInformation = NULL;

    pThis->callbacks = *callbacks;

    pThis->status = NB_NS_Created;

    pThis->navMode = NB_NM_Automatic;

    pThis->navState = NULL;

    pThis->cameraInformation = NULL;
    pThis->trafficInformation = NULL;
    pThis->poiInformation = NULL;

    nsl_memset(&pThis->poiConfiguration, 0, sizeof(pThis->poiConfiguration));

    pThis->cameraState = NULL;
    pThis->trafficState = NULL;
    pThis->guidanceState = NULL;
    pThis->naturalLaneGuidanceInformation = NULL;
    pThis->naturalLaneGuidanceState = NULL;

    pThis->routeHandler = NULL;
    pThis->routeHandlerCallback.callback = NB_NavigationRouteHandlerCallback;
    pThis->routeHandlerCallback.callbackData = pThis;

    pThis->trafficHandler = NULL;
    pThis->trafficHandlerCallback.callback = NB_NavigationTrafficHandlerCallback;
    pThis->trafficHandlerCallback.callbackData = pThis;

    pThis->cameraHandler = NULL;
    pThis->cameraHandlerCallback.callback = NB_NavigationCameraHandlerCallback;
    pThis->cameraHandlerCallback.callbackData = pThis;

    pThis->poiHandler = NULL;
    pThis->poiHandlerCallback.callback = NB_NavigationPointsOfInterestHandlerCallback;
    pThis->poiHandlerCallback.callbackData = pThis;

    pThis->pronunHandler = NULL;
    pThis->pronunHandlerCallback.callback = NB_NavigationPronunHandlerCallback;
    pThis->pronunHandlerCallback.callbackData = pThis;
    pThis->pronunDownloadRetries = 0;

    pThis->initialNotifyComplete = FALSE;
    pThis->trafficPollingIntervalMinutes = 0;

    pThis->announceType = NB_NAT_Street;
    pThis->announceUnits = NB_NAU_Miles;

    pThis->startNotification = FALSE;

    pThis->navSessionId = (uint32)PAL_ClockGetGPSTime(); /* for Device Analytics */

    if (( NB_RouteHandlerCreate(context, &pThis->routeHandlerCallback, &pThis->routeHandler) != NE_OK ) ||
        ( NB_TrafficHandlerCreate(context, &pThis->trafficHandlerCallback, &pThis->trafficHandler) != NE_OK ) ||
        ( NB_SearchHandlerCreate(context, &pThis->cameraHandlerCallback, &pThis->cameraHandler) != NE_OK ) ||
        ( NB_SingleSearchHandlerCreate(context, &pThis->poiHandlerCallback, &pThis->poiHandler) != NE_OK ) ||
        ( NB_PronunHandlerCreate(context, &pThis->pronunHandlerCallback, &pThis->pronunHandler) != NE_OK ) ||
        ( NB_CameraProcessorStateCreate(context, &pThis->cameraState) != NE_OK ) ||
        ( NB_TrafficProcessorStateCreate(context, &pThis->trafficState) != NE_OK ) ||
        ( NB_GuidanceProcessorStateCreate(context, &pThis->guidanceState) != NE_OK ) ||
        ( NB_NLGInformationCreate(context, &pThis->naturalLaneGuidanceInformation) != NE_OK) ||
        ( NB_NLGStateCreate(context, &pThis->naturalLaneGuidanceState) != NE_OK)||
        ( NB_SpeedLimitStateCreate(context, &pThis->speedlimitState) != NE_OK ) ||
        ( NB_GpsFilterCreate(context, pThis->configuration.filteredGpsMinimumSpeed, pThis->configuration.filteredGpsConstant, &pThis->gpsFilter) != NE_OK) ||
        ( NB_RecalcLimitCreate(context, pThis->configuration.excessiveRecalcMaximumCount, pThis->configuration.excessiveRecalcTimePeriod, &pThis->recalcLimit) != NE_OK) ||
        ( NB_SpecialRegionStateCreate(context, &pThis->specialRegionState) != NE_OK) )
    {
        NB_NavigationDestroy(pThis);
        return NE_NOMEM;
    }

    // create enhanced content processor only if an enhanced content manager has been associated with context
    if (NB_ContextGetEnhancedContentManager(context) != NULL)
    {
        if (NB_EnhancedContentProcessorStateCreate(context, &pThis->enhancedContentState) != NE_OK)
        {
            NB_NavigationDestroy(pThis);
            return NE_NOMEM;
        }
    }

    pThis->navigationNetworkCallback.callback = NB_NavigationNetworkCallback;
    pThis->navigationNetworkCallback.callbackData = pThis;

    CSL_NetworkAddEventNotifyCallback( NB_ContextGetNetwork(pThis->context), &pThis->navigationNetworkCallback );

    NB_QaLogNavigationSessionStart(pThis->context, pThis->navSessionId);
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_SESSION_START id = %d", pThis->navSessionId);

    NB_ContextSetSnippetLengthCallback(pThis->context, pThis->callbacks.getAnnounceSnippetLengthCallback, pThis->callbacks.userData);

    pThis->fastNavStartupMode = FALSE;

    pThis->traveledDistance = 0;

    pThis->lastPosition.latitude = INVALID_LATLON;
    pThis->lastPosition.longitude = INVALID_LATLON;

    set_namechanged_maneuver_enabled(configuration->nameChangedManeuverEnabled);

    set_m1_maneuver_enabled(configuration->m1ManeuverEnabled);

    *navigation = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationDestroy(NB_Navigation* navigation)
{
    CSL_Cache* voiceCache = NULL;
    int32 numberOfSavedEntries=0;
    NB_Error result = NE_OK;

    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    NB_QaLogNavigationSessionEnd(navigation->context, navigation->navSessionId);

    CSL_NetworkRemoveEventNotifyCallback(NB_ContextGetNetwork(navigation->context), &navigation->navigationNetworkCallback);

    CancelPendingRequests(navigation);

    //persist the voice cache
    voiceCache = NB_ContextGetVoiceCache(navigation->context);

    if (voiceCache)
    {
        //call save accessed with -1, to save all entries from memory to persistent cache
        result = CSL_CacheSaveAccessed(voiceCache, -1, &numberOfSavedEntries, FALSE);

        if (result == NE_OK)
        {
            //call save index to save the index file to be loaded the next time
            CSL_CacheSaveIndex(voiceCache);
        }
    }

#ifdef NB_ECM_ENABLED
    if (navigation->enhancedVectorMapState)
    {
        NB_EnhancedVectorMapStateDestroy(navigation->enhancedVectorMapState);
    }
#endif

    if (navigation->enhancedContentState)
    {
        NB_EnhancedContentProcessorStateDestroy(navigation->enhancedContentState);
    }

    if (navigation->cameraState)
    {
        NB_CameraProcessorStateDestroy(navigation->cameraState);
    }

    if (navigation->trafficInformation)
    {
        NB_TrafficInformationDestroy(navigation->trafficInformation);
    }

    if (navigation->trafficState)
    {
        NB_TrafficProcessorStateDestroy(navigation->trafficState);
    }

    if (navigation->cameraInformation)
    {
        NB_CameraInformationDestroy(navigation->cameraInformation);
    }

    if (navigation->poiInformation)
    {
        NB_PointsOfInterestInformationDestroy(navigation->poiInformation);
    }

    if (navigation->guidanceState)
    {
        NB_GuidanceProcessorStateDestroy(navigation->guidanceState);
    }
    if (navigation->naturalLaneGuidanceInformation)
    {
        NB_NLGInformationDestroy(navigation->naturalLaneGuidanceInformation);
        navigation->naturalLaneGuidanceInformation = NULL;
    }
    if (navigation->naturalLaneGuidanceState)
    {
        NB_NLGStateDestroy(navigation->naturalLaneGuidanceState);
        navigation->naturalLaneGuidanceState = NULL;
    }
    if (navigation->speedLimit)
    {
        NB_SpeedLimitInformationDestroy(navigation->speedLimit);
        navigation->speedLimit = NULL;
    }

    if (navigation->speedlimitState)
    {
        NB_SpeedLimitStateDestroy(navigation->speedlimitState);
        navigation->speedlimitState = NULL;
    }

    if (navigation->routeHandler)
    {
        NB_RouteHandlerDestroy(navigation->routeHandler);
    }

    if (navigation->trafficHandler)
    {
        NB_TrafficHandlerDestroy(navigation->trafficHandler);
    }

    if (navigation->cameraHandler)
    {
        NB_SearchHandlerDestroy(navigation->cameraHandler);
    }

    if (navigation->poiHandler)
    {
        NB_SingleSearchHandlerDestroy(navigation->poiHandler);
    }

    if (navigation->pronunHandler)
    {
        NB_PronunHandlerDestroy(navigation->pronunHandler);
    }

    if (navigation->navState)
    {
        if (navigation->navState->navRoute)
        {
            NB_RouteInformationDestroy(navigation->navState->navRoute);
            navigation->navState->navRoute = NULL;
        }

        NB_NavigationStateDestroy(navigation->navState);
        navigation->navState = NULL;
    }

    if (navigation->navPublicState)
    {
        NB_NavigationPublicStateDestroy(navigation->navPublicState);
    }

    if (navigation->gpsFilter)
    {
        NB_GpsFilterDestroy(navigation->gpsFilter);
    }

    if (navigation->recalcLimit)
    {
        NB_RecalcLimitDestroy(navigation->recalcLimit);
    }

    if (navigation->specialRegion)
    {
        NB_SpecialRegionInformationDestroy(navigation->specialRegion);
        navigation->specialRegion = NULL;
    }

    if (navigation->specialRegionState)
    {
        NB_SpecialRegionStateDestroy(navigation->specialRegionState);
        navigation->specialRegionState = NULL;
    }

    NB_ContextSetSnippetLengthCallback(navigation->context, NULL, NULL);

    nsl_free(navigation);

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetGuidanceInformation(NB_Navigation* navigation, NB_GuidanceInformation* information)
{
    NB_ASSERT_VALID_THREAD(navigation->context);

    navigation->guidanceInformation = information;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetPointsOfInterestConfiguration(NB_Navigation* navigation, NB_PointsOfInterestPrefetchConfiguration* poiConfiguration)
{
    NB_ASSERT_VALID_THREAD(navigation->context);

    navigation->poiConfiguration = *poiConfiguration;

    return NE_OK;
}

NB_DEC NB_Error
NB_NavigationSetActiveRoute(NB_Navigation* navigation)
{
    // DEPRECATED
    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationStartWithRouteInformation(NB_Navigation* navigation, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_CameraInformation* camera, NB_PointsOfInterestInformation* pointsOfInterest)
{
    NB_Error result = NE_OK;
    NB_RoutePrivateConfiguration* routePrivateConfig = NB_RouteInformationGetPrivateConfiguration(route);
    NB_NavigateTransportMode transportMode = NB_NTM_Invalid;

    if (!navigation || !route)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (navigation->navState)
    {
        if (navigation->navState->navRoute)
        {
            NB_RouteInformationDestroy(navigation->navState->navRoute);
            navigation->navState->navRoute = NULL;
        }
        NB_NavigationStateDestroy(navigation->navState);
        navigation->navState = NULL;
    }

    (void)NB_RouteInformationGetTransportMode(route, &transportMode);
    navigation->navState = AllocateNavigationState(navigation, routePrivateConfig, transportMode);

    if (!navigation->navState)
    {
        return NE_NOMEM;
    }

    result = NB_RecalcLimitReset(navigation->recalcLimit);
    if (navigation->poiInformation)
    {
        NB_PointsOfInterestInformationDestroy(navigation->poiInformation);
    }
    navigation->poiInformation = pointsOfInterest;

    result = result ? result : NB_NavigationNewRoute(navigation, route, traffic, camera); /* @todo (BUG 55820) -- figure out if this is the right thing to do */
    return result;
}


NB_DEF NB_Error
NB_NavigationStartWithRouteParameters(NB_Navigation* navigation, NB_RouteParameters* parameters, nb_boolean restart)
{
    NB_Error result;
    NB_RoutePrivateConfiguration config;
    NB_RouteSettings routeSettings = {0};

    if (!navigation || !parameters)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    result = NB_RouteParametersCopyPrivateConfigFromParameters(parameters, &config);

    if (result != NE_OK)
    {
        return result;
    }

    (void)NB_RouteParametersGetRouteSettingsFromQuery(parameters, &routeSettings);
    navigation->navState = AllocateNavigationState(navigation, &config,
                                                   routeSettings.vehicle_type == NB_TransportationMode_Pedestrian ? NB_NTM_Pedestrian : NB_NTM_Vehicle );

    if (!navigation->navState)
    {
        if (config.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableDestroy(config.prefs.laneGuidanceFontMap, TRUE);
            config.prefs.laneGuidanceFontMap = NULL;
        }
        return NE_NOMEM;
    }

    /* if pronoun files are not needed, change max pronoun file to 0 in the route request*/
    if (navigation->announceType != NB_NAT_Street || !nsl_strempty(parameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        NB_RouteParametersSetMaxPronunFiles(parameters, 0);
    }

    result = NB_RouteHandlerStartRequest(navigation->routeHandler, parameters);
    if (result == NE_OK && navigation->callbacks.routeRequestedCallback)
    {
        NB_NavigateRouteRequestReason reason = parameters->config.wantAlternateRoutes ? NB_NRRR_RouteSelector : NB_NRRR_InitialRoute;
        navigation->callbacks.routeRequestedCallback(navigation, reason, navigation->callbacks.userData);
    }

    if (result == NE_OK)
    {
        NB_NavigateRouteCalculationType type = NB_RCT_Initial;

        if (restart)
        {
            type = NB_RCT_Recalc;
        }

        SetStatus(navigation, NB_NS_InitialRoute);
        PlayRouteDownloadMessage(navigation, type);

        result = result ? result : NB_RecalcLimitReset(navigation->recalcLimit);
    }
    else
    {
        navigation->navState->error = result;
        SetStatus(navigation, NB_NS_Error);
        LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Success);
    }

    if (config.prefs.laneGuidanceFontMap)
    {
        CSL_HashTableDestroy(config.prefs.laneGuidanceFontMap, TRUE);
        config.prefs.laneGuidanceFontMap = NULL;
    }
    return result;
}

NB_DEF NB_Error
NB_NavigationNewRoute(NB_Navigation* navigation, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_CameraInformation* camera)
{
    NB_Error overallResult = NE_OK;
    NB_Error result = NE_OK;
    NB_GpsLocation  fix;
    nb_boolean receivedRemainingRoute = FALSE;
    NB_GpsHistory* gpsHistory = NULL;
    double traveledDistanceOfLastRoute = 0;

    gpsfix_clear(&fix);

    if (!navigation || !route)
    {
        return NE_INVAL;
    }

    if (navigation->navState && navigation->navState->navRoute)
    {
        // Get traveled distance of last route before new route validated.
        traveledDistanceOfLastRoute =  NB_RouteInformationTraveledDistance(
                                                       NB_ContextGetDataState(navigation->context),
                                                       &navigation->navState->navRoute->dataReply,
                                                       navigation->navState->currentRoutePosition.closest_maneuver,
                                                       navigation->navState->currentRoutePosition.closest_segment,
                                                       navigation->navState->currentRoutePosition.segment_remain);
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigation->navState->applyingNewRoute = TRUE;
    navigation->navMode = NB_NM_Automatic;
    navigation->theFirstFixProcessed = TRUE;

    // workaround for recalc case (BR#145668)
    // it looks like PAL_Audio does not call NavigationAudioManager::CCC_AudioPlayerCallback()
    // with ABPAL_AudioState_Ended for the first played audio message (ex. "turn right")
    // if the second message (ex. "recalc tone") is too close to the first message.
    // The QA record GUIDANCE_MESSAGE_RETRIEVED means that a message playback is started.
    // The time is 1 sec (1 GPS fix) between two qa records GUIDANCE_MESSAGE_RETRIEVED (see QA log from BR).
    // It is a little value. The proper behaviour NB_NavigationMessagePlayed() is called by CCI
    // and currentMessage is set to NULL for any cases. Probably the issue is in PAL_Audio.
    // The issue was reproduced once and root cause is not found therefore
    // currentMessage is reset here to avoid the issue when guidance messages are not played
    // currentMessage should not be destroyed because CCI has already destoyed currentMessage
    navigation->navState->currentMessage = NULL;

    NB_QaLogNavigationSessionRoute(navigation->context, navigation->navSessionId, route->routeID.data, route->routeID.size);
    {
        char routeId[MAX_FILE_NAME_LEN] = {0};
        nb_size length = MAX_FILE_NAME_LEN / 2;
        if ((int)route->routeID.size < length)
        {
            length = route->routeID.size;
        }

        hexlify(length, (const char*)route->routeID.data, routeId);

        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_SESSION_ROUTE sessionId = %d routeId = %d",
                navigation->navSessionId, routeId);
    }

    CancelPendingRequests(navigation);

    if ( NB_RouteInformationCompareRouteIDs(route, navigation->navState->navRoute) &&
         NB_RouteInformationIsRouteComplete(route) &&
         !NB_RouteInformationIsRouteComplete(navigation->navState->navRoute)
        )
    {
        receivedRemainingRoute = TRUE;
    }

    if (navigation->navState->navRoute != NULL)
    {
        if (navigation->fastNavStartupMode)
        {
            if ((route->dataReply.first_major_road != NULL && navigation->navState->navRoute->dataReply.first_major_road == NULL) ||
                (route->dataReply.first_major_road == NULL && navigation->navState->navRoute->dataReply.first_major_road != NULL) ||
                ((route->dataReply.first_major_road != NULL && navigation->navState->navRoute->dataReply.first_major_road != NULL) &&
                 (nsl_strcmp(route->dataReply.first_major_road->primary, navigation->navState->navRoute->dataReply.first_major_road->primary) != 0)))
            {
                PlayRouteDownloadMessage(navigation, NB_RCT_RouteUpdated);
            }
            else
            {
                navigation->fastNavStartupMode = FALSE;
            }
        }
        result = NB_RouteInformationDestroy(navigation->navState->navRoute);
        navigation->navState->navRoute = NULL;
    }

    overallResult = overallResult ? overallResult : result;

    if (navigation->trafficInformation != NULL)
    {
        result = NB_TrafficInformationDestroy(navigation->trafficInformation);
        navigation->trafficInformation = NULL;
    }

    overallResult = overallResult ? overallResult : result;

    if (navigation->cameraInformation != NULL)
    {
        result = NB_CameraInformationDestroy(navigation->cameraInformation);
        navigation->cameraInformation = NULL;
    }

    overallResult = overallResult ? overallResult : result;

    if (navigation->poiInformation != NULL)
    {
        result = NB_PointsOfInterestInformationDestroy(navigation->poiInformation);
        navigation->poiInformation = NULL;
    }

    overallResult = overallResult ? overallResult : result;

    if (navigation->poiInformation == NULL)
    {
        result = NB_PointsOfInterestInformationCreate(navigation->context, &navigation->poiInformation);
        navigation->poiLastQueryRouteRemaining = 0;
        navigation->poiLastQueryFailed = FALSE;
        navigation->poiQueryFailedTimes = 0;
    }

    overallResult = overallResult ? overallResult : result;

    if (navigation->speedLimit != NULL)
    {
        result = NB_SpeedLimitInformationDestroy(navigation->speedLimit);
        navigation->speedLimit = NULL;
    }

    overallResult = overallResult ? overallResult : result;

    if (overallResult == NE_OK)
    {
        navigation->navState->navRoute = route;
        navigation->trafficInformation = traffic;
        navigation->cameraInformation = camera;
        navigation->navState->completeRouteDownloadRetries = 0;
        navigation->pronunDownloadRetries = 0;

        if (!NB_RouteInformationIsRouteComplete(route) )
        {
            NB_ScheduleCompleteRouteRequest(navigation, navigation->configuration.completeRouteRequestDelay);
        }

        result = result ? result : NB_RouteProtectVoiceCache(navigation->navState->navRoute);

        if (result == NE_OK)
        {
            if (receivedRemainingRoute)
            {
                gpsHistory = NB_ContextGetGpsHistory(navigation->context);

                if (gpsHistory)
                {
                    NB_Error result2 = NB_GpsHistoryGetLatestHeadingFix(gpsHistory, &fix);
                    if (result2)
                    {
                        NB_QaLogAppState(navigation->context, "No heading fix");
                        result2 = NB_GpsHistoryGetLatest(gpsHistory, &fix);
                    }

                    if (result2)
                    {
                        NB_QaLogAppState(navigation->context, "No history fix");
                        result = result2;
                    }
                }
                else
                {
                    NB_QaLogAppState(navigation->context, "No GPS History -- Error");
                    result = NE_UNEXPECTED;
                }

                // If we haven't matched the route yet, increment the matches needed because the history fix won't
                // match the route
                if (!navigation->navState->initialRouteMatch)
                {
                    navigation->navState->initialRouteMatchesNeeded++;
                }

                // Reset the off route and wrong way counts in case the remaining route is received just after an off route point
                // This will prevent the same off route point from initiating a recalc from within the route handler callback
                navigation->navState->offRouteCount = 0;
                navigation->navState->wrongWayCount = 0;

                navigation->navState->applyingFullRoute = TRUE;
            }
            else
            {
                // If the route is realy a new route (with difference routeID), accumulate the traveled distance of navigation
                navigation->traveledDistance += traveledDistanceOfLastRoute;

                /* Update Startup Region */
                CalculateStartupRegion(navigation);

                result = result ? result : NB_NavigationStateNewRoute(navigation->navState);
                ResetNavigationPublicState(navigation->navPublicState);
                result = result ? result : NB_RouteInformationGetOriginGpsLocation(route, &fix);
                result = result ? result : NB_GuidanceProcessorStateReset(navigation->guidanceState, navigation->navState);
                result = result ? result : NB_TrafficProcessorStateReset(navigation->trafficState);
                result = result ? result : NB_CameraProcessorStateReset(navigation->cameraState);

                if (navigation->enhancedContentState)
                {
                    result = result ? result : NB_EnhancedContentProcessorStateReset(navigation->enhancedContentState);
                }
            }
        }

        if (result != NE_OK)
        {
            LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Failed);
        }

        overallResult = result;
    }
    else
    {
        LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Failed);
    }

    navigation->navState->applyingFullRoute = FALSE;
    navigation->navState->applyingNewRoute = FALSE;

    return overallResult;
}

static NB_Error NB_NavigationTheFirstFixProcess(NB_Navigation* navigation)
{
    NB_Error error = NE_OK;
    if (navigation->theFirstFixProcessed)
    {
        NB_SpeedLimitInformation* speedLimit = NULL;
        NB_PositionRoutePositionInformation currentPos = { 0 };
        NB_SpecialRegionInformation* specialRegion = NULL;

        if (NB_RouteInformationIsRouteComplete(navigation->navState->navRoute))
        {
            if (navigation->announceType == NB_NAT_Street
                && nsl_strempty(navigation->navState->navRoute->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
            {
                NB_SchedulePronunDownloadRequest(navigation, navigation->configuration.pronunRequestDelay);
            }

            NB_RouteInformationGetTransportMode(navigation->navState->navRoute, &navigation->navState->transportMode);
            if(navigation->navState->transportMode != NB_NTM_Pedestrian)
            {
                NB_ScheduleTrafficNotify(navigation, navigation->configuration.trafficNotifyDelay, !navigation->initialNotifyComplete);
            }
            ScheduleCameraDownloadRequest(navigation, navigation->configuration.cameraSearchDelay);
        }

        NB_NavigationGetRoutePositionInformation(navigation, &currentPos);

        error = NB_SpeedLimitInformationCreateFromDataNavReply(navigation->context, &navigation->navState->navRoute->dataReply, &speedLimit);
        if(error == NE_OK)
        {
            error = NE_OK;
            navigation->speedLimit = speedLimit;

            NB_SpeedLimitProcessorDownloadPicture(navigation->navState->navRoute, navigation->speedlimitState,
                                                  navigation->speedLimit);
            NB_SpeedLimitProcessorUpdate(navigation->speedlimitState, navigation->navState->navRoute, navigation->speedLimit, &currentPos, 0, NavigationSpeedLimitCallbackFunctionEx, navigation);
            SchedulePoiSearchRequest(navigation, DEFAULT_POI_SCHEDULE_TIMEOUT_MSECS);
        }
        //if route reply not have a speed region, speedlimit create will return NE_NOENT, that not a error.
        if (error == NE_NOENT)
        {
            error = NE_OK;
        }

        //download special region picture.
        if (error == NE_OK)
        {
            error = NB_SpecialRegionInformationCreateFromDataNavReply(navigation->context, &navigation->navState->navRoute->dataReply, &specialRegion);
            if (error == NE_OK)
            {
                if(navigation->specialRegion)
                {
                    NB_SpecialRegionInformationDestroy(navigation->specialRegion);
                }
                navigation->specialRegion = specialRegion;

                NB_SpecialRegionProcessorDownloadPicture(navigation->navState->navRoute, navigation->specialRegionState,
                                                         navigation->specialRegion);
                NB_SpecialRegionProcessorUpdate(navigation->specialRegionState, navigation->navState->navRoute, navigation->specialRegion,
                                                &currentPos, NavigationSpecialRegionCallbackFunction, navigation);
            }
        }
        //if route reply not have a special region, the information create function will return NE_NOENT, that is not a error.
        if (error == NE_NOENT)
        {
            error = NE_OK;
        }

        navigation->theFirstFixProcessed = FALSE;
    }

    return error;
}

NB_DEF NB_Error
NB_NavigationUpdateCameraInformation(NB_Navigation* navigation, NB_CameraInformation* cameraInformation)
{
    if (!navigation || !cameraInformation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (navigation->cameraInformation)
    {
        NB_CameraInformationDestroy(navigation->cameraInformation);
    }

    navigation->cameraInformation = cameraInformation;

    return NE_OK;
}

NB_DEC void
NB_NavigationProcessViaPoint(NB_Navigation* navigation, NB_PositionRoutePositionInformation* currentPos)
{
    //via point
    if(navigation->navState->navRoute && navigation->callbacks.reachViaPointCallback)
    {
        double minDistance = 0;
        double distance = 0;
        data_via_point*    minDistanceLocation = NULL;
        data_nav_reply* reply = NB_RouteInformationGetDataNavReply(navigation->navState->navRoute);
        int length = CSL_VectorGetLength(reply->via_point);
        for(int i = 0; i < length; i++)
        {
            double heading = 0;
            data_via_point* via = (data_via_point*)CSL_VectorGetPointer(reply->via_point, i);
            if(via->passed)
            {
                continue;
            }
            distance = NB_SpatialGetLineOfSightDistance(currentPos->projLat, currentPos->projLon, via->point.lat,
                                                        via->point.lon, &heading);
            if (distance < navigation->navState->navRoute->parameterConfiguration.viaPointThreshold)
            {
                if((!minDistanceLocation) || (i > 0 && (minDistance > distance)))
                {
                    minDistance = distance;
                    minDistanceLocation = via;
                }
            }
        }
        if(minDistanceLocation)
        {
            data_util_state* pds = NULL;
            minDistanceLocation->passed = TRUE;
            pds = NB_ContextGetDataState(navigation->context);
            NB_Location location;
            data_via_point_to_nimlocation(pds, minDistanceLocation, &location);
            navigation->callbacks.reachViaPointCallback(&location, navigation->callbacks.userData);
        }
    }
    
}


NB_DEF NB_Error
NB_NavigationUpdateTrafficInformation(NB_Navigation* navigation, NB_TrafficInformation* trafficInformation)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (navigation->trafficInformation)
    {
        NB_TrafficInformationDestroy(navigation->trafficInformation);
        navigation->trafficInformation = NULL;
    }

    navigation->trafficInformation = trafficInformation;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationUpdateGPSLocation(NB_Navigation* navigation, NB_GpsLocation* location)
{
    NB_Error error = NE_OK;
    NB_PositionRoutePositionInformation currentPos = { 0 };
    NB_GpsHistory* history = 0;
    nb_boolean recalcRequired = FALSE;
    enum nav_update_reason recalcReason = nav_update_none;
    NB_NavigateStatus newStatus = NB_NS_Invalid;
    NB_GpsLocation filteredFix = { 0 };
    nb_boolean newRoute = FALSE;
    uint32 previousManeuver = NAV_MANEUVER_NONE;
    NB_NavigateManeuverPos  previousManeuverPosition = NB_NMP_Invalid;
    nb_boolean switchToTBT = FALSE;
    nb_boolean switchToStatic = FALSE;

    if (!navigation || !navigation->navState || !location)
    {
        return NE_INVAL;
    }

    if (navigation->suspended)
    {
        return NE_UNEXPECTED;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    NB_QaLogGPSFix(navigation->context, location);
    
    NB_GpsLocation latestLocation;
    history = NB_ContextGetGpsHistory(navigation->context);
    if (history)
    {
        NB_GpsHistoryGetLatest(history, &latestLocation);
        error = NB_GpsHistoryAdd(history, location);
    }

    // Don't try to apply fix to session if 1) there is no a route or 2) a recalc is in progress
    if (navigation->status == NB_NS_Error || (navigation->status == NB_NS_InitialRoute && navigation->navState->navRoute == NULL) || (navigation->status == NB_NS_UpdatingRoute && !navigation->navState->newRoute && !navigation->navState->wasNetworkBlocked))
    {
        return error;
    }
    //for channel
    if (navigation->navState->navRoute && history)
    {
        data_nav_maneuver* preManeuver = NULL;
        data_nav_maneuver* curManeuver = NULL;
        uint32 maneuverNumber = NB_RouteInformationGetManeuverCount(navigation->navState->navRoute);
        data_util_state* pds = NULL;
        pds = NB_ContextGetDataState(navigation->context);
        if(maneuverNumber > navigation->navPublicState->currentManeuver)
        {
            curManeuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute,navigation->navPublicState->currentManeuver);
            if(curManeuver > 0)
            {
                preManeuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute,navigation->navPublicState->currentManeuver - 1);
            }
        }
        uint32 diff = location->gpsTime - latestLocation.gpsTime;
        if((preManeuver != NULL && is_tunnel_entry(data_string_get(pds,&preManeuver->command))) || (curManeuver != NULL && is_tunnel_entry(data_string_get(pds,&curManeuver->command))))
        {
            if(diff > GPS_UPDATING_TIMEOUT)
            {
                return NE_OK;
            }
            if(location->horizontalUncertaintyAlongAxis < 0 || location->horizontalUncertaintyAlongAxis > OUT_TUNNEL_GPS_SIFTER_ERROR_RADIUS)
            {
                return NE_OK;
            }
        }
        
    }
    error = error ? error : NB_NavigationTheFirstFixProcess(navigation);

    // save previous maneuver position for detecting changes.
    previousManeuver = navigation->navPublicState->currentManeuver;
    previousManeuverPosition = navigation->navPublicState->pos;

    newRoute = navigation->navState->newRoute;

    if (navigation->configuration.filteredGpsEnabled)
    {
        NB_GpsFilterGetFilteredFix(navigation->gpsFilter, location, &filteredFix);
        NB_QaLogGPSFixFiltered(navigation->context, &filteredFix);
        location = &filteredFix;
    }

    error = NB_PositionProcessorUpdate(navigation->navState, location);

    // if current maneuver has 'disable-guidance' flag, we should switch to static route mode or
    // switched back to automatic mode if current maneuver is turn by turn(TBT)
    if (error == NE_OK && navigation->navState->navRoute)
    {
        boolean guidanceDisabled = FALSE;
        if ( NE_OK == NB_RouteInformationGetManeuverGuidanceInformation(
                        navigation->navState->navRoute,
                        navigation->navState->manueverProgress,
                        &guidanceDisabled) )
        {
            NB_NavigationMode newMode = (guidanceDisabled == TRUE) ? NB_NM_StaticRoute : NB_NM_Automatic;

            // We shall only switch back to TBT mode from static routing mode when user is on route
            if ( navigation->navMode != newMode &&
                 (newMode != NB_NM_Automatic || navigation->navState->onRoute) )
            {
                // notify client about mode change
                if (navigation->callbacks.modeChangeCallback != NULL)
                {
                    navigation->callbacks.modeChangeCallback(navigation, newMode, navigation->callbacks.userData);
                }
                navigation->navMode = newMode;

                if (newMode == NB_NM_Automatic)
                {
                    switchToTBT = TRUE;
                }
                else if (newMode == NB_NM_StaticRoute)
                {
                    switchToStatic = TRUE;
                }
            }
        }
    }

    if (!error && (newRoute == FALSE) && (navigation->navState->applyingFullRoute == FALSE) && (navigation->status != NB_NS_NavigatingConfirmRecalc))
    {
        error = NB_NavigationCheckForRecalc(navigation, location, &recalcRequired, &recalcReason);
    }

    if (NB_ContextGetQaLog(navigation->context))
    {
        const char *action = newRoute ? "NEW" : recalcRequired ? "RECALC" : "NONE";

        if (navigation->navState->offRouteCount || navigation->navState->wrongWayCount)
        {
            NB_QaLogNavigationOffRoute(navigation->context, navigation->navState->offRouteCount, navigation->navState->wrongWayCount, action);
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_SESSION_RECALC orcount = %d wwcount = %d action = %d",
                navigation->navState->offRouteCount, navigation->navState->wrongWayCount, action);
        }

        if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(navigation->context)))
        {
            NB_QaLogNavigationPosition(navigation->context, navigation->navState, action);
        }
    }

    // we should not start automatic recalculation in static route mode
    if (recalcRequired && navigation->navMode != NB_NM_StaticRoute)
    {
        //We should never recalculation when a new route is being applied
        if (newRoute)
        {
            error =  NE_UNEXPECTED;
        }

        error = error ? error : RecalcCheckLimit(navigation, TRUE);
        if (error)
        {
            navigation->navState->error = error;
            SetStatus(navigation, NB_NS_Error);
            LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Success);
        }

        return error;
    }
    else if (navigation->navState->positionUpdated)
    {
        error = error ? error : NB_NavigationGetRoutePositionInformation(navigation, &currentPos);

        if (navigation->cameraState && navigation->cameraInformation)
        {
            error = error ? error : NB_CameraProcessorUpdate(navigation->cameraState, navigation->navState, navigation->cameraInformation, &currentPos);
        }

        if (navigation->trafficState && navigation->trafficInformation && navigation->navMode != NB_NM_StaticRoute)
        {
            error = error ? error : NB_TrafficProcessorUpdate(navigation->trafficState, navigation->navState, navigation->trafficInformation, &currentPos);
        }

        if (navigation->speedlimitState && navigation->speedLimit && navigation->navMode != NB_NM_StaticRoute)
        {
            error = error ? error : NB_SpeedLimitProcessorUpdate(navigation->speedlimitState, navigation->navState->navRoute, navigation->speedLimit, &currentPos, location->horizontalVelocity, NavigationSpeedLimitCallbackFunctionEx, navigation);
        }

        if (navigation->navState->navRoute && navigation->navState->navRoute->transportMode == NB_NTM_Vehicle)
        {
            if (navigation->enhancedContentState)
            {
                NB_EnhancedContentProcessorUpdate(navigation->enhancedContentState, navigation->navState);

                if (navigation->callbacks.enhancedContentNotificationCallback && navigation->enhancedContentState->stateChanged)
                {
                    navigation->callbacks.enhancedContentNotificationCallback(
                        navigation,
                        navigation->enhancedContentState,
                        navigation->enhancedContentState->availableContent,
                        navigation->callbacks.userData);
                }
            }
        }

        if (navigation->specialRegionState && navigation->specialRegion && navigation->navMode != NB_NM_StaticRoute)
        {
            error = error ? error : NB_SpecialRegionProcessorUpdate(navigation->specialRegionState, navigation->navState->navRoute, navigation->specialRegion, &currentPos, NavigationSpecialRegionCallbackFunction, navigation);
        }

        NB_NavigationProcessViaPoint(navigation, &currentPos);

        /* if POI search enabled and a search is NOT in progress */
        if (navigation->poiConfiguration.categoryCount && (!NB_SingleSearchHandlerIsRequestInProgress(navigation->poiHandler)))
        {
            /* If last search failed or POI search timer expired or the minimum POI search distance has passed, initiate a POI search */
            nb_boolean  poiSearchTimerExpStatus = FALSE;

            GetPoiSearchTimerTimeOutStatus(navigation, &poiSearchTimerExpStatus);
            // TODO add count for failed POI query
            if ((navigation->poiLastQueryFailed && navigation->poiQueryFailedTimes < 3)|| poiSearchTimerExpStatus || IsPointsOfInterestPreSearchPending(navigation))
            {
                if( poiSearchTimerExpStatus )
                {
                    SetPoiSearchTimerTimeOutStatus(navigation, FALSE);
                }
                error = error ? error : NB_NavigationSearchPointsOfInterest(navigation, navigation->navState->navRoute);
            }
        }

        /* If there was an error updating the position, save it in the nav state and reset error */
        if (error && navigation->navState->error == NE_OK)
        {
            navigation->navState->error = error;
            error = NE_OK;
        }

        if (navigation->navState->error != NE_OK)
        {
            newStatus = NB_NS_Error;
            LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Success);
        }
        else if (navigation->navState->pastArrivedThreshold)
        {
            newStatus = NB_NS_Arrived;
        }
        else if (navigation->navState->pastArrivingThreshold)
        {
            /* Getting close to the destination */
            newStatus = NB_NS_NavigatingArriving;

            /* If entering arriving state, calculate arriving region radius for suppressing recalcs */
            if (navigation->status != NB_NS_NavigatingArriving)
            {
                CalculateArrivingRegion(navigation, location->latitude, location->longitude);
            }
        }
        else if (is_ferry_exit(navigation->navState->turnCode))
        {
            /* Starting on ferry exit */
            /* Go to NavFerryRoute */
            newStatus = NB_NS_NavigatingFerry;
        }
        else if (!navigation->navState->initialRouteMatch)
        {
            newStatus = NB_NS_NavigatingStartup;
        }
        else
        {
            newStatus = NB_NS_Navigating;
        }
    }
    else if (!navigation->navState->initialRouteMatch)
    {
        if (!error)
        {
            /* Handle the case where we never match to the route */
            if (navigation->status == NB_NS_NavigatingConfirmRecalc)
            {
                /* stay in NB_NS_NavigatingConfirmRecalc until route match or recalc */
                newStatus = NB_NS_NavigatingConfirmRecalc;
            }
            else
            {
                newStatus = NB_NS_NavigatingStartup;
            }
        }
        else
        {
            navigation->navState->error = error;
            newStatus = NB_NS_Error;
            LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Success);
        }
    }
    else
    {
        // Off route but no recalc needed, use last status
        newStatus = navigation->status;
    }

    NB_NavigationUpdateNavigationState(navigation, location);
    if (newStatus != navigation->status)
    {
        SetStatus(navigation, newStatus);
    }


    if ((navigation->navPublicState->currentManeuver != previousManeuver) ||
        (navigation->navPublicState->pos != previousManeuverPosition))
    {
        if (switchToTBT)
        {
            (void)PlaySwitchToTbtMessage(navigation);
        }
        if (navigation->callbacks.maneuverPositionChangeCallback)
        {
            navigation->callbacks.maneuverPositionChangeCallback(navigation, navigation->navPublicState->currentManeuver,
                    navigation->navPublicState->pos, navigation->callbacks.userData);
        }
    }
    if (switchToStatic)
    {
    	NB_PlayTBTToStaticMessage(navigation);
    }
    NB_NavigationPlayMessage(navigation);

    if (error == NE_OK && navigation->navState->positionUpdated && navigation->naturalLaneGuidanceInformation && navigation->naturalLaneGuidanceState)
    {
        NB_NLGProcessorUpdate(navigation, navigation->navState->navRoute, &currentPos, navigation->naturalLaneGuidanceInformation, location->horizontalVelocity);
    }

    return error;
}

static void
NB_GuidanceHeadsUp(NB_Navigation* navigation)
{
    if (!navigation)
    {
        return;
    }
    NB_GuidanceMessage* message;
    if(navigation->callbacks.guidanceMessageCallback && NB_GuidanceMessageCreate(navigation->context, NB_NAS_Lookahead,NB_GMT_HeadsUp,&message) == NE_OK)
    {
        (navigation->callbacks.guidanceMessageCallback)(navigation, message, navigation->callbacks.userData);
        NB_GuidanceMessageDestroy(message);
    }
}

#define MIN_EXTRAPOLATE_SPEED   8.          // meters per second (17.9 mph)
#define ROTATE_DEGREE_TO_METERS .1          // turn angle will also be calculated as distance. This is the ratio. It effects turn speed
#define MIN_ROTATE              1.          // add extra points to do turn animation if angle bigger than this value
#define MAX_MERGE_DISTANCE      200         // if distance over 200 meters, just use new path

static double
StandardHeading( double heading )
{
    while ( heading > 180.f )
    {
        heading -= 360.f;
    }
    while ( heading < -180.f )
    {
        heading += 360.f;
    }
    return heading;
}

NB_DEF NB_Error
NB_NavigationGetExtrapolatePath(NB_Navigation* navigation, uint32 duration, uint32 timeInterval, NB_ExtrapolatePosition* path, uint32 length, uint32* count, double minSpeedExtrapolationThreshold)
{
    uint32 maneuver = 0, segment = 0, num_points=0, i = 0;
    data_nav_maneuver*    pmaneuver = NULL;
    data_util_state*    dataState = NULL;
    double                p1_lat=0.0, p1_lon=0.0;
    double                p2_lat=0.0, p2_lon=0.0;
    double                p1_p2_heading=0.0;
    double                p2_p3_heading=0.0;
    double              distanceInterval = 0.;
    double              distanceRemain = 0.;
    double              distance = 0.;
    double              rotateDist = 0.;
    double              diffHeading = 0.;
    double              minRotate = 0.;
    double              f = 0.;
    NB_NavigationState* state = NULL;
    NB_RouteInformation* route = NULL;
    double              speedRatio = 1;
    if (!navigation || !navigation->navState || !path || !length || !count)
    {
        return NE_INVAL;
    }
    state = navigation->navState;
    if(!state)
    {
        return NE_INVAL;
    }
    if ( !state->onRoute || !state->initialRouteMatch || state->currentSpeed < minSpeedExtrapolationThreshold || state->pastArrivedThreshold )
    {
        *count = 0;
        return NE_OK;
    }
    route = state->navRoute;
    *count = duration / timeInterval + 1;
    if ( *count > length )
    {
        *count = length;
    }

    dataState = NB_ContextGetDataState(route->context);

    i = 0;
    p1_lat = state->currentRoutePosition.proj_lat;
    p1_lon = state->currentRoutePosition.proj_lon;
    p1_p2_heading = state->currentRoutePosition.segment_heading;

    // only search one maneuver, in case user may drive off route
    maneuver = state->manueverProgress;
    segment = state->segmentProgress;

    if (navigation->lastPosition.latitude != INVALID_LATLON && navigation->lastPosition.maneuver == maneuver)
    {
        double p1_current_heading = 0;
        double p1_current_distance = 0;

        p1_lat = navigation->lastPosition.latitude;
        p1_lon = navigation->lastPosition.longitude;
        maneuver = navigation->lastPosition.maneuver;
        segment = navigation->lastPosition.segment;
        p1_current_distance = NB_SpatialGetLineOfSightDistance(p1_lat, p1_lon, state->currentRoutePosition.proj_lat,
                                                               state->currentRoutePosition.proj_lon, &p1_current_heading);
        if (p1_current_distance > 0)
        {
            double totalDistance = state->currentSpeed * (float)duration / 1000;
            if (totalDistance > 0)
            {
                double differ = nsl_fabs(p1_current_heading - p1_p2_heading);
                while(differ > 360)
                {
                    differ -= 360;
                }
                if (differ < 90 || differ > 270)
                {
                    speedRatio = (totalDistance + p1_current_distance) / totalDistance;
                }
                else
                {
                    speedRatio = (totalDistance - p1_current_distance) / totalDistance;
                }
            }
        }
        if(speedRatio<0)
        {
            speedRatio = 0;
        }
    }
    else
    {
        speedRatio = 1;
    }
    distanceRemain = distanceInterval = state->currentSpeed * (float)duration / (float)*count / 1000.f * speedRatio;
    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, maneuver);
    num_points = data_polyline_len(dataState, &pmaneuver->polyline);
    for (; segment < (num_points-1); ++segment)
    {
        (void)data_polyline_get(dataState, &pmaneuver->polyline, segment+1, &p2_lat, &p2_lon, NULL, &p2_p3_heading);

        if (p2_p3_heading == POLYLINE_INVALID_HEADING)
        {
            p2_p3_heading = p1_p2_heading;
        }

        /* Skip Zero Length Segments */
        if (p1_lat == p2_lat && p1_lon == p2_lon)
        {
            continue;
        }
        double heading;
        distance = NB_SpatialGetLineOfSightDistance(p2_lat, p2_lon, p1_lat, p1_lon, &heading);
        while ( distanceRemain < distance )
        {
            f = distanceRemain / distance;
            path[i].latitude = p1_lat + ( p2_lat - p1_lat ) * f;
            path[i].longitude = p1_lon + ( p2_lon - p1_lon ) * f;
            path[i].heading = p1_p2_heading;
            path[i].maneuver = maneuver;
            path[i].segment = segment;
            if ( ++i == *count )
            {
                goto extrapolateEnd;
            }
            distanceRemain += distanceInterval;
        }
        diffHeading = StandardHeading( p2_p3_heading - p1_p2_heading );
        // if rotate angle is bigger than MIN_ROTATE
        // keep position and turn round
        if ( nsl_fabs(diffHeading) > MIN_ROTATE )
        {
            minRotate = diffHeading > 0.0 ? MIN_ROTATE : -MIN_ROTATE;
            diffHeading -= minRotate;
            rotateDist = nsl_fabs(diffHeading) * ROTATE_DEGREE_TO_METERS;
            distance += rotateDist;
            while ( distanceRemain < distance )
            {
                f = ( distance - distanceRemain ) / rotateDist;
                path[i].latitude = p2_lat;
                path[i].longitude = p2_lon;
                path[i].heading = p2_p3_heading - f * diffHeading;
                path[i].maneuver = maneuver;
                path[i].segment = segment;
                if ( ++i == *count )
                {
                    goto extrapolateEnd;
                }
                distanceRemain += distanceInterval;
            }
        }

        distanceRemain -= distance;
        p1_lat = p2_lat;
        p1_lon = p2_lon;
        p1_p2_heading = p2_p3_heading;
    }

extrapolateEnd:
    // in case that route is not long enough
    *count = i;

    return NE_OK;
}

NB_DEF void
NB_NavigationMergeExtrapolatePath(NB_ExtrapolatePosition* newPath, uint32 newPathLength, NB_ExtrapolatePosition* oldPath, uint32 oldPathLength, NB_ExtrapolatePosition* output, uint32 outPutLength, uint32* count)
{
    double difLatitude = 0;
    double difLongitude = 0;
    double diffHeading = 0;
    double distance = 0;
    double minDistance = 0;
    double rotateDir = 0;
    uint32 i = 0;
    uint32 newPathCutPoint = 0;
    uint32 oldPathCutPoint = 0;
    uint32 size = 0;

    if (!newPath || !newPathLength || !output || !outPutLength || !count)
    {
        return;
    }

    // use new path if no old path
    if (!oldPath || !oldPathLength)
    {
        *count = MIN(newPathLength, outPutLength);
        nsl_memcpy(output, newPath, *count * sizeof(NB_ExtrapolatePosition));
        return;
    }

    // if two path is far away, use new path
    if (NB_SpatialGetLineOfSightDistance(newPath[0].latitude, newPath[0].longitude, oldPath[0].latitude, oldPath[0].longitude, NULL) > MAX_MERGE_DISTANCE)
    {
        *count = MIN(newPathLength, outPutLength);
        nsl_memcpy(output, newPath, *count * sizeof(NB_ExtrapolatePosition));
        return;
    }

    // cull all points before old path position in new path to prevent backward

    // calculate distance between first point and current position
    difLatitude = newPath[0].latitude - oldPath[0].latitude;
    minDistance = difLatitude * difLatitude;
    difLongitude = newPath[0].longitude - oldPath[0].longitude;
    minDistance += difLongitude * difLongitude;
    newPathCutPoint = 0;
    rotateDir = 0.;
    for (i = 1; i < newPathLength; ++i)
    {
        // calculate distance from current position
        difLatitude = newPath[i].latitude - oldPath[0].latitude;
        distance = difLatitude * difLatitude;
        difLongitude = newPath[i].longitude - oldPath[0].longitude;
        distance += difLongitude * difLongitude;

        // if distance is bigger than the point before
        // we found the cut point
        if (distance > minDistance)
        {
            newPathCutPoint = i;
            break;
        }
        else
        {
            if (nsl_fabs(minDistance - distance) < 1.e-10
                && distance < 5.) // at turn point
            {
                diffHeading = StandardHeading(newPath[i].heading - oldPath[0].heading);
                if (diffHeading * rotateDir < 0.)
                {
                    newPathCutPoint = i;
                    break;
                }
            }
            minDistance = distance;
            rotateDir = diffHeading;
        }
    }
    if (i == newPathLength && newPathLength > 1)
    {
        // old path exceeds new path
        newPathCutPoint = newPathLength - 1;
    }

    // remove all points behind newPath[newPathCutPoint]
    difLatitude = oldPath[0].latitude - newPath[newPathCutPoint].latitude;
    minDistance = difLatitude * difLatitude;
    difLongitude = oldPath[0].longitude - newPath[newPathCutPoint].longitude;
    minDistance += difLongitude * difLongitude;
    oldPathCutPoint = 1;
    rotateDir = 0.;
    for (i = 1; i < oldPathLength; ++i)
    {
        // calculate distance from current position
        difLatitude = oldPath[i].latitude - newPath[newPathCutPoint].latitude;
        distance = difLatitude * difLatitude;
        difLongitude = oldPath[i].longitude - newPath[newPathCutPoint].longitude;
        distance += difLongitude * difLongitude;

        // if distance is bigger than the point before
        // we found the cut point
        if (distance > minDistance)
        {
            oldPathCutPoint = i - 1;
            break;
        }
        else
        {
            if (nsl_fabs(minDistance - distance) < 1.e-10
                && distance < 5.) // at turn point
            {
                diffHeading = StandardHeading(oldPath[i].heading - newPath[newPathCutPoint].heading);
                if ( diffHeading * rotateDir < 0.)
                {
                    oldPathCutPoint = i - 1;
                    break;
                }
            }
            minDistance = distance;
            rotateDir = diffHeading;
        }
    }

    if (i >= oldPathLength && oldPathLength > 1)
    {
        oldPathCutPoint = oldPathLength - 1;
    }

    // keep oldPath in [0, oldPathCutPoint]
    // keep newPath in [newPathCutPoint, newPathLength]
    *count = MIN(oldPathCutPoint + newPathLength - newPathCutPoint, outPutLength);
    size = MIN(*count, oldPathCutPoint);
    if (size)
    {
        nsl_memcpy(output, oldPath, sizeof(NB_ExtrapolatePosition) * size);
        output += size;
    }
    size = MIN(*count - size, newPathLength - newPathCutPoint);
    if (size)
    {
        nsl_memcpy(output, &newPath[newPathCutPoint], sizeof(NB_ExtrapolatePosition) * size);
    }
}

NB_DEF NB_Error
NB_NavigationSetLastExtrapolatePosition(NB_Navigation* navigation, NB_ExtrapolatePosition* lastPosition)
{
    if (!navigation || !lastPosition)
    {
        return NE_NOMEM;
    }
    navigation->lastPosition = *lastPosition;
    return NE_OK;
}

NB_Error
NB_NavigationUpdateNavigationState(NB_Navigation* navigation, NB_GpsLocation* location)
{
    data_util_state* dataState = NULL;
    data_nav_maneuver* maneuver = NULL;
    data_nav_maneuver* pBaseManeuver = NULL;
    data_nav_maneuver* pNextManeuver = NULL;
    data_nav_reply* routeNavReply = NULL;
    uint32 closetManeuver = NAV_MANEUVER_NONE;
    double collapseDist = 0.f;
    uint32 collapseTime = 0;
    nb_boolean collapseStack = FALSE;
    uint32 baseManeuver = NAV_MANEUVER_NONE;
    uint32 pManeuver = NAV_MANEUVER_NONE;
    double                collapseNextDist = 0.f;
    uint32                collapseNextTime = 0;
    boolean                collapseNextStack = FALSE;
    uint32                nextManeuver = NAV_MANEUVER_NONE;
    uint32                nextManeuverMan = NAV_MANEUVER_NONE;
    enum manuever_type    type;
    double                cur_speed = 0.f;
    double                d = 0.f;
    NB_PositionRoutePositionInformation* routePos;
    NB_RoutePreferencesPrivate* routePrefs;
    NB_Place origin;

    if (!navigation || !navigation->navState || !navigation->navPublicState || !navigation->navState->navRoute )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigation->navState->stackCode[0] = '\0';

    routeNavReply = &navigation->navState->navRoute->dataReply;

    ResetNavigationPublicState(navigation->navPublicState);

    dataState = NB_ContextGetDataState(navigation->context);

    navigation->navPublicState->status =  navigation->status;

    (void)NB_RouteInformationGetSummaryInformation(navigation->navState->navRoute, NULL, NULL, &navigation->navPublicState->routeLength);

    NB_NavigationStateGetRoutePositionInformation(navigation->navState, &navigation->navPublicState->currentRoutePosition);
    routePos = &navigation->navPublicState->currentRoutePosition;

    routePrefs = &navigation->navState->navRoute->config.prefs;

    closetManeuver = routePos->closestManeuver;

    if (closetManeuver == NAV_MANEUVER_NONE || closetManeuver == NAV_MANEUVER_START)
    {
        maneuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, 0);
    }
    else
    {
        maneuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, closetManeuver);
    }

    if ( navigation->navState->initialRouteMatch && closetManeuver != NAV_MANEUVER_NONE && maneuver)
    {
        NB_RouteCollapseManeuvers(navigation->navState->navRoute, closetManeuver, &collapseDist, &collapseTime, &collapseStack,
            &baseManeuver, &pManeuver, &navigation->navPublicState->currentManeuver, &collapseNextDist, &collapseNextTime, &collapseNextStack, &nextManeuverMan, &nextManeuver, FALSE);

        pBaseManeuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, baseManeuver);
        pNextManeuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, nextManeuverMan);

        data_string_get_copy(dataState, &pBaseManeuver->current_roadinfo.primary, navigation->navPublicState->currentStreet, sizeof(navigation->navPublicState->currentStreet));
        data_string_get_copy(dataState, &pBaseManeuver->current_roadinfo.secondary, navigation->navPublicState->currentSecondary, sizeof(navigation->navPublicState->currentSecondary));

        NB_RouteInformationGetTurnInformation( navigation->navState->navRoute, navigation->navPublicState->currentManeuver,
            NULL, navigation->navPublicState->destinationStreet, sizeof(navigation->navPublicState->destinationStreet),
            navigation->navPublicState->destinationSecondary, sizeof(navigation->navPublicState->destinationSecondary),
            NULL, 0, NULL, TRUE);


        type = NB_RouteParametersGetCommandType(navigation->navState->navRoute->routeParameters, data_string_get(dataState, &maneuver->command));

        navigation->navPublicState->stackNman = NAV_MANEUVER_NONE;
        navigation->navPublicState->stackDistance = 0.0;

        cur_speed = MAX(location->horizontalVelocity, routePos->maneuverBaseSpeed);

        navigation->navPublicState->turnRemainDistance    =    routePos->maneuverDistanceRemaining + collapseDist;

        if (routeNavReply->route_settings.traffic_type & NB_TrafficType_UseSpeeds)
        {
            d = pBaseManeuver->distance - routePos->maneuverDistanceRemaining;
            data_nav_reply_get_traffic_info(dataState, routeNavReply,  routePrefs->navMinTmcSpeed, routePrefs->minNoDataDisableTrafficBar,
                                            closetManeuver, d, nextManeuverMan, TrafficSpeedAll, NULL, NULL, &navigation->navPublicState->turnRemainTime,
                                            (nav_speed_cb)NB_TrafficInformationGetSpeed, navigation->trafficInformation);
        }
        else
        {
            navigation->navPublicState->turnRemainTime = (uint32)(navigation->navPublicState->turnRemainDistance / get_nav_avg_speed(&navigation->navState->averageSpeed)) + collapseTime;
        }

        if (pNextManeuver != NULL)
        {
            if (collapseStack)
            {
                if (navigation->navPublicState->turnRemainDistance < NB_RouteInformationGetInstructionDistance(routePrefs, cur_speed, type, show_stack_dist))
                {
                    navigation->navPublicState->stackNman = nextManeuver;
                    navigation->navPublicState->stackDistance = pNextManeuver->distance + collapseNextDist;

                    nsl_strlcpy(navigation->navState->stackCode, data_string_get(dataState, &pNextManeuver->command), sizeof(navigation->navState->stackCode));
                }
            }
        }

        if ( (navigation->navState->recalcCount > 0 &&  (navigation->navPublicState->turnRemainDistance <= routePrefs->navHideDistThreshold)) || (navigation->navState->currentRoutePosition.match_type == rt_match_turn) )
        {
            navigation->navPublicState->pos = NB_NMP_Past;
        }
        else if (navigation->navPublicState->turnRemainDistance >= NB_RouteInformationGetInstructionDistance(routePrefs, cur_speed, type, continue_dist))
        {
            navigation->navPublicState->pos = NB_NMP_Continue;
        }
        else if (navigation->navPublicState->turnRemainDistance >= NB_RouteInformationGetInstructionDistance(routePrefs, cur_speed, type, prepare_dist))
        {
            navigation->navPublicState->pos = NB_NMP_ShowTurn;
        }
        else if (navigation->navPublicState->turnRemainDistance >= NB_RouteInformationGetInstructionDistance(routePrefs, cur_speed, type, instruct_dist))
        {
            navigation->navPublicState->pos = NB_NMP_Prepare;
        }
        else
        {
            navigation->navPublicState->pos = NB_NMP_Turn;
        }

        navigation->navPublicState->turnLatitude  = maneuver->point.lat;
        navigation->navPublicState->turnLongitude = maneuver->point.lon;

        navigation->navPublicState->tripRemainDistance =    navigation->navPublicState->turnRemainDistance + routePos->tripDistanceAfter - collapseDist;
        navigation->navPublicState->tripRemainTime     =    navigation->navPublicState->turnRemainTime + routePos->tripTimeAfter - collapseTime;

        navigation->navPublicState->heading = navigation->navState->lastHeading;
        navigation->navPublicState->viewHeading = navigation->navState->currentRoutePosition.segment_heading;
        navigation->navPublicState->speed = location->horizontalVelocity;
        navigation->navPublicState->latitude = location->latitude;
        navigation->navPublicState->longitude = location->longitude;
    }
    else if ((closetManeuver == NAV_MANEUVER_NONE || closetManeuver == NAV_MANEUVER_START) && maneuver)
    {
        data_string_get_copy(dataState, &maneuver->current_roadinfo.primary, navigation->navPublicState->destinationStreet, sizeof(navigation->navPublicState->destinationStreet));

        navigation->navPublicState->pos = NB_NMP_Turn;
        navigation->navPublicState->currentManeuver = NAV_MANEUVER_START;
        navigation->navPublicState->stackNman = NAV_MANEUVER_NONE;
        navigation->navPublicState->stackDistance = 0.0;

        navigation->navPublicState->turnLatitude            =    maneuver->point.lat;
        navigation->navPublicState->turnLongitude            =    maneuver->point.lon;

        navigation->navPublicState->turnRemainDistance    =    NB_SpatialGetLineOfSightDistance(location->latitude, location->longitude, maneuver->point.lat, maneuver->point.lon, NULL);
        navigation->navPublicState->turnRemainTime        =    (uint32) (navigation->navPublicState->turnRemainDistance / get_nav_avg_speed(&navigation->navState->averageSpeed));

        // the current maneuver is included by NB_RouteInformationTripRemainAfterManeuver
        NB_RouteInformationTripRemainAfterManeuver(routePrefs, dataState, &navigation->navState->navRoute->dataReply, NAV_MANEUVER_START, &navigation->navPublicState->tripRemainDistance, &navigation->navPublicState->tripRemainTime );

        if (IsHeadingValidForNavigation(navigation->navState, location))
            navigation->navPublicState->heading                    =    location->heading;
        else
            navigation->navPublicState->heading                    =    INVALID_HEADING;

        NB_RouteInformationGetOrigin(navigation->navState->navRoute, &origin);

        navigation->navPublicState->distToOrigin= NB_SpatialGetLineOfSightDistance(location->latitude, location->longitude, origin.location.latitude, origin.location.longitude, &navigation->navPublicState->viewHeading);
        navigation->navPublicState->speed = location->horizontalVelocity;
        navigation->navPublicState->latitude = location->latitude;
        navigation->navPublicState->longitude = location->longitude;
    }

    navigation->navPublicState->transportMode = navigation->navState->transportMode;
    navigation->navPublicState->snapMap = navigation->navState->snapMap;
    navigation->navPublicState->onRoute = navigation->navState->onRoute;
    navigation->navPublicState->announceType = navigation->announceType;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationHeartbeat(NB_Navigation* navigation)
{
    return NE_NOSUPPORT;
}

NB_DEF NB_Error
NB_NavigationRecalculate(NB_Navigation* navigation)
{
    NB_Error err = NE_OK;
    NB_GpsHistory* history = NULL;

    if (!navigation)
    {
        return NE_INVAL;
    }

    if (navigation->suspended)
    {
        return NE_UNEXPECTED;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    history = NB_ContextGetGpsHistory(navigation->context);

    err = NB_RecalcLimitReset(navigation->recalcLimit);
    err = err ? err : NB_NavigationDoRecalc(navigation, history, NULL, NULL, TRUE, NULL, 0);
    return err;
}

NB_DEF NB_Error
NB_NavigationRecalculateWithOptions(NB_Navigation* navigation, NB_RouteOptions* routeOptions, nb_boolean wantAlternateRoute)
{
    NB_Error err = NE_OK;
    NB_GpsHistory* history = NULL;

    if (!navigation)
    {
        return NE_INVAL;
    }

    if (navigation->suspended)
    {
        return NE_UNEXPECTED;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    history = NB_ContextGetGpsHistory(navigation->context);

    err = NB_RecalcLimitReset(navigation->recalcLimit);
    err = err ? err : NB_NavigationDoRecalc(navigation, history, routeOptions, NULL, wantAlternateRoute, NULL, 0);
    return err;
}

NB_DEF NB_Error
NB_NavigationRecalculateWithOptionsAndConfig(NB_Navigation* navigation, NB_RouteOptions* routeOptions, NB_RouteConfiguration* routeConfig, nb_boolean wantAlternateRoute)
{
    NB_Error err = NE_OK;
    NB_GpsHistory* history = NULL;

    if (!navigation)
    {
        return NE_INVAL;
    }

    if (navigation->suspended)
    {
        return NE_UNEXPECTED;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    history = NB_ContextGetGpsHistory(navigation->context);

    err = NB_RecalcLimitReset(navigation->recalcLimit);
    err = err ? err : NB_NavigationDoRecalc(navigation, history, routeOptions, routeConfig, wantAlternateRoute, NULL, 0);
    return err;
}

NB_DEC NB_Error NB_NavigationRecalculateWithWayPoint(NB_Navigation* navigation, NB_WayPoint* pWaypoint, uint32 wayPointSize)
{
    NB_Error err = NE_OK;
    NB_GpsHistory* history = NULL;
    
    if (!navigation)
    {
        return NE_INVAL;
    }
    
    if (navigation->suspended)
    {
        return NE_UNEXPECTED;
    }
    
    NB_ASSERT_VALID_THREAD(navigation->context);
    
    history = NB_ContextGetGpsHistory(navigation->context);
    
    err = NB_RecalcLimitReset(navigation->recalcLimit);
    err = err ? err : NB_NavigationDoRecalc(navigation, history, NULL, NULL, FALSE, pWaypoint, wayPointSize);
    return err;
}

NB_DEF NB_Error
NB_NavigationGetInformation(NB_Navigation* navigation, NB_RouteInformation** route, NB_TrafficInformation** traffic, NB_CameraInformation** camera, NB_PointsOfInterestInformation** pointsOfInterest)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (route)
    {
        if (navigation->navState)
        {
            *route = navigation->navState->navRoute;
        }
        else
        {
            *route = NULL;
        }
    }

    if (traffic)
    {
        *traffic = navigation->trafficInformation;
    }

    if (camera)
    {
        *camera = navigation->cameraInformation;
    }

    if (pointsOfInterest)
    {
        *pointsOfInterest = navigation->poiInformation;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetStatus(NB_Navigation* navigation, NB_NavigateStatus* status)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    *status = navigation->status;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetSessionId(NB_Navigation* navigation, uint32* navSessionId)
{
    if ((!navigation) || (!navSessionId))
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    *navSessionId = navigation->navSessionId;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetAnnounceType(NB_Navigation* navigation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units)
{
    if (!navigation || type == NB_NAT_None)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigation->announceType = type;
    navigation->announceUnits = units;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetCallbacks(NB_Navigation* navigation, NB_NavigationCallbacks* callbacks)
{
    if (!navigation || ! callbacks)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

#define SET_CALLBACK(X)       \
    if (callbacks->X) \
        navigation->callbacks.X = callbacks->X

    SET_CALLBACK(statusChangeCallback);
    SET_CALLBACK(routeDownloadCallback);
    SET_CALLBACK(routeCompleteDownloadCallback);
    SET_CALLBACK(guidanceMessageCallback);
    SET_CALLBACK(trafficNotificationCallback);
    SET_CALLBACK(pointsOfInterestNotificationCallback);
    SET_CALLBACK(cameraNotificationCallback);
    SET_CALLBACK(enhancedContentNotificationCallback);
    SET_CALLBACK(speedLimitCallback);
    SET_CALLBACK(visualLaneGuidanceCallback);
    SET_CALLBACK(getAnnounceSnippetLengthCallback);
    SET_CALLBACK(routeRequestedCallback);
    SET_CALLBACK(maneuverPositionChangeCallback);
    SET_CALLBACK(modeChangeCallback);
    SET_CALLBACK(specialRegionCallback);
    SET_CALLBACK(userData);

#undef SET_CALLBACK

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetManualGuidanceMessageEx(NB_Navigation* navigation, uint32 index, nb_boolean lookahead, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, nb_boolean buttonPressed, NB_GuidanceMessage** message)
{
    NB_Error result = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};
    NB_GuidanceProccessMessagesParameters parameters = {0};

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (navigation->guidanceInformation == NULL)
    {
        // need the guidance information for this to work
        *message = NULL;
        return NE_UNEXPECTED;
    }

    if (navigation->navState == NULL)
    {
        // need navState for this to work
        *message = NULL;
        return NE_UNEXPECTED;
    }

    result = NB_NavigationGetRoutePositionInformation(navigation, &routePositionInformation);
    if (routePositionInformation.projLat == INVALID_LATITUDE || routePositionInformation.projLon == INVALID_LONGITUDE)
    {
        routePositionInformation.projLat = navigation->navState->lastLatitude;
        routePositionInformation.projLon = navigation->navState->lastLongitude;
    }

    parameters.guidanceInformation = navigation->guidanceInformation;
    parameters.trafficInformation = navigation->trafficInformation;
    parameters.routeInformation = navigation->navState->navRoute;
    parameters.type = navigation->announceType;
    parameters.units = navigation->announceUnits;
    parameters.getAnnounceSnippetLengthCallback = navigation->callbacks.getAnnounceSnippetLengthCallback;
    parameters.getAnnounceSnippetLengthCallbackData = navigation->callbacks.userData;

    if (!lookahead)
    {
        result = result ? result : NB_GuidanceGetTypeMessageByCurrentRoutePosition(navigation->guidanceState, navigation->navState, &parameters,
                                                                                   NB_NAS_Button, &dataSourceOptions.maneuverPos);
    }
    else
    {
        dataSourceOptions.maneuverPos = NB_NMP_Turn;
    }

    dataSourceOptions.route = navigation->navState->navRoute;
    dataSourceOptions.navState = navigation->navState;
    dataSourceOptions.collapse = lookahead ? FALSE : TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.trafficInfo = navigation->trafficInformation;
    dataSourceOptions.guidanceState = navigation->guidanceState;
    dataSourceOptions.buttonPressed = buttonPressed;
    dataSourceOptions.isLookahead = lookahead;

    result = result ? result : NB_GuidanceInformationGetTurnAnnouncement(navigation->guidanceInformation,
                    NB_RouteInformationDataSource, &dataSourceOptions,
                                                                         index, dataSourceOptions.maneuverPos,
                                                                         lookahead ? NB_NAS_Lookahead : NB_NAS_Button, type, units, message, NULL);

    if (result == NE_OK && *message != NULL)
    {
        NB_QaLogGuidanceMessageTrigger(navigation->context, lookahead ? NB_NAS_Lookahead : NB_NAS_Button, NB_GMT_Guidance, -1, -1, -1, -1);
        // just write QA Log
        PlayGuidanceMessage(navigation, *message, FALSE);
    }

    return result;
}

NB_DEF NB_Error
NB_NavigationGetManualGuidanceMessage(NB_Navigation* navigation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, nb_boolean buttonPressed, NB_GuidanceMessage** message)
{
    NB_Error result = NE_OK;
    NB_PositionRoutePositionInformation routePositionInformation = {0};

    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    if (navigation->guidanceInformation == NULL)
    {
        // need the guidance information for this to work
        *message = NULL;
        return NE_UNEXPECTED;
    }
    if (navigation->navState == NULL)
    {
        // need navState for this to work
        *message = NULL;
        return NE_UNEXPECTED;
    }

    result = NB_NavigationGetRoutePositionInformation(navigation, &routePositionInformation);

    if (result != NE_OK)
    {
        return result;
    }

    return NB_NavigationGetManualGuidanceMessageEx(navigation, routePositionInformation.closestManeuver, FALSE, type, units, buttonPressed, message);
}

NB_DEF nb_boolean
NB_NavigationIsOnRoute(NB_Navigation* navigation)
{
    NB_NavigationState* navigationState = NULL;

    if (!navigation)
    {
        return FALSE;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigationState = navigation->navState;
    if (!navigationState)
    {
        return FALSE;
    }

    return navigationState->onRoute;
}

NB_DEF NB_Error
NB_NavigationGetTrafficState(NB_Navigation* navigation, NB_TrafficState** trafficState)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    *trafficState = navigation->trafficState;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetCameraState(NB_Navigation* navigation, NB_CameraState** cameraState)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

   *cameraState = navigation->cameraState;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetGuidanceState(NB_Navigation* navigation, NB_GuidanceState** guidanceState)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

   *guidanceState = navigation->guidanceState;

    return NE_OK;
}

#ifdef NB_ECM_ENABLED
NB_DEF NB_Error
NB_NavigationCreateEnhancedVectorMapState(NB_Context* context, NB_Navigation* navigation, NB_EnhancedVectorMapConfiguration* enhancedVectorMapConfig, NB_EnhancedVectorMapState** state)
{
    NB_ASSERT_VALID_THREAD(navigation->context);
    if (!navigation || !enhancedVectorMapConfig)
    {
        return NE_INVAL;
    }

    if (NB_EnhancedVectorMapStateCreate(context, navigation, enhancedVectorMapConfig, &navigation->enhancedVectorMapState) != NE_OK)
    {
        return NE_NOMEM;
    }

    *state = navigation->enhancedVectorMapState;

    return NE_OK;
}
#endif

NB_DEF NB_Error
NB_NavigationGetManeuverProgress(NB_Navigation* navigation, int* maneuverProgress)
{
    NB_NavigationState* navigationState = NULL;

    if (!navigation || !maneuverProgress)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigationState = navigation->navState;
    if (!navigationState)
    {
        *maneuverProgress = 0;
        return NE_UNEXPECTED;
    }

    *maneuverProgress = navigationState->manueverProgress;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetSegmentProgress(NB_Navigation* navigation, int* segmentProgress)
{
    NB_NavigationState* navigationState = NULL;

    if (!navigation || !segmentProgress)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigationState = navigation->navState;
    if (!navigationState)
    {
        *segmentProgress = 0;
        return NE_UNEXPECTED;
    }

    *segmentProgress = navigationState->segmentProgress;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetRoutePositionInformation(NB_Navigation* navigation, NB_PositionRoutePositionInformation* information)
{
    NB_NavigationState* navigationState = NULL;

    if (!navigation || !information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigationState = navigation->navState;
    if (!navigationState)
    {
        nsl_memset(information, 0, sizeof(*information));
        return NE_UNEXPECTED;
    }

    return NB_NavigationStateGetRoutePositionInformation(navigationState, information);
}

NB_DEF NB_Error
NB_NavigationGetCurrentManeuverPosition(NB_Navigation* navigation, NB_NavigateManeuverPos* position)
{
    NB_NavigationState* navigationState = NULL;

    if (!navigation || !position)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    navigationState = navigation->navState;
    if (!navigationState)
    {
        *position = NB_NMP_Invalid;
        return NE_UNEXPECTED;
    }

    *position = navigationState->maneuverPos;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetPublicState(NB_Navigation* navigation, NB_NavigationPublicState* publicState)
{
    if (!navigation || !publicState)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    *publicState = *navigation->navPublicState;

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationDoTrafficRequest(NB_Navigation* navigation, const char* trafficRecordIdentifier)
{
    NB_Error error = NE_OK;
    NB_TrafficParameters*   trafficParameters = 0;
    char* voice=NULL;
    uint32 progression = 0;

    if (!navigation || !navigation->navState)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    NB_QaLogAppState(navigation->context, "Traffic SMS");
    NB_QaLogTrafficId(navigation->context, trafficRecordIdentifier);

    if (!NB_TrafficInformationCompareTrafficRecordId(navigation->trafficInformation, trafficRecordIdentifier))
    {
        NB_QaLogAppState(navigation->context, "Traffic SMS not match");
        return NE_NOENT;
    }

    NB_GuidanceInformationGetVoiceStyle(navigation->guidanceInformation, &voice);
    progression = (uint32) (navigation->navPublicState->routeLength - navigation->navPublicState->tripRemainDistance);
    error = error ? error : NB_TrafficParametersCreateFromTRI(navigation->context, navigation->navState->navRoute, progression, voice, trafficRecordIdentifier, &trafficParameters);
    error = error ? error : NB_NavigationDoTrafficNotify(navigation, trafficParameters);
    error = error ? error : NB_TrafficParametersDestroy(trafficParameters);

    if(voice)
    {
        nsl_free(voice);
    }
    return error;
}

NB_DEF NB_Error
NB_NavigationGetNextTrafficEvent(NB_Navigation* navigation, NB_TrafficEvent* trafficEvent)
{
    if (!navigation || !trafficEvent || !navigation->trafficState)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    return NB_TrafficProcessorGetNextTrafficEvent(navigation->trafficState, trafficEvent);
}

NB_DEF NB_Error
NB_NavigationGetManeuverTrafficSummary(NB_Navigation* navigation, uint32 startManeuverIndex, double distanceOffset, uint32 endManeuverIndex, NB_TrafficEvent* firstTrafficEvent, double* meter, uint32* time, uint32* delay)
{
    double meterValue = 0.0;
    uint32 timeValue = 0;
    uint32 delayValue = 0;

    if (!navigation || !firstTrafficEvent || !navigation->trafficInformation )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    (void)NB_TrafficInformationGetSummary(navigation->trafficInformation, navigation->navState->navRoute,
                                            startManeuverIndex, distanceOffset, endManeuverIndex,
                                            &meterValue, NULL, &timeValue, &delayValue);
    if (meter)
    {
        *meter = meterValue;
    }
    if (time)
    {
        *time = timeValue;
    }
    if (delay)
    {
        *delay = delayValue;
    }

    return NB_TrafficInformationGetFirstTrafficEventInManeuvers(navigation->trafficInformation, startManeuverIndex, endManeuverIndex, firstTrafficEvent);
}

NB_DEF NB_Error
NB_NavigationGetUpcomingTrafficEvents(NB_Navigation* navigation, uint32* numberOfTrafficEvents, NB_TrafficEvent** trafficEvents)
{
    if (!navigation || !navigation->trafficInformation || !numberOfTrafficEvents || !trafficEvents)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(navigation->context);

    return NB_TrafficInformationGetUpcomingTrafficEvents(navigation->trafficInformation, numberOfTrafficEvents, trafficEvents);
}

NB_DEF NB_Error
NB_NavigationFreeTrafficEvents(NB_TrafficEvent* trafficEvents)
{
    return NB_TrafficInformationFreeTrafficEvents(trafficEvents);
}

NB_DEF NB_Error
NB_NavigationSetEnhancedContentMapConfiguration(NB_Navigation* navigation, const NB_EnhancedContentMapConfiguration* configuration)
{
    if (!navigation || !configuration)
    {
        return NE_INVAL;
    }

    if (navigation->enhancedContentState)
    {
        return NB_EnhancedContentProcessorSetConfiguration(navigation->enhancedContentState, configuration);
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationResetEnhancedContentMapState(NB_Navigation* navigation)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    if (navigation->enhancedContentState)
    {
        return NB_EnhancedContentProcessorStateReset(navigation->enhancedContentState);
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetMapOrientation(NB_Navigation* navigation, NB_MapOrientation orientation)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    if (navigation->enhancedContentState)
    {
        return NB_EnhancedContentProcessorSetMapOrientation(navigation->enhancedContentState, orientation);
    }

    return NE_OK;
}

NB_DEF
NB_Error NB_NavigationSetTrafficPollingInterval(NB_Navigation* navigation, uint32 pollingIntervalMinutes)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    navigation->trafficPollingIntervalMinutes = pollingIntervalMinutes;

    // If initial traffic notify request complete, cancel pending timers
    if (navigation->initialNotifyComplete)
    {
        (void)NB_NavigationCancelTrafficNotify(navigation);

        // If a non-zero polling interval is specified, schedule the initial request.
        // Polled requests at the specified interval will be scheduled when each pending
        // traffic update response is received.
        if (navigation->trafficPollingIntervalMinutes)
        {
            NB_ScheduleTrafficNotify(navigation, navigation->configuration.trafficNotifyDelay, FALSE);
        }
    }

    return NE_OK;
}

nb_boolean
IsHeadingValidForNavigation(NB_NavigationState* navState, NB_GpsLocation* location)
{
    return heading_good(navState->gpsFixMinimumSpeed, location);
}

static nb_boolean
IsNavigatingStatus(NB_NavigateStatus status)
{
    return (nb_boolean)(
        status == NB_NS_Navigating ||
        status == NB_NS_NavigatingArriving ||
        status == NB_NS_NavigatingConfirmRecalc ||
        status == NB_NS_NavigatingFerry ||
        status == NB_NS_NavigatingStartup
    );
}

static NB_Error
SetStatus(NB_Navigation* navigation, NB_NavigateStatus status)
{
    if (navigation->status != status || navigation->navPublicState->turnRemainDistance <= 0.0)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 currentStreet = %s destinationStreet = %s",
                navigation->navPublicState->currentStreet, navigation->navPublicState->destinationStreet);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 maneuverPos = %d turnCode = %s, stackCode = %s",
                navigation->navState->maneuverPos, navigation->navState->turnCode, navigation->navState->stackCode);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 turnLatitude = %f, turnLongitude = %f",
                navigation->navPublicState->turnLatitude, navigation->navPublicState->turnLongitude);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 turnRemainDistance = %f, turnRemainTime = %f",
                navigation->navPublicState->turnRemainDistance, navigation->navPublicState->turnRemainTime);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 remainingRouteDist = %f, remainingRouteTime = %f",
                navigation->navState->remainingRouteDist, navigation->navState->remainingRouteTime);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 lastHeading = %f, currentSpeed = %f",
                navigation->navState->lastHeading, navigation->navState->currentSpeed);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 currentRoutePosition.proj_lat = %f, currentRoutePosition.proj_lon = %f",
                navigation->navState->currentRoutePosition.proj_lat, navigation->navState->currentRoutePosition.proj_lon);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_STATE4 routeError = %d status = %d",
                navigation->navState->routeError, navigation->navPublicState->status);
    }

    navigation->status = status;
    navigation->navPublicState->status = navigation->status;

    NB_QaLogNavigationState(navigation->context, navigation->navState, navigation->navPublicState);

    if (navigation->callbacks.statusChangeCallback)
    {
        navigation->callbacks.statusChangeCallback(navigation, navigation->status, navigation->callbacks.userData);
    }

    return NE_OK;
}

static NB_Error
SetPoiSearchTimerTimeOutStatus(NB_Navigation* navigation, nb_boolean status)
{
    if( !navigation )
    {
        return NE_BADDATA;
    }

    navigation->poiSearchTimerExpired = status;
    return NE_OK;
}

static NB_Error
GetPoiSearchTimerTimeOutStatus(const NB_Navigation* navigation, nb_boolean* poiTimerStatus)
{
    if( !navigation || !poiTimerStatus )
    {
        return NE_BADDATA;
    }

    *poiTimerStatus = navigation->poiSearchTimerExpired;
    return NE_OK;
}

static NB_Error
PlayGuidanceMessage(NB_Navigation* navigation, NB_GuidanceMessage* message, nb_boolean invokeClientCallback)
{
    NB_QaLogGuidanceMessageData* userData = NULL;

    if (!message)
    {
        return  NE_INVAL;
    }

    userData = nsl_malloc(sizeof(NB_QaLogGuidanceMessageData));

    if (userData)
    {
        double calculatedPlaybackTime = 0.0;
        NB_GuidanceProccessMessagesParameters parameters = {0};
        NB_NavigateAnnouncementSource source = NB_GuidanceMessageGetSource(message);

        parameters.guidanceInformation = navigation->guidanceInformation;
        parameters.trafficInformation = navigation->trafficInformation;
        parameters.routeInformation = navigation->navState->navRoute;
        parameters.type = navigation->announceType;
        parameters.units = navigation->announceUnits;
        parameters.getAnnounceSnippetLengthCallback = navigation->callbacks.getAnnounceSnippetLengthCallback;
        parameters.getAnnounceSnippetLengthCallbackData = navigation->callbacks.userData;

        NB_GuidanceCalculatePlaybackTime(message, &parameters, &calculatedPlaybackTime);
        NB_GuidanceMessageSetPlaybackTime(message, calculatedPlaybackTime);
        userData->navigation = navigation;
        userData->actualStartTime = PAL_ClockGetGPSTime();
        if (source != NB_NAS_Lookahead && source != NB_NAS_Button)
        {
            NB_NavigationStateGetInstructionParameters(navigation->navState, &userData->actualStartDistance, NULL, NULL, NULL, NULL);
            userData->currentManeuver = navigation->navState->currentRoutePosition.closest_maneuver;
        }
        NB_GuidanceMessageSetQaLogPlayedCallback(message, NB_GuidanceMessageQaLogPlayedCallback, userData);

        NB_QaLogGuidanceMessageRetrieved(navigation->context, message);
        NB_QaLogGuidanceMessageRetrieved2(navigation->context, message, userData->actualStartDistance, navigation->navState->currentSpeed,
                                          calculatedPlaybackTime, navigation->navState->currentSpeed * calculatedPlaybackTime);
    }
    if (invokeClientCallback)
    {
        if (navigation->callbacks.guidanceMessageCallback != NULL)
        {
            (navigation->callbacks.guidanceMessageCallback)(navigation, message, navigation->callbacks.userData);
        }
        else
        {
            (void)NB_GuidanceMessageDestroy(message);
        }
    }
    return NE_OK;
}


static void
NB_GuidanceMessageQaLogPlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    NB_QaLogGuidanceMessageData* qaLogData = userData;

    if (qaLogData)
    {
        if (qaLogData->navigation)
        {
            double maneuverRemaining = 0.0;
            NB_NavigateAnnouncementSource source = NB_GuidanceMessageGetSource(message);

            if (source != NB_NAS_Lookahead && source != NB_NAS_Button &&
                qaLogData->currentManeuver == qaLogData->navigation->navState->currentRoutePosition.closest_maneuver)
            {
                NB_NavigationStateGetInstructionParameters(qaLogData->navigation->navState, &maneuverRemaining, NULL, NULL, NULL, NULL);
            }
            NB_QaLogGuidanceMessagePlayed(qaLogData->navigation->context, message);
            NB_QaLogGuidanceMessagePlayed2(qaLogData->navigation->context, message, maneuverRemaining,
                                           PAL_ClockGetGPSTime() - qaLogData->actualStartTime, qaLogData->actualStartDistance - maneuverRemaining);
        }
        nsl_free(qaLogData);
    }
}
static NB_Navigation*
AllocateNavigation(NB_Context* context)
{
    NB_Navigation* pThis = 0;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    return pThis;
}

static NB_NavigationState*
AllocateNavigationState(NB_Navigation* navigation, NB_RoutePrivateConfiguration* routePrivateConfig, NB_NavigateTransportMode transportMode)
{
    NB_NavigationState* pThis = 0;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = navigation->context;
    pThis->navRoute = NULL;

    clear_route_position(&pThis->currentRoutePosition);

    pThis->lastHeading = INVALID_LATLON;
    pThis->lastHeadingUTC = 0;

    pThis->manueverProgress = 0;
    pThis->segmentProgress = 0;

    pThis->transportMode = NB_NTM_Invalid;

    create_nav_avg_speed(&pThis->averageSpeed, routePrivateConfig->prefs.navAvgSpeedA, routePrivateConfig->prefs.navNumPtsForValid,
        routePrivateConfig->prefs.navDefaultManeuverSpeed, routePrivateConfig->prefs.navAvgSpeedThreshold,
        routePrivateConfig->prefs.navSvgSpeedMinUpdateBelow, routePrivateConfig->prefs.navAvgSpeedMinUpdateAbove);

    pThis->initialRouteMatch = FALSE;
    pThis->initialRouteMatchesNeeded = routePrivateConfig->prefs.navStartupMinMatches;

    pThis->offRouteCount = 0;
    pThis->wrongWayCount = 0;

    pThis->pastArrivingThreshold = FALSE;
    pThis->pastArrivedThreshold = FALSE;

    pThis->firstFixProcessed = FALSE;
    pThis->newRoute = TRUE;
    pThis->positionUpdated = FALSE;
    pThis->applyingNewRoute = FALSE;
    pThis->applyingFullRoute = FALSE;

    pThis->error = NE_OK;
    pThis->routeError = NB_NRE_None;

    pThis->turnCode[0] = '\0';
    pThis->stackCode[0] = '\0';

    pThis->trafficNotifyRetries = 0;
    pThis->completeRouteDownloadRetries = 0;

    pThis->recalcCount = 0;
    pThis->snapMap = FALSE;

    pThis->currentMessage = NULL;

    if (transportMode == NB_NTM_Pedestrian)
    {
        pThis->gpsFixMinimumSpeed = routePrivateConfig->prefs.navMinSpeedValidHeadingPed;
    }
    else
    {
        pThis->gpsFixMinimumSpeed = (navigation->configuration.gpsFixMinimumSpeed > 0.0) ? navigation->configuration.gpsFixMinimumSpeed : routePrivateConfig->prefs.navMinSpeedValidHeadingCar;
    }

    return pThis;
}

static NB_NavigationPublicState*
AllocateNavigationPublicState(NB_Context* context)
{
    NB_NavigationPublicState* pThis = 0;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    ResetNavigationPublicState(pThis);

    return pThis;
}

static void
ResetNavigationPublicState(NB_NavigationPublicState* publicState)
{
    publicState->status = NB_NS_Invalid;
    publicState->pos = NB_NMP_Invalid;
    publicState->turnLatitude = INVALID_LATLON;
    publicState->turnLongitude = INVALID_LATLON;
    publicState->turnRemainDistance = 0.f;
    publicState->turnRemainTime = 0;
    publicState->tripRemainDistance = 0.f;
    publicState->tripRemainTime = 0;
    publicState->currentManeuver = NAV_MANEUVER_NONE;
    publicState->stackNman = NAV_MANEUVER_NONE;
    publicState->stackDistance = 0.f;
    publicState->heading = 0.f;
    publicState->speed = 0.f;
    publicState->latitude = INVALID_LATLON;
    publicState->longitude = INVALID_LATLON;
    publicState->onRoute = 0;
    publicState->routeLength = 0.f;
    publicState->viewHeading = INVALID_HEADING;
    publicState->distToOrigin = 0.f;
    publicState->snapMap = FALSE;
}

static NB_Error
NB_NavigationStateNewRoute(NB_NavigationState* navigationState)
{
    NB_RoutePrivateConfiguration* routePrivateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    clear_route_position(&navigationState->currentRoutePosition);

    navigationState->lastHeading = INVALID_LATLON;
    navigationState->lastHeadingUTC = 0;
    navigationState->lastGpsTime = 0;

    navigationState->manueverProgress = 0;
    navigationState->segmentProgress = 0;

    NB_RouteInformationGetTransportMode(navigationState->navRoute, &navigationState->transportMode);

    navigationState->initialRouteMatch = FALSE;
    navigationState->initialRouteMatchesNeeded = routePrivateConfig->prefs.navStartupMinMatches;

    navigationState->offRouteCount = 0;
    navigationState->wrongWayCount = 0;

    navigationState->pastArrivingThreshold = FALSE;
    navigationState->pastArrivedThreshold = FALSE;

    navigationState->firstFixProcessed = FALSE;
    navigationState->newRoute = TRUE;
    navigationState->positionUpdated = FALSE;

    navigationState->error = NE_OK;
    navigationState->routeError = NB_NRE_None;

    return NE_OK;
}

static NB_Error
NB_NavigationStateDestroy(NB_NavigationState* navigationState)
{
    if (!navigationState)
    {
        return NE_INVAL;
    }

    clear_route_position(&navigationState->currentRoutePosition);

    free_nav_avg_speed(&navigationState->averageSpeed);

    navigationState->navRoute = NULL;

    nsl_free(navigationState);

    return NE_OK;
}

static NB_Error
NB_NavigationPublicStateDestroy(NB_NavigationPublicState* publicState)
{
    if (!publicState)
    {
        return NE_INVAL;
    }

    nsl_free(publicState);

    return NE_OK;
}

static NB_Error
NB_NavigationCheckForRecalc(NB_Navigation* navigation, const NB_GpsLocation* location, nb_boolean* recalcRequired, enum nav_update_reason* recalcReason)
{
    enum nav_update_reason reason = nav_update_none;
    NB_NavigationState* state = NULL;
    NB_RoutePrivateConfiguration* routePrivateConfig = NULL;

    if (navigation == NULL || navigation->navState == NULL || navigation->navState->navRoute == NULL)
    {
        return NE_INVAL;
    }

    if ( navigation->navState->transportMode == NB_NTM_Pedestrian )
    {
        if (recalcRequired)
            *recalcRequired = FALSE;
        return NE_OK;
    }

    state = navigation->navState;
    routePrivateConfig = NB_RouteInformationGetPrivateConfiguration(navigation->navState->navRoute);

    /* Only recalc if we have made an initial match to the route */
    if (state->initialRouteMatch)
    {
        if (!state->pastArrivingThreshold)
        {
            if (state->onRoute)
            {
                /* Check for too many wrong way matches */
                if (state->onRoute && state->wrongWayCount >= routePrivateConfig->prefs.navWWIgnore)
                {
                    reason = nav_update_wrong_way;
                }
                /* Check if we are close to the end of the route window -- need to do a soft-recalc to update */
                else if (state->currentRoutePosition.near_window_end && !state->newRoute &&
                    (state->maneuverPos == NB_NMP_Continue || state->maneuverPos == NB_NMP_ShowTurn))
                {
                    reason = nav_update_reload;
                }
            }
            else
            {
                if (state->offRouteCount >= navigation->configuration.offRouteIgnoreCount || state->newRoute)
                {
                    reason = nav_update_off_route;
                }
            }
        }
        /* Arriving threshold has been passed -- recalc only if out of arriving region */
        else
        {
            if ((state->offRouteCount >= navigation->configuration.offRouteIgnoreCount || state->newRoute) &&
                !InArrivingRegion(navigation, location))
            {
                reason = nav_update_off_route;
            }
        }
    }
    /* Initial match has not been made to the route */
    else
    {
        if (NB_PositionProcessorCheckFixQuality(navigation->navState, location) &&
            !InStartupRegion(navigation, location))
        {
            reason = nav_update_off_route;
        }
    }

    if (reason != nav_update_none)
    {
        if (recalcRequired)
        {
            *recalcRequired = TRUE;
        }

        NB_QaLogNavigationSessionRecalc(navigation->context, navigation->navSessionId, (uint8)reason);
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "NAV_SESSION_RECALC sessionId = %d reason = %d",
                navigation->navSessionId, reason);
    }

    return NE_OK;
}

static NB_Error
NB_NavigationPlayMessage(NB_Navigation* navigation)
{
    NB_Error error = NE_OK;
    NB_NavigationState* state = navigation->navState;
    NB_GuidanceMessage* message = NULL;
    NB_GuidanceMessageType playingMessageType = NB_GMT_None;
    NB_NavigateAnnouncementSource playingMessageSource = NB_NAS_Automatic;
    NB_GuidanceProccessMessagesParameters parameters = {0};

    parameters.guidanceInformation = navigation->guidanceInformation;
    parameters.trafficInformation = navigation->trafficInformation;
    parameters.routeInformation = navigation->navState->navRoute;
    parameters.type = navigation->announceType;
    parameters.units = navigation->announceUnits;
    parameters.getAnnounceSnippetLengthCallback = navigation->callbacks.getAnnounceSnippetLengthCallback;
    parameters.getAnnounceSnippetLengthCallbackData = navigation->callbacks.userData;

    //ignore any message when in fastNavStartupMode

    if (navigation->fastNavStartupMode)
    {
        return error;
    }

    playingMessageType = NB_GuidanceMessageGetType(state->currentMessage);
    playingMessageSource = NB_GuidanceMessageGetSource(state->currentMessage);

     // maneuver guidance shall NOT be supported in static routing except for arrival notification
    if ( navigation->guidanceState &&
         (navigation->navMode != NB_NM_StaticRoute || navigation->navState->pastArrivingThreshold || navigation->navState->pastArrivedThreshold) )
    {
        if (navigation->navState->positionUpdated || navigation->navState->applyingNewRoute || navigation->guidanceState->turnInfo.state != gmsPlayed)
        {
            if (navigation->guidanceState)
            {
                if (playingMessageType != NB_GMT_Guidance || (playingMessageType == NB_GMT_Guidance && playingMessageSource != NB_NAS_Automatic))
                {
                    error = error ? error : NB_GuidanceProcessorUpdate(navigation->guidanceState, navigation->navState, &parameters);
                }
            }
        }
    }

    //Guidance Messages have the highest priority, so allow them to interrupt other messages
    if (NB_GuidanceIsMessagePending(navigation->guidanceState))
    {
        if((playingMessageType != NB_GMT_Guidance ||
            (playingMessageType == NB_GMT_Guidance && playingMessageSource != NB_NAS_Automatic)))
        {
            error = NB_GuidanceGetPendingMessage(navigation->guidanceState, state, navigation->guidanceInformation, navigation->trafficInformation, navigation->announceType, navigation->announceUnits, &message);
        }
    }
    else
    {
        if (NB_SpeedLimitIsAnnouncementPending(navigation->speedlimitState) &&
            (playingMessageType == NB_GMT_None || playingMessageType > NB_GMT_SchoolZoneWarning))
        {
            error = NB_SpeedLimitGetPendingMessage(navigation->speedlimitState, state, navigation->guidanceInformation, navigation->speedLimit, navigation->announceUnits, &message);
        }
        else if (NB_CameraProcessorIsAnnouncementPending(navigation->cameraState) &&
                 (playingMessageType == NB_GMT_None || playingMessageType > NB_GMT_Camera))
        {
            error = NB_CameraGetPendingMessage(navigation->cameraState, state, navigation->guidanceInformation, navigation->cameraInformation, navigation->announceType, navigation->announceUnits, &message);
        }
        else if (NB_TrafficProcessorIsAnnouncementPending(navigation->trafficState) &&
                 (playingMessageType == NB_GMT_None || playingMessageType > NB_GMT_Traffic))
        {
            error = NB_TrafficGetPendingMessage(navigation->trafficState, state, navigation->guidanceInformation, navigation->trafficInformation, navigation->announceType, navigation->announceUnits, &message);
        }

        if (message != NULL)
        {
            if (!NB_GuidanceIsEnoughGapForTheMessage(navigation->guidanceState, message, navigation->navState, &parameters))
            {
                NB_GuidanceMessageDestroy(message);
                message = NULL;
            }
        }
    }
    double remainDistance  = 0;
    NB_NavigationStateGetInstructionParameters(navigation->navState, &remainDistance,
                                               NULL, NULL, NULL, NULL);
    double speed = MAX(navigation->navState->currentSpeed, MIN_SPEED);
    double checkDistance = remainDistance - speed*navigation->navState->navRoute->parameterConfiguration.announcementHeadsUpInterval/1000.0;
    NB_PositionRoutePositionInformation pos;
    NB_NavigationGetRoutePositionInformation(navigation, &pos);
    if (message != NULL)
    {
        state->currentMessage = message;
        error = error ? error : PlayGuidanceMessage(navigation, message, TRUE);
    }
    else
    {
        //continue
        if(checkDistance < 0)
        {
            if (!(NB_RouteParametersGetCommandType(navigation->navState->navRoute->routeParameters,
                                                   navigation->navState->turnCode) == maneuver_type_dest) &&
                !navigation->guidanceState->continueInfo.headupPlayed)
            {
                navigation->guidanceState->continueInfo.headupPlayed = TRUE;
                NB_GuidanceHeadsUp(navigation);
            }
        }
        if(navigation->guidanceState->continueInfo.allowedPlay && !navigation->guidanceState->continueInfo.headupPlayed)
        {
            if(checkDistance <= navigation->guidanceState->continueInfo.allowedStartDistance && remainDistance > navigation->guidanceState->continueInfo.allowedStartDistance)
            {
                navigation->guidanceState->continueInfo.headupPlayed = TRUE;
                NB_GuidanceHeadsUp(navigation);
            }
        }
        if(navigation->guidanceState->prepareInfo.allowedPlay && !navigation->guidanceState->prepareInfo.headupPlayed)
        {
            if(checkDistance <= navigation->guidanceState->prepareInfo.allowedStartDistance && remainDistance > navigation->guidanceState->prepareInfo.allowedStartDistance)
            {
                navigation->guidanceState->prepareInfo.headupPlayed = TRUE;
                NB_GuidanceHeadsUp(navigation);
            }
        }
        if(navigation->guidanceState->turnInfo.allowedPlay && !navigation->guidanceState->turnInfo.headupPlayed)
        {
            if(checkDistance <= navigation->guidanceState->turnInfo.allowedStartDistance && remainDistance > navigation->guidanceState->turnInfo.allowedStartDistance)
            {
                navigation->guidanceState->turnInfo.headupPlayed = TRUE;
                NB_GuidanceHeadsUp(navigation);
            }
        }
    }

    return error;
}

static void
NB_NavigateRouteDownloadSucceeded(NB_Navigation* navigation, NB_RouteHandler* handler)
{
    NB_NavigateStatus status;
    NB_Error error = NB_NavigationGetStatus(navigation, &status);

    NB_RouteInformation*    route = NULL;
    NB_TrafficInformation*  traffic = NULL;
    NB_CameraInformation*   camera = NULL;

    NB_GpsLocation  originFix;

    gpsfix_clear(&originFix);

    if (IsNavigatingStatus(status))
    {
        if (NB_RouteInformationIsRouteComplete(navigation->navState->navRoute))
        {
            NB_QaLogAppState(navigation->context, "route download: already complete");
            return;
        }
    }
    else if (status != NB_NS_InitialRoute && status != NB_NS_UpdatingRoute)
    {
        NB_QaLogAppState(navigation->context, "route download: invalid status");
        return;
    }

    error =  error ? error : NB_RouteHandlerGetRouteInformation(handler, &route);
    if (!route)
    {
        return;
    }
    if (!route->parameterConfiguration.wantAlternateRoutes || route->numberOfSelector == 1)
    {
        error =  error ? error : NB_RouteHandlerGetTrafficInformation(handler, &traffic);
    }
    if (navigation->configuration.offRouteIgnoreCount == 0)
    {
        NB_RoutePrivateConfiguration* routePrivateConfig = NB_RouteInformationGetPrivateConfiguration(route);
        navigation->configuration.offRouteIgnoreCount = routePrivateConfig->prefs.navOrIgnore;
    }

    if (error == NE_OK)
    {
        error = NB_NavigationNewRoute(navigation, route, traffic, camera);
    }

    if (error != NE_OK)
    {
        navigation->navState->error = error;

        // @todo: We want to log 'status', but this macro only takes the network result. We should probably call the QA logging function directly!
        LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Success);

        // the new route was NOT successfully applied, so free the memory

        if (route)
        {
            NB_RouteInformationDestroy(route);
        }

        if (traffic)
        {
            NB_TrafficInformationDestroy(traffic);
        }

        if (camera)
        {
            NB_CameraInformationDestroy(camera);
        }
        navigation->navState->navRoute = NULL;
        navigation->trafficInformation = NULL;
        navigation->cameraInformation = NULL;

        SetStatus(navigation, NB_NS_Error);
    }
}

static void
NB_NavigateRouteDownloadCanceled(NB_Navigation* navigation, NB_RouteHandler* handler)
{
    LOG_NAV_ERROR(navigation, NB_NetworkRequestStatus_Canceled);
}

static void
NB_NavigateRouteDownloadFailed(NB_Navigation* navigation, NB_RouteHandler* handler, NB_RequestStatus status, NB_Error err)
{
    NB_RouteHandlerGetRouteError(handler, &navigation->navState->routeError);

    navigation->navState->error = err;
    LOG_NAV_ERROR(navigation, status);

    SetStatus(navigation, NB_NS_Error);
}

static void
NB_NavigationRouteHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Navigation* navigation = userData;

    if (status != NB_NetworkRequestStatus_Progress)
    {
        NB_RouteHandlerGetRouteError(handler, &navigation->navState->routeError);
    }

    if (navigation->callbacks.routeDownloadCallback)
    {
        navigation->callbacks.routeDownloadCallback(handler, status, err, up, percent, navigation->callbacks.userData);
    }

    switch (status)
    {
    case NB_NetworkRequestStatus_Success:
        NB_NavigateRouteDownloadSucceeded(userData, handler);
        if (NB_RouteInformationIsRouteComplete(navigation->navState->navRoute))
        {
            if (navigation->callbacks.routeCompleteDownloadCallback)
            {
                navigation->callbacks.routeCompleteDownloadCallback(handler, status, err, up, percent, navigation->callbacks.userData);
            }
        }
        break;
    case NB_NetworkRequestStatus_Canceled:
        NB_NavigateRouteDownloadCanceled(userData, handler);
        break;
    case NB_NetworkRequestStatus_Failed:
    case NB_NetworkRequestStatus_TimedOut:
        if (navigation->status == NB_NS_UpdatingRoute)
        {
            // set wasNetworkBlocked to TRUE if recalculate is started before call is happened
            if (PAL_IsNetworkBlocked(NB_ContextGetPal(navigation->context)) == TRUE)
            {
                navigation->navState->wasNetworkBlocked = TRUE;
            }
        }
        else if (navigation->navState->navRoute && !NB_RouteInformationIsRouteComplete(navigation->navState->navRoute))
        {
            navigation->navState->completeRouteDownloadRetries++;
            NB_ScheduleCompleteRouteRequest(navigation, navigation->configuration.completeRouteRequestDelay);
        }
        else
        {
            NB_NavigateRouteDownloadFailed(userData, handler, status, err);
        }
        break;
    case NB_NetworkRequestStatus_Progress:
        /* No additional handling for now */
        break;
    }
}

static NB_Error
NB_NavigationDoRecalc(NB_Navigation* navigation, NB_GpsHistory* history, NB_RouteOptions* routeOptions, NB_RouteConfiguration* routeConfig, nb_boolean wantAlternateRoute, NB_WayPoint* pWaypoint, uint32 wayPointSize)
{
    NB_RouteParameters* recalcParams = NULL;
    NB_Error error = NE_OK;

    if (!navigation)
    {
        return NE_INVAL;
    }

    if (PAL_IsNetworkBlocked(NB_ContextGetPal(navigation->context)) == FALSE)
    {
        NB_GuidanceHeadsUp(navigation);
        CancelPendingRequests(navigation);

        // Reset enhanced content state for route recalc
        if (navigation->enhancedContentState)
        {
            (void)NB_EnhancedContentProcessorStateReset(navigation->enhancedContentState);
        }

        if (routeOptions)
        {
            error = error ? error : NB_RouteParametersCreateChangeRouteOptionsAndConfig(navigation->context, navigation->navState->navRoute, NULL, NULL, history, routeOptions, routeConfig, &recalcParams, wantAlternateRoute);
        }
        else
        {
            error = error ? error : NB_RouteParametersCreateOffRouteEx(navigation->context, history, navigation->navState->navRoute, &recalcParams, wantAlternateRoute);
        }

        if(pWaypoint && wayPointSize > 0)
        {
            CSL_VectorSetSize(recalcParams->dataQuery.via_point, 0);
            for(int i = 0; i < wayPointSize; i++)
            {
                NB_RouteParametersSetViaPoint(recalcParams, pWaypoint+i);
            }
        }
        error = error ? error : NB_RouteHandlerStartRequest(navigation->routeHandler, recalcParams);
        NB_RouteParametersDestroy(recalcParams);
        if (error == NE_OK && navigation->callbacks.routeRequestedCallback)
        {
            navigation->callbacks.routeRequestedCallback(navigation, NB_NRRR_Recalculation, navigation->callbacks.userData);
        }
        error = error ? error : NB_RecalcLimitAdd(navigation->recalcLimit, PAL_ClockGetGPSTime());

        if (!error)
        {
            // playing the recalc tone when start to navigate from plan trip.
            if (routeConfig && routeConfig->startToNavigate)
            {
                PlayRouteDownloadMessage(navigation, NB_RCT_Recalc);
            }
            else
            {
                if (routeOptions)
                {
                    PlayRouteDownloadMessage(navigation, NB_RCT_Initial);
                }
                else if ((navigation->status != NB_NS_UpdatingRoute || navigation->navState->wasNetworkBlocked == TRUE) &&
                         !navigation->fastNavStartupMode)
                {
                    PlayRouteDownloadMessage(navigation, NB_RCT_Recalc);
                }
            }
            SetStatus(navigation, NB_NS_UpdatingRoute);
        }
        navigation->navState->wasNetworkBlocked = FALSE;
    }
    else if (PAL_IsNetworkBlocked(NB_ContextGetPal(navigation->context)) == TRUE && navigation->status != NB_NS_UpdatingRoute)
    {
        if (!navigation->fastNavStartupMode)
        {
            NB_GuidanceHeadsUp(navigation);
            PlayRouteDownloadMessage(navigation, NB_RCT_Recalc);
        }
        SetStatus(navigation, NB_NS_UpdatingRoute);
        navigation->navState->wasNetworkBlocked = TRUE;
    }

    return error;
}

static void
NB_ScheduleCompleteRouteRequest(NB_Navigation* navigation, uint32 millisecs)
{
    PAL_Instance* pal = NULL;

    if (!navigation)
    {
        return;
    }

    //if we reach our max retries do recalc
    if (navigation->navState->completeRouteDownloadRetries > navigation->configuration.completeRouteMaximumAttempts)
    {
        // Schedule the recalc in this case as we are in a route callback and trying to do it
        // now is an invalid use of the route handler
        RecalcCheckLimit(navigation, FALSE);
        return;
    }

    pal = NB_ContextGetPal(navigation->context);
    PAL_TimerSet(pal, millisecs, NB_CompleteRouteRequestTimerCallback, navigation);
}

static void
NB_CompleteRouteRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;

    if (navigation)
    {
        if (reason == PTCBR_TimerFired)
        {
            NB_NavigationDoCompleteRouteRequest(navigation);
        }
    }
}

static NB_Error
NB_NavigationDoCompleteRouteRequest(NB_Navigation* navigation)
{
    NB_RouteParameters* recreateRouteParams = NULL;
    NB_Error error = NE_OK;

    if (!navigation || !navigation->navState || !navigation->navState->navRoute)
    {
        return NE_INVAL;
    }

    if (!NB_RouteHandlerIsRequestInProgress(navigation->routeHandler))
    {
        if (navigation->trafficHandler)
        {
            NB_TrafficHandlerCancelRequest(navigation->trafficHandler);
        }

        if ( NB_RouteHandlerIsRequestInProgress(navigation->routeHandler) )
        {
            NB_RouteHandlerCancelRequest(navigation->routeHandler);
        }

        error = error ? error : NB_RouteParametersRecreateRoute(navigation->context, navigation->navState->navRoute, &recreateRouteParams );
        error = error ? error : NB_RouteHandlerStartRequest(navigation->routeHandler, recreateRouteParams);
        if (error == NE_OK && navigation->callbacks.routeRequestedCallback)
        {
            NB_NavigateRouteRequestReason reason = recreateRouteParams->config.wantAlternateRoutes ? NB_NRRR_RouteSelector : NB_NRRR_InitialRoute;
            navigation->callbacks.routeRequestedCallback(navigation, reason, navigation->callbacks.userData);
        }
        NB_RouteParametersDestroy(recreateRouteParams);
    }
    else
    {
        navigation->navState->completeRouteDownloadRetries++;
        NB_ScheduleCompleteRouteRequest(navigation, navigation->configuration.completeRouteRequestDelay);
    }
    return error;
}

static NB_Error
NB_NavigationTrafficDownloadSucceeded(NB_Navigation* navigation, NB_TrafficHandler* handler)
{
    NB_Error error = NE_OK;
    NB_TrafficInformation* traffic = 0;

    error = NB_TrafficHandlerGetTrafficInformation(handler, &traffic);
    if (!error)
    {
        if (!NB_TrafficInformationReplyNotReady(traffic))
        {
            NB_PositionRoutePositionInformation currentPos = { 0 };
            double currentDistance = 0.0;
            double routeLength = 0.0;
            int newIncidentCount = 0;
            int clearedIncidentCount = 0;
            uint32 oldTripRemainingTime = 0;
            uint32 newTripRemainingTime = 0;
            uint32 closetManeuver = 0;
            double maneuverDistanceOffset = 0.0;
            uint32 maneuverCount = NB_RouteInformationGetManeuverCount(navigation->navState->navRoute);
            data_nav_maneuver* currentManeuver = NULL;

            error = error ? error : NB_TrafficInformationUpdateIncidentDistances(traffic, navigation->navState);

            error = error ? error : NB_NavigationGetRoutePositionInformation(navigation, &currentPos);
            error = error ? error : NB_RouteInformationGetSummaryInformation(navigation->navState->navRoute, 0, 0, &routeLength);
            if (!error)
            {
                currentDistance = routeLength - currentPos.remainingRouteDist;
            }

            closetManeuver = currentPos.closestManeuver;
            if ((closetManeuver == NAV_MANEUVER_START) ||
                (closetManeuver == NAV_MANEUVER_NONE))
            {
                closetManeuver = 0;
                currentDistance = 0;
            }

            currentManeuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, closetManeuver);
            if (currentManeuver && (currentPos.maneuverDistanceRemaining > 0.0))
            {
                maneuverDistanceOffset = currentManeuver->distance - currentPos.maneuverDistanceRemaining;
            }

            /* calculate old trip remain time */
            error = error ? error: NB_TrafficInformationGetSummary(navigation->trafficInformation, navigation->navState->navRoute,
                    closetManeuver, maneuverDistanceOffset, maneuverCount, NULL, NULL, &oldTripRemainingTime, NULL);

            error = error ? error : NB_TrafficInformationMergeFromNotify(navigation->trafficInformation, traffic,
                                                                         navigation->navState->navRoute, currentDistance,
                                                                         &newIncidentCount, &clearedIncidentCount);

            error = error ? error : NB_TrafficProcessorUpdate(navigation->trafficState, navigation->navState, navigation->trafficInformation, &currentPos);

            /* calculate new trip remain time */
            error = error ? error: NB_TrafficInformationGetSummary(navigation->trafficInformation, navigation->navState->navRoute,
                    closetManeuver, maneuverDistanceOffset, maneuverCount, NULL, NULL, &newTripRemainingTime, NULL);

            if (error == NE_OK)
            {
                uint32 travelTimeDelta = (uint32)(newTripRemainingTime - oldTripRemainingTime);
                NB_QaLogTrafficUpdate(navigation->context, newIncidentCount, clearedIncidentCount, travelTimeDelta);
            }
            else
            {
                NB_QaLogAppState(navigation->context, "Error updating traffic");
            }
        }
    }

    if (traffic)
    {
        NB_TrafficInformationDestroy(traffic);
    }
    navigation->initialNotifyComplete = TRUE;

    if (error == NE_OK)
    {
        navigation->navState->trafficNotifyRetries = 0;

        // If manually polling for traffic updates, schedule for the next notify
        if (navigation->trafficPollingIntervalMinutes)
        {
            NB_ScheduleTrafficNotify(navigation, navigation->trafficPollingIntervalMinutes * 60 * 1000, FALSE);
        }
    }

    return error;
}

static void
NB_NavigationTrafficDownloadCanceled(NB_Navigation* navigation)
{
    if (navigation)
    {
        NB_NavigationCancelTrafficNotify(navigation);
        navigation->initialNotifyComplete = TRUE;
    }
}

static void
NB_NavigationTrafficDownloadFailed(NB_Navigation* navigation)
{
    if (navigation)
    {
        NB_ScheduleTrafficNotify(navigation, navigation->configuration.trafficNotifyDelay, FALSE);
        navigation->initialNotifyComplete = TRUE;
    }
}

static void
NB_NavigationTrafficHandlerCallback(void* handler,NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;

    switch (status)
    {
        case NB_NetworkRequestStatus_Success:
            if (NB_NavigationTrafficDownloadSucceeded(navigation, handler) != NE_OK)
            {
                status = NB_NetworkRequestStatus_Failed;
                NB_NavigationTrafficDownloadFailed(navigation);
            }
            break;
        case NB_NetworkRequestStatus_Canceled:
            NB_NavigationTrafficDownloadCanceled(navigation);
            break;
        case NB_NetworkRequestStatus_Failed:
        case NB_NetworkRequestStatus_TimedOut:
            NB_NavigationTrafficDownloadFailed(navigation);
            break;
        case NB_NetworkRequestStatus_Progress:
            /* No additional handling for now */
            break;
    }

    if (navigation->callbacks.trafficNotificationCallback)
    {
        navigation->callbacks.trafficNotificationCallback(handler, status, err, up, percent, navigation->callbacks.userData);
    }
}

static void
NB_NavigationNetworkCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;

    if (navigation)
    {
        if (event == NB_NetworkNotifyEvent_ConnectionOpened && !NB_RouteHandlerIsRequestInProgress(navigation->routeHandler) && (navigation->navState && (navigation->navState->transportMode != NB_NTM_Pedestrian)))
        {
            NB_NavigationDoTrafficNotify(navigation, NULL);
        }
    }
}

static NB_Error
NB_NavigationDoTrafficNotify(NB_Navigation* navigation, NB_TrafficParameters* tp)
{
    NB_TrafficParameters*   trafficParameters = 0;
    NB_Error error = NE_OK;
    char* voice=NULL;
    uint32 progression = 0;
    nb_boolean needToFreeParameter = FALSE;

    if (!navigation || !navigation->navState)
    {
        return NE_INVAL;
    }

    if (navigation->navState->navRoute)
    {
        if (tp)
        {
            trafficParameters = tp;
        }
        else
        {
            NB_TrafficConfiguration trafficConfiguration = {0};

            NB_GuidanceInformationGetVoiceStyle(navigation->guidanceInformation, &voice);
            progression = (uint32) (navigation->navPublicState->routeLength - navigation->navPublicState->tripRemainDistance);

            trafficConfiguration.notify = TRUE;
            trafficConfiguration.startNotification = navigation->startNotification;
            trafficConfiguration.route = navigation->navState->navRoute;
            trafficConfiguration.navProgressPosition = progression;
            trafficConfiguration.voiceStyle = voice;
            trafficConfiguration.supportedPhoneticsFormats =
                navigation->navState->navRoute->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats;

            error = error ? error : NB_TrafficParametersCreateEx(navigation->context, &trafficConfiguration, &trafficParameters);

            needToFreeParameter = TRUE;
        }
    }

    if ( error == NE_OK && trafficParameters && navigation->trafficHandler && !NB_TrafficHandlerIsRequestInProgress(navigation->trafficHandler)  )
    {
        error = error ? error : NB_TrafficHandlerStartRequest(navigation->trafficHandler, trafficParameters);
    }

    if (needToFreeParameter)
    {
        NB_TrafficParametersDestroy(trafficParameters);
    }

    if (voice)
    {
        nsl_free(voice);
    }

    return error;
}

static NB_Error
NB_NavigationCancelTrafficNotify(NB_Navigation* navigation)
{
    NB_Error error = NE_OK;
    PAL_Instance* pal = NULL;

    if (!navigation || !navigation->navState)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(navigation->context);

    if (navigation->trafficHandler)
    {
        error = error ? error : NB_TrafficHandlerCancelRequest(navigation->trafficHandler);
    }

    PAL_TimerCancel(pal, NB_TrafficNotifyTimerCallback, navigation );
    navigation->navState->trafficNotifyRetries = 0;

    return error;
}

static void
NB_ScheduleTrafficNotify(NB_Navigation* navigation, uint32 millisecs, nb_boolean initial)
{
    PAL_Instance* pal = NULL;
    NB_TrafficType trafficFlags = NB_TrafficType_None;
    NB_Error err = NE_OK;

    if (!navigation)
    {
        return;
    }

    err = NB_RouteInformationGetTrafficFlags(navigation->navState->navRoute, &trafficFlags);
    if (err || (trafficFlags & NB_TrafficType_Notification) == 0)
    {
        return;
    }

    pal = NB_ContextGetPal(navigation->context);

    if (navigation->navState->trafficNotifyRetries < navigation->configuration.trafficNotifyMaximumRetries)
    {
        navigation->navState->trafficNotifyRetries++;
    }
    else
    {
        NB_NavigationCancelTrafficNotify(navigation);
        return;
    }

    navigation->startNotification = initial;

    PAL_TimerSet(pal, millisecs, NB_TrafficNotifyTimerCallback, navigation);

}

static void
NB_TrafficNotifyTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;

    if (navigation)
    {
        if (reason == PTCBR_TimerFired)
        {
            NB_NavigationDoTrafficNotify(navigation, NULL);
        }
    }
}

static void
NB_NavigationQaLogCameras(NB_Navigation* navigation)
{
    if (navigation && navigation->cameraInformation)
    {
        NB_Error err = NE_OK;
        uint32 i = 0;
        uint32 cameraCount = 0;

        err = NB_CameraInformationGetCameraCount(navigation->cameraInformation, &cameraCount);
        if (err != NE_OK)
        {
            return;
        }

        for(i=0; i < cameraCount; i++)
        {
            NB_Camera camera = {0};
            err = NB_CameraInformationGetCamera(navigation->cameraInformation, i, &camera);
            if (err == NE_OK)
            {
                NB_QaLogCamera(navigation->context, &camera);
            }
        }
    }
}

static void
NB_NavigationProcessCameras(NB_Navigation* navigation, NB_SearchHandler* handler)
{
    NB_Error err = NE_OK;
    NB_SearchInformation* searchInformation = 0;

    err = NB_SearchHandlerGetSearchInformation(handler, &searchInformation);
    if (!err)
    {
        NB_CameraInformation* cameraInformation = 0;

        (void)NB_CameraInformationCreateFromSearch(navigation->context, searchInformation, &cameraInformation);
        (void)NB_CameraInformationUpdateCameraDistances(cameraInformation, navigation->navState);
        (void)NB_NavigationUpdateCameraInformation(navigation, cameraInformation);
        NB_NavigationQaLogCameras(navigation);
        (void)NB_SearchInformationDestroy(searchInformation);
    }
}

static void
NB_NavigationCameraHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    NB_Navigation* navigation = userData;

    switch (status)
    {
    case NB_NetworkRequestStatus_Success:
        NB_NavigationProcessCameras(navigation, handler);
        break;
    case NB_NetworkRequestStatus_Canceled:
        break;
    case NB_NetworkRequestStatus_Failed:
    case NB_NetworkRequestStatus_TimedOut:
        CameraRequestFailed(navigation);
        break;
    case NB_NetworkRequestStatus_Progress:
        /* No additional handling for now */
        break;
    }

    if (navigation->callbacks.cameraNotificationCallback)
    {
        navigation->callbacks.cameraNotificationCallback(handler, status, err, up, percent, navigation->callbacks.userData);
    }
}

static void CameraRequestFailed(NB_Navigation* navigation)
{
    if (!navigation)
    {
        return;
    }

    ScheduleCameraDownloadRequest(navigation, navigation->configuration.cameraSearchDelay);
}

static NB_Error
NB_NavigationSearchCameras(NB_Navigation* navigation, NB_RouteInformation* route)
{
    NB_Error err = NE_OK;
    NB_Place origin = { { 0 } };

    err = NB_RouteInformationGetOrigin(route, &origin);
    if (!err)
    {
        NB_SearchRegion region = { 0 };
        NB_SearchParameters* parameters = 0;

        region.type = NB_ST_RouteAndCenter;
        region.route = route;
        region.center.latitude = origin.location.latitude;
        region.center.longitude = origin.location.longitude;
        region.distance = navigation->configuration.cameraSearchDistance;
        region.width = 0;

        err = NB_SearchParametersCreatePOI(navigation->context, &region, NULL, "safety-cameras", 0, 0, 100, NB_EXT_None, "", &parameters);

        err = err ? err : NB_SearchParametersSetSource(parameters, NB_SS_Navigation);

        err = err ? err : NB_SearchHandlerStartRequest(navigation->cameraHandler, parameters);

            (void)NB_SearchParametersDestroy(parameters);
        }


    return err;
}

static void
NB_NavigationProcessPointsOfInterest(NB_Navigation* navigation, NB_SingleSearchHandler* pointsOfInterestHandler)
{
    NB_Error err = NE_OK;
    NB_SingleSearchInformation* searchInformation = 0;

    err = NB_SingleSearchHandlerGetInformation(pointsOfInterestHandler, &searchInformation);
    if (!err)
    {
        (void)NB_PointsOfInterestInformationUpdateWithSingleSearch(navigation->poiInformation, navigation, searchInformation);

        if (NB_ContextGetQaLog(navigation->context) != NULL)
        {
            int count = 0;
            int i = 0;

            (void)NB_PointsOfInterestInformationGetPointOfInterestCount(navigation->poiInformation, &count);

            if (count == 0)
            {
                NB_QaLogAppState(navigation->context, "poi-amr-no-results");
            }

            for (i = 0; i < count; i++)
            {
                NB_PointOfInterest poi = {{{0}}};

                NB_PointsOfInterestInformationGetPointOfInterest(navigation->poiInformation, i, &poi);
                NB_QaLogPlace(navigation->context, &poi.place, "poi-amr-result");
            }
        }

        (void)NB_SingleSearchInformationDestroy(searchInformation);
    }
}

static void
NB_NavigationPointsOfInterestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    NB_Navigation* navigation = userData;

    switch (status)
    {
    case NB_NetworkRequestStatus_Success:
        NB_NavigationProcessPointsOfInterest(navigation, handler);

        /* last search did NOT fail */
        navigation->poiLastQueryFailed = FALSE;
        navigation->poiQueryFailedTimes = 0;

        if (navigation->callbacks.pointsOfInterestNotificationCallback)
        {
            navigation->callbacks.pointsOfInterestNotificationCallback(handler, status, err, up, percent, navigation->callbacks.userData);
        }
        break;
    case NB_NetworkRequestStatus_Canceled:
    case NB_NetworkRequestStatus_Failed:
    case NB_NetworkRequestStatus_TimedOut:
        /* last search failed, a new search will be initiated on the next GPS location update. */
        navigation->poiLastQueryFailed = TRUE;
        navigation->poiQueryFailedTimes++;
        break;
    case NB_NetworkRequestStatus_Progress:
        /* No additional handling for now */
        break;
    }
}

static NB_Error
NB_NavigationSearchPointsOfInterest(NB_Navigation* navigation, NB_RouteInformation* route)
{
    NB_Error err = NE_INVAL;
    NB_PositionRoutePositionInformation currentPosition = { 0 };
    int categoryCount = 0;

    if (!navigation || !navigation->poiConfiguration.categoryCount)
    {
        return err;
    }

    err = NB_NavigationGetRoutePositionInformation(navigation, &currentPosition);

    if (!err)
    {
        NB_SingleSearchParameters* searchParameters = 0;
        NB_SearchRegion region = {0};
        NB_Place originPlace;
        char* categories[NB_POI_PREFETCH_MAX_CATEGORIES] = { 0 };
        uint32 count = 0;
        int n = 0;
        int i = 0;

        region.type = NB_ST_RouteAndCenter;
        region.route = route;
        region.center.latitude = currentPosition.projLat;
        region.center.longitude = currentPosition.projLon;
        region.distance = navigation->poiConfiguration.searchLengthMeters > 0 ?
            navigation->poiConfiguration.searchLengthMeters : DEFAULT_POI_SEARCH_LENGTH_METERS;
        region.width = navigation->poiConfiguration.searchWidthMeters > 0 ?
            navigation->poiConfiguration.searchWidthMeters : DEFAULT_POI_SEARCH_WIDTH_METERS;

        categoryCount = (NB_POI_PREFETCH_MAX_CATEGORIES < navigation->poiConfiguration.categoryCount) ?
                NB_POI_PREFETCH_MAX_CATEGORIES : navigation->poiConfiguration.categoryCount;
        for (n = 0; n < categoryCount; n++)
        {
            categories[n] = &navigation->poiConfiguration.categories[n][0];
        }

        count = (uint32)((region.distance * navigation->poiConfiguration.densityFactor) / region.width);

        err = NB_SingleSearchParametersCreateByResultStyleType(navigation->context,
            &region,
            NULL,
            navigation->poiConfiguration.searchScheme,
            count,
            NB_EXT_None,
            NULL,
            NB_SRST_SingleSearch,
            &searchParameters
        );
        err = err ? err : NB_SingleSearchParametersSetSearchWithCorridor(searchParameters, TRUE);
        for(i = 0; i<navigation->poiConfiguration.categoryCount; i++)
        {
            err = err ? err : NB_SingleSearchParametersAddSearchFilterKeyValue(searchParameters, "category", categories[i]);
        }

        err = err ? err : NB_SingleSearchParametersSetSource(searchParameters, NB_SS_Navigation);
        err = err ? err : NB_RouteInformationGetOrigin(route, &originPlace);
        err = err ? err : NB_SingleSearchParametersSetOrigin(searchParameters, &(originPlace.location));

        // @todo: The key 'sort-by' is not supported so far.
//        err = err ? err : NB_SingleSearchParametersSetSearchSortBy(searchParameters, NB_SSB_Distance);

        NB_QaLogPOIAlongRouteSearch(navigation->context, region.center.latitude,
            region.center.longitude, region.distance, region.width,
            "", navigation->poiConfiguration.searchScheme,
            (const char**)(categories), categoryCount,
            count);

        if (!err)
        {
            navigation->poiLastQueryRouteRemaining = currentPosition.remainingRouteDist;
            err = NB_SingleSearchHandlerStartRequest(navigation->poiHandler, searchParameters);
            (void)NB_SingleSearchParametersDestroy(searchParameters);
        }
    }

    if (err)
    {
        NB_QaLogNavigationError(navigation->context, NB_NRE_None, err, NB_NetworkRequestStatus_Success, __FUNCTION__, __FILE__, __LINE__);
    }

    return NE_OK;
}

static nb_boolean
IsPointsOfInterestPreSearchPending(NB_Navigation* navigation)
{
    NB_PositionRoutePositionInformation currentPosition = { 0 };
    if (NB_NavigationGetRoutePositionInformation(navigation, &currentPosition) == NE_OK)
    {
        if (navigation->poiLastQueryRouteRemaining != 0)
        {
           // determine if POI query needed based on distance from last POI query
            int poiPreSearchMeters = navigation->poiConfiguration.searchPreSearchMeters >= 0 ?
            navigation->poiConfiguration.searchPreSearchMeters : DEFAULT_POI_PRE_SEARCH_METERS;
            int poiSearchLengthMeters = navigation->poiConfiguration.searchLengthMeters > poiPreSearchMeters ?
                    navigation->poiConfiguration.searchLengthMeters : DEFAULT_POI_SEARCH_LENGTH_METERS;

            double distanceForPreSearch = (double)(poiSearchLengthMeters - poiPreSearchMeters);
            double distanceSinceLastSearch = navigation->poiLastQueryRouteRemaining - currentPosition.remainingRouteDist;

            if (distanceSinceLastSearch > distanceForPreSearch)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void
NB_PronunRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Navigation* navigation = (NB_Navigation*) userData;

    if (navigation)
    {
        if (reason == PTCBR_TimerFired)
        {
            NB_NavigationDoPronunRequest(navigation);
        }
    }
}

static void
NB_SchedulePronunDownloadRequest(NB_Navigation* navigation, uint32 millisecs)
{
    PAL_Instance* pal = NULL;

    if (!navigation)
    {
        return;
    }

    pal = NB_ContextGetPal(navigation->context);
    PAL_TimerSet(pal, navigation->configuration.pronunRequestDelay, NB_PronunRequestTimerCallback, navigation);
}

void
CameraRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Navigation* navigation = (NB_Navigation*)userData;

    if (reason == PTCBR_TimerFired && navigation->configuration.cameraSearchEnabled && navigation->navState && navigation->navState->navRoute)
    {
        NB_NavigationSearchCameras(navigation, navigation->navState->navRoute);
    }
}

void
ScheduleCameraDownloadRequest(NB_Navigation* navigation, uint32 delay)
{
    if (navigation)
    {
        PAL_Instance* pal = NB_ContextGetPal(navigation->context);
        PAL_TimerSet(pal, delay, CameraRequestTimerCallback, navigation);
    }
}

static void
PoiSearchRequestTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Navigation* navigation = (NB_Navigation*)userData;

    if(PTCBR_TimerFired == reason && navigation->navState && navigation->navState->navRoute)
    {
        SetPoiSearchTimerTimeOutStatus(navigation, TRUE);
    }
}

static void
SchedulePoiSearchRequest(NB_Navigation* navigation, uint32 delay)
{
    if (navigation)
    {
        PAL_Instance* pal = NB_ContextGetPal(navigation->context);
        SetPoiSearchTimerTimeOutStatus(navigation, FALSE);
        PAL_TimerSet(pal, delay, PoiSearchRequestTimerCallback, navigation);
    }
}

static void
NB_NavigationUpdateVoices(NB_Navigation* navigation, NB_PronunHandler* pronunHandler)
{
    NB_PronunInformation* pronunInformation = 0;

    (void)NB_PronunHandlerGetInformation(pronunHandler, &pronunInformation); //This adds all our voices to the voice cache
    NB_PronunInformationDestroy(pronunInformation);
}

static void
NB_NavigationPronunHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    NB_Navigation* navigation = userData;

    switch (status)
    {
    case NB_NetworkRequestStatus_Success:
        NB_NavigationUpdateVoices(navigation, handler);
        navigation->pronunDownloadRetries = 0;
        break;
    case NB_NetworkRequestStatus_Canceled:
        break;
    case NB_NetworkRequestStatus_Failed:
    case NB_NetworkRequestStatus_TimedOut:
        if (navigation->pronunDownloadRetries < PRONUN_MAXIMUM_RETRIES)
        {
            navigation->pronunDownloadRetries++;
            NB_SchedulePronunDownloadRequest(navigation, navigation->configuration.pronunRequestDelay);
        }
        break;
    case NB_NetworkRequestStatus_Progress:
        /* No additional handling for now */
        break;
    }
}

static NB_Error
NB_NavigationDoPronunRequest(NB_Navigation* navigation)
{
    NB_PronunParameters* pronunParams = NULL;
    NB_Error error = NE_OK;
    char* voice=NULL;

    if (!navigation)
    {
        return NE_INVAL;
    }

    NB_GuidanceInformationGetVoiceStyle(navigation->guidanceInformation, &voice);

    error = error ? error : NB_PronunParametersCreate(navigation->context, voice, navigation->navState->navRoute, &pronunParams);
    error = error ? error : NB_PronunHandlerStartRequest(navigation->pronunHandler, pronunParams);
    NB_PronunParametersDestroy(pronunParams);

    if (voice)
        nsl_free(voice);

    return error;
}

NB_DEF NB_Error NB_NavigationSuspendSession(NB_Navigation* navigation)
{
   if (!navigation)
   {
       return NE_INVAL;
   }
   if (navigation->suspended)
   {
       return NE_UNEXPECTED;
   }
   navigation->suspended = TRUE;
   CancelPendingRequests(navigation);

   return NE_OK;
}

NB_DEF NB_Error NB_NavigationResumeSession(NB_Navigation* navigation)
{
    if (!navigation)
    {
        return NE_INVAL;
    }
    if (!navigation->suspended)
    {
        return NE_UNEXPECTED;
    }
    navigation->suspended = FALSE;

    return NE_OK;
}

void
CancelPendingRequests(NB_Navigation* navigation)
{
    PAL_Instance* pal = NB_ContextGetPal(navigation->context);

    PAL_TimerCancel(pal, RecalcTimerCallback, navigation);
    PAL_TimerCancel(pal, NB_CompleteRouteRequestTimerCallback, navigation);
    PAL_TimerCancel(pal, NB_TrafficNotifyTimerCallback, navigation );
    PAL_TimerCancel(pal, NB_PronunRequestTimerCallback, navigation);
    PAL_TimerCancel(pal, PoiSearchRequestTimerCallback, navigation);
    PAL_TimerCancel(pal, CameraRequestTimerCallback, navigation);

    if (navigation->routeHandler)
    {
        NB_RouteHandlerCancelRequest(navigation->routeHandler);
    }

    if (navigation->trafficHandler)
    {
        NB_TrafficHandlerCancelRequest(navigation->trafficHandler);
    }

    if (navigation->pronunHandler)
    {
        NB_PronunHandlerCancelRequest(navigation->pronunHandler);
    }

    if (navigation->poiHandler)
    {
        NB_SingleSearchHandlerCancelRequest(navigation->poiHandler);
    }

    if (navigation->cameraHandler)
    {
        NB_SearchHandlerCancelRequest(navigation->cameraHandler);
    }
}

NB_Error
PlayRouteDownloadMessage(NB_Navigation* navigation, NB_NavigateRouteCalculationType type)
{
    NB_Error error = NE_OK;

    if (navigation->guidanceInformation)
    {
        NB_GuidanceMessage* message = NULL;
        NB_RouteDataSourceOptions dataSourceOptions = {0};

        dataSourceOptions.route = navigation->navState->navRoute;
        dataSourceOptions.collapse = FALSE;
        dataSourceOptions.maneuverPos = NB_GuidanceInformationGetRouteCalculationManeuver(type);
        dataSourceOptions.trafficInfo = NULL;
        dataSourceOptions.currentRoutePosition = NULL;
        dataSourceOptions.guidanceState = navigation->guidanceState;

        error = NB_GuidanceInformationGetRouteCalculationAnnouncement(
            navigation->guidanceInformation, NB_RouteInformationDataSource,
            &dataSourceOptions, type, navigation->announceType, navigation->announceUnits, &message);

        if (type == NB_RCT_RouteUpdated)
        {
            NB_GuidanceMessageSetPlayedCallback(message, NB_RouteUpdatedMessagePlayedCallback, navigation);
        }

        if (message != NULL)
        {
            PlayGuidanceMessage(navigation, message, TRUE);
        }
    }

    return error;
}

void
CalculateStartupRegion(NB_Navigation* navigation)
{
    NB_GpsLocation origin = { 0 };
    double actualDistance = 0.0;

    // Get the start of the route
    NB_RouteInformationGetRouteStart(navigation->navState->navRoute, &navigation->startupRegion.lat, &navigation->startupRegion.lon);

    // Get the location where the route was requested
    NB_RouteParametersGetOriginGpsLocation(navigation->navState->navRoute->routeParameters, &origin);

    actualDistance = NB_SpatialGetLineOfSightDistance(navigation->startupRegion.lat, navigation->startupRegion.lon, origin.latitude, origin.longitude, NULL);
    navigation->startupRegion.radius = MAX(actualDistance * navigation->configuration.startupRegionSizeFactor, navigation->configuration.startupRegionMinimumSize);

    NB_QaLogNavigationStartupRegion(navigation->context, navigation->startupRegion.lat, navigation->startupRegion.lon , navigation->startupRegion.radius, actualDistance);
}

boolean InStartupRegion(NB_Navigation* navigation, const NB_GpsLocation* location)
{
    double currentDistance = NB_SpatialGetLineOfSightDistance(navigation->startupRegion.lat, navigation->startupRegion.lon, location->latitude, location->longitude, NULL);
    return (boolean)(currentDistance <= navigation->startupRegion.radius);
}

void CalculateArrivingRegion(NB_Navigation* navigation, double latitude, double longitude)
{
    NB_LatitudeLongitude destination  = { 0 };
    double distanceToDestination = 0.0;

    NB_RouteParametersGetDestination(navigation->navState->navRoute->routeParameters, &destination, NULL);

    // Compute distance from current location to destination to determine the arriving region radius
    distanceToDestination = NB_SpatialGetLineOfSightDistance(latitude, longitude, destination.latitude, destination.longitude, NULL);
    navigation->arrivingRegionRadius = MAX(distanceToDestination, navigation->configuration.startupRegionMinimumSize);
}

boolean InArrivingRegion(NB_Navigation* navigation, const NB_GpsLocation* location)
{
    NB_LatitudeLongitude destination  = { 0 };
    double currentDistance = 0.0;

    NB_RouteParametersGetDestination(navigation->navState->navRoute->routeParameters, &destination, NULL);

    // Determine if current location is within the arriving region radius surrounding the destination
    currentDistance = NB_SpatialGetLineOfSightDistance(destination.latitude, destination.longitude, location->latitude, location->longitude, NULL);
    return (boolean)(currentDistance <= navigation->arrivingRegionRadius);
}

NB_DEF NB_Error
NB_NavigationGetErrors(NB_Navigation* navigation, NB_NavigateRouteError* routeError, NB_Error* error)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    if (routeError)
    {
        *routeError = navigation->navState->routeError;
    }

    if (error)
    {
        *error = navigation->navState->error;
    }

    return NE_OK;
}

void RecalcTimerCallback(PAL_Instance* pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        NB_Navigation* navigation = userData;
        NB_NavigationDoRecalc(navigation, NB_ContextGetGpsHistory(navigation->context), NULL, NULL, TRUE, NULL, 0);
    }
}

NB_Error
RecalcCheckLimit(NB_Navigation* navigation, nb_boolean immediate)
{
    NB_Error error = NE_OK;

    if (!NB_RecalcLimitExceeded(navigation->recalcLimit, PAL_ClockGetGPSTime()))
    {
        navigation->navState->recalcCount++;
        if (immediate)
        {
            error = NB_NavigationDoRecalc(navigation, NB_ContextGetGpsHistory(navigation->context), NULL, NULL, TRUE, NULL, 0);
        }
        else
        {
            PAL_TimerSet(NB_ContextGetPal(navigation->context), 0, RecalcTimerCallback, navigation);
        }
    }
    else
    {
        SetStatus(navigation, NB_NS_NavigatingConfirmRecalc);
        PlayRouteDownloadMessage(navigation, NB_RCT_ConfirmRecalc);
    }

    return error;
}

NB_DEF NB_Error
NB_NavigationGetTripRemainingInformation(NB_Navigation* navigation, uint32* time, uint32* delay, double* meter)
{
    uint32 maneuverIndex = 0;
    double maneuverDistanceOffset = 0.0;
    data_nav_maneuver* maneuver = NULL;
    uint32 endMan = 0;

    if ((navigation == NULL) || (navigation->navState == NULL) || (navigation->navState->navRoute == NULL))
    {
        return NE_INVAL;
    }

    endMan = NB_RouteInformationGetManeuverCount(navigation->navState->navRoute);

    if (navigation->navState->initialRouteMatch)
    {
        maneuverIndex = navigation->navState->currentRoutePosition.closest_maneuver;
        maneuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, maneuverIndex);
        maneuverDistanceOffset = maneuver->distance - navigation->navState->currentRoutePosition.maneuver_distance_remaining;
    }
    else
    {
        maneuverIndex = 0;
        maneuverDistanceOffset = 0;
    }

    return NB_TrafficInformationGetSummary(navigation->trafficInformation, navigation->navState->navRoute,
                                    maneuverIndex, maneuverDistanceOffset, endMan, meter, NULL, time, delay);
}

/* See description in header file */

NB_DEF NB_Error
NB_NavigationSetVoiceCachePronunStyle (NB_Navigation* navigation, NB_Context* context, const char* pronunStyle)
{
    CSL_Cache* voiceCache = NULL;
    byte* userData = NULL;
    size_t userDataSize = 0;
    int result = 0;

    voiceCache = NB_ContextGetVoiceCache(context);

    //get and set the voice cache user data
    if (voiceCache)
    {
        CSL_CacheGetUserData(voiceCache, &userData, &userDataSize);
        if (userData == NULL)
        {
            //set the user Data as pronun style, account for NULL terminator
            CSL_CacheSetUserData(voiceCache, (byte*) pronunStyle, nsl_strlen(pronunStyle) + 1);

            //clear memory and persistent cache anyways since you have updated the pronunstyle
            CSL_CacheClear(voiceCache);
        }
        else
        {
            //if user data is valid, compare with the current pronun style
            result = nsl_strcmp((const char*) userData, pronunStyle);

            if (result != 0)
            {
                //clear memory and persistent cache
                CSL_CacheClear(voiceCache);

                //set new user data
                CSL_CacheSetUserData(voiceCache, (byte*) pronunStyle, nsl_strlen(pronunStyle) + 1);
            }
        }
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationSetSpeedLimitConfiguration(NB_Navigation* navigation,
                                        NB_SpeedLimitConfiguration* configuration
                                        )
{
    if(configuration == NULL || navigation == NULL)
    {
        return NE_INVAL;
    }

    if(navigation->speedlimitState == NULL)
    {
        return NE_INVAL;
    }
    return NB_SpeedLimitStateSetConfigure(navigation->speedlimitState, configuration);
}

void
NavigationSpeedLimitCallbackFunctionEx(NB_SpeedLimitStateData* speedlimitData, const char * sign_id, void* userData)
{
    NB_Navigation* pNavigation = (NB_Navigation*)userData;
    if (pNavigation == NULL || speedlimitData == NULL)
    {
        return;
    }
    if (sign_id)
    {
        nsl_memset(pNavigation->navState->speedSignId,0, NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN + 1);
        nsl_strncpy(pNavigation->navState->speedSignId, sign_id, NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN);

        NB_QaLogNavigationState(pNavigation->context, pNavigation->navState, pNavigation->navPublicState);
    }
    pNavigation->callbacks.speedLimitCallback(speedlimitData,pNavigation->callbacks.userData);
}

NB_DEF NB_Error
NB_NavigationFastStartupStatusChange(NB_Navigation* navigation, nb_boolean isEnter, const NB_GpsLocation* location, nb_boolean* needUpdateRoute)
{
    NB_GpsHistory* history = NULL;
    nb_boolean recalcRequired = FALSE;
    enum nav_update_reason recalcReason = nav_update_none;
    NB_RouteParameters* recreateRouteParams = NULL;
    NB_Error error = NE_OK;

    if (navigation == NULL)
    {
        return NE_INVAL;
    }

    if (isEnter)
    {
        navigation->fastNavStartupMode = isEnter;
    }
    else
    {
        if ( NB_RouteHandlerIsRequestInProgress(navigation->routeHandler) )
        {
            NB_RouteHandlerCancelRequest(navigation->routeHandler);
            NB_RouteParametersClone(navigation->navState->navRoute->routeParameters, &recreateRouteParams);
            error = NB_RouteHandlerStartRequest(navigation->routeHandler, recreateRouteParams);
            if (error == NE_OK && navigation->callbacks.routeRequestedCallback)
            {
                navigation->callbacks.routeRequestedCallback(navigation, NB_NRRR_InitialRoute, navigation->callbacks.userData);
            }
            NB_RouteParametersDestroy(recreateRouteParams);
            recalcRequired = TRUE;
        }
        else
        {
            recalcRequired = FALSE;
            recalcReason = nav_update_none;
            NB_NavigationCheckForRecalc(navigation, location, &recalcRequired, &recalcReason);
            *needUpdateRoute = recalcRequired;
            if (recalcRequired)
            {
                history = NB_ContextGetGpsHistory(navigation->context);
                if (history)
                {
                    NB_GpsHistoryAdd(history, (NB_GpsLocation*)location);
                    NB_NavigationDoRecalc(navigation, history, NULL, NULL, TRUE, NULL, 0);
                }
            }
            else
            {
                navigation->fastNavStartupMode = FALSE;
            }
        }
    }

    return NE_OK;
}

void NB_RouteUpdatedMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    NB_Navigation* nav = userData;
    nav->fastNavStartupMode = FALSE;
}

NB_Error PlaySwitchToTbtMessage(NB_Navigation* navigation)
{
    NB_Error error = NE_OK;

    if (navigation->guidanceInformation)
    {
        NB_GuidanceMessage* message = NULL;
        NB_RouteDataSourceOptions dataSourceOptions = {0};

        dataSourceOptions.route = navigation->navState->navRoute;
        dataSourceOptions.collapse = FALSE;
        dataSourceOptions.maneuverPos = NB_NMP_SwitchToTBT;
        dataSourceOptions.trafficInfo = NULL;
        dataSourceOptions.currentRoutePosition = NULL;

        error = NB_GuidanceInformationGetNavigationModeSwitchAnnouncement(
            navigation->guidanceInformation, NB_RouteInformationDataSource,
            &dataSourceOptions, navigation->announceType, navigation->announceUnits, &message);

        if (message != NULL)
        {
            PlayGuidanceMessage(navigation, message, TRUE);
        }
    }

    return error;
}

NB_DEF NB_Error NB_NavigationGetPronunInformation(NB_Navigation* navigation, const char* key,
                                                  char* textBuffer, uint32* textBufferSize,
                                                  byte* phoneticsData, uint32* phoneticsDataSize,
                                                  char* language, uint32* languageDataSize,
                                                  char* translatedText, uint32* translatedTextDataSize,
                                                  double* duration, nb_boolean* isBaseVoices)
{
    NB_Error err = NE_OK;

    if (!navigation || !key || (!textBuffer && !textBufferSize && (*textBufferSize) == 0
        && !duration && !phoneticsData && !phoneticsDataSize && (*phoneticsDataSize) == 0))
    {
        return NE_INVAL;
    }

    if(isBaseVoices)
    {
        *isBaseVoices = FALSE;
    }
    if (navigation->navState->navRoute)
    {
        err = NB_RouteInformationGetPronunInformation(navigation->navState->navRoute, key,textBuffer, textBufferSize, phoneticsData, phoneticsDataSize, language, languageDataSize, translatedText, translatedTextDataSize,duration);
    }
    else
    {
        err = NE_NOENT;
        if(languageDataSize)
        {
            *languageDataSize = 0;
        }
        if(translatedTextDataSize)
        {
            *translatedTextDataSize = 0;
        }
    }
    if (err != NE_OK && navigation->trafficInformation)
    {
        err = NB_TrafficInformationGetPronunInformation(navigation->trafficInformation, key,
            textBuffer, textBufferSize, phoneticsData, phoneticsDataSize, duration);
    }
    if (err != NE_OK && navigation->guidanceInformation)
    {
        err = NB_GuidanceInformationGetPronunInformation(navigation->guidanceInformation, key,
            textBuffer, textBufferSize, duration);
        if(isBaseVoices)
        {
            *isBaseVoices = TRUE;
        }
        if (phoneticsDataSize)
        {
            *phoneticsDataSize = 0;
        }
    }

    return err;
}

NB_DEF NB_Error NB_NavigationGetNameByKey(NB_Navigation* navigation, data_roadinfo* info, const char* key, char* textBuffer, uint32* textBufferSize)
{
    NB_Error err = NE_NOENT;
    data_util_state* pds = NULL;
    pds = NB_ContextGetDataState(navigation->context);
    char* pronun = nsl_malloc(info->pronun.size + 1);
    nsl_memset(pronun, 0, info->pronun.size + 1);
    nsl_memcpy(pronun, info->pronun.data, info->pronun.size);
    if(nsl_strcmp(pronun, key) == 0)
    {
        err = NE_OK;
        const char* primary = data_string_get(pds, &info->primary);
        nsl_strcpy(textBuffer, primary);
        *textBufferSize = nsl_strlen(primary)+1;
        err = NE_OK;
    }
    nsl_free(pronun);
    return err;
}

NB_DEF NB_Error NB_NavigationGetDescriptionInformation(NB_Navigation* navigation, const char* key,
                                                  char* textBuffer, uint32* textBufferSize,
                                                  byte* phoneticsData, uint32* phoneticsDataSize,
                                                  char* language, uint32* languageDataSize,
                                                  char* translatedText, uint32* translatedTextDataSize,
                                                  double* duration, nb_boolean* isBaseVoices)
{
    NB_Error err = NE_OK;
    
    if (!navigation || !key || (!textBuffer && !textBufferSize && (*textBufferSize) == 0
                                && !duration && !phoneticsData && !phoneticsDataSize && (*phoneticsDataSize) == 0))
    {
        return NE_INVAL;
    }
    
    if(isBaseVoices)
    {
        *isBaseVoices = FALSE;
    }
    if (navigation->navState->navRoute)
    {
        //try use primary name instead pronun
        nb_boolean find = FALSE;
        uint32 count = NB_RouteInformationGetManeuverCount(navigation->navState->navRoute);
        for(int i = 0; i < count; i++)
        {
            data_nav_maneuver* maneuver = NB_RouteInformationGetManeuver(navigation->navState->navRoute, i);
            if(maneuver != NULL)
            {
                NB_Error result = NB_NavigationGetNameByKey(navigation, &maneuver->turn_roadinfo, key, textBuffer, textBufferSize);
                if(result == NE_NOENT)
                {
                    result = NB_NavigationGetNameByKey(navigation, &maneuver->current_roadinfo, key, textBuffer, textBufferSize);
                }
                if(result == NE_NOENT)
                {
                    result = NB_NavigationGetNameByKey(navigation,&maneuver->intersection_roadinfo, key, textBuffer, textBufferSize);
                }
                if(result == NE_OK)
                {
                    find = TRUE;
                    break;
                }
            }
        }
        if(!find)
        {
            err = NB_RouteInformationGetPronunInformation(navigation->navState->navRoute, key,textBuffer, textBufferSize, phoneticsData, phoneticsDataSize, language, languageDataSize, translatedText, translatedTextDataSize,duration);
        }
    }
    else
    {
        err = NE_NOENT;
        if(languageDataSize)
        {
            *languageDataSize = 0;
        }
        if(translatedTextDataSize)
        {
            *translatedTextDataSize = 0;
        }
    }
    if (err != NE_OK && navigation->trafficInformation)
    {
        err = NB_TrafficInformationGetPronunInformation(navigation->trafficInformation, key,
                                                        textBuffer, textBufferSize, phoneticsData, phoneticsDataSize, duration);
    }
    if (err != NE_OK && navigation->guidanceInformation)
    {
        err = NB_GuidanceInformationGetPronunInformation(navigation->guidanceInformation, key,
                                                         textBuffer, textBufferSize, duration);
        if(isBaseVoices)
        {
            *isBaseVoices = TRUE;
        }
        if (phoneticsDataSize)
        {
            *phoneticsDataSize = 0;
        }
    }
    
    return err;
}

NB_DEF NB_Error NB_NavigationGetTraveledDistance(NB_Navigation* navigation, double* traveledDistance)
{
    double currentTraveledDistance = 0.0;
    if (navigation && navigation->navState && navigation->navState->navRoute && traveledDistance)
    {
        currentTraveledDistance =  NB_RouteInformationTraveledDistance(
                                                       NB_ContextGetDataState(navigation->context),
                                                       &navigation->navState->navRoute->dataReply,
                                                       navigation->navState->currentRoutePosition.closest_maneuver,
                                                       navigation->navState->currentRoutePosition.closest_segment,
                                                       navigation->navState->currentRoutePosition.segment_remain);
        *traveledDistance = navigation->traveledDistance + currentTraveledDistance;
        return NE_OK;
    }
    return NE_INVAL;
}

NB_DEF NB_Error NB_NavigationSetTrafficSetting(NB_Navigation* navigation,NB_TrafficSetting* setting)
{
    if (!navigation)
    {
        return NE_INVAL;
    }

    return NB_TrafficProcessorStateSetSetting(navigation->trafficState, setting);
}

static void
NavigationSpecialRegionCallbackFunction(NB_SpecialRegionStateData* specialRegionStateData, void* userData)
{
    NB_Navigation* pNavigation = (NB_Navigation*)userData;
    if (pNavigation == NULL || specialRegionStateData == NULL)
    {
        return;
    }
    pNavigation->callbacks.specialRegionCallback(specialRegionStateData, pNavigation->callbacks.userData);
}

NB_Error
NB_PlayTBTToStaticMessage(NB_Navigation* navigation)
{
    NB_Error error = NE_OK;
    if (navigation->guidanceInformation)
    {
        NB_GuidanceMessage* message = NULL;
        NB_RouteDataSourceOptions dataSourceOptions = {0};

        dataSourceOptions.route = navigation->navState->navRoute;
        dataSourceOptions.collapse = FALSE;
        dataSourceOptions.maneuverPos = NB_NMP_SwitchToStaticMode;
        dataSourceOptions.trafficInfo = NULL;
        dataSourceOptions.currentRoutePosition = NULL;
        error = NB_GuidanceInformationGetNavigationModeSwitchToStaticAnnouncement(
        navigation->guidanceInformation, NB_RouteInformationDataSource,
        &dataSourceOptions, NB_NAT_Tone, navigation->announceUnits, &message);

        if (message != NULL)
        {
            navigation->navState->currentMessage = message;
            PlayGuidanceMessage(navigation, message, TRUE);
        }
    }
    return error;
}

NB_DEF NB_Error
NB_NavigationGetTrafficColor(NB_Navigation* navigation, char* color)
{
    double delayPercent = 0;
    uint32 tripRemainTime = 0;
    uint32 tripRemainDelay = 0;
    if (!navigation || !navigation->navState || !navigation->navState->navRoute || !color)
    {
        return NE_INVAL;
    }

    NB_NavigationGetTripRemainingInformation(navigation, &tripRemainTime, &tripRemainDelay, NULL);
    delayPercent = (double)(tripRemainTime - tripRemainDelay);
    if (delayPercent != 0)
    {
        delayPercent = tripRemainDelay / delayPercent;
    }

    *color = get_traffic_color(delayPercent * 100, navigation->navState->navRoute->trafficDataValid);
    return NE_OK;
}

static void
NB_NavigationManualMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_NavigationState* navigationState = messagePlayedCallbackData ? messagePlayedCallbackData->navigationState : NULL;
    NB_NavigationMessagePlayed(navigationState, message);
}

static NB_Error
PlayManualMessage(NB_Navigation* navigation, NB_GuidanceMessage* pMessage)
{
    if (!navigation->navState)
    {
        return NE_INVAL;
    }

    if (pMessage)
    {
        // set the manual message into currentMessage, it is used to avoid other announce break it.
        navigation->navState->currentMessage = pMessage;
        navigation->navState->messageCallbackData.navigationState = navigation->navState;
        NB_GuidanceMessageSetPlayedCallback(pMessage, NB_NavigationManualMessagePlayedCallback, &navigation->navState->messageCallbackData);

        if (navigation->callbacks.guidanceMessageCallback != NULL)
        {
            (navigation->callbacks.guidanceMessageCallback)(navigation, pMessage, navigation->callbacks.userData);
        }
        else
        {
            NB_GuidanceMessageDestroy(pMessage);
        }
        return NE_OK;
    }
    else
    {
        return NE_NOMEM;
    }
}

NB_DEF NB_Error
NB_NavigationPlayManualGuidanceMessage(NB_Navigation* navigation, uint32 maneuverIndex,
                                       NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units)
{
    NB_GuidanceMessage* pMessage = NULL;
    uint32 maneuverCount = 0;
    NB_Error error = NE_INVAL;

    if (!navigation)
    {
        return error;
    }

    maneuverCount = NB_RouteInformationGetManeuverCount(navigation->navState->navRoute);
    if (maneuverIndex == NAV_MANEUVER_CURRENT || maneuverIndex == NAV_MANEUVER_START)
    {
        error = NB_NavigationGetManualGuidanceMessage(navigation, NB_NAT_Street, units, TRUE, &pMessage);
    }
    else if (maneuverIndex <= (int)maneuverCount)
    {
        error = NB_NavigationGetManualGuidanceMessageEx(navigation, maneuverIndex, TRUE, NB_NAT_Street, units, TRUE, &pMessage);
    }

    return error == NE_OK ? PlayManualMessage(navigation, pMessage) : error;
}


NB_DEF NB_Error
NB_NavigationPlayManualGuidanceMessageForManeuverList(NB_Navigation* navigation, NB_RouteInformation* route,
                                                      uint32 maneuverIndex, NB_NavigateAnnouncementType type,
                                                      NB_NavigateAnnouncementUnits units, double latitude,
                                                      double longitude)
{
    NB_GuidanceMessage* pMessage = NULL;
    NB_Error error = NE_INVAL;

    if (!navigation || route)
    {
        return error;
    }

    error = NB_RouteInformationGetTurnAnnouncementExt(route, navigation->guidanceInformation, maneuverIndex,
                                                      NB_NMP_Turn, NB_NAS_Lookahead, NB_NAT_Street, units,
                                                      &pMessage, latitude, longitude);

    return error == NE_OK ? PlayManualMessage(navigation, pMessage) : error;
}

NB_DEF NB_Error
NB_NavigationGetTrafficGuidanceInformation(NB_Navigation* navigation, NB_GuidanceMessage* message, double* playtimes,
                                           nb_boolean* isOverAll, nb_boolean* isCongestion, nb_boolean* isInCongestion,
                                           char* trafficColor)
{
    if (!navigation || !navigation->trafficState || !message)
    {
        return NE_INVAL;
    }

    if (NB_GuidanceMessageGetType(message) != NB_GMT_Traffic || NB_GuidanceMessageGetSource(message) != NB_NAS_Automatic)
    {
        return NE_INVAL;
    }

    if (isOverAll)
    {
        *isOverAll = FALSE;
    }
    if (isCongestion)
    {
        *isCongestion = FALSE;
    }
    if (isInCongestion)
    {
        *isInCongestion = FALSE;
    }
    if (trafficColor)
    {
        *trafficColor = '\0';
    }
    if (navigation->trafficState->currentManeuverPos == NB_NMP_TrafficCongestion)
    {
        if (isCongestion)
        {
            *isCongestion = TRUE;
        }
        if (isInCongestion)
        {
            *isInCongestion = navigation->trafficState->nextCongestionIn;
        }
        if (trafficColor)
        {
            switch (navigation->trafficState->nextCongestionSeverity) {
                case NB_NTC_Severe:
                    *trafficColor = 'R';
                    break;
                case NB_NTC_Moderate:
                    *trafficColor = 'Y';
                    break;
                default:
                    *trafficColor = 'G';
                    break;
            }
        }
    }
    else
    {
        if (isOverAll)
        {
            *isOverAll = TRUE;
        }
        if (trafficColor)
        {
            NB_NavigationGetTrafficColor(navigation, trafficColor);
        }
    }

    if (playtimes)
    {
        *playtimes = NB_GuidanceMessageGetPlaybackTime(message);
    }


    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationGetNextMockPoint(NB_Navigation* navigation, uint32* maneuver, uint32* segment, double speed, const NB_GpsLocation* lastLocation, NB_GpsLocation* location)
{
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    double p1_lat = 0;
    double p1_lon = 0;
    double p1_heading = 0;
    double current_lat = 0;
    double current_lon = 0;
    double currentSpeed = speed;
    NB_GpsLocation locationResult;

    if (!navigation || !location || !maneuver || !segment)
    {
        return NE_INVAL;
    }
    if (!navigation->navState->navRoute)
    {
        return NE_INVAL;
    }

    memset(&locationResult, 0, sizeof(NB_GpsLocation));
    dataState = NB_ContextGetDataState(navigation->context);

    if (lastLocation)
    {
        current_lat = lastLocation->latitude;
        current_lon = lastLocation->longitude;
    }

    while (TRUE)
    {
        uint32 segmentCount = 0;

        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &navigation->navState->navRoute->dataReply, *maneuver);
        if (!pmaneuver)
        {
            return NE_NOENT;
        }

        segmentCount = data_polyline_len(dataState, &pmaneuver->polyline);
        while (TRUE)
        {
            if (*segment >= segmentCount - 1)
            {
                ++(*maneuver);
                *segment = 0;
                break; //goto next maneuver
            }

            data_polyline_get(dataState, &pmaneuver->polyline, *segment, &p1_lat, &p1_lon, NULL, &p1_heading);
            if (lastLocation)
            {
                double p2_lat = 0;
                double p2_lon = 0;
                double p2_heading = 0;
                double p1_p2_distance = 0;
                double p1_current_distance = 0;
                data_polyline_get(dataState, &pmaneuver->polyline, *segment+1, &p2_lat, &p2_lon, NULL, &p2_heading);
                p1_p2_distance = NB_SpatialGetLineOfSightDistance(p1_lat, p1_lon, p2_lat, p2_lon, NULL);
                p1_current_distance = NB_SpatialGetLineOfSightDistance(p1_lat, p1_lon, current_lat, current_lon, NULL);
                if (p1_current_distance + currentSpeed > p1_p2_distance)
                {
                    currentSpeed -= (p1_p2_distance - p1_current_distance);
                    ++(*segment);
                    current_lat = p2_lat;
                    current_lon = p2_lon;
                    continue; //goto next segment
                }
                locationResult.latitude = current_lat + (p2_lat - p1_lat) * currentSpeed / p1_p2_distance;
                locationResult.longitude = current_lon + (p2_lon - p1_lon) * currentSpeed / p1_p2_distance;
            }
            else
            {
                locationResult.latitude = p1_lat;
                locationResult.longitude = p1_lon;
            }
            locationResult.heading = p1_heading;
            locationResult.gpsTime = PAL_ClockGetGPSTime();
            locationResult.horizontalVelocity = speed;
            locationResult.valid = NGV_Latitude | NGV_Longitude | NGV_Heading | NGV_HorizontalVelocity | NGV_HorizontalUncertainty;

            nsl_memcpy(location, &locationResult, sizeof(NB_GpsLocation));
            return NE_OK;
        }
    }

    return NE_NOENT;
}

void LogNavigationConfigurationAsJsonFormat(PAL_Instance* pal, NB_NavigationConfiguration* configuration)
{
    if(!pal || ! configuration)
    {
        return;
    }
    NB_JsonStyleLoggerBegin();
        NB_JsonStyleLoggerLogInt("off_route_ignore_count", configuration->offRouteIgnoreCount);
        NB_JsonStyleLoggerLogBoolean("filtered_gps", configuration->filteredGpsEnabled);
        NB_JsonStyleLoggerLogBoolean("camera_search", configuration->cameraSearchEnabled);
        NB_JsonStyleLoggerLogBoolean("name_changed_maneuver", configuration->nameChangedManeuverEnabled);
        NB_JsonStyleLoggerLogBoolean("M1_maneuver", configuration->m1ManeuverEnabled);
        NB_JsonStyleLoggerLogBoolean("continue_on_maneuver", configuration->continueOnManeuverEnabled);
        NB_JsonStyleLoggerLogInt("traffic_notify_maximum_retries", configuration->trafficNotifyMaximumRetries);
        NB_JsonStyleLoggerLogInt("trafficNotifyDelay", configuration->trafficNotifyDelay);
        NB_JsonStyleLoggerLogInt("complete_rute_maximum_attempts", configuration->completeRouteMaximumAttempts);
        NB_JsonStyleLoggerLogInt("complete_route_request_delay", configuration->completeRouteRequestDelay);
        NB_JsonStyleLoggerLogInt("pronun_request_delay", configuration->pronunRequestDelay);
        NB_JsonStyleLoggerLogInt("camera_search_delay", configuration->cameraSearchDelay);
        NB_JsonStyleLoggerLogInt("camera_search_distance", configuration->cameraSearchDistance);
        NB_JsonStyleLoggerLogInt("startup_region_minimum_size", configuration->startupRegionMinimumSize);
        NB_JsonStyleLoggerLogInt("excessive_recalc_maximum_count", configuration->excessiveRecalcMaximumCount);
        NB_JsonStyleLoggerLogInt("excessive_recalc_time_period", configuration->excessiveRecalcTimePeriod);
        NB_JsonStyleLoggerLogDouble("gps_fix_minimum_speed", configuration->gpsFixMinimumSpeed);
        NB_JsonStyleLoggerLogDouble("startup_region_size_factor", configuration->startupRegionSizeFactor);
        NB_JsonStyleLoggerLogDouble("filtered_gps_minimum_speed", configuration->filteredGpsMinimumSpeed);
        NB_JsonStyleLoggerLogDouble("filtered_gps_constant", configuration->filteredGpsConstant);
    NB_JsonStyleLoggerEnd();

    PAL_File* logFile = NULL;
    PAL_FileOpen(pal, "nav_configuration.log", PFM_Append, &logFile);
    if(logFile)
    {
        uint32 written = 0;
        const char* logHeader = "\n\n=========\nCreate navigation with configuration:\n";
        PAL_FileWrite(logFile, (const uint8*)logHeader, nsl_strlen(logHeader), &written);

        const char* logBuffer = NB_JsonStyleLoggerBuffer();
        PAL_FileWrite(logFile, (const uint8*)logBuffer, nsl_strlen(logBuffer), &written);
        PAL_FileClose(logFile);
    }
    NB_JsonStyleLoggerDestory();
}

/*! @} */
