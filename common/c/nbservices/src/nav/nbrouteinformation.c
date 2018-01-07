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

    @file     nbrouteinformation.c
    @defgroup nbrouteinfo

    Information about route
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "nbqalog.h"
#include "nbrouteinformation.h"
#include "nbrouteinformationprivate.h"
#include "nbguidanceinformationprivate.h"
#include "nbrouteparametersprivate.h"
#include "nbgpstypes.h"
#include "nbqalog.h"
#include "data_nav_reply.h"
#include "data_extended_content_region.h"
#include "data_content_path.h"
#include "data_string.h"
#include "navutil.h"
#include "navtypes.h"
#include "nbutility.h"
#include "nbformat.h"
#include "nbspatial.h"
#include "format.h"
#include "instructset.h"
#include "cslcache.h"
#include "nbnavigationprivate.h"
#include "nbcameraprocessorprivate.h"
#include "nbpointiteration.h"
#include "palmath.h"
#include "nbnaturallaneguidancetype.h"
#include "data_lane_guidance_item.h"
#include "data_lane_info.h"
#include "cslqarecorddefinitions.h"
// @todo: remove this header and use new API to get guidanceState
#include "nbguidanceprocessorprivate.h"
#include "data_pronun_extended.h"
#include "transformutility.h"
#include "vec.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"

#define COMMAND_ID_CHARACTERS_MAX 40
#define COMMAND_CHARACTERS_MAX 5
#define MIN_DISTANCE_BETWEEN_CROSS_STREETS 75
#define LONG_MANEUVER_LIMIT 8000

static void NB_RoutePronunCacheUpdate(data_file* pf, void* pUser);

static void ComputeContentRegionMeasurements(NB_RouteInformation* route, NB_RouteContentRegion* region);

static nb_boolean IsLastPolylineSegment(data_util_state* dataState, data_polyline* polyline, uint32 segment);
static nb_boolean IsItemInVoiceCache(NB_Context* context, byte* data, uint32 size);
static data_guidance_point* GetGuidancePointPointer(data_nav_maneuver* maneuver, NB_NavigateManeuverPos position);

static const char* GetHeadingName(double heading);
static const char* GetDirectionName(double turnAngle, double tolerance);

static void QaLogReply(NB_Context* context, NB_RouteInformation* route);
static void QaLogRouteTrafficRegion(NB_Context* context, NB_RouteInformation* route);
static void QaLogRouteRealisticSigns(NB_Context* context, NB_RouteInformation* route);
static void QaLogRouteEnhancedContentInfo(NB_Context* context, NB_RouteInformation* route);

static void QaLogNaturalLaneGuidanceInformation(NB_Context* context, int32 maneuverIndex, data_nav_maneuver* maneuver);

typedef enum
{
    NB_TIT_NONE = 0,
    NB_TIT_ACCIDENT = 1,
    NB_TIT_CONGESTION = 2,
    NB_TIT_DISABLED_VEHICLE = 3,
    NB_TIT_ROAD_HAZARD = 4,
    NB_TIT_UNSCHEDULED_CONSTRUCTION = 5,
    NB_TIT_SCHEDULED_CONSTRUCTION = 6,
    NB_TIT_PLANNED_EVENT = 7,
    NB_TIT_MASS_TRANSIT = 8,
    NB_TIT_OTHER_NEWS = 9,
    NB_TIT_MISC = 11
} NB_TrafficIncidentType;

static void UpdateOnRouteForTraffic(NB_Context* context, NB_OnRouteInformation* onRouteInfo, CSL_Vector* incidentPlaces, CSL_Vector* flowItems);

static const char* HEADING_NAME[] = {"north\0", "northeast\0", "east\0", "southeast\0", "south\0", "southwest\0", "west\0", "northwest\0"};

NB_Error
do_RouteInformationDestroy(NB_RouteInformation* route)
{
    data_util_state* dataState = NULL;
    if (route)
    {
        dataState = NB_ContextGetDataState(route->context);
        if (route->routeParameters)
            NB_RouteParametersDestroy(route->routeParameters);
        data_nav_reply_free(dataState, &route->dataReply);
        NB_RouteInformationDestroyDescription(route);
        NB_TrafficInformationDestroy(route->trafficInformation);
        nsl_free(route);
        return NE_OK;
    }

    return NE_INVAL;
}

static void DestroyRoutePending2(PAL_Instance *pal, void *data)
{
    NB_RouteInformation *info = (NB_RouteInformation*) data;
    do_RouteInformationDestroy(info);
}

static void DestroyRoutePending(PAL_Instance *pal, void *data)
{
    TaskId taskId;
    // Need to delete the route in Event thread, not the UI thread
    PAL_EventTaskQueueAdd(pal, DestroyRoutePending2, data, &taskId);
}

NB_DEF NB_Error
NB_RouteInformationDestroy(NB_RouteInformation* route)
{
    // KARMA-506: for long route, UI thread possibly can access old route; so make sure to serialize the delete of old route.
    PAL_UiTaskQueueAdd(NB_ContextGetPal(route->context), DestroyRoutePending, route);
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetDetourName(NB_RouteInformation* route, char* nameBuffer, uint32 nameBufferSize)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;
    data_detour_avoid* detour_avoid = NULL;

    if (!route)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(route->context);

    if (route->dataReply.vec_detour_avoid == NULL)
    {
        return NE_INVAL;
    }

    detour_avoid = CSL_VectorGetPointer(route->dataReply.vec_detour_avoid, 0);

    if (nameBuffer)
    {
        nsl_strncpy(nameBuffer, data_string_get(dataState, &detour_avoid->label), nameBufferSize);
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetRoutePolyline(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_MercatorPolyline** polyline)
{
    NB_MercatorPolyline* pThis = NULL;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    int numPoints = 0;
    uint32 i;
    int j;
    int length;
    double lon;
    double lat;

    if (!route)
    {
        return NE_INVAL;
    }
    dataState = NB_ContextGetDataState(route->context);

    for (i=startManeuver; i < endManeuver; i++)
    {
        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, i);

        numPoints += (int) data_polyline_len(dataState, &pmaneuver->polyline);
    }

    if (numPoints == 0)
    {
        // There are no points in the polygon.  Return immediately.
        return NE_NOENT;
    }

    if ((pThis = nsl_malloc(sizeof(NB_MercatorPolyline))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_MercatorPolyline));


    pThis->count = 0;
    pThis->points = nsl_malloc(sizeof(NB_MercatorPoint) * numPoints);

    if (!pThis->points)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    for (i=startManeuver; i < endManeuver; i++)
    {
        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, i);

        length = (int) data_polyline_len(dataState, &pmaneuver->polyline);

        for (j=0; j<length; j++)
        {
            data_polyline_get(dataState, &pmaneuver->polyline, j, &lat, &lon, NULL, NULL);
            NB_SpatialConvertLatLongToMercator(lat, lon, &pThis->points[pThis->count].mx, &pThis->points[pThis->count].my);
            pThis->count++;
        }
    }

    *polyline = pThis;

    return NE_OK;
}

/*! Get the route bounds

@param route NB_Route previously created route object
@param polyline On success, a NB_MercatorPolyline object; NULL otherwise.  A valid object must be destroyed with NB_MercatorPolylineDestroy()
@returns NB_Error
*/
NB_DEF NB_Error NB_RouteInformationGetRouteExtent(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max)
{
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    uint32 i;
    int j;
    int length;
    double lon;
    double lat;
    double maxLat = INVALID_LATLON;
    double maxLon = INVALID_LATLON;
    double minLat = -1*INVALID_LATLON;
    double minLon = -1*INVALID_LATLON;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);

    if (route->dataReply.hasRouteExtents)
    {
        maxLat = MAX(maxLat, route->dataReply.route_extents.topLeft.lat);
        maxLat = MAX(maxLat, route->dataReply.route_extents.bottomRight.lat);
        minLat = MIN(minLat, route->dataReply.route_extents.topLeft.lat);
        minLat = MIN(minLat, route->dataReply.route_extents.bottomRight.lat);

        maxLon = MAX(maxLon, route->dataReply.route_extents.topLeft.lon);
        maxLon = MAX(maxLon, route->dataReply.route_extents.bottomRight.lon);
        minLon = MIN(minLon, route->dataReply.route_extents.topLeft.lon);
        minLon = MIN(minLon, route->dataReply.route_extents.bottomRight.lon);
    }
    else
    {
        for (i=startManeuver; i < endManeuver; i++)
        {
            pmaneuver = NB_RouteInformationGetManeuver(route, i);

            length = (int) data_polyline_len(dataState, &pmaneuver->polyline);

            if (length > 0) //use the polyline it exists
            {
                for (j=0; j<length; j++)
                {
                    data_polyline_get(dataState, &pmaneuver->polyline, j, &lat, &lon, NULL, NULL);

                    maxLat = MAX(maxLat, lat);
                    maxLon = MAX(maxLon, lon);

                    minLat = MIN(minLat, lat);
                    minLon = MIN(minLon, lon);
                }
            }
            else //use the maneuver points if the polyline doesnt exist
            {
                maxLat = MAX(maxLat, pmaneuver->point.lat);
                maxLon = MAX(maxLon, pmaneuver->point.lon);

                minLat = MIN(minLat, pmaneuver->point.lat);
                minLon = MIN(minLon, pmaneuver->point.lon);

            }

        }
    }

    if (min)
    {
        min->latitude = minLat;
        min->longitude = minLon;
    }

    if (max)
    {
        max->latitude = maxLat;
        max->longitude = maxLon;
    }


    return NE_OK;
}

/*! Get the all route bounds

@param route NB_Route previously created route object
@param polyline On success, a NB_MercatorPolyline object; NULL otherwise.  A valid object must be destroyed with NB_MercatorPolylineDestroy()
@returns NB_Error
*/
NB_DEF NB_Error NB_RouteInformationGetRouteExtentAll(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max)
{
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    uint32 i = 0;
    int j = 0;
    int length = 0;
    double lon = INVALID_LATLON;
    double lat = INVALID_LATLON;
    double maxLat = INVALID_LATLON;
    double maxLon = INVALID_LATLON;
    double minLat = -1*INVALID_LATLON;
    double minLon = -1*INVALID_LATLON;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);

    if (route->dataReply.hasRouteExtentsAll)
    {
        maxLat = MAX(maxLat, route->dataReply.route_extents_all.topLeft.lat);
        maxLat = MAX(maxLat, route->dataReply.route_extents_all.bottomRight.lat);
        minLat = MIN(minLat, route->dataReply.route_extents_all.topLeft.lat);
        minLat = MIN(minLat, route->dataReply.route_extents_all.bottomRight.lat);

        maxLon = MAX(maxLon, route->dataReply.route_extents_all.topLeft.lon);
        maxLon = MAX(maxLon, route->dataReply.route_extents_all.bottomRight.lon);
        minLon = MIN(minLon, route->dataReply.route_extents_all.topLeft.lon);
        minLon = MIN(minLon, route->dataReply.route_extents_all.bottomRight.lon);
    }
    else
    {
        uint32 numManeuvers = NB_RouteInformationGetManeuverCount(route);

        if( endManeuver >= numManeuvers || startManeuver >= numManeuvers )
        {
            return NE_RANGE;
        }

        for (i=startManeuver; i < endManeuver; i++)
        {
            pmaneuver = NB_RouteInformationGetManeuver(route, i);

            if( pmaneuver == NULL )
            {
                continue;
            }

            length = (int) data_polyline_len(dataState, &pmaneuver->polyline);

            if (length > 0) //use the polyline it exists
            {
                for (j=0; j<length; j++)
                {
                    data_polyline_get(dataState, &pmaneuver->polyline, j, &lat, &lon, NULL, NULL);

                    maxLat = MAX(maxLat, lat);
                    maxLon = MAX(maxLon, lon);

                    minLat = MIN(minLat, lat);
                    minLon = MIN(minLon, lon);
                }
            }
            else //use the maneuver points if the polyline doesnt exist
            {
                maxLat = MAX(maxLat, pmaneuver->point.lat);
                maxLon = MAX(maxLon, pmaneuver->point.lon);

                minLat = MIN(minLat, pmaneuver->point.lat);
                minLon = MIN(minLon, pmaneuver->point.lon);
            }
        }
    }

    if (min)
    {
        min->latitude = minLat;
        min->longitude = minLon;
    }

    if (max)
    {
        max->latitude = maxLat;
        max->longitude = maxLon;
    }

    return NE_OK;
}

/// @todo (BUG 56041) Move to nbmercator.c
NB_DEF NB_Error NB_MercatorPolylineDestroy(NB_MercatorPolyline* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->points)
    {
        nsl_free(pThis->points);
    }
    nsl_free(pThis);

    return NE_OK;
}


NB_DEF NB_Error
NB_RouteInformationGetTurnImage(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, uint32 index, NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementUnits units, char* imageCodeBuffer, uint32 imageCodeBufferSize)
{
    NB_RouteDataSourceOptions dataSourceOptions = {0};

    if ( !route || !guidanceInformation )
    {
        return NE_INVAL;
    }

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    return NB_GuidanceInformationGetTurnImage(guidanceInformation, NB_RouteInformationDataSource,
        &dataSourceOptions, index, maneuverPosition, units, imageCodeBuffer, imageCodeBufferSize);
}


struct command_ttf_mapping
{
    char    command[COMMAND_ID_CHARACTERS_MAX];
    char    character;
};

struct command_ttf_mapping CommandMap[] =
{
    {"BE.",                 'q'},
    {"BE.L",                'p'},
    {"BE.R",                'r'},
    {"BR.",                 'q'},
    {"BR.L",                'p'},
    {"BR.R",                'r'},
    {"DT.",                 'V'},
    {"DT.L",                'U'},
    {"DT.R",                'W'},
    {"EC.",                 '0'},
    {"EC.L",                '0'},
    {"EC.R",                '0'},
    {"EE.",                 '5'},
    {"EE.L",                '4'},
    {"EE.R",                '6'},
    {"EN.",                 'P'},
    {"EN. (euro)",          'S'},
    {"EN.L",                'O'},
    {"EN.L (euro)",         'R'},
    {"EN.R",                'Q'},
    {"EN.R (euro)",         'T'},
    {"ER.",                 'h'},
    {"ER.L",                'g'},
    {"ER.R",                'i'},
    {"ES.",                 '2'},
    {"ES.L",                '1'},
    {"ES.R",                '3'},
    {"ET.",                 't'},
    {"ET.L",                's'},
    {"ET.R",                'u'},
    {"EX.",                 'h'},
    {"EX.L",                'g'},
    {"EX.R",                'i'},
    {"FE.",                 'v'},
    {"FX.",                 'w'},
    {"KH.",                 'y'},
    {"KH.L",                'e'},
    {"KH.R",                'f'},
    {"KP.",                 'y'},
    {"KP.L",                'e'},
    {"KP.R",                'f'},
    {"KR.L",                'e'},
    {"KR.R",                'f'},
    {"KS.",                 'y'},
    {"KS.L",                'e'},
    {"KS.R",                'f'},
    {"MR.L",                'c'},
    {"MR.R",                'd'},
    {"NC.",                 'y'},
    {"NR.",                 'y'},
    {"NR.L",                'x'},
    {"NR.R",                'z'},
    {"OR.",                 'X'},
    {"PE.",                 '7'},
    {"RE.",                 'k'},
    {"RE. (left side)",     'n'},
    {"RT.",                 'j'},
    {"RT. (left side)",     'm'},
    {"RX.n",                'l'},
    {"RX.n (left side)",    'o'},
    {"SH.",                 'y'},
    {"SH.L",                'e'},
    {"SH.R",                'f'},
    {"ST.",                 'y'},
    {"ST.L",                'e'},
    {"ST.R",                'f'},
    {"TE.",                 't'},
    {"TE.L",                's'},
    {"TE.R",                'u'},
    {"TR.",                 'y'},
    {"TR.HL",               'x'},
    {"TR.HR",               'z'},
    {"TR.L",                'x'},
    {"TR.R",                'z'},
    {"TR.SL",               'x'},
    {"TR.SR",               'z'},
    {"UT.",                 'a'},
    {"UT. (left side)",     'b'},

    // TODO: There are some ttf mapping missing. Wait for design.
    {"",                    0}
};

#define NB_COMMAND_FONT_COLOR_DEFAULT   0xffffffff

NB_Error
TransCommandCharacterLocal(char * commandID,
                           NB_CommandFont* commandFont)
{
    int i = 0;

    if (! commandID || !commandFont)
    {
        return NE_INVAL;
    }

    if (nsl_strlen(commandID) == 0)
    {
        return NE_INVAL;
    }

    while (nsl_strlen(CommandMap[i].command) > 0)
    {
        if (nsl_strncmp(CommandMap[i].command, commandID, COMMAND_ID_CHARACTERS_MAX) == 0)
        {
            commandFont->character = CommandMap[i].character;
            break;
        }
        i++;
    }

    if (!(nsl_strlen(CommandMap[i].command) > 0))
    {
        commandFont->character = ' ';
    }

    return  NE_OK;

}

struct command_ttf_mapping ImageCodeMap[] =
{
    {"en-l",                'O'},
    {"en",                  'P'},
    {"en-r",                'Q'},
    {"euro-en-l",           'R'},
    {"euro-en",             'S'},
    {"enro-en-r",           'T'},
    {"dt-l",                'U'},
    {"dt",                  'V'},
    {"dt-r",                'W'},
    {"start-up",            'X'},
    {"ut",                  'a'},
    {"lht-ut",              'b'},
    {"mr-l",                'c'},
    {"mr-r",                'd'},
    {"kp-l",                'e'},
    {"kp-r",                'f'},
    {"ex-l",                'g'},
    {"ex",                  'h'},
    {"ex-r",                'i'},
    {"rt",                  'j'},
    {"re",                  'k'},
    {"rx",                  'l'},
    {"lht-rt",              'm'},
    {"lht-re",              'n'},
    {"lht-rx",              'o'},
    {"br-l",                'p'},
    {"br",                  'q'},
    {"br-r",                'r'},
    {"et-l",                's'},
    {"et",                  't'},
    {"et-r",                'u'},
    {"enf",                 'v'},
    {"exf",                 'w'},
    {"tr-l",                'x'},
    {"nc",                  'y'},
    {"tr-r",                'z'},
    {"st-l",                '1'},
    {"st",                  '2'},
    {"st-r",                '3'},
    {"es-l",                '4'},
    {"es",                  '5'},
    {"es-r",                '6'},
    {"cf",                  '7'},
    {"ec",                  '8'},

    // TODO: There are some ttf mapping missing. Wait for design.
    {"",                    0}
};

NB_Error
TransCommandCharacterFromImageCode(NB_RouteInformation* route,
                                   NB_GuidanceInformation* guidanceInformation,
                                   uint32 index,
                                   NB_NavigateManeuverPos maneuverPosition,
                                   NB_NavigateAnnouncementUnits units,
                                   NB_CommandFont* commandFont)
{
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions ={0};
    char imageCodeBuffer[COMMAND_ID_CHARACTERS_MAX]= {0};
    int i = 0;

    if ( !route || !guidanceInformation || !commandFont)
    {
        return NE_INVAL;
    }

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    error = NB_GuidanceInformationGetTurnImage(guidanceInformation, NB_RouteInformationDataSource,
                                              &dataSourceOptions, index, maneuverPosition, units, imageCodeBuffer, COMMAND_ID_CHARACTERS_MAX);

    if (error != NE_OK)
    {
        commandFont->character = ' ';
        return error;
    }

    while (nsl_strlen(ImageCodeMap[i].command) > 0)
    {
        if (nsl_strncmp(ImageCodeMap[i].command, imageCodeBuffer, COMMAND_ID_CHARACTERS_MAX) == 0)
        {
            commandFont->character = ImageCodeMap[i].character;
            break;
        }
        i++;
    }

    if (!(nsl_strlen(CommandMap[i].command) > 0))
    {
        commandFont->character = ' ';
    }

    return  NE_OK;
}


NB_DEF NB_Error
NB_RouteInformationGetTurnCommandCharacter(NB_RouteInformation* route,
                                           NB_GuidanceInformation* guidanceInformation,
                                           uint32 index,
                                           NB_NavigateManeuverPos maneuverPosition,
                                           NB_NavigateAnnouncementUnits units,
                                           NB_CommandFont* commandFont)
{
    char commandChars[COMMAND_CHARACTERS_MAX] = {0};
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    data_util_state* dataState = 0;
    uint32 maneuverCount = 0;
    data_nav_maneuver* maneuver = NULL;

    if ( !route || !guidanceInformation || !commandFont)
    {
        return NE_INVAL;
    }

    if (index == NAV_MANEUVER_START)
    {
        // Start-up
        commandFont->character = 'X';
        commandFont->color = NB_COMMAND_FONT_COLOR_DEFAULT;

        return NE_OK;
    }

    dataState = NB_ContextGetDataState(route->context);
    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    // skip the NC. maneuver
    maneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, index);
    while (maneuver && index < maneuverCount && is_informational(maneuver->command))
    {
        index++;
        maneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, index);
    }

    error = NB_GuidanceInformationGetTurnCharacter(guidanceInformation, NB_RouteInformationDataSource,
                                              &dataSourceOptions, index, maneuverPosition, units, commandChars, COMMAND_CHARACTERS_MAX);

    if (error == NE_OK)
    {
        commandFont->character = commandChars[0];
    }
    else
    {
        commandFont->character = ' ';
    }


    //TODO: Wait for color define;
    commandFont->color = NB_COMMAND_FONT_COLOR_DEFAULT;

    return error;
}

NB_DEF NB_Error
NB_RouteInformationGetTurnAnnouncement(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, uint32 index, NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    return NB_RouteInformationGetTurnAnnouncementExt(route, guidanceInformation, index, maneuverPosition, source, type, units, message, INVALID_LATITUDE, INVALID_LONGITUDE);
}

NB_DEF NB_Error
NB_RouteInformationGetTurnAnnouncementExt(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, uint32 index, NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message, double projLatitude, double projLongtitude)
{
    NB_Error err = NE_OK;
    CSL_Cache* voiceCache = NULL;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation currentPosition = {0};
    data_util_state* dataState = NULL;
    data_nav_maneuver* maneuver = 0;

    if ( !route || !guidanceInformation )
    {
        return NE_INVAL;
    }
    dataState = NB_ContextGetDataState(route->context);
    maneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, index == NAV_MANEUVER_INITIAL ? 0 : index);

    voiceCache = NB_ContextGetVoiceCache(route->context);

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = maneuverPosition;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.isLookahead = source == NB_NAS_Lookahead? TRUE:FALSE;

    if (projLatitude != INVALID_LATITUDE && projLongtitude != INVALID_LONGITUDE)
    {
        currentPosition.projLat = projLatitude;
        currentPosition.projLon = projLongtitude;
        if (maneuver)
        {
            currentPosition.maneuverDistanceRemaining = maneuver->distance;
        }
        dataSourceOptions.currentRoutePosition = &currentPosition;
        dataSourceOptions.isPositionOnlyForLocation = TRUE;
    }
    else
    {
        dataSourceOptions.currentRoutePosition = NULL;
    }

    dataSourceOptions.guidanceState = NULL;
    err = NB_GuidanceProcessorStateCreate(route->context, &dataSourceOptions.guidanceState);

    if (err != NE_OK)
    {
        return err;
    }

    err =  NB_GuidanceInformationGetTurnAnnouncement(guidanceInformation, NB_RouteInformationDataSource,
        &dataSourceOptions, index, maneuverPosition, source, type, units, message, NULL);

    if (err == NE_OK)
    {
        if (voiceCache)
        {
            /*@todo (BUG 56042) I'm not sure we want to actually save the voice cache here,
              but rather when we actually play the message. Need to move to the played callback
            */
            CSL_CacheSaveAccessed(voiceCache, -1, NULL, TRUE);
        }
    }
    NB_GuidanceProcessorStateDestroy(dataSourceOptions.guidanceState);
    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetNextSignificantManeuverIndex(NB_RouteInformation* route, uint32 afterIndex, uint32* significantIndex)
{
    uint32 nman_next = NAV_MANEUVER_NONE;
    uint32 maneuverCount = 0;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!route)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if ( afterIndex >= maneuverCount)
    {
        return NE_INVAL;
    }

    for (nman_next = (afterIndex - 1 == NAV_MANEUVER_START || afterIndex - 1 == NAV_MANEUVER_INITIAL) ? 0 : afterIndex;
         nman_next < maneuverCount; nman_next++)
    {
        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, nman_next);

        if (!is_informational(data_string_get(dataState, &pmaneuver->command)))
        {
            (*significantIndex) = nman_next;
            break;
        }
    }

    if (nman_next == maneuverCount)
    {
        return NE_NOENT;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationEnumerateManeuverText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData)
{
    NB_Error err = NE_OK;
    instructset* steps = NULL;
    data_util_state* dataState = NULL;
    NB_RouteDataSourceOptions dataSourceOptions = {0};

    if (!route || !guidanceInformation)
        return NE_INVAL;

    // @todo (BUG 56043) - find a better place to do this
    dataState = NB_ContextGetDataState(route->context);
    err = NB_GuidanceInformationGetSteps(guidanceInformation, &steps);

    if (err != NE_OK)
    {
        return err;
    }

    convert_reserved_phrases(steps, dataState, &route->dataReply);

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    err = NB_GuidanceInformationEnumerateManeuverText(guidanceInformation,
        NB_RouteInformationDataSource, &dataSourceOptions, textType, units,
        index, callback, userData);
    return err;
}

NB_DEF NB_Error
NB_RouteInformationEnumerateStackedTurnText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData)
{
    uint32 maneuverCount = 0;
    NB_RouteDataSourceOptions dataSourceOptions = {0};

    if (!route || !guidanceInformation)
        return NE_INVAL;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if (maneuverCount == 0)
    {
        return NE_NOENT;
    }

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    return NB_GuidanceInformationEnumerateStackedTurnText(guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions, textType, units, index, callback, userData);
}

NB_DEF NB_Error
NB_RouteInformationEnumerateArrivalText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, NB_GuidanceInformationTextCallback callback, void* userData)
{
    uint32 maneuverCount = 0;
    NB_RouteDataSourceOptions dataSourceOptions = {0};

    if (!route || !guidanceInformation )
        return NE_INVAL;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if (maneuverCount == 0)
    {
        return NE_NOENT;
    }

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = FALSE;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;

    return NB_GuidanceInformationEnumerateArrivalText(guidanceInformation,
        NB_RouteInformationDataSource, &dataSourceOptions, textType, units,
        maneuverCount - 1, callback, userData);
}

NB_DEF uint32
NB_RouteInformationGetNextSignificantManeuver(NB_RouteInformation* route, uint32 index)
{
    uint32 nman_next = NAV_MANEUVER_NONE;
    uint32 maneuverCount = 0;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!route)
    {
        return NAV_MANEUVER_NONE;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if ( index + 1 >= maneuverCount)
    {
        return NAV_MANEUVER_NONE;
    }

    for (nman_next = (index - 1 == NAV_MANEUVER_START || index - 1 == NAV_MANEUVER_INITIAL) ? 0 : index + 1;
         nman_next < maneuverCount; nman_next++)
    {
        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, nman_next);

        if (!is_informational(data_string_get(dataState, &pmaneuver->command)))
        {
            return nman_next;
        }
    }

    return NAV_MANEUVER_NONE;
}

NB_DEF uint32
NB_RouteInformationGetPreviousSignificantManeuver(NB_RouteInformation* route, uint32 index)
{
    uint32 nman_prev = index;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!route)
    {
        return NAV_MANEUVER_NONE;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    do
    {
        if (nman_prev == 0)
        {
            nman_prev = NAV_MANEUVER_NONE;
        }
        else
        {
            nman_prev = nman_prev - 1;
        }

        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, nman_prev);
    }
    while (nman_prev != NAV_MANEUVER_NONE && is_informational(data_string_get(dataState, &pmaneuver->command)));

    return nman_prev;
}

NB_DEF nb_boolean
NB_RouteInformationIsManeuverSignificant(NB_RouteInformation* route, uint32 index)
{
    uint32 maneuverCount = 0;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!route)
        return FALSE;

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if ( index >= maneuverCount)
    {
        return NE_INVAL;
    }

    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (pmaneuver)
    {
        if (!is_informational(data_string_get(dataState, &pmaneuver->command)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

NB_DEF nb_boolean
NB_RouteInformationIsManeuverSignificantForTurnLists(NB_RouteInformation* route, uint32 index, nb_boolean isNextManeuver)
{
    uint32 maneuverCount = 0;
    uint32 nextManeuverIndex = 0;
    data_nav_maneuver* pmaneuver = NULL;
    data_nav_maneuver* pnextManeuver = NULL;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!route)
        return FALSE;

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if (index >= maneuverCount)
    {
        return FALSE;
    }

    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (pmaneuver)
    {
        boolean equal = data_string_equal(dataState,&pmaneuver->current_roadinfo.primary,&pmaneuver->turn_roadinfo.primary);
        if (is_informational(data_string_get(dataState, &pmaneuver->command)))
        {
            return FALSE;
        }

        if (is_stay(data_string_get(dataState, &pmaneuver->command)) ||
            is_stay_highway(data_string_get(dataState, &pmaneuver->command)))
        {
            if (!isNextManeuver && equal)
            {
                return FALSE;
            }
        }

        if (is_merge(data_string_get(dataState, &pmaneuver->command)))
        {
            if (!isNextManeuver  && equal)
            {
                return FALSE;
            }
        }

        if (is_keep(data_string_get(dataState, &pmaneuver->command)))
        {
            if (compare_latlon(route->start.location.latitude, route->start.location.longitude,
                               route->destination.location.latitude, route->destination.location.longitude,
                               0.0001)  && equal)
            {
                return FALSE;
            }
        }

        if (is_tunnel_entry(data_string_get(dataState, &pmaneuver->command)))
        {
            if (pnextManeuver &&
                !is_tunnel_entry(data_string_get(dataState, &pnextManeuver->command))  && equal)
            {
                return FALSE;
            }
        }

        if (is_roundaboutexit(data_string_get(dataState, &pmaneuver->command)))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

NB_DEF uint32
NB_RouteInformationGetNextManeuver(NB_RouteInformation* route, uint32 index)
{
    uint32 nman_next = NAV_MANEUVER_NONE;
    uint32 maneuverCount = 0;

    data_nav_reply* reply = NULL;

    if (!route)
        return NE_INVAL;

    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    if (index + 1 >= maneuverCount)
    {
        return NAV_MANEUVER_NONE;
    }

    nman_next = (index == NAV_MANEUVER_START || index == NAV_MANEUVER_INITIAL) ? 0 : index+1;

    if (nman_next == maneuverCount)
    {
        return NAV_MANEUVER_NONE;
    }

    return nman_next;
}

NB_DEF uint32
NB_RouteInformationGetPreviousManeuver(NB_RouteInformation* route, uint32 index)
{
    data_nav_reply* reply = NULL;

    if (!route)
        return NE_INVAL;

    reply = &route->dataReply;

    if (index >= NB_RouteInformationGetManeuverCount(route))
    {
        return NAV_MANEUVER_NONE;
    }

    if (index == 0)
    {
        return NAV_MANEUVER_NONE;
    }

    return index - 1;
}

NB_DEF NB_Error
NB_RouteInformationGetSummaryInformation(NB_RouteInformation* route, uint32* time, uint32* delay, double* distance)
{
    if (!route)
        return NE_INVAL;

    if (distance)
    {
        (*distance) = route->routeLength;
    }

    if (time)
    {
        (*time) = route->routeTime;
    }

    if (delay)
    {
        (*delay) =  route->routeDelay;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTurnInformation(NB_RouteInformation* route, uint32 index, NB_LatitudeLongitude* turnPoint, char* primaryNameBuffer, uint32 primaryNameBufferSize, char* secondaryNameBuffer, uint32 secondaryNameBufferSize, char* currentNameBuffer, uint32 currentNameBufferSize, double* distance, nb_boolean collapsed)
{
    return NB_RouteInformationGetTurnInformation2(route, index, turnPoint, primaryNameBuffer, primaryNameBufferSize,
                                                  secondaryNameBuffer, secondaryNameBufferSize, currentNameBuffer,
                                                  currentNameBufferSize, NULL, 0, distance, NULL, NULL, NULL, 0, collapsed);
}

NB_DEF NB_Error
NB_RouteInformationGetTurnInformation2(NB_RouteInformation* route, uint32 index, NB_LatitudeLongitude* turnPoint, char* primaryNameBuffer, uint32 primaryNameBufferSize,
    char* secondaryNameBuffer, uint32 secondaryNameBufferSize, char* currentNameBuffer, uint32 currentNameBufferSize, char* secondaryCurrentNameBuffer, uint32 secondaryCurrentNameBufferSize,
    double* distance, double* speed, nb_boolean* stackAdvise, char* commandBuffer, uint32 commandBufferSize, nb_boolean collapsed)
{
    NB_Error err = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    data_nav_maneuver* pmaneuver = NULL;
    data_nav_maneuver* pmaneuver_name = NULL;
    uint32 nman_name = 0;
    uint32 nman_dist = 0;
    const char*    data = NULL;
    nb_size    datalen  = 0;
    boolean    freedata = FALSE;
    data_nav_reply* reply = NULL;
    data_util_state* dataState = NULL;
    NB_Place destination = {{0}};

    if (!route)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;
    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    SetNIMLocationFromLocation(&destination.location, dataState, &reply->destination_location);

    if (pmaneuver)
    {
        if (NB_RouteParametersGetCommandType(route->routeParameters, data_string_get(dataState, &pmaneuver->command)) == maneuver_type_dest)
        {
            if (primaryNameBuffer != NULL && secondaryNameBuffer != NULL)
            {
                FormatPlace2(NULL, &destination, primaryNameBuffer, primaryNameBufferSize, secondaryNameBuffer, secondaryNameBufferSize);
            }
            else if (primaryNameBuffer != NULL)
            {
                FormatPlace(NULL, &destination, primaryNameBuffer, primaryNameBufferSize);
            }

            if (currentNameBuffer && currentNameBufferSize > 0)
            {
                currentNameBuffer[0] = '\0';
            }
        }
        else
        {
            nman_name = index;
            if (/*app_GetDeviceFlag(papp, "want-towards-roadinfo") == TRUE*/ 0)
            {
                pmaneuver_name = data_nav_reply_get_nav_maneuver(dataState, reply, nman_name++);

                if (pmaneuver_name != NULL && nsl_strlen(data_string_get(dataState, &pmaneuver_name->turn_roadinfo.primary)) > 0)
                {
                    data_string_get_copy(dataState, &pmaneuver_name->turn_roadinfo.primary, primaryNameBuffer, primaryNameBufferSize);
                    data_string_get_copy(dataState, &pmaneuver_name->turn_roadinfo.secondary, secondaryNameBuffer, secondaryNameBufferSize);
                }
            }
            else
            {
                do {

                    pmaneuver_name = data_nav_reply_get_nav_maneuver(dataState, reply, nman_name++);

                    if (pmaneuver_name != NULL && nsl_strlen(data_string_get(dataState, &pmaneuver_name->turn_roadinfo.primary)) > 0)
                    {
                        data_string_get_copy(dataState, &pmaneuver_name->turn_roadinfo.primary, primaryNameBuffer, primaryNameBufferSize);
                        data_string_get_copy(dataState, &pmaneuver_name->turn_roadinfo.secondary, secondaryNameBuffer, secondaryNameBufferSize);
                    }

                } while (pmaneuver_name == NULL);
            }
        }
    }
    else
    {
        if (primaryNameBuffer)
        {
            primaryNameBuffer[0] = 0;
        }

        if (secondaryNameBufferSize)
        {
            secondaryNameBuffer[0] = 0;
        }
    }

    nman_dist = index;

    if (collapsed)
    {
        if (nman_dist == NAV_MANEUVER_START || nman_dist == NAV_MANEUVER_INITIAL)
        {
            nman_dist = 0;
        }
        while (nman_dist != NAV_MANEUVER_NONE && nman_dist > 0 && !NB_RouteInformationIsManeuverSignificantForTurnLists(route, nman_dist - 1, FALSE))
            nman_dist--;
    }

    dataSourceOptions.route = route;
    dataSourceOptions.collapse = collapsed;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.trafficInfo = NULL;
    dataSourceOptions.currentRoutePosition = NULL;
    dataSourceOptions.isManeuverList = TRUE;

    if (distance != NULL && NB_RouteInformationDataSource(collapsed ? dist : bdist, nman_dist, &dataSourceOptions, &data, &datalen, &freedata) && datalen == sizeof(double))
    {
        nsl_memcpy(distance, data, sizeof(*distance));
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
        data = NULL;
    }

    if (currentNameBuffer != NULL && NB_RouteInformationDataSource(oname, nman_dist, &dataSourceOptions, &data, &datalen, &freedata))
        nsl_strlcpy(currentNameBuffer, data, currentNameBufferSize);

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
        data = NULL;
    }

    if (turnPoint)
    {
        if (pmaneuver)
        {
            turnPoint->latitude = pmaneuver->point.lat;
            turnPoint->longitude = pmaneuver->point.lon;
        }
    }

    if (currentNameBuffer != NULL)
    {
        if (pmaneuver)
        {
            nsl_strlcpy(currentNameBuffer, data_string_get(dataState, &pmaneuver->current_roadinfo.primary), currentNameBufferSize);
        }
    }
    if (secondaryCurrentNameBuffer != NULL)
    {
        if (pmaneuver)
        {
            nsl_strlcpy(secondaryCurrentNameBuffer, data_string_get(dataState, &pmaneuver->current_roadinfo.secondary), secondaryCurrentNameBufferSize);
        }
    }

    if (stackAdvise && pmaneuver)
    {
        *stackAdvise = pmaneuver->stack_advise;
    }

    if (speed && pmaneuver)
    {
        *speed = pmaneuver->speed;
    }

    if (commandBuffer != NULL && pmaneuver)
    {
        nsl_strlcpy(commandBuffer, data_string_get(dataState, &pmaneuver->command), commandBufferSize);
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetCurrentRoadNames(NB_RouteInformation* route, uint32 index,
    char* primaryNameBuffer, uint32 primaryNameBufferSize,
    char* secondaryNameBuffer, uint32 secondaryNameBufferSize)
{
    NB_Error err = NE_OK;
    data_nav_maneuver* pmaneuver = NULL;
    data_nav_reply* reply = NULL;
    data_util_state* dataState = NULL;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;
    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (!pmaneuver)
    {
        return NE_NOENT;
    }

    if (primaryNameBuffer != NULL)
    {
        data_string_get_copy(dataState, &pmaneuver->current_roadinfo.primary, primaryNameBuffer, primaryNameBufferSize);
    }

    if (secondaryNameBuffer != NULL)
    {
        data_string_get_copy(dataState, &pmaneuver->current_roadinfo.secondary, secondaryNameBuffer, secondaryNameBufferSize);
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetManeuverCrossStreetsCount(NB_RouteInformation* route, uint32 index, uint32* crossStreetCount)
{
    NB_Error err = NE_OK;
    data_nav_maneuver* pmaneuver = NULL;

    if (!route || !crossStreetCount)
    {
        return NE_INVAL;
    }

    pmaneuver = NB_RouteInformationGetManeuver(route, index);

    if (!pmaneuver)
    {
        return NE_NOENT;
    }

    *crossStreetCount = CSL_VectorGetLength(pmaneuver->vec_cross_streets);

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetManeuverCrossStreets(NB_RouteInformation* route, uint32 index, uint32 crossStreetIndex, char* crossStreets)
{
    NB_Error err = NE_OK;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    data_cross_street* crossStreet;
    int numCrossStreets=0;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    pmaneuver = NB_RouteInformationGetManeuver(route, index);

    if (!pmaneuver)
    {
        return NE_NOENT;
    }

    numCrossStreets = CSL_VectorGetLength(pmaneuver->vec_cross_streets);

    if ( (int) crossStreetIndex >= numCrossStreets )
    {
        return NE_NOENT;
    }

    crossStreet = (data_cross_street*) CSL_VectorGetPointer(pmaneuver->vec_cross_streets, crossStreetIndex);

    if (crossStreet)
    {
        nsl_strcpy(crossStreets, data_string_get(dataState, &crossStreet->name));
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficFlags(NB_RouteInformation* route, NB_TrafficType* flags)
{
    if (!route)
    {
        return NE_INVAL;
    }

    return NB_RouteParametersGetTrafficFlags(route->routeParameters, flags);
}

NB_DEF NB_Error
NB_RouteInformationGetFirstMajorRoadName(NB_RouteInformation* route,
    char* nameBuffer, uint32 nameBufferSize)
{
    data_util_state* dataState = NULL;
    data_nav_maneuver* maneuver = NULL;
    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);

    if (nameBuffer != NULL)
    {
        if (route->dataReply.first_major_road == NULL)
        {
            nameBuffer[0] = '\0';
        }
        else
        {
            data_string_get_copy(dataState, &route->dataReply.first_major_road->primary, nameBuffer, nameBufferSize);
        }

        if (nsl_strlen(nameBuffer) == 0)
        {
            maneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, 0);
            if (maneuver)
            {
                data_string_get_copy(dataState, &maneuver->current_roadinfo.primary, nameBuffer, nameBufferSize);
            }
        }
    }
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetOrigin(NB_RouteInformation* route, NB_Place* origin)
{
    data_util_state* dataState = NULL;

    if (!route || !origin)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);

    return SetNIMLocationFromLocation(&origin->location, dataState, &route->dataReply.origin_location);
}

NB_DEF NB_Error
NB_RouteInformationGetOriginGpsLocation(NB_RouteInformation* route, NB_GpsLocation* location)
{
    if (!route || !location)
    {
        return NE_INVAL;
    }

    *location = route->originFix;

    return NE_OK;
}


NB_DEF NB_Error
NB_RouteInformationGetDestination(NB_RouteInformation* route, NB_Place* destination)
{
    data_util_state* dataState = NULL;

    if (!route || !destination)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);

    return SetNIMLocationFromLocation(&destination->location, dataState, &route->dataReply.destination_location);
}

NB_DEF NB_Error
NB_RouteInformationGetTransportMode(NB_RouteInformation* route, NB_NavigateTransportMode* mode)
{
    if (!route || !mode)
    {
        return NE_INVAL;
    }

    *mode = route->transportMode;

    return NE_OK;
}

NB_DEF nb_boolean
NB_RouteInformationIsRouteComplete(NB_RouteInformation* route)
{
    int numMan = 0;
    int index = 0;

    if (!route)
        return FALSE;

    numMan = NB_RouteInformationGetManeuverCount(route);

    for ( index = numMan - 1; index >= 0; index-- )
    {
        if ( !NB_RouteInformationIsManeuverComplete(route, index) )
            return FALSE;
    }

    return TRUE;

}

NB_DEF nb_boolean
NB_RouteInformationIsManeuverComplete(NB_RouteInformation* route, uint32 index)
{
    data_nav_maneuver* maneuver = NULL;

    if (!route)
    {
        return FALSE;
    }

    maneuver = NB_RouteInformationGetManeuver(route, index);

    if (!maneuver)
    {
        return FALSE;
    }

    if ( nsl_strcmp(maneuver->command, "DT.U") == 0 )
    {
        return FALSE;
    }

    return TRUE;
}

NB_DEF NB_Error
NB_RouteInformationGetRouteId(NB_RouteInformation* route, NB_RouteId** routeId)
{
    if (!route || !routeId)
    {
        return NE_INVAL;
    }

    (*routeId) = &route->routeID;

    return NE_OK;
}

NB_DEF nb_boolean
NB_RouteInformationIsManeuverWithinRouteWindow(NB_RouteInformation* route, uint32 index)
{
    data_nav_maneuver* maneuver = NULL;
    data_util_state* dataState = NULL;

    if (!route)
    {
        return FALSE;
    }

    dataState = NB_ContextGetDataState(route->context);
    maneuver = NB_RouteInformationGetManeuver(route, index);

    if (!maneuver || !dataState)
    {
        return FALSE;
    }

    return (nb_boolean)data_nav_maneuver_polyline_complete(dataState, maneuver);
}

NB_DEF NB_Error
NB_RouteInformationGetManeuverTurnMapCenterPoint(NB_RouteInformation* route, uint32 index, NB_LatitudeLongitude* centerPoint, double* heading)
{
    data_nav_maneuver* maneuver = NULL;
    data_util_state* dataState = NULL;
    NB_RoutePrivateConfiguration* privateConfig = NULL;

    NB_PolylineForwardLengthIterationData nextLengthData = {0};
    NB_Vector point = {0};

    double lastPointLatitude = 0;
    double lastPointLongitude = 0;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    maneuver = NB_RouteInformationGetManeuver(route, index);
    privateConfig = NB_RouteInformationGetPrivateConfiguration(route);

    if (!maneuver || !dataState || !privateConfig
        || privateConfig->prefs.turnMapHeadingDistanceFromManeuver <= 0.0
        || !data_nav_maneuver_polyline_complete(dataState, maneuver))
    {
        return NE_INVAL;
    }

    // iterate backward through the points in this maneuver until length is reached
    nextLengthData.lastPoint.x = INVALID_LATLON;
    nextLengthData.lastPoint.y = INVALID_LATLON;
    nextLengthData.lastPoint.z = 0;
    nextLengthData.length = privateConfig->prefs.turnMapHeadingDistanceFromManeuver;
    nextLengthData.index = maneuver->polyline.numsegments - 1;
    nextLengthData.polyline = &maneuver->polyline;
    nextLengthData.dataState = dataState;

    while (NB_PolylineReverseLengthIteration(&nextLengthData, &point));

    /* Determine the rotation */
    NB_SpatialConvertMercatorToLatLong(point.x, point.y, &lastPointLatitude, &lastPointLongitude);

    if (heading)
    {
        NB_SpatialGetLineOfSightDistance(lastPointLatitude, lastPointLongitude, maneuver->point.lat, maneuver->point.lon, heading);
    }

    if (centerPoint)
    {
        centerPoint->latitude = maneuver->point.lat;
        centerPoint->longitude = maneuver->point.lon;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTurnCountryCode(NB_RouteInformation* route, uint32 index, const char** countryCode)
{
    data_util_state* dataState = NULL;
    data_nav_maneuver* maneuver = NULL;

    const char* code = 0;

    if (!route || !countryCode)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    maneuver = NB_RouteInformationGetManeuver(route, index);

    if (!maneuver || !dataState)
    {
        return NE_INVAL;
    }

    code = data_string_get(dataState, &maneuver->turn_roadinfo.country_info.code);

    // if country code found from turn road info, return it
    if (code && *code)
    {
        *countryCode = code;
    }
    // otherwise return the country code of the route start location
    else
    {
        *countryCode = route->start.location.country;
    }

    return NE_OK;
}

uint32
NB_RouteInformationGetManeuverRealisticSignCount(NB_RouteInformation* route, uint32 maneuverIndex)
{
    data_nav_maneuver* maneuver = 0;

    if (!route)
    {
        return 0;
    }

    maneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
    if (!maneuver)
    {
        return 0;
    }

    return CSL_VectorGetLength(maneuver->vec_realistic_signs);
}

NB_Error
NB_RouteInformationGetManeuverRealisticSign(NB_RouteInformation * route, uint32 maneuverIndex, uint32 signIndex, NB_RouteRealisticSign* realisticSign)
{
    data_nav_maneuver* maneuver = 0;
    uint32 signCount = 0;
    data_util_state* dataState = 0;
    data_realistic_sign* realisticSignData = 0;

    if (!route || !realisticSign)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    nsl_memset(realisticSign, 0, sizeof(NB_RouteRealisticSign));

    maneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
    if (!maneuver)
    {
        return NE_NOENT;
    }

    signCount = CSL_VectorGetLength(maneuver->vec_realistic_signs);
    if (signIndex >= signCount)
    {
        return NE_NOENT;
    }

    realisticSignData = (data_realistic_sign*)CSL_VectorGetPointer(maneuver->vec_realistic_signs, signIndex);
    if (!realisticSignData)
    {
        return NE_NOENT;
    }

    realisticSign->signId =         data_string_get(dataState, &realisticSignData->id);
    realisticSign->signPortraitId = data_string_get(dataState, &realisticSignData->portrait_id);
    realisticSign->signFileType =   data_string_get(dataState, &realisticSignData->type);
    realisticSign->datasetId =      data_string_get(dataState, &realisticSignData->dataset_id);
    realisticSign->version =        data_string_get(dataState, &realisticSignData->version);

    realisticSign->maneuver = maneuverIndex;
    realisticSign->positionMeters = realisticSignData->position;

    return NE_OK;
}

uint32
NB_RouteInformationGetContentRegionCount(NB_RouteInformation* route)
{
    if (!route)
    {
        return 0;
    }

    return CSL_VectorGetLength(route->dataReply.extended_content.vec_extended_content_region);
}

NB_Error
NB_RouteInformationGetContentRegion(NB_RouteInformation * route, uint32 regionIndex, NB_RouteContentRegion* contentRegion)
{
    uint32 regionCount = 0;
    data_util_state* dataState = 0;
    data_extended_content_region* contentRegionData = 0;

    if (!route || !contentRegion)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    nsl_memset(contentRegion, 0, sizeof(NB_RouteContentRegion));

    regionCount = CSL_VectorGetLength(route->dataReply.extended_content.vec_extended_content_region);
    if (regionIndex >= regionCount)
    {
        return NE_NOENT;
    }

    contentRegionData = (data_extended_content_region*)CSL_VectorGetPointer(route->dataReply.extended_content.vec_extended_content_region, regionIndex);
    if (!contentRegionData)
    {
        return NE_NOENT;
    }

    if (data_string_compare_cstr(&contentRegionData->type, "junction-model"))
    {
        contentRegion->type = NB_RCRT_JunctionModel;
    }
    else if (data_string_compare_cstr(&contentRegionData->type, "city-model"))
    {
        contentRegion->type = NB_RCRT_CityModel;
    }
    else
    {
        return NE_NOENT;
    }

    contentRegion->regionId =               data_string_get(dataState, &contentRegionData->id);
    contentRegion->datasetId =              data_string_get(dataState, &contentRegionData->dataset_id);
    contentRegion->version =                data_string_get(dataState, &contentRegionData->version);
    contentRegion->startManeuver =          contentRegionData->start_maneuver_index;
    contentRegion->startManeuverOffset =    contentRegionData->start_maneuver_offset;
    contentRegion->endManeuver =            contentRegionData->end_maneuver_index;
    contentRegion->endManeuverOffset =      contentRegionData->end_maneuver_offset;

    if (contentRegionData->vec_content_path)
    {
        contentRegion->splinePathCount = CSL_VectorGetLength(contentRegionData->vec_content_path);
    }

    if (contentRegionData->projection != NULL)
    {
        if (data_string_compare_cstr(&contentRegionData->projection, "mercator"))
        {
            contentRegion->projection = NB_ECPT_Mercator;
        }
        else if (data_string_compare_cstr(&contentRegionData->projection, "transverse-mercator"))
        {
            contentRegion->projection = NB_ECPT_TransverseMercator;
        }
        else if (data_string_compare_cstr(&contentRegionData->projection, "universal-transverse-mercator"))
        {
            contentRegion->projection = NB_ECPT_UniversalTransverseMercator;
        }
    }

    if (data_string_compare_cstr(&contentRegionData->projection_parameters.datum, "WGS84"))
    {
        contentRegion->datum = NB_ECPD_WGS84;
    }

    contentRegion->originLatitude =         contentRegionData->projection_parameters.origin_latitude;
    contentRegion->originLongitude =        contentRegionData->projection_parameters.origin_longitude;
    contentRegion->scaleFactor =            contentRegionData->projection_parameters.scale_factor;
    contentRegion->falseEasting =           contentRegionData->projection_parameters.false_easting;
    contentRegion->falseNorthing =          contentRegionData->projection_parameters.false_northing;
    contentRegion->zOffset =                contentRegionData->projection_parameters.z_offset;

    ComputeContentRegionMeasurements(route, contentRegion);

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_RouteIdClone(const NB_RouteId* originalRouteId, NB_RouteId** newRouteId)
{
    if (! originalRouteId)
    {
        return NE_INVAL;
    }

    // New route-ID
    *newRouteId = nsl_malloc(sizeof(NB_RouteId));
    if (! (*newRouteId))
    {
        return NE_NOMEM;
    }
    nsl_memset(*newRouteId, 0, sizeof(NB_RouteId));

    // Clone/copy data
    (*newRouteId)->size = originalRouteId->size;
    if (originalRouteId->size > 0)
    {
        (*newRouteId)->data = nsl_malloc(originalRouteId->size);
        if (! ((*newRouteId)->data))
        {
            nsl_free(*newRouteId);
            *newRouteId = NULL;
            return NE_NOMEM;
        }
        nsl_memcpy((*newRouteId)->data, originalRouteId->data, originalRouteId->size);
    }

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_RouteIdDestroy(NB_RouteId** routeId)
{
    // Free data if it is set. Ignore if it is already freed.
    if (*routeId)
    {
        if ((*routeId)->data)
        {
            nsl_free((*routeId)->data);
        }

        nsl_free(*routeId);
        *routeId = NULL;
    }

    return NE_OK;
}

static void
ComputeContentRegionMeasurements(NB_RouteInformation* route, NB_RouteContentRegion* region)
{
    data_nav_maneuver* maneuver = 0;

    double regionDistanceFromStart = 0.0;
    double regionLength = 0.0;

    NB_LatitudeLongitude regionEndPoint = { INVALID_LATLON, INVALID_LATLON };

    uint32 maneuverIndex = 0;

    // Go through maneuvers from start of route to end maneuver for given content region
    for (maneuverIndex = 0; maneuverIndex <= region->endManeuver; maneuverIndex++)
    {
        maneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
        if (maneuver == NULL)
        {
            return;
        }

        // Before region start maneuver, sum the distance from start of route
        if (maneuverIndex < region->startManeuver)
        {
            regionDistanceFromStart += maneuver->distance;
        }

        // At region start maneuver, add region start maneuver offset to finish the distance
        // from start of route, and start summing the region length
        else if (maneuverIndex == region->startManeuver)
        {
            regionDistanceFromStart += region->startManeuverOffset;
            regionLength = maneuver->distance - region->startManeuverOffset;
        }

        // Between region start and end maneuvers, sum region length
        else if (maneuverIndex < region->endManeuver)
        {
            regionLength += maneuver->distance;
        }

        // At region end maneuver, finish region length, and compute end point
        if (maneuverIndex >= region->endManeuver)
        {
            uint32 segmentCount = 0;
            uint32 segmentIndex = 0;

            double previousSegmentsLength = 0.0;

            // Finish region length
            regionLength += region->endManeuverOffset;

            // Determine region end point
            segmentCount = data_polyline_len(NB_ContextGetDataState(route->context), &maneuver->polyline);
            for (segmentIndex = 0; segmentIndex < segmentCount; segmentIndex++)
            {
                double segmentLatitude = 0.0;
                double segmentLongitude = 0.0;
                double segmentLength = 0.0;

                data_polyline_get(NB_ContextGetDataState(route->context), &maneuver->polyline, segmentIndex,
                    &segmentLatitude, &segmentLongitude, &segmentLength, NULL);

                if (previousSegmentsLength + segmentLength >= region->endManeuverOffset)
                {
                    double nextSegmentLatitude = 0.0;
                    double nextSegmentLongitude = 0.0;
                    double stepFactor = 0.0;

                    if (segmentIndex == segmentCount - 1)
                    {
                        nextSegmentLatitude = maneuver->point.lat;
                        nextSegmentLongitude = maneuver->point.lon;
                    }
                    else
                    {
                        data_polyline_get(NB_ContextGetDataState(route->context), &maneuver->polyline, segmentIndex + 1,
                            &nextSegmentLatitude, &nextSegmentLongitude, NULL, NULL);
                    }

                    // Interpolate end point
                    stepFactor = (region->endManeuverOffset - previousSegmentsLength) / segmentLength;
                    regionEndPoint.latitude = segmentLatitude + (nextSegmentLatitude - segmentLatitude) * stepFactor;
                    regionEndPoint.longitude = segmentLongitude + (nextSegmentLongitude - segmentLongitude) * stepFactor;

                    break;
                }

                previousSegmentsLength += segmentLength;
            }

            // If end point was not set, it's because server returned a region endManeuverOffset that went beyond
            // the length of the end maneuver.  Use the maneuver's end point in this case.
            if (regionEndPoint.latitude == INVALID_LATLON)
            {
                regionEndPoint.latitude = maneuver->point.lat;
                regionEndPoint.longitude = maneuver->point.lon;
            }

            // Exit for-loop. We're done.
            break;
        }
    }

    region->distanceFromRouteStart = regionDistanceFromStart;
    region->endPoint = regionEndPoint;

    // If the start and end maneuver match then the region length is siply the offset difference.
    if (region->startManeuver == region->endManeuver)
    {
        region->length = region->endManeuverOffset - region->startManeuverOffset;
    }
    else
    {
        region->length = regionLength;
    }
}

NB_Error
NB_RouteInformationGetContentRegionSplinePath(NB_RouteInformation* route, uint32 regionIndex, uint32 splineIndex, const char**pathId, uint32* splineSize, const uint8** splineData)
{
    data_extended_content_region* contentRegionData = 0;
    data_content_path* contentPath = 0;

    if (!route || !splineSize || !splineData)
    {
        return NE_INVAL;
    }

    if (regionIndex >= (uint32)CSL_VectorGetLength(route->dataReply.extended_content.vec_extended_content_region))
    {
        return NE_NOENT;
    }

    contentRegionData = (data_extended_content_region*)CSL_VectorGetPointer(route->dataReply.extended_content.vec_extended_content_region, regionIndex);
    if (!contentRegionData)
    {
        return NE_NOENT;
    }

    if (splineIndex >= (uint32)CSL_VectorGetLength(contentRegionData->vec_content_path))
    {
        return NE_NOENT;
    }

    contentPath = (data_content_path*)CSL_VectorGetPointer(contentRegionData->vec_content_path, splineIndex);
    if (!contentPath)
    {
        return NE_NOENT;
    }

    *pathId = contentPath->id;
    *splineSize = (uint32)contentPath->route_spline_packed.size;
    *splineData = contentPath->route_spline_packed.data;

    return NE_OK;
}

NB_Error
NB_RouteCollapseManeuvers(NB_RouteInformation* route,
                          uint32 nman,
                          double* padddist,
                          uint32* paddtime,
                          nb_boolean* pstack,
                          uint32* ppmaneuver_base,
                          uint32* ppmaneuver,
                          uint32* pnman,
                          double* padddist_next,
                          uint32* paddtime_next,
                          nb_boolean* pstacknext,
                          uint32* ppmaneuvernext,
                          uint32* pnman_next,
                          nb_boolean isManeuverList)
{
    data_nav_maneuver* maneuver = NULL;
    data_nav_maneuver* maneuver_stack = NULL;
    uint32 maneuver_next = 0;
    double adddist = 0;
    uint32 addtime = 0;
    boolean stack = FALSE;
    double adddist_next = 0;
    uint32 addtime_next = 0;
    boolean stack_next = FALSE;
    uint32 n = 0;
    uint32 nman_next = 0;
    uint32 nman_next_next = 0;
    data_util_state* dataState = NULL;

    if (!route)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(route->context);

    if (!dataState)
        return NE_INVAL;

    if (nman == NAV_MANEUVER_START)
    {
        if (padddist)
            *padddist = 0.0;
        if (paddtime)
            *paddtime = 0;
        if (pstack)
            *pstack = FALSE;
        if (ppmaneuver_base)
            *ppmaneuver_base = 0;
        if (ppmaneuver)
            *ppmaneuver = 0;
        if (pnman)
            *pnman = NAV_MANEUVER_NONE;
        if (padddist_next)
            *padddist_next = 0.0;
        if (paddtime_next)
            *paddtime_next = 0;
        if (pstacknext)
            *pstacknext = FALSE;
        if (ppmaneuvernext)
            *ppmaneuvernext = 0;
        if (pnman_next)
            *pnman_next = NAV_MANEUVER_NONE;

        return NE_OK;
    }

    if (nman == NAV_MANEUVER_INITIAL)
    {
        nman = 0;
    }

    adddist = 0.0;
    addtime = 0;
    stack = FALSE;
    adddist_next = 0.0;
    addtime_next = 0;
    stack_next = FALSE;
    nman_next = NAV_MANEUVER_NONE;
    nman_next_next = NAV_MANEUVER_NONE;

    if (ppmaneuver_base)
    {
        *ppmaneuver_base = nman;
    }

    for (n = nman; n < data_nav_reply_num_nav_maneuver(dataState, &route->dataReply); n++)
    {
        maneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, n);
        if (ppmaneuver)
        {
            *ppmaneuver = n;
        }
        nman_next = n;

        if (ppmaneuvernext)
        {
            NB_RouteCollapseManeuvers(route, n+1, &adddist_next, &addtime_next, &stack_next,
                NULL, &maneuver_next, &nman_next_next, NULL, NULL, NULL, NULL, NULL, isManeuverList);

            *ppmaneuvernext = n+1;
        }

        if (n != nman)
        {
            adddist += maneuver->distance;
            addtime += (uint32) (maneuver->speed != 0 ? (maneuver->distance / maneuver->speed) : 0);
        }

        if ((isManeuverList && NB_RouteInformationIsManeuverSignificantForTurnLists(route, n,FALSE)) ||
            (!isManeuverList && !is_informational(maneuver->command)))
        {
            break;
        }
    }

    for (n = nman; n < data_nav_reply_num_nav_maneuver(dataState, &route->dataReply); n++)
    {
        maneuver_stack = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, n);

        if (n > nman_next &&
            ((isManeuverList && !NB_RouteInformationIsManeuverSignificantForTurnLists(route, n,FALSE)) ||
            (!isManeuverList && !is_informational(maneuver_stack->command))))
        {
            break;
        }
        // set stack to TRUE for first maneuver NC. when isManeuverList is FALSE and
        // for first maneuver which is signficant when isManeuverList is TRUE
        // because there is probability to have stack advise for next significant maneuver
        if (n == 0 && ((isManeuverList && NB_RouteInformationIsManeuverSignificantForTurnLists(route, n,FALSE)) ||
            (!isManeuverList && is_informational(maneuver_stack->command))))
        {
            stack = TRUE;
        }
        else
        {
            stack =  (n != nman) ? (stack && maneuver_stack->stack_advise) : (maneuver_stack->stack_advise);
        }
    }

    if (padddist)
        *padddist  = adddist;
    if (paddtime)
        *paddtime  = addtime;
    if (pstack)
        *pstack    = stack;
    if (pnman)
        *pnman = nman_next;
    if (padddist_next)
        *padddist_next = adddist_next;
    if (paddtime_next)
        *paddtime_next = addtime_next;
    if (pstacknext)
        *pstacknext = stack_next;
    if (pnman_next)
        *pnman_next = nman_next_next;

    return NE_OK;
}

nb_boolean
NB_RouteInformationCompareRouteIDs(NB_RouteInformation* route1, NB_RouteInformation* route2)
{
    if (!route1 || !route2)
    {
        return FALSE;
    }

    if ( route1->routeID.size != route2->routeID.size )
    {
        return FALSE;
    }

    if ( nsl_memcmp(route1->routeID.data, route2->routeID.data, route1->routeID.size) != 0 )
    {
        return FALSE;
    }

    return TRUE;
}

struct CacheUpdateParams
{
    NB_Context* context;
    CSL_Cache* voiceCache;
};

NB_Error
NB_RouteInformationCreateFromTPSReply(NB_Context* context, tpselt reply, NB_RouteParameters* params, NB_RouteInformation** route)
{
    NB_Error err = NE_OK;
    NB_RoutePrivateConfiguration config;
    NB_RouteSettings settings = {0};
    int nman=0, n=0;
    NB_RouteInformation* newRoute = NB_RouteInformationAlloc(context);
    data_util_state* dataState = NULL;
    data_nav_maneuver* pman = NULL;
    struct CacheUpdateParams cacheParams = {0};
    nb_boolean verboseLog = CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context));

    nsl_memset(&config, 0, sizeof(config));
    if (newRoute == NULL)
    {
        return NE_NOMEM;
    }

    dataState = NB_ContextGetDataState(context);

    err = NB_RouteParametersClone(params, &newRoute->routeParameters);
    err = err ? err : NB_RouteParametersCopyPrivateConfigFromParameters(params, &config);

    err = err ? err : data_nav_reply_from_tps(NB_ContextGetDataState(newRoute->context), &newRoute->dataReply, reply, config.prefs.navRouteMapPasses);
    if (err != NE_OK)
    {
        NB_RouteInformationDestroy(newRoute);
        if (config.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableDestroy(config.prefs.laneGuidanceFontMap, TRUE);
            config.prefs.laneGuidanceFontMap = NULL;
        }
        *route = NULL;
        return err;
    }

    NB_RouteParametersGetRouteSettingsFromQuery(params, &settings);
    NB_RouteInformationCopyRouteSettings(&newRoute->dataReply.route_settings, &settings);
    NB_RouteParametersCopyConfigFromParameters(params, &newRoute->parameterConfiguration);

    newRoute->config = config;
    newRoute->transportMode = (newRoute->dataReply.route_settings.vehicle_type == NB_TransportationMode_Pedestrian) ? NB_NTM_Pedestrian : NB_NTM_Vehicle;
    newRoute->camerasOnRoute = FALSE;
    newRoute->collapseInformational = FALSE;
    newRoute->useFetchCorridor = TRUE;

    SetNIMLocationFromLocation(&newRoute->start.location, dataState, &newRoute->dataReply.origin_location);
    SetNIMLocationFromLocation(&newRoute->destination.location, dataState, &newRoute->dataReply.destination_location);

    NB_RouteParametersGetOriginGpsLocation(params, &newRoute->originFix);
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
            if (verboseLog)
            {
                QaLogNaturalLaneGuidanceInformation(context, n, pman);
            }
        }
    }

    cacheParams.context = context;
    cacheParams.voiceCache = NB_ContextGetVoiceCache(context);

    if (cacheParams.voiceCache)
    {
        data_nav_reply_file_foreach(dataState, &newRoute->dataReply, NB_RoutePronunCacheUpdate, &cacheParams);
    }

    newRoute->routeLength   = GetRouteLength(newRoute);
    GetRouteTimeAndDelay(newRoute, &newRoute->routeTime, &newRoute->routeDelay);

    newRoute->routeID.size = (nb_size)newRoute->dataReply.route_id.size;
    newRoute->routeID.data = newRoute->dataReply.route_id.data;

    data_nav_reply_get_route_count(dataState, &newRoute->dataReply, &newRoute->numberOfSelector);

    QaLogReply(context, newRoute);
    QaLogRouteTrafficRegion(context, newRoute);
    QaLogRouteRealisticSigns(context, newRoute);
    QaLogRouteEnhancedContentInfo(context, newRoute);

    *route = newRoute;

    if (config.prefs.laneGuidanceFontMap)
    {
        CSL_HashTableDestroy(config.prefs.laneGuidanceFontMap, TRUE);
        config.prefs.laneGuidanceFontMap = NULL;
    }

    return err;
}

static void
NB_RoutePronunCacheUpdate(data_file* pf, void* pUser)
{
    struct CacheUpdateParams* cacheParams = (struct CacheUpdateParams*) pUser;

    if (cacheParams->voiceCache != NULL)
    {
        CSL_CacheAdd(cacheParams->voiceCache, pf->name.data, pf->name.size, pf->data.data, pf->data.size, TRUE, TRUE, 0);
        NB_QaLogPronunFileDownload(cacheParams->context, (const char*) pf->name.data, (nb_size)pf->name.size);
    }
}

NB_Error
NB_RouteInformationCopyRouteSettings(NB_RouteSettings* destination, NB_RouteSettings* source)
{
    if (!destination || !source)
        return NE_INVAL;

    destination->route_avoid = source->route_avoid;
    destination->route_type = source->route_type;
    destination->traffic_type = source->traffic_type;
    destination->vehicle_type =source->vehicle_type;

    return NE_OK;
}

NB_RouteInformation*
NB_RouteInformationAlloc(NB_Context* context)
{
    NB_RouteInformation* pThis = NULL;

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    data_nav_reply_init( NB_ContextGetDataState(context), &pThis->dataReply);

    gpsfix_clear(&pThis->originFix);

    pThis->routeDescription = NULL;
    pThis->currentDescription = NULL;

    return pThis;
}

double
GetRouteLength(NB_RouteInformation* route)
{
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;
    data_nav_maneuver* pman;
    uint32 nm = 0;
    uint32 n = 0;
    double ret = 0;

    if (!route)
    {
        return 0;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;
    nm = NB_RouteInformationGetManeuverCount(route);

    for (n=0; n<nm; n++)
    {
        pman = data_nav_reply_get_nav_maneuver(dataState, reply, n);
        ret += pman->distance;
    }

    return ret;
}

static NB_Error
RouteNavSpeedCallback(data_util_state* pds, const char* tmcloc,
                      TrafficSpeedType speed_type, double minspeed,
                      double* pspeed, char* pcolor, double* pfreeflowspeed,
                      TrafficSpeedType* pspeed_type_out, NB_RouteInformation* route)
{
    TrafficSpeedType speed_type_out = TrafficSpeedStandard;
    NB_Error error = data_nav_reply_get_speed(pds, tmcloc, speed_type, minspeed,
                                              pspeed, pcolor, pfreeflowspeed,
                                              &speed_type_out, &route->dataReply);
    if (error == NE_OK && speed_type_out != TrafficSpeedStandard)
    {
        route->trafficDataValid = TRUE;
    }

    if (pspeed_type_out)
    {
        *pspeed_type_out = speed_type_out;
    }

    return error;
}


void
GetRouteTimeAndDelay(NB_RouteInformation* route,  uint32* pTime, uint32* pDelay)
{
    data_util_state* dataState = NULL;
    uint32 man_start = 0;
    uint32 man_end = 0;
    NB_Error trafficResult = NE_OK;

    nsl_assert(route != NULL);
    dataState = NB_ContextGetDataState(route->context);

    man_end = data_nav_reply_num_nav_maneuver(dataState, &route->dataReply);

    route->trafficDataValid = FALSE;

    trafficResult = data_nav_reply_get_traffic_info(dataState, &route->dataReply,
                                                    route->config.prefs.navMinTmcSpeed,
                                                    route->config.prefs.minNoDataDisableTrafficBar,
                                                    man_start, 0, man_end, TrafficSpeedAll,
                                                    pDelay, NULL, pTime,
                                                    (nav_speed_cb)RouteNavSpeedCallback,
                                                    route);

    // no traffic information to compute route time with, so compute time using speed and length of each maneuver
    if (trafficResult == NE_NOENT)
    {
        NB_RouteInformationTripRemainAfterManeuver(
                &route->config.prefs, dataState, &route->dataReply, NAV_MANEUVER_START, NULL, pTime);
        if (pDelay)
        {
            *pDelay = 0;
        }
    }
}

data_nav_maneuver*
NB_RouteInformationGetManeuver(NB_RouteInformation* route, uint32 maneuverIndex)
{
    data_util_state* dataState = NULL;

    if (!route || !route->context)
        return NULL;

    dataState = NB_ContextGetDataState(route->context);

    if (!dataState)
        return NULL;

    return data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, maneuverIndex);
}

//@todo (BUG 56045) compare against nav_distance_to_maneuver
NB_Error NB_RouteGetDistanceBetweenManeuvers(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, double* pdist)
{

    data_util_state* dataState = NULL;
    data_nav_maneuver* pmaneuver = NULL;
    uint32 nm, n;

    if (!route || !route->context || !pdist)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(route->context);

    nm = data_nav_reply_num_nav_maneuver(dataState, &route->dataReply);

    *pdist = 0.0;

    if (startManeuver  == NAV_MANEUVER_NONE)
    {
        *pdist = 0.0;

        for (n = 0; n <= endManeuver; n++) {

            pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, n);
            *pdist += pmaneuver->distance;
        }
    }
    else if (endManeuver < startManeuver)
    {
        *pdist = 0.0;
    }
    else
    {
        for (n = startManeuver; n <= endManeuver; n++) {

            pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, n);
            if (pmaneuver)
                *pdist += pmaneuver->distance;
        }
    }
    return NE_OK;
}


NB_Error NB_RouteProtectVoiceCache(NB_RouteInformation* route)
{
    CSL_Cache* voiceCache = NULL;
    data_nav_maneuver* pman = NULL;
    data_util_state* dataState = NULL;
    int nman=0, n=0;

    if (!route)
        return NE_INVAL;

    voiceCache = NB_ContextGetVoiceCache(route->context);
    dataState = NB_ContextGetDataState(route->context);

    if (!dataState)
        return NE_INVAL;

    if (!voiceCache)
        return NE_OK;

    CSL_CacheClearProtect(voiceCache);

    nman = data_nav_reply_num_nav_maneuver(dataState, &route->dataReply);
    for (n = 0; n < nman; n++)
    {
        pman = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, n);
        if(pman)
        {
            if (pman->current_roadinfo.pronun.data != NULL && pman->current_roadinfo.pronun.size > 0)
                CSL_CacheProtect(voiceCache, pman->current_roadinfo.pronun.data, pman->current_roadinfo.pronun.size);
            if (pman->turn_roadinfo.pronun.data != NULL && pman->turn_roadinfo.pronun.size > 0)
                CSL_CacheProtect(voiceCache, pman->turn_roadinfo.pronun.data, pman->turn_roadinfo.pronun.size);
        }
    }

    return NE_OK;
}

static nb_boolean
IsLastPolylineSegment(data_util_state* dataState, data_polyline* polyline, uint32 segment)
{
    // Since polyline points are zero-based, subtract 2 from the total number of points
    // to get the second to the last point, which would be the start of the last segment
    if (dataState && polyline && segment == data_polyline_len(dataState, polyline) - 2)
    {
        return TRUE;
    }
    return FALSE;
}

nb_boolean
IsItemInVoiceCache(NB_Context* context, byte* data, uint32 size)
{
    if (!context || !data || size == 0)
    {
        return FALSE;
    }
    else
    {
        CSL_Cache* voiceCache = NB_ContextGetVoiceCache(context);

        if (voiceCache && CSL_CacheIsItemInCache(voiceCache, data, size))
        {
            return TRUE;
        }

        //this function should return TRUE for base voices
        if (nsl_strncmp((const char*)data, "NIM", 3) == 0)
        {
            return TRUE;
        }

        return FALSE;
    }
}

data_guidance_point*
GetGuidancePointPointer(data_nav_maneuver* maneuver, NB_NavigateManeuverPos maneuverPos)
{
    data_guidance_point* guidancePoint = NULL;

    if (!maneuver)
    {
        return guidancePoint;
    }

    if (maneuver != NULL)
    {
        if (maneuverPos == NB_NMP_Prepare)
        {
            if (maneuver->guidance_pointer[0].use_for_prepare)
            {
                guidancePoint = &maneuver->guidance_pointer[0];
            }
            else if (maneuver->guidance_pointer[1].use_for_prepare)
            {
                guidancePoint = &maneuver->guidance_pointer[1];
            }
        }
        else if (maneuverPos == NB_NMP_Turn)
        {
            if (maneuver->guidance_pointer[0].use_for_turn)
            {
                guidancePoint = &maneuver->guidance_pointer[0];
            }
            else if (maneuver->guidance_pointer[1].use_for_turn)
            {
                guidancePoint = &maneuver->guidance_pointer[1];
            }
        }
    }

    return guidancePoint;
}

data_nav_reply*
NB_RouteInformationGetDataNavReply(NB_RouteInformation *route)
{
    return &route->dataReply;
}

NB_RoutePrivateConfiguration*
NB_RouteInformationGetPrivateConfiguration(NB_RouteInformation* route)
{
    return &route->config;
}

int
NB_RouteInformationFindSegmentMatches(NB_RouteInformation* route, NB_GpsLocation* pfix, double heading, uint32 manprogress, uint32 segprogress, double last_heading, struct seg_match* pmatches, int nmaxmatches, struct route_position* prevpos)
{
    uint32                maneuver=0, segment=0, num_manevers=0, num_points=0;
    data_nav_maneuver*    pmaneuver = NULL;
    data_nav_maneuver*    pnextmaneuver = NULL;
    struct seg_match*    psm=NULL;
    double                p1_lat=0.0, p1_lon=0.0;
    double                p2_lat=0.0, p2_lon=0.0;
    double                p1_p2_heading=0.0, p1_p2_len=0.0;
    double                p2_p3_heading=0.0, p2_p3_len=0.0;
    double                proj_lat=0.0, proj_lon=0.0, dist=0.0;
    int                    num_matches = 0;
    int                    c=0;
    double                unc=0.0;
    double                dist_turn=0.0;
    boolean                same_dir_match=FALSE;
    int                    n=0;
    double                tripremain=0.0, maneuverremain=0.0;
    uint32                timediff=0;
    data_util_state*    dataState = NULL;
    double navMinError = 0.0;
    double navMaxError = 0.0;

    if (!route || !pfix)
    {
        return NE_INVAL;
    }

    navMinError = (route->parameterConfiguration.routeDetectionLevel == NBRDL_Tight) ?
        route->config.prefs.navMinErrorTightLevel : route->config.prefs.navMinError;

    navMaxError = (route->parameterConfiguration.routeDetectionLevel == NBRDL_Tight) ?
        route->config.prefs.navMaxErrorTightLevel : route->config.prefs.navMaxError;

    dataState = NB_ContextGetDataState(route->context);

    num_manevers = data_nav_reply_num_nav_maneuver(dataState, &route->dataReply);

    for (maneuver = manprogress; maneuver < num_manevers && num_matches < (nmaxmatches-1); maneuver++)
    {
        pmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, maneuver);
        pnextmaneuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, maneuver+1);
        num_points = data_polyline_len(dataState, &pmaneuver->polyline);

        if (num_points < 2)
        {
            continue;
        }

        if (!data_polyline_in_bounds(dataState, &pmaneuver->polyline, pfix->latitude, pfix->longitude, route->config.prefs.bboxbuffer))
        {
            continue;
        }

        for (segment = (maneuver == manprogress) ? segprogress : 0; segment < (num_points-1) && num_matches < nmaxmatches; segment++)
        {
            uint16 multiplier = is_ferry_exit(data_string_get(dataState, &pmaneuver->command)) ? 4:1;
            uint16 headingmultiplier = is_ferry_exit(data_string_get(dataState, &pmaneuver->command)) ? 1:0;
            (void)data_polyline_get(dataState, &pmaneuver->polyline, segment, &p1_lat, &p1_lon, &p1_p2_len, &p1_p2_heading);
            (void)data_polyline_get(dataState, &pmaneuver->polyline, segment+1, &p2_lat, &p2_lon, &p2_p3_len, &p2_p3_heading);

            if (p2_p3_heading == POLYLINE_INVALID_HEADING && pnextmaneuver != NULL &&
                data_polyline_len(dataState, &pnextmaneuver->polyline) > 1)
            {
                (void)data_polyline_get(dataState, &pnextmaneuver->polyline, 0, NULL, NULL, &p2_p3_len, &p2_p3_heading);
            }

            /* Skip Zero Length Segments */
            if (p1_lat == p2_lat && p1_lon == p2_lon)
            {
                continue;
            }

            c = project_point_to_line(pfix->latitude, pfix->longitude, p1_lat, p1_lon, p2_lat, p2_lon, &proj_lat, &proj_lon, &dist);

            if (route->config.prefs.navEllErrorEnable)
            {
                unc = unc_to_pt(pfix, proj_lat, proj_lon);
            }
            else
            {
                unc = MAX(pfix->horizontalUncertaintyAlongAxis, pfix->horizontalUncertaintyAlongPerpendicular);
            }

            if (navMinError)
            {
                unc = navMinError;
            }

            if (unc > navMaxError)
            {
                unc = navMaxError;
            }

            same_dir_match = FALSE;

            if ((route->transportMode == NB_NTM_Pedestrian && dist <= route->config.prefs.navPedRecalcThreshold) ||
                (dist <= unc ||
                ((route->parameterConfiguration.routeDetectionLevel == NBRDL_Tight &&
                  compare_heading(heading, p1_p2_heading, route->config.prefs.navL1HeadingMarginTightLevel<<headingmultiplier, NULL) &&
                  dist <= route->config.prefs.navL1DistThresholdTightLevel * multiplier) ||
                (route->parameterConfiguration.routeDetectionLevel == NBRDL_Relaxed &&
                ((compare_heading(heading, p1_p2_heading, route->config.prefs.navL6HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL6DistThreshold*multiplier) ||
                (compare_heading(heading, p1_p2_heading, route->config.prefs.navL5HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL5DistThreshold*multiplier) ||
                (compare_heading(heading, p1_p2_heading, route->config.prefs.navL4HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL4DistThreshold*multiplier) ||
                (compare_heading(heading, p1_p2_heading, route->config.prefs.navL3HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL3DistThreshold*multiplier) ||
                (compare_heading(heading, p1_p2_heading, route->config.prefs.navL2HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL2DistThreshold*multiplier) ||
                (compare_heading(heading, p1_p2_heading, route->config.prefs.navL1HeadingMargin<<headingmultiplier, NULL) && dist <= route->config.prefs.navL1DistThreshold*multiplier))))))
            {
                psm = &pmatches[num_matches++];

                psm->time        = pfix->gpsTime;
                psm->travel_speed = -1;
                psm->maneuver    = maneuver;
                psm->segment    = segment;
                psm->remain        = NB_SpatialGetLineOfSightDistance(p2_lat, p2_lon, proj_lat, proj_lon, NULL);
                psm->distance    = dist;
                psm->proj_lat    = proj_lat;
                psm->proj_lon    = proj_lon;
                psm->heading    = p1_p2_heading;

                psm->dirmatch    = compare_heading(heading, p1_p2_heading, route->config.prefs.navHeadingMargin*multiplier, NULL);

                psm->type = c == 0 ? match_proj : c < 0 ? match_start : match_end;

                if (psm->dirmatch)
                {
                    same_dir_match = TRUE;
                }
            }

            if ((num_matches < (nmaxmatches-1)) &&                        // make sure we don't overrun the buffer
                (!same_dir_match) &&                                    // we don't already have a match
                IsLastPolylineSegment(dataState, &pmaneuver->polyline, segment) &&  // we are on the last polyline segment
                (p2_p3_heading != POLYLINE_INVALID_HEADING) &&            // we have a valid heading for the next segment
                (dist <= route->config.prefs.navTurnThreshold) &&       // we are within the threshold distance of the segment
                ((dist_turn = NB_SpatialGetLineOfSightDistance(pfix->latitude, pfix->longitude, p2_lat, p2_lon, NULL)) <= route->config.prefs.navTurnThreshold) &&  // we are within the threshold distance of the turn point
                heading != INVALID_HEADING &&                            // require the heading to be valid
                heading_between(p1_p2_heading, p2_p3_heading, heading, route->config.prefs.navTurnBuf))
            {
                psm = &pmatches[num_matches++];

                psm->time        = pfix->gpsTime;
                psm->travel_speed = -1;
                psm->maneuver    = maneuver;
                psm->segment    = segment;
                psm->remain        = NB_SpatialGetLineOfSightDistance(p2_lat, p2_lon, proj_lat, proj_lon, NULL);
                psm->distance    = dist;
                psm->proj_lat    = proj_lat;
                psm->proj_lon    = proj_lon;
                psm->heading    = p1_p2_heading;
                psm->dirmatch    = TRUE;

                psm->type        = match_turn;
            }
        }
    }

    // if there is more than one match, calculate the necessary travel speed
    // based on the previous fix.  This will help exclude impossible fixes.
    if (num_matches > 1 && prevpos->match_type != rt_match_none &&
        prevpos->closest_maneuver != NAV_MANEUVER_NONE &&
        prevpos->closest_segment != SEGMENT_NONE &&
        prevpos->time <= (uint32) pfix->gpsTime)
    {
        for (n = 0; n < num_matches; n++)
        {
            psm = pmatches+n;

            maneuverremain = NB_RouteInformationManeuverRemainingDist(dataState, &route->dataReply, psm->maneuver, psm->segment, psm->remain);
            NB_RouteInformationTripRemainAfterManeuver(&route->config.prefs, dataState, &route->dataReply, psm->maneuver, &tripremain, NULL);

            dist = maneuverremain + tripremain;

            timediff = psm->time-prevpos->time;

            // if we get two fixes in the same second, assume that they were one second apart for speed
            // calculation purposes.  This will be a lower bound on the speed
            if (timediff == 0)
            {
                timediff = 1;
            }

            psm->travel_speed = ((prevpos->trip_distance_after  + prevpos->maneuver_distance_remaining)-dist)/(timediff);
        }
    }

    return num_matches;
}

struct seg_match*
    NB_RouteInformationBestMatch(struct seg_match* new_match, struct seg_match* cur_match, boolean use_speed, boolean* pchange)
{
    if (use_speed && (cur_match->travel_speed <= 55.0) && (new_match->travel_speed >= 55.0))
    {
        /* The new match has a very high speed, so don't use it. */
        *pchange = FALSE;
    }
    else if ((use_speed && (cur_match->travel_speed > 55.0) && (new_match->travel_speed <= 55.0)) ||
        ((cur_match->type == match_start || cur_match->type == match_end) && new_match->type == match_proj && new_match->dirmatch) ||
        ((cur_match->type == match_start || cur_match->type == match_end) && new_match->type == match_turn && new_match->dirmatch) ||
        (cur_match->type == match_proj && !cur_match->dirmatch && new_match->type == match_turn && new_match->dirmatch) ||
        (cur_match->type == match_turn && new_match->type == match_proj && new_match->dirmatch) ||
        (cur_match->type == match_proj && !cur_match->dirmatch && new_match->type == match_proj && new_match->dirmatch) ||
        (cur_match->type == match_proj && cur_match->dirmatch && new_match->type == match_proj && new_match->dirmatch && new_match->distance<cur_match->distance) ||
        ((cur_match->type == match_start || cur_match->type == match_end || cur_match->type == match_turn || cur_match->type == match_proj)
        && !cur_match->dirmatch && new_match->type == match_proj && !new_match->dirmatch && new_match->distance < cur_match->distance))
    {
        cur_match = new_match;
        *pchange = TRUE;
    }
    else
    {
        *pchange = FALSE;
    }

    return cur_match;
}

int
NB_RouteInformationFindBestMatch(NB_RouteInformation* route, struct seg_match* pmatches, int nmatches, boolean use_speed)
{
    struct seg_match*   psm             = NULL;
    struct seg_match*   psm_nospeed     = NULL;
    int                 retval          = -1;
    int                 retval_nospeed  = -1;
    boolean             change          = FALSE;
    int                 n               = 0;

    if (nmatches > 0)
    {
        psm_nospeed = psm = pmatches;
        retval_nospeed = retval = 0;

        for (n = 1; n < nmatches; n++)
        {
            change = FALSE;
            psm = NB_RouteInformationBestMatch(&pmatches[n], psm, use_speed, &change);

            if (change)
            {
                retval = n;
            }

            change = FALSE;
            psm_nospeed = NB_RouteInformationBestMatch(&pmatches[n], psm_nospeed, FALSE, &change);

            if (change)
            {
                retval_nospeed = n;
            }
        }
    }

    if (retval != retval_nospeed)
    {
        NB_QaLogAppState(route->context, "high speed");
    }

    return retval;
}

uint32
NB_RouteInformationGetTurnTime(NB_RoutePreferencesPrivate* prefs, const char* szturn)
{
    uint32 default_time = 0;
    int n;

    for (n=0; n<prefs->numNavTurnTimes; n++) {

        if (prefs->navTurnTimes[n].command[0] == 0) {

            default_time = prefs->navTurnTimes[n].time;

            if (szturn == NULL || nsl_strlen(szturn) == 0)
                return default_time;
        }
        else if (nsl_strncmp(szturn, prefs->navTurnTimes[n].command, nsl_strlen(prefs->navTurnTimes[n].command)) == 0) {

            return prefs->navTurnTimes[n].time;
        }
    }

    return default_time;
}


void
NB_RouteInformationSpeedCameraRemain(NB_RoutePreferencesPrivate* prefs, data_util_state* pdus, data_nav_reply* preply, uint32 nmaneuver, double* pdist, uint32* ptime)
{
    data_nav_maneuver* pmaneuver;
    uint32 total_maneuver = CSL_VectorGetLength(preply->vec_nav_maneuver);
    uint32 cur_maneuver;
    double dist = 0.0;
    uint32 time = 0;

    for (cur_maneuver = (nmaneuver == NAV_MANEUVER_START || nmaneuver == NAV_MANEUVER_INITIAL) ? 0 : nmaneuver + 1;
         cur_maneuver < total_maneuver; cur_maneuver++)
    {
            pmaneuver = data_nav_reply_get_nav_maneuver(pdus, preply, cur_maneuver);

            dist += pmaneuver->distance;
            time += NB_RouteInformationGetManueverTime(prefs, pdus, preply, cur_maneuver);
            if (is_speedcamera(data_string_get(pdus, &pmaneuver->command)))
            {
                // Exit at first speed camera match
                break;
            }
    }

    if (pdist != NULL)
        *pdist = dist;

    if (ptime != NULL)
        *ptime = time;
}

uint32
NB_RouteInformationGetManueverTime(NB_RoutePreferencesPrivate* prefs, data_util_state* pds, data_nav_reply* preply, uint32 nmaneuver)
{
    data_nav_maneuver* pnm = data_nav_reply_get_nav_maneuver(pds, preply, nmaneuver);

    return NB_RouteInformationGetTurnTime(prefs, data_string_get(pds, &pnm->command)) +
        (pnm->speed != 0 ? (uint32)(pnm->distance / pnm->speed) : 0);
}

void
NB_RouteInformationTripRemainAfterManeuver(NB_RoutePreferencesPrivate* prefs, data_util_state* pdus, data_nav_reply* preply, uint32 nmaneuver, double* pdist, uint32* ptime)
{
    data_nav_maneuver* pmaneuver;
    uint32 total_maneuver = CSL_VectorGetLength(preply->vec_nav_maneuver);
    uint32 cur_maneuver;
    double dist = 0.0;
    uint32 time = 0;

    for (cur_maneuver = (nmaneuver == NAV_MANEUVER_START || nmaneuver == NAV_MANEUVER_INITIAL) ? 0 : nmaneuver + 1;
         cur_maneuver < total_maneuver; cur_maneuver++)
    {
            pmaneuver = data_nav_reply_get_nav_maneuver(pdus, preply, cur_maneuver);

            dist += pmaneuver->distance;
            time += NB_RouteInformationGetManueverTime(prefs, pdus, preply, cur_maneuver);
    }

    if (pdist != NULL)
        *pdist = dist;

    if (ptime != NULL)
        *ptime = time;
}

double
NB_RouteInformationManeuverRemainingDist(data_util_state* dataState, data_nav_reply* preply, uint32 maneuver, uint32 segment, double segment_remain)
{
    data_nav_maneuver* pnavmaneuver = NULL;
    uint32 n,l;
    double dist_remain = 0;
    double distance;

    pnavmaneuver = data_nav_reply_get_nav_maneuver(dataState, preply, maneuver);

    dist_remain = pnavmaneuver->distance;

    l = data_polyline_len(dataState, &pnavmaneuver->polyline);

    for (n = 0; n < l && n <= segment; n++) {

        data_polyline_get(dataState, &pnavmaneuver->polyline, n, NULL, NULL, &distance, NULL);
        dist_remain -= distance;
    }

    dist_remain += segment_remain;

    return dist_remain;
}

double
NB_RouteInformationTraveledDistance(data_util_state* dataState, data_nav_reply* preply, uint32 maneuver, uint32 segment, double segment_remain)
{
    data_nav_maneuver* pnavmaneuver = NULL;
    double distance = 0.0;
    double d = 0.0;
    uint32 i = 0;
    uint32 l = 0;

    if (maneuver == NAV_MANEUVER_NONE)
    {
        return 0;
    }
    if (maneuver == NAV_MANEUVER_START || maneuver == NAV_MANEUVER_INITIAL)
    {
        maneuver = 0;
    }

    l = CSL_VectorGetLength(preply->vec_nav_maneuver);
    for (i = 0; i < l && i < maneuver; i++)
    {
        pnavmaneuver = data_nav_reply_get_nav_maneuver(dataState, preply, i);
        distance += pnavmaneuver->distance;
    }

    pnavmaneuver = data_nav_reply_get_nav_maneuver(dataState, preply, maneuver);
    if (pnavmaneuver && segment != SEGMENT_NONE)
    {
        l = data_polyline_len(dataState, &pnavmaneuver->polyline);
        for (i = 0; i < l && i <= segment; i++)
        {
            data_polyline_get(dataState, &pnavmaneuver->polyline, i, NULL, NULL, &d, NULL);
            distance += d;
        }
    }

    if (segment_remain > 0)
    {
        distance -= segment_remain;
    }

    return distance;
}

double
NB_RouteInformationGetInstructionDistance(NB_RoutePreferencesPrivate* prefs, double speed, enum manuever_type manueuver_type, enum instruct_dist_type dist_type)
{
    return ((speed * prefs->instruct[manueuver_type].base_dist_mul)+prefs->instruct[manueuver_type].base_dist_add) * prefs->instruct[manueuver_type].mult[dist_type];
}


NB_Error
NB_RouteInformationGetTMCSpeed(NB_RouteInformation* route, const char* tmcloc, TrafficSpeedType speed_type, double* pspeed, NB_NavigateTrafficCongestionType* ptc)
{
    NB_Error err = NE_OK;
    char color = 0;
    data_util_state* dataState = NULL;
    data_nav_reply* navReply = NULL;
    NB_RoutePrivateConfiguration* privateConfig;

    dataState = NB_ContextGetDataState(route->context);
    navReply = NB_RouteInformationGetDataNavReply(route);
    privateConfig = NB_RouteInformationGetPrivateConfiguration(route);

    err = data_nav_reply_get_speed(dataState, tmcloc, speed_type, privateConfig->prefs.navMinTmcSpeed,
                                   pspeed, &color, NULL, NULL, navReply);

    // prevent divide by zero
    if (*pspeed == 0)
    {
        *pspeed = 0.0001;
    }

    if (err == NE_OK)
    {
        if (ptc)
        {
            switch (color)
            {
            case 'G':
                *ptc = NB_NTC_None;
                break;
            case 'Y':
                *ptc = NB_NTC_Moderate;
                break;
            case 'R':
                *ptc = NB_NTC_Severe;
                break;
            default:
                *ptc = NB_NTC_None;
                break;
            }
        }
    }

    return err;
}

boolean
NB_RouteInformationDataSource(uint32 elem, uint32 maneuverIndex, void* pUser, const char** pdata, nb_size* psize, boolean* pfreedata)
{
    NB_RouteDataSourceOptions* pThis        = (NB_RouteDataSourceOptions*) pUser;
    NB_Error err = NE_OK;
    data_nav_reply*     preply              = NULL;
    data_blob*    cache_id = NULL;
    data_nav_maneuver* maneuver = NULL;
    data_nav_maneuver* maneuver_next = NULL;
    data_nav_maneuver* maneuver_base = NULL;
    data_nav_maneuver* congestionManeuver = NULL;
    uint32 dManeuver = 0;
    uint32 dManeuver_next = 0;
    uint32 dManeuver_base = 0;
    uint32 dManeuver_stack = 0;
    data_nav_maneuver* maneuver_stack = NULL;
    char                    tmp[128]            = {0};
    nb_boolean                returntmp       = FALSE;
    nb_boolean              fail             = FALSE;
    double collapse_distance                = 0.0;
    nb_boolean collapse_stack               = FALSE;
    double collapse_next_distance           = 0.0;
    int                 l                   = 0;
    char*               p                   = NULL;
    const char*         t                   = NULL;
    uint32              traffic_delay_time  = 0;
    uint32              travel_time_val     = 0;
    double              traffic_meter       = 0;
    uint32 nman_show = NAV_MANEUVER_NONE;
    uint32 nman_start;
    uint32 nman_end;
    double start_offset=0.0;
    NB_RoutePrivateConfiguration* privateConfig = NULL;
    data_util_state* dataState                  = NULL;
    NB_TrafficInformation* trafficInfo = NULL;
    uint32 closetManeuver = NAV_MANEUVER_NONE;
    double maneuverDistanceRemaining = 0.0;
    uint32 remainingRouteTime = 0;
    double remainingRouteDist = 0.0;
    uint32 numberOfManeuvers = 0;
    double distToTrafficInfo = -1.0;

    if (!pThis || !pThis->route)
        return FALSE;

    dataState = NB_ContextGetDataState(pThis->route->context);
    preply = &pThis->route->dataReply;

    if (!preply)
        return FALSE;

    if (maneuverIndex == NAV_MANEUVER_START && elem == cardinal)
        maneuverIndex = 0;

    if (maneuverIndex == NAV_MANEUVER_START && elem == dpronun)
    {
        elem = opronun;
        maneuverIndex = 0;
    }

    if (maneuverIndex == NAV_MANEUVER_START && elem == dname)
    {
        elem = oname;
        maneuverIndex = 0;
    }

    if (maneuverIndex == NAV_MANEUVER_INITIAL && (elem != tcode && elem != scode && elem != nextmaneuverindex))
    {
        maneuverIndex = 0;
    }

    privateConfig = NB_RouteInformationGetPrivateConfiguration(pThis->route);
    trafficInfo = pThis->trafficInfo;

    maneuver = NB_RouteInformationGetManeuver(pThis->route, maneuverIndex);
    numberOfManeuvers = NB_RouteInformationGetManeuverCount(pThis->route);

    if (pThis->collapse)
    {
        NB_RouteCollapseManeuvers(pThis->route, maneuverIndex, &collapse_distance, NULL, &collapse_stack, &dManeuver_base, &dManeuver, &nman_show, &collapse_next_distance, NULL, NULL, &dManeuver_next, &dManeuver_stack, pThis->isManeuverList);
        maneuver = NB_RouteInformationGetManeuver(pThis->route, dManeuver);
        maneuver_next = NB_RouteInformationGetManeuver(pThis->route, dManeuver_next);
        maneuver_base = NB_RouteInformationGetManeuver(pThis->route, dManeuver_base);
        maneuver_stack = NB_RouteInformationGetManeuver(pThis->route, dManeuver_stack);
    }
    else
    {
        dManeuver = maneuverIndex == NAV_MANEUVER_INITIAL ? 0 : maneuverIndex;
        dManeuver_base = dManeuver;
        dManeuver_next = dManeuver + 1;
        nman_show = dManeuver_next;
        maneuver_base = maneuver;
        maneuver_next = NB_RouteInformationGetManeuver(pThis->route, dManeuver_next);
        maneuver_stack= maneuver_next;
    }

    if (maneuverIndex == NAV_MANEUVER_INITIAL)
    {
        if (pThis->collapse)
        {
            NB_RouteCollapseManeuvers(pThis->route, 0, &collapse_distance, NULL, &collapse_stack, &dManeuver_base, &dManeuver, &nman_show, &collapse_next_distance, NULL, NULL, &dManeuver_next, &dManeuver_stack, pThis->isManeuverList);
            maneuver = NB_RouteInformationGetManeuver(pThis->route, dManeuver);
            maneuver_base = NB_RouteInformationGetManeuver(pThis->route, dManeuver_base);
        }
        else
        {
            maneuver = NB_RouteInformationGetManeuver(pThis->route, 0);
            maneuver_base = maneuver;
        }
    }

    if (maneuver == NULL && maneuverIndex != NAV_MANEUVER_NONE &&
        maneuverIndex != NAV_MANEUVER_START && maneuverIndex != NAV_MANEUVER_ENHANCED_STARTUP)
    {
        fail = TRUE;
        goto exit_fail;
    }

    if (pThis->currentRoutePosition && !pThis->isPositionOnlyForLocation)
    {
        closetManeuver = pThis->currentRoutePosition->closestManeuver;
        maneuverDistanceRemaining = pThis->currentRoutePosition->maneuverDistanceRemaining;
        remainingRouteDist = pThis->currentRoutePosition->remainingRouteDist;
        remainingRouteTime = pThis->currentRoutePosition->remainingRouteTime;
    }
    else
    {
        if (maneuver_base)
        {
            maneuverDistanceRemaining = maneuver_base->distance;
        }
        else if (maneuver)
        {
            maneuverDistanceRemaining = maneuver->distance;
        }
        closetManeuver = dManeuver_base;
        NB_RouteInformationTripRemainAfterManeuver(&privateConfig->prefs, dataState, preply, NAV_MANEUVER_START, &remainingRouteDist, &remainingRouteTime);
    }

    if (closetManeuver == NAV_MANEUVER_INITIAL)
    {
        closetManeuver = 0;
    }

    if ( (elem == traffic_delay || elem == travel_time || elem == expect_traffic_type || elem == announce_delay) && trafficInfo )
    {
        nman_start = NAV_MANEUVER_START;
        nman_end = numberOfManeuvers;

        if (maneuverIndex == NAV_MANEUVER_NONE)
        {
            if (pThis->currentRoutePosition)
            {
                nman_start = pThis->currentRoutePosition->closestManeuver;

                maneuver = NB_RouteInformationGetManeuver(pThis->route, nman_start);

                if (maneuver != NULL)
                    start_offset = maneuver->distance - pThis->currentRoutePosition->maneuverDistanceRemaining;

                maneuver = NULL;
            }

            NB_TrafficInformationGetSummary(trafficInfo, pThis->route, nman_start, start_offset, nman_end, &traffic_meter, NULL, &travel_time_val, &traffic_delay_time);
        }
        else
        {
            nman_start = maneuverIndex;
            nman_end = nman_show + 1;

            if (maneuverIndex == closetManeuver && maneuver != NULL)
                start_offset = maneuver->distance - maneuverDistanceRemaining;

            err = NB_TrafficInformationGetSummary(pThis->trafficInfo, pThis->route, nman_start, start_offset, nman_end, &traffic_meter, NULL, &travel_time_val, &traffic_delay_time);

            if (err != NE_OK) {

                fail = TRUE;
                goto exit_fail;
            }
        }
    }

    switch (elem)
    {
    case dist:
        if (maneuver == NULL)
            fail = TRUE;
        else if (maneuverIndex == closetManeuver)
            pThis->route->tempDouble = maneuverDistanceRemaining + collapse_distance;
        else if (maneuverIndex > closetManeuver)
        {
            double distance = 0.0;

            (void)NB_RouteGetDistanceBetweenManeuvers(pThis->route, closetManeuver + 1, maneuverIndex, &distance);
            pThis->route->tempDouble = distance;
        }
        else
        {
            fail = TRUE;
        }

        if (!fail)
        {
            if (pdata)
                *pdata = (char*) &pThis->route->tempDouble;
            if (psize)
                *psize = sizeof(pThis->route->tempDouble);
        }

        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case dside:
        if (maneuver == NULL )
            fail = TRUE;
        else
        {
            t = data_string_get(dataState, &maneuver->turn_roadinfo.country_info.driving_side);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case mdist:            // double - maneuver distance
        if (maneuver == NULL)
            fail = TRUE;
        else if (maneuver_base == NULL)
            fail = TRUE;
        else {
            pThis->route->tempDouble = maneuver_base->distance + collapse_distance;

            if (pdata)
                *pdata = (char*) &pThis->route->tempDouble;
            if (psize)
                *psize = sizeof(pThis->route->tempDouble);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case bdist:
        if (maneuver == NULL)
            fail = TRUE;
        else if (maneuverIndex == closetManeuver)
            pThis->route->tempDouble = maneuverDistanceRemaining;
        else if (maneuver_base == NULL)
            fail = TRUE;
        else
            pThis->route->tempDouble = maneuver_base->distance;

        if (!fail) {
            if (pdata)
                *pdata = (char*) &pThis->route->tempDouble;
            if (psize)
                *psize = sizeof(pThis->route->tempDouble);
        }

        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case tripdist:
        NB_RouteInformationTripRemainAfterManeuver(&privateConfig->prefs, dataState, preply, maneuverIndex, &pThis->route->tempDouble, NULL);
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case triptime:
        NB_RouteInformationTripRemainAfterManeuver(&privateConfig->prefs, dataState, preply, maneuverIndex, NULL, &pThis->route->tempUint32);
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case cardinal:
        if (maneuver_base == NULL)
            fail = TRUE;
        else
        {
            format_heading_dir(maneuver_base->current_heading, tmp, sizeof(tmp));
            returntmp = TRUE;
        }
        break;
    case start:
        if (pdata)
        {
            *pdata = p = nsl_malloc(sizeof(NB_Place));

            if (p != NULL)
            {
                nsl_memset(p, 0, sizeof(NB_Place));
                if (SetNIMLocationFromLocation(&((NB_Place*)(p))->location, dataState, &preply->origin_location) == NE_OK)
                {
                    if (psize)
                        *psize = sizeof(NB_Place);
                    if (pfreedata)
                        *pfreedata = TRUE;
                }
                else
                {
                    nsl_free(p);
                    fail = TRUE;
                }
            }
            else
                fail = TRUE;
        }
        else {

            if (psize)
                *psize = sizeof(NB_Place);
            if (pfreedata)
                *pfreedata = TRUE;
        }
        break;
    case destination:
        if (pdata)
            *pdata = (const char*) &pThis->route->destination;
        if (psize)
            *psize = sizeof(pThis->route->destination);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case opronun:
        if (maneuver_base == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver_base->current_roadinfo.pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case dpronun:
        if (maneuver == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver->turn_roadinfo.pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case dpronun_base:
        if (maneuver_base == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver_base->turn_roadinfo.pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case ipronun:
        if (maneuver == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver->intersection_roadinfo.pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case oname:
        if (maneuver_base == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver_base->current_roadinfo.primary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case dname:
        if (maneuver == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver->turn_roadinfo.primary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case dname_base:
        if (maneuver_base == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver_base->turn_roadinfo.primary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case aname:
        if (maneuver == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver->turn_roadinfo.secondary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case aname_base:
        if (maneuver_base == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver_base->turn_roadinfo.secondary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case iname:
        if (maneuver == NULL)
            fail = TRUE;
        else {
            t = data_string_get(dataState, &maneuver->intersection_roadinfo.primary);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    // @todo (BUG 56046)
    // Commenting this out since it shouldn't be used since we now pass the position into the instruction set
    // function.  I am not deleting it since I am not sure that the camera announcement cases are handled properly
    //case navpos:
    //    if (maneuverIndex == NAV_MANEUVER_CAMERA || maneuverIndex == NAV_MANEUVER_CAMERA_SPEEDING)
    //    {
    //        if (pdata)
    //        {
    //            pThis->route->tempUint32 = NB_NMP_Turn;
    //            *pdata = (char*)&pThis->route->tempUint32;
    //        }
    //        if (psize)
    //            *psize = sizeof(pThis->route->tempUint32);
    //        if (pfreedata)
    //            *pfreedata = FALSE;
    //    }
    //    else
    //    {
    //        if (pdata)
    //            *pdata = (char*)&pThis->maneuverPos;
    //        if (psize)
    //            *psize = sizeof(pThis->maneuverPos);
    //        if (pfreedata)
    //            *pfreedata = FALSE;
    //    }
    //    break;
    case tcode:
        //@todo (BUG 56047) Remove these maneuver positions
        if (pThis->maneuverPos == NB_NMP_Calc ||
            pThis->maneuverPos == NB_NMP_Recalculate ||
            pThis->maneuverPos == NB_NMP_SoftRecalculate ||
            pThis->maneuverPos == NB_NMP_RecalculateConfirm ||
            pThis->maneuverPos == NB_NMP_RecalculateTraffic ||
            pThis->maneuverPos == NB_NMP_TrafficAlert ||
            pThis->maneuverPos == NB_NMP_TrafficDelay ||
            pThis->maneuverPos == NB_NMP_RouteUpdated ||
            pThis->maneuverPos == NB_NMP_SwitchToStaticMode)
        {
            fail = TRUE;
        }
        else if (pThis->maneuverPos == NB_NMP_TrafficCongestion)
        {
            t = "";
            switch (pThis->trafficState->nextCongestionSeverity)
            {
            case NB_NTC_None:
                fail = TRUE;
                break;
            case NB_NTC_Moderate:
                t = "TC.";
                break;
            case NB_NTC_Severe:
                t = "TC.S";
                break;
            case NB_NTC_Unknown:
                /* should not get here */
                nsl_assert(FALSE);
                break;
            }
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen (t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (pThis->maneuverPos == NB_NMP_TrafficIncident)
        {
            t = "";

            switch (pThis->trafficState->nextIncidentDetails.severity)
            {
            case NB_TrafficIncident_Severe:
                t = "TI.S";
                break;
            case NB_TrafficIncident_Major:
                t = "TI.S";
                break;
            case NB_TrafficIncident_Minor:
                t = "TI.";
                break;
            case NB_TrafficIncident_LowImpact:
                t = "TI.";
                break;
            default:
                fail = TRUE;
            }

            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuverIndex == NAV_MANEUVER_START)
        {
            t = "OR.";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuverIndex == NAV_MANEUVER_CAMERA)
        {
            t = "SC.";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuverIndex == NAV_MANEUVER_CAMERA_SPEEDING)
        {
            t = "SC.S";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuverIndex == NAV_MANEUVER_INITIAL)
        {
            if (get_maneuver_type(maneuver->command) != maneuver_type_hwy &&
                (pThis->maneuverPos == NB_NMP_Continue || pThis->maneuverPos == NB_NMP_Prepare) &&
                NB_RouteInformationGetManeuverCount(pThis->route) > 1 &&
                pThis->guidanceState && !pThis->guidanceState->hasFirstManeuverPlayed && get_m1_maneuver_enabled())
            {
                t = "M1.";
            }
            else
            {
                t = maneuver->command;
            }
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuverIndex == NAV_MANEUVER_ENHANCED_STARTUP)
        {
            t = "EH.";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (maneuver == NULL)
        {
            t = "TS.";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else if (pThis->navState && pThis->navState->transportMode == NB_NTM_Pedestrian &&
                 !pThis->navState->onRoute)
        {
            t = "PO.";
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        else {
            if (pdata)
                *pdata = maneuver->command;
            if (psize)
                *psize = (nb_size)(nsl_strlen(maneuver->command)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case scode:
        //@todo (BUG 56047) Remove these maneuver positions
        if (pThis->maneuverPos == NB_NMP_Calc ||
            pThis->maneuverPos == NB_NMP_Recalculate ||
            pThis->maneuverPos == NB_NMP_SoftRecalculate ||
            pThis->maneuverPos == NB_NMP_RecalculateConfirm ||
            pThis->maneuverPos == NB_NMP_RecalculateTraffic ||
            pThis->maneuverPos == NB_NMP_TrafficAlert ||
            pThis->maneuverPos == NB_NMP_TrafficDelay ||
            pThis->maneuverPos == NB_NMP_SwitchToStaticMode)
        {
            fail = TRUE;
        }
        else if (maneuver == NULL || (maneuverIndex != NAV_MANEUVER_INITIAL && !collapse_stack)
                 || maneuver_stack == NULL)
        {
            fail = TRUE;
        }
        else
        {
            if (pdata)
                *pdata = maneuver_stack->command;
            if (psize)
                *psize = (nb_size)(nsl_strlen(maneuver_stack->command)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case ncode:
        if (pThis->maneuverPos == NB_NMP_Calc ||
            pThis->maneuverPos == NB_NMP_Recalculate ||
            pThis->maneuverPos == NB_NMP_SoftRecalculate ||
            pThis->maneuverPos == NB_NMP_RecalculateConfirm ||
            pThis->maneuverPos == NB_NMP_RecalculateTraffic ||
            pThis->maneuverPos == NB_NMP_TrafficAlert ||
            pThis->maneuverPos == NB_NMP_TrafficDelay ||
            pThis->maneuverPos == NB_NMP_SwitchToStaticMode)
        {
            fail = TRUE;
        }
        else if (maneuver == NULL || maneuver_next == NULL)
            fail = TRUE;
        else
        {
            if (pdata)
                *pdata = maneuver_next->command;
            if (psize)
                *psize = (nb_size)(nsl_strlen(maneuver_next->command)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case unnamed:
        if (!maneuver_base)
        {
            fail = TRUE;
        }
        else {

            pThis->route->tempUint32 = (uint32) maneuver_base->turn_roadinfo.unnamed;
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case maxdist:
        if (pdata)
            *pdata = (char*) &privateConfig->prefs.maxTowardDistAnnounce;
        if (psize)
            *psize = sizeof(privateConfig->prefs.maxTowardDistAnnounce);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case step:
        if (pThis->collapse)
            fail = !get_maneuver_display(dataState, preply, nman_show, &pThis->route->tempUint32, NULL);
        else
            pThis->route->tempUint32 = dManeuver_next;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case totalsteps:
        if (pThis->collapse)
            fail = !get_maneuver_display(dataState, preply, nman_show, NULL, &pThis->route->tempUint32);
        else
            pThis->route->tempUint32 = numberOfManeuvers;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case tripsettings:
        if (pdata)
            *pdata = (char*)&preply->route_settings;
        if (psize)
            *psize = sizeof(preply->route_settings);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case navigating:
        pThis->route->tempUint32 = !(pThis->route->parameterConfiguration.disableNavigation);
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case aheaddist:
        NB_RouteGetDistanceBetweenManeuvers(pThis->route, closetManeuver, nman_show, &pThis->route->tempDouble);
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case tripremaindist:
        pThis->route->tempDouble = remainingRouteDist;
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case tripremaintime:
        pThis->route->tempDouble = remainingRouteTime;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case stackdist:
        ///@todo (BUG 56048) Need to pass the correct value here.  This value isn't currently used,
        /// but should be fixed.
        pThis->route->tempDouble = remainingRouteDist;
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case trafficincidentdist:
        distToTrafficInfo = pThis->trafficState->distToIncident;
        pThis->route->tempDouble = distToTrafficInfo;
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case trafficcongestiondist:
        distToTrafficInfo = pThis->trafficState->distToCongestion;
        pThis->route->tempDouble = distToTrafficInfo;
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case new_incident_count:
        //@TODO (BUG 56049) traffic
        //pThis->route->tempUint32 = pThis->state.lastupdate_newincidents;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case cleared_incident_count:
        //@TODO (BUG 56049) traffic
        //pThis->route->tempUint32 = pThis->state.lastupdate_clearedincidents;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case travel_time_delta:
        //@TODO (BUG 56049) traffic
        //pThis->route->tempInt32 = pThis->state.lastupdate_traveltimedelta;
        if (pdata)
            *pdata = (char*) &pThis->route->tempInt32;
        if (psize)
            *psize = sizeof(pThis->route->tempInt32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case travel_time:
        pThis->route->tempUint32 = travel_time_val;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case traffic_delay:
        pThis->route->tempUint32 = traffic_delay_time;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case traffic_enabled:
        if (pThis->trafficInfo && pThis->trafficState)
        {
            pThis->route->tempUint32  = 1;
        }
        else
        {
            pThis->route->tempUint32  = 0;
        }
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case ti_road_pronun:
        if (pThis->trafficState->nextIncidentRouteRemain == -1)
            fail = TRUE;
        else
            cache_id = NB_TrafficProcessorGetPronoun(pThis->trafficState, "road-pronun");
        if (cache_id == NULL || !IsItemInVoiceCache(pThis->route->context, cache_id->data, (uint32)cache_id->size))
            fail = TRUE;
        break;
    case ti_origin_proximity:
        t = "at";
        if (pdata)
            *pdata = t;
        if (psize)
            *psize = (nb_size)(nsl_strlen(t)+1);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case ti_origin_pronun:
        if (pThis->trafficState->nextIncidentRouteRemain == -1)
            fail = TRUE;
        else
            cache_id = NB_TrafficProcessorGetPronoun(pThis->trafficState, "origin-pronun");
        if (cache_id == NULL || !IsItemInVoiceCache(pThis->route->context, cache_id->data, (uint32)cache_id->size))
            fail = TRUE;
        break;
    case tc_road_pronun:
        if (pThis->trafficState->nextCongestionRouteRemain == -1)
        {
            fail = TRUE;
        }
        else
        {
            uint32 congestionManeuverIndex = NB_TrafficProcessorGetNextCongestionManeuverIndex(pThis->trafficState);
            if (congestionManeuverIndex != NAV_MANEUVER_NONE)
            {
                congestionManeuver = NB_RouteInformationGetManeuver(pThis->route, congestionManeuverIndex);
                cache_id = &congestionManeuver->current_roadinfo.pronun;
            }
            else
            {
                cache_id = NULL;
            }
        }
        if (cache_id == NULL || !IsItemInVoiceCache(pThis->route->context, cache_id->data, (uint32)cache_id->size))
            fail = TRUE;
        if (cache_id != NULL && !nsl_strempty(pThis->route->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
            fail = FALSE;
        break;
    // The following elements were designed for V4, but never fully implemented.
    case tc_origin_proximity:
        t = "at";
        if (pdata)
            *pdata = t;
        if (psize)
            *psize = (nb_size)(nsl_strlen(t)+1);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case tc_origin_pronun:
        if (pThis->trafficState->nextCongestionRouteRemain == -1)
            fail = TRUE;
        else
            cache_id = NULL;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    // End of incomplete implementation
    case expect_traffic_type:
        if (traffic_meter >= privateConfig->prefs.minTrafficMeterLightTraffic &&
            traffic_meter <  privateConfig->prefs.maxTrafficMeterLightTraffic)
        {
            pThis->route->tempUint32 = (uint32) expect_traffic_light;
        }
        else if (traffic_meter >= privateConfig->prefs.minTrafficMeterModerateTraffic &&
            traffic_meter <  privateConfig->prefs.maxTrafficMeterModerateTraffic)
        {
            pThis->route->tempUint32 = (uint32) expect_traffic_moderate;
        }
        else if (traffic_meter >= privateConfig->prefs.minTrafficMeterSevereTraffic &&
            traffic_meter <  privateConfig->prefs.maxTrafficMeterSevereTraffic)
        {
            pThis->route->tempUint32 = (uint32) expect_traffic_severe;
        }
        else
        {
            pThis->route->tempUint32 = (uint32) expect_traffic_none;
        }

        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case announce_delay:
        if (traffic_meter >= privateConfig->prefs.minTrafficMeterDelayAnnounce &&
            traffic_meter <  privateConfig->prefs.maxTrafficMeterDelayAnnounce)
            pThis->route->tempUint32 = 1;
        else
            pThis->route->tempUint32 = 0;

        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case in_congestion:
        if (pThis->trafficState->nextCongestionIn)
            pThis->route->tempUint32 = 1;
        else
            pThis->route->tempUint32 = 0;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case trafficcongestionlen:
        pThis->route->tempDouble = pThis->trafficState->nextCongestionLength;
        if (pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case speedcameraremaindist:
        if (pdata)
            *pdata = (char*)&pThis->cameraState->distanceToCurrentCamera;
        if (psize)
            *psize = sizeof(pThis->cameraState->distanceToCurrentCamera);
        if (pfreedata)
            *pfreedata = FALSE;
        break;

    case ferryonroute:
        pThis->route->tempUint32 = (uint32)preply->ferry_on_route;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case ccode:
        if (maneuver == NULL )
            fail = TRUE;
        else
        {
            t = data_string_get (dataState, &maneuver->turn_roadinfo.country_info.code);
            if (pdata)
                *pdata = t;
            if (psize)
                *psize = (nb_size)(nsl_strlen(t)+1);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case pedestrian:
        pThis->route->tempUint32 = 1;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
            *pfreedata = FALSE;
        break;
    case significant:
        if (!maneuver_base)
        {
            fail = TRUE;
        }
        else
        {
            pThis->route->tempUint32 = !is_informational(maneuver_base->command);
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case gpspoor:
        {
            NB_GpsHistory *history = NB_ContextGetGpsHistory(pThis->route->context);
            pThis->route->tempUint32 = NB_GpsHistoryIsGpsPoor(history);
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case laneguidance:
        {
            // return false if lane guidance is disabled for turn message
            // otherwise check lane guidance
            uint32* disablingFilter = &pThis->guidanceState->turnInfo.disablingFilter;
            uint32 nullFilter = 0;
            if (pThis->isLookahead)
            {
                disablingFilter = &nullFilter;
            }
            else if (maneuverIndex > closetManeuver)
            {
                disablingFilter = &pThis->guidanceState->stackedDisablingFilter;
            }
            if (((!(*disablingFilter & DISABLE_LANE_GUIDANCE_MASK) && pThis->maneuverPos == NB_NMP_Turn)
                 || pThis->maneuverPos != NB_NMP_Turn)
               && maneuver && maneuver->lane_info.lg_pronun.size > 0)
                pThis->route->tempUint32 = 1;
            else
                pThis->route->tempUint32 = 0;
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case naturalguidance:
        {
            uint32* disablingTurnFilter = &pThis->guidanceState->turnInfo.disablingFilter;
            uint32* disablingPrepareFilter = &pThis->guidanceState->prepareInfo.disablingFilter;
            uint32 nullFilter = 0;
            NB_NavigateManeuverPos pos = pThis->maneuverPos;

            pThis->route->tempUint32 = 0;
            // return false if natural guidance is disabled for prepare or turn message
            // otherwise check guidance points

            if (pThis->isLookahead)
            {
                disablingTurnFilter = &nullFilter;
                disablingPrepareFilter = &nullFilter;
            }
            else if (maneuverIndex > closetManeuver)
            {
                disablingPrepareFilter = &pThis->guidanceState->stackedDisablingFilter;
                pos = NB_NMP_Prepare;
            }
            if ((((!(*disablingPrepareFilter & DISABLE_NATURAL_GUIDANCE_MASK) && pos == NB_NMP_Prepare) ||
                 (!(*disablingTurnFilter & DISABLE_NATURAL_GUIDANCE_MASK) && pos == NB_NMP_Turn) ||
                  (pos != NB_NMP_Prepare && pos != NB_NMP_Turn))) && maneuver)
            {
                data_guidance_point* guidancePoint = GetGuidancePointPointer(maneuver, pos);
                pThis->route->tempUint32 = (guidancePoint != NULL) ? (uint32)guidancePoint->gp_pronun.size : 0;
            }
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case hwyexit:
    {
        if (maneuver && IsItemInVoiceCache(pThis->route->context, maneuver->turn_roadinfo.exit_number.pronun.data, (uint32)maneuver->turn_roadinfo.exit_number.pronun.size))
        {
            pThis->route->tempUint32 = 1;
        }
        else
        {
            pThis->route->tempUint32 = 0;
        }

        // when supported_phonetics_formats is not empty, then return true when exit number data is not null.
        if (pThis->route->routeParameters &&
            !nsl_strempty(pThis->route->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats) &&
            maneuver->turn_roadinfo.exit_number.pronun.size > 0)
        {
            pThis->route->tempUint32 = 1;
        }

        if (pdata && psize)
        {
            *pdata = (char*) &pThis->route->tempUint32;
            *psize = sizeof(pThis->route->tempUint32);
        }
        if (pfreedata)
        {
            *pfreedata = FALSE;
        }
    }
    break;
    case pgppronun:
       if (maneuver == NULL || !(maneuver->guidance_pointer[0].use_for_prepare || maneuver->guidance_pointer[1].use_for_prepare))
           fail = TRUE;
       else
           cache_id = (maneuver->guidance_pointer[0].use_for_prepare ? &maneuver->guidance_pointer[0].gp_pronun : &maneuver->guidance_pointer[1].gp_pronun);
       if (cache_id == NULL)
           fail = TRUE;
       break;
    case pgppreppronun:
        if (maneuver == NULL || !(maneuver->guidance_pointer[0].use_for_prepare || maneuver->guidance_pointer[1].use_for_prepare))
            fail = TRUE;
        else
            cache_id = (maneuver->guidance_pointer[0].use_for_prepare ? &maneuver->guidance_pointer[0].gpprep_pronun : &maneuver->guidance_pointer[1].gpprep_pronun);
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case tgppronun:
        if (maneuver == NULL || !(maneuver->guidance_pointer[0].use_for_turn || maneuver->guidance_pointer[1].use_for_turn))
            fail = TRUE;
        else
            cache_id = (maneuver->guidance_pointer[0].use_for_turn ? &maneuver->guidance_pointer[0].gp_pronun : &maneuver->guidance_pointer[1].gp_pronun);
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case tgppreppronun:
        if (maneuver == NULL || !(maneuver->guidance_pointer[0].use_for_turn || maneuver->guidance_pointer[1].use_for_turn))
            fail = TRUE;
        else
            cache_id = (maneuver->guidance_pointer[0].use_for_turn ? &maneuver->guidance_pointer[0].gpprep_pronun : &maneuver->guidance_pointer[1].gpprep_pronun);
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case hwyexitpronun:
        if (maneuver == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver->turn_roadinfo.exit_number.pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case lgpronun:
        if (maneuver == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver->lane_info.lg_pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case lgpreppronun:
        if (maneuver == NULL)
            fail = TRUE;
        else
            cache_id = &maneuver->lane_info.lgprep_pronun;
        if (cache_id == NULL)
            fail = TRUE;
        break;
    case speed:
        if (pThis->navState == NULL)
            fail = TRUE;
        else
            pThis->route->tempDouble = pThis->navState->currentSpeed;

        if (!fail && pdata)
            *pdata = (char*) &pThis->route->tempDouble;
        if (!fail && psize)
            *psize = sizeof(pThis->route->tempDouble);
        if (!fail && pfreedata)
            *pfreedata = FALSE;
        break;
    case dpronunavailable:
    case stackdpronunavailable:
        {
            nb_boolean found = FALSE;
            uint32* disablingFilter = &pThis->guidanceState->turnInfo.disablingFilter;
            uint32 nullFilter = 0;
            if (pThis->isLookahead)
            {
                disablingFilter = &nullFilter;
            }
            else if (maneuverIndex > closetManeuver)
            {
                disablingFilter = &pThis->guidanceState->stackedDisablingFilter;
            }

            // return false for short turn message
            if ((((!(*disablingFilter & DISABLE_DESTINATION_MASK) && elem == dpronunavailable) ||
                 (!(*disablingFilter & DISABLE_STACKED_DESTINATION_MASK) && elem == stackdpronunavailable))
                && pThis->maneuverPos == NB_NMP_Turn)
                || pThis->maneuverPos != NB_NMP_Turn)
            {
                if (maneuver)
                {
                    found = IsItemInVoiceCache(pThis->route->context, (byte*)maneuver->turn_roadinfo.pronun.data, (uint32)maneuver->turn_roadinfo.pronun.size);
                }
                if (!found && maneuver_base)
                {
                    found = IsItemInVoiceCache(pThis->route->context, (byte*)maneuver_base->turn_roadinfo.pronun.data, (uint32)maneuver_base->turn_roadinfo.pronun.size);
                }

                // when supported_phonetics_formats is not empty, set the found flag to true.
                if (!found && pThis->route->routeParameters &&
                    !nsl_strempty(pThis->route->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
                {
                    found = TRUE;
                }
            }

            pThis->route->tempUint32 = found ? 1 : 0;
            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
            {
                *pfreedata = FALSE;
            }
            break;
        }
    case prepareplayed:
        {
            pThis->route->tempUint32 = (pThis->guidanceState->prepareInfo.state == gmsPlayed) ? 1 : 0;
            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case continueplayed:
        {
            pThis->route->tempUint32 = (pThis->guidanceState->continueInfo.state == gmsPlayed) ? 1 : 0;
            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case guidancepointtype:
        {
            data_guidance_point* guidancePoint = GetGuidancePointPointer(maneuver, pThis->maneuverPos);

            pThis->route->tempUint32 = guidancePoint != NULL ? guidancePoint->gptype : 0;

            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case buttonpressed:
        {
            pThis->route->tempUint32 = pThis->buttonPressed ? 1 : 0;
            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case removestack:
        {
            uint32* disablingFilter = &pThis->guidanceState->turnInfo.disablingFilter;
            uint32 nullFilter = 0;
            if (pThis->isLookahead)
            {
                disablingFilter = &nullFilter;
            }
            else if (maneuverIndex > closetManeuver)
            {
                disablingFilter = &pThis->guidanceState->stackedDisablingFilter;
            }
            pThis->route->tempUint32 = ((*disablingFilter & DISABLE_STACKED_INSTRUCTION_MASK) && pThis->maneuverPos == NB_NMP_Turn) ? 1 : 0;
            if (pdata && psize)
            {
                *pdata = (char*) &pThis->route->tempUint32;
                *psize = sizeof(pThis->route->tempUint32);
            }
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case stackmaneuverindex:
            if (maneuver == NULL || maneuver_stack == NULL)
                fail = TRUE;
            else
                pThis->route->tempUint32 = dManeuver_stack;

            if (!fail)
            {
                if (pdata)
                    *pdata = (char*) &pThis->route->tempUint32;
                if (psize)
                    *psize = sizeof(pThis->route->tempUint32);
            }

            if (pfreedata)
                *pfreedata = FALSE;
            break;
    case nextmaneuverindex:
            if (maneuver == NULL || maneuver_next == NULL)
                fail = TRUE;
            else
            {
                if ((maneuverIndex == NAV_MANEUVER_INITIAL) &&
                    (get_maneuver_type(maneuver->command) != maneuver_type_hwy &&
                     (pThis->maneuverPos == NB_NMP_Continue || pThis->maneuverPos == NB_NMP_Prepare) &&
                     NB_RouteInformationGetManeuverCount(pThis->route) > 1 &&
                     pThis->guidanceState && !pThis->guidanceState->hasFirstManeuverPlayed))
                {
                    pThis->route->tempUint32 = dManeuver;
                }
                else
                {
                    pThis->route->tempUint32 = dManeuver_next;
                }
            }

            if (!fail)
            {
                if (pdata)
                    *pdata = (char*) &pThis->route->tempUint32;
                if (psize)
                    *psize = sizeof(pThis->route->tempUint32);
            }

            if (pfreedata)
                *pfreedata = FALSE;
            break;
    case noguidance:
        if (!maneuver_next)
        {
            fail = TRUE;
        }
        else
        {
            pThis->route->tempUint32 = maneuver_next->disable_guidance == TRUE ? 1 : 0;
            if (pdata)
                *pdata = (char*) &pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
        }
        break;
    case ttsenabled:
        pThis->route->tempUint32 = pThis->route->routeParameters &&
                                   nsl_strempty(pThis->route->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats) ?
                                   0 : 1;
        if (pdata)
            *pdata = (char*) &pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case fmaneuverheading:
        {
            double averageHeading    = 0;
            if (maneuver->polyline.length < 3)
            {
                fail = TRUE;
                break;
            }
            averageHeading = maneuver->polyline.segments[0].heading;
            averageHeading = get_average_heading(averageHeading, maneuver->polyline.segments[1].heading);
            averageHeading = get_average_heading(averageHeading, maneuver->polyline.segments[2].heading);

            if (pdata)
                *pdata = GetHeadingName(averageHeading);
            if (psize && pdata)
                *psize = (nb_size)(nsl_strlen(*pdata));
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case fmaneuvercrossstreet:
        {
            data_cross_street* crossStreet = NULL;
            double nowProgress = maneuver->distance - pThis->currentRoutePosition->maneuverDistanceRemaining;
            int32 crossStreetIndex = 0;
            int32 crossStreetCount = CSL_VectorGetLength(maneuver->vec_cross_streets);
            if (crossStreetCount <= 0)
            {
                if (pdata)
                {
                    if(maneuver->turn_roadinfo.pronun.size)
                    {
                        *pdata = (char*)maneuver->turn_roadinfo.pronun.data;
                    }
                    if (psize)
                        *psize = maneuver->turn_roadinfo.pronun.size;
                    if (pfreedata)
                        *pfreedata = FALSE;
                }
                else
                {
                    fail = TRUE;
                }
                break;
            }

            for (; crossStreetIndex < crossStreetCount; crossStreetIndex++)
            {
                crossStreet = (data_cross_street*)CSL_VectorGetPointer(maneuver->vec_cross_streets, crossStreetIndex);
                if (crossStreet && nowProgress < crossStreet->progress)
                {
                    break;
                }
            }

            if (crossStreetIndex == crossStreetCount)
            {
                fail = TRUE;
                break;
            }

            if (pdata)
            {
                if (crossStreet->pronun)
                {
                    *pdata = data_string_get(dataState, &crossStreet->pronun);
                }
                else
                {
                    *pdata = data_string_get(dataState, &crossStreet->name);
                }
                if (psize)
                    *psize = (nb_size)(nsl_strlen(*pdata));
                if (pfreedata)
                    *pfreedata = FALSE;
            }
            break;
        }
    case highway:
        pThis->route->tempUint32 = get_maneuver_type(maneuver->command) == maneuver_type_hwy ? 1 : 0;
        if (pdata)
            *pdata = (char*)&pThis->route->tempUint32;
        if (psize)
            *psize = sizeof(pThis->route->tempUint32);
        if (pfreedata)
            *pfreedata = FALSE;
        break;
    case routestartheading:
        {
            double headingToStart = 0;
            double closestLat = 0;
            double closestLon = 0;
            if (!pThis->currentRoutePosition || !pThis->route || !maneuver)
            {
                fail = TRUE;
                break;
            }
            if(pThis->currentRoutePosition->closestManeuver == 0 && maneuver->polyline.numsegments > pThis->currentRoutePosition->closestSegment)
            {
                data_polyline_get(dataState, &maneuver->polyline, pThis->currentRoutePosition->closestSegment, &closestLat, &closestLon, NULL, NULL);
            }
            else
            {
                data_polyline_get(dataState, &maneuver->polyline, 0, &closestLat, &closestLon, NULL, NULL);
            }
            NB_SpatialGetLineOfSightDistance(pThis->currentRoutePosition->projLat, pThis->currentRoutePosition->projLon,
                                             closestLat, closestLon,
                                             &headingToStart);
            if (pdata)
                *pdata = GetHeadingName(headingToStart);
            if (psize && pdata)
                *psize = (nb_size)(nsl_strlen(*pdata));
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case directiontoroute:
        {
            if (!pThis->currentRoutePosition || !pThis->route)
            {
                fail = TRUE;
                break;
            }

            if (pdata)
                NB_RouteInformationGetDirectionToRoute(pThis->route, pThis->currentRoutePosition->projLat, pThis->currentRoutePosition->projLon, pdata, NULL);
            if (psize && pdata)
                *psize = (nb_size)(nsl_strlen(*pdata));
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case ifdirectiontoroute:
        {
            const char* directionToRoute = NULL;
            NB_RouteInformationGetDirectionToRoute(pThis->route, pThis->currentRoutePosition->projLat, pThis->currentRoutePosition->projLon, &directionToRoute, NULL);
            pThis->route->tempUint32 = nsl_strlen(directionToRoute) > 0;
            if (pdata)
                *pdata = (char*)&pThis->route->tempUint32;
            if (psize)
                *psize = sizeof(pThis->route->tempUint32);
            if (pfreedata)
                *pfreedata = FALSE;
            break;
        }
    case fmrpronun:
        {
            if (pThis->route->dataReply.first_major_road == NULL)
                fail = TRUE;
            else
                cache_id = &pThis->route->dataReply.first_major_road->pronun;
            if (!cache_id || cache_id->size == 0)
            {
                data_nav_maneuver* firstManeuver  = NULL;
                firstManeuver =  data_nav_reply_get_nav_maneuver(dataState, &pThis->route->dataReply, 0);
                if (firstManeuver)
                {
                    cache_id = &firstManeuver->current_roadinfo.pronun;
                    if (!cache_id || cache_id->size == 0)
                    {
                        fail = TRUE;
                        break;
                    }
                }
                else
                {
                    fail = TRUE;
                    break;
                }
            }
            if (!IsItemInVoiceCache(pThis->route->context, cache_id->data, (uint32)cache_id->size))
                fail = TRUE;
            if (!nsl_strempty(pThis->route->routeParameters->dataQuery.want_extended_pronun_data.supported_phonetics_formats))
                fail = FALSE;
            break;
        }
    case longmaneuver:
        {
            if (maneuver == NULL)
            {
                fail = TRUE;
            }
            else
            {
                pThis->route->tempUint32 = maneuver->distance > LONG_MANEUVER_LIMIT;
                if (pdata)
                    *pdata = (char*)&pThis->route->tempUint32;
                if (psize)
                    *psize = sizeof(pThis->route->tempUint32);
                if (pfreedata)
                    *pfreedata = FALSE;
            }
            break;
        }
    case stackadvise:
        {
            if (maneuver == NULL)
            {
                fail = TRUE;
            }
            else
            {
                pThis->route->tempUint32 = maneuver->stack_advise;
                if (pdata)
                    *pdata = (char*)&pThis->route->tempUint32;
                if (psize)
                    *psize = sizeof(pThis->route->tempUint32);
                if (pfreedata)
                    *pfreedata = FALSE;
            }
            break;
        }
    default:
        fail = TRUE;
        break;
    }

exit_fail:

    if (fail)
    {
        if (pdata)
            *pdata = NULL;
        if (psize)
            *psize = 0;
        if (pfreedata)
            *pfreedata = FALSE;
    }
    else if (returntmp)
    {
        l = (nb_size)(nsl_strlen(tmp));

        if (pdata)
        {
            *pdata = p = nsl_malloc(l+1);

            if (p != NULL)
            {
                nsl_strlcpy(tmp, p, l+1);
                p[l] = 0;
                *psize = l+1;
                *pfreedata = TRUE;

                return TRUE;
            }
            else {
                *psize = 0;
                *pfreedata = FALSE;
            }
        }
        else
        {
            if (psize)
                *psize = l+1;
            if (pfreedata)
                *pfreedata = TRUE;
        }
    }
    else if (cache_id != NULL)
    {
        if (pdata)
        {
            *pdata = (const char*) cache_id->data;
        }

        if (psize)
        {
            *psize = (nb_size)cache_id->size;
        }

        if (pfreedata)
        {
            *pfreedata = FALSE;
        }

        return cache_id->data != NULL ? TRUE : FALSE;
    }
    else
        return TRUE;

    return FALSE;
}

void QaLogReply(NB_Context* context, NB_RouteInformation* route)
{
    NB_LatitudeLongitude origin;
    NB_LatitudeLongitude destination;

    origin.latitude = route->originFix.latitude;
    origin.longitude = route->originFix.longitude;

    destination.latitude = route->destination.location.latitude;
    destination.longitude = route->destination.location.longitude;

    NB_QaLogRouteReply(context, &origin, &destination, route->routeID.data, route->routeID.size, NB_RouteInformationIsRouteComplete(route));
}

void QaLogRouteTrafficRegion(NB_Context* context, NB_RouteInformation* route)
{
    uint32 maneuverCount = 0;
    uint32 maneuver = 0;
    data_util_state* dataState = NULL;
    data_nav_reply* reply = NULL;

    if (!context || !route || !CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context)))
        return;

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    for (maneuver = 0; maneuver < maneuverCount; maneuver++)
    {
        data_nav_maneuver* pman = data_nav_reply_get_nav_maneuver(dataState, reply, maneuver);
        int trafficRegionCount = CSL_VectorGetLength(pman->vec_traffic_regions);
        int i=0;

        for (i=0; i < trafficRegionCount; i++)
        {
            data_traffic_region* ptr = CSL_VectorGetPointer(pman->vec_traffic_regions, i);
            if (ptr != NULL)
            {
                NB_QaLogRouteTrafficRegion(route->context, maneuver, ptr->start, ptr->length, data_string_get(dataState, &ptr->location));
            }
        }
    }
}

void QaLogRouteRealisticSigns(NB_Context* context, NB_RouteInformation* route)
{
    uint32 maneuverCount = 0;
    uint32 maneuver = 0;

    if (!context || !route || !NB_ContextGetQaLog(context)  ||
        !CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context)))
    {
        return;
    }

    maneuverCount = NB_RouteInformationGetManeuverCount(route);

    for (maneuver = 0; maneuver < maneuverCount; maneuver++)
    {
        uint32 signCount = 0;
        uint32 sign = 0;

        signCount = NB_RouteInformationGetManeuverRealisticSignCount(route, maneuver);

        for (sign = 0; sign < signCount; sign++)
        {
            NB_RouteRealisticSign realisticSign = { 0 };

            if (NB_RouteInformationGetManeuverRealisticSign(route, maneuver, sign, &realisticSign) == NE_OK)
            {
                NB_QaLogRealisticSignInfo(
                    context,
                    realisticSign.signId,
                    realisticSign.signFileType,
                    realisticSign.datasetId,
                    realisticSign.version,
                    maneuver);
            }
        }
    }
}

void QaLogRouteEnhancedContentInfo(NB_Context* context, NB_RouteInformation* route)
{
    uint32 regionCount = 0;
    uint32 regionIndex = 0;

    if (!context || !route || !NB_ContextGetQaLog(context) ||
        !CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(context)))
    {
        return;
    }

    regionCount =  NB_RouteInformationGetContentRegionCount(route);
    for (regionIndex = 0; regionIndex < regionCount; regionIndex++)
    {
        NB_RouteContentRegion region = { 0 };

        if (NB_RouteInformationGetContentRegion(route, regionIndex, &region) == NE_OK)
        {
            // Convert enums to QA log constants
            NB_QaLogEnhancedContentType contentRegionType = NB_QLECT_Undefined;
            NB_QaLogProjectionType projectionType = NB_QLPT_Undefined;
            NB_QaLogProjectionDatum projectionDatum = NB_QLPD_Undefined;

            switch (region.type)
            {
            case NB_RCRT_JunctionModel:
                contentRegionType = NB_QLECT_Junctions;
                break;
            case NB_RCRT_CityModel:
                contentRegionType = NB_QLECT_CityModels;
                break;
            case NB_RCRT_Invalid:
                contentRegionType = NB_QLECT_Undefined;
                break;
            }

            switch (region.projection)
            {
            case NB_ECPT_Mercator:
                projectionType = NB_QLPT_Mercator;
                break;
            case NB_ECPT_TransverseMercator:
                projectionType = NB_QLPT_TransverseMercator;
                break;
            case NB_ECPT_UniversalTransverseMercator:
                projectionType = NB_QLPT_UniversalTransverseMercator;
                break;
            case NB_ECPT_Invalid:
                projectionType = NB_QLPT_Undefined;
                break;
            }

            switch (region.datum)
            {
            case NB_ECPD_WGS84:
                projectionDatum = NB_QLPD_WGS84;
                break;
            case NB_ECPD_Invalid:
                projectionDatum = NB_QLPD_Undefined;
                break;
            case NB_ECPD_DNorthAmerican1983:
                projectionDatum = NB_QLPD_NAD83;
                break;
            }

            NB_QaLogEnhancedContentInfo(context, region.regionId, contentRegionType, region.version, region.datasetId,
                region.startManeuver, region.startManeuverOffset, region.endManeuver, region.endManeuverOffset,
                projectionType, projectionDatum, region.falseEasting, region.falseNorthing,
                region.originLatitude, region.originLongitude, region.scaleFactor, region.zOffset);
        }
    }
}

void NB_RouteInformationGetRouteStart(NB_RouteInformation* pThis, double* latitude, double* longitude)
{
    data_util_state* dataState = NB_ContextGetDataState(pThis->context);
    data_nav_maneuver* maneuver = data_nav_reply_get_nav_maneuver(dataState, &pThis->dataReply, 0);
    if (maneuver)
    {
        data_polyline_get(dataState, &maneuver->polyline, 0, latitude, longitude, NULL, NULL);
    }
}

NB_Error NB_RouteInformationGetPlayTimeForMessageCode(NB_RouteInformation* route, const char* code, uint32* time)
{
    data_file* file = NULL;
    int i = 0;
    int len = 0;
    NB_Error err = NE_NOENT;

    if (!route || !code || !time)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(route->dataReply.vec_file);

    for(; i < len; i++)
    {
        char* file_name = NULL;
        file = CSL_VectorGetPointer(route->dataReply.vec_file, i);

        file_name = (char*)nsl_malloc(file->name.size + 1);
        if (!file_name)
        {
            return NE_NOMEM;
        }

        nsl_strlcpy(file_name, (char*)file->name.data, file->name.size + 1);

        if (nsl_strcmp(file_name, code) == 0 &&
            file->play_time.play_time != 0)
        {
            *time = file->play_time.play_time;
            err = NE_OK;
        }

        nsl_free(file_name);

        if (err == NE_OK)
        {
            return err;
        }
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetExitNumber(NB_RouteInformation* route,
                                 uint32 index,
                                 NB_LatitudeLongitude* turnPoint,
                                 char* exitNumber,
                                 uint32 exitNumberBufferSize)
{
    NB_Error err = NE_NOENT;
    data_nav_maneuver* pmaneuver = NULL;
    data_nav_reply* reply = NULL;
    data_util_state* dataState = NULL;

    if (!route || !exitNumber || exitNumberBufferSize == 0)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (pmaneuver)
    {
        if (NB_RouteParametersGetCommandType(route->routeParameters, data_string_get(dataState, &pmaneuver->command)) == maneuver_type_hwy)
        {
            if (exitNumber != NULL && nsl_strlen(data_string_get(dataState, &pmaneuver->turn_roadinfo.exit_number.number)) > 0)
            {
                data_string_get_copy(dataState, &pmaneuver->turn_roadinfo.exit_number.number, exitNumber, exitNumberBufferSize);
                err = NE_OK;
            }
        }
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetRoutesCount(NB_RouteInformation* route, uint32* routesCount)
{
    NB_Error  err = NE_OK;

    if( route == NULL || routesCount == NULL )
    {
        return NE_INVAL;
    }

    *routesCount = route->numberOfSelector;
    return err;
}

NB_DEF NB_Error
NB_RouteInformationSetActiveRoute(NB_RouteInformation* route, uint32 activeRoute)
{   // public activeRoute starting from 1
    NB_Error err = NE_OK;
    uint32   currentRoute;

    if( !route )
    {
        return NE_INVAL;
    }

    if( activeRoute < 1 || activeRoute > route->numberOfSelector )
    {
        return NE_INVAL;
    }

    err = data_nav_reply_get_active(NB_ContextGetDataState(route->context), &route->dataReply, &currentRoute);
    if( err == NE_OK && currentRoute < route->numberOfSelector && (currentRoute + 1) == activeRoute)
    {
        return err;
    }

    // protocol implementation index starting from 0
    err = data_nav_reply_select_active(NB_ContextGetDataState(route->context), &route->dataReply, activeRoute-1);
    if( err == NE_OK )
    {
        route->routeLength  = GetRouteLength(route);
        GetRouteTimeAndDelay(route, &route->routeTime, &route->routeDelay);
        route->routeID.size = (nb_size)route->dataReply.route_id.size;
        route->routeID.data = route->dataReply.route_id.data;
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationOnRouteInfo(NB_RouteInformation* route, NB_OnRouteInformation* onRouteInfo)
{
    NB_Error err = NE_OK;
    data_on_route_information dataOnRouteInfo;

    if( !route || !onRouteInfo )
    {
        return NE_INVAL;
    }

    err = data_nav_reply_get_on_route_info(NB_ContextGetDataState(route->context), &route->dataReply, &dataOnRouteInfo);
    onRouteInfo->isHighWayOnRoute  = dataOnRouteInfo.isHighWayOnRoute;
    onRouteInfo->isHovLanesOnRoute = dataOnRouteInfo.isHovLanesOnRoute;
    onRouteInfo->isTollsOnRoute    = dataOnRouteInfo.isTollsOnRoute;
    onRouteInfo->isUnpavedOnRoute  = dataOnRouteInfo.isUnpavedOnRoute;
    onRouteInfo->isFerryOnRoute    = dataOnRouteInfo.isFerryOnRoute;
    onRouteInfo->isPrivateRoadOnRoute = dataOnRouteInfo.isPrivateRoadOnRoute;
    onRouteInfo->isEnterCoutryOnRoute = dataOnRouteInfo.isEnterCoutryOnRoute;
    onRouteInfo->isGatedAccessOnRoute = dataOnRouteInfo.isGatedAccessOnRoute;

    if (route->dataReply.active_route == SINGLE_ROUTE)
    {
        UpdateOnRouteForTraffic(route->context,
                                onRouteInfo,
                                route->dataReply.traffic_incidents.vec_place,
                                route->dataReply.traffic_record_identifier.new_realtime_traffic_flow.vec_traffic_flow_items);
    }
    else
    {
        data_nav_route* current_route = CSL_VectorGetPointer(route->dataReply.route_selector_reply.vec_routes,
                                                             route->dataReply.active_route);
        UpdateOnRouteForTraffic(route->context,
                                onRouteInfo,
                                current_route->traffic_incidents.vec_place,
                                current_route->traffic_record_identifier.new_realtime_traffic_flow.vec_traffic_flow_items);
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetRouteStyle(NB_RouteInformation* route, NB_RouteSettings* routeStyle)
{
    if( !route || !routeStyle )
    {
        return NE_INVAL;
    }

    return NB_RouteInformationCopyRouteSettings(routeStyle, &route->dataReply.route_settings);
}

NB_DEF NB_Error
NB_RouteInformationGetLabelPoint(NB_RouteInformation* route, NB_LatitudeLongitude* latLong)
{
    NB_Error   err = NE_OK;
    data_point labelPoint;

    if (!route || !latLong)
    {
        return NE_INVAL;
    }

    err = data_nav_reply_get_label_point(NB_ContextGetDataState(route->context), &route->dataReply, &labelPoint);
    if (err == NE_OK)
    {
        latLong->latitude  = labelPoint.lat;
        latLong->longitude = labelPoint.lon;
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetDetourCurrentLabelPoint(NB_RouteInformation* route, NB_LatitudeLongitude* latLong)
{
    NB_Error   err = NE_OK;
    data_point labelPoint;

    if (!route || !latLong)
    {
        return NE_INVAL;
    }

    err = data_nav_reply_get_current_label_point(NB_ContextGetDataState(route->context), &route->dataReply, &labelPoint);
    if (err == NE_OK)
    {
        latLong->latitude  = labelPoint.lat;
        latLong->longitude = labelPoint.lon;
    }

    return err;
}

NB_DEF NB_Error
NB_RouteInformationGetLaneCharacters(NB_RouteInformation* route,
                                     uint32 index,
                                     NB_LatitudeLongitude* turnPoint,
                                     NB_Lane* LaneCharacters,
                                     uint32 LaneCountSize)
{
    NB_Error err = NE_OK;
    data_nav_maneuver* pmaneuver = NULL;
    data_nav_reply* reply = NULL;
    data_util_state* dataState = NULL;
    data_lane_info* laneInfo = NULL;
    uint32 i = 0;
    uint32 laneCount = 0;
    if (!route || !LaneCharacters)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    pmaneuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (!pmaneuver)
    {
        return NE_NOENT;
    }

    laneInfo = &pmaneuver->lane_info;
    laneCount = CSL_VectorGetLength(laneInfo->vec_lane_guidance_items);

    if (laneCount == 0)
    {
        return NE_NOENT;
    }

    if (laneCount > LaneCountSize)
    {
        return NE_NOMEM;
    }

    for (i = 0; i<laneCount; i++)
    {
        data_lane_guidance_item * item = CSL_VectorGetPointer(laneInfo->vec_lane_guidance_items, i);
        if (!item)
        {
            return NE_NOMEM;
        }
        GetLaneCharacters(route->routeParameters,&LaneCharacters[i], item);
    }

    return err;
}

static NB_Error
GetGuidancePointInformation(NB_RouteInformation* route,
                            uint32 index,
                            NB_GuidancePointType* pType,
                            NB_LatitudeLongitude* pGuidancePoint,
                            double* pManeuverPointOffset,
                            nb_boolean prepare)
{
    data_guidance_point* guidancePoint = NULL;
    data_nav_maneuver* pManeuver = NULL;
    data_nav_reply* reply = NULL;
    data_util_state* dataState = NULL;

    if( !route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    reply = &route->dataReply;

    if (index == NAV_MANEUVER_INITIAL)
    {
        index = 0;
    }
    pManeuver = data_nav_reply_get_nav_maneuver(dataState, reply, index);

    if (!pManeuver)
    {
        return NE_NOENT;
    }

    guidancePoint = &pManeuver->guidance_pointer[0];
    if ((NB_GuidancePointType)guidancePoint->gptype == NBGPT_Invalid)
    {
        return NE_NOENT;
    }

    if (prepare)
    {
        if (!guidancePoint->use_for_prepare)
        {
            guidancePoint = &pManeuver->guidance_pointer[1];
            if (!guidancePoint->use_for_prepare || (NB_GuidancePointType)guidancePoint->gptype == NBGPT_Invalid)
            {
                return NE_NOENT;
            }

        }
    }
    else
    {
        if (!guidancePoint->use_for_turn)
        {
            guidancePoint = &pManeuver->guidance_pointer[1];
            if (!guidancePoint->use_for_turn || (NB_GuidancePointType)guidancePoint->gptype == NBGPT_Invalid)
            {
                return NE_NOENT;
            }
        }
    }

    if (pType)
    {
        *pType = (NB_GuidancePointType)guidancePoint->gptype;
    }
    if (pGuidancePoint)
    {
        pGuidancePoint->latitude = guidancePoint->point.lat;
        pGuidancePoint->longitude = guidancePoint->point.lon;
    }
    if (pManeuverPointOffset)
    {
        *pManeuverPointOffset = guidancePoint->maneuver_point_offset;
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetPrepareGuidancePointInformation(NB_RouteInformation* route,
                                                      uint32 index,
                                                      NB_GuidancePointType* pType,
                                                      NB_LatitudeLongitude* pGuidancePoint,
                                                      double* pManeuverPointOffset)
{
    return GetGuidancePointInformation(route, index, pType, pGuidancePoint, pManeuverPointOffset, TRUE);
}

NB_DEF NB_Error
NB_RouteInformationGetTurnGuidancePointInformation(NB_RouteInformation* route,
                                                   uint32 index,
                                                   NB_GuidancePointType* pType,
                                                   NB_LatitudeLongitude* pGuidancePoint,
                                                   double* pManeuverPointOffset)
{
    return GetGuidancePointInformation(route, index, pType, pGuidancePoint, pManeuverPointOffset, FALSE);
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficInformation(NB_RouteInformation* route, NB_TrafficInformation** information)
{
    if (!route || !information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(route->context);

    return NB_TrafficInformationCreateFromRoute(route->context, route, information);
}

NB_Error
NB_RouteInformationGetDistancesForStreetCountGuidancePoint(NB_RouteInformation* route, uint32 index, double* startDistance, double* endDistance)
{
    NB_GuidancePointType gpType = NBGPT_Invalid;
    double gpOffset = 0.0;
    double maxInstructionDistance = 0.0;

    if (!route || !startDistance || !endDistance)
    {
        return NE_INVAL;
    }

    if (NE_OK != NB_RouteInformationGetMaxTurnDistanceForStreetCount(route, index, &maxInstructionDistance))
    {
        return NE_NOENT;
    }

    if (NE_OK != NB_RouteInformationGetTurnGuidancePointInformation(route, index, &gpType, NULL, &gpOffset) ||
        gpType != NBGPT_StreetCount)
    {
        return NE_NOENT;
    }

    *startDistance = maxInstructionDistance;
    *endDistance = gpOffset;

    return NE_OK;
}

NB_DEF NB_Error NB_RouteInformationGetPronunInformation(NB_RouteInformation* routeInformation, const char *key,
                                                        char* textBuffer, uint32* textBufferSize,
                                                        byte* phoneticsData, uint32* phoneticsDataSize,
                                                        char* language, uint32* languageDataSize,
                                                        char* translatedText, uint32* translatedTextDataSize,
                                                        double* duration)
{
    data_pronun_extended* pronunInfoItem = NULL;
    int totalPronunListItems = 0;
    int i = 0;

    if (!routeInformation || !key || (!textBuffer && !textBufferSize && (*textBufferSize) == 0
        && !duration && !phoneticsData && !phoneticsDataSize && (*phoneticsDataSize) == 0))
    {
        return NE_INVAL;
    }

    // Get pronun information from nav-reply for street voices
    if (routeInformation->dataReply.active_route == SINGLE_ROUTE)
    {
        totalPronunListItems = CSL_VectorGetLength(routeInformation->dataReply.pronun_list.vec_pronun_extended);
    }
    else
    {
        totalPronunListItems = CSL_VectorGetLength(routeInformation->dataReply.route_selector_reply.pronun_list.vec_pronun_extended);
    }

    for (i = 0; i < totalPronunListItems; ++i)
    {
        if (routeInformation->dataReply.active_route == SINGLE_ROUTE)
        {
            pronunInfoItem = (data_pronun_extended*)CSL_VectorGetPointer(routeInformation->dataReply.pronun_list.vec_pronun_extended, i);
        }
        else
        {
            pronunInfoItem = (data_pronun_extended*)CSL_VectorGetPointer(routeInformation->dataReply.route_selector_reply.pronun_list.vec_pronun_extended, i);
        }

        if (pronunInfoItem && pronunInfoItem->key && (nsl_strcmp(pronunInfoItem->key, key) == 0))
        {
            uint32 textLen = (uint32)nsl_strlen(pronunInfoItem->text);
            if (textBuffer && textBufferSize && (*textBufferSize) >= textLen && textLen > 0)
            {
                nsl_strlcpy(textBuffer, pronunInfoItem->text, (*textBufferSize));
                (*textBufferSize) = (uint32)(nsl_strlen(pronunInfoItem->text) + 1);
            }
            else if (textBufferSize)
            {
                (*textBufferSize) = 0;
            }

            if (phoneticsData && phoneticsDataSize &&
                ((*phoneticsDataSize) >= pronunInfoItem->phonetics.size)
                && pronunInfoItem->phonetics.size > 0)
            {
                nsl_memcpy(phoneticsData, pronunInfoItem->phonetics.data, pronunInfoItem->phonetics.size);
                (*phoneticsDataSize) = (uint32)pronunInfoItem->phonetics.size;
            }
            else if (phoneticsDataSize)
            {
                (*phoneticsDataSize) = 0;
            }

            if (languageDataSize && language &&pronunInfoItem->language&& (*languageDataSize) >= (uint32)(nsl_strlen(pronunInfoItem->language)+1))
            {
                *languageDataSize = (uint32)(nsl_strlen(pronunInfoItem->language)+1);
                nsl_strlcpy(language, pronunInfoItem->language, *languageDataSize);
            }

            if (translatedTextDataSize && translatedText && pronunInfoItem->transliterated_text && (*translatedTextDataSize) >= (uint32)(nsl_strlen(pronunInfoItem->transliterated_text)+1))
            {
                *translatedTextDataSize = (uint32)(nsl_strlen(pronunInfoItem->transliterated_text)+1);
                nsl_strlcpy(translatedText, pronunInfoItem->transliterated_text, *translatedTextDataSize);
            }

            if (duration)
            {
                // TODO: Clarify requirements for playTime. Should server send playtime as double instead of int?
                *duration = pronunInfoItem->playtime;
            }
            return NE_OK;
        }
    }

    return NE_NOENT;
}

NB_DEF NB_Error NB_RouteInformationGetMaxTurnDistanceForStreetCount(NB_RouteInformation* routeInformation, uint32 index, double* maxTurnDistance)
{
    data_nav_maneuver* pManeuver = NULL;
    NB_GuidancePointType gpType = NBGPT_Invalid;

    if (!routeInformation || !maxTurnDistance)
    {
        return NE_INVAL;
    }

    pManeuver = NB_RouteInformationGetManeuver(routeInformation, index);

    if (!pManeuver)
    {
        return NE_NOENT;
    }

    if (NE_OK != NB_RouteInformationGetTurnGuidancePointInformation(routeInformation, index, &gpType, NULL, NULL) ||
        gpType != NBGPT_StreetCount)
    {
        return NE_NOENT;
    }

    *maxTurnDistance = pManeuver->max_turn_distance_for_street_count.max_turn_distance;

    return NE_OK;
}


NB_DEF NB_Error NB_RouteInformationGetSingleRouteInformation(NB_RouteInformation* routeInformation,
                                                             uint32 index,
                                                             NB_RouteInformation** singleRouteInformation)
{
    NB_RouteInformation* singleRoute = NULL;
    NB_Error err = NE_OK;

    if(!routeInformation || !singleRouteInformation)
    {
        return NE_INVAL;
    }

    if(index <= 0 || index > routeInformation->numberOfSelector)
    {
        return NE_INVAL;
    }

    singleRoute = NB_RouteInformationAlloc(routeInformation->context);

    if (singleRoute == NULL)
    {
        return NE_NOMEM;
    }

    if (NE_OK == data_nav_reply_get_single_nav_reply(NB_ContextGetDataState(routeInformation->context),
                     &routeInformation->dataReply,
                     index - 1,
                     &singleRoute->dataReply))
    {
        singleRoute->camerasOnRoute = routeInformation->camerasOnRoute;
        singleRoute->hasUnpavedRoads = routeInformation->hasUnpavedRoads;
        singleRoute->transportMode = routeInformation->transportMode;
        singleRoute->collapseInformational = routeInformation->collapseInformational;
        singleRoute->useFetchCorridor = routeInformation->useFetchCorridor;
        nsl_memcpy(&singleRoute->start, &routeInformation->start, sizeof(NB_Place));
        nsl_memcpy(&singleRoute->destination, &routeInformation->destination, sizeof(NB_Place));
        nsl_memcpy(&singleRoute->originFix, &routeInformation->originFix, sizeof(NB_GpsLocation));
        singleRoute->routeLength = GetRouteLength(singleRoute);
        GetRouteTimeAndDelay(singleRoute, &singleRoute->routeTime, &singleRoute->routeDelay);
        err = NB_RouteParametersCopyPrivateConfigFromParameters(routeInformation->routeParameters, &singleRoute->config);
        err = err ? err : NB_RouteParametersCopyConfigFromParameters(routeInformation->routeParameters, &singleRoute->parameterConfiguration);
        err = err ? err : NB_RouteParametersClone(routeInformation->routeParameters, &singleRoute->routeParameters);
        singleRoute->routeID.size = (nb_size)singleRoute->dataReply.route_id.size;
        singleRoute->routeID.data = singleRoute->dataReply.route_id.data;
        singleRoute->tempDouble = routeInformation->tempDouble;
        singleRoute->tempUint32 = routeInformation->tempUint32;
        singleRoute->tempInt32 = routeInformation->tempInt32;
        singleRoute->numberOfSelector = 1; // single route!
        nsl_memcpy(&singleRoute->onRouteInfo, &routeInformation->onRouteInfo, sizeof(NB_OnRouteInformation));
        if (singleRoute->config.prefs.laneGuidanceFontMap)
        {
            CSL_HashTableDestroy(singleRoute->config.prefs.laneGuidanceFontMap, TRUE);
            singleRoute->config.prefs.laneGuidanceFontMap = NULL;
        }
        if (routeInformation->routeDescription)
        {
            singleRoute->currentDescription = routeInformation->routeDescription[index - 1];
        }
        NB_TrafficInformationCreateFromRoute(routeInformation->context, singleRoute, &singleRoute->trafficInformation);
    }

    if(err == NE_OK)
    {
        *singleRouteInformation = singleRoute;
    }
    else
    {
        NB_RouteInformationDestroy(singleRoute);
    }

    return err;
}

NB_DEF NB_Error NB_RouteInformationGetLaneInfo(NB_RouteInformation* route,
                                               uint32 index,
                                               uint32 *numberOfLanes,
                                               uint32 *lanePosition)
{
    data_nav_maneuver* man = NULL;

    if (!route)
    {
        return NE_INVAL;
    }

    man = NB_RouteInformationGetManeuver(route, index);
    if (!man)
    {
        return NE_NOENT;
    }
    if (numberOfLanes)
    {
        *numberOfLanes = man->lane_info.number_of_lanes;
    }
    if (lanePosition)
    {
        *lanePosition = man->lane_info.lane_position;
    }

    return NE_OK;
}

static void QaLogCurrentRoadInformation(NB_Context* context,int32 maneuverIndex, data_nav_maneuver* maneuver)
{
    NB_QaLogRoadInfoData roadInfo;
    data_roadinfo * road = NULL;
    data_util_state * dataState = NULL;

    nsl_memset(&roadInfo, 0, sizeof(roadInfo));
    if( !context || !maneuver)
    {
        return;
    }

    dataState = NB_ContextGetDataState(context);
    road = &maneuver->current_roadinfo;

    data_string_get_copy(dataState, &road->exit_number.number, roadInfo.exit_number, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, &road->primary, roadInfo.primary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, &road->secondary, roadInfo.secondary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, (data_string *)&road->exit_number.pronun.data, roadInfo.exit_number_pronun, NB_DATA_ROAD_INFO_TEXT_LENGTH);

    NB_QaLogRoadInfo(context, maneuverIndex, NB_ROAD_INFO_TYPE_CURRENT, &roadInfo);

}

static void QaLogTurnRoadInformation(NB_Context* context,int32 maneuverIndex, data_nav_maneuver* maneuver)
{
    NB_QaLogRoadInfoData roadInfo;
    data_roadinfo * road = NULL;
    data_util_state * dataState = NULL;

    nsl_memset(&roadInfo, 0, sizeof(roadInfo));
    if( !context || !maneuver)
    {
        return;
    }

    dataState = NB_ContextGetDataState(context);
    road = &maneuver->turn_roadinfo;

    data_string_get_copy(dataState, &road->exit_number.number, roadInfo.exit_number, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, &road->primary, roadInfo.primary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, &road->secondary, roadInfo.secondary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
    data_string_get_copy(dataState, (data_string *)&road->exit_number.pronun.data, roadInfo.exit_number_pronun, NB_DATA_ROAD_INFO_TEXT_LENGTH);

    NB_QaLogRoadInfo(context, maneuverIndex, NB_ROAD_INFO_TYPE_TURN, &roadInfo);

}
static void QalogLaneGuidanceInformation(NB_Context* context,int32 maneuverIndex, data_nav_maneuver* maneuver)
{
    NB_QaLogLaneInfoData laneInfo;
    NB_QaLogLaneItmeInfoData laneItemInfo ={0};
    data_lane_info * lane = NULL;
    data_lane_guidance_item * laneGuiance  = NULL;
    data_util_state * dataState = NULL;
    int count = 0;
    int i = 0;

    nsl_memset(&laneInfo, 0, sizeof(laneInfo));
    if( !context || !maneuver)
    {
        return;
    }

    dataState = NB_ContextGetDataState(context);

    lane = &maneuver->lane_info;
    laneInfo.lane_position = lane ->lane_position;
    laneInfo.number_of_lanes = lane ->number_of_lanes;

    if (laneInfo.lane_position != 0 || laneInfo.number_of_lanes != 0)
    {
        data_string_get_copy(dataState, &maneuver->command, laneInfo.turnCommane, NB_COMMAND_TEXT_LENGTH);
        data_string_get_copy(dataState, (data_string *)&lane->lg_pronun.data ,laneInfo.lg_pronun, NB_LANE_INFO_TEXT_LENGTH);
        data_string_get_copy(dataState, (data_string *)&lane->lgprep_pronun.data, laneInfo.lgprep_pronun, NB_LANE_INFO_TEXT_LENGTH);

        NB_QaLogLaneInfo(context, maneuverIndex, &laneInfo);


        count = CSL_VectorGetLength(lane->vec_lane_guidance_items);

        for (i = 0 ; i < count; i++)
        {
            laneGuiance = (data_lane_guidance_item *)CSL_VectorGetPointer(lane->vec_lane_guidance_items, i);
            laneItemInfo.lane_item = laneGuiance->lane_item.item;
            laneItemInfo.divider_item = laneGuiance->divider_item.item;
            laneItemInfo.highlight_arrow_item = laneGuiance->highlight_arrow_item.item;
            laneItemInfo.no_highlight_arrow_item = laneGuiance->no_highlight_arrow_item.item;
            NB_QaLogLaneItemInfo(context, maneuverIndex, &laneItemInfo);
        }
    }
}

#if 0
static void QaLogNaturalGuidanceInfomation(NB_Context* context,int32 maneuverIndex, data_nav_maneuver* maneuver)
{
    NB_QaLogGuidanceInfoData guidanceInfo;
    data_guidance_point* guidancePoint = NULL;
    data_util_state * dataState = NULL;
    int i = 0;

    nsl_memset(&guidanceInfo, 0, sizeof(guidanceInfo));
    if( !context || !maneuver)
    {
        return;
    }

    dataState = NB_ContextGetDataState(context);

    for (i = 0; i < 2; i++)
    {
        nsl_memset(&guidanceInfo, 0, sizeof(guidanceInfo));
        guidancePoint = &maneuver->guidance_pointer[i];

        guidanceInfo.point.lat = guidancePoint->point.lat;
        guidanceInfo.point.lon = guidancePoint->point.lon;
        data_string_get_copy(dataState, &maneuver->command, guidanceInfo.turnCommand, NB_COMMAND_TEXT_LENGTH);
        data_string_get_copy(dataState, &guidancePoint->description, guidanceInfo.description, NB_GUIDANCE_POINT_TEXT_LENGTH);
        if (nsl_strlen (guidanceInfo.description) != 0)
        {
            data_string_get_copy(dataState, (data_string *)&guidancePoint->gpprep_pronun.data, guidanceInfo.gpprep_pronun, NB_GUIDANCE_POINT_TEXT_LENGTH);
            data_string_get_copy(dataState, (data_string *)&guidancePoint->gp_pronun.data, guidanceInfo.gp_pronun, NB_GUIDANCE_POINT_TEXT_LENGTH);
            guidanceInfo.guidancePropuse =  (uint8)((guidancePoint->use_for_prepare ? 1 : 0) << 1 | (guidancePoint->use_for_turn ? 1 : 0));
            guidanceInfo.maneuver_point_offset = guidancePoint->maneuver_point_offset;

            NB_QaLogGuidancePoint(context, maneuverIndex, &guidanceInfo);
        }

    }
}
#endif

static void QaLogNaturalLaneGuidanceInformation(NB_Context* context,int32 maneuverIndex, data_nav_maneuver* maneuver)
{
    QaLogCurrentRoadInformation(context, maneuverIndex, maneuver);
    QaLogTurnRoadInformation(context, maneuverIndex, maneuver);
    QalogLaneGuidanceInformation(context, maneuverIndex, maneuver);
    QalogLaneGuidanceInformation(context, maneuverIndex, maneuver);
}

NB_DEF NB_Error
NB_RouteInformationGetManeuverGuidanceInformation(NB_RouteInformation* route, uint32 index, nb_boolean* pGuidanceDisabled)
{
    data_nav_maneuver* pmaneuver = NULL;

    if (!route || !pGuidanceDisabled)
    {
        return NE_INVAL;
    }

    *pGuidanceDisabled = FALSE;
    pmaneuver = NB_RouteInformationGetManeuver(route, index);
    if (pmaneuver)
    {
        *pGuidanceDisabled = pmaneuver->disable_guidance;
        return NE_OK;
    }

    return NE_NOENT;
}

void UpdateOnRouteForTraffic(NB_Context* context, NB_OnRouteInformation* onRouteInfo, CSL_Vector* incidentPlaces, CSL_Vector* flowItems)
{
    int i = 0;
    int n = 0;
    data_util_state* pds = NULL;
    data_place* place = NULL;
    data_traffic_flow_item* flowItem = NULL;
    char color = 0;

    if (!context || !onRouteInfo || !incidentPlaces || !flowItems)
    {
        return;
    }

    pds = NB_ContextGetDataState(context);

    n = CSL_VectorGetLength(incidentPlaces);
    for (i = 0; i < n; i++)
    {
        place = (data_place*)CSL_VectorGetPointer(incidentPlaces, i);
        if (place)
        {
            if (place->traffic_incident.type == NB_TIT_ACCIDENT)
            {
                onRouteInfo->isAccidentOnRoute = TRUE;
            }
            else if (place->traffic_incident.type == NB_TIT_UNSCHEDULED_CONSTRUCTION ||
                     place->traffic_incident.type == NB_TIT_SCHEDULED_CONSTRUCTION)
            {
                onRouteInfo->isConstructionOnRoute = TRUE;
            }
        }
        if (onRouteInfo->isAccidentOnRoute && onRouteInfo->isConstructionOnRoute)
        {
            break;
        }
    }

    n = CSL_VectorGetLength(flowItems);
    for (i = 0; i < n; i++)
    {
        flowItem = (data_traffic_flow_item*)CSL_VectorGetPointer(flowItems, i);
        if (flowItem)
        {
            color = data_string_get(pds, &flowItem->color)[0];
            if (color != 0 && color != 'G')
            {
                onRouteInfo->isConstructionOnRoute = TRUE;
                break;
            }
        }
    }
}

NB_DEF NB_Error
NB_RouteInformationGetRouteDescription(NB_RouteInformation* route, const char** description)
{
    if (!route || !description)
    {
        return NE_INVAL;
    }

    *description = route->currentDescription;
    return NE_OK;
}

NB_Error
NB_RouteInformationDestroyDescription(NB_RouteInformation* route)
{
    uint32 i = 0;
    if (!route || !route->numberOfSelector)
    {
        return NE_INVAL;
    }
    if (route->routeDescription)
    {
        for (i = 0; i < route->numberOfSelector; i++)
        {
            nsl_free(route->routeDescription[i]);
            route->routeDescription[i] = NULL;
        }
        nsl_free(route->routeDescription);
        route->routeDescription = NULL;
    }
    route->currentDescription = NULL;
    return NE_OK;
}

NB_DEC NB_Error NB_RouteInformationGetRouteSummaryInformationCount(NB_RouteInformation* route,
                                                                   uint32* count)
{
    if(!route || !count)
    {
        return NE_INVAL;
    }
    *count = CSL_VectorGetLength(route->dataReply.route_summarys);
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetRouteSummaryInformation(NB_RouteInformation* route, uint32* distance, uint32* travelTime, const char** via,uint32* delayTime,uint32  index)
{
    if (!route)
    {
        return NE_INVAL;
    }
    data_route_summary* routeSummary = NULL;
    size_t count = CSL_VectorGetLength(route->dataReply.route_summarys);
    if(count < index)
    {
        return NE_BADDATA;
    }
    routeSummary = (data_route_summary*)CSL_VectorGetPointer(route->dataReply.route_summarys, index);

    if (distance)
    {
        *distance = routeSummary->distance;
    }
    if (travelTime)
    {
        *travelTime = routeSummary->travel_time;
    }
    if (delayTime)
    {
        *delayTime = routeSummary->delay_time;
    }
    if (via)
    {
        data_util_state* pds = NB_ContextGetDataState(route->context);
        *via = data_string_get(pds, &routeSummary->via);
    }
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficEventCount(NB_RouteInformation* route, uint32* count)
{
    if (!route || !route->trafficInformation || !route->trafficInformation->trafficEvents)
    {
        if (count)
        {
            *count = 0;
        }
        return NE_INVAL;
    }

    if (!count)
    {
        return NE_INVAL;
    }

    *count = (uint32)CSL_VectorGetLength(route->trafficInformation->trafficEvents);
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficEvent(NB_RouteInformation* route, uint32 index, NB_TrafficEvent* trafficEvent)
{
    uint32 count = 0;
    NB_TrafficEvent* traffic = NULL;
    if (!route || !route->trafficInformation || !route->trafficInformation->trafficEvents)
    {
        return NE_INVAL;
    }

    if (!trafficEvent)
    {
        return NE_INVAL;
    }

    count = (uint32)CSL_VectorGetLength(route->trafficInformation->trafficEvents);
    if (index >= count)
    {
        return NE_NOENT;
    }

    traffic = (NB_TrafficEvent*)CSL_VectorGetPointer(route->trafficInformation->trafficEvents, index);
    if (!traffic)
    {
        return NE_NOENT;
    }

    *trafficEvent = *traffic;
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficColor(NB_RouteInformation* route, char* color)
{
    double delayPercent = 0;
    if (!route || !color)
    {
        return NE_INVAL;
    }

    delayPercent = (double)(route->routeTime - route->routeDelay);
    if (delayPercent != 0)
    {
        delayPercent = route->routeDelay / delayPercent;
    }

    *color = get_traffic_color(delayPercent * 100, route->trafficDataValid);
    return NE_OK;
}

NB_DEF NB_Error
NB_RouteInformationGetTrafficDelay(NB_RouteInformation* route, uint32 maneuverIndex, uint32* delay)
{
    if (!route || !route->trafficInformation || !delay)
    {
        return NE_INVAL;
    }
    return NB_TrafficInformationGetSummary(route->trafficInformation, route, maneuverIndex, 0.0, maneuverIndex, NULL, NULL, NULL, delay);
}

NB_DEF NB_Error
NB_RouteInformationGetDirectionToRoute(NB_RouteInformation* route, double projLatitude, double projLongitude, const char** directionName, double* headingToOrigin)
{
    data_util_state* dataState = NULL;
    data_nav_maneuver* firstManeuver = NULL;
    double startLatitude = INVALID_LATITUDE;
    double startLongitude = INVALID_LONGITUDE;
    double firstTurnLatitude = INVALID_LATITUDE;
    double firstTurnLongitude = INVALID_LONGITUDE;
    double currentToStartHeading = 0;
    double startToFirstTurnHeading = 0;
    double headingDiff = 0;

    if (!route)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(route->context);
    if (!dataState)
    {
        return NE_OK;
    }

    firstManeuver = data_nav_reply_get_nav_maneuver(dataState, &route->dataReply, 0);
    if (!firstManeuver)
    {
        return NE_NOENT;
    }

    if (firstManeuver->polyline.numsegments == 0)
    {
        return NE_NOENT;
    }

    data_polyline_get(dataState, &firstManeuver->polyline, 0, &startLatitude, &startLongitude, NULL, NULL);
    if(firstManeuver->polyline.numsegments > 1)
    {
        data_polyline_get(dataState, &firstManeuver->polyline, 1, &firstTurnLatitude, &firstTurnLongitude, NULL, NULL);
        NB_SpatialGetLineOfSightDistance(projLatitude, projLongitude, startLatitude, startLongitude, &currentToStartHeading);
        NB_SpatialGetLineOfSightDistance(startLatitude, startLongitude, firstTurnLatitude, firstTurnLongitude, &startToFirstTurnHeading);
        headingDiff = currentToStartHeading - startToFirstTurnHeading;
    }
    else
    {
        headingDiff = 0;
    }
    if (headingDiff > 180)
    {
        headingDiff = headingDiff - 360;
    }
    if (headingDiff < -180)
    {
        headingDiff = headingDiff + 360;
    }

    if (directionName)
    {
        *directionName = GetDirectionName(headingDiff, route->parameterConfiguration.orManeuverTurnAngleTolerance);
    }
    if (headingToOrigin)
    {
        *headingToOrigin = currentToStartHeading;
    }
    return NE_OK;
}

const char*
GetHeadingName(double heading)
{
    int headingNameIndex = 0;
    if (heading >= 22.5 && heading < 67.5)
        headingNameIndex = 1;
    else if (heading >= 67.5 && heading < 112.5)
        headingNameIndex = 2;
    else if (heading >= 112.5 && heading < 157.5)
        headingNameIndex = 3;
    else if (heading >= 157.5 && heading < 202.5)
        headingNameIndex = 4;
    else if (heading >= 202.5 && heading < 247.5)
        headingNameIndex = 5;
    else if (heading >= 247.5 && heading < 292.5)
        headingNameIndex = 6;
    else if (heading >= 292.5 || heading < 337.5)
        headingNameIndex = 7;
    return HEADING_NAME[headingNameIndex];
}

const char*
GetDirectionName(double turnAngle, double tolerance)
{
    if ((turnAngle >= 0) && nsl_fabs(turnAngle) > tolerance)
    {
        return "left";
    }
    else if ((turnAngle < 0) && nsl_fabs(turnAngle) > tolerance)
    {
        return "right";
    }
    else
    {
        return "";
    }
}

NB_Error NB_RouteInformationGetManeuverPropertys(NB_RouteInformation* route, uint32 index, NB_OnRouteInformation* propertys)
{
    if(!route || !propertys)
    {
        return NE_INVAL;
    }
    data_util_state* pds = NB_ContextGetDataState(route->context);
    data_nav_maneuver* man = data_nav_reply_get_nav_maneuver(pds, &route->dataReply, index);
    if (!man)
    {
        return NE_NOENT;
    }
    propertys->isFerryOnRoute = man->isFerryOnRoute;
    propertys->isGatedAccessOnRoute = man->isGatedAccessOnRoute;
    propertys->isHighWayOnRoute = man->isHighWayOnRoute;
    propertys->isHovLanesOnRoute = man->isHovLanesOnRoute;
    propertys->isPrivateRoadOnRoute = man->isPrivateRoadOnRoute;
    propertys->isTollsOnRoute = man->isTollsOnRoute;
    propertys->isUnpavedOnRoute = man->isUnpavedOnRoute;
    propertys->isEnterCoutryOnRoute = man->isEnterCoutryOnRoute;
    return NE_OK;
}

NB_TrafficInformation* NB_RouteInformationGetTrafficInformationPointer(NB_RouteInformation* route)
{
    if(route)
    {
        return route->trafficInformation;
    }
    else
    {
        return NULL;
    }
}

void NB_RouteInformationSetTrafficInformationPointer(NB_RouteInformation* route, NB_TrafficInformation* info)
{
    if(route)
    {
        route->trafficInformation = info;
    }
}
