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
 * data_map_query.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_map_query.h"
#include "data_map_style.h"
#include "data_map_source.h"

static void
data_map_query_clear(data_util_state* pds, data_map_query* pmq)
{
	pmq->identify_center = FALSE;
	pmq->want_traffic = FALSE;
}

NB_Error	
data_map_query_init(data_util_state* pds, data_map_query* pmq)
{
	NB_Error err = NE_OK;

	err = err ? err : data_image_init(pds, &pmq->image);
	err = err ? err : data_map_style_init(pds, &pmq->map_style);
	err = err ? err : data_map_source_init(pds, &pmq->map_source);

	data_map_query_clear(pds, pmq);

	if (err)
		data_map_query_free(pds, pmq);

	return err;
}

void		
data_map_query_free(data_util_state* pds, data_map_query* pmq)
{
	data_image_free(pds, &pmq->image);
	data_map_style_free(pds, &pmq->map_style);
	data_map_source_free(pds, &pmq->map_source);
	
	data_map_query_clear(pds, pmq);
}

tpselt		
data_map_query_to_tps(data_util_state* pds, data_map_query* pmq)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("map-query");

	if (te == NULL)
		goto errexit;

	if ((ce = data_image_to_tps(pds, &pmq->image)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;
	
	if ((ce = data_map_style_to_tps(pds, &pmq->map_style)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if ((ce = data_map_source_to_tps(pds, &pmq->map_source)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (pmq->identify_center) {

		if ((ce = te_new("identify-center")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (pmq->want_pois) {   // @todo: (BUG 55805) revisit poi-scheme...
		if ((ce = te_new("want-pois")) != NULL && te_setattrc(ce, "poi-scheme", "vzw-premium") && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (pmq->want_traffic) {
		if ((ce = te_new("want-traffic")) != NULL && te_attach(te, ce))
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

boolean		
data_map_query_equal(data_util_state* pds, data_map_query* pmq1, data_map_query* pmq2)
{
	return	(boolean) (data_image_equal(pds, &pmq1->image, &pmq2->image) &&
			data_map_style_equal(pds, &pmq1->map_style, &pmq2->map_style) &&
			data_map_source_equal(pds, &pmq1->map_source, &pmq2->map_source) &&
			pmq1->identify_center == pmq2->identify_center &&
			pmq1->want_traffic == pmq2->want_traffic);
}

NB_Error	
data_map_query_copy(data_util_state* pds, data_map_query* pmq_dest, data_map_query* pmq_src)
{
	NB_Error err = NE_OK;

	data_map_query_free(pds, pmq_dest);

	err = err ? err : data_map_query_init(pds, pmq_dest);

	err = err ? err : data_image_copy(pds, &pmq_dest->image, &pmq_src->image);
	err = err ? err : data_map_style_copy(pds, &pmq_dest->map_style, &pmq_src->map_style);
	err = err ? err : data_map_source_copy(pds, &pmq_dest->map_source, &pmq_src->map_source);

	pmq_dest->identify_center = pmq_src->identify_center;
	pmq_dest->want_traffic = pmq_src->want_traffic;

	return err;
}

