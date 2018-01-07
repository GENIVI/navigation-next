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

    @file     data_poi_content.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_poi_content.h"

NB_DEF NB_Error
data_poi_content_init(data_util_state* pds, data_poi_content* ppc)
{
    NB_Error err = NE_OK;

    ppc->hasHoursOfOperation = FALSE;
    ppc->has_vendor_content = FALSE;

    err = data_string_init(pds, &ppc->id);
    err = err ? err : data_tagline_init(pds, &ppc->tagline);
    err = err ? err : data_overall_rating_init(pds, &ppc->overall_rating);
    err = err ? err : data_formatted_text_init(pds, &ppc->formatted_text);
    err = err ? err : data_golden_cookie_init(pds, &ppc->golden_cookie);
    err = err ? err : data_place_event_cookie_init(pds, &ppc->place_event_cookie);
    err = err ? err : data_icon_init(pds, &ppc->icon);
    err = err ? err : data_hours_of_operation_for_search_init(pds, &ppc->hours_of_operation);
    DATA_VEC_ALLOC(err, ppc->vec_vendor_content, data_vendor_content);
    DATA_VEC_ALLOC(err, ppc->vec_pairs, data_pair);

    return err;
}

NB_DEF void
data_poi_content_free(data_util_state* pds, data_poi_content* ppc)
{
    ppc->hasHoursOfOperation = FALSE;
    ppc->has_vendor_content = FALSE;

    data_string_free(pds, &ppc->id);
    data_tagline_free(pds, &ppc->tagline);
    data_overall_rating_free(pds, &ppc->overall_rating);
    data_formatted_text_free(pds, &ppc->formatted_text);
    data_golden_cookie_free(pds, &ppc->golden_cookie);
    data_place_event_cookie_free(pds, &ppc->place_event_cookie);
    data_icon_free(pds, &ppc->icon);
    data_hours_of_operation_for_search_free(pds, &ppc->hours_of_operation);
    DATA_VEC_FREE(pds, ppc->vec_vendor_content, data_vendor_content);
    DATA_VEC_FREE(pds, ppc->vec_pairs, data_pair);
}

NB_DEF NB_Error
data_poi_content_from_tps(data_util_state* pds, data_poi_content* ppc, tpselt te)
{
    NB_Error err = NE_OK;
    int iter = 0;
    tpselt  ce;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }

    DATA_REINIT(pds, err, ppc, data_poi_content);

    if (err != NE_OK)
    {
        return err;
    }

    while (!err && (ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "tagline") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &ppc->tagline, data_tagline);
        }
        else if (nsl_strcmp(te_getname(ce), "overall-rating") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &ppc->overall_rating, data_overall_rating);
        }
        else if (nsl_strcmp(te_getname(ce), "formatted-text") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &ppc->formatted_text, data_formatted_text);
        }
        else if (nsl_strcmp(te_getname(ce), "golden-cookie") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &ppc->golden_cookie, data_golden_cookie);
        }
        else if (nsl_strcmp(te_getname(ce), "place-event-cookie") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &ppc->place_event_cookie, data_place_event_cookie);
        }
        else if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, ppc->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "icon") == 0)
        {
            err = data_icon_from_tps(pds, &ppc->icon, ce);
        }
        else if (nsl_strcmp(te_getname(ce), "hours-of-operation") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &(ppc->hours_of_operation), data_hours_of_operation_for_search);
            ppc->hasHoursOfOperation = (err == NE_OK) ? TRUE : FALSE;
        }
        else if (nsl_strcmp(te_getname(ce), "vendor-content") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, ppc->vec_vendor_content, data_vendor_content);
            ppc->has_vendor_content = (err == NE_OK) ? TRUE : FALSE;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &ppc->id, te, "id");

    if (err != NE_OK)
    {
        data_poi_content_free(pds, ppc);
    }

    return err;
}

NB_DEF boolean
data_poi_content_equal(data_util_state* pds, data_poi_content* ppc1, data_poi_content* ppc2)
{
    int ret = TRUE;

    if (ppc1->hasHoursOfOperation && ppc2->hasHoursOfOperation)
    {
        DATA_EQUAL(pds, ret, &(ppc1->hours_of_operation), &(ppc2->hours_of_operation), data_hours_of_operation_for_search);
    }
    else if (ppc1->hasHoursOfOperation || ppc2->hasHoursOfOperation)
    {
        return FALSE;
    }

    DATA_VEC_EQUAL(pds, ret, ppc1->vec_vendor_content, ppc2->vec_vendor_content, data_vendor_content);
    DATA_VEC_EQUAL(pds, ret, ppc1->vec_pairs, ppc2->vec_pairs, data_pair);

    return (boolean) (ret &&
                        data_string_equal(pds, &ppc1->id, &ppc2->id) &&
                        data_tagline_equal(pds, &ppc1->tagline, &ppc2->tagline) &&
                        data_overall_rating_equal(pds, &ppc1->overall_rating, &ppc2->overall_rating) &&
                        data_formatted_text_equal(pds, &ppc1->formatted_text, &ppc2->formatted_text) &&
                        data_golden_cookie_equal(pds, &ppc1->golden_cookie, &ppc2->golden_cookie) &&
                        data_place_event_cookie_equal(pds, &ppc1->place_event_cookie, &ppc2->place_event_cookie) &&
                        data_icon_equal(pds, &ppc1->icon, &ppc2->icon));
}

NB_DEF NB_Error
data_poi_content_copy(data_util_state* pds, data_poi_content* ppc_dest, data_poi_content* ppc_src)
{
    NB_Error err = NE_OK;

    data_poi_content_free(pds, ppc_dest);

    err = data_poi_content_init(pds, ppc_dest);
    err = err ? err : data_string_copy(pds, &ppc_dest->id, &ppc_src->id);
    err = err ? err : data_tagline_copy(pds, &ppc_dest->tagline, &ppc_src->tagline);
    err = err ? err : data_overall_rating_copy(pds, &ppc_dest->overall_rating, &ppc_src->overall_rating);
    err = err ? err : data_formatted_text_copy(pds, &ppc_dest->formatted_text, &ppc_src->formatted_text);
    err = err ? err : data_golden_cookie_copy(pds, &ppc_dest->golden_cookie, &ppc_src->golden_cookie);
    err = err ? err : data_place_event_cookie_copy(pds, &ppc_dest->place_event_cookie, &ppc_src->place_event_cookie);
    err = err ? err : data_icon_copy(pds, &ppc_dest->icon, &ppc_src->icon);
    err = err ? err : data_hours_of_operation_for_search_copy(pds, &ppc_dest->hours_of_operation, &ppc_src->hours_of_operation);
    DATA_VEC_COPY(pds, err, ppc_dest->vec_vendor_content, ppc_src->vec_vendor_content, data_vendor_content);
    DATA_VEC_COPY(pds, err, ppc_dest->vec_pairs, ppc_src->vec_pairs, data_pair);

    if (err == NE_OK)
    {
        ppc_dest->hasHoursOfOperation = ppc_src->hasHoursOfOperation;
        ppc_dest->has_vendor_content = ppc_src->has_vendor_content;
    }
    else
    {
        DATA_FREE(pds, ppc_dest, data_poi_content);
    }

    return err;
}

/*! @} */
