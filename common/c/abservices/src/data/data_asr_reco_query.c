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
 * data_asr_reco_query.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_reco_query.h"
#include "abexp.h"

NB_Error
data_asr_reco_query_init(data_util_state* pds, data_asr_reco_query* pgq)
{
	NB_Error err = NE_OK;

	pgq->vec_recent_locs = CSL_VectorAlloc(sizeof(data_asr_recentloc));
	err = data_gps_init(pds, &pgq->gps);
	err = err ? err : data_location_init(pds, &pgq->location);
	err = err ? err : data_asr_field_data_init(pds, &pgq->field_data);
	pgq->vec_field_data = CSL_VectorAlloc(sizeof(data_asr_field_data));

	err = err ? err : data_string_init(pds, &pgq->user_lang);
	err = err ? err : data_string_init(pds, &pgq->asr_engine);
	err = err ? err : data_string_init(pds, &pgq->asr_id);

	err = err ? err : data_string_set(pds, &pgq->user_lang, "en-US");
	err = err ? err : data_string_set(pds, &pgq->asr_engine, "unknown");
	err = err ? err : data_string_set(pds, &pgq->asr_id, "");

    pgq->want_accuracy = FALSE;
    pgq->want_extended_codes = FALSE;
    
	if (err)
		data_asr_reco_query_free(pds, pgq);

	return err;
}

void		
data_asr_reco_query_free(data_util_state* pds, data_asr_reco_query* pgq)
{
	int l = 0, n = 0;
	if (pgq->vec_recent_locs)
	{
		l = CSL_VectorGetLength(pgq->vec_recent_locs);

		for (n = 0; n < l; n++)
		{
			data_asr_recentloc_free(pds, (data_asr_recentloc*) CSL_VectorGetPointer(pgq->vec_recent_locs, n));
		}
		CSL_VectorDealloc(pgq->vec_recent_locs);
		pgq->vec_recent_locs = NULL;
	}

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

	data_gps_free(pds, &pgq->gps);
	data_location_free(pds, &pgq->location);
	data_asr_field_data_free(pds, &pgq->field_data);

	data_string_free(pds, &pgq->user_lang);
	data_string_free(pds, &pgq->asr_engine);
	data_string_free(pds, &pgq->asr_id);
}

NB_Error
data_asr_reco_query_add_field_data(data_util_state* pds, data_asr_reco_query* pgq, const char* fieldName, const char* fieldType, const char* fieldText, uint32 cursorPosition,
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
		err = data_asr_field_data_init(pds, &fd);

	return err;
}

NB_Error
data_asr_reco_query_set_gps_fix(data_util_state* pds, data_asr_reco_query* pgq, NB_GpsLocation fix)
{
	NB_Error err = NE_OK;
	data_gps_from_gpsfix(pds, &pgq->gps, &fix);
	return err;
}

NB_Error
data_asr_reco_set_id(data_util_state* pds, data_asr_reco_query* pgq, const char* asr_id)
{
	NB_Error err = NE_OK;
	err = data_string_set(pds, &pgq->asr_id, asr_id);
	return err;
}

NB_Error
data_asr_reco_query_clear_fields(data_util_state* pds, data_asr_reco_query* pgq)
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

tpselt		
data_asr_reco_query_to_tps(data_util_state* pds, data_asr_reco_query* pgq)
{
	int l = 0, n = 0;
	tpselt te;
	tpselt ce = NULL;
	byte ff = 0xFF;

	te = te_new("asr-reco-query");

	if (te == NULL)
		goto errexit;

	l = CSL_VectorGetLength(pgq->vec_recent_locs);
	for (n = 0; n < l; n++)
	{
		if ((ce = data_asr_recentloc_to_tps(pds, CSL_VectorGetPointer(pgq->vec_recent_locs, n))) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	l = CSL_VectorGetLength(pgq->vec_field_data);
	for (n = 0; n < l; n++)
	{
		if ((ce = data_asr_field_data_to_tps(pds, CSL_VectorGetPointer(pgq->vec_field_data, n))) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (nsl_memcmp(&pgq->gps.packed[0], &ff, sizeof(byte)) != 0)
	{
		if ((ce = data_gps_to_tps(pds, &pgq->gps)) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

    if (pgq->want_accuracy)
    {
        if ((ce = te_new("want-accuracy")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pgq->want_extended_codes)
    {
        if ((ce = te_new("want-extended-codes")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

	if (!te_setattrc(te, "user-lang", data_string_get(pds, &pgq->user_lang)))
		goto errexit;

	if (!te_setattrc(te, "asr-engine", data_string_get(pds, &pgq->asr_engine)))
		goto errexit;

	if (!te_setattrc(te, "asr-id", data_string_get(pds, &pgq->asr_id)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean		
data_asr_reco_query_equal(data_util_state* pds, data_asr_reco_query* pgq1, data_asr_reco_query* pgq2)
{
	int l1 = 0, l2 = 0, n = 0;

	if (pgq1->vec_recent_locs && pgq2->vec_recent_locs)
	{
		l1 = CSL_VectorGetLength(pgq1->vec_recent_locs);
		l2 = CSL_VectorGetLength(pgq2->vec_recent_locs);

		if (l1 != l2)
		{
			return FALSE;
		}

		for (n = 0; n < l1; n++)
		{
			if (!data_asr_recentloc_equal(pds, (data_asr_recentloc*) CSL_VectorGetPointer(pgq1->vec_recent_locs, n), (data_asr_recentloc*) CSL_VectorGetPointer(pgq2->vec_recent_locs, n)))
				return FALSE;
		}
	}
	else if ((pgq1->vec_recent_locs && !pgq2->vec_recent_locs) || (!pgq1->vec_recent_locs && pgq2->vec_recent_locs))
	{
		return FALSE;
	}

	return	(boolean) (data_gps_equal(pds, &pgq1->gps, &pgq2->gps) &&
			data_location_equal(pds, &pgq1->location, &pgq2->location) &&
			data_asr_field_data_equal(pds, &pgq1->field_data, &pgq2->field_data) &&
			data_string_equal(pds, &pgq1->user_lang, &pgq2->user_lang) &&
			data_string_equal(pds, &pgq1->asr_engine, &pgq2->asr_engine) &&
			data_string_equal(pds, &pgq1->asr_id, &pgq2->asr_id) &&
            pgq1->want_accuracy == pgq2->want_accuracy &&
            pgq1->want_extended_codes == pgq2->want_extended_codes);
}

NB_Error	
data_asr_reco_query_copy(data_util_state* pds, data_asr_reco_query* pgq_dest, data_asr_reco_query* pgq_src)
{
	NB_Error err = NE_OK;
	int n = 0, l = 0;
	data_asr_field_data vec_field_data;
	data_asr_reco_query_free(pds, pgq_dest);

	data_asr_field_data_init(pds, &vec_field_data);
	err = data_asr_reco_query_init(pds, pgq_dest);

	err = err ? err : data_gps_copy(pds, &pgq_dest->gps, &pgq_src->gps);
	err = err ? err : data_location_copy(pds, &pgq_dest->location, &pgq_src->location);
	err = err ? err : data_asr_field_data_copy(pds, &pgq_dest->field_data, &pgq_src->field_data);

	err = err ? err : data_string_copy(pds, &pgq_dest->user_lang, &pgq_src->user_lang);
	err = err ? err : data_string_copy(pds, &pgq_dest->asr_engine, &pgq_src->asr_engine);
	err = err ? err : data_string_copy(pds, &pgq_dest->asr_id, &pgq_src->asr_id);

	l = CSL_VectorGetLength(pgq_src->vec_field_data);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_asr_field_data_copy(pds, &vec_field_data, CSL_VectorGetPointer(pgq_src->vec_field_data, n));
		err = err ? err : CSL_VectorAppend(pgq_dest->vec_field_data, &vec_field_data) ? NE_OK : NE_NOMEM;

		if (err)
			data_asr_field_data_free(pds, &vec_field_data);
		else
			err = err ? err : data_asr_field_data_init(pds, &vec_field_data);
	}
	
    pgq_dest->want_accuracy = pgq_src->want_accuracy;
    pgq_dest->want_extended_codes = pgq_src->want_extended_codes;
	
	return err;
}
