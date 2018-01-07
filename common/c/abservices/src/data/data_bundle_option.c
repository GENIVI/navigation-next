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

    @file     data_bundle_option.c
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

#include "data_bundle_option.h"

NB_Error data_bundle_option_init(data_util_state* pds, data_bundle_option* pdbo)
{
    NB_Error err = NE_OK;

    err = err ? err : data_price_list_init(pds, &pdbo->price_list);
    err = err ? err : data_feature_list_init(pds, &pdbo->feature_list);
    err = err ? err : data_formatted_text_init(pds, &pdbo->formatted_text);

    err = err ? err : data_string_init(pds, &pdbo->title);
    err = err ? err : data_string_init(pds, &pdbo->description);
    err = err ? err : data_string_init(pds, &pdbo->name);
    err = err ? err : data_string_init(pds, &pdbo->region);
    pdbo->token_id =0;

    return err;
}

void data_bundle_option_free(data_util_state* pds, data_bundle_option* pdbo)
{
    data_price_list_free(pds, &pdbo->price_list);
    data_feature_list_free(pds, &pdbo->feature_list);
    data_formatted_text_free(pds, &pdbo->formatted_text);

    data_string_free(pds, &pdbo->title);
    data_string_free(pds, &pdbo->description);
    data_string_free(pds, &pdbo->name);
    data_string_free(pds, &pdbo->region);
}

NB_Error 
data_bundle_option_from_tps(data_util_state* pds, data_bundle_option* pdbo, tpselt ce)
{
    NB_Error err = NE_OK;

    tpselt te;

    data_bundle_option_free(pds, pdbo);

    err = data_bundle_option_init(pds, pdbo);

    if ((te = te_getchild(ce, "price-list")) != NULL)
    {
        err = err ? err : data_price_list_from_tps(pds, &pdbo->price_list, te);
    }

    if ((te = te_getchild(ce, "feature-list")) != NULL)
    {
        err = err ? err : data_feature_list_from_tps(pds, &pdbo->feature_list, te);
    }

    if ((te = te_getchild(ce, "formatted-text")) != NULL)
    {
        err = err ? err : data_formatted_text_from_tps(pds, &pdbo->formatted_text, te);
    }

    err = err ? err : data_string_from_tps_attr(pds, &pdbo->title, ce, "title");
    err = err ? err : data_string_from_tps_attr(pds, &pdbo->description, ce, "description");
    err = err ? err : data_string_from_tps_attr(pds, &pdbo->name, ce, "name");
    err = err ? err : data_string_from_tps_attr(pds, &pdbo->region, ce, "region");
    pdbo->token_id = te_getattru(ce, "token");

    return err;
}

NB_Error 
data_bundle_option_copy(data_util_state* pds, data_bundle_option* pdbo_dest, data_bundle_option* pdbo_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_price_list_copy(pds, &pdbo_dest->price_list, &pdbo_src->price_list);
    err = err ? err : data_feature_list_copy(pds, &pdbo_dest->feature_list, &pdbo_src->feature_list);
    err = err ? err : data_formatted_text_copy(pds, &pdbo_dest->formatted_text, &pdbo_src->formatted_text);

    err = err ? err : data_string_copy(pds, &pdbo_dest->title, &pdbo_src->title);
    err = err ? err : data_string_copy(pds, &pdbo_dest->description, &pdbo_src->description);
    err = err ? err : data_string_copy(pds, &pdbo_dest->name, &pdbo_src->name);
    err = err ? err : data_string_copy(pds, &pdbo_dest->region, &pdbo_src->region);
    pdbo_dest->token_id = pdbo_src->token_id;

    return err;
}

/*! @} */

