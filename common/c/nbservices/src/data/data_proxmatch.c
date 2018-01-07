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
 * data_proxmatch.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_proxmatch.h"
#include "data_extapp_content.h"

static void
data_proxmatch_clear(data_util_state* pds, data_proxmatch* pdat)
{
	pdat->distance = -1.0;
    pdat->accuracy = 0;
    pdat->enhanced_poi = FALSE;
    pdat->premium_placement = FALSE;
    pdat->unmappable = FALSE;
    pdat->has_poi_content = FALSE;
}

NB_DEF NB_Error	
data_proxmatch_init(data_util_state* pds, data_proxmatch* pdat)
{
	NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_proxmatch);

    DATA_INIT(pds, err, &pdat->place, data_place);

    DATA_INIT(pds, err, &pdat->search_filter, data_search_filter);

    DATA_INIT(pds, err, &pdat->poi_content, data_poi_content);

    DATA_VEC_ALLOC(err, pdat->vec_transit_poi, data_transit_poi);

    DATA_VEC_ALLOC(err, pdat->vec_extapp_content, data_extapp_content);

	data_proxmatch_clear(pds, pdat);
    pdat->data_availability = NULL;

	return err;
}

NB_DEF void
data_proxmatch_free(data_util_state* pds, data_proxmatch* pdat)
{
    DATA_FREE(pds, &pdat->search_filter, data_search_filter);

    DATA_FREE(pds, &pdat->place, data_place);

    DATA_FREE(pds, &pdat->poi_content, data_poi_content);

    DATA_VEC_FREE(pds, pdat->vec_transit_poi, data_transit_poi);

    DATA_VEC_FREE(pds, pdat->vec_extapp_content, data_extapp_content);

    if (pdat->data_availability)
    {
        data_data_availability_free(pds, pdat->data_availability);
        nsl_free(pdat->data_availability);
        pdat->data_availability = NULL;
    }

	data_proxmatch_clear(pds, pdat);
}

NB_DEF NB_Error
data_proxmatch_from_tps(data_util_state* pds, data_proxmatch* pdat, tpselt te)
{
	NB_Error err = NE_OK;
    int iter = 0;
    boolean has_place = FALSE;
    tpselt	ce;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

    DATA_REINIT(pds, err, pdat, data_proxmatch);

	if (err != NE_OK)
		return err;

    while (!err && (ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "place") == 0) 
        {
            has_place = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->place, data_place);
        }
        else if (nsl_strcmp(te_getname(ce), "search-filter") == 0) 
        {
            DATA_FROM_TPS(pds, err, ce, &pdat->search_filter, data_search_filter);
        }
        else if (nsl_strcmp(te_getname(ce), "poi-content") == 0)
        {
            pdat->has_poi_content = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->poi_content, data_poi_content);
        }
        else if (nsl_strcmp(te_getname(ce), "enhanced-poi") == 0)
        {
            pdat->enhanced_poi = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "premium-placement") == 0)
        {
            pdat->premium_placement = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "unmappable") == 0)
        {
            pdat->unmappable = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "transit-poi") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_transit_poi, data_transit_poi);
        }
        else if (nsl_strcmp(te_getname(ce), "extapp-content") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_extapp_content, data_extapp_content);
        }
        else if (nsl_strcmp(te_getname(ce), "data-availability") == 0)
        {
            pdat->data_availability = nsl_malloc(sizeof(data_data_availability));
            if (pdat->data_availability == NULL)
            {
                err = NE_NOMEM;
                goto errexit;
            }
            DATA_INIT(pds, err, pdat->data_availability, data_data_availability);
            err = data_data_availability_from_tps(pds, pdat->data_availability, ce);
        }
    }

	if (!err && !has_place) {
		err = NE_INVAL;
		goto errexit;
	}

    if (!err && !te_getattrf(te, "distance", &pdat->distance)) {
	
        err = NE_INVAL;
    }

    if (!err)
    {
        pdat->accuracy = te_getattru(te, "accuracy");
    }

errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_proxmatch (error_code: %d)", err));
		DATA_FREE(pds, pdat, data_proxmatch);
    }

	return err;
}

NB_DEF boolean		
data_proxmatch_equal(data_util_state* pds, data_proxmatch* lhs, data_proxmatch* rhs)
{
    int  ret = TRUE;

    DATA_EQUAL(pds, ret, &lhs->place, &rhs->place, data_place);

    DATA_EQUAL(pds, ret, &lhs->search_filter, &rhs->search_filter, data_search_filter);

    DATA_EQUAL(pds, ret, &lhs->poi_content, &rhs->poi_content, data_poi_content);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_transit_poi, rhs->vec_transit_poi, data_transit_poi);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_extapp_content, rhs->vec_extapp_content, data_extapp_content);

    if (lhs->data_availability && rhs->data_availability)
    {
        DATA_EQUAL(pds, ret, lhs->data_availability, rhs->data_availability, data_data_availability);
    }
    else if (lhs->data_availability || rhs->data_availability)
    {
        ret = FALSE;
    }

    return (boolean) ret;
}

NB_DEF NB_Error
data_proxmatch_copy(data_util_state* pds, data_proxmatch* dst, data_proxmatch* src)
{
	NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_proxmatch);

    DATA_COPY(pds, err, &dst->place, &src->place, data_place);

    DATA_COPY(pds, err, &dst->search_filter, &src->search_filter, data_search_filter);

    DATA_COPY(pds, err, &dst->poi_content, &src->poi_content, data_poi_content);

    DATA_VEC_COPY(pds, err, dst->vec_transit_poi, src->vec_transit_poi, data_transit_poi);

    DATA_VEC_COPY(pds, err, dst->vec_extapp_content, src->vec_extapp_content, data_extapp_content);

    if (dst->data_availability && src->data_availability)
    {
        DATA_COPY(pds, err, dst->data_availability, src->data_availability, data_data_availability);
    }

	dst->distance = src->distance;
    dst->enhanced_poi = src->enhanced_poi;
    dst->premium_placement = src->premium_placement;
    dst->unmappable = src->unmappable;
    dst->has_poi_content = src->has_poi_content;

	return err;
}
