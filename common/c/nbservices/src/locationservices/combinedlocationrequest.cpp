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

    @file     CombinedLocationRequest.cpp
    @date     05/17/2010
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    CombinedLocationRequest class implementation

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


#include "locationcontext.h"
#include "combinedlocationrequest.h"
#include "gpslocationrequest.h"
#include "networklocationrequest.h"
#include "emulocationrequest.h"

extern "C" {
  #include "nbqalog.h"
}

CombinedLocationRequest::CombinedLocationRequest(NB_LocationContext* locationContext, NB_Context* nbContext)
    : m_locationContext(locationContext),
      m_networkRequest(NULL),
      m_gpsRequest(locationContext->m_gpsLocationRequest),
      m_callback(NULL),
      m_userData(NULL),
      m_gpsReqComplete(FALSE),
      m_networkReqComplete(FALSE),
      m_gpsReqError(NE_GPS),
      m_networkReqError(NE_GPS)
{
    nsl_memset(&m_gpsLocation, 0, sizeof(m_gpsLocation));
    nsl_memset(&m_networkLocation, 0, sizeof(m_networkLocation));

    m_networkRequest = new NetworkLocationRequest(locationContext, nbContext);

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Combined request instance created", 1));
}

CombinedLocationRequest::~CombinedLocationRequest(void)
{
    RequestCancel();

    delete m_networkRequest;
    m_networkRequest = NULL;
}

NB_Error CombinedLocationRequest::RequestStart(NB_LS_LocationCallback callback, void* userData, NB_LS_FixType fixType)
{
    if (callback == NULL)
    {
        // it makes no sense to make the request when we don't have callback, result cannot be returned to user
        return NE_BADDATA;
    }

    m_callback = callback;
    m_userData = userData;

    if (!m_locationContext->locationConfig.emulationMode)
    {
        // Start GPS request first

        NB_QaLogAppState(m_locationContext->internalContext, "Starting GPS request");
        NB_Error gpsErr = m_gpsRequest->RequestStart((NB_LS_LocationCallback)Static_GpsRequestCallback, this, fixType);

        if (gpsErr != NE_OK)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "GPS req immediate error");
        }

        m_gpsReqComplete = gpsErr != NE_OK; // If request started -> complete FALSE

        // Then, network request

        NB_QaLogAppState(m_locationContext->internalContext, "Starting network request");
        NB_Error networkErr = m_networkRequest->RequestStart((NB_LS_LocationCallback)Static_NetworkRequestCallback, this, fixType);

        if (networkErr != NE_OK)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "Network req immediate error");
        }

        m_networkReqComplete = networkErr != NE_OK; // If request started -> complete FALSE

        return (networkErr == NE_OK || gpsErr == NE_OK) ? NE_OK : gpsErr;
    }
    else
    {
        NB_Error emuErr = m_locationContext->m_emuLocationRequest->RequestStart
            ((NB_LS_LocationCallback)Static_EmuRequestCallback, this, fixType);
        return emuErr;
    }
}

void CombinedLocationRequest::RequestCancel()
{
    m_networkRequest->RequestCancel( (NB_LS_LocationCallback)Static_NetworkRequestCallback, this );
    m_gpsRequest->RequestCancel( (NB_LS_LocationCallback)Static_GpsRequestCallback, this );
}

void CombinedLocationRequest::Static_NetworkRequestCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    CombinedLocationRequest* thisRequest = static_cast<CombinedLocationRequest*>(appData);
    thisRequest->NetworkRequestCallback(context, location, error);
}

void CombinedLocationRequest::NetworkRequestCallback(NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    m_networkReqError = error;
    m_networkReqComplete = TRUE;
    if (error == NE_OK && location->valid != 0)
    {
        LocationRequest::CopyLocation(m_networkLocation, *location);
        NB_QaLogAppState(m_locationContext->internalContext, "Network req SUCCESS");
    }
    else
    {
        NB_QaLogAppState(m_locationContext->internalContext, "Network req callback error");
    }

    if (m_gpsReqComplete)
    {
        CombinedRequestComplete( RetrieveError() );
    }
}

void CombinedLocationRequest::Static_GpsRequestCallback(void* appData, NB_LocationContext* context,
      const NB_LS_Location* location, NB_Error error)
{
    CombinedLocationRequest* thisRequest = static_cast<CombinedLocationRequest*>(appData);
    thisRequest->GpsRequestCallback(context, location, error);
}

void CombinedLocationRequest::GpsRequestCallback(NB_LocationContext* context,
      const NB_LS_Location* location, NB_Error error)
{
    m_gpsReqComplete = TRUE;
    m_gpsReqError = error;
    bool completed = FALSE;

    if (error == NE_OK && location->valid != 0)
    {
        // If we get valid GPS coordinates, we don't need to wait for network request any more
        m_networkRequest->RequestCancel( (NB_LS_LocationCallback)Static_NetworkRequestCallback, this );

        completed = TRUE;
        LocationRequest::CopyLocation(m_gpsLocation, *location);
        NB_QaLogAppState(m_locationContext->internalContext, "GPS req SUCCESS");
    }
    else
    {
        // GPS request returns error and network request completed -> combined request completed
        completed = m_networkReqComplete;

        if (error == NE_OK && location->valid == 0)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "GPS callback - invalid location");
        }
        else
        {
            NB_QaLogAppState(m_locationContext->internalContext, "GPS req callback error");
        }
    }

    if (completed)
    {
        CombinedRequestComplete( RetrieveError() );
    }
}

void CombinedLocationRequest::Static_EmuRequestCallback(void* appData, NB_LocationContext* context,
      const NB_LS_Location* location, NB_Error error)
{
    CombinedLocationRequest* thisRequest = static_cast<CombinedLocationRequest*>(appData);
    thisRequest->EmuRequestCallback(context, location, error);
}

void CombinedLocationRequest::EmuRequestCallback(NB_LocationContext* context,
      const NB_LS_Location* location, NB_Error error)
{
    if (m_callback)
    {
        m_callback(m_userData, m_locationContext, location, error);
    }
}

NB_LS_Location* CombinedLocationRequest::RetrieveLocation()
{
    NB_LS_Location* loc = NULL;

    // If we have valid GPS fix, return it
    if (m_gpsLocation.valid != 0)
    {
        loc = &m_gpsLocation;
    }
    // Then check for valid network location
    else if (m_networkLocation.valid != 0)
    {
        loc = &m_networkLocation;
    }

    return loc;
}

NB_Error CombinedLocationRequest::RetrieveError()
{
    if (LocationRequest::IsIdenError(m_networkReqError))
    {
        return m_networkReqError;
    }
    else if (m_gpsReqError == NE_OK || m_networkReqError == NE_OK)
    {
        return NE_OK;
    }

    return m_gpsReqError;
}

void CombinedLocationRequest::CombinedRequestComplete(NB_Error error)
{
    m_callback(m_userData, m_locationContext, RetrieveLocation(), error);
}

/*! @} */
