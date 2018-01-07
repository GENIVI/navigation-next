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

    @file     nbsinglesearchparameters.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsinglesearchparameters.h"
#include "nbsinglesearchinformationprivate.h"
#include "nbsinglesearchparametersprivate.h"
#include "nbcontextprotected.h"
#include "nbutility.h"
#include "nbsuggestmatchprivate.h"
#include "transformutility.h"
#include "data_search_query.h"
#include "data_pair.h"
#include "nbrouteinformationprivate.h"
#include "data_content_type.h"

/*! @{ */

// Constants .........................................................................................................

// @todo: There are some repeated definitions in both 'nbsinglesearchinformation.c' and
//        'nbsinglesearchparameters.c'. Add a private header file named 'nbsinglesearchtypes.h'
//        if it is necessary.

// Result style strings
#define RESULT_STYLE_SINGLE_SEARCH      "single-search"
#define RESULT_STYLE_SUGGEST            "suggest"
#define RESULT_STYLE_GEOCODE            "geocode"
#define RESULT_STYLE_AIRPORT            "airport"

// Iteration command strings
#define ITERATION_COMMAND_START         "start"
#define ITERATION_COMMAND_NEXT          "next"
#define ITERATION_COMMAND_PREV          "prev"

// Postion variant strings
#define POSITION_VARIANT_POINT          "point"
#define POSITION_VARIANT_GPS            "gps"
#define POSITION_GEOGRAPHIC_POSITION    "geographic-position"

// Pair key strings
#define PAIR_KEY_NAME                   "name"
#define PAIR_KEY_SOURCE                 "source"
#define PAIR_KEY_SORT_BY                "sort-by"

// Pair value strings
#define PAIR_VALUE_ADDRESS              "address-screen"
#define PAIR_VALUE_CAROUSEL             "main-screen"
#define PAIR_VALUE_PLACE                "place-screen"
#define PAIR_VALUE_AIRPORT              "airport-screen"
#define PAIR_VALUE_DEFAULT_LOCATION     "default-location"

#define PAIR_VALUE_RELEVANCE            "r"
#define PAIR_VALUE_DISTANCE             "d"

#define WANT_CONTENT_TYPE_TRAFFIC_INCIDENT "traffic-incident"

// Local types .......................................................................................................

/*! The single search parameters */
struct NB_SingleSearchParameters
{
    NB_Context*                     context;                        /*!< Pointer to current context */
    NB_SearchInputMethod            inputMethod;                    /*!< Input method for analytics */
    uint32                          analyticsEventId;               /*!< The analytics event id */
    nb_boolean                      isAnalyticsEventIdValid;        /*!< Is the analytics event id valid? */
    nb_boolean                      withCorridor;                   /*!< search query contain or not corridor filed. */
    nb_boolean                      wantDataAvailability;           /*!< country results shall include 'data-availability' in the proxmatch element. */
    NB_SearchSource                 searchSource;                   /*!< Search Source. */
    data_search_query               query;                          /*!< The search query */
};


// Declaration of local functions ....................................................................................

static NB_Error CreateSingleSearchParameters(NB_Context* context, NB_SingleSearchParameters** parameters);

static NB_Error SetIterationCommand(NB_SingleSearchParameters* parameters, NB_IterationCommand iterationCommand);
static NB_Error SetSliceSize(NB_SingleSearchParameters* parameters, uint32 sliceSize);
static NB_Error SetUserLocation(NB_SingleSearchParameters* parameters, NB_Location* location);
static NB_Error SetSearchRegion(NB_SingleSearchParameters* parameters, NB_SearchRegion* region);
static NB_Error SetSearchLanguage(NB_SingleSearchParameters* parameters, const char* language);
static NB_Error SetSearchScheme(NB_SingleSearchParameters* parameters, const char* scheme);
static NB_Error SetExtendedConfiguration(NB_SingleSearchParameters* parameters, NB_POIExtendedConfiguration extendedConfiguration);
static NB_Error SetSearchResultStyle(NB_SingleSearchParameters* parameters, NB_SearchResultStyleType resultStyleType);

static NB_Error UpdateSearchFilterKeyValue(NB_SingleSearchParameters* parameters, const char* key, const char* value);
static NB_Error AddSearchFilterKeyValue(NB_SingleSearchParameters* parameters, const char* key, const char* value);

static NB_Error AddWantContentType(NB_SingleSearchParameters* parameters, const char* type);

// Public functions ..................................................................................................

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersCreateByResultStyleType(NB_Context* context,
                                                 NB_SearchRegion* region,
                                                 const char* name,
                                                 const char* searchScheme,
                                                 uint32 sliceSize,
                                                 NB_POIExtendedConfiguration extendedConfiguration,
                                                 const char* language,
                                                 NB_SearchResultStyleType resultStyleType,
                                                 NB_SingleSearchParameters** parameters)
{
    NB_Error result = NE_OK;
    NB_SingleSearchParameters* pThis = NULL;

    if (!context || !region || !searchScheme || !parameters)
    {
        return NE_INVAL;
    }

    result = result ? result : CreateSingleSearchParameters(context, &pThis);

    result = result ? result : SetIterationCommand(pThis, NB_IterationCommand_Start);
    result = result ? result : SetSearchRegion(pThis, region);

    if (name)
    {
        result = result ? result : AddSearchFilterKeyValue(pThis, PAIR_KEY_NAME, name);
    }

    result = result ? result : SetSearchScheme(pThis, searchScheme);
    result = result ? result : SetSliceSize(pThis, sliceSize);
    result = result ? result : SetExtendedConfiguration(pThis, extendedConfiguration);

    // The language can be NULL because it is optional. If the argument 'language' is NULL
    // keep the attribute 'language' of 'search-query' NULL. So we could check this attribute
    // when converting to TPS. See the function 'data_search_query_to_tps' for detail.
    if (language)
    {
        result = result ? result : SetSearchLanguage(pThis, language);
    }

    result = result ? result : SetSearchResultStyle(pThis, resultStyleType);

    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    *parameters = pThis;

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersCreateBySuggestion(NB_Context* context,
                                            NB_SearchRegion* region,
                                            const char* name,
                                            const char* searchScheme,
                                            uint32 sliceSize,
                                            NB_POIExtendedConfiguration extendedConfiguration,
                                            const char* language,
                                            NB_SuggestMatch* suggestMatch,
                                            NB_SingleSearchParameters** parameters)
{
    NB_Error result = NE_OK;
    NB_SingleSearchParameters* pThis = NULL;

    if (!context || !region || !searchScheme || !suggestMatch || !parameters)
    {
        return NE_INVAL;
    }

    result = result ? result : CreateSingleSearchParameters(context, &pThis);

    result = result ? result : SetIterationCommand(pThis, NB_IterationCommand_Start);
    result = result ? result : SetSearchRegion(pThis, region);

    if (name)
    {
        result = result ? result : AddSearchFilterKeyValue(pThis, PAIR_KEY_NAME, name);
    }

    result = result ? result : SetSearchScheme(pThis, searchScheme);
    result = result ? result : SetSliceSize(pThis, sliceSize);
    result = result ? result : SetExtendedConfiguration(pThis, extendedConfiguration);

    // The language can be NULL because it is optional. If the argument 'language' is NULL
    // keep the attribute 'language' of 'search-query' NULL. So we could check this attribute
    // when converting to TPS. See the function 'data_search_query_to_tps' for detail.
    if (language)
    {
        result = result ? result : SetSearchLanguage(pThis, language);
    }

    result = result ? result : NB_SuggestMatchCopySearchFilter(suggestMatch, &(pThis->query.search_filter));

    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    /* @todo: Try to get the search source from this 'NB_SuggestMatch' object. Can not add
              this logic at present. Because 'NB_SS_Navigation' and 'NB_SS_Place' use the
              same string ('place-screen'). I think they are different strings when single
              search of server is finished. So the code of application (iPhone) should always
              set the search source when send request of single search before this logic added.
    */

    *parameters = pThis;

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersCreateByIteration(NB_Context* context,
                                           NB_SingleSearchInformation* information,
                                           NB_IterationCommand iterationCommand,
                                           NB_SingleSearchParameters** parameters)
{
    NB_Error result = NE_OK;
    uint32 analyticsEventId = 0;
    data_util_state* state = NULL;
    data_search_query* query = NULL;
    NB_SingleSearchParameters* pThis = NULL;

    if (!context || !information || !parameters)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    result = CreateSingleSearchParameters(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    // Copy the analytics event id from the last single search result
    result = NB_SingleSearchInformationGetAnalyticsEventId(information, &analyticsEventId);
    if (result == NE_OK)
    {
        pThis->analyticsEventId = analyticsEventId;
        pThis->isAnalyticsEventIdValid = TRUE;
    }

    // Ignore the error NE_NOENT returned from the function 'NB_SingleSearchInformationGetAnalyticsEventId'
    result = (result == NE_NOENT) ? NE_OK : result;
    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    // Copy the last search query
    query = NB_SingleSearchInformationGetQuery(information);
    if (!query)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return NE_UNEXPECTED;
    }
    result = data_search_query_copy(state, &(pThis->query), query);
    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    if (iterationCommand != NB_IterationCommand_Start)
    {
        // Copy the last iteration state
        data_blob* iterationState = NB_SingleSearchInformationGetIterationState(information);
        if (!iterationState)
        {
            NB_SingleSearchParametersDestroy(pThis);
            return NE_UNEXPECTED;
        }
        result = result ? result : data_blob_copy(state, &(pThis->query.iter_command.state), iterationState);
    }

    // Set the iteration command
    result = result ? result : SetIterationCommand(pThis, iterationCommand);

    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    pThis->searchSource = NB_SingleSearchInformationGetSource(information);

    *parameters = pThis;

    return result;
}

/* See header file for description */
NB_DEF void
NB_SingleSearchParametersDestroy(NB_SingleSearchParameters* parameters)
{
    data_util_state* state = NULL;

    if (!parameters)
    {
        return;
    }

    state = NB_ContextGetDataState(parameters->context);

    data_search_query_free(state, &(parameters->query));

    nsl_free(parameters);
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersGetSource(NB_SingleSearchParameters* parameters,
                                   NB_SearchSource* source)
{
    if (!parameters || !source)
    {
        return NE_INVAL;
    }

    *source = parameters->searchSource;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetSearchWithCorridor(NB_SingleSearchParameters* parameters, nb_boolean withCorridor)
{
    if (!parameters)
    {
        return NE_INVAL;
    }

    parameters->withCorridor = withCorridor;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetSource(NB_SingleSearchParameters* parameters,
                                   NB_SearchSource source)
{
    NB_Error result = NE_OK;
    const char* sourceValue = NULL;

    if (!parameters)
    {
        return NE_INVAL;
    }

    switch (source)
    {
        case NB_SS_Address:
        {
            sourceValue = PAIR_VALUE_ADDRESS;
            break;
        }
        case NB_SS_Carousel:
        {
            sourceValue = PAIR_VALUE_CAROUSEL;
            break;
        }
        case NB_SS_Place:
        case NB_SS_Navigation:
        {
            sourceValue = PAIR_VALUE_PLACE;
            break;
        }
        case NB_SS_Airport:
        {
            sourceValue = PAIR_VALUE_AIRPORT;
            break;
        }
       case NB_SS_DefaultLocation:
        {
            sourceValue = PAIR_VALUE_DEFAULT_LOCATION;
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

    if (parameters->searchSource == source)
    {
        // The search source was set before.
        return NE_OK;
    }

    result = UpdateSearchFilterKeyValue(parameters, PAIR_KEY_SOURCE, sourceValue);
    if (result != NE_OK)
    {
        return result;
    }

    parameters->searchSource = source;

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetSearchSortBy(NB_SingleSearchParameters* parameters,
                                         NB_SearchSortBy sortBy)
{
    NB_Error result = NE_OK;

    if (!parameters)
    {
        return NE_INVAL;
    }

    switch (sortBy)
    {
        case NB_SSB_Relevance:
        {
            result = AddSearchFilterKeyValue(parameters, PAIR_KEY_SORT_BY, PAIR_VALUE_RELEVANCE);
            break;
        }
        case NB_SSB_Distance:
        {
            result = AddSearchFilterKeyValue(parameters, PAIR_KEY_SORT_BY, PAIR_VALUE_DISTANCE);
            break;
        }
        default:
        {
            return NE_INVAL;
            break;
        }
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetOrigin(NB_SingleSearchParameters* parameters,
        NB_Location* location)
{
    NB_Error result = NE_OK;

    if (!parameters)
    {
        return NE_INVAL;
    }

    result = result ? result : SetUserLocation(parameters, location);

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersGetSearchInputMethod(NB_SingleSearchParameters* parameters,
                                              NB_SearchInputMethod* inputMethod)
{
    if (!parameters || !inputMethod)
    {
        return NE_INVAL;
    }

    *inputMethod = parameters->inputMethod;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetSearchInputMethod(NB_SingleSearchParameters* parameters,
                                              NB_SearchInputMethod inputMethod)
{
    if (!parameters)
    {
        return NE_INVAL;
    }

    parameters->inputMethod = inputMethod;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersAddExtendedKeyValue(NB_SingleSearchParameters* parameters,
                                             NB_POIExtendedConfiguration extendedConfiguration,
                                             const char* key,
                                             const char* value)
{
    NB_Error result = NE_OK;
    nb_boolean isAdded = FALSE;
    data_util_state* state = NULL;

    if (!parameters || !key || !value)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    if (extendedConfiguration & NB_EXT_WantPremiumPlacement)
    {
        result = result ? result : data_search_query_add_premium_pair(state, &(parameters->query), key, value);
        isAdded = TRUE;
    }

    if (extendedConfiguration & NB_EXT_WantEnhancedPOI)
    {
        result = result ? result : data_search_query_add_enhanced_pair(state, &(parameters->query), key, value);
        isAdded = TRUE;
    }

    if (extendedConfiguration & NB_EXT_WantFormattedText)
    {
        result = result ? result : data_search_query_add_formatted_pair(state, &(parameters->query), key, value);
        isAdded = TRUE;
    }

    if (!isAdded)
    {
        // The argument 'extendedConfiguration' is invalid
        return NE_INVAL;
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersAddExtendedImage(NB_SingleSearchParameters* parameters,
                                          NB_POIExtendedConfiguration extendedConfiguration,
                                          NB_ImageFormat format,
                                          NB_ImageSize* size,
                                          uint16 dpi)
{
    NB_Error result = NE_OK;
    nb_boolean isAdded = FALSE;
    const char* formatString = NULL;
    data_util_state* state = NULL;

    if (!parameters || !size)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    formatString = GetImageFormatString(format);
    if (!formatString)
    {
        return NE_UNEXPECTED;
    }

    if (extendedConfiguration & NB_EXT_WantPremiumPlacement)
    {
        result = result ? result : data_string_set(state, &(parameters->query.premium_placement_image.format), formatString);
        parameters->query.premium_placement_image.width = (uint16) size->width;
        parameters->query.premium_placement_image.height = (uint16) size->height;
        parameters->query.premium_placement_image.dpi = dpi;

        isAdded = TRUE;
    }

    if (extendedConfiguration & NB_EXT_WantEnhancedPOI)
    {
        result = result ? result : data_string_set(state, &(parameters->query.enhanced_pois_image.format), formatString);
        parameters->query.enhanced_pois_image.width = (uint16) size->width;
        parameters->query.enhanced_pois_image.height = (uint16) size->height;
        parameters->query.enhanced_pois_image.dpi = dpi;

        isAdded = TRUE;
    }

    if (extendedConfiguration & NB_EXT_WantFormattedText)
    {
        result = result ? result : data_string_set(state, &(parameters->query.formatted_image.format), formatString);
        parameters->query.formatted_image.width = (uint16) size->width;
        parameters->query.formatted_image.height = (uint16) size->height;
        parameters->query.formatted_image.dpi = dpi;

        isAdded = TRUE;
    }

    if (!isAdded)
    {
        // The argument 'extendedConfiguration' is invalid
        return NE_INVAL;
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersAddSearchFilterKeyValue(NB_SingleSearchParameters* parameters,
                                                 const char* key,
                                                 const char* value)
{
    return AddSearchFilterKeyValue(parameters, key, value);
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersSetWantDataAvailability(NB_SingleSearchParameters* parameters, nb_boolean wantDataAvailability)
{
    if (!parameters)
    {
        return NE_INVAL;
    }

    parameters->wantDataAvailability = wantDataAvailability;
    return NE_OK;
}

NB_DEC NB_Error
NB_SingleSearchParametersSetSliceSize(NB_SingleSearchParameters* parameters, uint32 sliceSize)
{
    return SetSliceSize(parameters, sliceSize);
}


// Private functions .................................................................................................

/* See 'nbsinglesearchparametersprivate.h' for description */
tpselt
NB_SingleSearchParametersToTPSQuery(NB_SingleSearchParameters* parameters)
{
    // @todo: Add the attribute of 'route-id' when supported
    NB_Error result = NE_OK;

    data_util_state* state = NULL;

    if (!parameters)
    {
        return NULL;
    }

    state = NB_ContextGetDataState(parameters->context);

    parameters->query.want_data_availability = parameters->wantDataAvailability;

    // @todo: Add the attribute of 'route-id' when supported
    // If route id was specified for the single search parameters and the search source
    // is 'NB_SS_Navigation', move the route-id to the route-corridor's route-id.
    if (parameters->query.routeId.size > 0 && parameters->withCorridor)
    {
        if (parameters->searchSource == NB_SS_Navigation)
        {
            result = data_blob_copy(state, &(parameters->query.route_corridor.route_id), &(parameters->query.routeId));
            data_blob_free(state, &(parameters->query.routeId));
        }
    }
    return data_search_query_to_tps(state, &(parameters->query));
}

/* See 'nbsinglesearchparametersprivate.h' for description */
NB_Error
NB_SingleSearchParametersCopyQuery(NB_SingleSearchParameters* parameters,
                                   data_search_query* query)
{
    data_util_state* state = NULL;

    if (!parameters || !query)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    return data_search_query_copy(state, query, &(parameters->query));
}

/* See 'nbsinglesearchparametersprivate.h' for description */
NB_Error
NB_SingleSearchParametersGetAnalyticsEventId(NB_SingleSearchParameters* parameters,
                                             uint32* analyticsEventId)
{
    if (!parameters || !analyticsEventId)
    {
        return NE_INVAL;
    }

    if (!(parameters->isAnalyticsEventIdValid))
    {
        return NE_NOENT;
    }

    *analyticsEventId = parameters->analyticsEventId;

    return NE_OK;
}

// Definition of local functions .....................................................................................

/*! Create a new instance of a NB_SingleSearchParameters object

    Use this function to create a new instance of a NB_SingleSearchParameters object.
    Call NB_SingleSearchParametersDestroy() to destroy the object.

    @return NE_OK if success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchParametersDestroy
*/
NB_Error CreateSingleSearchParameters(NB_Context* context,                              /*!< Pointer to current context */
                                      NB_SingleSearchParameters** parameters            /*!< On success, returns pointer to single search parameters */
                                      )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SingleSearchParameters* pThis = NULL;

    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    pThis->inputMethod = NB_SIM_Undefined;
    pThis->analyticsEventId = 0;
    pThis->isAnalyticsEventIdValid = FALSE;
    pThis->withCorridor = FALSE;
    pThis->wantDataAvailability = FALSE;

    result = result ? result : data_search_query_init(state, &(pThis->query));
    if (result != NE_OK)
    {
        NB_SingleSearchParametersDestroy(pThis);
        return result;
    }

    *parameters = pThis;

    return result;
}

/*! Set the iteration command

    @return NE_OK if success
*/
NB_Error
SetIterationCommand(NB_SingleSearchParameters* parameters,                              /*!< Single search parameters instance */
                    NB_IterationCommand iterationCommand                                /*!< The iteration command to set */
                    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;

    if (!parameters)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    switch (iterationCommand)
    {
        case NB_IterationCommand_Start:
        {
            result = data_string_set(state, &(parameters->query.iter_command.command), ITERATION_COMMAND_START);
            break;
        }
        case NB_IterationCommand_Next:
        {
            result = data_string_set(state, &(parameters->query.iter_command.command), ITERATION_COMMAND_NEXT);
            break;
        }
        case NB_IterationCommand_Previous:
        {
            result = data_string_set(state, &(parameters->query.iter_command.command), ITERATION_COMMAND_PREV);
            break;
        }
        default:
        {
            result = NE_INVAL;
            break;
        }
    }

    return result;
}

/*! Set the slice size

    @return NE_OK if success
*/
NB_Error
SetSliceSize(NB_SingleSearchParameters* parameters,                                     /*!< Single search parameters instance */
             uint32 sliceSize                                                           /*!< The slice size to set */
             )
{
    if (!parameters)
    {
        return NE_INVAL;
    }

    parameters->query.iter_command.number = sliceSize;

    return NE_OK;
}


/*! Set the user location

    @return NE_OK if success
*/
NB_Error
SetUserLocation(NB_SingleSearchParameters* parameters,                                     /*!< Single search parameters instance */
        NB_Location* location                                                              /*!< User location to set */
             )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    if (!parameters || !location)
    {
        return NE_INVAL;
    }
    state = NB_ContextGetDataState(parameters->context);
    result = data_string_set(state, &(parameters->query.user_position.variant), POSITION_GEOGRAPHIC_POSITION);
    parameters->query.user_position.geographic_position.latitude=location->latitude;
    parameters->query.user_position.geographic_position.longitude=location->longitude;
    if (location->latitude == INVALID_LATITUDE || location->longitude == INVALID_LONGITUDE)
    {
        parameters->query.need_user_position = FALSE;
    }
    else
    {
        parameters->query.need_user_position = TRUE;
    }

    return NE_OK;
}
/*! Set the search region

    @return NE_OK if success
*/
NB_Error
SetSearchRegion(NB_SingleSearchParameters* parameters,                                  /*!< Single search parameters instance */
                NB_SearchRegion* region                                                 /*!< The search region to set */
                )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;

    if (!parameters || !region)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    // @todo: Add the attribute of 'route-id' when supported
    data_blob_free(state, &(parameters->query.routeId));
    data_blob_free(state, &(parameters->query.route_corridor.route_id));

    switch (region->type)
    {
        case NB_ST_Center:
        {
            if (!IsLatitudeLongitudeValid(&(region->center)))
            {
                return NE_INVAL;
            }

            result = data_string_set(state, &(parameters->query.position.variant), POSITION_VARIANT_POINT);
            if (result != NE_OK)
            {
                return result;
            }

            parameters->query.position.point.lat = region->center.latitude;
            parameters->query.position.point.lon = region->center.longitude;

            break;
        }
        case NB_ST_RouteAndCenter:
        {
            // @todo: Add the attribute of 'route-id' when supported
            data_blob* routeId = NULL;

            if (!IsLatitudeLongitudeValid(&(region->center)))
            {
                return NE_INVAL;
            }

            // @todo: Add the attribute of 'route-id' when supported
            result = NB_RouteInformationGetID(region->route, &routeId);
            result = result ? result : data_blob_copy(state, &(parameters->query.routeId), routeId);

            result = result ? result : data_string_set(state, &(parameters->query.position.variant), POSITION_VARIANT_POINT);
            if (result != NE_OK)
            {
                return result;
            }

            parameters->query.position.point.lat = region->center.latitude;
            parameters->query.position.point.lon = region->center.longitude;

            parameters->query.route_corridor.distance = region->distance;
            parameters->query.route_corridor.width = region->width;

            break;
        }
        case NB_ST_DirectionAndCenter:
        {
            NB_GpsLocation gpsLocation = {0};

            gpsfix_clear(&gpsLocation);

            if (!IsLatitudeLongitudeValid(&(region->center)))
            {
                return NE_INVAL;
            }

            result = data_string_set(state, &(parameters->query.position.variant), POSITION_VARIANT_GPS);
            if (result != NE_OK)
            {
                return result;
            }

            gpsLocation.latitude = region->center.latitude;
            gpsLocation.longitude = region->center.longitude;
            gpsLocation.heading = region->direction.heading;
            gpsLocation.horizontalVelocity = region->direction.speed;

            parameters->query.directed = TRUE;

            if (gpsLocation.latitude != INVALID_LATLON)
            {
                gpsLocation.valid |= NGV_Latitude;
            }
            if (gpsLocation.longitude != INVALID_LATLON)
            {
                gpsLocation.valid |= NGV_Longitude;
            }
            if (gpsLocation.heading != INVALID_HEADING)
            {
                gpsLocation.valid |= NGV_Heading;
            }
            gpsLocation.valid |= NGV_HorizontalVelocity;

            data_gps_from_gpsfix(state,&(parameters->query.position.gps), &gpsLocation);
            parameters->query.directed = TRUE;

            break;
        }
        case NB_ST_BoundingBox:
        {
            if (!IsLatitudeLongitudeValid(&(region->boundingBox.topLeft)) ||
                !IsLatitudeLongitudeValid(&(region->boundingBox.bottomRight)))
            {
                return NE_INVAL;
            }

            parameters->query.position.boundingBox.topLeft.lat = region->boundingBox.topLeft.latitude;
            parameters->query.position.boundingBox.topLeft.lon = region->boundingBox.topLeft.longitude;
            parameters->query.position.boundingBox.bottomRight.lat = region->boundingBox.bottomRight.latitude;
            parameters->query.position.boundingBox.bottomRight.lon = region->boundingBox.bottomRight.longitude;
            parameters->query.position.boxValid = TRUE;

            break;
        }
        case NB_ST_None:
        default:
        {
            return NE_INVAL;
            break;
        }
    }

    return result;
}

/*! Set the search language

    @return NE_OK if success
*/
NB_Error
SetSearchLanguage(NB_SingleSearchParameters* parameters,                                /*!< Single search parameters instance */
                  const char* language                                                  /*!< The language to set */
                  )
{
    data_util_state* state = NULL;

    if (!parameters || !language)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    return data_string_set(state , &(parameters->query.language), language);
}

/*! Set the search scheme

    @return NE_OK if success
*/
NB_Error
SetSearchScheme(NB_SingleSearchParameters* parameters,                                  /*!< Single search parameters instance */
                const char* scheme                                                      /*!< The scheme to set */
                )
{
    data_util_state* state = NULL;

    if (!parameters || !scheme)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    return data_string_set(state , &(parameters->query.scheme), scheme);
}

/*! Set the extended configuration

    @return NE_OK if success
*/
NB_Error
SetExtendedConfiguration(NB_SingleSearchParameters* parameters,                         /*!< Single search parameters instance */
                         NB_POIExtendedConfiguration extendedConfiguration              /*!< Extended configuration to set */
                         )
{
    if (!parameters)
    {
        return NE_INVAL;
    }

    parameters->query.want_premium_placement = (extendedConfiguration & NB_EXT_WantPremiumPlacement) ? TRUE : FALSE;
    parameters->query.want_enhanced_pois = (extendedConfiguration & NB_EXT_WantEnhancedPOI) ? TRUE : FALSE;
    parameters->query.want_formatted = (extendedConfiguration & NB_EXT_WantFormattedText) ? TRUE : FALSE;
    parameters->query.want_spelling_suggestions = (extendedConfiguration & NB_EXT_WantSpellingSuggestions) ? TRUE : FALSE;
    parameters->query.want_suggest_distance = (extendedConfiguration & NB_EXT_WantSuggestDistance) ? TRUE : FALSE;
    parameters->query.want_distance_to_user = (extendedConfiguration & NB_EXT_WantDistanceToUser) ? TRUE : FALSE;
    parameters->query.want_suggest_three_lines = (extendedConfiguration & NB_EXT_WantSuggestThreeLines) ? TRUE: FALSE;
    parameters->query.want_country_name = (extendedConfiguration & NB_EXT_WantCountryName) ? TRUE : FALSE;
    parameters->query.want_extended_address = (extendedConfiguration & NB_EXT_WantExtendedAddress) ? TRUE: FALSE;
    parameters->query.want_compact_address = (extendedConfiguration & NB_EXT_WantCompactAddress) ? TRUE: FALSE;
    parameters->query.want_formatted_phone = (extendedConfiguration & NB_EXT_WantFormattedPhone) ? TRUE: FALSE;
    parameters->query.want_structured_hours_of_operation = (extendedConfiguration & NB_EXT_WantStructuredHoursOfOperation) ? TRUE: FALSE;
    parameters->query.want_parent_category = (extendedConfiguration & NB_EXT_WantParentCategory) ? TRUE: FALSE;
    parameters->query.want_want_content_types = (extendedConfiguration & NB_EXT_WantContentTypes) ? TRUE: FALSE;
    parameters->query.want_icon_id = (extendedConfiguration & NB_EXT_WantIconId) ? TRUE : FALSE;

    return NE_OK;
}

/*! Set the search result style

    @return NE_OK if success
*/
NB_Error
SetSearchResultStyle(NB_SingleSearchParameters* parameters, NB_SearchResultStyleType resultStyleType)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;

    if (!parameters)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    switch (resultStyleType)
    {
        case NB_SRST_SingleSearch:
        {
            result = data_string_set(state, &parameters->query.search_filter.result_style.key, RESULT_STYLE_SINGLE_SEARCH);
            break;
        }
        case NB_SRST_Suggestion:
        {
            result = data_string_set(state, &parameters->query.search_filter.result_style.key, RESULT_STYLE_SUGGEST);
            break;
        }
        case NB_SRST_Geocode:
        {
            result = data_string_set(state, &parameters->query.search_filter.result_style.key, RESULT_STYLE_GEOCODE);
            break;
        }
        case NB_SRST_Airport:
        {
            result = data_string_set(state, &parameters->query.search_filter.result_style.key, RESULT_STYLE_AIRPORT);
            break;
        }
        case NB_SRST_DefaultLocation:
        {
            result = data_string_set(state, &parameters->query.search_filter.result_style.key, RESULT_STYLE_DEFAULT_LOCATION);
            break;
        }
        case NB_SRST_None:
        default:
        {
            return NE_INVAL;
            break;
        }
    }

    return result;
}

/*! Add or update a search filter key/value pair

    @return NE_OK if success
*/
NB_Error
UpdateSearchFilterKeyValue(NB_SingleSearchParameters* parameters,                          /*!< Single search parameters instance */
                        const char* key,                                                /*!< The key name */
                        const char* value                                               /*!< The value */
                        )
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    data_util_state* state = NULL;
    CSL_Vector* pairVector = NULL;

    if (!parameters || !key || !value)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    pairVector = parameters->query.search_filter.vec_pairs;
    if (!pairVector)
    {
        pairVector = CSL_VectorAlloc(sizeof(data_pair));
        if (!pairVector)
        {
            return NE_NOMEM;
        }
        parameters->query.search_filter.vec_pairs = pairVector;
    }

    // Try to find the 'data_pair' object by 'key'.
    length = CSL_VectorGetLength(pairVector);
    for (n = 0; n < length; ++n)
    {
        const char* keyOfPair = NULL;
        const char* valueOfPair = NULL;
        data_pair* dataPair = (data_pair*) CSL_VectorGetPointer(pairVector, n);

        if (!dataPair)
        {
            continue;
        }

        // Compare the key of pair with the parameter 'key'.
        keyOfPair = data_string_get(state, &(dataPair->key));
        if (!keyOfPair || (nsl_strcmp(keyOfPair, key) != 0))
        {
            continue;
        }

        // Get the value related with the specified key.
        valueOfPair = data_string_get(state, &(dataPair->value));
        if (valueOfPair && (nsl_strcmp(valueOfPair, value) == 0))
        {
            // This pair of key and value already exists.
            return NE_OK;
        }

        // Reset the value of pair and return.
        return data_string_set(state, &(dataPair->value), value);
    }

    // Can not find a 'data_pair' object by 'key'. So add a new 'data_pair' object.
    {
        data_pair dataPair = {0};

        result = result ? result : data_pair_init(state, &dataPair);
        result = result ? result : data_string_set(state, &(dataPair.key), key);
        result = result ? result : data_string_set(state, &(dataPair.value), value);

        result = result ? result : (CSL_VectorAppend(pairVector, &dataPair) ? NE_OK : NE_NOMEM);

        if (result != NE_OK)
        {
            data_pair_free(state, &dataPair);
            return NE_OK;
        }

        // If the variable 'dataPair' is reused the function 'data_pair_init' should be called.
        // Because the last 'dataPair' has been added in the above vector.
//        data_pair_init(state, &dataPair);
    }

    return result;
}

/*! Add a search filter key/value pair

 @return NE_OK if success
 */
NB_Error
AddSearchFilterKeyValue(NB_SingleSearchParameters* parameters,                          /*!< Single search parameters instance */
                        const char* key,                                                /*!< The key name */
                        const char* value                                               /*!< The value */
                        )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    data_pair dataPair = {0};
    if (!parameters || !key || !value)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    if (!(parameters->query.search_filter.vec_pairs))
    {
        parameters->query.search_filter.vec_pairs = CSL_VectorAlloc(sizeof(data_pair));
        if (!(parameters->query.search_filter.vec_pairs))
        {
            return NE_NOMEM;
        }
    }

    result = result ? result : data_pair_init(state, &dataPair);
    result = result ? result : data_string_set(state, &(dataPair.key), key);
    result = result ? result : data_string_set(state, &(dataPair.value), value);

    result = result ? result : (CSL_VectorAppend(parameters->query.search_filter.vec_pairs, &dataPair) ? NE_OK : NE_NOMEM);

    if (result != NE_OK)
    {
        data_pair_free(state, &dataPair);
        return NE_OK;
    }

    // If the variable 'dataPair' is reused the function 'data_pair_init' should be called.
    // Because the last 'dataPair' has been added in the above vector.
    //    data_pair_init(state, &dataPair);

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchParametersAddWantContentType(NB_SingleSearchParameters* parameters,
                                            NB_SearchWantContentType contentType)
{
    NB_Error result = NE_OK;

    if (!parameters)
    {
        return NE_INVAL;
    }

    switch (contentType)
    {
        case NB_SWCT_TrafficIncident:
        {
            result = AddWantContentType(parameters, WANT_CONTENT_TYPE_TRAFFIC_INCIDENT);
            break;
        }
        default:
        {
            return NE_INVAL;
            break;
        }
    }

    return result;
}

/*! Add a want content type

 @return NE_OK if success
 */
NB_Error
AddWantContentType(NB_SingleSearchParameters* parameters,                          /*!< Single search parameters instance */
                   const char* type                                                /*!< The content type name */
                   )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    data_content_type contentType = {0};
    if (!parameters || !type)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(parameters->context);

    if (!(parameters->query.want_content_types.vec_want_content_types))
    {
        parameters->query.want_content_types.vec_want_content_types = CSL_VectorAlloc(sizeof(data_content_type));
        if (!(parameters->query.want_content_types.vec_want_content_types))
        {
            return NE_NOMEM;
        }
    }

    data_content_type_init(state, &contentType);
    data_string_set(state, &contentType.type, type);

    result = result ? result : CSL_VectorAppend(parameters->query.want_content_types.vec_want_content_types, &contentType) ? NE_OK : NE_NOMEM;

    return result;
}
/*! @} */
