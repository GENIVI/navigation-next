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

   @file     data_destination.c
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_destination.h"


static void
data_destination_clear(data_util_state* pds, data_destination* pod)
{

}

NB_Error
data_destination_init(data_util_state* pds, data_destination* pod)
{
    NB_Error err = NE_OK;

    pod->vec_gps = CSL_VectorAlloc(sizeof(data_gps));

    if (pod->vec_gps == NULL)
    {
        err = NE_NOMEM;
    }

    err = err ? err : data_point_init(pds, &pod->point);
    err = err ? err : data_address_init(pds, &pod->address);
    err = err ? err : data_string_init(pds, &pod->sa);
    err = err ? err : data_string_init(pds, &pod->str);

    data_destination_clear(pds, pod);

    if (err)
    {
        data_destination_free(pds, pod);
    }

    return err;
}

NB_Error
data_destination_copy(data_util_state* pds, data_destination* dst, data_destination* src)
{
    NB_Error err = NE_OK;
    int n = 0;
    int l = 0;
    data_gps gps = {{0}};

    err = data_gps_init(pds, &gps);

    l = CSL_VectorGetLength(src->vec_gps);
    for (n = 0; n < l && err == NE_OK; n++)
    {
        err = err ? err : data_gps_copy(pds, &gps, CSL_VectorGetPointer(src->vec_gps, n));
        err = err ? err : CSL_VectorAppend(dst->vec_gps, &gps) ? NE_OK : NE_NOMEM;

        if (err)
        {
            data_gps_free(pds, &gps);
        }
        else
        {
            err = data_gps_init(pds, &gps);
        }
    }

    err = err ? err : data_point_copy(pds, &dst->point, &src->point);
    err = err ? err : data_address_copy(pds, &dst->address, &src->address);
    err = err ? err : data_string_copy(pds, &dst->sa, &src->sa);
    err = err ? err : data_string_copy(pds, &dst->str, &src->str);

    return err;
}

void
data_destination_free(data_util_state* pds, data_destination* pod)
{
    int n = 0;
    int l = 0;

    if (pod->vec_gps)
    {
        l = CSL_VectorGetLength(pod->vec_gps);

        for (n=0;n<l;n++)
        {
            data_gps_free(pds, (data_gps*) CSL_VectorGetPointer(pod->vec_gps, n));
        }

        CSL_VectorDealloc(pod->vec_gps);
        pod->vec_gps = NULL;
    }

    data_point_free(pds, &pod->point);
    data_address_free(pds, &pod->address);
    data_string_free(pds, &pod->sa);
    data_string_free(pds, &pod->str);
    data_destination_clear(pds, pod);
}

tpselt
data_destination_to_tps(data_util_state* pds, data_destination* pod)
{
    tpselt te = NULL;
    tpselt ce = NULL;
    int n = 0;
    int l = 0;

    te = te_new("destination");

    if (te == NULL)
    {
        goto errexit;
    }

    if (pod->vec_gps)
    {
        l = CSL_VectorGetLength(pod->vec_gps);

        for (n=0; n<l; n++)
        {
            if ((ce = data_gps_to_tps(pds, (data_gps*) CSL_VectorGetPointer(pod->vec_gps, n))) != NULL && te_attach(te, ce))
            {
                ce = NULL;
            }
            else
            {
                goto errexit;
            }
        }
    }

    if ((ce = data_point_to_tps(pds, &pod->point)) != NULL && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if ((ce = data_address_to_tps(pds, &pod->address)) != NULL && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (!te_setattrc(te, "sa", data_string_get(pds, &pod->sa)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "str", data_string_get(pds, &pod->str)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);

    return NULL;
}

NB_Error
data_destination_clear_gps(data_util_state* pds, data_destination* pod)
{
    int n = 0;
    int l = 0;

    if (pod->vec_gps)
    {
        l = CSL_VectorGetLength(pod->vec_gps);

        for (n=0; n<l; n++)
        {
            data_gps_free(pds, (data_gps*) CSL_VectorGetPointer(pod->vec_gps, n));
        }

        CSL_VectorSetSize(pod->vec_gps, 0);
	}

    return NE_OK;
}

NB_Error
data_destination_add_gps(data_util_state* pds, data_destination* pod, data_gps* pg)
{
    NB_Error err = NE_OK;
    data_gps gps = {{0}};

    err = data_gps_init(pds, &gps);

    if( err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_gps_copy(pds, &gps, pg);
    err = err ? err : CSL_VectorAppend(pod->vec_gps, &gps) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_gps_free(pds, &gps);
    }

    return err;
}

int
data_destination_num_gps(data_util_state* pds, data_destination* pod)
{
    return CSL_VectorGetLength(pod->vec_gps);
}

boolean
data_destination_equal(data_util_state* pds, data_destination* pod1, data_destination* pod2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, pod1->vec_gps, pod2->vec_gps, data_gps);
    DATA_EQUAL(pds, ret, &pod1->point, &pod2->point, data_point);
    DATA_EQUAL(pds, ret, &pod1->address, &pod2->address, data_address);
    DATA_EQUAL(pds, ret, &pod1->sa, &pod2->sa, data_string);
    DATA_EQUAL(pds, ret, &pod1->str, &pod2->str, data_string);

    return (boolean) ret;
}

uint32
data_destination_get_tps_size(data_util_state* pds, data_destination* pod)
{
    uint32 size = 0;

    DATA_VEC_GET_TPS_SIZE(pds, size, pod->vec_gps, data_gps);
    size += data_point_get_tps_size(pds, &pod->point);
    size += data_address_get_tps_size(pds, &pod->address);
    size += data_string_get_tps_size(pds, &pod->sa);
    size += data_string_get_tps_size(pds, &pod->str);

    return size;
}

void
data_destination_to_buf(data_util_state* pds, data_destination* pod, struct dynbuf* pdb)
{
    DATA_VEC_TO_BUF(pds, pdb, pod->vec_gps, data_gps);
    data_point_to_buf(pds, &pod->point, pdb);
    data_address_to_buf(pds, &pod->address, pdb);
    data_string_to_buf(pds, &pod->sa, pdb);
    data_string_to_buf(pds, &pod->str, pdb);
}

NB_Error
data_destination_from_binary(data_util_state* pds, data_destination* pod, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pod->vec_gps, data_gps);
    err = err ? err : data_point_from_binary(pds, &pod->point, pdata, pdatalen);
    err = err ? err : data_address_from_binary(pds, &pod->address, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pod->sa, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pod->str, pdata, pdatalen);

    return err;
}
