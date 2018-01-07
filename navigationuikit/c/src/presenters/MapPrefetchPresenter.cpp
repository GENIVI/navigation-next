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
    @file         MapPrefetchPresenter.cpp
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#include <memory>
#include "MapPrefetchPresenter.h"
#include "NavApiNavUtils.h"

#define PREFETCH_ZOOM_LEVEL     18.f
#define MAX_PREFETCH_GRADE      2

static double s_prefetchWindow[MAX_PREFETCH_GRADE + 1] = {1200.0, 3000.0, 6000.0};
static double s_prefetchWindowOverlap[MAX_PREFETCH_GRADE + 1] = {300.0, 800.0, 1400.0};
static double s_prefetchWindowRTS =  1609.344; //1 mile = 1609.344 meter

using namespace nkui;
using namespace nbnav;

MapPrefetchPresenter::MapPrefetchPresenter()
    : m_nextPrefetchDistance(0.0),
      m_currentPrefetchGrade(0),
      m_prefetchZoomLevel(PREFETCH_ZOOM_LEVEL),
      m_bOnRoute(false)
{
}

MapPrefetchPresenter::~MapPrefetchPresenter()
{
}

void MapPrefetchPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_ROUTE_UPDATE:
        {
            const vector<RouteInformation>& allRoutes = m_pNavUIContex->GetAllRoutes();
            if (allRoutes.empty())
            {
                return;
            }
            /*! if in RTS or Detour, we will prefetch all routes but current route,
                in navigation, we just reset prefetch status if current route changed  */
            /*! eventData here is reason of EVT_ROUTE_UPDATE, NULL for button start pressed and others
                for route request reason defined in nbnav::SessionListener::RouteRequestReason */
            if (data.eventData == NULL)
            {
                const string& routeID = allRoutes[allRoutes.size()-1].GetRouteID();
                if (m_currentRouteID != routeID)
                {
                    m_currentRouteID = routeID;
                    ResetPrefetchStatus();
                }
            }
            else
            {
                switch (*(nbnav::SessionListener::RouteRequestReason*)data.eventData)
                {
                    case nbnav::SessionListener::Calculate:
                    case nbnav::SessionListener::RouteSelector:
                    {
                        vector<nbnav::RouteInformation>::const_iterator iter = allRoutes.begin();
                        while (iter != allRoutes.end())
                        {
                            Prefetch(*iter, 0.0, s_prefetchWindowRTS);
                            iter++;
                        }
                        break;
                    }
                    case nbnav::SessionListener::Detour:
                    {
                        for (int i = 0; i < allRoutes.size() - 1; ++i)
                        {
                            Prefetch(allRoutes[i], 0.0, s_prefetchWindowRTS);
                        }
                        break;
                    }
                    case nbnav::SessionListener::Recalculate:
                    {
                        m_currentRouteID = allRoutes[0].GetRouteID();
                        ResetPrefetchStatus();
                    }
                    default:
                        break;
                }
            }
            break;
        }
        case EVT_NAV_OFFROUTE:
        {
            m_bOnRoute = false;
            break;
        }
        case EVT_NAV_ONROUTE:
        {
            m_bOnRoute = true;
            break;
        }
        case EVT_MAP_ZOOM_UPDATED:
            m_prefetchZoomLevel = *(float*)(data.eventData);
        default:
            break;
    }
}

void MapPrefetchPresenter::HandleEvent(NKEventID id, void* data)
{

}

void MapPrefetchPresenter::OnActive()
{
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNKUIController->RegisterEvent(EVT_ROUTE_UPDATE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_OFFROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_ONROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_MAP_ZOOM_UPDATED, this);
}

void MapPrefetchPresenter::OnDeactivate()
{
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
}

void MapPrefetchPresenter::OnSetWidget()
{

}

void MapPrefetchPresenter::TripRemainingDistance(double remaingDistance)
{
    const vector<RouteInformation>& allRoutes = m_pNavUIContex->GetAllRoutes();
    //@todo: check if need to do pre-fetch in TripOverview....
    if (!allRoutes.empty() && m_pNavUIContex->GetMapSetting().trackingMode != NCTM_Free)
    {
        const nbnav::RouteInformation& route         = allRoutes[allRoutes.size()-1];
        double                         totalDistance = route.GetDistance();
        double                         tripDistance  = totalDistance - remaingDistance;
        if (tripDistance < m_nextPrefetchDistance)
        {
            return;
        }

        double prefetchDistance = tripDistance + m_nextPrefetchDistance +
                                  s_prefetchWindow[m_currentPrefetchGrade];
        Prefetch(route, tripDistance + m_nextPrefetchDistance, prefetchDistance);
        m_nextPrefetchDistance  = prefetchDistance -
                                  s_prefetchWindowOverlap[m_currentPrefetchGrade];
        if (m_currentPrefetchGrade < MAX_PREFETCH_GRADE)
        {
            ++m_currentPrefetchGrade;
        }
    }
}

void MapPrefetchPresenter::ResetPrefetchStatus()
{
    m_nextPrefetchDistance = 0.0;
    m_currentPrefetchGrade = 0;
}

void MapPrefetchPresenter::Prefetch(const nbnav::RouteInformation& route, double fromDistance, double toDistance)
{
    const vector<nbnav::Coordinates>& polyline = route.GetPolyline();
    vector<NKUICoordinate> subPolyline;
    SubPolyline(polyline, fromDistance, toDistance, subPolyline);
    NKUIMapInterface* pMap = m_pNavUIContex->GetMapInterface();
    if (pMap != NULL && subPolyline.size() > 1)
    {
        pMap->Prefetch(subPolyline, m_prefetchZoomLevel);
        NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),"Do Prefetch from distance: %f m, to distance: %f m with ZoomLevel: %f",
                   fromDistance, toDistance, m_prefetchZoomLevel);
        NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),"Prefetch start coordinate: %f, %f ; end coordinate: %f, %f .",
                       subPolyline.front().m_latitude, subPolyline.front().m_longitude,
                       subPolyline[subPolyline.size()-1].m_latitude, subPolyline[subPolyline.size()-1].m_longitude);
    }
}

void MapPrefetchPresenter::SubPolyline(const vector<nbnav::Coordinates>& polyline,
                                       double fromDistance, double toDistance,
                                       std::vector<NKUICoordinate>& subPolyline)
{
    if (polyline.size() < 2)
    {
        return;
    }
    double distance = 0.0;
    nbnav::Coordinates currentPoint = polyline[0];
    for (size_t i = 1; i < polyline.size(); ++i)
    {
        const nbnav::Coordinates& nextPoint = polyline[i];
        double length = nbnav::NavUtils::CalculateDistance(currentPoint.latitude,
                                                           currentPoint.longitude,
                                                           nextPoint.latitude,
                                                           nextPoint.longitude,
                                                           NULL);
        if (distance >= fromDistance)
        {
            if (distance > toDistance)
            {
                break;
            }
            NKUICoordinate coordinate(currentPoint.latitude, currentPoint.longitude);
            subPolyline.push_back(coordinate);
        }
        distance += length;
        currentPoint = nextPoint;
    }
}

/*! @} */
