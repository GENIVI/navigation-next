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

    @file     data_send_message_reply.c
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

#include "data_send_message_reply.h"
#include "vec.h"

NB_Error
data_send_message_reply_init(data_util_state* pds, data_send_message_reply* psmr)
{
    if (!psmr)
    {
        return NE_INVAL;
    }
    
    psmr->vec_send_msg_result = CSL_VectorAlloc(sizeof(data_send_msg_result));

    if (!(psmr->vec_send_msg_result))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

void
data_send_message_reply_free(data_util_state* pds, data_send_message_reply* psmr)
{
    if (psmr->vec_send_msg_result)
    {
        int n = 0;
        int l = CSL_VectorGetLength(psmr->vec_send_msg_result);

        for (n = 0; n < l; n++)
        {
            data_send_msg_result_free(pds, (data_send_msg_result*)CSL_VectorGetPointer(psmr->vec_send_msg_result, n));
        }

        CSL_VectorDealloc(psmr->vec_send_msg_result);
    }

    psmr->vec_send_msg_result = 0;
}

NB_Error
data_send_message_reply_from_tps(data_util_state* pds, data_send_message_reply* psmr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce = 0;
    int iter = 0;
    data_send_msg_result result;

    if (te == NULL)
    {
        err = NE_INVAL;
    }        

    err = err ? err :data_send_msg_result_init(pds, &result);

    if (err)
    {
        goto errexit;
    }

    data_send_message_reply_free(pds, psmr);

    err = data_send_message_reply_init(pds, psmr);

    if (err)
    {
        goto errexit;
    }

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "send-msg-result") == 0)
        {
            err = data_send_msg_result_from_tps(pds, &result, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(psmr->vec_send_msg_result, &result))
                {
                    err = NE_NOMEM;
                }

                if (err == NE_OK)
                {
                    // clear out the result since we have copied it (shallow copy)
                    nsl_memset(&result, 0, sizeof(result));
                }
                else
                {
                    // free the result if it was not copied
                    data_send_msg_result_free(pds, &result);
                }
            }
            else
            {
                goto errexit;
            }
        }
    }

errexit:
    data_send_msg_result_free(pds, &result);
    if (err != NE_OK)
    {
        data_send_message_reply_free(pds, psmr);
    }
    return err;
}

int
data_send_message_reply_num_results(data_util_state* pds, data_send_message_reply* psmr)
{
	return CSL_VectorGetLength(psmr->vec_send_msg_result);
}

data_send_msg_result*
data_send_message_reply_get_result(data_util_state* pds, data_send_message_reply* psmr, int n)
{
    data_send_msg_result* pResult = 0;
    
    int l = CSL_VectorGetLength(psmr->vec_send_msg_result);
    if (n >= 0 && n < l)
    {
        pResult = CSL_VectorGetPointer(psmr->vec_send_msg_result, n);
    }

    return pResult;
}


/*! @} */
