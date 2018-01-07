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

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbrouteinformation.h"
#include "nbrouteinformationprivate.h"
#include "cslnetwork.h"
#include "data_nav_query.h"
#include "data_nav_reply.h"
#include "nbutility.h"
#include "nbrouteparameters.h"
#include "nbrouteparametersprivate.h"
#include "nbtrafficinformation.h"
#include "nbtrafficinformationprivate.h"
#include "nbcamerainformation.h"
#include "nbcamerainformationprivate.h"
#include "nbqalog.h"
#include "nbnavqalog.h"
#include "palmath.h"
#include "csdict.h"
#include "nbroutehandlerprivate.h"
#include "offboardroutehandler.h"

#define MAX_DESCRIPTION_ROAD_COUNT 3

static void RouteHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void RouteHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

static void AppendManeuverDistanceInfo(data_util_state* pds, RouteDistanceInfo* info, data_nav_maneuver* maneuver, struct CSL_Dictionary* dict);
static void SortManeuverDistanceInfo(RouteDistanceInfo* info);
static void FindRouteDescriptions(RouteDistanceInfo* infos, uint32 routeCount, uint32 maxDescriptionCount);
static uint32 PartionManeuverDistanceInfo(ManeuverDistanceInfo* maneuvers, int32 startIndex, int32 endIndex);
static void QuickSortManeuverDistanceInfo(ManeuverDistanceInfo* maneuvers, int32 startIndex, int32 endIndex);
static void SwitchManeuverDistanceInfo(ManeuverDistanceInfo* maneuver1, ManeuverDistanceInfo* maneuver2);
static NB_Error AllocManeuverDistanceInfo(ManeuverDistanceInfo** maneuvers, uint32 maneuverCount, struct CSL_Dictionary** dict);

NB_Error
OffboardRouteHandlerSetup(NB_Context* context,
                          NB_RouteHandler* handler)
{
    nsl_memset(&handler->netquery, 0, sizeof(RouteHandler_NetQuery));
    handler->context = context;
    handler->netquery.net_query.action = RouteHandler_NetworkCallback;
    handler->netquery.net_query.progress = RouteHandler_NetworkProgressCallback;
    handler->netquery.net_query.qflags = TN_QF_DEFLATE;
    handler->netquery.net_query.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(handler->netquery.net_query.target,
                NB_ContextGetTargetMapping(handler->context, "nav"),
                sizeof(handler->netquery.net_query.target));
    handler->netquery.net_query.targetlen = -1;
    handler->netquery.pThis = handler;

    return NE_OK;
}

NB_Error OffboardRouteHandlerTearDown(NB_RouteHandler* handler)
{
    return NE_OK;
}

NB_Error OffboardRouteHandlerStartRequest(NB_RouteHandler* handler)
{
    NB_Error err = NE_OK;
    NB_LogNavigationStart(handler, handler->parameters);

    te_dealloc(handler->netquery.net_query.query);
    handler->netquery.net_query.query = NB_RouteParametersToTPSQuery(handler->parameters);
    if (handler->netquery.net_query.query == NULL)
    {
        return NE_NOMEM;
    }

    CSL_NetworkRequest( NB_ContextGetNetwork(handler->context), &handler->netquery.net_query);

    err = NE_OK;

    return err;
}

NB_Error OffboardRouteHandlerCancelRequest(NB_RouteHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(handler->context);

    // Don't cancel if not in progress, or if in callback (tpsReply is not null)
    if (!handler->download_inprogress || handler->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &handler->netquery.net_query);

    return NE_OK;
}

NB_Error OffboardRouteHandlerGetRouteInformation(NB_RouteHandler* handler, NB_RouteInformation** route)
{
    return NB_CreateAndUpdateRouteInformation(handler, route);
}

static void RouteHandler_NetworkCallback(NB_NetworkQuery* query,
                                         NB_NetworkResponse* resp)
{
    NB_Error err = NE_OK;
    NB_RouteHandler* handler = ((RouteHandler_NetQuery*)query)->pThis;
    NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;
    tpselt	te = NULL;
    uint32 replyerr = NE_OK;

    replyerr = GetReplyErrorCode(resp);

    handler->download_percent = 100;
    handler->route_err = NB_NRE_None;

    status = TranslateNetStatus(resp->status);

    if (status == NB_NetworkRequestStatus_Success)
    {
        te = te_getchild(resp->reply, "route-error");

        if (te != NULL)
        {
            const char* type = NULL;
            handler->route_err = NB_NRE_UnknownError;
            type = te_getattrc(te, "type");

            if (nsl_strcmp(type, "bad-origin") == 0)
            {
                handler->route_err = NB_NRE_BadOrigin;
            }
            else if (nsl_strcmp(type, "bad-destination") == 0)
            {
                handler->route_err = NB_NRE_BadDestination;
            }
            else if (nsl_strcmp(type, "no-route") == 0)
            {
                handler->route_err = NB_NRE_CannotRoute;
            }
            else if (nsl_strcmp(type, "no-detour") == 0)
            {
                handler->route_err = NB_NRE_NoDetour;
            }
            else if((nsl_strcmp(type, "route-too-long") == 0) ||
                    (nsl_strcmp(type, "too-long-route") == 0) ||
                    (nsl_strcmp(type, "ped-route-too-long") == 0))
            {
                NB_RouteSettings routeSetting = {0};
                NB_RouteParametersGetRouteSettingsFromQuery(handler->parameters, &routeSetting);
                if (routeSetting.vehicle_type == NB_TransportationMode_Pedestrian)
                {
                    handler->route_err = NB_NRE_PedRouteTooLong;
                }
                else if (routeSetting.vehicle_type == NB_TransportationMode_Bicycle)
                {
                    handler->route_err = NB_NRE_BicycleRouteTooLong;
                }
                else
                {
                    handler->route_err = NB_NRE_RouteTooLong;
                }
            }
            else if (nsl_strcmp(type, "origin-not-in-supported-country") == 0)
            {
                handler->route_err = NB_NRE_OriginCountryUnsupported;
            }
            else if (nsl_strcmp(type, "destination-not-in-supported-country") == 0)
            {
                handler->route_err = NB_NRE_DestinationCountryUnsupported;
            }
            status = NB_NetworkRequestStatus_Failed;
        }
    }
    else if (status == NB_NetworkRequestStatus_TimedOut)
    {
        handler->route_err = NB_NRE_TimedOut;
    }
    else if (status == NB_NetworkRequestStatus_Failed)
    {
        handler->route_err = NB_NRE_NetError;
    }

    if (handler->callback.callback != NULL)
    {
        handler->tpsReply = resp->reply;
        (handler->callback.callback)(handler, status, replyerr ? replyerr : err, FALSE, 100, handler->callback.callbackData);
        handler->tpsReply = 0;
    }

    handler->route = NULL;
    query->query = NULL;
    handler->download_inprogress = FALSE;
}

static void RouteHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    NB_RouteHandler* handler = ((struct RouteHandler_NetQuery*) query)->pThis;

    if (handler->callback.callback)
    {
        int percent = (cur * 100) / total;
        (handler->callback.callback)(handler, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, handler->callback.callbackData);
    }
}

static const char SPLIT_STRING[] = ", ";

static void
AppendManeuverDistanceInfo(data_util_state* pds, RouteDistanceInfo* info, data_nav_maneuver* maneuver, struct CSL_Dictionary* dict)
{
    size_t valueSize = 0;
    ManeuverDistanceInfo* foundManeuver = NULL;

    if (!pds || !info || !maneuver || !dict)
    {
        return;
    }
    foundManeuver = (ManeuverDistanceInfo*)CSL_DictionaryGet(dict,
                                                            data_string_get(pds, &maneuver->current_roadinfo.primary),
                                                            &valueSize);

    if (foundManeuver)
    {
        foundManeuver->distance += maneuver->distance;
    }
    else
    {
        info->maneuvers[info->size].distance = maneuver->distance;
        nsl_strlcpy(info->maneuvers[info->size].name,
                    data_string_get(pds, &maneuver->current_roadinfo.primary),
                    sizeof(info->maneuvers[info->size].name));
        CSL_DictionarySet(dict,
                          data_string_get(pds, &maneuver->current_roadinfo.primary),
                          (const char*)&info->maneuvers[info->size],
                          sizeof(ManeuverDistanceInfo));
        (info->size)++;
    }
}

static void
SortManeuverDistanceInfo(RouteDistanceInfo* info)
{
    if (!info || !info->maneuvers)
    {
        return;
    }
    QuickSortManeuverDistanceInfo(info->maneuvers, 0, info->size - 1);
}

static uint32
PartionManeuverDistanceInfo(ManeuverDistanceInfo* maneuvers, int32 startIndex, int32 endIndex)
{
    int32 i = 0;
    int32 keyIndex = endIndex;
    double keyValue = 0;

    if (endIndex - startIndex > 2)
    {
        double average = (maneuvers[startIndex].distance + maneuvers[startIndex + 1].distance + maneuvers[startIndex + 2].distance) / 3;
        double differ = average;
        keyIndex = 0;
        for (i = startIndex; i < startIndex + 2; i++)
        {
            if (nsl_fabs(maneuvers[i].distance - average) < differ)
            {
                differ = nsl_fabs(maneuvers[i].distance - average);
                keyIndex = i;
            }
        }
    }

    if (keyIndex != endIndex)
    {
        SwitchManeuverDistanceInfo(&maneuvers[keyIndex], &maneuvers[endIndex]);
    }

    keyIndex = startIndex;
    keyValue = maneuvers[endIndex].distance;

    for (i = startIndex; i < endIndex; i++)
    {
        if (maneuvers[i].distance > keyValue)
        {
            SwitchManeuverDistanceInfo(&maneuvers[i], &maneuvers[keyIndex]);
            keyIndex++;
        }
    }

    SwitchManeuverDistanceInfo(&maneuvers[keyIndex], &maneuvers[endIndex]);

    return keyIndex;
}

static void
QuickSortManeuverDistanceInfo(ManeuverDistanceInfo* maneuvers, int32 startIndex, int32 endIndex)
{
    if (startIndex < endIndex)
    {
        uint32 keyIndex = PartionManeuverDistanceInfo(maneuvers, startIndex, endIndex);
        QuickSortManeuverDistanceInfo(maneuvers, startIndex, keyIndex - 1);
        QuickSortManeuverDistanceInfo(maneuvers, keyIndex + 1, endIndex);
    }
}

static void
SwitchManeuverDistanceInfo(ManeuverDistanceInfo* maneuver1, ManeuverDistanceInfo* maneuver2)
{
    if (maneuver1 != maneuver2)
    {
        ManeuverDistanceInfo tempManeuverInfo;
        nsl_memset(&tempManeuverInfo, 0, sizeof(tempManeuverInfo));
        nsl_strlcpy(tempManeuverInfo.name, maneuver1->name, sizeof(tempManeuverInfo.name));
        tempManeuverInfo.distance = maneuver1->distance;
        nsl_strlcpy(maneuver1->name, maneuver2->name, sizeof(maneuver1->name));
        maneuver1->distance = maneuver2->distance;
        nsl_strlcpy(maneuver2->name, tempManeuverInfo.name, sizeof(maneuver2->name));
        maneuver2->distance = tempManeuverInfo.distance;
    }
}


static void
FindRouteDescriptions(RouteDistanceInfo* infos, uint32 routeCount, uint32 maxDescriptionCount)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 maxManeuverCount = 0;
    uint32 findIndex = 0;
    nb_boolean isOdd = FALSE;
    nb_boolean isAllStopped = FALSE;

    for (i = 0; i < routeCount; i++)
    {
        if (maxManeuverCount < infos[i].size)
        {
            maxManeuverCount = infos[i].size;
        }
    }

    while (!isAllStopped && findIndex < maxManeuverCount)
    {
        for (i = 0; i < routeCount; i++)
        {
            if (findIndex >= infos[i].size)
            {
                infos[i].stopped = TRUE;
                infos[i].maneuvers[findIndex].checked = TRUE;
            }
            if (infos[i].stopped) continue;
            isOdd = TRUE;

            for (j = 0; j < routeCount; j++)
            {
                if (findIndex >= infos[j].size)
                {
                    infos[j].stopped = TRUE;
                    infos[j].maneuvers[findIndex].checked = TRUE;
                }
                if (infos[j].stopped || i == j) continue;

                if (nsl_strcmp(infos[i].maneuvers[findIndex].name,
                    infos[j].maneuvers[findIndex].name) == 0)
                {
                    isOdd = FALSE;
                    break;
                }
            }

            if (isOdd)
            {
                infos[i].stopped = TRUE;
                infos[i].maneuvers[findIndex].checked = TRUE;
            }
            else
            {
                if (findIndex < maxDescriptionCount - 1)
                {
                    for (j = 0; j < routeCount; j++)
                    {
                        if (infos[j].stopped) continue;
                        infos[j].maneuvers[findIndex].checked = TRUE;
                    }
                }
            }
        }

        isAllStopped = TRUE;
        for (i = 0; i < routeCount; i++)
        {
            if (!infos[i].stopped)
            {
                isAllStopped = FALSE;
                break;
            }
        }
        findIndex++;
    }
}

static NB_Error
AllocManeuverDistanceInfo(ManeuverDistanceInfo** maneuvers, uint32 maneuverCount, struct CSL_Dictionary** dict)
{
    if (!maneuvers || !dict || !maneuverCount)
    {
        return NE_INVAL;
    }

    *maneuvers = nsl_malloc(sizeof(ManeuverDistanceInfo) * maneuverCount);
    if (!(*maneuvers))
    {
        return NE_NOMEM;
    }
    nsl_memset(*maneuvers, 0, sizeof(ManeuverDistanceInfo) * maneuverCount);

    *dict = CSL_DictionaryAlloc(maneuverCount);
    if (!(*dict))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

NB_Error Offboard_RouteInformationUpdateDescriptionWithDetour(NB_RouteInformation* route, NB_RouteInformation* detourRoute)
{
    return UpdateRouteInformationDescription(route->context, route, detourRoute);
}
