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

#include "data_hours_of_operation.h"

NB_DEF NB_Error
data_hours_of_operation_init(data_util_state* pds, data_hours_of_operation* pdata)
{
    NB_Error err = NE_OK;
    err = err ? err : data_operating_hours_init(pds, &pdata->operating_hours);
    err = err ? err : data_string_init(pds, &pdata->operating_days);
    return err;
}

NB_DEF void
data_hours_of_operation_free(data_util_state* pds, data_hours_of_operation* pdata)
{
    data_operating_hours_free(pds, &pdata->operating_hours);
    data_string_free(pds, &pdata->operating_days);
}

NB_DEF NB_Error
data_hours_of_operation_from_tps(data_util_state* pds, data_hours_of_operation* pdata, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;
    if (te == NULL)
    {
        return NE_INVAL;
    }

    data_hours_of_operation_free(pds, pdata);

    err = data_hours_of_operation_init(pds, pdata);

    if (err != NE_OK)
        return err;

    ce = te_getchild(te, "operating-hours");
    err = err ? err : data_operating_hours_from_tps(pds, &pdata->operating_hours, ce);
    err = err ? err : data_string_from_tps_attr(pds, &pdata->operating_days, te, "operating-days");

    if (err != NE_OK)
    {
        data_hours_of_operation_free(pds, pdata);
    }
    return err;
}
