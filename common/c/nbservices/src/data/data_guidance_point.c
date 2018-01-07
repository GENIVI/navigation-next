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

 @file     data_guidance_point.c
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

#include "data_guidance_point.h"

NB_Error
data_guidance_point_init(data_util_state* state,
                         data_guidance_point* items
                         )
{
    NB_Error err = NE_OK;
    err = data_point_init(state, &items->point);
    items->use_for_prepare  = FALSE;
    items->use_for_turn     = FALSE;

    err = ((err != NE_OK) ? err : data_string_init(state, &items->description));
    err = ((err != NE_OK) ? err : data_blob_init(state, &items->gp_pronun));
    err = ((err != NE_OK) ? err : data_blob_init(state, &items->gpprep_pronun));
    items->maneuver_point_offset = 0.0f;
    items->gptype = 0;

    if (err != NE_OK)
    {
        data_guidance_point_free(state, items);
    }
    return err;
}

void
data_guidance_point_free(data_util_state* state,
                         data_guidance_point* items
                         )
{
    data_blob_free(state, &items->gp_pronun);
    data_point_free(state, &items->point);
    data_string_free(state, &items->description);
    data_blob_free(state, &items->gpprep_pronun);
}

NB_Error
data_guidance_point_from_tps(data_util_state* state,
                             data_guidance_point* guidancePoint,
                             tpselt tpsElement
                             )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;
    double doubleValue = 0.0f;
    DATA_REINIT(state, err, guidancePoint, data_guidance_point);
    guidancePoint->gptype = te_getattru(tpsElement, "gp-type");
    err = ((err != NE_OK) ? err : data_string_from_tps_attr(state, &guidancePoint->description, tpsElement, "description"));
    err = ((err != NE_OK) ? err : data_blob_from_tps_attr(state, &guidancePoint->gpprep_pronun, tpsElement, "gpprep-pronun"));
    err = ((err != NE_OK) ? err : data_blob_from_tps_attr(state, &guidancePoint->gp_pronun, tpsElement, "gp-pronun"));

    if (err != NE_OK)
    {
        goto exitError;
    }

    te_getattrf(tpsElement, "maneuver-point-offset", &doubleValue);
    guidancePoint->maneuver_point_offset = (float)doubleValue;

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "point") == 0)
        {
            err = data_point_from_tps(state, &guidancePoint->point, ce);

            if (err != NE_OK)
            {
                goto exitError;
            }
        }
        else if( nsl_strcmp(ceName,"use-for-prepare") == 0)
        {
            guidancePoint->use_for_prepare = TRUE;
        }
        else if (nsl_strcmp(ceName,"use-for-turn") == 0)
        {
            guidancePoint->use_for_turn = TRUE;
        }
    }

exitError:
    if (err != NE_OK)
    {
        data_guidance_point_free(state, guidancePoint);
    }
    return err;
}

boolean
data_guidance_point_equal(data_util_state* state,
                          data_guidance_point* guidancePoint1,
                          data_guidance_point* guidancePoint2
                          )
{
    int ret = TRUE;
    ret = guidancePoint1->gptype == guidancePoint2->gptype;
    DATA_EQUAL(state, ret, &guidancePoint1->point, &guidancePoint2->point, data_point);
    DATA_EQUAL(state, ret, &guidancePoint1->gp_pronun, &guidancePoint2->gp_pronun, data_blob);
    DATA_EQUAL(state, ret, &guidancePoint1->description, &guidancePoint2->description, data_string);
    DATA_EQUAL(state, ret, &guidancePoint1->gpprep_pronun, &guidancePoint2->gpprep_pronun, data_blob);
    ret =!ret ? ret : guidancePoint1->use_for_prepare == guidancePoint2->use_for_prepare;
    ret =!ret ? ret : guidancePoint1->use_for_turn == guidancePoint2->use_for_turn;
    ret =!ret ? ret : guidancePoint1->maneuver_point_offset == guidancePoint2->maneuver_point_offset;
    return  (boolean)ret;
}

NB_Error
data_guidance_point_copy(data_util_state* state,
                         data_guidance_point* destinationGuidancePoint,
                         data_guidance_point* sourceGuidancePoint
                         )
{
    NB_Error err = NE_OK;
    DATA_REINIT(state, err, destinationGuidancePoint, data_guidance_point);
    DATA_COPY(state, err, &destinationGuidancePoint->point, &sourceGuidancePoint->point, data_point);
    DATA_COPY(state, err, &destinationGuidancePoint->gp_pronun, &sourceGuidancePoint->gp_pronun, data_blob);
    DATA_COPY(state, err, &destinationGuidancePoint->description, &sourceGuidancePoint->description, data_string);
    DATA_COPY(state, err, &destinationGuidancePoint->gpprep_pronun, &sourceGuidancePoint->gpprep_pronun, data_blob);
    if (err == NE_OK)
    {
        destinationGuidancePoint->use_for_prepare = sourceGuidancePoint->use_for_prepare;
        destinationGuidancePoint->use_for_turn = sourceGuidancePoint->use_for_turn;
        destinationGuidancePoint->maneuver_point_offset = sourceGuidancePoint->maneuver_point_offset;
        destinationGuidancePoint->gptype = sourceGuidancePoint->gptype;
    }

    return err;
}
