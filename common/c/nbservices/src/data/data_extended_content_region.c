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

    @file     data_extended_content_region.c
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

#include "data_extended_content_region.h"
#include "data_content_path.h"

NB_Error
data_extended_content_region_init(data_util_state* state, data_extended_content_region* extendedContentRegion)
{
    extendedContentRegion->vec_content_path = CSL_VectorAlloc(sizeof(data_content_path));

    if (!extendedContentRegion->vec_content_path)
    {
        return NE_NOMEM;
    }

    data_projection_parameters_init(state, &extendedContentRegion->projection_parameters);

    data_string_init(state, &extendedContentRegion->id);
    data_string_init(state, &extendedContentRegion->version);
    data_string_init(state, &extendedContentRegion->dataset_id);
    data_string_init(state, &extendedContentRegion->type);

    extendedContentRegion->start_maneuver_index = 0;
    extendedContentRegion->start_maneuver_offset = 0.0;
    extendedContentRegion->end_maneuver_index = 0;
    extendedContentRegion->end_maneuver_offset = 0.0;

    data_string_init(state, &extendedContentRegion->projection);

    return NE_OK;
}

void
data_extended_content_region_free(data_util_state* state, data_extended_content_region* extendedContentRegion)
{
    int i = 0;
    int length = 0;

    if (extendedContentRegion->vec_content_path)
    {
        length = CSL_VectorGetLength(extendedContentRegion->vec_content_path);

        for (i =0; i < length; i++)
        {
            data_content_path_free(state, (data_content_path*)CSL_VectorGetPointer(extendedContentRegion->vec_content_path, i));
        }
        CSL_VectorDealloc(extendedContentRegion->vec_content_path);
    }

    data_projection_parameters_free(state, &extendedContentRegion->projection_parameters);

    data_string_free(state, &extendedContentRegion->id);
    data_string_free(state, &extendedContentRegion->version);
    data_string_free(state, &extendedContentRegion->dataset_id);
    data_string_free(state, &extendedContentRegion->type);

    data_string_free(state, &extendedContentRegion->projection);
}

NB_Error
data_extended_content_region_from_tps(data_util_state* state, data_extended_content_region* extendedContentRegion, tpselt tpsElement)
{
    NB_Error err = NE_OK;

    tpselt childElement = 0;
    int iterator = 0;
    data_content_path contentPath;

    if (!tpsElement)
    {
        return NE_INVAL;
    }

    err = data_content_path_init(state, &contentPath);
    if (err != NE_OK)
    {
        return err;
    }

    data_extended_content_region_free(state, extendedContentRegion);

    err = data_extended_content_region_init(state, extendedContentRegion);

    if (err == NE_OK)
    {
        iterator = 0;

        while ((childElement = te_nextchild(tpsElement, &iterator)) != NULL) 
        {
            if (nsl_strcmp(te_getname(childElement), "content-path") == 0)
            {
                err = data_content_path_from_tps(state, &contentPath, childElement);
                if (err == NE_OK)
                {
                    if (!CSL_VectorAppend(extendedContentRegion->vec_content_path, &contentPath))
                    {
                        err = NE_NOMEM;
                    }
                    nsl_memset(&contentPath, 0, sizeof(data_content_path));
                }

                if (err != NE_OK)
                {
                    break;
                }
            }
            else if (nsl_strcmp(te_getname(childElement), "projection-parameters") == 0)
            {
                err = data_projection_parameters_from_tps(state, &extendedContentRegion->projection_parameters, childElement);
            }

            if (err != NE_OK)
            {
                break;
            }
        }
    }

    err = err ? err : data_string_from_tps_attr(state, &extendedContentRegion->id, tpsElement, "id");
    err = err ? err : data_string_from_tps_attr(state, &extendedContentRegion->version, tpsElement, "version");
    err = err ? err : data_string_from_tps_attr(state, &extendedContentRegion->dataset_id, tpsElement, "dataset-id");
    err = err ? err : data_string_from_tps_attr(state, &extendedContentRegion->type, tpsElement, "type");
    err = err ? err : data_string_from_tps_attr(state, &extendedContentRegion->projection, tpsElement, "projection");

    if (err == NE_OK) 
    {
        extendedContentRegion->start_maneuver_index = te_getattru(tpsElement, "start-maneuver-index");
        extendedContentRegion->end_maneuver_index = te_getattru(tpsElement, "end-maneuver-index");
        (void)te_getattrd(tpsElement, "start-maneuver-offset", &extendedContentRegion->start_maneuver_offset);
        (void)te_getattrd(tpsElement, "end-maneuver-offset", &extendedContentRegion->end_maneuver_offset);
    }

    data_content_path_free(state, &contentPath);

    if (err != NE_OK)
    {
        data_extended_content_region_free(state, extendedContentRegion);
    }
    return err;
}

NB_Error
data_extended_content_region_copy(data_util_state* state, data_extended_content_region* destinationExtendedContentRegion, data_extended_content_region* sourceExtendedContentRegion)
{
    NB_Error err = NE_OK;

    DATA_VEC_COPY(state, err, destinationExtendedContentRegion->vec_content_path,
                              sourceExtendedContentRegion->vec_content_path,  data_content_path);

    err = err ? err : data_projection_parameters_copy(state, &destinationExtendedContentRegion->projection_parameters, &sourceExtendedContentRegion->projection_parameters);

    err = err ? err : data_string_copy(state, &destinationExtendedContentRegion->id ,&sourceExtendedContentRegion->id);
    err = err ? err : data_string_copy(state, &destinationExtendedContentRegion->version ,&sourceExtendedContentRegion->version);
    err = err ? err : data_string_copy(state, &destinationExtendedContentRegion->dataset_id ,&sourceExtendedContentRegion->dataset_id);
    err = err ? err : data_string_copy(state, &destinationExtendedContentRegion->type ,&sourceExtendedContentRegion->type);

    destinationExtendedContentRegion->start_maneuver_index = sourceExtendedContentRegion->start_maneuver_index;
    destinationExtendedContentRegion->start_maneuver_offset = sourceExtendedContentRegion->start_maneuver_offset;
    destinationExtendedContentRegion->end_maneuver_index = sourceExtendedContentRegion->end_maneuver_index;
    destinationExtendedContentRegion->end_maneuver_offset = sourceExtendedContentRegion->end_maneuver_offset;

    return err;
}

/*! @} */
