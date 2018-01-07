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

    @file     data_golden_cookie.c
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

#include "data_golden_cookie.h"

NB_DEF NB_Error
data_golden_cookie_init(data_util_state* pds, data_golden_cookie* pgc)
{
    NB_Error err = NE_OK;

    err = data_string_init(pds, &pgc->provider_id);
    err = err ? err : data_blob_init(pds, &pgc->state);

    return err;
}

NB_DEF void
data_golden_cookie_free(data_util_state* pds, data_golden_cookie* pgc)
{
    data_string_free(pds, &pgc->provider_id);
    data_blob_free(pds, &pgc->state);
}

NB_DEF NB_Error
data_golden_cookie_from_tps(data_util_state* pds, data_golden_cookie* pgc, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }

    data_golden_cookie_free(pds, pgc);

    err = data_golden_cookie_init(pds, pgc);

    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(pds, &pgc->provider_id, te, "provider-id");
    err = err ? err : data_blob_from_tps_attr(pds, &pgc->state, te, "state");

    if (err != NE_OK)
    {
        data_golden_cookie_free(pds, pgc);
    }

    return err;
}

NB_DEF boolean
data_golden_cookie_equal(data_util_state* pds, data_golden_cookie* pgc1, data_golden_cookie* pgc2)
{
    return (boolean) (data_string_equal(pds, &pgc1->provider_id, &pgc2->provider_id) &&
                        data_blob_equal(pds, &pgc1->state, &pgc2->state));
}

NB_DEF NB_Error
data_golden_cookie_copy(data_util_state* pds, data_golden_cookie* pgc_dest, data_golden_cookie* pgc_src)
{
    NB_Error err = NE_OK;

    data_golden_cookie_free(pds, pgc_dest);

    err = data_golden_cookie_init(pds, pgc_dest);
    err = err ? err : data_string_copy(pds, &pgc_dest->provider_id, &pgc_src->provider_id);
    err = err ? err : data_blob_copy(pds, &pgc_dest->state, &pgc_src->state);

    return err;
}

NB_DEF tpselt
data_golden_cookie_to_tps(data_util_state* pds, data_golden_cookie* pgc)
{
    tpselt te;

    te = te_new("golden-cookie");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "provider-id", data_string_get(pds, &pgc->provider_id)))
    {
        goto errexit;
    }

    if (!te_setattr(te, "state", (const char*) pgc->state.data, pgc->state.size))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_DEF uint32
data_golden_cookie_get_tps_size(data_util_state* pds, data_golden_cookie* pgc)
{
    uint32 size = 0;

    if (pgc->provider_id) {
        size += nsl_strlen(pgc->provider_id);
    }

    size += pgc->state.size;

    return size;
}

NB_DEF void
data_golden_cookie_to_buf(data_util_state* pds,
            data_golden_cookie* pgc,
            struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pgc->provider_id, pdb);
    data_blob_to_buf(pds, &pgc->state, pdb);
}

NB_DEF NB_Error
data_golden_cookie_from_binary(data_util_state* pds,
            data_golden_cookie* pgc,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pgc->provider_id,
                                              pdata, pdatalen);

    err = err ? err : data_blob_from_binary(pds, &pgc->state,
                                            pdata, pdatalen);

    return err;
}

/*! @} */

