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
   @file        OnboardRouter.h
   @defgroup    navigation
*/
/*
   (C) Copyright 2015 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef _ONBOARDROUTE_H_
#define _ONBOARDROUTE_H_

#include "useonboard.h"
#include "nbtaskqueue.h"
#include "contextbasedsingleton.h"
extern "C"
{
#include "nbcontextprotected.h"
#include "nbroutehandlerprivate.h"
#include "nbroutehandlerprivate.h"
#include "data_nav_reply.h"
#include "pallock.h"
}

#include "nbmacros.h"
#include "NcdbTypes.h"
#include "Route.h"

namespace Ncdb
{
class Session;
class RouteManager;
}

typedef shared_ptr<Ncdb::Session> NcdbSessionPtr;

namespace nbnav
{
class OnboardRouteParameters;
typedef shared_ptr<OnboardRouteParameters> OnboardRouteParametersPtr;
class RouteRequestResult;
typedef shared_ptr<RouteRequestResult> RouteRequestResultPtr;


typedef void (*OnBoardRouterRequestCB)(NB_NavigateRouteError routeError, NB_Error err, data_nav_reply* navReply, void* userData);
struct OnBoardRouteRequest
{
    NB_RouteParameters*     parameters;
    OnBoardRouterRequestCB  requestCB;
    void*                   userData;
};

/**
 * OnboardRouter is the actual onboard navigation service provider
 */
class OnboardRouter
{
public:
    OnboardRouter(NB_Context* context);
    ~OnboardRouter();

    void Initialize();
    NB_Error StartRouteRequest(OnBoardRouteRequest request);
    void CancelRouteRequest();

    NB_Context* GetContext()
    {
        return m_pContext;
    }

    void SetNcdbSession(NcdbSessionPtr session)
    {
        m_pSession = session;
    }

    bool IsInitialized()
    {
        return m_initialized;
    }

private:
    void WorkerThread_StartRouteRequest(OnboardRouteParametersPtr params);
    void CCCThread_RequestComplete(RouteRequestResultPtr result);

    inline void ScheduleErrorCallback(OnboardRouteParametersPtr params,
                                      Ncdb::ReturnCode          calcResult);

    NB_Context*                    m_pContext;
    NcdbSessionPtr                 m_pSession;
    shared_ptr<WorkerTaskQueue>    m_pWorkTaskQueue;
    shared_ptr<EventTaskQueue>     m_pEventTaskQueue;
    shared_ptr<Ncdb::RouteManager> m_pRouteManager;
    bool                           m_initialized;
    OnboardRouteParametersPtr      m_currRequestParam;
    std::vector<std::string>       m_currentRouteIDs;
    Ncdb::AutoArray<Ncdb::Route>   m_currentRoutes;
    PAL_Lock*                      m_lock;
    NB_DISABLE_COPY(OnboardRouter)
};

typedef shared_ptr<OnboardRouter> OnboardRouterPtr;
}

#endif /* _ONBOARDROUTE_H_ */

/*! @} */
