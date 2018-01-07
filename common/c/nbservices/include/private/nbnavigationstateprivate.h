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

    @file     nbnavigationstateprivate.h
    @defgroup nbnavigation nbnavigation
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

#ifndef NBNAVIGATIONSTATEPRIVATE_H_
#define NBNAVIGATIONSTATEPRIVATE_H_

#include "nbcontext.h"
#include "nbgpshistory.h"
#include "nbrouteinformation.h"
#include "nbtrafficinformation.h"
#include "nbtrafficinformationprivate.h"
#include "nbguidancemessageprivate.h"
#include "navpublictypes.h"
#include "navtypes.h"
#include "nav_avg_speed.h"
#include "nbnavigationstate.h"
#include "data_traffic_flow.h"
#include "data_traffic_region.h"
#include "navprefs.h"

#define NB_NAVIGATION_STATE_TURN_CODE_LEN 12
#define NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN 16

typedef struct MessagePlayedCallbackData
{
    NB_NavigationState* navigationState;
    void*               messageState;
} MessagePlayedCallbackData;

/*!
    @addtogroup nbnavigationstate
    @{
*/

struct NB_NavigationState
{
    NB_Context*                 context;                    /*!< NBContext pointer */
    NB_RouteInformation*        navRoute;                   /*!< Route object associated with this nav state */

    NB_NavigateTransportMode    transportMode;              /*!< Specifies whether navigation is car or pedestrian */

    double			            lastHeading;                /*!< Last heading value */
    uint32			            lastHeadingUTC;             /*!< Last heading time code */
    nb_gpsTime                  lastGpsTime;                /*!< GPS time of the last fix */
    double                      lastLatitude;               /*!< Last latitude value */
    double                      lastLongitude;              /*!< Last longitude value */

    struct route_position	    currentRoutePosition;       /*!< Current route position information */

    uint32			            manueverProgress;           /*!< Current maneuver */
    uint32			            segmentProgress;            /*!< Current segment */

    NB_NavigateManeuverPos      maneuverPos;                /*!< Current maneuver position */

    nb_boolean                  onRoute;                    /*!< Specifies whether or not nav state is on route */

    double                      currentSpeed;               /*!< Current speed (obtained from last fix) */

    uint32                      remainingRouteTime;         /*!< Time remaining on current route */
    double                      remainingRouteDist;         /*!< Distance remaining on corrent route */

    struct nav_avg_speed        averageSpeed;               /*!< Average speed of nav state */

  	nb_boolean		            initialRouteMatch;          /*!< TRUE when the user's position has been matched to route */
	int				            initialRouteMatchesNeeded;  /*!< The number of time a user's position must be matched to the route to be considered "On Route" */

    int                         offRouteCount;              /*!< The number of positions that have been off route since the last on route position */
    int                         wrongWayCount;              /*!< The number of positions that have been wrong way since the last on route, right way position */

    nb_boolean                  pastArrivingThreshold;      /*!< TRUE when the user has passed the threshold distance to transition to the "Arriving" state */
    nb_boolean                  pastArrivedThreshold;       /*!< TRUE when the user has passed the threshold distance to transition to the "Arrived" state */

    nb_boolean                  firstFixProcessed;          /*!< TRUE when the first GPS fix has been processed after a new route is received. */

    nb_boolean                  newRoute;                   /*!< TRUE if a new route has just been applied to this navigation session. */

    nb_boolean                  applyingFullRoute;          /*!< TRUE if we are applying the full route to nav session with a partial route */

    nb_boolean                  applyingNewRoute;           /*!< TRUE when a new route is being applied to nav session */

    nb_boolean                  positionUpdated;            /*!< TRUE if the route position has been updated as a result of the last GPS fix. */

    NB_Error                    error;                      /*!< Error code which caused this session to enter an error state */
    NB_NavigateRouteError       routeError;                 /*!< Route error code which caused this session to enter an error state */

    int	                        recalcCount;

    char			            turnCode[NB_NAVIGATION_STATE_TURN_CODE_LEN];    /*!< Current maneuver turn code */
    char			            stackCode[NB_NAVIGATION_STATE_TURN_CODE_LEN];   /*!< Current maneuver stack turn code */
    char                        speedSignId[NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN + 1];

    uint32                      trafficNotifyRetries;

    uint32                      completeRouteDownloadRetries;

    nb_boolean                  snapMap;

    NB_GuidanceMessage*         currentMessage;             /*!< Guidance message currently playing */
    MessagePlayedCallbackData   messageCallbackData;
    nb_boolean                  wasNetworkBlocked;          /*!< TRUE when network was blocked */
    double                      gpsFixMinimumSpeed;         /*!< Minimum speed, in meters per second, for a GPS fix to be considered valid */
};

/*! Gets closest route position (maneuver, segment and distance) to given location

@param navigation A Navigation object created with NB_NavigationCreate()
@param location The location to find the route position for
@param maneuver If non-null, receives the maneuver closest to the location
@param segment If non-null, receives the segment closest to the location
@param distance If non-null, receives the distance along the route closest to the location
@return NB_Error
*/
NB_Error NB_RouteDistanceToClosestPoint(NB_NavigationState* navigationState, NB_LatitudeLongitude* location, uint32* maneuver, uint32* segment, double* distance);

/*! Finds next congestion region

@param navigation A Navigation object created with NB_NavigationCreate()
@return NB_Error
*/
NB_Error NB_NavigationFindNextCongestion(NB_NavigationState* navigationState, NB_TrafficInformation* trafficInfo, data_traffic_region* pdtr_start, data_traffic_region* pdtr_end, NB_NavigateTrafficCongestionType* ptc, double* prouteremaindist, double* plength, double* pspeed, uint32* pnman, boolean* pincongestion);

/*! Finds next congestion region

@param navigation A Navigation object created with NB_NavigationCreate()
@return NB_Error
*/
NB_Error NB_NavigationSetNavigationManeuverPosition(NB_NavigationState* navigationState, NB_NavigateManeuverPos maneuverPos);

/*! Gets length of route being navigated

@param navigation A Navigation object created with NB_NavigationCreate()
@param length The length of the route
@return NB_Error
*/
NB_Error NB_NavigationGetRouteLength(NB_NavigationState* navigationState, double* length);

/*! Notifies navigation state that message has been played

@param navigationState A NavigationState object
@param messagePlayed The message that is no longer being played
@return NB_Error
*/
void NB_NavigationMessagePlayed(NB_NavigationState* navigationState, NB_GuidanceMessage* messagePlayed);

NB_DEC NB_Error NB_NavigationStateGetInstructionParameters(NB_NavigationState* navigationState, double* instructionRemainDistance, double* instructionReferenceSpeed, enum manuever_type* instructionManeuverType, double* maximumInstructionDistance, double* totalManeuverDistance);

/*! Gets closest route position (manuever, segment and distance) to given location

@param navigationState  A NavigationState object
@param pAnncDist If non-null, receives announcement distance
@param pContinueDist If non-null, receives continue distance
@param pPrepDist If non-null, receives prepare message distance
@param pPrepMinDist If non-null, receives prepare message minimum distance
@param pInstDist If non-null, receives instruction message distance
@param pLaneDist If non-null, receives lane guidance message distance
@return NB_Error
 */
NB_Error NB_NavigationStateGetInstructionDistances(NB_NavigationState* navigationState,
        double* pAnncDist, double* pContinueDist, double* pLaneDist, double* pPlayPrepDist, double* pPlayPrepMinDist, double* pPlayInstDist);

#endif //NBNAVIGATIONSTATEPRIVATE_H_
