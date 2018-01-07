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

    @file     data_metadata_source_reply.c
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

#include "data_metadata_source_reply.h"
#include "data_content_source.h"
#include "data_extapp.h"

NB_Error
data_metadata_source_reply_init(data_util_state* state, data_metadata_source_reply* metadataSourceReply)
{
    NB_Error err = NE_OK;

    DATA_VEC_ALLOC(err, metadataSourceReply->vec_content_source, data_content_source);
    DATA_VEC_ALLOC(err, metadataSourceReply->vec_maptile_source, data_content_source);

    DATA_INIT(state, err, &metadataSourceReply->city_summary_display, data_string);
    DATA_VEC_ALLOC(err, metadataSourceReply->vec_extapp, data_extapp);

    metadataSourceReply->time_stamp = 0;

    if (err != NE_OK)
    {
        DATA_VEC_FREE(state, metadataSourceReply->vec_content_source, data_content_source);
        DATA_VEC_FREE(state, metadataSourceReply->vec_maptile_source, data_content_source);
        DATA_FREE(state, &metadataSourceReply->city_summary_display, data_string);
        DATA_VEC_FREE(state, metadataSourceReply->vec_extapp, data_extapp);
    }

    return NE_OK;
}

void
data_metadata_source_reply_free(data_util_state* state, data_metadata_source_reply* metadataSourceReply)
{
    int length = 0;
    int n = 0;

    if (metadataSourceReply->vec_content_source)
    {
        length = CSL_VectorGetLength(metadataSourceReply->vec_content_source);

        for (n = 0; n < length; ++n)
        {
            data_content_source_free(state, (data_content_source*) CSL_VectorGetPointer(metadataSourceReply->vec_content_source, n));
        }
        CSL_VectorDealloc(metadataSourceReply->vec_content_source);
    }

    if (metadataSourceReply->vec_maptile_source)
    {
        length = CSL_VectorGetLength(metadataSourceReply->vec_maptile_source);

        for (n = 0; n < length; ++n)
        {
            data_content_source_free(state, (data_content_source*) CSL_VectorGetPointer(metadataSourceReply->vec_maptile_source, n));
        }
        CSL_VectorDealloc(metadataSourceReply->vec_maptile_source);
    }

    data_string_free(state, &metadataSourceReply->city_summary_display);

    DATA_VEC_FREE(state, metadataSourceReply->vec_extapp, data_extapp);
}

NB_Error
data_metadata_source_reply_from_tps(data_util_state* state, data_metadata_source_reply* metadataSourceReply, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;
    data_content_source content_source = {0};
    data_content_source maptile_source = {0};

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    nsl_memset(&content_source, 0, sizeof(content_source));
    nsl_memset(&maptile_source, 0, sizeof(maptile_source));

    DATA_REINIT(state, err, metadataSourceReply, data_metadata_source_reply);

    if (err != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "content-source") == 0)
        {
            err = data_content_source_from_tps(state, &content_source, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            if (!CSL_VectorAppend(metadataSourceReply->vec_content_source, &content_source))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&content_source, 0, sizeof(content_source));
            }
            else
            {
                data_content_source_free(state, &content_source);
            }
        }
        else if (nsl_strcmp(te_getname(ce), "maptile-source") == 0)
        {
            err = data_content_source_from_tps(state, &maptile_source, ce);

            if (err != NE_OK)
            {
                goto errexit;
            }

            /*
             * TODO: Later need append maptile_source to vec_maptile_source
             *       when DB operation support maptile_source
             */
            if (!CSL_VectorAppend(metadataSourceReply->vec_content_source, &maptile_source))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                nsl_memset(&maptile_source, 0, sizeof(maptile_source));
            }
            else
            {
                data_content_source_free(state, &maptile_source);
            }
        }
        else if (nsl_strcmp(te_getname(ce), "city-summary-display") == 0)
        {
            err = data_string_from_tps_attr(state, &metadataSourceReply->city_summary_display, ce, "value");
        }
        else if (nsl_strcmp(te_getname(ce), "extapp-templates-content") == 0)
        {
            int childIter = 0;
            tpselt childElement = NULL;

            while ((childElement = te_nextchild(ce, &childIter)) != NULL)
            {
                if (nsl_strcmp(te_getname(childElement), "extapp") == 0)
                {
                    DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, metadataSourceReply->vec_extapp, data_extapp);
                }
            }
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    te_getattru64(tpsElement, "ts", &metadataSourceReply->time_stamp);

errexit:

    if (err != NE_OK)
    {
        data_metadata_source_reply_free(state, metadataSourceReply);
    }

    return err;
}

/*! @} */
