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

 @file     data_lane_info.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, Inc is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/
#include "cslutil.h"
#include "data_lane_info.h"
#include "data_lane_guidance_item.h"

NB_Error
data_lane_info_init(data_util_state* state,
                    data_lane_info* item
                    )
{
    NB_Error err = NE_OK;
    item->number_of_lanes = 0;
    item->lane_position   = 0;
    item->vec_lane_guidance_items = CSL_VectorAlloc(sizeof(data_lane_guidance_item));

    if (item->vec_lane_guidance_items == NULL)
    {
        err = NE_NOMEM;
    }

    err = ((err != NE_OK) ? err : data_blob_init(state, &item->lg_pronun));
    err = ((err != NE_OK) ? err : data_blob_init(state, &item->lgprep_pronun));

    if (err != NE_OK)
    {
        data_lane_info_free(state, item);
    }
    return err;
}

void
data_lane_info_free(data_util_state* state,
                    data_lane_info* items
                    )
{
    uint32 length = 0;
    uint32 n = 0;
    if (items->vec_lane_guidance_items)
    {
        length = CSL_VectorGetLength(items->vec_lane_guidance_items);
        for (n = 0; n < length ; n ++)
        {
            data_lane_guidance_item_free(state, CSL_VectorGetPointer(items->vec_lane_guidance_items, n));
        }
        CSL_VectorDealloc(items->vec_lane_guidance_items);
        items->vec_lane_guidance_items = NULL;
    }
    data_blob_free(state, &items->lgprep_pronun);
    data_blob_free(state, &items->lg_pronun);

    return ;
}

NB_Error
data_lane_info_from_tps(data_util_state* state,
                        data_lane_info* laneInfo,
                        tpselt tpsElement
                        )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    data_lane_guidance_item laneGuidanceItem;

    nsl_memset(&laneGuidanceItem, 0, sizeof(laneGuidanceItem));
    DATA_REINIT(state, err, laneInfo, data_lane_info);

    if (err != NE_OK)
    {
        return err;
    }

    laneInfo->number_of_lanes = te_getattru(tpsElement, "number-of-lanes");
    laneInfo->lane_position   = te_getattru(tpsElement, "lane-position");
    err = data_blob_from_tps_attr(state, &laneInfo->lg_pronun, tpsElement, "lg-pronun");
    err = ((err != NE_OK) ? err : data_blob_from_tps_attr(state, &laneInfo->lgprep_pronun, tpsElement, "lgprep-pronun"));

    if (err != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "lane-guidance-item") == 0)
        {
            err = data_lane_guidance_item_from_tps(state, &laneGuidanceItem, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(laneInfo->vec_lane_guidance_items, &laneGuidanceItem))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&laneGuidanceItem, 0, sizeof(laneGuidanceItem));
            }
            else
            {
                data_lane_guidance_item_free(state, &laneGuidanceItem);
            }
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_lane_info_free(state, laneInfo);
    }

    return err;
}

boolean
data_lane_info_equal(data_util_state* state,
                     data_lane_info* laneInfo1,
                     data_lane_info* laneInfo2
                     )
{
    int result = TRUE;

    result = laneInfo1->number_of_lanes == laneInfo2->number_of_lanes;
    result = !result ? result : laneInfo1->lane_position == laneInfo2->lane_position;
    DATA_EQUAL(state, result, &laneInfo1->lg_pronun, &laneInfo2->lg_pronun, data_blob);
    DATA_EQUAL(state, result, &laneInfo1->lgprep_pronun, &laneInfo2->lgprep_pronun, data_blob);
    if (result)
    {
        DATA_VEC_EQUAL(state, result, laneInfo1->vec_lane_guidance_items, laneInfo2->vec_lane_guidance_items, data_lane_guidance_item );
    }

    return (boolean)result;
}

NB_Error
data_lane_info_copy(data_util_state* state,
                    data_lane_info* destinationLaneInfo,
                    data_lane_info* sourceLaneInfo
                    )
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationLaneInfo, data_lane_info);

    DATA_VEC_COPY(state, err, destinationLaneInfo->vec_lane_guidance_items, sourceLaneInfo->vec_lane_guidance_items, data_lane_guidance_item);

    if (err == NE_OK)
    {
        destinationLaneInfo->number_of_lanes = sourceLaneInfo->number_of_lanes;
        destinationLaneInfo->number_of_lanes = sourceLaneInfo->number_of_lanes;
        DATA_COPY(state, err, &destinationLaneInfo->lgprep_pronun, &sourceLaneInfo->lgprep_pronun, data_blob);
        DATA_COPY(state, err, &destinationLaneInfo->lg_pronun, &sourceLaneInfo->lg_pronun, data_blob);
    }

    return err;

}

