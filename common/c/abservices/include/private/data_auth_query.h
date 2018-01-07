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

    @file     data_auth_query.h
    @defgroup authentication

    This API allows a client to create a query with TPS for authentication.

*/
/*
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

#ifndef DATAAUTHQUERY_H
#define DATAAUTHQUERY_H

#include "datautil.h"
#include "data_string.h"
#include "data_message_confirm.h"
#include "data_bind_license.h"
#include "data_add_feature.h"
#include "data_remove_feature.h"
#include "data_save_optin_response.h"
#include "data_want_license_message.h"

typedef enum data_auth_request_
{
    dar_bind_license = 0,
    dar_add_feature = 1,
    dar_list_feature = 2,
    dar_remove_feature = 3,
    dar_save_optin_response = 4
} data_auth_request;

typedef struct data_auth_query_
{
    // Child Elements
    data_auth_request           request;
    data_bind_license           bindLicense;
    data_add_feature            addFeature;
    data_remove_feature         removeFeature;
    data_save_optin_response    optinResponse;
    data_want_license_message   want_license_msg;

    // Attributes
    data_string                 language;
    boolean                     license_msg_valid;
    boolean                     purchase_msg;
    uint32                      purchase_msg_ts;
    boolean                     region_info;             //if true, attaches the want-region element to the query
    data_string                 transactionid;
    boolean                     want_marketing_msg;      // if true, attaches the want-marketing element to want-purchase element.
	boolean                     want_extended_marketing_msg; // if true, attaches the want-extended-marketing element to want-purchase element.
    boolean                     want_pin_msg;
    boolean                     want_promo_options;      //if true attaches the want-promo-options element to query, used to request dynamic promo code offerings
    boolean                     want_product_description; // if true, attaches the want-product-description element to want-purchase element.

} data_auth_query;

NB_Error    data_auth_query_init(data_util_state* pds, data_auth_query* paq);
void        data_auth_query_free(data_util_state* pds, data_auth_query* paq);
tpselt      data_auth_query_to_tps(data_util_state* pds, data_auth_query* paq);
tpselt      data_auth_query_listfeature_to_tps(data_util_state* pds, data_auth_query* paq);
tpselt      data_auth_query_want_purchase_to_tps(data_util_state* pds, data_auth_query* paq);
tpselt      data_auth_query_request_to_tps(data_util_state* pds, data_auth_query* paq);
NB_Error    data_auth_query_copy(data_util_state* pds, data_auth_query* paq_dest, data_auth_query* paq_src);

/*! @} */

#endif //DATAAUTHQUERY_H
