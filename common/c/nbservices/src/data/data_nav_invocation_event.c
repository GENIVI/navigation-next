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

    @file     data_nav_invocation_event.c
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

/*! @{ */
#include "data_nav_invocation_event.h"

NB_Error
data_nav_invocation_event_init(data_util_state* pds, data_nav_invocation_event* pni)
{
    NB_Error err = NE_OK;

    pni->event_type = nav_invocation_event_none;

    return err;
}

void
data_nav_invocation_event_free(data_util_state* pds, data_nav_invocation_event* pni)
{
    switch (pni->event_type)
    {
    case nav_invocation_event_app_discovery:
        DATA_FREE(pds, &pni->event.app_discovery, data_app_discovery);
        break;

    case nav_invocation_event_use_navigator:
        DATA_FREE(pds, &pni->event.use_navigator, data_use_navigator);
        break;
    default:
        break;
    }

    pni->event_type = nav_invocation_event_none;
}

boolean
data_nav_invocation_event_equal(data_util_state* pds, data_nav_invocation_event* pni1, data_nav_invocation_event* pni2)
{
    if (pni1->event_type != pni2->event_type)
    {
        return FALSE;
    }

    switch (pni1->event_type)
    {
    case nav_invocation_event_app_discovery:
        if (!data_app_discovery_equal(pds, &pni1->event.app_discovery, &pni2->event.app_discovery))
        {
            return FALSE;
        }
        break;

    case nav_invocation_event_use_navigator:
        if (!data_use_navigator_equal(pds, &pni1->event.use_navigator, &pni2->event.use_navigator))
        {
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

NB_Error
data_nav_invocation_event_copy(data_util_state* pds, data_nav_invocation_event* pni_dest, data_nav_invocation_event* pni_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pni_dest, data_nav_invocation_event);
    
    pni_dest->event_type = pni_src->event_type;

    switch (pni_src->event_type)
    {
    case nav_invocation_event_app_discovery:
        DATA_COPY(pds, err, &pni_dest->event.app_discovery, &pni_src->event.app_discovery, data_app_discovery);
        break;

    case nav_invocation_event_use_navigator:
        DATA_COPY(pds, err, &pni_dest->event.use_navigator, &pni_src->event.use_navigator, data_use_navigator);
        break;

    default:
        return NE_INVAL;
    }

    return err;
}

NB_Error
data_nav_invocation_event_set_event_type(data_util_state* pds, data_nav_invocation_event* pni, nav_invocation_event_type new_type)
{
    NB_Error err = NE_OK;

    if (new_type == pni->event_type)
    {
        return NE_OK;
    }

    DATA_REINIT(pds, err, pni, data_nav_invocation_event);

    switch (new_type)
    {
    case nav_invocation_event_app_discovery:
        err = data_app_discovery_init(pds,
                    &pni->event.app_discovery);
        break;

    case nav_invocation_event_use_navigator:
        err = data_use_navigator_init(pds,
                    &pni->event.use_navigator);
        break;

    default:
        return NE_INVAL;
    }

    if (!err)
    {
        pni->event_type = new_type;
    }

    return err;
}

tpselt
data_nav_invocation_event_to_tps(data_util_state* pds, data_nav_invocation_event* pni)
{
    tpselt te = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "nav-invocation-event");

    switch (pni->event_type)
    {
    case nav_invocation_event_app_discovery:
        DATA_TO_TPS(pds, errexit, te, &pni->event.app_discovery, data_app_discovery);
        break;

    case nav_invocation_event_use_navigator:
        DATA_TO_TPS(pds, errexit, te, &pni->event.use_navigator, data_use_navigator);
        break;

    default:
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

uint32
data_nav_invocation_event_get_tps_size(data_util_state* pds, data_nav_invocation_event* pni)
{
    uint32 size = 0;
    
    size += sizeof(pni->event_type);

    switch (pni->event_type)
    {
    case nav_invocation_event_app_discovery:
        size += data_app_discovery_get_tps_size(pds, &pni->event.app_discovery);
        break;

    case nav_invocation_event_use_navigator:
        size += data_use_navigator_get_tps_size(pds, &pni->event.use_navigator);
        break;
    }

    return size;
}

void
data_nav_invocation_event_to_buf(data_util_state* pds, data_nav_invocation_event* pni, struct dynbuf* pdb)
{
    uint32 type = pni->event_type;

    dbufcat(pdb, (const byte*) &type, sizeof(type));

    switch (pni->event_type)
    {
    case nav_invocation_event_app_discovery:
        data_app_discovery_to_buf(pds, &pni->event.app_discovery, pdb);
        break;

    case nav_invocation_event_use_navigator:
        data_use_navigator_to_buf(pds, &pni->event.use_navigator, pdb);
        break;
    }
}

NB_Error
data_nav_invocation_event_from_binary(data_util_state* pds, data_nav_invocation_event* pni, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 type = 0;

    DATA_REINIT(pds, err, pni, data_nav_invocation_event);

    err = (err) ? err : data_uint32_from_binary(pds, &type, pdata, pdatalen);
    pni->event_type = (nav_invocation_event_type)type;

    switch (pni->event_type)
    {
    case nav_invocation_event_app_discovery:
        err = (err) ? err : data_app_discovery_from_binary(pds, &pni->event.app_discovery, pdata, pdatalen);
        break;

    case nav_invocation_event_use_navigator:
        err = (err) ? err : data_use_navigator_from_binary(pds, &pni->event.use_navigator, pdata, pdatalen);
        break;
    }

    return err;
}
/*! @} */
