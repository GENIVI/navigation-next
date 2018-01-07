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
 * data_reverse_geocode_reply.c: created 2004/12/21 by Mark Goddard.
 */

#include "data_reverse_geocode_reply.h"

NB_Error
data_reverse_geocode_reply_init(data_util_state* pds, data_reverse_geocode_reply* prgr)
{
	NB_Error err = NE_OK;

	err = err ? err : data_location_init(pds, &prgr->location);

	return err;
}

void		
data_reverse_geocode_reply_free(data_util_state* pds, data_reverse_geocode_reply* prgr)
{
	data_location_free(pds, &prgr->location);
}

NB_Error	
data_reverse_geocode_reply_from_tps(data_util_state* pds, data_reverse_geocode_reply* prgr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;
	
    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_reverse_geocode_reply_free(pds, prgr);

    err = data_reverse_geocode_reply_init(pds, prgr);

    if (err != NE_OK)
        return err;

    ce = te_getchild(te, "location");

    if (ce == NULL) {
        err = NESERVER_UNSUPPORTEDLOCATION;
        goto errexit;
    }

    err = data_location_from_tps(pds, &prgr->location, ce);

    if (err != NE_OK)
        return err;

errexit:
    if (err != NE_OK)
        data_reverse_geocode_reply_free(pds, prgr);
    return err;
}

boolean		
data_reverse_geocode_reply_equal(data_util_state* pds, data_reverse_geocode_reply* prgr1, data_reverse_geocode_reply* prgr2)
{
	return (boolean) (data_location_equal(pds, &prgr1->location, &prgr2->location));
}

NB_Error	
data_reverse_geocode_reply_copy(data_util_state* pds, data_reverse_geocode_reply* prgr_dest, data_reverse_geocode_reply* prgr_src)
{
	NB_Error err = NE_OK;

	data_reverse_geocode_reply_free(pds, prgr_dest);

	err = err ? err : data_reverse_geocode_reply_init(pds, prgr_dest);

	err = err ? err : data_location_copy(pds, &prgr_dest->location, &prgr_src->location);

	return err;
}

