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
 * data_map_source.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_map_source.h"
#include "data_string.h"
#include "data_point.h"
#include "vec.h"

static void
data_map_source_clear(data_util_state* pds, data_map_source* pms)
{
	pms->scale = 0.0;
	pms->rotate = 0.0;
}

NB_Error
data_map_source_init(data_util_state* pds, data_map_source* pms)
{
	NB_Error err = NE_OK;

	err = err ? err : data_point_init(pds, &pms->point);
	err = err ? err : data_string_init(pds, &pms->variant);

	data_map_source_clear(pds, pms);

	return NE_OK;
}

void		
data_map_source_free(data_util_state* pds, data_map_source* pms)
{
	data_point_free(pds, &pms->point);
	data_string_free(pds, &pms->variant);

	data_map_source_clear(pds, pms);
}

tpselt		
data_map_source_to_tps(data_util_state* pds, data_map_source* pms)
{
	tpselt te = NULL;
	tpselt ce = NULL;

	te = te_new("map-source");
	
	if (te == NULL)
		goto errexit;

	if ((ce = data_point_to_tps(pds, &pms->point)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (!te_setattrc(te, "variant", data_string_get(pds, &pms->variant)))
		goto errexit;

	if (!te_setattrf(te, "scale", pms->scale))
		goto errexit;

	if (!te_setattrf(te, "rotate", pms->rotate))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean		
data_map_source_equal(data_util_state* pds, data_map_source* pms1, data_map_source* pms2)
{
	return (boolean) (data_point_equal(pds, &pms1->point, &pms2->point) && 
					  data_string_equal(pds, &pms1->variant, &pms2->variant) &&
					  pms1->scale == pms2->scale &&
					  pms1->rotate == pms2->rotate);
}

NB_Error	
data_map_source_copy(data_util_state* pds, data_map_source* pms_dest, data_map_source* pms_src)
{
	NB_Error err = NE_OK;

	data_map_source_free(pds, pms_dest);

	err = err ? err : data_map_source_init(pds, pms_dest);

	err = err ? err : data_point_copy(pds, &pms_dest->point, &pms_src->point);
	err = err ? err : data_string_copy(pds, &pms_dest->variant, &pms_src->variant);
	
	pms_dest->scale = pms_src->scale;
	pms_dest->rotate = pms_src->rotate;

	return err;
}

