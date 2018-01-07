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

 @file     data_speed_limits_data.c
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

#include "data_speed_limits_data.h"
#include "data_speed_region.h"

NB_Error data_speed_limits_data_init(data_util_state* state, data_speed_limits_data* speedLimitsData)
{
    NB_Error error = NE_OK;
    if (state == NULL || speedLimitsData == NULL)
    {
        return NE_INVAL;
    }
    speedLimitsData->speed_regions = CSL_VectorAlloc(sizeof(data_speed_region));

    if (speedLimitsData->speed_regions == NULL)
    {
        error = NE_NOMEM;
    }
    return error;
}

void data_speed_limits_data_free(data_util_state* state, data_speed_limits_data* speedLimitsData)
{
    int n=0, length = 0;
    if (state == NULL || speedLimitsData == NULL)
    {
        return;
    }
    if (speedLimitsData->speed_regions)
    {
		length = CSL_VectorGetLength(speedLimitsData->speed_regions);

		for (n=0;n<length;n++)
		{
            data_speed_region_free(state, (data_speed_region*) CSL_VectorGetPointer(speedLimitsData->speed_regions, n));
        }
		CSL_VectorDealloc(speedLimitsData->speed_regions);
        speedLimitsData->speed_regions = NULL;
	}

}

NB_Error data_speed_limits_data_from_tps(data_util_state* state, data_speed_limits_data* speedLimitsData, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    tpselt ce = NULL;
    int iter = 0;
    data_speed_region speedRegion = {0};

    if (state == NULL || speedLimitsData == NULL)
    {
        return NE_INVAL;
    }

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&speedRegion, 0, sizeof(speedRegion));

    DATA_REINIT(state, error, speedLimitsData, data_speed_limits_data);

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "speed-region") == 0)
        {
            error = data_speed_region_from_tps(state, &speedRegion, ce);

            if (error != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(speedLimitsData->speed_regions, &speedRegion))
            {
                error = NE_NOMEM;
            }

            if (error == NE_OK)
            {
                nsl_memset(&speedRegion, 0, sizeof(speedRegion));
            }
            else
            {
                data_speed_region_free(state, &speedRegion);
            }
        }

        if (error != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (error != NE_OK)
    {
        data_speed_limits_data_free(state, speedLimitsData);
    }

    return error;
}
NB_Error data_speed_limits_data_copy(data_util_state* state,
                                        data_speed_limits_data* destinationSpeedLimitsData,
                                        data_speed_limits_data* sourceSpeedLimitsData)
{
    NB_Error error = NE_OK;
    if (destinationSpeedLimitsData == NULL || sourceSpeedLimitsData == NULL || state == NULL)
    {
        return NE_INVAL;
    }
    DATA_VEC_COPY(state, error, destinationSpeedLimitsData->speed_regions, sourceSpeedLimitsData->speed_regions, data_speed_region);
    return error;
}


/*! @} */
