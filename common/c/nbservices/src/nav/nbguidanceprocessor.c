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

    @file     nbguidanceprocessor.c

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "paltypes.h"
#include "palmath.h"
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbguidanceprocessor.h"
#include "nbguidanceprocessorprivate.h"
#include "nbguidanceinformationprivate.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstate.h"
#include "nbrouteinformationprivate.h"
#include "navtypes.h"
#include "nbspatial.h"
#include "nbutility.h"
#include "navutil.h"
#include "nbnavigationstateprivate.h"
#include "nbguidancemessageprivate.h"
#include "nbnavqalog.h"

#define ONE_SEC_FOR_GAP   1.0
#define TWO_SEC_FOR_GAP   2.0
#define THREE_SEC_FOR_GAP 3.0
#define MIN_SPEED         4.47
static void NB_GuidanceMessagePlayedCallback(NB_GuidanceMessage* message, void* userData);
static NB_Error ResetPreparePlaybackTime(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                         NB_GuidanceProccessMessagesParameters* parameters);
static NB_Error ResetTurnPlaybackTime(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                         NB_GuidanceProccessMessagesParameters* parameters);

enum typeMessage
{
    tmNone = 0,
    tmContinue,
    tmPrepare,
    tmTurn,
    tmTraffic
};

NB_DEF NB_Error
NB_GuidanceProcessorStateCreate(NB_Context* context, NB_GuidanceState** guidanceState)
{
    NB_GuidanceState* pThis = NULL;

    if (!context)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    NB_GuidanceProcessorStateReset(pThis, NULL);

    pThis->recalcMode = FALSE;

    *guidanceState = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceProcessorStateDestroy(NB_GuidanceState* guidanceState)
{
    if (!guidanceState)
    {
        return NE_INVAL;
    }
    if (guidanceState->prepareMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
        guidanceState->prepareMessage = NULL;
    }
    if (guidanceState->turnMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->turnMessage);
        guidanceState->turnMessage = NULL;
    }

    nsl_free(guidanceState);

    return NE_OK;
}

NB_Error
NB_GuidanceProcessorStateReset(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState)
{
    NB_Context *context = guidanceState->context;

    if (guidanceState->prepareMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
        guidanceState->prepareMessage = NULL;
    }
    if (guidanceState->turnMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->turnMessage);
        guidanceState->turnMessage = NULL;
    }

    nsl_memset(guidanceState, 0, sizeof(NB_GuidanceState));
    guidanceState->context = context;
    guidanceState->recalcMode = TRUE;
    guidanceState->currentGuidanceManeuver = NAV_MANEUVER_NONE;
    if (navigationState)
    {
        guidanceState->currentGuidanceManeuver = navigationState->currentRoutePosition.closest_maneuver;
    }

    return NE_OK;
}

nb_boolean isGuidancePointPassed(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, enum typeMessage type, double playbackEndDistance)
{
    double manueverPointOffset = 0.0;

    if (type == tmPrepare)
    {
        if (NE_OK != NB_RouteInformationGetPrepareGuidancePointInformation(navigationState->navRoute,
                                                                           guidanceState->currentGuidanceManeuver,
                                                                           NULL, NULL, &manueverPointOffset))
        {
            return TRUE;
        }
    }
    else if (type == tmTurn)
    {
        if (NE_OK != NB_RouteInformationGetTurnGuidancePointInformation(navigationState->navRoute,
                                                                        guidanceState->currentGuidanceManeuver,
                                                                        NULL, NULL, &manueverPointOffset))
        {
            return TRUE;
        }
    }
    // At the time of starting the message playback, if the message refernces a guidance point,
    // the point must be positioned after expected end of playback. If this not the case,
    // but there is still time to play the message, the client shall reprocess the message without guidance point
    if (playbackEndDistance > manueverPointOffset)
    {
        return FALSE;
    }

    return TRUE;
}

nb_boolean HasContinueMessageToBePlayed(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                NB_NavigateAnnouncementSource source, NB_GuidanceProccessMessagesParameters* parameters)
{
    double                        minGapDistance = 0.0;
    double                        continuePlaybackDistance = 0.0;
    double                        preparePlaybackDistance = 0.0;
    double                        prepareInstructionDistance = 0.0;
    double                        instructionRemainDistance = 0.0;
    double                        allowedStartDistance = 0.0;
    double                        allowedEndDistance = 0.0;
    double                        playbackEndDistance = 0.0;
    NB_Error                      result = NE_OK;
    double                        speed = 0.0;
    NB_RoutePrivateConfiguration* privateConfig = NULL;
    uint32                        currentManeuverIndex = 0;
    data_nav_maneuver*            maneuver = NULL;

    if (guidanceState->continueInfo.playbackTime == 0.0 ||
        (guidanceState->currentGuidanceManeuver == NAV_MANEUVER_START && navigationState->transportMode != NB_NTM_Pedestrian) ||
        ((source == NB_NAS_Automatic || source == NB_NAS_Recalculation)
        && (guidanceState->continueInfo.state == gmsPlayed || guidanceState->continueInfo.state == gmsSkipped)))
    {
        return FALSE;
    }

    currentManeuverIndex = guidanceState->currentGuidanceManeuver;

    maneuver = NB_RouteInformationGetManeuver(navigationState->navRoute, currentManeuverIndex);
    if (!maneuver)
    {
        return FALSE;
    }

    result = NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                        NULL, NULL, NULL, NULL);

    result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         &prepareInstructionDistance,
                                                                         NULL,
                                                                         NULL);

    if (result != NE_OK)
    {
        return FALSE;
    }

    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    speed = (navigationState->currentSpeed + maneuver->speed) / 2;
    if (source != NB_NAS_Button && navigationState->transportMode != NB_NTM_Pedestrian)
    {
        minGapDistance = privateConfig->prefs.minGapBetweenContinueAndPrepareAnnounce * speed;
    }

    continuePlaybackDistance = guidanceState->continueInfo.playbackTime * speed;
    preparePlaybackDistance = guidanceState->prepareInfo.playbackTime * speed;

    // continue message should be played after maneuver is changed
    allowedStartDistance = instructionRemainDistance;
    // continue message should be finished at least with setting gap before start of the prepare message
    allowedEndDistance   = preparePlaybackDistance + prepareInstructionDistance + minGapDistance;
    playbackEndDistance  = instructionRemainDistance - continuePlaybackDistance;
    guidanceState->continueInfo.allowedPlay = (playbackEndDistance >= allowedEndDistance ||
                                               (playbackEndDistance > 0  && navigationState->transportMode == NB_NTM_Pedestrian));
    guidanceState->continueInfo.allowedStartDistance = allowedStartDistance;
    if (instructionRemainDistance <= allowedStartDistance)
    {
        if (playbackEndDistance >= allowedEndDistance ||
            (playbackEndDistance > 0  && navigationState->transportMode == NB_NTM_Pedestrian))
        {
            return TRUE;
        }

        guidanceState->continueInfo.state = gmsSkipped;
        guidanceState->prepareInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
        ResetPreparePlaybackTime(guidanceState, navigationState, parameters);
    }

    return FALSE;
}

nb_boolean HasPrepareMessageToBePlayed(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                NB_NavigateAnnouncementSource source, NB_GuidanceProccessMessagesParameters* parameters)
{
    double                  twoSecGapDistance = 0.0;
    double                  preparePlaybackDistance = 0.0;
    double                  turnPlaybackDistance = 0.0;
    double                  prepareInstructionDistance = 0.0;
    double                  turnInstructionDistance = 0.0;
    double                  instructionRemainDistance = 0.0;
    double                  maximumInstructionDistance = 0.0;
    double                  allowedStartDistance = 0.0;
    double                  allowedEndDistance = 0.0;
    double                  playbackEndDistance = 0.0;
    NB_Error                result = NE_OK;
    double                  speed = 0.0;

    if (guidanceState->prepareInfo.playbackTime == 0.0 ||
        guidanceState->currentGuidanceManeuver == NAV_MANEUVER_START ||
        ((source == NB_NAS_Automatic || source == NB_NAS_Recalculation)
         && (guidanceState->prepareInfo.state == gmsPlayed || guidanceState->prepareInfo.state == gmsSkipped)))
    {
        return FALSE;
    }

    result = NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                        NULL, NULL, &maximumInstructionDistance, NULL);

    result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         &prepareInstructionDistance,
                                                                         NULL,
                                                                         &turnInstructionDistance);

    if (result != NE_OK)
    {
        return FALSE;
    }

    speed = MAX(navigationState->currentSpeed, MIN_SPEED);
    if (source != NB_NAS_Button)
    {
        twoSecGapDistance = TWO_SEC_FOR_GAP * speed;
    }

    // Move prepare instruction to occur at the start of the maneuver
    // if continue message is skipped
    if (guidanceState->continueInfo.state == gmsSkipped)
    {
        allowedStartDistance = instructionRemainDistance;
    }
    else
    {
        // prepare message should try to finish at Prepare Instruction Distance
        allowedStartDistance = preparePlaybackDistance + prepareInstructionDistance;
    }
    preparePlaybackDistance = guidanceState->prepareInfo.playbackTime * speed;
    playbackEndDistance  = instructionRemainDistance - preparePlaybackDistance;

    // disable natural guidance and reset message
    // if guidance point will be occur after end of playback
    if (!(guidanceState->prepareInfo.disablingFilter & DISABLE_NATURAL_GUIDANCE_MASK) &&
        isGuidancePointPassed(guidanceState, navigationState, tmPrepare, playbackEndDistance))
    {
        guidanceState->prepareInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
        ResetPreparePlaybackTime(guidanceState, navigationState, parameters);
        preparePlaybackDistance = guidanceState->prepareInfo.playbackTime * speed;
        // prepare message MUST be finished at least 2-second gap before start of the turn message
        playbackEndDistance  = instructionRemainDistance - preparePlaybackDistance;
    }

    // prepare message MUST be finished at least 2-second gap before start of the turn message
    turnPlaybackDistance = guidanceState->turnInfo.playbackTime * speed;
    if (guidanceState->turnInfo.allowedStartDistance > 0)
    {
         allowedEndDistance = guidanceState->turnInfo.allowedStartDistance;
    }
    else if (maximumInstructionDistance > 0.0)
    {
        allowedEndDistance = turnPlaybackDistance + MIN(turnInstructionDistance, maximumInstructionDistance) + twoSecGapDistance;
    }
    else
    {
        allowedEndDistance = turnPlaybackDistance + turnInstructionDistance + twoSecGapDistance;
    }
    guidanceState->prepareInfo.allowedStartDistance = allowedStartDistance;
    guidanceState->prepareInfo.allowedPlay = playbackEndDistance >= allowedEndDistance;

    if (instructionRemainDistance <= allowedStartDistance)
    {
        if (playbackEndDistance >= allowedEndDistance)
        {
            return TRUE;
        }
        guidanceState->prepareInfo.state = gmsSkipped;
        guidanceState->laneInfo.state = gmsPending;
    }
    else if (guidanceState->continueInfo.state == gmsSkipped && playbackEndDistance >= allowedEndDistance)
    {
        return TRUE;
    }
    else
    {
        // message should not be played too early
        return FALSE;
    }

    return FALSE;
}

nb_boolean HasTurnMessageToBePlayed(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                    NB_NavigateAnnouncementSource source, NB_GuidanceProccessMessagesParameters *parameters)
{
    double                  oneSecGapDistance = 0.0;
    double                  threeSecGapDistance = 0.0;
    double                  turnPlaybackDistance = 0.0;
    double                  turnInstructionDistance = 0.0;
    double                  instructionRemainDistance = 0.0;
    double                  maximumInstructionDistance = 0.0;
    double                  allowedStartDistance = 0.0;
    double                  allowedEndDistance = 0.0;
    double                  playbackEndDistance = 0.0;
    double                  speed = 0.0;
    NB_Error                result = NE_OK;

    // off route case for origin
    if (navigationState->onRoute == FALSE
        && guidanceState->currentGuidanceManeuver == NAV_MANEUVER_START
        && guidanceState->turnInfo.state != gmsPlayed)
    {
        return TRUE;
    }

    if (guidanceState->turnInfo.playbackTime == 0.0 ||
        ((source == NB_NAS_Automatic || source == NB_NAS_Recalculation)
         && guidanceState->turnInfo.state == gmsPlayed))
    {
        return FALSE;
    }

    result = NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                        NULL, NULL, &maximumInstructionDistance, NULL);

    result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         &turnInstructionDistance);

    if (result != NE_OK)
    {
        return FALSE;
    }

    speed = MAX(navigationState->currentSpeed, MIN_SPEED);
    if (source != NB_NAS_Button)
    {
        oneSecGapDistance = ONE_SEC_FOR_GAP * speed;
        threeSecGapDistance = THREE_SEC_FOR_GAP * speed;
    }

    // handle destination maneuvers with old way
    if (NB_RouteParametersGetCommandType(navigationState->navRoute->routeParameters,
                                         navigationState->turnCode) == maneuver_type_dest)
    {
        guidanceState->turnInfo.allowedPlay = TRUE;
        if (instructionRemainDistance <= MIN(maximumInstructionDistance, turnInstructionDistance))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    // check full turn message
    turnPlaybackDistance = guidanceState->turnInfo.playbackTime * speed;

    // use the special code for street count guidance point
    // @todo: get allowedStartDistance and allowedEndDistance once for street count GP
    if (!(guidanceState->turnInfo.disablingFilter & DISABLE_NATURAL_GUIDANCE_MASK) &&
        NE_OK == NB_RouteInformationGetDistancesForStreetCountGuidancePoint(navigationState->navRoute,
                                                                            guidanceState->currentGuidanceManeuver,
                                                                            &allowedStartDistance, &allowedEndDistance))
    {
        double turnTriggerDistance = 0.0;

        turnTriggerDistance = allowedEndDistance + turnPlaybackDistance + threeSecGapDistance;
        if (allowedStartDistance >= turnTriggerDistance)
        {
            allowedStartDistance = turnTriggerDistance;
        }
        playbackEndDistance  = instructionRemainDistance - turnPlaybackDistance;
        guidanceState->turnInfo.allowedStartDistance = allowedStartDistance;
        guidanceState->turnInfo.allowedPlay = playbackEndDistance >= allowedEndDistance;
        if (instructionRemainDistance > allowedStartDistance)
        {
            // message should not be played too early
            return FALSE;
        }
        if (playbackEndDistance >= allowedEndDistance)
        {
            return TRUE;
        }
        else
        {
            // skip street count and use regular turn
            guidanceState->turnInfo.disablingFilter |= ~DISABLE_DESTINATION_MASK;
            guidanceState->stackedDisablingFilter |= ~DISABLE_DESTINATION_MASK;
            guidanceState->turnInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
            guidanceState->stackedDisablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
            ResetTurnPlaybackTime(guidanceState, navigationState, parameters);
            turnPlaybackDistance = guidanceState->turnInfo.playbackTime * speed;
        }
    }

    allowedEndDistance   = oneSecGapDistance;
    // turn message should try to finish at Turn Instruction Distance
    allowedStartDistance = turnPlaybackDistance + turnInstructionDistance;
    if (maximumInstructionDistance > 0)
    {
        allowedStartDistance = MIN(maximumInstructionDistance, allowedStartDistance);
    }
    // turn message MUST be finished at least 1-second gap before maneuver point
    playbackEndDistance  = instructionRemainDistance - turnPlaybackDistance;

    guidanceState->turnInfo.allowedStartDistance = allowedStartDistance;

    // disable natural guidance and reset message
    // if guidance point will be occur after end of playback
    if (!(guidanceState->turnInfo.disablingFilter & DISABLE_NATURAL_GUIDANCE_MASK) &&
        isGuidancePointPassed(guidanceState, navigationState, tmTurn, playbackEndDistance))
    {
        guidanceState->turnInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
        ResetTurnPlaybackTime(guidanceState, navigationState, parameters);
        turnPlaybackDistance = guidanceState->turnInfo.playbackTime * speed;
        playbackEndDistance  = instructionRemainDistance - turnPlaybackDistance;
    }
    guidanceState->turnInfo.allowedPlay = TRUE;
    if (instructionRemainDistance > allowedStartDistance)
    {
        // message should not be played too early
        return FALSE;
    }
    if (playbackEndDistance >= allowedEndDistance)
    {
        return TRUE;
    }

#define CHECK_TURN(DISABLE_MASK)                                                                         \
    if (!(guidanceState->turnInfo.disablingFilter & DISABLE_MASK))                                   \
    {                                                                                                \
        guidanceState->turnInfo.disablingFilter |= DISABLE_MASK;                                     \
        guidanceState->stackedDisablingFilter |= DISABLE_MASK;                                       \
        ResetTurnPlaybackTime(guidanceState, navigationState, parameters);                           \
        turnPlaybackDistance = guidanceState->turnInfo.playbackTime * speed;                         \
        playbackEndDistance  = instructionRemainDistance - turnPlaybackDistance;                     \
        if (playbackEndDistance >= allowedEndDistance)                                               \
        {                                                                                            \
            return TRUE;                                                                             \
        }                                                                                            \
    }

    // disable natural guidance
    CHECK_TURN(DISABLE_NATURAL_GUIDANCE_MASK)
    // disable lane guidance and reset message
    CHECK_TURN(DISABLE_LANE_GUIDANCE_MASK)
    // remove destination of stacked maneuver
    CHECK_TURN(DISABLE_STACKED_DESTINATION_MASK)
    // remove stacked instruction
    CHECK_TURN(DISABLE_STACKED_INSTRUCTION_MASK)
    // remove destination of current maneuver
    CHECK_TURN(DISABLE_DESTINATION_MASK)

#undef CHECK_TURN

    // play something if turn message does not fit
    return TRUE;
}


nb_boolean HasMessageToBePlayed(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, enum typeMessage type, double messagePlaybackTime,
                                NB_NavigateAnnouncementSource source, NB_GuidanceProccessMessagesParameters *parameters)
{
    double oneSecGapDistance = 0.0;
    double instructionRemainDistance = 0.0;
    double playbackEndDistance = 0.0;
    double speed = 0.0;

    if ( !guidanceState || !navigationState || type == tmNone || !parameters)
    {
        return FALSE;
    }

    switch (type)
    {
        case tmContinue: // CONTINUE
        {
            return HasContinueMessageToBePlayed(guidanceState, navigationState, source, parameters);
        }
        case tmPrepare: // PREPARE
        {
            return HasPrepareMessageToBePlayed(guidanceState, navigationState, source, parameters);
        }
        case tmTurn: // TURN
        {
            return HasTurnMessageToBePlayed(guidanceState, navigationState, source, parameters);
        }
        default:
            break;
    }

    if (type != tmTraffic || source != NB_NAS_Automatic)
    {
        return FALSE;
    }

    // traffic message should be played with 1 sec gap after announcement played
    if ((navigationState->lastGpsTime - guidanceState->continueInfo.timeWhenMessagePlayed < 1)
        || (navigationState->lastGpsTime - guidanceState->prepareInfo.timeWhenMessagePlayed < 1))
    {
        return FALSE;
    }

    if (NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                   NULL, NULL, NULL, NULL) != NE_OK)
    {
        return FALSE;
    }

    speed = MAX(navigationState->currentSpeed, MIN_SPEED);
    oneSecGapDistance = ONE_SEC_FOR_GAP * speed;
    playbackEndDistance = instructionRemainDistance - messagePlaybackTime * speed - oneSecGapDistance;

    if ((guidanceState->prepareInfo.allowedStartDistance > 0 &&
         instructionRemainDistance > guidanceState->prepareInfo.allowedStartDistance))
    {
        if (playbackEndDistance > guidanceState->prepareInfo.allowedStartDistance)
        {
            return TRUE;
        }
        return FALSE;
    }
    else if ((guidanceState->turnInfo.allowedStartDistance > 0 &&
              instructionRemainDistance > guidanceState->turnInfo.allowedStartDistance))
    {
        if (playbackEndDistance > guidanceState->turnInfo.allowedStartDistance)
        {
            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}

NB_Error CalculatePlaybackTime(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                               NB_GuidanceProccessMessagesParameters* parameters)
{
    NB_Error result = NE_OK;

    if (guidanceState->continueInfo.playbackTime == 0.0)
    {
        result = NB_GuidancePreprocessMessages(guidanceState, navigationState, parameters,
                                      NB_NMP_Continue, &guidanceState->continueInfo.playbackTime, NULL);
        result = result == NE_NOENT ? NE_OK : result;
    }
    if (guidanceState->prepareInfo.playbackTime == 0.0 || !guidanceState->prepareMessage || guidanceState->prepareInfo.announcementType != parameters->type)
    {
        if (guidanceState->prepareMessage)
        {
            NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
            guidanceState->prepareMessage = NULL;
        }
        result = result ? result : NB_GuidancePreprocessMessages(guidanceState, navigationState, parameters,
                                      NB_NMP_Prepare, &guidanceState->prepareInfo.playbackTime, &guidanceState->prepareMessage);
        result = result == NE_NOENT ? NE_OK : result;
    }
    if (guidanceState->turnInfo.playbackTime == 0.0 || !guidanceState->turnMessage || guidanceState->turnInfo.announcementType != parameters->type)
    {
        if (guidanceState->turnMessage)
        {
            NB_GuidanceMessageDestroy(guidanceState->turnMessage);
            guidanceState->turnMessage = NULL;
        }
        result = result ? result : NB_GuidancePreprocessMessages(guidanceState, navigationState, parameters,
                                      NB_NMP_Turn, &guidanceState->turnInfo.playbackTime, &guidanceState->turnMessage);
        result = result == NE_NOENT ? NE_OK : result;
    }
    return result;
}

NB_Error ResetPreparePlaybackTime(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                                  NB_GuidanceProccessMessagesParameters* parameters)
{
    guidanceState->prepareInfo.playbackTime = 0.0;
    if (guidanceState->prepareMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
        guidanceState->prepareMessage = NULL;
    }
    return CalculatePlaybackTime(guidanceState, navigationState, parameters);
}

NB_Error ResetTurnPlaybackTime(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                               NB_GuidanceProccessMessagesParameters* parameters)
{
    guidanceState->turnInfo.playbackTime = 0.0;
    if (guidanceState->turnMessage)
    {
        NB_GuidanceMessageDestroy(guidanceState->turnMessage);
        guidanceState->turnMessage = NULL;
    }
    return CalculatePlaybackTime(guidanceState, navigationState, parameters);
}

NB_DEF NB_Error
NB_GuidanceProcessorUpdate(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState,
                           NB_GuidanceProccessMessagesParameters* parameters)
{
    NB_Error                result = NE_OK;

    double                  continueDistance = 0.0;
    double                  prepareInstructionDistance = 0.0;
    double                  turnInstructionDistance = 0.0;

    double                  instructionRemainDistance = 0.0;
    double                  instructionReferenceSpeed = 0.0;
    enum manuever_type      instructionManeuverType = manuever_type_normal;
    double                  totalManeuverDistance = 0.0;
    NB_NavigateAnnouncementSource source = NB_NAS_Automatic;

    if ( !guidanceState || !navigationState || !parameters)
    {
        return NE_INVAL;
    }

    if (navigationState->currentRoutePosition.closest_maneuver != guidanceState->currentGuidanceManeuver)
    {
        nb_boolean needUpdate = TRUE;
        uint32 oldManeuverIndex = 0;

        if (!NB_RouteInformationIsManeuverSignificant(navigationState->navRoute, navigationState->currentRoutePosition.closest_maneuver))
        {
            uint32 nextSignificant = 0;
            NB_RouteInformationGetNextSignificantManeuverIndex(navigationState->navRoute, navigationState->currentRoutePosition.closest_maneuver, &nextSignificant);

            if (guidanceState->currentGuidanceManeuver == nextSignificant)
            {
                needUpdate = FALSE;
            }
            else
            {
                oldManeuverIndex = guidanceState->currentGuidanceManeuver;
                guidanceState->currentGuidanceManeuver = nextSignificant;
            }
        }
        else
        {
            guidanceState->currentGuidanceManeuver = navigationState->currentRoutePosition.closest_maneuver;
        }

        if (needUpdate)
        {
            NB_Context *context = guidanceState->context;
            NB_GuidancePointType type = NBGPT_Invalid;
            nb_boolean recalcMode = guidanceState->currentGuidanceManeuver == NAV_MANEUVER_NONE ? TRUE : FALSE;
            nb_boolean stackedInstructionPlayed = guidanceState->stackedInstructionOfPreviousManeuverPlayed;
            int maneuver = guidanceState->currentGuidanceManeuver;
            nb_boolean hasFirstManeuverPlayed = guidanceState->hasFirstManeuverPlayed;

            if (guidanceState->prepareMessage)
            {
                NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
            }
            if (guidanceState->turnMessage)
            {
                NB_GuidanceMessageDestroy(guidanceState->turnMessage);
            }
            nsl_memset(guidanceState, 0, sizeof(NB_GuidanceState));
            guidanceState->context = context;
            guidanceState->recalcMode = recalcMode;
            guidanceState->currentGuidanceManeuver = maneuver;
            guidanceState->hasFirstManeuverPlayed = hasFirstManeuverPlayed;
            guidanceState->continueInfo.allowedPlay = FALSE;
            guidanceState->continueInfo.headupPlayed = FALSE;
            guidanceState->prepareInfo.allowedPlay = FALSE;
            guidanceState->prepareInfo.headupPlayed = FALSE;
            guidanceState->turnInfo.allowedPlay = FALSE;
            guidanceState->turnInfo.headupPlayed = FALSE;
            // do not play prepare message of the current maneuver
            // if stack instruction of previous maneuver was played
            if (stackedInstructionPlayed
            // There is the situation when Position Processor skips one short maneuver.
            // In that case we should not skip prepare message
                && navigationState->currentRoutePosition.closest_maneuver - oldManeuverIndex == 1)
            {
                guidanceState->prepareInfo.state = gmsSkipped;
                guidanceState->laneInfo.state = gmsPending;
            }

            if (NE_OK != NB_RouteInformationGetPrepareGuidancePointInformation(navigationState->navRoute,
                                                                               guidanceState->currentGuidanceManeuver,
                                                                               &type, NULL, NULL) || type == NBGPT_Invalid)
            {
                guidanceState->prepareInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
            }
            if (NE_OK != NB_RouteInformationGetTurnGuidancePointInformation(navigationState->navRoute,
                                                                            guidanceState->currentGuidanceManeuver,
                                                                            &type, NULL, NULL) || type == NBGPT_Invalid)
            {
                guidanceState->turnInfo.disablingFilter |= DISABLE_NATURAL_GUIDANCE_MASK;
            }
            //@ todo: it will be better check lane guidance also
        }
    }

    CalculatePlaybackTime(guidanceState, navigationState, parameters);

    result = result ? result : NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                &instructionReferenceSpeed, &instructionManeuverType, NULL,
                &totalManeuverDistance);

    result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                         NULL,
                                                                         &continueDistance,
                                                                         NULL,
                                                                         &prepareInstructionDistance,
                                                                         NULL,
                                                                         &turnInstructionDistance);
    if (result != NE_OK)
    {
        if (guidanceState->recalcMode)
        {
            return NE_OK;
        }

        return result;
    }
    if (guidanceState->recalcMode)
    {
        source = NB_NAS_Recalculation;
    }

    // check CONTINUE message
    if (HasMessageToBePlayed(guidanceState, navigationState, tmContinue, 0, source, parameters))
    {
        guidanceState->continueInfo.state = gmsPending;
        NB_QaLogGuidanceMessageTrigger(navigationState->context, source, NB_GMT_Guidance,
           continueDistance, instructionRemainDistance, instructionReferenceSpeed, totalManeuverDistance);

    }
    // check PREPARE message
    else if (HasMessageToBePlayed(guidanceState, navigationState, tmPrepare, 0, source, parameters) &&
        navigationState->transportMode != NB_NTM_Pedestrian)
    {
        guidanceState->prepareInfo.state = gmsPending;
        guidanceState->laneInfo.state = gmsPending;
        NB_QaLogGuidanceMessageTrigger(navigationState->context, source, NB_GMT_Guidance,
                                       prepareInstructionDistance, instructionRemainDistance, instructionReferenceSpeed, totalManeuverDistance);
    }
    // check TURN message
    else if (HasMessageToBePlayed(guidanceState, navigationState, tmTurn, 0, source, parameters))
    {
        guidanceState->turnInfo.state = gmsPending;
        NB_QaLogGuidanceMessageTrigger(navigationState->context, source, NB_GMT_Guidance,
                                       turnInstructionDistance, instructionRemainDistance, instructionReferenceSpeed, totalManeuverDistance);
    }
    return result;
}

NB_DEF nb_boolean
NB_GuidanceIsMessagePending(NB_GuidanceState* guidanceState)
{
    if (!guidanceState)
    {
        return FALSE;
    }

    return (nb_boolean)(guidanceState->continueInfo.state == gmsPending || guidanceState->prepareInfo.state == gmsPending || guidanceState->turnInfo.state == gmsPending);
}

NB_DEF NB_Error
NB_GuidanceGetPendingMessage(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation,
                             NB_TrafficInformation* trafficInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};
    uint32 maneuverIndex = 0;

    if (message == NULL)
    {
        return NE_INVAL;
    }

    *message = NULL;

    if (!NB_GuidanceIsMessagePending(guidanceState))
    {
        return NE_NOENT;
    }

    error = NB_NavigationStateGetRoutePositionInformation(navigationState, &routePositionInformation);
    if (routePositionInformation.closestManeuver == 0 && navigationState->recalcCount == 0 &&
        !guidanceState->hasFirstManeuverPlayed)
    {
        maneuverIndex = NAV_MANEUVER_INITIAL;
    }
    else
    {
        maneuverIndex = routePositionInformation.closestManeuver;
    }
    dataSourceOptions.route = navigationState->navRoute;
    dataSourceOptions.collapse = TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.trafficInfo = trafficInformation;
    dataSourceOptions.navState = navigationState;
    dataSourceOptions.guidanceState = guidanceState;

    if (guidanceState->continueInfo.state == gmsPending)
    {
        dataSourceOptions.maneuverPos = NB_NMP_Continue;
        error = NB_GuidanceInformationGetTurnAnnouncement(guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions,
                                                          maneuverIndex, dataSourceOptions.maneuverPos, NB_NAS_Automatic, type, units, message, &guidanceState->continueInfo.soundTypes);
        if(error == NE_OK)
        {
            NB_GuidanceMessageSetManeuverPos(*message, NB_NMP_Continue);
        }
    }
    else if (guidanceState->prepareInfo.state == gmsPending)
    {
        if (guidanceState->prepareMessage &&
            !(guidanceState->prepareInfo.soundTypes & REAL_TIME_DATA_MASK) &&
            type == guidanceState->prepareInfo.announcementType &&
            maneuverIndex != NAV_MANEUVER_INITIAL)
        {
            *message = NB_GuidanceMessageClone(guidanceState->prepareMessage);
        }
        else
        {
            dataSourceOptions.maneuverPos = NB_NMP_Prepare;
            error = NB_GuidanceInformationGetTurnAnnouncement(guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions,
                                                              maneuverIndex, dataSourceOptions.maneuverPos, NB_NAS_Automatic, type, units, message, &guidanceState->prepareInfo.soundTypes);
        }
        if(error == NE_OK)
        {
            NB_GuidanceMessageSetManeuverPos(*message, NB_NMP_Prepare);
        }
    }
    else if (guidanceState->turnInfo.state == gmsPending)
    {
        if (guidanceState->turnMessage &&
            !(guidanceState->turnInfo.soundTypes & REAL_TIME_DATA_MASK) &&
            type == guidanceState->turnInfo.announcementType)
        {
            *message = NB_GuidanceMessageClone(guidanceState->turnMessage);
        }
        else
        {
            dataSourceOptions.maneuverPos = NB_NMP_Turn;
            error = NB_GuidanceInformationGetTurnAnnouncement(guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions,
                                                              routePositionInformation.closestManeuver, dataSourceOptions.maneuverPos, NB_NAS_Automatic, type, units, message, &guidanceState->turnInfo.soundTypes);
        }
        if(error == NE_OK)
        {
            NB_GuidanceMessageSetManeuverPos(*message, NB_NMP_Turn);
        }
    }

    if (error == NE_OK && *message != NULL)
    {
        navigationState->messageCallbackData.messageState = guidanceState;
        navigationState->messageCallbackData.navigationState = navigationState;
        error = NB_GuidanceMessageSetPlayedCallback(*message, NB_GuidanceMessagePlayedCallback, &navigationState->messageCallbackData);
    }

    guidanceState->hasFirstManeuverPlayed = TRUE;

    return error;
}


static void
NB_GuidanceMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_GuidanceState* guidanceState = messagePlayedCallbackData ? messagePlayedCallbackData->messageState : NULL;
    NB_NavigationState* navigationState = messagePlayedCallbackData ? messagePlayedCallbackData->navigationState : NULL;
    double instructionRemainDistance = 0.0;

    if (!guidanceState || !navigationState)
    {
        return;
    }

    if (NE_OK != NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                        NULL, NULL, NULL, NULL))
    {
        return;
    }

    if (guidanceState->continueInfo.state == gmsPending)
    {
        guidanceState->continueInfo.state = gmsPlayed;
        guidanceState->continueInfo.timeWhenMessagePlayed = navigationState->lastGpsTime;

        // Reset prepare and turn message to use if-continue-message-played rule
        guidanceState->prepareInfo.playbackTime = 0.0;
        if (guidanceState->prepareMessage)
        {
            NB_GuidanceMessageDestroy(guidanceState->prepareMessage);
            guidanceState->prepareMessage = NULL;
        }
        guidanceState->turnInfo.playbackTime = 0.0;
        if (guidanceState->turnMessage)
        {
            NB_GuidanceMessageDestroy(guidanceState->turnMessage);
            guidanceState->turnMessage = NULL;
        }
    }
    else if (guidanceState->prepareInfo.state == gmsPending)
    {
        guidanceState->prepareInfo.state = gmsPlayed;
        guidanceState->prepareInfo.timeWhenMessagePlayed = navigationState->lastGpsTime;

        // Reset turn message to use if-prepare-message-played rule
        guidanceState->turnInfo.playbackTime = 0.0;
        if (guidanceState->turnMessage)
        {
            NB_GuidanceMessageDestroy(guidanceState->turnMessage);
            guidanceState->turnMessage = NULL;
        }
    }
    else if (guidanceState->turnInfo.state == gmsPending)
    {
        guidanceState->turnInfo.state = gmsPlayed;
        // set flag to TRUE here in order to check it in next maneuver
        if (guidanceState->turnInfo.soundTypes & STACK_SOUND_MASK)
        {
            guidanceState->stackedInstructionOfPreviousManeuverPlayed = TRUE;
        }
    }

    NB_NavigationMessagePlayed(messagePlayedCallbackData->navigationState, message);
}

NB_Error
NB_GuidancePreprocessMessages(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters,
                              NB_NavigateManeuverPos maneuverPos, double* messagePlaybackTime, NB_GuidanceMessage** pMessage)
    {
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};
    NB_GuidanceMessage* message = NULL;
    uint32 soundTypes = 0;

    if (!messagePlaybackTime)
    {
        return NE_INVAL;
    }
    error = NB_NavigationStateGetRoutePositionInformation(navigationState, &routePositionInformation);
    if (routePositionInformation.projLat == INVALID_LATITUDE || routePositionInformation.projLon == INVALID_LONGITUDE)
    {
        routePositionInformation.projLat = navigationState->lastLatitude;
        routePositionInformation.projLon = navigationState->lastLongitude;
    }

    dataSourceOptions.route = navigationState->navRoute;
    dataSourceOptions.collapse = TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.trafficInfo = parameters->trafficInformation;
    dataSourceOptions.maneuverPos = maneuverPos;
    dataSourceOptions.navState = navigationState;
    dataSourceOptions.guidanceState = guidanceState;

    error = error ? error : NB_GuidanceInformationGetTurnAnnouncement(parameters->guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions,
        routePositionInformation.closestManeuver, maneuverPos, NB_NAS_Automatic, parameters->type, parameters->units, &message, &soundTypes);

    if (maneuverPos == NB_NMP_Continue)
    {
        guidanceState->continueInfo.soundTypes = soundTypes;
        guidanceState->continueInfo.announcementType = parameters->type;
    }
    else if (maneuverPos == NB_NMP_Prepare)
    {
        guidanceState->prepareInfo.soundTypes = soundTypes;
        guidanceState->prepareInfo.announcementType = parameters->type;
    }
    else if (maneuverPos == NB_NMP_Turn)
    {
        guidanceState->turnInfo.soundTypes = soundTypes;
        guidanceState->turnInfo.announcementType = parameters->type;
    }
    if (error == NE_OK && message != NULL)
    {
        error = NB_GuidanceCalculatePlaybackTime(message, parameters, messagePlaybackTime);
    }
    if (pMessage)
    {
        *pMessage = message;
    }
    else
    {
        NB_GuidanceMessageDestroy(message);
    }
    return error;
}

nb_boolean NB_GuidanceIsEnoughGapForTheMessage(NB_GuidanceState* guidanceState, NB_GuidanceMessage* message, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters)
{
    double messagePlaybackTime = 0.0;

    if (!guidanceState || !message || !navigationState || !parameters)
    {
        return FALSE;
    }

    if (NB_GuidanceCalculatePlaybackTime(message, parameters, &messagePlaybackTime) == NE_OK)
    {
        return HasMessageToBePlayed(guidanceState, navigationState, tmTraffic, messagePlaybackTime, NB_NAS_Automatic, parameters);
    }

    return FALSE;
}

NB_Error
NB_GuidanceCalculatePlaybackTime(NB_GuidanceMessage* message, NB_GuidanceProccessMessagesParameters* parameters, double* pPlaybackTime)
{
    double playbackTime = 0.0;
    uint32 playbackTimeFromServer = 0;
    int index = 0;
    int count = NB_GuidanceMessageGetCodeCount(message);

    if (!pPlaybackTime || !message)
    {
        return  NE_INVAL;
    }

    if (NB_GuidanceMessageGetPlaybackTime(message) > 0.0)
    {
        *pPlaybackTime = NB_GuidanceMessageGetPlaybackTime(message);
        return NE_OK;
    }

    if (!parameters)
    {
        return  NE_INVAL;
    }

    for (index = 0; index < count; ++index)
    {
        const char* code = NB_GuidanceMessageGetCode(message, index);

        if (!code)
        {
            return NE_NOENT;
        }
        if (nsl_strlen(code) > 0)
        {
            double duration = 0.0;
            if (NB_RouteInformationGetPlayTimeForMessageCode(parameters->routeInformation,
                                                             code, &playbackTimeFromServer) == NE_OK)
            {
                playbackTime += (double)playbackTimeFromServer / 1000.0f;
            }
            else if (NB_GuidanceInformationGetPronunInformation(parameters->guidanceInformation, code,
                                                                NULL, NULL, &duration) == NE_OK)
            {
                playbackTime = playbackTime + duration;
            }
            else if (parameters->getAnnounceSnippetLengthCallback)
            {
                playbackTime = playbackTime +
                parameters->getAnnounceSnippetLengthCallback(code, parameters->getAnnounceSnippetLengthCallbackData);
            }
        }
    }

    NB_GuidanceMessageSetPlaybackTime(message, playbackTime);

    *pPlaybackTime = playbackTime;

    return NE_OK;
}

NB_Error
NB_GuidanceGetTypeMessageByCurrentRoutePosition(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters,
                                                NB_NavigateAnnouncementSource source, NB_NavigateManeuverPos* pMessageType)
{
    NB_NavigateManeuverPos messageType = NB_NMP_Invalid;
    NB_Error result = NE_OK;
    double instructionRemainDistance  = 0.0;
    double prepareInstructionDistance = 0.0;
    double turnInstructionDistance    = 0.0;

    if (!pMessageType)
    {
        return NE_INVAL;
    }

    result = NB_NavigationStateGetInstructionParameters(navigationState, &instructionRemainDistance,
                                                        NULL, NULL, NULL, NULL);
    result = result ? result : NB_NavigationStateGetInstructionDistances(navigationState,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         &prepareInstructionDistance,
                                                                         NULL,
                                                                         &turnInstructionDistance);
    if (guidanceState->prepareInfo.allowedStartDistance > 0.0)
    {
        prepareInstructionDistance = guidanceState->prepareInfo.allowedStartDistance;
    }

    if (guidanceState->turnInfo.allowedStartDistance > 0.0)
    {
        turnInstructionDistance = guidanceState->turnInfo.allowedStartDistance;
    }

    if (instructionRemainDistance > prepareInstructionDistance)
    {
        messageType = NB_NMP_Continue;
    }
    else if (instructionRemainDistance > turnInstructionDistance)
    {
        messageType = NB_NMP_Prepare;
    }
    else
    {
        messageType = NB_NMP_Turn;
    }

    *pMessageType = messageType;

    return result;
}
