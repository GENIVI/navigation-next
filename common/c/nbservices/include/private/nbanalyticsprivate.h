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

    @file     nbanalyticspriviate.h
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

#ifndef NBANALYTICSPRIVATE_H_
#define NBANALYTICSPRIVATE_H_

#include "data_analytics_event.h"

/*!
    @addtogroup nbanalyticsprivate
    @{
*/

/*! @enum NB_AnalyticsEventsPriority
Analytics Events Priority
*/
typedef enum NB_AnalyticsEventsPriority
{
    NB_AEP_Invalid = 0,
    NB_AEP_Lowest = 1,
    NB_AEP_Highest = 10,
} NB_AnalyticsEventsPriority;

/*! @enum NB_AnalyticsEventsCategory
Analytics Events Category
*/
typedef enum NB_AnalyticsEventsCategory
{
    NB_AEC_Gold,
    NB_AEC_Poi,
    NB_AEC_RouteTracking,
    NB_AEC_GpsProbes,
    NB_AEC_AppError,
    NB_AEC_WifiProbes,
    NB_AEC_Session,
    NB_AEC_Settings,
    NB_AEC_Share,
    NB_AEC_UserAction,
    NB_AEC_Transaction,
    NB_AEC_Maps,
    NB_AEC_Feedback,

    NB_AEC_MAX
} NB_AnalyticsEventsCategory;

/*! @enum NB_AnalyticsEventsStatus
Analytics Events Status
*/
typedef enum NB_AnalyticsEventsStatus
{
    NB_AES_Invalid = 0,
    NB_AES_Created,
    NB_AES_Uploading,
    NB_AES_Uploaded,
    NB_AES_RemovedForAge,
    NB_AES_RemovedForSpace,
    NB_AES_RemovedForFailure,
    NB_AES_RemovedForPriority, /*due to config change, priority changed to 0 */
    NB_AES_Saved,    /* saved for next session */
    NB_AES_Restored, /* restored from previous session */
    NB_AES_MasterCleared, /* removed by master clear*/
    NB_AES_RemovedForUserOptOut, /* User Opted out gps probes collection */
} NB_AnalyticsEventsStatus;

/*! get the NB_Context */
NB_Context* NB_AnalyticsGetContext(NB_Analytics* pThis);

/*! Get the category the event belongs to*/
NB_AnalyticsEventsCategory NB_AnalyticsGetEventCategory(NB_Analytics* pThis,
        data_analytics_event* event);

/* log event status */
void NB_AnalyticsLogEventStatus(NB_Analytics* pThis,
        data_analytics_event* event, NB_AnalyticsEventsStatus evenStatus);

/* @} */
#endif

