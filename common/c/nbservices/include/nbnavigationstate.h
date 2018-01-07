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

    @file     nbnavigationstate.h

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

#ifndef NBNAVIGATIONSTATE_H_
#define NBNAVIGATIONSTATE_H_

#include "navpublictypes.h"
#include "nbcontext.h"

/*!
    @addtogroup nbnavigationstate
    @{
*/

typedef struct NB_NavigationState NB_NavigationState;

//@todo (BUG 55948) Review and remove
/*! @struct NB_PositionRoutePositionInformation TODO REVIEW AND REOMOVE
Information about the route position
*/
typedef struct NB_PositionRoutePositionInformation
{
    uint32					time;               /*!< Current time */
    uint32					closestSegment;     /*!< Closest segment */
    uint32					closestManeuver;    /*!< Closest maneuver */

    double					segmentRemain;      /*!< Distance remaining in segment */
    double					segmentDistance;    /*!< Total distance in segment */
    double					segmentHeading;     /*!< Current heading of segment */
    double					projLat;            /*!< Current latitude */
    double					projLon;            /*!< Current longitude */

    double					estSpeedAlongRoute; /*!< Estimated speed along route */
    nb_boolean				wrongWay;           /*!< Wrong way flag */
    nb_boolean				validHeading;       /*!< Valid heading flag */

    double					maneuverDistanceRemaining;  /*!< Maneuver distance remaining */
    double					maneuverBaseSpeed;          /*!< Maneuver base speed */

    double					tripDistanceAfter;          /*!< Trip distance after maneuver */
    uint32					tripTimeAfter;              /*!< Trip time after maneuver */

    uint32                  remainingRouteTime;         /*!< Remaining route time */
    double                  remainingRouteDist;         /*!< Remaining route distance */

    double                  averageSpeed;               /*!< Average speed */

    NB_NavigateManeuverPos  maneuverPosition;           /*!< Current maneuver position */

} NB_PositionRoutePositionInformation;

//@todo (BUG 56031)
#define NB_NAVIGATION_STREET_NAME_MAX_LEN 256

typedef struct NB_NavigationPublicState
{    
    NB_PositionRoutePositionInformation currentRoutePosition;    
    NB_NavigateStatus status;

    char	currentStreet[NB_NAVIGATION_STREET_NAME_MAX_LEN];
    char	currentSecondary[NB_NAVIGATION_STREET_NAME_MAX_LEN];
    char	destinationStreet[NB_NAVIGATION_STREET_NAME_MAX_LEN];
    char	destinationSecondary[NB_NAVIGATION_STREET_NAME_MAX_LEN];

    NB_NavigateManeuverPos	pos;

    double	turnLatitude;
    double	turnLongitude;
    double	turnRemainDistance;
    uint32	turnRemainTime;

    double	tripRemainDistance;
    uint32	tripRemainTime;
 
    uint32	currentManeuver;

    uint32	stackNman;
    double	stackDistance;

    double  viewHeading;
    double	heading;
    double	speed;
    double	latitude;
    double	longitude;
    double  distToOrigin;
    
    nb_boolean onRoute;    
    double routeLength;

    nb_boolean snapMap;

    NB_NavigateTransportMode    transportMode;

    NB_NavigateAnnouncementType announceType;
        
} NB_NavigationPublicState;

NB_DEC NB_Error NB_NavigationStateGetRoutePositionInformation(NB_NavigationState* navigationState, NB_PositionRoutePositionInformation* information);


/* @} */


#endif
