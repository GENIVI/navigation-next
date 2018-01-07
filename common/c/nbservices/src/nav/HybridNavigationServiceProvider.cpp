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
   @file        HybridNavigationServiceProvider.cpp
   @defgroup    c++
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

#include "HybridNavigationServiceProvider.h"
#include "commoncomponentconfiguration.h"
extern "C"
{
#include "nbcontextprotected.h"
#include "nbhybridmanager.h"
#include "nbrouteparameters.h"
#include "nbrouteparametersprivate.h"
#include "offboardroutehandler.h"
#include "nbrouteinformationprivate.h"
#include "palfile.h"
#include "jsonstylelogger.h"
}
#include "HybridStrategy.h"
#include "OnboardRouteHandler.h"
#include "HybridManager.h"

struct HybridRouteHandler
{
    NB_RouteHandler         routeHandler;       //It must be the first field of HybridRouteHandler
    OffBoardRouteHandler*   offBoardRouteHandler;
    OnboardRouteHandler*    onBoardRouteHandler;
    HybridMode              hybridMode;
};

NB_Error InitializeRouteHandler(HybridRouteHandler* hybridRouteHandler, NB_Context* context)
{
    NB_Error error;
    NB_RouteHandler* nbRouteHandler = &hybridRouteHandler->routeHandler;
    if(hybridRouteHandler->hybridMode == HBM_ONBOARD_ONLY)
    {
        error = OnboardRouteHandlerSetup(context, nbRouteHandler, hybridRouteHandler->onBoardRouteHandler);
    }
    else
    {
        error = OffboardRouteHandlerSetup(context, hybridRouteHandler->offBoardRouteHandler);
    }
    return error;
}

NB_Error Hybrid_RouteHandlerCreate(NB_Context*                context,
                                   NB_RequestHandlerCallback* callback,
                                   NB_RouteHandler**          handler)
{
    NB_ASSERT_VALID_THREAD(context);
    if (!context || !handler || !callback)
    {
        return NE_INVAL;
    }

    HybridRouteHandler* hybridRouteHandler = new HybridRouteHandler;
    memset(hybridRouteHandler, 0, sizeof(HybridRouteHandler));
    shared_ptr<HybridManager> hybridManager = HybridManager::GetInstance(context);
    hybridRouteHandler->hybridMode = hybridManager->GetMode();

    NB_RouteHandler* nbRouteHandler = &hybridRouteHandler->routeHandler;
    NB_Error error = NE_OK;
    hybridRouteHandler->onBoardRouteHandler = new OnboardRouteHandler();
    hybridRouteHandler->offBoardRouteHandler = &hybridRouteHandler->routeHandler;

    error = InitializeRouteHandler(hybridRouteHandler, context);

    if(error == NE_OK)
    {
        nbRouteHandler->callback.callback     = callback->callback;
        nbRouteHandler->callback.callbackData = callback->callbackData;
        *handler = (NB_RouteHandler*)hybridRouteHandler;
    }
    else
    {
        Hybrid_RouteHandlerDestroy((NB_RouteHandler*)hybridRouteHandler);
    }
    return error;
}

NB_Error Hybrid_RouteHandlerStartRequest(NB_RouteHandler*    handler,
                                         NB_RouteParameters* parameters)
{
    NB_ASSERT_VALID_THREAD(context);
    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

    if (Hybrid_RouteHandlerIsRequestInProgress(handler))
    {
        return NE_BUSY;
    }

    HybridRouteHandler* hybridRouteHandler = (HybridRouteHandler*)handler;
    shared_ptr<HybridManager> hybridManager = HybridManager::GetInstance(handler->context);
    HybridMode hybridMode = hybridManager->GetMode();
    if(hybridMode != hybridRouteHandler->hybridMode)
    {
        hybridRouteHandler->hybridMode = hybridMode;
        parameters->dataQuery.has_recalc = FALSE;
        InitializeRouteHandler(hybridRouteHandler, handler->context);
    }

    if (handler->parameters)
    {
        NB_RouteParametersDestroy(handler->parameters);
        handler->parameters = NULL;
    }
    NB_RouteParametersClone(parameters, &handler->parameters);
    if (!handler->parameters)
    {
        return NE_NOMEM;
    }

    handler->download_inprogress = TRUE;
    handler->download_percent = 0;

    NB_Error error = NE_OK;
    if(hybridRouteHandler->hybridMode == HBM_ONBOARD_ONLY)
    {
        error = OnboardRouteHandlerStartRequest(hybridRouteHandler->onBoardRouteHandler);
    }
    else
    {
        error = OffboardRouteHandlerStartRequest(hybridRouteHandler->offBoardRouteHandler);
    }
    return error;
}

NB_Error Hybrid_RouteHandlerDestroy(NB_RouteHandler* handler)
{
    NB_ASSERT_VALID_THREAD(context);
    if (!handler || !handler->context)
    {
        return NE_INVAL;
    }
    NB_RouteHandlerCancelRequest(handler);
    if (handler->parameters)
    {
        NB_RouteParametersDestroy(handler->parameters);
    }

    HybridRouteHandler* hybridRouteHandler = (HybridRouteHandler*)handler;
    NB_Error error = NE_OK;
    if(hybridRouteHandler->hybridMode == HBM_ONBOARD_ONLY)
    {
        error = OnboardRouteHandlerTearDown(hybridRouteHandler->onBoardRouteHandler);
    }
    else
    {
        error = OffboardRouteHandlerTearDown(hybridRouteHandler->offBoardRouteHandler);
    }
    delete hybridRouteHandler->onBoardRouteHandler;
    delete hybridRouteHandler;
    return error;
}

NB_Error
Hybrid_RouteHandlerCancelRequest(NB_RouteHandler* handler)
{
    NB_ASSERT_VALID_THREAD(context);
    if (!handler || !handler->context)
    {
        return NE_INVAL;
    }
    if (handler->parameters)
    {
        NB_RouteParametersDestroy(handler->parameters);
        handler->parameters = NULL;
    }

    HybridRouteHandler* hybridRouteHandler = (HybridRouteHandler*)handler;
    NB_Error error = NE_OK;
    if(hybridRouteHandler->hybridMode == HBM_ONBOARD_ONLY)
    {
        error = OnboardRouteHandlerCancelRequest(hybridRouteHandler->onBoardRouteHandler);
    }
    else
    {
        error = OffboardRouteHandlerCancelRequest(hybridRouteHandler->offBoardRouteHandler);
    }
    return error;
}

NB_Error
Hybrid_RouteHandlerGetRouteInformation(NB_RouteHandler* handler,
                                       NB_RouteInformation** route)
{
    if (!handler || !handler->context || !route)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(handler->context);

    HybridRouteHandler* hybridRouteHandler = (HybridRouteHandler*)handler;
    NB_Error error = NE_OK;
    if(hybridRouteHandler->hybridMode == HBM_ONBOARD_ONLY)
    {
        error = OnboardRouteHandlerGetRouteInformation(hybridRouteHandler->onBoardRouteHandler, route);
    }
    else
    {
        error = OffboardRouteHandlerGetRouteInformation(hybridRouteHandler->offBoardRouteHandler, route);
    }

    return error;
}

nb_boolean
Hybrid_RouteHandlerIsRequestInProgress(NB_RouteHandler*  handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }
    return handler ? handler->download_inprogress : 0;
}

static void LogRouteParametersAsJson(NB_RouteParameters* parameters)
{
    NB_JsonStyleLoggerBegin();
    //
    NB_JsonStyleLoggerLogString("pronun_style", parameters->dataQuery.pronun_style);
    NB_JsonStyleLoggerLogInt("command_set_version", parameters->dataQuery.command_set_version);
    //
    NB_JsonStyleLoggerLogInt("max_pronun_files", parameters->dataQuery.max_pronun_files);
    NB_JsonStyleLoggerLogString("language", parameters->dataQuery.language);
    NB_JsonStyleLoggerLogBoolean("start_to_navigate", parameters->dataQuery.start_to_navigate);
    //origin
    NB_JsonStyleLoggerStartObject("origin");
        NB_JsonStyleLoggerLogDouble("lat", parameters->dataQuery.origin.point.lat);
        NB_JsonStyleLoggerLogDouble("log", parameters->dataQuery.origin.point.lon);
    NB_JsonStyleLoggerEndObject();
    //destination
    NB_JsonStyleLoggerStartObject("destination");
        NB_JsonStyleLoggerLogDouble("lat", parameters->dataQuery.destination.point.lat);
        NB_JsonStyleLoggerLogDouble("log", parameters->dataQuery.destination.point.lon);
    NB_JsonStyleLoggerEndObject();
    //cache_contents
    NB_JsonStyleLoggerStartObject("cache_contents");
        NB_JsonStyleLoggerLogString("pronun_style", parameters->dataQuery.cache_contents.pronun_style_pair.value);
        NB_JsonStyleLoggerStartArray("cache_item");
        NB_JsonStyleLoggerEndArray();
    NB_JsonStyleLoggerEndObject();
    //map_cache_contents
    NB_JsonStyleLoggerStartObject("map_cache_contents");
    int l = CSL_VectorGetLength(parameters->dataQuery.map_cache_contents.vec_vector_tile_descriptors);
    for (int n=0;n<l;n++)
    {
        data_vector_tile_descriptor* vtd = (data_vector_tile_descriptor*)CSL_VectorGetPointer(
                    parameters->dataQuery.map_cache_contents.vec_vector_tile_descriptors, n);
        NB_JsonStyleLoggerStartObject("vector_tile_descriptors");
            NB_JsonStyleLoggerLogInt("tx", vtd->tx);
            NB_JsonStyleLoggerLogInt("ty", vtd->ty);
            NB_JsonStyleLoggerLogInt("tz", vtd->tz);
            NB_JsonStyleLoggerLogInt("min_priority", vtd->minPpriority);
            NB_JsonStyleLoggerLogInt("max_priority", vtd->maxPriority);
            NB_JsonStyleLoggerLogDouble("min_scale", vtd->minScale);
        NB_JsonStyleLoggerEndObject();
    }
    NB_JsonStyleLoggerEndObject();
    //route_style
    NB_JsonStyleLoggerStartObject("route_style");
        NB_JsonStyleLoggerLogBoolean("avoid_hov", parameters->dataQuery.route_style.avoid_hov);
        NB_JsonStyleLoggerLogBoolean("avoid_toll", parameters->dataQuery.route_style.avoid_toll);
        NB_JsonStyleLoggerLogBoolean("avoid_highway", parameters->dataQuery.route_style.avoid_highway);
        NB_JsonStyleLoggerLogBoolean("avoid_uturn", parameters->dataQuery.route_style.avoid_uturn);
        NB_JsonStyleLoggerLogBoolean("avoid_unpaved", parameters->dataQuery.route_style.avoid_unpaved);
        NB_JsonStyleLoggerLogBoolean("avoid_ferry", parameters->dataQuery.route_style.avoid_ferry);
        NB_JsonStyleLoggerLogString("optimize", parameters->dataQuery.route_style.optimize);
        NB_JsonStyleLoggerLogString("vehicle_type", parameters->dataQuery.route_style.vehicle_type);
    NB_JsonStyleLoggerEndObject();
    //recalc
    NB_JsonStyleLoggerStartObject("recalc");
        NB_JsonStyleLoggerLogString("why", parameters->dataQuery.recalc.why);
    NB_JsonStyleLoggerEndObject();
    //want_guidance_config
    NB_JsonStyleLoggerStartObject("want_guidance_config");
        NB_JsonStyleLoggerLogBoolean("want_enchanced_natural_guidance", parameters->dataQuery.want_guidance_config.want_enchanced_natural_guidance);
        NB_JsonStyleLoggerLogBoolean("want_natural_guidance", parameters->dataQuery.want_guidance_config.want_natural_guidance);
        NB_JsonStyleLoggerLogBoolean("want_lane_guidance", parameters->dataQuery.want_guidance_config.want_lane_guidance);
        NB_JsonStyleLoggerLogBoolean("want_play_times", parameters->dataQuery.want_guidance_config.want_play_times);
        NB_JsonStyleLoggerLogBoolean("want_guidance_config", parameters->dataQuery.want_guidance_config.want_guidance_config);
    NB_JsonStyleLoggerEndObject();
    //want_alternate_routes
    NB_JsonStyleLoggerStartObject("want_alternate_routes");
        NB_JsonStyleLoggerLogInt("vehicle_type", parameters->dataQuery.want_alternate_routes.max_routes);
    NB_JsonStyleLoggerEndObject();
    //want_extended_pronun_data
    NB_JsonStyleLoggerStartObject("want_extended_pronun_data");
        NB_JsonStyleLoggerLogString("supported_phonetics_formats", parameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats);
    NB_JsonStyleLoggerEndObject();
    //vec_detour_avoid
    NB_JsonStyleLoggerStartArray("detour_avoid");
    l = CSL_VectorGetLength(parameters->dataQuery.vec_detour_avoid);
    for(int i=0; i<l; ++i)
    {
        data_detour_avoid* detour_avoid = (data_detour_avoid*)CSL_VectorGetPointer(parameters->dataQuery.vec_detour_avoid, i);
        NB_JsonStyleLoggerStartObject("detour_avoid");
            NB_JsonStyleLoggerLogDouble("start", detour_avoid->start);
            NB_JsonStyleLoggerLogDouble("length", detour_avoid->length);
            NB_JsonStyleLoggerLogBoolean("want_historical_traffic", detour_avoid->want_historical_traffic);
            NB_JsonStyleLoggerLogBoolean("want_realtime_traffic", detour_avoid->want_realtime_traffic);
        NB_JsonStyleLoggerEndObject();
    }
    NB_JsonStyleLoggerEndArray();
    //
    NB_JsonStyleLoggerStartObject("want");
        NB_JsonStyleLoggerLogBoolean("want_complete_route", parameters->dataQuery.want_complete_route);
        NB_JsonStyleLoggerLogBoolean("want_origin_location", parameters->dataQuery.want_origin_location);
        NB_JsonStyleLoggerLogBoolean("want_destination_location", parameters->dataQuery.want_destination_location);
        NB_JsonStyleLoggerLogBoolean("want_route_map", parameters->dataQuery.want_route_map);
        NB_JsonStyleLoggerLogBoolean("want_cross_streets", parameters->dataQuery.want_cross_streets);
        NB_JsonStyleLoggerLogBoolean("want_maneuvers", parameters->dataQuery.want_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_all_maneuvers", parameters->dataQuery.want_all_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_historical_traffic", parameters->dataQuery.want_historical_traffic);
        NB_JsonStyleLoggerLogBoolean("want_realtime_traffic", parameters->dataQuery.want_realtime_traffic);
        NB_JsonStyleLoggerLogBoolean("want_historical_speed", parameters->dataQuery.want_historical_speed);
        NB_JsonStyleLoggerLogBoolean("want_realtime_speed", parameters->dataQuery.want_realtime_speed);
        NB_JsonStyleLoggerLogBoolean("want_traffic_notification", parameters->dataQuery.want_traffic_notification);
        NB_JsonStyleLoggerLogBoolean("want_unpaved_info", parameters->dataQuery.want_unpaved_info);
        NB_JsonStyleLoggerLogBoolean("want_speed_cameras", parameters->dataQuery.want_speed_cameras);
        NB_JsonStyleLoggerLogBoolean("want_ferry_maneuvers", parameters->dataQuery.want_ferry_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_towards_roadinfo", parameters->dataQuery.want_towards_roadinfo);
        NB_JsonStyleLoggerLogBoolean("want_intersection_roadinfo", parameters->dataQuery.want_intersection_roadinfo);
        NB_JsonStyleLoggerLogBoolean("want_country_info", parameters->dataQuery.want_country_info);
        NB_JsonStyleLoggerLogBoolean("want_enter_roundabout_maneuvers", parameters->dataQuery.want_enter_roundabout_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_enter_country_maneuvers", parameters->dataQuery.want_enter_country_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_bridge_maneuvers", parameters->dataQuery.want_bridge_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_tunnel_maneuvers", parameters->dataQuery.want_tunnel_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_route_extents", parameters->dataQuery.want_route_extents);
        NB_JsonStyleLoggerLogBoolean("want_realistic_road_signs", parameters->dataQuery.want_realistic_road_signs);
        NB_JsonStyleLoggerLogBoolean("want_junction_models", parameters->dataQuery.want_junction_models);
        NB_JsonStyleLoggerLogBoolean("want_city_models", parameters->dataQuery.want_city_models);
        NB_JsonStyleLoggerLogBoolean("want_exit_numbers", parameters->dataQuery.want_exit_numbers);
        NB_JsonStyleLoggerLogBoolean("want_speed_regions", parameters->dataQuery.want_speed_regions);
        NB_JsonStyleLoggerLogBoolean("want_first_major_road", parameters->dataQuery.want_first_major_road);
        NB_JsonStyleLoggerLogBoolean("want_label_point", parameters->dataQuery.want_label_point);
        NB_JsonStyleLoggerLogBoolean("want_non_guidance_maneuvers", parameters->dataQuery.want_non_guidance_maneuvers);
        NB_JsonStyleLoggerLogBoolean("want_max_turn_for_street_count", parameters->dataQuery.want_max_turn_for_street_count);
        NB_JsonStyleLoggerLogBoolean("want_route_summary", parameters->dataQuery.want_route_summary);
        NB_JsonStyleLoggerLogBoolean("want_special_regions", parameters->dataQuery.want_special_regions);
        NB_JsonStyleLoggerLogBoolean("want_first_maneuver_cross_streets", parameters->dataQuery.want_first_maneuver_cross_streets);
        NB_JsonStyleLoggerLogBoolean("want_secondary_name_for_exits", parameters->dataQuery.want_secondary_name_for_exits);
        NB_JsonStyleLoggerLogBoolean("want_check_unsupported_countries", parameters->dataQuery.want_check_unsupported_countries);
    NB_JsonStyleLoggerEndObject();

    NB_JsonStyleLoggerLogBoolean("has_recalc", parameters->dataQuery.has_recalc);
    NB_JsonStyleLoggerLogBoolean("disable_navigation", parameters->dataQuery.disable_navigation);
    //route map
    NB_JsonStyleLoggerStartObject("route_map");
        NB_JsonStyleLoggerLogInt("route_map_angle_of_view", parameters->dataQuery.route_map_angle_of_view);
        NB_JsonStyleLoggerLogInt("route_map_distance_of_view", parameters->dataQuery.route_map_distance_of_view);
        NB_JsonStyleLoggerLogInt("route_map_min_time", parameters->dataQuery.route_map_min_time);
        NB_JsonStyleLoggerLogInt("route_map_passes", parameters->dataQuery.route_map_passes);
        NB_JsonStyleLoggerLogInt("route_map_tile_z", parameters->dataQuery.route_map_tile_z);
        NB_JsonStyleLoggerLogInt("route_map_max_tiles", parameters->dataQuery.route_map_max_tiles);
    NB_JsonStyleLoggerEndObject();

    NB_JsonStyleLoggerEnd();


    PAL_File* logFile = NULL;
    PAL_FileOpen(NB_ContextGetPal(parameters->context), "nav_query.log", PFM_Append, &logFile);
    if(logFile)
    {
        uint32 written = 0;
        const char* logHeader = "\n\n=========\nRoute request with nav_query:\n";
        PAL_FileWrite(logFile, (const uint8*)logHeader, nsl_strlen(logHeader), &written);

        const char* logBuffer = NB_JsonStyleLoggerBuffer();
        PAL_FileWrite(logFile, (const uint8*)logBuffer, nsl_strlen(logBuffer), &written);
        PAL_FileClose(logFile);
    }
    NB_JsonStyleLoggerDestory();
}

static void LogRouteInformationAsJson(NB_RouteInformation* route)
{
    if(!route)
    {
        return;
    }

    data_nav_reply*  nav_reply = &route->dataReply;
    if(!nav_reply)
    {
        return;
    }

    NB_JsonStyleLoggerBegin();
    //nav_maneuver
    NB_JsonStyleLoggerStartArray("nav_maneuver");
    ///TODO
    NB_JsonStyleLoggerEndArray();

    //file
    NB_JsonStyleLoggerStartArray("file");
    ///TODO
    NB_JsonStyleLoggerEndArray();

    //detour_avoid
    NB_JsonStyleLoggerStartArray("detour_avoid");
    ///TODO
    NB_JsonStyleLoggerEndArray();

    //route_map
    NB_JsonStyleLoggerStartObject("route_map");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //origin
    NB_JsonStyleLoggerStartObject("origin");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //destination
    NB_JsonStyleLoggerStartObject("destination");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //traffic_record_identifier
    NB_JsonStyleLoggerStartObject("traffic_record_identifier");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //standard_traffic_flow
    NB_JsonStyleLoggerStartObject("standard_traffic_flow");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //historical_traffic_flow
    NB_JsonStyleLoggerStartObject("historical_traffic_flow");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //realtime_traffic_flow
    NB_JsonStyleLoggerStartObject("realtime_traffic_flow");
    ///TODO
    NB_JsonStyleLoggerEndObject();


    NB_JsonStyleLoggerLogInt("ferry_on_route", nav_reply->ferry_on_route);

    //vec_cameras
    NB_JsonStyleLoggerStartArray("speed_cameras");
    ///TODO
    NB_JsonStyleLoggerEndArray();

    //traffic_incidents
    NB_JsonStyleLoggerStartObject("traffic_incidents");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //route_extents
    NB_JsonStyleLoggerLogBoolean("has_route_extents", nav_reply->hasRouteExtents);
    NB_JsonStyleLoggerStartObject("route_extents");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //extended_content
    NB_JsonStyleLoggerStartObject("extended_content");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //speed_regions
    NB_JsonStyleLoggerStartObject("speed_regions");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //first_major_road
    NB_JsonStyleLoggerStartObject("first_major_road");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //pronun_list
    NB_JsonStyleLoggerStartObject("pronun_list");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //route_summary
    NB_JsonStyleLoggerStartObject("route_summary");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //vec_special_regions
    NB_JsonStyleLoggerStartArray("special_regions");
    ///TODO
    NB_JsonStyleLoggerEndArray();

    NB_JsonStyleLoggerLogDouble("polyline_length", nav_reply->polyline_length);
    NB_JsonStyleLoggerLogBoolean("polyline_complete", nav_reply->polyline_complete);

    NB_JsonStyleLoggerLogInt("nav_maneuver_start", nav_reply->navManeuverStart);
    NB_JsonStyleLoggerLogInt("nav_maneuver_end", nav_reply->navManeuverEnd);
    NB_JsonStyleLoggerLogInt("nav_maneuver_total", nav_reply->navManeuverTotal);

    //route_extents_all
    NB_JsonStyleLoggerStartObject("route_extents_all");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    NB_JsonStyleLoggerLogBoolean("has_route_extents_all", nav_reply->hasRouteExtentsAll);

    //route_selector_reply
    NB_JsonStyleLoggerStartObject("route_selector_reply");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    //route_selector_reply
    NB_JsonStyleLoggerStartObject("route_selector_reply");
    ///TODO
    NB_JsonStyleLoggerEndObject();
    NB_JsonStyleLoggerLogInt("active_route", nav_reply->active_route);


    NB_JsonStyleLoggerLogBoolean("tolls_on_route", nav_reply->tolls_on_route);
    NB_JsonStyleLoggerLogBoolean("hov_lanes_on_route", nav_reply->hov_lanes_on_route);
    NB_JsonStyleLoggerLogBoolean("highway_on_route", nav_reply->highway_on_route);
    NB_JsonStyleLoggerLogBoolean("unpaved_on_route", nav_reply->unpaved_on_route);

    //currentLabelPoint
    NB_JsonStyleLoggerStartObject("currentLabelPoint");
    ///TODO
    NB_JsonStyleLoggerEndObject();

    NB_JsonStyleLoggerEnd();
}
/*! @} */
