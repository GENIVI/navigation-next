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

 @file     data_special_region.c
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_special_region.h"

NB_Error data_special_region_init(data_util_state* state, data_special_region* specialRegion)
{
    NB_Error error = NE_OK;
    if (!state || !specialRegion)
    {
        return NE_INVAL;
    }

    error = error ? error : data_string_init(state, &specialRegion->region_type);
    error = error ? error : data_string_init(state, &specialRegion->region_description);
    error = error ? error : data_string_init(state, &specialRegion->sign_id);
    error = error ? error : data_string_init(state, &specialRegion->version);
    error = error ? error : data_string_init(state, &specialRegion->dataset_id);
    specialRegion->start_maneuver_index = 0;
    specialRegion->start_maneuver_offset = 0;
    specialRegion->end_maneuver_index = 0;
    specialRegion->end_maneuver_offset = 0;
    specialRegion->next_occurrence_start_time = 0;
    specialRegion->next_occurrence_end_time = 0;
    return error;
}

void data_special_region_free(data_util_state* state, data_special_region* specialRegion)
{
    if (!state || !specialRegion)
    {
        return;
    }

    data_string_free(state, &specialRegion->region_type);
    data_string_free(state, &specialRegion->region_description);
    data_string_free(state, &specialRegion->sign_id);
    data_string_free(state, &specialRegion->version);
    data_string_free(state, &specialRegion->dataset_id);
}

NB_Error data_special_region_from_tps(data_util_state* state, data_special_region* specialRegion, tpselt tpsElement)
{
    NB_Error error = NE_OK;
    if (!specialRegion || !state || !tpsElement)
    {
        return NE_INVAL;
    }

    DATA_REINIT(state, error, specialRegion, data_special_region);

    error = error ? error : data_string_from_tps_attr(state, &specialRegion->region_type, tpsElement, "region-type");
    error = error ? error : data_string_from_tps_attr(state, &specialRegion->region_description, tpsElement, "region-description");
    error = error ? error : data_string_from_tps_attr(state, &specialRegion->sign_id, tpsElement, "sign-id");
    error = error ? error : data_string_from_tps_attr(state, &specialRegion->version, tpsElement, "version");
    error = error ? error : data_string_from_tps_attr(state, &specialRegion->dataset_id, tpsElement, "dataset-id");
    if (error == NE_OK)
    {
        specialRegion->start_maneuver_index = te_getattru(tpsElement, "start-maneuver-index");
        te_getattrd(tpsElement, "start-maneuver-offset", &specialRegion->start_maneuver_offset);
        specialRegion->end_maneuver_index = te_getattru(tpsElement, "end-maneuver-index");
        te_getattrd(tpsElement, "end-maneuver-offset", &specialRegion->end_maneuver_offset);
        specialRegion->next_occurrence_start_time = te_getattru(tpsElement, "next-occurence-start-time");
        specialRegion->next_occurrence_end_time = te_getattru(tpsElement, "next-occurence-end-time");
    }
    return error;
}

NB_Error data_special_region_copy(data_util_state* state, data_special_region* destinationSpecialRegion, data_special_region* sourceSpecialRegion)
{
    NB_Error error = NE_OK;
    if (!destinationSpecialRegion || !sourceSpecialRegion || !state)
    {
        return NE_INVAL;
    }

    DATA_COPY(state, error, &destinationSpecialRegion->region_type, &sourceSpecialRegion->region_type,data_string);
    DATA_COPY(state, error, &destinationSpecialRegion->region_description, &sourceSpecialRegion->region_description, data_string);
    DATA_COPY(state, error, &destinationSpecialRegion->sign_id, &sourceSpecialRegion->sign_id, data_string);
    DATA_COPY(state, error, &destinationSpecialRegion->version, &sourceSpecialRegion->version, data_string);
    DATA_COPY(state, error, &destinationSpecialRegion->dataset_id, &sourceSpecialRegion->dataset_id, data_string);
    destinationSpecialRegion->start_maneuver_index = sourceSpecialRegion->start_maneuver_index;
    destinationSpecialRegion->start_maneuver_offset = sourceSpecialRegion->start_maneuver_offset;
    destinationSpecialRegion->end_maneuver_index = sourceSpecialRegion->end_maneuver_index;
    destinationSpecialRegion->end_maneuver_offset = sourceSpecialRegion->end_maneuver_offset;
    destinationSpecialRegion->next_occurrence_start_time = sourceSpecialRegion->next_occurrence_start_time;
    destinationSpecialRegion->next_occurrence_end_time = sourceSpecialRegion->next_occurrence_end_time;
    return error;
}

/*! @} */