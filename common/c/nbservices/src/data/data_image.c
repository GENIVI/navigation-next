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
 * data_image.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_image.h"

static void
data_image_clear(data_util_state* pds, data_image* pi)
{
	pi->width = 0;
	pi->height = 0;
	pi->dpi = 0;
}

NB_Error	
data_image_init(data_util_state* pds, data_image* pi)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pi->format);

	data_image_clear(pds, pi);

	return err;
}

void		
data_image_free(data_util_state* pds, data_image* pi)
{
	data_string_free(pds, &pi->format);

	data_image_clear(pds, pi);
}

tpselt		
data_image_to_tps(data_util_state* pds, data_image* pi)
{
	tpselt te;

	te = te_new("image");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "format", data_string_get(pds, &pi->format)))
		goto errexit;

	if (!te_setattru(te, "width", pi->width))
		goto errexit;

	if (!te_setattru(te, "height", pi->height))
		goto errexit;

	if (pi->dpi && !te_setattru(te, "dpi", pi->dpi))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

boolean		
data_image_equal(data_util_state* pds, data_image* pi1, data_image* pi2)
{
	return (boolean) (data_string_equal(pds, &pi1->format, &pi2->format) && 
					  pi1->width == pi2->width && pi1->height == pi2->height && pi1->dpi == pi2->dpi);
}

NB_Error	
data_image_copy(data_util_state* pds, data_image* pi_dest, data_image* pi_src)
{
	NB_Error err = NE_OK;

	data_image_free(pds, pi_dest);

	err = err ? err : data_image_init(pds, pi_dest);

	err = err ? err : data_string_copy(pds, &pi_dest->format, &pi_src->format);

	pi_dest->width = pi_src->width;
	pi_dest->height = pi_src->height;
	pi_dest->dpi = pi_src->dpi;

	return err;
}

