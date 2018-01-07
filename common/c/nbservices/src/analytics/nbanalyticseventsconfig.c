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

    @file     nbanalyticseventsconfig.c
    @defgroup nbanalyticseventsconfig
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
#include "nbpersistentdataprotected.h"
#include "palstdlib.h"
#include "nbanalytics.h"
#include "nbanalyticsprivate.h"
#include "nbanalyticseventsconfig.h"
#include "fileutil.h"
#include "vec.h"
#include "data_event_type.h"

/*! @{ */

#define ANALYTICS_CONFIG_NAME "ae.cfg"
#define ANALYTICS_CONFIG_DBUF_SIZE 2048

typedef struct NB_AnalyticsEventCategoryPriority
{
    NB_AnalyticsEventsPriority priority;
    NB_AnalyticsEventsCategory category;
} NB_AnalyticsEventCategoryPriority;


int
NB_AnalyticsEventCategoryPriorityCmp(const void* p1, const void* p2)
{
    const NB_AnalyticsEventCategoryPriority* pair1 =
        (const NB_AnalyticsEventCategoryPriority*)p1;

    const NB_AnalyticsEventCategoryPriority* pair2 =
        (const NB_AnalyticsEventCategoryPriority*)p2;

    /* higher priority comes first */
    if (pair1->priority < pair2->priority)
    {
        return 1;
    }
    else if (pair1->priority > pair2->priority)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void
NB_AnalyticsEventsConfigSortCategories(NB_AnalyticsEventsConfig* pThis)
{
    int i;
    NB_AnalyticsEventCategoryPriority categoryPriority[NB_AEC_MAX];
    NB_AnalyticsEventCategoryPriority tmp;

    /* build an array of category:priority pair */
    for (i=0; i<NB_AEC_MAX; i++)
    {
        categoryPriority[i].category = (NB_AnalyticsEventsCategory)i;
        categoryPriority[i].priority = pThis->priorities[i];
    }

    /* sort */
    nim_heapsort(categoryPriority, NB_AEC_MAX,
            sizeof(NB_AnalyticsEventCategoryPriority), &tmp,
            NB_AnalyticsEventCategoryPriorityCmp);

    for (i=0; i<NB_AEC_MAX; i++)
    {
        pThis->sortedCategories[i] = categoryPriority[i].category;
    }
}

void
NB_AnalyticsEventsConfigInit(NB_AnalyticsEventsConfig* pThis)
{
    nsl_memset(pThis, 0, sizeof(*pThis));
    NB_AnalyticsEventsConfigSortCategories(pThis);
}

static void
NB_AnalyticsEventsConfigToBuf(NB_AnalyticsEventsConfig* config,
        struct dynbuf* pdb)
{
    uint32 prioritiesSize = sizeof(config->priorities);
    uint32 errorCategoryIsAvailableSize = sizeof(config->errorCategoryIsAvailable);
    uint32 billableEventsSize = sizeof(config->billable_events);
    uint32 billableEventsSizeEx = sizeof(config->billable_eventsEx);

    dbufcat(pdb, (const byte*)&config->max_hold_time,
            sizeof(config->max_hold_time));

    dbufcat(pdb, (const byte*)&config->max_data_age,
            sizeof(config->max_data_age));

    dbufcat(pdb, (const byte*)&config->max_request_size,
            sizeof(config->max_request_size));

    dbufcat(pdb, (const byte*)&prioritiesSize, sizeof(prioritiesSize));
    dbufcat(pdb, (const byte*)&config->priorities[0], prioritiesSize);

    dbufcat(pdb, (const byte*)&errorCategoryIsAvailableSize, sizeof(errorCategoryIsAvailableSize));
    dbufcat(pdb, (const byte*)&config->errorCategoryIsAvailable[0], errorCategoryIsAvailableSize);

    dbufcat(pdb, (const byte*)&config->gps_probes_sample_rate,
            sizeof(config->gps_probes_sample_rate));

    dbufcat(pdb, (const byte*)&config->wifi_probes_config,
            sizeof(config->wifi_probes_config));

    dbufcat(pdb, (const byte*)&billableEventsSize, sizeof(billableEventsSize));
    dbufcat(pdb, (const byte*)&config->billable_events, billableEventsSize);
    dbufcat(pdb, (const byte*)&billableEventsSizeEx, sizeof(billableEventsSizeEx));
    dbufcat(pdb, (const byte*)&config->billable_eventsEx, billableEventsSizeEx);

    dbufcat(pdb, (const byte*)&config->session_max_idle_time,
        sizeof(config->session_max_idle_time));

    dbufcat(pdb, (const byte*)&config->route_tracking_collection_interval,
        sizeof(config->route_tracking_collection_interval));

    dbufcat(pdb, (const byte*)&config->max_billable_hold_time,
        sizeof(config->max_billable_hold_time));
}

static NB_Error
NB_AnalyticsEventsConfigFromBinary(data_util_state* pds,
        NB_AnalyticsEventsConfig* config,
        byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 prioritiesSize = 0;
    uint32 errorCategoryIsAvailableSize = 0;
    uint32 billableEventsSize = 0;
    uint32 billableEventsSizeEx = 0;

    err = err ? err : data_uint32_from_binary(pds, &config->max_hold_time,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &config->max_data_age,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &config->max_request_size,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &prioritiesSize,
            pdata, pdatalen);

    if (err)
    {
        return err;
    }

    if (*pdatalen < prioritiesSize)
    {
        err = NE_BADDATA;
        return err;
    }
    nsl_memcpy(&config->priorities[0], *pdata, prioritiesSize);

    *pdata += prioritiesSize;
    *pdatalen -= prioritiesSize;

    err = err ? err : data_uint32_from_binary(pds, &errorCategoryIsAvailableSize,
            pdata, pdatalen);

    if (err)
    {
        return err;
    }

    if (*pdatalen < errorCategoryIsAvailableSize)
    {
        return NE_BADDATA;
    }
    nsl_memcpy(&config->errorCategoryIsAvailable[0], *pdata, errorCategoryIsAvailableSize);

    *pdata += errorCategoryIsAvailableSize;
    *pdatalen -= errorCategoryIsAvailableSize;

    err = data_uint32_from_binary(pds,
            &config->gps_probes_sample_rate, pdata, pdatalen);

    nsl_memcpy(&config->wifi_probes_config, *pdata, sizeof(config->wifi_probes_config));

    *pdata += sizeof(config->wifi_probes_config);
    *pdatalen -= sizeof(config->wifi_probes_config);

    if (*pdatalen < sizeof(billableEventsSize))
    {
        // config binary does not include billable events array
        billableEventsSize = sizeof(config->billable_events);
        nsl_memset(&config->billable_events[0], 0, billableEventsSize);
        return err;
    }

    err = err ? err : data_uint32_from_binary(pds, &billableEventsSize,
        pdata, pdatalen);

    if (err)
    {
        return err;
    }

    if (*pdatalen < billableEventsSize)
    {
        err = NE_BADDATA;
        return err;
    }
    nsl_memcpy(&config->billable_events[0], *pdata, billableEventsSize);

    *pdata += billableEventsSize;
    *pdatalen -= billableEventsSize;


    if (*pdatalen < sizeof(billableEventsSizeEx))
    {
        // config binary does not include billable events array
        billableEventsSizeEx = sizeof(config->billable_eventsEx);
        nsl_memset(config->billable_eventsEx, 0, billableEventsSizeEx);
        return err;
    }

    err = err ? err : data_uint32_from_binary(pds, &billableEventsSizeEx,
        pdata, pdatalen);

    if (err != NE_OK)
    {
        return err;
    }

    if (*pdatalen < billableEventsSizeEx)
    {
        err = NE_BADDATA;
        return err;
    }
    nsl_memcpy(config->billable_eventsEx, *pdata, billableEventsSizeEx);

    *pdata += billableEventsSizeEx;
    *pdatalen -= billableEventsSizeEx;

    err = err ? err : data_uint32_from_binary(pds, &config->session_max_idle_time,
        pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &config->route_tracking_collection_interval,
        pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &config->max_billable_hold_time,
        pdata, pdatalen);
    return err;
}

static analytics_event_type
NB_AnalyticsEventsConfigTranslateEventTypeString(data_string* eventTypeString)
{
    analytics_event_type eventTypeEnum = analytics_event_none;

    if (data_string_compare_cstr(eventTypeString, "search-query-event"))
    {
        eventTypeEnum = analytics_event_search_query;
    }

    else if (data_string_compare_cstr(eventTypeString, "search-detail-event"))
    {
        eventTypeEnum = analytics_event_search_detail;
    }

    else if (data_string_compare_cstr(eventTypeString, "impression-event"))
    {
        eventTypeEnum = analytics_event_impression;
    }

    else if (data_string_compare_cstr(eventTypeString, "map-event"))
    {
        eventTypeEnum = analytics_event_map;
    }

    else if (data_string_compare_cstr(eventTypeString, "place-message-event"))
    {
        eventTypeEnum = analytics_event_place_message;
    }

    else if (data_string_compare_cstr(eventTypeString, "facebook-update-event"))
    {
        eventTypeEnum = analytics_event_facebook_update;
    }

    else if (data_string_compare_cstr(eventTypeString, "call-event"))
    {
        eventTypeEnum = analytics_event_call;
    }

    else if (data_string_compare_cstr(eventTypeString, "arrival-event"))
    {
        eventTypeEnum = analytics_event_arrival;
    }

    else if (data_string_compare_cstr(eventTypeString, "weburl-event"))
    {
        eventTypeEnum = analytics_event_web_url;
    }

    else if (data_string_compare_cstr(eventTypeString, "reveal-review-event"))
    {
        eventTypeEnum = analytics_event_reveal_review;
    }

    else if (data_string_compare_cstr(eventTypeString, "reveal-description-event"))
    {
        eventTypeEnum = analytics_event_reveal_description;
    }

    else if (data_string_compare_cstr(eventTypeString, "add-favorites-event"))
    {
        eventTypeEnum = analytics_event_add_favorites;
    }

    else if (data_string_compare_cstr(eventTypeString, "route-request-event"))
    {
        eventTypeEnum = analytics_event_route_request;
    }

    else if (data_string_compare_cstr(eventTypeString, "route-reply-event"))
    {
        eventTypeEnum = analytics_event_route_reply;
    }

    else if (data_string_compare_cstr(eventTypeString, "route-state-event"))
    {
        eventTypeEnum = analytics_event_route_state;
    }

    else if (data_string_compare_cstr(eventTypeString, "gps-probes-event"))
    {
        eventTypeEnum = analytics_event_gps_probes;
    }

    else if (data_string_compare_cstr(eventTypeString, "coupon-analytics-action-event"))
    {
        eventTypeEnum = analytics_event_coupon_analytics_action;
    }

    else if (data_string_compare_cstr(eventTypeString, "nav-invocation-event"))
    {
        eventTypeEnum = analytics_event_nav_invocation;
    }

    else if (data_string_compare_cstr(eventTypeString, "feedback-event"))
    {
        eventTypeEnum = analytics_event_feedback;
    }

    else if (data_string_compare_cstr(eventTypeString, "app-error-event"))
    {
        eventTypeEnum = analytics_event_app_error;
    }

    else if (data_string_compare_cstr(eventTypeString, "asr-event"))
    {
        eventTypeEnum = analytics_event_asr;
    }

    else if (data_string_compare_cstr(eventTypeString, "extpoi-event"))
    {
        eventTypeEnum = analytics_event_poiext;
    }

    return eventTypeEnum;
}

static int ExtractErrorCategoriesCallback(void* userData, void* itemPointer)
{
    data_error_category *error_category = (data_error_category*)itemPointer;
    nb_boolean *errorCategoryIsAvailable = (nb_boolean*)userData;
    int i = 0;

    if (!errorCategoryIsAvailable || !error_category)
    {
        return FALSE;
    }

    for (i = 0; i < NB_EC_MAX; i++)
    {
        if (data_string_compare_cstr(&error_category->type, NB_AnalyticsErrorCategoryToString(i)))
        {
            errorCategoryIsAvailable[i] = TRUE;
        }
    }

    return TRUE;
}

NB_Error
NB_AnalyticsEventsConfigFromConfig(NB_AnalyticsEventsConfig* config,
        data_analytics_config* analyticsConfig)
{
    if (!config || !analyticsConfig)
    {
        return NE_INVAL;
    }

    NB_AnalyticsEventsConfigInit(config);

    /*  convert minutes to seconds */
    config->max_hold_time = analyticsConfig->max_hold_time;
    config->max_data_age = analyticsConfig->max_data_age * 60;

    if (analyticsConfig->gold_events_config.max_hold_time)
    {
        config->max_billable_hold_time = analyticsConfig->gold_events_config.max_hold_time;
    }
    else
    {
        config->max_billable_hold_time = config->max_hold_time;
    }

    config->max_request_size = analyticsConfig->max_request_size;

    config->priorities[NB_AEC_Gold] =
            analyticsConfig->gold_events_config.priority;
    config->priorities[NB_AEC_Poi] =
            analyticsConfig->poi_events_config.priority;
    config->priorities[NB_AEC_RouteTracking] =
            analyticsConfig->route_tracking_config.priority;
    config->priorities[NB_AEC_GpsProbes] =
            analyticsConfig->gps_probes_config.priority;
    config->priorities[NB_AEC_AppError] =
            analyticsConfig->app_errors_config.priority;
    config->priorities[NB_AEC_WifiProbes] =
            analyticsConfig->wifi_probes_config.priority;
    config->priorities[NB_AEC_Session] =
        analyticsConfig->session_config.priority;
    config->priorities[NB_AEC_Settings] =
        analyticsConfig->settings_events_config.priority;
    config->priorities[NB_AEC_Share] =
        analyticsConfig->share_events_config.priority;
    config->priorities[NB_AEC_UserAction] =
        analyticsConfig->user_actions_events_config.priority;
    config->priorities[NB_AEC_Transaction] =
        analyticsConfig->transaction_events_config.priority;
    config->priorities[NB_AEC_Maps] =
        analyticsConfig->map_events_config.priority;
    config->priorities[NB_AEC_Feedback] =
            analyticsConfig->feedback_config.priority;

    if (!CSL_VectorForEach(analyticsConfig->app_errors_config.vec_error_categories,
                            ExtractErrorCategoriesCallback,
                            config->errorCategoryIsAvailable))
    {
        return NE_BADDATA;
    }

    config->route_tracking_collection_interval = analyticsConfig->route_tracking_config.collection_interval;
    config->session_max_idle_time = analyticsConfig->session_config.max_idle_time;

    config->gps_probes_sample_rate =
            analyticsConfig->gps_probes_config.sample_rate;

    config->wifi_probes_config.collection_interval =
        analyticsConfig->wifi_probes_config.collection_interval;
    config->wifi_probes_config.max_horizontal_uncertainty =
        analyticsConfig->wifi_probes_config.max_horizontal_uncertainty;
    config->wifi_probes_config.max_speed =
        analyticsConfig->wifi_probes_config.max_speed;
    config->wifi_probes_config.minimum_distance_delta =
        analyticsConfig->wifi_probes_config.minimum_distance_delta;

    nsl_memset(&config->billable_events, 0, sizeof(config->billable_events));
    nsl_memset(&config->billable_eventsEx, 0, sizeof(config->billable_eventsEx));
    if (analyticsConfig->gold_events_config.vec_event_type)
    {
        int billableEventCount = 0;

        int i = 0;

        for (i = 0; i < CSL_VectorGetLength(analyticsConfig->gold_events_config.vec_event_type); i++)
        {
            data_event_type* eventType = (data_event_type*)CSL_VectorGetPointer(analyticsConfig->gold_events_config.vec_event_type, i);
            if (eventType)
            {
                analytics_event_type eventTypeEnum = NB_AnalyticsEventsConfigTranslateEventTypeString(&eventType->name);
                if (eventTypeEnum != analytics_event_none)
                {
                    size_t len = strlen(eventType->name);
                    if(len>ANALYTICS_EVENT_NAME_MAXLEN)
                    {
                        return NE_BADDATA;
                    }
                    config->billable_events[billableEventCount] = eventTypeEnum;
                    nsl_strncpy(config->billable_eventsEx[billableEventCount],eventType->name,len);
                    billableEventCount++;
                }
            }
        }
    }

    NB_AnalyticsEventsConfigSortCategories(config);

    return NE_OK;
}


nb_boolean
NB_AnalyticsEventsConfigEqual(NB_AnalyticsEventsConfig* config1,
                              NB_AnalyticsEventsConfig* config2)
{
    if (nsl_memcmp(config1, config2, sizeof(NB_AnalyticsEventsConfig)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


NB_Error
NB_AnalyticsEventsConfigSave(NB_Context* context,
        NB_AnalyticsEventsConfig* config)
{
    NB_Error err = NE_OK;
    struct dynbuf dbuf;

    err = dbufnew(&dbuf, ANALYTICS_CONFIG_DBUF_SIZE);
    if (err)
    {
        return err;
    }

    NB_AnalyticsEventsConfigToBuf(config, &dbuf);
    err = dbuferr(&dbuf);
    if (!err)
    {
        const byte* data = dbufget(&dbuf);
        uint32 dataSize = (uint32)dbuflen(&dbuf);
        err = NB_PersistentDataSet(NB_ContextGetPersistentData(context), ANALYTICS_CONFIG_NAME, data, dataSize, FALSE, TRUE);
    }

    dbufdel(&dbuf);
    return err;

}

NB_Error
NB_AnalyticsEventsConfigRestore(NB_Context* context,
        NB_AnalyticsEventsConfig* config)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(context);
    byte* pData = NULL;
    byte* pDataTemp = NULL;
    nb_size size = 0;

    err = NB_PersistentDataGet(NB_ContextGetPersistentData(context), ANALYTICS_CONFIG_NAME, &pData, &size);

    if (err == NE_OK && size > 0)
    {
        NB_AnalyticsEventsConfig tmpConfig = {0};
        size_t tSize = (size_t)size;

        pDataTemp = pData;

        NB_AnalyticsEventsConfigInit(&tmpConfig);

        err = NB_AnalyticsEventsConfigFromBinary(pds, &tmpConfig, &pDataTemp, &tSize);

        if (!err)
        {
            *config = tmpConfig;

            NB_AnalyticsEventsConfigSortCategories(config);
        }
        nsl_free(pData);
    }

    return err;
}

NB_Error
NB_AnalyticsEventsConfigMasterClear(NB_Context* context,
        NB_AnalyticsEventsConfig* config)
{
    NB_Error err = NE_OK;

    /* reset config to default */
    NB_AnalyticsEventsConfigInit(config);

    /* save default config to persistent storage */
    err = NB_AnalyticsEventsConfigSave(context, config);

    return err;
}

NB_ErrorCategory
NB_AnalyticsGetErrorCategory(NB_Error err)
{
    switch (err)
    {
    case NE_GPS_TIMEOUT:
    case NE_GPS_NO_INFO:
    case NEGPS_TIMEOUT:
    case NEGPS_ACCURACY_UNAVAIL:
    case NEGPS_INFO_UNAVAIL:
    case NEGPS_STALE_BS:
    // TODO: code 1013
    // TODO: code 1014
    // TODO: code 1015
    // TODO: code 1016
    // TODO: code 1021
    // TODO: code 1024
        return NB_EC_GpsData;

    case NE_GPS_PRIVACY:
    case NEGPS_PRIVACY_REFUSED:
    // TODO: code 1025
    // TODO: code 1027
        return NB_EC_GpsPrivacy;

    case NE_GPS:
    case NE_GPS_BUSY:
    case NE_GPS_LOCATION_OFF:
    case NEGPS_GENERAL:
    case NEGPS_RESERVED1:
    case NEGPS_RESERVED2:
    case NEGPS_RESERVED3:
    case NEGPS_RESERVED4:
    case NEGPS_RECEIVER_BUSY:
    case NEGPS_REJECTED:
    // TODO: code 1017
    // TODO: code 1018
    // TODO: code 1019
    // TODO: code 1022
    // TODO: code 1023
    // TODO: code 1026
    // TODO: code 1030
    // TODO: code 1099
        return NB_EC_GpsDevice;

    case NESERVER_UNKNOWN:
    case NESERVER_FAILED:
    case NESERVER_TIMEDOUT:
    case NESERVER_UNKNOWNLOCALERROR:
    case NESERVER_CONNECTIONNOTFOUND:
    // TODO: code 2005
    // TODO: code 2006
        return NB_EC_Network;

    case NE_CORRUPTED:
    case NE_API:
    // TODO: code 50
    // TODO: code 99
        return NB_EC_Client;

    case NE_FSYS:
    case NE_FSYS_OPEN_ERROR:
    case NE_FSYS_PERMISSION_ERROR:
    case NE_FSYS_WRITE_ERROR:
    case NE_FSYS_NETWORK_READ_ERROR:
    case NE_FSYS_UNZIP_READ_ERROR:
    case NE_FSYS_CLOSE_ERROR:
    case NE_FSYS_DELETE_ERROR:
    case NE_FSYS_RENAME_ERROR:
        return NB_EC_FileSystem;
    default:
        return NB_EC_Other;
    }
}

const char*
NB_AnalyticsErrorCategoryToString(NB_ErrorCategory category)
{
    if (category >= NB_EC_MAX)
    {
        return NULL;
    }

    switch (category)
    {
    case NB_EC_GpsData:
        return "gps-data";
    case NB_EC_GpsPrivacy:
        return "gps-privacy";
    case NB_EC_GpsDevice:
        return "gps-device";
    case NB_EC_Network:
        return "network";
    case NB_EC_Client:
        return "client";
    case NB_EC_FileSystem:
        return "file-system";
    default:
        return "other";
    }
}

/* @} */

