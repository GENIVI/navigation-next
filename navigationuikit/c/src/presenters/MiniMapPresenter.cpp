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
@file         MiniMapPresenter.cpp
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

#include "MiniMapPresenter.h"
#include "nkui_macros.h"

namespace nkui
{
class TrafficInformation
{
public:
    TrafficInformation(const nbnav::TrafficEvent& event)
        : m_type(event.GetType()),
          m_severity(event.GetSeverity()) {}
    virtual ~TrafficInformation() {}

    nbnav::TrafficEvent::Type            m_type;
    nbnav::TrafficEvent::TrafficSeverity m_severity;
};
}

using namespace nkui;
using namespace std;
using namespace nbnav;


MiniMapPresenter::MiniMapPresenter()
    : m_pMiniMap(NULL),
      m_mapLocked(false)
{
}

MiniMapPresenter::~MiniMapPresenter()
{

}

void MiniMapPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if (!IsActive())
    {
        return;
    }

    switch (event)
    {
        case EVT_STATE_CHANGED:
        {
            ToggleTrafficTip();
            break;
        }
        case EVT_MAP_CAMERA_LOCKED:
        case EVT_MAP_CAMERA_UNLOCKED:
        {
            m_mapLocked = event == EVT_MAP_CAMERA_LOCKED;
            ToggleTrafficTip();
            break;
        }
        default:
        {
            break;
        }
    }
}

void MiniMapPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNKUIController->RegisterEvent(EVT_STATE_CHANGED, this);
    m_pNKUIController->RegisterEvent(EVT_MAP_CAMERA_UNLOCKED, this);
    m_pNKUIController->RegisterEvent(EVT_MAP_CAMERA_LOCKED, this);
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
}

void MiniMapPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    PresenterBase::OnDeactivate();
}

void MiniMapPresenter::OnSetWidget()
{
    m_pMiniMap = dynamic_cast<MiniMapWidget*>(m_pWidget);
}

void MiniMapPresenter::TrafficAlerted(const nbnav::TrafficEvent& trafficEvent)
{
    m_currentTraffic.reset(NKUI_NEW TrafficInformation(trafficEvent));
    ToggleTrafficTip();
}

void MiniMapPresenter::DisableTrafficAlerted()
{
    m_currentTraffic.reset();
    ToggleTrafficTip();
}

/* See description in header file. */
inline void MiniMapPresenter::ToggleTrafficTip()
{
    if (m_currentTraffic && m_mapLocked &&
        m_pNavUIContex->GetPublicState() == NPS_Navigation)
    {
        m_pMiniMap->updateTraffic(m_currentTraffic->m_severity,
                                  m_currentTraffic->m_type);
    }
    else
    {
        m_pMiniMap->HideTraffic();
    }
}

/*! @} */
