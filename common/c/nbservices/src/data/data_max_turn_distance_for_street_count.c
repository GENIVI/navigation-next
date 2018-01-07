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

    @file     data_max_turn_distance_for_street_count.c
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems, Inc is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/


/*! @{ */

#include "data_max_turn_distance_for_street_count.h"

NB_Error
data_max_turn_distance_for_street_count_init(data_util_state* pds, data_max_turn_distance_for_street_count* pmtd)
{
    pmtd->max_turn_distance = 0.0;

    return NE_OK;
}

void
data_max_turn_distance_for_street_count_free(data_util_state* pds, data_max_turn_distance_for_street_count* pmtd)
{
    pmtd->max_turn_distance = 0.0;

    return;
}

NB_Error
data_max_turn_distance_for_street_count_from_tps(data_util_state* pds, data_max_turn_distance_for_street_count* pmtd, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }
    DATA_REINIT(pds, err, pmtd, data_max_turn_distance_for_street_count);

    if (!te_getattrf(te, "max-turn-distance", &pmtd->max_turn_distance))
    {
        err = NE_BADDATA;
    }

    if (err != NE_OK)
    {
        data_max_turn_distance_for_street_count_free(pds, pmtd);
    }

    return err;
}

boolean
data_max_turn_distance_for_street_count_equal(data_util_state* pds, data_max_turn_distance_for_street_count* pmtd1, data_max_turn_distance_for_street_count* pmtd2)
{
    return (boolean) (pmtd1->max_turn_distance == pmtd2->max_turn_distance);
}

NB_Error
data_max_turn_distance_for_street_count_copy(data_util_state* pds, data_max_turn_distance_for_street_count* pmtd_dest, data_max_turn_distance_for_street_count* pmtd_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pmtd_dest, data_max_turn_distance_for_street_count);
    pmtd_dest->max_turn_distance = pmtd_src->max_turn_distance;

    return err;
}

/*! @} */
