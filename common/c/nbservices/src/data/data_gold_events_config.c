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

    @file     data_gold_events_config.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_gold_events_config.h"
#include "data_event_type.h"

NB_Error
data_gold_events_config_init(data_util_state* pds, data_gold_events_config* pgec)
{
    NB_Error err = NE_OK;

    pgec->vec_event_type = CSL_VectorAlloc(sizeof(data_event_type));

    if (pgec->vec_event_type == NULL)
    {
      err = NE_NOMEM;
    }

    pgec->priority = 0;
    pgec->max_hold_time = 0;

    return err;
}

void
data_gold_events_config_free(data_util_state* pds, data_gold_events_config* pgec)
{
    int len = 0;
    int i = 0;

    if (pgec->vec_event_type)
    {
        len = CSL_VectorGetLength(pgec->vec_event_type);

        for (i = 0; i < len; i++)
        {
            data_event_type_free(pds, (data_event_type*) CSL_VectorGetPointer(pgec->vec_event_type, i));
        }

        CSL_VectorDealloc(pgec->vec_event_type);
    }

   pgec->vec_event_type = NULL;
}

NB_Error
data_gold_events_config_from_tps(data_util_state* pds, data_gold_events_config* pgec, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    data_event_type et_elt = { 0 };
    int i = 0;

    err = data_event_type_init(pds, &et_elt);
    if (err != NE_OK)
    {
        goto errexit;
    }
    
    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_gold_events_config_free(pds, pgec);

    err = data_gold_events_config_init(pds, pgec);

    if (err != NE_OK)
        return err;

    i = 0;

    while ((ce = te_nextchild(te, &i)) != NULL)
    {
        err = data_event_type_from_tps(pds, &et_elt, ce);

        if (err == NE_OK)
        {
            if (!CSL_VectorAppend(pgec->vec_event_type, &et_elt))
            {
                err = NE_NOMEM;
            }

            if (err == NE_OK)
            {
                err = data_event_type_init(pds, &et_elt);
            }
            else
            {
                data_event_type_free(pds, &et_elt);
            }

            if (err != NE_OK)
            {
                goto errexit;
            }
        }
    }

    pgec->priority = te_getattru(te, "priority");
    pgec->max_hold_time = te_getattru(te, "max-hold-time");

errexit:
    if (err != NE_OK)
    {
        data_gold_events_config_free(pds, pgec);
    }
    return err;
}

boolean
data_gold_events_config_equal(data_util_state* pds, data_gold_events_config* pgec1, data_gold_events_config* pgec2)
{
    int n = 0;
    int len = 0;

    if (pgec1->priority != pgec2->priority || pgec1->max_hold_time != pgec2->max_hold_time)
    {
        return FALSE;
    }

    len = CSL_VectorGetLength(pgec1->vec_event_type);
    
    if (len != CSL_VectorGetLength(pgec2->vec_event_type))
    {
        return FALSE;
    }
    
    for (n = 0; n < len; n++)
    {
        if (!data_event_type_equal(pds, (data_event_type*) CSL_VectorGetPointer(pgec1->vec_event_type, n),
                                        (data_event_type*) CSL_VectorGetPointer(pgec2->vec_event_type, n)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

NB_Error
data_gold_events_config_copy(data_util_state* pds, data_gold_events_config* pgec_dest, data_gold_events_config* pgec_src)
{
    NB_Error err = NE_OK;
    data_event_type event_type = { 0 };
    int n = 0;
    int len = 0;

    data_event_type_init(pds, &event_type);

    DATA_REINIT(pds, err, pgec_dest, data_gold_events_config);

    pgec_dest->priority = pgec_src->priority;
    pgec_dest->max_hold_time = pgec_src->max_hold_time;

    len = CSL_VectorGetLength(pgec_src->vec_event_type);

    for (n = 0;n < len && err == NE_OK; n++)
    {
        err = err ? err : data_event_type_copy(pds, &event_type, CSL_VectorGetPointer(pgec_src->vec_event_type, n));
        err = err ? err : CSL_VectorAppend(pgec_dest->vec_event_type, &event_type) ? NE_OK : NE_NOMEM;

        if (err)
        {
            data_event_type_free(pds, &event_type);
        }
        else
        {
            data_event_type_init(pds, &event_type);
        }
    }

    return err;
}

/*! @} */

