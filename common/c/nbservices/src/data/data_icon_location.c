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

 @file     data_icon_location.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_icon_location.h"


NB_Error
data_icon_location_init(data_util_state* pds, data_icon_location* pil)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pil->name, data_string);
    DATA_INIT(pds, err, &pil->description, data_string);

    if (err)
    {
        DATA_FREE(pds, pil, data_icon_location);
    }

    return err;
}

void
data_icon_location_free(data_util_state* pds, data_icon_location* pil)
{
    DATA_FREE(pds, &pil->name, data_string);
    DATA_FREE(pds, &pil->description, data_string);
}

NB_Error
data_icon_location_from_tps(data_util_state* pds, data_icon_location* pil, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pil, data_icon_location);

    err = err ? err : data_string_from_tps_attr(pds, &pil->name, te, "name");
    err = err ? err : data_string_from_tps_attr(pds, &pil->description, te, "description");

errexit:

    if (err != NE_OK)
    {
        DATA_FREE(pds, pil, data_icon_location);
    }

    return err;
}

boolean
data_icon_location_equal(data_util_state* pds, data_icon_location* pil1, data_icon_location* pil2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pil1->name, &pil2->name, data_string);
    DATA_EQUAL(pds, ret, &pil1->description, &pil2->description, data_string);

    return (boolean)ret;
}

NB_Error
data_icon_location_copy(data_util_state* pds, data_icon_location* pil_dest, data_icon_location* pil_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pil_dest, data_icon_location);

    DATA_COPY(pds, err, &pil_dest->name, &pil_src->name, data_string);
    DATA_COPY(pds, err, &pil_dest->description, &pil_src->description, data_string);

    return err;
}

/*! @} */