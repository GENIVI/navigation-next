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

    @file     data_price_option.c
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

#include "data_price_option.h"

NB_Error data_price_option_init(data_util_state* pds, data_price_option* pdpo)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &pdpo->data);
    err = err ? err : data_string_init(pds, &pdpo->type);
    err = err ? err : data_recommended_option_init(pds, &pdpo->recommended_option);

    pdpo->endDate =0;
    pdpo->is_recommended_option = 0;
    pdpo->hasPromoCodeOffering = FALSE;

    return err;
}

void data_price_option_free(data_util_state* pds, data_price_option* pdpo)
{
    data_string_free(pds, &pdpo->data);
    data_string_free(pds, &pdpo->type);
    data_recommended_option_free(pds, &pdpo->recommended_option);
}

NB_Error
data_price_option_from_tps(data_util_state* pds, data_price_option* pdpo, tpselt ce)
{
    NB_Error err = NE_OK;
    tpselt te;

    data_price_option_free(pds, pdpo);

    err = data_price_option_init(pds, pdpo);

    if ((te = te_getchild(ce, "recommended-option")) != NULL)
    {
        err = err ? err : data_recommended_option_from_tps(pds, &pdpo->recommended_option, te);

        pdpo->is_recommended_option = TRUE;
    }
    
    if ((te = te_getchild(ce, "promo-offering")) != NULL)
    {
        pdpo->hasPromoCodeOffering = TRUE;
    }

    err = err ? err : data_string_from_tps_attr(pds, &pdpo->data, ce, "data");
    err = err ? err : data_string_from_tps_attr(pds, &pdpo->type, ce, "type");

    //if end date is not available, return 0 if attribute is not found.
    pdpo->endDate = te_getattru(ce, "enddate");

    return err;
}

NB_Error
data_price_option_copy(data_util_state* pds, data_price_option* pdpo_dest, data_price_option* pdpo_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(pds, &pdpo_dest->data, &pdpo_src->data);
    err = err ? err : data_string_copy(pds, &pdpo_dest->type, &pdpo_src->type);
    err = err ? err : data_recommended_option_copy(pds, &pdpo_dest->recommended_option, &pdpo_src->recommended_option);

    pdpo_dest->endDate = pdpo_src->endDate;
    pdpo_dest->is_recommended_option = pdpo_src->is_recommended_option;
    pdpo_dest->hasPromoCodeOffering = pdpo_src->hasPromoCodeOffering;

    return err;
}

/*! @} */

