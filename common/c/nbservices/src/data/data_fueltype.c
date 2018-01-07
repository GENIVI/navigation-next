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
 * data_fueltype.c: created 2007/09/10 by Angie Sheha.
 */

#include "data_fueltype.h"

NB_Error
data_fueltype_init(data_util_state* pds, data_fueltype* pft)
{
    NB_Error err = NE_OK;

    err = data_string_init(pds, &pft->product_name);
    err = err ? err : data_string_init(pds, &pft->code);
    err = err ? err : data_string_init(pds, &pft->type_name);

    return err;
}

void
data_fueltype_free(data_util_state* pds, data_fueltype* pft)
{
    data_string_free(pds, &pft->product_name);
    data_string_free(pds, &pft->code);
    data_string_free(pds, &pft->type_name);
}

NB_Error
data_fueltype_from_tps(data_util_state* pds, data_fueltype* pft, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_fueltype_free(pds, pft);

    err = data_fueltype_init(pds, pft);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pft->product_name, te, "product-name");
    err = err ? err : data_string_from_tps_attr(pds, &pft->code, te, "code");
    err = err ? err : data_string_from_tps_attr(pds, &pft->type_name, te, "type-name");

errexit:
    if (err != NE_OK)
        data_fueltype_free(pds, pft);
    return err;
}

boolean
data_fueltype_equal(data_util_state* pds, data_fueltype* pft1, data_fueltype* pft2)
{
    return (boolean) (data_string_equal(pds, &pft1->product_name, &pft2->product_name) &&
                      data_string_equal(pds, &pft1->code, &pft2->code) &&
                      data_string_equal(pds, &pft1->type_name, &pft2->type_name));
}

NB_Error
data_fueltype_copy(data_util_state* pds, data_fueltype* pft_dest, data_fueltype* pft_src)
{
    NB_Error err = NE_OK;

    data_fueltype_free(pds, pft_dest);

    err = data_fueltype_init(pds, pft_dest);

    err = err ? err : data_string_copy(pds, &pft_dest->product_name, &pft_src->product_name);
    err = err ? err : data_string_copy(pds, &pft_dest->code, &pft_src->code);
    err = err ? err : data_string_copy(pds, &pft_dest->type_name, &pft_src->type_name);

    return err;
}

uint32 data_fueltype_get_tps_size(data_util_state* pds, data_fueltype* pp)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pp->product_name);
    size += data_string_get_tps_size(pds, &pp->code);
    size += data_string_get_tps_size(pds, &pp->type_name);

    return size;
}

void data_fueltype_to_buf(data_util_state* pds, data_fueltype* pp, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pp->product_name, pdb);
    data_string_to_buf(pds, &pp->code, pdb);
    data_string_to_buf(pds, &pp->type_name, pdb);
}

NB_Error data_fueltype_from_binary(data_util_state* pds, data_fueltype* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pp->product_name, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->code, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->type_name, pdata, pdatalen);

    return err;
}
