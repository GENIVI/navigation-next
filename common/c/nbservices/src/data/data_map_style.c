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
 * data_map_style.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_map_style.h"
#include "data_string.h"
#include "vec.h"

static void
data_map_style_clear(data_util_state* pds, data_map_style* pms)
{
	pms->maneuver_valid = FALSE;
}

NB_Error
data_map_style_init(data_util_state* pds, data_map_style* pms)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pms->scheme);
	err = err ? err : data_string_init(pds, &pms->legend);
	err = err ? err : data_blob_init(pds, &pms->route_id);
	err = err ? err : data_blob_init(pds, &pms->alt_route_id);

	data_map_style_clear(pds, pms);

	return err;
}

void		
data_map_style_free(data_util_state* pds, data_map_style* pms)
{

	data_string_free(pds, &pms->scheme);
	data_string_free(pds, &pms->legend);
	data_blob_free(pds, &pms->route_id);
	data_blob_free(pds, &pms->alt_route_id);

	data_map_style_clear(pds, pms);
}

tpselt		
data_map_style_to_tps(data_util_state* pds, data_map_style* pms)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("map-style");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "scheme", data_string_get(pds, &pms->scheme)))
		goto errexit;

	if (!te_setattrc(te, "legend", data_string_get(pds, &pms->legend)))
		goto errexit;

	if (!te_setattr(te, "route-id", (const char*) pms->route_id.data, pms->route_id.size))
		goto errexit;

	if (!te_setattr(te, "alt-route-id", (const char*) pms->alt_route_id.data, pms->alt_route_id.size))
		goto errexit;

	if (pms->maneuver_valid) {

		if (!te_setattru(te, "maneuver", pms->maneuver))
			goto errexit;
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean		
data_map_style_equal(data_util_state* pds, data_map_style* pms1, data_map_style* pms2)
{
	int ret = TRUE;

	ret = ret && data_string_equal(pds, &pms1->scheme, &pms2->scheme);
	ret = ret && data_string_equal(pds, &pms1->legend, &pms2->legend);
	ret = ret && data_blob_equal(pds, &pms1->route_id, &pms2->route_id);
	ret = ret && data_blob_equal(pds, &pms1->alt_route_id, &pms2->alt_route_id);

	ret = ret && ((!pms1->maneuver_valid && !pms2->maneuver_valid) ||
				  (pms1->maneuver_valid && pms2->maneuver_valid &&
				   pms1->maneuver == pms2->maneuver));

	return (boolean) ret;
}

NB_Error	
data_map_style_copy(data_util_state* pds, data_map_style* pms_dest, data_map_style* pms_src)
{
	NB_Error err = NE_OK;

	data_map_style_free(pds, pms_dest);
	err = err ? err : data_map_style_init(pds, pms_dest);
	
	err = err ? err : data_string_copy(pds, &pms_dest->scheme, &pms_src->scheme);
	err = err ? err : data_string_copy(pds, &pms_dest->legend, &pms_src->legend);
	err = err ? err : data_blob_copy(pds, &pms_dest->route_id, &pms_src->route_id);
	err = err ? err : data_blob_copy(pds, &pms_dest->alt_route_id, &pms_src->alt_route_id);

	pms_dest->maneuver_valid = pms_src->maneuver_valid;
	pms_dest->maneuver = pms_src->maneuver;

	return err;
}
