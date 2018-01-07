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

    @file     data_metadata_manifest_reply.c
*/
/*
    See file description in header file.

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

#include "data_metadata_manifest_reply.h"
#include "data_dataset.h"

NB_Error
data_metadata_manifest_reply_init(data_util_state* state, data_metadata_manifest_reply* metadataManifestReply)
{
    metadataManifestReply->vec_dataset = CSL_VectorAlloc(sizeof(data_dataset));

    if (!metadataManifestReply->vec_dataset)
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

void		
data_metadata_manifest_reply_free(data_util_state* state, data_metadata_manifest_reply* metadataManifestReply)
{
    int length = 0;
    int n = 0;

    if (metadataManifestReply->vec_dataset)
    {
        length = CSL_VectorGetLength(metadataManifestReply->vec_dataset);

        for (n = 0; n < length; ++n)
        {
            data_dataset_free(state, (data_dataset*) CSL_VectorGetPointer(metadataManifestReply->vec_dataset, n));
        }
        CSL_VectorDealloc(metadataManifestReply->vec_dataset);
    }
}

NB_Error	
data_metadata_manifest_reply_from_tps(data_util_state* state, data_metadata_manifest_reply* metadataManifestReply, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_dataset dataset = {0};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&dataset, 0, sizeof(dataset));

    DATA_REINIT(state, err, metadataManifestReply, data_metadata_manifest_reply);

    if (err != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "dataset") == 0)
        {
            err = data_dataset_from_tps(state, &dataset, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(metadataManifestReply->vec_dataset, &dataset))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&dataset, 0, sizeof(dataset));
            }
            else
            {
                data_dataset_free(state, &dataset);
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
        data_metadata_manifest_reply_free(state, metadataManifestReply);
    }

    return err;
}

/*! @} */
