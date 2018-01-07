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
 * data_area.c: created 2007/11/07 by Michael Gilbert
 */

#include "data_area.h"

NB_Error
data_area_init(data_util_state* pds, data_area* pda)
{
	pda->priority = 0;

	data_polyline_pack_init(pds, &pda->polyline_pack);
	data_string_init(pds, &pda->label);

	return NE_OK;
}

void
data_area_free(data_util_state* pds, data_area* pda)
{
	data_polyline_pack_free(pds, &pda->polyline_pack);
	data_string_free(pds, &pda->label);
}

NB_Error	
data_area_from_tps(data_util_state* pds, data_area* pda, tpselt te, uint32 polyline_pack_passes)
{
	NB_Error err = NE_OK;
	tpselt	ce = NULL;
	const char* name;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_area_free(pds, pda);

	err = data_area_init(pds, pda);

	if (err != NE_OK)
		return err;

	name = te_getname(te);

	if (nsl_strcmp(name, "area-building") == 0)
		pda->type = area_building;
	else if (nsl_strcmp(name, "area-interior") == 0)
		pda->type = area_interior;
	else if (nsl_strcmp(name, "area-water") == 0)
		pda->type = area_water;
	else if (nsl_strcmp(name, "area-campus") == 0)
		pda->type = area_campus;
	else if (nsl_strcmp(name, "area-parks") == 0)
		pda->type = area_parks;
	else if (nsl_strcmp(name, "area-geopolitical") == 0)
		pda->type = area_geopolitical;
	else 
		pda->type = area_none;

	ce = te_getchild(te, "polyline-pack");
	err = data_polyline_pack_from_tps_attr(pds, &pda->polyline_pack, ce, "value", polyline_pack_passes);

	pda->priority = te_getattru(te, "priority");

	err = err ? err : data_string_from_tps_attr(pds, &pda->label, te, "label");
	
errexit:

	if (err != NE_OK)
		data_area_free(pds, pda);
	return err;
}

NB_Error	
data_area_copy(data_util_state* pds, data_area* pda_dest, data_area* pda_src)
{
	NB_Error err = NE_OK;

	data_area_free(pds, pda_dest);

	err = err ? err : data_area_init(pds, pda_dest);

	err = err ? err : data_polyline_pack_copy(pds, &pda_dest->polyline_pack, &pda_src->polyline_pack);

	pda_dest->priority = pda_src->priority;
	pda_dest->type = pda_src->type;
	err = err ? err : data_string_copy(pds, &pda_dest->label, &pda_src->label);

	return err;
}

int			
data_area_compare(const void* r1, const void* r2)
{
	const data_area* ln1 = r1;
	const data_area* ln2 = r2;

	if (ln1->priority < ln2->priority)
		return -1;
	else if (ln1->priority == ln2->priority)
		return 0;
	else
		return 1;
}

