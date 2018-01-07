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
/* (C) Copyright 2007 by Networks In Motion, Inc.                */
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
 * data_detour_avoid.c: created 2007/09/12 by Michael Gilbert.
 */

#include "data_detour_avoid.h"
//#include "tobuf.h"

NB_Error	
data_detour_avoid_init(data_util_state* pds, data_detour_avoid* pda)
{
	NB_Error err = NE_OK;

	pda->start = 0.0;
	pda->length = 0.0;

	err = err ? err : data_blob_init(pds, &pda->route_id);
	err = err ? err : data_route_polyline_init(pds, &pda->route_polyline);
	err = err ? err : data_string_init(pds, &pda->label);

	pda->want_historical_traffic = FALSE;
	pda->want_realtime_traffic = FALSE;

	if (err)
		data_detour_avoid_free(pds, pda);

	return err;
}

void	
data_detour_avoid_free(data_util_state* pds, data_detour_avoid* pda)
{
	data_blob_free(pds, &pda->route_id);
	data_string_free(pds, &pda->label);
}

tpselt		
data_detour_avoid_to_tps(data_util_state* pds, data_detour_avoid* pda)
{
	tpselt te = NULL;
	tpselt ce = NULL;

	te = te_new("detour-avoid");
	
	if (te == NULL)
		goto errexit;
	if (pda->start != 0.0)
		if (!te_setattrd(te, "start", pda->start))
			goto errexit;

	if (pda->length != 0.0)
		if (!te_setattrd(te, "length", pda->length))
			goto errexit;

	if (!te_setattrc(te, "label", data_string_get(pds, &pda->label)))
		goto errexit;

	if (pda->route_id.size > 0 && !te_setattr(te, "route-id", (const char*) pda->route_id.data, pda->route_id.size))
		goto errexit;

	if (pda->want_historical_traffic) {

		if ((ce = te_new("want-historical-traffic")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (pda->want_realtime_traffic) {

		if ((ce = te_new("want-realtime-traffic")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error	
data_detour_avoid_from_tps(data_util_state* pds, data_detour_avoid* pda, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;

	if (!te_getattrd(te, "start", &pda->start) || !te_getattrd(te, "length", &pda->length))
		return NE_INVAL;

	err = err ? err : data_blob_from_tps_attr(pds, &pda->route_id, te, "route-id");

	if (te_getattrc(te, "label") != NULL)
		err = err ? err : data_string_from_tps_attr(pds, &pda->label, te, "label");

	if ((ce = te_getchild(te, "route-polyline")) != NULL)
		err = err ? err : data_route_polyline_from_tps(pds, &pda->route_polyline, ce);

	pda->want_historical_traffic = (te_getchild(te, "want-historical-traffic") != NULL) ? TRUE : FALSE;
	pda->want_realtime_traffic = (te_getchild(te, "want-realtime-traffic") != NULL) ? TRUE : FALSE;
		
	return err;
}

NB_Error	
data_detour_avoid_copy(data_util_state* pds, data_detour_avoid* pda_dest, data_detour_avoid* pda_src, boolean copy_polyline)
{
	NB_Error err = NE_OK;

	data_detour_avoid_free(pds, pda_dest);
	data_detour_avoid_init(pds, pda_dest);

	pda_dest->start = pda_src->start;
	pda_dest->length = pda_src->length;

	err = err ? err : data_blob_copy(pds, &pda_dest->route_id, &pda_src->route_id);
	err = err ? err : data_string_copy(pds, &pda_dest->label, &pda_src->label);

	if (copy_polyline)
	{
		err = err ? err : data_route_polyline_copy(pds, &pda_dest->route_polyline, &pda_src->route_polyline);
	}

	pda_dest->want_historical_traffic = pda_src->want_historical_traffic;
	pda_dest->want_realtime_traffic = pda_src->want_realtime_traffic;

	return err;
}
