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

#include "HybridNavigationServiceProvider.h"

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

NB_DEF NB_Error
NB_RouteHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_RouteHandler** handler)
{
    return Hybrid_RouteHandlerCreate(context, callback, handler);
}

NB_DEF NB_Error
NB_RouteHandlerDestroy(NB_RouteHandler* handler)
{
    return Hybrid_RouteHandlerDestroy(handler);
}

NB_DEF NB_Error
NB_RouteHandlerStartRequest(NB_RouteHandler* handler, NB_RouteParameters* parameters)
{
    return Hybrid_RouteHandlerStartRequest(handler, parameters);
}

NB_DEF NB_Error
NB_RouteHandlerCancelRequest(NB_RouteHandler* handler)
{
    return Hybrid_RouteHandlerCancelRequest(handler);
}

NB_DEF nb_boolean
NB_RouteHandlerIsRequestInProgress(NB_RouteHandler* handler)
{
    return Hybrid_RouteHandlerIsRequestInProgress(handler);
}

NB_DEF NB_Error
NB_RouteHandlerGetRouteInformation(NB_RouteHandler* handler, NB_RouteInformation** route)
{
    return Hybrid_RouteHandlerGetRouteInformation(handler, route);
}

NB_DEF
NB_Error NB_RouteHandlerGetTrafficInformation(NB_RouteHandler* handler, NB_TrafficInformation** information)
{
    NB_Error err = NE_OK;

    if (!handler || !handler->route)
        return NE_INVAL;

    NB_ASSERT_VALID_THREAD(handler->context);

    err = NB_TrafficInformationCreateFromRoute(handler->context, handler->route, information);

    return err;
}

static void RouteHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
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

void NB_LogNavigationStart(NB_RouteHandler* pThis, NB_RouteParameters* parameters)
{
    NB_RouteSettings settings = { 0 };
    NB_LatitudeLongitude latLong = { 0 };
    const char* street = 0;
    NB_Error err = NE_OK;

    err = NB_RouteParametersGetRouteSettingsFromQuery(parameters, &settings);
    err = err ? err : NB_RouteParametersGetDestination(parameters, &latLong, &street);
    if (!err)
    {
        NB_QaLogNavigationStartup(pThis->context, &latLong, street, &settings);
    }
}

NB_DEF NB_Error
NB_RouteHandlerGetRouteError(NB_RouteHandler* pThis, NB_NavigateRouteError* error)
{
    if (!pThis || !error)
    {
        return NE_INVAL;
    }

    *error = pThis->route_err;
    return NE_OK;
}

static const char SPLIT_STRING[] = ", ";
static uint32 AppendAllManeuverDistanceInfo(NB_RouteInformation* routeInfo, data_util_state* pds, RouteDistanceInfo* infos)
{
    uint32 result = 0;
    uint32 maneuverCount = 0;
    uint32 routeCount = 0;
    data_nav_route* route = NULL;
    data_nav_maneuver* maneuver = NULL;
    struct CSL_Dictionary* dict = NULL;
    // append all maneuvers distance into data container(infos)
    if (routeInfo->dataReply.active_route == SINGLE_ROUTE)
    {
        maneuverCount = data_nav_reply_num_nav_maneuver(pds, &routeInfo->dataReply);
        AllocManeuverDistanceInfo(&infos[0].maneuvers, maneuverCount, &dict);
        for (uint32 i = 0; i < maneuverCount; i++)
        {
            maneuver = data_nav_reply_get_nav_maneuver(pds, &routeInfo->dataReply, i);
            AppendManeuverDistanceInfo(pds, &infos[0], maneuver, dict);
        }
        if(dict)
        {
            CSL_DictionaryDealloc(dict);
        }
        dict = NULL;
        result++;
    }
    else
    {
        routeCount = routeInfo->numberOfSelector;
        for (uint32 i = 0; i < routeCount; i++)
        {
            route = data_nav_reply_get_nav_route(pds, &routeInfo->dataReply, i);
            maneuverCount = data_nav_route_num_nav_maneuver(pds, route);
            AllocManeuverDistanceInfo(&infos[i].maneuvers, maneuverCount, &dict);

            for (uint32 j = 0; j < maneuverCount; j++)
            {
                maneuver = data_nav_route_get_nav_maneuver(pds, route, j);
                AppendManeuverDistanceInfo(pds, &infos[i], maneuver, dict);
            }

            CSL_DictionaryDealloc(dict);
            dict = NULL;
        }
        result++;
    }
    return result;
}

NB_Error
UpdateRouteInformationDescription(NB_Context* context,  NB_RouteInformation* routeInfo, NB_RouteInformation* routeInfo2)
{
    RouteDistanceInfo* infos = NULL;
    NB_Error err = NE_OK;
    uint32 i = 0;
    uint32 j = 0;
    uint32 routeCount = 0;
    uint32 routeCount2 = 0;
    data_util_state* pds = NULL;
    struct CSL_Dictionary* dict = NULL;

    do
    {
        if (!context || !routeInfo || !routeInfo->dataReply.vec_nav_maneuver)
        {
            err = NE_NOTINIT;
            break;
        }

        pds = NB_ContextGetDataState(context);

        routeCount = routeInfo->numberOfSelector;
        if (!routeCount)
        {
            err = NE_NOENT;
            break;
        }
        if (err = NB_RouteInformationDestroyDescription(routeInfo), err)
        {
            break;
        }

        if (routeInfo2)
        {
            routeCount2 = routeInfo2->numberOfSelector;
            if (err = NB_RouteInformationDestroyDescription(routeInfo2), err)
            {
                break;
            }
        }

        infos = nsl_malloc(sizeof(RouteDistanceInfo) * (routeCount + routeCount2));
        if (!infos)
        {
            err = NE_NOMEM;
            break;
        }
        nsl_memset(infos, 0, sizeof(RouteDistanceInfo) * (routeCount + routeCount2));

        // append all maneuvers distance into data container(infos)
        uint32 offset = AppendAllManeuverDistanceInfo(routeInfo, pds, infos);
        //treat routeInfo2 as detour and there is only one route in nav-reply
        if (routeInfo2)
        {
            AppendAllManeuverDistanceInfo(routeInfo2, pds, infos+offset);
        }

        //sort maneuvers by distance
        for (i = 0; i < routeCount + routeCount2; i++)
        {
            SortManeuverDistanceInfo(&infos[i]);
        }

        //find route description
        FindRouteDescriptions(infos, routeCount + routeCount2, MAX_DESCRIPTION_ROAD_COUNT);

        //set description to NB_RouteInformation instance
        routeInfo->routeDescription = (char**)nsl_malloc(sizeof(char*) * routeCount);
        if (!routeInfo->routeDescription)
        {
            err = NE_NOMEM;
            break;
        }
        nsl_memset(routeInfo->routeDescription, 0, sizeof(char*) * routeCount);
        if (routeCount2)
        {
            routeInfo2->routeDescription = (char**)nsl_malloc(sizeof(char*) * routeCount2);
            if (!routeInfo2->routeDescription)
            {
                err = NE_NOMEM;
                break;
            }
            nsl_memset(routeInfo2->routeDescription, 0, sizeof(char*) * routeCount2);
        }

        for (i = 0; i < routeCount + routeCount2; i++)
        {
            uint32 nameLength = 0;
            for (j = 0; j < infos[i].size; j++)
            {
                if (infos[i].maneuvers[j].checked)
                {
                    nameLength += (nsl_strlen(infos[i].maneuvers[j].name) + nsl_strlen(SPLIT_STRING));
                }
            }
            if (nameLength > 0)
            {
                nameLength++;
                if (i < routeCount)
                {
                    routeInfo->routeDescription[i] = (char*)nsl_malloc(nameLength);
                    if (!routeInfo->routeDescription[i])
                    {
                        err = NE_NOMEM;
                        break;
                    }
                    nsl_memset(routeInfo->routeDescription[i], 0, nameLength);
                }
                else
                {
                    routeInfo2->routeDescription[i - routeCount] = (char*)nsl_malloc(nameLength);
                    if (!routeInfo2->routeDescription[i - routeCount])
                    {
                        err = NE_NOMEM;
                        break;
                    }
                    nsl_memset(routeInfo2->routeDescription[i - routeCount], 0, nameLength);
                }
            }
        }

        for (i = 0; i < routeCount + routeCount2; i++)
        {
            char* desc = NULL;
            if (i < routeCount)
            {
                desc = routeInfo->routeDescription[i];
            }
            else
            {
                desc = routeInfo2->routeDescription[i - routeCount];
            }
            for (j = 0; j < infos[i].size; j++)
            {
                if (infos[i].maneuvers[j].checked)
                {
                    nsl_strcat(desc, infos[i].maneuvers[j].name);
                    nsl_strcat(desc, SPLIT_STRING);
                }
            }
            if (desc && nsl_strlen(desc) > 0 &&
                nsl_strncmp(&desc[nsl_strlen(desc) - nsl_strlen(SPLIT_STRING)],
                            SPLIT_STRING, nsl_strlen(SPLIT_STRING)) == 0)
            {
                desc[nsl_strlen(desc) - nsl_strlen(SPLIT_STRING)] = '\0';
            }

        }
        //set default current description
        if (routeCount > 0)
        {
            routeInfo->currentDescription = routeInfo->routeDescription[0];
        }
        if (routeCount2 > 0)
        {
            routeInfo2->currentDescription = routeInfo2->routeDescription[0];
        }
    } while(0);

    //free memory data
    if (dict)
    {
        CSL_DictionaryDealloc(dict);
    }
    if (infos)
    {
        for (i = 0; i < routeCount + routeCount2; i++)
        {
            if (infos[i].maneuvers)
            {
                nsl_free(infos[i].maneuvers);
            }
        }
        nsl_free(infos);
    }
    if (err)
    {
        NB_RouteInformationDestroyDescription(routeInfo);
        if (routeInfo2)
        {
            NB_RouteInformationDestroyDescription(routeInfo2);
        }
    }
    return err;
}

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

NB_DEF NB_Error
NB_RouteInformationUpdateDescriptionWithDetour(NB_RouteInformation* route, NB_RouteInformation* detourRoute)
{
    return UpdateRouteInformationDescription(route->context, route, detourRoute);
}


NB_Error NB_CreateAndUpdateRouteInformation(NB_RouteHandler* handler,
                                            NB_RouteInformation** route)
{
    NB_Error err = NB_RouteInformationCreateFromTPSReply(handler->context, handler->tpsReply, handler->parameters, route);
    if (err == NE_OK)
    {
        NB_Place* place = 0;

        handler->route = *route;

        UpdateRouteInformationDescription(handler->context, *route, NULL);

        place = nsl_malloc(sizeof(*place));
        if (place)
        {
            nsl_memset(place, 0, sizeof(*place));
            SetNIMLocationFromLocation(&place->location, NB_ContextGetDataState(handler->context), &handler->route->dataReply.origin_location);
            NB_QaLogPlace(handler->context, place, "route start");

            nsl_memset(place, 0, sizeof(*place));
            SetNIMLocationFromLocation(&place->location, NB_ContextGetDataState(handler->context), &handler->route->dataReply.destination_location);
            NB_QaLogPlace(handler->context, place, "route dest");

            nsl_free(place);
        }

        NB_QaLogRouteId(handler->context, handler->route->dataReply.route_id.data, (uint32)handler->route->dataReply.route_id.size);
    }

    return err;
}
