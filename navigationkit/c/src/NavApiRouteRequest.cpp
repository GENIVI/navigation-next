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
    @file     NavApiRouteRequest.cpp
*/
/*
 * (C) Copyright 2013 by TeleCommunication Systems, Inc.
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

#include "NavApiRouteRequest.h"
#include "NavigationImpl.h"
#include <vector>

namespace nbnav
{
RouteRequest::RouteRequest(NB_Context* context, const Place& origin, const Place& destination,const vector<WayPoint>
                           wayPoint, const RouteOptions& routeOptions, RouteListener* listener, bool multipleRoutes , const string workFolder)
                           : m_isRequested(false), m_listener(listener), m_navigation(NULL),
                             m_origin(origin)

{
    Preferences preference;
    preference.SetMultipleRoutes(multipleRoutes);
    std::vector<Place> destinations;
    destinations.push_back(destination);
    m_navigation = new NavigationImpl(context, destinations, routeOptions, preference, &wayPoint);
    m_navigation->SetWorkFolder(workFolder);
    m_navigation->Init();
}

RouteRequest::~RouteRequest()
{
    if (m_isRequested)
    {
        m_navigation->RemoveSessionListener(this);
    }
    m_navigation->Release();
}

void RouteRequest::StartRequest()
{
    if (!m_isRequested)
    {
        m_navigation->AddSessionListener(this);
        m_navigation->StartStaticRouteRequest(m_origin);
        m_isRequested = true;
    }
}

bool RouteRequest::IsRequestInProgress()
{
    return m_isRequested;
}

void RouteRequest::CancelRequest()
{
    if (m_isRequested)
    {
        m_navigation->RemoveSessionListener(this);
        m_navigation->StopSession();
        m_navigation->Release();
        m_isRequested = false;
    }
}

void RouteRequest::RouteReceived(RouteRequestReason reason, vector<RouteInformation>& routes)
{
    if (m_listener)
    {
        m_listener->OnRoute(routes, this);
    }
}

void RouteRequest::RouteError(NavigateRouteError error)
{
    if (m_listener)
    {
        m_listener->RouteError(error);
    }
}
}
