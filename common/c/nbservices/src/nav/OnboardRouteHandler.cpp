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
   @file        OnboardNavigationServiceProvider.cpp
   @defgroup    nbnav
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

#include "useonboard.h"
#include "OnboardRouteHandler.h"
#include "OnboardRouter.h"
extern "C"
{
#include "nbrouteinformationprivate.h"
#include "nbutility.h"
#include "nbcontextprotected.h"
}
#include "nbmacros.h"

// Hyrbid_NavigationServices will ensure input pointers are valid...

using namespace nbnav;

NB_Error OnboardRouteHandlerSetup(NB_Context*                context,
                                  NB_RouteHandler*           nbRouteHandler,
                                  OnboardRouteHandler*       handler)
{
    nsl_memset(&handler->navReply,0,sizeof(data_nav_reply));
    handler->nbRouteHandler = nbRouteHandler;
    handler->nbRouteHandler->context = context;
    return NE_OK;
}

NB_Error OnboardRouteHandlerTearDown(OnboardRouteHandler* handler)
{
    if(handler)
    {
        data_nav_reply_free(NB_ContextGetDataState(handler->nbRouteHandler->context),
                            &handler->navReply);
    }

    return NE_OK;
}

static void OnBoardRouterResult(NB_NavigateRouteError routeError, NB_Error err, data_nav_reply* navReply, void* userData)
{
    OnboardRouteHandler* handler = (OnboardRouteHandler*)userData;
    if(!handler->canceled)
    {
        data_nav_reply_free(NB_ContextGetDataState(handler->nbRouteHandler->context),&handler->navReply);
        data_nav_reply_copy(NB_ContextGetDataState(handler->nbRouteHandler->context),&handler->navReply,navReply);
        // NB_NavigateRouteError
        NB_RouteHandler* nbRouteHandler = handler->nbRouteHandler;
        nbRouteHandler->download_inprogress = FALSE;
        nbRouteHandler->download_percent = 100;
        nbRouteHandler->route_err = routeError;
        nbRouteHandler->callback.callback(nbRouteHandler,
                                            routeError?NB_NetworkRequestStatus_Failed:NB_NetworkRequestStatus_Success,
                                            err, FALSE, 100,
                                            nbRouteHandler->callback.callbackData);
    }
}

NB_Error OnboardRouteHandlerStartRequest(OnboardRouteHandler* handler)
{
    NB_RouteHandler* nbRouteHandler = handler->nbRouteHandler;
    OnboardRouterPtr router = ContextBasedSingleton<OnboardRouter>::getInstance(
        nbRouteHandler->context);
    if (!router->IsInitialized())
    {
        router->Initialize();
    }

    handler->canceled = false;
    NB_LogNavigationStart(nbRouteHandler, nbRouteHandler->parameters);

    OnBoardRouteRequest request;
    request.parameters = nbRouteHandler->parameters;
    request.requestCB = OnBoardRouterResult;
    request.userData = handler;
    return router->StartRouteRequest(request);
}

NB_Error OnboardRouteHandlerCancelRequest(OnboardRouteHandler* handler)
{
    OnboardRouterPtr router = ContextBasedSingleton<OnboardRouter>::getInstance(
        handler->nbRouteHandler->context);
    router->CancelRouteRequest();

    handler->canceled = true;
    handler->nbRouteHandler->download_inprogress = 0;
    return NE_OK;
}

NB_Error OnboardRouteHandlerGetRouteInformation(OnboardRouteHandler*  handler,
                                                NB_RouteInformation** route)
{
    NB_Error err = NE_OK;
    NB_RoutePrivateConfiguration config;
    int nman=0, n=0;
    NB_RouteInformation* newRoute = NB_RouteInformationAlloc(handler->nbRouteHandler->context);
    data_nav_maneuver* pman = NULL;
    data_util_state* dataState = NULL;
    NB_RouteSettings settings;
    nsl_memset(&settings, 0, sizeof(settings));
    nsl_memset(&config, 0, sizeof(config));

    if (newRoute == NULL)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(handler->nbRouteHandler->context);

    err = NB_RouteParametersClone(handler->nbRouteHandler->parameters, &newRoute->routeParameters);
    err = err ? err : NB_RouteParametersCopyPrivateConfigFromParameters(handler->nbRouteHandler->parameters,
                                                                        &config);

    NB_RouteParametersGetRouteSettingsFromQuery(handler->nbRouteHandler->parameters, &settings);
    NB_RouteInformationCopyRouteSettings(&newRoute->dataReply.route_settings, &settings);
    NB_RouteParametersCopyConfigFromParameters(handler->nbRouteHandler->parameters,
                                               &newRoute->parameterConfiguration);

    if (config.prefs.laneGuidanceFontMap)
    {
        CSL_HashTableDestroy(config.prefs.laneGuidanceFontMap, TRUE);
        config.prefs.laneGuidanceFontMap = NULL;
    }

    newRoute->config = config;
    data_nav_reply_copy(NB_ContextGetDataState(newRoute->context), &newRoute->dataReply, &handler->navReply);

    newRoute->transportMode = (newRoute->dataReply.route_settings.vehicle_type == NB_TransportationMode_Pedestrian) ? NB_NTM_Pedestrian : NB_NTM_Vehicle;
    newRoute->camerasOnRoute = FALSE;
    newRoute->collapseInformational = FALSE;
    newRoute->useFetchCorridor = TRUE;

    SetNIMLocationFromLocation(&newRoute->start.location, dataState, &newRoute->dataReply.origin_location);
    SetNIMLocationFromLocation(&newRoute->destination.location, dataState, &newRoute->dataReply.destination_location);

    if (newRoute->originFix.valid == 0)
    {
        newRoute->originFix.valid = NGV_Latitude | NGV_Longitude;
        newRoute->originFix.latitude = newRoute->start.location.latitude;
        newRoute->originFix.longitude = newRoute->start.location.longitude;
    }

    nman = data_nav_reply_num_nav_maneuver(dataState, &newRoute->dataReply);
    for (n = 0; n < nman; n++)
    {
        pman = data_nav_reply_get_nav_maneuver(dataState, &newRoute->dataReply, n);
        if(pman)
        {
            if(pman->current_roadinfo.unpaved)
            {
                newRoute->hasUnpavedRoads = TRUE;
            }
        }
    }

    newRoute->routeLength   = GetRouteLength(newRoute);
    GetRouteTimeAndDelay(newRoute, &newRoute->routeTime, &newRoute->routeDelay);

    newRoute->routeID.size = (nb_size)newRoute->dataReply.route_id.size;
    newRoute->routeID.data = newRoute->dataReply.route_id.data;

    data_nav_reply_get_route_count(dataState, &newRoute->dataReply, &newRoute->numberOfSelector);

    *route = newRoute;
    handler->nbRouteHandler->route = *route;

    if(err == NE_OK)
    {
        UpdateRouteInformationDescription(handler->nbRouteHandler->context, *route, NULL);
    }

    return err;
}
/*! @} */
