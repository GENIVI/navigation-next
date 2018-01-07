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

    @file     nbguidanceprocessor.h
       
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

#ifndef GUIDANCEPROCESSOR_H
#define GUIDANCEPROCESSOR_H

#include "paltypes.h"
#include "nbnavigationstate.h"
#include "nbguidanceinformation.h"
#include "nbtrafficinformation.h"

/*!
    @addtogroup nbguidanceprocessor
    @{
*/

/*! @struct NB_GuidanceState
Guidance state
*/
typedef struct NB_GuidanceState NB_GuidanceState;

/*! @struct NB_GuidanceProccessMessagesParameters
 Guidance Parameters for Proccess Messages
 */

/*! Callback function to ask client announcement snippet length

 @return the Announcement Snippet length.
 */
typedef
double (* NB_GetAnnounceSnippetLength)(const char* AnnounceCode,          /*!< The code of the ask announce snippet */
                                       void* userData                    /*!< The user data pointer
                                                                           registered with the callback*/
                                       );

typedef struct NB_GuidanceProccessMessagesParameters
{
    NB_GuidanceInformation*      guidanceInformation;
    NB_TrafficInformation*       trafficInformation;
    NB_RouteInformation*         routeInformation;
    NB_NavigateAnnouncementType  type;
    NB_NavigateAnnouncementUnits units;
    NB_GetAnnounceSnippetLength  getAnnounceSnippetLengthCallback;
    void*                        getAnnounceSnippetLengthCallbackData;
} NB_GuidanceProccessMessagesParameters;

/*! Updates the Guidance state using the route position

@param guidanceState Previously created NB_GuidanceState object
@param navigationState Navigation state
@param parameters NB_GuidanceProccessMessagesParameters
@returns NB_Error
*/
NB_DEC NB_Error NB_GuidanceProcessorUpdate(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceProccessMessagesParameters* parameters);

/*! Determine if a guidance message is pending

@param state NB_GuidanceState
@returns nb_boolean
*/
NB_DEC nb_boolean NB_GuidanceIsMessagePending(NB_GuidanceState* guidanceState);


/*! Retrieve the pending guidance message

*/
NB_DEC NB_Error NB_GuidanceGetPendingMessage(NB_GuidanceState* guidanceState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation, 
                                             NB_TrafficInformation* trafficInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);

/*! @} */

#endif
