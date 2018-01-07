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
@file         NKUIController.cpp
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

#include <algorithm>
#include <functional>
#include <vector>
#include "NKUIController.h"
#include "WidgetID.h"
#include "NKUIEventListener.h"
#include "NKUILocation.h"
#include "RTSState.h"
#include "RequestNavigationState.h"
#include "abpalgps.h"
#include "nbcontextaccess.h"
#include "palfile.h"
#include "nkui_macros.h"
#include "PresenterManager.h"
#include "NKUIContext.h"
#include "nbcontextprotected.h"
#include "NKUIDebug.h"
#include "NavApiNavUtils.h"
#include "analyticsmanager.h"
#include "NBProtocolAnalyticsEvent.h"
extern "C"
{
#include "palclock.h"
}

using namespace std;
using namespace nkui;
using namespace nbcommon;
using namespace protocol;
using namespace nbnav;

// Local Functions.
static inline NKUIPublicState InternalStateToPublicState(NKUIStateID state)
{
    NKUIPublicState publicState = NPS_Invalid;
    switch (state)
    {
        case NAVSTATE_DETOUR:
        case NAVSTATE_DETOUR_LIST:
        {
            publicState =  NPS_Detour;
            break;
        }
        case NAVSTATE_ROUTE_SELECTION_LIST:
        case NAVSTATE_ROUTE_SELECTION:
        {
            publicState = NPS_Route_Selection;
            break;
        }
        case NAVSTATE_NAVIGATION:
        case NAVSTATE_NAVIGATION_SAR:
        case NAVSTATE_NAVIGATION_LIST:
        case NAVSTATE_PEDESTRIAN:
        case NAVSTATE_PEDESTRIAN_UNLOCK:
        case NAVSTATE_PEDESTRIAN_LIST:
        case NAVSTATE_NAVIGATION_ENDTRIP:
        {
            publicState = NPS_Navigation;
            break;
        }
        case NAVSTATE_TRIP_OVERVIEW:
        case NAVSTATE_PEDESTRIAN_OVERVIEW:
        {
            publicState = NPS_Route_Overview;
            break;
        }
        case NAVSTATE_REQUEST_NAVIGATION:
        {
            publicState = NPS_Request_Navigation;
            break;
        }
        case NAVSTATE_STARING_NAVIGATION:
        {
            publicState = NPS_Starting_Navigation;
            break;
        }
        case NAVSTATE_ARRIVAL:
        {
            publicState = NPS_Arrival;
            break;
        }
        default:
        {
            publicState = NPS_Invalid;
            break;
        }
    }
    return publicState;
}

static const char* ANALYTICES_NAV_EVENT_CATEGORY= "2";
static const char* DEFAULT_STRING               = "default";
static const char* USER_ACTION_EVENT_NAME       = "user-action-event";

static void AddUserActionEvent(NB_Context* context, const char* name)
{
    AnalyticsManager* manager = AnalyticsManager::GetAnalyticsManager(context);
    if (!manager)
    {
        return;
    }
    InvocationContextSharedPtr invocation(new protocol::InvocationContext());
    if (!invocation)
    {
        return;
    }
    invocation->SetSourceModule(shared_ptr<string>(new string(DEFAULT_STRING)));
    invocation->SetScreenId(shared_ptr<string>(new string(DEFAULT_STRING)));
    invocation->SetInputSource(shared_ptr<string>(new string(DEFAULT_STRING)));
    invocation->SetInvocationMethod(shared_ptr<string>(new string(DEFAULT_STRING)));

    AnalyticsEventMetadataSharedPtr metadata(new AnalyticsEventMetadata());
    if (!metadata)
    {
        return;
    }
    metadata->SetCategory(shared_ptr<std::string>(new string(ANALYTICES_NAV_EVENT_CATEGORY)));
    metadata->SetName(shared_ptr<std::string>(new string(USER_ACTION_EVENT_NAME)));
    metadata->SetTimestamp(PAL_ClockGetTimeMs());

    UserActionEventSharedPtr userActionEvent((new UserActionEvent()));
    if (!userActionEvent)
    {
        return;
    }
    userActionEvent->SetActionName(shared_ptr<string>(new string(name)));

    AnalyticsEventSharedPtr analyticsEvent(new AnalyticsEvent());
    if (!analyticsEvent)
    {
        return;
    }
    analyticsEvent->SetInvocationContext(invocation);
    analyticsEvent->SetEventMetadata(metadata);
    analyticsEvent->SetUserActionEvent(userActionEvent);

    manager->AddEvent(analyticsEvent);
}

/*! implement NKUIController */


/*! implement NKUIController */
NKUIController::NKUIController(NKUIWidgetManagerPtr widgetManager,
                               NavPlacePtr          destination,
                               NavPlacePtr          origin,
                               NKUIPreferencesPtr   nkuiPreferences,
                               NB_Context*          context,
                               NKUIMapInterface*    mapInterface,
                               NKUIStringHelperPtr  stringHelper)
    : m_pNavigation(NULL),
      m_context(context),
      m_navUIContext(NULL),
      m_pWidgetManager(widgetManager),
      m_mapInterface(mapInterface),
      m_lastGpsLocation(0,0,0,0,0,0,false,0),
      m_pOrigin(origin),
      m_inCalling(false),
      m_pPresenterManager(NKUI_NEW NKUIPresenterManager(stringHelper)),
      m_activeRecalc(false),
      m_needsRecalc(false),
      m_logger(NULL),
      m_planningTrip(nkuiPreferences ? nkuiPreferences->m_isPlanTrip : false)
{
    if (!nkuiPreferences || !mapInterface || !context || !stringHelper)
    {
        abort();
    }

    m_navUIContext = NKUIContext::GetInstance();
    if(destination)
    {
        m_navUIContext->SetDestination(*destination, stringHelper);
    }
    else
    {
        NotifyError(NUE_BadDestination);
    }
    Initialize(nkuiPreferences);
}

//@todo: Unify types of parameters into either reference or shared_ptr....
NKUIController::NKUIController(NKUIWidgetManagerPtr           widgetManager,
                               shared_ptr<const nbnav::Place> destination,
                               shared_ptr<const nbnav::Place> origin,
                               const nbnav::RouteOptions&     routeOptions,
                               const nbnav::Preferences&      preferences,
                               NB_Context*                    context,
                               NKUIMapInterface*              mapInterface,
                               NKUIStringHelperPtr            stringHelper,
                               bool                           enableTurnByTurnNav,
                               bool                           isPlanTrip)
    : m_pWidgetManager(widgetManager),
      m_pNavigation(NULL),
      m_context(context),
      m_navUIContext(NULL),
      m_mapInterface(mapInterface),
      m_lastGpsLocation(0,0,0,0,0,0,false,0),
      m_pOrigin(origin),
      m_inCalling(false),
      m_needsRecalc(false),
      m_logger(NULL),
      m_pPresenterManager(NKUI_NEW NKUIPresenterManager(stringHelper)),
      m_activeRecalc(false),
      m_planningTrip(isPlanTrip)
{
    m_navUIContext = NKUIContext::GetInstance();
    if(destination)
    {
        m_navUIContext->SetDestination(*destination, stringHelper);
    }
    else
    {
        NotifyError(NUE_BadDestination);
    }

    NKUIPreferencesPtr nkuiPreferences(NKUI_NEW NKUIPreferences);
    NKUI_CHECK_SHARED_PTR(nkuiPreferences);

    nkuiPreferences->m_routeOptions         = routeOptions;
    nkuiPreferences->m_navPreferences       = preferences;
    nkuiPreferences->m_isPlanTrip           = isPlanTrip;
    nkuiPreferences->m_turnByTurnNavEnabled = enableTurnByTurnNav;

    Initialize(nkuiPreferences);
}

NKUIController::~NKUIController()
{
    ReleaseNavigation();

    // Clear all widgets and presenters before releasing NKUIContext, since some
    // member fields stored in NKUIContext might be needed during destroying
    // widgets/presenters.
    m_widgets.clear();
    m_presenters.clear();

    m_navUIContext->Release();

    set<NKUIState*>::iterator iter = m_stateList.begin();
    set<NKUIState*>::iterator end  = m_stateList.end();
    for (; iter != end; ++iter)
    {
        delete (*iter);
    }
}

void NKUIController::AddNKUIListener(NKUIListener* listener)
{
    if (listener)
    {
        m_uiListeners.insert(listener);
    }
}

void NKUIController::RemoveNKUIListener(NKUIListener* listener)
{
    if (listener)
    {
        m_uiListeners.erase(listener);
    }
}

void NKUIController::Start()
{
    if (m_pWidgetManager->InitLayoutConfigs())
    {
        InitializeNavigation();
        m_pCurrentState = new RequestNavigationState(m_navUIContext,NULL);
        m_navUIContext->SetPublicState(InternalStateToPublicState(m_pCurrentState->GetStateID()));
        m_stateList.insert(m_pCurrentState);
        StartState();

        // If it is not plan trip case, m_pOrigin is skipped and always use location from
        // locationProvider...
        if (m_pOrigin && m_planningTrip)
        {
            m_pNavigation->PlanTrip(*m_pOrigin);
        }
    }
    else
    {
        NotifyError(NUE_ParseLayoutConfigFileError);
    }
}

void NKUIController::Pause()
{
    if (m_pNavigation)
    {
        m_pNavigation->PauseSession();
        NKUI_DEBUG_LOG(m_logger, "Navigation Paused.\n");
    }
}

void NKUIController::Resume()
{
    if (m_pNavigation)
    {
        m_pNavigation->ResumeSession();
        NKUI_DEBUG_LOG(m_logger, "Navigation Resumed.\n");
    }
}

void NKUIController::Retry()
{
    //when at request navigation state, if have network error, NKUIController will delete old navigaiton and
    //restart it
    if(m_pCurrentState && m_pCurrentState->GetStateID() == NAVSTATE_REQUEST_NAVIGATION)
    {
        ReleaseNavigation();
        Start();
    }
    else
    {
        m_activeRecalc ? Recalculate((nbnav::RouteOptions*)&m_navUIContext->GetRouteOptions())
            : m_pNavigation->Recalculate();
    }
}

void NKUIController::Delete()
{
    if (m_pNavigation)
    {
        for (std::map<WidgetID, shared_ptr<NKUIPresenter> >::iterator it = m_presenters.begin(); it != m_presenters.end(); it++)
        {
            (it->second)->Deactivate();
        }
        m_pNavigation->StopSession();
    }
    ReleaseNavigation();
}

void NKUIController::RegisterEvent(NKEventID event, NKUIEventListener* listener)
{
    if (!listener)
    {
        return;
    }

    //@todo: this code will be changed later
    uiEventMapData::iterator it = m_uiEventListeners.find(event);
    if (it != m_uiEventListeners.end())
    {
        set<NKUIEventListener*> *listeners = &it->second;
        listeners->insert(listener);
    }
    else
    {
        set<NKUIEventListener*> listeners;
        listeners.insert(listener);
        m_uiEventListeners.insert(uiEventMapData::value_type(event, listeners));
    }
}

void NKUIController::SendEvent(NKEventID event, NKUIEventData data)
{
    if(!m_pCurrentState)
    {
        return;
    }
    NKUIStateID oldState = m_pCurrentState->GetStateID();
    if(event == EVT_START_BUTTON_PRESSED &&
       (m_pCurrentState->GetStateID() == NAVSTATE_ROUTE_SELECTION ||
        m_pCurrentState->GetStateID() == NAVSTATE_ROUTE_SELECTION_LIST))
    {
        m_navUIContext->StartNavigationTrip();
    }
    DoStateTransition(event);

    switch (event)
    {
        case EVT_START_BUTTON_PRESSED:
        {
            if (m_pNavigation)
            {
                m_navUIContext->ClearUnselectedRoutes();
                m_pNavigation->SetActiveRoute(m_navUIContext->GetChosenRoute());
                m_planningTrip = false;
                NotifyTurnByTurnNavigationStart();
                SendEvent(EVT_ROUTE_UPDATE, data);
            }
            break;
        }
        case EVT_ROUTE_OPTIONS:
        {
            NotifyChangeRouteOptions();
            break;
        }
        case EVT_END_TRIP:
        {
            NotifyNavigationEnd();
            break;
        }
        case EVT_CANCEL_RTS:
        {
            NotifyNavigationCancel();
            break;
        }
        case EVT_MENU_PRESSED:
        {
            data.eventData = m_pCurrentState;
            break;
        }
        case EVT_MENU_SELECTED:
        {
            // REQUIREMENT: should endtrip when any menu is pressed in arrival state.
            if (m_pCurrentState->GetStateID() == NAVSTATE_ARRIVAL && m_pNavigation)
            {
                NKUIEventData data = {0};
                SendEvent(EVT_END_TRIP, data);
            }
            MenuCommand menuEvent = *(static_cast<MenuCommand*>(data.eventData));
            DoMenuCommond(menuEvent);

            break;
        }
        case EVT_BACK_PRESSED:
        {
            if(m_navUIContext->IsGettingDetour())
            {
                m_navUIContext->SetGettingDetour(false);
                m_pNavigation->CancelDetour();
            }
            break;
        }
        case EVT_ERROR_OCCURRED:
        {
            break;
        }
        case EVT_SAR_HIDE:
        {
            m_navUIContext->RoadSignShown(false);
            break;
        }
        case EVT_TAP_MINIMAP:
        {
            if (m_pCurrentState->GetStateID() == NAVSTATE_TRIP_OVERVIEW)
            {
                AddUserActionEvent(m_context, "nav-trip-overview");
            }
            break;
        }
        case EVT_LIST_OPENED:
        {
            if (m_pCurrentState->GetStateID() == NAVSTATE_NAVIGATION_LIST)
            {
                AddUserActionEvent(m_context, "nav-turn-list");
            }
            break;
        }
        case EVT_TRAFFIC_TRY_T0_SHOW_DETOUR:
        {
            DoMenuCommond(CMD_DETOUR);
            m_navUIContext->SetTryDoDetour(true);
            break;
        }
        default:
            break;
    }
    uiEventMapData::iterator it = m_uiEventListeners.find(event);
    if (it != m_uiEventListeners.end())
    {
        set<NKUIEventListener*> *listeners = &it->second;
        for (set<NKUIEventListener*>::iterator item = listeners->begin();
             item != listeners->end(); item++)
        {
            if (*item != data.source)
            {
                (*item)->NotifyEvent(event, data);
            }
        }
    }

    if(m_pCurrentState && oldState != m_pCurrentState->GetStateID())
    {
        OnStateTransition(oldState, m_pCurrentState->GetStateID());
    }
}

void NKUIController::UpdateGpsLocation(const NKUILocation& location)
{
    m_lastGpsLocation = location;
    if (m_pCurrentState)
    {
        m_navUIContext->SetCurrentLocation(m_lastGpsLocation);

        uint32 feedType = m_pCurrentState->GetLocationFeedType();
        if (feedType & NKUIState::LFT_Nav)
        {
            if (!m_planningTrip)
            {
                m_pNavigation->UpdatePosition(location);
            }
            else if (!m_pOrigin)
            {
                // Don't feed gps location to Navkit for PlanTrip once first gps location is
                // send to server, this can avoid CCC use current location to recalc.
                nbnav::MapLocation mapLocation;
                mapLocation.center.latitude  = location.Latitude();
                mapLocation.center.longitude = location.Longitude();
                mapLocation.center.accuracy  = location.HorizontalUncertaintyAngleOfAxis();
                m_pOrigin.reset(new nbnav::Place(mapLocation));
                m_pNavigation->PlanTrip(*m_pOrigin);
            }
        }

        if (feedType & NKUIState::LFT_Map)
        {
            if (m_pCurrentState->GetStateID() == NAVSTATE_ROUTE_SELECTION &&
                m_navUIContext->IsEnhancedNavigationStartup())
            {
                // REQUIREMENT:
                // Don't update avatar position if we are in RTS view and it is in ENS mode.

                return;
            }

            NKUIEventData data = {0};
            SendEvent(EVT_CURRENTLOCATION_UPDATED, data);
        }
    }
}

void NKUIController::Recalculate(nbnav::RouteOptions* pRouteOptions)
{
    if(!m_pCurrentState)
    {
        return;
    }

    if(m_pNavigation && m_pCurrentState)
    {
        m_navUIContext->SetRecalcingRoute(true);
        if (pRouteOptions) // if new route options are available, use new options to get route.
        {

            m_navUIContext->SetRouteOption(*pRouteOptions);
            m_activeRecalc = true;

            if(m_pCurrentState->WantAlternativeRoute() &&
               m_navUIContext->GetPreferences().GetMultipleRoutesEnabled() &&
               m_navUIContext->GetPublicState() == NPS_Route_Selection)
            {
                m_pNavigation->Recalculate(*pRouteOptions,
                                           m_navUIContext->GetPreferences(), true);
            }
            else
            {
                m_pNavigation->Recalculate(*pRouteOptions,
                                           m_navUIContext->GetPreferences(), false);
            }
        }
        else // if new route options are not available, just recalc...
        {
            m_pNavigation->Recalculate();
        }
    }
}

const NKUISharedInformation& NKUIController::GetCurrentNavInformation()
{
    return m_navUIContext->GetNavSharedInformation();
}

void NKUIController::ShowOverflowMenu(std::vector<MenuOption>& menuItems, bool show)
{
    if(m_pCurrentState)
    {

        NKUIPublicState publicSate = m_navUIContext->GetPublicState();

        set<NKUIListener*>::const_iterator iter = m_uiListeners.begin();
        set<NKUIListener*>::const_iterator end  = m_uiListeners.end();
        for (; iter != end; ++iter)
        {
            (*iter)->ShowOverflowMenu(publicSate, menuItems, show);
        }
    }
}

//@todo: This function is being call frequently, need to be improved.
void NKUIController::DoStateTransition(NKEventID event)
{
    if(!m_pCurrentState)
    {
        return;
    }
    NKUIState* oldState = m_pCurrentState;
    m_pCurrentState = m_pCurrentState->DoTransition(event);
    m_stateList.insert(m_pCurrentState);
    //@todo set Map
    if(m_pCurrentState == NULL)
    {
        //end trip
        for (std::map<WidgetID, shared_ptr<NKUIPresenter> >::iterator it = m_presenters.begin(); it != m_presenters.end(); it++)
        {
            (it->second)->Deactivate();
        }
        m_pNavigation->StopSession();
    }
    else
    {
        if (m_pCurrentState != oldState)
        {
            // update current state to UIContext.
            NKUIPublicState state = InternalStateToPublicState(m_pCurrentState->GetStateID());
            if(m_navUIContext->IsEnhancedNavigationStartup() && state != NPS_Route_Selection)
            {
                state = NPS_Starting_Navigation;
            }
            m_navUIContext->SetPublicState(state);
            StartState();
            NotifyNavigationStateChanged(state);
        }

        // update camera setting
        MapSetting newMapSetting = m_pCurrentState->GetMapSetting();
        const MapSetting& oldMapSetting = m_navUIContext->GetMapSetting();
        if (newMapSetting.avatarMode == NAM_Remain)
        {
            newMapSetting.avatarMode = oldMapSetting.avatarMode;
        }
        if (newMapSetting.polylineType == NNM_Remain)
        {
            newMapSetting.polylineType = oldMapSetting.polylineType;
        }
        if (newMapSetting.cameraMode == NCM_Remain)
        {
            newMapSetting.cameraMode = oldMapSetting.cameraMode;
        }
        if (newMapSetting.trackingMode == NCTM_Remain)
        {
            newMapSetting.trackingMode = oldMapSetting.trackingMode;
        }
        m_navUIContext->SetMapSetting(newMapSetting);
        if(m_pCurrentState != oldState)
        {
            NKUIEventData data = {0};
            SendEvent(EVT_MAPSETTING_UPDATED, data);
        }
    }
}

void NKUIController::StartState()
{
    if(!m_pCurrentState)
    {
        return;
    }
    NKUIState* pStateToStart = m_pCurrentState;
    list<WidgetID> list = pStateToStart->GetActiveWidget();
    std::set<Widget*> widgets;

    //hide all widget not in active widget list
    for (PRESENTER_MAP::iterator it = m_presenters.begin(); it != m_presenters.end(); it++)
    {
        // Sometimes widget would send message on deactivation.
        // Check state pointer to make sure not to perform the following operations after
        // the state has changed
        if ( pStateToStart != m_pCurrentState )
        {
            return;
        }
        WidgetID widgetid = it->first;
        if(find(list.begin(), list.end(), widgetid) == list.end())
        {
            (it->second)->Deactivate();
        }
    }

    NKUIState* oldState = m_pCurrentState;
    //active/create widget in widget list
    for (std::list<WidgetID>::iterator it = list.begin(); it != list.end(); it++)
    {
        WidgetID widgetID = *it;
        shared_ptr<NKUIPresenter> pPresenter;
        shared_ptr<Widget> pWidget;
        PRESENTER_MAP::iterator presenterItem = m_presenters.find(widgetID);
        if (presenterItem != m_presenters.end())
        {
            pPresenter = presenterItem->second;
        }
        else
        {
            pPresenter = m_pPresenterManager->GetPresenter(widgetID);
            if(pPresenter == NULL)
            {
                continue;
            }
            pPresenter->SetNKUIController(this);
            m_presenters.insert(PRESENTER_MAP::value_type(widgetID,pPresenter));
        }

        WIDGET_MAP::iterator widgetItem = m_widgets.find(widgetID);
        if (widgetItem != m_widgets.end())
        {
            pWidget = widgetItem->second;
        }
        else
        {
            pWidget = m_pWidgetManager->GetWidget(widgetID);
            if(pWidget)
            {
                m_widgets.insert(WIDGET_MAP::value_type(widgetID,pWidget));
            }
        }
        pPresenter->SetNKUIController(this);
        pPresenter->SetWidget(pWidget.get());
        if (pWidget)
        {
            pWidget->SetPresenter(pPresenter.get());
        }

        pPresenter->Activate(m_navUIContext);

        // @TRICKY:
        // It is possible that state get changed during activating a presenter. If state
        // changed, old state is not valid anymore, break this loop and call startState() again.
        if (oldState != m_pCurrentState)
        {
            StartState();
            return ;
        }

        if(pWidget != NULL)
        {
            widgets.insert(pWidget.get());
        }
    }
    m_pWidgetManager->LayoutWidgets(widgets, m_pCurrentState->GetStateID());
}

bool NKUIController::InitializeNavigation()
{
    /* If navigation has already started, just return false to
       notify that navigation session can't be started again. */
    if (m_pNavigation)
    {
        return false;
    }
    vector<nbnav::WayPoint> vias;
    m_pNavigation = nbnav::Navigation::GetNavigationWithWorkFoler(
        m_context, m_navUIContext->GetDestination(),
        m_navUIContext->GetRouteOptions(),
        m_navUIContext->GetPreferences(),
        m_workPath,
        vias);
    if (m_pNavigation)
    {
        m_navUIContext->SetNavigation(m_pNavigation);
        m_navUIContext->SetWidgetManager(m_pWidgetManager);
        m_pNavigation->AddSessionListener(this);
        m_pNavigation->AddRoutePositionListener(this);
        m_pNavigation->AddTrafficListener(this);
        m_pNavigation->AddEnhancedNavigationStartupListener(this);
    }

    std::string strRouteAvoid  = "";
    std::string strTransportationMode = "";
    unsigned int routeAvoid = m_navUIContext->GetRouteOptions().GetAvoidFeatures();

#define CHECK_AVOID(X)    if (routeAvoid & nbnav::X)  { strRouteAvoid += #X",";}
    CHECK_AVOID(AVD_HOV);
    CHECK_AVOID(AVD_Toll);
    CHECK_AVOID(AVD_Highway);
    CHECK_AVOID(AVD_Ferry);
    CHECK_AVOID(AVD_Unpaved);
#undef CHECK_AVOID

    switch (m_navUIContext->GetRouteOptions().GetTransportationMode())
    {
        case nbnav::Car:
            strTransportationMode = "Car";
            break;
        case nbnav::Truck:
            strTransportationMode = "Truck";
            break;
        case nbnav::Bicycle:
            strTransportationMode = "Bicycle";
            break;
        case nbnav::Pedestrian:
            strTransportationMode = "Pedestrian";
            break;
        default:
            break;
    }
    NKUI_DEBUG_LOG(m_logger, "Starting a Navigation Session with navigation mode: '%s', navigation avoids: '%s'.\n",
                   strTransportationMode.c_str(), strRouteAvoid.c_str());
    return m_pNavigation != NULL;
}

void NKUIController::ReleaseNavigation()
{
    if (m_pNavigation)
    {
        m_pNavigation->RemoveRoutePositionListener(this);
        m_pNavigation->RemoveSessionListener(this);
        m_pNavigation->RemoveTrafficListener(this);
        delete m_pNavigation;
        m_pNavigation = NULL;
        m_navUIContext->SetNavigation(NULL);
    }
}

void NKUIController::OnStateTransition(NKUIStateID from, NKUIStateID to)
{
    NKUIEventData data = {0};
    if ((to <= NAVSTATE_PEDESTRIAN_OVERVIEW && to >= NAVSTATE_NAVIGATION) &&
        (from == NAVSTATE_DETOUR || from == NAVSTATE_DETOUR_LIST))
    {
        if(m_navUIContext->ClearDetourRoute())
        {
            SendEvent(EVT_ROUTE_UPDATE, data);
        }
    }

    SendEvent(EVT_STATE_CHANGED, data);
    SendEvent(EVT_MENU_HIDE, data);
}

void NKUIController::DoMenuCommond(MenuCommand command)
{
    NKUIEventData data = {0};
    switch (command)
    {
        case CMD_DETOUR:
        {
            SendEvent(EVT_GETTING_DETOUR, data);
            m_navUIContext->SetGettingDetour(true);
            m_pNavigation->DoDetour();
            NKUI_DEBUG_LOG(m_logger,"'Detour' in the Overflow Menu selected.\n");
            break;
        }
        case CMD_END_TRIP:
        {
            SendEvent(EVT_END_TRIP_PRESSED, data);
            NKUI_DEBUG_LOG(m_logger,"'End Trip' in the Overflow Menu selected.\n");
            break;
        }
        case CMD_RECALCULATE:
        {
            m_pNavigation->Recalculate();
            NKUI_DEBUG_LOG(m_logger,"'Recalculate' in the Overflow Menu selected.\n");
            break;
        }
        case CMD_ROUTE_OPTION:
        {
            NotifyChangeRouteOptions();
            NKUI_DEBUG_LOG(m_logger,"'Route Options' in the Overflow Menu selected.\n");
            break;
        }
        default:
            break;
    }
}

void NKUIController::NotifyNavigationStart()
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(), mem_fun(&NKUIListener::OnNavigationStart));
}

void NKUIController::NotifyChangeRouteOptions()
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(), mem_fun(&NKUIListener::OnRouteOptionsChanged));
}

void NKUIController::NotifyNavigationEnd()
{
    //@bug: if user set this NKUI instance to nil in this callback, NKUI Instance will be
    //      destroyed, which will cause NKUIController being destroyed while for_each is
    //      not ended....
    //      Need to fix this!
    for_each(m_uiListeners.begin(), m_uiListeners.end(),
             mem_fun(&NKUIListener::OnNavigationEnd));
}

void NKUIController::NotifyNavigationCancel()
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(), mem_fun(&NKUIListener::OnNavigationCancel));
}

void NKUIController::NotifyConfirmAction(NavigationUIAction action)
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(), bind2nd(mem_fun(&NKUIListener::OnConfirmAction), action));
}

void NKUIController::NotifyError(NKUI_Error errorCode)
{
    bool errorProcessed = false;
    set<NKUIListener*>::const_iterator iter = m_uiListeners.begin();
    set<NKUIListener*>::const_iterator end  = m_uiListeners.end();
    for (; iter != end; ++iter)
    {
        if (*iter)
        {
            errorProcessed |= (*iter)->OnError(errorCode);
        }
    }

    // if error is not handled in listener, errorProcessed should be false, NavkitUI needs
    // to handle this.
    if (!errorProcessed)
    {
        NKUIEventData data = {&errorCode, NULL};
        SendEvent(EVT_ERROR_OCCURRED, data);
    }
}

void NKUIController::OffRoute()
{
    m_navUIContext->SetOnRoute(false);
    NKUIEventData data = {0};
    SendEvent(EVT_NAV_OFFROUTE, data);
}

void NKUIController::OnRoute()
{
    m_navUIContext->SetOnRoute(true);
    m_navUIContext->SetStartupOffroute(false);

    m_needsRecalc      = false; // reset this flag to avoid recalc it if onRoute after calling ends.
    NKUIEventData data = {0};
    SendEvent(EVT_NAV_ONROUTE, data);
}

bool NKUIController::HaveFasterRoute(vector<nbnav::RouteInformation>& routes, vector<nbnav::RouteInformation>& faster)
{
    //TODO change to false
    bool ret = true;
    const nbnav::RouteInformation& currentRoute = m_navUIContext->GetCurrentRoute();
    for(vector<nbnav::RouteInformation>::iterator it = routes.begin(); it != routes.end(); it++)
    {
        if(it->GetTime() < currentRoute.GetTime())
        {
            faster.push_back(*it);
            ret = true;
            break;
        }
    }
    return ret;
}

void NKUIController::RouteReceived(SessionListener::RouteRequestReason reason,
                                   vector<nbnav::RouteInformation>& routes)
{
    if(!m_pCurrentState)
    {
        return;
    }
    m_activeRecalc = false;
    if(!((SessionListener::Recalculate == reason) &&
         (m_navUIContext->GetPublicState() == NPS_Detour)))
    {
        m_navUIContext->SetChosenRouteIndex(0);
    }
    NKUIEventData data = {0};
    data.eventData = &reason;
    switch (reason)
    {
        case SessionListener::Calculate:
        {
            m_navUIContext->SetAllRoutes(routes);
            m_navUIContext->ResetRetryCount();
            if (!routes.empty())
            {
                m_navUIContext->SetOrigin(routes[0].GetOrigin());
            }
            SendEvent(EVT_ROUTE_UPDATE, data);
            if (m_navUIContext->GetPreferences().GetMultipleRoutesEnabled())
            {
                SendEvent(EVT_ENT_RTS, data);
                NKUI_DEBUG_LOG(m_logger, "RTS view displayed.\n");
                if (m_navUIContext->IsTurnByTurnNavigationEnabled())
                {
                    // Announcement 'press start to navigate'
                    PreservedAudio audio = PreservedAudio_PressStartToNavigate;
                    NKUIEventData announceEventData = {0};
                    announceEventData.eventData = &audio;
                    SendEvent(EVT_ANNOUNCE, announceEventData);
                }
            }
            else
            {
                // needs to call ClearUnselectedRoutes() to update route indexes in navUIContext.
                m_navUIContext->ClearUnselectedRoutes();
                SendEvent(EVT_START_NAV, data);
            }
            NotifyNavigationStart();
            break;
        }
        case SessionListener::Recalculate:
        {
            m_navUIContext->SetRecalcingRoute(false);
            m_navUIContext->ResetRetryCount();
            if(!routes.empty())
            {
                m_navUIContext->SetOrigin(routes[0].GetOrigin());
                if(m_navUIContext->GetPublicState() == NPS_Route_Selection)
                {
                    m_navUIContext->SetAllRoutes(routes);
                }
                else
                {
                    m_navUIContext->UpdateCurrentRoute(routes[0]);
                }
            }

            SendEvent(EVT_ROUTE_UPDATE, data);
            SendEvent(EVT_RECALCING_END, data);

            // If we are in TripOverView or RTS View, zoom to show the entire route.
            if (m_navUIContext->GetPublicState() == NPS_Route_Selection ||
                m_navUIContext->GetPublicState() == NPS_Route_Overview)
            {
                SendEvent(EVT_ZOOM_FIT_ROUTE, data);
            }

            break;
        }
        case SessionListener::RouteSelector:
        {
            m_navUIContext->ResetRetryCount();
            m_navUIContext->SetAllRoutes(routes);
            SendEvent(EVT_ROUTE_UPDATE, data);
            SendEvent(EVT_ENT_RTS, data);
            NKUI_DEBUG_LOG(m_logger, "RTS view displayed.\n");
            if (m_navUIContext->IsTurnByTurnNavigationEnabled())
            {
                // Announcement 'press start to navigate'
                PreservedAudio audio = PreservedAudio_PressStartToNavigate;
                NKUIEventData announceEventData = {0};
                announceEventData.eventData = &audio;
                SendEvent(EVT_ANNOUNCE, announceEventData);
            }
            NotifyNavigationStart();
            break;
        }
        case SessionListener::Detour:
        {
            if(m_navUIContext->IsTryDoDetour())
            {
                vector<nbnav::RouteInformation> selectedRoute;
                if(HaveFasterRoute(routes, selectedRoute))
                {
                    if(!selectedRoute.empty())
                    {
                        m_navUIContext->SetOrigin(selectedRoute[0].GetOrigin());
                        if(m_navUIContext->GetPublicState() == NPS_Route_Selection)
                        {
                            m_navUIContext->SetAllRoutes(selectedRoute);
                        }
                        else
                        {
                            m_navUIContext->UpdateCurrentRoute(selectedRoute[0]);
                        }
                    }
                    SendEvent(EVT_TRIPOVERVIEW_SHOW, data);
                }
                SendEvent(EVT_ROUTE_UPDATE, data);
                SendEvent(EVT_ENDGETTING_DETOUR, data);
                m_navUIContext->SetGettingDetour(false);
                m_navUIContext->SetTryDoDetour(false);
            }
            else
            {
                m_navUIContext->UpdateDetourRoutes(routes);
                SendEvent(EVT_ROUTE_UPDATE, data);
                SendEvent(EVT_ENDGETTING_DETOUR, data);
                SendEvent(EVT_DETOUR_SUCCESS, data);
                m_navUIContext->SetGettingDetour(false);
                NKUI_DEBUG_LOG(m_logger, "Detour view displayed.\n");
            }
            break;
        }
        default:
            break;
    }
}

void NKUIController::RouteRequested(nbnav::SessionListener::RouteRequestReason reason)
{
    NKUIEventData data = {0};
    DoStateTransition(EVT_INVALID);
    if(reason == nbnav::SessionListener::Recalculate)
    {
        m_navUIContext->SetRecalcingRoute(true);
        SendEvent(m_activeRecalc ? EVT_GETTING_ROUTE : EVT_RECALCING, data);
        std::vector<MenuOption> tempItems;
        tempItems.clear();
        ShowOverflowMenu(tempItems, false);
    }
    SendEvent(EVT_ROUTE_REQUESTED, data);
}

void NKUIController::RouteProgress(int progress)
{
    NKUIEventData data = {0};
    data.eventData = &progress;
    SendEvent(EVT_ROUTE_PROGRESS, data);
}

void NKUIController::RouteError(nbnav::NavigateRouteError error)
{
    NKUIEventData eventData = {0};
    bool retry     = false;
    //ignore cancel error
    if(error == nbnav::NRE_Cancelled)
    {
        return;
    }
    //ignore getting detour error
    if(!m_navUIContext->IsGettingDetour())
    {
        /*! if is in calling state, and the error is netError or timeout, show tripOverView */
        if (m_inCalling &&
            (error == nbnav::NRE_NetError || error == nbnav::NRE_TimedOut))
        {
            //if in RTS state, return error code.
            if(m_pCurrentState->GetStateID() != NAVSTATE_ROUTE_SELECTION)
            {
                m_needsRecalc = true;
                SendEvent(EVT_TRIPOVERVIEW_SHOW, eventData);
                m_navUIContext->SetRecalcingRoute(false);
                return;
            }
        }
        else
        {
            switch (error)
            {
                case nbnav::NRE_TimedOut:
                case nbnav::NRE_NetError:
                case nbnav::NRE_ServerError:
                {
                    retry = m_navUIContext->RetryStart();
                    break;
                }
                default:
                    break;
            }
        }
    }

    //TODO: replace NavigateRouteError to LTKException
    if (!retry)
    {
        m_activeRecalc = false;
        NotifyError((NKUI_Error)error);
    }

    bool isDetourRequest = m_navUIContext->IsGettingDetour();
    if (isDetourRequest)
    {
        SendEvent(EVT_ENDGETTING_DETOUR, eventData);
    }

    m_navUIContext->SetGettingDetour(false);
    m_navUIContext->SetRecalcingRoute(false);

    if (retry)
    {
        SendEvent(EVT_NAV_RETRY, eventData);
    }
    else
    {
        if (error != nbnav::NRE_NoDetour && !isDetourRequest)
        {
            SendEvent(EVT_END_TRIP, eventData);
        }
    }
}

void NKUIController::RouteFinish()
{
    NKUIStateID state = NAVSTATE_INVALID;
    if(m_pCurrentState)
    {
        state = m_pCurrentState->GetStateID();
    }
    NKUIEventData data = {0};
    SendEvent(EVT_ARRIVAL, data);
    //when other state change to arrival state send OnArrived message, other case not send
    if(m_pCurrentState && m_pCurrentState->GetStateID() != state && m_pCurrentState->GetStateID() == NAVSTATE_ARRIVAL)
    {
        // Notify listeners about this state.
        for_each(m_uiListeners.begin(), m_uiListeners.end(),
                 mem_fun(&NKUIListener::OnArrived));
    }
}

void NKUIController::OffRoutePositionUpdate(double headingToRoute)
{
    if(!m_navUIContext->IsStartupOffroute() && !m_navUIContext->IsOnRoute())
    {
        m_navUIContext->SetStartupOffroute(true);
        NKUIEventData data = {0};
        SendEvent(EVT_ENT_STARTOFFROUTE, data);
    }
}

void NKUIController::RoadSign(const nbnav::RoadSign& signInfo)
{
    m_navUIContext->RoadSignShown(true);
    NKUIEventData data = {0};
    SendEvent(EVT_SAR_SHOW, data);
}

void NKUIController::DisableRoadSign()
{
    m_navUIContext->RoadSignShown(false);
    NKUIEventData data = {0};
    SendEvent(EVT_SAR_HIDE, data);
}

void NKUIController::PositionUpdated(const nbnav::Coordinates& coordinates, double speed, double heading)
{
    NKUIEventData data = {0};
    if(!m_navUIContext->IsOnRoute() || m_navUIContext->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
    {
        m_navUIContext->SetCurrentLocation(m_lastGpsLocation);
    }
    else
    {
        NKUILocation location(coordinates.latitude, coordinates.longitude, heading, m_lastGpsLocation.GpsTime(),
                              m_lastGpsLocation.HorizontalVelocity(), m_lastGpsLocation.HorizontalUncertaintyAlongAxis(),
                              m_lastGpsLocation.IsGpsFix(), m_lastGpsLocation.Valid());
        m_navUIContext->SetCurrentLocation(location);
    }
    SendEvent(EVT_CURRENTLOCATION_UPDATED, data);
}

void NKUIController::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    m_navUIContext->SetCurrentRoadName(primaryName);
}

void NKUIController::TripRemainingTime(int time)
{
    m_navUIContext->SetTripRemainingTime(time);
}

void NKUIController::TrafficAlerted(const nbnav::TrafficEvent& trafficEvent)
{
    m_navUIContext->SetInTrafficArea(true);
}

void NKUIController::DisableTrafficAlerted()
{
    m_navUIContext->SetInTrafficArea(false);
}

void NKUIController::EnterEnhancedNavigationStartup(nbnav::Coordinates coordinate)
{
    m_navUIContext->SetEnhancedNavigationStartup(true);
    NKUIEventData data = {0};
    SendEvent(EVT_ENTER_ENHANCED_STARTUP, data);
}

void NKUIController::ExitEnhancedNavigationStartup()
{
    m_navUIContext->SetEnhancedNavigationStartup(false);
    NKUIEventData data = {0};
    SendEvent(EVT_EXIT_ENHANCED_STARTUP, data);
}

void NKUIController::SetWorkFolder(const std::string& path)
{
    m_workPath = path;
}

std::string NKUIController::GetWorkFolder() const
{
    return m_workPath;
}

std::string NKUIController::GetLocale() const
{
    const char* locale = NULL;
    if (m_context != NULL)
    {
        locale = NB_ContextGetLocale(m_context);
    }
    if (locale != NULL)
    {
        return locale;
    }
    return std::string();
}

void* NKUIController::GetMapInterface()
{
    return m_mapInterface;
}

/* See description in header file. */
void NKUIController::NotifyTurnByTurnNavigationStart()
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(),
             mem_fun(&NKUIListener::OnTurnByTurnNavigationStart));
}


/* See description in header file. */
void NKUIController::NotifyNavigationStateChanged(NKUIPublicState state)
{
    for_each(m_uiListeners.begin(), m_uiListeners.end(),
             bind2nd(mem_fun(&NKUIListener::OnNavigationStateChanged), state));
}

void NKUIController::SetBackground(bool isBackground)
{
    NKUIEventData data = {0};
    if(isBackground)
    {
        SendEvent(EVT_ENTER_BACKGROUND, data);
    }
    else
    {
        SendEvent(EVT_ENTER_FOREGROUND, data);
    }
}

void NKUIController::CancelEndTripTimer()
{
    NKUIEventData data = {0};
    SendEvent(EVT_STILL_DRIVING, data);
}

void NKUIController::SetCallingState(bool isInCalling)
{
    if (isInCalling == m_inCalling)
    {
        return;
    }
    m_inCalling = isInCalling;
    NKUIEventData data = {0};
    if (m_inCalling)
    {
        /*! when enter calling state, notify player to stop play announcement */
        SendEvent(EVT_ENTER_CALLING_STATE, data);
    }
    else
    {
        /*! when leave calling state, return to turn by turn navigation */
        SendEvent(EVT_LEAVE_CALLING_STATE, data);
        /*! if calling disconnected, do recalc */
        if (m_needsRecalc && m_pNavigation)
        {
            m_needsRecalc = false;
            m_pNavigation->Recalculate();
        }
    }
}

bool NKUIController::GetCallingState() const
{
    return m_inCalling;
}

const WidgetPosition NKUIController::GetWidgetPosition(WidgetID widgetID) const
{
    WidgetPosition widgetPosition = {0};
    if (m_pWidgetManager && m_pCurrentState)
    {
        widgetPosition = m_pWidgetManager->GetWidgetPosition(widgetID, m_pCurrentState->GetStateID());
    }

    return widgetPosition;
}

nbnav::Logger* NKUIController::GetLogger() const
{
    return m_logger;
}

void NKUIController::SetLogLevel(nbnav::Logger::LoggerLevel level)
{
    if (m_logger)
    {
        m_logger->SetLogLevel(level);
    }
}

bool NKUIController::IsLandscape()
{
    if(m_pWidgetManager)
    {
        return m_pWidgetManager->IsLandscape();
    }
    else
    {
        return false;
    }
}

void NKUIController::Initialize(NKUIPreferencesPtr& nkuiPreference)
{
    m_navUIContext->SetNKUIPreferences(nkuiPreference);
    m_navUIContext->SetPalInstance(NB_ContextGetPal(m_context));
    m_navUIContext->SetMapInterface(m_mapInterface);
    m_pWidgetManager->SetNKUIController(this);

    // Construct path for logger
    const char* path = PAL_FileGetCachePath();
    char fullPath[PAL_FILE_MAX_FULL_PATH_LENGTH] = {'\0'};
    if (path && sprintf(fullPath, "%s", path) &&
        PAL_FileAppendPath(NB_ContextGetPal(m_context), fullPath,
                           PAL_FILE_MAX_NAME_LENGTH, "NavUIkit.log") == PAL_Ok)
    {
        path = fullPath;
        NB_ContextAddCachePath(m_context, fullPath);
    }
    else
    {
        path = NULL;
    }
    m_logger = nbnav::Logger::GetInstance(m_context, path, nbnav::Logger::LL_Debug);
}

// Implementation of NKUIPreference
NKUIPreferences::NKUIPreferences()
    : m_isPlanTrip(false),
      m_turnByTurnNavEnabled(true),
      m_viewMode(NVM_Normal),
      m_routeOptions(nbnav::Fastest, nbnav::Car, 0)
{
    //@todo: provide a better default routeOPtions and preferences.
}

NKUIPreferences::~NKUIPreferences()
{
}
/*! @} */

