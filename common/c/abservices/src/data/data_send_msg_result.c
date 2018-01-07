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

    @file     data_send_msg_result.c
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

#include "data_send_msg_result.h"

NB_Error
data_send_msg_result_init(data_util_state* pds, data_send_msg_result* psmr)
{
    NB_Error err = NE_OK;

    err = err ? err : data_message_id_init(pds, &psmr->message_id);
    err = err ? err : data_error_msg_init(pds, &psmr->error_msg);
    err = err ? err : data_string_init(pds, &psmr->to);

    return err;
}

void
data_send_msg_result_free(data_util_state* pds, data_send_msg_result* psmr)
{
    data_message_id_free(pds, &psmr->message_id);
    data_error_msg_free(pds, &psmr->error_msg);
    data_string_free(pds, &psmr->to);
}

NB_Error
data_send_msg_result_from_tps(data_util_state* pds, data_send_msg_result* psmr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = 0;

    data_send_msg_result_init(pds, psmr);

    ce = te_getchild(te, "message-id");
    if (ce)
    {
        err = err ? err : data_message_id_from_tps(pds, &psmr->message_id, ce);
    }
 
    ce = te_getchild(te, "error-msg");
    if (ce)
    {
        err = err ? err : data_error_msg_from_tps(pds, &psmr->error_msg, ce);
    }
    
    err = err ? err : data_string_from_tps_attr(pds, &psmr->to, te, "to");

    if (err != NE_OK)
    {
        data_send_msg_result_free(pds, psmr);
    }
    return err;
}

/*! @} */
