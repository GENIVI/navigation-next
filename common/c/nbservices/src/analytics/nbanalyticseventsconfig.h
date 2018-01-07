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

    @file     nbanalyticseventsconfig.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBANALYTICSEVENTSCONFIG_H_
#define NBANALYTICSEVENTSCONFIG_H_

#include "nbanalytics.h"
#include "nbanalyticsprotected.h"
#include "nbanalyticsprivate.h"
#include "datautil.h"
#include "data_analytics_config.h"

/*!
    @addtogroup nbanalyticsprivate
    @{
*/

/*! @struct NB_ErrorCategory
Error category for Application Errors Analytics feature
*/
typedef enum NB_ErrorCategory
{
    NB_EC_GpsData,
    NB_EC_GpsPrivacy,
    NB_EC_GpsDevice,
    NB_EC_Network,
    NB_EC_Client,
    NB_EC_FileSystem,
    NB_EC_Other,

    NB_EC_MAX
} NB_ErrorCategory;

#define ANALYTICS_EVENT_NAME_MAXLEN 64
typedef char BillableEventsX[ANALYTICS_EVENT_NAME_MAXLEN];

/*! @struct NB_AnalyticsEventsConfig
Analytics Events Configuration from LBS server
*/
typedef struct NB_AnalyticsEventsConfig
{
    uint32 max_hold_time; /* in seconds  */

    uint32 max_data_age;  /* in seconds  */

    uint32 max_billable_hold_time;  /* in seconds */

    uint32 max_request_size;

    /* event priorities */
    NB_AnalyticsEventsPriority priorities[NB_AEC_MAX];

    /* application errors settings */
    nb_boolean errorCategoryIsAvailable[NB_EC_MAX];

    /* gps probes config */
    uint32 gps_probes_sample_rate;

    /* wifi probes config */
    NB_AnalyticsWifiConfig wifi_probes_config;

    /* billable event types from gold events configuration */
    analytics_event_type billable_events[analytics_event_max];

    BillableEventsX billable_eventsEx[analytics_event_max];
    /* internal: categories by priority */
    NB_AnalyticsEventsCategory sortedCategories[NB_AEC_MAX];

    uint32 route_tracking_collection_interval;

    uint32 session_max_idle_time;
} NB_AnalyticsEventsConfig;

/*! Initialize a NB_AnalyticsEventsConfig object
@param config pointer to NB_AnalyticsEventsConfig object.
*/
void NB_AnalyticsEventsConfigInit(NB_AnalyticsEventsConfig* config);

/*! Initialize a NB_AnalyticsEventsConfig object from a blob object
@param config pointer to NB_AnalyticsEventsConfig object.
@param analyticsConifg pointer to a data_analytics_config object
@return NB_Error
 */
NB_Error NB_AnalyticsEventsConfigFromConfig(NB_AnalyticsEventsConfig* config,
        data_analytics_config* analyticsConfig);

/*! Are two NB_AnalyticsEventsConfig objects equal
@param config1 pointer to NB_AnalyticsEventsConfig object.
@param config2 pointer to NB_AnalyticsEventsConfig object.
@return nb_boolean TRUE if equal, FALSE otherwise
 */
nb_boolean NB_AnalyticsEventsConfigEqual(NB_AnalyticsEventsConfig* config1,
                                         NB_AnalyticsEventsConfig* config2);

/*! Save Analytics Events Config
@param context NB_Context
@param config pointer to NB_AnalyticsEventsConfig object.
@return NB_Error
 */
NB_Error NB_AnalyticsEventsConfigSave(NB_Context* context,
        NB_AnalyticsEventsConfig* config);

/*! Restore Analytics Events Config
@param context NB_Context
@param config pointer to NB_AnalyticsEventsConfig object.
@return NB_Error
 */
NB_Error NB_AnalyticsEventsConfigRestore(NB_Context* context,
        NB_AnalyticsEventsConfig* config);

/*! Master Clear Analytics Events Config
@param context NB_Context
@param config pointer to NB_AnalyticsEventsConfig object.
@return NB_Error
 */
NB_Error NB_AnalyticsEventsConfigMasterClear(NB_Context* context,
        NB_AnalyticsEventsConfig* config);


/*! Returns the name of the category
@param category selected category
@return const char*
 */
const char*
NB_AnalyticsErrorCategoryToString(NB_ErrorCategory category);


/*! Returns the category of the error
@param err NB_Error
@return NB_ErrorCategory
 */
NB_ErrorCategory
NB_AnalyticsGetErrorCategory(NB_Error err);


/* @} */
#endif



