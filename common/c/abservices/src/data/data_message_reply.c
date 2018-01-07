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
 * data_message_reply.c: created 2006/09/29 by Chetan Nagaraj.
 */

#include "data_message.h"
#include "data_message_reply.h"
#include "vec.h"
#include "abexp.h"

NB_Error
data_message_reply_init(data_util_state* pds, data_message_reply* pmr)
{
	NB_Error err = NE_OK;

	pmr->vec_messages = CSL_VectorAlloc(sizeof(data_message));

	if (pmr->vec_messages == NULL)
		return NE_NOMEM;

	return err;
}

void		
data_message_reply_free(data_util_state* pds, data_message_reply* pmr)
{
	int i, len;

	if (pmr->vec_messages) {

		len = CSL_VectorGetLength(pmr->vec_messages);

		for (i = 0; i < len; i++)
			data_message_free(pds, (data_message*)CSL_VectorGetPointer(pmr->vec_messages, i));

		CSL_VectorDealloc(pmr->vec_messages);
	}

	pmr->vec_messages = NULL;
}

NB_Error	
data_message_reply_from_tps(data_util_state* pds, data_message_reply* pmr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_message	message;
	
	err = data_message_init(pds, &message);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_message_reply_free(pds, pmr);

	err = data_message_reply_init(pds, pmr);

	if (err != NE_OK)
		return err;

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "message") == 0) {

			err = data_message_from_tps(pds, &message, ce);

			if (err == NE_OK) {

				if (!CSL_VectorAppend(pmr->vec_messages, &message))
					err = NE_NOMEM;

				if (err == NE_OK)
					data_message_init(pds, &message);
				else
					data_message_free(pds, &message);
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

	te_getattru64(te, "ts", &pmr->ts);
	
errexit:
	data_message_free(pds, &message);
	if (err != NE_OK)
		data_message_reply_free(pds, pmr);
	return err;
}

boolean		
data_message_reply_equal(data_util_state* pds, data_message_reply* pmr1, data_message_reply* pmr2)
{
	int ret;
	int i, len;

	ret = ((len = CSL_VectorGetLength(pmr1->vec_messages)) == CSL_VectorGetLength(pmr2->vec_messages));
	
	for (i = 0; i < len && ret; i++) {
		
		ret = ret && data_message_equal(pds, (data_message*)CSL_VectorGetPointer(pmr1->vec_messages, i), (data_message*)CSL_VectorGetPointer(pmr2->vec_messages, i));
	}

	return (boolean) ret;
}

NB_Error	
data_message_reply_copy(data_util_state* pds, data_message_reply* pmr_dest, data_message_reply* pmr_src)
{
	NB_Error err = NE_OK;
	int i, len;
	data_message	message;

	data_message_init(pds, &message);

	data_message_reply_free(pds, pmr_dest);
	err = err ? err : data_message_reply_init(pds, pmr_dest);
	
	len = CSL_VectorGetLength(pmr_src->vec_messages);

	for (i = 0; i < len && err == NE_OK; i++) {

		err = err ? err : data_message_copy(pds, &message, CSL_VectorGetPointer(pmr_src->vec_messages, i));
		err = err ? err : CSL_VectorAppend(pmr_dest->vec_messages, &message) ? NE_OK : NE_NOMEM;

		if (err)
			data_message_free(pds, &message);
		else
			data_message_init(pds, &message);
	}

	return err;
}
