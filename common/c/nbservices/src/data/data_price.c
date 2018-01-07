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
 * data_price.c: created 2007/09/10 by Angie Sheha.
 */

#include "data_price.h"

NB_Error
data_price_init(data_util_state* pds, data_price* pp)
{
    NB_Error err = NE_OK;

    pp->value = 0.0;
    pp->modtime = 0;
    err = data_string_init(pds, &pp->currency);

    return err;
}

void
data_price_free(data_util_state* pds, data_price* pp)
{
    pp->value = 0.0;
    data_string_free(pds, &pp->currency);
}

NB_Error
data_price_from_tps(data_util_state* pds, data_price* pp, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_price_free(pds, pp);

    err = data_price_init(pds, pp);

    if (err != NE_OK)
        return err;

    if (!te_getattrd(te, "value", &pp->value))
        err = NE_BADDATA;

    pp->modtime = te_getattru(te, "modtime");
    err = err ? err : data_string_from_tps_attr(pds, &pp->currency, te, "currency");

errexit:
    if (err != NE_OK)
        data_price_free(pds, pp);
    return err;
}

boolean
data_price_equal(data_util_state* pds, data_price* pp1, data_price* pp2)
{
    return (boolean) (data_string_equal(pds, &pp1->currency, &pp2->currency) &&
                      pp1->value == pp2->value);
}

NB_Error
data_price_copy(data_util_state* pds, data_price* pp_dest, data_price* pp_src)
{
    NB_Error err = NE_OK;

    data_price_free(pds, pp_dest);

    err = data_price_init(pds, pp_dest);

    pp_dest->value = pp_src->value;
    err = err ? err : data_string_copy(pds, &pp_dest->currency, &pp_src->currency);

    return err;
}

uint32 data_price_get_tps_size(data_util_state* pds, data_price* pp)
{
    uint32 size = 0;

    size += sizeof(pp->value);
    size += data_string_get_tps_size(pds, &pp->currency);
    size += sizeof(pp->modtime);

    return size;
}

void data_price_to_buf(data_util_state* pds, data_price* pp, struct dynbuf* pdb)
{
    dbufcat(pdb, (byte*)&pp->value, sizeof(pp->value));
    data_string_to_buf(pds, &pp->currency, pdb);
    dbufcat(pdb, (byte*)&pp->modtime, sizeof(pp->modtime));
}

NB_Error data_price_from_binary(data_util_state* pds, data_price* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_double_from_binary(pds, &pp->value, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->currency, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->modtime, pdata, pdatalen);

    return err;
}
