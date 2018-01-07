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
 * data_asr_stats_query.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_stats_query.h"
#include "abexp.h"

NB_Error	
data_asr_stats_query_init(data_util_state* pds, data_asr_stats_query* pgq)
{
	NB_Error err = NE_OK;

	pgq->vec_field_data = CSL_VectorAlloc(sizeof(data_asr_field_data));

	err = err ? err : data_string_init(pds, &pgq->user_lang);
    err = err ? err : data_string_init(pds, &pgq->asr_engine);
    err = err ? err : data_string_init(pds, &pgq->asr_id);
	err = err ? err : data_string_init(pds, &pgq->asr_timing);


	err = err ? err : data_string_set(pds, &pgq->user_lang, "en-US");
    err = err ? err : data_string_set(pds, &pgq->asr_engine, "unknown");
    err = err ? err : data_string_set(pds, &pgq->asr_id, "");
	err = err ? err : data_string_set(pds, &pgq->asr_timing, "");

	if (err)
		data_asr_stats_query_free(pds, pgq);

	return err;
}

void		
data_asr_stats_query_free(data_util_state* pds, data_asr_stats_query* pgq)
{
	int l = 0, n = 0;

	if (pgq->vec_field_data)
	{
		l = CSL_VectorGetLength(pgq->vec_field_data);

		for (n = 0; n < l; n++)
		{
			data_asr_field_data_free(pds, (data_asr_field_data*) CSL_VectorGetPointer(pgq->vec_field_data, n));
		}
		CSL_VectorDealloc(pgq->vec_field_data);
		pgq->vec_field_data = NULL;
	}

	data_string_free(pds, &pgq->user_lang);
    data_string_free(pds, &pgq->asr_engine);
    data_string_free(pds, &pgq->asr_id);
	data_string_free(pds, &pgq->asr_timing);
}

NB_Error
data_asr_stats_set_id(data_util_state* pds, data_asr_stats_query* pgq, const char* asr_id)
{
	NB_Error err = NE_OK;
	err = data_string_set(pds, &pgq->asr_id, asr_id);
	return err;
}
NB_Error
data_asr_stats_query_add_field_data(data_util_state* pds, data_asr_stats_query* pgq, const char* fieldName, const char* fieldType, const char* fieldText, uint32 cursorPosition,
																	 const char* utteranceId, const char* utteranceEncoding, byte* pUtteranceData, uint32 pUtteranceSize)
{
	NB_Error err = NE_OK;
	data_asr_field_data fd;

	err = data_asr_field_data_init(pds, &fd);

	if (fieldName != NULL)
	{
		err = err ? err : data_string_set(pds, &fd.field_name, fieldName);
	}
	if (fieldType != NULL)
	{
		err = err ? err : data_string_set(pds, &fd.field_type, fieldType);
	}
	if (fieldText != NULL)
	{
		err = err ? err : data_string_set(pds, &fd.field_text, fieldText);
	}
	if (utteranceId != NULL)
	{
		err = err ? err : data_string_set(pds, &fd.utterance_id, utteranceId);
	}
	if (utteranceEncoding != NULL)
	{
		err = err ? err : data_string_set(pds, &fd.utterance_encoding, utteranceEncoding);
	}
	if (pUtteranceData != NULL && pUtteranceSize > 0)
	{
		err = err ? err : data_blob_set(pds, &fd.utterance_data, pUtteranceData, pUtteranceSize);
	}

	err = err ? err : CSL_VectorAppend(pgq->vec_field_data, &fd) ? NE_OK : NE_NOMEM;

	if (err)
		data_asr_field_data_free(pds, &fd);
	else
		err = err ? err : data_asr_field_data_init(pds, &fd);

	return err;
}

NB_Error
data_asr_stats_query_clear_fields(data_util_state* pds, data_asr_stats_query* pgq)
{
	int l = 0, n = 0;
	if (pgq->vec_field_data)
	{
		l = CSL_VectorGetLength(pgq->vec_field_data);

		for (n = 0; n < l; n++)
		{
			data_asr_field_data_free(pds, (data_asr_field_data*) CSL_VectorGetPointer(pgq->vec_field_data, n));
		}
		CSL_VectorDealloc(pgq->vec_field_data);
		pgq->vec_field_data = NULL;
	}

	pgq->vec_field_data = CSL_VectorAlloc(sizeof(data_asr_field_data));

	return NE_OK;
}

NB_Error
data_asr_stats_query_set_timing(data_util_state* pds, data_asr_stats_query* pgq, const char* timing)
{
    NB_Error err = NE_OK;
    err = data_string_set(pds, &pgq->asr_timing, timing);
    return err;
}

tpselt		
data_asr_stats_query_to_tps(data_util_state* pds, data_asr_stats_query* pgq)
{
	tpselt te;
	tpselt ce = NULL;
	int l = 0, n = 0;

	te = te_new("asr-stats-query");

	if (te == NULL)
		goto errexit;
	
	l = CSL_VectorGetLength(pgq->vec_field_data);
	for (n = 0; n < l; n++)
	{
		if ((ce = data_asr_field_data_to_tps(pds, CSL_VectorGetPointer(pgq->vec_field_data, n))) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (!te_setattrc(te, "user-lang", data_string_get(pds, &pgq->user_lang)))
		goto errexit;

    if (!te_setattrc(te, "asr-engine", data_string_get(pds, &pgq->asr_engine)))
        goto errexit;

    if (!te_setattrc(te, "asr-id", data_string_get(pds, &pgq->asr_id)))
        goto errexit;

	if (!te_setattrc(te, "timing", data_string_get(pds, &pgq->asr_timing)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean		
data_asr_stats_query_equal(data_util_state* pds, data_asr_stats_query* pgq1, data_asr_stats_query* pgq2)
{
	return	(boolean) (data_string_equal(pds, &pgq1->user_lang, &pgq2->user_lang) &&
            data_string_equal(pds, &pgq1->asr_engine, &pgq2->asr_engine) &&
            data_string_equal(pds, &pgq1->asr_id, &pgq2->asr_id) &&
			data_string_equal(pds, &pgq1->asr_timing, &pgq2->asr_timing));
}

NB_Error	
data_asr_stats_query_copy(data_util_state* pds, data_asr_stats_query* pgq_dest, data_asr_stats_query* pgq_src)
{
	NB_Error err = NE_OK;
	int n = 0, l = 0;
	data_asr_field_data vec_field_data;
	data_asr_stats_query_free(pds, pgq_dest);

	err = err ? err : data_asr_stats_query_init(pds, pgq_dest);

	err = err ? err : data_string_copy(pds, &pgq_dest->user_lang, &pgq_src->user_lang);
    err = err ? err : data_string_copy(pds, &pgq_dest->asr_engine, &pgq_src->asr_engine);
    err = err ? err : data_string_copy(pds, &pgq_dest->asr_id, &pgq_src->asr_id);
	err = err ? err : data_string_copy(pds, &pgq_dest->asr_timing, &pgq_src->asr_timing);

	l = CSL_VectorGetLength(pgq_src->vec_field_data);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_asr_field_data_copy(pds, &vec_field_data, CSL_VectorGetPointer(pgq_src->vec_field_data, n));
		err = err ? err : CSL_VectorAppend(pgq_dest->vec_field_data, &vec_field_data) ? NE_OK : NE_NOMEM;

		if (err)
			data_asr_field_data_free(pds, &vec_field_data);
		else
			err = err ? err : data_asr_field_data_init(pds, &vec_field_data);
	}
	return err;
}
