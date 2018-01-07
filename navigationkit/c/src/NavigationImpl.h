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
    @file     NavigationImpl.h
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVIGATIONIMPL_H__
#define __NAVIGATIONIMPL_H__

#include "NavAnalytics.h"
#include "NavigationImplNotifier.h"
#include "Logger.h"
#include <vector>
#include <set>
#include "NavMockGPSProvider.h"
extern "C"
{
#include "nbnavigation.h"
#include "nbenhancedcontentmanager.h"
#include "paltimer.h"
}

namespace nbnav
{

using namespace std;

Place NbPlaceToPlace(const NB_Place& nbPlace);

/*! Internal implementation of navigation session. It is used to hide all internal specific from the public header.
*/

class Detour;

/*! NavigationImpl */
class NavigationImpl : public NavMockGPSListener
{
public:
    NavigationImpl(NB_Context* nbContext, const std::vector<Place> destinations, const RouteOptions& routeOptions, const Preferences& preferences, const vector<WayPoint>* wayPoint = NULL);
    NB_Error Init();
    void Release();

    /*! NavApiNavigation public methods (they are synchronized) */
    void UpdatePosition(const Location& location);
    void PauseSession();
    void ResumeSession();
    void StopSession();
    void PlanTrip(const Place& originPlace);

    void AddSessionListener(SessionListener* listener);
    void RemoveSessionListener(SessionListener* listener);
    void AddRoutePositionListener(RoutePositionListener* listener);
    void RemoveRoutePositionListener(RoutePositionListener* listener);
    void AddAnnouncementListener(AnnouncementListener* listener);
    void RemoveAnnouncementListener(AnnouncementListener* listener);
    void AddTrafficListener(TrafficListener* listener);
    void RemoveTrafficListener(TrafficListener* listener);
    void AddPositionUpdateListener(PositionUpdateListener* listener);
    void RemovePositionUpdateListener(PositionUpdateListener* listener);
    void AddManeuverUpdateListener(ManeuverUpdateListener* listener);
    void RemoveManeuverUpdateListener(ManeuverUpdateListener* listener);
    void AddNavEventListener(NavEventListener* listener);
    void RemoveNavEventListener(NavEventListener* listener);
    void AddSpecialRegionListener(SpecialRegionListener* listener);
    void RemoveSpecialRegionListener(SpecialRegionListener* listener);
    void AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener);
    void RemoveEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener);
    void Announce();
    void Announce(int manueverNumber);
    void Announce(int manueverNumber, const RouteInformation& route);
    void DoDetour();
    void CancelDetour();
    bool SetActiveRoute(const RouteInformation& route);
    NavigateRouteError NbRouteErrorToRouteError(NB_NavigateRouteError nbRouteError);
    NavigateRouteError NbErrorToRouteError(NB_Error nbError);
    void Recalculate();
    void Recalculate(const RouteOptions& newRouteOptions, const Preferences& newPreferences, bool wantAlternateRoute);
    void Recalculate(const vector<WayPoint>* wayPoint);
    void Lock();
    void UnLock();

    /*! Set work folder this Navigation.

        Navigation will try to load necessary resources from this work folder.

        @param path: path of work folder.
        @return void
    */
    void SetWorkFolder(const string& path);
    std::string GetBaseVoicesPath() const;

    /*! other public methods, NavKit internal using methods */
    NB_Navigation* GetNbNavigation() const;
    NB_RouteInformation* GetNbNavRouteInfo() const;
    NB_TrafficInformation* GetNbNavTrafficInfo() const;
    NB_Context* GetNbContext() const;
    PAL_Instance* GetPal() const;
    NB_GuidanceInformation* GetNbGuidanceInfo() const;
    NB_GuidanceInformation* GetNbGuidanceInfoForUI() const;
    bool IsTtsEnabled() const;
    bool IsNCEnabled() const;
    void StartStaticRouteRequest(const Place& originPlace);
    void StartRouteSummaryRequest(const Place& originPlace);
    NB_NavigateAnnouncementUnits GetMeasurementUnits() const;
    void ConvertGuidanceMessageToString(NB_GuidanceMessage* pMessage, string& plainText) const;
    NavigateRouteError GetRouteError() const;
    const NB_GpsLocation* GetLastLocation() const;
    unsigned int GetSignificantManeuverIndex(unsigned int maneuverIndex) const;
    unsigned int GetClosestManeuver() const;
    shared_ptr<bool> GetValidPointer() const;
    NB_RouteInformation* GetNbRoute(const RouteInformation& route) const;
    NB_RouteInformation* GetNbRoute(unsigned int routeIndex) const;

    NB_Error CreateNbNavSession(NB_NavigationCallbacks* callbacks, NB_GpsHistory* history, bool enhancedStartup);
    NB_Error CreateStaticNbNavSession(const Place& origin, NB_NavigationCallbacks* callbacks);
    NB_Error CreateSummaryNbNavSession(const Place& origin, NB_NavigationCallbacks* callbacks);
    NB_Error RecalcWithOption(const RouteOptions& options, const Preferences& preference,
                              NB_RouteOptions* nbOptions, NB_RouteConfiguration* nbConfig);
    const std::set<std::string>& GetSupportLocal();
    bool IsSupportedLocal(std::string local);
    /*! mock gps listener */
    virtual void OnMockGPSUpdate(NB_GpsLocation* location);
    
    const std::string GetLocal() const;

private:

    ~NavigationImpl();

    void Announce(NB_GuidanceMessage* message);
    NB_Error CreateNbNavSession();
    void DestroyNbNavSession();
    void NotifyPosition(const Coordinates& coordinates, double speed, double heading, bool isExtrapolateFix);

    /*! The methods are executed in Event thread. They are for internal methods */
    void UpdatePositionScheduled(NB_GpsLocation* data);
    void PauseSessionScheduled(int* data);
    void ResumeSessionScheduled(int* data);
    void StopSessionScheduled(int* data);
    void AnnounceScheduled(AnnounceData* data);
    void DoDetourScheduled(int* data);
    void DoCancelDetourScheduled(int* data);
    void SetActiveRouteScheduled(RouteInformation* data);
    void DeleteThisScheduled(int* data);
    void StartStaticRouteRequestScheduled(int* data);
    void StartRouteSummaryRequestScheduled(int* data);
    void RecalculateScheduled(int* data);
    void RecalculateWithRouteOptionScheduled(RouteOptionsData* options);
    void RecalculateWithWayPointScheduled(int* data);

    /*! Static timer callbacks. */
    static void Static_PositionChangeCb(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);
    static void Static_ExtraPolationPositionChangeCb(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);
    static void Static_StartupCb(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);

    /*! CCC NB_Navigation callbacks */
    static void NbSessionStatusChangeCb(NB_Navigation* navigation, NB_NavigateStatus status, void* userData);
    static void NbSessionGuidanceCb(NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData);
    static void NbManeuverPositionChangeCb(NB_Navigation* navigation, uint32 maneuver, NB_NavigateManeuverPos maneuverPosition, void* userData);
    static void NbEnhancedContentStateNotificationCb(NB_Navigation* navigation, NB_EnhancedContentState* state, NB_EnhancedContentStateData available, void* userData);
    static void NbSpeedLimitCb(NB_SpeedLimitStateData* speedlimitData, void* userData);
    static void NbVisualLaneGuidanceCb(NB_Navigation* navigation, unsigned long manueverIndex, unsigned long laneCount, NB_Lane * laneItems, void* userData);
    static void NbCompleteRouteDownloadCb(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    static void NbRouteDownloadCb(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    static void NbTrafficNotifcationCb(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
    static void NbRouteRequestedCb(NB_Navigation* navigation, NB_NavigateRouteRequestReason reason, void* userData);
    static void NbSpecialRegionCb(NB_SpecialRegionStateData* specialRegionData, void* userData);
    static double NbAnnounceSnippetLength(const char* AnnounceCode, void* userData);
    static void NbReachViaPointCB(NB_Location* location, void* userData);

    unsigned int GetNbRouteIndex(const RouteInformation& route);
    bool IsRouteValid(const RouteInformation& route) const;
    bool IsDetourValid(const RouteInformation& route) const;
    bool UpdateNbRoutes(NB_RouteInformation* route, bool detour);
    void ClearNbRoutes();
    void ClearNbDetours();
    void ClearActiveRoute();
    void AddAnalyticsNavigationState(const char* state);
    void AddAnalyticsDetourRequest();
    void AddAnalyticsDetourReply();
    NB_Error UpdateManeuverPositionInformation(uint32 maneuver);
    void SetRouteError(NavigateRouteError error);
    void UpdateManeuverList();
    const char* GetPrivateWorkFolder() const;
    bool IsExtrapolateEnabled() const;
    bool IsSuitablePosition() const;

    /*! Here are friend memebers. */
    friend class Detour;

    /*! Here are members which are used in Event thread only */
    NB_Context*                         m_context;
    PAL_Instance*                       m_pal;
    RouteOptions                        m_routeOptions;
    Preferences                         m_preferences;
    string                              m_workPath;
    string                              m_voicesStyle;
    string                              m_locale;
    NB_Navigation*                      m_nbNavigation;
    NB_EnhancedContentManager*          m_nbEnhancedManager;
    NB_GuidanceInformation*             m_nbGuidanceInfo;
    NB_GuidanceInformation*             m_nbGuidanceInfoForUI;
    Place                               m_origin;
    std::vector<Place>                  m_destinations;
    Detour*                             m_detour;
    RouteInformation*                   m_activeRoute;
    shared_ptr<Announcement>            m_currentAnnouncement;
    uint32                              m_currentManeuver;
    uint32                              m_closestManeuver;
    OnOffRouteType                      m_onOffRoute;
    SessionListener::RouteRequestReason m_routeRequestedReason;
    NB_GpsLocation                      m_lastPosition;
    vector<NB_RouteInformation*>        m_nbRoutes;
    vector<NB_RouteInformation*>        m_nbDetours;
    vector<NB_ExtrapolatePosition>      m_extrapolatePosition;
    uint32                              m_extrapolatePathLen;
    uint32                              m_extrapolatePointsInterval;
    uint32                              m_extrapolateCurrentPosition;
    double                              m_horizontalVelocity;
    PAL_Lock*                           m_methodLock;
    PAL_Lock*                           m_memberLock;
    NavAnalytics                        m_analytics;
    vector<RouteInformation>            m_receivedRoutes;
    int                                 m_requestedPercent;
    uint32                              m_tripRemainingTime;
    double                              m_tripRemainingDistance;
    uint32                              m_tripRemainingDelay;
    char                                m_tripTrafficColor;
    uint32                              m_maneuverRemainingTime;
    double                              m_maneuverRemainingDistance;
    uint32                              m_maneuverRemainingDelay;
    double                              m_maneuverTraveledDistance;
    uint32                              m_trafficDelay;
    Coordinates                         m_currentPosition;
    double                              m_currentSpeed;
    double                              m_currentHeading;
    string                              m_currentPrimaryRoadNameStr;
    string                              m_currentSecondaryRoadNameStr;
    string                              m_nextPrimaryRoadNameStr;
    string                              m_nextSecondaryRoadNameStr;
    string                              m_commandStr;
    string                              m_imageIdStr;
    string                              m_stackTurnImageStr;
    string                              m_exitNumberStr;
    Coordinates                         m_maneuverTurnPoint;
    NavEventTypeMask                    m_currentMask;
    NavEventTypeMask                    m_currentTotalMask;
    RoadSign*                           m_currentSignInfo;
    LaneInformation*                    m_currentLaneInfo;
    SpeedLimitInformation*              m_currentSpeedLimitInfo;
    TrafficEvent*                       m_currentTrafficEvent;
    TrafficInformation*                 m_currentTrafficInfo;
    ManeuverList*                       m_currentManeuverList;
    shared_ptr<bool>                    m_isValid;
    shared_ptr<bool>                    m_isDeleteValid;
    NavigateRouteError                  m_routeError;
    bool                                m_isRouteArrived;
    StartupState                        m_startupState;
    unsigned int                        m_startupTimeoff;
    bool                                m_isStartupRunning;
    NB_GpsHistory*                      m_gpsHistory;
    Coordinates                         m_enhancedStartupPosition;
    NB_GpsLocation*                     m_lastNoGpsFix;
    bool                                m_isRouteActive;
    Logger*                             m_logger;
    NavigationImplNotifier              m_notifier;
    nb_gpsTime                          m_lastETATime;
    bool                                m_needToRecalcForPlanTrip;
    bool                                m_isNewRouteForPlanTripReceived;
    bool                                m_planningTrip;
    vector<WayPoint>                    m_wayPoints;
    NavMockGPSProvider*                 m_mockProvider;
    NB_ExtrapolatePosition              m_lastExtrapolatePosition;
    NB_NavigateStatus                   m_status;
    std::set<std::string>               m_supportLocal;
    uint32                              m_lastGpsEventTime;
    bool                                m_updateTrafficInternal;
};

}


#endif

/*! @} */
