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

    @file     data_search_query.h
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

#ifndef DATA_SEARCH_QUERY_H
#define DATA_SEARCH_QUERY_H

/*! @{ */

#include "datautil.h"
#include "data_position.h"
#include "data_iter_command.h"
#include "data_search_filter.h"
#include "data_route_corridor.h"
#include "data_image.h"
#include "data_want_content_types.h"

typedef struct data_search_query_
{
    /* Child Elements */
    data_position           position;                   /*!< Center of the proximity search. If the client can obtain
                                                             the current position of the user or a recent position, it
                                                             should be provided. Only when a fix is unavailable may the
                                                             position be omitted. */
    data_position           user_position;              /*!< Position distance calculated to */
    data_iter_command       iter_command;               /*!< Iteration command to use for this operation */
    data_search_filter      search_filter;              /*!< The filter criteria for the search */
    data_route_corridor     route_corridor;             /*!< Perform search along a route */
    data_blob               routeId;                    /*!< route id for search along my route*/
    data_want_content_types want_content_types;         /*!< Want content types to use for this operation */
    boolean                 directed;                   /*!< Optimize search for the direction of travel (as specified
                                                             in the location). */
    boolean                 want_premium_placement;     /*!< If present, want a premium placement ad in the response. */
    boolean                 want_enhanced_pois;         /*!< If present, want Enhanced POIs in the response. */
    boolean                 want_formatted;             /*!< If present, additional POI content will be returned as
                                                             formatted text. Otherwise, it will be returned as key/value
                                                             pairs.*/
    boolean                 want_spelling_suggestions;  /*!< If present, want spelling suggestions in the response. */
    boolean                 want_suggest_distance;      /*!< If present, want suggestions distance in the response. */
    boolean                 want_suggest_three_lines;   /*!< If present, want three lines for suggestions in the response. */
    boolean                 want_data_availability;     /*!< If present, country results shall include a new data-availability
                                                             element in the proxmatch element. */
    boolean                 want_distance_to_user;      /*!< If present, want suggestions distance to user, not to search center. */
    boolean                 want_country_name;          /*!< If present, country name shall be returned  the address element. */
    boolean                 need_user_position;         /*!< If present, user position will be added to the query. */
    boolean                 want_extended_address;      /*!< If present, full formatted address (max 5 lines) shall be returned. */
    boolean                 want_compact_address;       /*!< If present, compact formatted address (max 2 lines) shall be returned. */
    boolean                 want_formatted_phone;       /*!< If present, formatted phone shall be returned. */

    boolean                 want_structured_hours_of_operation;     /*!< If present, hours of operation shall be returned. */
    boolean                 want_parent_category;       /*!< If present, parent category shall be returned. */
    boolean                 want_want_content_types;    /*!< If present, want content types will be added to the query. */
    boolean                 want_icon_id;               /*!< If present, Icon ID shall be returned. */

    CSL_Vector*             vector_premium_pairs;       /*!< Additional key/value pairs for the premium placement */
    CSL_Vector*             vector_enhanced_pairs;      /*!< Additional key/value pairs for Enhanced POIs */
    CSL_Vector*             vector_formatted_pairs;     /*!< Additional key/value pairs for formatted text */
    data_image              premium_placement_image;    /*!< The image for the premium placement */
    data_image              enhanced_pois_image;        /*!< The image for the Enhanced POIs */
    data_image              formatted_image;            /*!< The image for the formatted text */

    /* Attributes */
    data_string             scheme;                     /*!< Data scheme to use */
    data_string             language;                   /*!< Optional. 2 to 5 character string specifying the language
                                                             for the request. If not specified, US English is default. */

} data_search_query;

NB_Error    data_search_query_init(data_util_state* state, data_search_query* searchQuery);
void        data_search_query_free(data_util_state* state, data_search_query* searchQuery);

tpselt      data_search_query_to_tps(data_util_state* state, data_search_query* searchQuery);

NB_Error    data_search_query_copy(data_util_state* state, data_search_query* destinationSearchQuery, data_search_query* sourceSearchQuery);

NB_Error    data_search_query_add_premium_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value);
NB_Error    data_search_query_add_enhanced_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value);
NB_Error    data_search_query_add_formatted_pair(data_util_state* state, data_search_query* searchQuery, const char* key, const char* value);

/*! @} */

#endif
