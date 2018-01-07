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

    @file     nbsinglesearchinformation.c
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

#include "nbsinglesearchinformation.h"
#include "nbcontextprotected.h"
#include "nbutility.h"
#include "nbsuggestmatchprivate.h"
#include "nbsuggestlistprivate.h"
#include "data_search_query.h"
#include "data_search_reply.h"
#include "data_proxmatch.h"
#include "data_suggest_match.h"
#include "data_suggest_list.h"
#include "data_time_range.h"
#include "nbsinglesearchparametersprivate.h"

/*! @{ */

// Constants .........................................................................................................

// @todo: There are some repeated definitions in both 'nbsinglesearchinformation.c' and
//        'nbsinglesearchparameters.c'. Add a private header file named 'nbsinglesearchtypes.h'
//        if it is necessary.

// Result style strings
#define RESULT_STYLE_SINGLE_SEARCH      "single-search"
#define RESULT_STYLE_SUGGEST            "suggest"
#define RESULT_STYLE_LIST_INTEREST_SUGGEST "list-interest-suggest"
#define RESULT_STYLE_GEOCODE            "geocode"
#define RESULT_STYLE_AIRPORT            "airport"

// Pair key strings
#define PAIR_KEY_NAME                   "name"
#define KEY_THUMBNAIL_PHOTO_URL         "thumbnail-photo-url"
#define KEY_PHOTO_URL                   "photo-url"


// Local types .......................................................................................................

/*! The single search information */
struct NB_SingleSearchInformation
{
    NB_Context*                     context;                        /*!< Pointer to current context */
    CSL_Vector*                     extendedPlaceVector;            /*!< Vector of NB_ExtendedPlace */
    uint32                          analyticsEventId;               /*!< The analytics event id */
    nb_boolean                      isAnalyticsEventIdValid;        /*!< Is the analytics event id valid? */
    NB_SearchSource                 searchSource;                   /*!< Search source. */
    data_search_query               query;                          /*!< The search query */
    data_search_reply               reply;                          /*!< The search reply */
};


// Declaration of local functions ....................................................................................

static NB_Error CreateSingleSearchInformation(NB_Context* context, NB_SingleSearchInformation** information);
static NB_Error GetSearchFilterValueByKey(NB_SingleSearchInformation* information, const char* key, uint32 maxValueSize, char* value);


// Public functions ..................................................................................................

/* See header file for description */
NB_DEF void
NB_SingleSearchInformationDestroy(NB_SingleSearchInformation* information)
{
    data_util_state* state = NULL;

    if (!information)
    {
        return;
    }

    state = NB_ContextGetDataState(information->context);

    data_search_query_free(state, &(information->query));
    data_search_reply_free(state, &(information->reply));

    // Free the extended places internal. Ignore the returned error.
    NB_SingleSearchInformationFreeExtendedPlaces(information);

    if (information->extendedPlaceVector)
    {
        CSL_VectorDealloc(information->extendedPlaceVector);
        information->extendedPlaceVector = NULL;
    }

    nsl_free(information);
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetResultStyleType(NB_SingleSearchInformation* information,
                                             NB_SearchResultStyleType* resultStyleType)
{
    const char* resultStyleString = NULL;
    data_util_state* state = NULL;

    if (!information || !resultStyleType)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    resultStyleString = data_string_get(state, &(information->query.search_filter.result_style.key));
    if (!resultStyleString)
    {
        *resultStyleType = NB_SRST_None;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_SINGLE_SEARCH) == 0)
    {
        *resultStyleType = NB_SRST_SingleSearch;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_SUGGEST) == 0)
    {
        *resultStyleType = NB_SRST_Suggestion;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_LIST_INTEREST_SUGGEST) == 0)
    {
        *resultStyleType = NB_SRST_ListInterestSuggestion;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_GEOCODE) == 0)
    {
        *resultStyleType = NB_SRST_Geocode;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_AIRPORT) == 0)
    {
        *resultStyleType = NB_SRST_Airport;
    }
    else if (nsl_strcmp(resultStyleString, RESULT_STYLE_DEFAULT_LOCATION) == 0)
    {
        *resultStyleType = NB_SRST_DefaultLocation;
    }
    else
    {
        *resultStyleType = NB_SRST_None;
    }

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetResultCount(NB_SingleSearchInformation* information,
                                         uint32* count)
{
    NB_Error result = NE_OK;
    NB_SearchResultStyleType resultStyleType = NB_SRST_None;

    if (!information || !count)
    {
        return NE_INVAL;
    }

    result = NB_SingleSearchInformationGetResultStyleType(information, &resultStyleType);
    if (result != NE_OK)
    {
        return result;
    }

    switch (resultStyleType)
    {
        case NB_SRST_SingleSearch:
        case NB_SRST_Geocode:
        case NB_SRST_Airport:
        case NB_SRST_DefaultLocation:
        {
            *count = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
            break;
        }
        case NB_SRST_Suggestion:
        {
            *count = (uint32) CSL_VectorGetLength(information->reply.vector_suggest_match);
            break;
        }
        case NB_SRST_ListInterestSuggestion:
        {
            *count = (uint32) CSL_VectorGetLength(information->reply.vector_suggest_list);
            break;
        }
        case NB_SRST_None:
        default:
        {
            *count = 0;
            break;
        }
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationHasMoreResults(NB_SingleSearchInformation* information,
                                         nb_boolean* hasMoreResults)
{
    if (!information || !hasMoreResults)
    {
        return NE_INVAL;
    }

    *hasMoreResults = !information->reply.iter_result.exhausted;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetResultType(NB_SingleSearchInformation* information,
                                        uint32 resultIndex,
                                        NB_SearchResultType* resultType)
{
    uint32 count = 0;
    NB_SearchResultType searchResultType = NB_SRT_None;
    data_proxmatch* proxmatch = NULL;

    if (!information || !resultType)
    {
        return NE_INVAL;
    }

    count = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, (int) resultIndex);
    if (proxmatch)
    {
        searchResultType |= NB_SRT_Place;

        // @todo: Need to check sub search for single search?
        if (CSL_VectorGetLength(proxmatch->search_filter.vec_pairs) > 0)
        {
            searchResultType |= NB_SRT_SubSearch;
        }

        if ((proxmatch->place.vec_fuel_product) && (CSL_VectorGetLength(proxmatch->place.vec_fuel_product) > 0))
        {
            searchResultType |= NB_SRT_FuelDetails;
        }
    }

    *resultType = searchResultType;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetSuggestMatch(NB_SingleSearchInformation* information,
                                          uint32 resultIndex,
                                          NB_SuggestMatch** suggestMatch)
{
    NB_Error result = NE_OK;
    uint32 count = 0;
    data_suggest_match* dataSuggestMatch = NULL;
    NB_SuggestMatch* newSuggestMatch = NULL;

    if (!information || !suggestMatch || !(information->context))
    {
        return NE_INVAL;
    }

    count = (uint32) CSL_VectorGetLength(information->reply.vector_suggest_match);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    dataSuggestMatch = (data_suggest_match*) CSL_VectorGetPointer(information->reply.vector_suggest_match, (int) resultIndex);
    if (!dataSuggestMatch)
    {
        return NE_NOENT;
    }

    result = NB_SuggestMatchCreateByDataSuggestMatch(information->context, dataSuggestMatch, &newSuggestMatch);
    if (result != NE_OK)
    {
        return result;
    }

    *suggestMatch = newSuggestMatch;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetSuggestList(NB_SingleSearchInformation* information,
                                         uint32 resultIndex,
                                         NB_SuggestList** suggestList)
{
    NB_Error result = NE_OK;
    uint32 count = 0;
    data_suggest_list* dataSuggestList = NULL;
    NB_SuggestList* newSuggestList = NULL;

    if (!information || !suggestList || !(information->context))
    {
        return NE_INVAL;
    }

    count = (uint32) CSL_VectorGetLength(information->reply.vector_suggest_list);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    dataSuggestList = (data_suggest_list*) CSL_VectorGetPointer(information->reply.vector_suggest_list, (int) resultIndex);
    if (!dataSuggestList)
    {
        return NE_NOENT;
    }

    result = NB_SuggestListCreateByDataSuggestList(information->context, dataSuggestList, &newSuggestList);
    if (result != NE_OK)
    {
        return result;
    }

    *suggestList = newSuggestList;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetPlace(NB_SingleSearchInformation* information,
                                   uint32 resultIndex,
                                   NB_Place* place,
                                   double* distance,
                                   NB_ExtendedPlace** extendedPlace)
{
    NB_Error result = NE_OK;
    uint32 count = 0;
    data_util_state* state = NULL;
    data_proxmatch* proxmatch = NULL;

    if (!information || !place || !distance || !extendedPlace)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    count = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, (int) resultIndex);
    if (!proxmatch)
    {
        return NE_NOENT;
    }

    // Set the NB_Place
    nsl_memset(place, 0, sizeof(*place));
    result = SetNIMPlaceFromPlace(place, state, &(proxmatch->place));
    if (result != NE_OK)
    {
        return result;
    }

    // If the extended place pointer was passed in then we also fill in the extended place information.
    // We allocate a dynamic structure here and pass it back to the caller. We save the pointer in a
    // vector so we can free the data in NB_SearchInformationDestroy().
    if (extendedPlace)
    {
        result = CreateNIMExtendedPlace(state, proxmatch, extendedPlace);
        if (result != NE_OK)
        {
            return result;
        }

        // Add the created extended place to the internal vector
        result = CSL_VectorAppend(information->extendedPlaceVector, extendedPlace) ? NE_OK : NE_NOMEM;
        if (result != NE_OK)
        {
            nsl_memset(place, 0, sizeof(*place));
            FreeNIMExtendedPlace(state, *extendedPlace);
            return result;
        }
    }

    // Set the distance. The argument 'distance' is optional
    if (distance)
    {
        *distance = proxmatch->distance;
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationFreeExtendedPlaces(NB_SingleSearchInformation* information)
{
    int n = 0;
    int length = 0;
    data_util_state* state = NULL;

    if (!information)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    // Free all extended places
    length = CSL_VectorGetLength(information->extendedPlaceVector);
    for (n = 0; n < length; ++n)
    {
        NB_ExtendedPlace* extendedPlace = NULL;

        // Get the pointer to NB_ExtendedPlace and free it
        CSL_VectorGet(information->extendedPlaceVector, n, (void*) &extendedPlace);
        FreeNIMExtendedPlace(state, extendedPlace);
    }

    // Remove all items from the vector but do not destroy the vector. It could be reused.
    // It is destroyed in the function NB_SingleSearchInformationDestroy.
    CSL_VectorRemoveAll(information->extendedPlaceVector);

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetFuelDetails(NB_SingleSearchInformation* information,
                                         uint32 resultIndex,
                                         NB_FuelDetails* fuelDetails)
{
    NB_Error result = NE_OK;
    uint32 count = 0;
    int n = 0;
    int length = 0;
    data_util_state* state = NULL;
    data_proxmatch* proxmatch = NULL;

    if (!information || !fuelDetails)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    count = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, (int) resultIndex);
    if (!proxmatch || !(proxmatch->place.vec_fuel_product))
    {
        // The result type is not NB_SRT_FuelDetails
        return NE_NOENT;
    }

    length = CSL_VectorGetLength(proxmatch->place.vec_fuel_product);
    if (length <= 0)
    {
        // The result type is not NB_SRT_FuelDetails
        return NE_NOENT;
    }

    nsl_memset(fuelDetails, 0, sizeof(*fuelDetails));
    fuelDetails->num_fuelproducts = length;

    for (n = 0; n < length; ++n)
    {
        data_fuelproduct* fuelProduct = (data_fuelproduct*) CSL_VectorGetPointer(proxmatch->place.vec_fuel_product, n);
        if (!fuelProduct)
        {
            continue;
        }

        result = SetNIMFuelProdFromFuelProd(&(fuelDetails->fuel_product[n]), state, fuelProduct);
        if (result != NE_OK)
        {
            nsl_memset(fuelDetails, 0, sizeof(*fuelDetails));
            return result;
        }
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetFuelSummary(NB_SingleSearchInformation* information,
                                         NB_FuelSummary* fuelSummary)
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    data_util_state* state = NULL;
    CSL_Vector*	fuelProductVector = NULL;

    if (!information || !fuelSummary)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    fuelProductVector = information->reply.fuel_pricesummary.vec_fuelproducts;
    if (!fuelProductVector)
    {
        // There is no fuel summary returned from server
        return NE_NOENT;
    }

    length = CSL_VectorGetLength(fuelProductVector);
    if (length <= 0)
    {
        // There is no fuel summary returned from server
        return NE_NOENT;
    }

    for (n = 0; n < length; ++n)
    {
        data_fuelproduct* fuelProduct = (data_fuelproduct*) CSL_VectorGetPointer(fuelProductVector, n);
        if (!fuelProduct)
        {
            continue;
        }

        if (fuelProduct->average)
        {
            result = SetNIMFuelProdFromFuelProd(&(fuelSummary->average), state, fuelProduct);
        }
        else if (fuelProduct->low)
        {
            result = SetNIMFuelProdFromFuelProd(&(fuelSummary->low), state, fuelProduct);
        }

        if (result != NE_OK)
        {
            nsl_memset(fuelSummary, 0, sizeof(*fuelSummary));
            return result;
        }
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetAnalyticsEventId(NB_SingleSearchInformation* information,
                                              uint32* analyticsEventId)
{
    if (!information || !analyticsEventId)
    {
        return NE_INVAL;
    }

    if (!(information->isAnalyticsEventIdValid))
    {
        return NE_NOENT;
    }

    *analyticsEventId = information->analyticsEventId;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationIsMappable(NB_SingleSearchInformation* information,
                                     uint32 resultIndex,
                                     nb_boolean* isMappable)
{
    uint32 count = 0;
    data_proxmatch* proxmatch = NULL;

    if (!information || !isMappable)
    {
        return NE_INVAL;
    }

    count = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, (int) resultIndex);
    if (!proxmatch)
    {
        return NE_NOENT;
    }

    *isMappable = (proxmatch->unmappable == 0);

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetRequestedKeyword(NB_SingleSearchInformation* information,
                                              uint32 maxKeywordSize,
                                              char* keyword)
{
    if (!information || (maxKeywordSize <= 0) || !keyword)
    {
        return NE_INVAL;
    }

    return GetSearchFilterValueByKey(information, PAIR_KEY_NAME, maxKeywordSize, keyword);
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetDataAvailability(NB_SingleSearchInformation* information,
                                              uint32 resultIndex,
                                              NB_DataAvailability* dataAvailability)
{
    uint32 count = 0;
    data_proxmatch* proxmatch = NULL;

    if (!information || !dataAvailability)
    {
        return NE_INVAL;
    }

    count = (uint32)CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_RANGE;
    }

    proxmatch = (data_proxmatch*)CSL_VectorGetPointer(information->reply.vector_proxmatch, (int)resultIndex);
    if (!proxmatch || !proxmatch->data_availability)
    {
        return NE_NOENT;
    }

    dataAvailability->noMapData = proxmatch->data_availability->no_map_data;
    dataAvailability->partialVisualMapData = proxmatch->data_availability->partial_visual_map_data;
    dataAvailability->fullVisualMapData = proxmatch->data_availability->full_visual_map_data;
    dataAvailability->partialRoutingData = proxmatch->data_availability->partial_routing_data;
    dataAvailability->fullRoutingData = proxmatch->data_availability->full_routing_data;
    dataAvailability->partialGeocodingData = proxmatch->data_availability->partial_geocoding_data;
    dataAvailability->fullGeocodingData = proxmatch->data_availability->full_geocoding_data;
    dataAvailability->partialPoiData = proxmatch->data_availability->partial_poi_data;
    dataAvailability->fullPoiData = proxmatch->data_availability->full_poi_data;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetDataBox(NB_SingleSearchInformation* information,
                                              uint32 resultIndex,
                                              double* topLeftLatitude,
                                              double* topLeftLongitude,
                                              double* bottomRightLatitude,
                                              double* bottomRightLongitude)
{
    uint32 count = 0;
    data_proxmatch* proxmatch = NULL;

    if (!information )
    {
        return NE_INVAL;
    }

    count = (uint32)CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_RANGE;
    }

    proxmatch = (data_proxmatch*)CSL_VectorGetPointer(information->reply.vector_proxmatch, (int)resultIndex);
    if (!proxmatch || !proxmatch->place.location.box)
    {
        return NE_NOENT;
    }

    *topLeftLatitude = proxmatch->place.location.box->topLeft.lat;
    *topLeftLongitude = proxmatch->place.location.box->topLeft.lon;
    *bottomRightLatitude = proxmatch->place.location.box->bottomRight.lat;
    *bottomRightLongitude = proxmatch->place.location.box->bottomRight.lon;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetParentCategory(NB_SingleSearchInformation* information,
                                            const char* categoryCode,
                                            NB_Category* parentCategory)
{
    int i = 0;
    int resultCount = 0;
    data_util_state* state = NULL;

    if ((!information) ||
        (!categoryCode) ||
        (nsl_strlen(categoryCode) == 0) ||
        (!parentCategory))
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    // Find the category by the specified category code.
    resultCount = CSL_VectorGetLength(information->reply.vector_proxmatch);
    for (i = 0; i < resultCount; ++i)
    {
        int j = 0;
        int categoryCount = 0;
        data_proxmatch* proxmatch = NULL;
        CSL_Vector* categoryVector = NULL;

        proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, i);
        if (!proxmatch)
        {
            continue;
        }

        categoryVector = proxmatch->place.vec_category;
        if (!categoryVector)
        {
            continue;
        }

        categoryCount = CSL_VectorGetLength(categoryVector);
        for (j = 0; j < categoryCount; ++j)
        {
            data_category* category = NULL;

            category = (data_category*) CSL_VectorGetPointer(categoryVector, j);
            if (!category)
            {
                continue;
            }

            // Compare the category codes to check if the specified category is found.
            if (data_string_compare_cstr(&(category->code), categoryCode))
            {
                if (category->hasParentCategory)
                {
                    // Copy the parent category to return.
                    data_string_get_copy(state, &(category->parent_category.code), parentCategory->code, sizeof(parentCategory->code));
                    data_string_get_copy(state, &(category->parent_category.name), parentCategory->name, sizeof(parentCategory->name));
                    return NE_OK;
                }
                else
                {
                    return NE_NOENT;
                }
            }
        }
    }

    return NE_NOENT;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchInformationGetHoursOfOperation(NB_SingleSearchInformation* information,
                                              uint32 resultIndex,
                                              NB_HoursOfOperation* hoursOfOperation)
{
    NB_Error error = NE_OK;
    uint32 resultCount = 0;
    int timeRangeCount = 0;
    data_util_state* state = NULL;
    data_proxmatch* proxmatch = NULL;
    CSL_Vector* dataTimeRangeVector = NULL;
    NB_TimeRange* timeRangeArray = NULL;

    if ((!information) ||
        (!hoursOfOperation))
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    resultCount = (uint32) CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= resultCount)
    {
        return NE_NOENT;
    }

    // Retrieve the indexed search result.
    proxmatch = (data_proxmatch*) CSL_VectorGetPointer(information->reply.vector_proxmatch, (int) resultIndex);
    if ((!proxmatch) ||
        (!(proxmatch->has_poi_content)) ||
        (!(proxmatch->poi_content.hasHoursOfOperation)))
    {
        return NE_NOENT;
    }

    // Retrieve the array of time range.
    dataTimeRangeVector = proxmatch->poi_content.hours_of_operation.vector_time_range;
    timeRangeCount = CSL_VectorGetLength(dataTimeRangeVector);
    if (timeRangeCount > 0)
    {
        int i = 0;

        timeRangeArray = nsl_malloc(sizeof(NB_TimeRange) * timeRangeCount);
        if (!timeRangeArray)
        {
            return NE_NOMEM;
        }
        nsl_memset(timeRangeArray, 0, sizeof(NB_TimeRange) * timeRangeCount);

        for (i = 0; i < timeRangeCount; ++i)
        {
            NB_DayOfWeek dayOfWeek = NB_DOW_Sunday;
            data_time_range* timeRange = NULL;

            timeRange = (data_time_range*) CSL_VectorGetPointer(dataTimeRangeVector, i);
            if (!timeRange)
            {
                continue;
            }

            switch (timeRange->day_of_week)
            {
            case 0:
            {
                dayOfWeek = NB_DOW_Sunday;
                break;
            }
            case 1:
            {
                dayOfWeek = NB_DOW_Monday;
                break;
            }
            case 2:
            {
                dayOfWeek = NB_DOW_Tuesday;
                break;
            }
            case 3:
            {
                dayOfWeek = NB_DOW_Wednesday;
                break;
            }
            case 4:
            {
                dayOfWeek = NB_DOW_Thursday;
                break;
            }
            case 5:
            {
                dayOfWeek = NB_DOW_Friday;
                break;
            }
            case 6:
            {
                dayOfWeek = NB_DOW_Saturday;
                break;
            }
            default:
            {
                error = NE_UNEXPECTED;
                break;
            }
            }

            if (error != NE_OK)
            {
                break;
            }

            timeRangeArray[i].dayOfWeek = dayOfWeek;
            timeRangeArray[i].startTime = timeRange->start_time;
            timeRangeArray[i].endTime = timeRange->end_time;
        }

        if (error != NE_OK)
        {
            nsl_free(timeRangeArray);
            timeRangeArray = NULL;
            return error;
        }
    }

    hoursOfOperation->hasUtcOffset = (nb_boolean) (proxmatch->poi_content.hours_of_operation.hasUtcOffset);
    hoursOfOperation->utcOffset = (int32) (proxmatch->poi_content.hours_of_operation.utc_offset);
    hoursOfOperation->timeRangeCount = timeRangeCount;
    hoursOfOperation->timeRangeArray = timeRangeArray;
    return NE_OK;
}

/* See header file for description */
NB_DEF void
NB_HoursOfOperationDestroy(NB_HoursOfOperation* hoursOfOperation)
{
    if (hoursOfOperation)
    {
        hoursOfOperation->hasUtcOffset = FALSE;
        hoursOfOperation->utcOffset = 0;
        hoursOfOperation->timeRangeCount = 0;
        if (hoursOfOperation->timeRangeArray)
        {
            nsl_free(hoursOfOperation->timeRangeArray);
            hoursOfOperation->timeRangeArray = NULL;
        }
    }
}


// Private functions .................................................................................................

/* See 'nbsinglesearchinformationprivate.h' for description */
NB_Error
NB_SingleSearchInformationCreateFromTPSReply(tpselt reply,
                                             data_search_query* query,
                                             NB_Context* context,
                                             NB_SearchSource searchSource,
                                             NB_SingleSearchInformation** information)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SingleSearchInformation* pThis = NULL;

    if (!reply || !query || !context || !information)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    result = CreateSingleSearchInformation(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    result = result ? result : data_search_query_copy(state, &(pThis->query), query);
    result = result ? result : data_search_reply_from_tps(state, &(pThis->reply), reply);
    if (result != NE_OK)
    {
        NB_SingleSearchInformationDestroy(pThis);
        return result;
    }

    *information = pThis;
    pThis->searchSource = searchSource;

    return result;
}

/* See 'nbsinglesearchinformationprivate.h' for description */
data_blob*
NB_SingleSearchInformationGetIterationState(NB_SingleSearchInformation* information)
{
    if (!information)
    {
        return NULL;
    }

    return &(information->reply.iter_result.state);
}

/* See 'nbsinglesearchinformationprivate.h' for description */
data_search_query*
NB_SingleSearchInformationGetQuery(NB_SingleSearchInformation* information)
{
    if (!information)
    {
        return NULL;
    }

    return &(information->query);
}

/* See 'nbsinglesearchinformationprivate.h' for description */
NB_SearchSource
NB_SingleSearchInformationGetSource(NB_SingleSearchInformation* information)
{
    if(information)
    {
        return information->searchSource;
    }
    return NB_SS_None;
}

/* See 'nbsinglesearchinformationprivate.h' for description */
NB_Error
NB_SingleSearchInformationSetAnalyticsEventId(NB_SingleSearchInformation* information,
                                              uint32 analyticsEventId)
{
    if (!information)
    {
        return NE_INVAL;
    }

    information->analyticsEventId = analyticsEventId;
    information->isAnalyticsEventIdValid = TRUE;

    return NE_OK;
}

/* See 'nbsinglesearchinformationprivate.h' for description */
NB_Error
NB_SingleSearchInformationCopySearchFilterOfSuggestion(NB_SingleSearchInformation* information,
                                                       uint32 resultIndex,
                                                       data_search_filter* searchFilter)
{
    uint32 count = 0;
    data_util_state* state = NULL;
    data_suggest_match* suggestMatch = NULL;

    if (!information || !searchFilter)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    count = (uint32) CSL_VectorGetLength(information->reply.vector_suggest_match);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    suggestMatch = (data_suggest_match*) CSL_VectorGetPointer(information->reply.vector_suggest_match, (int) resultIndex);
    if (!suggestMatch)
    {
        return NE_NOENT;
    }

    return data_search_filter_copy(state, searchFilter, &(suggestMatch->search_filter));
}


// Definition of local functions .....................................................................................

/*! Create a new instance of a NB_SingleSearchInformation object

    Use this function to create a new instance of a NB_SingleSearchInformation object.
    Call NB_SingleSearchInformationDestroy() to destroy the object.

    @return NE_OK if success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchInformationDestroy
*/
NB_Error CreateSingleSearchInformation(NB_Context* context,                             /*!< Pointer to current context */
                                       NB_SingleSearchInformation** information         /*!< On success, returns pointer to single search information */
                                       )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SingleSearchInformation* pThis = NULL;

    if (!context || !information)
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
    pThis->analyticsEventId = 0;
    pThis->isAnalyticsEventIdValid = FALSE;

    pThis->extendedPlaceVector = CSL_VectorAlloc(sizeof(NB_ExtendedPlace*));
    if (!(pThis->extendedPlaceVector))
    {
        NB_SingleSearchInformationDestroy(pThis);
        return NE_NOMEM;
    }

    result = result ? result : data_search_query_init(state, &(pThis->query));
    result = result ? result : data_search_reply_init(state, &(pThis->reply));
    if (result != NE_OK)
    {
        NB_SingleSearchInformationDestroy(pThis);
        return result;
    }

    *information = pThis;

    return result;
}

/*! Get the value of search filter by key

    @return NE_OK if success, NE_NOENT if there is no value for this key, NE_RANGE if 'maxValueSize' is not enough.
*/
NB_Error
GetSearchFilterValueByKey(NB_SingleSearchInformation* information,                      /*!< Single search information instance */
                          const char* key,                                              /*!< The key name */
                          uint32 maxValueSize,                                          /*!< Maximum size of the value. This size contains the end '\0'. */
                          char* value                                                   /*!< On success, returns the value */
                          )
{
    int n = 0;
    int length = 0;
    data_util_state* state = NULL;
    CSL_Vector* pairVector = NULL;

    if (!information || !key || (maxValueSize <= 0) || !value)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    // Clear the returned string 'value'.
    nsl_memset(value, 0, maxValueSize);

    pairVector = information->query.search_filter.vec_pairs;
    if (!pairVector)
    {
        return NE_NOENT;
    }

    length = CSL_VectorGetLength(pairVector);
    if (length <= 0)
    {
        return NE_NOENT;
    }

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
        if (!valueOfPair)
        {
            return NE_NOENT;
        }
        else if (maxValueSize < nsl_strlen(valueOfPair) + 1)
        {
            return NE_RANGE;
        }

        nsl_memset(value, 0, maxValueSize);
        nsl_strlcpy(value, valueOfPair, maxValueSize);

        return NE_OK;
    }

    // The specified key can not be found.
    return NE_NOENT;
}

/*! Refer to header file for more information. */
NB_DEF const char*
NB_SingleSearchInformationGetPoiImageUrl(NB_SingleSearchInformation* information,
                                         uint32 index, nb_boolean wantThumbnail)
{
    data_util_state* state = NULL;
    const char* url = NULL;
    CSL_Vector* vec_proxmatch = information->reply.vector_proxmatch;
    if (vec_proxmatch && (index < (uint32) CSL_VectorGetLength(vec_proxmatch)))
    {
        data_proxmatch* proxmatch = (data_proxmatch*)CSL_VectorGetPointer(vec_proxmatch, index);
        CSL_Vector*     vec_pairs = NULL;
        if (!proxmatch || (proxmatch->has_poi_content == FALSE))
            return NULL;

        state = NB_ContextGetDataState(information->context);
        vec_pairs = proxmatch->poi_content.vec_pairs;
        if (vec_pairs)
        {
            const char* key = wantThumbnail ? KEY_THUMBNAIL_PHOTO_URL : KEY_PHOTO_URL;
            int pair_length = CSL_VectorGetLength(vec_pairs);
            int i;
            data_pair* dataPair = NULL;

            for (i = 0; i < pair_length; ++i) // Walk through vec_pairs to search proper key.
            {
                const char* keyOfPair = NULL;
                dataPair = (data_pair*) CSL_VectorGetPointer(vec_pairs, i);

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
                url = data_string_get(state, &(dataPair->value));
                break;
            }
        }
    }
    return url;
}

NB_DEF NB_Error
NB_SingleSearchInformationGetPlaceExtAppContent(NB_SingleSearchInformation* information,
                                                uint32 resultIndex,
                                                NB_ExtAppContent* extAppContent)
{
    NB_Error result = NE_OK;
    uint32 count = 0;
    data_util_state* state = NULL;
    data_proxmatch* proxmatch = NULL;

    if (!information || !extAppContent)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(information->context);

    count = (uint32)CSL_VectorGetLength(information->reply.vector_proxmatch);
    if (resultIndex >= count)
    {
        return NE_NOENT;
    }

    proxmatch = (data_proxmatch*)CSL_VectorGetPointer(information->reply.vector_proxmatch, (int)resultIndex);
    if (!proxmatch)
    {
        return NE_NOENT;
    }

    nsl_memset(extAppContent, 0, sizeof(*extAppContent));

    result = SetNIMExtAppContentFromExtAppContentVector(extAppContent, state, proxmatch->vec_extapp_content);

    return result;
}


// See header file for description
NB_DEF NB_Error
NB_SingleSearchInformationGetTrafficIncident(NB_SingleSearchInformation* information,
                                             int resultIndex,
                                             NB_TrafficIncident* incident,
                                             double* distance)
{
    data_proxmatch* pproxmatch = NULL;
    int resultCount = 0;
    if (!information || !incident)
    {
        return NE_INVAL;
    }

    resultCount = CSL_VectorGetLength(information->reply.vector_proxmatch);

    nsl_memset(incident, 0, sizeof(*incident));

    pproxmatch = CSL_VectorGetPointer(information->reply.vector_proxmatch, (int)resultIndex);

    if (!(pproxmatch->place.elements & PlaceElement_TrafficIncident))
    {
        return NE_NOENT;
    }

    // Set distance. Distance is an optional parameter
    if (distance)
    {
        *distance = pproxmatch->distance;
    }

    // Set traffic incident from the single search result
    return SetNIMTrafficIncidentDetailsFromTrafficIncident(incident,
                                                           NB_ContextGetDataState(information->context),
                                                           &pproxmatch->place.traffic_incident);
}
/*! @} */
