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
 * data_proxmatch_summary.c: created 2007/12/06 by Tom Phan.
 */

#include "data_proxmatch_summary.h"

static void
data_proxmatch_summary_clear(data_util_state* pds, data_proxmatch_summary* pdat)
{
	pdat->count = 0;
    pdat->time = 0;
}

NB_Error	
data_proxmatch_summary_init(data_util_state* pds, data_proxmatch_summary* pdat)
{
	NB_Error err = NE_OK;

	err = err ? err : data_category_init(pds, &pdat->category);
    
    err = err ? err : data_search_filter_init(pds, &pdat->search_filter);

	data_proxmatch_summary_clear(pds, pdat);

	return err;
}

void	
data_proxmatch_summary_free(data_util_state* pds, data_proxmatch_summary* pdat)
{
	data_proxmatch_summary_clear(pds, pdat);

    data_search_filter_free(pds, &pdat->search_filter);

    data_category_free(pds, &pdat->category);
}

NB_Error	
data_proxmatch_summary_from_tps(data_util_state* pds, data_proxmatch_summary* pdat, tpselt te)
{
	NB_Error err = NE_OK;
    boolean has_category = FALSE;
    boolean has_search_filter = FALSE;
	tpselt	ce;
    int iter;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

    DATA_REINIT(pds, err, pdat, data_proxmatch_summary);

	if (err != NE_OK)
		return err;

    iter = 0;

    while (err == NE_OK && (ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "category") == 0) {

            has_category = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->category, data_category);
        }
        else if (nsl_strcmp(te_getname(ce), "search-filter") == 0) {

            has_search_filter = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->search_filter, data_search_filter);
        }
        if (err != NE_OK)
            goto errexit;
    }

    pdat->count = te_getattru(te, "count");
    pdat->time = te_getattru(te, "time"); 

    if (pdat->time == 0 || !has_search_filter || !has_category)
        err = NE_INVAL;

errexit:
	if (err != NE_OK)
		data_proxmatch_summary_free(pds, pdat);
	return err;
}

boolean		
data_proxmatch_summary_equal(data_util_state* pds, data_proxmatch_summary* lhs, data_proxmatch_summary* rhs)
{
    int ret = TRUE;

    ret = ret && data_category_equal(pds, &lhs->category, &rhs->category);

    ret = ret && data_search_filter_equal(pds, &lhs->search_filter, &rhs->search_filter);

    if (ret) {

        ret = ret && lhs->count == rhs->count;
        ret = ret && lhs->time == rhs->time;
    }

    return (boolean) ret;
}

NB_Error	
data_proxmatch_summary_copy(data_util_state* pds, data_proxmatch_summary* dst, data_proxmatch_summary* src)
{
	NB_Error err = NE_OK;


    DATA_REINIT(pds, err, dst, data_proxmatch_summary);

    DATA_COPY(pds, err, &dst->category, &src->category, data_category);

    DATA_COPY(pds, err, &dst->search_filter, &src->search_filter, data_search_filter);

    dst->count = src->count;

    dst->time = src->time;

	return err;
}
