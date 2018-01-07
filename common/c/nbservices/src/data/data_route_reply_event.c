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

    @file     data_route_reply_event.c
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
#include "data_route_reply_event.h"

NB_Error
data_route_reply_event_init(data_util_state* pds, data_route_reply_event* prre)
{
    NB_Error err = NE_OK;

    prre->nav_session_id = 0;
    prre->route_request_event_id = 0;
    prre->duration = 0;
    prre->distance = 0;
    DATA_INIT(pds, err, &prre->route_id, data_blob);

    return err;
}

void
data_route_reply_event_free(data_util_state* pds, data_route_reply_event* prre)
{
    DATA_FREE(pds, &prre->route_id, data_blob);
}

tpselt
data_route_reply_event_to_tps(data_util_state* pds,
        data_route_reply_event* prre)
{
    tpselt te = NULL;

    te = te_new("route-reply-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", prre->nav_session_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "route-request-event-id", prre->route_request_event_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "duration", prre->duration))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "distance", prre->distance))
    {
        goto errexit;
    }

    if (!te_setattr(te, "route-id", (const char*)prre->route_id.data, prre->route_id.size))
    {
        goto errexit;
    }
    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_route_reply_event_equal(data_util_state* pds,
        data_route_reply_event* prre1, data_route_reply_event* prre2)
{
    if ((prre1->nav_session_id == prre2->nav_session_id) &&
        (prre1->route_request_event_id == prre2->route_request_event_id) &&
        (prre1->duration == prre2->duration) &&
        (prre1->distance == prre2->distance) &&
        data_blob_equal(pds, &prre1->route_id, &prre2->route_id))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_Error
data_route_reply_event_copy(data_util_state* pds,
        data_route_reply_event* prre_dest, data_route_reply_event* prre_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, prre_dest, data_route_reply_event);

    prre_dest->nav_session_id = prre_src->nav_session_id;
    prre_dest->route_request_event_id = prre_src->route_request_event_id;
    prre_dest->duration = prre_src->duration;
    prre_dest->distance = prre_src->distance;
    DATA_COPY(pds, err, &prre_dest->route_id, &prre_src->route_id, data_blob);

    return err;
}

uint32 data_route_reply_event_get_tps_size(data_util_state* pds,
            data_route_reply_event* prre)
{
    uint32 size = 0;

    size += sizeof(prre->nav_session_id);
    size += sizeof(prre->route_request_event_id);
    size += sizeof(prre->duration);
    size += sizeof(prre->distance);
    size += prre->route_id.size;

    return size;
}

void data_route_reply_event_to_buf(data_util_state* pds,
            data_route_reply_event* prre,
            struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*) &prre->nav_session_id,
            sizeof(prre->nav_session_id));

    dbufcat(pdb, (const byte*) &prre->route_request_event_id,
            sizeof(prre->route_request_event_id));

    dbufcat(pdb, (const byte*) &prre->duration, sizeof(prre->duration));
    dbufcat(pdb, (const byte*) &prre->distance, sizeof(prre->distance));

    data_blob_to_buf(pds, &prre->route_id, pdb);
}

NB_Error data_route_reply_event_from_binary(data_util_state* pds,
            data_route_reply_event* prre,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_uint32_from_binary(pds, &prre->nav_session_id,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds,
                                              &prre->route_request_event_id,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &prre->duration,
                                              pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &prre->distance,
                                              pdata, pdatalen);

    err = err ? err : data_blob_from_binary(pds, &prre->route_id,
                                            pdata, pdatalen);
    return err;
}

/*! @} */

