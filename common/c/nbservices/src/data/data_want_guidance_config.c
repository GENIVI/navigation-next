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

 @file     data_want_guidance_config.c
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

#include "data_want_guidance_config.h"


static boolean AttachSimpleWantElement(boolean attach, const char* name, tpselt parent);


NB_Error
data_want_guidance_config_init(data_util_state* state,
                    data_want_guidance_config* item
                    )
{
    NB_Error err = NE_OK;

    item->want_enchanced_natural_guidance = FALSE;
    item->want_lane_guidance = FALSE;
    item->want_natural_guidance = FALSE;
    item->want_play_times = FALSE;

    return err;
}

void
data_want_guidance_config_free(data_util_state* state,
                    data_want_guidance_config* items
                    )
{
    return ;
}

NB_Error
data_want_guidance_config_from_tps(data_util_state* state,
                        data_want_guidance_config* wantGuidanceConfig,
                        tpselt tpsElement
                        )
{
    tpselt ce = NULL;
    int iter = 0;
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, wantGuidanceConfig, data_want_guidance_config);

    if (err != NE_OK)
    {
        return err;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        const char * ceName = te_getname(ce);
        if (nsl_strcmp(ceName, "want-enhanced-natural-guidance") == 0)
        {
            wantGuidanceConfig->want_enchanced_natural_guidance = TRUE;
        }
        else if (nsl_strcmp(ceName, "want-natural-guidance") == 0)
        {
            wantGuidanceConfig->want_natural_guidance = TRUE;
        }
        else if (nsl_strcmp(ceName, "want-lane-guidance") == 0)
        {
            wantGuidanceConfig->want_lane_guidance = TRUE;
        }
        else if (nsl_strcmp(ceName, "want-play-times") == 0)
        {
            wantGuidanceConfig->want_play_times = TRUE;
        }
    }

    if (err != NE_OK)
    {
        data_want_guidance_config_free(state, wantGuidanceConfig);
    }

    return err;
}

boolean
data_want_guidance_config_equal(data_util_state* state,
                     data_want_guidance_config* wantGuidanceConfig1,
                     data_want_guidance_config* wantGuidanceConfig2
                     )
{
    return (boolean)((wantGuidanceConfig1->want_play_times == wantGuidanceConfig2->want_play_times) &&
                     (wantGuidanceConfig1->want_natural_guidance == wantGuidanceConfig2->want_natural_guidance) &&
                     (wantGuidanceConfig1->want_lane_guidance == wantGuidanceConfig2->want_lane_guidance) &&
                     (wantGuidanceConfig1->want_enchanced_natural_guidance == wantGuidanceConfig2->want_enchanced_natural_guidance));
}

NB_Error
data_want_guidance_config_copy(data_util_state* state,
                    data_want_guidance_config* destinationWantGuidanceConfig,
                    data_want_guidance_config* sourceWantGuidanceConfig
                    )
{
    destinationWantGuidanceConfig->want_enchanced_natural_guidance = sourceWantGuidanceConfig->want_enchanced_natural_guidance;
    destinationWantGuidanceConfig->want_lane_guidance = sourceWantGuidanceConfig->want_lane_guidance;
    destinationWantGuidanceConfig->want_natural_guidance = sourceWantGuidanceConfig->want_natural_guidance;
    destinationWantGuidanceConfig->want_play_times = sourceWantGuidanceConfig->want_play_times;

    return NE_OK;

}

tpselt
data_want_guidance_config_to_tps(data_util_state* state, data_want_guidance_config* wantGuidanceConfig)
{
    tpselt te;

    te = te_new("want-guidance-config");

    if (te == NULL)
        goto errexit;

    if (!AttachSimpleWantElement(wantGuidanceConfig->want_enchanced_natural_guidance, "want-enhanced-natural-guidance", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(wantGuidanceConfig->want_natural_guidance, "want-natural-guidance", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(wantGuidanceConfig->want_lane_guidance, "want-lane-guidance", te))
    {
        goto errexit;
    }

    return te;

errexit:
    if (te)
    {
        te_dealloc(te);
    }
    return NULL;
}

boolean AttachSimpleWantElement(boolean attach, const char* name, tpselt parent)
{
    if (attach)
    {
        tpselt child = te_new(name);
        if (!child || !te_attach(parent, child))
        {
            return FALSE;
        }
    }

    return TRUE;
}
