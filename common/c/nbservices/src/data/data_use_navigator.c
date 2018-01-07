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

    @file     data_use_navigator.c
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
#include "data_use_navigator.h"

NB_Error
data_use_navigator_init(data_util_state* pds, data_use_navigator* pun)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pun->place, data_place);
    DATA_INIT(pds, err, &pun->route_style, data_route_style);

    DATA_INIT(pds, err, &pun->platform, data_string);
    DATA_INIT(pds, err, &pun->carrier, data_string);
    DATA_INIT(pds, err, &pun->sdk_version, data_string);

    return err;
}

void
data_use_navigator_free(data_util_state* pds, data_use_navigator* pun)
{
    DATA_FREE(pds, &pun->place, data_place);
    DATA_FREE(pds, &pun->route_style, data_route_style);

    DATA_FREE(pds, &pun->platform, data_string);
    DATA_FREE(pds, &pun->carrier, data_string);
    DATA_FREE(pds, &pun->sdk_version, data_string);
}

boolean
data_use_navigator_equal(data_util_state* pds, data_use_navigator* pun1, data_use_navigator* pun2)
{
    return (boolean)data_place_equal(pds, &pun1->place, &pun2->place)
        && (boolean)data_route_style_equal(pds, &pun1->route_style, &pun2->route_style)
        && (boolean)data_string_equal(pds, &pun1->platform, &pun2->platform)
        && (boolean)data_string_equal(pds, &pun1->carrier, &pun2->carrier)
        && (boolean)data_string_equal(pds, &pun1->sdk_version, &pun2->sdk_version);
}

NB_Error
data_use_navigator_copy(data_util_state* pds, data_use_navigator* pun_dest, data_use_navigator* pun_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pun_dest, data_use_navigator);
    
    DATA_COPY(pds, err, &pun_dest->place, &pun_src->place, data_place);
    DATA_COPY(pds, err, &pun_dest->route_style, &pun_src->route_style, data_route_style);
    
    DATA_COPY(pds, err, &pun_dest->platform, &pun_src->platform, data_string);
    DATA_COPY(pds, err, &pun_dest->carrier, &pun_src->carrier, data_string);
    DATA_COPY(pds, err, &pun_dest->sdk_version, &pun_src->sdk_version, data_string);

    return err;
}

tpselt
data_use_navigator_to_tps(data_util_state* pds, data_use_navigator* pun)
{
    tpselt te = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "use-navigator");

    DATA_TO_TPS(pds, errexit, te, &pun->place, data_place);
    DATA_TO_TPS(pds, errexit, te, &pun->route_style, data_route_style);

    DATA_STR_SETATTR(pds, errexit, te, "platform", &pun->platform);
    DATA_STR_SETATTR(pds, errexit, te, "carrier", &pun->carrier);
    DATA_STR_SETATTR(pds, errexit, te, "sdk-version", &pun->sdk_version);

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

uint32
data_use_navigator_get_tps_size(data_util_state* pds, data_use_navigator* pun)
{
    uint32 size = 0;

    size += data_place_get_tps_size(pds, &pun->place);
    size += data_route_style_get_tps_size(pds, &pun->route_style);

    size += data_string_get_tps_size(pds, &pun->platform);
    size += data_string_get_tps_size(pds, &pun->carrier);
    size += data_string_get_tps_size(pds, &pun->sdk_version);

    return size;
}

void
data_use_navigator_to_buf(data_util_state* pds, data_use_navigator* pun, struct dynbuf* pdb)
{
    data_place_to_buf(pds, &pun->place, pdb);
    data_route_style_to_buf(pds, &pun->route_style, pdb);
    
    data_string_to_buf(pds, &pun->platform, pdb);
    data_string_to_buf(pds, &pun->carrier, pdb);
    data_string_to_buf(pds, &pun->sdk_version, pdb);
}

NB_Error
data_use_navigator_from_binary(data_util_state* pds, data_use_navigator* pun, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    
    err = (err) ? err : data_place_from_binary(pds, &pun->place, pdata, pdatalen);
    err = (err) ? err : data_route_style_from_binary(pds, &pun->route_style, pdata, pdatalen);
    
    err = (err) ? err : data_string_from_binary(pds, &pun->platform, pdata, pdatalen);
    err = (err) ? err : data_string_from_binary(pds, &pun->carrier, pdata, pdatalen);
    err = (err) ? err : data_string_from_binary(pds, &pun->sdk_version, pdata, pdatalen);
    
    return err;
}
/*! @} */
