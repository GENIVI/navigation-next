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
 @file         NKUIContext.h
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

#ifndef __NAVIGATIONUIKIT___NAVUICONTEXT_H__
#define __NAVIGATIONUIKIT___NAVUICONTEXT_H__

#include "NavApiNavigation.h"
#include "MapSetting.h"
#include "NKUIMapInterface.h"
#include "NKUILocation.h"
#include "NKUIStringHelper.h"
#include "NKUISharedInformation.h"
#include "NKUIPublicState.h"
#include "NKUIController.h"
#include "NKUIWidgetManager.h"
#include "nkui_macros.h"
#include <vector>

/**
 *  Data container
 *  This class is a singleton data container for navui kit internal using.
 */
namespace nkui
{
class NKUIContext
{
public:
    /**
     *  Get the instance of NKUIContext.
     *
     *  @return NKUIContext instance pointer
     */
    static NKUIContext* GetInstance();

    /** Releas the instance of NKUIContext. */
    void Release();

    /** Public methods for get/set data. */
    nbnav::Navigation* GetNavigation();
    void SetNavigation(nbnav::Navigation* navigation);
    const std::vector<nbnav::RouteInformation>& GetAllRoutes() const;
    void SetAllRoutes(const std::vector<nbnav::RouteInformation>& routes);
    void UpdateCurrentRoute(const nbnav::RouteInformation& route);
    void UpdateDetourRoutes(const std::vector<nbnav::RouteInformation>& routes);
    bool IsSelectingDetour() const;
    const nbnav::RouteInformation& GetChosenRoute() const;
    const nbnav::RouteInformation& GetCurrentRoute() const;
    bool IsChosenCurrentRoute() const;
    bool IsRecalcingRoute() const;
    bool IsGettingDetour() const;
    void SetRecalcingRoute(bool recalcingRoute);
    void SetGettingDetour(bool gettingDetour);
    void SetChosenRouteIndex(int index);
    int GetChosenRouteIndex() const;
    int GetCurrentRouteIndex() const;
    void SetStartupOffroute(bool startOffroute);
    bool IsStartupOffroute() const;
    void SetCurrentLocation(const NKUILocation& currentLocation);
    NKUILocation& GetCurrentLocation();
    const nbnav::Place& GetDestination() const;
    void SetDestination(const nbnav::Place& place,
                        NKUIStringHelperPtr stringHelper);
    const nbnav::Place& GetOrigin() const;
    void SetOrigin(const nbnav::Place& place);
    void SetPalInstance(PAL_Instance* pal);
    void ClearUnselectedRoutes();
    void SetEnhancedNavigationStartup(bool value);
    bool IsEnhancedNavigationStartup() const;
    void SetTryDoDetour(bool tryDetour);
    bool IsTryDoDetour() const;
    /**
     *  clear detour route
     *
     *  @return if route have been clear and no need update route return false.
     */
    bool ClearDetourRoute();
    void SetOnRoute(bool onRoute);
    bool IsOnRoute() const;
    void SetMapSetting(const MapSetting& mapSetting);
    const MapSetting& GetMapSetting() const;
    std::string GetDestinationStreetSideText(NKUIStringHelperPtr& stringHelper);
    std::string GetDestinationStreetText();
    std::string GetDestinationAddress();
    void SetMapInterface(NKUIMapInterface* pMapInterface);
    NKUIMapInterface* GetMapInterface();
    void SetCurrentRoadName(const std::string& roadName);
    void SetTripRemainingTime(int time);
    void SetInTrafficArea(bool inTrafficArea);
    void SetWidgetManager(NKUIWidgetManagerPtr manager);
    NKUIWidgetManagerPtr GetWidgetManager();
    const NKUISharedInformation& GetNavSharedInformation() const;
    void StartNavigationTrip();
    /**
     *  Check conditions whether we can retry or not.
     *
     *  @return: Return false if no more retry
     */
    bool RetryStart();

    /**
     *  Clear the retry flag
     */
    void RetryEnd();

    /**
     *  Reset the retry count to initial value
     */
    void ResetRetryCount();

    /** Setter/Getter for current state.
     */
    void SetPublicState(NKUIPublicState state)
    {
        m_currentState = state;
    }

    NKUIPublicState GetPublicState() const
    {
        return m_currentState;
    }

    //@todo: split this class into two parts:
    //       public: to be used by NKUI Widgets which is platform dependent and out of CCC.
    //       private: to be used by Presenters which is CCC internal.

    void RoadSignShown(bool shown)
    {
        m_roadSignShown = shown;
    }

    bool IsRoadSignShown() const
    {
        return m_roadSignShown;
    }

    void SetNKUIPreferences(NKUIPreferencesPtr preference)
    {
        m_preferences = preference;
    }

    PAL_Instance* GetPalInstance() const
    {
        return m_pal;
    }

    const nbnav::RouteOptions& GetRouteOptions() const
    {
        NKUI_CHECK_SHARED_PTR(m_preferences);
        return m_preferences->m_routeOptions;
    }

    const nbnav::Preferences& GetPreferences() const
    {
        NKUI_CHECK_SHARED_PTR(m_preferences);
        return m_preferences->m_navPreferences;
    }

    bool IsTurnByTurnNavigationEnabled() const
    {
        NKUI_CHECK_SHARED_PTR(m_preferences);
        return m_preferences->m_turnByTurnNavEnabled;
    }

    void SetRouteOption(nbnav::RouteOptions& options)
    {
        NKUI_CHECK_SHARED_PTR(m_preferences);
        m_preferences->m_routeOptions = options;
    }

private:
    NKUIContext();

    /** private members which are set into container for presenters or contoller using. */
    PAL_Instance*      m_pal;           //the pal instance.
    nbnav::Navigation* m_navigation;    //instace of navkit navigation instance.
    nbnav::Place       m_destination;   //the destination of current navigation.
    nbnav::Place       m_origin;        //the origin of current navigation.

    int m_chosenRouteIndex;             // index which was chosen by RST view.
    int m_currentRouteIndex;            // index of route which is currently used.

    MapSetting        m_mapSetting;     //the map settings.
    NKUIMapInterface* m_pMapInterface;  //map interface.

    uint32 m_retryCount;                //how many times left to retry getting route
    bool   m_processingRetry;           //true if a retry is processing
    bool   m_onRoute;                   //if current is on route
    bool   m_recalcingRoute;            //if recalcingRoute
    bool   m_gettingDetour;             //if getting detour
    bool   m_isStartoffRoute;           //if start up offroute case
    bool   m_enableTurnByTurnNav;       //enable turn by turn navigation
    bool   m_enhancedNavigationStartup; //enhanced start up

    bool                  m_isSelectingDetour;
    NKUILocation          m_currentLocation;         //current location, when on route or startup
    NKUISharedInformation m_navSharedInformation; //navigation shared information
    NKUIPublicState       m_currentState; // Current State.
    bool                  m_roadSignShown;
    NKUIPreferencesPtr    m_preferences;

    std::vector<nbnav::RouteInformation> m_routes;   //route information array received.
    shared_ptr<NKUIWidgetManager>    m_widgetManager;
    bool                  m_tryDetour;
};
}

#endif

/*! @} */
