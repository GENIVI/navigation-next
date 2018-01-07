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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_url.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_url.h"

NB_DEF NB_Error
data_url_init(data_util_state* pds, data_url* pu)
{
    NB_Error err = NE_OK;

    err = data_string_init(pds, &pu->value);

    return err;
}

NB_DEF void
data_url_free(data_util_state* pds, data_url* pu)
{
    data_string_free(pds, &pu->value);
}

NB_DEF NB_Error
data_url_from_tps(data_util_state* pds, data_url* pu, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_url_free(pds, pu);

    err = data_url_init(pds, pu);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pu->value, te, "value");

errexit:
    if (err != NE_OK)
        data_url_free(pds, pu);
    return err;
}

NB_DEF boolean
data_url_equal(data_util_state* pds, data_url* pu1, data_url* pu2)
{
    return (boolean) (data_string_equal(pds, &pu1->value, &pu2->value));
}

NB_DEF NB_Error
data_url_copy(data_util_state* pds, data_url* pu_dest, data_url* pu_src)
{
    NB_Error err = NE_OK;

    data_url_free(pds, pu_dest);

    err = data_url_init(pds, pu_dest);

    err = err ? err : data_string_copy(pds, &pu_dest->value, &pu_src->value);

    return err;
}

NB_DEF void
data_url_to_buf(data_util_state* pds, data_url* pu, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pu->value, pdb);
}

NB_DEF NB_Error
data_url_from_binary(data_util_state* pds, data_url* pu, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pu->value, pdata, pdatalen);

    return err;

}

uint32 data_url_get_tps_size(data_util_state* pds, data_url* pu)
{
    return data_string_get_tps_size(pds, &pu->value);
}
