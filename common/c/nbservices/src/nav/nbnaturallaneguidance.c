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
 @file     nbnaturallaneguidance.c
 @defgroup natural and lane gudiance

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */
#include "nbnaturallaneguidancetype.h"
#include "nbnaturallaneguidanceinterface.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "data_nav_reply.h"
#include "data_nav_maneuver.h"
#include "data_lane_info.h"
#include "data_lane_guidance_item.h"
#include "datautil.h"
#include "navutil.h"
#include "nbcontextprotected.h"
#include "nbnaturallaneguidanceprocessor.h"
#include "nbcontextaccess.h"
#include "nbrouteparametersprivate.h"
#include "nbrouteparameters.h"
#include "nbguidanceprocessorprivate.h"

NB_Error
NB_NavigationSetGuidanceConfiguration(NB_Navigation* navigation,                /*!< The navigation instance
                                                                                     that is invoking the callback */
                                      NB_RouteConfiguration* routeConfiguation, /*!< The routecConfiguartion
                                                                                     instance that is invoking
                                                                                     the callback */
                                      NB_GuidanceConfiguration* configuration   /*!< The configuration of
                                                                                     natural and lane guidance
                                                                                     using */
                                      )
{
    routeConfiguation->wantNaturalGuidance = configuration->naturalGuidanceEnable;
    routeConfiguation->wantExtendedNaturalGuidance = configuration->extendedNaturalGuidanceEnable;
    routeConfiguation->wantLaneGuidance = configuration->laneGuidanceEnable;
    return NE_OK;
}

NB_Error
NB_NLGInformationReset(NB_NLGInformation* information)
{
    // TODO: Reset memebers.
    return NE_OK;
}
NB_Error
NB_NLGStateReset(NB_NLGState* state)
{
    // TODO: Reset memebers.
    state->laneGuidancePlayingManeuverId = NAV_MANEUVER_NONE;
    return NE_OK;
}

NB_Error
NB_NLGInformationCreate(NB_Context * context, NB_NLGInformation ** nlgInformation)
{
    NB_NLGInformation* pThis = NULL;

    if (nlgInformation == NULL || context == NULL)
    {
        return NE_INVAL;
    }

	if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));
    NB_NLGInformationReset(pThis);

    *nlgInformation = pThis;
    return NE_OK;
}

NB_Error
NB_NLGInformationDestroy(NB_NLGInformation * nlgInformation)
{
    if (! nlgInformation)
    {
        return NE_INVAL;
    }
    nsl_free(nlgInformation);
    return NE_OK;
}

NB_Error
NB_NLGStateCreate(NB_Context * context, NB_NLGState ** nlgState)
{
    NB_NLGState* pThis = NULL;

    if (nlgState == NULL || context == NULL)
    {
        return NE_INVAL;
    }

	if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));
    NB_NLGStateReset(pThis);

    *nlgState = pThis;
    return NE_OK;
}

NB_Error
NB_NLGStateDestroy(NB_NLGState * nlgState)
{
    if (! nlgState)
    {
        return NE_INVAL;
    }
    nsl_free(nlgState);
    return NE_OK;
}

NB_Error
NB_LaneGuidanceProcessorUpdate(NB_Navigation* navigation,
                               NB_RouteInformation * route,
                               NB_PositionRoutePositionInformation * currentPosition,
                               NB_NLGInformation* information,
                               double speed
                               )
{
    data_nav_maneuver* maneuver = NULL;
    data_lane_info *laneInfo = NULL;
    uint32 closetManeuver = 0;
    uint32 length = 0;
    uint32 maxIndex = 0;
    enum manuever_type type = manuever_type_normal;
    NB_RoutePreferencesPrivate* routePrefs = NULL;
    data_util_state* dataState = NULL;
    NB_Error err = NE_OK;

    //check input valid
    if (navigation == NULL || route == NULL || currentPosition == NULL)
    {
        return NE_INVAL;
    }

    //check configure first, if wantLaneGuidance == false, direct return.
    if (!route->parameterConfiguration.wantLaneGuidance)
    {
        return NE_OK;
    }

    //Check Transportation mode is not bicycle or predestrian.
    if (route->dataReply.route_settings.vehicle_type == NB_TransportationMode_Bicycle ||
        route->dataReply.route_settings.vehicle_type == NB_TransportationMode_Pedestrian)
    {
        return NE_OK;
    }

    closetManeuver = currentPosition->closestManeuver;

    if (closetManeuver == NAV_MANEUVER_NONE || closetManeuver == NAV_MANEUVER_START)
    {
        navigation->naturalLaneGuidanceState->laneGuidancePlayingManeuverId = NAV_MANEUVER_NONE;
        return NE_OK;
    }

    //hidden lane guidance when maneuver changed
    if (navigation->callbacks.visualLaneGuidanceCallback)
    {
        if ((currentPosition->maneuverPosition == NB_NMP_Past || currentPosition->maneuverPosition == NB_NMP_Continue ||
             currentPosition->maneuverPosition == NB_NMP_ShowTurn) &&
            navigation->guidanceState->laneInfo.state == gmsInvalid)
        {
            navigation->callbacks.visualLaneGuidanceCallback(navigation, NAV_MANEUVER_NONE, 0, NULL, navigation->callbacks.userData);
            navigation->guidanceState->laneInfo.state = gmsPlayed;
            return NE_OK;
        }
    }

    /* Get the next Significant maneuver,  check it have lane guidance information */
    err = NB_RouteInformationGetNextSignificantManeuverIndex(route, closetManeuver, &maxIndex);
    if (err != NE_OK)
    {
        return err;
    }

    maneuver = NB_RouteInformationGetManeuver(route, maxIndex);

    laneInfo = &maneuver->lane_info;
    length = CSL_VectorGetLength(laneInfo->vec_lane_guidance_items);

    if (length == 0)
    {
        navigation->naturalLaneGuidanceState->laneGuidancePlayingManeuverId = NAV_MANEUVER_NONE;
        //notify there is none lane guidance information.
        if (navigation->callbacks.visualLaneGuidanceCallback)
        {
            if (navigation->guidanceState->laneInfo.state == gmsPending)
            {
                navigation->callbacks.visualLaneGuidanceCallback(navigation, NAV_MANEUVER_NONE, 0, NULL, navigation->callbacks.userData);
                navigation->guidanceState->laneInfo.state = gmsPlayed;
            }
        }
        return NE_OK;
    }

    //Get the maneuver type, Calculate distance range
    dataState = NB_ContextGetDataState(route->context);
    routePrefs = &route->config.prefs;

    // Get maneuver type to know the maneuver is freeway

    type = NB_RouteParametersGetCommandType(route->routeParameters, data_string_get(dataState, &maneuver->command));

    if (navigation->guidanceState->laneInfo.state == gmsPending)
    {
        // Show visual lane guidance
        NB_NavigationVisualLaneGuidanceShowing(navigation, maxIndex, laneInfo);
        navigation->guidanceState->laneInfo.state = gmsPlayed;
    }
    else
    {
        navigation->naturalLaneGuidanceState->laneGuidancePlayingManeuverId = NAV_MANEUVER_NONE;
    }

    return NE_OK;

}

NB_Error
NB_NLGProcessorUpdate(NB_Navigation* navigation,
                      NB_RouteInformation * route,
                      NB_PositionRoutePositionInformation * currentPosition,
                      NB_NLGInformation* information,
                      double speed
                      )
{
    NB_Error err = NE_OK;

    err = NB_LaneGuidanceProcessorUpdate(navigation, route, currentPosition, information, speed);

    return err;

}

NB_Error
NB_NavigationVisualLaneGuidanceShowing(NB_Navigation* navigation,
                                       uint32 maneuverIndex,
                                       data_lane_info *laneInfo)
{
    NB_Error error = NE_OK;
    NB_Lane * lanes = NULL;
    int laneCount = 0;
    int i = 0;

    if (!laneInfo || !navigation) {
        return NE_NOENT;
    }

    if (!navigation->naturalLaneGuidanceState)
    {
        return NE_NOTINIT;
    }

    if (navigation->naturalLaneGuidanceState->laneGuidancePlayingManeuverId
        == maneuverIndex)
    {
        return NE_OK;
    }

    if (!navigation->callbacks.visualLaneGuidanceCallback)
    {
        return NE_NOTINIT;
    }

    laneCount = CSL_VectorGetLength(laneInfo->vec_lane_guidance_items);
    if (laneCount == 0)
    {
        return NE_NOENT;
    }

    /* Get the lane informations,
       Fill the translate informatio for visual lane guidance callback*/
    lanes = (NB_Lane*) nsl_malloc(sizeof(NB_Lane) * laneCount);
    if (!lanes)
    {
        return NE_NOMEM;
    }
    nsl_memset(lanes, 0, sizeof(NB_Lane) * laneCount);
    for (i = 0; i<laneCount; i++)
    {
        data_lane_guidance_item * item = CSL_VectorGetPointer(laneInfo->vec_lane_guidance_items, i);
        if (!item)
        {
            if (lanes)
            {
                nsl_free(lanes);
                lanes = NULL;
            }
            return NE_NOMEM;
        }
        GetLaneCharacters(navigation->navState->navRoute->routeParameters ,&lanes[i], item);
    }
    /* Call back*/
    if (navigation->callbacks.visualLaneGuidanceCallback)
    {
        navigation->naturalLaneGuidanceState->laneGuidancePlayingManeuverId = maneuverIndex;
        navigation->callbacks.visualLaneGuidanceCallback(navigation,
                                                         maneuverIndex,
                                                         laneCount,
                                                         lanes,
                                                         navigation->callbacks.userData
                                                         );

    }
    nsl_free(lanes);

    return error;
}

NB_Error GetLaneCharacters(NB_RouteParameters * pRouteParmaters, NB_Lane * lane, data_lane_guidance_item * item)
{
    NB_Error err = NE_OK;
    char selectChars[NB_TURN_CODE_LEN];
    char unselectChars[NB_TURN_CODE_LEN];
    int i = 0;
    nsl_memset(selectChars, 0, NB_TURN_CODE_LEN);
    nsl_memset(unselectChars, 0, NB_TURN_CODE_LEN);
    err = NB_RouteParametersGetLaneCharactersFromMapFont(pRouteParmaters, item->lane_item.item, item->no_highlight_arrow_item.item, item->highlight_arrow_item.item, unselectChars, NB_TURN_CODE_LEN, selectChars,NB_TURN_CODE_LEN);
    if (err == NE_OK)
    {
        for (i = 0; i < LANE_ARROW_MAX; i ++)
        {
            lane->arrows[i].character = unselectChars[i];
            lane->arrows[i].color = NB_AC_DEFAULT_COLOR;
            lane->selectArrows[i].character = selectChars[i];
            lane->selectArrows[i].color = NB_AC_DEFAULT_COLOR;
        }
    }
    else
    {
        // TODO: Wait for define. Current if not found ttf-mapping of the combinations, fill a ' '
        lane->arrows[0].character = ' ';
        lane->arrows[0].color = NB_AC_DEFAULT_COLOR;
    }
    return err;
}
