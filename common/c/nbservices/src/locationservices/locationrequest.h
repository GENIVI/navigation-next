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

    @file     locationrequest.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    LocationRequest abstract base class declaration

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma once

extern "C" {
  #include <nbexp.h>
  #include <nbcontext.h>
  #include "nberror.h"
  #include <nbhandler.h>
  #include <nblocationhandler.h>
  #include "paltimer.h"
}

#include "base.h"
#include "locationcontext.h"
#include "locationrequestmanager.h"

/*! Class LocationRequest - abstract base class for all location requests to NavBuilder

    Most of the requests' common code is placed here.
*/
struct CachedLocationReply : public Base
{
    NB_LS_LocationCallback  callback;
    void*                 userData;
    NB_LS_Location          location;
    NB_LocationContext*   context;
};

class LocationRequest : public Base
{
public:
    LocationRequest(NB_LocationContext* locationContext);
    virtual ~LocationRequest();

    /*! Set the callback and start the request
        @return NB_Error code
    */
    virtual NB_Error RequestStart(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType = NBFixType_Normal);

    /*! Query the status of the request
        @return NB_Error code
    */
    NB_Error RequestInProgress(bool* inProgress);

    /*! Cancel the request
        @return NB_Error code
    */
    NB_Error RequestCancel(NB_LS_LocationCallback callback, void* appData);

    /*! Get location state
    */
    virtual NB_Error GetState( NB_LS_LocationState * state ) = 0;

    /*! Check whether last fix location meets the user criteria
    */
    virtual bool IsCachedLocationValid(NB_LS_FixType fixType);

    /*! Copy location utility function
    */
    static void CopyLocation(NB_LS_Location& dest, const NB_LS_Location& src);

    /*! Check if returned error is a critical error from IDEN
    */
    static bool IsIdenError(NB_Error err);

    /*! Check for presence of this particular request (callback/info pair) in the request queue
        @return nb_boolean
    */
    virtual nb_boolean AlreadyStarted(NB_LS_LocationCallback callback, void* applicationData)
    {
        return m_queue.CheckForPresence(callback, applicationData);
    }


protected:

    /*! Generic request functionality
    */
    virtual NB_Error StartLocationRequest() = 0;
    virtual bool IsInProgress() = 0;
    virtual void CancelLocationRequest() = 0;
    virtual NB_LS_Location* RetrieveLocation() = 0;
    virtual int GetSpecificTimeout(NB_LS_FixType fixType) = 0;
    virtual void LocationCallback(NB_Error error);

    bool CheckCache(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType);
    void UpdateCache(const NB_LS_Location& newLocation);
    NB_Error ScheduleRequest(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType);
    void SetTimeout(int timeout);
    bool IsLastFixValid(int maxAge); // max age is in seconds
    void TimerCallbackResult();
    static void StaticRequestTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
    static void StaticCacheCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

    LocationRequestManager   m_queue;

    NB_LocationContext*        m_locationContext;
    int                         m_timeout;

    NB_LS_Location                m_lastFixLocation;
    NB_LS_LocalTime               m_lastFixTimestamp;

    NB_LS_FixType                 m_fixType;
};

/*! @} */
