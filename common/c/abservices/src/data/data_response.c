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

    @file     data_response.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized emproyees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_response.h"

NB_Error data_response_init(data_util_state* pds, data_response* pr)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, &pr->license_response, data_license_response);
    DATA_INIT(pds, err, &pr->requestid, data_string);
    return err;
}

void data_response_free(data_util_state* pds, data_response* pr)
{
    DATA_FREE(pds, &pr->requestid, data_string);
    DATA_FREE(pds, &pr->license_response, data_license_response);
}

NB_Error    data_response_from_tps(data_util_state* pds, data_response* pr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;
    int iter;

    DATA_REINIT(pds, err, pr, data_response);

    if (err != NE_OK)
        goto errexit;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if(nsl_strcmp(te_getname(ce), "license-response") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pr->license_response, data_license_response);
        }

        if (err != NE_OK) {

            goto errexit;
        }
    }

    if (!err) {
        err = data_string_from_tps_attr(pds, &pr->requestid, te, "requestid");
    }

errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_response_from_tps failed (license_response: %d)", err));
        DATA_FREE(pds, pr, data_response);
    }

    return err;
}


NB_Error data_response_copy(data_util_state* pds, data_response* pr_dest, data_response* pr_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pr_dest, data_response);

    DATA_COPY(pds, err, &pr_dest->license_response, &pr_src->license_response, data_license_response);
    DATA_COPY(pds, err, &pr_dest->requestid, &pr_src->requestid, data_string);

    return err;
}

/*! @} */

