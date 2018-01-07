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
 * data_send_place_message_reply.c: created 2006/10/24 by Mark Goddard.
 */

#include "data_send_place_message_reply.h"
#include "vec.h"
#include "abexp.h"

NB_Error
data_send_place_message_reply_init(data_util_state* pds, data_send_place_message_reply* spr)
{
	NB_Error err = NE_OK;

	spr->vec_recipient_result = CSL_VectorAlloc(sizeof(data_place_msg_recipient_result));

	if (spr->vec_recipient_result == NULL)
		return NE_NOMEM;

	return err;
}

void		
data_send_place_message_reply_free(data_util_state* pds, data_send_place_message_reply* spr)
{
	int n,l;

	if (spr->vec_recipient_result) {
		l = CSL_VectorGetLength(spr->vec_recipient_result);

		for (n=0;n<l;n++)
			data_place_msg_recipient_result_free(pds, (data_place_msg_recipient_result*) CSL_VectorGetPointer(spr->vec_recipient_result, n));

		CSL_VectorDealloc(spr->vec_recipient_result);
	}

	spr->vec_recipient_result = NULL;
}

NB_Error	
data_send_place_message_reply_from_tps(data_util_state* pds, data_send_place_message_reply* spr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_place_msg_recipient_result	result;
	
	err = data_place_msg_recipient_result_init(pds, &result);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_send_place_message_reply_free(pds, spr);

	err = data_send_place_message_reply_init(pds, spr);

	if (err != NE_OK)
		return err;
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "place-msg-recipient-result") == 0) {

			err = data_place_msg_recipient_result_from_tps(pds, &result, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spr->vec_recipient_result, &result))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&result, 0, sizeof(result)); // clear out the result since we have copied it (shallow copy)
				else
					data_place_msg_recipient_result_free(pds, &result); // free the result if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}
	
errexit:
	data_place_msg_recipient_result_free(pds, &result);
	if (err != NE_OK)
		data_send_place_message_reply_free(pds, spr);
	return err;
}

int
data_send_place_message_reply_num_results(data_util_state* pds, data_send_place_message_reply* spr)
{
	return CSL_VectorGetLength(spr->vec_recipient_result);
}

data_place_msg_recipient_result*
data_send_place_message_reply_get_result(data_util_state* pds, data_send_place_message_reply* spr, int n)
{
	int l;
	data_place_msg_recipient_result* pResult = NULL;

	l = CSL_VectorGetLength(spr->vec_recipient_result);
	if (n >= 0 && n < l)
		pResult = CSL_VectorGetPointer(spr->vec_recipient_result, n);

	return pResult;
}

