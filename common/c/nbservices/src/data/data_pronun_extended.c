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

    @file     data_pronun_extended.c
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems, Inc is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#include "data_pronun_extended.h"
#include "data_string.h"
#include "datautil.h"

NB_Error
data_pronun_extended_init(data_util_state* pds, data_pronun_extended* ppe)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &ppe->key, data_string);
    DATA_INIT(pds, err, &ppe->language, data_string);
    DATA_INIT(pds, err, &ppe->text, data_string);
    DATA_INIT(pds, err, &ppe->phonetics, data_blob);
    ppe->playtime = 0;
    DATA_INIT(pds, err, &ppe->transliterated_text, data_string);

    return NE_OK;
}

void
data_pronun_extended_free(data_util_state* pds, data_pronun_extended* ppe)
{
    DATA_FREE(pds, &ppe->key, data_string);
    DATA_FREE(pds, &ppe->language, data_string);
    DATA_FREE(pds, &ppe->text, data_string);
    DATA_FREE(pds, &ppe->phonetics, data_blob);
    ppe->playtime = 0;
    DATA_FREE(pds, &ppe->transliterated_text, data_string);
}

NB_Error
data_pronun_extended_from_tps(data_util_state* pds, data_pronun_extended* ppe, tpselt te)
{
    NB_Error err = NE_OK;
    char*    data;
    size_t    size;

    if (te == NULL)
    {
        return NE_INVAL;
    }

    DATA_REINIT(pds, err, ppe, data_pronun_extended);

    err = err ? err : data_string_from_tps_attr(pds, &ppe->key, te, "key");
    err = err ? err : data_string_from_tps_attr(pds, &ppe->language, te, "language");
    err = err ? err : data_string_from_tps_attr(pds, &ppe->text, te, "text");
    if (te_getattr(te, "phonetics", &data, &size))
    {
        err = err ? err : data_blob_set(pds, &ppe->phonetics, (byte*) data, size);
    }
    ppe->playtime = te_getattru(te, "playtime");
    err = err ? err : data_string_from_tps_attr(pds, &ppe->transliterated_text, te, "transliterated-text");

    if (err != NE_OK)
    {
        data_pronun_extended_free(pds, ppe);
    }

    return err;
}

boolean
data_pronun_extended_equal(data_util_state* pds, data_pronun_extended* ppe1, data_pronun_extended* ppe2)
{
    int ret = TRUE;

    ret = ret ? (ppe1->playtime == ppe2->playtime): ret;
    DATA_EQUAL(pds, ret, &ppe1->key, &ppe2->key, data_string);
    DATA_EQUAL(pds, ret, &ppe1->language, &ppe2->language, data_string);
    DATA_EQUAL(pds, ret, &ppe1->text, &ppe2->text, data_string);
    DATA_EQUAL(pds, ret, &ppe1->phonetics, &ppe2->phonetics, data_blob);
    DATA_EQUAL(pds, ret, &ppe1->transliterated_text, &ppe2->transliterated_text, data_string);

    return (boolean) ret;
}

NB_Error
data_pronun_extended_copy(data_util_state* pds, data_pronun_extended* ppe_dest, data_pronun_extended* ppe_src)
{
	NB_Error err = NE_OK;

    DATA_REINIT(pds, err, ppe_dest, data_pronun_extended);
    DATA_COPY(pds, err, &ppe_dest->key, &ppe_src->key, data_string);
    DATA_COPY(pds, err, &ppe_dest->language, &ppe_src->language, data_string);
    DATA_COPY(pds, err, &ppe_dest->text, &ppe_src->text, data_string);
    DATA_COPY(pds, err, &ppe_dest->phonetics, &ppe_src->phonetics, data_blob);
    DATA_COPY(pds, err, &ppe_dest->transliterated_text, &ppe_src->transliterated_text, data_string);
    ppe_dest->playtime = ppe_src->playtime;

    return err;
}

