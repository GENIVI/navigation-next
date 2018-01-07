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
/* (C) Copyright 2006 by Networks In Motion, Inc.                */
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
 * data_sync_places_db_status_reply.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_places_db_status_reply.h"
#include "vec.h"
#include "abexp.h"

 NB_Error
data_sync_places_db_status_reply_init(data_util_state* pds, data_sync_places_db_status_reply* spdr)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &spdr->name);

	spdr->generation = 0;

	return err;
}

 void		
data_sync_places_db_status_reply_free(data_util_state* pds, data_sync_places_db_status_reply* spdr)
{
	data_string_free(pds, &spdr->name);

	spdr->generation = 0;
}

 NB_Error	
data_sync_places_db_status_reply_from_tps(data_util_state* pds, data_sync_places_db_status_reply* spdr, tpselt te)
{
	NB_Error err = NE_OK;

	if (te == NULL)
		return NE_INVAL;

	data_sync_places_db_status_reply_free(pds, spdr);

	err = data_sync_places_db_status_reply_init(pds, spdr);

	err = err ? err : data_string_from_tps_attr(pds, &spdr->name, te, "name");

	spdr->generation = te_getattru(te, "generation");
		
	if (err != NE_OK)
		data_sync_places_db_status_reply_free(pds, spdr);

	return err;
}

 NB_Error	
data_sync_places_db_status_reply_copy(data_util_state* pds, data_sync_places_db_status_reply* spdr_dest, data_sync_places_db_status_reply* spdr_src)
{
	NB_Error err = NE_OK;

	data_sync_places_db_status_reply_free(pds, spdr_dest);
	err = data_sync_places_db_status_reply_init(pds, spdr_dest);

	if (err != NE_OK)
		goto errexit;

	err = err ? err : data_string_copy(pds, &spdr_dest->name, &spdr_src->name);

	spdr_dest->generation = spdr_src->generation;

errexit:

	if (err != NE_OK)
		data_sync_places_db_status_reply_free(pds, spdr_dest);

	return err;
}
