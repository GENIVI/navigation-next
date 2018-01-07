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


#include "paltypes.h"
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbtrafficprocessor.h"
#include "nbtrafficprocessorprivate.h"
#include "nbtrafficinformationprivate.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstate.h"
#include "nbrouteinformationprivate.h"
#include "nbrouteinformation.h"
#include "navtypes.h"
#include "nbspatial.h"
#include "nbutility.h"
#include "data_traffic_record_identifier.h"
#include "data_traffic_region.h"
#include "nbnavigationstateprivate.h"
#include "nbguidancemessage.h"
#include "nbguidancemessageprivate.h"
#include "nbguidanceinformationprivate.h"
#include "nbqalog.h"
#include "nbnavqalog.h"

static NB_Error NB_TrafficProcessorUpdateTrafficState(NB_NavigationState* navigationState, NB_TrafficState* trafficState, NB_TrafficInformation* information);
static NB_Error NB_TrafficProcessorClearCurrentCongestion(NB_TrafficState* trafficState);
static NB_Error NB_TrafficProcessorClearNextIncident(NB_TrafficState* trafficState);
static NB_Error NB_TrafficCheckCurrentIncident(NB_TrafficState* trafficState, NB_TrafficInformation* trafficInformation);

#if 0
static int NB_TrafficCompareDistanceFunction(const void *p1, const void *p2);
#endif

static void NB_TrafficMessagePlayedCallback(NB_GuidanceMessage* message, void* userData);
static NB_Error NB_TrafficInformationGetTurnAnnouncement(NB_TrafficState* trafficState, NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index, NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);

NB_Error
NB_TrafficProcessorStateCreate(NB_Context* context, NB_TrafficState** trafficState)
{
    NB_TrafficState* pThis = NULL;
    data_util_state* dataState = NULL;

    if (!context)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context        = context;

    pThis->initialDelayCheckPending = TRUE;

    pThis->messageIncidentPending = FALSE;
    pThis->messageCongestionPending = FALSE;

    pThis->currentManeuverPos = NB_NMP_Invalid;
    pThis->prevTrafficMeter = 0;
    pThis->distToCongestion = -1;
    pThis->distToIncident = -1;

    dataState = NB_ContextGetDataState(context);

    data_place_init(dataState, &pThis->nextIncidentPlace);

    data_traffic_region_init(dataState, &pThis->nextCongestionRegion);
    data_traffic_region_init(dataState, &pThis->nextCongestionEndRegion);

    data_blob_init(dataState, &pThis->trafficPronoun);

    NB_TrafficInformationClearTrafficEvent(&pThis->nextTrafficEvent);

    NB_TrafficProcessorClearCurrentCongestion(pThis);
    NB_TrafficProcessorClearNextIncident(pThis);

    *trafficState = pThis;

    return NE_OK;
}

NB_Error
NB_TrafficProcessorStateDestroy(NB_TrafficState* trafficState)
{
    data_util_state* dataState = NULL;

    if (!trafficState)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(trafficState->context);

    data_place_free(dataState, &trafficState->nextIncidentPlace);

    data_traffic_region_free(dataState, &trafficState->nextCongestionRegion);
    data_traffic_region_free(dataState, &trafficState->nextCongestionEndRegion);

    data_blob_free(dataState, &trafficState->trafficPronoun);

    nsl_free(trafficState);

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficProcessorUpdate(NB_TrafficState* trafficState, NB_NavigationState* navigationState,  NB_TrafficInformation* trafficInformation, NB_PositionRoutePositionInformation* currentPosition)
{
    if (trafficInformation->hasNewInformation)
    {
        NB_TrafficCheckCurrentIncident(trafficState, trafficInformation);
        trafficInformation->hasNewInformation = FALSE;
    }

    // update traffic events with current position and get first pending traffic event for traffic state
    NB_TrafficInformationUpdateTrafficEventsState(trafficInformation, currentPosition);
    NB_TrafficInformationGetFirstTrafficEvent(trafficInformation, currentPosition->closestManeuver, &trafficState->nextTrafficEvent);

    NB_TrafficProcessorUpdateTrafficState(navigationState, trafficState, trafficInformation);

    return NE_OK;
}

NB_Error
NB_TrafficCheckCurrentIncident(NB_TrafficState* trafficState, NB_TrafficInformation* trafficInformation)
{
    NB_Error err = NE_OK;
    data_traffic_incidents* trafficIncidents;

    if (!trafficInformation)
    {
        return NE_INVAL;
    }

    trafficIncidents = NB_TrafficInformationGetTrafficIncidents(trafficInformation);
    if (!trafficIncidents)
    {
        return NE_INVAL;
    }

    // Clear next incident if it is not the first incident in the incident list
    if (CSL_VectorGetLength(trafficIncidents->vec_place) > 0)
    {
        data_util_state* dataState = NB_ContextGetDataState(trafficState->context);
        data_place* place = CSL_VectorGetPointer(trafficIncidents->vec_place, 0);
        if (!data_place_equal(dataState, place, &trafficState->nextIncidentPlace))
        {
            err = NB_TrafficProcessorClearNextIncident(trafficState);
        }
    }

    return err;
}

NB_Error
NB_TrafficProcessorUpdateTrafficState(NB_NavigationState* navigation, NB_TrafficState* trafficState, NB_TrafficInformation* information)
{
    NB_Error err = NE_OK;
    data_traffic_region start_rgn;
    data_traffic_region end_rgn;
    NB_NavigateTrafficCongestionType severity = NB_NTC_Unknown;
    double nextCongestionRouteRemain = 0.0;
    double incidentRouteRemain = 0.0;
    double speed= 0.0;
    uint32 nman = 0;
    double length = 0.0;
    boolean in_congestion = FALSE;
    data_util_state* dataState = NULL;
    data_nav_maneuver* maneuver = NULL;
    double routeDist = 0.0;
    double routeRemain = 0.0;
    uint32 routeRemainTime = 0;
    uint32 routeRemainDelay = 0;
    double maneuverDistanceOffset = 0.0;
    NB_RoutePrivateConfiguration* privateConfig;
    double trafficMeter = 0;
    uint32 endMan = 0;
    data_traffic_flow* realTimeFlow=NULL;
    data_traffic_flow* historicTimeFlow=NULL;
    data_traffic_incidents* incidentList = NULL;

    dataState = NB_ContextGetDataState(navigation->context);

    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigation->navRoute);

    routeRemain = navigation->remainingRouteDist;

    NB_RouteInformationGetSummaryInformation(navigation->navRoute, NULL, NULL, &routeDist);

    endMan = NB_RouteInformationGetManeuverCount(navigation->navRoute);

    if (navigation->currentRoutePosition.closest_maneuver != NAV_MANEUVER_START && navigation->currentRoutePosition.closest_maneuver != NAV_MANEUVER_NONE)
    {
        maneuver = NB_RouteInformationGetManeuver(navigation->navRoute, navigation->currentRoutePosition.closest_maneuver);
        maneuverDistanceOffset = maneuver->distance - navigation->currentRoutePosition.maneuver_distance_remaining;
    }

    NB_TrafficInformationGetSummary(information, navigation->navRoute, navigation->currentRoutePosition.closest_maneuver,
        maneuverDistanceOffset, endMan, &trafficMeter, NULL, &routeRemainTime, &routeRemainDelay);

    trafficState->remainingTravelTime = routeRemainTime;
    trafficState->remainingTravelDelay = routeRemainDelay;
    trafficState->remainingTravelMeter = trafficMeter;

    NB_QaLogTrafficState(navigation->context, navigation->currentRoutePosition.closest_maneuver, maneuverDistanceOffset,
                         trafficState->remainingTravelTime, trafficState->remainingTravelDelay, trafficState->remainingTravelMeter );

    if (trafficState->initialDelayCheckPending)
    {
        trafficState->initialDelayCheckPending = FALSE;
        if (trafficMeter >= 0 && trafficMeter <= privateConfig->prefs.maxTrafficMeterModerateTraffic)
        {
            trafficState->currentManeuverPos = NB_NMP_TrafficDelay;
            trafficState->messageAlertPending = TRUE;
        }
    }
    else if (trafficState->prevTrafficMeter >= 0.0 && trafficMeter >=0.0 && trafficMeter + privateConfig->prefs.minTrafficMeterChangeToAnnounce <= trafficState->prevTrafficMeter && routeRemainTime > privateConfig->prefs.minRemainTrafficUpdateAnnounce)
    {
        trafficState->currentManeuverPos = NB_NMP_TrafficAlert;
        trafficState->messageAlertPending = TRUE;
    }

    trafficState->prevTrafficMeter = trafficMeter;
    if (trafficState->messageAlertPending)
    {
        return NE_OK;
    }

    err = data_traffic_region_init(dataState, &start_rgn);
    if (err)
    {
        goto errexit;
    }

    err = data_traffic_region_init(dataState, &end_rgn);
    if (err)
    {
        goto errexit;
    }

    severity = NB_NTC_None;
    nextCongestionRouteRemain = -1;
    speed = -1;

    NB_TrafficInformationGetTrafficFlows(information, NULL, &historicTimeFlow, &realTimeFlow );

    if (CSL_VectorGetLength(realTimeFlow->vec_traffic_flow_items) > 0 ||
        CSL_VectorGetLength(historicTimeFlow->vec_traffic_flow_items) > 0)
    {
        err = NB_NavigationFindNextCongestion(navigation, information, &start_rgn, &end_rgn, &severity, &nextCongestionRouteRemain, &length, &speed, &nman, &in_congestion);

        // it is OK if there is no congestion
        if (err == NE_NOENT)
        {
            err = NE_OK;
            NB_TrafficProcessorClearCurrentCongestion(trafficState);
        }
        else
        {
            if (!data_traffic_region_equal(dataState, &end_rgn, &trafficState->nextCongestionEndRegion))
            {
                // if the end-point of the congestion changes, it is "new"
                err = err ? err : data_traffic_region_copy(dataState, &trafficState->nextCongestionRegion, &start_rgn);
                err = err ? err : data_traffic_region_copy(dataState, &trafficState->nextCongestionEndRegion, &end_rgn);

                trafficState->nextCongestionMessageReported = FALSE;
                trafficState->messageCongestionPending = FALSE;
            }
            else if (!data_traffic_region_equal(dataState, &start_rgn, &trafficState->nextCongestionRegion))
            {
                // we are still dealing with the same congestion update the appropriate flags
                err = err ? err : data_traffic_region_copy(dataState, &trafficState->nextCongestionRegion, &start_rgn);
            }
            else if ((!trafficState->nextCongestionIn) && in_congestion)
            {
                // just entered congestion region, warns again
                trafficState->nextCongestionMessageReported = FALSE;
                trafficState->messageCongestionPending = FALSE;
            }

            trafficState->nextCongestionSeverity    = severity;
            trafficState->nextCongestionSpeed       = speed;
            trafficState->nextCongestionRouteRemain = nextCongestionRouteRemain;
            trafficState->nextCongestionManeuver    = nman;
            trafficState->nextCongestionLength      = length;
            trafficState->nextCongestionIn          = in_congestion;
            trafficState->distToCongestion          = routeRemain - trafficState->nextCongestionRouteRemain;

            if (!trafficState->nextCongestionMessageReported)
            {
                if ( (trafficState->nextCongestionRouteRemain >= 0 && trafficState->distToCongestion  >= 0 &&
                    trafficState->distToCongestion  < privateConfig->prefs.navMaxTrafficCongestionAnnounce &&
                    trafficState->distToCongestion  > privateConfig->prefs.navMinTrafficCongestionAnnounce) ||
                    (trafficState->nextCongestionIn && trafficState->nextCongestionLength > privateConfig->prefs.navMinTrafficCongestionLengthAnnounce) )
                {
                    trafficState->messageCongestionPending = TRUE;

                    // do not warn on this congestion any more
                    trafficState->nextCongestionMessageReported = TRUE;

                    NB_QaLogTrafficCongestionWarning(navigation->context,
                        data_string_get(NB_ContextGetDataState(navigation->context), &trafficState->nextCongestionRegion.location),
                        trafficState->distToCongestion,
                        trafficState->nextCongestionSeverity,
                        trafficState->nextCongestionSpeed);

                    NB_QaLogGuidanceMessageTrigger(navigation->context, NB_NAS_Automatic, NB_GMT_Traffic,
                        privateConfig->prefs.navMaxTrafficCongestionAnnounce, trafficState->distToCongestion, -1, -1);
                }
            }

            if (trafficState->messageCongestionPending)
            {
                trafficState->currentManeuverPos = NB_NMP_TrafficCongestion;
            }
        }
    }

    // clear the incident if we pass it
    if (trafficState->nextIncidentRouteRemain > routeRemain)
    {
        NB_TrafficProcessorClearNextIncident(trafficState);
        trafficState->messageIncidentPending = FALSE;
        trafficState->nextIncidentMessageReported = FALSE;
    }

    incidentList = NB_TrafficInformationGetTrafficIncidents(information);
    if (incidentList)
    {
        data_place* place = 0;
        uint32 n = 0;
        uint32 l = CSL_VectorGetLength(incidentList->vec_place);

        for (n = 0; n < l; n++)
        {
            place = CSL_VectorGetPointer(incidentList->vec_place, n);
            if (place->traffic_incident.distance_from_start != -1)
            {
                incidentRouteRemain = routeDist - place->traffic_incident.distance_from_start;

                // check if this incident is behind us
                if (incidentRouteRemain > routeRemain)
                {
                    continue;
                }

                if (!trafficState->nextIncidentMessageReported || incidentRouteRemain > trafficState->nextIncidentRouteRemain)
                {
                   /* got a new traffic incident */
                   err = data_place_copy(dataState, &trafficState->nextIncidentPlace, place);

                   SetNIMTrafficIncidentDetailsFromTrafficIncident(&trafficState->nextIncidentDetails, dataState, &place->traffic_incident);

                   trafficState->nextIncidentRouteRemain = incidentRouteRemain;
                   trafficState->distToIncident = routeRemain - trafficState->nextIncidentRouteRemain;

                    if (trafficState->distToIncident >= 0 &&
                        trafficState->distToIncident< privateConfig->prefs.navMaxTrafficIncidentAnnounce &&
                        trafficState->distToIncident > privateConfig->prefs.navMinTrafficIncidentAnnounce)
                    {
                        trafficState->messageIncidentPending = TRUE;

                        // do not warn on this incident any more
                        trafficState->nextIncidentMessageReported = TRUE;

                        NB_QaLogTrafficIncidentWarning(navigation->context,
                            trafficState->distToIncident,
                            trafficState->nextIncidentPlace.location.point.lat,
                            trafficState->nextIncidentPlace.location.point.lon);

                        NB_QaLogGuidanceMessageTrigger(navigation->context, NB_NAS_Automatic, NB_GMT_Traffic,
                            privateConfig->prefs.navMaxTrafficIncidentAnnounce, trafficState->distToIncident, -1, -1);
                    }
                }

                if (trafficState->messageIncidentPending)
                {
                    trafficState->currentManeuverPos = NB_NMP_TrafficIncident;
                }
            }
        }

        /* update distToIndicent as routeRemain is updated */
        if (trafficState->nextIncidentRouteRemain >= 0)
        {
            trafficState->distToIncident = routeRemain - trafficState->nextIncidentRouteRemain;
        }
        else
        {
            trafficState->distToIncident = -1;
        }

    }

errexit:
    data_traffic_region_free(dataState, &start_rgn);
    data_traffic_region_free(dataState, &end_rgn);

    return err;
}

NB_Error NB_TrafficProcessorStateReset(NB_TrafficState* trafficState)
{
    if (!trafficState)
        return NE_INVAL;

    trafficState->messageIncidentPending = FALSE;
    trafficState->messageCongestionPending = FALSE;

    trafficState->currentManeuverPos = NB_NMP_Invalid;
    trafficState->prevTrafficMeter = 0;
    trafficState->distToCongestion = -1;
    trafficState->distToIncident = -1;

    NB_TrafficProcessorClearCurrentCongestion(trafficState);
    NB_TrafficProcessorClearNextIncident(trafficState);

    return NE_OK;
}

NB_Error
NB_TrafficProcessorClearCurrentCongestion(NB_TrafficState* trafficState)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    if (!trafficState)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(trafficState->context);

    data_traffic_region_free(dataState, &trafficState->nextCongestionRegion);
    data_traffic_region_free(dataState, &trafficState->nextCongestionEndRegion);

    err = err ? err : data_traffic_region_init(dataState, &trafficState->nextCongestionRegion);
    err = err ? err : data_traffic_region_init(dataState, &trafficState->nextCongestionEndRegion);

    trafficState->nextCongestionSeverity    = NB_NTC_None;
    trafficState->nextCongestionSpeed       = -1;
    trafficState->nextCongestionLength      = -1;
    trafficState->nextCongestionRouteRemain = -1;
    trafficState->nextCongestionManeuver    = NAV_MANEUVER_NONE;
    trafficState->nextCongestionIn          = FALSE;
    trafficState->distToCongestion          = -1;

    return err;
}

NB_Error
NB_TrafficProcessorClearNextIncident(NB_TrafficState* trafficState)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    if (!trafficState)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(trafficState->context);

    data_place_free(dataState, &trafficState->nextIncidentPlace);
    err = data_place_init(dataState, &trafficState->nextIncidentPlace);

    trafficState->nextIncidentRouteRemain     = -1;
    trafficState->distToIncident              = -1;

    return err;
}

#if 0
static int
NB_TrafficCompareDistanceFunction(const void *p1, const void *p2)
{
    const data_place* ti1 = p1;
    const data_place* ti2 = p2;

    if (ti1->traffic_incident.distance_from_start < ti2->traffic_incident.distance_from_start)
    {
        return -1;
    }
    else if (ti1->traffic_incident.distance_from_start > ti2->traffic_incident.distance_from_start)
    {
        return 1;
    }

    return 0;
}
#endif

NB_DEF NB_Error
NB_TrafficProcessorGetNextTrafficEvent(NB_TrafficState* trafficState, NB_TrafficEvent* trafficEvent)
{
    if (!trafficState || !trafficEvent)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(trafficState->context);

    *trafficEvent = trafficState->nextTrafficEvent;

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficProcessorGetNextIncident(NB_TrafficState* trafficState,
                                   NB_Place* incident,
                                   NB_TrafficIncident* incidentDetail,
                                   double* incidentRouteRemain,
                                   double* distToIncident)
{
    data_util_state* dataState = NULL;

    if (!trafficState)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(trafficState->context);

    if (incident)
    {
        SetNIMPlaceFromPlace(incident, dataState, &trafficState->nextIncidentPlace);
    }

    if (incidentDetail)
    {
        *incidentDetail = trafficState->nextIncidentDetails;
    }

    if (incidentRouteRemain)
    {
        *incidentRouteRemain = trafficState->nextIncidentRouteRemain;
    }

    if (distToIncident)
    {
        *distToIncident = trafficState->distToIncident;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficProcessorGetNextCongestionInformation(NB_TrafficState* trafficState,
                                              NB_NavigateTrafficCongestionType* severity,
                                              double* routeRemain,
                                              double* speed,
                                              uint32* maneuver,
                                              double* length,
                                              nb_boolean* inCongestion,
                                              double* distToCongestion)
{
    if (!trafficState)
    {
        return NE_INVAL;
    }

    if (severity)
    {
        *severity = trafficState->nextCongestionSeverity;
    }

    if (routeRemain)
    {
        *routeRemain = trafficState->nextCongestionRouteRemain;
    }

    if (speed)
    {
        *speed = trafficState->nextCongestionSpeed;
    }

    if (maneuver)
    {
        *maneuver = trafficState->nextCongestionManeuver;
    }

    if (length)
    {
        *length = trafficState->nextCongestionLength;
    }

    if (inCongestion)
    {
        *inCongestion = trafficState->nextCongestionIn;
    }

    if (distToCongestion)
    {
        *distToCongestion = trafficState->distToCongestion;
    }

    return NE_OK;
}

NB_DEF nb_boolean
NB_TrafficProcessorIsAnnouncementPending(NB_TrafficState* trafficState)
{
    if (!trafficState || !trafficState->setting.trafficAnnouncementEnabled ||
            (!trafficState->setting.trafficIncidentAnnouncementEnabled && trafficState->messageIncidentPending))
    {
        return FALSE;
    }

    return trafficState->messageCongestionPending || trafficState->messageIncidentPending || trafficState->messageAlertPending;
}


NB_DEF NB_Error NB_TrafficGetPendingMessage(NB_TrafficState* trafficState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation,
                                            NB_TrafficInformation* trafficInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};

    if (message == NULL)
    {
        return NE_INVAL;
    }

    *message = NULL;

    if (!NB_TrafficProcessorIsAnnouncementPending(trafficState))
    {
        return NE_NOENT;
    }

    error = NB_NavigationStateGetRoutePositionInformation(navigationState, &routePositionInformation);

    dataSourceOptions.route = navigationState->navRoute;
    dataSourceOptions.maneuverPos = trafficState->currentManeuverPos;
    dataSourceOptions.collapse = TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.trafficInfo = trafficInformation;
    dataSourceOptions.trafficState = trafficState;

    error = NB_TrafficInformationGetTurnAnnouncement(trafficState, guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions,
        NAV_MANEUVER_NONE, NB_NAS_Automatic, type, units, message);

    if (error == NE_OK && message != NULL)
    {
        navigationState->messageCallbackData.messageState = trafficState;
        navigationState->messageCallbackData.navigationState = navigationState;
        error = NB_GuidanceMessageSetPlayedCallback(*message, NB_TrafficMessagePlayedCallback, &navigationState->messageCallbackData);
    }

    return error;
}

data_blob* NB_TrafficProcessorGetPronoun(NB_TrafficState* trafficState, const char* key)
{
    data_pair* pdp;
    int n,l;
    data_util_state* dataState = NULL;

    if (!trafficState)
    {
        return NULL;
    }

    dataState = NB_ContextGetDataState(trafficState->context);

    l = CSL_VectorGetLength(trafficState->nextIncidentPlace.vec_detail);

    for (n=0; n<l; n++)
    {
        pdp = CSL_VectorGetPointer(trafficState->nextIncidentPlace.vec_detail, n);

        if (data_string_compare_cstr(&pdp->key, key))
        {
            if (data_blob_set_cstr(dataState, &trafficState->trafficPronoun, data_string_get(dataState, &pdp->value)) != NE_OK)
                return NULL;

            return &trafficState->trafficPronoun;
        }
    }

    return NULL;
}

uint32
NB_TrafficProcessorGetNextCongestionManeuverIndex(NB_TrafficState* trafficState)
{
    if (!trafficState)
    {
        return NAV_MANEUVER_NONE;
    }

    return trafficState->nextCongestionManeuver;
}

static void
NB_TrafficMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_TrafficState* trafficState = messagePlayedCallbackData ? messagePlayedCallbackData->messageState : NULL;

    if (!trafficState)
    {
        return;
    }

    if (trafficState->messageIncidentPending)
    {
        trafficState->messageIncidentPending = FALSE;
    }
    else if (trafficState->messageCongestionPending)
    {
        trafficState->messageCongestionPending = FALSE;
    }
    else if (trafficState->messageAlertPending)
    {
        trafficState->messageAlertPending = FALSE;
    }

    NB_NavigationMessagePlayed(messagePlayedCallbackData->navigationState, message);
}

static NB_Error
NB_TrafficInformationGetTurnAnnouncement(NB_TrafficState* trafficState, NB_GuidanceInformation* information,
                                          NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index,
                                          NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type,
                                          NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* turnMessage = NULL;
    InstructionCodes* codes = NULL;
    instructtype istype = it_audio_traffic;
    instructset* voices = NULL;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    err = NB_GuidanceMessageCreate(trafficState->context, source, NB_GMT_Traffic, &turnMessage);

    if (err != NE_OK)
    {
        return err;
    }

    *message = turnMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(turnMessage);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    if ( trafficState->currentManeuverPos == NB_NMP_TrafficAlert || trafficState->currentManeuverPos == NB_NMP_TrafficDelay )
    {
        istype = it_audio;
    }

    NB_GuidanceInformationGetVoices(information, &voices);

    if (err == NE_OK)
    {
        err = nb_instructset_getdata(voices, index, codes, NULL, NULL, NULL,
            istype, type, (source != NB_NAS_Automatic) ? TRUE : FALSE, units, trafficState->currentManeuverPos, NULL);
    }

    if (err != NE_OK)
    {
        if (turnMessage)
        {
            NB_GuidanceMessageDestroy(turnMessage);
            turnMessage = NULL;
            *message = NULL;
        }
    }

    return err;
}

NB_Error
NB_TrafficProcessorGetTripRemainingInformation(NB_TrafficState* trafficState, uint32* time, uint32* delay, double* meter)
{
    if (!trafficState)
    {
        return NE_INVAL;
    }

    if (time)
    {
        *time = trafficState->remainingTravelTime;
    }

    if (delay)
    {
        *delay = trafficState->remainingTravelDelay;
    }

    if (meter)
    {
        *meter = trafficState->remainingTravelMeter;
    }

    return NE_OK;
}


NB_Error NB_TrafficProcessorStateSetSetting(NB_TrafficState* trafficState, NB_TrafficSetting* setting)
{
    if (!trafficState || !setting)
    {
        return NE_INVAL;
    }

    trafficState->setting = *setting;

    return NE_OK;
}
