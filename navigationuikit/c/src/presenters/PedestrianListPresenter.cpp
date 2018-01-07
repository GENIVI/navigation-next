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
    @file         PedestrianListPresenter.cpp
    @defgroup     LNUNavigationUIKit
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

#include "PedestrianListPresenter.h"
#include "FormatController.h"
#include "palclock.h"

using namespace nkui;
using namespace nbnav;

#define INVALID_DISTANCE       -1.0


PedestrianListPresenter::PedestrianListPresenter()
    : m_pListWedget(NULL),
      m_currentManeuverDistance(INVALID_DISTANCE)
{
}

PedestrianListPresenter::~PedestrianListPresenter()
{
}

void
PedestrianListPresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void
PedestrianListPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
    m_pNKUIController->RegisterEvent(EVT_ENT_STARTOFFROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_ONROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_OFFROUTE, this);
    UpdateListTitle(m_pNavUIContex->IsOnRoute());
    NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),
                   "Pedestrian navigation List view displayed.\n");
}

void
PedestrianListPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    PresenterBase::OnDeactivate();

    NKUIEventData eventData;
    m_pNKUIController->SendEvent(EVT_LIST_CLOSED, eventData);
}

void
PedestrianListPresenter::OnSetWidget()
{
    m_pListWedget = dynamic_cast<NavigationListWidget*>(m_pWidget);
}


void
PedestrianListPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_ENT_STARTOFFROUTE:
        {
            UpdateListTitle(false);
            break;
        }
        case EVT_NAV_ONROUTE:
        {
            UpdateListTitle(true);
            break;
        }
        case EVT_NAV_OFFROUTE :
        {
            UpdateListTitle(false);
            break;
        }
        default:
            break;
    }
}

void
PedestrianListPresenter::ManeuverRemainingDistance(double distance)
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
PedestrianListPresenter::UpdateManeuverList(const nbnav::ManeuverList& maneuverList)
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

        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(),
                                distance, maneuverListData.distanceText,
                                maneuverListData.distanceUnit);

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
        maneuvers.push_back(maneuverListData);
    }
    m_pListWedget->UpdateManeuvers(maneuvers);
    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
}

void
PedestrianListPresenter::NextRoadName(std::string primaryName, std::string secondaryName)
{
    m_nextRoadName = primaryName;
    UpdateListTitle(m_pNavUIContex->IsOnRoute());
}

void
PedestrianListPresenter::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    m_currentRoadName = primaryName;
    UpdateListTitle(m_pNavUIContex->IsOnRoute());
}

void
PedestrianListPresenter::TrafficChanged(const nbnav::TrafficInformation& trafficInfo)
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
        trafficListData.description = trafficEvent.GetDescription();
        trafficListData.delayTime = trafficEvent.GetDelay();
        formater.FormatTime(trafficEvent.GetDelay(), trafficListData.delayText, trafficListData.delayUnit, trafficListData.delayText2, trafficListData.delayUnit2);
        trafficListData.maneuverID = trafficEvent.GetManeuverNumber();

        m_trafficEvents.push_back(trafficListData);

        iter++;
    }
    m_pListWedget->UpdateTrafficEvents(m_trafficEvents);
}

/* See description in header file. */
void PedestrianListPresenter::UpdateListTitle(bool isOnRoute)
{
    if (isOnRoute && !m_nextRoadName.empty())
    {
        m_pListWedget->UpdateTitle( m_pStringHelper->GetLocalizedString("IDS_WALKING_ON"),
                                    m_nextRoadName);
    }
    else
    {
        m_pListWedget->UpdateTitle( m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD"),
                                    m_pStringHelper->GetLocalizedString("IDS_HIGHLIGHTED_ROUTE"));
    }
}


/*! @} */
