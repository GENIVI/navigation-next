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

 @file     data_speed_region.c
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

#include "data_speed_region.h"

NB_Error data_speed_region_init(data_util_state* state, data_speed_region* speedRegion)
{
    NB_Error error = NE_OK;
    if (state == NULL || speedRegion == NULL)
    {
        return NE_INVAL;
    }
    speedRegion ->special_speed_zone = (data_special_speed_zone*) nsl_malloc(sizeof(data_special_speed_zone));
    if (speedRegion->special_speed_zone == NULL)
    {
        error = NE_NOMEM;
    }
    error = error ? error : data_special_speed_zone_init(state, speedRegion->special_speed_zone);
    error = error ? error : data_string_init(state, &speedRegion->sign_id);
    error = error ? error : data_string_init(state, &speedRegion->sign_id_highlighted);
    error = error ? error : data_string_init(state, &speedRegion->version);
    error = error ? error : data_string_init(state, &speedRegion->version_highlighted);
    error = error ? error : data_string_init(state, &speedRegion->dataset_id);
    speedRegion->speed_limit = 0;
    speedRegion->start_maneuver_index = 0;
    speedRegion->start_maneuver_offset = 0;
    speedRegion->end_maneuver_index = 0;
    speedRegion->end_maneuver_offset = 0;
    return error;
}

void data_speed_region_free(data_util_state* state, data_speed_region* speedRegion)
{
    if (state == NULL || speedRegion == NULL)
    {
        return;
    }
    if (speedRegion->special_speed_zone != NULL) {
        data_special_speed_zone_free(state, speedRegion->special_speed_zone);
        nsl_free (speedRegion->special_speed_zone);
        speedRegion->special_speed_zone = NULL;
    }
    data_string_free(state, &speedRegion->sign_id);
    data_string_free(state, &speedRegion->sign_id_highlighted);
    data_string_free(state, &speedRegion->version);
    data_string_free(state, &speedRegion->version_highlighted);
    data_string_free(state, &speedRegion->dataset_id);
}

NB_Error data_speed_region_from_tps(data_util_state* state, data_speed_region* speedRegion, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    tpselt ce = NULL;
    if (state == NULL || speedRegion == NULL)
    {
        return NE_INVAL;
    }

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, speedRegion, data_speed_region);

    error = error ? error : data_string_from_tps_attr(state, &speedRegion->sign_id, tpsElement, "sign-id");
    error = error ? error : data_string_from_tps_attr(state, &speedRegion->sign_id_highlighted, tpsElement, "sign-id-highlighted");
    error = error ? error : data_string_from_tps_attr(state, &speedRegion->version, tpsElement, "version");
    error = error ? error : data_string_from_tps_attr(state, &speedRegion->version_highlighted, tpsElement, "version-highlighted");
    error = error ? error : data_string_from_tps_attr(state, &speedRegion->dataset_id, tpsElement, "dataset-id");

    if (error != NE_OK) {
        goto errexit;
    }

    te_getattrd(tpsElement, "speed-limit", &speedRegion->speed_limit);

    speedRegion->start_maneuver_index = te_getattru(tpsElement, "start-maneuver-index");
    te_getattrd(tpsElement, "start-maneuver-offset", &speedRegion->start_maneuver_offset);
    speedRegion->end_maneuver_index = te_getattru(tpsElement, "end-maneuver-index");
    te_getattrd(tpsElement, "end-maneuver-offset", &speedRegion->end_maneuver_offset);

    ce = te_getchild(tpsElement,"special-speed-zone");

    if (ce != NULL)
    {
        if (speedRegion->special_speed_zone == NULL)
        {
            speedRegion->special_speed_zone = nsl_malloc(sizeof(data_special_speed_zone));
            if (speedRegion->special_speed_zone == NULL)
            {
                error = error ? error : data_special_speed_zone_init(state, speedRegion->special_speed_zone);
            }
        }
        error = error ? error : data_special_speed_zone_from_tps(state, speedRegion->special_speed_zone, ce);
    }
    else
    {
        data_special_speed_zone_free(state, speedRegion->special_speed_zone);
        nsl_free(speedRegion->special_speed_zone);
        speedRegion->special_speed_zone = NULL;
    }
errexit:

    if (error != NE_OK)
    {
        data_speed_region_free(state, speedRegion);
    }

    return error;
}

NB_Error data_speed_region_copy(data_util_state* state,
                                        data_speed_region* destinationSpeedRegion,
                                        data_speed_region* sourceSpeedRegion)
{
    NB_Error error = NE_OK;
    if (state == NULL || destinationSpeedRegion == NULL || sourceSpeedRegion == NULL)
    {
        return NE_INVAL;
    }
    DATA_COPY(state, error, &destinationSpeedRegion->sign_id, &sourceSpeedRegion->sign_id, data_string);
    DATA_COPY(state, error, &destinationSpeedRegion->sign_id_highlighted, &sourceSpeedRegion->sign_id_highlighted, data_string);
    DATA_COPY(state, error, &destinationSpeedRegion->version, &sourceSpeedRegion->version, data_string);
    DATA_COPY(state, error, &destinationSpeedRegion->version_highlighted, &sourceSpeedRegion->version_highlighted, data_string);
    DATA_COPY(state, error, &destinationSpeedRegion->dataset_id, &sourceSpeedRegion->dataset_id, data_string);
    if (sourceSpeedRegion->special_speed_zone != NULL)
    {
        if (destinationSpeedRegion->special_speed_zone == NULL) {
            destinationSpeedRegion->special_speed_zone = (data_special_speed_zone*) nsl_malloc(sizeof(data_special_speed_zone));
            if (destinationSpeedRegion->special_speed_zone != NULL)
            {
                error = error ? error : data_special_speed_zone_init(state, destinationSpeedRegion->special_speed_zone);
            }
        }
        DATA_COPY(state, error, destinationSpeedRegion->special_speed_zone, sourceSpeedRegion->special_speed_zone, data_special_speed_zone);
    }
    else
    {
        if (destinationSpeedRegion -> special_speed_zone != NULL)
        {
            data_special_speed_zone_free(state, destinationSpeedRegion->special_speed_zone);
            nsl_free(destinationSpeedRegion->special_speed_zone);
            destinationSpeedRegion->special_speed_zone = NULL;
        }
    }

    destinationSpeedRegion->speed_limit = sourceSpeedRegion->speed_limit;
    destinationSpeedRegion->start_maneuver_index = sourceSpeedRegion->start_maneuver_index;
    destinationSpeedRegion->start_maneuver_offset = sourceSpeedRegion->start_maneuver_offset;
    destinationSpeedRegion->end_maneuver_index = sourceSpeedRegion->end_maneuver_index;
    destinationSpeedRegion->end_maneuver_offset = sourceSpeedRegion->end_maneuver_offset;

    return error;
}


/*! @} */
