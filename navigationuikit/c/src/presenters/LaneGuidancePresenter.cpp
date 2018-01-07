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
    @file         LaneGuidancePresenter.cpp
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

#include "LaneGuidancePresenter.h"

using namespace std;
using namespace nkui;

LaneGuidancePresenter::LaneGuidancePresenter(bool bWithSAR)
    : m_pLaneGuidanceWidget(NULL)
    , m_bWithSAR(bWithSAR)
{
}

LaneGuidancePresenter::~LaneGuidancePresenter()
{
}

void
LaneGuidancePresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void
LaneGuidancePresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
}

void
LaneGuidancePresenter::OnActive()
{
    m_pNavUIContex->GetNavigation()->AddNavEventListener(this);
}

void
LaneGuidancePresenter::OnDeactivate()
{
    m_pWidget->Hide();
    m_pNavUIContex->GetNavigation()->RemoveNavEventListener(this);
}

void
LaneGuidancePresenter::OnSetWidget()
{
    m_pLaneGuidanceWidget = dynamic_cast<LaneGuidanceWidget*>(m_pWidget);
    if (m_bWithSAR)
    {
        m_pLaneGuidanceWidget->SetType(LGW_LaneGuidanceWidgetType_SAR);
    }
    else
    {
        m_pLaneGuidanceWidget->SetType(LGW_LaneGuidanceWidgetType_Turn);
    }
}

void
LaneGuidancePresenter::NavEvent(nbnav::NavEventTypeMask navEventTypeMask, const nbnav::LaneInformation& laneInfo,
                       const nbnav::RoadSign& signInfo, const nbnav::SpeedLimitInformation& speedLimitInfo,
                       const nbnav::TrafficEvent& trafficEvent, const nbnav::TrafficInformation& trafficInfo)
{
    if(!IsActive())
    {
        return;
    }
    if (navEventTypeMask & nbnav::NETP_LaneInformationEnable)
    {
        vector<nbnav::Lane> lanes = laneInfo.Lanes();
        m_pLaneGuidanceWidget->UpdateLaneGuidance(lanes, laneInfo.SelectedLanePosition(), laneInfo.SelectedLanes());
        if (m_bWithSAR)
        {
            m_pLaneGuidanceWidget->SetType(LGW_LaneGuidanceWidgetType_SAR);
        }
        else
        {
            m_pLaneGuidanceWidget->SetType(LGW_LaneGuidanceWidgetType_Turn);
        }
        m_pWidget->Show();
        NKUIEventData data = {0};
        m_pNKUIController->SendEvent(EVT_LANEGUIDANCE_SHOW, data);
    }
    else if (navEventTypeMask & nbnav::NETP_LaneInformationDisable)
    {
        m_pWidget->Hide();
        NKUIEventData data = {0};
        m_pNKUIController->SendEvent(EVT_LANEGUIDANCE_HIDE, data);
    }
}
