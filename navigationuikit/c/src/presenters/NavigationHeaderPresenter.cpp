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
@file         NavigationHeaderPresenter.cpp
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

#include "NavigationHeaderPresenter.h"
#include "FormatController.h"

using namespace nkui;
using namespace std;
using namespace nbnav;

#define TURNARROW_COLOR             0x222222
#define DESTINATION_COLOR           0xb70100
#define ORIGINATION_COLOR           0x2dc62d

#define RGB_2_RGBA(X)               ((X) << 8)


//@todo: should move this functionality to navkit.
static inline uint32
GetImageColor(uint16 character)
{
    uint32 color = TURNARROW_COLOR;
    switch (character)
    {
        case 'X':
        {
            color = ORIGINATION_COLOR;
            break;
        }
        case 'V':
        case 'W':
        case 'U':
        {
            color = DESTINATION_COLOR;
            break;
        }
        default:
        {
            break;
        }
    }

    return RGB_2_RGBA(color);
}

NavigationHeaderPresenter::NavigationHeaderPresenter()
    : m_pHeader(NULL), m_bNeedUpdateAll(true), m_bArrivaling(false)
{
    m_headerData.viewStyle           = MIT_Normal;
    m_headerData.turnArrow.color     = TURNARROW_COLOR;
    m_headerData.turnArrow.character = 0;
}

NavigationHeaderPresenter::~NavigationHeaderPresenter()
{

}

void NavigationHeaderPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_ENT_STARTOFFROUTE:
        {
            m_headerData.primaryText = m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD");
            m_headerData.secondText = m_nextRoadName;
            m_headerData.viewStyle = MIT_StartOffroute;
            m_pHeader->Update(m_headerData);
            break;
        }
        case EVT_NAV_ONROUTE:
        {
            if(!m_bArrivaling)
            {
                m_headerData.primaryText = m_nextRoadName;
                m_headerData.secondText = m_secondaryName;
                m_headerData.viewStyle = MIT_Normal;
                m_pHeader->Update(m_headerData);
            }
            break;
        }
        case EVT_NAV_OFFROUTE:
        {
            m_bArrivaling = false;
            if(m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
            {
                UpdateOffRoutePedestrianHeader();
            }
        }
        default:
            break;
    }
}

void NavigationHeaderPresenter::UpdateOffRoutePedestrianHeader()
{
    m_headerData.primaryText = m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD");
    m_headerData.secondText = m_pStringHelper->GetLocalizedString("IDS_HIGHLIGHTED_ROUTE");
    m_headerData.viewStyle = MIT_StartOffroute;
    m_pHeader->Update(m_headerData);
}

void NavigationHeaderPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNKUIController->RegisterEvent(EVT_ENT_STARTOFFROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_ONROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_OFFROUTE, this);
    //after add listener ui should update data immediately
    m_pHeader->Update(m_headerData);
    if(!m_pNavUIContex->IsOnRoute() && m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
    {
        UpdateOffRoutePedestrianHeader();
    }
}

void NavigationHeaderPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    PresenterBase::OnDeactivate();
}

void NavigationHeaderPresenter::OnSetWidget()
{
    m_pHeader = dynamic_cast<NavigationHeaderWidget*>(m_pWidget);
}

void NavigationHeaderPresenter::NextRoadName(std::string primaryName, std::string secondaryName)
{
    m_nextRoadName = primaryName;
    m_secondaryName = secondaryName;
    if(m_bArrivaling)
    {
        return;
    }
    this->m_bNeedUpdateAll = true;
    if (m_headerData.viewStyle != MIT_Arrival)
    {
        if(!m_pNavUIContex->IsStartupOffroute())
        {
            //when pedestrian and offroute the header will show headtoward highlighted route
            if(!(!m_pNavUIContex->IsOnRoute() && m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian))
            {
                m_headerData.primaryText = primaryName;
                m_headerData.secondText = secondaryName;
            }
        }
        else
        {
            m_headerData.primaryText = m_pStringHelper->GetLocalizedString("IDS_HEAD_TOWARD");
            m_headerData.secondText = m_nextRoadName;
            m_headerData.viewStyle = MIT_StartOffroute;
            m_pHeader->Update(m_headerData);
        }
    }
}

void NavigationHeaderPresenter::ManeuverRemainingDistance(double distance)
{
    FormatController formater(m_pStringHelper);
    // Format Distance
    string distanceText;
    string distanceUnit;
    formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, distanceText, distanceUnit);
    m_headerData.distance = distanceText + " " + distanceUnit;
    if(!m_bNeedUpdateAll)
    {
        m_pHeader->UpdateTripDistance(m_headerData.distance);
    }
    else
    {
        m_pHeader->Update(m_headerData);
        m_bNeedUpdateAll = false;
    }
}

void NavigationHeaderPresenter::ManeuverImageId(string imageId)
{
    if(!imageId.empty())
    {
        m_headerData.turnArrow.character = imageId[0];
        m_headerData.turnArrow.color = GetImageColor(m_headerData.turnArrow.character);
    }
    this->m_bNeedUpdateAll = true;
}

void NavigationHeaderPresenter::ManeuverExitNumber(std::string exitNumber)
{
    this->m_bNeedUpdateAll = true;
    m_headerData.exitText = exitNumber;
}

void NavigationHeaderPresenter::UpdateManeuverList(const nbnav::ManeuverList& maneuverList)
{
    if(maneuverList.GetNumberOfManeuvers() == 1 && maneuverList.GetManeuver(0)->IsDestination() && !m_pNavUIContex->IsStartupOffroute())
    {
        m_bArrivaling = true;
        m_headerData.viewStyle = MIT_Arrival;
        m_headerData.primaryText = m_pNavUIContex->GetDestinationStreetSideText(m_pStringHelper);
        m_headerData.secondText = m_pNavUIContex->GetDestinationStreetText();
        m_pHeader->Update(m_headerData);
    }
    else
    {
        m_headerData.viewStyle = MIT_Normal;
        m_bArrivaling = false;
    }
}

/*! @} */
