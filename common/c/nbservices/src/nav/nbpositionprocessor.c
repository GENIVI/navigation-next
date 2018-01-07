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

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbpositionprocessor.h"
#include "nbpositionprocessorprivate.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstateprivate.h"
#include "nbrouteinformationprivate.h"
#include "data_polyline.h"
#include "data_nav_reply.h"
#include "gpsutil.h"
#include "navutil.h"
#include "nbqalog.h"

static nb_boolean NB_PositionProcessorUpdatePosition(NB_NavigationState* pNav, NB_GpsLocation* location);
static NB_Error NB_PositionProcessorUpdateRouteProgress(NB_NavigationState* pNav, NB_GpsLocation* location);
static NB_Error NB_PositionProcessorUpdateNavigationState(NB_NavigationState* navigationState);
static void NB_PositionProcessorUpdateRoutePosition(NB_Context* context, NB_RouteInformation* route, struct route_position* pos, char* turnCode, nb_size turnCodeSize);

NB_DEF NB_Error
NB_PositionProcessorUpdate(NB_NavigationState* navigationState, NB_GpsLocation* location)
{
    NB_Error err = NE_OK;

    if (!navigationState || !navigationState->navRoute)
    {
        return NE_INVAL;
    }

    navigationState->positionUpdated = FALSE;

    if (NB_PositionProcessorCheckFixQuality(navigationState, location))
    {
        navigationState->firstFixProcessed = TRUE;
        navigationState->lastGpsTime = location->gpsTime;
        navigationState->lastLatitude = location->latitude;
        navigationState->lastLongitude = location->longitude;

        if (IsHeadingValidForNavigation(navigationState, location))
        {
            navigationState->lastHeading = location->heading;
            navigationState->lastHeadingUTC = location->gpsTime;
        }

        navigationState->currentSpeed = location->horizontalVelocity;

        if (NB_PositionProcessorUpdatePosition(navigationState, location))
        {
            navigationState->positionUpdated = TRUE;
        }

        err = NB_PositionProcessorUpdateNavigationState(navigationState);
    }

    navigationState->newRoute = FALSE;

    return err;
}

/*! Determine if a GPS fix is of sufficient quality for navigation.

@param navigationState NB_NavigationState object representing the current navigation state
@param location current GPS location information
@returns nb_boolean TRUE if the fix can be used, FALSE if not.
*/
NB_DEF nb_boolean
NB_PositionProcessorCheckFixQuality(NB_NavigationState* navigationState, const NB_GpsLocation* fix)
{
    NB_RoutePrivateConfiguration* privateConfig;

    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    if (!navigationState->firstFixProcessed)
    {
        /* We always process the first fix, regardless of quality.  This allows use the use a lower
           quality fix to start a route (we always process the origin fix first when we receive a
           new route */
        return TRUE;
    }

    if (fix->gpsTime < navigationState->lastGpsTime)
    {
        /* This fix is earlier than last good fix, a historical fix reappeared somehow?
           A historical fix with valid heading and speed could really confuse navigation position
           tracking, so it should be rejected immediately.
        */
        return FALSE;
    }

    // @todo (BUG 56033)- determine which if all of these values are actually needed.
    if (fix->valid & (NGV_HorizontalUncertainty | NGV_AxisUncertainty | NGV_PerpendicularUncertainty))
    {
        if ((fix->horizontalUncertaintyAlongAxis < 0 || fix->horizontalUncertaintyAlongAxis > privateConfig->prefs.navSifterError) ||
            (fix->horizontalUncertaintyAlongPerpendicular < 0 || fix->horizontalUncertaintyAlongPerpendicular > privateConfig->prefs.navSifterError))
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        /* We don't have a valid uncertainty value for this fix, so we will go ahead an use it anyway */
        return TRUE;
    }
}


/*! Find the position on the route corresponding to the specified fix and store to newRoutePosition.

Find the position on the route corresponding to the specified fix and store to newRoutePosition.
If the fix is not valid, or does not lie on the route, update the appropriate off-route or wrong-way state.

@param navigationState NB_NavigationState object representing the current navigation state
@param location current GPS location information
@returns nb_boolean TRUE if the new position should be used, FALSE if not.
*/
static nb_boolean
NB_PositionProcessorUpdatePosition(NB_NavigationState* navigationState, NB_GpsLocation* location)
{
    struct route_position newPosition;
    nb_boolean maneuverChanged = FALSE;

    navigationState->positionUpdated = FALSE;

    clear_route_position(&newPosition);

    navigationState->onRoute = NB_PositionProcessorFindRoutePosition(navigationState->context, navigationState, navigationState->navRoute, location, navigationState->manueverProgress,
        navigationState->segmentProgress, navigationState->lastHeading, &newPosition, &navigationState->currentRoutePosition,
        navigationState->turnCode, sizeof(navigationState->turnCode));

    if (navigationState->onRoute)
    {
        if (!navigationState->initialRouteMatch)
        {
            /* If the first fix doesn't match to the route, then we require several matches in order to switch out
               of startup case */
            if (!newPosition.wrong_way && newPosition.valid_heading)
            {
                navigationState->initialRouteMatchesNeeded--;
            }

            if (navigationState->initialRouteMatchesNeeded <= 0 || navigationState->newRoute)
            {
                navigationState->initialRouteMatch = TRUE;
                navigationState->initialRouteMatchesNeeded = 0;
            }
        }

        if (newPosition.wrong_way)
        {
            navigationState->wrongWayCount++;
        }
        else
        {
            navigationState->wrongWayCount = 0;
            navigationState->offRouteCount = 0;
        }

        if (navigationState->initialRouteMatch)
        {
            if (newPosition.closest_maneuver != navigationState->currentRoutePosition.closest_maneuver)
            {
                maneuverChanged = TRUE;
            }

            navigationState->currentRoutePosition = newPosition;
            navigationState->positionUpdated = TRUE;
        }

        NB_PositionProcessorUpdateRouteProgress(navigationState, location);

        if (maneuverChanged)
        {
            uint32 currentManeuver = navigationState->currentRoutePosition.closest_maneuver;

            if (currentManeuver != NAV_MANEUVER_START && currentManeuver != NAV_MANEUVER_NONE && currentManeuver != NAV_MANEUVER_CURRENT)
            {
                /* reset current maneuver speed */
                data_nav_maneuver* pman = NB_RouteInformationGetManeuver(navigationState->navRoute, currentManeuver);
                reset_nav_avg_speed(&navigationState->averageSpeed, pman->speed);
            }
        }

        add_nav_avg_speed(&navigationState->averageSpeed, location->horizontalVelocity, location->gpsTime, (location->valid & NGV_HorizontalVelocity) ? TRUE : FALSE);
    }
    else
    {
        if (!navigationState->initialRouteMatch)
        {
            navigationState->currentRoutePosition.closest_maneuver = NAV_MANEUVER_START;
        }
        else
        {
            // off route during navigation, keep the current closet_maneuver
        }

        navigationState->offRouteCount++;
    }

    return navigationState->positionUpdated;
}

static NB_Error
NB_PositionProcessorUpdateRouteProgress(NB_NavigationState* navigationState, NB_GpsLocation* location)
{
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    NB_RoutePrivateConfiguration* privateConfig = NULL;
    double routeDistRemainAfterManeuver = 0;
    uint32 routeTimeRemainAfterManeuver = 0;
    double maneuverDistRemain = 0;
    uint32 maneuverTimeRemain = 0;

    dataState = NB_ContextGetDataState(navigationState->context);
    navReply = NB_RouteInformationGetDataNavReply(navigationState->navRoute);
    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    if (navigationState->currentRoutePosition.closest_maneuver != NAV_MANEUVER_NONE &&
        (navigationState->currentRoutePosition.match_type == rt_match_normal || navigationState->currentRoutePosition.match_type == rt_match_turn))
    {
        navigationState->manueverProgress = navigationState->currentRoutePosition.closest_maneuver;
        navigationState->segmentProgress = navigationState->currentRoutePosition.closest_segment;
    }
    else
    {
        navigationState->manueverProgress = 0;
        navigationState->segmentProgress = 0;
    }

    NB_RouteInformationTripRemainAfterManeuver(&privateConfig->prefs, dataState, navReply,
        navigationState->currentRoutePosition.closest_maneuver,
        &routeDistRemainAfterManeuver,
        &routeTimeRemainAfterManeuver);

    maneuverDistRemain = NB_RouteInformationManeuverRemainingDist(dataState, navReply,
            navigationState->manueverProgress, navigationState->segmentProgress,
            navigationState->currentRoutePosition.segment_remain);
    maneuverTimeRemain = (uint32) (maneuverDistRemain / get_nav_avg_speed(&navigationState->averageSpeed));

    navigationState->remainingRouteDist = routeDistRemainAfterManeuver + maneuverDistRemain;
    navigationState->remainingRouteTime = routeTimeRemainAfterManeuver + maneuverTimeRemain;

    return NE_OK;
}

static NB_Error
NB_PositionProcessorUpdateNavigationState(NB_NavigationState* navigationState)
{
    NB_Error result = NE_OK;
    data_util_state* dataState = NULL;
    data_nav_reply* dataReply = NULL;
    NB_RoutePrivateConfiguration* privateConfig =NULL;

    double instructionReferenceDistance = 0.0;
    double instructionReferenceSpeed = 0.0;
    enum manuever_type instructionManeuverType = manuever_type_normal;
    double maximumInstructionDistance = 0.0;

    double anncDist     = 0.0;
    double continueDist = 0.0;
    double prepDist     = 0.0;
    double prepMinDist  = 0.0;
    double instDist     = 0.0;

    dataState = NB_ContextGetDataState(navigationState->context);

    if (navigationState->navRoute)
    {
        dataReply = NB_RouteInformationGetDataNavReply(navigationState->navRoute);
        privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);
    }

    if (!dataState || !dataReply || !privateConfig)
    {
        return NE_INVAL;
    }

    if ((navigationState->initialRouteMatch) && (navigationState->currentRoutePosition.closest_maneuver != NAV_MANEUVER_NONE))
    {
        result = NB_NavigationStateGetInstructionParameters(navigationState,
            &instructionReferenceDistance, &instructionReferenceSpeed,
            &instructionManeuverType, &maximumInstructionDistance, NULL);

        result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                             &anncDist,
                                                                             &continueDist,
                                                                             NULL,
                                                                             &prepDist,
                                                                             &prepMinDist,
                                                                             &instDist);
        if (result != NE_OK)
        {
            return result;
        }

        if ((navigationState->offRouteCount > 0 && instructionReferenceDistance <= privateConfig->prefs.navHideDistThreshold) ||
            (navigationState->currentRoutePosition.match_type == rt_match_turn))
        {
            navigationState->maneuverPos = NB_NMP_Past;
        }
        else if (instructionReferenceDistance >= continueDist)
        {
            navigationState->maneuverPos = NB_NMP_Continue;
        }
        else if (instructionReferenceDistance >= prepDist)
        {
            navigationState->maneuverPos = NB_NMP_ShowTurn;
        }
        else if (instructionReferenceDistance >= instDist)
        {
            navigationState->maneuverPos = NB_NMP_Prepare;
        }
        else
        {
            navigationState->maneuverPos = NB_NMP_Turn;
        }

        if (instructionManeuverType == maneuver_type_dest &&
            (navigationState->maneuverPos == NB_NMP_Prepare ||
             navigationState->maneuverPos == NB_NMP_Turn ||
             navigationState->maneuverPos == NB_NMP_Past))
        {
            navigationState->pastArrivingThreshold = TRUE;
        }

        if (instructionManeuverType == maneuver_type_dest &&
            (navigationState->maneuverPos == NB_NMP_Turn ||
             navigationState->maneuverPos == NB_NMP_Past))
        {
            navigationState->pastArrivedThreshold = TRUE;
        }

        navigationState->snapMap        = navigationState->currentRoutePosition.segment_distance < privateConfig->prefs.snapMapThreshold ? TRUE : FALSE;
    }
    else
    {
        // state for startup case
        navigationState->maneuverPos = NB_NMP_Turn;
        navigationState->pastArrivingThreshold = FALSE;
        navigationState->pastArrivedThreshold = FALSE;
        navigationState->snapMap = FALSE;
    }

    return NE_OK;
}

#define NAV_MODE_MAX_MATCHES 16 // How to determine this: this is an arbitrary limit based on the anticipated number of candidate route matches.
                                // Used to be 8, but we found a case that exceeded that, .

NB_DEF nb_boolean
NB_PositionProcessorFindRoutePosition(NB_Context* context, NB_NavigationState* navigateState, NB_RouteInformation* route, NB_GpsLocation* location, uint32 maneuverProgress, uint32 segmentProgress,
                                      double last_heading, struct route_position* ppos, struct route_position* prevpos, char* turnCode, nb_size turnCodeSize)
{
    struct seg_match    seg_matches[NAV_MODE_MAX_MATCHES];
    int                    num_matches     = 0;
    int                    best_match      = -1;
    nb_boolean            on_route        = FALSE;
    double                dist            = 0;
    double                heading         = 0;
    nb_boolean            good_heading    = IsHeadingValidForNavigation(navigateState, location);

    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    NB_RoutePrivateConfiguration* privateConfig = NULL;
    double routeLength = 0.0;

    NB_NavigateTransportMode transportMode;

    if (!route)
    {
        return TRUE;
    }

    (void)NB_RouteInformationGetTransportMode(route, &transportMode);

    dataState = NB_ContextGetDataState(context);
    navReply = NB_RouteInformationGetDataNavReply(route);
    privateConfig = NB_RouteInformationGetPrivateConfiguration(route);
    NB_RouteInformationGetSummaryInformation(route, NULL, NULL, &routeLength);

    heading = good_heading ? location->heading : last_heading;

    num_matches = NB_RouteInformationFindSegmentMatches(route, location, heading, maneuverProgress, segmentProgress, heading, seg_matches, sizeof(seg_matches)/sizeof(seg_matches[0]),prevpos);
    best_match = NB_RouteInformationFindBestMatch(route, seg_matches, num_matches, TRUE);

    if (num_matches > 0 && CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context)))
    {
        int i = 0;

        for (i = 0; i < num_matches; i++)
        {
            NB_QaLogSegmentMatch(context, &seg_matches[i], (nb_boolean)(best_match == i));
        }
    }

    if (best_match != -1)
    {
        /* We have a valid segment match */

        on_route = TRUE;

        ppos->time                    = seg_matches[best_match].time;
        ppos->closest_segment        = seg_matches[best_match].segment;
        ppos->closest_maneuver        = seg_matches[best_match].maneuver;
        ppos->segment_remain        = seg_matches[best_match].remain;
        ppos->segment_distance        = seg_matches[best_match].distance;
        ppos->proj_lat                = seg_matches[best_match].proj_lat;
        ppos->proj_lon                = seg_matches[best_match].proj_lon;
        ppos->segment_heading        = seg_matches[best_match].heading;
        ppos->valid_heading            = good_heading;

        switch (seg_matches[best_match].type)
        {
        case match_turn:
            ppos->match_type = rt_match_turn;
            break;
        case match_end:
            if (ppos->closest_maneuver != NAV_MANEUVER_NONE &&
                ppos->closest_segment == data_polyline_len(dataState, &data_nav_reply_get_nav_maneuver(dataState, navReply, ppos->closest_maneuver)->polyline) - 2)
            {
                ppos->match_type = rt_match_turn;
            }
            else
            {
                ppos->match_type = rt_match_normal;
            }
            break;
        default:
            ppos->match_type = rt_match_normal;
            break;
        }

        /* Compare our direction of travel to the direction of the segment to see if we are going the right way */
        if (compare_heading(heading, ppos->segment_heading, privateConfig->prefs.navHeadingMargin, NULL) || ppos->match_type == rt_match_turn || !good_heading )
        {
            /* Consistent direction of travel */
            ppos->est_speed_along_route = location->horizontalVelocity;
            ppos->wrong_way = FALSE;
        }
        else
        {
            /* Inconsistent direction of travel */
            ppos->est_speed_along_route = -1.0;
            if (transportMode != NB_NTM_Pedestrian)
            {
                ppos->wrong_way = TRUE;
            }
        }
    }

    ppos->near_window_end = FALSE;

    if (on_route)
    {
        NB_PositionProcessorUpdateRoutePosition(context, route, ppos, turnCode, turnCodeSize);

        /* This only applies if we don't have the complete polyline */
        if (!navReply->polyline_complete)
        {
            dist = routeLength - (ppos->maneuver_distance_remaining + ppos->trip_distance_after);
            if ( (navReply->polyline_length - dist) <= privateConfig->prefs.navSoftRecalcDist)
            {
                 ppos->near_window_end = TRUE;
            }
        }
    }

    return on_route;
}

static void
NB_PositionProcessorUpdateRoutePosition(NB_Context* context, NB_RouteInformation* route, struct route_position* pos, char* turnCode, nb_size turnCodeSize)
{
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    data_nav_maneuver* pnavmaneuver = NULL;
    NB_RoutePrivateConfiguration* privateConfig = NULL;

    if (!context || !route)
    {
        return;
    }

    dataState = NB_ContextGetDataState(context);
    navReply = NB_RouteInformationGetDataNavReply(route);
    privateConfig = NB_RouteInformationGetPrivateConfiguration(route);

    pnavmaneuver = data_nav_reply_get_nav_maneuver(dataState, navReply, pos->closest_maneuver);

    pos->maneuver_base_speed = pnavmaneuver->speed;
    pos->maneuver_distance_remaining = NB_RouteInformationManeuverRemainingDist(dataState, navReply, pos->closest_maneuver, pos->closest_segment, pos->segment_remain);
    pos->maneuver_max_inst_distance = pnavmaneuver->max_instruction_distance;

    NB_RouteInformationTripRemainAfterManeuver(&privateConfig->prefs, dataState, navReply, pos->closest_maneuver, &pos->trip_distance_after, &pos->trip_time_after);

    if (turnCode != NULL && turnCodeSize > 0)
    {
        nsl_strlcpy(turnCode, data_string_get(dataState, &pnavmaneuver->command), turnCodeSize);
    }
}

NB_DEF NB_Error
NB_PositionProcessorFindClosestPositionOnRoute(NB_Context* context, NB_NavigationState* navigationState, NB_RouteInformation* route, NB_LatitudeLongitude* point, uint32* closestManeuver, uint32* closestSegment, double* distanceToRoute)
{
    NB_Error error = NE_OK;
    double dist = -1.0;
    NB_GpsLocation fix;
    struct route_position pos;
    struct route_position prevpos; // not really used
    double routeLength = 0;

    clear_route_position(&pos);
    clear_route_position(&prevpos);

    gpsfix_setpoint(&fix, point->latitude, point->longitude);

    error = NB_RouteInformationGetSummaryInformation(route, NULL, NULL, &routeLength);

    if (error != NE_OK)
    {
        return error;
    }

    if (NB_PositionProcessorFindRoutePosition(context, navigationState, route, &fix, 0, 0, INVALID_HEADING, &pos, &prevpos, NULL, 0))
    {
        dist = routeLength - (pos.maneuver_distance_remaining + pos.trip_distance_after);
    }

    if (closestManeuver)
        *closestManeuver = pos.closest_maneuver;

    if (closestSegment)
        *closestSegment = pos.closest_segment;

    if (distanceToRoute)
        *distanceToRoute = dist;

    return error;
}
