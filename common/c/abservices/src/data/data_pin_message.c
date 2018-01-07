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

    @file        data_pin_message.c
*/
/*
    See file description in header file.

    (C) Copyright 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_pin_message.h"

NB_Error
data_pin_message_init(data_util_state* pds, data_pin_message* psm)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_text_init(pds, &psm->formatted_text);

    return err;
}

void
data_pin_message_free(data_util_state* pds, data_pin_message* psm)
{
    data_formatted_text_free(pds, &psm->formatted_text);
}

NB_Error
data_pin_message_from_tps(data_util_state* pds, data_pin_message* psm, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;

    data_pin_message_free(pds, psm);

    err = data_pin_message_init(pds, psm);

    if ((ce = te_getchild (te, "formatted-text")) != NULL)    
    {
        err = err ? err : data_formatted_text_from_tps(pds, &psm->formatted_text, ce);
    }

    err = err ? err : data_string_from_tps_attr(pds, &psm->pin, te, "pin");

    return err;
}

NB_Error    
data_pin_message_copy(data_util_state* pds, data_pin_message* psm_dst, data_pin_message* psm_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_text_copy(pds, &psm_dst->formatted_text, &psm_src->formatted_text);
    err = err ? err : data_string_copy(pds, &psm_dst->pin, &psm_src->pin); 
    return err;
}


/*! @} */
