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
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
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
 * data_reverse_geocode_query.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_reverse_geocode_query.h"

static void
data_reverse_geocode_clear(data_util_state* pds, data_reverse_geocode_query* prgq)
{
	prgq->scale = 0.0;
	prgq->routeable = FALSE;
    prgq->avoidLocalRoad = FALSE;
}

NB_Error	
data_reverse_geocode_query_init(data_util_state* pds, data_reverse_geocode_query* prgq)
{
	NB_Error err = NE_OK;

	err = err ? err : data_position_init(pds, &prgq->position);

	data_reverse_geocode_clear(pds, prgq);

	if (err)
		data_reverse_geocode_query_free(pds, prgq);

	return err;
}

void		
data_reverse_geocode_query_free(data_util_state* pds, data_reverse_geocode_query* prgq)
{
	data_position_free(pds, &prgq->position);

	data_reverse_geocode_query_init(pds, prgq);
}

tpselt		
data_reverse_geocode_query_to_tps(data_util_state* pds, data_reverse_geocode_query* prgq)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("reverse-geocode-query");

	if (te == NULL)
		goto errexit;

	if ((ce = data_position_to_tps(pds, &prgq->position)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;
	
	if (prgq->routeable) {

		if ((ce = te_new("routeable")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

    if (prgq->avoidLocalRoad)
    {
        if ((ce = te_new("avoid-local-road")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

	if (!te_setattrf(te, "scale", prgq->scale))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);

	return NULL;
}

boolean		
data_reverse_geocode_query_equal(data_util_state* pds, data_reverse_geocode_query* prgq1, data_reverse_geocode_query* prgq2)
{
	return	(boolean) (data_position_equal(pds, &prgq1->position, &prgq2->position) &&
			prgq1->scale == prgq2->scale &&	prgq1->routeable == prgq2->routeable && prgq1->avoidLocalRoad == prgq2->avoidLocalRoad);
}

NB_Error	
data_reverse_geocode_query_copy(data_util_state* pds, data_reverse_geocode_query* prgq_dest, data_reverse_geocode_query* prgq_src)
{
	NB_Error err = NE_OK;

	data_reverse_geocode_query_free(pds, prgq_dest);

	err = err ? err : data_reverse_geocode_query_init(pds, prgq_dest);

	err = err ? err : data_position_copy(pds, &prgq_dest->position, &prgq_src->position);

	prgq_dest->scale = prgq_src->scale;
	prgq_dest->routeable = prgq_src->routeable;
	prgq_dest->avoidLocalRoad = prgq_src->avoidLocalRoad;

	return err;
}

