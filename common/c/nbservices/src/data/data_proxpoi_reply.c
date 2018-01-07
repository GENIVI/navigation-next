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
 * data_proxpoi_reply.c: created 2004/01/02 by Mark Goddard.
 */

#include "data_proxpoi_reply.h"
#include "vec.h"

NB_Error
data_proxpoi_reply_init(data_util_state* pds, data_proxpoi_reply* pdat)
{
	NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_proxpoi_reply);

    DATA_VEC_ALLOC(err, pdat->vec_proxmatch, data_proxmatch);

    DATA_VEC_ALLOC(err, pdat->vec_proxmatch_content, data_proxmatch_content);

    DATA_VEC_ALLOC(err, pdat->vec_proxmatch_summary, data_proxmatch_summary);

    DATA_VEC_ALLOC(err, pdat->vec_proxmatch_ambiguous, data_proxmatch_ambiguous);

    DATA_INIT(pds, err, &pdat->iter_result, data_iter_result);

    DATA_VEC_ALLOC(err, pdat->vec_files, data_file);    

    DATA_INIT(pds, err, &pdat->fuel_pricesummary, data_fuel_pricesummary);

    DATA_INIT(pds, err, &pdat->event_content, data_event_content);

    DATA_INIT(pds, err, &pdat->spelling_suggestion, data_spelling_suggestion);

    DATA_INIT(pds, err, &pdat->search_cookie, data_search_cookie);

    DATA_INIT(pds, err, &pdat->search_event_cookie, data_search_event_cookie);

    DATA_INIT(pds, err, &pdat->proxpoi_error, data_proxpoi_error);

    return err;
}

void		
data_proxpoi_reply_free(data_util_state* pds, data_proxpoi_reply* pdat)
{
    DATA_FREE(pds, &pdat->proxpoi_error, data_proxpoi_error);

    DATA_FREE(pds, &pdat->search_event_cookie, data_search_event_cookie);

    DATA_FREE(pds, &pdat->search_cookie, data_search_cookie);

    DATA_FREE(pds, &pdat->spelling_suggestion, data_spelling_suggestion);

    DATA_FREE(pds, &pdat->event_content, data_event_content);

    DATA_FREE(pds, &pdat->fuel_pricesummary, data_fuel_pricesummary);

    DATA_VEC_FREE(pds, pdat->vec_files, data_file);

    DATA_FREE(pds, &pdat->iter_result, data_iter_result);

    DATA_VEC_FREE(pds, pdat->vec_proxmatch_ambiguous, data_proxmatch_ambiguous);

    DATA_VEC_FREE(pds, pdat->vec_proxmatch_summary, data_proxmatch_summary);
	
    DATA_VEC_FREE(pds, pdat->vec_proxmatch_content, data_proxmatch_content);

    DATA_VEC_FREE(pds, pdat->vec_proxmatch, data_proxmatch);
}

NB_Error	
data_proxpoi_reply_from_tps(data_util_state* pds, data_proxpoi_reply* pdat, tpselt te)
{
	NB_Error err = NE_OK;
    boolean has_iter_result = FALSE;
	tpselt	ce;
	int iter;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

    DATA_REINIT(pds, err, pdat, data_proxpoi_reply);

	if (err != NE_OK)
		goto errexit;
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "proxmatch") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_proxmatch, data_proxmatch);
		}
        else if (nsl_strcmp(te_getname(ce), "proxmatch-content") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_proxmatch_content, data_proxmatch_content);
        }
        else if (nsl_strcmp(te_getname(ce), "proxmatch-summary") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_proxmatch_summary, data_proxmatch_summary);
        }
        else if (nsl_strcmp(te_getname(ce), "proxmatch-ambiguous") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_proxmatch_ambiguous, data_proxmatch_ambiguous);
        }
        else if(nsl_strcmp(te_getname(ce), "iter-result") == 0) {

            has_iter_result = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->iter_result, data_iter_result);
        }
        else if (nsl_strcmp(te_getname(ce), "file") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_files, data_file);
        }
		else if(nsl_strcmp(te_getname(ce), "fuel-price-summary") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->fuel_pricesummary, data_fuel_pricesummary);
		}
        else if(nsl_strcmp(te_getname(ce), "event-content") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->event_content, data_event_content);
        }
        else if (nsl_strcmp(te_getname(ce), "spelling-suggestion") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pdat->spelling_suggestion, data_spelling_suggestion);
        }
        else if (nsl_strcmp(te_getname(ce), "search-cookie") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pdat->search_cookie, data_search_cookie);
        }
        else if (nsl_strcmp(te_getname(ce), "search-event-cookie") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pdat->search_event_cookie, data_search_event_cookie);
        }
        else if (nsl_strcmp(te_getname(ce), "proxpoi-error") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pdat->proxpoi_error, data_proxpoi_error);
        }

        if (err != NE_OK)
            goto errexit;
	}

    if (!has_iter_result)
        err = NE_INVAL;
	
errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_proxpoi_reply failed (error_code: %d)", err));
		DATA_FREE(pds, pdat, data_proxpoi_reply);
    }

	return err;
}

boolean		
data_proxpoi_reply_equal(data_util_state* pds, data_proxpoi_reply* lhs, data_proxpoi_reply* rhs)
{
	int  ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, lhs->vec_proxmatch, rhs->vec_proxmatch, data_proxmatch);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_proxmatch_content, rhs->vec_proxmatch_content, data_proxmatch_content);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_proxmatch_summary, rhs->vec_proxmatch_summary, data_proxmatch_summary);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_proxmatch_ambiguous, rhs->vec_proxmatch_ambiguous, data_proxmatch_ambiguous);

    DATA_EQUAL(pds, ret, &lhs->iter_result, &rhs->iter_result, data_iter_result);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_files, rhs->vec_files, data_file);

    DATA_EQUAL(pds, ret, &lhs->fuel_pricesummary, &rhs->fuel_pricesummary, data_fuel_pricesummary);

    DATA_EQUAL(pds, ret, &lhs->event_content, &rhs->event_content, data_event_content);

    DATA_EQUAL(pds, ret, &lhs->spelling_suggestion, &rhs->spelling_suggestion, data_spelling_suggestion);

    DATA_EQUAL(pds, ret, &lhs->search_cookie, &rhs->search_cookie, data_search_cookie);

    DATA_EQUAL(pds, ret, &lhs->search_event_cookie, &rhs->search_event_cookie, data_search_event_cookie);

    DATA_EQUAL(pds, ret, &lhs->proxpoi_error, &rhs->proxpoi_error, data_proxpoi_error);

	return (boolean) ret;
}

NB_Error	
data_proxpoi_reply_copy(data_util_state* pds, data_proxpoi_reply* dst, data_proxpoi_reply* src)
{
	NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_proxpoi_reply);
	
    DATA_VEC_COPY(pds, err, dst->vec_proxmatch, src->vec_proxmatch, data_proxmatch);

    DATA_COPY(pds, err, &dst->iter_result, &src->iter_result, data_iter_result);

    DATA_COPY(pds, err, &dst->fuel_pricesummary, &src->fuel_pricesummary, data_fuel_pricesummary);

    DATA_VEC_COPY(pds, err, dst->vec_proxmatch_content, src->vec_proxmatch_content, data_proxmatch_content);

    DATA_VEC_COPY(pds, err, dst->vec_proxmatch_summary, src->vec_proxmatch_summary, data_proxmatch_summary);

    DATA_VEC_COPY(pds, err, dst->vec_proxmatch_ambiguous, src->vec_proxmatch_ambiguous, data_proxmatch_ambiguous);

    DATA_COPY(pds, err, &dst->event_content, &src->event_content, data_event_content);

    DATA_COPY(pds, err, &dst->spelling_suggestion, &src->spelling_suggestion, data_spelling_suggestion);

    DATA_COPY(pds, err, &dst->search_cookie, &src->search_cookie, data_search_cookie);

    DATA_COPY(pds, err, &dst->search_event_cookie, &src->search_event_cookie, data_search_event_cookie);

    DATA_COPY(pds, err, &dst->spelling_suggestion, &src->spelling_suggestion, data_spelling_suggestion);

	return err;
}

NB_Error	
data_proxpoi_reply_from_any_tps_with_proxmatch(data_util_state* pds, data_proxpoi_reply* pdat, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

    DATA_REINIT(pds, err, pdat, data_proxpoi_reply);

	if (err != NE_OK)
		goto errexit;
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "proxmatch") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_proxmatch, data_proxmatch);
		}

        if (err != NE_OK)
            goto errexit;
	}

errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_proxpoi_reply_from_any_tps_with_proxmatch failed (error_code: %d)", err));
		DATA_FREE(pds, pdat, data_proxpoi_reply);
    }

	return err;
}
