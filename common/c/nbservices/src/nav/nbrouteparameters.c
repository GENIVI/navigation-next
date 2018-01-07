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

#include "palstdlib.h"
#include "palmath.h"
#include "nbexp.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "nbrouteparameters.h"
#include "nbrouteinformationprivate.h"
#include "nbrouteparametersprivate.h"
#include "nbutility.h"
#include "data_nav_query.h"
#include "navutil.h"
#include "nbgpshistory.h"
#include "nbdetourparametersprivate.h"
#include "nbtrafficinformationprivate.h"
#include "nbqalog.h"
#include "nbspatial.h"
#include "tpslib.h"
#include "tpsio.h"
#include "instructset.h"
#include "nbnaturallaneguidancetype.h"
#include "data_via_point.h"

// @todo: add following constants as configurable variables
const double MAX_HORIZON_UNCERT_DISTANCE_METERS = 100.0;
const double MAX_VALID_HEADING_DISTANCE_METERS  = 100.0;
const double MAX_VALID_HEADING_TIME_DIFF_SECS   = 300.0;

static NB_RouteParameters* AllocateRouteParameters(NB_Context* context);

static void SetRouteSettingsFromOptions(NB_RouteOptions* options, NB_RouteSettings* settings);
static void SetDataQueryOptions(NB_RouteParameters* parameters, NB_RouteOptions* options, data_nav_query* query);
static void SetQueryConfigurationFromRouteConfiguration(NB_RouteParameters* parameters, NB_RouteConfiguration* config, data_nav_query* query);
static void SetDataQueryRouteSettingsFromReply(data_nav_query* query, data_nav_reply* reply, NB_RouteInformation* route);
static void NB_RouteParametersQueryCacheContentsAddItem(uint32 ts, byte* name, size_t namelen, byte* data, size_t datalen, void* pUser);
static NB_Error SetRouteSettings(data_util_state* pds, data_nav_query* pnq, NB_RouteSettings* pSettings);
static NB_Error SetQueryTrafficFlags(data_nav_query* pnq, NB_TrafficType trafficType);
static NB_Error AddDetours(NB_RouteParameters* pThis, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, data_util_state* dataState, nb_boolean* detoursWantHistoricalTraffic, nb_boolean* detoursWantRealtimeTraffic);
static void SetPronunStyle(data_util_state* pds, NB_RouteParameters* params, char* pronunStyle);
static void DataPointToLatitudeLongitude(data_point* point, NB_LatitudeLongitude* latLong);
static NB_Error   AddGpsFixToRouteOrigin(data_util_state* dataState, data_gps* gps, NB_GpsLocation* fix, NB_RouteParameters* routeParameters);
static nb_boolean IsValidLastHeadingFix(NB_GpsLocation* lastFix, NB_GpsLocation* lastHeadingFix);
static void NavigationInstructionToPrivateInstruction(NB_NavigationInstruction* navigationInstruction, navinstruct* instruction);

static NB_Error RouteParametersCreateInternal(NB_Context* context,
                                              NB_RouteInformation* route,
                                              NB_TrafficInformation* traffic,
                                              NB_DetourParameters* detour,
                                              NB_GpsHistory* gpsHistory,
                                              NB_RouteOptions* options,
                                              NB_RouteConfiguration* config,
                                              nb_boolean clearDetour,
                                              nb_boolean wantLabelPoints,
                                              uint32 alternateRoutesCount,
                                              NB_QaLogRouteQueryType qalogType,
                                              const char* reason,
                                              NB_RouteParameters** parameters);

static NB_Error
NB_RouteParametersSetOriginFromGPSHistory(NB_RouteParameters* routeParameters, NB_GpsHistory* history, nb_boolean isNewRoutes)
{
    NB_Error error = NE_OK;
    NB_GpsLocation firstfix = {0};
    NB_GpsLocation lastFix = {0};
    NB_GpsLocation lastHeadingFix = {0};
    data_util_state* dataState = NULL;
    data_gps gps;
    nsl_memset(&gps, 0, sizeof(gps));

    error = NB_GpsHistoryGetLatest(history, &lastFix);
    if (error != NE_OK)
    {
        return error;
    }

    nsl_memcpy(&routeParameters->originFix, &lastFix, sizeof(routeParameters->originFix));

    dataState = NB_ContextGetDataState(routeParameters->context);
    error = error ? error : data_gps_init(dataState, &gps);
    error = error ? error : data_origin_clear_gps(dataState, &routeParameters->dataQuery.origin);

    if (error != NE_OK)
    {
        return error;
    }

    if (isNewRoutes)
    {
        error = NB_GpsHistoryGet(history, 0, &firstfix);
        if (error == NE_OK)
        {
            if (!gpsfix_identical(&firstfix, &lastFix))
            {
                error = error ? error : AddGpsFixToRouteOrigin(dataState, &gps, &lastFix, routeParameters);
            }

            if (error == NE_NOENT )
            {
                error = NE_OK;
            }
        }
        error = error ? error : AddGpsFixToRouteOrigin(dataState, &gps, &firstfix, routeParameters);
    }
    else
    {
        error = error ? error : AddGpsFixToRouteOrigin(dataState, &gps, &lastFix, routeParameters);
        if (error == NE_OK)
        {
            error = NB_GpsHistoryGetLatestHeadingFix(history, &lastHeadingFix);
            if (error == NE_OK)
            {
                if (IsValidLastHeadingFix(&lastFix, &lastHeadingFix))
                {
                    error = error ? error : AddGpsFixToRouteOrigin(dataState, &gps, &lastHeadingFix, routeParameters);
                }
            }
            if (error == NE_NOENT)
            {
                error = NE_OK;
            }
        }
    }

    return error;
}

static NB_Error
AddGpsFixToRouteOrigin(data_util_state* dataState, data_gps* gps, NB_GpsLocation* fix, NB_RouteParameters* routeParameters)
{
    NB_Error error = NE_OK;

    data_gps_from_gpsfix(dataState, gps, fix);
    error = data_origin_add_gps(dataState, &routeParameters->dataQuery.origin, gps);
    if( NE_OK == error )
    {
        NB_QaLogRouteOrigin(routeParameters->context, fix);
    }

    return error;
}

static nb_boolean
IsValidLastHeadingFix(NB_GpsLocation* lastFix, NB_GpsLocation* lastHeadingFix)
{
    if( lastFix && lastHeadingFix )
    {
        if( gpsfix_identical(lastFix, lastHeadingFix) )
        {
            return TRUE;
        }

        if( (lastHeadingFix->valid & NGV_Latitude)         &&
            (lastHeadingFix->valid & NGV_Longitude)        &&
            (lastHeadingFix->valid & NGV_AxisUncertainty)  &&
            lastHeadingFix->horizontalUncertaintyAlongAxis <= MAX_HORIZON_UNCERT_DISTANCE_METERS )
        {
            if( (lastFix->valid & NGV_Latitude) && (lastFix->valid & NGV_Longitude) )
            {
                double deltaDistance = 0.0;
                double deltaTime     = 0.0;

                deltaTime     = lastFix->gpsTime - lastHeadingFix->gpsTime;
                deltaDistance = NB_SpatialGetLineOfSightDistance(lastFix->latitude, lastFix->longitude, lastHeadingFix->latitude, lastHeadingFix->longitude, NULL);
                if( deltaTime <= MAX_VALID_HEADING_TIME_DIFF_SECS && deltaDistance <= MAX_VALID_HEADING_DISTANCE_METERS )
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

NB_DEF NB_Error
NB_RouteParametersCreateFromGpsHistory(NB_Context* context, NB_GpsHistory* history, NB_Place* destination, NB_RouteOptions* options, NB_RouteConfiguration* configuration, NB_RouteParameters** parameters)
{
    NB_Error err = NE_OK;
    NB_RouteParameters* pThis = 0;
    data_util_state* dataState = NB_ContextGetDataState(context);
    CSL_Cache* voiceCache = NULL;

    if ( !context || !destination || !options || !configuration || !history )
    {
        return NE_INVAL;
    }

    pThis = AllocateRouteParameters(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }

    err = NB_RouteParametersSetOriginFromGPSHistory(pThis, history, TRUE);

    SetDataQueryOptions(pThis, options, &pThis->dataQuery);
    SetQueryConfigurationFromRouteConfiguration(pThis, configuration, &pThis->dataQuery);

    err = err ? err : SetDestinationFromPlace(dataState, &pThis->dataQuery.destination, destination);

    pThis->type = NB_QLRQT_FromHistory;

    SetPronunStyle(dataState, pThis, options->pronunStyle);

    // Do not include the cache-contents element as downloadeable prononunciation are not needed
    if (nsl_strempty(pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, NB_RouteParametersQueryCacheContentsAddItem, pThis, FALSE);
        }
    }

    NB_QaLogPlace(pThis->context, destination, "route dest rqst");

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_RouteParametersCreateFromPlace(NB_Context* context, NB_Place* origin, NB_Place* destination, NB_RouteOptions* options, NB_RouteConfiguration* configuration, NB_RouteParameters** parameters)
{
    NB_RouteParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;
    CSL_Cache* voiceCache = NULL;

    if ( !context || !destination || !options || !configuration )
    {
        return NE_INVAL;
    }

    pThis = AllocateRouteParameters(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(context);

    gpsfix_clear(&pThis->originFix);

    SetDataQueryOptions(pThis, options, &pThis->dataQuery);
    SetQueryConfigurationFromRouteConfiguration(pThis, configuration, &pThis->dataQuery);

    err = err ? err : SetOriginFromPlace(dataState, &pThis->dataQuery.origin, origin);
    err = err ? err : SetDestinationFromPlace(dataState, &pThis->dataQuery.destination, destination);

    pThis->type = NB_QLRQT_FromPlace;

    SetPronunStyle(dataState, pThis, options->pronunStyle);

    // Do not include the cache-contents element as downloadeable prononunciation are not needed
    if (nsl_strempty(pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, NB_RouteParametersQueryCacheContentsAddItem, pThis, FALSE);
        }
    }

    NB_QaLogPlace(pThis->context, destination, "route dest rqst");

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_RouteParametersCreateOffRouteEx(NB_Context* context, NB_GpsHistory* gpsHistory, NB_RouteInformation* route, NB_RouteParameters** parameters, nb_boolean wantAlternateRoute)
{
    NB_Error err = NE_OK;
    NB_RouteParameters* pThis = NULL;
    NB_GpsHistory* history = NULL;
    data_util_state* dataState = NB_ContextGetDataState(context);
    const char* reasontxt = "off-route";
    CSL_Cache* voiceCache = NULL;

    history = NB_ContextGetGpsHistory(context);
    if (history == NULL)
    {
        return NE_NOTINIT;
    }

    pThis = AllocateRouteParameters(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }

    pThis->type = NB_QLRQT_OffRoute;

    data_cache_contents_free(dataState, &pThis->dataQuery.cache_contents);
    err = data_cache_contents_init(dataState, &pThis->dataQuery.cache_contents);
    err = err ? err : data_nav_query_copy(dataState, &pThis->dataQuery, &route->routeParameters->dataQuery);

    err = err ? err : NB_RouteParametersSetOriginFromGPSHistory(pThis, history, FALSE);

    // Clear the shared gps history because old fixes are useless.
    NB_GpsHistoryClear(history);

    pThis->dataQuery.has_recalc = TRUE;

    err = err ? err : data_blob_copy(dataState, &pThis->dataQuery.recalc.route_id,
                                     &route->dataReply.route_id);

    err = err ? err : data_string_set(dataState, &pThis->dataQuery.recalc.why, reasontxt);

    SetQueryConfigurationFromRouteConfiguration(pThis, &route->parameterConfiguration, &pThis->dataQuery);
    SetDataQueryRouteSettingsFromReply(&pThis->dataQuery, &route->dataReply, route);
    pThis->config.wantAlternateRoutes = wantAlternateRoute;
    SetPronunStyle(dataState, pThis, route->routeParameters->pronunStyle);
    NB_RouteParametersSetLanguage(pThis, route->routeParameters->language);
    SetQueryTrafficFlags(&pThis->dataQuery, route->dataReply.route_settings.traffic_type);
    nsl_memcpy(&pThis->privateConfig, &route->routeParameters->privateConfig, sizeof(pThis->privateConfig));

    // Do not include the cache-contents element as downloadeable prononunciation are not needed
    if (nsl_strempty(pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, NB_RouteParametersQueryCacheContentsAddItem, pThis, FALSE);
        }
    }

    if (route->routeParameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        pThis->privateConfig.prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
        if (pThis->privateConfig.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableForEach(route->routeParameters->privateConfig.prefs.laneGuidanceFontMap, CopyEachEntryToNewHashTable, pThis->privateConfig.prefs.laneGuidanceFontMap);
        }
        else
        {
            err = NE_INVAL;
        }

    }
    err = err ? err : data_destination_copy(dataState, &pThis->dataQuery.destination,
                                        &route->routeParameters->dataQuery.destination);
    pThis->dataQuery.want_maneuvers = route->routeParameters->dataQuery.want_maneuvers;
    pThis->dataQuery.want_all_maneuvers = route->routeParameters->dataQuery.want_all_maneuvers;

    if (!wantAlternateRoute)
    {
        err = err ? err : data_nav_query_set_route_selector_params(dataState, &pThis->dataQuery, 0, FALSE);
    }
    else
    {
        err = err ? err : data_nav_query_set_route_selector_params(dataState, &pThis->dataQuery, 1, TRUE);
    }

    if (err != NE_OK)
    {
        NB_RouteParametersDestroy(pThis);
        return err;
    }

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_RouteParametersCreateOffRoute(NB_Context* context, NB_GpsHistory* gpsHistory, NB_RouteInformation* route, NB_RouteParameters** parameters)
{
    return NB_RouteParametersCreateOffRouteEx(context, gpsHistory, route, parameters, FALSE);
}

NB_DEF NB_Error
NB_RouteParametersCreateDetour(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteParameters** parameters)
{
    return RouteParametersCreateInternal(context,
                                         route,
                                         traffic,
                                         detour,
                                         gpsHistory,
                                         NULL,
                                         NULL,
                                         TRUE,
                                         TRUE,
                                         ROUTE_SELECTOR_DETOURS_MAX_COUNT,
                                         NB_QLRQT_Detour,
                                         "detour",
                                         parameters);
}

NB_DEF NB_Error
NB_RouteParametersCreateDetourWithChangedOptions(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters)
{
    return RouteParametersCreateInternal(context,
                                         route,
                                         traffic,
                                         detour,
                                         gpsHistory,
                                         options,
                                         NULL,
                                         TRUE,
                                         TRUE,
                                         ROUTE_SELECTOR_DETOURS_MAX_COUNT,
                                         NB_QLRQT_Detour,
                                         "detour",
                                         parameters);
}

NB_DEF NB_Error
NB_RouteParametersCreateChangeRouteOptions(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters, nb_boolean wantAlternateRoute)
{
    return NB_RouteParametersCreateChangeRouteOptionsAndConfig(context, route, traffic, detour, gpsHistory, options, NULL, parameters, wantAlternateRoute);
}

NB_DEF NB_Error
NB_RouteParametersCreateChangeRouteOptionsAndConfig(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteConfiguration* config, NB_RouteParameters** parameters, nb_boolean wantAlternateRoute)
{
    uint32 alternateRoute = ROUTE_SELECTOR_ALTERNATE_ROUTES_MAX_COUNT;
    if (!wantAlternateRoute)
    {
        alternateRoute = 1;
    }

    return RouteParametersCreateInternal(context,
                                         route,
                                         traffic,
                                         detour,
                                         gpsHistory,
                                         options,
                                         config,
                                         FALSE,
                                         TRUE,
                                         alternateRoute,
                                         NB_QLRQT_Recreate,
                                         "user",
                                         parameters);
}

NB_DEF NB_Error
NB_RouteParametersCreateRecalculateRoute(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters)
{
    return RouteParametersCreateInternal(context,
                                         route,
                                         traffic,
                                         detour,
                                         gpsHistory,
                                         options,
                                         NULL,
                                         FALSE,
                                         FALSE,
                                         0,
                                         NB_QLRQT_OffRoute,
                                         "off-route",
                                         parameters);
}


NB_DEF NB_Error
NB_RouteParametersRecreateRoute(NB_Context* context, NB_RouteInformation* route, NB_RouteParameters** parameters)
{
    NB_Error err = NE_OK;
    NB_RouteParameters* pThis = NULL;
    data_util_state* dataState = NB_ContextGetDataState(context);
    CSL_Cache* voiceCache = NULL;

    pThis = AllocateRouteParameters(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }

    err = err ? err : data_blob_copy(dataState, &pThis->dataQuery.recreate_route_id, &route->dataReply.route_id);

    // save the original route destination
    err = err ? err : data_destination_copy(dataState, &pThis->dataQuery.destination,
                                        &route->routeParameters->dataQuery.destination);

    /* save the origin fix from the original route */
    pThis->originFix = route->routeParameters->originFix;

    SetDataQueryRouteSettingsFromReply(&pThis->dataQuery, &route->dataReply, route);
    SetQueryConfigurationFromRouteConfiguration(pThis, &route->parameterConfiguration, &pThis->dataQuery);
    SetRouteSettings(dataState, &pThis->dataQuery, &route->dataReply.route_settings);
    SetQueryTrafficFlags(&pThis->dataQuery, route->dataReply.route_settings.traffic_type);
    SetPronunStyle(dataState, pThis, route->routeParameters->pronunStyle);
    NB_RouteParametersSetLanguage(pThis, route->routeParameters->language);
    nsl_memcpy(&pThis->privateConfig, &route->routeParameters->privateConfig, sizeof(pThis->privateConfig));

    // Do not include the cache-contents element as downloadeable prononunciation are not needed
    if (nsl_strempty(pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, NB_RouteParametersQueryCacheContentsAddItem, pThis, FALSE);
        }
    }

    if (route->routeParameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        pThis->privateConfig.prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
        if (pThis->privateConfig.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableForEach(route->routeParameters->privateConfig.prefs.laneGuidanceFontMap, CopyEachEntryToNewHashTable, pThis->privateConfig.prefs.laneGuidanceFontMap);
        }
        else
        {
            err = NE_INVAL;
        }

    }

    pThis->dataQuery.max_pronun_files = 0;
    pThis->dataQuery.want_maneuvers = route->routeParameters->dataQuery.want_maneuvers;
    pThis->dataQuery.want_all_maneuvers = route->routeParameters->dataQuery.want_all_maneuvers;
    pThis->type = NB_QLRQT_Recreate;

    if (err != NE_OK)
    {
        NB_RouteParametersDestroy(pThis);
        return err;
    }

    (*parameters) = pThis;
    return err;
}

NB_DEF NB_Error
NB_RouteParametersDestroy(NB_RouteParameters* parameters)
{
    if (parameters)
    {
        data_nav_query_free(NB_ContextGetDataState(parameters->context), &parameters->dataQuery);

        if (parameters->pronunStyle != NULL)
            nsl_free(parameters->pronunStyle);

        if (parameters->language != NULL)
        {
            nsl_free(parameters->language);
        }

        if (parameters->privateConfig.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableDestroy(parameters->privateConfig.prefs.laneGuidanceFontMap, TRUE);
            parameters->privateConfig.prefs.laneGuidanceFontMap = NULL;
        }

        nsl_free(parameters);
        return NE_OK;
    }
    return NE_INVAL;
}

tpselt
NB_RouteParametersToTPSQuery(NB_RouteParameters* parameters)
{
    NB_LatitudeLongitude origin = { 0 };
    NB_LatitudeLongitude destination = { 0 };

    DataPointToLatitudeLongitude(&parameters->dataQuery.origin.point, &origin);
    DataPointToLatitudeLongitude(&parameters->dataQuery.destination.point, &destination);

    NB_QaLogRouteQuery(parameters->context, parameters->type, &origin, &destination, &parameters->dataQuery.route_settings);

    return data_nav_query_to_tps( NB_ContextGetDataState(parameters->context), &parameters->dataQuery);
}

NB_Error
NB_RouteParametersCopyPrivateConfigFromParameters(NB_RouteParameters* parameters, NB_RoutePrivateConfiguration* dst)
{
    if ( !dst || !parameters )
    {
        return NE_INVAL;
    }

    *dst = parameters->privateConfig;
    if (parameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        dst->prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
        if (dst->prefs.laneGuidanceFontMap)
        {
            CSL_HashTableForEach(parameters->privateConfig.prefs.laneGuidanceFontMap, CopyEachEntryToNewHashTable, dst->prefs.laneGuidanceFontMap);
        }
        else
        {
            return NE_INVAL;
        }

    }
    return NE_OK;
}

NB_Error
NB_RouteParametersCopyConfigFromParameters(NB_RouteParameters* parameters, NB_RouteConfiguration* dst)
{
    if ( !dst || !parameters )
    {
        return NE_INVAL;
    }

    *dst = parameters->config;

    return NE_OK;
}

NB_Error
NB_RouteParametersGetRouteSettingsFromQuery(NB_RouteParameters* parameters, NB_RouteSettings* settings)
{
    if (!parameters)
        return NE_INVAL;

    *settings = parameters->dataQuery.route_settings;

    return NE_OK;
}

NB_Error
NB_RouteParametersGetOriginGpsLocation(NB_RouteParameters* parameters, NB_GpsLocation* location)
{
    if (!parameters || !location)
    {
        return NE_INVAL;
    }

    *location = parameters->originFix;

    return NE_OK;
}

nb_boolean
NB_RouteParametersIsTrafficRoute(NB_RouteParameters* parameters)
{
    return (parameters->dataQuery.route_settings.traffic_type & NB_TrafficType_Route) ? TRUE : FALSE;
}

static void
NB_RouteParametersQueryCacheContentsAddItem(uint32 ts, byte* name, size_t namelen, byte* data, size_t datalen, void* pUser)
{
    NB_RouteParameters* pThis = (NB_RouteParameters*) pUser;
    data_util_state* dataState=NULL;
    data_cache_item ce;
    int i = 0;
    int length = 0;

    if (!pThis)
        return;

    // check if pThis->dataQuery.cache_contents contain the cache item, if not, append to vec_cache_item
    length = CSL_VectorGetLength(pThis->dataQuery.cache_contents.vec_cache_item);
    for(i = 0; i < length; i++)
    {
        data_blob* tmpCacheItem = CSL_VectorGetPointer(pThis->dataQuery.cache_contents.vec_cache_item, i);
        if(tmpCacheItem->size == namelen && nsl_memcmp(tmpCacheItem->data, name, namelen) == 0)
        {
            return;
        }
    }

    dataState = NB_ContextGetDataState(pThis->context);

    data_cache_item_init(dataState, &ce);
    data_blob_set(dataState, &ce.name, name, namelen);
    data_cache_contents_add(dataState, &pThis->dataQuery.cache_contents, &ce);
    data_cache_item_free(dataState, &ce);
}

NB_Error
RouteParametersCreateInternal(NB_Context* context,
                              NB_RouteInformation* route,
                              NB_TrafficInformation* traffic,
                              NB_DetourParameters* detour,
                              NB_GpsHistory* gpsHistory,
                              NB_RouteOptions* options,
                              NB_RouteConfiguration* config,
                              nb_boolean clearDetour,
                              nb_boolean wantLabelPoints,
                              uint32 alternateRoutesCount,
                              NB_QaLogRouteQueryType qalogType,
                              const char* reason,
                              NB_RouteParameters** parameters)
{
    NB_Error err = NE_OK;
    NB_RouteParameters* pThis = NULL;
    data_util_state* dataState = NB_ContextGetDataState(context);
    nb_boolean detoursWantHistoricalTraffic = FALSE;
    nb_boolean detoursWantRealTimeTraffic = FALSE;
    CSL_Cache* voiceCache = NULL;

    if (gpsHistory == NULL)
    {
        return NE_NOTINIT;
    }

    pThis = AllocateRouteParameters(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }

    if (!route)
    {
        NB_RouteParametersDestroy(pThis);
        return NE_INVAL;
    }

    data_cache_contents_free(dataState, &pThis->dataQuery.cache_contents);
    err = data_cache_contents_init(dataState, &pThis->dataQuery.cache_contents);
    err = err ? err : data_nav_query_copy(dataState, &pThis->dataQuery,
                              &route->routeParameters->dataQuery);
    err = err ? err : data_nav_query_set_route_selector_params(dataState, &pThis->dataQuery, alternateRoutesCount, wantLabelPoints);

    err = err ? err : NB_RouteParametersSetOriginFromGPSHistory(pThis, gpsHistory, FALSE);
    err = err ? err : data_destination_copy(dataState, &pThis->dataQuery.destination,
                                          &route->routeParameters->dataQuery.destination);

    SetQueryConfigurationFromRouteConfiguration(pThis, config ? config : &route->parameterConfiguration, &pThis->dataQuery);
    if (options)
    {
        SetDataQueryOptions(pThis, options, &pThis->dataQuery);
    }
    else
    {
        SetRouteSettings(dataState, &pThis->dataQuery, &route->dataReply.route_settings);
    }
    pThis->type = qalogType;
    pThis->dataQuery.has_recalc = TRUE;

    if (clearDetour && !detour)
    {
        reason = "user";
    }

    err = err ? err : data_nav_query_set_route_selector_params(dataState, &pThis->dataQuery, alternateRoutesCount, wantLabelPoints);
    pThis->config.wantAlternateRoutes = alternateRoutesCount > 0 ? TRUE : FALSE;
    err = err ? err : data_blob_copy(dataState, &pThis->dataQuery.recalc.route_id,
                                     &route->dataReply.route_id);
    err = err ? err : data_string_set(dataState, &pThis->dataQuery.recalc.why, reason);

    pThis->dataQuery.want_maneuvers = route->routeParameters->dataQuery.want_maneuvers;
    pThis->dataQuery.want_all_maneuvers = route->routeParameters->dataQuery.want_all_maneuvers;

    if (detour)
    {
        err = err ? err : AddDetours(pThis, route, traffic, detour, dataState,
                                     &detoursWantHistoricalTraffic,
                                     &detoursWantRealTimeTraffic);
        if (err == NE_OK)
        {
            if (detoursWantHistoricalTraffic)
            {
                pThis->dataQuery.route_settings.traffic_type |= NB_TrafficType_HistoricalRoute;
            }

            if (detoursWantRealTimeTraffic)
            {
                pThis->dataQuery.route_settings.traffic_type |= NB_TrafficType_RealTimeRoute;
            }
        }
    }
    else if (clearDetour)/* Clear All Detours */
    {
        err = data_nav_query_clear_detours(dataState, &pThis->dataQuery);
        pThis->dataQuery.route_settings.traffic_type ^= (NB_TrafficType_HistoricalRoute | NB_TrafficType_RealTimeRoute);
    }

    SetQueryTrafficFlags(&pThis->dataQuery, pThis->dataQuery.route_settings.traffic_type);
    SetPronunStyle(dataState, pThis, route->routeParameters->pronunStyle);
    NB_RouteParametersSetLanguage(pThis, route->routeParameters->language);
    nsl_memcpy(&pThis->privateConfig, &route->routeParameters->privateConfig, sizeof(pThis->privateConfig));

    // Do not include the cache-contents element as downloadeable prononunciation are not needed
    if (nsl_strempty(pThis->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
    {
        voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache)
        {
            CSL_CacheForEach(voiceCache, NB_RouteParametersQueryCacheContentsAddItem, pThis, FALSE);
        }
    }

    if (route->routeParameters->privateConfig.prefs.laneGuidanceFontMap)
    {
        pThis->privateConfig.prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
        if (pThis->privateConfig.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableForEach(route->routeParameters->privateConfig.prefs.laneGuidanceFontMap, CopyEachEntryToNewHashTable, pThis->privateConfig.prefs.laneGuidanceFontMap);
        }
        else
        {
            err = NE_INVAL;
        }

    }

    if (err != NE_OK)
    {
        NB_RouteParametersDestroy(pThis);
        return err;
    }

    (*parameters) = pThis;

    return err;
}

static NB_RouteParameters*
AllocateRouteParameters(NB_Context* context)
{
    NB_RouteParameters* pThis = NULL;

    if (!context)
        return NULL;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    NB_RouteParametersPreferencesPrivateInit(&pThis->privateConfig.prefs);

    data_nav_query_init( NB_ContextGetDataState(context), &pThis->dataQuery);

    pThis->dataQuery.want_ferry_maneuvers	= FALSE;
    pThis->dataQuery.want_unpaved_info		= TRUE;
    pThis->dataQuery.want_towards_roadinfo = FALSE;
    pThis->dataQuery.disable_navigation = FALSE;

    return pThis;
}

static void
SetPronunStyle(data_util_state* pds, NB_RouteParameters* params, char* pronunStyle)
{
    if (params && pronunStyle)
    {
        params->pronunStyle = nsl_strdup(pronunStyle);
        data_string_set(pds, &params->dataQuery.pronun_style, pronunStyle);
    }
    else if (params)
    {
        // PronunStyle = NULL is possible in case if voice navigation is not supported, like in NBI SDK
        params->pronunStyle = NULL;
        data_string_set(pds, &params->dataQuery.pronun_style, "");
    }
}

static void
SetQueryConfigurationFromRouteConfiguration(NB_RouteParameters* parameters, NB_RouteConfiguration* config, data_nav_query* query)
{
    query->want_origin_location = config->wantOriginLocation;
    query->want_destination_location = config->wantDestinationLocation;
    query->want_route_map = config->wantRouteMap;
    query->want_cross_streets = config->wantCrossStreets;
    query->want_maneuvers = config->wantManeuvers;
    query->want_all_maneuvers = config->wantManeuvers;
    query->want_ferry_maneuvers = config->wantFerryManeuvers;
    query->want_towards_roadinfo = config->wantTowardsRoadInformation;
    query->want_intersection_roadinfo = config->wantIntersectionRoadInformation;
    query->want_country_info = config->wantCountryInformation;
    query->want_enter_roundabout_maneuvers = config->wantEnterRoundaboutManeuvers;
    query->want_enter_country_maneuvers = config->wantEnterCountryManeuvers;
    query->want_bridge_maneuvers = config->wantBridgeManeuvers;
    query->want_tunnel_maneuvers = config->wantTunnelManeuvers;
    query->want_route_extents = config->wantRouteExtents;
    query->want_unpaved_info = config->wantUnpavedRoadInformation;

    //config->wantPolyline;
    //config->routeTimeOverride;
    query->route_map_angle_of_view = config->routeMapAngleOfView;
    query->route_map_distance_of_view = config->routeMapDistanceOfView;
    query->route_map_min_time = config->routeMapMinTime;
    query->route_map_passes = config->routeMapPasses;
    query->route_map_tile_z = config->routeMapTileZ;
    query->route_map_max_tiles = config->routeMapMaxTiles;
    query->command_set_version = config->commandSetVersion;
    query->disable_navigation = config->disableNavigation;
    query->max_pronun_files = config->maxPronunFiles;

    // @todo: Investigate another way for NBI to set this value
    // query->want_historical_traffic = TRUE;
    // query->want_realtime_traffic = TRUE;

    query->want_realistic_road_signs = config->wantRealisticSigns;
    query->want_junction_models = config->wantJunctionModels;
    query->want_city_models = config->wantCityModels;
    query->want_speed_regions = config->wantSpeedLimit;
    query->want_speed_cameras = config->wantSpeedCameras;
    query->want_guidance_config.want_lane_guidance = config->wantLaneGuidance;
    query->want_guidance_config.want_enchanced_natural_guidance = config->wantExtendedNaturalGuidance;
    query->want_guidance_config.want_natural_guidance = config->wantNaturalGuidance;
    query->want_guidance_config.want_play_times = config->wantPlayTimes;
    query->want_guidance_config.want_guidance_config = config->wantGuidanceConfig;
    query->want_exit_numbers = config->wantExitNumbers;
    query->want_label_point  = config->wantLabelPoints;
    query->want_complete_route = config->wantCompleteRoute;
    if (!nsl_strempty(config->supportedPhoneticsFormats))
    {
        data_util_state* dataState = NB_ContextGetDataState(parameters->context);
        data_string_set(dataState, &query->want_extended_pronun_data.supported_phonetics_formats, config->supportedPhoneticsFormats);
    }

    if (config->wantAlternateRoutes)
    {
        query->want_alternate_routes.max_routes = ROUTE_SELECTOR_ALTERNATE_ROUTES_MAX_COUNT;
    }
    query->want_non_guidance_maneuvers = config->wantNonGuidanceManeuvers;
    query->want_max_turn_for_street_count = config->wantMaxTurnDistanceForStreetCount;
    query->want_route_summary = config->wantRouteSummary;
    query->want_route_summary_delaytime = config->wantRouteSummaryDelayTime;
    query->want_special_regions = config->wantSpecialRegions;
    query->want_first_maneuver_cross_streets = config->wantFirstManeuverCrossStreets;
    query->want_secondary_name_for_exits = config->wantSecondaryNameForExits;
    query->want_check_unsupported_countries = config->wantCheckUnsupportedCountries;
    query->want_first_major_road = config->wantFirstMajorRoad;
    query->start_to_navigate = config->startToNavigate;
    query->want_pedestrian_aneuvers = config->wantPedestrianManeuvers;

    parameters->config = *config;
}

static void
SetRouteSettingsFromOptions(NB_RouteOptions* options, NB_RouteSettings* settings)
{
    settings->route_avoid = options->avoid;
    settings->route_type = options->type;
    settings->vehicle_type = options->transport;
    settings->traffic_type = options->traffic;
}

static void
SetDataQueryOptions(NB_RouteParameters* parameters, NB_RouteOptions* options, data_nav_query* query)
{
    NB_RouteSettings settings;

    data_util_state* dataState = NB_ContextGetDataState(parameters->context);

    SetRouteSettingsFromOptions(options, &settings);
    SetRouteSettings(dataState, query, &settings);
    SetQueryTrafficFlags(query, settings.traffic_type);
}

static void
SetDataQueryRouteSettingsFromReply(data_nav_query* query, data_nav_reply* reply, NB_RouteInformation* route)
{
    query->route_settings = reply->route_settings;
}

NB_Error
NB_RouteParametersClone(NB_RouteParameters* params, NB_RouteParameters** clone)
{
    NB_RouteParameters* copy = NULL;
    data_util_state* dataState = NULL;

    if (!params)
        return NE_INVAL;

    copy = nsl_malloc(sizeof(NB_RouteParameters));
    if (!copy)
    {
        return NE_NOMEM;
    }
    nsl_memset(copy, 0, sizeof(*copy));
    nsl_memcpy(&copy->privateConfig, &params->privateConfig, sizeof(params->privateConfig));
    if (params->privateConfig.prefs.laneGuidanceFontMap)
    {
        copy->privateConfig.prefs.laneGuidanceFontMap = CSL_HashTableCreate(MAX_NUM_FONT_MAP, NULL);
        if (copy->privateConfig.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableForEach(params->privateConfig.prefs.laneGuidanceFontMap, CopyEachEntryToNewHashTable, copy->privateConfig.prefs.laneGuidanceFontMap);
        }
        else
        {
            nsl_free(copy);
            return NE_INVAL;
        }

    }

    dataState = NB_ContextGetDataState(params->context);

    copy->context = params->context;
    copy->type = params->type;
    data_nav_query_init( dataState, &copy->dataQuery);
    data_nav_query_copy(dataState, &copy->dataQuery, &params->dataQuery);

    nsl_memcpy(&copy->originFix, &params->originFix, sizeof(params->originFix));
    nsl_memcpy(&copy->config, &params->config, sizeof(params->config));
    SetPronunStyle(dataState, copy, params->pronunStyle);
    NB_RouteParametersSetLanguage(copy, params->language);

    (*clone) = copy;

    return NE_OK;
}

NB_Error
NB_RouteParametersPreferencesPrivateInit(NB_RoutePreferencesPrivate* pprefs)
{
    if (!pprefs)
        return NE_INVAL;

    pprefs->minNoDataDisableTrafficBar       = 100;
    pprefs->navRouteMapPasses                = 8;
    pprefs->navRouteMapTZ                    = 16;
    pprefs->numInitialTurnmapTiles           = 15;
    pprefs->navMinTmcSpeed                   = 0.44704; // 1 mph in m/s

    pprefs->numNavTurnIndex = 0;

    pprefs->numNavTurnTimes = MAX_NUM_TURN_TIMES;

    nsl_strcpy(pprefs->navTurnTimes[0].command, ""); //default
    pprefs->navTurnTimes[0].time = 0;

    nsl_strcpy(pprefs->navTurnTimes[1].command, "TR.L");
    pprefs->navTurnTimes[1].time = 15;

    nsl_strcpy(pprefs->navTurnTimes[2].command, "TR.R");
    pprefs->navTurnTimes[2].time = 10;

    nsl_strcpy(pprefs->navTurnTimes[3].command, "EN.L");
    pprefs->navTurnTimes[3].time = 10;

    nsl_strcpy(pprefs->navTurnTimes[4].command, "EN.R");
    pprefs->navTurnTimes[4].time = 5;

    nsl_strcpy(pprefs->navTurnTimes[5].command, "UT.");
    pprefs->navTurnTimes[5].time = 45;

    nsl_strcpy(pprefs->navTurnTimes[6].command, "RE.");
    pprefs->navTurnTimes[6].time = 30;

    nsl_strcpy(pprefs->navTurnTimes[7].command, "NC.");
    pprefs->navTurnTimes[7].time = 0;

    nsl_strcpy(pprefs->navTurnTimes[8].command, "SC.");
    pprefs->navTurnTimes[8].time = 0;

    nsl_strcpy(pprefs->navTurnTimes[9].command, "FE.");
    pprefs->navTurnTimes[9].time = 5;

    nsl_strcpy(pprefs->navTurnTimes[10].command, "FX.");
    pprefs->navTurnTimes[10].time = 5;


    pprefs->laneGuidanceFontMapVersion = 0;

    pprefs->laneGuidanceFontMap = NULL;

    pprefs->hovFontMapVersion = 0;
    pprefs->numHovFontMap = 0;
    nsl_memset(pprefs->hovFontMap, 0, sizeof(pprefs->hovFontMap));

    pprefs->bboxbuffer                              = 0.01;     // buffer (in degrees) around the actual bounds of the

    pprefs->navWWIgnore                             = 2;        // ww-number of wrong way points before a recalc
    pprefs->navOrIgnore                             = 2;        // or-number of off route points before a recalc
    pprefs->navStartupMinMatches                    = 3;        // number of on route points with valid heading to exit startup case
    pprefs->navEllErrorEnable                       = 1;        // ellerr-enable/disable elliptical error radius calculation
    pprefs->navL1HeadingMargin                      = 5;        // ht1-heading margin for this level of distance relaxation
    pprefs->navL1DistThreshold                      = 95.0;     // hd1-distance threshold for this level of distance relaxation
    pprefs->navL2HeadingMargin                      = 10;       // ht2-heading margin for this level of distance relaxation
    pprefs->navL2DistThreshold                      = 90.0;     // hd2-distance threshold for this level of distance relaxation
    pprefs->navL3HeadingMargin                      = 15;       // ht3-heading margin for this level of distance relaxation
    pprefs->navL3DistThreshold                      = 65.0;     // hd3-distance threshold for this level of distance relaxation
    pprefs->navL4HeadingMargin                      = 20;       // ht4-heading margin for this level of distance relaxation
    pprefs->navL4DistThreshold                      = 55.0;     // hd4-distance threshold for this level of distance relaxation
    pprefs->navL5HeadingMargin                      = 30;       // ht5-heading margin for this level of distance relaxation
    pprefs->navL5DistThreshold                      = 50.0;     // hd5-distance threshold for this level of distance relaxation
    pprefs->navL6HeadingMargin                      = 40;       // ht6-heading margin for this level of distance relaxation
    pprefs->navL6DistThreshold                      = 45.0;     // hd6-distance threshold for this level of distance relaxation
    pprefs->navL1HeadingMarginTightLevel            = 8;        // ht1-heading margin for tight level of distance relaxation
    pprefs->navL1DistThresholdTightLevel            = 25;       // hd1-distance threshold for tight level of distance relaxation

    pprefs->navMinSpeedValidHeadingCar              = 5.50;     // msvhc-minimum speed at which the heading portion of the GPS fix is considered valid (car)
    pprefs->navMinSpeedValidHeadingPed              = 0.75;     // msvhv-minimum speed at which the heading portion of the GPS fix is considered valid (pedestrian)
    pprefs->navHeadingMargin                        = 55;       // hm-max heading delta to be considered "right way" on a road
    pprefs->navMaxError                             = 55.0;     // maxerr-maximum error radius used for distance comparisons in navigation mode
    pprefs->navMinError                             = 40.0;     // minerr-minimum error assumed in all GPS fixes even if a low error radius is claimed
    pprefs->navMaxErrorTightLevel                   = 55.0;     // maxerr-maximum error radius used for distance comparisons in navigation mode for tight level
    pprefs->navMinErrorTightLevel                   = 30.0;     // minerr-minimum error assumed in all GPS fixes even if a low error radius is claimed for tight level
    pprefs->navSifterError                          = 100.0;    // sifter-maximum error radius for a valid GPS fix
    pprefs->navStartThresh                          = 250.0;    // start-distance threshold for special start case
    pprefs->navTurnThreshold                        = 45.0;     // turn-distance threshold for special turn case
    pprefs->navHideDistThreshold                    = 100.0;    // hide-distance threshold to hide distance on OR
    pprefs->navTurnBuf                              = 5.0;      // turnbuf-heading buffer for turn case

    pprefs->navSoftRecalcDist                       = 1607;

    pprefs->minCameraInstructDistance               = 200.0;    // Default 200 meters
    pprefs->navCameraDistThreshold                  = 100.0;    // Default 100 meters

    pprefs->navAvgSpeedA                            = 0.98;
    pprefs->navNumPtsForValid                       = 5;        // nas-min-val-min points for valid average
    pprefs->navDefaultManeuverSpeed                 = 5.5;      // nas-def-spd-default speed for maneuvers
    pprefs->navAvgSpeedThreshold                    = 5.5;
    pprefs->navSvgSpeedMinUpdateBelow               = 60;
    pprefs->navAvgSpeedMinUpdateAbove               = 1;

    pprefs->navMaxTrafficIncidentAnnounce           = 10000.0;  // 10km,6.2miles
    pprefs->navMinTrafficIncidentAnnounce           = 500;      // 500 m
    pprefs->navMaxTrafficCongestionAnnounce         = 10000.0;  // 10km,6.2miles
    pprefs->navMinTrafficCongestionAnnounce         = 500;      // 500 m
    pprefs->navMinTrafficCongestionLengthAnnounce   = 1609;     // 1 mile in meters

    pprefs->maxTowardDistAnnounce                   = 8046.72;

    pprefs->minTrafficMeterLightTraffic             = 0;
    pprefs->maxTrafficMeterLightTraffic             = 0;
    pprefs->minTrafficMeterModerateTraffic          = 25;
    pprefs->maxTrafficMeterModerateTraffic          = 50;
    pprefs->minTrafficMeterSevereTraffic            = 0;
    pprefs->maxTrafficMeterSevereTraffic            = 25;
    pprefs->minTrafficMeterDelayAnnounce            = 0;
    pprefs->maxTrafficMeterDelayAnnounce            = 50;
    pprefs->minTrafficMeterChangeToAnnounce         = 50;
    pprefs->minRemainTrafficUpdateAnnounce          = 10 * 60;

    pprefs->snapMapThreshold                        = 50.0;

    pprefs->navPedRecalcThreshold                   = 50.0;

    pprefs->turnMapHeadingDistanceFromManeuver      = 90.0;     /* ArrowBeforePx * pixelPerMeter = 45 * 2 = 90 meter */

    pprefs->maxEnhancedContentPrefetchManeuvers     = 5;
    pprefs->minGapBetweenContinueAndPrepareAnnounce = 15;

    init_navinstruct(&pprefs->instruct[manuever_type_normal], 7.999, 37.144,   6.9,   11.5,  4.0,  3.0,   0.5,  4.0,  4.0,  4.0,  1000.0, 300.0, 1500.0, 450.0);
    init_navinstruct(&pprefs->instruct[maneuver_type_hwy],    8.0,   40.0,     8.05,  17.25, 6.0,  4.0,   1.0,  6.0,  4.0,  6.0,  1500.0, 450.0, 2000.0, 600.0);
    init_navinstruct(&pprefs->instruct[maneuver_type_dest],   3.0,   50.0,     86.25, 115.0, 8.0,  5.0,   0.5,  8.0,  4.0,  8.0,  1000.0, 300.0, 1500.0, 450.0);
    init_navinstruct(&pprefs->instruct[maneuver_type_origin], 0,     100000.0, 1.4,   1.5,   1.2,  1.1,   1.0,  1.2,  4.0,  1.2,  1000.0, 300.0, 1500.0, 450.0);
    init_navinstruct(&pprefs->instruct[maneuver_type_merge],  8.0,   40.0,     8.05,  17.25, 5.75, 4.025, 2.3,  5.75, 4.0,  5.75, 1000.0, 300.0, 1500.0, 450.0);
    init_navinstruct(&pprefs->instruct[maneuver_type_ferry],  7.999, 37.144,   6.9,   11.5,  4.6,  2.3,   1.15, 4.6,  4.0,  4.6,  1000.0, 300.0, 1500.0, 450.0);
    /* Simulated speed camera instruction */
    init_navinstruct(&pprefs->instruct[maneuver_type_camera], 13.0,  10.0,     5.0,   0.0,   0.0,  0.0,   1.0,  0.0,  1.15, 0,   0.0,    0.0,   0.0,    0.0);

    return NE_OK;
}

NB_Error
NB_RouteParametersGetDestination(NB_RouteParameters* pThis, NB_LatitudeLongitude* destinationLatitudeLongitude, const char** destinationStreet)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (destinationLatitudeLongitude)
    {
        destinationLatitudeLongitude->latitude = pThis->dataQuery.destination.point.lat;
        destinationLatitudeLongitude->longitude = pThis->dataQuery.destination.point.lon;
    }

    if (destinationStreet)
    {
        *destinationStreet = data_string_get(NB_ContextGetDataState(pThis->context), &pThis->dataQuery.destination.str);
    }

    return NE_OK;
}

NB_Error
SetRouteSettings(data_util_state* pds, data_nav_query* pnq, NB_RouteSettings* pSettings)
{
    NB_Error err = NE_OK;
    const char* optimize;
    const char* vehicle;

    switch (pSettings->route_type)
    {
    case NB_RouteType_Fastest:
        optimize = "fastest";
        break;
    case NB_RouteType_Shortest:
        optimize = "shortest";
        break;
    case NB_RouteType_Easiest:
        optimize = "easiest";
        break;
    default:
        optimize = "fastest";
    }

    switch (pSettings->vehicle_type) {

        case NB_TransportationMode_Car:
            vehicle = "car";
            break;
        case NB_TransportationMode_Truck:
            vehicle = "truck";
            break;
        case NB_TransportationMode_Bicycle:
            vehicle = "bicycle";
            break;
        case NB_TransportationMode_Pedestrian:
            vehicle = "pedestrian";
            break;
        default:
            vehicle = "car";
    }

    err = err ? err : data_string_set(pds, &pnq->route_style.optimize, optimize);
    err = err ? err : data_string_set(pds, &pnq->route_style.vehicle_type, vehicle);

    pnq->route_style.avoid_hov      = (pSettings->route_avoid & NB_RouteAvoid_HOV) ? TRUE : FALSE;
    pnq->route_style.avoid_toll     = (pSettings->route_avoid & NB_RouteAvoid_Toll) ? TRUE : FALSE;
    pnq->route_style.avoid_highway  = (pSettings->route_avoid & NB_RouteAvoid_Highway) ? TRUE : FALSE;
    pnq->route_style.avoid_uturn    = (pSettings->route_avoid & NB_RouteAvoid_UTurn) ? TRUE : FALSE;
    pnq->route_style.avoid_unpaved  = (pSettings->route_avoid & NB_RouteAvoid_Unpaved) ? TRUE : FALSE;
    pnq->route_style.avoid_ferry    = (pSettings->route_avoid & NB_RouteAvoid_Ferry) ? TRUE : FALSE;

    pnq->route_settings = *pSettings;

    return err;
}

NB_Error
SetQueryTrafficFlags(data_nav_query* pnq, NB_TrafficType trafficType)
{
    pnq->want_historical_traffic    = (trafficType & NB_TrafficType_HistoricalRoute) ? TRUE : FALSE;
    pnq->want_realtime_traffic      = (trafficType & NB_TrafficType_RealTimeRoute) ? TRUE : FALSE;
    pnq->want_realtime_speed        = (trafficType & NB_TrafficType_RealTimeSpeed) ? TRUE : FALSE;
    pnq->want_historical_speed      = (trafficType & NB_TrafficType_HistoricalSpeed) ? TRUE : FALSE;
    pnq->want_traffic_notification  = (trafficType & NB_TrafficType_Notification) ? TRUE : FALSE;

    return NE_OK;
}

NB_Error
AddDetours(NB_RouteParameters* pThis, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, data_util_state* dataState, nb_boolean* detoursWantHistoricalTraffic, nb_boolean* detoursWantRealtimeTraffic)
{
    const double PRE_INCIDENT_AVOID_LENGTH = 1000.0;
    const double MINIMUM_START_DISTANCE = 0.1;

    NB_Error err = NE_OK;
    int i = 0;
    int detourCount = NB_DetourParametersGetCount(detour);

    *detoursWantHistoricalTraffic = FALSE;
    *detoursWantRealtimeTraffic = FALSE;

    for (i = 0; !err && i < detourCount; i++)
    {
        boolean wantHistoricalTraffic = FALSE;
        boolean wantRealtimeTraffic = FALSE;
        NB_DetourDetails details = { 0 };
        double startDistance = 0.0;
        double length = 0.0;

        err = NB_DetourParametersGetDetourDetails(detour, i, &details);
        if (!err)
        {
            switch (details.type)
            {
            case NB_DT_EntireRoute:
                startDistance = MINIMUM_START_DISTANCE;
                err = NB_RouteInformationGetSummaryInformation(route, NULL, NULL, &length);
                break;

            case NB_DT_PartOfRoute:
                startDistance = details.startDistance;
                length = details.length;
                break;

            case NB_DT_Incident:
                if (traffic)
                {
                    wantHistoricalTraffic = TRUE;
                    wantRealtimeTraffic = TRUE;

                    err = NB_TrafficInformationGetIncident(traffic, details.incidentIndex, NULL, NULL, &startDistance);

                    startDistance -= PRE_INCIDENT_AVOID_LENGTH;
                    length = PRE_INCIDENT_AVOID_LENGTH;
                }
                else
                {
                    err = NE_INVAL;
                }
                break;

            case NB_DT_Maneuver:
                {
                    uint32 numberOfManeuvers = NB_RouteInformationGetManeuverCount(route);
                    uint32 maneuverIndex = 0;
                    data_nav_maneuver* pmaneuver = NULL;
                    double distance = 0.0;
                    double routedist = 0.0;

                    for (maneuverIndex = 0; maneuverIndex < details.startIndex && maneuverIndex < numberOfManeuvers; maneuverIndex++)
                    {
                        pmaneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
                        routedist = routedist + pmaneuver->distance;
                    }

                    distance = 0.0;
                    while (maneuverIndex <= details.endIndex && maneuverIndex < numberOfManeuvers)
                    {
                        pmaneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
                        distance = distance + pmaneuver->distance;
                        maneuverIndex++;
                    }

                    startDistance = routedist;
                    length = distance;
                }
                break;

            case NB_DT_Congestion:
                wantHistoricalTraffic = TRUE;
                wantRealtimeTraffic = TRUE;
                break;

            case NB_DT_TrafficEvent:
                if (traffic)
                {
                   NB_TrafficEvent trafficEvent = { 0 };
                   double routeLength = 0.0;

                    err = NB_TrafficInformationGetTrafficEvent(traffic, details.eventIndex, &trafficEvent);
                    err = err ? err : NB_RouteInformationGetSummaryInformation(route, NULL, NULL, &routeLength);

                    if (!err)
                    {
                        wantHistoricalTraffic = TRUE;
                        wantRealtimeTraffic = TRUE;

                        switch (trafficEvent.type)
                        {
                        case NB_TET_Incident:
                            startDistance = routeLength - trafficEvent.routeRemainAfterEvent - PRE_INCIDENT_AVOID_LENGTH;
                            startDistance = startDistance > 0.0 ? startDistance : MINIMUM_START_DISTANCE;
                            length = PRE_INCIDENT_AVOID_LENGTH;
                            break;

                        case NB_TET_Congestion:
                            startDistance = routeLength - trafficEvent.routeRemainAfterEvent;
                            length = trafficEvent.detail.congestion.length;
                            break;

                        default:
                            err = NE_INVAL;
                            break;
                        }
                    }
                }
            }

            if (wantHistoricalTraffic)
            {
                *detoursWantHistoricalTraffic = TRUE;
            }

            if (wantRealtimeTraffic)
            {
                *detoursWantRealtimeTraffic = TRUE;
            }

             // server avoids the whole route if startDistance == 0.0 AND length == 0.0
            if ( (startDistance != 0.0) || (length != 0.0) )
            {
                err = err ? err : data_nav_query_add_detour(dataState, &pThis->dataQuery, &route->dataReply.route_id, startDistance, length, details.detourName, wantHistoricalTraffic, wantRealtimeTraffic);
            }
        }
    }

    return err;
}

void DataPointToLatitudeLongitude(data_point* point, NB_LatitudeLongitude* latLong)
{
    latLong->latitude = point->lat;
    latLong->longitude = point->lon;
}

NB_Error NB_RouteParametersGetTrafficFlags(NB_RouteParameters* pThis, NB_TrafficType* flags)
{
    if (!pThis || !flags)
    {
        return NE_INVAL;
    }
    *flags = NB_TrafficType_None;

    if (pThis->dataQuery.want_historical_traffic)
    {
        *flags |= NB_TrafficType_HistoricalRoute;
    }

    if (pThis->dataQuery.want_realtime_traffic)
    {
        *flags |= NB_TrafficType_RealTimeRoute;
    }

    if (pThis->dataQuery.want_realtime_speed)
    {
        *flags |= NB_TrafficType_RealTimeSpeed;
    }

    if (pThis->dataQuery.want_historical_speed)
    {
        *flags |= NB_TrafficType_HistoricalSpeed;
    }

    if (pThis->dataQuery.want_traffic_notification)
    {
        *flags |= NB_TrafficType_Notification;
    }

    return NE_OK;
}

NB_Error
NB_RouteParametersGetMaxPronunFiles(NB_RouteParameters* pThis, uint32* maxPronunFiles)
{
    if (!pThis || !maxPronunFiles)
    {
        return NE_INVAL;
    }

    *maxPronunFiles = pThis->config.maxPronunFiles;
    return NE_OK;
}

NB_Error
NB_RouteParametersSetMaxPronunFiles(NB_RouteParameters* pThis, uint32 maxPronunFiles)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->config.maxPronunFiles = maxPronunFiles;
    pThis->dataQuery.max_pronun_files = pThis->config.maxPronunFiles;

    return NE_OK;
}

NB_DEF NB_Error
NB_RouteParametersSetLanguage(NB_RouteParameters* pThis, const char* language)
{
    data_util_state* pds = NULL;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);

    if (language)
    {
        pThis->language = nsl_strdup(language);
        data_string_set(pds, &pThis->dataQuery.language, language);
    }
    else
    {
        /* set language to default */
        pThis->language = NULL;
        data_string_set(pds, &pThis->dataQuery.language, "");
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_SetDataQueryOptionsForRouteParameters(NB_RouteParameters* pRouteParameters, NB_RouteOptions* options)
{
    if(!pRouteParameters || !options)
    {
        return NE_INVAL;
    }

    SetDataQueryOptions(pRouteParameters, options, &pRouteParameters->dataQuery);
    return NE_OK;
}

void NavigationInstructionToPrivateInstruction(NB_NavigationInstruction* navigationInstruction, navinstruct* instruction)
{
    init_navinstruct(instruction,
                     navigationInstruction->baseDistMul,
                     navigationInstruction->baseDistAdd,
                     navigationInstruction->mult[NBIDT_AnnounceDistance],
                     navigationInstruction->mult[NBIDT_ContinueDistance],
                     navigationInstruction->mult[NBIDT_PrepareDistance],
                     navigationInstruction->mult[NBIDT_PrepareMinDistance],
                     navigationInstruction->mult[NBIDT_InstructionDistance],
                     navigationInstruction->mult[NBIDT_ShowStackDistance],
                     navigationInstruction->mult[NBIDT_ShowVectorDistance],
                     navigationInstruction->mult[NBIDT_LaneGuidanceDistance],
                     navigationInstruction->ShowTapeDistanceInInches,
                     navigationInstruction->ShowTapeDistanceInMeters,
                     navigationInstruction->TapeDistanceInMeters,
                     navigationInstruction->TapeDistanceInInches);
}


NB_Error
NB_RouteParametersSetNavigationCommandType(NB_RouteParameters* pRouteParameters, const char * command, NB_ManueverType type)
{
    NB_Error err = NE_OK;
    int i = 0;
    enum manuever_type manueverType;

    if (! pRouteParameters || !command ||type >= NBMT_Max || type < NBMT_Normal)
    {
        return NE_INVAL;
    }

    switch (type)
    {
        case NBMT_Normal:
        {
            manueverType = manuever_type_normal;
            break;
        }
        case NBMT_Highway:
        {
            manueverType = maneuver_type_hwy;
            break;
        }
        case NBMT_Destination:
        {
            manueverType = maneuver_type_dest;
            break;
        }
        case NBMT_Origin:
        {
            manueverType = maneuver_type_origin;
            break;
        }
        case NBMT_Merge:
        {
            manueverType = maneuver_type_merge;
            break;
        }
        case NBMT_Ferry:
        {
            manueverType = maneuver_type_ferry;
            break;
        }
        case NBMT_Camera:
        {
            manueverType = maneuver_type_camera;
            break;
        }
        default:
        {
            manueverType = manuever_type_normal;
            break;
        }
    }

    for (i =0; i < pRouteParameters->privateConfig.prefs.numNavTurnIndex; i++)
    {
        if (nsl_strcmp(command, pRouteParameters->privateConfig.prefs.navTurnIndex[i].command) == 0)
        {
            pRouteParameters->privateConfig.prefs.navTurnIndex[i].type = manueverType;
            return NE_OK;
        }
    }

    if (i < MAX_NUM_TURN_COMMAND)
    {
        nsl_strlcpy(pRouteParameters->privateConfig.prefs.navTurnIndex[i].command, command,
                    sizeof(pRouteParameters->privateConfig.prefs.navTurnIndex[i].command));
        pRouteParameters->privateConfig.prefs.navTurnIndex[i].type = manueverType;
        pRouteParameters->privateConfig.prefs.numNavTurnIndex ++;
    }

    return err;
}

NB_DEF NB_Error
NB_RouteParametersSetNavigationInstruction(NB_RouteParameters* pRouteParameters, NB_NavigationInstruction* instruction, NB_ManueverType maneuverType)
{
    if(!pRouteParameters || !instruction || maneuverType >= NBMT_Max || maneuverType < NBMT_Normal)
    {
        return NE_INVAL;
    }

    switch (maneuverType)
    {
        case NBMT_Normal:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[manuever_type_normal]);
            break;
        }
        case NBMT_Highway:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_hwy]);
            break;
        }
        case NBMT_Destination:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_dest]);
            break;
        }
        case NBMT_Origin:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_origin]);
            break;
        }
        case NBMT_Merge:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_merge]);
            break;
        }
        case NBMT_Ferry:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_ferry]);
            break;
        }
        case NBMT_Camera:
        {
            NavigationInstructionToPrivateInstruction(instruction, &pRouteParameters->privateConfig.prefs.instruct[maneuver_type_camera]);
            break;
        }
        default:
        {
            break;
        }
    }

    return NE_OK;
}

enum manuever_type
NB_RouteParametersGetCommandType(NB_RouteParameters* pRouteParameters,
                                 const char * command
                                 )
{
    int i = 0;
    if (!command)
    {
        return manuever_type_normal;
    }

    if (! pRouteParameters ||pRouteParameters->privateConfig.prefs.numNavTurnIndex <= 0  )
    {
        return get_maneuver_type(command);
    }

    for (i = 0; i < pRouteParameters->privateConfig.prefs.numNavTurnIndex; i++)
    {
        if (nsl_strcmp(command, pRouteParameters->privateConfig.prefs.navTurnIndex[i].command) == 0)
        {
            return pRouteParameters->privateConfig.prefs.navTurnIndex[i].type;
        }
    }

    return manuever_type_normal;
}

NB_DEF NB_Error
NB_RouteParametersSetConfigurationFromBuffer(NB_Context* context,
                                             NB_RouteParameters* pRouteParameters,
                                             char *datalib,
                                             nb_size datalibSize,
                                             char * configBuffer,
                                             nb_size configBufferSize
                                             )
{
    NB_Error err = NE_OK;
    if (! context || !pRouteParameters || !datalib || !configBuffer)
    {
        return NE_INVAL;
    }

    err = GetConfigurationDataFromBuffer(context, datalib, datalibSize, configBuffer, configBufferSize, pRouteParameters);
    return err ;
}

NB_DEF NB_Error
NB_RouteParametersSetFontMapFromBuffer(NB_Context * context,
                                       NB_RouteParameters* pRouteParameters,
                                       char *datalib,
                                       nb_size datalibSize,
                                       char * fontMapBuffer,
                                       nb_size fontMapBufferSize
                                       )
{
    NB_Error err = NE_OK;
    if (! context || !pRouteParameters || !datalib || !fontMapBuffer)
    {
        return NE_INVAL;
    }

    err = GetFontMapFromBuffer(context, datalib, datalibSize, fontMapBuffer, fontMapBufferSize, pRouteParameters);
    return err ;
}

NB_DEF NB_Error
NB_RouteParametersSetHOVFontMapFromBuffer(NB_Context * context,
                                          NB_RouteParameters* pRouteParameters,
                                          char *datalib,
                                          nb_size datalibSize,
                                          char * HOVFontMapBuffer,
                                          nb_size HOVFontMapBufferSize
                                          )
{
    NB_Error err = NE_OK;
    if (! context || !pRouteParameters || !datalib || !HOVFontMapBuffer)
    {
        return NE_INVAL;
    }

    err = GetHOVFontMapFromBuffer(context, datalib, datalibSize, HOVFontMapBuffer, HOVFontMapBufferSize, pRouteParameters);
    return err ;
}

NB_Error
NB_RouteParametersGetCommandCharactersFromMapFont(NB_RouteParameters* pRouteParameters,
                                                  char * command,
                                                  char * commandChars,
                                                  int    commandCharsBufferSize
                                                  )
{
    if( ! commandChars || !pRouteParameters || !command )
    {
        return NE_INVAL;
    }
    // TODO: Current not support
    return NE_NOSUPPORT;
    }

NB_Error
NB_RouteParametersGetLaneCharactersFromMapFont(NB_RouteParameters* pRouteParameters,
                                               uint32 laneType,
                                               uint32 unselectedArrows,
                                               uint32 selectedArrows,
                                               char * unselectedCharsBuffer,
                                               int    unselectedCharsBufferSize,
                                               char * selectedCharsBuffer,
                                               int    selectedCharsBufferSize
                                               )
{
    int  count = 0;
    int  i = 0;
    LaneGuidanceFontMap * map =  NULL;
    HovFontMap * hovMap = NULL;
    uint32 key = 0;

    if( ! unselectedCharsBuffer || !pRouteParameters || !selectedCharsBuffer )
    {
        return NE_INVAL;
    }

    key = unselectedArrows << 8 | selectedArrows;

    map = CSL_HashTableFind(pRouteParameters->privateConfig.prefs.laneGuidanceFontMap, (byte *)&key, sizeof(key));

    if (map)
    {
        nsl_strlcpy(selectedCharsBuffer, map->selectedChars, selectedCharsBufferSize);
        nsl_strlcpy(unselectedCharsBuffer, map->unselectedChars, unselectedCharsBufferSize);
    }
    else
    {
        nsl_strlcpy(unselectedCharsBuffer, " ", unselectedCharsBufferSize);
        return NE_NOENT;
    }

    if (laneType == LT_HOV)
    {
        count = pRouteParameters->privateConfig.prefs.numHovFontMap;
        hovMap = pRouteParameters->privateConfig.prefs.hovFontMap;
        for (i = 0; i < count; i++)
        {
            if (
                nsl_strncmp(hovMap[i].arrowChars, selectedCharsBuffer,selectedCharsBufferSize) == 0
                || nsl_strncmp(hovMap[i].arrowChars, unselectedCharsBuffer,unselectedCharsBufferSize) == 0
                )
            {
                nsl_strlcat(unselectedCharsBuffer, hovMap[i].hovChars, unselectedCharsBufferSize);
                break;
            }
        }
    }
    return NE_OK;
}

NB_Error
NB_RouteParametersSetViaPoint(NB_RouteParameters* pRouteParameters, NB_WayPoint* pWayPoint)
{
    data_util_state* pds = NULL;
    data_via_point viapoint;
    if(!pRouteParameters || !pWayPoint)
    {
        return NE_INVAL;
    }
    pds = NB_ContextGetDataState(pRouteParameters->context);
    if(pRouteParameters->dataQuery.via_point)
    {
        data_via_point_init(pds, &viapoint);
        data_via_point_from_nimlocation(pds, &viapoint, &(pWayPoint->location));
        viapoint.isStopPoint = pWayPoint->isStopPoint;
        CSL_VectorAppend(pRouteParameters->dataQuery.via_point, &viapoint);
    }
    return NE_OK;
}

NB_Error
NB_RouteParametersSetRouteSummaryDestination(NB_RouteParameters* pRouteParameters, NB_Place* pPlace)
{
    data_util_state* pds = NULL;
    data_destination dest;
    if(!pRouteParameters || !pPlace)
    {
        return NE_INVAL;
    }
    pds = NB_ContextGetDataState(pRouteParameters->context);
    if(pRouteParameters->dataQuery.extra_destination)
    {
        data_destination_init(pds, &dest);
        SetDestinationFromPlace(pds, &dest, pPlace);
        CSL_VectorAppend(pRouteParameters->dataQuery.extra_destination, &dest);
    }
    return NE_OK;
}

