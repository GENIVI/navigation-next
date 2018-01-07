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
 * data_ers_query.c: created 2007/06/05 by Michael Gilbert.
 */

#include "data_ers_query.h"

NB_Error
data_ers_query_init(data_util_state* pds, data_ers_query* peq)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &peq->id);
	err = err ? err : data_gps_init(pds, &peq->gps);
	err = err ? err : data_phone_init(pds, &peq->phone);

	return err;
}

void
data_ers_query_free(data_util_state* pds, data_ers_query* peq)
{
	data_string_free(pds, &peq->id);
	data_gps_free(pds, &peq->gps);
	data_phone_free(pds, &peq->phone);
}

tpselt
data_ers_query_to_tps(data_util_state* pds, data_ers_query* peq)
{
	tpselt te;
	tpselt ce = NULL;
    byte ff = 0xFF;

	te = te_new("ers-query");
	
	if (te == NULL)
		goto errexit;

    if (nsl_memcmp(&peq->gps.packed[0], &ff, sizeof(byte)) != 0)
    {
	    if ((ce = data_gps_to_tps(pds, &peq->gps)) != NULL && te_attach(te, ce))
		    ce = NULL;
	    else
		    goto errexit;
    }

	if ((ce = data_phone_to_tps(pds, &peq->phone)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (!te_setattrc(te, "id", data_string_get(pds, &peq->id)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error
data_ers_query_copy(data_util_state* pds, data_ers_query* peq_dest, data_ers_query* peq_src)
{
	NB_Error err = NE_OK;

	data_ers_query_free(pds, peq_dest);

	err = err ? err : data_ers_query_init(pds, peq_dest);

	err = err ? err : data_gps_copy(pds, &peq_dest->gps, &peq_src->gps);
	err = err ? err : data_phone_copy(pds, &peq_dest->phone, &peq_src->phone);
	err = err ? err : data_string_copy(pds, &peq_dest->id, &peq_src->id);

	return err;
}

