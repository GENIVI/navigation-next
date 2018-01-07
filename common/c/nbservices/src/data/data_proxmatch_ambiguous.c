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
 * data_proxmatch_ambiguous.c: created 2007/11/06 by Tom Phan.
 */

#include "data_proxmatch_ambiguous.h"

NB_Error	
data_proxmatch_ambiguous_init(data_util_state* pds, data_proxmatch_ambiguous* pdat)
{
	NB_Error err = NE_OK;

	err = err ? err : data_search_filter_init(pds, &pdat->search_filter);

    err = err ? err : data_string_init(pds, &pdat->caption);

	return err;
}

void	
data_proxmatch_ambiguous_free(data_util_state* pds, data_proxmatch_ambiguous* pdat)
{
    data_string_free(pds, &pdat->caption);

	data_search_filter_free(pds, &pdat->search_filter);
}

NB_Error	
data_proxmatch_ambiguous_from_tps(data_util_state* pds, data_proxmatch_ambiguous* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_proxmatch_ambiguous);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pdat->caption, te, "caption");

    if (err != NE_OK)
        goto errexit;

    ce = te_getchild(te, "search-filter");

    if (ce == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_FROM_TPS(pds, err, ce, &pdat->search_filter, data_search_filter);


errexit:
    if (err != NE_OK)
        DATA_FREE(pds, pdat, data_proxmatch_ambiguous);

    return err;
}

boolean		
data_proxmatch_ambiguous_equal(data_util_state* pds, data_proxmatch_ambiguous* lhs, data_proxmatch_ambiguous* rhs)
{
    int ret = TRUE;

    ret = ret && data_search_filter_equal(pds, &lhs->search_filter, &rhs->search_filter);

    ret = ret && data_string_equal(pds, &lhs->caption, &rhs->caption);

    return (boolean) ret;
}

NB_Error	
data_proxmatch_ambiguous_copy(data_util_state* pds, data_proxmatch_ambiguous* dst, data_proxmatch_ambiguous* src)
{
	NB_Error err = NE_OK;

	data_proxmatch_ambiguous_free(pds, dst);
	data_proxmatch_ambiguous_init(pds, dst);

	err = err ? err : data_search_filter_copy(pds, &dst->search_filter, &src->search_filter);

    err = err ? err : data_string_copy(pds, &dst->caption, &src->caption);

	return err;
}
