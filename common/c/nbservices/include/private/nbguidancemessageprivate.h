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

    @file     nbguidancemessageprivate.h

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

#ifndef GUIDANCEMESSAGEPRIVATE_H
#define GUIDANCEMESSAGEPRIVATE_H

#include "nbguidancemessage.h"
#include "instructset.h"

/*!
    @addtogroup nbguidancemessage
    @{
*/

#define DISABLE_NATURAL_GUIDANCE_MASK     1
#define DISABLE_LANE_GUIDANCE_MASK        2
#define DISABLE_DESTINATION_MASK          4
#define DISABLE_STACKED_INSTRUCTION_MASK  8
#define DISABLE_STACKED_DESTINATION_MASK  16

typedef enum guidanceMessageState
{
    gmsInvalid = 0,
    gmsPending,                      /*!< State indicating that the message should be played as soon as possible for the current maneuver */
    gmsPlayed,                       /*!< State indicating that the message has been played for the current maneuver */
    gmsSkipped                       /*!< State indicating that the message is skipped for the current maneuver */
} guidanceMessageState;

typedef struct NB_GuidanceMessageInternal
{
    guidanceMessageState state;                /*!< It indicates state of the message */
    double playbackTime;                       /*!< PlaybackTime of the message */
    double timeWhenMessagePlayed;              /*!< Time when the message played */
    uint32 disablingFilter;
    uint32 soundTypes;
    double allowedStartDistance;
    nb_boolean allowedPlay;
    nb_boolean headupPlayed;
    NB_NavigateAnnouncementType announcementType;
}NB_GuidanceMessageInternal;

/*! Guidance message played callback 

This callback will be called when NB_GuidanceMessagePlayed is called on the message

@param message The message which was played
@param userData A pointer to opaque user data
*/
typedef void (*NB_GuidanceMessageCallbackFunction)(NB_GuidanceMessage* message, void* userData);


/*! Create and initialize a new GuidanceMessage object

@param context NB_Context
@param source Announcement source
@param type Guidance message type
@param message On success, the newly created message object; NULL otherwise.  A valid object must be destroyed using NB_GuidanceMessageDestroy.
@returns NB_Error
*/
NB_Error
NB_GuidanceMessageCreate(NB_Context* context, NB_NavigateAnnouncementSource source, NB_GuidanceMessageType type, NB_GuidanceMessage** message);

/*! Create copy of GuidanceMessage object

 @param message   A valid object must be destroyed using NB_GuidanceMessageDestroy.
 @returns message On success, the newly created message object; NULL otherwise.
 */
NB_GuidanceMessage*
NB_GuidanceMessageClone(NB_GuidanceMessage* message);

InstructionCodes*
NB_GuidanceMessageGetInstructionCodes(NB_GuidanceMessage* message);


NB_NavigateAnnouncementSource
NB_GuidanceMessageGetSource(NB_GuidanceMessage* message);


/*! Set message qa log played callback for a GuidanceMessage object

 @param message Guidance message object
 @param playedCallback Callback to be called when message played
 @param playedCallbackUserData User data to be passed to callback
 @returns NB_Error
 */
NB_Error
NB_GuidanceMessageSetQaLogPlayedCallback(NB_GuidanceMessage* message, NB_GuidanceMessageCallbackFunction playedCallback, void* playedCallbackUserData);

/*! Set message played callback for a GuidanceMessage object

@param message Guidance message object
@param playedCallback Callback to be called when message played
@param playedCallbackUserData User data to be passed to callback
@returns NB_Error
*/
NB_Error
NB_GuidanceMessageSetPlayedCallback(NB_GuidanceMessage* message, NB_GuidanceMessageCallbackFunction playedCallback, void* playedCallbackUserData);


/*! Get message type of a GuidanceMessage object

@param message A guidance message object
@returns Message type if message not NULL, NB_GMT_None otherwise
*/
NB_GuidanceMessageType
NB_GuidanceMessageGetType(NB_GuidanceMessage* message);

/*! Clear instruction codes of a GuidanceMessage object

@param message A guidance message object
@returns Pointer to cleared GuidanceMessage instruction codes
*/
InstructionCodes*
NB_GuidanceMessageClearInstructionCodes(NB_GuidanceMessage* message);

/*! Get playbacktime of a GuidanceMessage object

 @param message A guidance message object
 @returns playbackTime
 */
double
NB_GuidanceMessageGetPlaybackTime(NB_GuidanceMessage* message);

/*! Set playbacktime for a GuidanceMessage object

 @param message A guidance message object
 @param playbackTime playbackTime
 @returns error
 */
NB_Error
NB_GuidanceMessageSetPlaybackTime(NB_GuidanceMessage* message, double playbackTime);
/*! @} */

#endif // GUIDANCEMESSAGEPRIVATE_H
