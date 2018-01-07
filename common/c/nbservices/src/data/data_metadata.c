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

/*!--------------------------------------------------------------------------

    @file     data_metadata.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_metadata.h"

NB_Error
data_metadata_init(data_util_state* state, data_metadata* metadata)
{
    data_box_init(state, &metadata->box);

    data_projection_parameters_init(state, &metadata->projection_parameters);

    data_string_init(state, &metadata->id);
    data_string_init(state, &metadata->display_name);
    data_string_init(state, &metadata->manifest_version);

    metadata->force_delete = FALSE;
    metadata->size = 0;

    return NE_OK;
}

void
data_metadata_free(data_util_state* state, data_metadata* metadata)
{
    data_box_free(state, &metadata->box);

    data_projection_parameters_free(state, &metadata->projection_parameters);

    data_string_free(state, &metadata->id);
    data_string_free(state, &metadata->display_name);
    data_string_free(state, &metadata->manifest_version);
}

NB_Error
data_metadata_from_tps(data_util_state* state, data_metadata* metadata, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, err, metadata, data_metadata);

    err = err ? err : data_string_from_tps_attr(state, &metadata->id, tpsElement, "id");
	err = err ? err : data_string_from_tps_attr(state, &metadata->display_name, tpsElement, "display-name");
	err = err ? err : data_string_from_tps_attr(state, &metadata->manifest_version, tpsElement, "manifest-version");

    if (err != NE_OK)
    {
        goto errexit;
    }

    metadata->size = te_getattru(tpsElement, "size");

    ce = te_getchild(tpsElement, "force-delete");
    if (ce != NULL)
    {
        metadata->force_delete = TRUE;
    }

    ce = te_getchild(tpsElement, "box");
    if (ce != NULL)
    {
        err = data_box_from_tps(state, &metadata->box, ce);
        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    ce = te_getchild(tpsElement, "projection-parameters");
    if (ce != NULL)
    {
        err = data_projection_parameters_from_tps(state, &metadata->projection_parameters, ce);
    }

errexit:

    if (err != NE_OK)
    {
        data_metadata_free(state, metadata);
    }

    return err;
}

boolean
data_metadata_equal(data_util_state* state, data_metadata* metadata1, data_metadata* metadata2)
{
    return (boolean) (metadata1->force_delete == metadata2->force_delete &&
					  metadata1->size == metadata2->size &&
				      data_string_equal(state, &metadata1->id, &metadata2->id) &&
					  data_string_equal(state, &metadata1->display_name, &metadata2->display_name) &&
					  data_string_equal(state, &metadata1->manifest_version, &metadata2->manifest_version) &&
					  data_box_equal(state, &metadata1->box, &metadata2->box) &&
					  data_projection_parameters_equal(state, &metadata1->projection_parameters, &metadata2->projection_parameters));
}

NB_Error
data_metadata_copy(data_util_state* state, data_metadata* destinationMetadata, data_metadata* sourceMetadata)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationMetadata, data_metadata);

    DATA_COPY(state, err, &destinationMetadata->box, &sourceMetadata->box, data_box);

    DATA_COPY(state, err, &destinationMetadata->projection_parameters, &sourceMetadata->projection_parameters, data_projection_parameters);

    DATA_COPY(state, err, &destinationMetadata->id, &sourceMetadata->id, data_string);

    DATA_COPY(state, err, &destinationMetadata->display_name, &sourceMetadata->display_name, data_string);

    DATA_COPY(state, err, &destinationMetadata->manifest_version, &sourceMetadata->manifest_version, data_string);

    if (err == NE_OK)
    {
        destinationMetadata->force_delete = sourceMetadata->force_delete;
        destinationMetadata->size = sourceMetadata->size;
    }

    return err;
}

/*! @} */
