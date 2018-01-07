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

    This API allows a client to parse reply of authentication from server.

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

#ifndef DATAAUTHREPLY_H
#define DATAAUTHREPLY_H

/*! @{ */

#include "datautil.h"
#include "data_message.h"
#include "data_feature.h"
#include "data_client_stored_message.h"
#include "abexp.h"
#include "data_subscribed_message.h"
#include "data_pin_message.h"

typedef struct data_auth_reply_
{
    // Child Elements
    struct CSL_Vector*          vec_message;
    struct CSL_Vector*		    vec_feature;
    data_client_stored_message  client_stored_message;
    data_subscribed_message     subscribed_message;
    data_pin_message            pin_message;

    // Attributes
    int32			    status_code;    
    boolean             client_stored_message_valid;
    boolean             subscribed_message_valid;
    boolean             pin_message_valid;
} data_auth_reply;

NB_Error	data_auth_reply_init(data_util_state* pds, data_auth_reply* par);
void		data_auth_reply_free(data_util_state* pds, data_auth_reply* par);
NB_Error	data_auth_reply_from_tps(data_util_state* pds, data_auth_reply* par, tpselt te);
NB_Error	data_auth_reply_copy(data_util_state* pds, data_auth_reply* par_dest, data_auth_reply* par_src);
int     	data_auth_reply_num_feature(data_util_state* pds, data_auth_reply* par);
int     	data_auth_reply_num_message(data_util_state* pds, data_auth_reply* par);
int32		data_auth_reply_extapp_features(data_util_state* pds, data_auth_reply* par);

/*! @} */

#endif //DATAAUTHREPLY_H

