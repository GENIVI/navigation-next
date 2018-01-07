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
* data_nav_progress.c: created 2007/11/27 by Mark Goddard.
*/

#include "data_nav_progress.h"

NB_Error
data_nav_progress_init(data_util_state* pds, data_nav_progress* pnp)
{
	NB_Error err = NE_OK;

	DATA_MEM_ZERO(pnp, data_nav_progress);

	DATA_INIT(pds, err, &pnp->route_id, data_blob);
	DATA_INIT(pds, err, &pnp->session_id, data_string);
	DATA_INIT(pds, err, &pnp->state, data_string);

	pnp->position = 0;

	return err;
}

void		
data_nav_progress_free(data_util_state* pds, data_nav_progress* pnp)
{
	DATA_FREE(pds, &pnp->route_id, data_blob);
	DATA_FREE(pds, &pnp->session_id, data_string);
	DATA_FREE(pds, &pnp->state, data_string);
}

tpselt
data_nav_progress_to_tps(data_util_state* pds, data_nav_progress* pnp)
{
	tpselt te = NULL;

	te = te_new("nav-progress");

	if (te == NULL)
		goto errexit;

	if (!te_setattr(te, "route-id", (const char*) pnp->route_id.data, pnp->route_id.size))
		goto errexit;

	if (!te_setattrc(te, "session-id", data_string_get(pds, &pnp->session_id)))
		goto errexit;

	if (!te_setattrc(te, "state", data_string_get(pds, &pnp->state)))
		goto errexit;

	if (!te_setattru(te, "position", pnp->position))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_Error	
data_nav_progress_copy(data_util_state* pds, data_nav_progress* pnp_dest, data_nav_progress* pnp_src)
{
	NB_Error err = NE_OK;

	DATA_REINIT(pds, err, pnp_dest, data_nav_progress);

	DATA_COPY(pds, err, &pnp_dest->route_id, &pnp_src->route_id, data_blob);
	DATA_COPY(pds, err, &pnp_dest->session_id, &pnp_src->session_id, data_string);
	DATA_COPY(pds, err, &pnp_dest->state, &pnp_src->state, data_string);
	pnp_dest->position = pnp_src->position;

	return err;
}

