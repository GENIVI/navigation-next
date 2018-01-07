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

   @file     data_want_extended_pronun_data.c
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_want_extended_pronun_data.h"

NB_Error
data_want_extended_pronun_data_init(data_util_state* pds, data_want_extended_pronun_data* pwepd)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pwepd, data_want_extended_pronun_data);
    DATA_INIT(pds, err, &pwepd->supported_phonetics_formats, data_string);

    return err;
}

void
data_want_extended_pronun_data_free(data_util_state* pds, data_want_extended_pronun_data* pwepd)
{
    DATA_FREE(pds, &pwepd->supported_phonetics_formats, data_string);
}

tpselt
data_want_extended_pronun_data_to_tps(data_util_state* state,
                                  data_want_extended_pronun_data* pwepd
                                 )
{
    tpselt te = NULL;

    te = te_new("want-extended-pronun-data");

    if (te == NULL)
    {
        goto errexit;
    }
    
    tpselt child = te_new("want-tts-ssml");
    if (!child || !te_attach(te, child))
    {
        return FALSE;
    }

    if (!te_setattrc(te, "supported-phonetics-formats", data_string_get(state, &pwepd->supported_phonetics_formats)))
    {
        goto errexit;
    }

    return te;

errexit:

    if (te)
    {
        te_dealloc(te);
    }

    return NULL;
}

NB_Error
data_want_extended_pronun_data_copy(data_util_state* pds, data_want_extended_pronun_data* pwepd_dest, data_want_extended_pronun_data* pwepd_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pwepd_dest, data_want_extended_pronun_data);
    DATA_COPY(pds, err, &pwepd_dest->supported_phonetics_formats, &pwepd_src->supported_phonetics_formats, data_string);

    return err;
}

/*! @} */
