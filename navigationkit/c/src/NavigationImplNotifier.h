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
 @file     NavigationImplNotifier.h
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVIGATIONIMPLNOTIFIER_H__
#define __NAVIGATIONIMPLNOTIFIER_H__

#include "NavApiListeners.h"
#include "NavApiRouteOptions.h"
#include "NavApiPreferences.h"
extern "C"
{
#include "nbgpstypes.h"
#include "pallock.h"
}
#include <set>
#include <vector>

namespace nbnav
{
typedef struct AnnounceData
{
    int maneuver;
    const RouteInformation* route;
    NB_NavigateAnnouncementUnits units;
} AnnounceData;

class NavEventData
{
public:
    NavEventData(NavEventTypeMask mask, LaneInformation* lane, RoadSign* sign,
                 SpeedLimitInformation* speedLimit, TrafficEvent* traffic,
                 TrafficInformation* trafficInformation);
    ~NavEventData();

    NavEventTypeMask currentMask;
    LaneInformation* laneInfo;
    RoadSign* signInfo;
    SpeedLimitInformation* speedlimitInfo;
    TrafficEvent* trafficEvent;
    TrafficInformation* trafficInfo;
};

class PositionChangeData
{
public:
    uint32 tripRemainingTime;
    uint32 tripRemainingDelay;
    double tripRemainingDistance;
    char   tripTrafficColor;
    uint32 maneuverRemainingTime;
    uint32 trafficDelay;
    double maneuverRemainingDistance;
    Coordinates currentPosition;
    double currentSpeed;
    double currentHeading;
    bool ifNeedUpdateManeuverImage;
    bool ifNeedUpdateStackImage;
    string maneuverImageStr;
    string stackTurnImageStr;
    uint32 maneuverRemainingDelay;
};

class PositionUpdatedData
{
public:
    Coordinates currentPosition;
    double currentSpeed;
    double currentHeading;
};

class ManeuverPositionChangeData
{
public:
    ManeuverPositionChangeData() : maneuvers(NULL) {}
    ~ManeuverPositionChangeData();

    int maneuverID;
    string currentPrimaryRoadNameStr;
    string currentSecondaryRoadNameStr;
    string exitNumberStr;
    string imageIdStr;
    string commandStr;
    Coordinates maneuverTurnPoint;
    string nextPrimaryRoadNameStr;
    string nextSecondaryRoadNameStr;
    string stackTurnImageStr;
    ManeuverList* maneuvers;
};

class RouteReceivedData
{
public:
    SessionListener::RouteRequestReason routeRequestedReason;
    vector<RouteInformation> receivedRoutes;
};

class RouteOptionsData
{
public:
    RouteOptionsData() : options(Fastest, Car, 0), wantAlternateRoute(false) {};
    RouteOptions options;
    bool wantAlternateRoute;
    Preferences preference;
};

typedef enum
{
    rtNone = 0,
    rtOffRoute,
    rtOnRoute
} OnOffRouteType;

typedef enum
{
    SU_GoodInitial = 0,
    SU_BadInitial,
    SU_EnterEnhancedStartup,
    SU_ExitEnhancedStartup
} StartupState;

class NavigationImplNotifier
{
public:
    NavigationImplNotifier();
    ~NavigationImplNotifier();

    NB_Error Init(PAL_Instance* pal, shared_ptr<bool> valid);

    /*! Listeners add and remove. */
    void AddSessionListener(SessionListener* listener, OnOffRouteType onRouteType);
    void RemoveSessionListener(SessionListener* listener);
    void AddRoutePositionListener(RoutePositionListener* listener, PositionChangeData* data,
                                  ManeuverPositionChangeData* maneuverPositionData, NavEventData* navEventData);
    void RemoveRoutePositionListener(RoutePositionListener* listener);
    void AddAnnouncementListener(AnnouncementListener* listener);
    void RemoveAnnouncementListener(AnnouncementListener* listener);
    void AddTrafficListener(TrafficListener* listener, NavEventData* data);
    void RemoveTrafficListener(TrafficListener* listener);
    void AddPositionUpdateListener(PositionUpdateListener* listener, PositionChangeData* data);
    void RemovePositionUpdateListener(PositionUpdateListener* listener);
    void AddManeuverUpdateListener(ManeuverUpdateListener* listener,
                                   ManeuverPositionChangeData* maneuverPositionChangeData);
    void RemoveManeuverUpdateListener(ManeuverUpdateListener* listener);
    void AddNavEventListener(NavEventListener* listener, NavEventData* data);
    void RemoveNavEventListener(NavEventListener* listener);
    void AddSpecialRegionListener(SpecialRegionListener* listener);
    void RemoveSpecialRegionListener(SpecialRegionListener* listener);
    void AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener,
                                              bool enabled, StartupState state, Coordinates* position);
    void RemoveEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener);
    void ClearListeners();

    /*! The methods are excuted in UI thread, they are for internal used. */
    void ManeuverPositionChange(ManeuverPositionChangeData* maneuverPositionChangeData);
    void PositionChange(PositionChangeData* positionChangeData);
    void NavEvent(NavEventData* navEventData);
    void PositionUpdated(PositionUpdatedData* positionUpdatedData);
    void OnRoute();
    void OffRoute();
    void RouteReceived(RouteReceivedData* routeReceivedData);
    void RouteRequested(SessionListener::RouteRequestReason* reason);
    void RouteProgress(int* percent);
    void RouteError(NavigateRouteError* error);
    void RouteFinish();
    void RouteArriving(DestinationStreetSide* streetSide);
    void Announce(shared_ptr<Announcement>* announcement);
    void SpecialRegion(SpecialRegionInformation* specialRegion);
    void DisableSpecialRegion(SpecialRegionInformation* specialRegion);
    void OffRoutePositionUpdate(double* headingToOrigin);
    void EnterEnhancedStartup(Coordinates* coordinate);
    void ExitEnhancedStartup();
    void DisableNavEvents();
    void UpdateManeuverList(ManeuverList* maneuverList);
    void ReachViaPoint(Place* place);

    bool IsNoneAnnounceListener();
    void SetRouteReceived(bool received);
    void AnnouncementHeadsUp();

private:
    /*! Methods for immediate callback. */
    void CallbackSessionListener(SessionListener* listener, OnOffRouteType onRouteType);
    void CallbackManeuverUpdateListener(ManeuverUpdateListener* listener, ManeuverPositionChangeData* data);
    void CallbackNavEventListener(NavEventListener* listener, NavEventData* data);
    void CallbackPositionUpdateListener(PositionUpdateListener* listener, PositionChangeData* data);
    void CallbackRoutePositionListener(RoutePositionListener* listener, PositionChangeData* data,
                                       ManeuverPositionChangeData* maneuverPositionData, NavEventData* navEventData);
    void CallbackTrafficListener(TrafficListener* listener, NavEventData* data);
    void CallbackEnhancedStartupListener(EnhancedNavigationStartupListener* listener,
                                         bool enabled, StartupState state, Coordinates* position);

    /*! The methods are excuted in UI thread, they are for internal used. */
    void ManeuverPositionChangeUIScheduled(ManeuverPositionChangeData* maneuverPositionChangeData);
    void PositionChangeUIScheduled(PositionChangeData* positionChangeData);
    void NavEventUIScheduled(NavEventData* navEventData);
    void PositionUpdatedUIScheduled(PositionUpdatedData* positionUpdatedData);
    void OnRouteUIScheduled(int* data);
    void OffRouteUIScheduled(int* data);
    void RouteReceivedUIScheduled(RouteReceivedData* routeReceivedData);
    void RouteRequestedUIScheduled(SessionListener::RouteRequestReason* reason);
    void RouteProgressUIScheduled(int* percent);
    void RouteErrorUIScheduled(NavigateRouteError* error);
    void RouteFinishUIScheduled(int* data);
    void RouteArrivingUIScheduled(DestinationStreetSide* streetSide);
    void AnnounceUIScheduled(shared_ptr<Announcement>* announcement);
    void SpecialRegionUIScheduled(SpecialRegionInformation* specialRegion);
    void DisableSpecialRegionUIScheduled(SpecialRegionInformation* specialRegion);
    void OffRoutePositionUpdateUIScheduled(double* headingToOrigin);
    void EnterEnhancedStartupUIScheduled(Coordinates* coordinate);
    void ExitEnhancedStartupUIScheduled(int* data);
    void DisableNavEventsUIScheduled(int* data);
    void UpdateManeuverListUIScheduled(ManeuverList* maneuverList);
    void ReachViaPointUIScheduled(Place* place);
    void AnnouncementHeadsUpUIScheduled(NavigationImplNotifier* data);

    PAL_Instance*                                m_pal;
    PAL_Lock*                                    m_listenerListsLock;
    shared_ptr<bool>                             m_isValid;
    bool                                         m_isRouteReceived;

    /*! Here are members which are syncronized by listenerListsLock */
    std::vector<SessionListener*>                   m_sessionListeners;
    std::set<RoutePositionListener*>             m_routePositionListeners;
    std::set<AnnouncementListener*>              m_announcementListeners;
    std::set<TrafficListener*>                   m_trafficListeners;
    std::set<PositionUpdateListener*>            m_positionUpdateListeners;
    std::set<ManeuverUpdateListener*>            m_maneuverUpdateListeners;
    std::set<NavEventListener*>                  m_navEventListeners;
    std::set<SpecialRegionListener*>             m_specialRegionListeners;
    std::set<EnhancedNavigationStartupListener*> m_enhancedNavigationStartupListeners;
};

}

#endif

/*! @} */
