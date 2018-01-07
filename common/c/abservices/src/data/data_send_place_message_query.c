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
/* (C) Copyright 2006 by Networks In Motion, Inc.                */
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
 * data_send_place_message_query.c: created 2006/10/24 by Mark Goddard.
 */

#include "data_send_place_message_query.h"
#include "vec.h"
#include "abexp.h"

NB_Error
data_send_place_message_query_init(data_util_state* pds, data_send_place_message_query* spq)
{
	NB_Error err = NE_OK;

	spq->vec_recipient = CSL_VectorAlloc(sizeof(data_place_msg_recipient));

	if (spq->vec_recipient == NULL)
		return NE_NOMEM;

	err = err ? err : data_string_init(pds, &spq->from);
	err = err ? err : data_string_init(pds, &spq->from_name);
	err = err ? err : data_string_init(pds, &spq->message);
	err = err ? err : data_string_init(pds, &spq->signature);
	err = err ? err : data_place_init(pds, &spq->place);

	spq->want_id = TRUE;

	return err;
}

void		
data_send_place_message_query_free(data_util_state* pds, data_send_place_message_query* spq)
{
	int n,l;

	if (spq->vec_recipient) {
		l = CSL_VectorGetLength(spq->vec_recipient);

		for (n=0;n<l;n++)
			data_place_msg_recipient_free(pds, (data_place_msg_recipient*) CSL_VectorGetPointer(spq->vec_recipient, n));

		CSL_VectorDealloc(spq->vec_recipient);
	}

	spq->vec_recipient = NULL;

	data_string_free(pds, &spq->from);
	data_string_free(pds, &spq->from_name);
	data_string_free(pds, &spq->message);
	data_string_free(pds, &spq->signature);
	data_place_free(pds, &spq->place);
}

tpselt		
data_send_place_message_query_to_tps(data_util_state* pds, data_send_place_message_query* spq)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;

	te = te_new("send-place-message-query");

	if (te == NULL)
		goto errexit;

	if (spq->vec_recipient) {
		l = CSL_VectorGetLength(spq->vec_recipient);

		for (n=0;n<l;n++) {
			data_place_msg_recipient* pmr = CSL_VectorGetPointer(spq->vec_recipient, n);

			if ((ce = data_place_msg_recipient_to_tps(pds, pmr)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}	

	if ((ce = data_place_to_tps(pds, &spq->place)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (!te_setattrc(te, "from", data_string_get(pds, &spq->from)))
		goto errexit;

	if (!te_setattrc(te, "from-name", data_string_get(pds, &spq->from_name)))
		goto errexit;

	if (!te_setattrc(te, "message", data_string_get(pds, &spq->message)))
		goto errexit;

	if (!te_setattrc(te, "signature", data_string_get(pds, &spq->signature)))
		goto errexit;

	if (spq->want_id) {

		if ((ce = te_new("place-msg-want-id")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error	
data_send_place_message_query_add_recipient(data_util_state* pds, data_send_place_message_query* spq, const char* to)
{
    NB_Error err = NE_OK;
    data_place_msg_recipient pmr = { 0 };

    err = data_place_msg_recipient_init(pds, &pmr);
    err = err ? err : data_string_set(pds, &pmr.to, to);

    if (err == NE_OK) {
        if (!CSL_VectorAppend(spq->vec_recipient, &pmr)) {
            err = NE_NOMEM;
        }
    }

    return err;
}
