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
 * data_proxmatch_content.c: created 2007/11/06 by Tom Phan.
 */

#include "data_proxmatch_content.h"

NB_Error	
data_proxmatch_content_init(data_util_state* pds, data_proxmatch_content* pdat)
{
	NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_proxmatch_content);

    DATA_INIT(pds, err, &pdat->search_filter, data_search_filter);

    DATA_INIT(pds, err, &pdat->event_content, data_event_content);

	return err;
}

void	
data_proxmatch_content_free(data_util_state* pds, data_proxmatch_content* pdat)
{
    DATA_FREE(pds, &pdat->event_content, data_event_content);

    DATA_FREE(pds, &pdat->search_filter, data_search_filter);
}

NB_Error	
data_proxmatch_content_from_tps(data_util_state* pds, data_proxmatch_content* pdat, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}


    DATA_REINIT(pds, err, pdat, data_proxmatch_content);

	if (err != NE_OK)
		return err;
	
 	ce = te_getchild(te, "search-filter");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

    DATA_FROM_TPS(pds, err, ce, &pdat->search_filter, data_search_filter);

    ce = te_getchild(te, "event-content");

    if (ce == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_FROM_TPS(pds, err, ce, &pdat->event_content, data_event_content);


errexit:
	if (err != NE_OK)
		data_proxmatch_content_free(pds, pdat);
	return err;
}

boolean		
data_proxmatch_content_equal(data_util_state* pds, data_proxmatch_content* lhs, data_proxmatch_content* rhs)
{
    int ret = TRUE;

    ret = ret && data_search_filter_equal(pds, &lhs->search_filter, &rhs->search_filter);

    ret = ret && data_event_content_equal(pds, &lhs->event_content, &rhs->event_content);

    return (boolean) ret;
}

NB_Error	
data_proxmatch_content_copy(data_util_state* pds, data_proxmatch_content* dst, data_proxmatch_content* src)
{
	NB_Error err = NE_OK;

	data_proxmatch_content_free(pds, dst);
	data_proxmatch_content_init(pds, dst);

	err = err ? err : data_search_filter_copy(pds, &dst->search_filter, &src->search_filter);

    err = err ? err : data_event_content_copy(pds, &dst->event_content, &dst->event_content);

	return err;
}
