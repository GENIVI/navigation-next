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
    @file         NavigationListPresenter.cpp
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

#include "NavigationListPresenter.h"
#include "FormatController.h"
#include "palclock.h"

using namespace nkui;
using namespace nbnav;

static const double INVALID_DISTANCE = -1.0;

NavigationListPresenter::NavigationListPresenter()
    : m_pListWedget(NULL)
    , m_currentManeuverDistance(INVALID_DISTANCE)
    , m_tripDistance(0)
{
}

NavigationListPresenter::~NavigationListPresenter()
{
}

void
NavigationListPresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void
NavigationListPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
    m_pNKUIController->RegisterEvent(EVT_ENT_STARTOFFROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_ONROUTE, this);

    // In enhanced start up, UpdateManeuverList is not reliable
    // we need to initialize maneuver the original route information
    // but in normal navigation, we should not reload the whole route, because some of the maneuvers are already passed.
    if (m_pNavUIContex->IsEnhancedNavigationStartup())
    {
        UpdateManeuverList(m_pNavUIContex->GetChosenRoute().GetManeuverList());
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), m_pNavUIContex->GetChosenRoute().GetFirstMajorRoad());
    }
    m_pListWedget->SetDestinationText(m_pNavUIContex->GetDestinationStreetSideText(m_pStringHelper),
                                        m_pNavUIContex->GetDestinationStreetText());
    UpdateTrafficFromRouteInformation(m_pNavUIContex->GetChosenRoute());
    NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(), "Navigaiton List view displayed.\n");
}

void
NavigationListPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    PresenterBase::OnDeactivate();

    NKUIEventData eventData;
    m_pNKUIController->SendEvent(EVT_LIST_CLOSED, eventData);
}

void
NavigationListPresenter::OnSetWidget()
{
    m_pListWedget = dynamic_cast<NavigationListWidget*>(m_pWidget);
    m_pListWedget->SetListener(this);
}

void
NavigationListPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_ENT_STARTOFFROUTE:
        {
            m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), m_nextRoadName);
            break;
        }
        case EVT_NAV_ONROUTE:
        {
            m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_DRIVING_ON"), m_currentRoadName);
            break;
        }
        default:
            break;
    }
}

void
NavigationListPresenter::ManeuverRemainingDistance(double distance)
{
    if(!m_pNavUIContex->IsStartupOffroute())
    {
        m_currentManeuverDistance = distance;
        std::string distanceString;
        std::string distanceUnitString;
        FormatController formater(m_pStringHelper);
        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, distanceString, distanceUnitString);
        m_pListWedget->UpdateDistance(distanceString.c_str(), distanceUnitString.c_str());
    }
}
    
void
NavigationListPresenter::UpdateManeuverList(const nbnav::ManeuverList& maneuverList)
{
    std::vector<ManeuverListData> maneuvers;
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
        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance,
                                maneuverListData.distanceText,
                                maneuverListData.distanceUnit);
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
            maneuverListData.isDestination = true;
            maneuverListData.destAddress = m_pNavUIContex->GetDestinationAddress();
        }
        maneuvers.push_back(maneuverListData);
    }
    m_pListWedget->UpdateManeuvers(maneuvers);
    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
}

void
NavigationListPresenter::NextRoadName(std::string primaryName, std::string secondaryName)
{
    m_nextRoadName = primaryName;
    if(m_pNavUIContex->IsStartupOffroute())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"), m_nextRoadName);
    }
}

void
NavigationListPresenter::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    m_currentRoadName = primaryName;
    if(!m_pNavUIContex->IsStartupOffroute() && !m_pNavUIContex->IsEnhancedNavigationStartup())
    {
        m_pListWedget->UpdateTitle(m_pStringHelper->GetLocalizedString("IDS_DRIVING_ON"), m_currentRoadName);
    }
}

void
NavigationListPresenter::TrafficChanged(const nbnav::TrafficInformation& trafficInfo)
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
        formater.FormatTime(trafficListData.delayTime, trafficListData.delayText, trafficListData.delayUnit, trafficListData.delayText2, trafficListData.delayUnit2);
        trafficListData.maneuverID = trafficEvent.GetManeuverNumber();
        m_trafficEvents.push_back(trafficListData);

        iter++;
    }
    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
}

void
NavigationListPresenter::UpdateTrafficFromRouteInformation(const nbnav::RouteInformation& route)
{
    m_trafficEvents.clear();
    FormatController formater(m_pStringHelper);
    int trafficEventCount = route.GetNumberOfTrafficEvent();
    for (int i = 0; i < trafficEventCount; ++i)
    {
        const nbnav::TrafficEvent* pTrafficEvent = route.GetTrafficEvent(i);
        TrafficListData trafficListData;
        trafficListData.type = pTrafficEvent->GetType();
        trafficListData.severity = pTrafficEvent->GetSeverity();
        trafficListData.description = pTrafficEvent->GetDescription();
        trafficListData.delayTime = pTrafficEvent->GetDelay();
        formater.FormatTime(trafficListData.delayTime, trafficListData.delayText,
                            trafficListData.delayUnit, trafficListData.delayText2,
                            trafficListData.delayUnit2);
        trafficListData.maneuverID = pTrafficEvent->GetManeuverNumber();
        m_trafficEvents.push_back(trafficListData);
    }

    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
}

void
NavigationListPresenter::OnManeuverItemTapped(int maneuverID)
{
    NKUIEventData eventData;
    eventData.eventData = &maneuverID;
    m_pNKUIController->SendEvent(EVT_ANNOUNCE_MANEUVER, eventData);
}

void
NavigationListPresenter::TripRemainingTime(int time)
{
    static const double MileToMeter   = 1609.33;
    static const int    DistanceLimit = 500;
    FormatController formater(m_pStringHelper);
    std::string timeText;
    //if trip longer than 500mil then show duration
    if(m_tripDistance / MileToMeter > DistanceLimit)
    {
        formater.FormatTimeToHour(time, timeText);
    }
    else
    {
        uint32 currentTime = PAL_ClockGetUnixTime();
        int arrivalTime = currentTime + time;
        timeText = m_pStringHelper->GetLocalizedTimeString(arrivalTime);
    }
    m_pListWedget->UpdateArrivalTime(timeText);
}
void
NavigationListPresenter::TripRemainingDistance(double distance)
{
    m_tripDistance = distance;
    FormatController formater(m_pStringHelper);
    // Format Distance
    string distanceText;
    string distanceUnit;
    formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, distanceText, distanceUnit);
    string tripDistance = distanceText + " " + distanceUnit;
    m_pListWedget->UpdateTripDistance(tripDistance);
}
void
NavigationListPresenter::TripTrafficColor(char color)
{
    m_pListWedget->UpdateTraffic(color);
}

/*! @} */
