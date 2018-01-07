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

    @file     data_price_list.c
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

#include "data_price_list.h"

NB_Error data_price_list_init(data_util_state* pds, data_price_list* pdpl)
{
    NB_Error err = NE_OK;

    pdpl->vec_price_option = CSL_VectorAlloc(sizeof(data_price_option));
    if(!pdpl->vec_price_option)
    {
        return NE_NOMEM;
    }

    return err;
}

void data_price_list_free(data_util_state* pds, data_price_list* pdpl)
{
    data_price_option* price_option;
    int len=0, i= 0;

    if (pdpl->vec_price_option)
    {
        len = CSL_VectorGetLength(pdpl->vec_price_option);

        for (i =0; i<len; i++)
        {
            price_option = (data_price_option*)CSL_VectorGetPointer(pdpl->vec_price_option, i);
            data_price_option_free(pds, price_option);
        }

        CSL_VectorDealloc(pdpl->vec_price_option);
        pdpl->vec_price_option = NULL;
    }
}

NB_Error 
data_price_list_from_tps(data_util_state* pds, data_price_list* pdpl, tpselt ce)
{
    NB_Error err = NE_OK;

    tpselt poe = NULL;
    int iter=0;
    data_price_option price;

    data_price_list_free(pds, pdpl);

    err = data_price_list_init(pds, pdpl);

    //price list can have one or more price options
    while ((poe = te_nextchild(ce, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(poe), "price-option") == 0)
        {
            err = err ? err : data_price_option_init(pds, &price);

            err = err ? err : data_price_option_from_tps(pds, &price, poe);
            if (err == NE_OK)
            {
                if(!CSL_VectorAppend(pdpl->vec_price_option, &price))
                {
                    err = NE_NOMEM;
                    goto errexit;
                }
            }
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_price_option_free(pds, &price);

        data_price_list_free(pds, pdpl);
    }
    return err;
}

NB_Error 
data_price_list_copy(data_util_state* pds, data_price_list* pdpl_dest, data_price_list* pdpl_src)
{
    NB_Error err = NE_OK;

    if (pdpl_src->vec_price_option)
    {
        DATA_VEC_COPY(pds, err, pdpl_dest->vec_price_option, pdpl_src->vec_price_option, data_price_option);
    }

    return err;
}

/*! @} */

