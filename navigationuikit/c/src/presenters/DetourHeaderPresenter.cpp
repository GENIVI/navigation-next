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
@file         DetourHeaderPresenter.cpp
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

#include "DetourHeaderPresenter.h"
#include "FormatController.h"
#include "palclock.h"
#include "RouteSelectInformation.h"

using namespace std;
using namespace nbnav;
using namespace nkui;

DetourHeaderPresenter::DetourHeaderPresenter():m_pRouteSelectHeader(NULL)
{

}

DetourHeaderPresenter::~DetourHeaderPresenter()
{

}

void DetourHeaderPresenter::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_NEW_ROUTE_SELECTED)
    {
        int* routeIndex = (int*)data;
        if(m_pNavUIContex)
        {
            if(*routeIndex < m_pNavUIContex->GetAllRoutes().size())
            {
                m_pNavUIContex->SetChosenRouteIndex(*routeIndex);
            }
        }
    }
    else if(id == EVT_TAP_HEADER)
    {
        id = EVT_ZOOM_FIT_ROUTE;
    }
    PresenterBase::HandleEvent(id, data);
}

void DetourHeaderPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_NEW_ROUTE_SELECTED:
        {
            m_pRouteSelectHeader->RouteSelected(m_pNavUIContex->GetChosenRouteIndex());
        }
            break;
        default:
            break;
    }
}

void DetourHeaderPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNKUIController->RegisterEvent(EVT_NEW_ROUTE_SELECTED, this);
    UpdateRouteInfomation(m_pNavUIContex->GetAllRoutes());
}

void DetourHeaderPresenter::OnSetWidget()
{
    m_pRouteSelectHeader = dynamic_cast<RouteSelectHeaderWidget*>(m_pWidget);
}

void DetourHeaderPresenter::OnDeactivate()
{
    PresenterBase::OnDeactivate();
}

void DetourHeaderPresenter::UpdateRouteInfomation(const vector<RouteInformation>& routes)
{
    RouteSelectInformation rtsInfo = RouteSelectInformation(m_pStringHelper,m_pNavUIContex);
    m_pRouteSelectHeader->SetRouteInfomation(rtsInfo.GetRouteInfoData(), m_pNavUIContex->GetChosenRouteIndex());
}

/*! @} */
