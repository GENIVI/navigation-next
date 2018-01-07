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
@file         CurrentRoadPresenter.cpp
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

#include "CurrentRoadPresenter.h"

using namespace nkui;
using namespace std;
using namespace nbnav;

CurrentRoadPresenter::CurrentRoadPresenter()
    : m_pCurrentRoad(NULL),
      m_status(NWS_None)
{
}

CurrentRoadPresenter::~CurrentRoadPresenter()
{

}

void CurrentRoadPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if(!IsActive())
    {
        return;
    }
    switch (event)
    {
        case EVT_LANEGUIDANCE_SHOW:
        {
            m_status |= NWS_LaneGuidance;
            break;
        }
        case EVT_LANEGUIDANCE_HIDE:
        {
            m_status &= ~NWS_LaneGuidance;
            break;
        }
        case EVT_SAR_SHOW:
        {
            m_status |= NWS_SAR;
            break;
        }
        case EVT_SAR_HIDE:
        {
            m_status &= ~NWS_SAR;
            break;
        }
        case EVT_NAV_OFFROUTE:
        {
            m_status |= NWS_OffRoute;
            break;
        }
        case EVT_NAV_ONROUTE:
        {
            m_status &= ~NWS_OffRoute;
            break;
        }
        case EVT_SHOW_CURRENTROAD:
        {
            m_status &= ~NWS_HideCurrentRoad;
            break;
        }
        case EVT_HIDE_CURRENTROAD:
        {
            m_status |= NWS_HideCurrentRoad;
            break;
        }
        default:
            break;
    }

    if (ShouldHideCurrentRoad())
    {
        m_pWidget->Hide();
    }
    else
    {
        m_pWidget->Show();
    }

}

void CurrentRoadPresenter::OnActive()
{
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNavUIContex->GetNavigation()->AddNavEventListener(this);
    m_pNKUIController->RegisterEvent(EVT_NAV_OFFROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_ONROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_SAR_SHOW, this);
    m_pNKUIController->RegisterEvent(EVT_SAR_HIDE, this);
    m_pNKUIController->RegisterEvent(EVT_LANEGUIDANCE_SHOW, this);
    m_pNKUIController->RegisterEvent(EVT_LANEGUIDANCE_HIDE, this);
    m_pNKUIController->RegisterEvent(EVT_HIDE_CURRENTROAD, this);
    m_pNKUIController->RegisterEvent(EVT_SHOW_CURRENTROAD, this);

    if(m_pNavUIContex->IsOnRoute())
    {
        m_status &= ~NWS_OffRoute;
    }
    else
    {
        m_status |= NWS_OffRoute;
    }

    if (m_pNavUIContex->IsRoadSignShown())
    {
        m_status |= NWS_SAR;
    }
    else
    {
        m_status &= ~NWS_SAR;
    }

    if(ShouldHideCurrentRoad())
    {
        m_pWidget->Hide();
    }
    else
    {
        m_pWidget->Show();
    }

}

void CurrentRoadPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    m_pWidget->Hide();
}

void CurrentRoadPresenter::OnSetWidget()
{
    m_pCurrentRoad = dynamic_cast<CurrentRoadWidget*>(m_pWidget);
}

void CurrentRoadPresenter::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    m_pCurrentRoad->UpdateCurrentRoad(primaryName);
}

/*! @} */
