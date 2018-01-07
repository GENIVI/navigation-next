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

    @file     nbtrafficinformationprivate.h
    
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

#ifndef TRAFFICINFORMATIONPRIVATE_H
#define TRAFFICINFORMATIONPRIVATE_H

#include "nbcontext.h"
#include "nbtrafficinformation.h"
#include "nbrouteinformation.h"
#include "data_traffic_record_identifier.h"
#include "data_traffic_notify_reply.h"
#include "nbnavigationstate.h"

/*!
    @addtogroup nbtrafficinformation
    @{
*/

struct NB_TrafficInformation
{
    NB_Context*                     context;
    double                          trafficMeter;                   /*!< Traffic meters state: 100 = max green, 0 = max red, <0: not available */
    uint32                          travelTime;                     /*!< Travel time in seconds along this maneuver */
    uint32                          trafficDelay;                   /*!< Traffic delay in seconds along this maneuver */ 
    boolean                         isNotify;
    double                          navMinTmcSpeed;
    uint32                          minNoDataDisableTrafficBar;
    data_blob                       routeId;  
    data_traffic_record_identifier  trafficRecordIdentifier;
    data_traffic_notify_reply       dataReply;    

    data_traffic_flow               standardTrafficFlow;
    data_traffic_flow               historicalTrafficFlow;
    data_traffic_flow               realtimeTrafficFlow;
    data_traffic_incidents          trafficIncidents;

    nb_boolean                      replyNotReady;
    nb_boolean                      hasNewInformation;              /*!< true if contains data not yet seen by traffic processor */

    struct CSL_Vector*              trafficEvents;                  /*!< current traffic events on route */
    struct CSL_Vector*              groupedTrafficEvents;           /*!< the same as trafficEvents, but if some events are placed continuously, they will be presented as a single event here */
};

/*! Create an information object from a tps network reply

@param context NAVBuilder Context Instance
@param route previously created route object
@param information On success, a newly created NB_TrafficInformation object; NULL otherwise.  A valid object must be destroyed using NB_TrafficInformationDestroy()
@returns NB_Error
*/
NB_Error NB_TrafficInformationCreateFromRoute(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation** information);

/*! Create an information object from a tps network reply

@param context NAVBuilder Context Instance
@param reply The TPS reply from the server
@param information On success, a newly created NB_TrafficInformation object; NULL otherwise.  A valid object must be destroyed using NB_TrafficInformationDestroy()
@returns NB_Error
*/
NB_Error NB_TrafficInformationCreateFromTPSReply(NB_Context* context, tpselt reply, NB_TrafficInformation** information);

/*! Gets data_traffic_record_identifier from TrafficInformation

@param information Previously created NB_TrafficInformation
@returns data_traffic_record_identifier
*/
data_traffic_record_identifier* NB_TrafficInformationGetRecordIdentifier(NB_TrafficInformation* information);

data_traffic_incidents* NB_TrafficInformationGetTrafficIncidents(NB_TrafficInformation* information);

/*! Gets data_traffic_notify_reply from TrafficInformation

@param information Previously created NB_TrafficInformation
@returns data_traffic_notify_reply
*/
data_traffic_notify_reply* NB_TrafficInformationGetDataReply(NB_TrafficInformation* information);

NB_Error NB_TrafficInformationGetTrafficFlows(NB_TrafficInformation* information, data_traffic_flow** standardTrafficFlow, data_traffic_flow** historicalTrafficFlow, data_traffic_flow** realtimeTrafficFlow);

/*! Merge traffic information

@param destination A NB_TrafficInformation object in which to merge to notify NB_TrafficInformation to (Generally the route traffic information object)
@param source A NB_TrafficInformation object in which to merge to notify NB_TrafficInformation from (Generally the traffic notify object)
@param route Current route information object
@param distance Current distance traveled along the route
@param pNewIncidentCount On success, the number of new incidents from souce is stored here
@param pClearedIncientCount On success, the number of new cleared incidents is stored here
@returns NB_Error
*/
NB_Error NB_TrafficInformationMergeFromNotify(NB_TrafficInformation* destination, NB_TrafficInformation* source, NB_RouteInformation* route, double distance,
                                               int* pNewIncidentCount, int* pClearedIncientCount);

/*! Set the incident distances to the distance from the start of the route

Sets the distance from the start of the route for every place in both the old and new incident vectors

@param information The traffic information object to update
@param navigationState The navigation state object
@return NB_Error
*/
NB_Error NB_TrafficInformationUpdateIncidentDistances(NB_TrafficInformation* information, NB_NavigationState* navigationState);

/*! Check to see if traffic reply not ready

@param information The traffic information object to check
@return Non-zero if reply not ready received from server; zero otherwise
*/
nb_boolean NB_TrafficInformationReplyNotReady(NB_TrafficInformation* information);

/*! Get the speed, color, free flow speed and speed type of a particular TMC traffic location

@param pds  data util state
@param tmcloc  TMC traffic locatioin
@param speed_type which speed data to retrieve
@param minspeed minimum speed
@param pspeed On Success, where speed value will be returned
@param pcolor On success, where the color of the traffic region will be returned
@param pfreeflowspeed On Success, where free flow traffic speed will be returned
@param pspeed_type_out On Success, where the actual type of speed is returned
@param pThis The traffic information object.
@return NB_Error
*/
NB_Error NB_TrafficInformationGetSpeed(data_util_state* pds, const char* tmcloc, TrafficSpeedType speed_type, double minspeed,
                                      double* pspeed, char* pcolor, double* pfreeflowspeed,
                                      TrafficSpeedType* pspeed_type_out, NB_TrafficInformation* pThis);

/*! Get the real time speed and congestion type for a traffic region

@param traffic Traffic information object
@param tmcLocation TMC traffic location
@param speed On success, where speed value will be returned
@param ffspeed Returns free flow speed, can be NULL if not used
@param congestionType On success, where the congestion type will be returned
@return NB_Error
*/
NB_Error NB_TrafficInformationGetTrafficRegionRealTimeSpeed(
    NB_TrafficInformation* traffic,
    const char* tmcLocation,
    double* speed,
    double* ffspeed,
    NB_NavigateTrafficCongestionType* congestionType);


/*! Initialize traffic event object

@param trafficEvent A NB_TrafficEvent object
*/
void NB_TrafficInformationClearTrafficEvent(NB_TrafficEvent* trafficEvent);

/*! Update traffic events state with current position

@param information A NB_TrafficInformation object
@param currentPosition The current position
@returns NB_Error
*/
NB_Error NB_TrafficInformationUpdateTrafficEventsState(NB_TrafficInformation* trafficInformation, NB_PositionRoutePositionInformation* currentPosition);

/*! Get first traffic event occurring within a maneuver

@param information A NB_TrafficInformation object
@param startManeuverIndex The maneuver index to start at
@param endManeuverIndex The maneuver index to end before
@param event Pointer to NB_TrafficEvent object to copy next event info to
@returns NB_Error
*/
NB_Error NB_TrafficInformationGetFirstTrafficEventInManeuvers(NB_TrafficInformation* information, uint32 startManeuverIndex, uint32 endManeuverIndex, NB_TrafficEvent* trafficEvent);

/*! Get array of upcoming traffic events

Allocates and returns an array of upcoming traffic events along the route.  The array must be freed
when no longer needed by calling the NB_TrafficInformationFreeTrafficEvents() function.

@param information A NB_TrafficInformation object
@param numberOfTrafficEvents Pointer to where the traffic event count is to be stored
@param trafficEvents Pointer to where the NB_TrafficEvent array pointer is to be stored
@returns NB_Error
*/
NB_Error NB_TrafficInformationGetUpcomingTrafficEvents(NB_TrafficInformation* information, uint32* numberOfTrafficEvents, NB_TrafficEvent** trafficEvents);

/*! Free allocated array of upcoming traffic events

@param trafficEvents Pointer to the NB_TrafficEvent to be freed
@returns NB_Error
*/
NB_Error NB_TrafficInformationFreeTrafficEvents(NB_TrafficEvent* trafficEvents);

/*! Clear traffic event object

@param trafficEvent A NB_TrafficEvent object
*/
void NB_TrafficInformationClearTrafficEvent(NB_TrafficEvent* trafficEvent);

/*! Get traffic event by index

@param information A NB_TrafficInformation object
@param eventIndex The index of the traffic event to get
@param trafficEvents Pointer to where the NB_TrafficEvent is to be stored
@returns NB_Error
*/
NB_Error NB_TrafficInformationGetTrafficEvent(NB_TrafficInformation* information, uint32 eventIndex, NB_TrafficEvent* trafficEvent);

NB_Error NB_TrafficInformationGetFirstTrafficEvent(NB_TrafficInformation* information, uint32 startManeuverIndex, NB_TrafficEvent* trafficEvent);

/*! @} */

#endif

