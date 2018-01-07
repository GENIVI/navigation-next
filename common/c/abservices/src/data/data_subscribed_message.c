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

    @file        data_subscribed_message.c
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

#include "data_subscribed_message.h"

NB_Error
data_subscribed_message_init(data_util_state* pds, data_subscribed_message* psm)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_text_init(pds, &psm->formatted_text);
    err = err ? err : data_purchase_option_init(pds, &psm->purchase_option);
    err = err ? err : data_maptile_access_token_init(pds, &psm->maptile_access_token);

    err = err ? err : data_string_init(pds, &psm->language);
    psm->timestamp = 0;

    return err;
}

void
data_subscribed_message_free(data_util_state* pds, data_subscribed_message* psm)
{
    data_formatted_text_free(pds, &psm->formatted_text);
    data_purchase_option_free(pds, &psm->purchase_option);
    data_maptile_access_token_free(pds, &psm->maptile_access_token);
    data_string_free(pds, &psm->language);
}

NB_Error
data_subscribed_message_from_tps(data_util_state* pds, data_subscribed_message* psm, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;

    data_subscribed_message_free(pds, psm);

    err = data_subscribed_message_init(pds, psm);

    if ((ce = te_getchild (te, "formatted-text")) != NULL)    
    {
        err = err ? err : data_formatted_text_from_tps(pds, &psm->formatted_text, ce);
    }

    if ((ce = te_getchild(te, "purchase-option")) != NULL)
    {
        err = err ? err : data_purchase_option_from_tps(pds, &psm->purchase_option, ce);
    }

    if ((ce = te_getchild(te, "maptile-access-token")) != NULL)
    {
        err = err ? err : data_maptile_access_token_from_tps(pds, &psm->maptile_access_token, ce);
    }

    err = err ? err : data_string_from_tps_attr(pds, &psm->language, te, "language");

    psm->timestamp = te_getattru (te, "ts");

    return err;
}

NB_Error    
data_subscribed_message_copy(data_util_state* pds, data_subscribed_message* psm_dst, data_subscribed_message* psm_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_text_copy(pds, &psm_dst->formatted_text, &psm_src->formatted_text);    
    err = err ? err : data_purchase_option_copy(pds, &psm_dst->purchase_option, &psm_src->purchase_option);
    err = err ? err : data_maptile_access_token_copy(pds, &psm_dst->maptile_access_token, &psm_src->maptile_access_token);

    err = err ? err : data_string_copy(pds, &psm_dst->language, &psm_src->language);

    psm_dst->timestamp = psm_src->timestamp;
    return err;
}


/*! @} */
