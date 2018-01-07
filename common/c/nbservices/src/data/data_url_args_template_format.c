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

 @file     data_url_args_template_format.c
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

#include "data_url_args_template_format.h"
#include "data_pair.h"

NB_Error
data_url_args_template_format_init(data_util_state* state, data_url_args_template_format* urlArgsTemplateFormat)
{
    NB_Error result = NE_OK;
    result = result ? result : data_string_init(state, &urlArgsTemplateFormat->name);
    urlArgsTemplateFormat->max_version = 0;
    urlArgsTemplateFormat->min_version = 0;

    return result ;
}

void
data_url_args_template_format_free(data_util_state* state, data_url_args_template_format* urlArgsTemplateFormat)
{
    data_string_free(state, &urlArgsTemplateFormat->name);
}

NB_Error
data_url_args_template_format_from_tps(data_util_state* state,
                                       data_url_args_template_format* urlArgsTemplateFormat,
                                       tpselt tpsElement)
{
    NB_Error err = NE_OK;
    DATA_REINIT(state, err, urlArgsTemplateFormat, data_url_args_template_format);
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    if (te_getattrc(tpsElement, "name") != NULL)
    {
        err = err ? err : data_string_from_tps_attr(state, &urlArgsTemplateFormat->name, tpsElement, "name");
    }

    urlArgsTemplateFormat->max_version = te_getattru(tpsElement, "max-version");
    urlArgsTemplateFormat->min_version = te_getattru(tpsElement, "min-version");

errexit:

    if (err != NE_OK)
    {
        data_url_args_template_format_free(state, urlArgsTemplateFormat);
    }

    return err;
}

boolean
data_url_args_template_format_equal(data_util_state* state,
                                             data_url_args_template_format* urlArgsTemplateFormat1,
                                             data_url_args_template_format* urlArgsTemplateFormat2)
{
    int ret = TRUE;

    ret =  data_string_equal(state, &urlArgsTemplateFormat1->name, &urlArgsTemplateFormat2->name)
        && (urlArgsTemplateFormat1->max_version == urlArgsTemplateFormat2->max_version)
        && (urlArgsTemplateFormat1->min_version == urlArgsTemplateFormat2->min_version)
    ;

    return (boolean) ret;
}

NB_Error
data_url_args_template_format_copy(data_util_state* state,
                                            data_url_args_template_format* destinationUrlArgsTemplateFormat,
                                            data_url_args_template_format* sourceUrlArgsTemplateFormat)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(state, &destinationUrlArgsTemplateFormat->name, &sourceUrlArgsTemplateFormat->name);

    destinationUrlArgsTemplateFormat->max_version = sourceUrlArgsTemplateFormat->max_version;
    destinationUrlArgsTemplateFormat->min_version = sourceUrlArgsTemplateFormat->min_version;

    return err;
}
/*! @} */
