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

    @file     data_app_errors_config.c
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_app_errors_config.h"

NB_Error
data_app_errors_config_init(data_util_state* pds, data_app_errors_config* appErrorsConfig)
{
    NB_Error err = NE_OK;

    DATA_VEC_ALLOC(err, appErrorsConfig->vec_error_categories, data_error_category);

    appErrorsConfig->priority = 0;

    return err;
}

void
data_app_errors_config_free(data_util_state* pds, data_app_errors_config* appErrorsConfig)
{
    DATA_VEC_FREE(pds, appErrorsConfig->vec_error_categories, data_error_category);

    appErrorsConfig->priority = 0;
}

NB_Error
data_app_errors_config_from_tps(data_util_state* pds, data_app_errors_config* appErrorsConfig, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iteration = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_app_errors_config_free(pds, appErrorsConfig);
    err = data_app_errors_config_init(pds, appErrorsConfig);
    if (err != NE_OK)
    {
        return err;
    }

    while (err == NE_OK && (ce = te_nextchild(te, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "error-category") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, appErrorsConfig->vec_error_categories, data_error_category);
        }
    }

    appErrorsConfig->priority = te_getattru(te, "priority");

errexit:
    if (err != NE_OK)
    {
        data_app_errors_config_free(pds, appErrorsConfig);
    }
    return err;
}

tpselt
data_app_errors_config_to_tps(data_util_state* pds, data_app_errors_config* appErrorsConfig)
{
    tpselt te = te_new("app-errors-config");

    if (te == NULL)
    {
        return NULL;
    }

    DATA_VEC_TO_TPS(pds, errexit, te, appErrorsConfig->vec_error_categories, data_error_category);
    
    if (!te_setattru(te, "error-code", appErrorsConfig->priority))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean
data_app_errors_config_equal(data_util_state* pds, data_app_errors_config* left, data_app_errors_config* right)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, left->vec_error_categories, right->vec_error_categories, data_error_category);

    ret = ret && (left->priority == right->priority);

    return (boolean) ret;
}

NB_Error
data_app_errors_config_copy(data_util_state* pds, data_app_errors_config* dest, data_app_errors_config* src)
{
    NB_Error err = NE_OK;

    data_app_errors_config_free(pds, dest);
    err = data_app_errors_config_init(pds, dest);

    DATA_VEC_COPY(pds, err, dest->vec_error_categories, src->vec_error_categories, data_error_category);

    dest->priority = src->priority;

    return err;
}

uint32
data_app_errors_config_get_tps_size(data_util_state* pds, data_app_errors_config* appErrorsConfig)
{
    uint32 size = 0;

    DATA_VEC_GET_TPS_SIZE(pds, size, appErrorsConfig->vec_error_categories, data_error_category);

    size += sizeof(appErrorsConfig->priority);

    return size;
}

void
data_app_errors_config_to_buf(data_util_state* pds, data_app_errors_config* appErrorsConfig, struct dynbuf *buffer)
{
    DATA_VEC_TO_BUF(pds, buffer, appErrorsConfig->vec_error_categories, data_error_category);

    dbufcat(buffer, (const byte*) &appErrorsConfig->priority, sizeof(appErrorsConfig->priority));
}

NB_Error
data_app_errors_config_from_binary(data_util_state* pds, data_app_errors_config* appErrorsConfig, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    DATA_VEC_FROM_BINARY(pds, err, ppdata, pdatalen, appErrorsConfig->vec_error_categories, data_error_category);

    err = err ? err : data_uint32_from_binary(pds, &appErrorsConfig->priority, ppdata, pdatalen);

    return err;
}

/*! @} */
