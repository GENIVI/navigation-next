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

    @file     data_search_query.c
*/
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_search_query.h"
#include "data_pair.h"

static NB_Error AddPairToVector(data_util_state* state, CSL_Vector* pairs, const char* key, const char* value);

NB_Error
data_search_query_init(data_util_state* state, data_search_query* searchQuery)
{
    NB_Error result = NE_OK;

    DATA_MEM_ZERO(searchQuery, data_search_query);

    DATA_INIT(state, result, &(searchQuery->position), data_position);
    DATA_INIT(state, result, &(searchQuery->user_position), data_position);
    DATA_INIT(state, result, &(searchQuery->iter_command), data_iter_command);
    DATA_INIT(state, result, &(searchQuery->search_filter), data_search_filter);
    DATA_INIT(state, result, &(searchQuery->route_corridor), data_route_corridor);
    DATA_INIT(state, result, &(searchQuery->premium_placement_image), data_image);
    DATA_INIT(state, result, &(searchQuery->enhanced_pois_image), data_image);
    DATA_INIT(state, result, &(searchQuery->formatted_image), data_image);
    DATA_INIT(state, result, &(searchQuery->scheme), data_string);
    DATA_INIT(state, result, &(searchQuery->language), data_string);
    DATA_INIT(state, result, &(searchQuery->routeId), data_blob);
    DATA_INIT(state, result, &(searchQuery->want_content_types), data_want_content_types);

    DATA_VEC_ALLOC(result, searchQuery->vector_premium_pairs, data_pair);
    DATA_VEC_ALLOC(result, searchQuery->vector_enhanced_pairs, data_pair);
    DATA_VEC_ALLOC(result, searchQuery->vector_formatted_pairs, data_pair);

    if (result == NE_OK)
    {
        searchQuery->directed = FALSE;
        searchQuery->want_premium_placement = FALSE;
        searchQuery->want_enhanced_pois = FALSE;
        searchQuery->want_formatted = FALSE;
        searchQuery->want_spelling_suggestions = FALSE;
        searchQuery->want_suggest_distance = FALSE;
        searchQuery->want_distance_to_user = FALSE;
        searchQuery->want_suggest_three_lines = FALSE;
        searchQuery->want_data_availability = FALSE;
        searchQuery->need_user_position = FALSE;
        searchQuery->want_country_name = FALSE;
        searchQuery->want_extended_address = FALSE;
        searchQuery->want_compact_address = FALSE;
        searchQuery->want_formatted_phone = FALSE;
        searchQuery->want_structured_hours_of_operation = FALSE;
        searchQuery->want_parent_category = FALSE;
        searchQuery->want_want_content_types = FALSE;
        searchQuery->want_icon_id = FALSE;
    }

    return result;
}

void
data_search_query_free(data_util_state* state, data_search_query* searchQuery)
{
    DATA_FREE(state, &(searchQuery->position), data_position);
    DATA_FREE(state, &(searchQuery->user_position), data_position);
    DATA_FREE(state, &(searchQuery->iter_command), data_iter_command);
    DATA_FREE(state, &(searchQuery->search_filter), data_search_filter);
    DATA_FREE(state, &(searchQuery->route_corridor), data_route_corridor);
    DATA_FREE(state, &(searchQuery->premium_placement_image), data_image);
    DATA_FREE(state, &(searchQuery->enhanced_pois_image), data_image);
    DATA_FREE(state, &(searchQuery->formatted_image), data_image);
    DATA_FREE(state, &(searchQuery->scheme), data_string);
    DATA_FREE(state, &(searchQuery->language), data_string);
    DATA_FREE(state, &(searchQuery->routeId), data_blob);
    DATA_FREE(state, &(searchQuery->want_content_types), data_want_content_types);

    DATA_VEC_FREE(state, searchQuery->vector_premium_pairs, data_pair);
    DATA_VEC_FREE(state, searchQuery->vector_enhanced_pairs, data_pair);
    DATA_VEC_FREE(state, searchQuery->vector_formatted_pairs, data_pair);

    searchQuery->directed = FALSE;
    searchQuery->want_premium_placement = FALSE;
    searchQuery->want_enhanced_pois = FALSE;
    searchQuery->want_formatted = FALSE;
    searchQuery->want_spelling_suggestions = FALSE;
    searchQuery->want_suggest_distance = FALSE;
    searchQuery->want_distance_to_user = FALSE;
    searchQuery->want_suggest_three_lines = FALSE;
    searchQuery->want_data_availability = FALSE;
    searchQuery->need_user_position = FALSE;
    searchQuery->want_country_name = FALSE;
    searchQuery->want_extended_address = FALSE;
    searchQuery->want_compact_address = FALSE;
    searchQuery->want_formatted_phone = FALSE;
    searchQuery->want_structured_hours_of_operation = FALSE;
    searchQuery->want_parent_category = FALSE;
    searchQuery->want_want_content_types = FALSE;
    searchQuery->want_icon_id = FALSE;
}

tpselt
data_search_query_to_tps(data_util_state* state, data_search_query* searchQuery)
{
    tpselt tpsElement = NULL;
    tpselt childElement = NULL;
    const char* format = NULL;

    DATA_ALLOC_TPSELT(errexit, tpsElement, "search-query");

    if (!searchQuery->want_data_availability)
    {
        DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->position), data_position);
        if (searchQuery->need_user_position)
        {
            DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->user_position), data_position);
        }
    }
    DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->iter_command), data_iter_command);
    DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->search_filter), data_search_filter);

    if (searchQuery->route_corridor.route_id.size > 0)
    {
        DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->route_corridor), data_route_corridor);
    }

    if (searchQuery->directed)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "directed");
    }

    if (searchQuery->want_premium_placement)
    {
        DATA_ALLOC_TPSELT(errexit, childElement, "want-premium-placement");
        if (searchQuery->vector_premium_pairs)
        {
            DATA_VEC_TO_TPS(state, errexit, childElement, searchQuery->vector_premium_pairs, data_pair);
        }
        format = data_string_get(state, &(searchQuery->premium_placement_image.format));
        if (format && nsl_strlen(format) > 0)
        {
            DATA_TO_TPS(state, errexit, childElement, &(searchQuery->premium_placement_image), data_image);
        }
        if (te_attach(tpsElement, childElement))
        {
            childElement = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (searchQuery->want_enhanced_pois)
    {
        DATA_ALLOC_TPSELT(errexit, childElement, "want-enhanced-pois");
        if (searchQuery->vector_enhanced_pairs)
        {
            DATA_VEC_TO_TPS(state, errexit, childElement, searchQuery->vector_enhanced_pairs, data_pair);
        }
        format = data_string_get(state, &(searchQuery->enhanced_pois_image.format));
        if (format && nsl_strlen(format) > 0)
        {
            DATA_TO_TPS(state, errexit, childElement, &(searchQuery->enhanced_pois_image), data_image);
        }
        if (te_attach(tpsElement, childElement))
        {
            childElement = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (searchQuery->want_formatted)
    {
        DATA_ALLOC_TPSELT(errexit, childElement, "want-formatted");
        if (searchQuery->vector_formatted_pairs)
        {
            DATA_VEC_TO_TPS(state, errexit, childElement, searchQuery->vector_formatted_pairs, data_pair);
        }
        format = data_string_get(state, &(searchQuery->formatted_image.format));
        if (format && nsl_strlen(format) > 0)
        {
            DATA_TO_TPS(state, errexit, childElement, &(searchQuery->formatted_image), data_image);
        }
        if (te_attach(tpsElement, childElement))
        {
            childElement = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (searchQuery->want_spelling_suggestions && nsl_stricmp(data_string_get(state, &(searchQuery->iter_command.command)), "start") == 0)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-spelling-suggestions");
    }

    if (searchQuery->want_suggest_distance)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-suggest-distance");
    }

    if (searchQuery->want_distance_to_user)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-distance-to-user");
    }

    if (searchQuery->want_suggest_three_lines)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-suggest-three-lines");
    }

    if (searchQuery->want_data_availability)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-data-availability");
    }

    if (searchQuery->want_country_name)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-country-name");
    }

    if (searchQuery->want_extended_address)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-extended-address");
    }

    if (searchQuery->want_compact_address)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-compact-address");
    }

    if (searchQuery->want_formatted_phone)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-formatted-phone");
    }

    if (searchQuery->want_structured_hours_of_operation)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-structured-hours-of-operation");
    }

    if (searchQuery->want_parent_category)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-parent-category");
    }

    if (searchQuery->want_want_content_types)
    {
        DATA_TO_TPS(state, errexit, tpsElement, &(searchQuery->want_content_types), data_want_content_types);
    }

    if (searchQuery->want_icon_id)
    {
        EMPTY_ELEMENT_TO_TPS(state, errexit, tpsElement, "want-icon-id");
    }

    DATA_STR_SETATTR(state, errexit, tpsElement, "scheme", &(searchQuery->scheme));

    // Check if the language is NULL because it is optional. If the language is NULL the attribute of 'language'
    // should not be set. Use the protocol '- search-query: [scheme]'.
    {
        const char* language = data_string_get(state, &(searchQuery->language));

        if (language && (nsl_strlen(language) > 0))
        {
            DATA_STR_SETATTR(state, errexit, tpsElement, "language", &(searchQuery->language));
        }

        if (searchQuery->routeId.size > 0)
        {
            DATA_BLOB_SETATTR(state, errexit, tpsElement, "route-id", &(searchQuery->routeId));
        }
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    te_dealloc(childElement);
    return NULL;
}

NB_Error
data_search_query_copy(data_util_state* state, data_search_query* destinationSearchQuery, data_search_query* sourceSearchQuery)
{
    NB_Error result = NE_OK;

    DATA_REINIT(state, result, destinationSearchQuery, data_search_query);

    DATA_COPY(state, result, &(destinationSearchQuery->position), &(sourceSearchQuery->position), data_position);
    DATA_COPY(state, result, &(destinationSearchQuery->user_position), &(sourceSearchQuery->position), data_position);
    DATA_COPY(state, result, &(destinationSearchQuery->iter_command), &(sourceSearchQuery->iter_command), data_iter_command);
    DATA_COPY(state, result, &(destinationSearchQuery->search_filter), &(sourceSearchQuery->search_filter), data_search_filter);
    DATA_COPY(state, result, &(destinationSearchQuery->route_corridor), &(sourceSearchQuery->route_corridor), data_route_corridor);
    DATA_COPY(state, result, &(destinationSearchQuery->premium_placement_image), &(sourceSearchQuery->premium_placement_image), data_image);
    DATA_COPY(state, result, &(destinationSearchQuery->enhanced_pois_image), &(sourceSearchQuery->enhanced_pois_image), data_image);
    DATA_COPY(state, result, &(destinationSearchQuery->formatted_image), &(sourceSearchQuery->formatted_image), data_image);
    DATA_COPY(state, result, &(destinationSearchQuery->scheme), &(sourceSearchQuery->scheme), data_string);
    DATA_COPY(state, result, &(destinationSearchQuery->language), &(sourceSearchQuery->language), data_string);
    DATA_COPY(state, result, &(destinationSearchQuery->routeId), &(sourceSearchQuery->routeId), data_blob);
    DATA_COPY(state, result, &(destinationSearchQuery->want_content_types), &(sourceSearchQuery->want_content_types), data_want_content_types);

    DATA_VEC_COPY(state, result, destinationSearchQuery->vector_premium_pairs, sourceSearchQuery->vector_premium_pairs, data_pair);
    DATA_VEC_COPY(state, result, destinationSearchQuery->vector_enhanced_pairs, sourceSearchQuery->vector_enhanced_pairs, data_pair);
    DATA_VEC_COPY(state, result, destinationSearchQuery->vector_formatted_pairs, sourceSearchQuery->vector_formatted_pairs, data_pair);

    if (result == NE_OK)
    {
        destinationSearchQuery->directed = sourceSearchQuery->directed;
        destinationSearchQuery->want_premium_placement = sourceSearchQuery->want_premium_placement;
        destinationSearchQuery->want_enhanced_pois = sourceSearchQuery->want_enhanced_pois;
        destinationSearchQuery->want_formatted = sourceSearchQuery->want_formatted;
        destinationSearchQuery->want_spelling_suggestions = sourceSearchQuery->want_spelling_suggestions;
        destinationSearchQuery->want_suggest_distance = sourceSearchQuery->want_suggest_distance;
        destinationSearchQuery->want_distance_to_user = sourceSearchQuery->want_distance_to_user;
        destinationSearchQuery->want_suggest_three_lines = sourceSearchQuery->want_suggest_three_lines;
        destinationSearchQuery->want_data_availability = sourceSearchQuery->want_data_availability;
        destinationSearchQuery->need_user_position = sourceSearchQuery->need_user_position;
        destinationSearchQuery->want_country_name = sourceSearchQuery->want_country_name;
        destinationSearchQuery->want_extended_address = sourceSearchQuery->want_extended_address;
        destinationSearchQuery->want_compact_address = sourceSearchQuery->want_compact_address;
        destinationSearchQuery->want_formatted_phone = sourceSearchQuery->want_formatted_phone;
        destinationSearchQuery->want_structured_hours_of_operation = sourceSearchQuery->want_structured_hours_of_operation;
        destinationSearchQuery->want_parent_category = sourceSearchQuery->want_parent_category;
        destinationSearchQuery->want_want_content_types = sourceSearchQuery->want_want_content_types;
        destinationSearchQuery->want_icon_id = sourceSearchQuery->want_icon_id;
    }

    return result;
}

NB_Error
data_search_query_add_premium_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value)
{
    return AddPairToVector(state, searchQuery->vector_premium_pairs, key, value);
}

NB_Error
data_search_query_add_enhanced_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value)
{
    return AddPairToVector(state, searchQuery->vector_enhanced_pairs, key, value);
}

NB_Error
data_search_query_add_formatted_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value)
{
    return AddPairToVector(state, searchQuery->vector_formatted_pairs, key, value);
}

NB_Error
AddPairToVector(data_util_state* state, CSL_Vector* pairs, const char* key, const char* value)
{
    NB_Error result = NE_OK;
    data_pair pair = {0};

    DATA_INIT(state, result, &pair, data_pair);

    result = result ? result : data_string_set(state, &(pair.key), key);
    result = result ? result : data_string_set(state, &(pair.value), value);
    result = result ? result : (CSL_VectorAppend(pairs, &pair) ? NE_OK : NE_NOMEM);

    if (result != NE_OK)
    {
        DATA_FREE(state, &pair, data_pair);
    }

    return result;
}

/*! @} */
