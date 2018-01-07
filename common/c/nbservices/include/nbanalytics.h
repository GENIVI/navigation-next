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

    @file     nbanalytics.h
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

#ifndef NBANALYTICS_H_
#define NBANALYTICS_H_

#include "nbgpstypes.h"
#include "navpublictypes.h"
#include "nbplace.h"
#include "palradio.h"
#include "nbmobilecoupons.h"
#include "nbroutesettings.h"
#include "nbextappcontent.h"
#include "nbrouteinformation.h"

/*!
    @addtogroup nbanalytics
    @{
*/

struct CSL_Vector;
/*! @struct NB_Analytics
Analytics object
*/
typedef struct NB_Analytics NB_Analytics;

/*! Origin of place event action.

    Specifies origin of place action.

    @see NB_AnalyticsEventPlace
*/
typedef enum
{
    NB_PAO_List,                    /*!< Action originated from list */
    NB_PAO_Map,                     /*!< Action originated from map */
    NB_PAO_Detail,                  /*!< Action originated from detail */
    NB_PAO_Arrival                  /*!< Action originated from arrival */
} NB_PlaceActionOrigin;

/*! @struct NB_AnalyticsWantAnalyticsConfig
This structure specifies the categories for which the client wants configuration information.
*/

typedef struct NB_AnalyticsWantAnalyticsConfig
{
    nb_boolean wantGoldCategory;             /*!< If present, the client suports
                                          gold events */
    nb_boolean wantPoiCategory;              /*!< If present, the client supports
                                          POI events */
    nb_boolean wantRouteTrackingCategory;    /*!< If present, the client supports
                                          route trakcing events */
    nb_boolean wantGpsProbesCategory;        /*!< If present, the client supports
                                          gps probes events */
    nb_boolean wantFeedbackCategory;         /*!< If present, the client supports
                                          feedback events */
    nb_boolean wantAppErrorsCategory;        /*!< If present, the client supports
                                          application errors events */
    nb_boolean wantWifiProbesCategory;       /*!< If present, the client supports
                                          wifi probes events */
    nb_boolean wantSessionCategory;

    nb_boolean wantSettingsCategory;

    nb_boolean wantShareCategory;

    nb_boolean wantUserActionsCategory;

    nb_boolean wantTransactionCategory;

    nb_boolean wantMapCategory;
    nb_boolean wantCouponCategory;           /*!< If present, the client supports
                                          coupon action events */
    nb_boolean wantNavInvocationCategory;    /*!< If present, the client supports
                                          nav-invocation events */

} NB_AnalyticsWantAnalyticsConfig;

/*! @struct NB_AnalyticsConfig
Analytics Configuration
*/
typedef struct NB_AnalyticsConfig
{
    uint32 maxDataSize; /*!< A parameter in the admin fileset shall specify
                          the maximum number of bytes that can be used for
                          storing collected event data on local device. */

    const char* dir;    /*!< The directory where analytics can store event data
                          so that event data not uploaded to the server can be
                          preserved across application session, and uploaded in
                          the future application sessions. If NULL, the current
                          working directory will be used. */

    const char* timeZone;   /*!< Time zone in which the event took place (optional)  @todo: define format and values */
} NB_AnalyticsConfig;

/*! @struct NB_AnalyticsGoldenCookie
Analytics Golden Cookie
*/
typedef struct NB_AnalyticsGoldenCookie
{
    const char* providerId; /*!< Unique identifier for the feed provider */

    const byte* stateData;  /*!< Opaque state information, contains information
                              required for monetizing the ad for the specific
                              feed provider.  */

    uint32  stateSize;      /*! size of the state information */

} NB_AnalyticsGoldenCookie;

/*! @struct NB_AnalyticsEventPlace
Analytics Event Place
*/
typedef struct NB_AnalyticsEventPlace
{
    nb_boolean  isGpsBased; /*!< Indicates GPS based place, used for search
                              centers, map, and routes.*/

    NB_AnalyticsGoldenCookie* goldenCookie; /*!< Present if event performed on
                                              the place is billable. */
    nb_boolean isPremiumPlacement; /*!< Indicates a premium placement AD. */

    nb_boolean isEnhancedPoi;       /*!< Indicates an Enhanced POI.  */

    const char* id;  /*!< Unique identifier associated with this place. */

    nb_boolean  isSearchQueryEventIdPresent; /* whether Search Query Event ID
                                                is valid*/

    uint32      searchQueryEventId; /* Search Query Event ID. */

    NB_PlaceEventCookie*       placeEventCookie;       /*!< Contains information to be sent
                                                            back to server when reporting an place event */
    uint32                     searchResultsIndex;     /*!< Zero-based index of the place (POI or ad)
                                                            in the search results list. */
    NB_PlaceActionOrigin       origin;                 /*!< Origin of the action performed on the  place. */

} NB_AnalyticsEventPlace;

/*! @struct NB_AnalyticsFeedbackEvent
Analytics Feedback
*/
typedef struct NB_AnalyticsFeedbackEvent
{
    NB_Place* poiPlace;
    NB_Location *originLocation;
    NB_GpsLocation *originGpsLocation;
    NB_Location *destinationLocation;
    NB_GpsLocation *destinationGpsLocation;
    const char* issueType;/*!< Concern issue type:
                                            ADDRESS_INCORRECT
                                            LOCATION_DNE
                                            NAME_INCORRECT
                                            IS_RESIDENCE
                                            PHONE_INCORRECT
                                            ONEWAY_STREET
                                            DIRECTION_INCORRECT
                                            TRAFFIC
                                            OTHER*/
    const char* screenId;/*!< The screen ID, through where report the concern:
                                            poi_details
                                            nav_input
                                            auto_nav_arrival*/
    const char* providerId;
    const char* poiId;
    const char* enteredText;
    uint32 navSessionId;
    uint32 routeRequestEventId;
    uint32 searchQueryEventId;
}NB_AnalyticsFeedbackEvent;

typedef enum NB_AnalyticsActivity
{
    NB_AA_PiggybackOpportunity,
    NB_AA_QuerySent,
    NB_AA_QueryFailure,
    NB_AA_QueryTimeout,
    NB_AA_ReplyReceived,
    NB_AA_ConfigReceived,
} NB_AnalyticsActivity;

/*! @enum NB_AnalyticsEnhancedContentType
*/
typedef enum
{
    NB_AECD_None = 0,
    NB_AECD_Junction,
    NB_AECD_RealisticSign,
    NB_AECD_CityModels

} NB_AnalyticsEnhancedContentType;


// @note: Following functions are deprecated and may not work, please use analyticsmanager
//        to record analytics events!

/*! Create an Analytics object

@param context NB_Context pointer
@param analyticsConfig Configuration that the newly created analytics object
       should use. if NULL, the following default values will be used:
       maxDataSize = 0: no data size limit;
       dir = NULL: current working directory will be used.
@param analytics On success, pointer to NB_Analytics object returned.
@return NB_Error
*/
NB_DEC NB_Error NB_AnalyticsCreate(NB_Context* context, NB_AnalyticsConfig* analyticsConfig, NB_Analytics** analytics);

/*! Destroy a previously created Analytics object

@param context NB_Context pointer
@param analytics A Analytics object created with NB_AnalyticsCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsDestroy(NB_Analytics* analytics);

/*! Return whether the event is a golden event

@param analytics NB_Analytics  pointer
@param name Event name
@returns nb_boolean TRUE if it's a golden event, otherwise FALSE
*/
NB_DEC nb_boolean NB_AnalyticsIsGoldenEvent(NB_Analytics* analytics, const char* name);

/*! Returns whether initial query is sent

@param analytics NB_Analytics  pointer
@returns nb_boolean whether initial query is sent
*/
NB_DEC nb_boolean NB_IsInitialQuerySent(NB_Analytics* analytics);

/*! Return whether the query is in process

@param analytics NB_Analytics  pointer
@returns nb_boolean TRUE if in progress, otherwise FALSE
*/
NB_DEC nb_boolean NB_AnalyticsQueryInProgress(NB_Analytics* analytics);

/*! Return the session threshold time.

Session id beyond this time old session id will expire,

@param analytics NB_Analytics  pointer
@returns uint32 Threshold of session(in seconds)
*/
NB_DEC uint32 NB_AnalyticsGetMaxSessionTime(NB_Analytics* analytics);

/*! Set force upload flag

@param analytics NB_Analytics pointer
@param bForceUpload whether force to upload
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsSetForceUpload(NB_Analytics* analytics, nb_boolean bForceUpload);

/*! Set analytics user data

@param analytics NB_Analytics pointer
@param userData User data pointer
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsSetUserData(NB_Analytics* analytics, void* userData);

/*! Get analytics user data

@param analytics NB_Analytics pointer
@returns userData User data pointer
*/
NB_DEC void* NB_AnalyticsGetUserData(NB_Analytics* analytics);

/*! Return the sorted category array. This category array is sorted by priorities.

This uint32 pointer is a stack array's begin address, it should not be "free"
@param analytics NB_Analytics  pointer
@returns Category array
*/
NB_DEC uint32* NB_AnalyticsGetSortedCategory(NB_Analytics* analytics);

/*! Return the priority array.

This uint32 pointer is a stack array's begin address, it should not be "free"
@param analytics NB_Analytics  pointer
@returns The priority array of categories
*/
NB_DEC uint32* NB_AnalyticsGetCategoryPriority(NB_Analytics* analytics);

/*! Return the max hold time for events

@param analytics NB_Analytics  pointer
@returns The max hold time of events
*/
NB_DEC uint32 NB_AnalyticsDetermineMaxHoldTimeForEvents(NB_Analytics* analytics);

/*! Return the max hold time for events
@param analytics NB_Analytics  pointer
@returns The max hold time of events
*/
NB_DEC uint32 NB_AnalyticsGetMaxHoldTime(NB_Analytics* analytics);

/*! Return the max hold time for billable events

 @param analytics NB_Analytics  pointer
 @returns The max hold time of billable events
 */
NB_DEC uint32 NB_AnalyticsGetMaxBillableHoldTime(NB_Analytics* analytics);

/*! Return the max data age. beyond this age, old data will be cleaned out of queue/file/database

@param analytics NB_Analytics  pointer
@returns The max data age
*/
NB_DEC uint32 NB_AnalyticsGetMaxDataAge(NB_Analytics* analytics);

/*! Return the max byte limitation  of analytics event

@param analytics NB_Analytics  pointer
@returns uint32 max byte limitation  of analytics event, configured by server.
*/
NB_DEC uint32 NB_AnalyticsGetMaxEventSize(NB_Analytics* analytics);

/*! Get the max data size that persistent storage can hold

@param analytics NB_Analytics  pointer
@returns uint32 max data size of persistent storage
*/
NB_DEC uint32 NB_AnalyticsGetMaxDataSize(NB_Analytics* analytics);

/*! Get route tracking interval

@param analytics NB_Analytics  pointer
@returns uint32 The route tracking interval
*/
NB_DEC uint32 NB_AnalyticsGetRouteCollectionInterval(NB_Analytics* analytics);

/*! Get GPS sample rate

@param analytics NB_Analytics  pointer
@returns uint32 The GPS sample rate
*/
NB_DEC uint32 NB_AnalyticsGetGPSSampleRate(NB_Analytics* analytics);

/*! Set Data Roaming Status

This function should be called when data roaming status changes.
@param context NB_Context
@param bRoaming whether the device is in data roaming
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsSetDataRoamingStatus(NB_Context* context,
        nb_boolean bRoaming);


/*! Analytics processing routine

This function should be called periodically, at least once a minute, and
preferably when the client is idle.

@param context NB_Context
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsProcess(NB_Context* context);

/*! Analytics processing routine

This function is called to send events in a vector.

CSL_Vector need to be not null, and it will be released inside this function.

@param context NB_Context
@param events CSL_Vector pointer, contains all events tps that needed to be sent.
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsSendEvents(NB_Context* context, struct CSL_Vector* events);

/*! Force Uploading Events

This function can be called to force uploading all collected events. It should
be called prior to the arrival maneuver announcement in a navigation session,
when a navigation session is ended or paused, or when a Follow Me Map session is
ended.

@param context NB_Context
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsForceUpload(NB_Context* context);

/*! Master Clear all saved events and configuration

This function removes all saved events and configuration.

@param context NB_Context
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsMasterClear(NB_Context* context);


/*! enable/disable GPS probes collection.

This function shall be called according to User preference. If user is opting
out GPS probes collection, this function should be called with FALSE,
otherwise, this function should be called with TRUE.

By default, GPS probes is enabled.

@param context NB_Context
@param bEnable whether to enable/disable GPS probes.
               TRUE:   enable GPS probes
               FALSE: disable GPS probes
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsEnableGpsProbes(NB_Context* context,
        nb_boolean bEnable);

/*! Specify the categories for which the client wants configuration information.

@param context NB_Context
@param wantAnalyticsConfig specify the categories for which the client wants configuration information.
@returns NB_Error
*/

NB_DEC NB_Error
NB_AnalyticsSetWantAnalyticsConfig(NB_Context* context, NB_AnalyticsWantAnalyticsConfig* wantAnalyticsConfig);

/*! Add file system error event

This function should be invoked when a file error is occured

@param context NB_Context
@param errorCode the numeric code of the error
@param detailedCode technical summary of the problem
@param detailedDescription details that will help debugging the error
@param navSessionID navigation session ID, NULL if not available
@param fileName file name, required
@returns NB_Error
*/
NB_DEC NB_Error NB_AnalyticsAddAppFileSystemErrorEvent(NB_Context* context,
                                                NB_Error errorCode,
                                                const char* detailedCode,
                                                const char* detailedDescription,
                                                const uint32* navSessionID,
                                                const char* fileName
                                                );

/* @} */

#endif

