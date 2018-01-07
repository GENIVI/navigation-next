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

    @file     data_wanted_content.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_wanted_content.h"

NB_Error
data_wanted_content_init(data_util_state* state, data_wanted_content* wantedContent)
{
    data_string_init(state, &wantedContent->country);
    data_string_init(state, &wantedContent->dataset_id);
    data_string_init(state, &wantedContent->type);

    return NE_OK;
}

void
data_wanted_content_free(data_util_state* state, data_wanted_content* wantedContent)
{
    data_string_free(state, &wantedContent->country);
    data_string_free(state, &wantedContent->dataset_id);
    data_string_free(state, &wantedContent->type);
}

tpselt
data_wanted_content_to_tps(data_util_state* state, data_wanted_content* wantedContent)
{
    tpselt te = NULL;

    te = te_new("wanted-content");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "country", data_string_get(state, &wantedContent->country)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "dataset-id", data_string_get(state, &wantedContent->dataset_id)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "type", data_string_get(state, &wantedContent->type)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_wanted_content_equal(data_util_state* state, data_wanted_content* wantedContent1, data_wanted_content* wantedContent2)
{
    return (data_string_equal(state, &wantedContent1->country, &wantedContent2->country) &&
            data_string_equal(state, &wantedContent1->dataset_id, &wantedContent2->dataset_id) &&
            data_string_equal(state, &wantedContent1->type, &wantedContent2->type));
}

NB_Error
data_wanted_content_copy(data_util_state* state, data_wanted_content* destinationWantedContent, data_wanted_content* sourceWantedContent)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationWantedContent, data_wanted_content);

    err = err ? err : data_string_copy(state, &destinationWantedContent->country, &sourceWantedContent->country);
    err = err ? err : data_string_copy(state, &destinationWantedContent->dataset_id, &sourceWantedContent->dataset_id);
    err = err ? err : data_string_copy(state, &destinationWantedContent->type, &sourceWantedContent->type);

    return err;
}

/*! @} */
