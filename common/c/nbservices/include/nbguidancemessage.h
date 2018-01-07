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

    @file     nbguidancemessage.h

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

#ifndef GUIDANCEMESSAGE_H
#define GUIDANCEMESSAGE_H

#include "nbexp.h"
#include "nbcontext.h"
#include "navpublictypes.h"

// @todo (BUG 55815) - should this be exposed
#include "csltypes.h" 

/*!
    @addtogroup nbguidancemessage
    @{
*/

/*! Navigation voice message types */
typedef enum
{
    NB_GMT_None = 0,

    NB_GMT_Guidance,
    NB_GMT_SpeedWarning,
    NB_GMT_SchoolZoneWarning,
    NB_GMT_Camera,
    NB_GMT_Traffic,
    NB_GMT_HeadsUp

} NB_GuidanceMessageType;

/*! @struct NB_GuidanceMessage
Information about a Guidance message
*/
typedef struct NB_GuidanceMessage NB_GuidanceMessage;

/*! Destroy a previously created GuidanceInformation object

@param message A previously created NB_GuidanceMessage object.
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceMessageDestroy(NB_GuidanceMessage* message);

/*! Get the number of codes in this NB_GuidanceMessage

@param message A previously created NB_GuidanceMessage object.
@returns int
*/
NB_DEC int NB_GuidanceMessageGetCodeCount(NB_GuidanceMessage* message);

/*! Get a specific code from this NB_GuidanceMessage.

@param message A previously created NB_GuidanceMessage object.
@param index the index of the code to return
@returns const char* the value of the code at the specified index, NULL if the index is invalid
*/
NB_DEC const char* NB_GuidanceMessageGetCode(NB_GuidanceMessage* message, int index);

/*! Called to Indicate that this message has been played successfully to the user.

@param message A previously created NB_GuidanceMessage object.
*/
NB_DEC void NB_GuidanceMessagePlayed(NB_GuidanceMessage* message);

/*! Get guidance Message type. */
NB_DEC NB_GuidanceMessageType
NB_GuidanceMessageGetType(NB_GuidanceMessage* message);

/* get maneuver pos. */
NB_DEC NB_NavigateManeuverPos NB_GuidanceMessageGetManeuverPos(NB_GuidanceMessage* message);

/* set maneuver pos. */
NB_DEC NB_Error NB_GuidanceMessageSetManeuverPos(NB_GuidanceMessage* message, NB_NavigateManeuverPos pos);

/*! @} */

#endif // GUIDANCEMESSAGE_H
