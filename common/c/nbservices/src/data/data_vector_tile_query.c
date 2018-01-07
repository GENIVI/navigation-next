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
* data_vector_tile_query.c: created 2007/11/14 by Mark Goddard
*/

#include "data_vector_tile_query.h"

NB_Error
data_vector_tile_query_init(data_util_state* state, data_vector_tile_query* query)
{
	NB_Error err = NE_OK;

	query->vec_vector_tile_descriptors = NULL;

	VEC_ALLOC_FSA(&state->vec_heap, sizeof(data_vector_tile_descriptor), query->vec_vector_tile_descriptors);

	if (query->vec_vector_tile_descriptors == NULL) 
    {
		err = NE_NOMEM;
		goto errexit;
	}

	return NE_OK;

errexit:

	data_vector_tile_query_free(state, query);
	return err;
}

void
data_vector_tile_query_free(data_util_state* state, data_vector_tile_query* query)
{
	int n = 0;
    int l = 0;

	if (query->vec_vector_tile_descriptors) 
    {
		l = CSL_VectorGetLength(query->vec_vector_tile_descriptors);

		for (n = 0; n < l; n++)
        {
			data_vector_tile_descriptor_free(state, (data_vector_tile_descriptor*) CSL_VectorGetPointer(query->vec_vector_tile_descriptors, n));
        }

		VEC_DEALLOC_FSA(&state->vec_heap, query->vec_vector_tile_descriptors);
	}

	query->vec_vector_tile_descriptors = NULL;
}

tpselt		
data_vector_tile_query_to_tps(data_util_state* state, data_vector_tile_query* query)
{
	tpselt te = NULL;
	tpselt ce = NULL;
	int n = 0;
    int l = 0;

	te = te_new("vector-tile-query");

	if (te == NULL)
		goto errexit;

	if (query->vec_vector_tile_descriptors) 
    {
		l = CSL_VectorGetLength(query->vec_vector_tile_descriptors);

		for (n=0;n<l;n++) 
        {
			data_vector_tile_descriptor* vtd = CSL_VectorGetPointer(query->vec_vector_tile_descriptors, n);

			if ((ce = data_vector_tile_descriptor_to_tps(state, vtd)) != NULL && te_attach(te, ce))
            {
				ce = NULL;
            }
			else
            {
				goto errexit;
            }
		}
	}

	if (!te_setattru(te, "passes", query->passes))
    {
		goto errexit;
    }

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error	
data_vector_tile_query_copy(data_util_state* state, data_vector_tile_query* destinationQuery, data_vector_tile_query* sourceQuery)
{
	NB_Error err = NE_OK;
	int n = 0;
    int l = 0;

	data_vector_tile_descriptor	tempDescriptor;
	data_vector_tile_descriptor_init(state, &tempDescriptor);

	data_vector_tile_query_free(state, destinationQuery);
	err = err ? err : data_vector_tile_query_init(state, destinationQuery);

	l = CSL_VectorGetLength(sourceQuery->vec_vector_tile_descriptors);

	for (n = 0; n < l && err == NE_OK; n++) 
    {
		err = err ? err : data_vector_tile_descriptor_copy(state, &tempDescriptor, CSL_VectorGetPointer(sourceQuery->vec_vector_tile_descriptors, n));
		err = err ? err : CSL_VectorAppend(destinationQuery->vec_vector_tile_descriptors, &tempDescriptor) ? NE_OK : NE_NOMEM;
	}

	// free scratch vector_tile
	data_vector_tile_descriptor_free(state, &tempDescriptor);

	return err;
}

NB_Error
data_vector_tile_add_descriptor(data_util_state* state, 
                                data_vector_tile_query* query, 
                                uint32 tx, 
                                uint32 ty, 
                                uint32 tz, 
                                uint32 minPpriority, 
                                uint32 maxPriority, 
                                double minScale)
{
	NB_Error err = NE_OK;
	data_vector_tile_descriptor tileDescriptor;

	err = data_vector_tile_descriptor_init(state, &tileDescriptor);

	if (err == NE_OK) 
    {
		tileDescriptor.tx               = tx;
		tileDescriptor.ty               = ty;
		tileDescriptor.tz               = tz;
		tileDescriptor.minPpriority     = minPpriority;
		tileDescriptor.maxPriority      = maxPriority;
		tileDescriptor.minScale         = minScale;

		err = err ? err : CSL_VectorAppend(query->vec_vector_tile_descriptors, &tileDescriptor) ? NE_OK : NE_NOMEM;

		if (err != NE_OK)
        {
			data_vector_tile_descriptor_free(state, &tileDescriptor);
        }
	}

	return err;
}
