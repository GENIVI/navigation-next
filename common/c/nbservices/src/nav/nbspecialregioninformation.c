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

 @file     nbspecialregioninformation.c
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */
#include "nbcontextprotected.h"
#include "nbspecialregioninformationprivate.h"

NB_Error
DataStringCopy(data_util_state* state, char** destination, data_string* string)
{
    int32 length = 0;
    char* buffer = NULL;
    if (destination == NULL || string == NULL)
    {
        return NE_INVAL;
    }

    length = data_string_get_tps_size(state, string) + 1 ;

    buffer = (char*)nsl_malloc(length);
    nsl_memset(buffer, 0, length);

    if (buffer == NULL) {
        return NE_NOMEM;
    }

    data_string_get_copy(state, string, buffer, length);
    *destination = buffer;
    return NE_OK;
}

static void
NB_SpecialRegion_free(data_util_state* state, NB_SpecialRegion* specialRegion)
{
    if (!state || !specialRegion)
    {
        return;
    }

    if (specialRegion->regionType)
    {
        nsl_free(specialRegion->regionType);
    }
    if (specialRegion->regionDescription)
    {
        nsl_free(specialRegion->regionDescription);
    }
    if (specialRegion->signId)
    {
        nsl_free(specialRegion->signId);
    }
    if (specialRegion->version)
    {
        nsl_free(specialRegion->version);
    }
    if (specialRegion->datasetid)
    {
        nsl_free(specialRegion->datasetid);
    }
}


NB_Error
NB_SpecialRegionInformationCreateFromDataNavReply(NB_Context* context,data_nav_reply* reply,NB_SpecialRegionInformation** information)
{
    NB_Error error = NE_OK;
    NB_SpecialRegionInformation* pThis = NULL;
    data_util_state* state = NULL;
    int32 regionLength = 0;
    int32 i = 0;

    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }
    if (!reply->vec_special_regions)
    {
        *information = NULL;
        return NE_NOENT;
    }

    pThis = (NB_SpecialRegionInformation*)nsl_malloc(sizeof(NB_SpecialRegionInformation));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_SpecialRegionInformation));
    pThis->specialRegions = CSL_VectorAlloc(sizeof(NB_SpecialRegion));
    if (!pThis->specialRegions)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    pThis->context = context;
    state = NB_ContextGetDataState(context);

    regionLength = CSL_VectorGetLength(reply->vec_special_regions);
    for (i = 0; i < regionLength; i++)
    {
        data_special_region* region = (data_special_region*)CSL_VectorGetPointer(reply->vec_special_regions, i);
        if (region)
        {
            NB_SpecialRegion nbRegion = {0};
            nbRegion.index = i;
            error = error ? error : DataStringCopy(state, &nbRegion.regionType, &region->region_type);
            error = error ? error : DataStringCopy(state, &nbRegion.regionDescription, &region->region_description);
            error = error ? error : DataStringCopy(state, &nbRegion.signId, &region->sign_id);
            error = error ? error : DataStringCopy(state, &nbRegion.version, &region->version);
            error = error ? error : DataStringCopy(state, &nbRegion.datasetid, &region->dataset_id);
            if (error != NE_OK)
            {
                break;
            }
            nbRegion.startManeuver = region->start_maneuver_index;
            nbRegion.startManeuverOffset = region->start_maneuver_offset;
            nbRegion.endManeuver = region->end_maneuver_index;
            nbRegion.endManeuverOffset = region->end_maneuver_offset;
            nbRegion.nextOccurStartTime = region->next_occurrence_start_time;
            nbRegion.nextOccurEndTime = region->next_occurrence_end_time;
            CSL_VectorAppend(pThis->specialRegions, &nbRegion);
        }
    }
    if (error != NE_OK)
    {
        DATA_VEC_FREE(state, pThis->specialRegions, NB_SpecialRegion);
        pThis->specialRegions = NULL;
    }

    *information = pThis;
    return error;
}

NB_Error
NB_SpecialRegionInformationDestroy(NB_SpecialRegionInformation* information)
{
    data_util_state* state = NULL;
    if (!information)
    {
        return NE_INVAL;
    }
    state = NB_ContextGetDataState(information->context);
    DATA_VEC_FREE(state, information->specialRegions, NB_SpecialRegion);
    nsl_free(information);
    return NE_OK;
}

/*! @} */
