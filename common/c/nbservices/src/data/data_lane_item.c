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

 @file     data_lane_item.c
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

#include "data_lane_item.h"
#include "nbnaturallaneguidancetype.h"
NB_Error
data_lane_item_init(data_util_state* state,
                    data_lane_item* item
                    )
{
    item->item = 0;
    return NE_OK;

}
void
data_lane_item_free(data_util_state* state,
                    data_lane_item* items
                    )
{
    return ;
}

NB_Error
data_lane_item_from_tps(data_util_state* state,
                        data_lane_item* laneItem,
                        tpselt tpsElement
                        )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    laneItem->item = 0;
    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "regular-lane") == 0)
        {
            laneItem->item |= LT_REGULAR;
        }
        else if (nsl_strcmp(ceName, "hov-lane") == 0)
        {
            laneItem->item |= LT_HOV;
        }
        else if (nsl_strcmp(ceName, "reversible-lane") == 0)
        {
            laneItem->item |= LT_REVERSIBLE;
        }
        else if (nsl_strcmp(ceName, "express-lane") == 0)
        {
            laneItem->item |= LT_EXPRESS;
        }
        else if (nsl_strcmp(ceName, "acceleration-lane") == 0)
        {
            laneItem->item |= LT_ACCEL;
        }
        else if (nsl_strcmp(ceName, "deceleration-lane") == 0)
        {
            laneItem->item |= LT_DECEL;
        }
        else if (nsl_strcmp(ceName, "auxiliary-lane") == 0)
        {
            laneItem->item |= LT_AUX;
        }
        else if (nsl_strcmp(ceName, "slow-lane") == 0)
        {
            laneItem->item |= LT_SLOW;
        }
        else if (nsl_strcmp(ceName, "passing-lane") == 0)
        {
            laneItem->item |= LT_PASSING;
        }
        else if (nsl_strcmp(ceName, "drivable-shoulder-lane") == 0)
        {
            laneItem->item |= LT_SHOULDER;
        }
        else if (nsl_strcmp(ceName, "regulated-access-lane") == 0)
        {
            laneItem->item |= LT_REGULAR;
        }
        else if (nsl_strcmp(ceName, "turn-lane") == 0)
        {
            laneItem->item |= LT_TURN;
        }
        else if (nsl_strcmp(ceName, "center-turn-lane") == 0)
        {
            laneItem->item |= LT_CENTER_TURN;
        }
        else if (nsl_strcmp(ceName, "truck-parking-lane") == 0)
        {
            laneItem->item |= LT_TRUCK_PARK;
        }
    }

    return err;
}

boolean
data_lane_item_equal(data_util_state* state,
                     data_lane_item* laneItem1,
                     data_lane_item* laneItem2
                     )
{
    return (boolean)(laneItem1->item == laneItem2->item);
}

NB_Error
data_lane_item_copy(data_util_state* state,
                    data_lane_item* destinationLaneItem,
                    data_lane_item* sourceLaneItem
                    )
{
    destinationLaneItem->item = sourceLaneItem -> item;
    return NE_OK;
}

/* divider item */
NB_Error
data_divider_item_init(data_util_state* state,
                       data_divider_item* items
                       )
{
    items->item = 0;
    return NE_OK;
}

void
data_divider_item_free(data_util_state* state,
                       data_divider_item* items
                       )
{
    return;
}

NB_Error
data_divider_item_from_tps(data_util_state* state,
                           data_divider_item* dividerItem,
                           tpselt tpsElement
                           )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    dividerItem->item = 0;
    if((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "long-dash-divider") == 0)
        {
            dividerItem->item = LDT_LONG_DASHED;
        }
        else if (nsl_strcmp(ceName, "double-solid-divider") == 0)
        {
            dividerItem->item = LDT_DOUBLE_SOLID;
        }
        else if (nsl_strcmp(ceName, "single-solid-divider") == 0)
        {
            dividerItem->item = LDT_SINGLE_SOLID;
        }
        else if (nsl_strcmp(ceName, "solid-dash-divider") == 0)
        {
            dividerItem->item = LDT_SOLID_DASHED;
        }
        else if (nsl_strcmp(ceName, "short-dash-divider") == 0)
        {
            dividerItem->item = LDT_SHORT_DASHED;
        }
        else if (nsl_strcmp(ceName, "shaded-area-divider") == 0)
        {
            dividerItem->item = LDT_SHADED_AREA;
        }
        else if (nsl_strcmp(ceName, "block-dash-divider") == 0)
        {
            dividerItem->item = LDT_DASHED_BLOCKS;
        }
        else if (nsl_strcmp(ceName, "physical-divider") == 0)
        {
            dividerItem->item = LDT_PHYS_DIVIDER;
        }
        else if (nsl_strcmp(ceName, "double-dash-divider") == 0)
        {
            dividerItem->item = LDT_DOUBLE_DASHED;
        }
        else if (nsl_strcmp(ceName, "no-divider") == 0)
        {
            dividerItem->item = LDT_NONE;
        }
        else if (nsl_strcmp(ceName, "crossing-alert-divider") == 0)
        {
            dividerItem->item = LDT_XING_ALERT;
        }
        else if (nsl_strcmp(ceName, "center-turn-line-divider") == 0)
        {
            dividerItem->item = LDT_CTR_TURN_LANE;
        }
    }
    return err;

}

boolean
data_divider_item_equal(data_util_state* state,
                        data_divider_item* dividerItem1,
                        data_divider_item* dividerItem2
                        )
{
    return (boolean)(dividerItem1->item == dividerItem2->item);
}

NB_Error
data_divider_item_copy(data_util_state* state,
                       data_divider_item* destinationDividerItem,
                       data_divider_item* sourceDividerItem
                       )
{
    destinationDividerItem->item = sourceDividerItem->item;
    return NE_OK;
}



/* arrow item */
NB_Error
data_arrow_item_init(data_util_state* state,
                     data_arrow_item* items
                     )
{
    items->item = 0;
    return NE_OK;
}
void
data_arrow_item_free(data_util_state* state,
                     data_arrow_item* items
                     )
{
    return;
}

NB_Error
data_arrow_item_from_tps(data_util_state* state,
                         data_arrow_item* arrowItem,
                         tpselt tpsElement
                         )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    arrowItem->item = 0;
    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "straight-arrow") == 0)
        {
            arrowItem->item |= AT_STRAIGHT;
        }
        else if (nsl_strcmp(ceName, "slight-right-arrow") == 0)
        {
            arrowItem->item |= AT_SLIGHT_RIGHT;
        }
        else if (nsl_strcmp(ceName, "right-arrow") == 0)
        {
            arrowItem->item |= AT_RIGHT;
        }
        else if (nsl_strcmp(ceName, "hard-right-arrow") == 0)
        {
            arrowItem->item |= AT_HARD_RIGHT;
        }
        else if (nsl_strcmp(ceName, "u-turn-left-arrow") == 0)
        {
            arrowItem->item |= AT_UTURN_LEFT;
        }
        else if (nsl_strcmp(ceName, "hard-left-arrow") == 0)
        {
            arrowItem->item |= AT_HARD_LEFT;
        }
        else if (nsl_strcmp(ceName, "left-arrow") == 0)
        {
            arrowItem->item |= AT_LEFT;
        }
        else if (nsl_strcmp(ceName, "slight-left-arrow") == 0)
        {
            arrowItem->item |= AT_SLIGHT_LEFT;
        }
        else if (nsl_strcmp(ceName, "merge-right-arrow") == 0)
        {
            arrowItem->item |= AT_MERGE_RIGHT;
        }
        else if (nsl_strcmp(ceName, "merge-left-arrow") == 0)
        {
            arrowItem->item |= AT_MERGE_LEFT;
        }
        else if (nsl_strcmp(ceName, "merge-lane-arrow") == 0)
        {
            arrowItem->item |= AT_MERGE_LANES;
        }
        else if (nsl_strcmp(ceName, "u-turn-right-arrow") == 0)
        {
            arrowItem->item |= AT_UTURN_RIGHT;
        }
        else if (nsl_strcmp(ceName, "second-right-arrow") == 0)
        {
            arrowItem->item |= AT_SECOND_RIGHT;
        }
        else if (nsl_strcmp(ceName, "second-left-arrow") == 0)
        {
            arrowItem->item |= AT_SECOND_LEFT;
        }
    }

    return err;
}

boolean
data_arrow_item_equal(data_util_state* state,
                      data_arrow_item* arrowItem1,
                      data_arrow_item* arrowItem2
                      )
{
    return (boolean)(arrowItem1->item == arrowItem2->item);
}
NB_Error
data_arrow_item_copy(data_util_state* state,
                     data_arrow_item* destinationArrowItem,
                     data_arrow_item* sourceArrowItem
                     )
{
    destinationArrowItem->item = sourceArrowItem->item;
    return NE_OK;
}