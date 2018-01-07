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

    @file     data_app_discovery.c
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

#include "data_app_discovery.h"

NB_Error
data_app_discovery_init(data_util_state* pds, data_app_discovery* pad)
{
    NB_Error err = NE_OK;

    pad->app_present = FALSE;

    DATA_INIT(pds, err, &pad->platform, data_string);
    DATA_INIT(pds, err, &pad->carrier, data_string);
    DATA_INIT(pds, err, &pad->sdk_version, data_string);

    return err;
}

void
data_app_discovery_free(data_util_state* pds, data_app_discovery* pad)
{
    pad->app_present = FALSE;

    DATA_FREE(pds, &pad->platform, data_string);
    DATA_FREE(pds, &pad->carrier, data_string);
    DATA_FREE(pds, &pad->sdk_version, data_string);
}

boolean
data_app_discovery_equal(data_util_state* pds, data_app_discovery* pad1, data_app_discovery* pad2)
{
    return (boolean)(pad1->app_present == pad2->app_present)
        && (boolean)data_string_equal(pds, &pad1->platform, &pad2->platform)
        && (boolean)data_string_equal(pds, &pad1->carrier, &pad2->carrier)
        && (boolean)data_string_equal(pds, &pad1->sdk_version, &pad2->sdk_version);
}

NB_Error
data_app_discovery_copy(data_util_state* pds, data_app_discovery* pad_dest, data_app_discovery* pad_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pad_dest, data_app_discovery);

    pad_dest->app_present = pad_src->app_present;

    DATA_COPY(pds, err, &pad_dest->platform, &pad_src->platform, data_string);
    DATA_COPY(pds, err, &pad_dest->carrier, &pad_src->carrier, data_string);
    DATA_COPY(pds, err, &pad_dest->sdk_version, &pad_src->sdk_version, data_string);

    return err;
}

tpselt
data_app_discovery_to_tps(data_util_state* pds, data_app_discovery* pad)
{
    tpselt te = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "app-discovery");

    if (pad->app_present)
    {
        EMPTY_ELEMENT_TO_TPS(pds, errexit, te, "app-present");
    }

    DATA_STR_SETATTR(pds, errexit, te, "platform", &pad->platform );
    DATA_STR_SETATTR(pds, errexit, te, "carrier", &pad->carrier );
    DATA_STR_SETATTR(pds, errexit, te, "sdk-version", &pad->sdk_version );

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

uint32
data_app_discovery_get_tps_size(data_util_state* pds, data_app_discovery* pad)
{
    uint32 size = 0;

    size += sizeof(pad->app_present);

    size += data_string_get_tps_size(pds, &pad->platform);
    size += data_string_get_tps_size(pds, &pad->carrier);
    size += data_string_get_tps_size(pds, &pad->sdk_version);

    return size;
}

void
data_app_discovery_to_buf(data_util_state* pds, data_app_discovery* pad, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*) &pad->app_present, sizeof(pad->app_present));

    data_string_to_buf(pds, &pad->platform, pdb);
    data_string_to_buf(pds, &pad->carrier, pdb);
    data_string_to_buf(pds, &pad->sdk_version, pdb);
}

NB_Error
data_app_discovery_from_binary(data_util_state* pds, data_app_discovery* pad, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = (err) ? err : data_boolean_from_binary(pds, &pad->app_present, pdata, pdatalen);

    err = (err) ? err : data_string_from_binary(pds, &pad->platform, pdata, pdatalen);
    err = (err) ? err : data_string_from_binary(pds, &pad->carrier, pdata, pdatalen);
    err = (err) ? err : data_string_from_binary(pds, &pad->sdk_version, pdata, pdatalen);

    return err;
}

/*! @} */
