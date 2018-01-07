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
 * data_line.c: created 2007/11/07 by Michael Gilbert
 */

#include "data_line.h"
#include "nbspatial.h"

NB_Error
data_line_init(data_util_state* pds, data_line* pdl)
{
	pdl->priority = 0;

	data_polyline_pack_init(pds, &pdl->polyline_pack);
	data_string_init(pds, &pdl->label);

	return NE_OK;
}

void
data_line_free(data_util_state* pds, data_line* pdl)
{
	data_polyline_pack_free(pds, &pdl->polyline_pack);
	data_string_free(pds, &pdl->label);
}

NB_Error	
data_line_from_tps(data_util_state* pds, data_line* pdl, tpselt te, uint32 polyline_pack_passes)
{
	NB_Error err = NE_OK;
	tpselt	ce = NULL;
	const char* name;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_line_free(pds, pdl);

	err = data_line_init(pds, pdl);

	if (err != NE_OK)
		return err;

	name = te_getname(te);

	if (nsl_strcmp(name, "line-boundary") == 0)
		pdl->type = line_boundary;
	else if (nsl_strcmp(name, "line-rail") == 0)
		pdl->type = line_rail;
	else if (nsl_strcmp(name, "line-water") == 0)
		pdl->type = line_water;
	else
		pdl->type = line_none;

	ce = te_getchild(te, "polyline-pack");
	err = data_polyline_pack_from_tps_attr(pds, &pdl->polyline_pack, ce, "value", polyline_pack_passes);

	pdl->priority = te_getattru(te, "priority");

	err = err ? err : data_string_from_tps_attr(pds, &pdl->label, te, "label");
	
errexit:

	if (err != NE_OK)
		data_line_free(pds, pdl);
	return err;
}

NB_Error	
data_line_copy(data_util_state* pds, data_line* pdl_dest, data_line* pdl_src)
{
	NB_Error err = NE_OK;

	data_line_free(pds, pdl_dest);

	err = err ? err : data_line_init(pds, pdl_dest);

	err = err ? err : data_polyline_pack_copy(pds, &pdl_dest->polyline_pack, &pdl_src->polyline_pack);

	pdl_dest->priority = pdl_src->priority;
	pdl_dest->type = pdl_src->type;
	err = err ? err : data_string_copy(pds, &pdl_dest->label, &pdl_src->label);

	return err;
}

int			
data_line_compare(const void* r1, const void* r2)
{
	const data_line* ln1 = r1;
	const data_line* ln2 = r2;

	if (ln1->priority < ln2->priority)
		return -1;
	else if (ln1->priority == ln2->priority)
		return 0;
	else
		return 1;
}

