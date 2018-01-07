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

    @file     data_overall_rating.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_overall_rating.h"

NB_DEF NB_Error
data_overall_rating_init(data_util_state* pds, data_overall_rating* por)
{
    por->average_rating = 0.0;
    por->rating_count = 0;

    return NE_OK;
}

NB_DEF void
data_overall_rating_free(data_util_state* pds, data_overall_rating* por)
{
    por->average_rating = 0.0;
    por->rating_count = 0;

    return;
}

NB_DEF NB_Error
data_overall_rating_from_tps(data_util_state* pds, data_overall_rating* por, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }

    data_overall_rating_free(pds, por);

    err = data_overall_rating_init(pds, por);

    if (err != NE_OK)
    {
        return err;
    }

    por->rating_count = te_getattru(te, "rating-count");

    if (!te_getattrd(te, "average-rating", &por->average_rating))
    {
        err = NE_BADDATA;
    }

    if (err != NE_OK)
    {
        data_overall_rating_free(pds, por);
    }

    return err;
}

NB_DEF boolean
data_overall_rating_equal(data_util_state* pds, data_overall_rating* por1, data_overall_rating* por2)
{
    return (boolean) ((por1->average_rating == por2->average_rating) && 
                        (por1->rating_count == por2->rating_count));
}

NB_DEF NB_Error
data_overall_rating_copy(data_util_state* pds, data_overall_rating* por_dest, data_overall_rating* por_src)
{
    NB_Error err = NE_OK;

    data_overall_rating_free(pds, por_dest);

    err = data_overall_rating_init(pds, por_dest);

    por_dest->average_rating = por_src->average_rating;
    por_dest->rating_count = por_src->rating_count;

    return err;
}

/*! @} */
