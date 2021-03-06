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

    @file     data_share_events_config.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_share_events_config.h"

NB_Error
data_share_events_config_init(data_util_state* pds, data_share_events_config* pcec)
{
    NB_Error err = NE_OK;

    pcec->priority = 0;

    return err;
}

void
data_share_events_config_free(data_util_state* pds, data_share_events_config* pcec)
{
	pcec->priority = 0;
}

NB_Error
data_share_events_config_from_tps(data_util_state* pds, data_share_events_config* pcec, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_share_events_config_free(pds, pcec);

    err = data_share_events_config_init(pds, pcec);

    if (err != NE_OK)
        return err;

    pcec->priority = te_getattru(te, "priority");

	return err;

errexit:
    if (err != NE_OK)
        data_share_events_config_free(pds, pcec);
    return err;
}

boolean
data_share_events_config_equal(data_util_state* pds, data_share_events_config* pcec1, data_share_events_config* pcec2)
{
    return (boolean) (pcec1->priority == pcec2->priority);
}

NB_Error
data_share_events_config_copy(data_util_state* pds, data_share_events_config* pcec_dest, data_share_events_config* pcec_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pcec_dest, data_share_events_config);

    pcec_dest->priority = pcec_src->priority;

    return err;
}

/*! @} */

