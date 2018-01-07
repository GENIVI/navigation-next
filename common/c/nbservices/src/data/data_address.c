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
 * data_address.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_address.h"

#define	FOREACH_BEGIN(iter) \
	do { \
		const struct data_address_info *iter = data_address_info_table; \
		for (; iter < data_address_info_table + sizeof(data_address_info_table) / \
				sizeof(*data_address_info_table); ++iter) { (void)0
#define	FOREACH_END } } while (0)

#define	ATTR(ob, x)	(data_string *)((char *)ob + (x)->offset)

static const struct data_address_info {
	const char name[20];
	const size_t offset;
	const int optional;
} data_address_info_table[] = {

// member field name is set to wire name which should be present in tps
// protocol. This is somewhat tricky, but it can save us from string
// replacement using hard-coded values.
#define X(name, wirename, optional) { #wirename, offsetof(data_address, name), optional},
	DATA_ADDRESS_ATTRS
#undef X
};

NB_Error
data_address_init(data_util_state* pds, data_address* pa)
{
    NB_Error error = NE_OK;

    error = error ? error : data_extended_address_init(pds, &pa->extended_address);
    error = error ? error : data_compact_address_init(pds, &pa->compact_address);

    if (error == NE_OK)
    {
        FOREACH_BEGIN(x);
            data_string_init(pds, ATTR(pa, x));
        FOREACH_END;
    }

    return error;
}

void
data_address_free(data_util_state* pds, data_address* pa)
{
    data_extended_address_free(pds, &pa->extended_address);
    data_compact_address_free(pds, &pa->compact_address);

    FOREACH_BEGIN(x);
        data_string_free(pds, ATTR(pa, x));
    FOREACH_END;
}

tpselt
data_address_to_tps(data_util_state* pds, data_address* pa)
{
    tpselt te;

    te = te_new("address");

    if (te == NULL)
        goto errexit;

    /* TRICKY: Child elements extended-address and compact-address are both
               used to display, they should not be sent to the server side.
               Please pay attention for this when using the code generator.
    */

    FOREACH_BEGIN(x);
        if (!te_setattrc(te, x->name, data_string_get(pds, ATTR(pa, x))))
            goto errexit;
    FOREACH_END;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_Error
data_address_from_tps(data_util_state* pds, data_address* pa, tpselt te)
{
    NB_Error err = NE_OK;
    char* data = NULL;
    size_t size = 0;
    tpselt childTpsElement = NULL;
    int iterator = 0;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_address_free(pds, pa);

    err = data_address_init(pds, pa);

    if (err != NE_OK)
        goto errexit;

    while ((childTpsElement = te_nextchild(te, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "extended-address") == 0)
        {
            err = err ? err : data_extended_address_from_tps(pds, &pa->extended_address, childTpsElement);
        }
        else if (nsl_strcmp(te_getname(childTpsElement), "compact-address") == 0)
        {
            err = err ? err : data_compact_address_from_tps(pds, &pa->compact_address, childTpsElement);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    FOREACH_BEGIN(x);
    data = NULL;
    size = 0;
    // Skip optional attributes if they don't exist.
    if (x->optional && !te_getattr(te, x->name, &data, &size))
    {
        continue;
    }

    err = err ? err : data_string_from_tps_attr(pds, ATTR(pa, x), te, x->name);

    FOREACH_END;

errexit:
    if (err != NE_OK)
        data_address_free(pds, pa);

    return err;
}

boolean
data_address_equal(data_util_state* pds, data_address* pa1, data_address* pa2)
{
     if (!data_extended_address_equal(pds, &pa1->extended_address, &pa2->extended_address))
     {
         return FALSE;
     }

     if (!data_compact_address_equal(pds, &pa1->compact_address, &pa2->compact_address))
     {
         return FALSE;
     }

    FOREACH_BEGIN(x);
        if (!data_string_equal(pds, ATTR(pa1, x), ATTR(pa2, x)))
            return FALSE;
    FOREACH_END;
    return TRUE;
}

NB_Error
data_address_copy(data_util_state* pds, data_address* pa_dest, data_address* pa_src)
{
    NB_Error err = NE_OK;

    data_address_free(pds, pa_dest);

    err = data_address_init(pds, pa_dest);

    err = err ? err : data_extended_address_copy(pds, &pa_dest->extended_address, &pa_src->extended_address);
    err = err ? err : data_compact_address_copy(pds, &pa_dest->compact_address, &pa_src->compact_address);

    FOREACH_BEGIN(x);
        err = err ? err : data_string_copy(pds, ATTR(pa_dest, x), ATTR(pa_src, x));
    FOREACH_END;

    if (err != NE_OK)
    {
        data_address_free(pds, pa_dest);
    }

    return err;
}

void
data_address_to_buf(data_util_state* pds, data_address* pa, struct dynbuf *dbp)
{
    data_extended_address_to_buf(pds, &pa->extended_address, dbp);
    data_compact_address_to_buf(pds, &pa->compact_address, dbp);

    FOREACH_BEGIN(x);
        data_string_to_buf(pds, ATTR(pa,x), dbp);
    FOREACH_END;
}

NB_Error
data_address_from_binary(data_util_state* pds, data_address* pa, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_extended_address_from_binary(pds, &pa->extended_address, ppdata, pdatalen);
    err = err ? err : data_compact_address_from_binary(pds, &pa->compact_address, ppdata, pdatalen);

    FOREACH_BEGIN(x);
        err = err ? err : data_string_from_binary(pds, ATTR(pa, x), ppdata, pdatalen);
    FOREACH_END;

    return err;
}

NB_Error
data_address_from_nimlocation(data_util_state* pds, data_address* pa, const NB_Location* pLocation)
{
    NB_Error err = NE_OK;
    const char* type = "";

    data_address_free(pds, pa);

    err = data_address_init(pds, pa);

    err = err ? err : data_string_set(pds, &pa->airport, pLocation->airport);
    err = err ? err : data_string_set(pds, &pa->city, pLocation->city);
    err = err ? err : data_string_set(pds, &pa->country, pLocation->country);
    err = err ? err : data_string_set(pds, &pa->county, pLocation->county);
    err = err ? err : data_string_set(pds, &pa->postal, pLocation->postal);
    err = err ? err : data_string_set(pds, &pa->sa, pLocation->streetnum);
    err = err ? err : data_string_set(pds, &pa->state, pLocation->state);
    err = err ? err : data_string_set(pds, &pa->str, pLocation->street1);
    err = err ? err : data_string_set(pds, &pa->xstr, pLocation->street2);
    err = err ? err : data_string_set(pds, &pa->freeform, pLocation->freeform);
    err = err ? err : data_string_set(pds, &pa->country_name, pLocation->country_name);

    switch (pLocation->type) {

        case NB_Location_Address:
            type = "address";
            break;
        case NB_Location_Intersection:
            type = "intersect";
            break;
        case NB_Location_Airport:
            type = "airport";
            break;
        case NB_Location_AddressFreeForm:
            type = "freeform";
            break;
        default:
            break;
    }

    err = err ? err : data_string_set(pds, &pa->type, type);

    err = err ? err : data_extended_address_from_nimlocation(pds, &(pa->extended_address), pLocation);
    err = err ? err : data_compact_address_from_nimlocation(pds, &(pa->compact_address), pLocation);

    if (err != NE_OK)
        data_address_free(pds, pa);

    return err;
}

NB_Error
data_address_to_nimlocation(data_util_state* state, data_address* address, NB_Location* location)
{
    NB_Error error = NE_OK;

    if ((!state) || (!address) || (!location))
    {
        return NE_INVAL;
    }

    if (data_string_compare_cstr(&(address->type), "address"))
    {
        location->type = NB_Location_Address;
    }
    else if (data_string_compare_cstr(&(address->type), "intersect"))
    {
        location->type = NB_Location_Intersection;
    }
    else if (data_string_compare_cstr(&(address->type), "airport"))
    {
        location->type = NB_Location_Airport;
    }
    else if (data_string_compare_cstr(&(address->type), "freeform"))
    {
        location->type = NB_Location_AddressFreeForm;
    }
    else
    {
        location->type = NB_Location_None;
    }

    data_string_get_copy(state, &(address->sa), location->streetnum, sizeof(location->streetnum));
    data_string_get_copy(state, &(address->str), location->street1, sizeof(location->street1));
    data_string_get_copy(state, &(address->xstr), location->street2, sizeof(location->street2));
    data_string_get_copy(state, &(address->city), location->city, sizeof(location->city));
    data_string_get_copy(state, &(address->county), location->county, sizeof(location->county));
    data_string_get_copy(state, &(address->state), location->state, sizeof(location->state));
    data_string_get_copy(state, &(address->postal), location->postal, sizeof(location->postal));
    data_string_get_copy(state, &(address->country), location->country, sizeof(location->country));
    data_string_get_copy(state, &(address->airport), location->airport, sizeof(location->airport));
    data_string_get_copy(state, &(address->freeform), location->freeform, sizeof(location->freeform));
    data_string_get_copy(state, &(address->country_name), location->country_name, sizeof(location->country_name));

    error = error ? error : data_extended_address_to_nimlocation(state, &(address->extended_address), location);
    error = error ? error : data_compact_address_to_nimlocation(state, &(address->compact_address), location);

    return error;
}

uint32   data_address_get_tps_size(data_util_state* pds, data_address* pa)
{
    uint32 size = 0;

    size += data_extended_address_get_tps_size(pds, &pa->extended_address);
    size += data_compact_address_get_tps_size(pds, &pa->compact_address);

    size += data_string_get_tps_size(pds, &pa->type);
    size += data_string_get_tps_size(pds, &pa->sa);
    size += data_string_get_tps_size(pds, &pa->xstr);
    size += data_string_get_tps_size(pds, &pa->str);
    size += data_string_get_tps_size(pds, &pa->city);
    size += data_string_get_tps_size(pds, &pa->county);
    size += data_string_get_tps_size(pds, &pa->state);
    size += data_string_get_tps_size(pds, &pa->postal);
    size += data_string_get_tps_size(pds, &pa->country);
    size += data_string_get_tps_size(pds, &pa->airport);
    size += data_string_get_tps_size(pds, &pa->freeform);
    size += data_string_get_tps_size(pds, &pa->country_name);

    return size;
}
