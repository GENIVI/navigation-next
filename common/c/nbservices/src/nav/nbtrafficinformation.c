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

    @file     nbtrafficinformation.c
    @defgroup trafficinfo TrafficInformation

    Information about the results of a traffic request
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbtrafficinformation.h"
#include "nbtrafficinformationprivate.h"
#include "nbutilityprotected.h"
#include "datautil.h"
#include "vec.h"
#include "nbrouteinformationprivate.h"
#include "nbutility.h"
#include "nbqalog.h"
#include "nbnavigationstateprivate.h"
#include "palclock.h"
#include "palmath.h"
#include "nbspatial.h"

static NB_TrafficInformation* AllocateTrafficInformation(NB_Context* context);
static NB_Error TransferFreeFlowSpeeds(NB_TrafficInformation* trafficInfo, struct CSL_Vector* vec_tfi);
static NB_Error UpdateRealtimeFlow(NB_TrafficInformation* trafficInfo);
static void QaLogIncidents(NB_TrafficInformation* information, data_traffic_incidents* incidents);
static void QaLogFlow(NB_TrafficInformation* pThis, data_traffic_flow* flow);
static void UpdateIncidentDistancesVector(NB_NavigationState* navigationState, struct CSL_Vector* vec_incidents);
static void RemovePassedIncidents(NB_TrafficInformation* pThis, struct CSL_Vector* incidents, double distance);
static int ComparePlaceDistance(const void* left, const void* right);
static NB_Error MergeIncidents(NB_TrafficInformation* destination, NB_TrafficInformation* source, data_util_state* dataState);

#define MIN_SPEED 0.0001

NB_DEF NB_Error
NB_TrafficInformationDestroy(NB_TrafficInformation* information)
{
    data_util_state* dataState = NULL;

    if (!information)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(information->context);

    data_blob_free(dataState, &information->routeId);
    data_traffic_notify_reply_free(dataState, &information->dataReply);
    data_traffic_record_identifier_free(dataState, &information->trafficRecordIdentifier);

    data_traffic_flow_free(dataState, &information->standardTrafficFlow);
    data_traffic_flow_free(dataState, &information->historicalTrafficFlow);
    data_traffic_flow_free(dataState, &information->realtimeTrafficFlow);
    data_traffic_incidents_free(dataState, &information->trafficIncidents);

    if (information->trafficEvents)
    {
        CSL_VectorDealloc(information->trafficEvents);
        information->trafficEvents = NULL;
    }

    if (information->groupedTrafficEvents)
    {
        CSL_VectorDealloc(information->groupedTrafficEvents);
        information->groupedTrafficEvents = NULL;
    }

    nsl_free(information);

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficInformationGetIncident(NB_TrafficInformation* information, uint32 index, NB_Place* place, NB_TrafficIncident* incident, double* distance)
{
    NB_Error error = NE_OK;
    data_util_state* dataState = NB_ContextGetDataState(information->context);
    data_place* dataPlace = NULL;

    if (index >= (uint32) CSL_VectorGetLength(information->trafficIncidents.vec_place))
    {
        return NE_NOENT;
    }

    dataPlace = CSL_VectorGetPointer(information->trafficIncidents.vec_place, (int) index);

    if (dataPlace == NULL)
    {
        return NE_NOENT;
    }

    if (place)
    {
        error = error ? error : SetNIMPlaceFromPlace(place, dataState, dataPlace);
    }

    if (incident)
    {
        error = error ? error : SetNIMTrafficIncidentDetailsFromTrafficIncident(incident, dataState, &dataPlace->traffic_incident);
    }

    if (distance)
    {
        *distance = dataPlace->traffic_incident.distance_from_start;
    }

    return error;
}

NB_DEF uint32
NB_TrafficInformationGetIncidentCount(NB_TrafficInformation* information)
{
    if (!information)
    {
        return 0;
    }

    return CSL_VectorGetLength(information->trafficIncidents.vec_place);
}

static NB_Error
GetDistanceToStart(NB_RouteInformation* route, uint32 startManeuver, double distanceOffset, double* distance)
{
    NB_Error result = NE_OK;
    uint32 i=0;
    double distanceToStart = 0;

    uint32 maneuverCount = 0;
    if ((route == NULL) || (distance == NULL))
    {
        return NE_INVAL;
    }

    if ( (startManeuver == NAV_MANEUVER_NONE) ||
         (startManeuver == NAV_MANEUVER_START))
    {
        startManeuver = 0;
    }

    maneuverCount = NB_RouteInformationGetManeuverCount(route);
    if (startManeuver >= maneuverCount)
    {
        return NE_INVAL;
    }

    for (i=0; i < startManeuver; i++)
    {
        double maneuverDistance = 0;
        result = NB_RouteInformationGetTurnInformation(route, i, NULL,
                NULL, 0, // do not need dest primary
                NULL, 0, // do not need dest secondary
                NULL, 0,    /*  do not need current street here */
                &maneuverDistance, FALSE); // not collapsed

        if (result != NE_OK)
        {
            return result;
        }

        distanceToStart += maneuverDistance;
    }

    distanceToStart += distanceOffset;

    if (distance != NULL)
    {
        *distance = distanceToStart;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficInformationGetSummary(NB_TrafficInformation* information, NB_RouteInformation* route, uint32 startManeuver, double distanceOffset, uint32 endManeuver, double* meter, uint32* incidents, uint32* time, uint32* delay)
{
    NB_Error result = NE_OK;
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;

    if (!route || !information)
    {
        return NE_INVAL;
    }

    navReply = NB_RouteInformationGetDataNavReply(route);

    dataState = NB_ContextGetDataState(route->context);

    if (incidents != NULL)
    {
        int index = 0;
        int count = 0;
        data_place* place = 0;
        double currentPosition = 0.0; // position to start

        result = GetDistanceToStart(route, startManeuver, distanceOffset, &currentPosition);
        if (result != NE_OK)
        {
            return result;
        }

        // If the request is for a single maneuver, increment end maneuver
        if (startManeuver == endManeuver)
        {
            endManeuver++;
        }

        *incidents = 0;

        count = CSL_VectorGetLength(information->trafficIncidents.vec_place);

        for (index = 0; index < count; index++)
        {
            place = CSL_VectorGetPointer(information->trafficIncidents.vec_place, index);
            if (place->traffic_incident.nman_closest >= startManeuver && place->traffic_incident.nman_closest < endManeuver)
            {
                if (place->traffic_incident.distance_from_start >= currentPosition)
                {
                    (*incidents)++;
                }
            }
        }
    }

    data_nav_reply_get_traffic_info(dataState, navReply,
        information->navMinTmcSpeed, information->minNoDataDisableTrafficBar,
        startManeuver, distanceOffset, endManeuver, TrafficSpeedAll,
        delay, meter, time, (nav_speed_cb)NB_TrafficInformationGetSpeed, information);

    return NE_OK;
}

static NB_Error
TransferFreeFlowSpeeds(NB_TrafficInformation* trafficInfo, struct CSL_Vector* vec_tfi)
{
    NB_Error err = NE_OK;
    data_traffic_flow_item* ptfi;
    uint32 n, l;
    double ffspeed;

    data_util_state* dataState = NULL;

    if (!trafficInfo)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(trafficInfo->context);

    l = CSL_VectorGetLength(vec_tfi);

    for (n=0;n<l;n++)
    {
        ptfi = CSL_VectorGetPointer(vec_tfi, n);

        if (ptfi->free_flow_speed == -1)
        {
            if ( data_traffic_flow_get_speed(dataState, &trafficInfo->realtimeTrafficFlow, trafficInfo->navMinTmcSpeed, data_string_get(dataState, &ptfi->location), NULL, NULL, &ffspeed) == NE_OK )

            {
                    ptfi->free_flow_speed = ffspeed;
            }
        }
    }

    return err;
}

static NB_Error
UpdateRealtimeFlow(NB_TrafficInformation* trafficInfo)
{
    NB_Error err = NE_OK;
    uint32 n, l;
    data_traffic_flow_item* ptfi;

    data_util_state* dataState = NULL;

    if (!trafficInfo)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(trafficInfo->context);

    err = err ? err : TransferFreeFlowSpeeds(trafficInfo, trafficInfo->dataReply.traffic_record_identifier.new_realtime_traffic_flow.vec_traffic_flow_items);
    err = err ? err : TransferFreeFlowSpeeds(trafficInfo, trafficInfo->dataReply.traffic_record_identifier.old_realtime_traffic_flow.vec_traffic_flow_items);

    DATA_VEC_FREE(dataState, trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items, data_traffic_flow_item);
    DATA_VEC_ALLOC(err, trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items, data_traffic_flow_item);

    DATA_VEC_COPY(dataState, err, trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items,
        trafficInfo->dataReply.traffic_record_identifier.new_realtime_traffic_flow.vec_traffic_flow_items,
        data_traffic_flow_item);

    l = CSL_VectorGetLength(trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items);

    for (n=0;n<l;n++)
    {
        ptfi = CSL_VectorGetPointer(trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items, n);
        ptfi->new_item = TRUE;
    }

    DATA_VEC_COPY(dataState, err, trafficInfo->realtimeTrafficFlow.vec_traffic_flow_items,
        trafficInfo->dataReply.traffic_record_identifier.old_realtime_traffic_flow.vec_traffic_flow_items,
        data_traffic_flow_item);

    trafficInfo->hasNewInformation = TRUE;

    return err;
}

static int
CompareTrafficEventDistanceToRouteEndForDescendingSort(const void* left, const void* right)
{
    const NB_TrafficEvent* eventLeft = left;
    const NB_TrafficEvent* eventRight= right;

    if (eventLeft->routeRemainAfterEvent < eventRight->routeRemainAfterEvent)
    {
        return 1;
    }
    else if (eventLeft->routeRemainAfterEvent > eventRight->routeRemainAfterEvent)
    {
        return -1;
    }

    return 0;
}

static NB_Error
CollectCongestionTrafficEvents(
    struct CSL_Vector* trafficEventList,
    NB_RouteInformation* routeInformation,
    NB_TrafficInformation* trafficInformation,
    data_util_state* dataState,
    double routeDistanceOffset,
    double routeTotalDistance,
    nb_boolean grouped)
{
    NB_Error error = NE_OK;

    data_nav_reply* navReply = NULL;

    uint32 maneuverCount = 0;
    uint32 maneuverIndex = 0;

    double maneuverDistanceFromRouteStart = 0.0;

    NB_NavigateTrafficCongestionType overallCongestionType = NB_NTC_None;
    NB_TrafficEvent congestionEvent = { 0 };

    double overallCongestionDistance = 0.0;
    double overallCongestionTime = 0.0;
    double overallCongestionFreeFlowTime = 0.0;

    navReply = NB_RouteInformationGetDataNavReply(routeInformation);

    if (!navReply)
    {
        return NE_INVAL;
    }

    // collect all traffic congestion events
    maneuverCount = NB_RouteInformationGetManeuverCount(routeInformation);
    for (maneuverIndex = 0; maneuverIndex < maneuverCount; maneuverIndex++)
    {
        data_nav_maneuver* maneuver = NULL;

        uint32 regionCount = 0;
        uint32 regionIndex = 0;
        uint32 polylineIndex = 0;
        double polylineLen = 0;
        double trafficOffset = 0;

        uint32 updateGpsTimeSeconds = PAL_ClockGetGPSTime();

        maneuver = data_nav_reply_get_nav_maneuver(dataState, navReply, maneuverIndex);

        regionCount = CSL_VectorGetLength(maneuver->vec_traffic_regions);
        for (regionIndex = 0; regionIndex < regionCount; regionIndex++)
        {
            data_traffic_region* trafficRegion = NULL;

            trafficRegion = (data_traffic_region*) CSL_VectorGetPointer(maneuver->vec_traffic_regions, regionIndex);
            if (trafficRegion != NULL)
            {
                NB_NavigateTrafficCongestionType regionCongestionType = NB_NTC_None;
                double regionSpeed = 0;
                double freeFlowRegionSpeed = 0;

                error = NB_TrafficInformationGetTrafficRegionRealTimeSpeed(
                    trafficInformation,
                    data_string_get(dataState, &trafficRegion->location),
                    &regionSpeed,
                    &freeFlowRegionSpeed,
                    &regionCongestionType);

                if (error == NE_OK)
                {
                    if (regionCongestionType > NB_NTC_None)
                    {
                        double trafficDelay = 0.0;

                        if (overallCongestionType == NB_NTC_None)
                        {
                            overallCongestionType = regionCongestionType;

                            // congestion segment(s) ended; create congestion traffic event
                            congestionEvent.type = NB_TET_Congestion;
                            congestionEvent.maneuverIndex = maneuverIndex;
                            congestionEvent.distanceToEvent = maneuverDistanceFromRouteStart + trafficRegion->start - routeDistanceOffset;
                            congestionEvent.detail.congestion.severity = overallCongestionType;
                            trafficOffset = trafficRegion->start;

                            (void)NB_RouteInformationGetCurrentRoadNames(
                                routeInformation,
                                maneuverIndex,
                                congestionEvent.detail.congestion.road,
                                sizeof(congestionEvent.detail.congestion.road),
                                NULL, 0);

                            polylineLen = 0;
                            for(polylineIndex = 0; polylineIndex < (uint32)maneuver->polyline.numsegments; polylineIndex++)
                            {
                                if (polylineLen - trafficOffset >= 0)
                                {
                                    congestionEvent.place.location.latitude = maneuver->polyline.segments[polylineIndex].lat/POLYLINE_LL_SCALE_FACTOR;
                                    congestionEvent.place.location.longitude = maneuver->polyline.segments[polylineIndex].lon/POLYLINE_LL_SCALE_FACTOR;
                                    break;
                                }
                                polylineLen += maneuver->polyline.segments[polylineIndex].len / POLYLINE_LEN_SCALE_FACTOR;
                            }
                        }

                        if (overallCongestionType == regionCongestionType)
                        {
                            overallCongestionDistance += trafficRegion->length;
                            overallCongestionTime += trafficRegion->length / MAX(regionSpeed, MIN_SPEED);
                            overallCongestionFreeFlowTime += trafficRegion->length / MAX(freeFlowRegionSpeed, MIN_SPEED);

                            congestionEvent.routeRemainAfterEvent =  routeTotalDistance - (routeDistanceOffset + congestionEvent.distanceToEvent);
                            congestionEvent.detail.congestion.length = overallCongestionDistance;
                            congestionEvent.detail.congestion.speed = overallCongestionDistance / MAX(overallCongestionTime, MIN_SPEED);
                            trafficDelay = overallCongestionTime - overallCongestionFreeFlowTime;
                            congestionEvent.detail.congestion.delaySeconds = (trafficDelay > 0.0) ? (uint32)trafficDelay : 0;
                            congestionEvent.detail.congestion.updateGpsTimeSeconds = updateGpsTimeSeconds;
                        }
                        else
                        {
                            if (grouped)
                            {
                                if (!CSL_VectorAppend(trafficInformation->groupedTrafficEvents, &congestionEvent))
                                {
                                    return NE_NOMEM;
                                }
                            }
                            else
                            {
                                if (!CSL_VectorAppend(trafficInformation->trafficEvents, &congestionEvent))
                                {
                                    return NE_NOMEM;
                                }
                            }

                            overallCongestionType = regionCongestionType;
                            overallCongestionDistance = trafficRegion->length;
                            overallCongestionTime = trafficRegion->length / MAX(regionSpeed, MIN_SPEED);
                            overallCongestionFreeFlowTime = trafficRegion->length / MAX(freeFlowRegionSpeed, MIN_SPEED);

                            congestionEvent.type = NB_TET_Congestion;
                            congestionEvent.maneuverIndex = maneuverIndex;
                            congestionEvent.distanceToEvent = maneuverDistanceFromRouteStart + trafficRegion->start - routeDistanceOffset;
                            congestionEvent.detail.congestion.severity = overallCongestionType;
                            trafficOffset = trafficRegion->start;
                            congestionEvent.routeRemainAfterEvent =  routeTotalDistance - (routeDistanceOffset + congestionEvent.distanceToEvent);
                            congestionEvent.detail.congestion.length = overallCongestionDistance;
                            congestionEvent.detail.congestion.speed = overallCongestionDistance / MAX(overallCongestionTime, MIN_SPEED);
                            trafficDelay = overallCongestionTime - overallCongestionFreeFlowTime;
                            congestionEvent.detail.congestion.delaySeconds = (trafficDelay > 0.0) ? (uint32)trafficDelay : 0;
                            congestionEvent.detail.congestion.updateGpsTimeSeconds = updateGpsTimeSeconds;

                            (void)NB_RouteInformationGetCurrentRoadNames(
                                                                         routeInformation,
                                                                         maneuverIndex,
                                                                         congestionEvent.detail.congestion.road,
                                                                         sizeof(congestionEvent.detail.congestion.road),
                                                                         NULL, 0);

                            polylineLen = 0;
                            for(polylineIndex = 0; polylineIndex < (uint32)maneuver->polyline.numsegments; polylineIndex++)
                            {
                                if (polylineLen - trafficOffset >= 0)
                                {
                                    congestionEvent.place.location.latitude = maneuver->polyline.segments[polylineIndex].lat/POLYLINE_LL_SCALE_FACTOR;
                                    congestionEvent.place.location.longitude = maneuver->polyline.segments[polylineIndex].lon/POLYLINE_LL_SCALE_FACTOR;
                                    break;
                                }
                                polylineLen += maneuver->polyline.segments[polylineIndex].len / POLYLINE_LEN_SCALE_FACTOR;
                            }
                        }
                    }
                }

                if (overallCongestionType > NB_NTC_None
                    && (regionCongestionType <= NB_NTC_None ||
                          (!grouped && regionIndex + 1 == regionCount)))
                {
                    // append traffic event
                    if (grouped)
                    {
                        if (!CSL_VectorAppend(trafficInformation->groupedTrafficEvents, &congestionEvent))
                        {
                            return NE_NOMEM;
                        }
                    }
                    else
                    {
                        if (!CSL_VectorAppend(trafficInformation->trafficEvents, &congestionEvent))
                        {
                            return NE_NOMEM;
                        }
                    }

                    // reset for next congestion
                    overallCongestionType = NB_NTC_None;
                    overallCongestionDistance = 0.0;
                    overallCongestionTime = 0.0;
                    overallCongestionFreeFlowTime = 0.0;
                    NB_TrafficInformationClearTrafficEvent(&congestionEvent);
                }
            }
        }

        maneuverDistanceFromRouteStart += maneuver->distance;
    }

    if (grouped && overallCongestionType > NB_NTC_None)
    {
        if (!CSL_VectorAppend(trafficInformation->groupedTrafficEvents, &congestionEvent))
        {
            return NE_NOMEM;
        }
    }

    return NE_OK;
}

static NB_Error
CollectIncidentTrafficEvents(
    struct CSL_Vector* trafficEventList,
    NB_TrafficInformation* trafficInformation,
    data_util_state* dataState,
    double routeDistanceOffset,
    double routeTotalDistance)
{
    uint32 incidentCount = 0;
    uint32 incidentIndex = 0;

    // collect all traffic incident events
    incidentCount = CSL_VectorGetLength(trafficInformation->trafficIncidents.vec_place);
    for (incidentIndex = 0; incidentIndex < incidentCount; incidentIndex++)
    {
        NB_TrafficEvent incidentEvent = { 0 };

        data_place* incidentDataPlace = 0;

        incidentDataPlace = CSL_VectorGetPointer(trafficInformation->trafficIncidents.vec_place, (int)incidentIndex);
        if (incidentDataPlace)
        {
            (void)SetNIMPlaceFromPlace(&incidentEvent.place, dataState, incidentDataPlace);
            (void)SetNIMTrafficIncidentDetailsFromTrafficIncident(&incidentEvent.detail.incident, dataState, &incidentDataPlace->traffic_incident);

            // create and append incident traffic event
            incidentEvent.type = NB_TET_Incident;
            incidentEvent.maneuverIndex = incidentDataPlace->traffic_incident.nman_closest;
            incidentEvent.routeRemainAfterEvent = routeTotalDistance - incidentDataPlace->traffic_incident.distance_from_start;
            incidentEvent.distanceToEvent = incidentDataPlace->traffic_incident.distance_from_start - routeDistanceOffset;

            // append traffic event
            if (!CSL_VectorAppend(trafficInformation->trafficEvents, &incidentEvent))
            {
                return NE_NOMEM;
            }
            if (!CSL_VectorAppend(trafficInformation->groupedTrafficEvents, &incidentEvent))
            {
                return NE_NOMEM;
            }
        }
    }

    return NE_OK;
}

static NB_Error
CreateTrafficEventList(NB_TrafficInformation* trafficInformation, NB_RouteInformation* routeInformation, double routeDistanceOffset)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;
    double routeTotalDistance = 0.0;

    if (!trafficInformation || !routeInformation)
    {
        return NE_INVAL;
    }

    if (trafficInformation->trafficEvents)
    {
        // clear existing traffic event collection
        CSL_VectorRemoveAll(trafficInformation->trafficEvents);
    }
    else
    {
        // create traffic event collection
        trafficInformation->trafficEvents = CSL_VectorAlloc(sizeof(NB_TrafficEvent));
        if (!trafficInformation->trafficEvents)
        {
            return NE_NOMEM;
        }
    }

    if (trafficInformation->groupedTrafficEvents)
    {
        // clear existing traffic event collection
        CSL_VectorRemoveAll(trafficInformation->groupedTrafficEvents);
    }
    else
    {
        // create traffic event collection
        trafficInformation->groupedTrafficEvents = CSL_VectorAlloc(sizeof(NB_TrafficEvent));
        if (!trafficInformation->groupedTrafficEvents)
        {
            return NE_NOMEM;
        }
    }

    dataState = NB_ContextGetDataState(trafficInformation->context);

    (void)NB_RouteInformationGetSummaryInformation(routeInformation, NULL, NULL, &routeTotalDistance);

    err = CollectCongestionTrafficEvents(
        trafficInformation->trafficEvents,
        routeInformation,
        trafficInformation,
        dataState,
        routeDistanceOffset,
        routeTotalDistance,
        FALSE);
    if (err != NE_OK)
    {
        return err;
    }

    err = CollectCongestionTrafficEvents(
        trafficInformation->trafficEvents,
        routeInformation,
        trafficInformation,
        dataState,
        routeDistanceOffset,
        routeTotalDistance,
        TRUE);
    if (err != NE_OK)
    {
        return err;
    }

    err = CollectIncidentTrafficEvents(
        trafficInformation->trafficEvents,
        trafficInformation,
        dataState,
        routeDistanceOffset,
        routeTotalDistance);
    if (err != NE_OK)
    {
        return err;
    }

    // sort traffic events
    CSL_VectorSort(trafficInformation->trafficEvents, CompareTrafficEventDistanceToRouteEndForDescendingSort);

    // sort grouped traffic events
    CSL_VectorSort(trafficInformation->groupedTrafficEvents, CompareTrafficEventDistanceToRouteEndForDescendingSort);

    // qa log newly collected traffic event list
    if (NB_ContextGetQaLog(trafficInformation->context))
    {
        uint32 eventCount = (int)CSL_VectorGetLength(trafficInformation->trafficEvents);
        uint32 eventIndex = 0;

        for (eventIndex = 0; eventIndex < eventCount; eventIndex++)
        {
            NB_TrafficEvent* trafficEvent = (NB_TrafficEvent*)CSL_VectorGetPointer(trafficInformation->trafficEvents, (int)eventIndex);
            if (trafficEvent)
            {
                NB_QaLogTrafficEvent(trafficInformation->context, NB_QLTELO_Add, trafficEvent);
            }
        }
    }

    return NE_OK;
}

NB_Error
NB_TrafficInformationMergeFromNotify(NB_TrafficInformation* destination, NB_TrafficInformation* source, NB_RouteInformation* route, double distance,
                                     int* pNewIncidentCount, int* pClearedIncientCount)
{
    NB_Error err = NE_OK;
    int newIncidentCount = 0;
    int clearedIncidentCount = 0;
    data_util_state* dataState = NULL;

    if (!destination || !source || !source->isNotify)
    {
        return NE_INVAL;
    }

    if (source->replyNotReady)
    {
        return NE_AGAIN;
    }

    dataState = NB_ContextGetDataState(destination->context);

    if (data_string_equal(dataState, &destination->trafficRecordIdentifier.value, &source->dataReply.traffic_record_identifier.value))
    {
        int currentIncidentCount = 0;
        int oldIncidentCount = 0;
        data_place tempSortPlace;

        // Copy the new reply into the destination
        data_traffic_notify_reply_copy(dataState, &destination->dataReply, &source->dataReply);
        data_traffic_record_identifier_copy(dataState, &destination->trafficRecordIdentifier, &source->dataReply.traffic_record_identifier);

        nsl_memset(&tempSortPlace, 0, sizeof(tempSortPlace));
        // Get rid of all of the accidents that are behind the current point in the route
        RemovePassedIncidents(destination, destination->trafficIncidents.vec_place, distance);
        RemovePassedIncidents(source, source->dataReply.traffic_record_identifier.old_traffic_incidents.vec_place, distance);
        RemovePassedIncidents(source, source->dataReply.traffic_record_identifier.new_traffic_incidents.vec_place, distance);

        currentIncidentCount = CSL_VectorGetLength(destination->trafficIncidents.vec_place);
        oldIncidentCount = CSL_VectorGetLength(source->dataReply.traffic_record_identifier.old_traffic_incidents.vec_place);
        newIncidentCount = CSL_VectorGetLength(source->dataReply.traffic_record_identifier.new_traffic_incidents.vec_place);

        if (currentIncidentCount >= oldIncidentCount)
        {
            clearedIncidentCount = currentIncidentCount - oldIncidentCount;
        }
        else
        {
            NB_QaLogAppState(destination->context, "error: new incidents in old list");
        }

        MergeIncidents(destination, source, dataState);
        UpdateRealtimeFlow(destination);

        CSL_VectorSort2(destination->trafficIncidents.vec_place, ComparePlaceDistance, &tempSortPlace);

        err = CreateTrafficEventList(destination, route, distance);
        if (err != NE_OK)
        {
            return err;
        }
    }
    else
    {
        NB_QaLogAppState(destination->context, "tn-reply: wrong session");
    }

    if (pNewIncidentCount)
    {
        *pNewIncidentCount = newIncidentCount;
    }

    if (pClearedIncientCount)
    {
        *pClearedIncientCount = clearedIncidentCount;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_TrafficInformationGetRouteManeuverIncident(NB_TrafficInformation* information, NB_RouteInformation* route, uint32 maneuverIndex, uint32 incidentIndex, NB_Place* place, NB_TrafficIncident* incident, double* distance)
{
    NB_Error error = NE_OK;
    data_util_state* dataState = NULL;
    data_place* dataPlace = NULL;
    int incidentCount = 0;
    uint32 maneuverCount = 0;
    int index = 0;
    uint32 incidentOnManeuver = 0;

    if (!information || !route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(information->context);

    maneuverCount = NB_RouteInformationGetManeuverCount(route);
    if (maneuverIndex >= maneuverCount)
    {
        return NE_NOENT;
    }

    incidentCount = CSL_VectorGetLength(information->trafficIncidents.vec_place);
    for (index = 0; index < incidentCount; index++)
    {
        dataPlace = CSL_VectorGetPointer(information->trafficIncidents.vec_place, (int) index);
        if (dataPlace)
        {
            if (maneuverIndex == dataPlace->traffic_incident.nman_closest)
            {
                if (incidentIndex == incidentOnManeuver)
                {
                    if (place)
                    {
                        error = error ? error : SetNIMPlaceFromPlace(place, dataState, dataPlace);
                    }

                    if (incident)
                    {
                        error = error ? error : SetNIMTrafficIncidentDetailsFromTrafficIncident(incident, dataState, &dataPlace->traffic_incident);
                    }

                    if (distance)
                    {
                        *distance = dataPlace->traffic_incident.distance_from_start;
                    }

                    return error;
                }

                incidentOnManeuver++;
            }
        }
    }

    return NE_NOENT;
}

void
NB_TrafficInformationClearTrafficEvent(NB_TrafficEvent* trafficEvent)
{
    if (trafficEvent)
    {
        nsl_memset(trafficEvent, 0, sizeof(NB_TrafficEvent));
        trafficEvent->type = NB_TET_None;
    }
}

static NB_Error
UpdateTrafficEventsState(NB_Context* context, struct CSL_Vector* trafficEvents, NB_PositionRoutePositionInformation* currentPosition)
{
    NB_TrafficEvent* trafficEvent = NULL;
    uint32 eventCount = 0;
    uint32 eventIndex = 0;

    // don't update traffic event state until on route
    if (currentPosition->closestManeuver == NAV_MANEUVER_START || currentPosition->closestManeuver == NAV_MANEUVER_NONE)
    {
        return NE_OK;
    }

    eventIndex = 0;
    eventCount = CSL_VectorGetLength(trafficEvents);

    // based on current position, update distance to event and remove events that have been passed
    while (eventIndex < eventCount)
    {
        double distanceRemainingToEvent = 0;

        trafficEvent = (NB_TrafficEvent*)CSL_VectorGetPointer(trafficEvents, eventIndex);
        if (trafficEvent)
        {
            distanceRemainingToEvent = currentPosition->remainingRouteDist - trafficEvent->routeRemainAfterEvent;

            // determine if currently in congestion and extend congestion distance remaining to include length
            if (trafficEvent->type == NB_TET_Congestion
                && distanceRemainingToEvent < 0
                && -distanceRemainingToEvent < trafficEvent->detail.congestion.length)
            {
                trafficEvent->detail.congestion.inCongestion = TRUE;
                trafficEvent->distanceToEvent = 0;
                distanceRemainingToEvent += trafficEvent->detail.congestion.length;
            }
            else
            {
                trafficEvent->distanceToEvent = distanceRemainingToEvent;
            }

            // remove events from the event list that have been passed
            if (distanceRemainingToEvent < 0)
            {
                if (NB_ContextGetQaLog(context))
                {
                    NB_QaLogTrafficEvent(context, NB_QLTELO_Remove, trafficEvent);
                }
                CSL_VectorRemove(trafficEvents, eventIndex);
                eventCount--;
            }
            else
            {
                eventIndex++;
            }
        }
    }

    return NE_OK;
}

NB_Error
NB_TrafficInformationUpdateTrafficEventsState(NB_TrafficInformation* trafficInformation, NB_PositionRoutePositionInformation* currentPosition)
{
    NB_Error ret = NE_OK;

    if (!trafficInformation || !currentPosition)
    {
        return NE_INVAL;
    }

    ret = ret ? ret : UpdateTrafficEventsState(trafficInformation->context, trafficInformation->trafficEvents, currentPosition);
    ret = ret ? ret : UpdateTrafficEventsState(trafficInformation->context, trafficInformation->groupedTrafficEvents, currentPosition);

    return ret;
}

NB_Error
NB_TrafficInformationGetFirstTrafficEventInManeuvers(NB_TrafficInformation* information, uint32 startManeuverIndex, uint32 endManeuverIndex, NB_TrafficEvent* trafficEvent)
{
    NB_Error err = NE_NOENT;

    uint32 eventCount = 0;
    uint32 eventIndex = 0;

    if (!information || !trafficEvent)
    {
        return NE_INVAL;
    }

    NB_TrafficInformationClearTrafficEvent(trafficEvent);

    // if start or current specified, just return the first event in the list
    if (startManeuverIndex == NAV_MANEUVER_START ||
        startManeuverIndex == NAV_MANEUVER_CURRENT)
    {
        startManeuverIndex = 0;
    }

    // If the request is for a single maneuver, increment end maneuver
    if (startManeuverIndex == endManeuverIndex)
    {
        endManeuverIndex++;
    }

    eventCount = CSL_VectorGetLength(information->trafficEvents);
    for (eventIndex = 0; eventIndex < eventCount; eventIndex++)
    {
        NB_TrafficEvent* currentEvent = CSL_VectorGetPointer(information->trafficEvents, eventIndex);

        if (!currentEvent || currentEvent->maneuverIndex >= endManeuverIndex)
        {
            break;
        }

        if (currentEvent->maneuverIndex >= startManeuverIndex)
        {
            *trafficEvent = *currentEvent;
            return NE_OK;
        }
    }

    return err;
}

NB_Error
NB_TrafficInformationGetFirstTrafficEvent(NB_TrafficInformation* information, uint32 startManeuverIndex, NB_TrafficEvent* trafficEvent)
{
    NB_Error err = NE_NOENT;
    
    uint32 eventCount = 0;
    uint32 eventIndex = 0;
    
    if (!information || !trafficEvent)
    {
        return NE_INVAL;
    }
    
    NB_TrafficInformationClearTrafficEvent(trafficEvent);
    
    // if start or current specified, just return the first event in the list
    if (startManeuverIndex == NAV_MANEUVER_START ||
        startManeuverIndex == NAV_MANEUVER_CURRENT)
    {
        startManeuverIndex = 0;
    }
    
    eventCount = CSL_VectorGetLength(information->trafficEvents);
    for (eventIndex = 0; eventIndex < eventCount; eventIndex++)
    {
        NB_TrafficEvent* currentEvent = CSL_VectorGetPointer(information->trafficEvents, eventIndex);
        
        if (!currentEvent)
        {
            break;
        }
        
        if (currentEvent->maneuverIndex >= startManeuverIndex)
        {
            *trafficEvent = *currentEvent;
            return NE_OK;
        }
    }
    
    return err;
}

NB_Error
NB_TrafficInformationGetTrafficEvent(NB_TrafficInformation* information, uint32 eventIndex, NB_TrafficEvent* trafficEvent)
{
    NB_TrafficEvent* event = NULL;

    if (!information || !trafficEvent)
    {
        return NE_INVAL;
    }

    NB_TrafficInformationClearTrafficEvent(trafficEvent);

    if (!CSL_VectorGetLength(information->trafficEvents))
    {
        return NE_NOENT;
    }

    event = (NB_TrafficEvent*)CSL_VectorGetPointer(information->trafficEvents, eventIndex);
    if (!event)
    {
        return NE_NOENT;
    }

    *trafficEvent = *event;

    return NE_OK;
}

NB_Error
NB_TrafficInformationGetUpcomingTrafficEvents(NB_TrafficInformation* information, uint32* numberOfTrafficEvents, NB_TrafficEvent** groupedTrafficEvents)
{
    NB_TrafficEvent* events = NULL;
    uint32 eventCount = 0;
    uint32 eventIndex = 0;

    if (!information || !numberOfTrafficEvents || !groupedTrafficEvents)
    {
        return NE_INVAL;
    }

    *numberOfTrafficEvents = 0;
    *groupedTrafficEvents = NULL;

    eventCount = CSL_VectorGetLength(information->groupedTrafficEvents);
    if (!eventCount)
    {
        return NE_NOENT;
    }

    events = nsl_malloc(eventCount * sizeof(NB_TrafficEvent));
    if (!events)
    {
        return NE_NOMEM;
    }

    for (eventIndex = 0; eventIndex < eventCount; eventIndex++)
    {
        events[eventIndex] = *(NB_TrafficEvent*) CSL_VectorGetPointer(information->groupedTrafficEvents, eventIndex);
    }

    *groupedTrafficEvents = events;
    *numberOfTrafficEvents = eventCount;

    return NE_OK;
}

NB_Error
NB_TrafficInformationFreeTrafficEvents(NB_TrafficEvent* trafficEvents)
{
    if (!trafficEvents)
    {
        return NE_INVAL;
    }

    nsl_free(trafficEvents);
    return NE_OK;
}

NB_Error
NB_TrafficInformationCreateFromRoute(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation** information)
{
    NB_Error err = NE_OK;
    NB_TrafficInformation* trafficInfo = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    uint32 count=0;

    if (!route || !context || !information)
    {
        return NE_INVAL;
    }

    trafficInfo = AllocateTrafficInformation(context);
	if (trafficInfo == NULL)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(context);
    navReply = NB_RouteInformationGetDataNavReply(route);

    trafficInfo->isNotify = FALSE;
    trafficInfo->navMinTmcSpeed = route->config.prefs.navMinTmcSpeed;
    trafficInfo->minNoDataDisableTrafficBar = route->config.prefs.minNoDataDisableTrafficBar;

    count = NB_RouteInformationGetManeuverCount(route);

    data_nav_reply_get_traffic_info(dataState, navReply, trafficInfo->navMinTmcSpeed, trafficInfo->minNoDataDisableTrafficBar,
                                    0, 0, count, TrafficSpeedAll, &trafficInfo->trafficDelay, &trafficInfo->trafficMeter,
                                    &trafficInfo->travelTime, (nav_speed_cb)data_nav_reply_get_speed, navReply);
    data_traffic_record_identifier_copy(dataState, &trafficInfo->trafficRecordIdentifier, &navReply->traffic_record_identifier);
    data_blob_copy(dataState, &trafficInfo->routeId, &route->dataReply.route_id);

    data_traffic_flow_copy(dataState, &trafficInfo->realtimeTrafficFlow, &navReply->realtime_traffic_flow);
    data_traffic_flow_copy(dataState, &trafficInfo->historicalTrafficFlow, &navReply->historical_traffic_flow);
    data_traffic_flow_copy(dataState, &trafficInfo->standardTrafficFlow, &navReply->standard_traffic_flow);

    data_traffic_incidents_copy(dataState, &trafficInfo->trafficIncidents, &navReply->traffic_incidents);

    NB_QaLogTrafficId(context, data_string_get(dataState, &navReply->traffic_record_identifier.value));
    QaLogIncidents(trafficInfo, &navReply->traffic_incidents);
    QaLogFlow(trafficInfo, &navReply->realtime_traffic_flow);
    QaLogFlow(trafficInfo, &navReply->historical_traffic_flow);
    QaLogFlow(trafficInfo, &navReply->standard_traffic_flow);

    trafficInfo->hasNewInformation = TRUE;

    err = CreateTrafficEventList(trafficInfo, route, 0);
    if (err != NE_OK)
    {
        return err;
    }

    *information = trafficInfo;

    return err;
}

NB_Error
NB_TrafficInformationCreateFromTPSReply(NB_Context* context, tpselt reply, NB_TrafficInformation** information)
{
    NB_TrafficInformation* pThis = NULL;
    data_util_state* dataState = NULL;
    CSL_Cache* voiceCache = NULL;
    int length = 0;
    int i = 0;

    if (!context)
    {
        return NE_INVAL;
    }

    voiceCache = NB_ContextGetVoiceCache(context);

    if ( !voiceCache )
    {
        return NE_INVAL;
    }

    pThis = AllocateTrafficInformation(context);

    if (!pThis)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(context);

    data_traffic_notify_reply_from_tps(dataState, &pThis->dataReply, reply);

    pThis->isNotify = TRUE;
    pThis->replyNotReady = pThis->dataReply.traffic_record_identifier.reply_not_ready;
    pThis->hasNewInformation = !pThis->replyNotReady;

    NB_QaLogTrafficId(context, data_string_get(dataState, &pThis->dataReply.traffic_record_identifier.value));
    if (pThis->replyNotReady)
    {
        NB_QaLogAppState(context, "traffic-notify-reply: not ready");
    }
    else
    {
        QaLogIncidents(pThis, &pThis->dataReply.traffic_record_identifier.old_traffic_incidents);
        QaLogIncidents(pThis, &pThis->dataReply.traffic_record_identifier.new_traffic_incidents);
        QaLogFlow(pThis, &pThis->dataReply.traffic_record_identifier.old_realtime_traffic_flow);
        QaLogFlow(pThis, &pThis->dataReply.traffic_record_identifier.new_realtime_traffic_flow);

        length = CSL_VectorGetLength(pThis->dataReply.vec_file);
        for (i=0; i<length; i++)
        {
            data_file* file = CSL_VectorGetPointer(pThis->dataReply.vec_file, i);
            if (file)
            {
                CSL_CacheAdd(voiceCache, file->name.data, file->name.size, file->data.data, file->data.size, TRUE, TRUE, 0);

                NB_QaLogPronunFileDownload(context, (const char*) file->name.data, (nb_size)file->name.size);
            }
        }
    }

    *information = pThis;

    return NE_OK;
}

NB_DEF nb_boolean
NB_TrafficInformationCompareTrafficRecordId(NB_TrafficInformation* information, const char* id)
{
    data_util_state* dataState = NULL;
    NB_Context* context = NULL;
    const char* currentId = NULL;

    if ((information == NULL) || (id == NULL))
    {
        return FALSE;
    }

    context = information->context;
    if (context == NULL)
    {
        return FALSE;
    }

    dataState = NB_ContextGetDataState(context);
    if (dataState == NULL)
    {
        return FALSE;
    }

    currentId = data_string_get(dataState, &information->dataReply.traffic_record_identifier.value);
    if (currentId == NULL)
    {
        return FALSE;
    }

    if (nsl_strcmp(currentId, id) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

data_traffic_record_identifier*
NB_TrafficInformationGetRecordIdentifier(NB_TrafficInformation* information)
{
    if (information)
    {
        return &information->trafficRecordIdentifier;
    }
    return NULL;
}

data_traffic_incidents*
NB_TrafficInformationGetTrafficIncidents(NB_TrafficInformation* information)
{
    if (information)
    {
        return &information->trafficIncidents;
    }
    return NULL;
}

data_traffic_notify_reply*
NB_TrafficInformationGetDataReply(NB_TrafficInformation* information)
{
    if (information)
    {
        return &information->dataReply;
    }
    return NULL;
}

NB_Error
NB_TrafficInformationGetTrafficFlows(NB_TrafficInformation* information, data_traffic_flow** standardTrafficFlow, data_traffic_flow** historicalTrafficFlow, data_traffic_flow** realtimeTrafficFlow)
{
    if (!information)
    {
        return NE_INVAL;
    }

    if (standardTrafficFlow)
    {
        *standardTrafficFlow = &information->standardTrafficFlow;
    }

    if (historicalTrafficFlow)
    {
        *historicalTrafficFlow = &information->historicalTrafficFlow;
    }

    if (realtimeTrafficFlow)
    {
        *realtimeTrafficFlow = &information->realtimeTrafficFlow;
    }

    return NE_OK;

}

static NB_TrafficInformation*
AllocateTrafficInformation(NB_Context* context)
{
    NB_TrafficInformation* pThis = NULL;

    if (!context)
    {
        return NULL;
    }

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    return pThis;
}

void
QaLogIncidents(NB_TrafficInformation* pThis, data_traffic_incidents* incidents)
{
    NB_Place place = {{ 0 }};
    NB_TrafficIncident incident = { 0 };
    int index = 0;
    int count = 0;
    const char* label = 0;
    data_place* replyPlace = 0;
    data_util_state* dataState = NB_ContextGetDataState(pThis->context);
    if (!CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(pThis->context)))
    {
        return;
    }

    label = incidents->age ? "new incidents" : "old incidents";
    count = CSL_VectorGetLength(incidents->vec_place);

    for (index = 0; index < count; index++)
    {
        replyPlace = (data_place*)CSL_VectorGetPointer(incidents->vec_place, index);

        ClearNIMPlace(&place);
        ClearNIMTrafficIncidentDetails(&incident);

        SetNIMPlaceFromPlace(&place, dataState, replyPlace);
        SetNIMTrafficIncidentDetailsFromTrafficIncident(&incident, dataState, &replyPlace->traffic_incident);

        NB_QaLogTrafficIncident(pThis->context, &incident, &place, label);
    }
}

void
QaLogFlow(NB_TrafficInformation* pThis, data_traffic_flow* flow)
{
    int index = 0;
    int count = 0;
    const char* label = "unknown";
    data_traffic_flow_item* item;
    data_util_state* dataState = NB_ContextGetDataState(pThis->context);
    if (!CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(pThis->context)))
    {
        return;
    }

    count = CSL_VectorGetLength(flow->vec_traffic_flow_items);

    if (flow->age == 0)
    {
        if (flow->type == 0)
        {
            label = "old historical";
        }
        else if (flow->type == 1)
        {
            label = "old real time";
        }
        else if (flow->type == 2)
        {
            label = "old standard";
        }
    }
    else if (flow->age == 1)
    {
        if (flow->type == 0)
        {
            label = "new historical";
        }
        else if (flow->type == 1)
        {
            label = "new real time";
        }
        else if (flow->type == 2)
        {
            label = "new standard";
        }
    }

    for (index = 0; index < count; index++)
    {
        item = (data_traffic_flow_item*)CSL_VectorGetPointer(flow->vec_traffic_flow_items, index);
        NB_QaLogTrafficFlow(pThis->context, label, data_string_get(dataState, &item->location), data_string_get(dataState, &item->color), item->speed, item->free_flow_speed);
    }
}

NB_Error
NB_TrafficInformationUpdateIncidentDistances(NB_TrafficInformation* pThis, NB_NavigationState* navigationState)
{
    UpdateIncidentDistancesVector(navigationState, pThis->dataReply.traffic_record_identifier.old_traffic_incidents.vec_place);
    UpdateIncidentDistancesVector(navigationState, pThis->dataReply.traffic_record_identifier.new_traffic_incidents.vec_place);

    return NE_OK;
}

void
UpdateIncidentDistancesVector(NB_NavigationState* navigationState, struct CSL_Vector* incidents)
{
    uint32 index = 0;
    uint32 count = 0;
    data_place* place = NULL;
    NB_LatitudeLongitude location = { 0 };

    count = CSL_VectorGetLength(incidents);

    for (index = 0; index < count; index++)
    {
        place = CSL_VectorGetPointer(incidents, index);

        location.latitude = place->location.point.lat;
        location.longitude = place->location.point.lon;

        NB_RouteDistanceToClosestPoint(navigationState, &location, &place->traffic_incident.nman_closest, &place->traffic_incident.nseg_closest, &place->traffic_incident.distance_from_start);
    }
}


nb_boolean
NB_TrafficInformationReplyNotReady(NB_TrafficInformation* pThis)
{
    return pThis->replyNotReady;
}

void
RemovePassedIncidents(NB_TrafficInformation* pThis, struct CSL_Vector* incidents, double distance)
{
    int index;
    int count;
    data_place* place;
    data_util_state* dataState = NB_ContextGetDataState(pThis->context);

    count = CSL_VectorGetLength(incidents);

    /* Go backwards to prevent problems with removing items */
    for (index = count - 1; index >= 0; index--)
    {
        place = CSL_VectorGetPointer(incidents, index);
        if (place->traffic_incident.distance_from_start == -1 || place->traffic_incident.distance_from_start < distance)
        {
            // this incident is behind the user, so remove it
            data_place_free(dataState, place);
            CSL_VectorRemove(incidents, index);
        }
    }
}

int ComparePlaceDistance(const void* left, const void* right)
{
    const data_place* place1 = left;
    const data_place* place2 = right;

    if (place1->traffic_incident.distance_from_start < place2->traffic_incident.distance_from_start)
    {
        return -1;
    }
    else if (place1->traffic_incident.distance_from_start > place2->traffic_incident.distance_from_start)
    {
        return 1;
    }

    return 0;
}

NB_Error MergeIncidents(NB_TrafficInformation* destination, NB_TrafficInformation* source, data_util_state* dataState)
{
    NB_Error error = NE_OK;
    int index = 0;
    int count = 0;
    data_place* place = 0;

    DATA_VEC_FREE(dataState, destination->trafficIncidents.vec_place, data_place);
    DATA_VEC_ALLOC(error, destination->trafficIncidents.vec_place, data_place);

    DATA_VEC_COPY(dataState, error, destination->trafficIncidents.vec_place, source->dataReply.traffic_record_identifier.new_traffic_incidents.vec_place, data_place);

    count = CSL_VectorGetLength(destination->trafficIncidents.vec_place);

    for (index = 0; index < count; index++)
    {
        place = CSL_VectorGetPointer(destination->trafficIncidents.vec_place, index);
        place->traffic_incident.new_item = TRUE;
    }

    DATA_VEC_COPY(dataState, error, destination->trafficIncidents.vec_place, source->dataReply.traffic_record_identifier.old_traffic_incidents.vec_place, data_place);

    return error;
}

NB_Error
NB_TrafficInformationGetSpeed(data_util_state* pds, const char* tmcloc, TrafficSpeedType speed_type, double minspeed,
                   double* pspeed, char* pcolor, double* pfreeflowspeed,
                   TrafficSpeedType* pspeed_type_out, NB_TrafficInformation* pThis)
{
    NB_Error err = NE_OK;

    if ((pThis == NULL) || (speed_type == TrafficSpeedNone))
    {
        return NE_INVAL;
    }

    if (speed_type & TrafficSpeedRealTime)
    {
        err = data_traffic_flow_get_speed(pds, &pThis->realtimeTrafficFlow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);
        if (err == NE_OK && pspeed_type_out != NULL)
        {
            *pspeed_type_out = TrafficSpeedRealTime;
            return NE_OK;
        }
    }

    if ((speed_type & TrafficSpeedHistoric))
    {
        err = data_traffic_flow_get_speed(pds, &pThis->historicalTrafficFlow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);
        if (err == NE_OK && pspeed_type_out != NULL)
        {
            *pspeed_type_out = TrafficSpeedHistoric;
            return NE_OK;
        }
    }

    if ((speed_type & TrafficSpeedStandard)) {

        err = data_traffic_flow_get_speed(pds, &pThis->standardTrafficFlow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);
        if (err == NE_OK && pspeed_type_out != NULL)
        {
            *pspeed_type_out = TrafficSpeedStandard;
            return NE_OK;
        }
    }

    return err;
}

NB_Error
NB_TrafficInformationGetTrafficRegionRealTimeSpeed(NB_TrafficInformation* traffic, const char* tmcLocation, double* speed, double* ffspeed, NB_NavigateTrafficCongestionType* congestionType)
{
    NB_Error error = NE_OK;

    char color = 0;
    TrafficSpeedType speedType = TrafficSpeedNone;
    double freeFlowSpeed = 0;

    if (!traffic || !tmcLocation || !speed)
    {
        return NE_INVAL;
    }

    error = NB_TrafficInformationGetSpeed(
        NB_ContextGetDataState(traffic->context), tmcLocation, TrafficSpeedRealTime, 0,
        speed, &color, &freeFlowSpeed, &speedType, traffic);

    if (error == NE_OK)
    {
        // prevent divide by zero
        if (*speed < MIN_SPEED)
        {
            *speed = MIN_SPEED;
        }

        if (ffspeed)
        {
            if (freeFlowSpeed < MIN_SPEED)
            {
                // prevent divide by zero
                *ffspeed = MIN_SPEED;
            }
            else
            {
                *ffspeed = freeFlowSpeed;
            }
        }

        if (congestionType)
        {
            switch (color)
            {
            case 'G':
                *congestionType = NB_NTC_None;
                break;
            case 'Y':
                *congestionType = NB_NTC_Moderate;
                break;
            case 'R':
                *congestionType = NB_NTC_Severe;
                break;
            default:
                *congestionType = NB_NTC_None;
                break;
            }
        }
    }

    return error;
}

NB_DEF NB_Error NB_TrafficInformationGetPronunInformation(NB_TrafficInformation* trafficInformation, const char* key,
                                                          char* textBuffer, uint32* textBufferSize,
                                                          byte* phoneticsData, uint32* phoneticsDataSize,
                                                          double* duration)
{
    data_pronun_extended* pronunInfoItem = NULL;
    int totalPronunListItems = 0;
    int i = 0;

    if (!trafficInformation || !key || (!textBuffer && !textBufferSize && (*textBufferSize) == 0
        && !duration && !phoneticsData && !phoneticsDataSize && (*phoneticsDataSize) == 0))
    {
        return NE_INVAL;
    }

    // Get pronun information from traffic-notify-reply for street voices
    totalPronunListItems = CSL_VectorGetLength(trafficInformation->dataReply.pronun_list.vec_pronun_extended);

    for (i = 0; i < totalPronunListItems; ++i)
    {
        pronunInfoItem = (data_pronun_extended*)CSL_VectorGetPointer(trafficInformation->dataReply.pronun_list.vec_pronun_extended, i);

        if (pronunInfoItem && pronunInfoItem->key && (nsl_strcmp(pronunInfoItem->key, key) == 0))
        {
            nb_size textLen = (nb_size)nsl_strlen(pronunInfoItem->text);
            if (textBuffer && textBufferSize && (*textBufferSize) >= (uint32)textLen && textLen > 0)
            {
                nsl_strlcpy(textBuffer, pronunInfoItem->text, (*textBufferSize));
                (*textBufferSize) = (nb_size)(nsl_strlen(pronunInfoItem->text) + 1);
            }	
            else if (textBufferSize)
            {
                (*textBufferSize) = 0;
            }

            if (phoneticsData && phoneticsDataSize &&
                ((*phoneticsDataSize) >= pronunInfoItem->phonetics.size)
                && pronunInfoItem->phonetics.size > 0)
            {
                nsl_memcpy(phoneticsData, pronunInfoItem->phonetics.data, pronunInfoItem->phonetics.size);
                (*phoneticsDataSize) = (nb_size)pronunInfoItem->phonetics.size;
            }
            else if (phoneticsDataSize)
            {
                (*phoneticsDataSize) = 0;
            }

            if (duration)
            {
                // TODO: Clarify requirements for playTime. Should server send playtime as double instead of int?
                *duration = pronunInfoItem->playtime;
            }
            return NE_OK;
        }
    }

    return NE_NOENT;
}

/*! @} */
