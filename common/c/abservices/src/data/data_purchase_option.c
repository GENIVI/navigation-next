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

    @file     data_purchase_option.c
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

#include "data_purchase_option.h"

NB_Error data_purchase_option_init(data_util_state* pds, data_purchase_option* pdpo)
{
    NB_Error err = NE_OK;

    pdpo->vec_bundle_option = CSL_VectorAlloc(sizeof(data_bundle_option));
    if (!pdpo->vec_bundle_option)
    {
        return NE_NOMEM;
    }

    return err;
}

void data_purchase_option_free(data_util_state* pds, data_purchase_option* pdpo)
{
    data_bundle_option* bundle = NULL;
    int len =0, i=0;

    if (pdpo->vec_bundle_option)
    {
        len = CSL_VectorGetLength(pdpo->vec_bundle_option);

        for (i=0; i<len; i++)
        {
            bundle = (data_bundle_option*) CSL_VectorGetPointer(pdpo->vec_bundle_option, i);
            data_bundle_option_free(pds, bundle);
        }

        CSL_VectorDealloc(pdpo->vec_bundle_option);
        pdpo->vec_bundle_option = NULL;
    }
}

NB_Error
data_purchase_option_from_tps(data_util_state* pds, data_purchase_option* pdpo, tpselt ce)
{
    NB_Error err = NE_OK;

    tpselt te = NULL;
    int iter =0;
    data_bundle_option bundle;

    data_purchase_option_free(pds, pdpo);

    err = data_purchase_option_init(pds, pdpo);

    while ((te = te_nextchild(ce, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(te), "bundle-option") == 0)
        {
            err = err ? err : data_bundle_option_init(pds, &bundle);

            err = err ? err: data_bundle_option_from_tps(pds, &bundle, te);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pdpo->vec_bundle_option, &bundle))
                {
                    err = NE_NOMEM;
                    goto errexit;
                }
            }

            //data_bundle_option_free(pds, &bundle);
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_bundle_option_free(pds, &bundle);

        data_purchase_option_free(pds, pdpo);
    }

    return err;
}

NB_Error 
data_purchase_option_copy(data_util_state* pds, data_purchase_option* pdpo_dest, data_purchase_option* pdpo_src)
{
    NB_Error err = NE_OK;

    if (pdpo_src->vec_bundle_option)
    {
        DATA_VEC_COPY(pds, err, pdpo_dest->vec_bundle_option, pdpo_src->vec_bundle_option, data_bundle_option);
    }

    return err;
}

/*! @} */

