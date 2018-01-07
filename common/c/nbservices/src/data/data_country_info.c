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

/*********************************************************************/
/*                                                                   */
/* (C) Copyright 2000 - 2005 by Networks In Motion, Inc.             */
/*                                                                   */
/* The information contained herein is confidential and proprietary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains proprietary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
* data_country_info.c created 2008/12/05 by Chitra R
*/

#include "data_country_info.h"

NB_Error	
data_country_info_init (data_util_state *pds, data_country_info* pci)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pci->code);
	err = err ? err : data_string_init(pds, &pci->driving_side);

	return err;
}

void		
data_country_info_free (data_util_state* pds, data_country_info* pci)
{
	data_string_free(pds, &pci->code);
	data_string_free(pds, &pci->driving_side);
}

NB_Error
data_country_info_from_tps (data_util_state* pds, data_country_info* pci, tpselt te)
{
	NB_Error err = NE_OK;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_string_from_tps_attr(pds, &pci->code, te, "code");
	err = err ? err : data_string_from_tps_attr(pds, &pci->driving_side, te, "driving-side");

errexit:
	if (err != NE_OK)
		data_country_info_free(pds, pci);

	return err;
}

tpselt
data_country_info_to_tps (data_util_state* pds, data_country_info* pci)
{
	tpselt te;

	te = te_new("country-info");

	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "code", data_string_get(pds, &pci->code)))
		goto errexit;

	if (!te_setattrc(te, "driving-side", data_string_get(pds, &pci->driving_side)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

boolean
data_country_info_equal	(data_util_state* pds, data_country_info* pci1, data_country_info* pci2)
{
	return (boolean) (data_string_equal(pds, &pci1->code, &pci2->driving_side)
		&& data_string_equal(pds, &pci1->driving_side, &pci2->driving_side));
}

NB_Error
data_country_info_copy (data_util_state* pds, data_country_info* pci_dest, data_country_info* pci_src)
{
	NB_Error err = NE_OK;

	data_country_info_free(pds, pci_dest);
	err = data_country_info_init(pds, pci_dest);

	err = err ? err : data_string_copy(pds, &pci_dest->code, &pci_src->code);
	err = err ? err : data_string_copy(pds, &pci_dest->driving_side, &pci_src->driving_side);

	return err;
}

