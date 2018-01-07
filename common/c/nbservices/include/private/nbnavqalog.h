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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#ifndef NB_NAVQALOG_
#define NB_NAVQALOG_

#include "nbexp.h"
#include "paltypes.h"
#include "nbcontext.h"

#include "nbnavigationstate.h"
#include "nbnavigationstateprivate.h"

#include "nbguidancemessageprivate.h"

/*! Guidance Message Triggered

@param context A pointer to the shared NB_Context instance
@param messageType The type of message being triggered.
@param triggerDistance The trigger distance for this message
@param currentDistance The current distance when the message was triggered
*/
NB_DEC void NB_QaLogGuidanceMessageTrigger(NB_Context* context, NB_NavigateAnnouncementSource source,
                                           NB_GuidanceMessageType type, double triggerDistance,
                                           double currentDistance, double currentSpeed,
                                           double totalManeuverDistance);

/*! Guidance Message Retrieved

@param context A pointer to the shared NB_Context instance
@param source  The source of the message
@param message The message retrieved
*/
NB_DEC void NB_QaLogGuidanceMessageRetrieved(NB_Context* context, NB_GuidanceMessage* message);

/*! Guidance Message Played

@param context A pointer to the shared NB_Context instance
@param message The message retrieved
*/
NB_DEC void NB_QaLogGuidanceMessagePlayed(NB_Context* context, NB_GuidanceMessage* message);

/*! Guidance Message Played 2

 @param context A pointer to the shared NB_Context instance
 @param message The message retrieved
 @param maneuverRemaining distance to maneuver point
 @param playtime Actual length in milliseconds
 @param distance Actual length in meters
 */
NB_DEC void NB_QaLogGuidanceMessagePlayed2(NB_Context* context, NB_GuidanceMessage* message, double maneuverRemaining, double playtime, double distance);

/*! Guidance Message Retrieved 2

 @param context A pointer to the shared NB_Context instance
 @param message The message retrieved
 @param maneuverRemaining distance to maneuver point
 @param currentSpeed current speed of vehicle
 @param playtime Actual length in milliseconds
 @param distance Calculated length in meters length at the current speed
 */
NB_DEC void NB_QaLogGuidanceMessageRetrieved2(NB_Context* context, NB_GuidanceMessage* message, double maneuverRemaining, double currentSpeed, double playtime, double distance);

/*! Guidance Message Audio Playtime

 @param context A pointer to the shared NB_Context instance
 @param message The message retrieved
 @param playtime playtime of the message
 */
NB_DEC void NB_QaLogGuidanceAudioPlayTime(NB_Context* context, double playtime, NB_GuidanceMessage* message);

/*! Guidance Message Retrieved

 @param context A pointer to the shared NB_Context instance
 @param turnGap gap after turn message
 @param turn Distance when turn message should be played
 @param prepareGap gap after prepare message
 @param prepare Distance when prepare message should be played
 @param continueGap gap after continue message
 @param turnLength Playback distance of turn message
 @param prepareLength Playback distance of prepare message
 @param isPrepareGuideUsed Will prepare message be played
 @param isTurnGuideUsed Will turn message be played
 */
NB_DEC void NB_QaLogGuidanceTimimgData(NB_Context* context, double turnGap, double turn,
                                       double prepareGap, double prepare,
                                       double continueGap, double turnLength,
                                       double prepareLength, nb_boolean isPrepareGuideUsed,
                                       nb_boolean isTurnGuideUsed);

/*! Log the current NB_NavigationState to the QA Log

@param context A pointer to the shared NB_Context instance
@param state A pointer to the navigation state to log
@param publicState A pointer to the public navigation state
*/
NB_DEC void NB_QaLogNavigationState(NB_Context* context, NB_NavigationState* state, NB_NavigationPublicState* publicState);

/*! Log the current navigation position from the NB_NavigationState to the QA Log

@param context A pointer to the shared NB_Context instance
@param state A pointer to the navigation state containing the position to log
@param action Action name
*/
NB_DEC void NB_QaLogNavigationPosition(NB_Context* context, NB_NavigationState* state, const char* action);

/*! Log a navigation request to the QA Log

@param context A pointer to the shared NB_Context instance
@param destinationLatLong A pointer to the destination lat/long
@param destinationStreet A pointer to the destination street
@param settings A pointer to the route settings
*/
NB_DEC void NB_QaLogNavigationStartup(NB_Context* context, NB_LatitudeLongitude* destinationLatLong, const char* destinationStreet, NB_RouteSettings* settings);

/*! Log an off route notification to the QA Log

@param context A pointer to the shared NB_Context instance
@param offRouteCount The number of off route fixes
@param wrongWayCount The number of wrong way fixes
@param action Action name
*/
NB_DEC void NB_QaLogNavigationOffRoute(NB_Context* context, uint32 offRouteCount, uint32 wrongWayCount, const char* action);


#endif //NB_NAVQALOG_
