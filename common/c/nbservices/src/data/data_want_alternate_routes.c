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

 @file     data_want_alternate_routes.c
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

#include "data_want_alternate_routes.h"

NB_Error
data_want_alternate_routes_init(data_util_state* state,
                                data_want_alternate_routes* item
                                )
{
    NB_Error err = NE_OK;

    item->max_routes = 0;

    return err;
}

void
data_want_alternate_routes_free(data_util_state* state,
                    data_want_alternate_routes* items
                    )
{
}

NB_Error
data_want_alternate_routes_from_tps(data_util_state* state,
                                    data_want_alternate_routes* wantAlternateRoutes,
                                    tpselt tpsElement
                                    )
{
    tpselt ce = NULL;
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, wantAlternateRoutes, data_want_alternate_routes);

    if (err != NE_OK)
    {
        return err;
    }

    wantAlternateRoutes->max_routes = te_getattru(ce, "max-routes");

    return err;
}

boolean
data_want_alternate_routes_equal(data_util_state* state,
                                 data_want_alternate_routes* wantAlternateRoutes1,
                                 data_want_alternate_routes* wantAlternateRoutes2
                                 )
{
    return (boolean)(wantAlternateRoutes1->max_routes == wantAlternateRoutes2->max_routes);
}

NB_Error
data_want_alternate_routes_copy(data_util_state* state,
                                data_want_alternate_routes* destinationWantAlternateRoutes,
                                data_want_alternate_routes* sourceWantAlternateRoutes
                                )
{
    destinationWantAlternateRoutes->max_routes = sourceWantAlternateRoutes->max_routes;
    return NE_OK;
}

tpselt
data_want_alternate_routes_to_tps(data_util_state* state, data_want_alternate_routes* items)
{
    tpselt te;

    te = te_new("want-alternate-routes");

    if (te == NULL)
        goto errexit;


    te_setattru(te, "max-routes",  items->max_routes);

    return te;

errexit:
    if (te)
    {
        te_dealloc(te);
    }
    return NULL;
}
