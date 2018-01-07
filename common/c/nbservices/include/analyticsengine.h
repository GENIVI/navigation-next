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

/*! --------------------------------------------------------------------------

    @file     AnalyticsEngine.h

    \brief A class that contains public interface of CCC-analytics module
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

#ifndef __ANALYTICS_ENGINE_H_
#define __ANALYTICS_ENGINE_H_

#include <string>
#include "paltypes.h"
#include "paltimer.h"
#include "nberror.h"

/*!
    @{
*/

template <class T>
class ContextBasedSingleton;

struct NB_Context;

namespace nbcommon
{

struct AnalyticsWifiConfig
{
    uint32  max_horizontal_uncertainty; /*!< Maximum horizontal uncertainty in meters allowed for the GPS fix
                                          associated with a set of Wi-Fi probes. */

    uint32  max_speed;                  /*!< Maximum allowed device speed in m/sec as reported in the GPS fix.
                                          Fixes with speed higher than this number shall be skipped. */

    uint32  collection_interval;        /*!< Minimum time in seconds allowed between two consecutive Wi-Fi probe
                                          collections. */

    uint32  minimum_distance_delta;     /*!< Minimum distance in meters from previous Wi-Fi probe collection point.
                                          If the distance is less than this number, then skip the collection until
                                          the next time slot. */
};

class AnalyticsEngineImpl;

/*! @class AnalyticsListener

\brief The Listener class of analytics
*/
class AnalyticsListener
{
public:
    /*! The analytics session created callback function type

    This function will be called in CCC thread, so DO NOT do any works that is not belong to ccc thread!
    */
    virtual void OnSessionCreated(const std::string& oldSessionId, const std::string& newSessionId) = 0;
};

/*! @class AnalyticsEngine

\brief The wrapper class of CCC-analytics module, all calls about analytics events from JNI or LTK should begin from this class.
*/
class AnalyticsEngine
{
public:
    /*! This is used to get the singleton of this class

    Please DO NOT keep this shared_ptr, GetInstance when you need to use it, or there will be
    an error if NB_Context has destroyed and you are still holding it
    @param context The NB_Context handler
    @returns The AnalyticsEngine pointer
    */
    static AnalyticsEngine* GetInstance(NB_Context* context);

    /*! Initialize the analytics wrapper class.

    This function should be called before any events passed in
    This function should only be called only once

    @param dbPath The path of the database
    @param schema The xml schema body as a string(it's not a path!)
    @returns NB_Error Returns internal error's error code, if successful, returns NE_Ok.
    */
    NB_Error Initialize(const std::string& dbPath, const std::string& schema);

    /*! Add an analytics event

    All analytics events are logged here. Events will be validated in this function,
    and then be parsed and converted into tps format in CCC thread. Currently event is
    xml format, and contains two parts:
    (fake code)
    <event>
    <metadata name= type= />
    <tpsbody>
    ...
    </tpsbody>
    </event>
    metadata contains basic information that describes this event,tps part is the real event body.
    This function is thread-safe

    @param event: The event body
    @returns NB_Error, Returns internal error's error code. If validation and adding to another thread is successful, returns NE_Ok.
    */
    NB_Error AddAnalyticsEvent(const std::string& event);

    /*! Send a notice for an one-shot event

    Send an one-shot event notice(e.g. view is pressed) to analytics engine to reset the session-id refresh timer
    This function is thread-safe

    @returns NB_Error an error code
    */
    NB_Error NoticeAlive();

    /*! Send a notice for an event which last for a period of time

    Send a notice for period of time event(e.g. a navigation) to
    analytics engine to make session id refresh timer never times out or begins to recount.
    This function is thread-safe

    @param alive:bool. true if one action begins and session id refresh timer will never time out.
    false should be passed in when this action ends. if all actions are ended, session-id refresh timer begins to work.
    @returns NB_Error an error code
    */
    NB_Error KeepAlive(bool alive);

    /*! Get a session's id

    This function is thread-safe

    @returns The current session id.
    */
    std::string GetSessionId();

    /*! Clear the Event storage.
    */
    NB_Error MasterClear();

    /*! Upload analytics events right now

    @returns NB_Error If put upload into ccc queue ok, returns NE_OK, else returns other error code.
    */
    NB_Error ForceUpload();

    /*! Set data roaming status

    @param bRoaming roaming status
    @returns NB_Error, the return status of set data roaming operation
    */
    NB_Error SetDataRoaming(bool bRoaming);

    /*! Set analytics callback function

    This listener should be registered when user code want to be noticed by some special events.
    @param listener The analytics listener
    @returns NB_Error, returns an error code
    */
    NB_Error SetAnalyticsListener(AnalyticsListener* listener);

    /*! Get Wifi config

    Get this config everytime you want to use it, do not keep it in your own memory!
    @returns const AnalyticsWifiConfig* analytics wifi config
    */
    const AnalyticsWifiConfig* GetAnalyticsWifiConfig() const;

private:
    AnalyticsEngine(NB_Context* context);
    ~AnalyticsEngine();

    /*! This function is required by ContextBasedSingleton*/
    NB_Context* GetContext() const
    {
        return m_context;
    }

private:
    friend class ContextBasedSingleton<AnalyticsEngine>;

private:
    AnalyticsEngineImpl*                   m_impl;               /*!The implementation class of this analytics engine*/
    NB_Context*                            m_context;            /*! NB_Context handler*/
};

}

#endif // __ANALYTICS_ENGINE_H_

/*!
 * @}
*/
