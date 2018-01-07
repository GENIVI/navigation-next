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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

#include "NavApiNavigation.h"
#include "NavigationImpl.h"

namespace nbnav
{

Navigation* Navigation::GetNavigation(NB_Context* nbContext, const Place& destination,
                                      const RouteOptions& routeOptions, const Preferences& preferences)
{
    vector<WayPoint> vias;
    return Navigation::GetNavigationWithWorkFoler(nbContext, destination, routeOptions, preferences, "",vias);
}

Navigation* Navigation::GetNavigationWithWorkFoler(NB_Context* nbContext,
                                                   const Place& destination,
                                                   const RouteOptions& routeOptions,
                                                   const Preferences& preferences,
                                                   const string& workFolder,
                                                   const vector<WayPoint> viaPoint)
{
    if (nbContext == NULL)
    {
        return NULL;
    }

    std::vector<Place> destinations;
    destinations.push_back(destination);
    NavigationImpl* impl = new NavigationImpl(nbContext, destinations, routeOptions, preferences, &viaPoint);
    if (impl == NULL)
    {
        return NULL;
    }

    if(!workFolder.empty())
    {
        impl->SetWorkFolder(workFolder);
    }
    if (impl->Init() != NE_OK)
    {
        return NULL;
    }

    Navigation* me = new Navigation;
    if (me != NULL)
    {
        me->m_impl = impl;
    }
    else
    {
        impl->Release();
    }

    return me;
}


Navigation::~Navigation()
{
    if (m_impl)
    {
        m_impl->Release();
    }
}

void Navigation::UpdatePosition(const Location& location)
{
    m_impl->UpdatePosition(location);
}

void Navigation::PauseSession()
{
    m_impl->PauseSession();
}

void Navigation::ResumeSession()
{
    m_impl->ResumeSession();
}

void Navigation::StopSession()
{
    m_impl->StopSession();
}

void Navigation::AddSessionListener(SessionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddSessionListener(listener);
    }
}

void Navigation::RemoveSessionListener(SessionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveSessionListener(listener);
    }
}

void Navigation::AddRoutePositionListener(RoutePositionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddRoutePositionListener(listener);
    }
}

void Navigation::RemoveRoutePositionListener(RoutePositionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveRoutePositionListener(listener);
    }
}

void Navigation::AddAnnouncementListener(AnnouncementListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddAnnouncementListener(listener);
    }
}

void Navigation::RemoveAnnouncementListener(AnnouncementListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveAnnouncementListener(listener);
    }
}

void Navigation::AddTrafficListener(TrafficListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddTrafficListener(listener);
    }
}

void Navigation::RemoveTrafficListener(TrafficListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveTrafficListener(listener);
    }
}

void Navigation::AddPositionUpdateListener(PositionUpdateListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddPositionUpdateListener(listener);
    }
}

void Navigation::RemovePositionUpdateListener(PositionUpdateListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemovePositionUpdateListener(listener);
    }
}

void Navigation::AddManeuverUpdateListener(ManeuverUpdateListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddManeuverUpdateListener(listener);
    }
}

void Navigation::RemoveManeuverUpdateListener(ManeuverUpdateListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveManeuverUpdateListener(listener);
    }
}

void Navigation::AddNavEventListener(NavEventListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddNavEventListener(listener);
    }
}

void Navigation::RemoveNavEventListener(NavEventListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveNavEventListener(listener);
    }
}

void Navigation::AddSpecialRegionListener(SpecialRegionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddSpecialRegionListener(listener);
    }
}

void Navigation::RemoveSpecialRegionListener(SpecialRegionListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveSpecialRegionListener(listener);
    }
}

void Navigation::AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener)
{
    if (listener != NULL)
    {
        m_impl->AddEnhancedNavigationStartupListener(listener);
    }
}

void Navigation::RemoveEnhancedNavigationStartupListener(nbnav::EnhancedNavigationStartupListener* listener)
{
    if (listener != NULL)
    {
        m_impl->RemoveEnhancedNavigationStartupListener(listener);
    }
}

void Navigation::Announce()
{
    m_impl->Announce();
}

void Navigation::Announce(int manueverNumber)
{
    m_impl->Announce(manueverNumber);
}

void Navigation::Announce(int manueverNumber, const RouteInformation& route)
{
    m_impl->Announce(manueverNumber, route);
}

void Navigation::DoDetour()
{
    m_impl->DoDetour();
}

bool Navigation::SetActiveRoute(const RouteInformation& route)
{
    return m_impl->SetActiveRoute(route);
}

void Navigation::Recalculate()
{
    m_impl->Recalculate();
}

void Navigation::Recalculate(const RouteOptions& routeOptions, const Preferences& preference,
                             bool wantAlternateRoute)
{
    m_impl->Recalculate(routeOptions, preference, wantAlternateRoute);
}
    
void Navigation::Recalculate(const vector<WayPoint>& wayPoint)
{
    m_impl->Recalculate(&wayPoint);
}

void Navigation::CancelDetour()
{
    m_impl->CancelDetour();
}

void Navigation::PlanTrip(const Place& originPlace)
{
    m_impl->PlanTrip(originPlace);
}

}
