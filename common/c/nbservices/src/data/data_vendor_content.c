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
/* (C) Copyright 2014 by Networks In Motion, Inc.                */
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

#include "data_pair.h"
#include "data_vendor_content.h"

static void data_vendor_content_clear(data_util_state* pds, data_vendor_content* dvc)
{
    dvc->has_overall_rating = FALSE;
}

NB_Error data_vendor_content_init(data_util_state* pds, data_vendor_content* dvc)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(dvc, data_vendor_content);
    DATA_VEC_ALLOC(err, dvc->vec_pairs, data_pair);
    DATA_INIT(pds, err, &dvc->overall_rating, data_overall_rating);
    DATA_INIT(pds, err, &dvc->name, data_string);

    data_vendor_content_clear(pds, dvc);

    if (err)
    {
        data_vendor_content_free(pds, dvc);
    }

    return err;
}

void data_vendor_content_free(data_util_state* pds, data_vendor_content* dvc)
{
    DATA_VEC_FREE(pds, dvc->vec_pairs, data_pair);
    DATA_FREE(pds, &dvc->overall_rating, data_overall_rating);
    DATA_FREE(pds, &dvc->name, data_string);

    data_vendor_content_clear(pds, dvc);
}

NB_Error data_vendor_content_from_tps(data_util_state* pds, data_vendor_content* dvc, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, dvc, data_vendor_content);

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, dvc->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "overall-rating") == 0)
        {
            dvc->has_overall_rating = TRUE;
            DATA_FROM_TPS(pds, err, ce, &dvc->overall_rating, data_overall_rating);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &dvc->name, te, "name");
    if (err != NE_OK)
    {
        goto errexit;
    }

errexit:
    if (err != NE_OK)
    {
        DATA_FREE(pds, dvc, data_vendor_content);
    }

    return err;
}


boolean data_vendor_content_equal(data_util_state* pds, data_vendor_content* dvc1, data_vendor_content* dvc2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, dvc1->vec_pairs, dvc2->vec_pairs, data_pair);
    DATA_EQUAL(pds, ret, &dvc1->overall_rating, &dvc2->overall_rating, data_overall_rating);
    DATA_EQUAL(pds, ret, &dvc1->name, &dvc2->name, data_string);

    return (boolean)ret;
}

NB_Error data_vendor_content_copy(data_util_state* pds, data_vendor_content* dvc_dest, data_vendor_content* dvc_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dvc_dest, data_vendor_content);

    DATA_VEC_COPY(pds, err, dvc_dest->vec_pairs, dvc_src->vec_pairs, data_pair);
    DATA_COPY(pds, err, &dvc_dest->overall_rating, &dvc_src->overall_rating, data_overall_rating);
    DATA_COPY(pds, err, &dvc_dest->name, &dvc_src->name, data_string);

    dvc_dest->has_overall_rating = dvc_src->has_overall_rating;

    return err;
}

