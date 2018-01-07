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

    @file     data_app_error_event.c
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

#include "data_app_error_event.h"

NB_Error
data_app_error_event_init(data_util_state* pds, data_app_error_event* appErrorEvent)
{
    NB_Error err = NE_OK;

    err = data_error_category_init(pds, &appErrorEvent->error_category);
    DATA_VEC_ALLOC(err, appErrorEvent->vec_parameters, data_pair);

    appErrorEvent->error_code = 0;
    err = err ? err : data_string_init(pds, &appErrorEvent->detailed_code);
    err = err ? err : data_string_init(pds, &appErrorEvent->detailed_description);

    return err;
}

void
data_app_error_event_free(data_util_state* pds, data_app_error_event* appErrorEvent)
{
    data_error_category_free(pds, &appErrorEvent->error_category);
    DATA_VEC_FREE(pds, appErrorEvent->vec_parameters, data_pair);

    appErrorEvent->error_code = 0;
    data_string_free(pds, &appErrorEvent->detailed_code);
    data_string_free(pds, &appErrorEvent->detailed_description);
}

NB_Error
data_app_error_event_from_tps(data_util_state* pds, data_app_error_event* appErrorEvent, tpselt te)
{
    tpselt ce = NULL;
    NB_Error err = NE_OK;
    int iteration = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_app_error_event_free(pds, appErrorEvent);
    err = data_app_error_event_init(pds, appErrorEvent);
    if (err != NE_OK)
    {
        return err;
    }

    while (err == NE_OK && (ce = te_nextchild(te, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "error-category") == 0)
        {
            err = data_error_category_from_tps(pds, &appErrorEvent->error_category, ce);
        }
        else if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, appErrorEvent->vec_parameters, data_pair);
        }
    }

    appErrorEvent->error_code = te_getattru(te, "error-code");
    err = err ? err : data_string_from_tps_attr(pds, &appErrorEvent->detailed_code, te, "detailed-code");
    err = err ? err : data_string_from_tps_attr(pds, &appErrorEvent->detailed_description, te, "detailed-description");

errexit:
    if (err != NE_OK)
    {
        data_app_error_event_free(pds, appErrorEvent);
    }
    return err;
}

tpselt
data_app_error_event_to_tps(data_util_state* pds, data_app_error_event* appErrorEvent)
{
    tpselt ce = NULL;
    tpselt te = te_new("app-error-event");

    if (te == NULL)
    {
        return NULL;
    }

    ce = data_error_category_to_tps(pds, &appErrorEvent->error_category);

    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    DATA_VEC_TO_TPS(pds, errexit, te, appErrorEvent->vec_parameters, data_pair);

    if (!te_setattru(te, "error-code", appErrorEvent->error_code))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "detailed-code", data_string_get(pds, &appErrorEvent->detailed_code)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "detailed-description", data_string_get(pds, &appErrorEvent->detailed_description)))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean
data_app_error_event_equal(data_util_state* pds, data_app_error_event* left, data_app_error_event* right)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &left->error_category, &right->error_category, data_error_category);
    DATA_VEC_EQUAL(pds, ret, left->vec_parameters, right->vec_parameters, data_pair);

    ret = ret && (left->error_code == right->error_code);
    DATA_EQUAL(pds, ret, &left->detailed_code, &right->detailed_code, data_string);
    DATA_EQUAL(pds, ret, &left->detailed_description, &right->detailed_description, data_string);

    return (boolean) ret;
}

NB_Error
data_app_error_event_copy(data_util_state* pds, data_app_error_event* dest, data_app_error_event* src)
{
    NB_Error err = NE_OK;

    data_app_error_event_free(pds, dest);
    err = data_app_error_event_init(pds, dest);

    err = err ? err : data_error_category_copy(pds, &dest->error_category, &src->error_category);
    DATA_VEC_COPY(pds, err, dest->vec_parameters, src->vec_parameters, data_pair);

    dest->error_code = src->error_code;
    err = err ? err : data_string_copy(pds, &dest->detailed_code, &src->detailed_code);
    err = err ? err : data_string_copy(pds, &dest->detailed_description, &src->detailed_description);

    return err;
}

uint32
data_app_error_event_get_tps_size(data_util_state* pds, data_app_error_event* appErrorEvent)
{
    uint32 size = 0;

    size += data_error_category_get_tps_size(pds, &appErrorEvent->error_category);
    DATA_VEC_GET_TPS_SIZE(pds, size, appErrorEvent->vec_parameters, data_pair);

    size += sizeof(appErrorEvent->error_code);
    size += data_string_get_tps_size(pds, &appErrorEvent->detailed_code);
    size += data_string_get_tps_size(pds, &appErrorEvent->detailed_description);

    return size;
}

void
data_app_error_event_to_buf(data_util_state* pds, data_app_error_event* appErrorEvent, struct dynbuf *buffer)
{
    data_error_category_to_buf(pds, &appErrorEvent->error_category, buffer);
    DATA_VEC_TO_BUF(pds, buffer, appErrorEvent->vec_parameters, data_pair);

    dbufcat(buffer, (const byte*) &appErrorEvent->error_code, sizeof(appErrorEvent->error_code));
    data_string_to_buf(pds, &appErrorEvent->detailed_code, buffer);
    data_string_to_buf(pds, &appErrorEvent->detailed_description, buffer);
}

NB_Error
data_app_error_event_from_binary(data_util_state* pds, data_app_error_event* appErrorEvent, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = data_error_category_from_binary(pds, &appErrorEvent->error_category, ppdata, pdatalen);
    DATA_VEC_FROM_BINARY(pds, err, ppdata, pdatalen, appErrorEvent->vec_parameters, data_pair);

    err = err ? err : data_uint32_from_binary(pds, &appErrorEvent->error_code, ppdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &appErrorEvent->detailed_code, ppdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &appErrorEvent->detailed_description, ppdata, pdatalen);

    return err;
}

/*! @} */
