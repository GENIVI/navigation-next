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

    @file     data_msg_content.h
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

#ifndef DATA_MSG_CONTENT_H
#define DATA_MSG_CONTENT_H

#include "datautil.h"
#include "data_text_msg_content.h"
#include "data_place_msg_content.h"
#include "data_share_content.h"

typedef enum data_msg_content_type_
{
    MSG_CONTENT_UNDEFINED = 0,
    MSG_CONTENT_PLACE = 1,
    MSG_CONTENT_SHARE = 2,
    MSG_CONTENT_TEXT = 3

} data_msg_content_type;

typedef struct data_msg_content_
{
    data_msg_content_type  type;

    /* Child Elements */
    data_text_msg_content   text_msg_content;
    data_place_msg_content  place_msg_content;
    data_share_content      share_content;

    /* Attributes */

} data_msg_content;

NB_Error    data_msg_content_init(data_util_state* pds, data_msg_content* pmc);
void        data_msg_content_free(data_util_state* pds, data_msg_content* pmc);

tpselt      data_msg_content_to_tps(data_util_state* pds, data_msg_content* pmc);

#endif // DATA_MSG_CONTENT_H
