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

   @file     data_poiext_event.c
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

#include "data_poiext_event.h"


NB_Error
data_poiext_event_init(data_util_state* pds, data_poiext_event* ppe)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(ppe, data_poiext_event);

    DATA_INIT(pds, err, &ppe->analytics_event_place, data_analytics_event_place);
    DATA_INIT(pds, err, &ppe->name, data_string);

    if (err)
    {
        DATA_FREE(pds, ppe, data_poiext_event);
    }

    return err;
}

void
data_poiext_event_free(data_util_state* pds, data_poiext_event* ppe)
{
    DATA_FREE(pds, &ppe->analytics_event_place, data_analytics_event_place);
    DATA_FREE(pds, &ppe->name, data_string);
}

NB_Error
data_poiext_event_copy(data_util_state* pds, data_poiext_event* ppe_dst, data_poiext_event* ppe_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, ppe_dst, data_poiext_event);

    DATA_COPY(pds, err, &ppe_dst->analytics_event_place, &ppe_src->analytics_event_place, data_analytics_event_place);
    DATA_COPY(pds, err, &ppe_dst->name, &ppe_src->name, data_string);

    return err;
}

boolean
data_poiext_event_equal(data_util_state* pds, data_poiext_event* ppe1, data_poiext_event* ppe2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &ppe1->analytics_event_place, &ppe2->analytics_event_place, data_analytics_event_place);
    DATA_EQUAL(pds, ret, &ppe1->name, &ppe2->name, data_string);

    return (boolean)ret;
}

tpselt
data_poiext_event_to_tps(data_util_state* pds, data_poiext_event* ppe)
{
    tpselt te = NULL;

    te = te_new("extpoi-event");

    if (te == NULL)
    {
        goto errexit;
    }

    DATA_TO_TPS(pds, errexit, te, &ppe->analytics_event_place, data_analytics_event_place);
    DATA_STR_SETATTR(pds, errexit, te, "name",  &ppe->name);

    return te;

errexit:

    if (te)
    {
        te_dealloc(te);
    }

    return NULL;
}

uint32
data_poiext_event_get_tps_size(data_util_state* pds, data_poiext_event* ppe)
{
    uint32 size = 0;

    size += data_analytics_event_place_get_tps_size(pds, &ppe->analytics_event_place);
    size += data_string_get_tps_size(pds, &ppe->name);

    return size;
}

void
data_poiext_event_to_buf(data_util_state* pds, data_poiext_event* ppe, struct dynbuf* pdb)
{
    data_analytics_event_place_to_buf(pds, &ppe->analytics_event_place, pdb);
    data_string_to_buf(pds, &ppe->name, pdb);
}

NB_Error
data_poiext_event_from_binary(data_util_state* pds, data_poiext_event* ppe, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_analytics_event_place_from_binary(pds, &ppe->analytics_event_place, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &ppe->name, pdata, pdatalen);

    return err;
}

NB_Error
data_poiext_event_set_app(data_util_state* pds, data_poiext_event* ppe, NB_ThirdPartyApp app)
{
    NB_Error err = NE_OK;
    const char* name = NULL;

    switch (app)
    {
        case NB_TPA_Aisle411:
            name = "ASAPP";
            break;

        case NB_TPA_HopStop:
            name = "HSAPP";
            break;

        case NB_TPA_Fandango:
            name = "FDAPP";
            break;

        case NB_TPA_OpenTable:
            name = "OTAPP";
            break;

        default:
            return NE_BADDATA;
    }

    err = data_string_set(pds, &ppe->name, name);
    return err;
}

/*! @} */
