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

/*-
 * data_analytics_events_query.h: created 2007/06/12 by Michael Gilbert.
 */

#ifndef DATA_ANALYTICS_EVENTS_QUERY_
#define DATA_ANALYTICS_EVENTS_QUERY_

#include "datautil.h"
#include "vec.h"
#include "dynbuf.h"
#include "data_event_detail.h"
#include "data_want_analytics_config.h"
#include "data_analytics_event.h"

typedef struct data_analytics_events_query_ {
    struct CSL_Vector*  vec_analytics_event;    /*!< A set of events. */
    data_want_analytics_config want_analytics_config; /*!< categories for which
                                                        client asks for
                                                        configuration.*/
    /* Attributes */

    /* internal */
    uint32 size; /* current size, in bytes, of the tps query */

} data_analytics_events_query;

NB_Error data_analytics_events_query_init(data_util_state* pds,
            data_analytics_events_query* paeq);

void data_analytics_events_query_free(data_util_state* pds,
            data_analytics_events_query* paeq);

NB_Error data_analytics_events_query_add_event(data_util_state* pds,
            data_analytics_events_query* paeq, data_analytics_event* pae);

tpselt data_analytics_events_query_to_tps(data_util_state* pds,
            data_analytics_events_query* paeq);

NB_Error data_analytics_events_query_copy(data_util_state* pds,
            data_analytics_events_query* paeq_dest,
            data_analytics_events_query* paeq_src);

uint32  data_analytics_events_query_get_tps_size(data_util_state* pds,
        data_analytics_events_query* paeq);

void data_analytics_events_query_to_buf(data_util_state* pds,
            data_analytics_events_query* paeq,
            struct dynbuf* pdb);

NB_Error data_analytics_events_query_from_binary(data_util_state* pds,
            data_analytics_events_query* paeq,
            byte** pdata, size_t* pdatalen);

#endif

