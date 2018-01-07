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

    @file     data_license.c
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

#include "data_license.h"

NB_Error data_license_init(data_util_state* pds, data_license* pl)
{
    NB_Error err = NE_OK;
    DATA_VEC_ALLOC(err, pl->vec_bundle, data_bundle);
    DATA_INIT(pds, err, &pl->error, data_error_msg);
    DATA_INIT(pds, err, &pl->key, data_string);
    DATA_INIT(pds, err, &pl->productid, data_string);
    DATA_INIT(pds, err, &pl->licrequestid, data_string);
    return err;
}

void data_license_free(data_util_state* pds, data_license* pl)
{
    DATA_FREE(pds, &pl->licrequestid, data_string);
    DATA_FREE(pds, &pl->productid, data_string);
    DATA_FREE(pds, &pl->key, data_string);
    DATA_FREE(pds, &pl->error, data_error_msg);
    DATA_VEC_FREE(pds, pl->vec_bundle, data_bundle);
}

tpselt data_license_to_tps(data_util_state* pds, data_license* pl)
{
    tpselt te = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "license");

    DATA_VEC_TO_TPS(pds, errexit,te, pl->vec_bundle, data_bundle);

    if (!te_setattrc(te, "key", data_string_get(pds, &pl->key)))
    {
        goto errexit;
    }
    if (!te_setattrc(te, "productid", data_string_get(pds, &pl->productid)))
    {
        goto errexit;
    }
    if (!te_setattrc(te, "licrequestid", data_string_get(pds, &pl->licrequestid)))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

NB_Error    data_license_from_tps(data_util_state* pds, data_license* pl, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;
    int iter;

    DATA_REINIT(pds, err, pl, data_license);

    if (err != NE_OK)
        goto errexit;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "bundle") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pl->vec_bundle, data_bundle);
        }
        else if(nsl_strcmp(te_getname(ce), "error") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pl->error, data_error_msg);
        }

        if (err != NE_OK) {

            goto errexit;
        }
    }

    if (!err) {

        /* optional */ data_string_from_tps_attr(pds, &pl->key, te, "key");
        /* optional */ data_string_from_tps_attr(pds, &pl->productid, te, "productid");
        err = data_string_from_tps_attr(pds, &pl->licrequestid, te, "licrequestid");
    }

errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_license_from_tps failed (error_code: %d)", err));
        DATA_FREE(pds, pl, data_license);
    }

    return err;
}


NB_Error data_license_copy(data_util_state* pds, data_license* pl_dest, data_license* pl_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pl_dest, data_license);

    DATA_VEC_COPY(pds, err, pl_dest->vec_bundle, pl_src->vec_bundle, data_bundle);
    DATA_COPY(pds, err, &pl_dest->error, &pl_src->error, data_error_msg);
    DATA_COPY(pds, err, &pl_dest->key, &pl_src->key, data_string);
    DATA_COPY(pds, err, &pl_dest->productid, &pl_src->productid, data_string);
    DATA_COPY(pds, err, &pl_dest->licrequestid, &pl_src->licrequestid, data_string);

    return err;
}

/*! @} */

