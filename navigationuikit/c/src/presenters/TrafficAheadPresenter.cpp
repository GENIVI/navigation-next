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
@file         TrafficAheadPresenter.cpp
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

#include "TrafficAheadPresenter.h"
#include "nkui_macros.h"

namespace nkui
{
using namespace nkui;
using namespace std;
using namespace nbnav;


TrafficAheadPresenter::TrafficAheadPresenter() : m_isShow(false), m_isClosed(false)
{
}

TrafficAheadPresenter::~TrafficAheadPresenter()
{

}

void TrafficAheadPresenter::OnActive()
{
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
}

void TrafficAheadPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    PresenterBase::OnDeactivate();
}

void TrafficAheadPresenter::OnSetWidget()
{
    if(!m_isShow && m_pWidget)
    {
        m_pWidget->Hide();
    }
}

void TrafficAheadPresenter::TrafficAlerted(const nbnav::TrafficEvent& trafficEvent)
{
    m_isShow = true;
    if(m_pWidget && !m_isClosed)
    {
        m_pWidget->Show();
    }
}

void TrafficAheadPresenter::DisableTrafficAlerted()
{
    m_isShow = false;
    m_isClosed = false;
    if(m_pWidget)
    {
        m_pWidget->Hide();
    }
}

void TrafficAheadPresenter::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_TRAFFIC_CLOSE)
    {
        m_isShow = false;
        m_isClosed = true;
        m_pWidget->Hide();
    }
    PresenterBase::HandleEvent(id, data);
}

void TrafficAheadPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
}
}
/*! @} */
