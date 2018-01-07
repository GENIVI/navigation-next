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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_category.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_category.h"

NB_Error
data_category_init(data_util_state* pds, data_category* pc)
{
    NB_Error err = NE_OK;

    pc->hasParentCategory = FALSE;
    err = data_parent_category_init(pds, &(pc->parent_category));
    err = err ? err : data_string_init(pds, &pc->code);
    err = err ? err : data_string_init(pds, &pc->name);

    return err;
}

void
data_category_free(data_util_state* pds, data_category* pc)
{
    data_parent_category_free(pds, &(pc->parent_category));
    data_string_free(pds, &pc->code);
    data_string_free(pds, &pc->name);
}

NB_Error
data_category_from_tps(data_util_state* pds, data_category* pc, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt childTpsElement = NULL;
    int iterator = 0;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_category_free(pds, pc);

    err = data_category_init(pds, pc);

    if (err != NE_OK)
        return err;

    while ((childTpsElement = te_nextchild(te, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "parent-category") == 0)
        {
            DATA_FROM_TPS(pds, err, childTpsElement, &(pc->parent_category), data_parent_category);
            pc->hasParentCategory = (err == NE_OK) ? TRUE : FALSE;
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &pc->code, te, "code");

    if (te_getattrc(te, "name") != NULL)
        err = err ? err : data_string_from_tps_attr(pds, &pc->name, te, "name");

errexit:
    if (err != NE_OK)
        data_category_free(pds, pc);
    return err;
}

tpselt
data_category_to_tps(data_util_state* pds, data_category* pp)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("category");

    if (te == NULL)
        goto errexit;

    if (pp->hasParentCategory)
    {
        if (((ce = data_parent_category_to_tps(pds, &pp->parent_category)) != NULL) &&
            te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (!te_setattrc(te, "code", data_string_get(pds, &pp->code)))
        goto errexit;

    if (!te_setattrc(te, "name", data_string_get(pds, &pp->name)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

boolean
data_category_equal(data_util_state* pds, data_category* pc1, data_category* pc2)
{
    boolean result = TRUE;

    if (pc1->hasParentCategory && pc2->hasParentCategory)
    {
        DATA_EQUAL(pds, result, &(pc1->parent_category), &(pc2->parent_category), data_parent_category);
    }
    else if (pc1->hasParentCategory || pc2->hasParentCategory)
    {
        return FALSE;
    }

    result = (result &&
              data_string_equal(pds, &pc1->code, &pc2->code) &&
              data_string_equal(pds, &pc1->name, &pc2->name));

    return result;
}

NB_Error
data_category_copy(data_util_state* pds, data_category* pc_dest, data_category* pc_src)
{
    NB_Error err = NE_OK;

    data_category_free(pds, pc_dest);

    err = data_category_init(pds, pc_dest);

    if (pc_src->hasParentCategory)
    {
        err = err ? err : data_parent_category_copy(pds, &(pc_dest->parent_category), &(pc_src->parent_category));
    }

    err = err ? err : data_string_copy(pds, &pc_dest->code, &pc_src->code);
    err = err ? err : data_string_copy(pds, &pc_dest->name, &pc_src->name);

    if (err == NE_OK)
    {
        pc_dest->hasParentCategory = pc_src->hasParentCategory;
    }
    else
    {
        data_category_free(pds, pc_dest);
    }

    return err;
}

NB_Error
data_category_from_nimcategory(data_util_state* pds, data_category* pp, const NB_Category* pCat)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pp, data_category);

    err = err ? err : data_string_set(pds, &pp->code, pCat->code);
    err = err ? err : data_string_set(pds, &pp->name, pCat->name);

    return err;
}

uint32 data_category_get_tps_size(data_util_state* pds, data_category* pp)
{
    uint32 size = 0;

    size += sizeof(pp->hasParentCategory);
    size += data_parent_category_get_tps_size(pds, &(pp->parent_category));
    size += data_string_get_tps_size(pds, &pp->code);
    size += data_string_get_tps_size(pds, &pp->name);

    return size;
}

void data_category_to_buf(data_util_state* pds, data_category* pp, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*) (&(pp->hasParentCategory)), sizeof(pp->hasParentCategory));
    data_parent_category_to_buf(pds, &(pp->parent_category), pdb);
    data_string_to_buf(pds, &pp->code, pdb);
    data_string_to_buf(pds, &pp->name, pdb);
}

NB_Error data_category_from_binary(data_util_state* pds, data_category* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_boolean_from_binary(pds, &(pp->hasParentCategory), pdata, pdatalen);
    err = err ? err : data_parent_category_from_binary(pds, &(pp->parent_category), pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->code, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->name, pdata, pdatalen);

    return err;
}
