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

    @file     data_route_request_event.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_route_request_event.h"

NB_Error
data_route_request_event_init(data_util_state* pds,
        data_route_request_event* prre)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &prre->analytics_event_origin,
            data_analytics_event_origin);

    DATA_INIT(pds, err, &prre->analytics_event_destination,
            data_analytics_event_destination);

    prre->nav_session_id = 0;

    DATA_INIT(pds, err, &prre->reason, data_string);

    return err;
}

void
data_route_request_event_free(data_util_state* pds,
        data_route_request_event* prre)
{
    DATA_FREE(pds, &prre->analytics_event_origin,
            data_analytics_event_origin);

    DATA_FREE(pds, &prre->analytics_event_destination,
            data_analytics_event_destination);

    DATA_FREE(pds, &prre->reason, data_string);
}

tpselt
data_route_request_event_to_tps(data_util_state* pds,
        data_route_request_event* prre)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("route-request-event");

    if (te == NULL)
    {
        goto errexit;
    }

    ce = data_analytics_event_origin_to_tps(pds,
            &prre->analytics_event_origin);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    ce = data_analytics_event_destination_to_tps(pds,
            &prre->analytics_event_destination);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", prre->nav_session_id))
        goto errexit;

    if (!te_setattrc(te, "reason", data_string_get(pds, &prre->reason)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_route_request_event_equal(data_util_state* pds,
        data_route_request_event* prre1, data_route_request_event* prre2)
{
    int ret = TRUE;

    ret = ret ? (prre1->nav_session_id == prre2->nav_session_id): ret;

    DATA_EQUAL(pds, ret, &prre1->reason, &prre2->reason, data_string);

    DATA_EQUAL(pds, ret, &prre1->analytics_event_origin,
            &prre2->analytics_event_origin,
            data_analytics_event_origin);

    DATA_EQUAL(pds, ret, &prre1->analytics_event_destination,
            &prre2->analytics_event_destination,
            data_analytics_event_destination);

    return (boolean) ret;
}

NB_Error
data_route_request_event_copy(data_util_state* pds,
        data_route_request_event* prre_dest, data_route_request_event* prre_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, prre_dest, data_route_request_event);

    prre_dest->nav_session_id = prre_src->nav_session_id;

    DATA_COPY(pds, err, &prre_dest->reason, &prre_src->reason, data_string);

    DATA_COPY(pds, err, &prre_dest->analytics_event_origin,
            &prre_src->analytics_event_origin,
            data_analytics_event_origin);

    DATA_COPY(pds, err, &prre_dest->analytics_event_destination,
            &prre_src->analytics_event_destination,
            data_analytics_event_destination);

    return err;
}

uint32
data_route_request_event_get_tps_size(data_util_state* pds,
            data_route_request_event* prre)
{
    uint32 size = 0;

    size += sizeof(prre->nav_session_id);

    if (prre->reason)
    {
        size += nsl_strlen(prre->reason);
    }

    size += data_analytics_event_origin_get_tps_size(pds,
            &prre->analytics_event_origin);

    size += data_analytics_event_destination_get_tps_size(pds,
            &prre->analytics_event_destination);

    return size;
}

void
data_route_request_event_to_buf(data_util_state* pds,
            data_route_request_event* prre,
            struct dynbuf* pdb)
{
    data_analytics_event_origin_to_buf(pds,
            &prre->analytics_event_origin, pdb);

    data_analytics_event_destination_to_buf(pds,
            &prre->analytics_event_destination, pdb);

    dbufcat(pdb, (const byte*)&prre->nav_session_id,
            sizeof(prre->nav_session_id));

    data_string_to_buf(pds, &prre->reason, pdb);
}

NB_Error
data_route_request_event_from_binary(data_util_state* pds,
            data_route_request_event* prre,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_analytics_event_origin_from_binary(pds,
            &prre->analytics_event_origin, pdata, pdatalen);

    err = err ? err : data_analytics_event_destination_from_binary(pds,
            &prre->analytics_event_destination, pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &prre->nav_session_id,
            pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &prre->reason,
            pdata, pdatalen);

    return err;
}


/*! @} */

