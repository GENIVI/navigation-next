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

    @file     data_place_msg_content.c
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

#include "data_place_msg_content.h"

NB_Error
data_place_msg_content_init(data_util_state* pds, data_place_msg_content* ppmc)
{
    NB_Error err = NE_OK;
    
    err = err ? err : data_place_init(pds, &ppmc->place);
    err = err ? err : data_place_msg_banner_init(pds, &ppmc->place_msg_banner);
    ppmc->place_msg_want_id = TRUE;
    err = err ? err : data_message_text_init(pds, &ppmc->message_text);
    
    err = err ? err : data_string_init(pds, &ppmc->signature);
    
    return err;
}

void
data_place_msg_content_free(data_util_state* pds, data_place_msg_content* ppmc)
{
    data_place_free(pds, &ppmc->place);
    data_place_msg_banner_free(pds, &ppmc->place_msg_banner);
    data_message_text_free(pds, &ppmc->message_text);
    data_string_free(pds, &ppmc->signature);
}

tpselt
data_place_msg_content_to_tps(data_util_state* pds, data_place_msg_content* ppmc)
{
    tpselt te = 0;
    tpselt ce = 0;

    te = te_new("place-msg-content");

    if (!te)
    {
        goto errexit;
    }

    ce = data_place_to_tps(pds, &ppmc->place);
    if (ce && te_attach(te, ce))
    {
        ce = 0;
    }
    else
    {
        goto errexit;
    }

    ce = data_place_msg_banner_to_tps(pds, &ppmc->place_msg_banner);
    if (ce && te_attach(te, ce))
    {
        ce = 0;
    }
    else
    {
        goto errexit;
    }

    if (ppmc->place_msg_want_id)
    {
        ce = te_new("place-msg-want-id");
        if (ce && te_attach(te, ce))
        {
            ce = 0;
        }
        else
        {
            goto errexit;
        }
    }

    ce = data_message_text_to_tps(pds, &ppmc->message_text);
    if (ce && te_attach(te, ce))
    {
        ce = 0;
    }
    else
    {
        goto errexit;
    }

    if (!te_setattrc(te, "signature", data_string_get(pds, &ppmc->signature)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return 0;
}

/*! @} */
