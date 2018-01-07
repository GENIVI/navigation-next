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

    @file     data_want_content_types.c
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_want_content_types.h"

NB_Error
data_want_content_types_init(data_util_state* state, data_want_content_types* wantContentTypes)
{
    wantContentTypes->vec_want_content_types = CSL_VectorAlloc(sizeof(data_content_type));

    if (wantContentTypes->vec_want_content_types == NULL)
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

void
data_want_content_types_free(data_util_state* state, data_want_content_types* wantContentTypes)
{
    int n = 0;
    if (wantContentTypes->vec_want_content_types != NULL)
    {
        int length = CSL_VectorGetLength(wantContentTypes->vec_want_content_types);

        for (n = 0; n < length; ++n)
        {
            data_content_type_free(state, (data_content_type*) CSL_VectorGetPointer(wantContentTypes->vec_want_content_types, n));
        }
        CSL_VectorDealloc(wantContentTypes->vec_want_content_types);
        wantContentTypes->vec_want_content_types = NULL;

    }
}

tpselt
data_want_content_types_to_tps(data_util_state* state, data_want_content_types* wantContentTypes)
{
    int i = 0;
    tpselt ce = NULL;
    tpselt te = te_new("want-content-types");

    if (te == NULL)
    {
        goto errexit;
    }

    if (wantContentTypes->vec_want_content_types) {

        int len = CSL_VectorGetLength(wantContentTypes->vec_want_content_types);

        for (i = 0; i < len; i++) {

            if ((ce = data_content_type_to_tps(state, (data_content_type*)CSL_VectorGetPointer(wantContentTypes->vec_want_content_types, i))) != NULL && te_attach(te, ce))
            {
                ce = NULL;
            }
            else
            {
                goto errexit;
            }
        }
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_want_content_types_equal(data_util_state* state, data_want_content_types* wantContentTypes1, data_want_content_types* wantContentTypes2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(state, ret, wantContentTypes1->vec_want_content_types, wantContentTypes2->vec_want_content_types, data_content_type);

    return (boolean) ret;
}

NB_Error
data_want_content_types_copy(data_util_state* state, data_want_content_types* destinationWantContentTypes, data_want_content_types* sourceWantContentTypes)
{
    NB_Error err = NE_OK;

    DATA_VEC_COPY(state, err, destinationWantContentTypes->vec_want_content_types, sourceWantContentTypes->vec_want_content_types, data_content_type);

    return err;
}

/*! @} */
