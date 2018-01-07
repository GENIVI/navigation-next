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

 @file     data_play_time.c
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

#include "data_play_time.h"

NB_Error
data_play_time_init(data_util_state* state,
                                data_play_time* item
                                )
{
    NB_Error err = NE_OK;

    item->play_time = 0;

    return err;
}

void
data_play_time_free(data_util_state* state,
                    data_play_time* items
                    )
{
}

NB_Error
data_play_time_from_tps(data_util_state* state,
                        data_play_time* playTime,
                        tpselt tpsElement
                        )
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, playTime, data_play_time);

    if (err != NE_OK)
    {
        return err;
    }

    playTime->play_time = te_getattru(tpsElement, "time");

    return err;
}

boolean
data_play_time_equal(data_util_state* state,
                                 data_play_time* playTime1,
                                 data_play_time* playTime2
                                 )
{
    return (boolean)(playTime1->play_time == playTime2->play_time);
}

NB_Error
data_play_time_copy(data_util_state* state,
                                data_play_time* destinationPlayTime,
                                data_play_time* sourcePlayTime
                                )
{
    destinationPlayTime->play_time = sourcePlayTime->play_time;
    return NE_OK;
}

tpselt
data_play_time_to_tps(data_util_state* state, data_play_time* items)
{
    tpselt te;

    te = te_new("play-time");

    if (te == NULL)
        goto errexit;

    te_setattru(te, "time", items->play_time);

    return te;

errexit:
    if (te)
    {
        te_dealloc(te);
    }
    return NULL;
}
