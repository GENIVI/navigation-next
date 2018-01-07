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

@file     analyticsengineimpl.h

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

#ifndef __ANALYTICS__ENGINE__IMPL_H_
#define __ANALYTICS__ENGINE__IMPL_H_

#include "nbanalytics.h"
#include "TpsElement.h"
#include "paltimer.h"
#include "nberror.h"
#include "smartpointer.h"
#include "nbanalyticstestprotected.h"
#include "analyticsdbstorage.h"
#include "analyticsengine.h"
#include <vector>

struct NB_Context;
struct NB_Analytics;
struct PAL_Instance;

namespace nbcommon
{

class FileOperatingTaskManager;
class AnalyticsLibXmlParser;

struct AnalyticsEventCollectLimitation
{
    AnalyticsEventCollectLimitation()
    {
        gpsLastSendTime = 0;
        routeLastSendTime = 0;
    }

    ~AnalyticsEventCollectLimitation(){}
    uint32 gpsLastSendTime;
    uint32 routeLastSendTime;

};

/*!@class AnalyticsEngineImpl

\brief The implementation of analytics engine

*/
class AnalyticsEngineImpl
{
public:
    enum UploadReason
    {
        UR_FORCE,    /*! force upload*/
        UR_TIMER,    /*! timer upload*/
        UR_PIGGYBACK /*! piggytail upload*/
    };

public:
    AnalyticsEngineImpl();
    ~AnalyticsEngineImpl();

    /*! Initialize this class. this should be called before any other functionalities are used.

    @param context NB_Context
    @returns NB_Error Returns NE_OK if initialize successful, else returns an error code.
    */
    NB_Error Initialize(NB_Context* context, const std::string& dbPath, const std::string& schema);

    /*! Validate a string with a default schema.

    @param event: An input  string.
    @returns NB_Error if event format correct, returns NE_OK, otherwise returns one error code.
    */
    NB_Error ValidateString(const std::string& event);

    /*! Add an event to event Queue/database/file/other persistent store media.

    Currently we use database
    @param event The std::string event.
    @returns NB_Error Storage will be executed in CCC I/O thread, returns NE_OK if this task has been pushed into
    that thread successfully.
    */

    NB_Error AddEvent(const std::string& event);

    /*! Convert a string to tps format

    @param event The event string
    @param ptr the TpsElementPtr structure
    @returns NB_Error Returns NE_OK if conversion successful,otherwise returns one error code.
    */
    NB_Error StringToTps(const std::string& event, protocol::TpsElementPtr& ptr);

    /*! Upload event right now

    @param forceUpload Whether upload without checking.
    @returns NB_Error Whether this task is put into ccc queue successfully
    */
    NB_Error Upload(UploadReason reason);

    /*! Remove All Analytics Events from storage

    */
    NB_Error RemoveAllEvents();

    /*! Set analytics module data roaming status

    */
    NB_Error SetDataRoaming(bool bRoaming);

    /*! Set analytics listener

    */
    NB_Error SetAnalyticsListener(AnalyticsListener* listener);

    /*! Refresh session timestamp immediately*/
    NB_Error NoticeAlive();

    /*! Stop or recount the session counting*/
    NB_Error KeepAlive(bool alive);

    /*! Get current session id*/
    std::string GetSessionId() const;

    const AnalyticsWifiConfig* GetAnalyticsWifiConfig();

private:

    /*! Send all events to server

    @param eventsQueue A vector contains the events needed to be uploaded, this vector is not const,
    sent events will be removed from this vector
    @param AnalyticsCallback analytics callback function, this function will be called if send has some responces.
    @param sendedQueue A vector contains the sent events
    @returns NB_Error Returns NE_OK if sent successful, otherwise returns one error code.
    */
    NB_Error SendToServer(std::vector<AnalyticsEvent>& eventsQueue,
                          NB_AnalyticsCallbacks analyticsCallback,
                          std::vector<AnalyticsEvent>& sendedQueue);

    /*! Separate Meta data from event body and save event into persistent storage in ccc queue.

    @returns If Separate successfully, returns NE_OK, else returns other error code.
    */
    NB_Error SeparateMetadata(const std::string& event);

    /*! Judge if upload is allowed by now

    @param reason Upload reason
    @param oldestBillableTimeStamp Oldest billable events timestamp
    @param oldestNonBillableTimeStamp Oldest Non-billable events timestamp
    @returns If upload is allowed
    */
    bool UploadAllowed(UploadReason reason, int64 oldestBillableTimeStamp, int64 oldestNonBillableTimeStamp);

    /*! Called by the timer, used to update session id
    */
    void UpdateSession();

    /*! Timer function. Triggered when timer is fired, call @see updateSession to update session id.
    */
    static void SessionTimeUp(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

    /*! Called by @see TimeUp, used to upload events to server in CCC thread.

    This is a callback function used in CCC task queue.
    */
    static void SendToServerInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Pal timer task callback function. used to upload events to server per period time through @see SendToServerInCCCQueue

    */
    static void SendTimeUp(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

    /*!User callback of analytics events

    Some resources and memories will be released here or in RemoveObsoleteTask called in this function.
    */
    static void AnalyticsActivityCallback(NB_Analytics* analytics, void* userData, NB_AnalyticsActivity activity);

    /*! Called by @see AddEvent, used to separate metadata information from event string in CCC queue.

    This is a callback function used in CCC task queue.
    */
    static void SeparateMetadataInCCCQueue(PAL_Instance* pal, void* userData);

    /*! The callback function of @see AnalyticsTask, called when task finished

    @param id the enumeration of task type
    @param error the error number generated when task executes
    @param a parameter passed in to do some extra works
    */
    static void TaskCallback(int id, NB_Error error, void* param);

    /*! Begin a read query in ccc queue*/
    static void BeginSendQueryInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Revert Send flag in ccc thread*/
    static void FinishQueryInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Called in analytics uploading callback function

    Analytics callback function is executed in send thread, so all works must be moved to ccc main thread, this is this
    function's main usage
    */
    static void AnalyticsCallBackInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Executed in ccc main thread to master clear configurations*/
    static void FinishMasterClearInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Begin a master clear in ccc queue*/
    static void BeginClearQueryInCCCQueue(PAL_Instance* pal, void* userData);

    /*! Begin to check upload condition*/
    static void CheckUploadInCCCQueue(PAL_Instance* pal, void* userData);
private:

    /*! A enumeration to represent class status*/
    enum ClassStatus
    {
        CS_CONSTRUCTED, /*! Constructed but not initialized*/
        CS_INITIALIZED, /*! Initialized*/
        CS_DESTRUCTED   /*! Destructed*/
    };

private:

    friend class ReadTask;
    friend class RemoveObsoleteTask;
    friend class StoreTask;
    friend class ClearTask;
    friend class CheckUploadTask;
    friend class AnalyticsEngine;

    NB_Context*                                         m_context;            /*! NB_context handler*/
    NB_Analytics*                                       m_analytics;          /*! NB_Analytics handler*/
    PAL_Instance*                                       m_pal;                /*! PAL_Instance handler*/
    shared_ptr<nbcommon::FileOperatingTaskManager>      m_manager;            /*! File operation queue handler*/
    shared_ptr<AnalyticsLibXmlParser>                   m_parser;             /*! Event Parser handler*/
    shared_ptr<AnalyticsDBStorage>                      m_storage;            /*! Persistent Storage handler*/
    bool                                                m_readTaskProcessing; /*! If a read task is processing, new read task will directly return*/
    bool                                                m_clearTaskProcessing;/*! Master Clear task is processing, no add or read task should be executed*/

    /*!This class has been initialized or not
    CS_CONSTRUCTED:Constructed but not initialized
    CS_INITIALIZED:Initialized
    CS_DESTRUCTED: Destructed
    @see ClassStatus
    */
    shared_ptr<int>                                     m_classStatus;
    int32                                               m_refCount;           /*!Session's reference count, until it's zero, session id updated again*/
    uint32                                              m_sessionTime;        /*!Current session timestamp*/
    std::string                                         m_sessionId;          /*!Session id*/
    AnalyticsListener*                                  m_sessionListener;    /*! Session timeout callback function*/
    AnalyticsWifiConfig                                 m_wifiConfig;         /*! Wifi config*/
    AnalyticsEventCollectLimitation                     m_collectLimitation;  /*! Some events have collect limitations*/
    uint32                                              m_lastSendFailTime;   /*! Last send failed timestamp*/
};

}

#endif //__ANALYTICS__ENGINE__IMPL_H_

/*!

    @}

*/
