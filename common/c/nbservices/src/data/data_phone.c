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
 * data_phone.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_phone.h"

NB_DEF NB_Error
data_phone_init(data_util_state* pds, data_phone* pp)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_phone_init(pds, &pp->formatted_phone);

    err = err ? err : data_string_init(pds, &pp->kind);
    err = err ? err : data_string_init(pds, &pp->country);
    err = err ? err : data_string_init(pds, &pp->area);
    err = err ? err : data_string_init(pds, &pp->number);

    return err;

}

NB_DEF void
data_phone_free(data_util_state* pds, data_phone* pp)
{
    data_formatted_phone_free(pds, &pp->formatted_phone);

    data_string_free(pds, &pp->kind);
    data_string_free(pds, &pp->country);
    data_string_free(pds, &pp->area);
    data_string_free(pds, &pp->number);
}

NB_DEF NB_Error
data_phone_from_tps(data_util_state* pds, data_phone* pp, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt childTpsElement = NULL;
    int iterator = 0;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_phone_free(pds, pp);

    err = data_phone_init(pds, pp);

    if (err != NE_OK)
        return err;

    while ((childTpsElement = te_nextchild(te, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "formatted-phone") == 0)
        {
            err = err ? err : data_formatted_phone_from_tps(pds, &pp->formatted_phone, childTpsElement);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &pp->kind, te, "kind");
    err = err ? err : data_string_from_tps_attr(pds, &pp->country, te, "country");
    err = err ? err : data_string_from_tps_attr(pds, &pp->area, te, "area");
    err = err ? err : data_string_from_tps_attr(pds, &pp->number, te, "number");

errexit:
    if (err != NE_OK)
        data_phone_free(pds, pp);
    return err;
}

NB_DEF tpselt
data_phone_to_tps(data_util_state* pds, data_phone* pp)
{
    tpselt te;

    te = te_new("phone");

    if (te == NULL)
        goto errexit;

    /* TRICKY: Child element formatted-phone is only used to display, it
               should not be sent to the server side.
               Please pay attention for this when using the code generator.
    */

    if (!te_setattrc(te, "kind", data_string_get(pds, &pp->kind)))
        goto errexit;

    if (!te_setattrc(te, "country", data_string_get(pds, &pp->country)))
        goto errexit;

    if (!te_setattrc(te, "area", data_string_get(pds, &pp->area)))
        goto errexit;

    if (!te_setattrc(te, "number", data_string_get(pds, &pp->number)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);

    return NULL;
}

NB_DEF boolean
data_phone_equal(data_util_state* pds, data_phone* pp1, data_phone* pp2)
{
    return (boolean) (data_formatted_phone_equal(pds, &pp1->formatted_phone, &pp2->formatted_phone) &&
                      data_string_equal(pds, &pp1->kind, &pp2->kind) &&
                      data_string_equal(pds, &pp1->country, &pp2->country) &&
                      data_string_equal(pds, &pp1->area, &pp2->area) &&
                      data_string_equal(pds, &pp1->number, &pp2->number));
}

NB_DEF NB_Error
data_phone_copy(data_util_state* pds, data_phone* pp_dest, data_phone* pp_src)
{
    NB_Error err = NE_OK;

    data_phone_free(pds, pp_dest);

    err = err ? err : data_phone_init(pds, pp_dest);

    err = err ? err : data_formatted_phone_copy(pds, &pp_dest->formatted_phone, &pp_src->formatted_phone);

    err = err ? err : data_string_copy(pds, &pp_dest->kind, &pp_src->kind);
    err = err ? err : data_string_copy(pds, &pp_dest->country, &pp_src->country);
    err = err ? err : data_string_copy(pds, &pp_dest->area, &pp_src->area);
    err = err ? err : data_string_copy(pds, &pp_dest->number, &pp_src->number);

    return err;
}

NB_DEF NB_Error
data_phone_from_nimphone(data_util_state* pds, data_phone* pp, const NB_Phone* pPhone)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_set(pds, &pp->formatted_phone.text, pPhone->formattedText);

    switch (pPhone->type) {

        case NB_Phone_Primary:
            err = err ? err : data_string_set(pds, &pp->kind, "primary");
            break;
        case NB_Phone_Secondary:
            err = err ? err : data_string_set(pds, &pp->kind, "secondary");
            break;
        case NB_Phone_National:
            err = err ? err : data_string_set(pds, &pp->kind, "national");
            break;
        case NB_Phone_Mobile:
            err = err ? err : data_string_set(pds, &pp->kind, "mobile");
            break;
        case NB_Phone_Fax:
            err = err ? err : data_string_set(pds, &pp->kind, "fax");
            break;
    }

    err = err ? err : data_string_set(pds, &pp->area, pPhone->area);
    err = err ? err : data_string_set(pds, &pp->country, pPhone->country);
    err = err ? err : data_string_set(pds, &pp->number, pPhone->number);

    return err;
}

uint32   data_phone_get_tps_size(data_util_state* pds, data_phone* pp)
{
    uint32 size = 0;

    size += data_formatted_phone_get_tps_size(pds, &pp->formatted_phone);
    size += data_string_get_tps_size(pds, &pp->kind);
    size += data_string_get_tps_size(pds, &pp->country);
    size += data_string_get_tps_size(pds, &pp->area);
    size += data_string_get_tps_size(pds, &pp->number);

    return size;
}

void     data_phone_to_buf(data_util_state* pds, data_phone* pp, struct dynbuf* pdb)
{
    data_formatted_phone_to_buf(pds, &pp->formatted_phone, pdb);
    data_string_to_buf(pds, &pp->kind, pdb);
    data_string_to_buf(pds, &pp->country, pdb);
    data_string_to_buf(pds, &pp->area, pdb);
    data_string_to_buf(pds, &pp->number, pdb);
}

NB_Error data_phone_from_binary(data_util_state* pds, data_phone* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_formatted_phone_from_binary(pds, &pp->formatted_phone, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->kind, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->country, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->area, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->number, pdata, pdatalen);

    return err;
}

