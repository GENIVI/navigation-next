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
 * data_traffic_notify_query.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_traffic_notify_query.h"
#include "datautil.h"

NB_Error
data_traffic_notify_query_init(data_util_state* pds, data_traffic_notify_query* tnq)
{
	NB_Error err = NE_OK;

	DATA_INIT(pds, err, &tnq->traffic_record_identifier, data_traffic_record_identifier);
	DATA_INIT(pds, err, &tnq->cache_contents, data_cache_contents);
	DATA_INIT(pds, err, &tnq->want_extended_pronun_data, data_want_extended_pronun_data);

	return err;
}

void		
data_traffic_notify_query_free(data_util_state* pds, data_traffic_notify_query* tnq)
{
	DATA_FREE(pds, &tnq->traffic_record_identifier, data_traffic_record_identifier);
	DATA_FREE(pds, &tnq->cache_contents, data_cache_contents);
	DATA_FREE(pds, &tnq->want_extended_pronun_data, data_want_extended_pronun_data);
}

tpselt	
data_traffic_notify_query_to_tps(data_util_state* pds, data_traffic_notify_query* tnq)
{
	tpselt te = NULL;
	tpselt ce = NULL;

	te = te_new("traffic-notify-query");

	if (te == NULL)
		goto errexit;

	if ((ce = data_traffic_record_identifier_to_tps(pds, &tnq->traffic_record_identifier)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (!nsl_strempty(data_string_get(pds, &tnq->want_extended_pronun_data.supported_phonetics_formats)))
	{
		if ((ce = data_want_extended_pronun_data_to_tps(pds, &tnq->want_extended_pronun_data)) != NULL && te_attach(te, ce))
		    ce = NULL;
	    else
		    goto errexit;
	}
	else // Include the cache-contents element in case when downloadable prononunciation are needed only
	{
		if ((ce = data_cache_contents_to_tps(pds, &tnq->cache_contents)) != NULL && te_attach(te, ce))
		    ce = NULL;
	    else
		    goto errexit;
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}
