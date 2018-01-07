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
 
 @file     analyticsmanager.h
 
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

/*!
 @{
 */


#ifndef __NBSERVICES__ANALYTICSMANAGER__
#define __NBSERVICES__ANALYTICSMANAGER__

#include <string>
extern "C" {
#include "nbcontext.h"
#include "nbanalytics.h"
}
#include "smartpointer.h"
#include "CommonTypes.h"
#include "analyticsengine.h"

namespace protocol
{
    class AnalyticsEvent;
}

namespace nbcommon
{
class AnalyticsManagerImpl;
/*! analyticsmanager interface */
class AnalyticsManager
{
public:
    /*! night mode */
    typedef enum
    {
        NM_Invalid = 0, /*!< not applicable. */
        NM_Automatic,   /*!< automatic */
        NM_ON,
        NM_OFF
    } NightMode;

    /*! distance display */
    typedef enum
    {
        DD_Invalid = 0,  /*!< not applicable. */
        DD_FIMI,         /*!< ft/mi */
        DD_MKM           /*!< m/km */
    } DistanceDisplay;

    /*! Option State */
    typedef enum
    {
        OS_Inavlid = 0,  /*!< not applicable. */
        OS_ON,           /*!< on */
        OS_OFF           /*!< off */
    } OptionState;

    /*! Warning speed. */
    typedef enum
    {
        WS_Invalid = 0, /*!< not applicable. */
        WS_1MPH,        /*!< 1 mph over. */
        WS_5MPH,        /*!< 5 mph over. */
        WS_10MPH,       /*!< 10 mph over. */
        WS_15MPH,       /*!< 15 mph over. */
        WS_1KPH,        /*!< 1 kph over. */
        WS_5KPH,        /*!< 5 kph over. */
        WS_10KPH,       /*!< 10 kph over. */
        WS_15KPH        /*!< 15 kph over. */
    } WarningSpeed;

    /*! Vehicle mode . */
    typedef enum
    {
        VM_Invalid = 0,
        VM_Auto,
        VM_Bycycle,
        VM_Pedestrian,
        VM_Truck
    } VehicleMode;

    /*! User action */
    typedef enum
    {
        UA_Invalid = 0,
        UA_SearchBoxSelected,
        UA_SessionCleared,
        UA_NavTripOverview,
        UA_NavTurnList
    } UserAction;

    /*! application general setting */
    typedef struct
    {
        NightMode mode;
        DistanceDisplay distance;
    } GeneralSetting;

    /*! application navigation setting */
    typedef struct
    {
        OptionState hightWaySigns;
        OptionState speedLimitSigns;
        OptionState speedAlerts;
        OptionState speedWarningTone;
        WarningSpeed warningSpeed;
    } NavigationSetting;

    /*! application route setting */
    typedef struct
    {
        VehicleMode mode;
        OptionState avoidTolls;
        OptionState avoidCarPool;
        OptionState avoidFerries;
        OptionState avoidHighWays;
    } RouteSetting;

    /*! map setting. */
    typedef struct
    {
        OptionState trafficLayer;
        OptionState satelliteLayer;
        OptionState dopperLayer;
    } MapSetting;

    /*! analytics event place. */
    typedef struct
    {
        nb_boolean            isGpsBased;                 /*!< Indicates GPS based place, used for search centers, map, and routes.*/
        std::string           goldenCookie;                     /*!< Present if event performed on the place is billable. */
        nb_boolean            isPremiumPlacement;         /*!< Indicates a premium placement AD. */
        nb_boolean            isEnhancedPoi;              /*!< Indicates an Enhanced POI.  */
        std::string           id;                         /*!< Unique identifier associated with this place. */
        nb_boolean            isSearchQueryEventIdPresent;/* whether Search Query Event ID is valid*/
        uint32                searchQueryEventId;         /* Search Query Event ID. */
        std::string           placeEventCookie;           /*!< Contains information to be sent back to server when reporting an place event */
        std::string           placeAttributeCookie;       /*!< contain information to be sent back to server */
        uint32                searchResultsIndex;         /*!< Zero-based index of the place (POI or ad) in the search results list. */
        std::string           origin;                     /*!< Origin of the action performed on the  place.*/
    } AnalyticsEventPlace;

    /*!
     Analytics Feedback
     */
    typedef struct
    {
        NB_Place* poiPlace;
        NB_Location *originLocation;
        NB_GpsLocation *originGpsLocation;
        NB_Location *destinationLocation;
        NB_GpsLocation *destinationGpsLocation;
        std::string issueType;/*!< Concern issue type:
                               ADDRESS_INCORRECT
                               LOCATION_DNE
                               NAME_INCORRECT
                               IS_RESIDENCE
                               PHONE_INCORRECT
                               ONEWAY_STREET
                               DIRECTION_INCORRECT
                               TRAFFIC
                               OTHER*/
        std::string screenId;/*!< The screen ID, through where report the concern:
                              poi_details
                              nav_input
                              auto_nav_arrival*/
        std::string providerId;
        std::string poiId;
        std::string enteredText;
        uint32 navSessionId;
        uint32 routeRequestEventId;
        uint32 searchQueryEventId;
    } AnalyticsFeedbackEvent;

    virtual ~AnalyticsManager(){}
    /*! get analyticsmanager

     @return pointer to analyticsmanager interface
     */
    static AnalyticsManager* GetAnalyticsManager(NB_Context* nbContext);

    /*! add an analytics event 

     @param event this event is inherit from AnalyticsEvent, analytics use ToXML generate xml string and send it to analytics engine.
     @return return NE_OK if success add a event
     */
    virtual NB_Error AddEvent(const shared_ptr<protocol::AnalyticsEvent> event) = 0;

    /*! Add GPS error event

     This function should be invoked when a GPS error is occured

     @param invocation InvocationContext
     @param errorCode the numeric code of the error
     @param detailedCode technical summary of the problem
     @param detailedDescription details that will help debugging the error
     @param gpsMode can take values: "single", "tracking". Required, can't empty
     @param gpsFixType can take values: "fast", "normal", "accurate". can't empty
     @returns NB_Error
     */
    virtual NB_Error AddAppGpsErrorEvent(const InvocationContext& invocation,
                                         NB_Error errorCode,
                                         const std::string& detailedCode,
                                         const std::string& detailedDescription,
                                         const std::string& gpsMode,
                                         const std::string& gpsFixType
                                         ) = 0;

    /*! Add Setting change event *

     This function should be call after application setting changed

     @param invocation InvocationContext
     @param generalSetting general setting
     @param navSetting navigation setting
     @param routeSetting route setting
     @param mapSetting map setting
     */
    virtual NB_Error AddSettingChangeEvent(const InvocationContext& invocation,
                                           const GeneralSetting& generalSetting,
                                           const NavigationSetting& navSetting,
                                           const RouteSetting& routeSetting,
                                           const MapSetting& mapSetting) = 0;

    /*! Add feedback event

     This function can be used to report a concern by end user.

     @param invocation InvocationContext
     @param feedback Feedback event
     @returns NB_Error
     */
    virtual NB_Error AddFeedbackEvent(const InvocationContext& invocation,
                                      const AnalyticsFeedbackEvent& feedback) = 0;

    /*! Add session start event 

     this function should be called when a new session is start.

     @param invocation InvocationContext
     @param location gps location
     */
    virtual NB_Error AddSessionStartEvent(const InvocationContext& invocation,
                                          const NB_GpsLocation& location) = 0;

    /*! Add user action event

     this function should be called when a user do a special action

     @param invocation InvocationContext
     @param action user action
     */
    virtual NB_Error AddUserActionEvent(const InvocationContext& invocation,
                                        UserAction action) = 0;

    /*! Add Share event

     this function should be call when share

     @param invocation InvocationContext
     @param subject subject
     @param body share body
     @param url share url
     @param urlText url text
     @param signature 
     @param channel channel used for sharing
     @param place the place associated with the event
     */

    virtual NB_Error AddShareEvent(const InvocationContext& invocation,
                                   const std::string& subject,
                                   const std::string& body,
                                   const std::string& url,
                                   const std::string& urlText,
                                   const std::string& signature,
                                   uint channnel,
                                   const AnalyticsEventPlace* place) = 0;

    /*! Send a notice for an one-shot event

     Send an one-shot event notice(e.g. view is pressed) to analytics engine to reset the session-id refresh timer
     This function is thread-safe

     @returns nothing
     */
    virtual void NoticeAlive() = 0;

    /*! Send a notice for an event which last for a period of time

     Send a notice for period of time event(e.g. a navigation) to
     analytics engine to make session id refresh timer never times out or begins to recount.
     This function is thread-safe

     @param alive:bool. true if one action begins and session id refresh timer will never time out.
     false should be passed in when this action ends. if all actions are ended, session-id refresh timer begins to work.
     @returns nothing
     */
    virtual void KeepAlive(bool alive) = 0;

    /*! Set analytics callback function

     This listener should be registered when user code want to be noticed by some special events.
     @param listener The analytics listener
     @returns NB_Error, returns an error code
     */
    virtual NB_Error SetAnalyticsListener(AnalyticsListener* listener) = 0;

private:
    static AnalyticsManagerImpl* m_managerImpl;
};
}

#endif /* defined(__nbservices__analyticsManager__) */

/*!
 @}
 */
