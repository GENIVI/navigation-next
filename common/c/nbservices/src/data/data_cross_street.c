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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2007 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
* data_cross_street.c: created 2007/11/26 by Mark Goddard.
*/

#include "data_cross_street.h"

static void
data_cross_street_clear(data_util_state* pds, data_cross_street* pcs)
{
    pcs->progress = -1;
}

NB_Error	
data_cross_street_init(data_util_state* pds, data_cross_street* pcs)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pcs, data_cross_street);

    DATA_INIT(pds, err, &pcs->name, data_string);
    DATA_INIT(pds, err, &pcs->pronun, data_string);

    data_cross_street_clear(pds, pcs);
    pcs->playTime = 0;

    return err;
}

void
data_cross_street_free(data_util_state* pds, data_cross_street* pcs)
{
    DATA_FREE(pds, &pcs->name, data_string);
    DATA_FREE(pds, &pcs->pronun, data_string);

    data_cross_street_clear(pds, pcs);
}

NB_Error
data_cross_street_from_tps(data_util_state* pds, data_cross_street* pcs, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pcs, data_cross_street);

    if (err != NE_OK)
        return err;

    if (!err && !te_getattrf(te, "progress", &pcs->progress))
        err = NE_INVAL;

    err = err ? err : data_string_from_tps_attr(pds, &pcs->name, te, "name");
    err = err ? err : data_string_from_tps_attr(pds, &pcs->pronun, te, "pronun");

errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_cross_street: (error_code: %d)", err));
        DATA_FREE(pds, pcs, data_cross_street);
    }

    return err;
}

NB_Error
data_cross_street_copy(data_util_state* pds, data_cross_street* pcs_dest, data_cross_street* pcs_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pcs_dest, data_cross_street);

    DATA_COPY(pds, err, &pcs_dest->name, &pcs_src->name, data_string);
    DATA_COPY(pds, err, &pcs_dest->pronun, &pcs_src->pronun, data_string);

    pcs_dest->progress = pcs_src->progress;
    pcs_dest->playTime = pcs_src->playTime;

    return err;
}
