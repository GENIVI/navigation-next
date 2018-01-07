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

    @file     data_bundle.c
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

#include "data_bundle.h"

NB_Error data_bundle_init(data_util_state* pds, data_bundle* pb)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, &pb->name,data_string);
    DATA_INIT(pds, err, &pb->enddate,data_string);
    DATA_INIT(pds, err, &pb->type,data_string);
    DATA_INIT(pds, err, &pb->promo_code,data_promo_code);
    return err;
}

void data_bundle_free(data_util_state* pds, data_bundle* pb)
{
    DATA_FREE(pds, &pb->type, data_string);
    DATA_FREE(pds, &pb->enddate, data_string);
    DATA_FREE(pds, &pb->name, data_string);
    DATA_FREE(pds, &pb->promo_code, data_promo_code);
}

tpselt data_bundle_to_tps(data_util_state* pds, data_bundle* pb)
{
    tpselt te = NULL;

    te = te_new("bundle");
    if (!te)
    {
        goto errexit;
    }

    if (pb->promo_code.value)
    {
        DATA_TO_TPS(pds, errexit, te, &pb->promo_code, data_promo_code);
    }

    if (!te_setattrc(te, "name", data_string_get(pds, &pb->name)))
    {
        goto errexit;
    }
    if (!te_setattrc(te, "enddate", data_string_get(pds, &pb->enddate)))
    {
        goto errexit;
    }
    if (!te_setattrc(te, "type", data_string_get(pds, &pb->type)))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;

}

NB_Error data_bundle_from_tps(data_util_state* pds, data_bundle* pb, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;
    int iter;

    DATA_REINIT(pds, err, pb, data_bundle);

    if (err != NE_OK)
        goto errexit;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "promo-code") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pb->promo_code, data_promo_code);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    if (!err)
    {
        err = err ? err : data_string_from_tps_attr(pds, &pb->name, te, "name");
        err = err ? err : data_string_from_tps_attr(pds, &pb->enddate, te, "enddate");
        err = err ? err : data_string_from_tps_attr(pds, &pb->type, te, "type");
    }

    errexit:
    if (err != NE_OK) {
        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_bundle_from_tps failed (error_code: %d)", err));
        DATA_FREE(pds, pb, data_bundle);
    }

    return err;
}


NB_Error data_bundle_copy(data_util_state* pds, data_bundle* pb_dest, data_bundle* pb_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pb_dest, data_bundle);

    DATA_COPY(pds, err, &pb_dest->promo_code, &pb_src->promo_code, data_promo_code);
    DATA_COPY(pds, err, &pb_dest->name, &pb_src->name, data_string);
    DATA_COPY(pds, err, &pb_dest->enddate, &pb_src->enddate, data_string);
    DATA_COPY(pds, err, &pb_dest->type, &pb_src->type, data_string);

    return err;
}

/*! @} */

