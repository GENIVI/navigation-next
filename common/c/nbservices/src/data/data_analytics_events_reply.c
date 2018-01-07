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

/*
 * data_analytics_events_reply.c: created 2007/06/12 by Michael Gilbert.
 */

#include "data_analytics_events_reply.h"
#include "datautil.h"

NB_Error
data_analytics_events_reply_init(data_util_state* pds, data_analytics_events_reply* paer)
{
	NB_Error err = NE_OK;

	paer->result = 0;

    paer->got_analytics_config = FALSE;
    DATA_INIT(pds, err, &paer->analytics_config, data_analytics_config);

	return err;
}

void
data_analytics_events_reply_free(data_util_state* pds, data_analytics_events_reply* paer)
{
    DATA_FREE(pds, &paer->analytics_config, data_analytics_config);
}

NB_Error
data_analytics_events_reply_from_tps(data_util_state* pds, data_analytics_events_reply* paer, tpselt te)
{
	NB_Error err = NE_OK;
    tpselt ce = NULL;

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_analytics_events_reply_free(pds, paer);

	err = data_analytics_events_reply_init(pds, paer);

	if (err != NE_OK)
		return err;

	if (te_getattru64(te, "result", &paer->result) == 0) {
        /* optional result field not available */
        paer->result = 0;
	}

    ce = te_getchild(te, "analytics-config");
	if (ce != NULL)
    {
		err = err ? err : data_analytics_config_from_tps(pds, &paer->analytics_config, ce);
        if (err == NE_OK)
        {
            /* got config */
            paer->got_analytics_config = TRUE;
        }
    }

	return err;

errexit:
	if (err != NE_OK)
		data_analytics_events_reply_free(pds, paer);
	return err;
}


NB_Error
data_analytics_events_reply_copy(data_util_state* pds, data_analytics_events_reply* paer_dest, data_analytics_events_reply* paer_src)
{
	NB_Error err = NE_OK;

	data_analytics_events_reply_free(pds, paer_dest);

	err = err ? err : data_analytics_events_reply_init(pds, paer_dest);

	paer_dest->result = paer_src->result;

    paer_dest->got_analytics_config = paer_src->got_analytics_config;
    DATA_COPY(pds, err, &paer_dest->analytics_config, &paer_src->analytics_config,
            data_analytics_config);

	return err;
}

