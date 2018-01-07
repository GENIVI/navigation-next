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

    @file     data_add_feature.c
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

#include "data_save_optin_response.h"

NB_Error data_save_optin_response_init(data_util_state* pds, data_save_optin_response* psor)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &psor->message_type);
    err = err ? err : data_message_confirm_init(pds, &psor->message_confirm);

    return err;
}

void data_save_optin_response_free(data_util_state* pds, data_save_optin_response* psor)
{
    data_string_free(pds, &psor->message_type);
    data_message_confirm_free(pds, &psor->message_confirm);
}

tpselt data_save_optin_response_to_tps(data_util_state* pds, data_save_optin_response* psor)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("save-optin-response");
    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "optin-type", data_string_get(pds, &psor->message_type)))
    {
        goto errexit;	
    }

    ce = data_message_confirm_to_tps(pds, &psor->message_confirm);

    if(ce != NULL)
    {
        if(te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

errexit:
    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

NB_Error data_save_optin_response_copy(data_util_state* pds, data_save_optin_response* psor_dest, data_save_optin_response* psor_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(pds, &psor_dest->message_type, &psor_src->message_type);
    err = err ? err : data_message_confirm_copy(pds, &psor_dest->message_confirm, &psor_src->message_confirm);

    return err;
}

/*! @} */
