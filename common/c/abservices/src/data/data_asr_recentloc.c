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
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
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
 * data_asr_recentloc.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_recentloc.h"
#include "abexp.h"

NB_Error	
data_asr_recentloc_init(data_util_state* pds, data_asr_recentloc* ps)
{

	data_location_init(pds, &ps->recent_location);
	data_string_init(pds, &ps->city);
	data_string_init(pds, &ps->state);
	data_string_init(pds, &ps->country);
	data_string_init(pds, &ps->last_used);

	ps->frequency = 0;

	return NE_OK;
}

void		
data_asr_recentloc_free(data_util_state* pds, data_asr_recentloc* ps)
{
	data_location_free(pds, &ps->recent_location);
	data_string_free(pds, &ps->city);
	data_string_free(pds, &ps->state);
	data_string_free(pds, &ps->country);
	data_string_free(pds, &ps->last_used);
	ps->frequency = 0;
}

tpselt		
data_asr_recentloc_to_tps(data_util_state* pds, data_asr_recentloc* ps)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("recent-loc");

	if (te == NULL)
		goto errexit;

/*////////////// REMOVE ME ///////////////
	data_string_set(pds, &ps->city, "Los Angeles");
	data_string_set(pds, &ps->state, "CA");
	data_string_set(pds, &ps->country, "USA");*/

	ce = data_location_to_tps(pds, &ps->recent_location);
	te_attach(te, ce);
	ce = NULL;

	data_string_set(pds, &ps->last_used, "11/13/2007");
	ps->frequency = 45;
////////////////////////////////////////

	/*if (!te_setattrc(te, "city", data_string_get(pds, &ps->city)))
		goto errexit;

	if (!te_setattrc(te, "state", data_string_get(pds, &ps->state)))
		goto errexit;

	if (!te_setattrc(te, "country", data_string_get(pds, &ps->country)))
		goto errexit;*/

	if (!te_setattrc(te, "last-used", data_string_get(pds, &ps->last_used)))
		goto errexit;

	if (!te_setattru(te, "frequency", ps->frequency))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_Error	
data_asr_recentloc_from_tps(data_util_state* pds, data_asr_recentloc* ps, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_asr_recentloc_free(pds, ps);

	err = data_asr_recentloc_init(pds, ps);

	if (err != NE_OK)
		goto errexit;

	err = data_string_from_tps_attr(pds, &ps->city, te, "city");
	err = err ? err : data_string_from_tps_attr(pds, &ps->state, te, "state");
	err = err ? err : data_string_from_tps_attr(pds, &ps->country, te, "country");
	err = err ? err : data_string_from_tps_attr(pds, &ps->last_used, te, "last_used");
	ps->frequency = te_getattru(te, "frequency");

errexit:
	if (err != NE_OK)
		data_asr_recentloc_free(pds, ps);

	return err;
}

boolean		
data_asr_recentloc_equal(data_util_state* pds, data_asr_recentloc* ps1, data_asr_recentloc* ps2)
{
	return	(boolean) (data_string_equal(pds, &ps1->city, &ps2->city) &&
		data_string_equal(pds, &ps1->state, &ps2->state) &&
		data_string_equal(pds, &ps1->country, &ps2->country) &&
		data_string_equal(pds, &ps1->last_used, &ps2->last_used) &&
		ps1->frequency == ps2->frequency);
}

NB_Error	
data_asr_recentloc_copy(data_util_state* pds, data_asr_recentloc* ps_dest, data_asr_recentloc* ps_src)
{
	NB_Error err = NE_OK;

	data_asr_recentloc_free(pds, ps_dest);

	err = data_asr_recentloc_init(pds, ps_dest);

	err = err ? err : data_string_copy(pds, &ps_dest->city, &ps_src->city);
	err = err ? err : data_string_copy(pds, &ps_dest->state, &ps_src->state);
	err = err ? err : data_string_copy(pds, &ps_dest->country, &ps_src->country);
	err = err ? err : data_string_copy(pds, &ps_dest->last_used, &ps_src->last_used);

	ps_dest->frequency = ps_src->frequency;

	return err;
}
