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

    @file     data_auth_query.c
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

#include "data_auth_query.h"
#include "data_message.h"
#include "data_feature.h"

NB_Error
data_auth_query_init(data_util_state* pds, data_auth_query* paq)
{
    NB_Error err = NE_OK;

    paq->request = 0;

    err = err ? err : data_save_optin_response_init(pds, &paq->optinResponse);
    err = err ? err : data_bind_license_init(pds, &paq->bindLicense);
    err = err ? err : data_add_feature_init(pds, &paq->addFeature);
    err = err ? err : data_remove_feature_init(pds, &paq->removeFeature);
    err = err ? err : data_string_init(pds, &paq->language);
    err = err ? err : data_string_init(pds, &paq->transactionid);

    paq->want_promo_options = FALSE;
    paq->want_product_description = FALSE;

    return err;
}

void
data_auth_query_free(data_util_state* pds, data_auth_query* paq)
{
    data_save_optin_response_free(pds, &paq->optinResponse);
    data_bind_license_free(pds, &paq->bindLicense);
    data_add_feature_free(pds, &paq->addFeature);
    data_remove_feature_free(pds, &paq->removeFeature);
    data_string_free(pds, &paq->language);
    data_string_free(pds, &paq->transactionid);
}

tpselt
data_auth_query_to_tps(data_util_state* pds, data_auth_query* paq)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("auth-query");

    if (te == NULL)
    {
        goto errexit;
    }
    if (paq->want_pin_msg)
    {
        if ((ce = te_new("want-pin")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (!te_setattrc(te, "language", data_string_get(pds, &paq->language)))
    {
        goto errexit;
    }

    if(paq->request == dar_bind_license)
    {
        if (!te_setattrc(te, "transactionid", data_string_get(pds, &paq->transactionid)))
        {
            goto errexit;
        }
    }

    if ((ce = data_auth_query_listfeature_to_tps(pds, paq)) != NULL && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if(paq->request == dar_bind_license)
    {
        if ((ce = data_bind_license_to_tps(pds, &paq->bindLicense)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if(paq->request == dar_add_feature)
    {
        if ((ce = data_add_feature_to_tps(pds, &paq->addFeature)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if(paq->request == dar_remove_feature)
    {
        if ((ce = data_remove_feature_to_tps(pds, &paq->removeFeature)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if(paq->request == dar_save_optin_response)
    {
        if ((ce = data_save_optin_response_to_tps(pds, &paq->optinResponse)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (paq->license_msg_valid)
    {
        if ((ce = data_want_license_message_to_tps(pds, &paq->want_license_msg)) != NULL && te_attach(te, ce))		
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    //attach the want purchase message element
    if (paq->purchase_msg)
    {
        if ((ce = data_auth_query_want_purchase_to_tps(pds, paq)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
    }
    
    if (paq->want_promo_options)
    {
        if ((ce = te_new("want-promo-options")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

tpselt
data_auth_query_listfeature_to_tps(data_util_state* pds, data_auth_query* paq)
{
    tpselt te = NULL;
    tpselt re = NULL;

    te = te_new("list-features");

    if (te == NULL)
    {
        goto errexit;
    }

    //attach want-region element - this is part of V6 auth query to request region attribute in reply.
    if (paq->region_info)
    {
        re = te_new("want-region-info");

        if (re == NULL)
        {
            goto errexit;
        }

        if (!te_attach(te, re))
        {
            goto errexit;
        }
        else
        {
            re = NULL;
        }
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(re);
    return NULL;
}

tpselt
data_auth_query_want_purchase_to_tps(data_util_state* pds, data_auth_query* paq)
{
    tpselt te = NULL;
    tpselt re = NULL;
	tpselt pe = NULL;

    te = te_new("want-purchase-message");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattru(te, "ts", paq->purchase_msg_ts))
    {
        goto errexit;
    }

    if (paq->want_marketing_msg)
    {
        re = te_new("want-marketing-message");

        if (re == NULL)
        {
            goto errexit;
        }

        if (!te_attach(te, re))
        {
            goto errexit;
        }
        else
        {
            re = NULL;
        }
    }
	if (paq->want_extended_marketing_msg)
    {
        pe = te_new("want-extended-marketing-message");

        if (pe == NULL)
        {
            goto errexit;
        }

        if (!te_attach(te, pe))
        {
            goto errexit;
        }
        else
        {
            pe = NULL;
        }
    }
    if (paq->want_product_description)
    {
        pe = te_new("want-product-description");

        if (pe == NULL)
        {
            goto errexit;
        }

        if (!te_attach(te, pe))
        {
            goto errexit;
        }
        else
        {
            pe = NULL;
        }
    }
    return te;
errexit:

    te_dealloc(te);
    te_dealloc(re);
	te_dealloc(pe);
    return NULL;
}

NB_Error
data_auth_query_copy(data_util_state* pds, data_auth_query* paq_dest, data_auth_query* paq_src)
{
    NB_Error err = NE_OK;

    data_auth_query_free(pds, paq_dest);

    err = err ? err : data_auth_query_init(pds, paq_dest);

    err = err ? err : data_bind_license_copy(pds, &paq_dest->bindLicense, &paq_src->bindLicense);
    err = err ? err : data_add_feature_copy(pds, &paq_dest->addFeature, &paq_src->addFeature);
    err = err ? err : data_remove_feature_copy(pds, &paq_dest->removeFeature, &paq_src->removeFeature);
    err = err ? err : data_save_optin_response_copy(pds, &paq_dest->optinResponse, &paq_src->optinResponse);
    paq_dest->request = paq_src->request;
    err = err ? err : data_want_license_message_copy (pds, &paq_dest->want_license_msg, &paq_src->want_license_msg);
    paq_dest->want_pin_msg = paq_src->want_pin_msg;

    paq_dest->want_promo_options = paq_src->want_promo_options;
    paq_dest->want_product_description = paq_src->want_product_description;

    return err;
}

/*! @} */
