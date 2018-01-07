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
 * data_vector_tile_descriptor.c: created 2007/10/09 by Michael Gilbert
 */

#include "data_vector_tile_descriptor.h"

NB_Error
data_vector_tile_descriptor_init(data_util_state* pds, data_vector_tile_descriptor* ptd)
{
	NB_Error err = NE_OK;

	nsl_memset(ptd, 0, sizeof(*ptd));

	return err;
}

void
data_vector_tile_descriptor_free(data_util_state* pds, data_vector_tile_descriptor* ptd)
{
	// no data to free
}

tpselt
data_vector_tile_descriptor_to_tps(data_util_state* pds, data_vector_tile_descriptor* ptd)
{
	tpselt te;

	te = te_new("vector-tile-descriptor");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattru(te, "tx", ptd->tx))
		goto errexit;

	if (!te_setattru(te, "ty", ptd->ty))
		goto errexit;

	if (!te_setattru(te, "tz", ptd->tz))
		goto errexit;

	if (!te_setattru(te, "min-priority", ptd->minPpriority))
		goto errexit;

	if (!te_setattru(te, "max-priority", ptd->maxPriority))
		goto errexit;

	if (!te_setattrf(te, "min-scale", ptd->minScale))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}
NB_Error
data_vector_tile_descriptor_from_tps(data_util_state* pds, data_vector_tile_descriptor* ptd, tpselt te)
{
	ptd->tx = te_getattru(te, "tx");

	ptd->ty = te_getattru(te, "ty");

	ptd->tz = te_getattru(te, "tz");

	ptd->minPpriority = te_getattru(te, "min-priority");

	ptd->maxPriority = te_getattru(te, "max-priority");

	if (!te_getattrf(te, "min-scale", &ptd->minScale))
		return NE_INVAL;

	return NE_OK;
}

NB_Error
data_vector_tile_descriptor_copy(data_util_state* pds, data_vector_tile_descriptor* ptd_dest, data_vector_tile_descriptor* ptd_src)
{
	ptd_dest->tx = ptd_src->tx;
	ptd_dest->ty = ptd_src->ty;
	ptd_dest->tz = ptd_src->tz;
	ptd_dest->minPpriority = ptd_src->minPpriority;
	ptd_dest->maxPriority = ptd_src->maxPriority;
	ptd_dest->minScale = ptd_src->minScale;

	return NE_OK;
}

