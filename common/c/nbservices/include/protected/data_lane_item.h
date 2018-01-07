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

 @file     data_lane_item.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_LANE_ITEM_H
#define DATA_LANE_ITEM_H

#include "datautil.h"
#include "data_string.h"


struct data_lane_item_template
{

    /* Child Elements */
    uint32 item;
    /* Attributes */

};

typedef struct data_lane_item_template data_lane_item;
typedef struct data_lane_item_template data_divider_item;
typedef struct data_lane_item_template data_arrow_item;

/* lane item */
NB_Error
data_lane_item_init(data_util_state* state,
                    data_lane_item* items
                    );
void
data_lane_item_free(data_util_state* state,
                    data_lane_item* items
                    );

NB_Error
data_lane_item_from_tps(data_util_state* state,
                        data_lane_item* laneItem,
                        tpselt tpsElement
                        );

boolean
data_lane_item_equal(data_util_state* state,
                     data_lane_item* laneItem1,
                     data_lane_item* laneItem2
                     );
NB_Error
data_lane_item_copy(data_util_state* state,
                    data_lane_item* destinationLaneItem,
                    data_lane_item* sourceLaneItem
                    );
/* divider item */
NB_Error
data_divider_item_init(data_util_state* state,
                       data_divider_item* items
                       );
void
data_divider_item_free(data_util_state* state,
                       data_divider_item* items
                       );

NB_Error
data_divider_item_from_tps(data_util_state* state,
                           data_divider_item* dividerItem,
                           tpselt tpsElement
                           );

boolean
data_divider_item_equal(data_util_state* state,
                        data_divider_item* dividerItem1,
                        data_divider_item* dividerItem2
                        );
NB_Error
data_divider_item_copy(data_util_state* state,
                       data_divider_item* destinationDividerItem,
                       data_divider_item* sourceDividerItem
                       );
/* arrow item */
NB_Error
data_arrow_item_init(data_util_state* state,
                     data_arrow_item* items
                     );
void
data_arrow_item_free(data_util_state* state,
                     data_arrow_item* items
                     );

NB_Error
data_arrow_item_from_tps(data_util_state* state,
                         data_arrow_item* arrowItem,
                         tpselt tpsElement
                         );

boolean
data_arrow_item_equal(data_util_state* state,
                      data_arrow_item* arrowItem1,
                      data_arrow_item* arrowItem2
                      );
NB_Error
data_arrow_item_copy(data_util_state* state,
                     data_arrow_item* destinationArrowItem,
                     data_arrow_item* sourceArrowItem
                     );


#endif
/*! @} */