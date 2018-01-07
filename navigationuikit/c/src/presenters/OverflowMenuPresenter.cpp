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
    @file         OverflowMenuPresenter.cpp
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

#include "OverflowMenuPresenter.h"
#include "NKUIState.h"
#include <algorithm>

using namespace std;
using namespace nbnav;
using namespace nkui;

static bool compareMenuItem(const ExtraMenuItem& m1, const ExtraMenuItem& m2)
{
    return m1.position < m2.position;
}

OverflowMenuPresenter::OverflowMenuPresenter()
    : m_pMenu(NULL)
{
}

OverflowMenuPresenter::~OverflowMenuPresenter()
{
}

void
OverflowMenuPresenter::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_MENU_HIDE || id == EVT_MENU_SELECTED)
    {
        m_pWidget->Hide();
        m_bActive = false;
    }
    PresenterBase::HandleEvent(id, data);
}

void
OverflowMenuPresenter::GetMenuItems(NKUIStateID state, std::vector<MenuOption>& menuItems)
{
    menuItems.clear();
    switch (state) {
        case NAVSTATE_NAVIGATION:
        case NAVSTATE_NAVIGATION_SAR:
        case NAVSTATE_NAVIGATION_LIST:
        case NAVSTATE_TRIP_OVERVIEW:
        case NAVSTATE_NAVIGATION_ENDTRIP:
        {
            MenuOption item = {0};
            item.displayName = m_pStringHelper->GetLocalizedString("IDS_END_TRIP");
            item.optionID = CMD_END_TRIP;
            menuItems.push_back(item);
            if(!m_pNavUIContex->IsEnhancedNavigationStartup())
            {
                item.displayName = m_pStringHelper->GetLocalizedString("IDS_ROUTE_OPTIONS");
                item.optionID = CMD_ROUTE_OPTION;
                menuItems.push_back(item);
                if(!m_pNavUIContex->IsRecalcingRoute() && !m_pNavUIContex->IsGettingDetour())
                {
                    item.displayName = m_pStringHelper->GetLocalizedString("IDS_DETOUR");
                    item.optionID = CMD_DETOUR;
                    menuItems.push_back(item);
                }
            }
            break;
        }
        case NAVSTATE_DETOUR:
        case NAVSTATE_DETOUR_LIST:
        {
            MenuOption item = {0};
            item.displayName = m_pStringHelper->GetLocalizedString("IDS_END_TRIP");
            item.optionID = CMD_END_TRIP;
            menuItems.push_back(item);
            item.displayName = m_pStringHelper->GetLocalizedString("IDS_ROUTE_OPTIONS");
            item.optionID = CMD_ROUTE_OPTION;
            menuItems.push_back(item);
            break;
        }
        case NAVSTATE_PEDESTRIAN:
        case NAVSTATE_PEDESTRIAN_LIST:
        case NAVSTATE_PEDESTRIAN_OVERVIEW:
        case NAVSTATE_PEDESTRIAN_UNLOCK:
        {
            MenuOption item = {0};
            item.displayName = m_pStringHelper->GetLocalizedString("IDS_END_TRIP");
            item.optionID = CMD_END_TRIP;
            menuItems.push_back(item);
            if(!m_pNavUIContex->IsEnhancedNavigationStartup())
            {
                item.displayName = m_pStringHelper->GetLocalizedString("IDS_RECALCULATE_ROUTE");
                item.optionID = CMD_RECALCULATE;
                menuItems.push_back(item);
                item.displayName = m_pStringHelper->GetLocalizedString("IDS_ROUTE_OPTIONS");
                item.optionID = CMD_ROUTE_OPTION;
                menuItems.push_back(item);
                item.displayName = m_pStringHelper->GetLocalizedString("IDS_DETOUR");
                item.optionID = CMD_DETOUR;
                menuItems.push_back(item);
            }
            break;
        }
        case NAVSTATE_ROUTE_SELECTION:
        case NAVSTATE_ROUTE_SELECTION_LIST:
        {
            MenuOption item = {0};
            item.displayName = m_pStringHelper->GetLocalizedString("IDS_ROUTE_OPTIONS");
            item.optionID = CMD_ROUTE_OPTION;
            menuItems.push_back(item);
            break;
        }
        default:
        {
            MenuOption endTrip = {0};
            endTrip.displayName = m_pStringHelper->GetLocalizedString("IDS_END_TRIP");
            endTrip.optionID = CMD_END_TRIP;
            menuItems.push_back(endTrip);
            break;
        }
    }
}

void
OverflowMenuPresenter::MergeMenuItems(std::vector<MenuOption>& menuItems, std::vector<ExtraMenuItem>& extraMenuItems)
{
    stable_sort(extraMenuItems.begin(), extraMenuItems.end(), compareMenuItem);
    for(std::vector<ExtraMenuItem>::iterator iter = extraMenuItems.begin(); iter != extraMenuItems.end(); iter++)
    {
        if(iter->position < menuItems.size())
        {
            menuItems.insert(menuItems.begin() + iter->position, iter->option);
        }
        else
        {
            menuItems.push_back(iter->option);
        }
    }
}

void
OverflowMenuPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if(event == EVT_MENU_PRESSED)
    {
        std::vector<MenuOption> menuItems;
        NKUIState* pCurrentState = (NKUIState*)data.eventData;
        if(pCurrentState)
        {
            GetMenuItems(pCurrentState->GetStateID(), menuItems);
        }
        m_pNKUIController->ShowOverflowMenu(menuItems,true);
        m_bActive = true;
    }
    else if(event == EVT_MENU_HIDE)
    {
        m_pWidget->Hide();
        m_bActive = false;
    }
    else if(event == EVT_MENU_SELECTED)
    {
        m_bActive = false;
    }
}

void
OverflowMenuPresenter::Activate(NKUIContext* navUIContext)
{
    PresenterBase::Activate(navUIContext);
}

void
OverflowMenuPresenter::OnActive()
{
    m_pNKUIController->RegisterEvent(EVT_MENU_PRESSED, this);
    m_pNKUIController->RegisterEvent(EVT_MENU_HIDE, this);
    m_pNKUIController->RegisterEvent(EVT_MENU_SELECTED, this);
    m_bActive = false;
}

void
OverflowMenuPresenter::OnDeactivate()
{
    m_pWidget->Hide();
}

void
OverflowMenuPresenter::OnSetWidget()
{
    m_pMenu = dynamic_cast<OverflowMenuWidget*>(m_pWidget);
}
