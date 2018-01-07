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

    @file     data_analytics_event_place.c
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

/*! @{ */
#include "data_analytics_event_place.h"

NB_Error
data_analytics_event_place_init(data_util_state* pds,
        data_analytics_event_place* paep)
{
    NB_Error err = NE_OK;

    paep->gps_based = FALSE;
    paep->premium_placement = FALSE;
    paep->enhanced_poi = FALSE;
    DATA_INIT(pds, err, &paep->golden_cookie, data_golden_cookie);

    DATA_INIT(pds, err, &paep->id, data_string);
    paep->search_query_event_id = 0;
    paep->search_query_event_id_present = FALSE;

    DATA_INIT(pds, err, &paep->place_event_cookie, data_place_event_cookie);
    paep->index = 0;
    DATA_INIT(pds, err, &paep->origin, data_string);

    return err;
}

void
data_analytics_event_place_free(data_util_state* pds,
        data_analytics_event_place* paep)
{
    DATA_FREE(pds, &paep->golden_cookie, data_golden_cookie);
    DATA_FREE(pds, &paep->id, data_string);
    DATA_FREE(pds, &paep->place_event_cookie, data_place_event_cookie);
    DATA_FREE(pds, &paep->origin, data_string);
}

tpselt
data_analytics_event_place_to_tps(data_util_state* pds,
        data_analytics_event_place* paep)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("analytics-event-place");

    if (te == NULL)
    {
        goto errexit;
    }

    // if any attributes are to be added, id attribute is required
    if (paep->id)
    {
        if (!te_setattrc(te, "id", data_string_get(pds, &paep->id)))
        {
            goto errexit;
        }

        // check for search-query-event attribute, which is required for all other attribute combos
        if (paep->search_query_event_id_present)
        {
            if (!te_setattru(te, "search-query-event-id",
                paep->search_query_event_id))
            {
                goto errexit;
            }

            // origin attribute required for index & origin
            if (paep->origin)
            {
                if (!te_setattru(te, "index", paep->index))
                {
                    goto errexit;
                }

                if (!te_setattrc(te, "origin", data_string_get(pds, &paep->origin)))
                {
                    goto errexit;
                }
            }
        }
    }

    if (paep->gps_based)
    {
        if ((ce = te_new("gps-based")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (paep->premium_placement)
    {
        if ((ce = te_new("premium-placement")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (paep->enhanced_poi)
    {
        if ((ce = te_new("enhanced-poi")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (paep->golden_cookie.provider_id)    /*  has valid golden_cookie? */
    {
        ce = data_golden_cookie_to_tps(pds, &paep->golden_cookie);
        if ((ce != NULL) && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (paep->place_event_cookie.provider_id)    /*  has valid place_event_cookie? */
    {
        ce = data_place_event_cookie_to_tps(pds, &paep->place_event_cookie);
        if ((ce != NULL) && te_attach(te, ce))
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
    return NULL;
}

boolean
data_analytics_event_place_equal(data_util_state* pds,
        data_analytics_event_place* paep1, data_analytics_event_place* paep2)
{
    return (boolean) ((paep1->gps_based == paep2->gps_based) &&
                      (paep1->premium_placement == paep2->premium_placement) &&
                      (paep1->enhanced_poi == paep2->enhanced_poi) &&
                      data_golden_cookie_equal(pds, &paep1->golden_cookie,
                        &paep2->golden_cookie) &&
                      data_place_event_cookie_equal(pds, &paep1->place_event_cookie,
                        &paep2->place_event_cookie) &&
                      data_string_equal(pds, &paep1->id, &paep2->id) &&
                      (paep1->search_query_event_id == paep2->search_query_event_id) &&
                      (paep1->search_query_event_id_present == paep2->search_query_event_id_present) &&
                      (paep1->index == paep2->index) &&
                      data_string_equal(pds, &paep1->origin, &paep2->origin));
}

NB_Error
data_analytics_event_place_copy(data_util_state* pds,
        data_analytics_event_place* paep_dest,
        data_analytics_event_place* paep_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, paep_dest, data_analytics_event_place);

    paep_dest->gps_based = paep_src->gps_based;
    paep_dest->premium_placement = paep_src->premium_placement;
    paep_dest->enhanced_poi = paep_src->enhanced_poi;
    DATA_COPY(pds, err, &paep_dest->golden_cookie,
        &paep_src->golden_cookie, data_golden_cookie);
    DATA_COPY(pds, err, &paep_dest->place_event_cookie,
        &paep_src->place_event_cookie, data_place_event_cookie);

    DATA_COPY(pds, err, &paep_dest->id, &paep_src->id, data_string);
    paep_dest->search_query_event_id = paep_src->search_query_event_id;
    paep_dest->search_query_event_id_present =
        paep_src->search_query_event_id_present;
    paep_dest->index = paep_src->index;
    DATA_COPY(pds, err, &paep_dest->origin, &paep_src->origin, data_string);

    return err;
}

uint32
data_analytics_event_place_get_tps_size(data_util_state* pds,
        data_analytics_event_place* paep)
{
    uint32 size = 0;

    size += sizeof(paep->gps_based);
    size += sizeof(paep->premium_placement);
    size += sizeof(paep->enhanced_poi);
    size += data_golden_cookie_get_tps_size(pds, &paep->golden_cookie);
    size += data_place_event_cookie_get_tps_size(pds, &paep->place_event_cookie);
    size += data_string_get_tps_size(pds, &paep->id);

    if (paep->search_query_event_id_present)
    {
        size += sizeof(paep->search_query_event_id);
    }
    size += sizeof(paep->index);
    size += data_string_get_tps_size(pds, &paep->origin);

    return size;
}

void
data_analytics_event_place_to_buf(data_util_state* pds,
            data_analytics_event_place* paep,
            struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*)&paep->gps_based, sizeof(paep->gps_based));

    dbufcat(pdb, (const byte*)&paep->premium_placement,
            sizeof(paep->premium_placement));

    dbufcat(pdb, (const byte*)&paep->enhanced_poi, sizeof(paep->enhanced_poi));

    data_golden_cookie_to_buf(pds, &paep->golden_cookie, pdb);
    data_place_event_cookie_to_buf(pds, &paep->place_event_cookie, pdb);
    data_string_to_buf(pds, &paep->id, pdb);

    dbufcat(pdb, (const byte*)&paep->search_query_event_id,
            sizeof(paep->search_query_event_id));

    dbufcat(pdb, (const byte*)&paep->search_query_event_id_present,
            sizeof(paep->search_query_event_id_present));
            
    dbufcat(pdb, (const byte*)&paep->index, sizeof(paep->index));
    data_string_to_buf(pds, &paep->origin, pdb);
}

NB_Error
data_analytics_event_place_from_binary(data_util_state* pds,
            data_analytics_event_place* paep,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_boolean_from_binary(pds, &paep->gps_based,
            pdata, pdatalen);

    err = err ? err : data_boolean_from_binary(pds, &paep->premium_placement,
            pdata, pdatalen);

    err = err ? err : data_boolean_from_binary(pds, &paep->enhanced_poi,
            pdata, pdatalen);

    err = err ? err : data_golden_cookie_from_binary(pds, &paep->golden_cookie,
        pdata, pdatalen);

    err = err ? err : data_place_event_cookie_from_binary(pds, &paep->place_event_cookie,
        pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &paep->id,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &paep->search_query_event_id,
            pdata, pdatalen);

    err = err ? err : data_boolean_from_binary(pds,
            &paep->search_query_event_id_present, pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &paep->index, pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &paep->origin, pdata, pdatalen);

    return err;
}

/*! @} */

