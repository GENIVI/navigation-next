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
    @file         RouteSelectBubblePresenter.cpp
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

#include "RouteSelectBubblePresenter.h"
#include "FormatController.h"
#include <algorithm>
#include <sstream>

using namespace std;
using namespace nkui;
using namespace nbnav;

RouteSelectBubblePresenter::RouteSelectBubblePresenter()
{
}

RouteSelectBubblePresenter::~RouteSelectBubblePresenter()
{
}

void
RouteSelectBubblePresenter::HandleEvent(NKEventID id, void* data)
{
    switch (id)
    {
        case EVT_NEW_ROUTE_SELECTED:
            {
                UnselectBubble();
                int* routeIndex = (int*)data;
                if(m_pNavUIContex)
                {
                    m_pNavUIContex->SetChosenRouteIndex(*routeIndex);
                }
                SelectBubble(*routeIndex);
            }
            break;
        default:
            break;
    }
    PresenterBase::HandleEvent(id, data);
}

void
RouteSelectBubblePresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if (!IsActive())
    {
        return;
    }
    switch (event)
    {
        case EVT_NEW_ROUTE_SELECTED:
            {
                UnselectBubble();
                int* routeIndex = (int*)data.eventData;
                SelectBubble(*routeIndex);
            }
            break;
        case EVT_MAP_CAMERA_UPDATED:
        case EVT_SCREEN_SIZE_CHANGING:
        case EVT_SCREEN_SIZE_CHANGED:
            {
                nkui::NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
                vector<BubbleInformation>::iterator iter = m_bubbles.begin();
                while (iter != m_bubbles.end())
                {
                    ScreenPoint screenPoint = pMapInterface->ToScreenLocation(iter->second);
                    iter->first->SetPosition(screenPoint);
                    iter++;
                }
                m_pBubbleLayer->UpdateLayout(false);
            }
            break;
        default:
            break;
    }
}

void
RouteSelectBubblePresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNKUIController->RegisterEvent(EVT_NEW_ROUTE_SELECTED, this);
    m_pNKUIController->RegisterEvent(EVT_MAP_CAMERA_UPDATED, this);
    m_pNKUIController->RegisterEvent(EVT_SCREEN_SIZE_CHANGING, this);
    m_pNKUIController->RegisterEvent(EVT_SCREEN_SIZE_CHANGED, this);
    m_pNavUIContex->GetNavigation()->AddSessionListener(this);
    const vector<RouteInformation>& routes = m_pNavUIContex->GetAllRoutes();
    UpdateBubbles(routes);
    UnselectBubble();
    SelectBubble(m_pNavUIContex->GetChosenRouteIndex());
    m_pBubbleLayer->UpdateLayout(true);
}

void
RouteSelectBubblePresenter::OnDeactivate()
{
    m_pBubbleLayer->ClearBubbles();
    m_bubbles.clear();
    m_pNavUIContex->GetNavigation()->RemoveSessionListener(this);
    PresenterBase::OnDeactivate();
}

void
RouteSelectBubblePresenter::OnSetWidget()
{
    m_pBubbleLayer = dynamic_cast<RouteSelectBubbleLayer*>(m_pWidget);
}

void
RouteSelectBubblePresenter::RouteReceived(RouteRequestReason reason, vector<RouteInformation>& routes)
{
    UpdateBubbles(routes);
    UnselectBubble();
    if (m_pNavUIContex->GetPublicState() == NPS_Detour)
    {
        // for detour, the last route is the current route
        SelectBubble((int)(routes.size() - 1));
    }
    else
    {
        SelectBubble(0);
    }
}

void
RouteSelectBubblePresenter::UpdateBubbles(const vector<RouteInformation>& routes)
{
    m_pBubbleLayer->ClearBubbles();
    m_bubbles.clear();
    if (routes.empty())
    {
        return;
    }
    nkui::NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (m_pNavUIContex->GetPublicState() == NPS_Detour)
    {
        size_t routeCount = routes.size();
        // last one is the current route
        const RouteInformation& currentRoute = *routes.rbegin();
        int32 currentTime = (int32)currentRoute.GetTime();
        for (size_t index = 0; index < routeCount; ++index)
        {
            const RouteInformation& route       = routes.at(index);
            Coordinates             coordinate  = route.GetLabelPoint();

            NKUICoordinate     nkuiCoordinate(coordinate.latitude, coordinate.longitude);
            ScreenPoint        screenPoint = pMapInterface->ToScreenLocation(nkuiCoordinate);
            RouteSelectBubble* pBubble     = m_pBubbleLayer->CreateBubble(screenPoint);
            if (!pBubble)
            {
                continue;
            }

            string label;
            if (index == (routeCount-1))
            {
                label = m_pStringHelper->GetLocalizedString("IDS_CURRENT");
            }
            else
            {
                // don't need to care about singed/unsigned here, (2<<31) seconds is long
                // enough for normal travels...
                int32 delta = (int32)route.GetTime() - currentTime;
                label = FormatTime((uint32)abs(delta)) + "\n" + \
                        m_pStringHelper->GetLocalizedString(
                            delta < 0 ? "IDS_FASTER" : "IDS_SLOWER");
            }
            pBubble->SetText(label);
            m_bubbles.push_back(BubbleInformation(pBubble, nkuiCoordinate));
        }
    }
    else
    {
        for (vector<RouteInformation>::size_type i = 0; i < routes.size(); ++i)
        {
            const RouteInformation& route = routes[i];
            Coordinates coordinate = route.GetLabelPoint();
            NKUICoordinate nkuiCoordinate(coordinate.latitude, coordinate.longitude);
            ScreenPoint screenPoint = pMapInterface->ToScreenLocation(nkuiCoordinate);
            RouteSelectBubble* pBubble = m_pBubbleLayer->CreateBubble(screenPoint);

            if (pBubble)
            {
                pBubble->SetText(FormatTime(route.GetTime()));
                m_bubbles.push_back(BubbleInformation(pBubble, nkuiCoordinate));
            }
        }
    }
    m_pBubbleLayer->UpdateLayout(false);
}

void
RouteSelectBubblePresenter::SelectBubble(int index)
{
    if (index < m_bubbles.size())
    {
        BubbleInformation& bubbleInformation = m_bubbles[index];
        bubbleInformation.first->SetSelected(true);
        if (m_pNKUIController)
        {
            NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),"Taping on a route bubble.\n");
        }
    }
}

void
RouteSelectBubblePresenter::UnselectBubble()
{
    vector<BubbleInformation>::iterator iter = m_bubbles.begin();
    while (iter != m_bubbles.end())
    {
        iter->first->SetSelected(false);
        iter ++;
    }
}

/* See description in header file. */
string RouteSelectBubblePresenter::FormatTime(uint32 time)
{
    string timeText;
    string timeText2;
    string timeUnit;
    string timeUnit2;
    FormatController formater(m_pStringHelper);
    formater.FormatTime(time, timeText, timeUnit, timeText2, timeUnit2);
    timeText = timeText + " " + timeUnit;
    if(timeText2.size() > 0)
    {
        timeText = timeText + " " + timeText2 + " " + timeUnit2;
    }

    return timeText;
}
