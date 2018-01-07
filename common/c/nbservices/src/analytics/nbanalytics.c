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

    @file     nbanalytics.c
    @defgroup nbanalytics
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "palstdlib.h"
#include "palclock.h"
#include "paltimer.h"
#include "gpsutil.h"
#include "nbqalog.h"
#include "cslnetwork.h"
#include "nbanalytics.h"
#include "nbanalyticsprivate.h"
#include "nbanalyticsprotected.h"
#include "nbanalyticseventsconfig.h"
#include "nbanalyticshandler.h"
#include "nbanalyticsparameters.h"
#include "nbanalyticsinformation.h"
#include "nbanalyticseventstailq.h"
#include "nbanalyticseventsdb.h"
#include "nbanalyticstestprotected.h"
#include "data_analytics_event.h"
#include "data_pair.h"
#include "data_coupon_analytics_action_event.h"
#include "vec.h"
/*! @{ */

struct NB_Analytics
{
    NB_Context* context;
    uint32      sessionId;      /*!< Unique session ID for this client. */
    uint32      nextEventId;    /*!< Identifier for the next Analytics event */

    /* configuration from application */
    char* dir;
    uint32 maxDataSize;
    NB_AnalyticsCallbacks callbacks;

    /* analytics events database*/
    NB_AnalyticsEventsDb*       eventsDb;

    /* configuration from server */
    NB_AnalyticsEventsConfig    eventsConfig;

    /* network status callback */
    NB_NetworkNotifyEventCallback   networkEventCallback;

    /* network query callback */
    NB_RequestHandlerCallback   handlerCallback;

    /* handler */
    NB_AnalyticsHandler*        handler;

    NB_AnalyticsParameters*     parameters;

    /* events reported to server in current query */
    NB_AnalyticsEventsTailQ     eventsInQuery;
    uint32                      retryCount;
    nb_gpsTime                  lastQueryTime;

    /* Is the initial query to server sent? */
    nb_boolean                  isInitialQuerySent;

    /* A TAILQ of data_analytcs_events for each category of events, in reverse
     * time order, i.e, newer events in the head, older events in the tail
     */
    NB_AnalyticsEventsTailQ     eventsTailQs[NB_AEC_MAX];

    /* gps probes status */
    nb_gpsTime                  lastGpsProbeTime;
    nb_boolean                  isLastGpsProbeTimeValid;

    /* current event data size */
    uint32                      currentDataSize;

    /* is data pending event caused by Analytics itself */
    nb_boolean                  dataPendingByAnalytics;

    /* Does Data roaming charge apply */
    nb_boolean                  isRoaming;

    /* Force Uploading requested, keep sending events until all events
     * are sent */
    nb_boolean                  forceUploadRequested;

    /* whether GPS probes collection is enabled. */
    nb_boolean                  gpsProbesEnabled;

    /* Vector of content displayed elements. */
    struct CSL_Vector*          vecContentDisplayedElements;

    /* This structure specifies the categories for which the client wants configuration information. */
    NB_AnalyticsWantAnalyticsConfig   wantAnalyticsConfig;

    /*! Vector to fill events*/
    struct CSL_Vector*          tpsEventsVector;

    /*! User Defined Data, can be set by @see NB_AnalyticsSetUserData*/
    void*                       userData;
};

/* local functions */
static nb_boolean NB_AnalyticsHasEvent(NB_Analytics* pThis);
static NB_Error NB_AnalyticsCreateHandler(NB_Analytics* pThis);
static NB_Error NB_AnalyticsCreateParameters(NB_Analytics* pThis);
static NB_Error NB_AnalyticsCreateParametersEx(NB_Analytics* pThis);
static NB_Error NB_AnalyticsAddEventToQuery(NB_Analytics* pThis,
        data_analytics_event* event);
static NB_Error NB_AnalyticsSendQuery(NB_Analytics* pThis);
static NB_Error NB_AnalyticsSendQueryEx(NB_Analytics* pThis);
static NB_Error NB_AnalyticsHandleReply(NB_Analytics* pThis);
static NB_Error NB_AnalyticsHandleQueryFailure(NB_Analytics* pThis);
static NB_Error NB_AnalyticsHandleQueryTimeout(NB_Analytics* pThis);
static NB_Error NB_AnalyticsApplyConfig(NB_Analytics* pThis);
static NB_Error NB_AnalyticsHandleConfigChange(NB_Analytics* pThis,
        NB_AnalyticsEventsConfig* newConfig);

static NB_Error NB_AnalyticsRemoveEventsInTailQ(NB_Analytics* pThis,
        NB_AnalyticsEventsTailQ* tailq, NB_AnalyticsEventsStatus reason);

static uint32 NB_AnalyticsRemoveAgedEvents(NB_Analytics* pThis);

static void NB_AnalyticsNetworkEventCallback(NB_NetworkNotifyEvent event,
        void* data, void* userData);

static void NB_AnalyticsHandlerCallback(void* handler, NB_RequestStatus status,
        NB_Error err, nb_boolean up, int percent, void* userData);

static void NB_AnalyticsInvokeUserCallback(NB_Analytics* pThis,
        NB_AnalyticsActivity activity);

static void NB_AnalyticsLogConfig(NB_Analytics* pThis,
        NB_AnalyticsEventsConfig* config);

static void NB_AnalyticsTriggerTimerCallback(PAL_Instance *pal, void *userData,
        PAL_TimerCBReason reason);

NB_DEF NB_Error
NB_AnalyticsCreate(NB_Context* context, NB_AnalyticsConfig* analyticsConfig, NB_Analytics** analytics)
{
    NB_Error err = NE_OK;
    NB_Analytics* pThis = 0;
    int i;
    NB_Error configRestoreResult = NE_OK;

    if (!context || !analytics)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->sessionId = PAL_ClockGetGPSTime();
    pThis->nextEventId = 0;

    for(i=0; i<NB_AEC_MAX; i++)
    {
        NB_AE_TAILQ_INIT(&pThis->eventsTailQs[i]);
    }

    NB_AE_TAILQ_INIT(&pThis->eventsInQuery);

    if (analyticsConfig)
    {
        /* save analyticsConfig */
        pThis->maxDataSize = analyticsConfig->maxDataSize;
        if (analyticsConfig->dir)
        {
            pThis->dir = nsl_strdup(analyticsConfig->dir);
            if (pThis->dir == NULL)
            {
                err = NE_NOMEM;
                goto errexit;
            }
        }
    }

    pThis->context = context;

    pThis->handlerCallback.callback = NB_AnalyticsHandlerCallback;
    pThis->handlerCallback.callbackData = pThis;

    pThis->gpsProbesEnabled= TRUE;  /* enable gps probes by default */

    pThis->isRoaming = FALSE;
    pThis->forceUploadRequested = FALSE;

    NB_AnalyticsEventsConfigInit(&pThis->eventsConfig);

    /* restore config from NB_Context */
    configRestoreResult = NB_AnalyticsEventsConfigRestore(context, &pThis->eventsConfig);

    /* log current config */
    NB_AnalyticsLogConfig(pThis, &pThis->eventsConfig);

    /* restore saved events from events database */
    pThis->eventsDb = NB_AnalyticsEventsDbOpen(pThis, pThis->dir);
    if (pThis->eventsDb)
    {
        NB_AnalyticsEventsDbRestoreEvents(pThis->eventsDb,
                pThis->eventsTailQs, NB_AEC_MAX);

        /* since events are recovered, remove them from the database */
        NB_AnalyticsEventsDbRemoveSavedEvents(pThis->eventsDb);
    }

    /* if any error occures during config restore we shouldn't remove aged events */
    if (NE_OK == configRestoreResult)
    {
        /* apply restored config to remove aged events. */
        NB_AnalyticsApplyConfig(pThis);
    }

    /* register network notify event callback */
    pThis->networkEventCallback.callback = NB_AnalyticsNetworkEventCallback;
    pThis->networkEventCallback.callbackData = pThis;

    err = CSL_NetworkAddEventNotifyCallback(NB_ContextGetNetwork(context),
            &pThis->networkEventCallback);

    pThis->wantAnalyticsConfig.wantGoldCategory = TRUE;
    pThis->wantAnalyticsConfig.wantPoiCategory = TRUE;
    pThis->wantAnalyticsConfig.wantRouteTrackingCategory = TRUE;
    pThis->wantAnalyticsConfig.wantGpsProbesCategory = TRUE;
    pThis->wantAnalyticsConfig.wantWifiProbesCategory = TRUE;
    // some servers do not support the following types of analytics
    pThis->wantAnalyticsConfig.wantCouponCategory = FALSE;
    pThis->wantAnalyticsConfig.wantNavInvocationCategory = FALSE;
    pThis->wantAnalyticsConfig.wantFeedbackCategory = TRUE;
    pThis->wantAnalyticsConfig.wantAppErrorsCategory = TRUE;
    pThis->wantAnalyticsConfig.wantSessionCategory = TRUE;
    pThis->wantAnalyticsConfig.wantSettingsCategory = TRUE;
    pThis->wantAnalyticsConfig.wantShareCategory = TRUE;
    pThis->wantAnalyticsConfig.wantUserActionsCategory = TRUE;
    pThis->wantAnalyticsConfig.wantTransactionCategory = TRUE;
    pThis->wantAnalyticsConfig.wantMapCategory = TRUE;
    pThis->tpsEventsVector = NULL;
    pThis->userData = NULL;
errexit:
    if (err)
    {
        NB_AnalyticsDestroy(pThis);
        return err;
    }

    *analytics = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_AnalyticsDestroy(NB_Analytics* pThis)
{
    NB_Error error = NE_OK;

    if (pThis)
    {
        NB_Context* context = pThis->context;
        data_util_state* pds = NB_ContextGetDataState(pThis->context);
        int i = 0;
        int len = 0;

        NB_ASSERT_VALID_THREAD(context);

        /* cancel trigger timer */
        PAL_TimerCancel(NB_ContextGetPal(context), NB_AnalyticsTriggerTimerCallback, pThis);

        if (pThis->eventsDb)
        {
            NB_AnalyticsEventsDbSaveEvents(pThis->eventsDb,
                    pThis->eventsTailQs, NB_AEC_MAX);

            /* save events in the the unfinished query */
            NB_AnalyticsEventsDbSaveEvents(pThis->eventsDb,
                    &pThis->eventsInQuery, 1);

            NB_AnalyticsEventsDbClose(pThis->eventsDb);

            pThis->eventsDb = NULL;
        }

        /* unregister network notify even callback */
        CSL_NetworkRemoveEventNotifyCallback(NB_ContextGetNetwork(context),
            &pThis->networkEventCallback);

        if (pThis->dir)
        {
            nsl_free(pThis->dir);
            pThis->dir = NULL;
        }

        if (pThis->parameters)
        {
            NB_AnalyticsParametersDestroy(pThis->parameters);
            pThis->parameters = NULL;
        }

        if (pThis->handler)
        {
            NB_AnalyticsHandlerDestroy(pThis->handler);
            pThis->handler = NULL;
        }

        NB_AnalyticsEventsTailQFreeAllEvents(pds, &pThis->eventsInQuery);

        for(i=0; i<NB_AEC_MAX; i++)
        {
            NB_AnalyticsEventsTailQFreeAllEvents(pds,&pThis->eventsTailQs[i]);
        }

        if (pThis->vecContentDisplayedElements)
        {
            len = CSL_VectorGetLength(pThis->vecContentDisplayedElements);

            for (i = 0; i < len; i++)
            {
                DATA_FREE(pds, (data_content_displayed*) CSL_VectorGetPointer(pThis->vecContentDisplayedElements, i), data_content_displayed);
            }
            CSL_VectorDealloc(pThis->vecContentDisplayedElements);
            pThis->vecContentDisplayedElements = NULL;
        }
        nsl_free(pThis);
    }

    return error;
}

NB_DEF NB_Error
NB_AnalyticsSetWantAnalyticsConfig(NB_Context* context, NB_AnalyticsWantAnalyticsConfig* wantAnalyticsConfig)
{
    NB_Analytics* pThis = 0;

    if (!context || !wantAnalyticsConfig)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    pThis = NB_ContextGetAnalytics(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    if (wantAnalyticsConfig)
    {
        nsl_memcpy(&pThis->wantAnalyticsConfig, wantAnalyticsConfig, sizeof(pThis->wantAnalyticsConfig));
    }

    return NE_OK;
}

static void
NB_AnalyticsInvokeUserCallback(NB_Analytics* pThis,
        NB_AnalyticsActivity activity)
{
    if (pThis->callbacks.activityCallback)
    {
        pThis->callbacks.activityCallback(pThis, pThis->callbacks.userData,
                                          activity);
    }
}

NB_Context*
NB_AnalyticsGetContext(NB_Analytics* pThis)
{
    return pThis->context;
}

void
NB_AnalyticsLogEventStatus(NB_Analytics* pThis, data_analytics_event* event,
        NB_AnalyticsEventsStatus eventStatus)
{
    NB_QaLogAnalyticsEvent(pThis->context, event->id, event->ts,
            event->session_id, (uint8)event->event_type, (uint8)eventStatus);
}

static void
NB_AnalyticsLogConfig(NB_Analytics* pThis,
        NB_AnalyticsEventsConfig* config)
{
    NB_QaLogAnalyticsConfig(pThis->context, config->max_hold_time,
            config->max_data_age, config->max_request_size,
            (uint8)config->priorities[NB_AEC_Gold],
            (uint8)config->priorities[NB_AEC_Poi],
            (uint8)config->priorities[NB_AEC_RouteTracking],
            (uint8)config->priorities[NB_AEC_GpsProbes],
            config->gps_probes_sample_rate);
}

nb_boolean
NB_AnalyticsHasEvent(NB_Analytics* analytics)
{

    if(analytics && analytics->tpsEventsVector && CSL_VectorGetLength(analytics->tpsEventsVector))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_DEF  nb_boolean
NB_AnalyticsIsGoldenEvent(NB_Analytics* analytics, const char* name)
{
    uint32 i =0;
    if(!analytics)
    {
        return FALSE;
    }
    for(;i< analytics_event_max; ++i){
        if(nsl_strcmp(analytics->eventsConfig.billable_eventsEx[i],name)==0)
        {
            return TRUE;
        }
    }
    return FALSE;
}


NB_DEF nb_boolean
NB_AnalyticsQueryInProgress(NB_Analytics* analytics)
{
    return (nb_boolean) (analytics->handler &&
            NB_AnalyticsHandlerIsRequestInProgress(analytics->handler));
}

NB_Error
NB_AnalyticsCreateHandler(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;
    if (pThis->handler)
    {
        err = NB_AnalyticsHandlerDestroy(pThis->handler);
        if (err != NE_OK)
        {
            return err;
        }
    }

    err = NB_AnalyticsHandlerCreate(pThis->context, &pThis->handlerCallback,
            &pThis->handler);
    if (err != NE_OK)
    {
        return err;
    }

    return NE_OK;
}

/* add event to parameter*/
NB_Error
NB_AnalyticsAddEventToQuery(NB_Analytics* pThis,
        data_analytics_event* event)
{
    data_analytics_event eventCopy;

    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    uint32 eventSize = data_analytics_event_get_tps_size(pds, event);
    uint32 currentSize = NB_AnalyticsParametersGetSize(pThis->parameters);
    uint32 maxRequestSize = pThis->eventsConfig.max_request_size;

    if ((maxRequestSize != 0) &&
        (eventSize + currentSize) > maxRequestSize)
    {
        return NE_NOMEM;    /* exceed size limit */
    }

    DATA_INIT(pds, err, &eventCopy, data_analytics_event);
    if (err != NE_OK)
    {
        return err;
    }

    DATA_COPY(pds, err, &eventCopy, event, data_analytics_event);
    if (err != NE_OK)
    {
        /*  free the copy */
        DATA_FREE(pds, &eventCopy, data_analytics_event);
        return err;
    }

    /* add event to parameter */
    err = NB_AnalyticsParametersAddEvent(pThis->parameters, &eventCopy);
    if (err != NE_OK)
    {
        /*  free the copy */
        DATA_FREE(pds, &eventCopy, data_analytics_event);
        return err;
    }

    return NE_OK;
}


/* add gps probes events to the query */
static NB_Error
NB_AnalyticsAddGpsProbesEventsToQuery(NB_Analytics* pThis)
{
    return NE_NOSUPPORT;
}

/* add all events of a category to the query */
static NB_Error
NB_AnalyticsAddEventCategoryToQuery(NB_Analytics* pThis,
        NB_AnalyticsEventsCategory category)
{
    NB_Error err = NE_OK;
    NB_AnalyticsEventsTailQ* tailq = &pThis->eventsTailQs[category];
    NB_AnalyticsEventsTailQEntry* pEntry;
    NB_AnalyticsEventsTailQEntry* pTemp;

    if (category == NB_AEC_GpsProbes)
    {
        /* GPS probes events need to packed, and is handled differently. */
        return NB_AnalyticsAddGpsProbesEventsToQuery(pThis);
    }

    if (NB_AE_TAILQ_EMPTY(tailq))
    {
        return NE_OK; /* no events to add */
    }

    NB_AE_TAILQ_FOREACH_REVERSE_SAFE(pEntry, tailq, pTemp)
    {
        data_analytics_event* event = pEntry->event;

        err = NB_AnalyticsAddEventToQuery(pThis, event);
        if (err == NE_OK)
        {
            /* log event status */
            NB_AnalyticsLogEventStatus(pThis, event, NB_AES_Uploading);

            /* move this entry to the eventsInQuery tailq */
            NB_AE_TAILQ_REMOVE(tailq, pEntry);
            NB_AE_TAILQ_INSERT_TAIL(&pThis->eventsInQuery, pEntry);
        }
        else
        {
            /* add event failed*/
            break;
        }
    }

    return err;
}

NB_Error
NB_AnalyticsCreateParameters(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;
    uint32 i = 0;
    NB_AnalyticsEventsCategory category;
    NB_AnalyticsEventsPriority priority;

    if (pThis->parameters)
    {
        /* last request failed, reuse this parameter */
        return NE_OK;
    }

    err =  NB_AnalyticsParametersCreate(pThis->context, &pThis->wantAnalyticsConfig, &pThis->parameters);
    if (err != NE_OK)
    {
        return err;
    }
    pThis->retryCount = 0;  /*  new query */

    /* remove events that are too old */
    NB_AnalyticsRemoveAgedEvents(pThis);

    /* add events to parameters */
    for (i = 0; i < NB_AEC_MAX; i++)
    {
        category = pThis->eventsConfig.sortedCategories[i];
        priority = pThis->eventsConfig.priorities[category];

        if (priority == NB_AEP_Invalid)
        {
            /* skip this category */
            continue;
        }

        if (NB_AnalyticsAddEventCategoryToQuery(pThis, category) != NE_OK)
        {
            /* size limit reached */
            break;
        }
    }

    return NE_OK;
}

NB_Error
NB_AnalyticsCreateParametersEx(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;

    if (pThis->parameters)
    {
        /* last request failed, reuse this parameter */
        return NE_OK;
    }

    err =  NB_AnalyticsParametersCreate(pThis->context, &pThis->wantAnalyticsConfig, &pThis->parameters);
    if (err != NE_OK)
    {
        return err;
    }
    pThis->retryCount = 0;  /*  new query */

    return NE_OK;
}

NB_Error
NB_AnalyticsSendQuery(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;

    if (pThis->isRoaming)   /* do not send query if roaming charges apply */
    {
        return NE_OK;
    }

    if (pThis->isInitialQuerySent && !NB_AnalyticsHasEvent(pThis))
    {
        /* has already send initial request, and currently there is no event,
         * do thing.
         */
        return NE_OK;
    }

    if (NB_AnalyticsQueryInProgress(pThis))
    {
        /* query already in progress */
        return NE_OK;
    }

    if ((err = NB_AnalyticsCreateParameters(pThis)) != NE_OK)
    {
        return err;
    }

    if ((err = NB_AnalyticsCreateHandler(pThis)) != NE_OK)
    {
        return err;
    }

    /* start query */
    err = NB_AnalyticsHandlerStartRequest(pThis->handler, pThis->parameters);
    if (err == NE_OK)
    {
        pThis->retryCount++;
        pThis->lastQueryTime = PAL_ClockGetGPSTime();

        NB_AnalyticsInvokeUserCallback(pThis, NB_AA_QuerySent);
    }
    return err;
}

NB_Error
NB_AnalyticsSendQueryEx(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;
    if (pThis->isRoaming)   /* do not send query if roaming charges apply */
    {
        return NE_INTERRUPTED;
    }

    if (pThis->isInitialQuerySent && !NB_AnalyticsHasEvent(pThis))
    {
        /* has already send initial request, and currently there is no event,
         * do thing.
         */
        return NE_NOENT;
    }

    if (NB_AnalyticsQueryInProgress(pThis))
    {
        /* query already in progress */
        return NE_INTERRUPTED;
    }
    if ((err = NB_AnalyticsCreateParametersEx(pThis)) != NE_OK)
    {
        return err;
    }

    if ((err = NB_AnalyticsCreateHandler(pThis)) != NE_OK)
    {
        return err;
    }

    /* start query */
    err = NB_AnalyticsHandlerStartRequestEx(pThis->handler, pThis->parameters,
                                          pThis->tpsEventsVector);

    if (err == NE_OK)
    {
        pThis->retryCount++;
        pThis->lastQueryTime = PAL_ClockGetGPSTime();

        NB_AnalyticsInvokeUserCallback(pThis, NB_AA_QuerySent);
    }

    return err;
}

NB_Error
NB_AnalyticsHandleReply(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;
    NB_AnalyticsInformation* information = NULL;
    NB_AnalyticsEventsConfig newConfig;

    NB_AnalyticsInvokeUserCallback(pThis, NB_AA_ReplyReceived);

    /* remove reported events */
    NB_AnalyticsRemoveEventsInTailQ(pThis, &pThis->eventsInQuery,
            NB_AES_Uploaded);

    /* remove current parameters */
    if (pThis->parameters)
    {
        NB_AnalyticsParametersDestroy(pThis->parameters);
        pThis->parameters = NULL;
    }

    err = NB_AnalyticsHandlerGetInformation(pThis->handler, &information);
    if (err != NE_OK)
    {
        /* should not happen. */
        nsl_assert(FALSE);
        return err;
    }

    if (NB_AnalyticsInformationIsConfigReceived(information))
    {
        NB_AnalyticsInformationGetConfig(information, &newConfig);

        if (!NB_AnalyticsEventsConfigEqual(&pThis->eventsConfig, &newConfig))
        {
            NB_AnalyticsHandleConfigChange(pThis, &newConfig);
        }

        NB_AnalyticsInvokeUserCallback(pThis, NB_AA_ConfigReceived);
    }

    pThis->isInitialQuerySent = TRUE;

    err = NB_AnalyticsInformationDestroy(information);
    return err;
}

/* insert an entry to an ordered tailq */
static void
NB_AnalyticsInserEntryToTailQ(NB_AnalyticsEventsTailQEntry* pEntryToInsert,
                              NB_AnalyticsEventsTailQ* tailq)
{
    data_analytics_event* pEventToInsert = pEntryToInsert->event;

    NB_AnalyticsEventsTailQEntry* pInsertBeforeEntry = NULL;
    NB_AnalyticsEventsTailQEntry* pEntry = NULL;
    NB_AnalyticsEventsTailQEntry* pTemp = NULL;

    NB_AE_TAILQ_FOREACH_SAFE(pEntry, tailq, pTemp)
    {
        data_analytics_event* pEvent = pEntry->event;
        if (pEventToInsert->ts >= pEvent->ts) /* is eventToInsert newer than event? */
        {
            pInsertBeforeEntry = pEntry;
            break;
        }
    }

    if (pInsertBeforeEntry != NULL)
    {
        /* insert before this entry */
        NB_AE_TAILQ_INSERT_BEFORE(pInsertBeforeEntry, pEntryToInsert);
    }
    else
    {
        /* insert as the last one in the tailq */
        NB_AE_TAILQ_INSERT_TAIL(tailq, pEntryToInsert);
    }
}

/* the current upload failed for some reason, move the events
in current query back to its own category tailq
*/
static void
NB_AnalyticsRestoreEventsInQuery(NB_Analytics* pThis)
{
    NB_AnalyticsEventsTailQ* tailq = &pThis->eventsInQuery;
    NB_AnalyticsEventsTailQEntry* pEntry = NULL;
    NB_AnalyticsEventsTailQEntry* pTemp = NULL;

    /* move back events in the query queue to different category queue */
    NB_AE_TAILQ_FOREACH_SAFE(pEntry, tailq, pTemp)
    {
        data_analytics_event* pEvent = pEntry->event;
        NB_AnalyticsEventsCategory eventCategory = NB_AnalyticsGetEventCategory(pThis, pEvent);
        NB_AnalyticsEventsTailQ* categoryTailq = &pThis->eventsTailQs[eventCategory];

        NB_AE_TAILQ_REMOVE(tailq, pEntry);
        NB_AnalyticsInserEntryToTailQ(pEntry, categoryTailq);
    }
}

NB_Error
NB_AnalyticsHandleQueryFailure(NB_Analytics* pThis)
{
    NB_Error err = NE_OK;

    NB_AnalyticsInvokeUserCallback(pThis, NB_AA_QueryFailure);

    /* restore events in current query to category queues */
    NB_AnalyticsRestoreEventsInQuery(pThis);

    /* remove current parameters */
    if (pThis->parameters)
    {
        NB_AnalyticsParametersDestroy(pThis->parameters);
        pThis->parameters = NULL;
    }

    pThis->forceUploadRequested = FALSE; /* stop force upload */

    return err;
}

NB_Error
NB_AnalyticsHandleQueryTimeout(NB_Analytics* pThis)
{
    NB_AnalyticsInvokeUserCallback(pThis, NB_AA_QueryTimeout);
    return NB_AnalyticsHandleQueryFailure(pThis);
}

/* remove events that are reported, failed, aged, or removed for space */
static NB_Error NB_AnalyticsRemoveEventsInTailQ(NB_Analytics* pThis,
        NB_AnalyticsEventsTailQ* tailq, NB_AnalyticsEventsStatus reason)
{
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    NB_AnalyticsEventsTailQEntry* pEntry;
    NB_AnalyticsEventsTailQEntry* pTemp;

    /* report event removal */
    NB_AE_TAILQ_FOREACH_SAFE(pEntry, tailq, pTemp)
    {
        data_analytics_event* event = pEntry->event;

        /* event data size accouting */
        pThis->currentDataSize -= data_analytics_event_get_tps_size(pds, event);

        /* log event status */
        NB_AnalyticsLogEventStatus(pThis, event, reason);

        /* remove entry */
        NB_AE_TAILQ_REMOVE(tailq, pEntry);

        /* free entry */
        NB_AnalyticsEventsTailQEntryDestroy(pds, pEntry);
    }

    return NE_OK;
}

/* remove all events are older than max_data_age
 * return the size of the space recovered
 */
uint32
NB_AnalyticsRemoveAgedEvents(NB_Analytics* pThis)
{
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    uint32 i;
    uint32 spaceRecovered = 0;
    uint32 maxDataAge = pThis->eventsConfig.max_data_age;
    nb_gpsTime earliestAllowedTime;

    if (maxDataAge == 0)    /*  there is no age limit */
    {
        return 0;   /* no space receovered. */
    }

    /* earliest allowed time. events earlier than this should be removed. */
    earliestAllowedTime = PAL_ClockGetGPSTime() - maxDataAge;

    for (i = 0; i<NB_AEC_MAX; i++)
    {
        NB_AnalyticsEventsCategory category;
        NB_AnalyticsEventsTailQ* tailq;
        NB_AnalyticsEventsTailQEntry* pEntry;
        NB_AnalyticsEventsTailQEntry* pTemp;

        category = pThis->eventsConfig.sortedCategories[i];
        tailq = &pThis->eventsTailQs[category];

        if (NB_AE_TAILQ_EMPTY(tailq))
        {
           continue; /* no events to remove in this category */
        }

        NB_AE_TAILQ_FOREACH_REVERSE_SAFE(pEntry, tailq, pTemp)
        {
            /*  check the oldest event */
            data_analytics_event* event = pEntry->event;

            if (event->ts < earliestAllowedTime)
            {
                uint32 eventSize =
                    data_analytics_event_get_tps_size(pds, event);

                /* calculate size */
                spaceRecovered += eventSize;
                pThis->currentDataSize -= eventSize;

                /* log event status */
                NB_AnalyticsLogEventStatus(pThis, event, NB_AES_RemovedForAge);

                /* remove entry */
                NB_AE_TAILQ_REMOVE(tailq, pEntry);

                /* free entry */
                NB_AnalyticsEventsTailQEntryDestroy(pds, pEntry);
            }
            else
            {
                /* all other events inthis category are newer: skip */
                break;
            }
        }
    }

    return spaceRecovered;
}

NB_DEF uint32
NB_AnalyticsGetMaxHoldTime(NB_Analytics* analytics)
{
    if(analytics)
    {
        return analytics->eventsConfig.max_hold_time;
    }
    else
    {
        return 0;
    }
}

NB_DEF uint32
NB_AnalyticsGetMaxBillableHoldTime(NB_Analytics* analytics)
{
    if(analytics)
    {
        return analytics->eventsConfig.max_billable_hold_time;
    }
    else
    {
        return 0;
    }
}

NB_DEC uint32
NB_AnalyticsDetermineMaxHoldTimeForEvents(NB_Analytics* pThis)
{
    return NB_AE_TAILQ_EMPTY(&pThis->eventsTailQs[NB_AEC_Gold]) ?
        pThis->eventsConfig.max_hold_time : pThis->eventsConfig.max_billable_hold_time;
}

NB_Error
NB_AnalyticsApplyConfig(NB_Analytics* pThis)
{
    uint32 i = 0;
    NB_AnalyticsEventsCategory category;
    NB_AnalyticsEventsPriority priority;

    /* remove all events of disabled categories*/
    for (i = 0; i < NB_AEC_MAX; i++)
    {
        category = pThis->eventsConfig.sortedCategories[i];
        priority = pThis->eventsConfig.priorities[category];

        if (priority == NB_AEP_Invalid)
        {
            /* remove all events in this category */
            NB_AnalyticsRemoveEventsInTailQ(pThis,
                    &pThis->eventsTailQs[category],
                    NB_AES_RemovedForPriority);
        }
    }

    NB_AnalyticsRemoveAgedEvents(pThis);

    return NE_OK;
}

NB_Error
NB_AnalyticsHandleConfigChange(NB_Analytics* pThis,
        NB_AnalyticsEventsConfig* newConfig)
{
    pThis->eventsConfig = *newConfig;

    /* log new config */
    NB_AnalyticsLogConfig(pThis, &pThis->eventsConfig);

    /* save config to context */
    NB_AnalyticsEventsConfigSave(pThis->context, &pThis->eventsConfig);

    return NB_AnalyticsApplyConfig(pThis);
}

static void
NB_AnalyticsTriggerTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_Analytics* pThis = (NB_Analytics*) userData;

    if (pThis)
    {
        if (reason == PTCBR_TimerFired)
        {
            /* send piggyback query */
            NB_AnalyticsSendQuery(pThis);
        }
    }
}

void
NB_AnalyticsNetworkEventCallback(NB_NetworkNotifyEvent event, void* data,
        void* userData)
{
    NB_Analytics* pThis = (NB_Analytics*) userData;

    if (event == NB_NetworkNotifyEvent_DataPending)
    {
        nb_boolean isDataPending = *((nb_boolean*)data);

        if (isDataPending) /* data pending TRUE, network busy */
        {
            if (NB_AnalyticsQueryInProgress(pThis))
            {
                pThis->dataPendingByAnalytics = TRUE;
            }
            else
            {
                pThis->dataPendingByAnalytics = FALSE;
            }
        }
        else /* data pending FALSE, network idle */
        {
            if (pThis->forceUploadRequested ||
                 (!pThis->dataPendingByAnalytics))
            {
                /* force uploading, or a new piggyback not triggered by
                 * analytics itself */
                NB_AnalyticsInvokeUserCallback(pThis,
                            NB_AA_PiggybackOpportunity);
            }
        }
    }
}


void
NB_AnalyticsHandlerCallback(void* handler, NB_RequestStatus status,
        NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_Analytics* pThis = (NB_Analytics*) userData;

    if (err != NE_OK)
    {
        /* failed */
        NB_AnalyticsHandleQueryFailure(pThis);
        return;
    }

    switch (status)
    {
        case NB_NetworkRequestStatus_Progress:
            /* not complete yet... return */
            return;

        case NB_NetworkRequestStatus_Success:
            /* got reply back */
            NB_AnalyticsHandleReply(pThis);
            break;

        case NB_NetworkRequestStatus_TimedOut:
            /* timeout, clean up */
            NB_AnalyticsHandleQueryTimeout(pThis);
            break;
        case NB_NetworkRequestStatus_Canceled:
            /* cancelled */
            NB_AnalyticsHandleQueryFailure(pThis);
            break;
        case NB_NetworkRequestStatus_Failed:
            /* failed */
            NB_AnalyticsHandleQueryFailure(pThis);
            break;
    }

    return;
}

NB_AnalyticsEventsCategory
NB_AnalyticsGetEventCategory(NB_Analytics* pThis, data_analytics_event* pae)
{
    return NB_AEC_MAX;
}

NB_DEF NB_Error
NB_AnalyticsProcess(NB_Context* context)
{
    NB_Error err = NE_OK;
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);
    uint32  secondsSinceLastQuery = 0;
    uint32  maxHoldTime = 0;

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    /* remove events that are too old */
    NB_AnalyticsRemoveAgedEvents(pThis);

    maxHoldTime = NB_AnalyticsDetermineMaxHoldTimeForEvents(pThis);

    secondsSinceLastQuery = PAL_ClockGetGPSTime() - pThis->lastQueryTime;
    if (secondsSinceLastQuery >= maxHoldTime)
    {
        /*  can send query */
        NB_AnalyticsSendQuery(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_AnalyticsSendEvents(NB_Context* context, struct CSL_Vector* events)
{
    NB_Error error;
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);
    NB_ASSERT_VALID_THREAD(context);

    if(!events)
    {
        return NE_INVAL;
    }

    if (!pThis) /* analytics has not been created */
    {
        CSL_VectorDealloc(events);
        return NE_INVAL;
    }

    pThis->tpsEventsVector = events;
    error = NB_AnalyticsSendQueryEx(pThis);
    CSL_VectorDealloc(events);
    pThis->tpsEventsVector = NULL;
    return error;
}

NB_DEF NB_Error
NB_AnalyticsSetDataRoamingStatus(NB_Context* context, nb_boolean bRoaming)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    pThis->isRoaming = bRoaming;

    return NE_OK;
}

NB_DEF NB_Error
NB_AnalyticsForceUpload(NB_Context* context)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    pThis->forceUploadRequested = TRUE;
    /*  send events */
    return NB_AnalyticsSendQuery(pThis);
}

NB_DEF NB_Error
NB_AnalyticsMasterClear(NB_Context* context)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    /* cancel current query */
    if (pThis->parameters)
    {
        NB_AnalyticsParametersDestroy(pThis->parameters);
        pThis->parameters = NULL;
    }

    if (pThis->handler)
    {
        NB_AnalyticsHandlerDestroy(pThis->handler);
        pThis->handler = NULL;
    }

    /* master clear config */
    NB_AnalyticsEventsConfigMasterClear(context, &pThis->eventsConfig);

    /* log new config */
    NB_AnalyticsLogConfig(pThis, &pThis->eventsConfig);

    /*  reset internal state */
    pThis->retryCount = 0;
    pThis->isInitialQuerySent = FALSE;
    pThis->isLastGpsProbeTimeValid = FALSE;
    pThis->forceUploadRequested = FALSE;

    return NE_OK;
}

NB_DEF NB_Error
NB_AnalyticsSetCallbacks(NB_Context* context, NB_AnalyticsCallbacks* callbacks)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (callbacks)
    {
        pThis->callbacks = *callbacks;
    }
    else
    {
        nsl_memset(&pThis->callbacks, 0, sizeof(pThis->callbacks));
    }
    return NE_OK;
}

NB_DEF uint32
NB_AnalyticsGetMaxEventSize(NB_Analytics* analytics)
{
    if(analytics)
    {
        return analytics->eventsConfig.max_request_size;
    }
    else
    {
        return 0;
    }
}

NB_DEF uint32
NB_AnalyticsGetMaxDataAge(NB_Analytics* analytics)
{
    if(analytics)
    {
        return analytics->eventsConfig.max_data_age;
    }
    else
    {
        return 0;
    }
}

NB_DEF uint32*
NB_AnalyticsGetSortedCategory(NB_Analytics* analytics)
{
    if(analytics)
    {
        return (uint32*)(analytics->eventsConfig.sortedCategories);
    }
    else
    {
        return NULL;
    }

}

NB_DEF uint32*
NB_AnalyticsGetCategoryPriority(NB_Analytics* analytics)
{
    if(analytics)
    {
        return (uint32*)(analytics->eventsConfig.priorities);
    }
    else
    {
        return NULL;
    }
}

NB_DEF uint32
NB_AnalyticsGetMaxSessionTime(NB_Analytics* analytics)
{
    if(analytics && analytics->eventsConfig.session_max_idle_time)
    {
        return analytics->eventsConfig.session_max_idle_time;
    }
    else
    {
        return 1200;//This value is specified in SDS
    }
}

NB_DEF NB_Error
NB_AnalyticsSetForceUpload(NB_Analytics* analytics, nb_boolean bForceUpload)
{
    if (analytics)
    {
        analytics->forceUploadRequested = bForceUpload;
        return NE_OK;
    }
    else
    {
        return NE_INVAL;
    }
}

NB_DEF NB_Error
NB_AnalyticsSetUserData(NB_Analytics* analytics, void* userData)
{
    if (analytics)
    {
        analytics->userData = userData;
        return NE_OK;
    }
    else
    {
        return NE_INVAL;
    }
}

NB_DEF void*
NB_AnalyticsGetUserData(NB_Analytics* analytics)
{
    if (analytics)
    {
        return analytics->userData;
    }
    else
    {
        return NULL;
    }
}

NB_DEF uint32
NB_AnalyticsGetMaxDataSize(NB_Analytics* analytics)
{
    if (analytics)
    {
        return analytics->maxDataSize;
    }
    else
    {
        return 0;
    }
}

NB_DEF nb_boolean
NB_IsInitialQuerySent(NB_Analytics* analytics)
{
    if (analytics)
    {
        return analytics->isInitialQuerySent;
    }
    else
    {
        return FALSE;
    }
}

NB_DEF uint32
NB_AnalyticsGetRouteCollectionInterval(NB_Analytics* analytics)
{
    if (analytics)
    {
        return analytics->eventsConfig.route_tracking_collection_interval;
    }
    else
    {
        return 0;
    }
}

NB_DEF uint32
NB_AnalyticsGetGPSSampleRate(NB_Analytics* analytics)
{
    if (analytics)
    {
        return analytics->eventsConfig.gps_probes_sample_rate;
    }
    else
    {
        return 0;
    }
}


NB_DEF nb_boolean
NB_AnalyticsIsWifiProbesAllowed(NB_Context* context)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    if (!pThis) /* analytics has not been created */
    {
        return FALSE;
    }

    return (nb_boolean)(pThis->eventsConfig.priorities[NB_AEC_WifiProbes] != NB_AEP_Invalid);
}

NB_DEF const NB_AnalyticsWifiConfig*
NB_AnalyticsGetWifiConfig(NB_Context* context)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    if (!pThis) /* analytics has not been created */
    {
        return NULL;
    }

    return &pThis->eventsConfig.wifi_probes_config;
}

NB_DEF NB_Error
NB_AnalyticsEnableGpsProbes(NB_Context* context,
                            nb_boolean bEnable)
{
    NB_Analytics* pThis = NB_ContextGetAnalytics(context);

    NB_ASSERT_VALID_THREAD(context);

    if (!pThis) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    pThis->gpsProbesEnabled = bEnable;

    if (!pThis->gpsProbesEnabled)
    {
        // remove collected GPS probes
        NB_AnalyticsRemoveEventsInTailQ(pThis,
                                        &pThis->eventsTailQs[NB_AEC_GpsProbes],
                                        NB_AES_RemovedForUserOptOut);
    }
    return NE_OK;
}
/* @} */
