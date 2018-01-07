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

    @file     data_metadata_source_query.c
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

#include "data_metadata_source_query.h"
#include "data_wanted_content.h"
#include "data_pair.h"

static NB_Error data_metadata_source_query_append_want_extapp_contents_tps(data_util_state* state, data_metadata_source_query* metadataSourceQuery, tpselt tpsElement);

NB_Error
data_metadata_source_query_init(data_util_state* state, data_metadata_source_query* metadataSourceQuery)
{
    NB_Error err = NE_OK;

    metadataSourceQuery->vec_wanted_content = CSL_VectorAlloc(sizeof(data_wanted_content));

    if (!metadataSourceQuery->vec_wanted_content)
    {
        return NE_NOMEM;
    }

    metadataSourceQuery->want_extended_maps = FALSE;
    metadataSourceQuery->want_shared_maps = FALSE;
    metadataSourceQuery->want_unified_maps = FALSE;

    DATA_INIT(state, err, &metadataSourceQuery->language, data_string);
    DATA_VEC_ALLOC(err, metadataSourceQuery->vec_want_extapp_contents, data_pair);

    metadataSourceQuery->screen_width = 0;
    metadataSourceQuery->screen_height = 0;
    metadataSourceQuery->screen_resolution = 0;

    if (err != NE_OK)
    {
        DATA_VEC_FREE(state, metadataSourceQuery->vec_wanted_content, data_wanted_content);
        DATA_FREE(state, &metadataSourceQuery->language, data_string);
        DATA_VEC_FREE(state, metadataSourceQuery->vec_want_extapp_contents, data_pair);
    }

    return err;
}

void
data_metadata_source_query_free(data_util_state* state, data_metadata_source_query* metadataSourceQuery)
{
    int length = 0;
    int n = 0;

    if (metadataSourceQuery->vec_wanted_content)
    {
        length = CSL_VectorGetLength(metadataSourceQuery->vec_wanted_content);

        for (n = 0; n < length; ++n)
        {
            data_wanted_content_free(state, (data_wanted_content*) CSL_VectorGetPointer(metadataSourceQuery->vec_wanted_content, n));
        }
        CSL_VectorDealloc(metadataSourceQuery->vec_wanted_content);
    }

    data_string_free(state, &metadataSourceQuery->language);

    DATA_VEC_FREE(state, metadataSourceQuery->vec_want_extapp_contents, data_pair);
}

tpselt
data_metadata_source_query_to_tps(data_util_state* state, data_metadata_source_query* metadataSourceQuery)
{
    int length = 0;
    int n = 0;

    tpselt te = 0;
    tpselt ce = 0;

    te = te_new("metadata-source-query");

    if (te == NULL)
    {
        return NULL;
    }

    length = CSL_VectorGetLength(metadataSourceQuery->vec_wanted_content);
    for (n = 0; n < length; ++n)
    {
        data_wanted_content* pwc = NULL;
        pwc = (data_wanted_content*)CSL_VectorGetPointer(metadataSourceQuery->vec_wanted_content, n);
        if (pwc)
        {
            ce = data_wanted_content_to_tps(state, pwc);
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

    if (metadataSourceQuery->want_extended_maps)
    {
        ce = te_new("want-extended-maps");
        if (!ce)
        {
            goto errexit;
        }

        if (!te_attach(te, ce))
        {
            goto errexit;
        }
    }

    if (metadataSourceQuery->want_shared_maps)
    {
        ce = te_new("want-shared-maps");
        if (!ce)
        {
            goto errexit;
        }

        if (!te_attach(te, ce))
        {
            goto errexit;
        }
    }

    if (metadataSourceQuery->want_unified_maps)
    {
        ce = te_new("want-unified-maps");
        if (!ce)
        {
            goto errexit;
        }

        if (!te_attach(te, ce))
        {
            goto errexit;
        }
    }

    if (data_metadata_source_query_append_want_extapp_contents_tps(state, metadataSourceQuery, te) != NE_OK)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "language", data_string_get(state, &metadataSourceQuery->language)))
    {
        goto errexit;
    }

    if (!te_setattru(te, "screen-width", metadataSourceQuery->screen_width))
    {
        goto errexit;
    }

    if (!te_setattru(te, "screen-height", metadataSourceQuery->screen_height))
    {
        goto errexit;
    }

    if (!te_setattru(te, "screen-resolution", metadataSourceQuery->screen_resolution))
    {
        goto errexit;
    }

    if (!te_setattru64(te, "ts", metadataSourceQuery->time_stamp))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

NB_Error
data_metadata_source_query_add_wanted_content(data_util_state* state, data_metadata_source_query* metadataSourceQuery, const char* country, const char* dataset_id, const char* type)
{
    NB_Error err = NE_OK;
    data_wanted_content wanted_content = {0};

    err = data_wanted_content_init(state, &wanted_content);
    err = err ? err : data_string_set(state, &wanted_content.country, country);
    err = err ? err : data_string_set(state, &wanted_content.dataset_id, dataset_id);
    err = err ? err : data_string_set(state, &wanted_content.type, type);
    err = err ? err : CSL_VectorAppend(metadataSourceQuery->vec_wanted_content, &wanted_content) ? NE_OK : NE_NOMEM;

    if (err != NE_OK)
    {
        data_wanted_content_free(state, &wanted_content);
    }

    return err;
}

NB_Error
data_metadata_source_query_add_extapp_content(data_util_state* state, data_metadata_source_query* metadataSourceQuery, const char* appCode)
{
    NB_Error err = NE_OK;
    data_pair pair;

    if (!state || !metadataSourceQuery || !metadataSourceQuery->vec_want_extapp_contents || !appCode)
    {
        return NE_BADDATA;
    }

    DATA_MEM_ZERO(&pair, data_pair);
    DATA_INIT(state, err, &pair, data_pair);

    err = err ? err : data_pair_set(state, &pair, "name", appCode);

    if (err == NE_OK)
    {
        if (!CSL_VectorAppend(metadataSourceQuery->vec_want_extapp_contents, &pair))
        {
            err = NE_NOMEM;
        }
    }

    if (err != NE_OK)
    {
        DATA_FREE(state, &pair, data_pair);
    }

    return err;
}

NB_Error
data_metadata_source_query_append_want_extapp_contents_tps(data_util_state* state, data_metadata_source_query* metadataSourceQuery, tpselt tpsElement)
{
    int length = 0;
    tpselt te = NULL;

    if (!state || !metadataSourceQuery || !metadataSourceQuery->vec_want_extapp_contents || !tpsElement)
    {
        return NE_BADDATA;
    }

    length = CSL_VectorGetLength(metadataSourceQuery->vec_want_extapp_contents);
    if (length)
    {
        te = te_new("want-extapp-contents");
        if (!te)
        {
            goto errexit;
        }

        DATA_VEC_TO_TPS(state, errexit, te, metadataSourceQuery->vec_want_extapp_contents, data_pair);

        if (!te_attach(tpsElement, te))
        {
            goto errexit;
        }
    }

    return NE_OK;

errexit:

    if (te)
    {
        te_dealloc(te);
        te = NULL;
    }

    return NE_NOMEM;
}

/*! @} */
