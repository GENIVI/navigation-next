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
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
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

/*
 * data_message_confirm_query.c: created 2006/09/29 by Chetan Nagaraj.
 */

#include "data_message_confirm_query.h"
#include "abexp.h"

NB_Error
data_message_confirm_query_init(data_util_state* pds, data_message_confirm_query* pmcq)
{
	NB_Error err = NE_OK;

	DATA_VEC_ALLOC(err, pmcq->vec_message_confirm, data_message_confirm);
	DATA_VEC_ALLOC(err, pmcq->vec_retrieve_confirmed, data_retrieve_confirmed);

	return err;
}

void		
data_message_confirm_query_free(data_util_state* pds, data_message_confirm_query* pmcq)
{
	DATA_VEC_FREE(pds, pmcq->vec_message_confirm, data_message_confirm);
	DATA_VEC_FREE(pds, pmcq->vec_retrieve_confirmed, data_retrieve_confirmed);
}

NB_Error
data_message_confirm_query_add_message_confirm(data_util_state* pds, data_message_confirm_query* pmcq, data_message_confirm* pmc)
{
	return CSL_VectorAppend(pmcq->vec_message_confirm, pmc) ? NE_OK : NE_NOMEM;
}

NB_Error
data_message_confirm_query_add_retrieve_confirmed(data_util_state* pds, data_message_confirm_query* pmcq, data_retrieve_confirmed* prc)
{
	return CSL_VectorAppend(pmcq->vec_retrieve_confirmed, prc) ? NE_OK : NE_NOMEM;
}

tpselt		
data_message_confirm_query_to_tps(data_util_state* pds, data_message_confirm_query* pmcq)
{
	tpselt te;
	tpselt ce = NULL;
	int i, len;
	data_message_confirm* message_confirm;
	data_retrieve_confirmed* retrieveConfirmed;

	te = te_new("message-confirm-query");
	if (te == NULL) {
		goto errexit;
	}

	len = CSL_VectorGetLength(pmcq->vec_message_confirm);
	for (i = 0; i < len; i++) {
		message_confirm = (data_message_confirm*)CSL_VectorGetPointer(pmcq->vec_message_confirm, i);		
		if ((ce = data_message_confirm_to_tps(pds, message_confirm)) != NULL && te_attach(te, ce)) {
			ce = NULL;
		} else {
			goto errexit;
		}
	}

	len = CSL_VectorGetLength(pmcq->vec_retrieve_confirmed);
	for (i = 0; i < len; i++) {
		retrieveConfirmed = (data_retrieve_confirmed*)CSL_VectorGetPointer(pmcq->vec_retrieve_confirmed, i);		
		if ((ce = data_retrieve_confirmed_to_tps(pds, retrieveConfirmed)) != NULL && te_attach(te, ce)) {
			ce = NULL;
		} else {
			goto errexit;
		}
	}

	return te;

errexit:
	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean		
data_message_confirm_query_equal(data_util_state* pds, data_message_confirm_query* pmcq1, data_message_confirm_query* pmcq2)
{
	int ret = TRUE;

	DATA_VEC_EQUAL(pds, ret, pmcq1->vec_message_confirm, pmcq2->vec_message_confirm, data_message_confirm);
	DATA_VEC_EQUAL(pds, ret, pmcq1->vec_retrieve_confirmed, pmcq2->vec_retrieve_confirmed, data_retrieve_confirmed);

	return (boolean) ret;
}

NB_Error	
data_message_confirm_query_copy(data_util_state* pds, data_message_confirm_query* pmcq_dest, data_message_confirm_query* pmcq_src)
{
	NB_Error err = NE_OK;

	data_message_confirm_query_free(pds, pmcq_dest);
	err = err ? err : data_message_confirm_query_init(pds, pmcq_dest);

	DATA_VEC_COPY(pds, err, pmcq_dest->vec_message_confirm, pmcq_src->vec_message_confirm, data_message_confirm);
	DATA_VEC_COPY(pds, err, pmcq_dest->vec_retrieve_confirmed, pmcq_src->vec_retrieve_confirmed, data_retrieve_confirmed);
	
	return err;
}
