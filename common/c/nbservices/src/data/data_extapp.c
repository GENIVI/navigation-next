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

 @file     data_extapp.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_extapp.h"


NB_Error
data_extapp_init(data_util_state* pds, data_extapp* pea)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pea->url, data_url);
    DATA_INIT(pds, err, &pea->url_args_template, data_url_args_template);
    DATA_INIT(pds, err, &pea->icon_location, data_icon_location);
    DATA_INIT(pds, err, &pea->name, data_string);

    if (err)
    {
        DATA_FREE(pds, pea, data_extapp);
    }

    return err;
}

void
data_extapp_free(data_util_state* pds, data_extapp* pea)
{
    DATA_FREE(pds, &pea->url, data_url);
    DATA_FREE(pds, &pea->url_args_template, data_url_args_template);
    DATA_FREE(pds, &pea->icon_location, data_icon_location);
    DATA_FREE(pds, &pea->name, data_string);
}

NB_Error
data_extapp_from_tps(data_util_state* pds, data_extapp* pea, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pea, data_extapp);

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "url") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pea->url, data_url);
        }
        else if (nsl_strcmp(te_getname(ce), "url-args-template") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pea->url_args_template, data_url_args_template);
        }
        else if (nsl_strcmp(te_getname(ce), "icon-location") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pea->icon_location, data_icon_location);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &pea->name, te, "name");

errexit:

    if (err != NE_OK)
    {
        DATA_FREE(pds, pea, data_extapp);
    }

    return err;
}

boolean
data_extapp_equal(data_util_state* pds, data_extapp* pea1, data_extapp* pea2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pea1->url, &pea2->url, data_url);
    DATA_EQUAL(pds, ret, &pea1->url_args_template, &pea2->url_args_template, data_url_args_template);
    DATA_EQUAL(pds, ret, &pea1->icon_location, &pea2->icon_location, data_icon_location);
    DATA_EQUAL(pds, ret, &pea1->name, &pea2->name, data_string);

    return (boolean)ret;
}

NB_Error
data_extapp_copy(data_util_state* pds, data_extapp* pea_dest, data_extapp* pea_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pea_dest, data_extapp);

    DATA_COPY(pds, err, &pea_dest->url, &pea_src->url, data_url);
    DATA_COPY(pds, err, &pea_dest->url_args_template, &pea_src->url_args_template, data_url_args_template);
    DATA_COPY(pds, err, &pea_dest->icon_location, &pea_src->icon_location, data_icon_location);
    DATA_COPY(pds, err, &pea_dest->name, &pea_src->name, data_string);

    return err;
}

/*! @} */