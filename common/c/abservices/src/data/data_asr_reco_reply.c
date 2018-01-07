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
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
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
 * data_asr_reco_reply.c: created 2008/04/29 by NikunK.
 */

#include "data_asr_reco_reply.h"
#include "vec.h"
#include "abexp.h"

NB_Error
data_asr_reco_reply_init(data_util_state* pds, data_asr_reco_reply* pgr)
{
    NB_Error err = NE_OK;

    pgr->vec_proxmatch = CSL_VectorAlloc(sizeof(data_proxmatch));
    if (pgr->vec_proxmatch == NULL)
    {
        err = NE_NOMEM;
    }

    pgr->iter_result = 0;

    if (err == NE_OK)
    {
        pgr->vec_locmatch = CSL_VectorAlloc(sizeof(data_locmatch));
        if (pgr->vec_locmatch == NULL)
        {
            err = NE_NOMEM;
        }
    }

    if (err == NE_OK)
    {
        pgr->vec_data_elem_id = CSL_VectorAlloc(sizeof(data_string));
        if (pgr->vec_data_elem_id == NULL)
        {
            err = NE_NOMEM;
        }
    }   

    pgr->completion_code = 0;

    err = err ? err : data_string_init(pds, &pgr->asr_id);
    err = err ? err : data_string_init(pds, &pgr->ambiguous_interaction);

    if (err)
        data_asr_reco_reply_free(pds, pgr);

    return err;
}

void
data_asr_reco_reply_free(data_util_state* pds, data_asr_reco_reply* pgr)
{
    int l = 0, n = 0;
    
    if (pgr->vec_proxmatch)
    {
        l = CSL_VectorGetLength(pgr->vec_proxmatch);

        for (n = 0; n < l; n++)
        {
            data_proxmatch_free(pds, (data_proxmatch*) CSL_VectorGetPointer(pgr->vec_proxmatch, n));
        }
        CSL_VectorDealloc(pgr->vec_proxmatch);
        pgr->vec_proxmatch = NULL;
    }

    if (pgr->vec_locmatch)
    {
        l = CSL_VectorGetLength(pgr->vec_locmatch);

        for (n = 0; n < l; n++)
        {
            data_locmatch_free(pds, (data_locmatch*) CSL_VectorGetPointer(pgr->vec_locmatch, n));
        }
        CSL_VectorDealloc(pgr->vec_locmatch);
        pgr->vec_locmatch = NULL;
    }

    if (pgr->vec_data_elem_id)
    {
        l = CSL_VectorGetLength(pgr->vec_data_elem_id);

        for (n = 0; n < l; n++)
        {
            data_string_free(pds, (data_string*) CSL_VectorGetPointer(pgr->vec_data_elem_id, n));
        }
        CSL_VectorDealloc(pgr->vec_data_elem_id);
        pgr->vec_data_elem_id = NULL;
    }

    if (pgr->asr_id)
        data_string_free(pds, &pgr->asr_id);
    if (pgr->ambiguous_interaction)
        data_string_free(pds, &pgr->ambiguous_interaction);
}

NB_Error
data_asr_reco_reply_from_tps(data_util_state* pds, data_asr_reco_reply* pgr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt  ce;
    int iter = 0;
    data_proxmatch proxmatch;
    data_locmatch locmatch;
    data_string data_elem_id;

    if (te == NULL || pds == NULL || pgr == NULL)
    {
        return NE_INVAL;
    }

    data_proxmatch_init(pds, &proxmatch);
    data_locmatch_init(pds, &locmatch);
    data_string_init(pds, &data_elem_id);

    data_asr_reco_reply_free(pds, pgr);
    err = data_asr_reco_reply_init(pds, pgr);

    if (err != NE_OK)
        return err;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "proxmatch") == 0)
        {
            err = err ? err : data_proxmatch_from_tps(pds, &proxmatch, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pgr->vec_proxmatch, &proxmatch))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    data_proxmatch_init(pds, &proxmatch);
            }
            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(te_getname(ce), "locmatch") == 0)
        {
            err = err ? err : data_locmatch_from_tps(pds, &locmatch, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pgr->vec_locmatch, &locmatch))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    data_locmatch_init(pds, &locmatch);
            }
            if (err != NE_OK)
                goto errexit;
        }       
        else if (nsl_strcmp(te_getname(ce), "data-elem") == 0)
        {
            err = err ? err : data_string_from_tps_attr(pds, &data_elem_id, ce, "id");

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pgr->vec_data_elem_id, &data_elem_id))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    data_string_init(pds, &data_elem_id);
            }
            if (err != NE_OK)
                goto errexit;
        }       
    }

    pgr->completion_code = te_getattru(te, "completion-code");

    err = err ? err : data_string_set(pds, &pgr->asr_id, te_getattrc(te, "asr-id"));
    err = err ? err : data_string_set(pds, &pgr->ambiguous_interaction, te_getattrc(te, "ambiguous-interaction"));

errexit:
    data_proxmatch_free(pds, &proxmatch);
    data_locmatch_free(pds, &locmatch);
    data_string_free(pds, &data_elem_id);
    

    if (err != NE_OK)
        data_asr_reco_reply_free(pds, pgr);
    return err;
}

boolean     
data_asr_reco_reply_equal(data_util_state* pds, data_asr_reco_reply* pgr1, data_asr_reco_reply* pgr2)
{
    return  (boolean) ((pgr1->iter_result == pgr2->iter_result) &&
        (pgr1->completion_code == pgr2->completion_code) &&
        data_string_equal(pds, &pgr1->asr_id, &pgr2->asr_id));
}

NB_Error    
data_asr_reco_reply_copy(data_util_state* pds, data_asr_reco_reply* pgr_dest, data_asr_reco_reply* pgr_src)
{
    NB_Error err = NE_OK;
    int n = 0, l = 0;
    data_locmatch vec_locmatch;
    data_proxmatch vec_proxmatch;

    data_locmatch_init(pds, &vec_locmatch);
    data_proxmatch_init(pds, &vec_proxmatch);
    if (pgr_dest)
        data_asr_reco_reply_free(pds, pgr_dest);
    err = err ? err : data_asr_reco_reply_init(pds, pgr_dest);

    pgr_dest->completion_code = pgr_src->completion_code;
    data_string_copy(pds, &pgr_dest->asr_id, &pgr_src->asr_id);

    l = CSL_VectorGetLength(pgr_src->vec_locmatch);

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_locmatch_copy(pds, &vec_locmatch, CSL_VectorGetPointer(pgr_src->vec_locmatch, n));
        err = err ? err : CSL_VectorAppend(pgr_dest->vec_locmatch, &vec_locmatch) ? NE_OK : NE_NOMEM;

        if (err)
            data_locmatch_free(pds, &vec_locmatch);
        else
            err = err ? err : data_locmatch_init(pds, &vec_locmatch);
    }

    l = CSL_VectorGetLength(pgr_src->vec_proxmatch);

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_proxmatch_copy(pds, &vec_proxmatch, CSL_VectorGetPointer(pgr_src->vec_proxmatch, n));
        err = err ? err : CSL_VectorAppend(pgr_dest->vec_proxmatch, &vec_proxmatch) ? NE_OK : NE_NOMEM;

        if (err)
            data_proxmatch_free(pds, &vec_proxmatch);
        else
            err = err ? err : data_proxmatch_init(pds, &vec_proxmatch);
    }

    return err;
}
