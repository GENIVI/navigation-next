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
   @file        NKUIPresenterManager.cpp
   @defgroup    nkui
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "PresenterManager.h"
#include "AnnouncementPresenter.h"
#include "ArrivalFooterPresenter.h"
#include "ArrivalHeaderPresenter.h"
#include "BackgroundPresenter.h"
#include "CurrentRoadPresenter.h"
#include "DetourFooterPresenter.h"
#include "DetourHeaderPresenter.h"
#include "EnhancedNavStartupNotifyPresenter.h"
#include "EnhancedStartHeaderPresenter.h"
#include "LaneGuidancePresenter.h"
#include "MapPrefetchPresenter.h"
#include "MapPresenter.h"
#include "MiniMapPresenter.h"
#include "NavigationFooterPresenter.h"
#include "NavigationHeaderPresenter.h"
#include "NavigationListPresenter.h"
#include "NoGPSPresenter.h"
#include "OverflowMenuPresenter.h"
#include "PedestrianListPresenter.h"
#include "RecalcOnCallTextPresenter.h"
#include "RetryPresenter.h"
#include "RouteRequestFooterPresenter.h"
#include "RouteSelectBubblePresenter.h"
#include "RouteSelectHeaderPresenter.h"
#include "RouteSelectListPresenter.h"
#include "RouteselectFooterPresenter.h"
#include "RouteSelectStatusBarPresenter.h"
#include "SARPresenter.h"
#include "SpeedLimitPresenter.h"
#include "StackManeuverPresenter.h"
#include "StartingNavFooterPresenter.h"
#include "StatusBarPresenter.h"
#include "TripOverviewHeaderPresenter.h"
#include "StopNavigationPresenter.h"
#include "PinBubblePresenter.h"
#include "EndTripStatusBarPresenter.h"
#include "nkui_macros.h"
#include "TrafficAheadPresenter.h"
#include "OverviewFooterPresenter.h"

using namespace nkui;



/* See description in header file. */
NKUIPresenterManager::NKUIPresenterManager(NKUIStringHelperPtr pStringHelper)
    : m_pStringHelper(pStringHelper)
{
}

/* See description in header file. */
NKUIPresenterManager::~NKUIPresenterManager()
{
}

/* See description in header file. */
NKUIPresenterPtr NKUIPresenterManager::GetPresenter(WidgetID widgetID)
{
    PresenterBase* presenter = NULL;
    switch (widgetID)
    {
        case WGT_REQUEST_FOOTER:
        {
            presenter = NKUI_NEW RouteRequestFooterPresenter;
            break;
        }
        case WGT_ROUTE_SELECT_HEADER:
        {
            presenter = NKUI_NEW RouteSelectHeaderPresenter;
            break;
        }
        case WGT_ROUTE_DETALLS_LIST:
        {
            presenter = NKUI_NEW RouteSelectListPresenter;
            break;
        }
        case WGT_ROUTE_SELECT_FOOTER:
        {
            presenter = NKUI_NEW RouteselectFooterPresenter;
            break;
        }
        case WGT_NAV_FOOTER:
        {
            presenter = NKUI_NEW NavigationFooterPresenter;
            break;
        }
        case WGT_NAV_HEADER:
        {
            presenter = NKUI_NEW NavigationHeaderPresenter;
            break;
        }
        case WGT_OVERFLOW_MENU:
        {
            presenter = NKUI_NEW OverflowMenuPresenter;
            break;
        }
        case WGT_ANNOUNCER:
        {
            presenter = NKUI_NEW AnnouncementPresenter;
            break;
        }
        case WGT_STACKED_MANEUVER:
        {
            presenter = NKUI_NEW StackManeuverPresenter;
            break;
        }
        case WGT_SAR:
        {
            presenter = NKUI_NEW SARPresenter;
            break;
        }
        case WGT_LANE_GUIDANCE:
        {
            presenter = NKUI_NEW LaneGuidancePresenter(false);
            break;
        }
        case WGT_LANE_GUIDANCE_SAR:
        {
            presenter = NKUI_NEW LaneGuidancePresenter(true);
            break;
        }
        case WGT_CURRENTROAD:
        {
            presenter = NKUI_NEW CurrentRoadPresenter;
            break;
        }
        case WGT_SPEED_LIMITS:
        {
            presenter = NKUI_NEW SpeedLimitPresenter;
            break;
        }
        case WGT_MINI_MAP_NAV:
        case WGT_MINI_MAP_LOCATEME:
        case WGT_MINI_MAP_RTS:
        {
            presenter = NKUI_NEW MiniMapPresenter;
            break;
        }
        case WGT_TRIP_OVERVIEW_HEADER:
        {
            presenter = NKUI_NEW TripOverviewHeaderPresenter;
            break;
        }
        case WGT_NAVIGATION_LIST:
        {
            presenter = NKUI_NEW NavigationListPresenter;
            break;
        }
        case WGT_MAP:
        {
            presenter = NKUI_NEW MapPresenter();
            break;
        }
        case WGT_ARRIVAL_FOOTER:
        {
            presenter = NKUI_NEW ArrivalFooterPresenter();
            break;
        }
        case WGT_ARRIVAL_HEADER:
        {
            presenter = NKUI_NEW ArrivalHeaderPresenter();
            break;
        }
        case WGT_RETRY:
        {
            presenter = NKUI_NEW RetryPresenter();
            break;
        }
        case WGT_DETOUR_FOOTER:
        {
            presenter = NKUI_NEW DetourFooterPresenter();
            break;
        }
        case WGT_DETOUR_HEADER:
        {
            presenter = NKUI_NEW DetourHeaderPresenter();
            break;
        }
        case WGT_ROUTE_SELECT_BUBBLE:
        {
            presenter = NKUI_NEW RouteSelectBubblePresenter();
            break;
        }
        case WGT_NOGPS:
        {
            presenter = NKUI_NEW NoGPSPresenter();
            break;
        }
        case WGT_PEDESTRIAN_LIST:
        {
            presenter = NKUI_NEW PedestrianListPresenter();
            break;
        }
        case WGT_RECALCULATE:
        {
            presenter = NKUI_NEW RecalcOnCallTextPresenter();
            break;
        }
        case WGT_ENHANCE_NAV_NOTIFICATION:
        {
            presenter = NKUI_NEW EnhancedNavStartupNotifyPresenter();
            break;
        }
        case WGT_STARTINGNAV_FOOTER:
        {
            presenter = NKUI_NEW StartingNavFooterPresenter();
            break;
        }
        case WGT_ENHANCE_HEADER:
        {
            presenter = NKUI_NEW EnhancedStartHeaderPresenter();
            break;
        }
        case WGT_STATUS_BAR:
        {
            presenter = NKUI_NEW StatusBarPresenter();
            break;
        }
        case WGT_BACKGROUND:
        {
            presenter = NKUI_NEW BackgroundPresenter();
            break;
        }
        case WGT_MAP_PREFETCH:
        {
            presenter = NKUI_NEW MapPrefetchPresenter();
            break;
        }
        case WGT_ROUTE_SELECT_STATUS_BAR:
        {
            presenter = NKUI_NEW RouteSelectStatusBarPresenter();
            break;
        }
        case WGT_STOP_NAVIGATION:
        {
            presenter = NKUI_NEW StopNavigationPresenter();
            break;
        }
        case WGT_PIN_BUBBLE:
        {
            presenter = NKUI_NEW PinBubblePresenter();
            break;
        }
        case WGT_ENDTRIP_BAR:
        {
            presenter = NKUI_NEW EndTripStatusBarPresenter();
            break;
        }
        case WGT_TRAFFIC_AHEAD:
        {
            presenter = NKUI_NEW TrafficAheadPresenter();
            break;
        }
        case WGT_OVERVIEW_FOOTER:
        {
            presenter = NKUI_NEW OverviewFooterPresenter();
            break;
        }
        default:
            break;
    }
    if(presenter)
    {
        presenter->SetStringHelper(m_pStringHelper);
    }
    return shared_ptr<NKUIPresenter>(presenter);
}


/*! @} */
