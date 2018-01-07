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

    @file     nbguidanceprocessorprivate.h
       
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

#ifndef NBGUIDANCEPROCESSORPRIVATE_H
#define NBGUIDANCEPROCESSORPRIVATE_H

#include "paltypes.h"
#include "nbguidanceprocessor.h"
#include "nbnavigation.h"
#include "nbnavigationstate.h"
#include "navtypes.h"
#include "nbguidancemessageprivate.h"

/*!
    @addtogroup nbguidanceprocessor
    @{
*/


struct NB_GuidanceState
{
    NB_Context* context;                /*!< NB_Context object */

    nb_boolean recalcMode;              /*!< Flag which indicates that messages are played in RECALC mode */

    NB_GuidanceMessageInternal continueInfo; /*!< Info of the continue message */
    NB_GuidanceMessageInternal prepareInfo;  /*!< Info of the prepare message */
    NB_GuidanceMessageInternal turnInfo;     /*!< Info of  the turn message */
    NB_GuidanceMessageInternal laneInfo;     /*!< Info of the lane guidance */

    uint32              currentGuidanceManeuver; /*!< Maneuver number that the Guidance State applies to */
    NB_GuidanceMessage* prepareMessage;          /*!< Prepare message */
    NB_GuidanceMessage* turnMessage;             /*!< Turn message */
    uint32              stackedDisablingFilter;  /*!< Disabling filter of the stacked instruction of a turn message */

    nb_boolean          stackedInstructionOfPreviousManeuverPlayed;
    nb_boolean          hasFirstManeuverPlayed;   /*!< TRUE if M1 guidance massage played, FALSE if it hasn't. */
};

/*! Create a NB_GuidanceState object

@param context NB_Context
@param guidanceState On success, a newly created NB_GuidanceState object; NULL otherwise.  A valid object must be destroyed using NB_GuidanceState()
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceProcessorStateCreate(NB_Context* context, NB_GuidanceState** guidanceState);

/*! Destroy a previously created NB_GuidanceState object

@param guidanceState A NB_GuidanceState object created with any of the NB_GuidanceProcessorStateCreate functions
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceProcessorStateDestroy(NB_GuidanceState* guidanceState);

/*! Reset a NB_GuidanceState object

Reset the guidance state for a new route
@param guidanceState Guidance state to reset 
@returns NB_Error
*/
NB_Error NB_GuidanceProcessorStateReset(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState);

/*! Is enough gap to play message

Is enough gap between continue and prepare messages
or between prepare and turn messages to play message
@param guidanceState Guidance state
@param message Guidance message which should be checked
@param parameters NB_GuidanceProccessMessagesParameters
@returns nb_boolean TRUE - if enough to play
*/

nb_boolean NB_GuidanceIsEnoughGapForTheMessage(NB_GuidanceState* guidanceState, NB_GuidanceMessage* message, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters);

/*! Preprocess turn and prepare messages

Preprocess and calculate playback times for prepare and turn messages
@param navigationState A Navigation state object
@param parameters NB_GuidanceProccessMessagesParameters
@returns NB_Error
*/
NB_Error
NB_GuidancePreprocessMessages(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters,
                              NB_NavigateManeuverPos maneuverPos, double* messagePlaybackTime, NB_GuidanceMessage** message);

/*! calculate playback time for message

calculate playback time for any message
@param message Guidance message the playback time of which should be calculated
@param parameters NB_GuidanceProccessMessagesParameters
@param pPlaybackTime playback time in sec
@returns NB_Error
*/
NB_Error
NB_GuidanceCalculatePlaybackTime(NB_GuidanceMessage* message, NB_GuidanceProccessMessagesParameters* parameters, double* pPlaybackTime);

/*! Determine what type of message should be played

 Determine what type of message should be played
 @param guidanceState Guidance state
 @param navigationState navigation state
 @param parameters NB_GuidanceProccessMessagesParameters
 @param source Automatic, recalc and button
 @param pMessageType message type could be NB_NMP_Continue, NB_NMP_Prepare and NB_NMP_Turn
 @returns NB_Error
 */
NB_Error
NB_GuidanceGetTypeMessageByCurrentRoutePosition(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters,
                                                NB_NavigateAnnouncementSource source, NB_NavigateManeuverPos* pMessageType);

/*! @} */

#endif
