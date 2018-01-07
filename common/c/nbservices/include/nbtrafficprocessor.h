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

    @file     nbtrafficprocessor.h
    
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

#ifndef TRAFFICPROCESSOR_H
#define TRAFFICPROCESSOR_H

#include "nbnavigationstate.h"
#include "nbtrafficinformation.h"
#include "nbguidancemessage.h"

/*!
    @addtogroup nbtrafficprocessor
    @{
*/

/*! Traffic setting
*/
typedef struct NB_TrafficSetting
{
    nb_boolean        trafficAnnouncementEnabled;    /*!< traffic announcement is enabled. */
    nb_boolean        trafficIncidentAnnouncementEnabled;   /*!< traffic incident announcement is enabled. */
}NB_TrafficSetting;

typedef struct NB_TrafficState NB_TrafficState;

/*! Update the traffic state

@param state NB_TrafficState
@param location Current GPS position
@returns NB_Error
*/

/*! Updates the Traffic state using the route position

@param trafficState Previously created NB_TrafficState object
@param navigation Navigation Session object
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficProcessorUpdate(NB_TrafficState* trafficState, NB_NavigationState* navigation, NB_TrafficInformation* trafficInformation,  NB_PositionRoutePositionInformation* currentPosition);


/*! Gets next traffic incident on route

@param trafficState Previously created NB_TrafficState object
@param incident NB_Place pointer
@param incidentDetail NB_TrafficIncident Gets the detail of the next incident
@param incidentRouteRemain Gets next incident remain route distance
@param distToIncident Gets the distance from the current position to the next incident
@return NB_Error
*/
NB_DEC NB_Error NB_TrafficProcessorGetNextIncident(NB_TrafficState* trafficState, NB_Place* incident, NB_TrafficIncident* incidentDetail, double* incidentRouteRemain, double* distToIncident);

/*! Gets next traffic congestion information

@param trafficState Previously created NB_TrafficState object
@param severity NB_NavigateTrafficCongestionType gets next congestion severity
@param routeRemain Gets congestion remain route distance
@param speed Gets congestion speed
@param maneuver Gets congestion maneuver
@param length Gets congestion length
@param inCongestion Gets whether or not in congestion
@param distToCongestion Gets the distance from the current position to the next congestion
@return NB_Error
*/
NB_DEC NB_Error NB_TrafficProcessorGetNextCongestionInformation(NB_TrafficState* trafficState, NB_NavigateTrafficCongestionType* severity, double* routeRemain, double* speed, uint32* maneuver, double* length, nb_boolean* inCongestion, double* distToCongestion);

/*! Checks to see if traffic message requirements are met

@param trafficState Previously created NB_TrafficState object
@param messageCongestionPending nb_boolean TRUE if congestion message audio is pending
@param messageIncidentPending nb_boolean TRUE if incident message audio is pending
@return NB_Error
*/
NB_DEC nb_boolean NB_TrafficProcessorIsAnnouncementPending(NB_TrafficState* trafficState);

/*! Retrieve the pending guidance message

*/
NB_DEC NB_Error NB_TrafficGetPendingMessage(NB_TrafficState* trafficState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation,
                                             NB_TrafficInformation* trafficInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);


/*! @} */

#endif
