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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "data_operating_hours.h"

NB_DEF NB_Error
data_operating_hours_init(data_util_state* pds, data_operating_hours* pdata)
{
    NB_Error err = NE_OK;
    err = err ? err : data_string_init(pds, &pdata->starttime);
    err = err ? err : data_string_init(pds, &pdata->endtime);
    err = err ? err : data_string_init(pds, &pdata->headway_sec);
    return err;
}

NB_DEF void
data_operating_hours_free(data_util_state* pds, data_operating_hours* pdata)
{
    data_string_free(pds, &pdata->starttime);
    data_string_free(pds, &pdata->endtime);
    data_string_free(pds, &pdata->headway_sec);
}

NB_DEF NB_Error
data_operating_hours_from_tps(data_util_state* pds, data_operating_hours* pdata, tpselt te)
{
    NB_Error err = NE_OK;
    if (te == NULL)
    {
        return NE_INVAL;
    }

    data_operating_hours_free(pds, pdata);

    err = data_operating_hours_init(pds, pdata);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pdata->starttime, te, "starttime");
    err = err ? err : data_string_from_tps_attr(pds, &pdata->endtime, te, "endtime");
    err = err ? err : data_string_from_tps_attr(pds, &pdata->headway_sec, te, "headway-sec");

    if (err != NE_OK)
    {
        data_operating_hours_free(pds, pdata);
    }
    return err;
}
