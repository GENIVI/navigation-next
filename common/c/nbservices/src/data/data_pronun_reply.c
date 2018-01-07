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
 * data_pronun_reply.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_pronun_reply.h"
#include "csltypes.h"

NB_Error
data_pronun_reply_init(data_util_state* pds, data_pronun_reply* pnr)
{
	NB_Error err = NE_OK;

	pnr->vec_file = CSL_VectorAlloc(sizeof(data_file));

	if (pnr->vec_file == NULL)
    {
		return NE_NOMEM;
	}

	return err;
}

void
data_pronun_reply_free(data_util_state* pds, data_pronun_reply* pnr)
{
	int n,l;

    if (pnr->vec_file) {
        l = CSL_VectorGetLength(pnr->vec_file);

        for (n=0;n<l;n++)
        {
            data_file_free(pds, (data_file*) CSL_VectorGetPointer(pnr->vec_file, n));
        }
        CSL_VectorDealloc(pnr->vec_file);
    }
    pnr->vec_file = NULL;
}

NB_Error
data_pronun_reply_from_tps(data_util_state* pds, data_pronun_reply* pnr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;	
	data_file			file;	
	
	err = data_file_init(pds, &file);
	
    if (err != NE_OK)
		return err;

    if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_pronun_reply_free(pds, pnr);

	err = data_pronun_reply_init(pds, pnr);

	if (err != NE_OK)
		return err;

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) 
    {	
	   if (nsl_strcmp(te_getname(ce), "file") == 0) 
       {
			err = data_file_from_tps(pds, &file, ce);

			if (err == NE_OK) 
            {
				if (!CSL_VectorAppend(pnr->vec_file, &file))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&file, 0, sizeof(file)); // clear out the file since we have copied it (shallow copy)
				else
					data_file_free(pds, &file); // free the file if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

errexit:

	data_file_free(pds, &file);
		
	if (err != NE_OK)
	{
		data_pronun_reply_free(pds, pnr);
	}
	return err;
}

void
data_pronun_reply_file_foreach(data_util_state* pds, data_pronun_reply* pnr,
				void (*cb)(data_file* pf, void* pUser), void* pUser)
{
	data_file* f;
	int n,l;

	l = CSL_VectorGetLength(pnr->vec_file);

	for (n=0; n<l; n++) {

		f = (data_file*) CSL_VectorGetPointer(pnr->vec_file, n);

		if (cb)
			cb(f, pUser);
	}
}

void
data_pronun_reply_discard_files(data_util_state* pds, data_pronun_reply* pnr)
{
	int n,l;

	if (pnr->vec_file) {
		l = CSL_VectorGetLength(pnr->vec_file);

		for (n=0;n<l;n++)
			data_file_free(pds, (data_file*) CSL_VectorGetPointer(pnr->vec_file, n));

		CSL_VectorDealloc(pnr->vec_file);
	}

	pnr->vec_file = NULL;
}
