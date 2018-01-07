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

/*!--------------------------------------------------------------------------

    @file     data_send_message_query.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_send_message_query.h"

static boolean attachPairElement(data_util_state* pds, tpselt te, data_pair* pair);

NB_Error
data_send_message_query_init(data_util_state* pds, data_send_message_query* psmq)
{
    NB_Error err = NE_OK;

    psmq->vec_recipient = CSL_VectorAlloc(sizeof(data_msg_recipient));

    if (!psmq->vec_recipient)
    {
        return NE_NOMEM;
    }

    err = err ? err : data_msg_content_init(pds, &psmq->content);
    err = err ? err : data_string_init(pds, &psmq->from);
    err = err ? err : data_string_init(pds, &psmq->from_name);
	err = err ? err : data_pair_init(pds, &psmq->setValue);

    return err;
}

void
data_send_message_query_free(data_util_state* pds, data_send_message_query* psmq)
{
    if (psmq->vec_recipient)
    {
        int n = 0;
        int l = CSL_VectorGetLength(psmq->vec_recipient);
 
        for (n = 0; n < l; n++)
        {
            data_msg_recipient_free(pds, (data_msg_recipient*)CSL_VectorGetPointer(psmq->vec_recipient, n));
        }

        CSL_VectorDealloc(psmq->vec_recipient);
    }

    psmq->vec_recipient = 0;

    data_msg_content_free(pds, &psmq->content);
    data_string_free(pds, &psmq->from);
    data_string_free(pds, &psmq->from_name);
	data_pair_free(pds, &psmq->setValue);
}

tpselt
data_send_message_query_to_tps(data_util_state* pds, data_send_message_query* psmq)
{
    tpselt te = 0;
	tpselt ce = 0;
    int n = 0;
	int len = 0;
	
    DATA_ALLOC_TPSELT(errexit, te, "send-message-query");

    if (psmq->vec_recipient)
    {
        len = CSL_VectorGetLength(psmq->vec_recipient);
        for (n = 0; n < len; n++) 
		{
            data_msg_recipient* p = CSL_VectorGetPointer(psmq->vec_recipient, n);
			
			ce = data_msg_recipient_to_tps(pds, p);
			if (!ce || !te_attach(te, ce))
			{
				goto errexit;
			}
			
			if (!attachPairElement(pds, ce, &psmq->setValue)) 
			{
				goto errexit;
			}
        }
    }	

    DATA_TO_TPS(pds, errexit, te, &psmq->content, data_msg_content);

    if (!te_setattrc(te, "from", data_string_get(pds, &psmq->from)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "from-name", data_string_get(pds, &psmq->from_name)))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    te_dealloc(ce);
    return 0;
}

NB_Error
data_send_message_query_add_recipient(data_util_state* pds, data_send_message_query* psmq, const char* to)
{
    NB_Error err = NE_OK;
    data_msg_recipient mr;

    err = data_msg_recipient_init(pds, &mr);
    err = err ? err : data_string_set(pds, &mr.to, to);

    if (err == NE_OK)
    {
        if (!CSL_VectorAppend(psmq->vec_recipient, &mr))
        {
            err = NE_NOMEM;
        }
    }

    return err;
}

static boolean
attachPairElement(data_util_state* pds, tpselt te, data_pair* pair)
{
	tpselt pairElement = NULL;
	
	if ((pairElement = data_pair_to_tps(pds, pair)) == NULL) {
		goto errexit;
	}
	
	if (!te_attach(te, pairElement)) {
		goto errexit;
	}
	
	return TRUE;
	
errexit:
	te_dealloc(pairElement);
	return FALSE;
}

/*! @} */
