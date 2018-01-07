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

    @file     nbnavigationstate.c
    @defgroup nbnavigationstate Navigation State
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
#include "nbcontextprotected.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstate.h"
#include "nbnavigationstateprivate.h"
#include "nbrouteinformationprivate.h"
#include "nbrouteparametersprivate.h"
#include "nbtrafficinformation.h"
#include "nbpositionprocessorprivate.h"
#include "gpsutil.h"
#include "navutil.h"

/*! @{ */

/* @todo (BUG 55947) - get rid of this file */

NB_Error
NB_RouteDistanceToClosestPoint(NB_NavigationState* navigationState, NB_LatitudeLongitude* location, uint32* maneuver, uint32* segment, double* distance)
{
    double dist = -1.0;
    double routeDist = 0.0;
    nb_boolean onRoute = FALSE;
    NB_GpsLocation gpsLocation = { 0 };
    struct route_position position = { 0 };
    struct route_position previousPosition = { 0 };

    if (!navigationState || !location)
    {
        return NE_INVAL;
    }

    clear_route_position(&position);
    clear_route_position(&previousPosition);
    gpsfix_setpoint(&gpsLocation, location->latitude, location->longitude);

    onRoute = NB_PositionProcessorFindRoutePosition(navigationState->context, navigationState, navigationState->navRoute, &gpsLocation, 0, 0, INVALID_HEADING, &position, &previousPosition, 0, 0);
    if (onRoute)
    {
        NB_RouteInformationGetSummaryInformation(navigationState->navRoute, NULL, NULL, &routeDist);
        dist = routeDist - (position.maneuver_distance_remaining + position.trip_distance_after);
    }

    if (maneuver)
    {
        *maneuver = position.closest_maneuver;
    }

    if (segment)
    {
        *segment = position.closest_segment;
    }

    if (distance)
    {
        *distance = dist;
    }

    return NE_OK;
}


NB_Error
NB_NavigationFindNextCongestion(NB_NavigationState* navigationState, NB_TrafficInformation* trafficInfo, data_traffic_region* pdtr_start, data_traffic_region* pdtr_end, NB_NavigateTrafficCongestionType* ptc, double* prouteremaindist, double* plength, double* pspeed, uint32* pnman, boolean* pincongestion)
{
    struct route_position* currentPos;
    NB_Error err = NE_OK;
    data_nav_maneuver* pman = NULL;
    data_traffic_region* ptr = NULL;
    uint32 n = 0;
    uint32 nman = 0;
    uint32 numman = 0;
    uint32 ntr = 0;
    uint32 numtr = 0;
    double speed = 0.0;
    NB_NavigateTrafficCongestionType congestion = NB_NTC_Unknown;
    double prev_man_dist = 0.0;
    double start_distance = 0.0;
    double end_distance = 0.0;
    double this_distance = 0.0;
    boolean in_congestion = FALSE;
    boolean found_congestion = FALSE;
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    double routeDist = 0.0;

    currentPos = &navigationState->currentRoutePosition;

    dataState = NB_ContextGetDataState(navigationState->context);
    navReply = NB_RouteInformationGetDataNavReply(navigationState->navRoute);

    numman = data_nav_reply_num_nav_maneuver(dataState, navReply);

    NB_RouteInformationGetSummaryInformation(navigationState->navRoute, NULL, NULL, &routeDist);


    nman = currentPos->closest_maneuver;

    if (pincongestion != NULL)
    {
        *pincongestion = FALSE;
    }

    if (nman == NAV_MANEUVER_CURRENT)
    {
        return NE_INVAL;
    }
    else if (nman == NAV_MANEUVER_START || nman == NAV_MANEUVER_NONE)
    {
        nman = 0;
    }

    if (currentPos->trip_distance_after >= 0.0 || currentPos->maneuver_distance_remaining >= 0.0)
    {
        this_distance = routeDist - currentPos->trip_distance_after - currentPos->maneuver_distance_remaining;
    }
    else
    {
        this_distance = 0.0;
    }

    for (n=0; n<nman; n++)
    {
        pman = data_nav_reply_get_nav_maneuver(dataState, navReply, n);

        if (pman != NULL)
        {
            prev_man_dist += pman->distance;
        }
    }

    for (; nman<numman; nman++)
    {
        pman = data_nav_reply_get_nav_maneuver(dataState, navReply, nman);

        if (pman != NULL)
        {
            numtr = CSL_VectorGetLength(pman->vec_traffic_regions);

            for (ntr = 0; ntr < numtr; ntr++)
            {
                ptr = (data_traffic_region*) CSL_VectorGetPointer(pman->vec_traffic_regions, ntr);

                if (ptr != NULL && NB_TrafficInformationGetTrafficRegionRealTimeSpeed(trafficInfo,
                                            data_string_get(dataState, &ptr->location),
                                            &speed, NULL, &congestion) == NE_OK && congestion != NB_NTC_None)
                {
                    start_distance = prev_man_dist + ptr->start;
                    end_distance = start_distance + ptr->length;

                    // check if this congestion is behind us
                    if (end_distance < this_distance)
                    {
                        continue;
                    }

                    found_congestion = TRUE;

                    if (!in_congestion)
                    {
                        in_congestion = TRUE;

                        // if the user is currently in this congestion, set the flag
                        if (start_distance < this_distance && pincongestion != NULL)
                            *pincongestion = TRUE;

                        err = err ? err : data_traffic_region_copy(dataState, pdtr_start, ptr);
                        err = err ? err : data_traffic_region_copy(dataState, pdtr_end, ptr);

                        *ptc = congestion;
                        *prouteremaindist = routeDist - start_distance;
                        *pspeed = speed;
                        *plength = ptr->length;
                        *pnman = nman;
                    }
                    else
                    {
                        err = err ? err : data_traffic_region_copy(dataState, pdtr_end, ptr);

                        // use the max severity just follow the logic of traffic merge
                        if (*ptc < congestion)
                        {
                            *ptc = congestion;
                        }

                        *plength += ptr->length;
                    }
                }
                else if (in_congestion)
                {
                    goto exit;
                }

                if (err != NE_OK)
                {
                    return err;
                }
            }

            prev_man_dist += pman->distance;
        }
    }

exit:

    return found_congestion ? NE_OK : NE_NOENT;
}


NB_Error
NB_NavigationSetNavigationManeuverPosition(NB_NavigationState* navigationState, NB_NavigateManeuverPos maneuverPos)
{
    if (!navigationState)
    {
        return NE_INVAL;
    }

    navigationState->maneuverPos = maneuverPos;

    return NE_OK;

}

NB_DEF NB_Error
NB_NavigationStateGetRoutePositionInformation(NB_NavigationState* navigationState, NB_PositionRoutePositionInformation* information)
{
    if (!navigationState || !information)
    {
        return NE_INVAL;
    }

    information->time                       = navigationState->currentRoutePosition.time;
    information->closestSegment             = navigationState->currentRoutePosition.closest_segment;
    information->closestManeuver            = navigationState->currentRoutePosition.closest_maneuver;
    information->segmentRemain              = navigationState->currentRoutePosition.segment_remain;
    information->segmentDistance            = navigationState->currentRoutePosition.segment_distance;
    information->segmentHeading             = navigationState->currentRoutePosition.segment_heading;
    information->projLat                    = navigationState->currentRoutePosition.proj_lat;
    information->projLon                    = navigationState->currentRoutePosition.proj_lon;
    information->estSpeedAlongRoute         = navigationState->currentRoutePosition.est_speed_along_route;
    information->wrongWay                   = navigationState->currentRoutePosition.wrong_way;
    information->validHeading               = navigationState->currentRoutePosition.valid_heading;
    information->maneuverDistanceRemaining  = navigationState->currentRoutePosition.maneuver_distance_remaining;
    information->maneuverBaseSpeed          = navigationState->currentRoutePosition.maneuver_base_speed;
    information->tripDistanceAfter          = navigationState->currentRoutePosition.trip_distance_after;
    information->tripTimeAfter              = navigationState->currentRoutePosition.trip_time_after;
    information->maneuverPosition           = navigationState->maneuverPos;
    information->remainingRouteTime         = navigationState->remainingRouteTime;
    information->remainingRouteDist         = navigationState->remainingRouteDist;

    information->averageSpeed               = get_nav_avg_speed(&navigationState->averageSpeed);

    return NE_OK;
}

NB_DEF NB_Error
NB_NavigationStateGetInstructionParameters(NB_NavigationState* navigationState, double* instructionRemainDistance, double* instructionReferenceSpeed, enum manuever_type* instructionManeuverType, double* maximumInstructionDistance, double* totalManeuverDistance)
{
    NB_Error result = NE_OK;
    double distance = 0.0;
    double speed = 0.0;
    double collapse_distance = 0.0;
    enum manuever_type type = manuever_type_normal;
    double maximum_instruction_distance = 0.0;

    data_nav_maneuver* maneuver  = NULL;
    data_nav_maneuver* baseManeuver  = NULL;
    data_util_state* dataState = NB_ContextGetDataState(navigationState->context);

    uint32 instructionManeuverIndex = NAV_MANEUVER_NONE;
    uint32 baseManeuverIndex = NAV_MANEUVER_NONE;

    speed = navigationState->currentRoutePosition.maneuver_base_speed;

    result = NB_RouteCollapseManeuvers(navigationState->navRoute, navigationState->currentRoutePosition.closest_maneuver, &collapse_distance, NULL, NULL,
        &baseManeuverIndex, &instructionManeuverIndex, NULL, NULL, NULL, NULL, NULL, NULL, FALSE);

    distance = navigationState->currentRoutePosition.maneuver_distance_remaining + collapse_distance;

    maneuver = NB_RouteInformationGetManeuver(navigationState->navRoute, instructionManeuverIndex);
    baseManeuver = NB_RouteInformationGetManeuver(navigationState->navRoute, baseManeuverIndex);

    if (maneuver == NULL || baseManeuver == NULL)
    {
        result = NE_INVAL;
    }
    else
    {
        maximum_instruction_distance = maneuver->max_instruction_distance;

        type = NB_RouteParametersGetCommandType(navigationState->navRoute->routeParameters, data_string_get(dataState, &maneuver->command));

        if (instructionRemainDistance)
        {
            *instructionRemainDistance = distance;
        }

        if (instructionReferenceSpeed)
        {
            *instructionReferenceSpeed = speed;
        }

        if (instructionManeuverType)
        {
            *instructionManeuverType = type;
        }

        if (maximumInstructionDistance)
        {
            *maximumInstructionDistance = maximum_instruction_distance;
        }

        if (totalManeuverDistance)
        {
            *totalManeuverDistance = baseManeuver->distance + collapse_distance;
        }
    }

    return result;
}

NB_Error
NB_NavigationStateGetInstructionDistances(NB_NavigationState* navigationState,
                                          double* pAnncDist,
                                          double* pContinueDist,
                                          double* pLaneDist,
                                          double* pPrepDist,
                                          double* pPrepMinDist,
                                          double* pInstDist)
{
    NB_Error result = NE_OK;

    double anncDist     = 0.0;
    double continueDist = 0.0;
    double prepDist     = 0.0;
    double prepMinDist  = 0.0;
    double instDist     = 0.0;
    double laneDist     = 0.0;

    double                  instructionRemainDistance = 0.0;
    double                  instructionReferenceSpeed = 0.0;
    enum manuever_type      instructionManeuverType = manuever_type_normal;
    double                  totalManeuverDistance = 0.0;

    NB_RoutePrivateConfiguration* privateConfig = NULL;

    if (!navigationState )
    {
        return NE_INVAL;
    }

    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    result = NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
            &instructionReferenceSpeed, &instructionManeuverType, NULL,
            &totalManeuverDistance);
    if (result != NE_OK)
    {
        return result;
    }

    /* Calculate the distances for the current speed and maneuver type */
    anncDist     =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, announce_dist);
    continueDist =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, continue_dist);
    prepDist     =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, prepare_dist);
    prepMinDist  =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, prepare_min_dist);
    instDist     =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, instruct_dist);
    laneDist     =   NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, instructionReferenceSpeed, instructionManeuverType, lane_guidance_dist);

    if (pAnncDist)
    {
        *pAnncDist = anncDist;
    }

    if (pContinueDist)
    {
        *pContinueDist = continueDist;
    }

    if (pPrepDist)
    {
        *pPrepDist = prepDist;
    }

    if (pPrepMinDist)
    {
        *pPrepMinDist = prepMinDist;
    }

    if (pInstDist)
    {
        *pInstDist = instDist;
    }

    if (pLaneDist)
    {
        *pLaneDist = laneDist;
    }

    return NE_OK;
}

NB_Error
NB_NavigationGetRouteLength(NB_NavigationState* pThis, double* length)
{
    if (!pThis || !length)
    {
        return NE_INVAL;
    }

    return NB_RouteInformationGetSummaryInformation(pThis->navRoute, NULL, NULL, length);
}

void
NB_NavigationMessagePlayed(NB_NavigationState* navigationState, NB_GuidanceMessage* messagePlayed)
{
    if (navigationState && messagePlayed)
    {
        if (navigationState->currentMessage == messagePlayed)
        {
            navigationState->currentMessage = NULL;
        }
    }
}

/*! @} */
