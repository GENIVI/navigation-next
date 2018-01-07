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
 @file         NKUIContext.cpp
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include "NKUIContext.h"
#include "palclock.h"
#include "nkui_macros.h"

#define MAX_RETRY_COUNT     3

using namespace std;
using namespace nbnav;
using namespace nkui;

NKUIContext* NKUIContext::GetInstance()
{
    return new NKUIContext();
}

void NKUIContext::Release()
{
    delete this;
}

NKUIContext::NKUIContext()
    : m_navigation(NULL),
      m_currentRouteIndex(-1),
      m_chosenRouteIndex(-1),
      m_retryCount(MAX_RETRY_COUNT),
      m_processingRetry(false),
      m_onRoute(true),
      m_recalcingRoute(false),
      m_gettingDetour(false),
      m_isSelectingDetour(false),
      m_currentLocation(0,0,0,0,0,0,false,0),
      m_isStartoffRoute(false),
      m_enableTurnByTurnNav(true),
      m_enhancedNavigationStartup(false),
      m_currentState(NPS_Invalid),
      m_roadSignShown(false),
      m_tryDetour(false)
{
    m_mapSetting.avatarMode        = NAM_Remain;
    m_mapSetting.zoomType          = NZTAT_None;
    m_mapSetting.polylineType      = NNM_RTS;
    m_mapSetting.cameraMode        = NCM_Remain;
    m_mapSetting.trackingMode      = NCTM_Remain;
    m_mapSetting.useFixedGPS       = FALSE;
    m_mapSetting.showManeuverArrow = FALSE;
}

Navigation* NKUIContext::GetNavigation()
{
    return m_navigation;
}

void NKUIContext::SetNavigation(Navigation* navigation)
{
    m_navigation = navigation;
}

void NKUIContext::SetStartupOffroute(bool startOffroute)
{
    m_isStartoffRoute = startOffroute;
}

bool NKUIContext::IsStartupOffroute() const
{
    return m_isStartoffRoute;
}

void NKUIContext::SetCurrentLocation(const NKUILocation& currentLocation)
{
    m_currentLocation = currentLocation;
}

NKUILocation& NKUIContext::GetCurrentLocation()
{
    return m_currentLocation;
}

const vector<RouteInformation>& NKUIContext::GetAllRoutes() const
{
    return m_routes;
}

void NKUIContext::SetAllRoutes(const vector<RouteInformation>& routes)
{
    m_routes = routes;
    m_currentRouteIndex = 0;
    m_isSelectingDetour = false;
}

const RouteInformation& NKUIContext::GetChosenRoute() const
{
    if (m_chosenRouteIndex >= 0 && m_chosenRouteIndex < m_routes.size())
    {
        return m_routes[m_chosenRouteIndex];
    }
    return m_routes[0];
}

const RouteInformation& NKUIContext::GetCurrentRoute() const
{
    if (m_currentRouteIndex >= 0 && m_currentRouteIndex < m_routes.size())
    {
        return m_routes[m_currentRouteIndex];
    }
    return m_routes[0];
}

bool NKUIContext::IsChosenCurrentRoute() const
{
    return m_chosenRouteIndex == m_currentRouteIndex;
}

bool NKUIContext::IsRecalcingRoute() const
{
    return m_recalcingRoute;
}

bool NKUIContext::IsGettingDetour() const
{
    return m_gettingDetour;
}

bool NKUIContext::IsSelectingDetour() const
{
    return m_isSelectingDetour;
}

bool NKUIContext::IsTryDoDetour() const
{
    return m_tryDetour;
}

void NKUIContext::SetRecalcingRoute(bool recalcingRoute)
{
    m_recalcingRoute = recalcingRoute;
}

void NKUIContext::SetGettingDetour(bool gettingDetour)
{
    m_gettingDetour = gettingDetour;
    if(!gettingDetour)
    {
        m_tryDetour = false;
    }
}

void NKUIContext::SetTryDoDetour(bool tryDetour)
{
    m_tryDetour = tryDetour;
}

void NKUIContext::SetChosenRouteIndex(int index)
{
    m_chosenRouteIndex = index;
}

int NKUIContext::GetChosenRouteIndex() const
{
    return m_chosenRouteIndex;
}

int NKUIContext::GetCurrentRouteIndex() const
{
    return m_currentRouteIndex;
}

const Place& NKUIContext::GetDestination() const
{
    return m_destination;
}

void NKUIContext::SetDestination(const Place& place, NKUIStringHelperPtr stringHelper)
{
    m_destination = place;

    // Update name of destination if it is empty. This name will be used in multiple places.
    if (m_destination.GetName().empty() && stringHelper)
    {
        m_destination.SetName(stringHelper->GetLocalizedString("IDS_DESTINATION"));
    }
}

const Place& NKUIContext::GetOrigin() const
{
    return m_origin;
}

void NKUIContext::SetOrigin(const Place& place)
{
    m_origin = place;
}

void NKUIContext::ClearUnselectedRoutes()
{
    nbnav::RouteInformation currentRoad = GetChosenRoute();
    m_routes.clear();
    m_routes.push_back(currentRoad);
    m_currentRouteIndex = 0;
    m_chosenRouteIndex = 0;
}

void NKUIContext::SetEnhancedNavigationStartup(bool value)
{
    m_enhancedNavigationStartup = value;
}

bool NKUIContext::IsEnhancedNavigationStartup() const
{
    return m_enhancedNavigationStartup;
}

bool NKUIContext::ClearDetourRoute()
{
    bool ret = false;
    if(m_routes.size() > 1)
    {
        nbnav::RouteInformation currentRoute = m_routes[m_currentRouteIndex];
        m_routes.clear();
        m_routes.push_back(currentRoute);
        m_chosenRouteIndex  = 0;
        m_currentRouteIndex = 0;
        ret = true;
    }
    m_isSelectingDetour = false;
    return ret;
}

void NKUIContext::SetPalInstance(PAL_Instance* pal)
{
    m_pal = pal;
}

void NKUIContext::SetMapSetting(const MapSetting& mapSetting)
{
    m_mapSetting = mapSetting;
}

const MapSetting& NKUIContext::GetMapSetting() const
{
    return m_mapSetting;
}

void NKUIContext::SetCurrentRoadName(const std::string& roadName)
{
    m_navSharedInformation.m_currentRoadName = roadName;
}

void NKUIContext::SetTripRemainingTime(int time)
{
    m_navSharedInformation.m_tripRemainingTime = time;
}

void NKUIContext::SetInTrafficArea(bool inTrafficArea)
{
    m_navSharedInformation.m_inTrafficArea = inTrafficArea;
}

const NKUISharedInformation& NKUIContext::GetNavSharedInformation() const
{
    return m_navSharedInformation;
}

bool NKUIContext::RetryStart()
{
    if (m_processingRetry)
    {
        return true;
    }
    if (m_retryCount > 0)
    {
        m_processingRetry = true;
        --m_retryCount;
        return true;
    }
    return false;
}

void NKUIContext::RetryEnd()
{
    m_processingRetry = false;
}

void NKUIContext::ResetRetryCount()
{
    m_retryCount = MAX_RETRY_COUNT;
}

void NKUIContext::SetOnRoute(bool onRoute)
{
    m_onRoute = onRoute;
}

bool NKUIContext::IsOnRoute() const
{
    return m_onRoute;
}

void NKUIContext::UpdateCurrentRoute(const nbnav::RouteInformation& route)
{
    if(m_routes.size() > 0)
    {
        NKUI_ASSERT(m_currentRouteIndex >= 0);
        m_routes[m_currentRouteIndex] = route;
    }
    else
    {
        m_routes.push_back(route);
        m_currentRouteIndex = 0;
    }
}

void NKUIContext::UpdateDetourRoutes(const std::vector<nbnav::RouteInformation>& routes)
{
    m_routes = routes;
    NKUI_ASSERT(m_routes.size() > 0);
    //the last route is current route, route order is detour1, detour2,current
    m_currentRouteIndex = (int)m_routes.size() - 1;
    m_isSelectingDetour = true;
}

std::string NKUIContext::GetDestinationStreetSideText(NKUIStringHelperPtr& stringHelper)
{
    std::string name = GetDestination().GetName();
    size_t len = nsl_strlen(name.c_str());
    if (name.empty() || len == 0)
    {
        name = stringHelper->GetLocalizedString("IDS_DESTINATION");
    }
    nbnav::DestinationStreetSide side = GetChosenRoute().GetDestinationStreetSide();
    std::string tipText;
    if (side == nbnav::Center)
    {
        tipText = stringHelper->GetLocalizedString("IDS_IS_AHEAD");
    }
    else if (side == nbnav::Left )
    {
        tipText = stringHelper->GetLocalizedString("IDS_IS_ON_THE_LEFT");
    }
    else
    {
        tipText = stringHelper->GetLocalizedString("IDS_IS_ON_THE_RIGHT");
    }
    return name + " " + tipText;
}

std::string NKUIContext::GetDestinationStreetText()
{
    std::string street = GetDestination().GetLocation().street;
    //if the place's street send by app is empty, so use the destination return by route reply
    if(street.empty())
    {
        street = GetChosenRoute().GetDestination().GetLocation().street;
    }
    return street;
}

std::string NKUIContext::GetDestinationAddress()
{
    std::string address = GetDestination().GetLocation().freeform;
    //if the place's street send by app is empty, so use the destination return by route reply
    if(address.empty())
    {
        address = GetChosenRoute().GetDestination().GetLocation().freeform;
    }
    return address;
}

void NKUIContext::SetMapInterface(NKUIMapInterface* pMapInterface)
{
    m_pMapInterface = pMapInterface;
}

NKUIMapInterface* NKUIContext::GetMapInterface()
{
    return m_pMapInterface;
}

/* See description in header file. */
void NKUIContext::StartNavigationTrip()
{
    m_navSharedInformation.m_tripStartTime = PAL_ClockGetGPSTime();
    m_navSharedInformation.m_navigationSpendingTime = 0;
}

void NKUIContext::SetWidgetManager(NKUIWidgetManagerPtr manager)
{
    m_widgetManager = manager;
}

NKUIWidgetManagerPtr NKUIContext::GetWidgetManager()
{
    return m_widgetManager;
}

/*! @} */
