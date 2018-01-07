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

    @file     WifiLocationRequest.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        WifiLocationRequest class implementation

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


extern "C" {
  #include "palclock.h"
  #include "nberror.h"
  #include <nbqalog.h>
}

#include "locationcontext.h"
#include "wifilocationrequest.h"

// This value is not used after retrieving the source from request.
// Introduced just to retrieve the string.
#define NETWORK_LOCATION_SOURCE_LEN 255

WifiLocationRequest::WifiLocationRequest(NB_LocationContext* locationContext, NB_Context* nbContext)
    : LocationRequest(locationContext),
      m_cancelled(FALSE),
      m_PALRequestInProgress(FALSE),
      m_tpsHandler(NULL),
      m_reqNBContext(nbContext)
{
    nsl_memset(&m_location, 0, sizeof(m_location));

    if (!m_locationContext->m_PAL_RadioContext)
    {
        m_state = NBPGS_Undefined;
    }
    else
    {
        m_state = NBPGS_Initialized;
    }

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi request instance created, state=%d", m_state));
}

WifiLocationRequest::~WifiLocationRequest(void)
{
    CancelLocationRequest();

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerDestroy(m_tpsHandler);
        m_tpsHandler = NULL;
    }
}

NB_Error WifiLocationRequest::GetState( NB_LS_LocationState * state )
{
    *state = m_state;
    return NE_OK;
}

bool WifiLocationRequest::IsCachedLocationValid(NB_LS_FixType fixType)
{
    int maxAge = 0;
    switch (fixType)
    {
        case NBFixType_Fast:
        case NBFixType_Normal:
            maxAge = m_locationContext->maxWiFiAge;
            break;
        default:
            break;
    }

    return IsLastFixValid(maxAge);
}

int WifiLocationRequest::GetSpecificTimeout(NB_LS_FixType fixType)
{
    return m_locationContext->networkLocationTimeout;
}

NB_Error WifiLocationRequest::StartLocationRequest()
{
    if (m_state == NBPGS_Undefined)
    {
        return NEGPS_GENERAL;
    }
    else if (m_state == NBPGS_Oneshot)
    {
        return NE_BUSY;
    }

    nsl_memset(&m_location, 0, sizeof(m_location));
    PAL_Error err = PAL_Ok;

    if (m_tpsHandler)
    {   // Handler could contain stale NB_Context data
        NB_LocationHandlerDestroy(m_tpsHandler);
        m_tpsHandler = NULL;
    }

    // Possible situation: previous request has been cancelled, but we haven't yet catched its PAL callback.
    // (Flag m_PALRequestInProgress == true indicates it.)
    // Since it contains valid data, let's reuse this callback. No need to start new request.
    if (m_PALRequestInProgress == FALSE)
    {
        err = PAL_RadioGetAllWifiNetworksInfo(m_locationContext->m_PAL_RadioContext,
            Static_PAL_Wifi_RequestCallback, (void*)this);
    }

    m_cancelled = FALSE;

    if (err != PAL_Ok)
    {
        return NEGPS_GENERAL;
    }
    else
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi location request started", 1));
        m_PALRequestInProgress = true;
        m_state = NBPGS_Oneshot;
        return NE_OK;
    }
}

bool WifiLocationRequest::IsInProgress()
{
    return (m_state == NBPGS_Oneshot);
}

void WifiLocationRequest::CancelLocationRequest()
{
    // Mark the request as cancelled.
    if (m_PALRequestInProgress == true)
    {
        m_cancelled = true;
    }
    m_state = NBPGS_Initialized;

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerCancelRequest(m_tpsHandler);
    }
}

NB_LS_Location* WifiLocationRequest::RetrieveLocation()
{
    return &m_location;
}

void WifiLocationRequest::Static_PAL_Wifi_RequestCallback(PAL_Error status,
      PAL_RadioCompleteWifiInfo** wifisInfo, unsigned long wifisInfoCount, void* userData)
{
    WifiLocationRequest* thisRequest = (WifiLocationRequest*)userData;
    thisRequest->WifiInfoRequestResult(status, wifisInfo, (int)wifisInfoCount);
}

void WifiLocationRequest::WifiInfoRequestResult(PAL_Error status, PAL_RadioCompleteWifiInfo** wifiInfo, int wifisInfoCount)
{
    if (m_cancelled)
    {
        // Do not return anything to user, just complete the work silently.
        ResetState(FALSE);
    }
    else
    {
        m_PALRequestInProgress = FALSE;
        NB_Error err = NE_INVAL;
        if (status == PAL_Ok)
        {
            NB_LocationParameters* parameters = 0;
            err = NB_LocationParametersCreate(m_reqNBContext,
                                                   &parameters);
            if (PAL_IsSimulator(m_locationContext->m_PAL_Instance))
            {
                // app is launched on simulator. Use predefined wifi info.
                char* macAddrstr = NULL;
                PAL_RadioWifiInfo simWifiInfo = {0};
                SimulatorGetWifiNetworkInfo(&simWifiInfo);
                macAddrstr = ConvertMacToString(simWifiInfo.macAddress);
                err = NB_LocationParametersAddWifi(parameters,
                                                   macAddrstr,
                                                   simWifiInfo.signalStrength);
                nsl_free((void*)simWifiInfo.macAddress);
                nsl_free((void*)macAddrstr);
            }
            else
            {
                // add all available wifi info to the TPS request.
                for (int i = 0; i < wifisInfoCount; ++i)
                {
                    char* macAddrstr = NULL;
                    macAddrstr = ConvertMacToString((*wifiInfo)[i].macAddress);
                    if (NB_LocationParametersAddWifi(parameters, macAddrstr, (*wifiInfo)[i].signalStrength) == NE_OK)
                    {
                        // if at least one wifi info is added then it is OK. we can send TPS request.
                        err = NE_OK;
                    }

                    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi request [%d]: MAC=%s", i, macAddrstr));
                    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi request [%d]: signal strength=%d", i, (*wifiInfo)[i].signalStrength));
                    nsl_free((void*)macAddrstr);
                }
            }

            if (err == NE_OK)
            {
                NB_RequestHandlerCallback callback = { Static_TPS_RequestCallback, (void*)this };

                err = NB_LocationHandlerCreate(m_reqNBContext,
                                               &callback,
                                               &m_tpsHandler);

                if (err == NE_OK)
                {
                    err = NB_LocationHandlerStartRequest(m_tpsHandler, parameters);
                    if (err == NE_OK)
                    {
                        NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Request);
                    }
                }
                NB_LocationParametersDestroy(parameters);
            }
            else
            {
                NB_LocationParametersDestroy(parameters);
            }
        }


        if (status != PAL_Ok || err != NE_OK)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "PAL WiFi info error");
            // Reset and notify user about general error.
            ResetState(true);
        }
    }
}

void WifiLocationRequest::Static_TPS_RequestCallback(void* handler, NB_RequestStatus status, NB_Error err,
                                      nb_boolean up, int percent, void* userData)
{
    WifiLocationRequest* thisRequest = (WifiLocationRequest*)userData;
    thisRequest->TPSRequestResult(handler, status, err, up, percent);
}

void WifiLocationRequest::TPSRequestResult(void* handler, NB_RequestStatus status, NB_Error err,
                                      nb_boolean up, int percent)
{
    if (percent == 100) // Completed
    {
        NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Reply);

        switch(status)
        {
        case NB_NetworkRequestStatus_Success:
            {
                NB_Error clientError = NESERVER_FAILED;
                NB_LocationHandler* locHandler = (NB_LocationHandler*)handler;
                NB_LocationInformation* information = NULL;

                err = NB_LocationHandlerGetInformation(locHandler, &information);

                if (information && err == NE_OK)
                {
                    NB_LocationRequestStatus requestStatus = NB_LRS_Success;
                    uint32 accuracyMeters = 0;
                    NB_LatitudeLongitude position = {0};
                    char networkLocationSource[NETWORK_LOCATION_SOURCE_LEN] = {0};
                    err = NB_LocationInformationGetNetworkPosition(
                                        information,
                                        &requestStatus,
                                        &position,
                                        &accuracyMeters,
                                        networkLocationSource,
                                        NETWORK_LOCATION_SOURCE_LEN);

                    if ((err == NE_OK) && (requestStatus == NB_LRS_Success))
                    {
                        // Copy all useful data
                        m_location.latitude = position.latitude;
                        m_location.longitude = position.longitude;
                        m_location.horizontalUncertaintyAlongAxis = accuracyMeters;
                        m_location.horizontalUncertaintyAlongPerpendicular = accuracyMeters;
                        m_location.horizontalUncertaintyAngleOfAxis = 0.;
                        m_location.valid |= NBPGV_Latitude;
                        m_location.valid |= NBPGV_Longitude;
                        m_location.valid |= NBPGV_HorizontalUncertainty;
                        m_location.valid |= NBPGV_AxisUncertainty;
                        m_location.valid |= NBPGV_PerpendicularUncertainty;

                        NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Reply);

                        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi location: latitude=%f", m_location.latitude));
                        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Wifi location: longitude=%f", m_location.longitude));

                        m_state = NBPGS_Initialized;
                        LocationCallback(NE_OK);

                    }
                    else if ((err == NE_OK) && (requestStatus != NB_LRS_Success))
                    {
                        err = NE_BADDATA;
                        clientError = NE_ERROR_INVALID_LOCATION;
                    }

                    (void)NB_LocationInformationDestroy(information);
                }

                if (err != NE_OK)
                {
                    // Some error with information - notify user
                    ResetState(true, clientError);
                }
            }
            break;

        case NB_NetworkRequestStatus_Canceled:
            // It means, we cancelled the request by ourselves, so complete the work silently
            ResetState(FALSE);
            break;

        case NB_NetworkRequestStatus_TimedOut:
            // Tell user about timeout
            ResetState(true, NEGPS_TIMEOUT);
            break;
        case NB_NetworkRequestStatus_Failed:
            if (err == NESERVERX_DAILY_REQUEST_LIMIT_ERROR)
            {
                // Daily request limit exhausted
                ResetState(true, NESERVERX_DAILY_REQUEST_LIMIT_ERROR);
            }
            else if (NB_ContextGetIdenError(m_reqNBContext) == NESERVERX_INVALID_CREDENTIAL_ERROR)
            {
                ResetState(true, NESERVERX_INVALID_CREDENTIAL_ERROR);
            }
            else if (NB_ContextGetIdenError(m_reqNBContext) == NESERVERX_DAILY_USER_LIMIT_ERROR)
            {
                ResetState(true, NESERVERX_DAILY_USER_LIMIT_ERROR);
            }
            else
            {
                // Tell user about some general error
                ResetState(true);
            }
            break;
        default:
            break;

        } // switch(status)
    } // if (percent == 100)

}

void WifiLocationRequest::ResetState(bool returnError, NB_Error errorCode)
{
    m_cancelled = FALSE;
    m_PALRequestInProgress = FALSE;
    m_state = NBPGS_Initialized;
    if (returnError)
    {
        LocationCallback(errorCode);
    }
}

char* WifiLocationRequest::ConvertMacToString(const unsigned char* macAddress)
{
    char* buf = (char*)nsl_malloc(sizeof(char) * 20);
    if (buf)
    {
        nsl_sprintf(buf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", macAddress[0], macAddress[1],
            macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    }

    return buf;
}

void WifiLocationRequest::SimulatorGetWifiNetworkInfo(PAL_RadioWifiInfo* wifiInfo)
{
    // Standard location, requested to be returned for all simulators.
    unsigned char* buf = (unsigned char*)nsl_malloc(sizeof(unsigned char) * 6);
    if (buf)
    {
        buf[0] = 0x01; buf[1] = 0x23; buf[2] = 0x45;
        buf[3] = 0x67; buf[4] = 0x89; buf[5] = 0xab;
    }
    wifiInfo->macAddress = buf;
    wifiInfo->signalStrength = -5;
}

/*! @} */
