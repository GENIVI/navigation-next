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

    @file     data_content_source.c
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

#include "data_content_source.h"
#include "data_url_args_template.h"



NB_Error
data_content_source_init(data_util_state* state, data_content_source* contentSource)
{
    NB_Error err = NE_OK;

    contentSource->vec_url_args_template = CSL_VectorAlloc(sizeof(data_url_args_template));

    if (!contentSource->vec_url_args_template)
    {
        return NE_NOMEM;
    }

    contentSource->internal_source = FALSE;

    err = data_url_init(state, &(contentSource->url));

    err = err ? err : data_string_init(state, &contentSource->gen);
    err = err ? err : data_string_init(state, &contentSource->projection);
    err = err ? err : data_string_init(state, &contentSource->country);

    if (err)
    {
        data_content_source_free(state, contentSource);
    }

    return err;
}

void
data_content_source_free(data_util_state* state, data_content_source* contentSource)
{
    int length = 0;
    int n = 0;

    if (contentSource->vec_url_args_template)
    {
        length = CSL_VectorGetLength(contentSource->vec_url_args_template);

        for (n = 0; n < length; ++n)
        {
            data_url_args_template_free(state, (data_url_args_template*) CSL_VectorGetPointer(contentSource->vec_url_args_template, n));
        }
        CSL_VectorDealloc(contentSource->vec_url_args_template);
        contentSource->vec_url_args_template = NULL;
    }

    data_url_free(state, &(contentSource->url));

    data_string_free(state, &contentSource->gen);
    data_string_free(state, &contentSource->projection);
    data_string_free(state, &contentSource->country);
}

NB_Error
data_content_source_from_tps(data_util_state* state, data_content_source* contentSource, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_url_args_template url_args_template = {0};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&url_args_template, 0, sizeof(url_args_template));

    DATA_REINIT(state, err, contentSource, data_content_source);

	err = err ? err : data_string_from_tps_attr(state, &contentSource->projection, tpsElement, "projection");
	err = err ? err : data_string_from_tps_attr(state, &contentSource->country, tpsElement, "country");

    if (err != NE_OK)
    {
        goto errexit;
    }

    // This attribute is deprecated. It should not be returned if want-extended-maps is present in the query.
    data_string_from_tps_attr(state, &contentSource->gen, tpsElement, "gen");

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "internal-source") == 0)
        {
            contentSource->internal_source = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "url") == 0)
        {
            err = data_url_from_tps(state, &contentSource->url, ce);
        }
        else if (nsl_strcmp(te_getname(ce), "url-args-template") == 0)
        {
            err = data_url_args_template_from_tps(state, &url_args_template, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(contentSource->vec_url_args_template, &url_args_template))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&url_args_template, 0, sizeof(url_args_template));
            }
            else
            {
                data_url_args_template_free(state, &url_args_template);
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
        data_content_source_free(state, contentSource);
    }

    return err;
}

boolean
data_content_source_equal(data_util_state* state, data_content_source* contentSource1, data_content_source* contentSource2)
{
    int ret = TRUE;

    ret = (int)(contentSource1->internal_source == contentSource2->internal_source &&
                data_url_equal(state, &contentSource1->url, &contentSource2->url) &&
                data_string_equal(state, &contentSource1->gen, &contentSource2->gen) &&
                data_string_equal(state, &contentSource1->projection, &contentSource2->projection) &&
                data_string_equal(state, &contentSource1->country, &contentSource2->country));

    DATA_VEC_EQUAL(state, ret, contentSource1->vec_url_args_template, contentSource2->vec_url_args_template, data_url_args_template);

    return (boolean) ret;
}

NB_Error
data_content_source_copy(data_util_state* state, data_content_source* destinationContentSource, data_content_source* sourceContentSource)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationContentSource, data_content_source);

    DATA_COPY(state, err, &destinationContentSource->url, &sourceContentSource->url, data_url);

    DATA_VEC_COPY(state, err, destinationContentSource->vec_url_args_template, sourceContentSource->vec_url_args_template, data_url_args_template);
    DATA_COPY(state, err, &destinationContentSource->gen, &sourceContentSource->gen, data_string);

    DATA_COPY(state, err, &destinationContentSource->projection, &sourceContentSource->projection, data_string);

    DATA_COPY(state, err, &destinationContentSource->country, &sourceContentSource->country, data_string);

    destinationContentSource->internal_source = sourceContentSource->internal_source;

    return err;
}

/*! @} */
