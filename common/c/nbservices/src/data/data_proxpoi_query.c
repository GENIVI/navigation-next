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
 * data_proxpoi_query.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_proxpoi_query.h"
#include "data_pair.h"

static int AddPairsToElement(data_util_state* dataState, tpselt parent, struct CSL_Vector* pairs);
static NB_Error AddPairToVector(data_util_state* dataState, struct CSL_Vector* pairs, const char* key, const char* value);
static int AddImageToElement(data_util_state* dataState, tpselt parent, data_image* image);

static void
data_proxpoi_query_clear(data_util_state* pds, data_proxpoi_query* ppq)
{
    ppq->directed = FALSE;
    ppq->maneuver_valid = FALSE;
    ppq->maneuver = 0;
    ppq->want_premium_placement = FALSE;
    ppq->want_enhanced_pois = FALSE;
    ppq->want_formatted = FALSE;
    ppq->want_accuracy = FALSE;
    ppq->want_spelling_suggestions = FALSE;
    ppq->want_non_proximity_pois = FALSE;
    ppq->enforce_slice_size = FALSE;
}

NB_Error    
data_proxpoi_query_init(data_util_state* pds, data_proxpoi_query* ppq)
{
    NB_Error err = NE_OK;

    err = err ? err : data_position_init(pds, &ppq->position);
    err = err ? err : data_iter_command_init(pds, &ppq->iter_command);
    err = err ? err : data_search_filter_init(pds, &ppq->filter);
    err = err ? err : data_route_corridor_init(pds, &ppq->route_corridor);
    err = err ? err : data_search_cookie_init(pds, &ppq->search_cookie);
    
    err = err ? err : data_string_init(pds, &ppq->scheme);
    err = err ? err : data_blob_init(pds, &ppq->route_id);
    err = err ? err : data_string_init(pds, &ppq->language);

    data_proxpoi_query_clear(pds, ppq);

    if (!err)
    {
        ppq->vec_premium_pairs = 0;
        ppq->vec_enhanced_pairs = 0;
        ppq->vec_formatted_pairs = 0;

        DATA_VEC_ALLOC(err, ppq->vec_premium_pairs, data_pair);
        DATA_VEC_ALLOC(err, ppq->vec_enhanced_pairs, data_pair);
        DATA_VEC_ALLOC(err, ppq->vec_formatted_pairs, data_pair);
    }

    err = err ? err : data_image_init(pds, &ppq->premium_placement_image);
    err = err ? err : data_image_init(pds, &ppq->enhanced_pois_image);
    err = err ? err : data_image_init(pds, &ppq->formatted_image);

    if (err)
    {
        data_proxpoi_query_free(pds, ppq);
    }

    return err;
}

void        
data_proxpoi_query_free(data_util_state* pds, data_proxpoi_query* ppq)
{
    data_position_free(pds, &ppq->position);
    data_iter_command_free(pds, &ppq->iter_command);
    data_route_corridor_free(pds, &ppq->route_corridor);
    data_search_cookie_free(pds, &ppq->search_cookie);

    data_string_free(pds, &ppq->scheme);
    data_blob_free(pds, &ppq->route_id);
    data_string_free(pds, &ppq->language);
    data_search_filter_free(pds, &ppq->filter);

    DATA_VEC_FREE(pds, ppq->vec_premium_pairs, data_pair);
    DATA_VEC_FREE(pds, ppq->vec_enhanced_pairs, data_pair);
    DATA_VEC_FREE(pds, ppq->vec_formatted_pairs, data_pair);

    data_image_free(pds, &ppq->premium_placement_image);
    data_image_free(pds, &ppq->enhanced_pois_image);
    data_image_free(pds, &ppq->formatted_image);
}

tpselt        
data_proxpoi_query_to_tps(data_util_state* pds, data_proxpoi_query* ppq)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("proxpoi-query");

    if (te == NULL)
        goto errexit;

    if ((ce = data_position_to_tps(pds, &ppq->position)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;
    
    if ((ce = data_iter_command_to_tps(pds, &ppq->iter_command)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;
    
    if ((ce = data_search_filter_to_tps(pds, &ppq->filter)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;
    
    if (ppq->directed) {

        if ((ce = te_new("directed")) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (ppq->route_corridor.route_id.size)
    {
        if ((ce = data_route_corridor_to_tps(pds, &ppq->route_corridor)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->search_cookie.provider_id)
    {
        if ((ce = data_search_cookie_to_tps(pds, &ppq->search_cookie)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (!te_setattrc(te, "scheme", data_string_get(pds, &ppq->scheme)))
        goto errexit;

    if (ppq->route_id.size > 0 && !te_setattr(te, "route-id", (const char*) ppq->route_id.data, ppq->route_id.size))
        goto errexit;

    if (ppq->maneuver_valid) {

        if (!te_setattru(te, "maneuver", ppq->maneuver))
            goto errexit;
    }

    // Fill attributes of search filter here.

    if (!te_setattrc(te, "language", data_string_get(pds, &ppq->language)))
        goto errexit;

    if (ppq->want_premium_placement)
    {
        if ((ce = te_new("want-premium-placement")) != NULL &&
            AddPairsToElement(pds, ce, ppq->vec_premium_pairs) &&
            AddImageToElement(pds, ce, &ppq->premium_placement_image) &&
            te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->want_enhanced_pois)
    {
        if ((ce = te_new("want-enhanced-pois")) != NULL &&
            AddPairsToElement(pds, ce, ppq->vec_enhanced_pairs) &&
            AddImageToElement(pds, ce, &ppq->enhanced_pois_image) &&
            te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->want_formatted)
    {
        if ((ce = te_new("want-formatted")) != NULL &&
            AddPairsToElement(pds, ce, ppq->vec_formatted_pairs) &&
            AddImageToElement(pds, ce, &ppq->formatted_image) &&
            te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->want_accuracy)
    {
        if ((ce = te_new("want-accuracy")) != NULL && te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->want_spelling_suggestions && nsl_stricmp(data_string_get(pds, &ppq->iter_command.command), "start") == 0)
    {
        if ((ce = te_new("want-spelling-suggestions")) != NULL && te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->want_non_proximity_pois && nsl_stricmp(data_string_get(pds, &ppq->iter_command.command), "start") == 0)
    {
        if ((ce = te_new("want-non-proximity-pois")) != NULL && te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (ppq->enforce_slice_size)
    {
        if ((ce = te_new("enforce-slice-size")) != NULL && te_attach(te,ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);

    return NULL;
}

NB_Error    
data_proxpoi_query_copy(data_util_state* pds, data_proxpoi_query* ppq_dest, data_proxpoi_query* ppq_src)
{
    NB_Error err = NE_OK;

    data_proxpoi_query_free(pds, ppq_dest);

    err = err ? err : data_proxpoi_query_init(pds, ppq_dest);

    err = err ? err : data_position_copy(pds, &ppq_dest->position, &ppq_src->position);
    err = err ? err : data_iter_command_copy(pds, &ppq_dest->iter_command, &ppq_src->iter_command);

    ppq_dest->directed = ppq_src->directed;

    err = err ? err : data_string_copy(pds, &ppq_dest->scheme, &ppq_src->scheme);
    err = err ? err : data_blob_copy(pds, &ppq_dest->route_id, &ppq_src->route_id);
    err = err ? err : data_string_copy(pds, &ppq_dest->language, &ppq_src->language);
    err = err ? err : data_search_filter_copy(pds, &ppq_dest->filter, &ppq_src->filter);
    err = err ? err : data_route_corridor_copy(pds, &ppq_dest->route_corridor, &ppq_src->route_corridor);
    err = err ? err : data_search_cookie_copy(pds, &ppq_dest->search_cookie, &ppq_src->search_cookie);

    DATA_VEC_COPY(pds, err, ppq_dest->vec_premium_pairs, ppq_src->vec_premium_pairs, data_pair);
    DATA_VEC_COPY(pds, err, ppq_dest->vec_enhanced_pairs, ppq_src->vec_enhanced_pairs, data_pair);
    DATA_VEC_COPY(pds, err, ppq_dest->vec_formatted_pairs, ppq_src->vec_formatted_pairs, data_pair);

    ppq_dest->maneuver = ppq_src->maneuver;
    ppq_dest->maneuver_valid = ppq_src->maneuver_valid;

    ppq_dest->want_premium_placement = ppq_src->want_premium_placement;
    ppq_dest->want_enhanced_pois = ppq_src->want_enhanced_pois;
    ppq_dest->want_formatted = ppq_src->want_formatted;
    ppq_dest->want_accuracy = ppq_src->want_accuracy;
    ppq_dest->want_spelling_suggestions = ppq_src->want_spelling_suggestions;
    ppq_dest->want_non_proximity_pois = ppq_src->want_non_proximity_pois;
    ppq_dest->enforce_slice_size = ppq_src->enforce_slice_size;

    err = err ? err : data_image_copy(pds, &ppq_dest->premium_placement_image, &ppq_src->premium_placement_image);
    err = err ? err : data_image_copy(pds, &ppq_dest->enhanced_pois_image, &ppq_src->enhanced_pois_image);
    err = err ? err : data_image_copy(pds, &ppq_dest->formatted_image, &ppq_src->formatted_image);

    return err;
}

NB_Error
data_proxpoi_query_add_premium_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value)
{
    return AddPairToVector(pds, ppq->vec_premium_pairs, key, value);
}

NB_Error data_proxpoi_query_add_enhanced_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value)
{
    return AddPairToVector(pds, ppq->vec_enhanced_pairs, key, value);
}

NB_Error data_proxpoi_query_add_formatted_pair(data_util_state* pds, data_proxpoi_query* ppq, const char* key, const char* value)
{
    return AddPairToVector(pds, ppq->vec_formatted_pairs, key, value);
}

NB_Error data_proxpoi_query_set_search_cookie(data_util_state* pds, data_proxpoi_query* ppq, const data_search_cookie* psc)
{
    return data_search_cookie_copy(pds, &ppq->search_cookie, (data_search_cookie*)psc);
}

int AddPairsToElement(data_util_state* dataState, tpselt parent, struct CSL_Vector* pairs)
{
    int rc = 1;

    if (pairs)
    {
        int index = 0;
        int length = CSL_VectorGetLength(pairs);

        for (index = 0; rc && index < length; index++)
        {
            data_pair* pair = (data_pair*)CSL_VectorGetPointer(pairs, index);
            tpselt child = data_pair_to_tps(dataState, pair);
            if (child)
            {
                rc = te_attach(parent, child);
            }
            else
            {
                rc = 0;
            }
        }
    }

    return rc;
}

NB_Error AddPairToVector(data_util_state* dataState, struct CSL_Vector* pairs, const char* key, const char* value)
{
    NB_Error err = NE_OK;
    data_pair pair;

    err = data_pair_init(dataState, &pair);

    err = err ? err : data_string_set(dataState, &pair.key, key);
    err = err ? err : data_string_set(dataState, &pair.value, value);

    err = err ? err : CSL_VectorAppend(pairs, &pair) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_pair_free(dataState, &pair);
    }

    return err;
}

int AddImageToElement(data_util_state* dataState, tpselt parent, data_image* image)
{
    int rc = 1;
    tpselt child = 0;
    const char* format = data_string_get(dataState, &image->format);

    if (format == 0 || format[0] == '\0')
    {
        // Nothing to do if no format specified
        return 1;
    }

    child = data_image_to_tps(dataState, image);
    if (child)
    {
        rc = te_attach(parent, child);
    }
    else
    {
        rc = 0;
    }

    return rc;
}
