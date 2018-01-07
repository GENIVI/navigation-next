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

    @file     data_content_details.c
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

#include "data_content_details.h"

NB_Error
data_content_details_init(data_util_state* state, data_content_details* contentDetails)
{
    data_box_init(state, &contentDetails->box);

    data_string_init(state, &contentDetails->file_id);
    data_string_init(state, &contentDetails->version);

    contentDetails->size = 0;

    return NE_OK;
}

void
data_content_details_free(data_util_state* state, data_content_details* contentDetails)
{
    data_box_free(state, &contentDetails->box);

    data_string_free(state, &contentDetails->file_id);
    data_string_free(state, &contentDetails->version);
}

NB_Error
data_content_details_from_tps(data_util_state* state, data_content_details* contentDetails, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, err, contentDetails, data_content_details);

	err = err ? err : data_string_from_tps_attr(state, &contentDetails->file_id, tpsElement, "file-id");
	err = err ? err : data_string_from_tps_attr(state, &contentDetails->version, tpsElement, "version");

    if (err != NE_OK)
    {
        goto errexit;
    }

    contentDetails->size = te_getattru(tpsElement, "size");

    ce = te_getchild(tpsElement, "box");
    if (ce != NULL)
    {
        err = data_box_from_tps(state, &contentDetails->box, ce);
    }

errexit:

    if (err != NE_OK)
    {
        data_content_details_free(state, contentDetails);
    }

    return err;
}

boolean
data_content_details_equal(data_util_state* state, data_content_details* contentDetails1, data_content_details* contentDetails2)
{
    return (boolean) (contentDetails1->size == contentDetails2->size &&
					  data_string_equal(state, &contentDetails1->file_id, &contentDetails2->file_id) &&
					  data_string_equal(state, &contentDetails1->version, &contentDetails2->version) &&
					  data_box_equal(state, &contentDetails1->box, &contentDetails2->box));
}

NB_Error
data_content_details_copy(data_util_state* state, data_content_details* destinationContentDetails, data_content_details* sourceContentDetails)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationContentDetails, data_content_details);

    DATA_COPY(state, err, &destinationContentDetails->box, &sourceContentDetails->box, data_box);

    DATA_COPY(state, err, &destinationContentDetails->file_id, &sourceContentDetails->file_id, data_string);

    DATA_COPY(state, err, &destinationContentDetails->version, &sourceContentDetails->version, data_string);

    if (err == NE_OK)
    {
        destinationContentDetails->size = sourceContentDetails->size;
    }

    return err;
}

/*! @} */
