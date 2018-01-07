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

#include "Detour.h"
#include "NavigationImpl.h"
extern "C"
{
#include "nbgpstypes.h"
#include "nbcontextaccess.h"
#include "paltaskqueue.h"
#include "palfile.h"
}
#include "palmath.h"

#include <functional>
#include <algorithm>

namespace nbnav
{

Detour::Detour(NavigationImpl* _impl)
    : m_impl(_impl),
      m_requestingDetour(true),
      m_routeHandler(NULL),
      m_route(NULL),
      m_trafficInfo(NULL),
      m_cameraInfo(NULL),
      m_destorying(FALSE)
{
    NB_Error error = DetourRouteParameters();

    SessionListener::RouteRequestReason* data = new SessionListener::RouteRequestReason(SessionListener::Detour);
    m_impl->m_notifier.RouteRequested(data);

    if (error != NE_OK)
    {
        NotifyError(NRE_UnknownError);
    }
}

Detour::~Detour()
{
    m_destorying = TRUE;
    ClearDetour(false);
}

bool Detour::ApplyDetour(const RouteInformation& route)
{
    if (AcceptDetour(route) != NE_OK)
    {
        ClearDetour(true);
        return false;
    }
    return true;
}

NB_Error
Detour::DetourRouteParameters()
{
    NB_Error error = NE_OK;
    NB_DetourParameters* detourParameters = NULL;

    error = NB_DetourParametersCreate(m_impl->m_context, &detourParameters);

    if (!error)
    {
        error = RequestDetour(detourParameters);
    }

    if (detourParameters)
    {
        NB_DetourParametersDestroy(detourParameters);
    }

    return error;
}

NB_Error
Detour::RequestDetour(NB_DetourParameters* detourParameters)
{
    NB_Error error = NE_OK;
    NB_RouteParameters* pParameters = NULL;
    NB_RouteInformation* pRoute = NULL;
    NB_TrafficInformation* pTraffic = NULL;
    NB_Context* pContext = m_impl->m_context;
    NB_GpsHistory* pHistory = NB_ContextGetGpsHistory(pContext);

    NB_RequestHandlerCallback detourHandlerCallback = {(NB_RequestHandlerCallbackFunction)Static_CallbackDetourRoute, 0};
    detourHandlerCallback.callbackData = static_cast<void*>(this);

    error = NB_NavigationGetInformation(m_impl->GetNbNavigation(), &pRoute, &pTraffic, NULL, NULL);

    if (!error)
    {
        error = NB_RouteParametersCreateDetour(pContext, pRoute, pTraffic, detourParameters,
                                               pHistory, &pParameters);
    }

    if (!error)
    {
        error = NB_RouteHandlerCreate(pContext, &detourHandlerCallback, &(m_routeHandler));
    }

    if (!error)
    {
        error = NB_RouteHandlerStartRequest(m_routeHandler, pParameters);
        if (error == NE_OK)
        {
            m_impl->AddAnalyticsDetourRequest();
        }
    }

    if (pParameters)
    {
        NB_RouteParametersDestroy(pParameters);
    }

    return error;
}

NB_Error Detour::ClearDetour(bool noDestroy)
{
    NB_Error res = NE_OK;
    if(m_routeHandler != NULL)
    {
        NB_Error err = NB_RouteHandlerDestroy(m_routeHandler);
        m_routeHandler = NULL;
        res = (res == NE_OK) ? err : res;
    }
    if (!noDestroy)
    {
        // route is destroyed in NavImpl
        if(m_trafficInfo != NULL)
        {
            NB_Error err = NB_TrafficInformationDestroy(m_trafficInfo);

            res = (res == NE_OK) ? err : res;
        }
        if(m_cameraInfo != NULL)
        {
            NB_Error err = NB_CameraInformationDestroy(m_cameraInfo);

            res = (res == NE_OK) ? err : res;
        }
    }
    m_trafficInfo = NULL;
    m_cameraInfo = NULL;
    m_route = NULL;

    return res;
}

void
Detour::Static_CallbackDetourRoute(void* handler, NB_RequestStatus status, NB_Error err,
                                   uint8 up, int percent, void* pUserData)
{
    if (up || pUserData == NULL)
    {
        return;
    }

    Detour* me = (Detour*)pUserData;
    
    if(me->m_destorying)
    {
        return;
    }

    if (err != NE_OK)
    {
        me->NotifyError(NRE_NetError);
        return;
    }

    switch (status)
    {
        case NB_NetworkRequestStatus_Success:
        {
            me->m_requestingDetour = false;
            NB_Error error = me->UpdateDetour();
            if (error == NE_OK)
            {
                me->m_impl->AddAnalyticsDetourReply();
                me->m_impl->AddAnalyticsNavigationState("new-route");
                me->m_impl->UpdateNbRoutes(me->m_route, true);
            }
            else
            {
                me->NotifyError(NRE_NetError);
            }
            break;
        }
        case NB_NetworkRequestStatus_Failed:
        {
            me->m_requestingDetour = false;
            NB_NavigateRouteError nbRouteError = NB_NRE_None;
            err = NB_RouteHandlerGetRouteError(me->m_routeHandler, &nbRouteError);
            if(err != NE_OK)
            {
                me->NotifyError(NRE_NetError);
            }
            else if(nbRouteError != NB_NRE_None)
            {
                me->NotifyError(me->m_impl->NbRouteErrorToRouteError(nbRouteError));
            }
            break;
        }
        case NB_NetworkRequestStatus_Canceled:
        {
            me->m_requestingDetour = false;
            me->NotifyError(NRE_Cancelled);
            break;
        }
        case NB_NetworkRequestStatus_TimedOut:
        {
            me->m_requestingDetour = false;
            me->NotifyError(NRE_TimedOut);
            break;
        }
        case NB_NetworkRequestStatus_Progress:
        {
            int* data = new int(percent);
            me->m_impl->m_notifier.RouteProgress(data);
            break;
        }
        default:
        {
            nsl_assert(false);
            break;
        }
    }
}
void Detour::NotifyError(NavigateRouteError error)
{
    m_impl->SetRouteError(error);
}

NB_Error
Detour::UpdateDetour()
{
    NB_Error error = NE_OK;

    error = NB_RouteHandlerGetRouteInformation(m_routeHandler, &(m_route));

    if (!error)
    {
        error = NB_RouteHandlerGetTrafficInformation(m_routeHandler, &(m_trafficInfo));
    }

    return error;
}

NB_Error
Detour::AcceptDetour(const RouteInformation& routeInfo)
{
    NB_Error error = NE_OK;

    if (m_trafficInfo)
    {
        error = NB_TrafficInformationDestroy(m_trafficInfo);
        m_trafficInfo = NULL;
        error = error ? error : NB_RouteInformationSetActiveRoute(m_route, m_impl->GetNbRouteIndex(routeInfo) + 1);
        error = error ? error : NB_RouteInformationGetTrafficInformation(m_route, &m_trafficInfo);
    }
    error = error ? error : NB_NavigationNewRoute(m_impl->GetNbNavigation(), m_route, m_trafficInfo, m_cameraInfo);
    m_trafficInfo = NULL;
    if (error == NE_OK)
    {
        NB_NavigationResetEnhancedContentMapState(m_impl->GetNbNavigation());
        if (m_impl->m_activeRoute)
        {
            m_impl->GetNbRoute(*m_impl->m_activeRoute);
        }

        NB_RouteInformation* nbRouteInformation = m_impl->GetNbRoute(routeInfo);
        vector<NB_RouteInformation*>::iterator iterFind = find(m_impl->m_nbDetours.begin(),
                                                               m_impl->m_nbDetours.end(), nbRouteInformation);
        if (iterFind != m_impl->m_nbDetours.end())
        {
            m_impl->m_nbDetours.erase(iterFind);
        }
        m_impl->ClearNbRoutes();
        m_impl->m_nbRoutes.push_back(nbRouteInformation);
        m_impl->ClearActiveRoute();
        m_impl->m_activeRoute = new RouteInformation(routeInfo);
        m_impl->m_activeRoute->m_index = 0;
        m_impl->UpdateManeuverList();
    }

    return error;
}

bool
Detour::IsRequesting()
{
    return m_requestingDetour;
}

}
