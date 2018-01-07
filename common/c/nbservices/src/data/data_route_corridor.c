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

    @file     data_route_corridor.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_route_corridor.h"

NB_Error
data_route_corridor_init(data_util_state* pds, data_route_corridor* prc)
{
    NB_Error err = NE_OK;

    prc->distance = 0;
    prc->width = 0;
    err = err ? err : data_blob_init(pds, &prc->route_id);

    return err;
}

void
data_route_corridor_free(data_util_state* pds, data_route_corridor* prc)
{
    data_blob_free(pds, &prc->route_id);
}

tpselt
data_route_corridor_to_tps(data_util_state* pds, data_route_corridor* prc)
{
    tpselt te = te_new("route-corridor");

    if (!te)
    {
        goto errexit;
    }

    if (!te_setattru(te, "distance", prc->distance))
    {
        goto errexit;
    }

    if (!te_setattru(te, "width", prc->width))
    {
        goto errexit;
    }

    if (!te_setattr(te, "route-id", (const char*)prc->route_id.data, prc->route_id.size))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_route_corridor_equal(data_util_state* pds, data_route_corridor* prc1, data_route_corridor* prc2)
{
    return ((boolean)(prc1->distance == prc2->distance && prc1->width == prc2->width)
            && data_blob_equal(pds, &prc1->route_id, &prc2->route_id));
}

NB_Error
data_route_corridor_copy(data_util_state* pds, data_route_corridor* prc_dest, data_route_corridor* prc_src)
{
    NB_Error err = NE_OK;

    prc_dest->distance = prc_src->distance;
    prc_dest->width = prc_src->width;
    err = err ? err : data_blob_copy(pds, &prc_dest->route_id, &prc_src->route_id);

    return err;
}

/*! @} */
