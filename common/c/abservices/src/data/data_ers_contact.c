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
 * data_ers_contact.c: created 2007/06/07 by Michael Gilbert.
 */

#include "data_ers_contact.h"

NB_Error
data_ers_contact_init(data_util_state* pds, data_ers_contact* pec)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pec->name);
	err = err ? err : data_phone_init(pds, &pec->phone);

	return err;
}

void
data_ers_contact_free(data_util_state* pds, data_ers_contact* pec)
{
	data_string_free(pds, &pec->name);
	data_phone_free(pds, &pec->phone);
}

NB_Error
data_ers_contact_from_tps(data_util_state* pds, data_ers_contact* pec, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;

	data_ers_contact_free(pds, pec);
	err = data_ers_contact_init(pds, pec);

	if (err != NE_OK)
		return err;
	
	ce = te_getchild(te, "phone");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}
	
	err = err ? err : data_phone_from_tps(pds, &pec->phone, ce);

	if (err != NE_OK)
		return err;

	err = err ? err : data_string_from_tps_attr(pds, &pec->name, te, "name");

	errexit:
	if (err != NE_OK)
		data_ers_contact_free(pds, pec);

	return err;
}

NB_Error
data_ers_contact_copy(data_util_state* pds, data_ers_contact* pec_dest, data_ers_contact* pec_src)
{
	NB_Error err = NE_OK;

	data_ers_contact_free(pds, pec_dest);

	err = err ? err : data_ers_contact_init(pds, pec_dest);

	err = err ? err : data_phone_copy(pds, &pec_dest->phone, &pec_src->phone);
	err = err ? err : data_string_copy(pds, &pec_dest->name, &pec_src->name);

	return err;
}

