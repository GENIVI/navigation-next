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

    @file     data_pronun_list.c
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems, Inc is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_pronun_list.h"


NB_Error
data_pronun_list_init(data_util_state* state, data_pronun_list* ppl)
{
    NB_Error err = NE_OK;

    DATA_VEC_ALLOC(err, ppl->vec_pronun_extended, data_pronun_extended);
    DATA_INIT(state, err, &ppl->phonetics_format, data_string);

    return NE_OK;
}

void
data_pronun_list_free(data_util_state* state, data_pronun_list* ppl)
{
    DATA_VEC_FREE(state, ppl->vec_pronun_extended, data_pronun_extended);
    DATA_FREE(state, &ppl->phonetics_format, data_string);
}

NB_Error
data_pronun_list_from_tps(data_util_state* state, data_pronun_list* ppl, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    int iteration = 0;
    tpselt childElement = NULL;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }
    DATA_REINIT(state, err, ppl, data_pronun_list);

    while (err == NE_OK && (childElement = te_nextchild(tpsElement, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(childElement), "pronun-extended") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, ppl->vec_pronun_extended, data_pronun_extended);
        }
    }
    err = err ? err : data_string_from_tps_attr(state, &ppl->phonetics_format, tpsElement, "phonetics-format");

errexit:
    if (err != NE_OK)
    {
        DATA_FREE(state, ppl, data_pronun_list);
    }
    return err;
}

boolean
data_pronun_list_equal(data_util_state* state, data_pronun_list* ppl1, data_pronun_list* ppl2)
{
    int ret = TRUE;

    ret = (int) data_string_equal(state, &ppl1->phonetics_format, &ppl2->phonetics_format);
    DATA_VEC_EQUAL(state, ret, ppl1->vec_pronun_extended, ppl2->vec_pronun_extended, data_pronun_extended);

    return (boolean) ret;
}

NB_Error
data_pronun_list_copy(data_util_state* state, data_pronun_list* destinationppl, data_pronun_list* sourceppl)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationppl, data_pronun_list);
    DATA_VEC_COPY(state, err, destinationppl->vec_pronun_extended, sourceppl->vec_pronun_extended, data_pronun_extended);
    DATA_COPY(state, err, &destinationppl->phonetics_format, &sourceppl->phonetics_format, data_string);

    return err;
}

/*! @} */
