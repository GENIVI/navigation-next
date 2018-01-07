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
 * data_ers_reply.c: created 2007/06/07 by Michael Gilbert.
 */

#include "data_ers_reply.h"
#include "data_ers_contact.h"
#include "vec.h"

NB_Error
data_ers_reply_init(data_util_state* pds, data_ers_reply* per)
{
	NB_Error err = NE_OK;

	per->vec_contacts = CSL_VectorAlloc(sizeof(data_ers_contact));

	if (per->vec_contacts == NULL)
		return NE_NOMEM;

	return err;
}

void		
data_ers_reply_free(data_util_state* pds, data_ers_reply* per)
{
	int n,l;

	if (per->vec_contacts) {
		l = CSL_VectorGetLength(per->vec_contacts);

		for (n=0;n<l;n++)
			data_ers_contact_free(pds, (data_ers_contact*) CSL_VectorGetPointer(per->vec_contacts, n));

		CSL_VectorDealloc(per->vec_contacts);
	}

	per->vec_contacts = NULL;
}

NB_Error	
data_ers_reply_from_tps(data_util_state* pds, data_ers_reply* per, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_ers_contact	contact;
	
	err = data_ers_contact_init(pds, &contact);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_ers_reply_free(pds, per);

	err = data_ers_reply_init(pds, per);

	if (err != NE_OK)
		return err;
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "ers-contact") == 0) {

			err = data_ers_contact_from_tps(pds, &contact, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(per->vec_contacts, &contact))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&contact, 0, sizeof(contact)); // clear out the contact since we have copied it (shallow copy)
				else
					data_ers_contact_free(pds, &contact); // free the contact if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}
	
errexit:
	data_ers_contact_free(pds, &contact);
	if (err != NE_OK)
		data_ers_reply_free(pds, per);
	return err;
}

NB_Error	
data_ers_reply_copy(data_util_state* pds, data_ers_reply* per_dest, data_ers_reply* per_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_ers_contact	contact;

	data_ers_contact_init(pds, &contact);

	data_ers_reply_free(pds, per_dest);
	err = err ? err : data_ers_reply_init(pds, per_dest);
	
	l = CSL_VectorGetLength(per_src->vec_contacts);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_ers_contact_copy(pds, &contact, CSL_VectorGetPointer(per_src->vec_contacts, n));
		err = err ? err : CSL_VectorAppend(per_dest->vec_contacts, &contact) ? NE_OK : NE_NOMEM;

		if (err)
			data_ers_contact_free(pds, &contact);
		else
			data_ers_contact_init(pds, &contact);
	}

	return err;
}

