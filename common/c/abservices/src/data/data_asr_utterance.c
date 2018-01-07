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
 * data_asr_utterance.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_utterance.h"
#include "abexp.h"

NB_Error	
data_asr_utterance_init(data_util_state* pds, data_asr_utterance* ps)
{
	NB_Error err = NE_OK;

	err = err ? err : data_blob_init(pds, &ps->utterance_data);
	err = err ? err : data_string_init(pds, &ps->audio_encoding);
	err = err ? err : data_string_init(pds, &ps->utterance_saved_id);
	err = err ? err : data_string_init(pds, &ps->utterance_context);

	return NE_OK;
}

void		
data_asr_utterance_free(data_util_state* pds, data_asr_utterance* ps)
{
	data_blob_free(pds, &ps->utterance_data);
	data_string_free(pds, &ps->audio_encoding);
	data_string_free(pds, &ps->utterance_saved_id);
	data_string_free(pds, &ps->utterance_context);
}

tpselt		
data_asr_utterance_to_tps(data_util_state* pds, data_asr_utterance* ps)
{
	tpselt te;
	//tpselt ce = NULL;

	te = te_new("sliceres");

	if (te == NULL)
		goto errexit;

/*	if ((ce = data_blob_to_tps(pds, &pgq->utterance_data)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;
*/

	if (!te_setattrc(te, "audio-encoding", data_string_get(pds, &ps->audio_encoding)))
		goto errexit;

	if (!te_setattrc(te, "utterance-saved-id", data_string_get(pds, &ps->utterance_saved_id)))
		goto errexit;

	if (!te_setattrc(te, "utterance-context", data_string_get(pds, &ps->utterance_context)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_Error	
data_asr_utterance_from_tps(data_util_state* pds, data_asr_utterance* ps, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce = NULL;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_asr_utterance_free(pds, ps);

	err = data_asr_utterance_init(pds, ps);

	if (err != NE_OK)
		goto errexit;

	ce = te_getchild(te, "utterance-data");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	//err = data_blob_from_tps(pds, &ps->utterance_data, ce);

	if (err != NE_OK)
		return err;

	err = err ? err : data_string_from_tps_attr(pds, &ps->audio_encoding, te, "audio-encoding");
	err = err ? err : data_string_from_tps_attr(pds, &ps->utterance_saved_id, te, "utterance-saved-id");
	err = err ? err : data_string_from_tps_attr(pds, &ps->utterance_context, te, "utterance-context");

errexit:
	if (err != NE_OK)
		data_asr_utterance_free(pds, ps);

	return err;
}

boolean		
data_asr_utterance_equal(data_util_state* pds, data_asr_utterance* ps1, data_asr_utterance* ps2)
{
	return	(boolean) (data_blob_equal(pds, &ps1->utterance_data, &ps2->utterance_data) && 
							data_string_equal(pds, &ps1->audio_encoding, &ps2->audio_encoding) &&
							data_string_equal(pds, &ps1->utterance_saved_id, &ps2->utterance_saved_id) &&
							data_string_equal(pds, &ps1->utterance_context, &ps2->utterance_context));
}

NB_Error	
data_asr_utterance_copy(data_util_state* pds, data_asr_utterance* ps_dest, data_asr_utterance* ps_src)
{
	NB_Error err = NE_OK;

	data_asr_utterance_free(pds, ps_dest);

	err = err ? err : data_asr_utterance_init(pds, ps_dest);

	err = err ? err : data_blob_copy(pds, &ps_dest->utterance_data, &ps_src->utterance_data);
	err = err ? err : data_string_copy(pds, &ps_dest->audio_encoding, &ps_src->audio_encoding);
	err = err ? err : data_string_copy(pds, &ps_dest->utterance_saved_id, &ps_src->utterance_saved_id);
	err = err ? err : data_string_copy(pds, &ps_dest->utterance_context, &ps_src->utterance_context);

	return err;
}
