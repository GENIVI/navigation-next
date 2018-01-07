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
    @file     NavApiRouteSummaryRequest.cpp
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

#include "NavApiRouteSummaryRequest.h"
#include "NavApiPreferences.h"
#include "NavigationImpl.h"
#include <vector>
extern "C"
{
#include "nbrouteinformation.h"
}

namespace nbnav
{
using namespace std;

RouteSummaryRequest::RouteSummaryRequest(NB_Context* context, const Place origin,
                                         const std::vector<Place> destinations, RouteSummaryListener* listener, const string &workFolder)
    : m_isRequested(false),
    m_listener(listener),
    m_navigation(NULL),
    m_origin(origin)
{
    Preferences preference;
    RouteOptions routeOption(Fastest, Car, 0);
    m_navigation = new NavigationImpl(context, destinations, routeOption, preference);
    m_navigation->SetWorkFolder(workFolder);
    m_navigation->Init();
}

RouteSummaryRequest::~RouteSummaryRequest()
{
    if(m_navigation)
    {
        if (m_isRequested)
        {
            m_navigation->RemoveSessionListener(this);
        }
        m_navigation->Release();
    }
}

void RouteSummaryRequest::StartRequest()
{
    if (!m_isRequested && m_navigation)
    {
        m_navigation->AddSessionListener(this);
        m_navigation->StartRouteSummaryRequest(m_origin);
        m_isRequested = true;
    }
}

bool RouteSummaryRequest::IsRequestInProgress()
{
    return m_isRequested;
}

void RouteSummaryRequest::CancelRequest()
{
    if (m_isRequested)
    {
        ReleaseSession();
    }
}

void RouteSummaryRequest::RouteReceived(RouteRequestReason reason, vector<RouteInformation>& routes)
{
    if (m_listener)
    {
        if(routes.size() > 0)
        {
            m_listener->OnRouteSummary(routes[0].GetRouteSummary(), this);
        }
    }
    if(m_navigation)
    {
        ReleaseSession();
    }
}

void RouteSummaryRequest::RouteError(NavigateRouteError error)
{
    if (m_listener)
    {
        m_listener->RouteError(error);
    }
    if(m_navigation)
    {
        ReleaseSession();
    }
}
    
void RouteSummaryRequest::ReleaseSession()
{
    if(m_navigation)
    {
        m_navigation->RemoveSessionListener(this);
        m_navigation->StopSession();
        m_navigation->Release();
        m_isRequested = false;
        m_navigation = NULL;
    }
}
}
