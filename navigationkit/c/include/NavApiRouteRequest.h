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
    @file     NavApiRouteRequest.h
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


#ifndef __NAVAPIROUTEREQUEST_H__
#define __NAVAPIROUTEREQUEST_H__

#include "NavApiListeners.h"
#include "NavApiRouteOptions.h"

extern "C"
{
#include "nbcontext.h"
#include "paltaskqueue.h"
}

namespace nbnav
{

class NavigationImpl;
class RouteRequest;

/*! Routelistener

    The listener is used to get route information from static route request call back.
*/
class RouteListener
{
public:
    /*! On route call back.

        Call back to provide the result of the route search. The number of RouteInformation elements
        in the vector depends on the multipleRoutes parameter in the request. If route request fail,
        the size of information would be 0 and also you can check IsRequestInProgress of request instance.

        @param information  The route information is gotten from static route request.
        @param request  The route request instance.
        @return None
    */
    virtual void OnRoute(vector<RouteInformation>& information, RouteRequest* request){};

    /** Indicates that an error happened.
     *
     *  @param error The route error
     *  @return None
     */
    virtual void RouteError(NavigateRouteError error) {}
    virtual ~RouteListener(){};
};

/*! Routerequest class

    The Route class provides an API to get static routes.
*/
class RouteRequest : public SessionListener
{
public:
    /*! Construction

        Creates a new instance of route request. If multipleRoutes is TRUE, then the request will return
        alternate routes like the Navigation Session API

        @param context The nb context.
        @param origin The origin place of navigation.
        @param destination The destination place of navigation.
        @param routeOptions The route options of navigation.
        @param listener The route listener of route request.
        @param multipleRoutes true for multiple routes, false for single route.
    */
    RouteRequest(NB_Context* context, const Place& origin, const Place& destination,const vector<WayPoint> wayPoint,
                 const RouteOptions& routeOptions, RouteListener* listener, bool multipleRoutes = false, const string workFolder = "");

    /*! Deconstruction */
    ~RouteRequest();

    /*! Start request.

        Starts the static route request with the parameters from construction.

        @return None
    */
    void StartRequest();

    /*! Get request process

        Checks if this request is in progress

        @return true is in process, false is not.
    */
    bool IsRequestInProgress();

    /*! Cancels the request

        The request can be canceled when it is in the request process.

        @return None
    */
    void CancelRequest();

    /*! session listener callbacks. */
    virtual void RouteReceived(RouteRequestReason reason, vector<RouteInformation>& routes);
    virtual void RouteError(NavigateRouteError error);

private:
    bool            m_isRequested;
    RouteListener*  m_listener;
    NavigationImpl* m_navigation;
    Place           m_origin;
};
}
#endif

/*! @} */