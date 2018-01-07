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
 
 @file     analyticsmanagerimpl.h
 
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


#ifndef __NBSERVICES__ANALYTICSMANAGERIMPL__
#define __NBSERVICES__ANALYTICSMANAGERIMPL__

#include "analyticsmanager.h"
#include "analyticsengine.h"
#include "NBProtocolAnalyticsEvent.h"
#include "contextbasedsingleton.h"
#include "TpsElement.h"

namespace nbcommon
{
class AnalyticsManagerImpl : public AnalyticsManager
{
public:
    NB_Error Initialize();
    /*! add an analytics event

     @param event this event is inherit from AnalyticsEvent, analytics use ToXML generate xml string and send it to analytics engine.
     @return return NE_OK if success add a event
     */
    virtual NB_Error AddEvent(const shared_ptr<protocol::AnalyticsEvent> event);

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
                                         );

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
                                           const MapSetting& mapSetting);

    /*! Add feedback event

     This function can be used to report a concern by end user.

     @param invocation InvocationContext
     @param feedback Feedback event
     @returns NB_Error
     */
    virtual NB_Error AddFeedbackEvent(const InvocationContext& invocation,
                                      const AnalyticsFeedbackEvent& feedback);

    /*! Add session start event

     this function should be called when a new session is start.

     @param invocation InvocationContext
     @param location gps location
     */
    virtual NB_Error AddSessionStartEvent(const InvocationContext& invocation,
                                          const NB_GpsLocation& location);

    /*! Add user action event

     this function should be called when a user do a special action

     @param invocation InvocationContext
     @param action user action
     */
    virtual NB_Error AddUserActionEvent(const InvocationContext& invocation,
                                        UserAction action);

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
                                   const AnalyticsEventPlace* place);

    /*! Send a notice for an one-shot event

     Send an one-shot event notice(e.g. view is pressed) to analytics engine to reset the session-id refresh timer
     This function is thread-safe

     @returns nothing
     */
    virtual void NoticeAlive();

    /*! Send a notice for an event which last for a period of time

     Send a notice for period of time event(e.g. a navigation) to
     analytics engine to make session id refresh timer never times out or begins to recount.
     This function is thread-safe

     @param alive:bool. true if one action begins and session id refresh timer will never time out.
     false should be passed in when this action ends. if all actions are ended, session-id refresh timer begins to work.
     @returns nothing
     */
    virtual void KeepAlive(bool alive);

    virtual NB_Error SetAnalyticsListener(AnalyticsListener* listener);

private:
    AnalyticsManagerImpl(NB_Context* context);

    /*! This function is required by ContextBasedSingleton*/
    NB_Context* GetContext() const
    {
        return m_context;
    }
private:
    friend class ContextBasedSingleton<AnalyticsManagerImpl>;
private:
    /*! Initialize some members in ccc thread, else app will crash*/
    static void InitializeInCCCThread(PAL_Instance* pal, void* userData);
    /*! Add event at ccc thread. */
    static void AddEventInCCCThread(PAL_Instance* pal, void* userData);
    /*! add listener in ccc thread. */
    static void AddAnalyticsListenerInCCCThread(PAL_Instance* pal, void* userData);
    /*! keep alive in ccc thread. */
    static void KeepAliveInCCCThread(PAL_Instance* pal, void* userData);
    /*! notice alive in ccc thread. */
    static void NoticeAliveInCCCThread(PAL_Instance* pal, void* userData);
    std::string GetSessionId();
    NB_Error GetPlace(const NB_Place& place, shared_ptr<protocol::Place>& proPlace);
    NB_Error GetLocation(const NB_Location& location, shared_ptr<protocol::Location>& proLocation);
    NB_Error GetAnalyticsPlace(const AnalyticsEventPlace& place, shared_ptr<protocol::AnalyticsEventPlace>& analyticsPlace);
    NB_Error GetAnalyticsMetadata(const std::string& name, const std::string& category, protocol::AnalyticsEventMetadataSharedPtr& metadata);
    NB_Error GetInvocation(const InvocationContext& context, shared_ptr<protocol::InvocationContext>& invocation);
    NB_Error SendAnalyticsEvent(const InvocationContext& context, const protocol::AnalyticsEventMetadataSharedPtr& metadata, shared_ptr<protocol::AnalyticsEvent> analytics);
    protocol::TpsElementPtr CreateTpsElementByBase64Data(const std::string& serializedData);
private:
    NB_Context* m_context;
    bool        m_inited;
    uint32      m_sessionId;
    uint32      m_id;
};
}

#endif /* defined(__nbservices__analyticsManagerImpl__) */

/*!
 @}
 */