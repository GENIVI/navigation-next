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

 @file     data_lane_guidance_item.c
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

#include "data_lane_guidance_item.h"

NB_Error
data_lane_guidance_item_init(data_util_state* state,
                             data_lane_guidance_item* items
                             )
{
    NB_Error err = NE_OK;
    err = data_lane_item_init(state, &items->lane_item);
    err = ((err != NE_OK) ? err : data_divider_item_init(state, &items->divider_item));
    err = ((err != NE_OK) ? err : data_arrow_item_init(state, &items->no_highlight_arrow_item));
    err = ((err != NE_OK) ? err : data_arrow_item_init(state, &items->highlight_arrow_item));
    return err;
}

void
data_lane_guidance_item_free(data_util_state* state,
                             data_lane_guidance_item* items
                             )
{
    data_lane_item_free(state, &items->lane_item);
    data_divider_item_free(state, &items->divider_item);
    data_arrow_item_free(state, &items->no_highlight_arrow_item);
    data_arrow_item_free(state, &items->highlight_arrow_item);
}

NB_Error
data_lane_guidance_item_from_tps(data_util_state* state,
                                 data_lane_guidance_item* laneGuidanceItem,
                                 tpselt tpsElement
                                 )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    int arrowItemCount = 0;
    DATA_REINIT(state, err, laneGuidanceItem, data_lane_guidance_item);
    if (err != NE_OK)
    {
        return err;
    }
    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "lane-item") == 0)
        {
            err = data_lane_item_from_tps(state, &laneGuidanceItem->lane_item, ce);
            if (err != NE_OK)
            {
                break;
            }
        }
        else if (nsl_strcmp(ceName, "divider-item") == 0)
        {
            err = data_divider_item_from_tps(state, &laneGuidanceItem->divider_item, ce);
            if (err != NE_OK)
            {
                break;
            }
        }
        else if (nsl_strcmp(ceName, "arrow-item") == 0)
        {
            if (arrowItemCount == 0)
            {
                err  = data_arrow_item_from_tps(state, &laneGuidanceItem->no_highlight_arrow_item, ce);
                if (err != NE_OK)
                {
                    break;
                }
            }
            else if (arrowItemCount == 1)
            {
                err  = data_arrow_item_from_tps(state, &laneGuidanceItem->highlight_arrow_item, ce);
                if (err != NE_OK)
                {
                    break;
                }
            }
            arrowItemCount ++;
        }

    }

    return err;
}

boolean
data_lane_guidance_item_equal(data_util_state* state,
                              data_lane_guidance_item* laneGuidanceItem1,
                              data_lane_guidance_item* laneGuidanceItem2
                              )
{
    int ret = TRUE;
    ret = data_lane_item_equal(state, &laneGuidanceItem1->lane_item, &laneGuidanceItem2->lane_item);
    ret = (!ret ? ret : data_divider_item_equal(state, &laneGuidanceItem1->divider_item, &laneGuidanceItem2->divider_item));
    ret = (!ret ? ret : data_arrow_item_equal(state, &laneGuidanceItem1->no_highlight_arrow_item, &laneGuidanceItem2->no_highlight_arrow_item));
    ret = (!ret ? ret : data_arrow_item_equal(state, &laneGuidanceItem1->highlight_arrow_item, &laneGuidanceItem2->highlight_arrow_item));
    return  (boolean)ret;
}
NB_Error
data_lane_guidance_item_copy(data_util_state* state,
                             data_lane_guidance_item* destinationLaneGuidanceItem,
                             data_lane_guidance_item* sourceLaneGuidanceItem
                             )
{
    NB_Error err = NE_OK;
    DATA_REINIT(state, err, destinationLaneGuidanceItem, data_lane_guidance_item);
    err = ((err != NE_OK) ? err : data_lane_item_copy(state, &destinationLaneGuidanceItem->lane_item, &sourceLaneGuidanceItem->lane_item));
    err = ((err != NE_OK) ? err : data_divider_item_copy(state, &destinationLaneGuidanceItem->divider_item, &sourceLaneGuidanceItem->divider_item));
    err = ((err != NE_OK) ? err : data_arrow_item_copy(state, &destinationLaneGuidanceItem->no_highlight_arrow_item, &sourceLaneGuidanceItem->no_highlight_arrow_item));
    err = ((err != NE_OK) ? err : data_arrow_item_copy(state, &destinationLaneGuidanceItem->highlight_arrow_item, &sourceLaneGuidanceItem->highlight_arrow_item));
    return err;
}
