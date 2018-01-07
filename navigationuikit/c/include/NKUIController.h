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
@file         NKUIController.h
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

#ifndef __NAVIGATIONUIKIT__NKUICONTROLLERIMPL_H__
#define __NAVIGATIONUIKIT__NKUICONTROLLERIMPL_H__

#include <map>
#include <vector>
#include <set>
#include <list>
#include "NKUILocation.h"
#include "NKUIListener.h"
#include "NKUIEventListener.h"
#include "NKUIWidgetManager.h"
#include "NKUIMapInterface.h"
#include "NKUISharedInformation.h"
#include "NKUIMenuCommand.h"
#include "NKUIStringHelper.h"
#include "NKUIPresenter.h"
#include "NavApiNavigation.h"
#include "NavApiRouteRequest.h"
#include "Logger.h"

namespace nkui
{
#define  NKUI_WRITE_LOG(X, L, ...)                           \
    do                                                      \
    {                                                       \
        if (X)                                              \
        {                                                   \
            (X)->WriteLog((L), "NavUIKit",  __VA_ARGS__);   \
        }                                                   \
    } while (0)

#define NKUI_DEBUG_LOG(X, ...) NKUI_WRITE_LOG(X, nbnav::Logger::LL_Debug, __VA_ARGS__)
#define NKUI_INFO_LOG(X, ...)  NKUI_WRITE_LOG(X, nbnav::Logger::LL_Info, __VA_ARGS__)
#define NKUI_WARN_LOG(X, ...)  NKUI_WRITE_LOG(X, nbnav::Logger::LL_Warn, __VA_ARGS__)
#define NKUI_FATAL_LOG(X, ...) NKUI_WRITE_LOG(X, nbnav::Logger::LL_Fatal, __VA_ARGS__)

typedef std::map<NKEventID,std::set<NKUIEventListener*> > uiEventMapData;
typedef std::set<NKUIListener*> uiListenerData;

class NKUIPresenterManager;
class NKUIContext;
class NKUIState;
class NKUIPreferences;

typedef shared_ptr<NKUIPreferences>      NKUIPreferencesPtr;
typedef shared_ptr<const nbnav::Place>  NavPlacePtr;

/*! Controller of NavkitUI. */
class NKUIController  : public nbnav::SessionListener,
                        public nbnav::RoutePositionListener,
                        public nbnav::TrafficListener,
                        public nbnav::EnhancedNavigationStartupListener,
                        public nbnav::RouteListener
{
public:

    /**
     * @name NKUIController - Constructor of NKUIController.
     * @param widgetManager - SharedPtr of WidgetManager instance.
     * @param destination -  destination of navigation
     * @param origin -  origin of navigation
     * @param nkuiPreference - Preferences for nkui..
     * @param context -  pointer of NB_Context.
     * @param mapInterface - Mmap Interface used by this session.
     * @param stringHelper -  string Helper to get localized strings.
     */
    NKUIController(NKUIWidgetManagerPtr widgetManager,
                   NavPlacePtr          destination,
                   NavPlacePtr          origin,
                   NKUIPreferencesPtr   nkuiPreferences,
                   NB_Context*          context,
                   NKUIMapInterface*    mapInterface,
                   NKUIStringHelperPtr  stringHelper);

    /**
     * @name NKUIController - Constructor of NKUIController. Deprecated.
     * @param widgetManager - SharedPtr of WidgetManager instance.
     * @param destination -  destination of navigation
     * @param origin -  origin of navigation
     * @param routeOptions -  route Options applied to this session.
     * @param preferences -  preferences applied to this session.
     * @param context -  pointer of NB_Context.
     * @param mapInterface - Mmap Interface used by this session.
     * @param stringHelper -  string Helper to get localized strings.
     * @param enableTurnByTurnNav - Flag to indicate if enable turn by turn or not.
     * @param isPlanTrip - Flag to indicate if it planTrip.
     */
    NKUIController(NKUIWidgetManagerPtr           widgetManager,
                   shared_ptr<const nbnav::Place> destination,
                   shared_ptr<const nbnav::Place> origin,
                   const nbnav::RouteOptions&     routeOptions,
                   const nbnav::Preferences&      preferences,
                   NB_Context*                    context,
                   NKUIMapInterface*              mapInterface,
                   NKUIStringHelperPtr            stringHelper,
                   bool                           enableTurnByTurnNav,
                   bool                           isPlanTrip=false);

    /**
     * @name ~NKUIController - Destructor
     */
    virtual ~NKUIController();

    /**
     * @name AddNKUIListener - Add an NKUIListens
     * @param listener -  listener to be added.
     * @return void
     */
    void AddNKUIListener(NKUIListener* listener);

    /**
     * @name RemoveNKUIListener - Remove a NKUIListens.
     * @param listener -  listener to be removed.
     * @return void
     */
    void RemoveNKUIListener(NKUIListener* listener);

    /**
     * @name Start - Start this navigation session.
     * @return void
     */
    void Start();

    /**
     * @name Pause - Pause this navigation session.
     * @return void
     */
    void Pause();

    /**
     * @name Resume - Resume this navigation session.
     * @return void
     */
    void Resume();

    /**
     * @name Retry - Retry this navigation session.
     * @return void
     */
    void Retry();

    /**
     * @name Delete - Delete this navigation session.
     * @return void
     */
    void Delete();

    /**
     * @name RegisterEvent - Register event handler.
     * @param event -  event to be notified.
     * @param listener -  callback to be invoked when specified event happens.
     * @return void
     */
    void RegisterEvent(NKEventID event, NKUIEventListener* listener);
    void SendEvent(NKEventID event, NKUIEventData data);
    void UpdateGpsLocation(const NKUILocation& location);
    void Recalculate(nbnav::RouteOptions* pRouteOptions);
    const NKUISharedInformation& GetCurrentNavInformation();
    void  ShowOverflowMenu(std::vector<MenuOption>& menuItems, bool show);

    /*! Set work folder, so that the app could load the file from where
     *  the nkui set.
     *
     *  @param path the relative path
     *  @return none
     */
    void SetWorkFolder(const std::string& path);
    /*! Get the relative work folder.
     *
     *  @return: Returns the relative path
     */
    std::string GetWorkFolder() const;

    /*!
     *  Get the locale string
     *
     *  @return: Returns the current locale.
     */
    std::string GetLocale() const;

    /*! please reference SessionListener */
    virtual void OffRoute();
    virtual void OnRoute();
    virtual void RouteReceived(nbnav::SessionListener::RouteRequestReason reason, std::vector<nbnav::RouteInformation>& routes);
    virtual void RouteRequested(nbnav::SessionListener::RouteRequestReason reason);
    virtual void RouteProgress(int progress);
    virtual void RouteError(nbnav::NavigateRouteError error);
    virtual void RouteFinish();
    virtual void OffRoutePositionUpdate(double headingToRoute);

    // override RoutePositionListener
    virtual void RoadSign(const nbnav::RoadSign& signInfo);
    virtual void DisableRoadSign();
    virtual void PositionUpdated(const nbnav::Coordinates& coordinates, double speed, double heading);
    virtual void* GetMapInterface();
    virtual void CurrentRoadName(std::string primaryName, std::string secondaryName);
    virtual void TripRemainingTime(int time);
    //override trafficlistener
    virtual void TrafficAlerted(const nbnav::TrafficEvent& trafficEvent);
    virtual void DisableTrafficAlerted();

    //override EnhancedNavigationStartupListener
    void EnterEnhancedNavigationStartup(nbnav::Coordinates coordinate);

    void ExitEnhancedNavigationStartup();

    /*! methods for idling 5 minutes */
    /*! NavUIKit was sent to background or brought to foreground
     *
     * @param isBackground true means sent to background otherwise brought to foreground
     * @return none.
     */
    void SetBackground(bool isBackground);
    /*! cancel the timer which will end trip
     *
     * @param none.
     * @return none.
     */
    void CancelEndTripTimer();

    /*!
     * set the calling state
     *
     * @param isInCalling true means in calling state, otherwise not in calling state.
     * @return none
     */
    void SetCallingState(bool isInCalling);

    /*!
     * get current calling state
     *
     * @return none
     */
    bool GetCallingState() const;

    /*!
     * get the position of widget by widget id and navigation state
     *
     * @param widgetID  widget id
     * @return the position of the widget
     */
    const WidgetPosition GetWidgetPosition(WidgetID widgetID) const;

    /*!
     * set the calling state
     *
     * @return if screen mode is landscape
     */
    bool IsLandscape();

    /*!
     * get faster route
     *
     * return if have faster route return true
    */
    bool HaveFasterRoute(vector<nbnav::RouteInformation>& routes, vector<nbnav::RouteInformation>& faster);

    /*!
     * get Logger object
     *
     * @return Logger object
     */
    nbnav::Logger* GetLogger() const;

    /*!
     * set Logger level
     *
     * @param level log level
     * @return none
     */
    void SetLogLevel(nbnav::Logger::LoggerLevel level);

private:
    void DoStateTransition(NKEventID event);
    void StartState();
    bool InitializeNavigation();
    void ReleaseNavigation();
    /**
     *  this function is called when state changed
     *
     *  @param from the old state
     *  @param to   the new state
     */
    void OnStateTransition(NKUIStateID from, NKUIStateID to);
    void DoMenuCommond(MenuCommand command);

    /* used to invoke NKUIListeners callback. */
    void NotifyNavigationStart();
    void NotifyChangeRouteOptions();
    void NotifyNavigationEnd();
    void NotifyNavigationCancel();
    void NotifyConfirmAction(NavigationUIAction action);
    void NotifyTurnByTurnNavigationStart();
    void NotifyError(NKUI_Error errorCode);
    void NotifyNavigationStateChanged(NKUIPublicState state);

    void Initialize(NKUIPreferencesPtr& nkuiPreferences);

    uiEventMapData          m_uiEventListeners;
    uiListenerData          m_uiListeners;
    NKUIWidgetManagerPtr    m_pWidgetManager;
    NKUIState*             m_pCurrentState;
    nbnav::Navigation*      m_pNavigation;
    NB_Context*             m_context;
    NKUIContext*            m_navUIContext;
    NKUIMapInterface*       m_mapInterface;
    /*! set the work folder of the fileset of nav */
    std::string             m_workPath;
    NKUILocation            m_lastGpsLocation;
    shared_ptr<const nbnav::Place> m_pOrigin;
    /*! is in calling state or not */
    bool                    m_inCalling;
    /*! uesed for case: offroute and recalc failed during the calling,
        we should do recalc when the call ended. */
    bool                    m_needsRecalc;
    nbnav::Logger*          m_logger;

    shared_ptr<NKUIPresenterManager>     m_pPresenterManager;
    typedef std::map<WidgetID, WidgetPtr>        WIDGET_MAP;
    typedef std::map<WidgetID, NKUIPresenterPtr> PRESENTER_MAP;

    WIDGET_MAP    m_widgets;
    PRESENTER_MAP m_presenters;
    bool          m_activeRecalc;
    bool          m_planningTrip;

    set<NKUIState*> m_stateList; // list of created states.
};

/*! View mode of nkui. */
typedef enum _NKUIViewMode
{
    NVM_Normal = 0, /*! Normal mode for phones.. */
    NVM_Autoview,   /*! Autoview mode for Pad..  */
    NVM_Invalid,
} NKUIViewMode;

class NKUIPreferences
{
public:
    NKUIPreferences();
    ~NKUIPreferences();

    bool         m_isPlanTrip;
    bool         m_turnByTurnNavEnabled;
    NKUIViewMode m_viewMode;

    nbnav::RouteOptions m_routeOptions;
    nbnav::Preferences  m_navPreferences;
};
}

#endif /* defined(__nkui__NKUIController__) */

/*! @} */
