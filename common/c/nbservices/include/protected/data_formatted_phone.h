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

    @file   data_formatted_phone.h
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef DATA_FORMATTED_PHONE_H
#define DATA_FORMATTED_PHONE_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"

typedef struct data_formatted_phone_
{
    /* Child Elements */

    /* Attributes */
    data_string     text;       /*!< Formatted text of phone to display */

} data_formatted_phone;

NB_Error    data_formatted_phone_init(data_util_state* state, data_formatted_phone* formattedPhone);
void        data_formatted_phone_free(data_util_state* state, data_formatted_phone* formattedPhone);

NB_Error    data_formatted_phone_from_tps(data_util_state* state, data_formatted_phone* formattedPhone, tpselt tpsElement);

boolean     data_formatted_phone_equal(data_util_state* state, data_formatted_phone* formattedPhone1, data_formatted_phone* formattedPhone2);
NB_Error    data_formatted_phone_copy(data_util_state* state, data_formatted_phone* destinationFormattedPhone, data_formatted_phone* sourceFormattedPhone);

uint32      data_formatted_phone_get_tps_size(data_util_state* state, data_formatted_phone* formattedPhone);
void        data_formatted_phone_to_buf(data_util_state* state, data_formatted_phone* formattedPhone, struct dynbuf* buf);
NB_Error    data_formatted_phone_from_binary(data_util_state* state, data_formatted_phone* formattedPhone, byte** data, size_t* dataSize);

/*! @} */

#endif
