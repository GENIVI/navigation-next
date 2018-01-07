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

    @file     nbsearchparameters.c
    @defgroup nbparameters Parameters

    Parameter object contain the information required to download information
    from the NAVBuilder server.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsearchparameters.h"
#include "nbsearchparametersprivate.h"
#include "nbsearchinformationprivate.h"
#include "nbrouteinformationprivate.h"
#include "nbcontextprotected.h"
#include "data_proxpoi_query.h"
#include "csltypes.h"
#include "nbutility.h"
#include "transformutility.h"

// Local Structures ..............................................................................

struct NB_SearchParameters
{
    NB_Context*                 context;
    data_proxpoi_query          dataQuery;
    NB_IterationCommand         command;
    int                         slice;
    uint32                      analyticsEventId;
    nb_boolean                  hasValidAnalyticsEventId;    // This is need because valid value for analyticsEventId can be zero(0).
    NB_SearchInputMethod        inputMethod;
    NB_SearchSource             searchSource;
};


// Local Functions ...............................................................................

static NB_SearchParameters* AllocateSearchParameters(NB_Context* context);

static NB_Error SetIterationCommand(NB_SearchParameters* parameters, NB_IterationCommand command);
static void SetSliceSize(NB_SearchParameters* parameters, int sliceSize);
static NB_Error SetSearchLocation(NB_SearchParameters* parameters, NB_SearchRegion* region);
static NB_Error SetSearchLanguage(NB_SearchParameters* parameters, const char* language);
static NB_Error SetSearchScheme(NB_SearchParameters* parameters, const char* scheme);
static NB_Error SetExtendedConfiguration(NB_SearchParameters* parameters, NB_POIExtendedConfiguration extendedConfiguration);
static NB_Error AddSearchFilterKeyValue(NB_SearchParameters* parameters, const char* key, const char* value);
static NB_Error SetSearchResultStyle(NB_SearchParameters* parameters, const char* style);
static NB_Error AddFiltersForFuelSearch(NB_SearchParameters* parameters, NB_FuelSearchType SearchType, NB_FuelResultsType resultsType);
static NB_Error AddFiltersForWeatherSearch(NB_SearchParameters* parameters, int ForecastDays, NB_WeatherResultsType resultsType);
static NB_Error AddFiltersForTrafficIncidentSearch(NB_SearchParameters* parameters, int severity, NB_TrafficIncidentResultsType resultsType);
static NB_Error AddFiltersForEventSearch(NB_SearchParameters* parameters, const char* categoryCodes[], int categoryCodeCount, uint32 startTime, uint32 endTime, NB_EventRating rating, NB_EventResultsType resultsType);
static NB_Error AddFiltersForEventVenueSearch(NB_SearchParameters* parameters, const char* categoryCodes[], int categoryCodeCount, uint32 startTime, uint32 endTime, NB_EventRating rating, NB_EventVenueResultsType resultsType);
static NB_Error AddFiltersForMovieSearch(NB_SearchParameters* parameters, uint32 startTime, uint32 endTime, NB_MovieShowing showing, const char* genreCodes[], int genreCodeCount, NB_MovieSortBy sortBy, NB_MovieResultsType resultsType);
static NB_Error AddFiltersForMovieTheaterSearch(NB_SearchParameters* parameters, uint32 startTime, uint32 endTime, NB_MovieTheaterResultsType resultsType);
static NB_Error SetImage(data_util_state* dataState, data_image* image, NB_ImageFormat format, NB_ImageSize* size, uint16 dpi);


// Public Functions ..............................................................................

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreatePOI(NB_Context* context,
                             NB_SearchRegion* region,
                             const char* name,
                             const char* searchScheme,
                             const char* categoryCodes[],
                             int categoryCodeCount,
                             uint32 sliceSize,
                             NB_POIExtendedConfiguration extendedConfiguration,
                             const char* language,
                             NB_SearchParameters** parameters)
{
    NB_Error error = NE_OK;
    int i = 0;
    NB_SearchParameters* newParameters = NULL;

    if(!context || categoryCodeCount < 0 || (!categoryCodes && categoryCodeCount > 0) || !parameters)
    {
        return NE_INVAL;
    }

    newParameters = AllocateSearchParameters(context);
    if (newParameters == NULL)
    {
        return NE_NOMEM;
    }

    error = error ? error : SetIterationCommand(newParameters, NB_IterationCommand_Start);
    SetSliceSize(newParameters, sliceSize);
    error = error ? error : SetSearchLocation(newParameters, region);
    error = error ? error : SetSearchLanguage(newParameters, language);
    error = error ? error : SetSearchScheme(newParameters, searchScheme);
    error = error ? error : SetExtendedConfiguration(newParameters, extendedConfiguration);

    if (name != NULL)
    {
        error = error ? error : AddSearchFilterKeyValue(newParameters, "name", name);
    }

    // Add all the event categories to search for. A combination of categories can be used for a search
    for (i = 0; i < categoryCodeCount; i++)
    {
        // Add the category to the search filter
        error = error ? error :AddSearchFilterKeyValue(newParameters, "category", categoryCodes[i]);
    }

    error = error ? error : SetSearchResultStyle(newParameters, "all");

    if(NE_OK == error)
    {
        *parameters = newParameters;
    }
    else
    {
        NB_SearchParametersDestroy(newParameters);
        newParameters = NULL;
        *parameters = NULL;
    }

    return error;
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreatePublicTransit(NB_Context* context,
                                       NB_SearchRegion* region,
                                       const char* name,
                                       const char* categoryCodes[],
                                       int categoryCodeCount,
                                       uint32 sliceSize,
                                       const char* language,
                                       NB_SearchParameters** parameters)
{
    return NB_SearchParametersCreatePOI(context, region, name, "public-transit", categoryCodes, categoryCodeCount, sliceSize, NB_EXT_None, language, parameters);
}

NB_DEF NB_Error
NB_SearchParametersCreatePublicTransitOverlay(NB_Context* context,
                                            NB_SearchRegion* region,
                                            const char* name,
                                            const char* transitTypes[],
                                            int transitTypeCount,
                                            uint32 departureTime,
                                            uint32 sliceSize,
                                            const char* language,
                                            NB_SearchParameters** parameters)
{
    char text[64] = {0};
    int i = 0;
    NB_Error error = NE_OK;

    if (transitTypes == NULL || transitTypeCount == 0)
    {
        return NE_INVAL;
    }

    error = NB_SearchParametersCreatePOI(context, region, name, "public-transit", NULL, 0, sliceSize, NB_EXT_None, language, parameters);

    for (i = 0; i < transitTypeCount; i++)
    {
        error = error ? error : AddSearchFilterKeyValue(*parameters, "transit-type", transitTypes[i]);
    }

    if(departureTime != 0)
    {
        nsl_sprintf(text, "%u", departureTime);
        error = error ? error : AddSearchFilterKeyValue(*parameters, "departure-time", text);
    }

    return error;
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateFuel(NB_Context* context,
                              NB_SearchRegion* region,
                              const char* name,
                              const char* searchScheme,
                              const char* categoryCodes[],
                              int categoryCodeCount,
                              NB_FuelSearchType searchType,
                              NB_FuelResultsType resultsType,
                              uint32 sliceSize,
                              NB_POIExtendedConfiguration extendedConfiguration,
                              const char* language,
                              NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, name, searchScheme, categoryCodes, categoryCodeCount, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForFuelSearch(*parameters, searchType, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateWeather(NB_Context* context,
                                 NB_LatitudeLongitude* center,
                                 int forecastDays,
                                 NB_WeatherResultsType resultsType,
                                 NB_POIExtendedConfiguration extendedConfiguration,
                                 const char* language,
                                 NB_SearchParameters** parameters)
{
    NB_Error error = NE_OK;
    NB_SearchRegion region = {0};

    region.type = NB_ST_Center;
    region.center.latitude = center->latitude;
    region.center.longitude = center->longitude;

    error = NB_SearchParametersCreatePOI(context, &region, NULL, "weather", NULL, 0, 1, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForWeatherSearch(*parameters, forecastDays, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateTrafficIncident(NB_Context* context,
                                         NB_SearchRegion* region,
                                         int severity,
                                         NB_TrafficIncidentResultsType resultsType,
                                         uint32 sliceSize,
                                         NB_POIExtendedConfiguration extendedConfiguration,
                                         const char* language,
                                         NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, NULL, "traffic-incident", NULL, 0, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForTrafficIncidentSearch(*parameters, severity, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateEvent(NB_Context* context,
                               NB_SearchRegion* region,
                               const char* name,
                               const char* categoryCodes[],
                               int categoryCodeCount,
                               uint32 startTime,
                               uint32 endTime,
                               NB_EventRating rating,
                               NB_EventResultsType resultsType,
                               uint32 sliceSize,
                               NB_POIExtendedConfiguration extendedConfiguration,
                               const char* language,
                               NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, name, "event", NULL, 0, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForEventSearch(*parameters, categoryCodes, categoryCodeCount, startTime, endTime, rating, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateEventVenue(
                NB_Context* context,
                NB_SearchRegion* region,
                const char* name,
                const char* categoryCodes[],
                int categoryCodeCount,
                uint32 startTime,
                uint32 endTime,
                NB_EventRating rating,
                NB_EventVenueResultsType resultsType,
                uint32 sliceSize,
                NB_POIExtendedConfiguration extendedConfiguration,
                const char* language,
                NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, name, "event-venue", NULL, 0, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForEventVenueSearch(*parameters, categoryCodes, categoryCodeCount, startTime, endTime, rating, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateMovie(NB_Context* context,
                               NB_SearchRegion* region,
                               const char* name,
                               uint32 startTime,
                               uint32 endTime,
                               NB_MovieShowing showing,
                               const char* genreCodes[],
                               int genreCodeCount,
                               NB_MovieSortBy sortBy,
                               NB_MovieResultsType resultsType,
                               uint32 sliceSize,
                               NB_POIExtendedConfiguration extendedConfiguration,
                               const char* language,
                               NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, name, "movie", NULL, 0, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForMovieSearch(*parameters, startTime, endTime, showing, genreCodes, genreCodeCount, sortBy, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateMovieTheater(NB_Context* context,
                                      NB_SearchRegion* region,
                                      const char* name,
                                      uint32 startTime,
                                      uint32 endTime,
                                      NB_MovieTheaterResultsType resultsType,
                                      uint32 sliceSize,
                                      NB_POIExtendedConfiguration extendedConfiguration,
                                      const char* language,
                                      NB_SearchParameters** parameters)
{
    NB_Error error = NB_SearchParametersCreatePOI(context, region, name, "movie-theater", NULL, 0, sliceSize, extendedConfiguration, language, parameters);
    if (error != NE_OK)
    {
        return error;
    }

    return AddFiltersForMovieTheaterSearch(*parameters, startTime, endTime, resultsType);
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateIteration(NB_Context* context,
                                   NB_SearchInformation* information,
                                   NB_IterationCommand iterationCommand,
                                   NB_SearchParameters** parameters)
{
    NB_Error error = NE_OK;
    nb_boolean hasAnalyticsEventId = FALSE;
    uint32 analyticsEventId = 0;
    data_proxpoi_query* previousQuery = NB_SearchInformationGetQuery(information);
    NB_SearchParameters* newParameters = AllocateSearchParameters(context);

    if (newParameters == NULL)
    {
        return NE_NOMEM;
    }

    // Copy previous analytics event id
    error = NB_SearchInformationHasAnalyticsEventId(information, &hasAnalyticsEventId);
    if (error == NE_OK)
    {
        if (hasAnalyticsEventId)
        {
            error = NB_SearchInformationGetAnalyticsEventId(information, &analyticsEventId);
            if (error == NE_OK)
            {
                newParameters->analyticsEventId = analyticsEventId;
                newParameters->hasValidAnalyticsEventId = TRUE;
            }
        }
    }

    /* Copy the previous query which was retrieved from the Search Information.  In theory the iteration state
       should contain all the necessary information, but that doesn't appear to be the case. */
    error = error ? error : data_proxpoi_query_copy(NB_ContextGetDataState(context),
                                &newParameters->dataQuery, previousQuery);

    error = error ? error : SetIterationCommand(newParameters, iterationCommand);

    if (iterationCommand != NB_IterationCommand_Start)
    {
         error = error ? error : data_blob_copy(NB_ContextGetDataState(newParameters->context),
                                               &newParameters->dataQuery.iter_command.state,
                                               NB_SearchInformationGetIterationState(information));
    }

    *parameters = newParameters;

    return error;
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersCreateSubSearch(NB_Context* context,
                                   NB_SearchInformation* information,
                                   int resultIndex,
                                   NB_SearchParameters** parameters)
{
    NB_Error error = NE_OK;
    data_proxpoi_query* previousQuery = NB_SearchInformationGetQuery(information);
    NB_SearchParameters* newParameters = AllocateSearchParameters(context);

    if (newParameters == NULL)
    {
        return NE_NOMEM;
    }

    /* Copy the previous query which was retrieved from the Search Information.  In theory the iteration state
       should contain all the necessary information, but that doesn't appear to be the case. */
    error = error ? error : data_proxpoi_query_copy(NB_ContextGetDataState(context),
                                &newParameters->dataQuery, previousQuery);

    error = error ? error : SetIterationCommand(newParameters, NB_IterationCommand_Start);
    if (error != NE_OK)
    {
        NB_SearchParametersDestroy(newParameters);
        return error;
    }

    // Clone the search filter from server reply and assign it to our new parameters object
    error = NB_SearchInformationCloneSearchFilter(information, resultIndex, &(newParameters->dataQuery.filter));
    if (error != NE_OK)
    {
        NB_SearchParametersDestroy(newParameters);
        return error;
    }

    *parameters = newParameters;

    return NE_OK;
}

// See header file for description
NB_DEF NB_Error
NB_SearchParametersDestroy(NB_SearchParameters* parameters)
{
    if (parameters == NULL)
    {
        return NE_INVAL;
    }

    // Free All Application Data
    data_proxpoi_query_free(NB_ContextGetDataState(parameters->context), &parameters->dataQuery);

    nsl_free(parameters);

    return NE_OK;
}

/* Private Functions - for use within the SDK */

tpselt
NB_SearchParametersToTPSQuery(NB_SearchParameters* parameters)
{
    data_util_state* dataState = NB_ContextGetDataState(parameters->context);

    // if context contains a previously returned search cookie, add it to the outgoing query
    const data_search_cookie* searchCookie = NB_ContextGetSearchCookie(parameters->context);
    if (searchCookie)
    {
        data_proxpoi_query_set_search_cookie(dataState, &parameters->dataQuery, searchCookie);
    }

    // if route id was specified for POI search and search source is a navigation search
    // (that is fast POI or cameras), move the route-id to the route-corridor's route-id
    if (parameters->dataQuery.route_id.size && parameters->searchSource == NB_SS_Navigation)
    {
        data_blob_copy(dataState, &parameters->dataQuery.route_corridor.route_id, &parameters->dataQuery.route_id);
        data_blob_free(dataState, &parameters->dataQuery.route_id);
    }

    return data_proxpoi_query_to_tps(NB_ContextGetDataState(parameters->context), &parameters->dataQuery);
}

NB_Error
NB_SearchParametersCopyQuery(NB_SearchParameters* parameters, data_proxpoi_query* query_dest)
{
    return data_proxpoi_query_copy(NB_ContextGetDataState(parameters->context), query_dest,&parameters->dataQuery);
}

NB_DEF NB_Error
NB_SearchParametersAddSearchFilterKeyValue(NB_SearchParameters* parameters,
                                            const char* key,
                                            const char*  value)
{
    return AddSearchFilterKeyValue(parameters, key, value);
}

static NB_SearchParameters*
AllocateSearchParameters(NB_Context* context)
{
    NB_SearchParameters* pThis = nsl_malloc(sizeof(*pThis));

    if (!pThis)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    data_proxpoi_query_init(NB_ContextGetDataState(pThis->context), &pThis->dataQuery);

    pThis->searchSource = NB_SS_None;

    return pThis;
}

static NB_Error
SetIterationCommand(NB_SearchParameters* parameters, NB_IterationCommand command)
{
    NB_Error error = NE_OK;

    switch (command)
    {
        case NB_IterationCommand_Start:
            error = data_string_set(NB_ContextGetDataState(parameters->context),
                                    &parameters->dataQuery.iter_command.command,
                                    "start");
            break;
        case NB_IterationCommand_Next:
            error = data_string_set(NB_ContextGetDataState(parameters->context),
                                    &parameters->dataQuery.iter_command.command,
                                    "next");
            break;
        case NB_IterationCommand_Previous:
            error = data_string_set(NB_ContextGetDataState(parameters->context),
                                    &parameters->dataQuery.iter_command.command,
                                    "prev");
            break;
        default:
        error = NE_INVAL;
    }

    return error;
}

static void
SetSliceSize(NB_SearchParameters* parameters, int sliceSize)
{
    parameters->dataQuery.iter_command.number = sliceSize;
}

static NB_Error
SetSearchLocation(NB_SearchParameters* parameters, NB_SearchRegion* region)
{
    NB_Error error = NE_OK;

    if(!parameters || !region)
    {
        return NE_INVAL;
    }

    data_blob_free(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.route_id);
    data_blob_free(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.route_corridor.route_id);

    parameters->dataQuery.maneuver_valid = FALSE;
    parameters->dataQuery.maneuver = NAV_MANEUVER_NONE;

    switch(region->type)
    {
        case NB_ST_Center:
        {
            error = IsLatitudeLongitudeValid(&region->center) ? NE_OK : NE_INVAL;

            error = error ? error : data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.position.variant, "point");

            parameters->dataQuery.position.point.lat = region->center.latitude;
            parameters->dataQuery.position.point.lon = region->center.longitude;
            break;
        }

        case NB_ST_RouteAndCenter:
        {
            data_blob* routeId = 0;

            error = IsLatitudeLongitudeValid(&region->center) ? NE_OK : NE_INVAL;

            error = error ? error : NB_RouteInformationGetID(region->route, &routeId);
            error = error ? error : data_blob_copy(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.route_id, routeId);

            error = error ? error : data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.position.variant , "point");
            parameters->dataQuery.position.point.lat = region->center.latitude;
            parameters->dataQuery.position.point.lon = region->center.longitude;

            parameters->dataQuery.route_corridor.distance = region->distance;
            parameters->dataQuery.route_corridor.width = region->width;

            break;
        }

        case NB_ST_DirectionAndCenter:
        {
            NB_GpsLocation fix;
            gpsfix_clear(&fix);

            error = IsLatitudeLongitudeValid(&region->center) ? NE_OK : NE_INVAL;

            parameters->dataQuery.directed = TRUE;
            error = error ? error : data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.position.variant, "gps");

            fix.latitude = region->center.latitude;
            fix.longitude = region->center.longitude;
            fix.heading = region->direction.heading;
            fix.horizontalVelocity = region->direction.speed;

            if (region->center.latitude != INVALID_LATLON)
            {
                fix.valid |= NGV_Latitude;
            }
            if (region->center.longitude != INVALID_LATLON)
            {
                fix.valid |= NGV_Longitude;
            }
            if (region->direction.heading != INVALID_HEADING)
            {
                fix.valid |= NGV_Heading;
            }

            fix.valid |= NGV_HorizontalVelocity;

            data_gps_from_gpsfix(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.position.gps, &fix);
            break;
        }

        case NB_ST_BoundingBox:
        {
            error = (IsLatitudeLongitudeValid(&region->boundingBox.topLeft) &&
                     IsLatitudeLongitudeValid(&region->boundingBox.bottomRight)) ? NE_OK : NE_INVAL;

            parameters->dataQuery.position.boxValid = TRUE;
            parameters->dataQuery.position.boundingBox.topLeft.lat = region->boundingBox.topLeft.latitude;
            parameters->dataQuery.position.boundingBox.topLeft.lon = region->boundingBox.topLeft.longitude;
            parameters->dataQuery.position.boundingBox.bottomRight.lat = region->boundingBox.bottomRight.latitude;
            parameters->dataQuery.position.boundingBox.bottomRight.lon = region->boundingBox.bottomRight.longitude;
            break;
        }

        case NB_ST_None:
        default:
            return NE_INVAL;
    }

    return error;
}

static NB_Error
SetSearchLanguage(NB_SearchParameters* parameters, const char* language)
{
    return data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.language, language);
}

static NB_Error
SetSearchScheme(NB_SearchParameters* parameters, const char* scheme)
{
    return data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.scheme, scheme);
}

static NB_Error
SetExtendedConfiguration(NB_SearchParameters* parameters, NB_POIExtendedConfiguration extendedConfiguration)
{
    parameters->dataQuery.want_premium_placement = extendedConfiguration & NB_EXT_WantPremiumPlacement ? TRUE : FALSE;
    parameters->dataQuery.want_enhanced_pois = extendedConfiguration & NB_EXT_WantEnhancedPOI ? TRUE : FALSE;
    parameters->dataQuery.want_formatted = extendedConfiguration & NB_EXT_WantFormattedText  ? TRUE : FALSE;
    parameters->dataQuery.want_spelling_suggestions = extendedConfiguration & NB_EXT_WantSpellingSuggestions ? TRUE : FALSE;
    parameters->dataQuery.want_non_proximity_pois = extendedConfiguration & NB_EXT_WantNonProximityPois ? TRUE : FALSE;

    return NE_OK;
}


static NB_Error
AddSearchFilterKeyValue(NB_SearchParameters* parameters, const char* key, const char* value)
{
    NB_Error error = NE_OK;
    data_pair DataPair;

    data_pair_init(NB_ContextGetDataState(parameters->context), &DataPair);

    if (parameters->dataQuery.filter.vec_pairs == NULL)
    {
        parameters->dataQuery.filter.vec_pairs = CSL_VectorAlloc(sizeof(data_pair));

        if (parameters->dataQuery.filter.vec_pairs == NULL)
        {
            error = NE_NOMEM;
            goto exit;
        }
    }

    data_string_set(NB_ContextGetDataState(parameters->context), &DataPair.key, key);
    data_string_set(NB_ContextGetDataState(parameters->context), &DataPair.value, value);

    if (CSL_VectorAppend(parameters->dataQuery.filter.vec_pairs, &DataPair))
    {
        // memory taken over by the vector
        data_pair_init(NB_ContextGetDataState(parameters->context), &DataPair);
    }
    else
    {
        error = NE_NOMEM;
    }

exit:
    if (error != NE_OK)
    {
        data_pair_free(NB_ContextGetDataState(parameters->context), &DataPair);
    }

    return error;
}

static NB_Error
SetSearchResultStyle(NB_SearchParameters* parameters, const char* style)
{
    data_string_set(NB_ContextGetDataState(parameters->context), &parameters->dataQuery.filter.result_style.key, style);
    return NE_OK;
}

static NB_Error
AddFiltersForFuelSearch(NB_SearchParameters* parameters, NB_FuelSearchType SearchType, NB_FuelResultsType resultsType)
{
    NB_Error error = NE_OK;

    switch (SearchType)
    {
        case NB_FST_Regular:
            error = error ? error : AddSearchFilterKeyValue(parameters, "fuel-type", "r");
            break;
        case NB_FST_Diesel:
            error = error ? error : AddSearchFilterKeyValue(parameters, "fuel-type", "d");
            break;
        case NB_FST_Ethanol85:
            error = error ? error : AddSearchFilterKeyValue(parameters, "fuel-type", "e");
            break;
        case NB_FST_All:
            error = error ? error : AddSearchFilterKeyValue(parameters, "fuel-type", "a");
            break;
    }

    switch (resultsType)
    {
        case NB_FRT_All:
            SetSearchResultStyle(parameters, "all");
            break;
        case NB_FRT_CarouselLite:
            error = error ? error : AddSearchFilterKeyValue(parameters, "proxmatch", "1");
            SetSearchResultStyle(parameters, "carousel-lite");
            break;
    }

    return error;
}

static NB_Error
AddFiltersForWeatherSearch(NB_SearchParameters* parameters, int ForecastDays, NB_WeatherResultsType resultsType)
{
    NB_Error error = NE_OK;

    switch (resultsType)
    {
        case NB_WRT_Current:
            SetSearchResultStyle(parameters, "current");
            break;
        case NB_WRT_Forecast:
            SetSearchResultStyle(parameters, "forecast");
            break;
        case NB_WRT_All:
            SetSearchResultStyle(parameters, "all");
            break;
        case NB_WRT_CarouselLite:
            SetSearchResultStyle(parameters, "carousel-lite");
            break;
    }

    return error;
}

static NB_Error
AddFiltersForTrafficIncidentSearch(NB_SearchParameters* parameters, int severity, NB_TrafficIncidentResultsType resultsType)
{
    NB_Error error = NE_OK;
    char severityString[12]; /* ten digits + minus + terminating null */

    nsl_sprintf(severityString, "%d", severity);

    error = error ? error : AddSearchFilterKeyValue(parameters, "severity", severityString);

    switch (resultsType)
    {
        case NB_TIRT_All:
            SetSearchResultStyle(parameters, "all");
            break;
    }

    return error;
}

static NB_Error
AddFiltersForEventSearch(NB_SearchParameters* parameters,
                         const char* categoryCodes[],
                         int categoryCodeCount,
                         uint32 startTime,
                         uint32 endTime,
                         NB_EventRating rating,
                         NB_EventResultsType resultsType)
{
    NB_Error error = NE_OK;
    int i = 0;

    // Add event rating
    if (rating == NB_ER_KidFriendly)
    {
        error = error ? error : AddSearchFilterKeyValue(parameters, "rating", "KidFriendly");
    }
    else
    {
        error = error ? error : AddSearchFilterKeyValue(parameters, "rating", "All");
    }

    // Add all the event categories to search for. A combination of categories can be used for a search
    for (i = 0; i < categoryCodeCount; i++)
    {
        // Add the category to the search filter
        error = error ? error : AddSearchFilterKeyValue(parameters, "event-type", categoryCodes[i]);
    }

    {
        char text[64];

        nsl_sprintf(text, "%u", startTime);
        error = error ? error : AddSearchFilterKeyValue(parameters, "start-time", text);

        if (endTime != 0)
        {
            nsl_sprintf(text, "%u", endTime);
            error = error ? error : AddSearchFilterKeyValue(parameters, "end-time", text);
        }
    }

    switch (resultsType)
    {
        case NB_ERT_All:
            SetSearchResultStyle(parameters, "all");
            break;
        case NB_ERT_CarouselLite:
            error = error ? error : AddSearchFilterKeyValue(parameters, "proxmatch", "1");
            SetSearchResultStyle(parameters, "carousel-lite");
            break;
    }

    return error;
}

NB_Error
AddFiltersForEventVenueSearch(NB_SearchParameters* parameters,
                              const char* categoryCodes[],
                              int categoryCodeCount,
                              uint32 startTime,
                              uint32 endTime,
                              NB_EventRating rating,
                              NB_EventVenueResultsType resultsType)
{
    NB_Error error = NE_OK;

    // Set name and category to empty.
    error = error ? error : AddSearchFilterKeyValue(parameters, "name", "");
    error = error ? error : AddSearchFilterKeyValue(parameters, "category", "");

    // Set start and end time
    {
        char text[64];

        nsl_sprintf(text, "%u", startTime);
        error = error ? error : AddSearchFilterKeyValue(parameters, "start-time", text);

        if (endTime != 0)
        {
            nsl_sprintf(text, "%u", endTime);
            error = error ? error : AddSearchFilterKeyValue(parameters, "end-time", text);
        }
    }

    return error;
}

static NB_Error
AddFiltersForMovieSearch(NB_SearchParameters* parameters, uint32 startTime, uint32 endTime, NB_MovieShowing showing,
                         const char* genreCodes[], int genreCodeCount, NB_MovieSortBy sortBy, NB_MovieResultsType resultsType)
{
    NB_Error error = NE_OK;
    char* showingValue = NULL;
    char* sortingValue = NULL;

    if (genreCodes == NULL || genreCodeCount == 0)
    {
        error = error ? error : AddSearchFilterKeyValue(parameters, "genre", "All");
    }
    else
    {
        int i = 0;

        // Add all the movie categories based on the passed in flags
        for (i = 0; i < genreCodeCount; i++)
        {
            error = error ? error : AddSearchFilterKeyValue(parameters, "genre", genreCodes[i]);
        }
    }

    // Add showing
    switch (showing)
    {
        case NB_MS_NowInTheaters:       showingValue = "NowInTheater";       break;
        case NB_MS_TopBoxOffice:        showingValue = "TopBoxOffice";       break;
        case NB_MS_OpeningThisWeek:     showingValue = "OpeningThisWeek";    break;
        case NB_MS_ComingSoon:          showingValue = "ComingSoon";         break;
        case NB_MS_SpecificTime:        showingValue = "SpecifyTheDay";      break;
    }

    error = error ? error : AddSearchFilterKeyValue(parameters, "showing", showingValue);

    // Add sorting
    switch (sortBy)
    {
        case NB_MSB_MostPopular:        sortingValue = "MostPopular";        break;
        case NB_MSB_MostRecent:         sortingValue = "MostRecent";         break;
        case NB_MSB_Alphabetical:       sortingValue = "Alphabetical";       break;
    }

    error = error ? error : AddSearchFilterKeyValue(parameters, "sort-by", sortingValue);

    if (showing == NB_MS_SpecificTime && startTime != 0)
    {
        char text[64];

        nsl_sprintf(text, "%u", startTime);
        error = error ? error : AddSearchFilterKeyValue(parameters, "start-time", text);
        if (endTime != 0)
        {
            nsl_sprintf(text, "%u", endTime);
            error = error ? error : AddSearchFilterKeyValue(parameters, "end-time", text);
        }
    }

    if (resultsType == NB_MRT_CarouselLite)
    {
        error = error ? error : AddSearchFilterKeyValue(parameters, "proxmatch", "1");
        SetSearchResultStyle(parameters, "carousel-lite");
    }

    return error;
}

static NB_Error
AddFiltersForMovieTheaterSearch(NB_SearchParameters* parameters,
                                uint32 startTime,
                                uint32 endTime,
                                NB_MovieTheaterResultsType resultsType)
{
    NB_Error error = NE_OK;
    char text[64]; /* temp buffer */

    // Add start and end time.
    nsl_sprintf(text, "%u", startTime);
    error = error ? error : AddSearchFilterKeyValue(parameters, "start-time", text);
    nsl_sprintf(text, "%u", endTime);
    error = error ? error : AddSearchFilterKeyValue(parameters, "end-time", text);

    return error;
}


uint32 NB_SearchParametersGetAnalyticsEventId(NB_SearchParameters* parameters)
{
    return parameters->analyticsEventId;
}

nb_boolean NB_SearchParametersHasAnalyticsEventId(NB_SearchParameters* parameters)
{
    return parameters->hasValidAnalyticsEventId;
}

NB_DEF NB_Error
NB_SearchParametersAddExtendedKeyValue(NB_SearchParameters* pThis, NB_POIExtendedConfiguration configuration, const char* key, const char* value)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;

    if (!pThis || !key || !value || configuration == NB_EXT_None)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    if (configuration & NB_EXT_WantPremiumPlacement)
    {
        err = err ? err : data_proxpoi_query_add_premium_pair(dataState, &pThis->dataQuery, key, value);
    }

    if (configuration & NB_EXT_WantEnhancedPOI)
    {
        err = err ? err : data_proxpoi_query_add_enhanced_pair(dataState, &pThis->dataQuery, key, value);
    }

    if (configuration & NB_EXT_WantFormattedText)
    {
        err = err ? err : data_proxpoi_query_add_formatted_pair(dataState, &pThis->dataQuery, key, value);
    }

    return err;
}

NB_DEF NB_Error
NB_SearchParametersAddExtendedImage(NB_SearchParameters* pThis, NB_POIExtendedConfiguration configuration, NB_ImageFormat format, NB_ImageSize* size, uint16 dpi)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;

    if (!pThis || !size || configuration == NB_EXT_None)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    if (configuration & NB_EXT_WantPremiumPlacement)
    {
        err = err ? err : SetImage(dataState, &pThis->dataQuery.premium_placement_image, format, size, dpi);
    }

    if (configuration & NB_EXT_WantEnhancedPOI)
    {
        err = err ? err : SetImage(dataState, &pThis->dataQuery.enhanced_pois_image, format, size, dpi);
    }

    if (configuration & NB_EXT_WantFormattedText)
    {
        err = err ? err : SetImage(dataState, &pThis->dataQuery.formatted_image, format, size, dpi);
    }

    return err;
}

NB_Error
SetImage(data_util_state* dataState, data_image* image, NB_ImageFormat format, NB_ImageSize* size, uint16 dpi)
{
    image->height = (uint16)size->height;
    image->width = (uint16)size->width;
    image->dpi = dpi;

    return data_string_set(dataState, &image->format, GetImageFormatString(format));
}

NB_DEF NB_Error
NB_SearchParametersSetSource(NB_SearchParameters* pThis, NB_SearchSource source)
{
    NB_Error result = NE_OK;
    const char* sourceValue = NULL;

    if (!pThis)
    {
        return NE_INVAL;
    }

    switch (source)
    {
    case NB_SS_User:
        {
            sourceValue = "user";
        break;
        }
    case NB_SS_Carousel:
        {
            sourceValue = "carousel";
        break;
        }
    case NB_SS_Navigation:
        {
            sourceValue = "navigation";
        break;
        }
    default:
        {
            return NE_INVAL;
        break;
    }
    }

    if (!sourceValue)
    {
        // The 'sourceValue' is set above.
        return NE_UNEXPECTED;
    }

    if (pThis->searchSource == source)
    {
        // The search source was set before.
        return NE_OK;
    }

    result = AddSearchFilterKeyValue(pThis, "source", sourceValue);
    if (result != NE_OK)
    {
        return result;
    }

    pThis->searchSource = source;

    return result;
}

NB_DEF NB_Error
NB_SearchParametersGetSource(NB_SearchParameters* pThis, NB_SearchSource* source)
{
    if ((pThis == NULL) || (source == NULL))
    {
        return NE_INVAL;
    }

    *source = pThis->searchSource;

    return NE_OK;
}

NB_DEF NB_Error
NB_SearchParametersSetSearchSortBy(NB_SearchParameters* pThis, NB_SearchSortBy sortBy)
{
    NB_Error err = NE_OK;

    if (pThis == NULL)
    {
        return NE_INVAL;
    }

    switch (sortBy)
    {
        case NB_SSB_Distance:
            err = err ? err : AddSearchFilterKeyValue(pThis, "sort-by", "d");
            break;
        case NB_SSB_Relevance:
            err = err ? err : AddSearchFilterKeyValue(pThis, "sort-by", "r");
            break;
        default:
            break;
    }

    return err;
}

NB_DEF NB_Error
NB_SearchParametersSetSearchInputMethod(NB_SearchParameters* pThis, NB_SearchInputMethod inputMethod)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->inputMethod = inputMethod;

    return NE_OK;
}

NB_DEF NB_SearchInputMethod
NB_SearchParametersGetSearchInputMethod(NB_SearchParameters* pThis)
{
    if (!pThis)
    {
        return NB_SIM_Undefined;
    }

    return pThis->inputMethod;
}

NB_DEF NB_Error
NB_SearchParametersEnforceSliceSize(NB_SearchParameters* pThis)
{
    if (pThis == NULL)
    {
        return NE_INVAL;
    }

    pThis->dataQuery.enforce_slice_size = TRUE;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SearchParametersSetSearchRegion(NB_SearchParameters* pThis, NB_SearchRegion* region)
{
    if (!pThis || !region)
    {
        return NE_INVAL;
    }

    return SetSearchLocation(pThis, region);
}
