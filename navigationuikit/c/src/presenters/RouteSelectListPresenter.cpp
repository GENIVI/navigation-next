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
    @file         RouteSelectListPresenter.cpp
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#include "RouteSelectListPresenter.h"
#include "FormatController.h"
#include "palclock.h"
#include "RouteSelectInformation.h"


namespace nkui
{

static const double INVALID_DISTANCE = -1.0;

RouteSelectListPresenter::RouteSelectListPresenter()
    : m_pListWedget(NULL)
    , m_currentManeuverDistance(INVALID_DISTANCE)
{
}

RouteSelectListPresenter::~RouteSelectListPresenter()
{
}

void
RouteSelectListPresenter::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_NEW_ROUTE_SELECTED)
    {
        int* routeIndex = (int*)data;
        if(m_pNavUIContex)
        {
            m_pNavUIContex->SetChosenRouteIndex(*routeIndex);
            if (m_pNavUIContex->IsChosenCurrentRoute() && m_pNavUIContex->IsSelectingDetour())
            {
                ShowCurrentRoute();
            }
            else
            {
                UpdateRoute(m_pNavUIContex->GetChosenRoute());
                m_prtsHeader->RouteSelected(m_pNavUIContex->GetChosenRouteIndex());
            }
        }
    }
    PresenterBase::HandleEvent(id, data);
}

void
RouteSelectListPresenter::OnActive()
{
    PresenterBase::OnActive();

    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
    m_pNKUIController->RegisterEvent(EVT_NEW_ROUTE_SELECTED, this);
    m_pNKUIController->RegisterEvent(EVT_ROUTE_UPDATE, this);
    if (m_pNavUIContex->IsChosenCurrentRoute() && m_pNavUIContex->IsSelectingDetour())
    {
        ShowCurrentRoute();
    }
    else
    {
        RouteSelectInformation rtsInfo = RouteSelectInformation(m_pStringHelper,m_pNavUIContex);
        m_prtsHeader->SetRouteInfomation(rtsInfo.GetRouteInfoData(), m_pNavUIContex->GetChosenRouteIndex());
        UpdateRoute(m_pNavUIContex->GetChosenRoute());
    }
}

void
RouteSelectListPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveSessionListener(this);
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    PresenterBase::OnDeactivate();

    NKUIEventData eventData;
    m_pNKUIController->SendEvent(EVT_LIST_CLOSED, eventData);
}

void
RouteSelectListPresenter::OnSetWidget()
{
    m_pListWedget = dynamic_cast<RouteSelectListWidget*>(m_pWidget);
    m_prtsHeader = dynamic_cast<RouteSelectHeaderWidget*>(m_pWidget);
}

void
RouteSelectListPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if(event == EVT_NEW_ROUTE_SELECTED)
    {
        m_prtsHeader->RouteSelected(m_pNavUIContex->GetChosenRouteIndex());
        if (m_pNavUIContex->IsChosenCurrentRoute() && m_pNavUIContex->IsSelectingDetour())
        {
            ShowCurrentRoute();
        }
        else
        {
            UpdateRoute(m_pNavUIContex->GetChosenRoute());
        }
    }
    else if (event == EVT_ROUTE_UPDATE)
    {
        const vector<nbnav::RouteInformation>& routes = m_pNavUIContex->GetAllRoutes();
        if (!routes.empty())
        {
            UpdateRoute(routes[0]);
            if(m_prtsHeader)
            {
                RouteSelectInformation rtsInfo = RouteSelectInformation(m_pStringHelper,m_pNavUIContex);
                m_prtsHeader->SetRouteInfomation(rtsInfo.GetRouteInfoData(), m_pNavUIContex->GetChosenRouteIndex());
            }
        }
    }
}

void
RouteSelectListPresenter::UpdateRoute(const nbnav::RouteInformation& route)
{
    // renew maneuvers
    std::vector<ManeuverListData> maneuvers;
    nbnav::ManeuverList maneuverList = route.GetManeuverList();
    int maneuverCount = maneuverList.GetNumberOfManeuvers();
    FormatController formater(m_pStringHelper);
    for (int i = 0; i < maneuverCount; ++i)
    {
        const nbnav::Maneuver* pManeuver = maneuverList.GetManeuver(i);
        ManeuverListData maneuverListData;
        maneuverListData.maneuverID = pManeuver->GetManeuverID();
        maneuverListData.primary = pManeuver->GetPrimaryStreet();
        maneuverListData.secondary = pManeuver->GetSecondaryStreet();
        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), pManeuver->GetDistance(), maneuverListData.distanceText, maneuverListData.distanceUnit);
        std::string turnCharacter = pManeuver->GetRoutingTTF();
        std::string exitnumber = pManeuver->GetExitNumber();
        if(!exitnumber.empty())
        {
            maneuverListData.exitText = m_pStringHelper->GetLocalizedString("IDS_EXIT") + " " + exitnumber;
        }
        maneuverListData.nextTurnCharacter = turnCharacter[0];

        if(pManeuver->IsDestination())
        {
            maneuverListData.primary = m_pNavUIContex->GetDestinationStreetSideText(m_pStringHelper);
            maneuverListData.secondary = m_pNavUIContex->GetDestinationStreetText();
        }
        maneuvers.push_back(maneuverListData);
    }
    m_pListWedget->UpdateManeuvers(maneuvers);

    // update traffic events
    std::vector<TrafficListData> trafficEvents;
    int trafficEventCount = route.GetNumberOfTrafficEvent();
    for (int i = 0; i < trafficEventCount; ++i)
    {
        const nbnav::TrafficEvent* pTrafficEvent = route.GetTrafficEvent(i);
        TrafficListData trafficListData;
        trafficListData.type = pTrafficEvent->GetType();
        trafficListData.severity = pTrafficEvent->GetSeverity();
        trafficListData.description = pTrafficEvent->GetDescription();
        trafficListData.delayTime = pTrafficEvent->GetDelay();
        formater.FormatTime((uint32)pTrafficEvent->GetDelay(), trafficListData.delayText,
                            trafficListData.delayUnit, trafficListData.delayText2,
                            trafficListData.delayUnit2);
        trafficListData.maneuverID = pTrafficEvent->GetManeuverNumber();
        trafficEvents.push_back(trafficListData);
    }

    m_pListWedget->UpdateTrafficEvents(trafficEvents);

    const nbnav::Maneuver* pManeuver = maneuverList.GetManeuver(0);

    string title = pManeuver->GetPrimaryStreet();
    if (pManeuver->IsDestination() && title.empty())
    {
        title = m_pNavUIContex->GetDestinationStreetText();
        if (title.empty())
        {
            title = m_pNavUIContex->GetDestination().GetName();
        }
    }

    if(m_pNavUIContex->IsStartupOffroute() || m_pNavUIContex->IsEnhancedNavigationStartup())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), title);
    }
    else
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_STARTING_ON"), title);
    }
}

void
RouteSelectListPresenter::ManeuverRemainingDistance(double distance)
{
    if(!m_pNavUIContex->IsStartupOffroute())
    {
        m_currentManeuverDistance = distance;
        if (m_pNavUIContex->IsChosenCurrentRoute())
        {
            std::string distanceString;
            std::string distanceUnitString;
            FormatController formater(m_pStringHelper);
            formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, distanceString, distanceUnitString);
            m_pListWedget->UpdateDistance(distanceString.c_str(), distanceUnitString.c_str());
        }
    }
}

void
RouteSelectListPresenter::UpdateManeuverList(const nbnav::ManeuverList& maneuverList)
{
    m_currentManeuvers.clear();
    int maneuverCount = maneuverList.GetNumberOfManeuvers();
    FormatController formater(m_pStringHelper);
    for (int i = 0; i < maneuverCount; ++i)
    {
        const nbnav::Maneuver* pManeuver = maneuverList.GetManeuver(i);
        ManeuverListData maneuverListData;
        maneuverListData.maneuverID = pManeuver->GetManeuverID();
        maneuverListData.primary = pManeuver->GetPrimaryStreet();
        maneuverListData.secondary = pManeuver->GetSecondaryStreet();
        double distance = pManeuver->GetDistance();
        if (i == 0 && m_currentManeuverDistance != INVALID_DISTANCE)
        {
            // the distanc of current maneuver has been updated before
            distance = m_currentManeuverDistance;
        }
        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, maneuverListData.distanceText, maneuverListData.distanceUnit);
        std::string turnCharacter = pManeuver->GetRoutingTTF();
        //maneuverListData.exitText = m_pStringHelper->GetLocalizedString("IDS_EXIT");
        maneuverListData.nextTurnCharacter = turnCharacter[0];
        if(pManeuver->IsDestination())
        {
            maneuverListData.primary = m_pNavUIContex->GetDestinationStreetSideText(m_pStringHelper);
            maneuverListData.secondary = m_pNavUIContex->GetDestinationStreetText();
            maneuverListData.isDestination = true;
            maneuverListData.destAddress = m_pNavUIContex->GetDestinationAddress();
        }
        m_currentManeuvers.push_back(maneuverListData);
    }
    if (m_pNavUIContex->IsChosenCurrentRoute())
    {
        m_pListWedget->UpdateManeuvers(m_currentManeuvers);
        m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
    }
}

void
RouteSelectListPresenter::NextRoadName(std::string primaryName, std::string secondaryName)
{
    m_nextRoadName = primaryName;
    if(m_pNavUIContex->IsStartupOffroute() && m_pNavUIContex->IsChosenCurrentRoute())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), m_nextRoadName);
    }
}

void
RouteSelectListPresenter::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    m_currentRoadName = primaryName;
    if(!m_pNavUIContex->IsStartupOffroute() && !m_pNavUIContex->IsEnhancedNavigationStartup() && m_pNavUIContex->IsChosenCurrentRoute())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_DRIVING_ON"), m_currentRoadName);
    }
}

void
RouteSelectListPresenter::TrafficChanged(const nbnav::TrafficInformation& trafficInfo)
{
    m_trafficEvents.clear();
    FormatController formater(m_pStringHelper);
    std::vector<nbnav::TrafficEvent> nbTrafficEvents = trafficInfo.GetTrafficEvents();
    std::vector<nbnav::TrafficEvent>::iterator iter = nbTrafficEvents.begin();
    while (iter != nbTrafficEvents.end())
    {
        const nbnav::TrafficEvent& trafficEvent = *iter;
        TrafficListData trafficListData;
        trafficListData.type = trafficEvent.GetType();
        trafficListData.severity = trafficEvent.GetSeverity();
        trafficListData.description = trafficEvent.GetDescription();
        trafficListData.delayTime = trafficEvent.GetDelay();
        formater.FormatTime((uint32)trafficEvent.GetDelay(), trafficListData.delayText, trafficListData.delayUnit, trafficListData.delayText2, trafficListData.delayUnit2);
        trafficListData.maneuverID = trafficEvent.GetManeuverNumber();
        m_trafficEvents.push_back(trafficListData);
        iter++;
    }
    if (m_pNavUIContex->IsChosenCurrentRoute())
    {
        m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
    }
}

void
RouteSelectListPresenter::ShowCurrentRoute()
{
    std::string distanceString;
    std::string distanceUnitString;
    FormatController formater(m_pStringHelper);
    formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), m_currentManeuverDistance, distanceString, distanceUnitString);
    m_pListWedget->UpdateDistance(distanceString.c_str(), distanceUnitString.c_str());
    if(m_pNavUIContex->IsStartupOffroute() || m_pNavUIContex->IsEnhancedNavigationStartup())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), m_currentRoadName);
    }
    else
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_DRIVING_ON"), m_currentRoadName);
    }
    m_pListWedget->UpdateManeuvers(m_currentManeuvers);
    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
    RouteSelectInformation rtsInfo = RouteSelectInformation(m_pStringHelper,m_pNavUIContex);
    m_prtsHeader->SetRouteInfomation(rtsInfo.GetRouteInfoData(), m_pNavUIContex->GetChosenRouteIndex());
}

}
/*! @} */
