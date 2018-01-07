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

 @file     data_url_args_template_characteristics.c
 */
/*
 (C) Copyright 2011 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_url_args_template_characteristics.h"
#include "data_pair.h"

NB_Error
data_url_args_template_characteristics_init(data_util_state* state, data_url_args_template_characteristics* urlArgsTemplateCharacteristics)
{
    urlArgsTemplateCharacteristics->pairs = CSL_VectorAlloc(sizeof(data_pair));
    if (urlArgsTemplateCharacteristics->pairs == NULL)
    {
        return NE_NOMEM;
    }
    return NE_OK;
}

void
data_url_args_template_characteristics_free(data_util_state* state, data_url_args_template_characteristics* urlArgsTemplateCharacteristics)
{
    int length = 0;
    int n = 0;
    if (urlArgsTemplateCharacteristics->pairs != NULL)
    {
        length = CSL_VectorGetLength(urlArgsTemplateCharacteristics->pairs);

        for (n = 0; n < length; ++n)
        {
            data_pair_free(state, (data_pair*) CSL_VectorGetPointer(urlArgsTemplateCharacteristics->pairs, n));
        }
        CSL_VectorDealloc(urlArgsTemplateCharacteristics->pairs);

    }
}

NB_Error
data_url_args_template_characteristics_from_tps(data_util_state* state, data_url_args_template_characteristics* urlArgsTemplateCharacteristics, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_pair pair= {0};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&pair, 0, sizeof(pair));
    DATA_REINIT(state, err, urlArgsTemplateCharacteristics, data_url_args_template_characteristics);

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            err = data_pair_from_tps(state, &pair, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(urlArgsTemplateCharacteristics->pairs, &pair))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&pair, 0, sizeof(data_pair));
            }
            else
            {
                data_pair_free(state, &pair);
            }
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (err != NE_OK)
    {
        data_url_args_template_characteristics_free(state, urlArgsTemplateCharacteristics);
    }

    return err;
}

boolean
data_url_args_template_characteristics_equal(data_util_state* state,
                                             data_url_args_template_characteristics* urlArgsTemplateCharacteristics1,
                                             data_url_args_template_characteristics* urlArgsTemplateCharacteristics2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(state, ret, urlArgsTemplateCharacteristics1->pairs, urlArgsTemplateCharacteristics2->pairs, data_pair);

    return (boolean) ret;
}

NB_Error
data_url_args_template_characteristics_copy(data_util_state* state,
                                            data_url_args_template_characteristics* destinationUrlArgsTemplateCharacteristics,
                                            data_url_args_template_characteristics* sourceUrlArgsTemplateCharacteristics)
{
    NB_Error err = NE_OK;

    DATA_VEC_COPY(state, err, destinationUrlArgsTemplateCharacteristics->pairs, sourceUrlArgsTemplateCharacteristics->pairs, data_pair);

    return err;
}



/*! @} */
