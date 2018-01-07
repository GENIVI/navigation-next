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

    @file		data_client_stored_message.c
	@created	7/14/2009
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

#include "data_client_stored_message.h"

NB_Error
data_client_stored_message_init(data_util_state* pds, data_client_stored_message* pcsm)
{
	NB_Error err = NE_OK;

	err = err ? err : data_formatted_text_init(pds, &pcsm->formatted_text);
    err = err ? err : data_purchase_option_init(pds, &pcsm->purchase_option);
    err = err ? err : data_product_description_init(pds, &pcsm->product_description);

	err = err ? err : data_string_init(pds, &pcsm->type);
	err = err ? err : data_string_init(pds, &pcsm->language);
	pcsm->timestamp = 0;
    //V6 added attributes initialization
    err = err ? err : data_string_init(pds, &pcsm->accept_text);
    err = err ? err : data_string_init(pds, &pcsm->decline_text);
    err = err ? err : data_string_init(pds, &pcsm->option_text);

	return err;
}

void
data_client_stored_message_free(data_util_state* pds, data_client_stored_message* pcsm)
{
	data_formatted_text_free(pds, &pcsm->formatted_text);
    data_purchase_option_free(pds, &pcsm->purchase_option);
    data_product_description_free(pds, &pcsm->product_description);

	data_string_free(pds, &pcsm->type);
	data_string_free(pds, &pcsm->language);
    //V6 free added attributes
    data_string_free(pds, &pcsm->accept_text);
    data_string_free(pds, &pcsm->decline_text);
    data_string_free(pds, &pcsm->option_text);
}

NB_Error
data_client_stored_message_from_tps(data_util_state* pds, data_client_stored_message* pcsm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;

	data_client_stored_message_free(pds, pcsm);

	err = data_client_stored_message_init(pds, pcsm);

	if (err != NE_OK)
	{
		return err;
	}

    //formatted-text element is now optional
	if ((ce = te_getchild (te, "formatted-text")) != NULL)
	{
        err = err ? err : data_formatted_text_from_tps(pds, &pcsm->formatted_text, ce);
	}

    if ((ce = te_getchild(te, "purchase-option")) != NULL)
    {
        err = err ? err : data_purchase_option_from_tps(pds, &pcsm->purchase_option, ce);
    }

    if ((ce = te_getchild(te, "product-description")) != NULL)
    {
        err = err ? err : data_product_description_from_tps(pds, &pcsm->product_description, ce);
    }

	err = err ? err : data_string_from_tps_attr(pds, &pcsm->type, te, "type");
	err = err ? err : data_string_from_tps_attr(pds, &pcsm->language, te, "language");

	pcsm->timestamp = te_getattru (te, "ts");	

    //V6 added attributes, optional attributes if not available is set to empty string
    err = err ? err : data_string_set(pds, &pcsm->accept_text, te_getattrc(te,"accept-text"));
    err = err ? err : data_string_set(pds, &pcsm->decline_text, te_getattrc(te, "decline-text"));
    err = err ? err : data_string_set(pds, &pcsm->option_text, te_getattrc(te, "option-text"));

	if (err != NE_OK)
	{
		data_client_stored_message_free(pds, pcsm);
	}

	return err;
}

NB_Error	
data_client_stored_message_copy(data_util_state* pds, data_client_stored_message* pcsm_dst, data_client_stored_message* pcsm_src)
{
	NB_Error err = NE_OK;

	err = err ? err : data_formatted_text_copy(pds, &pcsm_dst->formatted_text, &pcsm_src->formatted_text);	
    err = err ? err : data_purchase_option_copy(pds, &pcsm_dst->purchase_option, &pcsm_src->purchase_option);
    err = err ? err : data_product_description_copy(pds, &pcsm_dst->product_description, &pcsm_src->product_description);

	err = err ? err : data_string_copy(pds, &pcsm_dst->type, &pcsm_src->type);
	err = err ? err : data_string_copy(pds, &pcsm_dst->language, &pcsm_src->language);

	pcsm_dst->timestamp = pcsm_src->timestamp;

    //V6 added attributes, optional attributes set to empty string if not available
    err = err ? err : data_string_copy(pds, &pcsm_dst->accept_text, &pcsm_src->accept_text);
    err = err ? err : data_string_copy(pds, &pcsm_dst->decline_text, &pcsm_src->decline_text);
    err = err ? err : data_string_copy(pds, &pcsm_dst->option_text, &pcsm_src->option_text);

	return err;
}


/*! @} */
