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
@file         NavigaitonFooterPresenter.cpp
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

#include "NavigationFooterPresenter.h"
#include "palclock.h"
#include "FormatController.h"

using namespace nkui;
using namespace std;
using namespace nbnav;

static const double MileToMeter   = 1609.33;
static const int    DistanceLimit = 500;

NavigationFooterPresenter::NavigationFooterPresenter()
    : m_pFooter(NULL),
      m_tripDistance(0)
{
}

NavigationFooterPresenter::~NavigationFooterPresenter()
{

}

void NavigationFooterPresenter::OnActive()
{
    FooterPresenterBase::OnActive();
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    UpdateFooter();
    m_pNKUIController->RegisterEvent(EVT_RECALCING_END, this);
}

void NavigationFooterPresenter::OnDeactivate()
{
    FooterPresenterBase::OnDeactivate();
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
}

void NavigationFooterPresenter::OnSetWidget()
{
    FooterPresenterBase::OnSetWidget();
    m_pFooter = dynamic_cast<NavigationFooterWidget*>(m_pWidget);
}

void NavigationFooterPresenter::TripRemainingTime(int time)
{
    FormatController formater(m_pStringHelper);
    //if trip longer than 500mil then show duration
    if(m_tripDistance / MileToMeter > DistanceLimit)
    {
        m_footerData.arrivalTitle = m_pStringHelper->GetLocalizedString("IDS_ARRIVAL_IN");
        formater.FormatTimeToHour(time, m_footerData.arrivalTime);
    }
    else
    {
        uint32 currentTime = PAL_ClockGetUnixTime();
        int arrivalTime = currentTime + time;
        m_footerData.arrivalTitle = m_pStringHelper->GetLocalizedString("IDS_ARRIVING_AT");
        m_footerData.arrivalTime = m_pStringHelper->GetLocalizedTimeString(arrivalTime);
    }
    m_pFooter->Update(m_footerData);
}

void NavigationFooterPresenter::TripRemainingDistance(double distance)
{
    m_tripDistance = distance;
    FormatController formater(m_pStringHelper);
    // Format Distance
    string distanceText;
    string distanceUnit;
    formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), distance, distanceText, distanceUnit);
    m_footerData.distance = distanceText + " " + distanceUnit;
    m_pFooter->Update(m_footerData);
}

/* See description in header file. */
void NavigationFooterPresenter::TripTrafficColor(char color)
{
    if (color != m_footerData.traffic)
    {
        if(m_pNavUIContex->GetRouteOptions().GetTransportationMode() != Pedestrian)
        {
            m_footerData.traffic = color;
        }
        else
        {
            m_footerData.traffic = 0;
        }
        m_pFooter->Update(m_footerData);
    }
}

void NavigationFooterPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    FooterPresenterBase::NotifyEvent(event, data);
    if(event == EVT_RECALCING_END)
    {
        UpdateFooter();
    }
}

void NavigationFooterPresenter::UpdateFooter()
{
    if(m_pNavUIContex->GetRouteOptions().GetTransportationMode() != Pedestrian)
    {
        m_footerData.traffic = m_pNavUIContex->GetChosenRoute().GetTrafficColor();
    }
    else
    {
        m_footerData.traffic = 0;
    }
    m_pFooter->Update(m_footerData);
    TripRemainingTime(m_pNavUIContex->GetChosenRoute().GetTime());
    TripRemainingDistance(m_pNavUIContex->GetChosenRoute().GetDistance());
}

/*! @} */
