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
 * data_iter_command.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_iter_command.h"

static void
data_iter_command_clear(data_util_state* pds, data_iter_command* pic)
{
	pic->number = 0;
}

NB_Error	
data_iter_command_init(data_util_state* pds, data_iter_command* pic)
{
	NB_Error err = NE_OK;

	err = err ? err : data_blob_init(pds, &pic->state);
	err = err ? err : data_string_init(pds, &pic->command);

	data_iter_command_clear(pds, pic);

	if (err)
		data_iter_command_free(pds, pic);

	return err;
}

void		
data_iter_command_free(data_util_state* pds, data_iter_command* pic)
{
	data_blob_free(pds, &pic->state);
	data_string_free(pds, &pic->command);

	data_iter_command_init(pds, pic);
}

tpselt		
data_iter_command_to_tps(data_util_state* pds, data_iter_command* pic)
{
	tpselt te;

	te = te_new("iter-command");

	if (te == NULL)
		goto errexit;

	if (!te_setattr(te, "state", (const char*) pic->state.data, pic->state.size))
		goto errexit;

	if (!te_setattrc(te, "command", data_string_get(pds, &pic->command)))
		goto errexit;

	if (!te_setattru(te, "number", pic->number))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

boolean		
data_iter_command_equal(data_util_state* pds, data_iter_command* pic1, data_iter_command* pic2)
{
	return	(boolean) (data_blob_equal(pds, &pic1->state, &pic2->state) &&
					   data_string_equal(pds, &pic1->command, &pic2->command) &&
					   pic1->number == pic2->number);
}

NB_Error	
data_iter_command_copy(data_util_state* pds, data_iter_command* pic_dest, data_iter_command* pic_src)
{
	NB_Error err = NE_OK;

	data_iter_command_free(pds, pic_dest);

	err = err ? err : data_iter_command_init(pds, pic_dest);

	err = err ? err : data_blob_copy(pds, &pic_dest->state, &pic_src->state);
	err = err ? err : data_string_copy(pds, &pic_dest->command, &pic_src->command);

	pic_dest->number = pic_src->number;

	return err;
}

