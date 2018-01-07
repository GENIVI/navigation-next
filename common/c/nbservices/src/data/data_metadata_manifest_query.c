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

    @file     data_metadata_manifest_query.c
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

#include "data_metadata_manifest_query.h"
#include "data_wanted_content.h"

NB_Error
data_metadata_manifest_query_init(data_util_state* state, data_metadata_manifest_query* metadataManifestQuery)
{
    metadataManifestQuery->vec_wanted_content = CSL_VectorAlloc(sizeof(data_wanted_content));

    if (!metadataManifestQuery->vec_wanted_content)
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

void
data_metadata_manifest_query_free(data_util_state* state, data_metadata_manifest_query* metadataManifestQuery)
{
    int length = 0;
    int n = 0;

    if (metadataManifestQuery->vec_wanted_content)
    {
        length = CSL_VectorGetLength(metadataManifestQuery->vec_wanted_content);

        for (n = 0; n < length; ++n)
        {
            data_wanted_content_free(state, (data_wanted_content*) CSL_VectorGetPointer(metadataManifestQuery->vec_wanted_content, n));
        }
        CSL_VectorDealloc(metadataManifestQuery->vec_wanted_content);
    }
}

tpselt
data_metadata_manifest_query_to_tps(data_util_state* state, data_metadata_manifest_query* metadataManifestQuery)
{
    int length = 0;
    int n = 0;
    tpselt te = 0;
    tpselt ce = 0;
    data_wanted_content* wanted_content = NULL;

    te = te_new("metadata-manifest-query");

    if (!te)
    {
        return NULL;
    }

    length = CSL_VectorGetLength(metadataManifestQuery->vec_wanted_content);
    for (n = 0; n < length; ++n)
    {
        wanted_content = (data_wanted_content*)CSL_VectorGetPointer(metadataManifestQuery->vec_wanted_content, n);
        if (wanted_content)
        {
            ce = data_wanted_content_to_tps(state, wanted_content);
            if (!ce)
            {
                goto errexit;
            }

            if (!te_attach(te, ce))
            {
                goto errexit;
            }
        }
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

NB_Error
data_metadata_manifest_query_add_wanted_content(data_util_state* state, data_metadata_manifest_query* metadataManifestQuery, const char* country, const char* dataset_id, const char* type)
{
    NB_Error err = NE_OK;
    data_wanted_content wanted_content = {0};

    err = data_wanted_content_init(state, &wanted_content);
    err = err ? err : data_string_set(state, &wanted_content.country, country);
    err = err ? err : data_string_set(state, &wanted_content.dataset_id, dataset_id);
    err = err ? err : data_string_set(state, &wanted_content.type, type);
    err = err ? err : CSL_VectorAppend(metadataManifestQuery->vec_wanted_content, &wanted_content) ? NE_OK : NE_NOMEM;

    if (err != NE_OK)
    {
        data_wanted_content_free(state, &wanted_content);
    }

    return err;
}

/*! @} */
