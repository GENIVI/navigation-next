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

    @file     data_dataset.c
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

#include "data_dataset.h"
#include "data_content_details.h"

NB_Error
data_dataset_init(data_util_state* state, data_dataset* dataset)
{
    dataset->vec_content_details = CSL_VectorAlloc(sizeof(data_content_details));

    if (!dataset->vec_content_details)
    {
        return NE_NOMEM;
    }

    data_string_init(state, &dataset->dataset_id);
    data_string_init(state, &dataset->type);

    return NE_OK;
}

void
data_dataset_free(data_util_state* state, data_dataset* dataset)
{
    int length = 0;
    int n = 0;

    if (dataset->vec_content_details)
    {
        length = CSL_VectorGetLength(dataset->vec_content_details);

        for (n = 0; n < length; ++n)
        {
            data_content_details_free(state, (data_content_details*) CSL_VectorGetPointer(dataset->vec_content_details, n));
        }
        CSL_VectorDealloc(dataset->vec_content_details);
    }

    data_string_free(state, &dataset->dataset_id);
    data_string_free(state, &dataset->type);
}

NB_Error
data_dataset_from_tps(data_util_state* state, data_dataset* dataset, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_content_details content_details = {{{0}}};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&content_details, 0, sizeof(content_details));

    DATA_REINIT(state, err, dataset, data_dataset);

    err = err ? err : data_string_from_tps_attr(state, &dataset->dataset_id, tpsElement, "dataset-id");
	err = err ? err : data_string_from_tps_attr(state, &dataset->type, tpsElement, "type");

    if (err != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "content-details") == 0)
        {
            err = data_content_details_from_tps(state, &content_details, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(dataset->vec_content_details, &content_details))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&content_details, 0, sizeof(content_details));
            }
            else
            {
                data_content_details_free(state, &content_details);
            }
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (err != NE_OK)
    {
        data_dataset_free(state, dataset);
    }

    return err;
}

boolean
data_dataset_equal(data_util_state* state, data_dataset* dataset1, data_dataset* dataset2)
{
    int ret = TRUE;

    ret = (int) (data_string_equal(state, &dataset1->dataset_id, &dataset2->dataset_id) &&
				 data_string_equal(state, &dataset1->type, &dataset2->type));

    DATA_VEC_EQUAL(state, ret, dataset1->vec_content_details, dataset2->vec_content_details, data_content_details);

    return (boolean) ret;
}

NB_Error
data_dataset_copy(data_util_state* state, data_dataset* destinationDataset, data_dataset* sourceDataset)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationDataset, data_dataset);

    DATA_VEC_COPY(state, err, destinationDataset->vec_content_details, sourceDataset->vec_content_details, data_content_details);

    DATA_COPY(state, err, &destinationDataset->dataset_id, &sourceDataset->dataset_id, data_string);

    DATA_COPY(state, err, &destinationDataset->type, &sourceDataset->type, data_string);

    return err;
}

/*! @} */
