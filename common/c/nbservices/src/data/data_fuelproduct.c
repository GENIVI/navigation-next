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
 * data_fuelproduct.c: created 2007/09/10 by Angie Sheha.
 */

#include "data_fuelproduct.h"

NB_Error
data_fuelproduct_init(data_util_state* pds, data_fuelproduct* pfp)
{
    NB_Error err = NE_OK;

    err = data_string_init(pds, &pfp->units);
    err = err ? err : data_price_init(pds, &pfp->price);
    err = err ? err : data_fueltype_init(pds, &pfp->fuel_type);

    pfp->average = FALSE;
    pfp->low = FALSE;

    return err;
}

void
data_fuelproduct_free(data_util_state* pds, data_fuelproduct* pfp)
{
    data_string_free(pds, &pfp->units);
    data_price_free(pds, &pfp->price);
    data_fueltype_free(pds, &pfp->fuel_type);

    pfp->average = FALSE;
    pfp->low = FALSE;
}

NB_Error
data_fuelproduct_from_tps(data_util_state* pds, data_fuelproduct* pfp, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt  ce;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_fuelproduct_free(pds, pfp);

    err = data_fuelproduct_init(pds, pfp);

    if (err != NE_OK)
        return err;

    //price
    ce = te_getchild(te, "price");

    if (ce == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    err = err ? err : data_price_from_tps(pds, &pfp->price, ce);

    //fuel type
    ce = te_getchild(te, "fuel-type");

    if (ce == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    err = err ? err : data_fueltype_from_tps(pds, &pfp->fuel_type, ce);

    pfp->average = te_getchild(te, "average") != NULL ? TRUE : FALSE;
    pfp->low     = te_getchild(te, "low") != NULL ? TRUE : FALSE;

    err = err ? err : data_string_from_tps_attr(pds, &pfp->units, te, "units");

errexit:
    if (err != NE_OK)
        data_fuelproduct_free(pds, pfp);
    return err;
}

boolean
data_fuelproduct_equal(data_util_state* pds, data_fuelproduct* pfp1, data_fuelproduct* pfp2)
{
    return (boolean) (data_string_equal(pds, &pfp1->units, &pfp2->units) &&
                    data_price_equal(pds, &pfp1->price, &pfp2->price) &&
                    data_fueltype_equal(pds, &pfp1->fuel_type, &pfp2->fuel_type) &&
                    pfp1->average == pfp2->average &&
                    pfp1->low == pfp2->low);
}

NB_Error
data_fuelproduct_copy(data_util_state* pds, data_fuelproduct* pfp_dest, data_fuelproduct* pfp_src)
{
    NB_Error err = NE_OK;

    data_fuelproduct_free(pds, pfp_dest);

    err = data_fuelproduct_init(pds, pfp_dest);

    err = err ? err : data_string_copy(pds, &pfp_dest->units, &pfp_src->units);
    err = err ? err : data_price_copy(pds, &pfp_dest->price, &pfp_src->price);
    err = err ? err : data_fueltype_copy(pds, &pfp_dest->fuel_type, &pfp_src->fuel_type);
    pfp_dest->average = pfp_src->average;
    pfp_dest->low = pfp_src->low;
    return err;
}

uint32 data_fuelproduct_get_tps_size(data_util_state* pds, data_fuelproduct* pp)
{
    uint32 size = 0;

    size += data_price_get_tps_size(pds, &pp->price);
    size += data_fueltype_get_tps_size(pds, &pp->fuel_type);

    size += data_string_get_tps_size(pds, &pp->units);

    size += sizeof(pp->average);
    size += sizeof(pp->low);

    return size;
}

void data_fuelproduct_to_buf(data_util_state* pds, data_fuelproduct* pp, struct dynbuf* pdb)
{
    data_price_to_buf(pds, &pp->price, pdb);
    data_fueltype_to_buf(pds, &pp->fuel_type, pdb);

    data_string_to_buf(pds, &pp->units, pdb);

    dbufcat(pdb, (byte*)&pp->average, sizeof(pp->average));
    dbufcat(pdb, (byte*)&pp->low, sizeof(pp->low));
}

NB_Error data_fuelproduct_from_binary(data_util_state* pds, data_fuelproduct* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_price_from_binary(pds, &pp->price, pdata, pdatalen);
    err = err ? err : data_fueltype_from_binary(pds, &pp->fuel_type, pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &pp->units, pdata, pdatalen);

    err = err ? err : data_boolean_from_binary(pds, &pp->average, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &pp->low, pdata, pdatalen);

    return err;
}
