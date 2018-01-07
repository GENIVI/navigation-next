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
   @file        nbroutehandlerprivate.h
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

#ifndef _NBROUTEHANDLERPRIVATE_H_
#define _NBROUTEHANDLERPRIVATE_H_

#include "nbroutehandler.h"
#include "cslnetwork.h"

typedef struct RouteHandler_NetQuery
{
    NB_NetworkQuery     net_query;
    NB_RouteHandler*    pThis;
} RouteHandler_NetQuery;

struct NB_RouteHandler
{
    NB_Context*               context;
    NB_RouteParameters*       parameters;
    NB_RequestHandlerCallback callback;
    RouteHandler_NetQuery     netquery;
    boolean                   download_inprogress;
    int                       download_percent;
    NB_NavigateRouteError     route_err;
    tpselt                    tpsReply;
    NB_RouteInformation*      route;
};

typedef struct
{
    char name[64];
    double distance;
    nb_boolean checked;
} ManeuverDistanceInfo;

typedef struct
{
    ManeuverDistanceInfo* maneuvers;
    uint32 size;
    nb_boolean stopped;
} RouteDistanceInfo;

NB_Error NB_CreateAndUpdateRouteInformation(NB_RouteHandler*      handler,
                                            NB_RouteInformation** route);
NB_Error UpdateRouteInformationDescription(NB_Context* context,
                                           NB_RouteInformation* routeInfo,
                                           NB_RouteInformation* routeInfo2);
void NB_LogNavigationStart(NB_RouteHandler* pThis, NB_RouteParameters* parameters);

#endif /* _NBROUTEHANDLERPRIVATE_H_ */

/*! @} */
