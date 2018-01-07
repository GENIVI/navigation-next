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

 @file     data_label_point.c
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

#include "data_label_point.h"

NB_Error
data_label_point_init(data_util_state* state,
                      data_label_point* items
                      )
{
    NB_Error err = NE_OK;

    err = data_point_init(state, &items->point);

    if (err != NE_OK)
    {
        data_label_point_free(state, items);
    }

    return err;
}

void
data_label_point_free(data_util_state* state,
                         data_label_point* items
                         )
{
    data_point_free(state, &items->point);
}

NB_Error
data_label_point_from_tps(data_util_state* state,
                             data_label_point* labelPoint,
                             tpselt tpsElement
                             )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, labelPoint, data_label_point);

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "point") == 0)
        {
            err = data_point_from_tps(state, &labelPoint->point, ce);

            if (err != NE_OK)
            {
                goto exitError;
            }
        }
    }

exitError:
    if (err != NE_OK)
    {
        data_label_point_free(state, labelPoint);
    }
    return err;
}

boolean
data_label_point_equal(data_util_state* state,
                          data_label_point* labelPoint1,
                          data_label_point* labelPoint2
                          )
{
    int ret = TRUE;

    DATA_EQUAL(state, ret, &labelPoint1->point, &labelPoint2->point, data_point);

    return  (boolean)ret;
}

NB_Error
data_label_point_copy(data_util_state* state,
                      data_label_point* destinationLabelPoint,
                      data_label_point* sourceLabelPoint
                      )
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationLabelPoint, data_label_point);

    DATA_COPY(state, err, &destinationLabelPoint->point, &sourceLabelPoint->point, data_point);

    return err;
}
