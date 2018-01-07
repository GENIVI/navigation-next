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

    @file     data_url_args_template.c
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

#include "data_url_args_template.h"
#include "data_url_args_template_format.h"
#include "data_metadata.h"

NB_Error
data_url_args_template_init(data_util_state* state, data_url_args_template* urlArgsTemplate)
{
    NB_Error result = NE_OK;
    urlArgsTemplate->vec_metadata = CSL_VectorAlloc(sizeof(data_metadata));

    if (!urlArgsTemplate->vec_metadata)
    {
        result = NE_NOMEM;
        goto errorExit;
    }

    urlArgsTemplate ->vec_formats = CSL_VectorAlloc(sizeof(data_url_args_template_format));

    if (urlArgsTemplate->vec_formats == NULL)
    {

        result = NE_NOMEM;
        goto errorExit;
    }

    result = result ? result : data_url_args_template_characteristics_init(state, &urlArgsTemplate->characteristics);

    if (result != NE_OK)
    {
        goto errorExit;
    }

    data_string_init(state, &urlArgsTemplate->type);
    data_string_init(state, &urlArgsTemplate->template);
    data_string_init(state, &urlArgsTemplate->tile_store_template);
    data_string_init(state, &urlArgsTemplate->ds_gen_id);

    urlArgsTemplate->tile_size = 0;
    urlArgsTemplate->min_zoom = 0;
    urlArgsTemplate->max_zoom = 0;
    urlArgsTemplate->cache_id = 0;
    urlArgsTemplate->ref_tile_grid_level = 0;
    urlArgsTemplate->lam_index = 0;
    urlArgsTemplate->base_draw_order = 0;
    urlArgsTemplate->label_draw_order = 0;
    urlArgsTemplate->download_priority = 0;

errorExit:
    if (result != NE_OK) {
        data_url_args_template_free(state, urlArgsTemplate);
    }
    return result;
}

void
data_url_args_template_free(data_util_state* state, data_url_args_template* urlArgsTemplate)
{
    int length = 0;
    int n = 0;

    if (urlArgsTemplate->vec_metadata)
    {
        length = CSL_VectorGetLength(urlArgsTemplate->vec_metadata);

        for (n = 0; n < length; ++n)
        {
            data_metadata_free(state, (data_metadata*) CSL_VectorGetPointer(urlArgsTemplate->vec_metadata, n));
        }
        CSL_VectorDealloc(urlArgsTemplate->vec_metadata);
    }

    if (urlArgsTemplate->vec_formats)
    {
        length = CSL_VectorGetLength(urlArgsTemplate->vec_formats);

        for (n = 0; n < length; ++n)
        {
            data_url_args_template_format_free(state, (data_url_args_template_format*) CSL_VectorGetPointer(urlArgsTemplate->vec_formats, n));
        }
        CSL_VectorDealloc(urlArgsTemplate->vec_formats);
    }

    data_url_args_template_characteristics_free(state, &urlArgsTemplate->characteristics);

    data_string_free(state, &urlArgsTemplate->type);
    data_string_free(state, &urlArgsTemplate->template);
    data_string_free(state, &urlArgsTemplate->tile_store_template);
    data_string_free(state, &urlArgsTemplate->ds_gen_id);
}

NB_Error
data_url_args_template_from_tps(data_util_state* state, data_url_args_template* urlArgsTemplate, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_metadata metadata = {{0}};
    data_url_args_template_format format = {0};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&metadata, 0, sizeof(metadata));
    nsl_memset(&format, 0, sizeof(format));

    DATA_REINIT(state, err, urlArgsTemplate, data_url_args_template);

    err = err ? err : data_string_from_tps_attr(state, &urlArgsTemplate->type, tpsElement, "type");
    err = err ? err : data_string_from_tps_attr(state, &urlArgsTemplate->template, tpsElement, "template");

    if (err != NE_OK)
    {
        goto errexit;
    }
    err = err ? err : data_string_from_tps_attr(state, &urlArgsTemplate->tile_store_template,
                                                tpsElement,
                                                "tile-store-template");
    err = err ? err : data_string_from_tps_attr(state, &urlArgsTemplate->ds_gen_id, tpsElement, "ds-gen-id");

    urlArgsTemplate->tile_size = te_getattru(tpsElement, "tile-size");
    urlArgsTemplate->min_zoom = te_getattru(tpsElement, "min-zoom");
    urlArgsTemplate->max_zoom = te_getattru(tpsElement, "max-zoom");
    urlArgsTemplate->cache_id = te_getattru(tpsElement, "cache-id");
    urlArgsTemplate->ref_tile_grid_level = te_getattru(tpsElement, "ref-tile-grid-level");
    urlArgsTemplate->lam_index = te_getattru(tpsElement, "lam-index");
    urlArgsTemplate->base_draw_order = te_getattru(tpsElement, "base-draw-order");
    urlArgsTemplate->label_draw_order = te_getattru(tpsElement, "label-draw-order");
    urlArgsTemplate->download_priority = te_getattru(tpsElement, "download-priority");

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "metadata") == 0)
        {
            err = data_metadata_from_tps(state, &metadata, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(urlArgsTemplate->vec_metadata, &metadata))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&metadata, 0, sizeof(metadata));
            }
            else
            {
                data_metadata_free(state, &metadata);
            }
        }
        else if (nsl_strcmp(te_getname(ce), "format") == 0)
        {
            err = data_url_args_template_format_from_tps(state, &format, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(urlArgsTemplate->vec_formats, &format))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&format, 0, sizeof(format));
            }
            else
            {
                data_url_args_template_format_free(state, &format);
            }
        }
        else if (nsl_strcmp(te_getname(ce), "characteristics") == 0)
        {
            err = data_url_args_template_characteristics_from_tps(state, &urlArgsTemplate->characteristics, ce);

        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (err != NE_OK)
    {
        data_url_args_template_free(state, urlArgsTemplate);
    }

    return err;
}

boolean
data_url_args_template_equal(data_util_state* state, data_url_args_template* urlArgsTemplate1, data_url_args_template* urlArgsTemplate2)
{
    int ret = TRUE;

    ret = (int) (data_string_equal(state, &urlArgsTemplate1->type, &urlArgsTemplate2->type) &&
         data_string_equal(state, &urlArgsTemplate1->template, &urlArgsTemplate2->template) &&
         data_string_equal(state, &urlArgsTemplate1->tile_store_template, &urlArgsTemplate2->tile_store_template) &&
         data_string_equal(state, &urlArgsTemplate1->ds_gen_id, &urlArgsTemplate2->ds_gen_id) &&
         urlArgsTemplate1->tile_size == urlArgsTemplate2->tile_size &&
         urlArgsTemplate1->min_zoom == urlArgsTemplate2->min_zoom &&
         urlArgsTemplate1->max_zoom == urlArgsTemplate2->max_zoom &&
         urlArgsTemplate1->cache_id == urlArgsTemplate2->cache_id) &&
         urlArgsTemplate1->ref_tile_grid_level == urlArgsTemplate2->ref_tile_grid_level &&
         urlArgsTemplate1->lam_index == urlArgsTemplate2->lam_index &&
         urlArgsTemplate1->base_draw_order == urlArgsTemplate2->base_draw_order &&
         urlArgsTemplate1->label_draw_order == urlArgsTemplate2->label_draw_order &&
         urlArgsTemplate1->download_priority == urlArgsTemplate2->download_priority;

    DATA_VEC_EQUAL(state, ret, urlArgsTemplate1->vec_metadata, urlArgsTemplate2->vec_metadata, data_metadata);
    DATA_VEC_EQUAL(state, ret, urlArgsTemplate1->vec_formats, urlArgsTemplate2->vec_formats, data_url_args_template_format);
    DATA_EQUAL(state, ret, &urlArgsTemplate1->characteristics, &urlArgsTemplate2->characteristics, data_url_args_template_characteristics);

    return (boolean) ret;
}

NB_Error
data_url_args_template_copy(data_util_state* state, data_url_args_template* destinationUrlArgsTemplate, data_url_args_template* sourceUrlArgsTemplate)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationUrlArgsTemplate, data_url_args_template);

    DATA_VEC_COPY(state, err, destinationUrlArgsTemplate->vec_metadata, sourceUrlArgsTemplate->vec_metadata, data_metadata);

    DATA_VEC_COPY(state, err, destinationUrlArgsTemplate->vec_formats, sourceUrlArgsTemplate->vec_formats, data_url_args_template_format);

    DATA_COPY(state, err, &destinationUrlArgsTemplate->characteristics, &sourceUrlArgsTemplate->characteristics, data_url_args_template_characteristics);

    DATA_COPY(state, err, &destinationUrlArgsTemplate->type, &sourceUrlArgsTemplate->type, data_string);

    DATA_COPY(state, err, &destinationUrlArgsTemplate->template, &sourceUrlArgsTemplate->template, data_string);

    DATA_COPY(state, err, &destinationUrlArgsTemplate->tile_store_template,
                &sourceUrlArgsTemplate->tile_store_template, data_string);
    DATA_COPY(state, err, &destinationUrlArgsTemplate->ds_gen_id, &sourceUrlArgsTemplate->ds_gen_id, data_string);

    if (err == NE_OK)
    {
        destinationUrlArgsTemplate->tile_size = sourceUrlArgsTemplate->tile_size;
        destinationUrlArgsTemplate->min_zoom = sourceUrlArgsTemplate->min_zoom;
        destinationUrlArgsTemplate->max_zoom = sourceUrlArgsTemplate->max_zoom;
        destinationUrlArgsTemplate->cache_id = sourceUrlArgsTemplate->cache_id;
    }

    return err;
}

/*! @} */
