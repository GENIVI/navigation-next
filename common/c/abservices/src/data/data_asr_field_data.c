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
 * data_asr_field_data.c: created 2008/05/13 by NikunK.
 */

#include "data_asr_field_data.h"
#include "abexp.h"

NB_Error
data_asr_field_data_init(data_util_state* pds, data_asr_field_data* ps)
{
	NB_Error err = NE_OK;

	err = data_string_init(pds, &ps->field_name);
	err = err ? err : data_string_init(pds, &ps->field_type);
	err = err ? err : data_string_init(pds, &ps->field_text);
	ps->cursor_position = 0;
	err = err ? err : data_string_init(pds, &ps->utterance_id);
	err = err ? err : data_string_init(pds, &ps->utterance_encoding);
	err = err ? err : data_blob_init(pds, &ps->utterance_data);

	return NE_OK;
}

NB_Error	
data_asr_field_data_set(data_util_state* pds, data_asr_field_data* ps, const char* fieldName, const char* fieldType, const char* fieldText, uint32 cursorPosition,
												const char* utteranceId, const char* utteranceEncoding, byte* pUtteranceData, uint32 pUtteranceSize)
{
	NB_Error err = NE_OK;
	//data_blob addData;
	if (fieldName != NULL)
	{
		err = data_string_set(pds, &ps->field_name, fieldName);
	}
	if (fieldType != NULL)
	{
		err = err ? err : data_string_set(pds, &ps->field_type, fieldType);
	}
	if (fieldText != NULL)
	{
		err = err ? err : data_string_set(pds, &ps->field_text, fieldText);
	}
	if (utteranceId != NULL)
	{
		err = err ? err : data_string_set(pds, &ps->utterance_id, utteranceId);
	}
	//if (utteranceEncoding != NULL)
	{
		err = err ? err : data_string_set(pds, &ps->utterance_encoding, utteranceEncoding);
	}
	//if (pUtteranceData != NULL && pUtteranceSize > 0)
	{
		err = err ? err : data_blob_set(pds, &ps->utterance_data, pUtteranceData, pUtteranceSize);
	}

	return err;
}

void		
data_asr_field_data_free(data_util_state* pds, data_asr_field_data* ps)
{
	data_string_free(pds, &ps->field_name);
	data_string_free(pds, &ps->field_type);
	data_string_free(pds, &ps->field_text);
	data_string_free(pds, &ps->utterance_id);
	data_string_free(pds, &ps->utterance_encoding);
	data_blob_free(pds, &ps->utterance_data);

	ps->cursor_position = 0;
}

tpselt		
data_asr_field_data_to_tps(data_util_state* pds, data_asr_field_data* ps)
{
	tpselt te;
	//tpselt ce = NULL;

	te = te_new("field-data");

	if (te == NULL)
		goto errexit;

/*	if ((ce = data_blob_to_tps(pds, &pgq->utterance_data)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;
*/

	if (!te_setattrc(te, "field-name", data_string_get(pds, &ps->field_name)))
		goto errexit;

	if (!te_setattrc(te, "field-type", data_string_get(pds, &ps->field_type)))
		goto errexit;

	if (!te_setattrc(te, "field-text", data_string_get(pds, &ps->field_text)))
		goto errexit;

	if (!te_setattru(te, "cursor-position", ps->cursor_position))
		goto errexit;

	if (!te_setattrc(te, "utterance-id", data_string_get(pds, &ps->utterance_id)))
		goto errexit;

	if (!te_setattrc(te, "utterance-encoding", data_string_get(pds, &ps->utterance_encoding)))
		goto errexit;

	if (&ps->utterance_data && ps->utterance_data.size > 0)
	{
		if (!te_setattr(te, "utterance-data", (const char*) ps->utterance_data.data, ps->utterance_data.size))
			goto errexit;
	}
	else
	{
		if (!te_setattrc(te, "utterance-data", ""))
			goto errexit;
	}

	/*if (!te_setattrc(te, "utterance-data", data_string_get(pds, &ps->utterance_data)))
		goto errexit;*/

	return te;

errexit:

	te_dealloc(te);
    return NULL;
}

NB_Error	
data_asr_field_data_from_tps(data_util_state* pds, data_asr_field_data* ps, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce = NULL;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_asr_field_data_free(pds, ps);

	err = data_asr_field_data_init(pds, ps);

	if (err != NE_OK)
		goto errexit;

	ce = te_getchild(te, "field-data");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = data_string_from_tps_attr(pds, &ps->field_name, te, "field-name");
	err = err ? err : data_string_from_tps_attr(pds, &ps->field_type, te, "field-type");
	err = err ? err : data_string_from_tps_attr(pds, &ps->field_text, te, "field-text");
	ps->cursor_position = te_getattru(te, "cursor-position");
	err = err ? err : data_string_from_tps_attr(pds, &ps->utterance_id, te, "utterance-id");
	err = err ? err : data_string_from_tps_attr(pds, &ps->utterance_encoding, te, "utterance-encoding");
	err = err ? err : data_blob_from_tps_attr(pds, &ps->utterance_data, ce, "utterance-data");

errexit:
	if (err != NE_OK)
		data_asr_field_data_free(pds, ps);

	return err;
}

boolean		
data_asr_field_data_equal(data_util_state* pds, data_asr_field_data* ps1, data_asr_field_data* ps2)
{
	return	(boolean) (data_string_equal(pds, &ps1->field_name, &ps2->field_name) &&
							data_string_equal(pds, &ps1->field_type, &ps2->field_type) &&
							data_string_equal(pds, &ps1->field_text, &ps2->field_text) &&
							ps1->cursor_position == ps2->cursor_position &&
							data_string_equal(pds, &ps1->utterance_id, &ps2->utterance_id) &&
							data_string_equal(pds, &ps1->utterance_encoding, &ps2->utterance_encoding) &&
							data_blob_equal(pds, &ps1->utterance_data, &ps2->utterance_data)
							);
}

NB_Error	
data_asr_field_data_copy(data_util_state* pds, data_asr_field_data* ps_dest, data_asr_field_data* ps_src)
{
	NB_Error err = NE_OK;

	data_asr_field_data_free(pds, ps_dest);

	err = data_asr_field_data_init(pds, ps_dest);

	err = err ? err : data_string_copy(pds, &ps_dest->field_name, &ps_src->field_name);
	err = err ? err : data_string_copy(pds, &ps_dest->field_type, &ps_src->field_type);
	err = err ? err : data_string_copy(pds, &ps_dest->field_text, &ps_src->field_text);
	ps_dest->cursor_position = ps_src->cursor_position;
	err = err ? err : data_string_copy(pds, &ps_dest->utterance_id, &ps_src->utterance_id);
	err = err ? err : data_string_copy(pds, &ps_dest->utterance_encoding, &ps_src->utterance_encoding);
	err = err ? err : data_blob_copy(pds, &ps_dest->utterance_data, &ps_src->utterance_data);

	return err;
}
