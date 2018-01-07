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

    @file     data_license_request.c
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

#include "data_license_request.h"

NB_Error data_license_request_init(data_util_state* pds, data_license_request* plr)
{
    NB_Error err = NE_OK;
    DATA_VEC_ALLOC(err, plr->vec_license, data_license);
    DATA_INIT(pds, err, &plr->transactionid, data_string);
    DATA_INIT(pds, err, &plr->vendorname, data_string);
    DATA_INIT(pds, err, &plr->country, data_string);
    DATA_INIT(pds, err, &plr->vendoruserid, data_string);
    DATA_INIT(pds, err, &plr->subscriberkey, data_string);
    DATA_INIT(pds, err, &plr->action, data_string);
    return err;
}

void data_license_request_free(data_util_state* pds, data_license_request* plr)
{
    DATA_FREE(pds, &plr->action, data_string);
    DATA_FREE(pds, &plr->subscriberkey, data_string);
    DATA_FREE(pds, &plr->vendoruserid, data_string);
    DATA_FREE(pds, &plr->country, data_string);
    DATA_FREE(pds, &plr->vendorname, data_string);
    DATA_FREE(pds, &plr->transactionid, data_string);
    DATA_VEC_FREE(pds, plr->vec_license, data_license);
}

tpselt data_license_request_to_tps(data_util_state* pds, data_license_request* plr)
{
    tpselt te = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "license-request");

    DATA_VEC_TO_TPS(pds, errexit,te, plr->vec_license, data_license);

    // spec states transactionid is optional but the server does not
    DATA_STR_SETATTR(pds, errexit, te, "transactionid", &plr->transactionid );

    DATA_STR_SETATTR(pds, errexit, te, "vendorname", &plr->vendorname);

    DATA_STR_SETATTR(pds, errexit, te, "country", &plr->country);

    DATA_STR_SETATTR(pds, errexit, te, "vendoruserid", &plr->vendoruserid);

    DATA_STR_SETATTR(pds, errexit, te, "subscriberkey", &plr->subscriberkey);

    DATA_STR_SETATTR(pds, errexit, te, "action", &plr->action);

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

NB_Error data_license_request_copy(data_util_state* pds, data_license_request* plr_dest, data_license_request* plr_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, plr_dest, data_license_request);

    DATA_VEC_COPY(pds, err, plr_dest->vec_license, plr_src->vec_license, data_license);
    DATA_COPY(pds, err, &plr_dest->transactionid, &plr_src->transactionid, data_string);
    DATA_COPY(pds, err, &plr_dest->vendorname, &plr_src->vendorname, data_string);
    DATA_COPY(pds, err, &plr_dest->country, &plr_src->country, data_string);
    DATA_COPY(pds, err, &plr_dest->vendoruserid, &plr_src->vendoruserid, data_string);
    DATA_COPY(pds, err, &plr_dest->subscriberkey, &plr_src->subscriberkey, data_string);
    DATA_COPY(pds, err, &plr_dest->action, &plr_src->action, data_string);

    return err;
}

/*! @} */

