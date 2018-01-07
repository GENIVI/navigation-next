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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * navpublictypes.h: created 2005/01/12 by Mark Goddard.
 */

#ifndef NAVPUBLICTYPES_H
#define NAVPUBLICTYPES_H

#include "nberror.h"
#include "nblocation.h"
#include "nbplace.h"
#include "nbroutesettings.h"

// Public constants ..........................................................

#define NAV_MANEUVER_NONE               0xFFFFFFFF
#define NAV_MANEUVER_CURRENT            0xFFFFFFFE
#define NAV_MANEUVER_START              0xFFFFFFFD

/* Simulated Speed camera maneuver type. For internal use only. */
#define NAV_MANEUVER_CAMERA             0xFFFFFFFC

/* Simulated Speeding maneuver type. For internal use only. */
#define NAV_MANEUVER_CAMERA_SPEEDING    0xFFFFFFFB

/* Simulated first maneuver type, is internal used only. */
#define NAV_MANEUVER_INITIAL            0xFFFFFFFA

/* Simulated enhanced startup type, For our kits use only. */
#define NAV_MANEUVER_ENHANCED_STARTUP   0xFFFFFFF9

// Public types ..............................................................

typedef struct
{
    int16 x;
    int16 y;

} NB_Point;

typedef struct
{
    int16 x;
    int16 y;
    int16 dx;
    int16 dy;

} NB_Rectangle;

typedef enum
{
    NB_NMP_Invalid = 0,

    NB_NMP_Current,
    NB_NMP_Continue,
    NB_NMP_ShowTurn,
    NB_NMP_Prepare,
    NB_NMP_Turn,
    NB_NMP_Past,
    NB_NMP_Recalculate,
    NB_NMP_SoftRecalculate,
    NB_NMP_RecalculateConfirm,
    NB_NMP_RecalculateTraffic,
    NB_NMP_TrafficCongestion,
    NB_NMP_TrafficIncident,
    NB_NMP_TrafficAlert,
    NB_NMP_TrafficDelay,
    NB_NMP_Calc,
    NB_NMP_ConfirmDetour,
    NB_NMP_SpeedLimitZone,
    NB_NMP_SchoolZone,
    NB_NMP_RouteUpdated,
    NB_NMP_SwitchToTBT,
    NB_NMP_SwitchToStaticMode

} NB_NavigateManeuverPos;

typedef enum
{
    NB_NS_Invalid = 0,

    NB_NS_Created,              /*!< The navigation session has been created, but not started  */
    NB_NS_InitialRoute,         /*!< The navigation session has been started and the initial route is being downloaded */
    NB_NS_UpdatingRoute,        /*!< The navigation session is updating the route because of an off-route condition or manual update */
    NB_NS_NavigatingStartup,    /*!< The user is navigating, but has not yet begun to follow the route */
    NB_NS_Navigating,           /*!< The user is navigating and is on the route */
    NB_NS_NavigatingFerry,      /*!< The user is navigating and is currently on a portion of the route involving a ferry */
    NB_NS_NavigatingArriving,   /*!< The user is navigating and has reached the vicinity of their destination */
    NB_NS_Arrived,              /*!< The user has arrived and their destination */
    NB_NS_Error,                /*!< An error condition has occurred in the navigation session */
    NB_NS_NavigatingConfirmRecalc  /*!< User is, and has been, off route and needs to confirm this recalc.  Will automatically switch back to NB_NS_Navigating if user starts following route again */

} NB_NavigateStatus;

typedef enum
{
    NB_NRE_None = 0,

    NB_NRE_TimedOut,
    NB_NRE_BadDestination,
    NB_NRE_BadOrigin,
    NB_NRE_CannotRoute,
    NB_NRE_EmptyRoute,
    NB_NRE_NetError,
    NB_NRE_UnknownError,
    NB_NRE_NoMatch,
    NB_NRE_ServerError,
    NB_NRE_NoDetour,
    NB_NRE_PedRouteTooLong,
    NB_NRE_OriginCountryUnsupported,
    NB_NRE_DestinationCountryUnsupported,
    NB_NRE_BicycleRouteTooLong,
    NB_NRE_RouteTooLong
} NB_NavigateRouteError;

typedef enum
{
    NB_NTT_Invalid = 0,

    NB_NTT_Primary,
    NB_NTT_Secondary,
    NB_NTT_Lookahead

} NB_RouteTextType;

typedef enum
{
    NB_NTC_Unknown = 0,

    NB_NTC_None,
    NB_NTC_Moderate,
    NB_NTC_Severe

} NB_NavigateTrafficCongestionType;

typedef enum
{
    NB_NSV_Invalid = 0,

    NB_NSV_LC,
    NB_NSV_Sliding2d,
    NB_NSV_Sliding3d

} NB_NavigateStatusViewType;

typedef enum
{
    NB_NTM_Invalid = 0,

    NB_NTM_Vehicle,
    NB_NTM_Pedestrian

} NB_NavigateTransportMode;

typedef enum
{
    NB_NAT_None = 0,
    NB_NAT_Tone,
    NB_NAT_Voice,
    NB_NAT_Street

} NB_NavigateAnnouncementType;

typedef enum
{

    NB_NAS_Automatic,
    NB_NAS_Recalculation,
    NB_NAS_Button,
    NB_NAS_Lookahead,
    NB_NAS_TrafficButton

} NB_NavigateAnnouncementSource;

typedef enum
{
    NB_NAU_Miles = 100,
    NB_NAU_MilesYards,
    NB_NAU_Kilometers,
} NB_NavigateAnnouncementUnits;

typedef enum
{
    NB_RCT_None = 0,
    NB_RCT_Initial,
    NB_RCT_Traffic,
    NB_RCT_Recalc,
    NB_RCT_Reload,
    NB_RCT_ConfirmRecalc,
    NB_RCT_RouteUpdated

} NB_NavigateRouteCalculationType;

/// @todo Move this someplace more appropriate
typedef struct
{
    double      latitude;
    double      longitude;
} NB_LatitudeLongitude;

typedef enum
{
    NB_NM_Automatic = 0,
    NB_NM_StaticRoute

} NB_NavigationMode;

typedef enum
{
    NB_NRRR_None = 0,
    NB_NRRR_Recalculation,
    NB_NRRR_InitialRoute,
    NB_NRRR_RouteSelector,
    NB_NRRR_Detour
} NB_NavigateRouteRequestReason;

#endif
