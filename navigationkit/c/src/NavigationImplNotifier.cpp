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
 *
 */

#include "NavigationImplNotifier.h"
#include "NavTask.h"

#define MPERF

#ifdef MPERF
#include <QDateTime>
#include <QDebug>
#endif

namespace nbnav
{
NavEventData::NavEventData(NavEventTypeMask mask, LaneInformation* lane, RoadSign* sign,
                           SpeedLimitInformation* speedLimit, TrafficEvent* traffic,
                           TrafficInformation* trafficInformation)
    : currentMask(mask),
      laneInfo(lane),
      signInfo(sign),
      speedlimitInfo(speedLimit),
      trafficEvent(traffic),
      trafficInfo(trafficInformation)
{
}

NavEventData::~NavEventData()
{
    if (laneInfo)
    {
        delete laneInfo;
    }
    if (signInfo)
    {
        delete signInfo;
    }
    if (speedlimitInfo)
    {
        delete speedlimitInfo;
    }
    if (trafficEvent)
    {
        delete trafficEvent;
    }
    if (trafficInfo)
    {
        delete trafficInfo;
    }
}

ManeuverPositionChangeData::~ManeuverPositionChangeData()
{
    if (maneuvers)
    {
        delete maneuvers;
    }
}

NavigationImplNotifier::NavigationImplNotifier()
    : m_pal(NULL),
      m_listenerListsLock(NULL),
      m_isRouteReceived(false)
{
}

NavigationImplNotifier::~NavigationImplNotifier()
{
    (void)PAL_LockDestroy(m_listenerListsLock);
    m_listenerListsLock = NULL;
}

NB_Error NavigationImplNotifier::Init(PAL_Instance* pal, shared_ptr<bool> valid)
{
    m_pal = pal;
    m_isValid = valid;
    return (PAL_LockCreate(m_pal, &m_listenerListsLock) == PAL_Ok) ? NE_OK : NE_INVAL;
}

void NavigationImplNotifier::ClearListeners()
{
    PAL_LockLock(m_listenerListsLock);
    m_sessionListeners.clear();
    m_routePositionListeners.clear();
    m_announcementListeners.clear();
    m_trafficListeners.clear();
    m_positionUpdateListeners.clear();
    m_maneuverUpdateListeners.clear();
    m_navEventListeners.clear();
    m_specialRegionListeners.clear();
    PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddSessionListener(SessionListener* listener, OnOffRouteType onRouteType)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_sessionListeners.push_back(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackSessionListener(listener, onRouteType);
}

void NavigationImplNotifier::RemoveSessionListener(SessionListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*>::iterator it = m_sessionListeners.begin();
    for (; it != m_sessionListeners.end(); ++it)
    {
        if((*it) == listener)
        {
            m_sessionListeners.erase(it);
            break;
        }
    }
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddRoutePositionListener(RoutePositionListener* listener, PositionChangeData* data,
                                                      ManeuverPositionChangeData* maneuverPositionData, NavEventData* navEventData)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_routePositionListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackRoutePositionListener(listener, data, maneuverPositionData, navEventData);
}

void NavigationImplNotifier::RemoveRoutePositionListener(RoutePositionListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_routePositionListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddAnnouncementListener(AnnouncementListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_announcementListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::RemoveAnnouncementListener(AnnouncementListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_announcementListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddTrafficListener(TrafficListener* listener, NavEventData* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_trafficListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackTrafficListener(listener, data);
}

void NavigationImplNotifier::RemoveTrafficListener(TrafficListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_trafficListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddPositionUpdateListener(PositionUpdateListener* listener,
                                                       PositionChangeData* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_positionUpdateListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackPositionUpdateListener(listener, data);
}

void NavigationImplNotifier::RemovePositionUpdateListener(PositionUpdateListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_positionUpdateListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddManeuverUpdateListener(ManeuverUpdateListener* listener,
                                                       ManeuverPositionChangeData* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_maneuverUpdateListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackManeuverUpdateListener(listener, data);
}

void NavigationImplNotifier::RemoveManeuverUpdateListener(ManeuverUpdateListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_maneuverUpdateListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddNavEventListener(NavEventListener* listener, NavEventData* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_navEventListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackNavEventListener(listener, data);
}

void NavigationImplNotifier::RemoveNavEventListener(NavEventListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_navEventListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddSpecialRegionListener(SpecialRegionListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_specialRegionListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::RemoveSpecialRegionListener(SpecialRegionListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_specialRegionListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener,
                                                                  bool enabled, StartupState state, Coordinates* position)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_enhancedNavigationStartupListeners.insert(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
    CallbackEnhancedStartupListener(listener, enabled, state, position);
}

void NavigationImplNotifier::RemoveEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener)
{
    (void)PAL_LockLock(m_listenerListsLock);
    m_enhancedNavigationStartupListeners.erase(listener);
    (void)PAL_LockUnlock(m_listenerListsLock);
}

void NavigationImplNotifier::ManeuverPositionChangeUIScheduled(ManeuverPositionChangeData* maneuverPositionChangeData)
{
    // We need to work with the copy of listeners set, because set should be thread-safe. It isn't good
    // to call user callbacks under lock, this might lead to deadlocks. The set is light-weight, it contains pointers.
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> listeners(m_routePositionListeners);
    set<ManeuverUpdateListener*> maneuverListeners(m_maneuverUpdateListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (maneuverPositionChangeData && listeners.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->UpdateManeuverList(*maneuverPositionChangeData->maneuvers);
            (*it)->CurrentRoadName(maneuverPositionChangeData->currentPrimaryRoadNameStr,
                                   maneuverPositionChangeData->currentSecondaryRoadNameStr);
            (*it)->ManeuverExitNumber(maneuverPositionChangeData->exitNumberStr);
            (*it)->ManeuverImageId(maneuverPositionChangeData->imageIdStr);
            (*it)->ManeuverType(maneuverPositionChangeData->commandStr);
            (*it)->ManeuverPoint(maneuverPositionChangeData->maneuverTurnPoint);
            (*it)->NextRoadName(maneuverPositionChangeData->nextPrimaryRoadNameStr,
                                maneuverPositionChangeData->nextSecondaryRoadNameStr);
            (*it)->StackTurnImageTTF(maneuverPositionChangeData->stackTurnImageStr);
        }
    }

    if (maneuverListeners.size() > 0)
    {
        set<ManeuverUpdateListener*>::iterator itManeuverListeners = maneuverListeners.begin();
        for (; itManeuverListeners != maneuverListeners.end(); ++itManeuverListeners)
        {
            (*itManeuverListeners)->ManeuverUpdate(maneuverPositionChangeData->maneuverID,
                                                   maneuverPositionChangeData->currentPrimaryRoadNameStr,
                                                   maneuverPositionChangeData->currentSecondaryRoadNameStr,
                                                   maneuverPositionChangeData->exitNumberStr,
                                                   maneuverPositionChangeData->imageIdStr,
                                                   maneuverPositionChangeData->maneuverTurnPoint,
                                                   maneuverPositionChangeData->commandStr,
                                                   maneuverPositionChangeData->nextPrimaryRoadNameStr,
                                                   maneuverPositionChangeData->nextSecondaryRoadNameStr,
                                                   maneuverPositionChangeData->stackTurnImageStr);
        }
    }
}

void NavigationImplNotifier::PositionChangeUIScheduled(PositionChangeData* positionChangeData)
{
    // We need to work with the copy of listeners set, because set should be thread-safe. It isn't good
    // to call user callbacks under lock, this might lead to deadlocks. The set is light-weight, it contains pointers.
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> listeners(m_routePositionListeners);
    set<PositionUpdateListener*> positionListeners(m_positionUpdateListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (positionChangeData && listeners.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->TripRemainingTime(positionChangeData->tripRemainingTime);
            (*it)->TripRemainingDelay(positionChangeData->tripRemainingDelay);
            (*it)->TripRemainingDistance(positionChangeData->tripRemainingDistance);
            (*it)->TripTrafficColor(positionChangeData->tripTrafficColor);

            (*it)->ManeuverRemainingTime(static_cast<int>(positionChangeData->maneuverRemainingTime));
            (*it)->ManeuverRemainingDelay(static_cast<int>(positionChangeData->trafficDelay));
            (*it)->ManeuverRemainingDistance(positionChangeData->maneuverRemainingDistance);

            if (positionChangeData->ifNeedUpdateStackImage)
            {
                (*it)->StackTurnImageTTF(positionChangeData->stackTurnImageStr);
            }

            if (positionChangeData->ifNeedUpdateManeuverImage)
            {
                (*it)->ManeuverImageId(positionChangeData->maneuverImageStr);
            }
        }
    }
    if (positionListeners.size() > 0)
    {
        set<PositionUpdateListener*>::iterator itPosition = positionListeners.begin();
        for (; itPosition != positionListeners.end(); ++itPosition)
        {
            (*itPosition)->PositionUpdate(static_cast<int>(positionChangeData->trafficDelay),
                                          positionChangeData->tripRemainingDistance,
                                          static_cast<int>(positionChangeData->tripRemainingTime),
                                          positionChangeData->currentPosition,
                                          static_cast<int>(positionChangeData->currentSpeed),
                                          static_cast<int>(positionChangeData->currentHeading),
                                          static_cast<int>(positionChangeData->tripRemainingDelay),
                                          positionChangeData->tripRemainingDistance,
                                          static_cast<int>(positionChangeData->tripRemainingTime));
        }
    }
}

void NavigationImplNotifier::NavEventUIScheduled(NavEventData* navEventData)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> positionListeners(m_routePositionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (navEventData && positionListeners.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = positionListeners.begin();
        for (; it != positionListeners.end(); ++it)
        {
            switch (navEventData->currentMask)
            {
                case NETP_RoadSignEnable:
                {
                    (*it)->RoadSign(*navEventData->signInfo);
                    break;
                }
                case NETP_RoadSignDisable:
                {
                    (*it)->DisableRoadSign();
                    break;
                }
                case NETP_SpeedLimitEnable:
                {
                    (*it)->SpeedLimit(*navEventData->speedlimitInfo);
                    break;
                }
                case NETP_SpeedLimitDisable:
                {
                    (*it)->DisableSpeedLimit();
                    break;
                }
                case NETP_LaneInformationEnable:
                {
                    (*it)->LaneInformation(*navEventData->laneInfo);
                    break;
                }
                case NETP_LaneInformationDisable:
                {
                    (*it)->DisableLaneInformation();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    (void)PAL_LockLock(m_listenerListsLock);
    set<TrafficListener*> trafficListenersCopy(m_trafficListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (navEventData && trafficListenersCopy.size() > 0)
    {
        set<TrafficListener*>::iterator it = trafficListenersCopy.begin();
        for (; it != trafficListenersCopy.end(); ++it)
        {
            switch (navEventData->currentMask)
            {
                case NETP_TrafficEventNotification:
                {
                    (*it)->TrafficAlerted(*navEventData->trafficEvent);
                    break;
                }
                case NETP_TrafficUpdate:
                {
                    (*it)->TrafficChanged(*navEventData->trafficInfo);
                    break;
                }
                case NETP_TrafficEventDisable:
                {
                    (*it)->DisableTrafficAlerted();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    (void)PAL_LockLock(m_listenerListsLock);
    set<NavEventListener*> listeners(m_navEventListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);
    if (navEventData && listeners.size() > 0)
    {
        set<NavEventListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->NavEvent(navEventData->currentMask, *navEventData->laneInfo, *navEventData->signInfo,
                            *navEventData->speedlimitInfo, *navEventData->trafficEvent, *navEventData->trafficInfo);
        }
    }
}

void NavigationImplNotifier::PositionUpdatedUIScheduled(PositionUpdatedData* positionUpdatedData)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> listeners(m_routePositionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (positionUpdatedData && listeners.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->PositionUpdated(positionUpdatedData->currentPosition,
                                   positionUpdatedData->currentSpeed,
                                   positionUpdatedData->currentHeading);
        }
    }
}

void NavigationImplNotifier::OnRouteUIScheduled(int* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->OnRoute();
        }
    }
}

void NavigationImplNotifier::OffRouteUIScheduled(int* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->OffRoute();
        }
    }
}

void NavigationImplNotifier::RouteReceivedUIScheduled(RouteReceivedData* routeReceivedData)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (routeReceivedData && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
#ifdef MPERF
            qint64 startTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager begin RouteReceivedUIScheduled calculate route at"<<startTime;
#endif
            

            (*it)->RouteReceived(routeReceivedData->routeRequestedReason,
                                 routeReceivedData->receivedRoutes);
            
#ifdef MPERF
            qint64 endTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager finish RouteReceivedUIScheduled calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif
        }
    }
}

void NavigationImplNotifier::RouteRequestedUIScheduled(SessionListener::RouteRequestReason* reason)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (reason && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->RouteRequested(*reason);
        }
    }
}

void NavigationImplNotifier::RouteProgressUIScheduled(int* percent)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (percent && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->RouteProgress(*percent);
        }
    }
}

void NavigationImplNotifier::RouteErrorUIScheduled(NavigateRouteError* error)
{
    NavigateRouteError err = *error;
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (error && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->RouteError(err);
        }
    }
}

void NavigationImplNotifier::RouteFinishUIScheduled(int* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->RouteFinish();
        }
    }
}

void NavigationImplNotifier::RouteArrivingUIScheduled(DestinationStreetSide* streetSide)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (streetSide && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->ArrivingToDestination(*streetSide);
        }
    }
}

void NavigationImplNotifier::OffRoutePositionUpdateUIScheduled(double* headingToOrigin)
{
    (void)PAL_LockLock(m_listenerListsLock);
    vector<SessionListener*> listeners(m_sessionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (headingToOrigin && listeners.size() > 0)
    {
        vector<SessionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->OffRoutePositionUpdate(*headingToOrigin);
        }
    }
}

void NavigationImplNotifier::AnnounceUIScheduled(shared_ptr<Announcement>* announcement)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<AnnouncementListener*> listeners(m_announcementListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (announcement && listeners.size() > 0)
    {
        set<AnnouncementListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->Announce(*announcement);
        }
    }
}

void NavigationImplNotifier::SpecialRegionUIScheduled(SpecialRegionInformation* specialRegion)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<SpecialRegionListener*> listeners(m_specialRegionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (specialRegion && listeners.size() > 0)
    {
        set<SpecialRegionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->SpecialRegion(specialRegion);
        }
    }
}

void NavigationImplNotifier::DisableSpecialRegionUIScheduled(SpecialRegionInformation* specialRegion)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<SpecialRegionListener*> listeners(m_specialRegionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (specialRegion && listeners.size() > 0)
    {
        set<SpecialRegionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->DisableSpecialRegion(specialRegion);
        }
    }
}

void NavigationImplNotifier::EnterEnhancedStartupUIScheduled(Coordinates* coordinate)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<EnhancedNavigationStartupListener*> listeners(m_enhancedNavigationStartupListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);
    if (coordinate && listeners.size() > 0)
    {
        set<EnhancedNavigationStartupListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->EnterEnhancedNavigationStartup(*coordinate);
        }
    }
}

void NavigationImplNotifier::ExitEnhancedStartupUIScheduled(int* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<EnhancedNavigationStartupListener*> listeners(m_enhancedNavigationStartupListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);
    if (listeners.size() > 0)
    {
        set<EnhancedNavigationStartupListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->ExitEnhancedNavigationStartup();
        }
    }
}

void NavigationImplNotifier::DisableNavEventsUIScheduled(int* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> listenersCopy(m_routePositionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (listenersCopy.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = listenersCopy.begin();
        for (; it != listenersCopy.end(); ++it)
        {
            (*it)->DisableLaneInformation();
            (*it)->DisableRoadSign();
            (*it)->DisableSpeedLimit();
        }
    }

    (void)PAL_LockLock(m_listenerListsLock);
    set<TrafficListener*> trafficListenersCopy(m_trafficListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (trafficListenersCopy.size() > 0)
    {
        set<TrafficListener*>::iterator it = trafficListenersCopy.begin();
        for (; it != trafficListenersCopy.end(); ++it)
        {
            (*it)->DisableTrafficAlerted();
        }
    }

    (void)PAL_LockLock(m_listenerListsLock);
    set<NavEventListener*> navEventListenersCopy(m_navEventListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (navEventListenersCopy.size() > 0)
    {
        LaneInformation* laneInfo = new LaneInformation();
        RoadSign* roadSign = new RoadSign();
        SpeedLimitInformation* speedLimitInfo = new SpeedLimitInformation();
        TrafficEvent* trafficEvent = new TrafficEvent();
        TrafficInformation* trafficInfo = new TrafficInformation();

        set<NavEventListener*>::iterator it = navEventListenersCopy.begin();
        for (; it != navEventListenersCopy.end(); ++it)
        {
            (*it)->NavEvent(NavEventTypeMask(NETP_RoadSignDisable | NETP_SpeedLimitDisable | NETP_LaneInformationDisable | NETP_TrafficEventDisable),
                            *laneInfo, *roadSign, *speedLimitInfo, *trafficEvent, *trafficInfo);
        }

        delete laneInfo;
        delete roadSign;
        delete speedLimitInfo;
        delete trafficEvent;
        delete trafficInfo;
    }
}

void NavigationImplNotifier::CallbackSessionListener(SessionListener* listener, OnOffRouteType onRoutetype)
{
    if (!m_isRouteReceived)
    {
        return;
    }

    if (onRoutetype == rtOnRoute)
    {
        listener->OnRoute();
    }
    else if (onRoutetype == rtOffRoute)
    {
        listener->OffRoute();
    }
}

void NavigationImplNotifier::CallbackManeuverUpdateListener(ManeuverUpdateListener* listener, ManeuverPositionChangeData* data)
{
    if (!m_isRouteReceived)
    {
        return;
    }
    listener->ManeuverUpdate(data->maneuverID,
                             data->currentPrimaryRoadNameStr, data->currentSecondaryRoadNameStr,
                             data->exitNumberStr, data->imageIdStr, data->maneuverTurnPoint,
                             data->commandStr, data->nextPrimaryRoadNameStr, data->nextSecondaryRoadNameStr,
                             data->stackTurnImageStr);
}

void NavigationImplNotifier::CallbackNavEventListener(NavEventListener* listener, NavEventData* data)
{
    if (!m_isRouteReceived)
    {
        return;
    }
    listener->NavEvent(data->currentMask, *data->laneInfo, *data->signInfo, *data->speedlimitInfo,
                       *data->trafficEvent, *data->trafficInfo);
}

void NavigationImplNotifier::CallbackPositionUpdateListener(PositionUpdateListener* listener, PositionChangeData* data)
{
    if (!m_isRouteReceived)
    {
        return;
    }
    listener->PositionUpdate(data->maneuverRemainingDelay, data->maneuverRemainingDistance,
                             data->maneuverRemainingTime, data->currentPosition, (int)data->currentSpeed,
                             (int)data->currentHeading, data->trafficDelay, data->tripRemainingDistance,
                             data->tripRemainingTime);
}

void NavigationImplNotifier::CallbackRoutePositionListener(RoutePositionListener* listener, PositionChangeData* data,
                                                           ManeuverPositionChangeData* maneuverPositionData, NavEventData* navEventData)
{
    if (!m_isRouteReceived)
    {
        return;
    }
    if (maneuverPositionData->maneuvers)
    {
        listener->UpdateManeuverList(*maneuverPositionData->maneuvers);
    }
    listener->TripRemainingTime(data->tripRemainingTime);
    listener->TripRemainingDelay(data->tripRemainingDelay);
    listener->TripRemainingDistance(data->tripRemainingDistance);
    listener->TripTrafficColor(data->tripTrafficColor);
    listener->CurrentRoadName(maneuverPositionData->currentPrimaryRoadNameStr,
                              maneuverPositionData->currentSecondaryRoadNameStr);
    listener->ManeuverExitNumber(maneuverPositionData->exitNumberStr);
    listener->NextRoadName(maneuverPositionData->nextPrimaryRoadNameStr,
                           maneuverPositionData->nextSecondaryRoadNameStr);
    listener->ManeuverType(maneuverPositionData->commandStr);
    listener->ManeuverRemainingTime(data->maneuverRemainingTime);
    listener->ManeuverRemainingDelay(data->maneuverRemainingDelay);
    listener->ManeuverRemainingDistance(data->maneuverRemainingDistance);
    listener->PositionUpdated(data->currentPosition, data->currentSpeed, data->currentHeading);
    listener->ManeuverPoint(maneuverPositionData->maneuverTurnPoint);
    listener->ManeuverImageId(maneuverPositionData->imageIdStr);
    listener->StackTurnImageTTF(maneuverPositionData->stackTurnImageStr);
    if (navEventData->signInfo)
    {
        if (navEventData->currentMask & NETP_RoadSignEnable)
        {
            listener->RoadSign(*navEventData->signInfo);
        }
        else
        {
            listener->DisableRoadSign();
        }
    }
    if (navEventData->laneInfo)
    {
        if (navEventData->currentMask & NETP_LaneInformationEnable)
        {
            listener->LaneInformation(*navEventData->laneInfo);
        }
        else
        {
            listener->DisableLaneInformation();
        }
    }
    if (navEventData->speedlimitInfo)
    {
        if (navEventData->currentMask & NETP_SpeedLimitEnable)
        {
            listener->SpeedLimit(*navEventData->speedlimitInfo);
        }
        else
        {
            listener->DisableSpeedLimit();
        }
    }
}

void NavigationImplNotifier::CallbackTrafficListener(TrafficListener* listener, NavEventData* data)
{
    if (!m_isRouteReceived)
    {
        return;
    }
    if (data->trafficEvent)
    {
        listener->TrafficAlerted(*data->trafficEvent);
    }
    if (data->currentMask & NETP_TrafficEventDisable)
    {
        listener->DisableTrafficAlerted();
    }
    if (data->trafficInfo)
    {
        listener->TrafficChanged(*data->trafficInfo);
    }
}

void NavigationImplNotifier::CallbackEnhancedStartupListener(EnhancedNavigationStartupListener* listener,
                                                             bool enabled, StartupState state, Coordinates* position)
{
    if (enabled)
    {
        if (state == SU_EnterEnhancedStartup)
        {
            listener->EnterEnhancedNavigationStartup(*position);
        }
        else if (state == SU_ExitEnhancedStartup)
        {
            listener->ExitEnhancedNavigationStartup();
        }
    }
}

void NavigationImplNotifier::UpdateManeuverListUIScheduled(ManeuverList* maneuverList)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<RoutePositionListener*> listeners(m_routePositionListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);

    if (maneuverList && listeners.size() > 0)
    {
        set<RoutePositionListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->UpdateManeuverList(*maneuverList);
        }
    }
}

void NavigationImplNotifier::ReachViaPointUIScheduled(Place* place)
{
    if(place)
    {
        (void)PAL_LockLock(m_listenerListsLock);
        set<RoutePositionListener*> listeners(m_routePositionListeners);
        (void)PAL_LockUnlock(m_listenerListsLock);

        if (listeners.size() > 0)
        {
            set<RoutePositionListener*>::iterator it = listeners.begin();
            for (; it != listeners.end(); ++it)
            {
                (*it)->ReachViaPoint(place);
            }
        }
    }
}

void NavigationImplNotifier::ManeuverPositionChange(ManeuverPositionChangeData* maneuverPositionChangeData)
{
    NavTask<NavigationImplNotifier, ManeuverPositionChangeData>* task = new NavTask<NavigationImplNotifier, ManeuverPositionChangeData>(
        m_pal, this, maneuverPositionChangeData, &NavigationImplNotifier::ManeuverPositionChangeUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::PositionChange(PositionChangeData* positionChangeData)
{
    NavTask<NavigationImplNotifier, PositionChangeData>* task = new NavTask<NavigationImplNotifier, PositionChangeData>(
        m_pal, this, positionChangeData, &NavigationImplNotifier::PositionChangeUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::NavEvent(NavEventData* navEventData)
{
    NavTask<NavigationImplNotifier, NavEventData>* task = new NavTask<NavigationImplNotifier, NavEventData>(
        m_pal, this, navEventData, &NavigationImplNotifier::NavEventUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::PositionUpdated(PositionUpdatedData* positionUpdatedData)
{
    NavTask<NavigationImplNotifier, PositionUpdatedData>* task = new NavTask<NavigationImplNotifier, PositionUpdatedData>(
        m_pal, this, positionUpdatedData, &NavigationImplNotifier::PositionUpdatedUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::OnRoute()
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, NULL, &NavigationImplNotifier::OnRouteUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::OffRoute()
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, NULL, &NavigationImplNotifier::OffRouteUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteReceived(RouteReceivedData* routeReceivedData)
{
    NavTask<NavigationImplNotifier, RouteReceivedData>* task = new NavTask<NavigationImplNotifier, RouteReceivedData>(
        m_pal, this, routeReceivedData, &NavigationImplNotifier::RouteReceivedUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteRequested(SessionListener::RouteRequestReason* reason)
{
    NavTask<NavigationImplNotifier, SessionListener::RouteRequestReason>* task =
        new NavTask<NavigationImplNotifier, SessionListener::RouteRequestReason>(
        m_pal, this, reason, &NavigationImplNotifier::RouteRequestedUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteProgress(int* percent)
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, percent, &NavigationImplNotifier::RouteProgressUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteError(NavigateRouteError* error)
{
    NavTask<NavigationImplNotifier, NavigateRouteError>* task = new NavTask<NavigationImplNotifier, NavigateRouteError>(
        m_pal, this, error, &NavigationImplNotifier::RouteErrorUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteFinish()
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, NULL, &NavigationImplNotifier::RouteFinishUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::RouteArriving(DestinationStreetSide* streetSide)
{
    NavTask<NavigationImplNotifier, DestinationStreetSide>* task = new NavTask<NavigationImplNotifier, DestinationStreetSide>(
        m_pal, this, streetSide, &NavigationImplNotifier::RouteArrivingUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::Announce(shared_ptr<Announcement>* announcement)
{
    NavTask<NavigationImplNotifier, shared_ptr<Announcement> >* task = new NavTask<NavigationImplNotifier, shared_ptr<Announcement> >(
        m_pal, this, announcement, &NavigationImplNotifier::AnnounceUIScheduled, m_isValid, false, true, true);
    task->Execute();
}

void NavigationImplNotifier::SpecialRegion(SpecialRegionInformation* specialRegion)
{
    NavTask<NavigationImplNotifier, SpecialRegionInformation>* task =
        new NavTask<NavigationImplNotifier, SpecialRegionInformation>(
        m_pal, this, specialRegion, &NavigationImplNotifier::SpecialRegionUIScheduled, m_isValid, false, true);

    task->Execute();
}

void NavigationImplNotifier::DisableSpecialRegion(SpecialRegionInformation* specialRegion)
{
    NavTask<NavigationImplNotifier, SpecialRegionInformation>* task =
    new NavTask<NavigationImplNotifier, SpecialRegionInformation>(
        m_pal, this, specialRegion, &NavigationImplNotifier::DisableSpecialRegionUIScheduled, m_isValid, false, true);

    task->Execute();
}

void NavigationImplNotifier::OffRoutePositionUpdate(double* headingToOrigin)
{
    NavTask<NavigationImplNotifier, double>* task = new NavTask<NavigationImplNotifier, double>(
        m_pal, this, headingToOrigin, &NavigationImplNotifier::OffRoutePositionUpdateUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::EnterEnhancedStartup(Coordinates* coordinate)
{
    NavTask<NavigationImplNotifier, Coordinates>* task = new NavTask<NavigationImplNotifier, Coordinates>(
        m_pal, this, coordinate, &NavigationImplNotifier::EnterEnhancedStartupUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::ExitEnhancedStartup()
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, NULL, &NavigationImplNotifier::ExitEnhancedStartupUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::DisableNavEvents()
{
    NavTask<NavigationImplNotifier, int>* task = new NavTask<NavigationImplNotifier, int>(
        m_pal, this, NULL, &NavigationImplNotifier::DisableNavEventsUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::UpdateManeuverList(ManeuverList* maneuverList)
{
    NavTask<NavigationImplNotifier, ManeuverList>* task = new NavTask<NavigationImplNotifier, ManeuverList>(
        m_pal, this, maneuverList, &NavigationImplNotifier::UpdateManeuverListUIScheduled, m_isValid, false, true);
    task->Execute();
}

void NavigationImplNotifier::ReachViaPoint(Place* place)
{
    NavTask<NavigationImplNotifier, Place>* task = new NavTask<NavigationImplNotifier, Place>(m_pal, this, place, &NavigationImplNotifier::ReachViaPointUIScheduled, m_isValid, false, true);
    task->Execute();
}

bool NavigationImplNotifier::IsNoneAnnounceListener()
{
    return m_announcementListeners.size() == 0;
}

void NavigationImplNotifier::SetRouteReceived(bool received)
{
    m_isRouteReceived = received;
}
    
void NavigationImplNotifier::AnnouncementHeadsUp()
{
    NavTask<NavigationImplNotifier, NavigationImplNotifier>* task = new NavTask<NavigationImplNotifier, NavigationImplNotifier>(m_pal, this, this, &NavigationImplNotifier::AnnouncementHeadsUpUIScheduled, m_isValid, false, true, false);
    task->Execute();
}
    
void NavigationImplNotifier::AnnouncementHeadsUpUIScheduled(NavigationImplNotifier* data)
{
    (void)PAL_LockLock(m_listenerListsLock);
    set<AnnouncementListener*> listeners(m_announcementListeners);
    (void)PAL_LockUnlock(m_listenerListsLock);
    
    if (listeners.size() > 0)
    {
        set<AnnouncementListener*>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it)
        {
            (*it)->AnnounceHeadsUp();
        }
    }
}

}
