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
    @file widgetmanager.cpp
    @date 10/20/2014
    @addtogroup navigationuikit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

#include "widgetmanager.h"
#include "NKUIController.h"
#include "announcementimpl.h"
#include "currentroadimpl.h"
#include "stackmaneuverwidgetimpl.h"
#include "minimapwidgetimpl.h"
#include "nogpswidgetimpl.h"
#include "speedlimitwidgetimpl.h"
#include "navigationheaderwidgetimpl.h"
#include "navigationfooterwidgetimpl.h"
#include "sarwidgetimpl.h"
#include "laneguidancewidgetimpl.h"
#include "routerequestfooterimpl.h"
#include "routeselectheaderwidgetimpl.h"
#include "routeselectlistwidgetimpl.h"
#include "routeselectfooterimpl.h"
#include "routerequestfooterimpl.h"
#include "overflowmenuwidgetimpl.h"
#include "minimaptripoverviewimpl.h"
#include "tripoverviewimpl.h"
#include "navigationlistwidgetimpl.h"
#include "mapinterfacewrapper.h"
#include "arrivalfooterwidgetimpl.h"
#include "arrivalheaderwidgetimpl.h"
#include "retrywidgetimpl.h"
#include "detourfooterimpl.h"
#include "detourheaderwidgetimpl.h"
#include "routeselectbubblelayerimpl.h"
#include "minimaprtsimpl.h"
#include "enhancednavstartupnotifyimpl.h"
#include "startingnavfooterimpl.h"
#include "enhancedstartheaderwidgetimpl.h"
#include "recalconcalltextwidgetimpl.h"
#include "statusbarimpl.h"
#include "routeselectstatusbarimpl.h"
#include "stopnavigationwidgetimpl.h"
#include "pinbubblelayerimpl.h"
#include "endtripstatusbarimpl.h"
#include "trafficaheadwidgetimpl.h"
#include "overviewfooterimpl.h"
#include <QMultiMap>

using namespace nkui;
using namespace locationtoolkit;

WidgetManager::WidgetManager(QWidget *parent, WidgetLayoutConfig *config)
    : mParentWidget(parent)
    , mLayoutConfig(config)
    , mNavController(NULL)
    , mIsLandscape(true)
    , mState(NAVSTATE_INVALID)
{
    mWidgets.clear();

    if(mParentWidget->width() > mParentWidget->height())
    {
        mIsLandscape = true;
    }
    else
    {
        mIsLandscape = false;
    }
}

WidgetManager::~WidgetManager()
{
    std::map<nkui::WidgetID, nkui::Widget*>::iterator it = mWidgets.begin();
    for(; it != mWidgets.end(); ++it)
    {
        nkui::Widget* widget = it->second;
        if(widget != NULL)
        {
            delete widget;
            widget = NULL;
        }
    }
    mWidgets.clear();
}

void WidgetManager::LayoutWidgets(const std::set<Widget *> &activeWidget, NKUIStateID state)
{
    std::set<Widget*> intersection;
    set_difference(activeWidget.begin(), activeWidget.end(),
                   mActiveWidgets.begin(), mActiveWidgets.end(),
                   inserter(intersection, intersection.end()));

    QMultiMap<int, QWidget*> widgets;
    //state changed or activeWidgets changed need relayout
    if(mState != state || !intersection.empty())
    {
        mActiveWidgets = activeWidget;
        bool isLandscape = false;

        if(mParentWidget->width() > mParentWidget->height())
        {
            isLandscape = true;
        }

        for(std::set<Widget*>::const_iterator item = activeWidget.begin(); item != activeWidget.end(); item++)
        {
            WidgetBase* widget = dynamic_cast<WidgetBase*>(*item);
            if (!widget)
            {
                continue;
            }

            WidgetFrame parent = {0};
            WidgetFrame viewFrame = {0};
            bool flag = false; //this flag indicate a widget exist on the two state when state change
            parent.height = mParentWidget->height();
            parent.width = mParentWidget->width();
            QWidget* view = widget->GetNativeView();
            if(view != NULL &&(mState != state))
            {
                int zorder = mLayoutConfig->GetWidgetZorder(state, widget->GetWidgetID());
                widgets.insert(zorder, view);

                float alpha = mLayoutConfig->GetWidgetAlpha(state, widget->GetWidgetID());
                if (alpha >= 0.f)
                {
                    view->setWindowOpacity(alpha);
                }
            }

            if(mLayoutConfig->GetWidgetFrame(state, widget->GetWidgetID(), parent, viewFrame))
            {
                QRect frame(viewFrame.posx, viewFrame.posy, viewFrame.width, viewFrame.height);
                if(flag && !view->isHidden() &&((view->pos().x() != frame.x())
                    || (view->pos().y() != frame.y())
                    || (view->width() != frame.width())
                    || (view->height() != frame.height())) && mState != NAVSTATE_INVALID)
                {
                    widget->UpdateWidgetFrame(frame);
                }
                else
                {
                    widget->SetFrame(viewFrame);
                }

                widget->UpdateLayout();
                widget->SetLandscape(isLandscape);
            }
        }
        //change widgets zorder
         QMultiMap<int, QWidget*>::iterator it = widgets.begin();
         while (it != widgets.end())
         {
             it.value()->raise();
             it++;
         }
        mState = state;
    }
}

void WidgetManager::ScreenSizeChanged()
{
    if(mNavController)
    {
        NKUIEventData data = {0};
        mNavController->SendEvent(EVT_SCREEN_SIZE_CHANGED, data);
    }
}

shared_ptr<Widget> WidgetManager::GetWidget(WidgetID widgetID)
{
    WidgetBase* pWidget = NULL;
    //create widget by widget ID
    switch (widgetID)
    {
    case WGT_ANNOUNCER:
    {
        pWidget = new AnnouncementImpl();
        break;
    }
    case WGT_NOGPS:
    {
        pWidget = new NoGPSWidgetImpl(widgetID);
        break;
    }
    case WGT_NAV_FOOTER:
    {
        pWidget = new NavigationFooterWidgetImpl();
        break;
    }
    case WGT_NAV_HEADER:
    {
        pWidget = new NavigationHeaderWidgetImpl(widgetID);
        break;
    }
    case WGT_STACKED_MANEUVER:
    {
        pWidget = new StackManeuverWidgetImpl(widgetID);
        break;
    }
    case WGT_SAR:
    {
        pWidget = new SARWidgetImpl();
        break;
    }
    case WGT_LANE_GUIDANCE:
    case WGT_LANE_GUIDANCE_SAR:
    {
        pWidget = new LaneGuidanceWidgetImpl(widgetID);
        break;
    }
    case WGT_CURRENTROAD:
    {
        pWidget = new CurrentRoadImpl();
        break;
    }
    case WGT_SPEED_LIMITS:
    {
        pWidget = new SpeedLimitWidgetImpl();
        break;
    }
    case WGT_MINI_MAP_NAV:
    {
        pWidget = new MiniMapWidgetImpl(nkui::WGT_MINI_MAP_NAV);
        break;
    }
    case WGT_OVERFLOW_MENU:
    {
        pWidget = new OverflowMenuWidgetImpl();
        break;
    }
    case WGT_OVERVIEW_FOOTER:
    {
        pWidget = new OverviewFooterImpl();
        break;
    }
    case WGT_REQUEST_FOOTER:
    {
        pWidget = new RouteRequestfooterImpl();
        break;
    }
    case WGT_ROUTE_SELECT_HEADER:
    {
        pWidget = new RouteSelectHeaderWidgetImpl();
        break;
    }
    case WGT_ROUTE_DETALLS_LIST:
    {
        pWidget = new RouteSelectListWidgetImpl();
        break;
    }
    case WGT_ROUTE_SELECT_FOOTER:
    {
        pWidget = new RouteselectfooterImpl();
        break;
    }
    case WGT_MINI_MAP_LOCATEME:
    {
        pWidget = new MiniMapTripOverviewImpl();
        break;
    }
    case WGT_MINI_MAP_RTS:
    {
        pWidget = new MiniMapRtsImpl();
        break;
    }
    case WGT_TRIP_OVERVIEW_HEADER:
    {
        pWidget = new TripOverviewImpl();
        break;
    }
    case WGT_NAVIGATION_LIST:
    {
        pWidget = new NavigationListWidgetImpl();
        break;
    }
    case WGT_ARRIVAL_FOOTER:
    {
        pWidget = new ArrivalFooterWidgetImpl();
        break;
    }
    case WGT_ARRIVAL_HEADER:
    {
        pWidget = new ArrivalHeaderWidgetImpl();
        break;
    }
    case WGT_RETRY:
    {
        pWidget = new RetryWidgetImpl();
        break;
    }
    case WGT_DETOUR_HEADER:
    {
        pWidget = new DetourHeaderWidgetImpl();
        break;
    }
    case WGT_DETOUR_FOOTER:
    {
        pWidget = new DetourFooterImpl();
        break;
    }
    case WGT_ROUTE_SELECT_BUBBLE:
    {
        pWidget = new RouteSelectBubbleLayerImpl();
        break;
    }
    case WGT_RECALCULATE:
    {
        pWidget = new RecalcOnCallTextWidgetImpl();
        break;
    }
    case WGT_ENHANCE_NAV_NOTIFICATION:
    {
        pWidget = new EnhancedNavStartupNotifyImpl();
        break;
    }
    case WGT_STARTINGNAV_FOOTER:
    {
        pWidget = new StartingNavfooterImpl();
        break;
    }
    case WGT_ENHANCE_HEADER:
    {
        pWidget = new EnhancedStartHeaderWidgetImpl();
        break;
    }
    case WGT_STATUS_BAR:
    {
        pWidget = new StatusBarImpl();
        break;
    }
    case WGT_ROUTE_SELECT_STATUS_BAR:
    {
        pWidget = new RouteSelectStatusBarImpl();
        break;
    }
    case WGT_STOP_NAVIGATION:
    {
        pWidget = new StopNavigationWidgetImpl();
        break;
    }
    case WGT_PIN_BUBBLE:
    {
        pWidget = new PinBubbleLayerImpl();
        break;
    }
    case WGT_ENDTRIP_BAR:
    {
        pWidget = new EndTripStatusBarImpl();
        break;
    }
    case WGT_TRAFFIC_AHEAD:
    {
        pWidget = new TrafficAheadWidgetImpl();
        break;
    }
    default:
        break;
    }

    if(pWidget != NULL)
    {
        //the native view, so a widget can create it native ui view
        pWidget->InitWidget(mParentWidget);
        mWidgets[widgetID] = pWidget;
    }

    return shared_ptr<Widget>(pWidget);
}

void WidgetManager::LayoutWidgets()
{
    NKUIStateID temporaryState = mState;
    if(mParentWidget->width() > mParentWidget->height())
    {
        mIsLandscape = true;
    }
    else
    {
        mIsLandscape = false;
    }

    for(std::set<Widget*>::const_iterator item = mActiveWidgets.begin(); item != mActiveWidgets.end(); item++)
    {
        WidgetBase* widget = dynamic_cast<WidgetBase*>(*item);
        if(widget)
        {
            widget->SetLandscape(mIsLandscape);
        }
    }

    // because LayoutWidgets will not do layout when state and activeWidget not change,
    // so set mState to invalid to force it layout widget
    mState = NAVSTATE_INVALID;
    LayoutWidgets(mActiveWidgets, temporaryState);

    if(mNavController)
    {
        NKUIEventData data = {0};
        mNavController->SendEvent(EVT_SCREEN_SIZE_CHANGING, data);
    }
}

bool WidgetManager::InitLayoutConfigs()
{
    return mLayoutConfig->LoadConfigs();
}

void WidgetManager::SetNKUIController(NKUIController *controller)
{
    mNavController = controller;
}

const WidgetPosition WidgetManager::GetWidgetPosition(WidgetID widgetID, NKUIStateID state) const
{
    WidgetFrame parent = {0, 0, mParentWidget->width(), mParentWidget->height()};
    WidgetFrame viewFrame = {0};
    WidgetPosition widgetPosition = {0};
    if(mLayoutConfig->GetWidgetFrame(state, widgetID, parent, viewFrame))
    {
        widgetPosition.left = viewFrame.posx;
        widgetPosition.top = viewFrame.posy;
        widgetPosition.width = viewFrame.width;
        widgetPosition.height = viewFrame.height;
    }

    return widgetPosition;
}

bool WidgetManager::GetWidgetList(NKUIStateID state, std::list<WidgetID> &list)
{
    return mLayoutConfig->GetWidgetList(state, list);
}

bool WidgetManager::IsLandscape() const
{
    return mIsLandscape;
}
