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
 * data_asr_timing.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_timing.h"
#include "abexp.h"

NB_Error	
data_asr_timing_init(data_util_state* pds, data_asr_timing* ps)
{
	NB_Error err = NE_OK;

	err = data_string_init(pds, &ps->type);
	ps->value = 0;

	return err;
}

void		
data_asr_timing_free(data_util_state* pds, data_asr_timing* ps)
{
	data_string_free(pds, &ps->type);
}

tpselt		
data_asr_timing_to_tps(data_util_state* pds, data_asr_timing* ps)
{
	tpselt te;

	te = te_new("asr-timing");

	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "type", ps->type))
		goto errexit;

	if (!te_setattru(te, "value", ps->value))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_Error	
data_asr_timing_from_tps(data_util_state* pds, data_asr_timing* ps, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_asr_timing_free(pds, ps);

	err = data_asr_timing_init(pds, ps);

	if (err != NE_OK)
		goto errexit;

	err = err ? err : data_string_from_tps_attr(pds, &ps->type, te, "type");
	ps->value = te_getattru(te, "value");

errexit:
	if (err != NE_OK)
		data_asr_timing_free(pds, ps);

	return err;
}

boolean		
data_asr_timing_equal(data_util_state* pds, data_asr_timing* ps1, data_asr_timing* ps2)
{
	return	(boolean) (data_string_equal(pds, &ps1->type, &ps2->type) &&
					   ps1->value == ps2->value);
}

NB_Error	
data_asr_timing_copy(data_util_state* pds, data_asr_timing* ps_dest, data_asr_timing* ps_src)
{
	NB_Error err = NE_OK;

	data_asr_timing_free(pds, ps_dest);

	err = err ? err : data_asr_timing_init(pds, ps_dest);

	err = err ? err : data_string_copy(pds, &ps_dest->type, &ps_src->type);
	ps_dest->value = ps_src->value;

	return err;
}
