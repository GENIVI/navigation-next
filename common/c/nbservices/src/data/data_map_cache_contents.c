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
 * data_map_cache_contents.c: created 2007/11/16 by Mark Goddard.
 */

#include "data_map_cache_contents.h"

static void
data_map_cache_contents_clear_variables(data_util_state* pds, data_map_cache_contents* pmc)
{
	
}

NB_Error	
data_map_cache_contents_init(data_util_state* pds, data_map_cache_contents* pmc)
{
	NB_Error err = NE_OK;

	pmc->vec_vector_tile_descriptors = CSL_VectorAlloc(sizeof(data_vector_tile_descriptor));
	
	if (pmc->vec_vector_tile_descriptors == NULL)
		err = NE_NOMEM;

	data_map_cache_contents_clear_variables(pds, pmc);

	if (err)
		data_map_cache_contents_free(pds, pmc);

	return err;
}

void		
data_map_cache_contents_free(data_util_state* pds, data_map_cache_contents* pmc)
{
	int n,l;

	if (pmc->vec_vector_tile_descriptors) {
		l = CSL_VectorGetLength(pmc->vec_vector_tile_descriptors);

		for (n=0;n<l;n++)
			data_vector_tile_descriptor_free(pds, (data_vector_tile_descriptor*) CSL_VectorGetPointer(pmc->vec_vector_tile_descriptors, n));

		CSL_VectorDealloc(pmc->vec_vector_tile_descriptors);
	}

	pmc->vec_vector_tile_descriptors = NULL;

	data_map_cache_contents_clear_variables(pds, pmc);
}

NB_Error	
data_map_cache_contents_copy(data_util_state* pds, data_map_cache_contents* dst, data_map_cache_contents* src)
{
    NB_Error err = NE_OK;
    int n, l;
    data_vector_tile_descriptor dvtd;
    err = err ? err : data_vector_tile_descriptor_init(pds, &dvtd);

    l = CSL_VectorGetLength(src->vec_vector_tile_descriptors);
    for (n = 0; n < l && err == NE_OK; n++) 
    {
        err = err ? err : data_vector_tile_descriptor_copy(pds, &dvtd, CSL_VectorGetPointer(src->vec_vector_tile_descriptors, n));
        err = err ? err : CSL_VectorAppend(dst->vec_vector_tile_descriptors, &dvtd) ? NE_OK : NE_NOMEM;

        if (err)
            data_vector_tile_descriptor_free(pds, &dvtd);
        else
            data_vector_tile_descriptor_init(pds, &dvtd);
    }

    return err;
}

tpselt		
data_map_cache_contents_to_tps(data_util_state* pds, data_map_cache_contents* pmc)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;
	data_vector_tile_descriptor* pci = NULL;

	te = te_new("map-cache-contents");

	if (te == NULL)
		goto errexit;

	if (pmc->vec_vector_tile_descriptors) {
		l = CSL_VectorGetLength(pmc->vec_vector_tile_descriptors);

		for (n=0;n<l;n++) {

			pci = (data_vector_tile_descriptor*) CSL_VectorGetPointer(pmc->vec_vector_tile_descriptors, n);

			if ((ce = data_vector_tile_descriptor_to_tps(pds, pci)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error	
data_map_cache_contents_clear(data_util_state* pds, data_map_cache_contents* pmc)
{
	int n,l;

	if (pmc->vec_vector_tile_descriptors)
    {
		l = CSL_VectorGetLength(pmc->vec_vector_tile_descriptors);

		for (n=0;n<l;n++)
			data_vector_tile_descriptor_free(pds, (data_vector_tile_descriptor*) CSL_VectorGetPointer(pmc->vec_vector_tile_descriptors, n));

		CSL_VectorSetSize(pmc->vec_vector_tile_descriptors, 0);
    }

	return NE_OK;
}

NB_Error	
data_map_cache_contents_add(data_util_state* pds, data_map_cache_contents* pmc, data_vector_tile_descriptor* pce)
{
	NB_Error err = NE_OK;
	data_vector_tile_descriptor	ce;

	data_vector_tile_descriptor_init(pds, &ce);

	err = err ? err : data_vector_tile_descriptor_copy(pds, &ce, pce);
	err = err ? err : CSL_VectorAppend(pmc->vec_vector_tile_descriptors, &ce) ? NE_OK : NE_NOMEM;

	if (err)
		data_vector_tile_descriptor_free(pds, &ce);

	return err;
}

int			
data_map_cache_contents_num(data_util_state* pds, data_map_cache_contents* pmc)
{
	return CSL_VectorGetLength(pmc->vec_vector_tile_descriptors);
}

