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

    @file   data_parent_category.c
*/
/*
    See file description in header file.

    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_parent_category.h"

NB_Error
data_parent_category_init(data_util_state* state, data_parent_category* parentCategory)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(parentCategory, data_parent_category);

    DATA_INIT(state, error, &(parentCategory->code), data_string);
    DATA_INIT(state, error, &(parentCategory->name), data_string);

    return error;
}

void
data_parent_category_free(data_util_state* state, data_parent_category* parentCategory)
{
    DATA_FREE(state, &(parentCategory->code), data_string);
    DATA_FREE(state, &(parentCategory->name), data_string);
}

NB_Error
data_parent_category_from_tps(data_util_state* state, data_parent_category* parentCategory, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, parentCategory, data_parent_category);
    if (error != NE_OK)
    {
        goto errexit;
    }

    error = error ? error : data_string_from_tps_attr(state, &(parentCategory->code), tpsElement, "code");

    if (te_getattrc(tpsElement, "name") != NULL)
    {
        error = error ? error : data_string_from_tps_attr(state, &(parentCategory->name), tpsElement, "name");
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, parentCategory, data_parent_category);
    }

    return error;
}

tpselt
data_parent_category_to_tps(data_util_state* state, data_parent_category* parentCategory)
{
    tpselt tpsElement = NULL;

    tpsElement = te_new("parent-category");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "code", data_string_get(state, &parentCategory->code)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "name", data_string_get(state, &parentCategory->name)))
    {
        goto errexit;
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_parent_category_equal(data_util_state* state, data_parent_category* parentCategory1, data_parent_category* parentCategory2)
{
    int result = TRUE;

    DATA_EQUAL(state, result, &(parentCategory1->code), &(parentCategory2->code), data_string);
    DATA_EQUAL(state, result, &(parentCategory1->name), &(parentCategory2->name), data_string);

    return (boolean) result;
}

NB_Error
data_parent_category_copy(data_util_state* state, data_parent_category* destinationParentCategory, data_parent_category* sourceParentCategory)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationParentCategory, data_parent_category);
    DATA_COPY(state, error, &(destinationParentCategory->code), &(sourceParentCategory->code), data_string);
    DATA_COPY(state, error, &(destinationParentCategory->name), &(sourceParentCategory->name), data_string);

    if (error != NE_OK)
    {
        DATA_FREE(state, destinationParentCategory, data_parent_category);
    }

    return error;
}

uint32
data_parent_category_get_tps_size(data_util_state* state, data_parent_category* parentCategory)
{
    uint32 size = 0;

    size += data_string_get_tps_size(state, &(parentCategory->code));
    size += data_string_get_tps_size(state, &(parentCategory->name));

    return size;
}

void
data_parent_category_to_buf(data_util_state* state, data_parent_category* parentCategory, struct dynbuf* buf)
{
    data_string_to_buf(state, &(parentCategory->code), buf);
    data_string_to_buf(state, &(parentCategory->name), buf);
}

NB_Error
data_parent_category_from_binary(data_util_state* state, data_parent_category* parentCategory, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    error = error ? error : data_string_from_binary(state, &(parentCategory->code), data, dataSize);
    error = error ? error : data_string_from_binary(state, &(parentCategory->name), data, dataSize);

    return error;
}

/*! @} */
