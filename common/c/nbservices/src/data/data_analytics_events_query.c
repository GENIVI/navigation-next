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
/* (C) Copyright 2007 by Networks In Motion, Inc.                */
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

/*
 * data_analytics_events_query.c: created 2007/06/13 by Michael Gilbert.
 */

#include "data_analytics_events_query.h"

NB_Error
data_analytics_events_query_init(data_util_state* pds,
        data_analytics_events_query* paeq)
{
    NB_Error err = NE_OK;

    nsl_memset(paeq, 0, sizeof(*paeq));

    DATA_INIT(pds, err, &paeq->want_analytics_config,
            data_want_analytics_config);

    paeq->size = data_want_analytics_config_get_tps_size(pds,
                    &paeq->want_analytics_config);

    return err;
}

void
data_analytics_events_query_free(data_util_state* pds,
        data_analytics_events_query* paeq)
{
    if (paeq->vec_analytics_event)
    {
        DATA_VEC_FREE(pds, paeq->vec_analytics_event, data_analytics_event);
    }

    data_want_analytics_config_free(pds, &paeq->want_analytics_config);

}

NB_Error
data_analytics_events_query_add_event(data_util_state* pds,
        data_analytics_events_query* paeq, data_analytics_event* pae)
{
    if (paeq->vec_analytics_event == NULL)
    {
        paeq->vec_analytics_event =
            CSL_VectorAlloc(sizeof(data_analytics_event));
        if (paeq->vec_analytics_event == NULL)
            return NE_NOMEM;
    }

    if (!CSL_VectorAppend(paeq->vec_analytics_event, pae))
        return NE_NOMEM;

    paeq->size += data_analytics_event_get_tps_size(pds, pae);

    return NE_OK;
}

tpselt
data_analytics_events_query_to_tps(data_util_state* pds,
        data_analytics_events_query* paeq)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("analytics-events-query");

    if (te == NULL)
        goto errexit;

    ce = data_want_analytics_config_to_tps(pds,
            &paeq->want_analytics_config);

    if ( (ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_Error
data_analytics_events_query_copy(data_util_state* pds,
        data_analytics_events_query* paeq_dest,
        data_analytics_events_query* paeq_src)
{
    NB_Error err = NE_OK;

    data_analytics_events_query_free(pds, paeq_dest);

    err = err ? err : data_analytics_events_query_init(pds, paeq_dest);

    if (paeq_src->vec_analytics_event)
    {
        paeq_dest->vec_analytics_event =
            CSL_VectorAlloc(sizeof(data_analytics_event));
        if (paeq_dest->vec_analytics_event == NULL)
        {
            return NE_NOMEM;
        }

        DATA_VEC_COPY(pds, err, paeq_dest->vec_analytics_event,
                paeq_src->vec_analytics_event,
                data_analytics_event);
    }

    DATA_COPY(pds, err, &paeq_dest->want_analytics_config,
            &paeq_src->want_analytics_config,
            data_want_analytics_config);

    paeq_src->size = paeq_dest->size;

    return err;
}

uint32
data_analytics_events_query_get_tps_size(data_util_state* pds,
        data_analytics_events_query* paeq)
{
    return paeq->size;
}
