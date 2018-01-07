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

    @file     nbanalyticsinformation.c
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

#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "datautil.h"
#include "nbanalyticsinformation.h"
#include "data_analytics_events_reply.h"

struct NB_AnalyticsInformation
{
    NB_Context*             context;

    data_analytics_config   analytics_config;     /*!< config received. */
    boolean                 got_analytics_config; /*!< whether config has been
                                                    received.*/
};

NB_Error
NB_AnalyticsInformationCreate(NB_Context* context, tpselt reply,
        NB_AnalyticsInformation** information)
{
    NB_AnalyticsInformation* pThis = NULL;
    NB_Error err = NE_OK;
    data_util_state* pds = NULL;
    data_analytics_events_reply replyData;

    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    DATA_INIT(pds, err, &pThis->analytics_config, data_analytics_config);
    if (err)
    {
        nsl_free(pThis);
        return err;
    }

    pThis->context = context;
    pds = NB_ContextGetDataState(context);

    DATA_INIT(pds, err, &replyData, data_analytics_events_reply);
    if (err)
    {
        DATA_FREE(pds, &pThis->analytics_config, data_analytics_config);
        nsl_free(pThis);
        return err;
    }

    err = data_analytics_events_reply_from_tps(pds, &replyData, reply);
    if (err)
    {
        DATA_FREE(pds, &replyData, data_analytics_events_reply);
        DATA_FREE(pds, &pThis->analytics_config, data_analytics_config);
        nsl_free(pThis);
        return err;
    }

    if (replyData.got_analytics_config)
    {
        DATA_COPY(pds, err, &pThis->analytics_config,
                &replyData.analytics_config, data_analytics_config);
        pThis->got_analytics_config = TRUE;
    }

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        DATA_FREE(pds, &pThis->analytics_config, data_analytics_config);
        nsl_free(pThis);
    }

    DATA_FREE(pds, &replyData, data_analytics_events_reply);
    return err;
}

boolean
NB_AnalyticsInformationIsConfigReceived(NB_AnalyticsInformation* pThis)
{
    if (pThis && pThis->got_analytics_config)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_Error
NB_AnalyticsInformationGetConfig(NB_AnalyticsInformation* pThis,
                                 NB_AnalyticsEventsConfig* config)
{
    if (!pThis || !config || !pThis->got_analytics_config)
    {
        return NE_INVAL;
    }

    return NB_AnalyticsEventsConfigFromConfig(config, &pThis->analytics_config);
}


NB_Error
NB_AnalyticsInformationDestroy(NB_AnalyticsInformation* pThis)
{
    data_util_state* pds = NULL;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);
    DATA_FREE(pds, &pThis->analytics_config, data_analytics_config);

    nsl_free(pThis);

    return NE_OK;
}

/*! @} */
