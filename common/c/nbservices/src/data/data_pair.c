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
 * data_pair.c: created 2007/05/29 by Mike Gilbert.
 */

#include "data_pair.h"

NB_DEF NB_Error
data_pair_init(data_util_state* pds, data_pair* pp)
{
    NB_Error err = NE_OK;

    pp->transient = FALSE;
    err = data_string_init(pds, &pp->key);
    err = err ? err : data_string_init(pds, &pp->value);
    err = err ? err : data_string_init(pds, &pp->section);
    err = err ? err : data_string_init(pds, &pp->type);

    return err;
}

NB_DEF void
data_pair_free(data_util_state* pds, data_pair* pp)
{
    data_string_free(pds, &pp->key);
    data_string_free(pds, &pp->value);
    data_string_free(pds, &pp->section);
    data_string_free(pds, &pp->type);
}

NB_DEF tpselt
data_pair_to_tps(data_util_state* pds, data_pair* pp)
{
    tpselt te;

    te = te_new("pair");

    if (te == NULL)
        goto errexit;

    if (!te_setattrc(te, "value", data_string_get(pds, &pp->value)))
        goto errexit;

    if (!te_setattrc(te, "key", data_string_get(pds, &pp->key)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_DEF NB_Error 
data_pair_from_tps(data_util_state* pds, data_pair* pp, tpselt te)
{
    NB_Error err = NE_OK;
    
    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_pair_free(pds, pp);

    err = data_pair_init(pds, pp);

    if (err != NE_OK)
        return err;
    
    pp->transient = (boolean)(te_getchild(te, "transient") != NULL);

    err = err ? err : data_string_from_tps_attr(pds, &pp->key, te, "key");
    err = err ? err : data_string_from_tps_attr(pds, &pp->value, te, "value");

    if (!err)
    {
        NB_Error sectionErr = data_string_from_tps_attr(pds, &pp->section, te, "section");
        NB_Error typeErr = data_string_from_tps_attr(pds, &pp->type, te, "type");

        // A pair is "key,value", "key,value,type" or "key,value,section,type"
        if (!((sectionErr == NE_INVAL && typeErr == NE_INVAL) ||
              (typeErr == NE_OK && (sectionErr == NE_OK || sectionErr == NE_INVAL))))
        {
            err = NE_INVAL;
        }
    }

errexit:
    if (err != NE_OK)
        data_pair_free(pds, pp);
    return err;
}

NB_DEF boolean      
data_pair_equal(data_util_state* pds, data_pair* pp1, data_pair* pp2)
{
    return (boolean)(data_string_equal(pds, &pp1->key, &pp2->key) &&
                     data_string_equal(pds, &pp1->value, &pp2->value) &&
                     data_string_equal(pds, &pp1->section, &pp2->section) &&
                     data_string_equal(pds, &pp1->type, &pp2->type));
}

NB_DEF NB_Error 
data_pair_copy(data_util_state* pds, data_pair* pp_dest, data_pair* pp_src)
{
    NB_Error err = NE_OK;

    data_pair_free(pds, pp_dest);

    err = data_pair_init(pds, pp_dest);

    err = err ? err : data_string_copy(pds, &pp_dest->key, &pp_src->key);
    err = err ? err : data_string_copy(pds, &pp_dest->value, &pp_src->value);
    err = err ? err : data_string_copy(pds, &pp_dest->section, &pp_src->section);
    err = err ? err : data_string_copy(pds, &pp_dest->type, &pp_src->type);

    return err;
}

NB_DEF uint32
data_pair_get_tps_size(data_util_state* pds, data_pair* pp)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pp->key);
    size += data_string_get_tps_size(pds, &pp->value);
    size += data_string_get_tps_size(pds, &pp->section);
    size += data_string_get_tps_size(pds, &pp->type);

    return size;
}

NB_DEF void
data_pair_to_buf(data_util_state* pds,
            data_pair* pp,
            struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pp->key, pdb);
    data_string_to_buf(pds, &pp->value, pdb);
    data_string_to_buf(pds, &pp->section, pdb);
    data_string_to_buf(pds, &pp->type, pdb);
}

NB_DEF NB_Error
data_pair_from_binary(data_util_state* pds,
            data_pair* pp,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pp->key, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->value, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->section, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->type, pdata, pdatalen);

    return err;
}

NB_DEF NB_Error
data_pair_set(data_util_state* pds, data_pair* pp, const char* key, const char* value)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_set(pds, &pp->key, key);
    err = err ? err : data_string_set(pds, &pp->value, value);

    return err;
}