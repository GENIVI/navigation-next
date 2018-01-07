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

    @file     nbtrafficprocessorprivate.h

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

#ifndef NBTRAFFICPROCESSORPRIVATE_H
#define NBTRAFFICPROCESSORPRIVATE_H

#include "paltypes.h"
#include "nbcontext.h"
#include "navtypes.h"
#include "data_traffic_region.h"
#include "nbtrafficprocessor.h"
#include "data_place.h"
#include "data_blob.h"

/*!
    @addtogroup nbtrafficprocessor
    @{
*/

struct NB_TrafficState
{
    NB_Context*                 context;

    NB_TrafficEvent             nextTrafficEvent;       /* current/next traffic event from traffic event list */

    data_traffic_region         nextCongestionRegion;
    data_traffic_region         nextCongestionEndRegion;
    NB_NavigateTrafficCongestionType nextCongestionSeverity;
    double                      nextCongestionRouteRemain;
    double                      nextCongestionSpeed;
    uint32                      nextCongestionManeuver;
    double                      nextCongestionLength;
    nb_boolean                  nextCongestionIn;
    nb_boolean                  nextCongestionMessageReported;
    nb_boolean                  nextIncidentMessageReported;

    data_place                  nextIncidentPlace;
    double                      nextIncidentRouteRemain;
    NB_TrafficIncident          nextIncidentDetails;

    uint32                      lastUpdateNewIncidents;
    uint32                      lastUpdateClearedIncidents;
    int32                       lastUpdateTravelTimeDelta;

    uint32                      currentManeuver;

    nb_boolean                  initialDelayCheckPending;

    nb_boolean                  messageIncidentPending;
    nb_boolean                  messageCongestionPending;
    nb_boolean                  messageAlertPending;

    data_blob                   trafficPronoun;

    double                      prevTrafficMeter;

    NB_NavigateManeuverPos      currentManeuverPos;

    double                      distToCongestion;
    double                      distToIncident;

    uint32                      remainingTravelTime;    /* remaining travel time to destination, in seconds */
    uint32                      remainingTravelDelay;   /* remaining travel delay to destination, in seconds*/
    double                      remainingTravelMeter;   /* remaining travel meter to destination, percentage */

    NB_TrafficSetting           setting;
};

/*! Create a NB_TrafficState object

@param context NB_Context
@param trafficState On success, a newly created NB_TrafficState object; NULL otherwise.  A valid object must be destroyed using NB_TrafficState()
@returns NB_Error
*/
NB_Error NB_TrafficProcessorStateCreate(NB_Context* context, NB_TrafficState** trafficState);

/*! Destroy a previously created NB_TrafficState object

@param trafficState A NB_TrafficState object created with any of the NB_TrafficProcessorStateCreate functions
@returns NB_Error
*/
NB_Error NB_TrafficProcessorStateDestroy(NB_TrafficState* trafficState);


data_blob* NB_TrafficProcessorGetPronoun(NB_TrafficState* pThis, const char* key);


uint32 NB_TrafficProcessorGetNextCongestionManeuverIndex(NB_TrafficState* trafficState);


NB_Error NB_TrafficProcessorStateReset(NB_TrafficState* trafficState);


/*! Gets trip remaining information

@param trafficState Previously created NB_TrafficState object
@param time Gets the trip remaining time in seconds
@param delay Gets the trip remaining delay in seconds
@param meter Gets the trip remaining meter in percentage, meter = 100 - delay percentage
@return NB_Error
 */
NB_Error NB_TrafficProcessorGetTripRemainingInformation(NB_TrafficState* trafficState, uint32* time, uint32* delay, double* meter);


/*! Get next traffic event on route

Gets the next traffic event (incident or congestion) ahead on the current route for display.

@param trafficState Previously created NB_TrafficState object
@param event Pointer to NB_TrafficEvent object to copy next event info to
@returns NB_Error
*/
NB_Error NB_TrafficProcessorGetNextTrafficEvent(NB_TrafficState* trafficState, NB_TrafficEvent* trafficEvent);

/*! Sets traffic state setting

@param trafficState Previously created NB_TrafficState object
@param setting NB_TrafficSetting object contains processor setting
@return NB_Error
*/
NB_Error NB_TrafficProcessorStateSetSetting(NB_TrafficState* trafficState, NB_TrafficSetting* setting);
/*! @} */

#endif
