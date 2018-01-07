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
    @file     NavApiRouteSummaryRequest.h
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

#ifndef __NAVAPIROUTESUMMARYREQUEST_H__
#define __NAVAPIROUTESUMMARYREQUEST_H__

#include "NavApiListeners.h"
#include "NavApiRouteSummaryInformation.h"
#include <vector>

extern "C"
{
#include "nbcontext.h"
}

namespace nbnav
{

class NavigationImpl;
class RouteSummaryRequest;
/**
 * Route Summary Listener
 * The listener is used to get route summary information from route summary request call back.
 */
class RouteSummaryListener
{
public:
    /**
     * On route call back.
     * Callback to provide the result of the route summary search.
     * @param information Route summary information contains result data.
     * @param request The route summary request instance.
     */
    virtual void OnRouteSummary(const std::vector<RouteSummaryInformation> /*information*/, RouteSummaryRequest* /*request*/){};

    /** Indicates that an error happened.
     *
     *  @param error The route error
     *  @return None
     */
    virtual void RouteError(NavigateRouteError /*error*/) {}
    virtual ~RouteSummaryListener() {}
};

/**
 * Route summary request
 * The class provides an API to get routes summary.
 */
class RouteSummaryRequest : SessionListener
{
public:
    /**
     * Construction
     * Create a new instance of route summary request construction.
     * @param context The nb context.
     * @param origin The origin place of navigation.
     * @param destinations The destination place's array of navigation.
     * @param listener The route summary listener of route summary request.
     */
    RouteSummaryRequest(NB_Context* context, const Place origin, std::vector<Place> destinations,
                        RouteSummaryListener* listener, const string& workFolder);
    /**
     * Deconstruction.
     */
    ~RouteSummaryRequest();

    /**
     * Start request.
     * Starts the route summary request with the parameters from construction.
     */
    void StartRequest();

    /**
     * Get request process
     * Checks if this request is in progress
     * @return true is in process, false is not.
     */
    bool IsRequestInProgress();

    /**
     * Cancels the request
     * The request can be canceled when it is in the request process.
     */
    void CancelRequest();

    /* session listener callbacks. */
    virtual void RouteReceived(RouteRequestReason reason, std::vector<RouteInformation>& routes);
    virtual void RouteError(NavigateRouteError error);
private:
    void ReleaseSession();

private:
    bool                  m_isRequested;
    RouteSummaryListener* m_listener;
    NavigationImpl*       m_navigation;
    Place                 m_origin;
};
}
#endif
