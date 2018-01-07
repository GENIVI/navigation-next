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

    @file     data_suggest_match.h
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

#ifndef DATA_SUGGEST_MATCH_H
#define DATA_SUGGEST_MATCH_H

/*! @{ */

#include "datautil.h"
#include "data_search_filter.h"

typedef struct data_suggest_match_
{
    /* Child Elements */
    struct CSL_Vector*      vec_pairs;                  /*!< Pairs of icon-id. */
    data_search_filter      search_filter;              /*!< The filter criteria for the search */

    /* Attributes */
    data_string             line1;                      /*!< The first line of text to be displayed for this suggestion.
                                                             For POI results, this will be the name of the place. For
                                                             address results it will be the address text as formatted by
                                                             the server.*/
    data_string             line2;                      /*!< The second line of text to be displayed for this suggestion,
                                                             if present. For POI results this will be the address of the
                                                             place, pre-formatted as text by the server. */
    data_string             line3;                      /*!< The third line of text to be displayed for this suggestion,
                                                             if present. For POI results this will be the additional address
                                                             of the place, pre-formatted as text by the server */
    data_string             match_type;                 /*!< Identifies the type of result indicated by the suggestion.
                                                             Used by the client for showing the appropriate icons. */
    double                  distance;                   /*!< The distance to be displayed for this suggestion. */

} data_suggest_match;

NB_Error    data_suggest_match_init(data_util_state* state, data_suggest_match* suggestMatch);
void        data_suggest_match_free(data_util_state* state, data_suggest_match* suggestMatch);

NB_Error    data_suggest_match_from_tps(data_util_state* state, data_suggest_match* suggestMatch, tpselt tpsElement);

boolean     data_suggest_match_equal(data_util_state* state, data_suggest_match* suggestMatch1, data_suggest_match* suggestMatch2);
NB_Error    data_suggest_match_copy(data_util_state* state, data_suggest_match* destinationSuggestMatch, data_suggest_match* sourceSuggestMatch);

/*! @} */

#endif
