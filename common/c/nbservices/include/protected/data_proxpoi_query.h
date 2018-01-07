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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_proxpoi_query.h: created 2005/01/02 by Mark Goddard.
 */

#ifndef DATA_PROXPOI_QUERY_H
#define DATA_PROXPOI_QUERY_H

#include "nbexp.h"
#include "datautil.h"
#include "data_position.h"
#include "data_iter_command.h"
#include "data_search_filter.h"
#include "data_route_corridor.h"
#include "data_string.h"
#include "vec.h"
#include "data_image.h"
#include "data_search_cookie.h"

typedef struct data_proxpoi_query_ 
{
	/* Child Elements */
	data_position		position;
	data_iter_command	iter_command;

	boolean				directed;

	data_search_filter	filter;
	
	data_route_corridor route_corridor;

    data_search_cookie  search_cookie;

	/* Attributes */

    // Name and category got moved to the search filter.
    // Category is handled as a pair now.

	data_string			scheme;
	data_blob			route_id;

	uint32				maneuver;
	boolean				maneuver_valid;

    boolean             want_premium_placement;
    boolean             want_enhanced_pois;
    boolean             want_formatted;
    boolean             want_accuracy;
    boolean             want_spelling_suggestions;
    boolean             want_non_proximity_pois;

    boolean             enforce_slice_size;

	data_string         language;

    struct CSL_Vector*  vec_premium_pairs;
    struct CSL_Vector*  vec_enhanced_pairs;
    struct CSL_Vector*  vec_formatted_pairs;

    data_image          premium_placement_image;
    data_image          enhanced_pois_image;
    data_image          formatted_image;

} data_proxpoi_query;

NB_Error	data_proxpoi_query_init(data_util_state* pds, data_proxpoi_query* ppq);
void		data_proxpoi_query_free(data_util_state* pds, data_proxpoi_query* ppq);

tpselt		data_proxpoi_query_to_tps(data_util_state* pds, data_proxpoi_query* ppq);

NB_Error	data_proxpoi_query_copy(data_util_state* pds, data_proxpoi_query* ppq_dest, data_proxpoi_query* ppq_src);

NB_Error    data_proxpoi_query_add_premium_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value);
NB_Error    data_proxpoi_query_add_enhanced_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value);
NB_Error    data_proxpoi_query_add_formatted_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value);

NB_Error    data_proxpoi_query_set_search_cookie(data_util_state* pds, data_proxpoi_query* ppq, const data_search_cookie* psc);

#endif
