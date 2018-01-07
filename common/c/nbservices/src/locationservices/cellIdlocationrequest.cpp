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

    @file     CellIDLocationRequest.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        CellIDLocationRequest class implementation

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
#include "cellIdlocationrequest.h"

extern "C" {
  #include "nbqalog.h"
  #include "nberror.h"
}

// This value is not used after retrieving the source from request.
// Introduced just to retrieve the string.
#define NETWORK_LOCATION_SOURCE_LEN 255

CellIDLocationRequest::CellIDLocationRequest(NB_LocationContext* locationContext, NB_Context* nbContext)
    : LocationRequest(locationContext),
      m_cancelled(FALSE),
      m_PALRequestInProgress(FALSE),
      m_tpsHandler(NULL),
      m_tpsParameters(NULL),
      m_reqNBContext(nbContext),
      m_cellInfoChanged(FALSE)
{
    nsl_memset(&m_location, 0, sizeof(m_location));
    SetInfo(PRSM_CDMA, NULL, NULL);

    if (!m_locationContext->m_PAL_RadioContext)
    {
        m_state = NBPGS_Undefined;
    }
    else
    {
        m_state = NBPGS_Initialized;
    }

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID request instance created, state=%d", m_state));
}

CellIDLocationRequest::~CellIDLocationRequest(void)
{
    CancelLocationRequest();

    if (m_tpsParameters != NULL)
    {
        NB_LocationParametersDestroy(m_tpsParameters);
        m_tpsParameters = NULL;
    }

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerDestroy(m_tpsHandler);
        m_tpsHandler = NULL;
    }
}

int CellIDLocationRequest::GetSpecificTimeout(NB_LS_FixType fixType)
{
    return m_locationContext->networkLocationTimeout;
}

NB_Error CellIDLocationRequest::GetState( NB_LS_LocationState * state )
{
    *state = m_state;
    return NE_OK;
}

NB_Error CellIDLocationRequest::StartLocationRequest()
{
    if (m_state == NBPGS_Undefined)
    {
        return NE_GPS;
    }
    else if (m_state == NBPGS_Oneshot)
    {
        return NE_BUSY;
    }

    nsl_memset(&m_location, 0, sizeof(m_location));
    PAL_Error err = PAL_Ok;

    if (m_tpsParameters != NULL)
    {   // Parameters could contain stale NB_Context data
        NB_LocationParametersDestroy(m_tpsParameters);
        m_tpsParameters = NULL;
    }

    if (m_tpsHandler)
    {   // Handler could contain stale NB_Context data
        NB_LocationHandlerDestroy(m_tpsHandler);
        m_tpsHandler = NULL;
    }

    // Possible situation: previous request has been cancelled, but we haven't yet catched its PAL callback.
    // (Flag m_PALRequestInProgress == TRUE indicates it.)
    // Since it contains valid data, let's reuse this callback. No need to start new request.
    if (m_PALRequestInProgress == FALSE)
    {
        err = PAL_RadioGetSystemMode(m_locationContext->m_PAL_RadioContext,
            Static_PAL_SystemMode_RequestCallback, (void*)this);
    }

    m_cancelled = FALSE;

    if (err != PAL_Ok)
    {
        return NE_GPS;
    }
    else
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID location request started", 1));
        m_PALRequestInProgress = TRUE;
        m_cellInfoChanged = FALSE;
        m_state = NBPGS_Oneshot;
        return NE_OK;
    }
}

bool CellIDLocationRequest::IsInProgress()
{
    return (m_state == NBPGS_Oneshot);
}

void CellIDLocationRequest::CancelLocationRequest()
{
    // Mark the request as cancelled.
    if (m_PALRequestInProgress == TRUE)
    {
        m_cancelled = TRUE;
    }

    m_state = NBPGS_Initialized;

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerCancelRequest(m_tpsHandler);
    }
}

NB_LS_Location* CellIDLocationRequest::RetrieveLocation()
{
    return &m_location;
}

void CellIDLocationRequest::Static_PAL_SystemMode_RequestCallback
                (PAL_Error status, PAL_RadioSystemMode systemMode, void* userData)
{
    CellIDLocationRequest* thisRequest = (CellIDLocationRequest*)userData;
    thisRequest->SystemModeRequestResult(status, systemMode);
}

void CellIDLocationRequest::SystemModeRequestResult(PAL_Error status, PAL_RadioSystemMode systemMode)
{
    if (m_cancelled)
    {
        // Do not return anything to user, just complete the work silently.
        ResetState(FALSE);
    }
    else
    {
        if ((status != PAL_Ok) || (systemMode == PRSM_NoService))
        {
            // Internal error, or no service; both means we can't proceed with our work.
            // Notify user about general error.
            if (systemMode == PRSM_NoService)
            {
                NB_QaLogAppState(m_locationContext->internalContext, "PAL system mode is NoService");
            }
            ResetState(TRUE);
        }
        else
        {
            // Continue our work. Now we know what our system is; go and retrieve the cell information.
            PAL_Error err = PAL_Ok;

            if (systemMode == PRSM_CDMA)
            {
                err = PAL_RadioGetCDMANetworkInfo(m_locationContext->m_PAL_RadioContext,
                    Static_PAL_CDMA_RequestCallback, (void*)this);
            }
            else // PRSM_GSM
            {
                err = PAL_RadioGetGSMNetworkInfo(m_locationContext->m_PAL_RadioContext,
                    Static_PAL_GSM_RequestCallback, (void*)this);
            }

            if (err != PAL_Ok)
            {
                // Reset and notify user about general error.
                ResetState(TRUE);
            }
        }
    }
}

void CellIDLocationRequest::Static_PAL_CDMA_RequestCallback
                 (PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo, void* userData)
{
    CellIDLocationRequest* thisRequest = (CellIDLocationRequest*)userData;
    thisRequest->CDMAInfoRequestResult(status, cdmaInfo);
}

void CellIDLocationRequest::CDMAInfoRequestResult(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo)
{
    if (m_cancelled)
    {
        // Do not return anything to user, just complete the work silently.
        ResetState(FALSE);
    }
    else
    {
        m_PALRequestInProgress = FALSE;
        NB_Error err = NE_OK;
        if (status == PAL_Ok)
        {
            m_cellInfoChanged = IsNewInfo(PRSM_CDMA, cdmaInfo, NULL);
            SetInfo(PRSM_CDMA, cdmaInfo, NULL);
            err = NB_LocationParametersCreate(m_reqNBContext,
                                              &m_tpsParameters);
            if (PAL_IsSimulator(m_locationContext->m_PAL_Instance))
            {
                PAL_RadioCDMAInfo simCdmaInfo = {0};
                SimulatorGetCDMANetworkInfo(&simCdmaInfo);
                err = NB_LocationParametersAddCdma(m_tpsParameters,
                                                   simCdmaInfo.systemId,
                                                   simCdmaInfo.networkId,
                                                   simCdmaInfo.cellId,
                                                   simCdmaInfo.signalStrength);
            }
            else
            {
                err = NB_LocationParametersAddCdma(m_tpsParameters,
                                                   cdmaInfo->systemId,
                                                   cdmaInfo->networkId,
                                                   cdmaInfo->cellId,
                                                   cdmaInfo->signalStrength);
            }

            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID request: cell ID=%d", cdmaInfo->cellId));
            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID request: signal strength=%d", cdmaInfo->signalStrength));

            if (err == NE_OK)
            {
                NB_RequestHandlerCallback callback = { Static_TPS_RequestCallback, (void*)this };

                err = NB_LocationHandlerCreate(m_reqNBContext,
                                               &callback,
                                               &m_tpsHandler);

                if (err == NE_OK)
                {
                    NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Request);
                    err = NB_LocationHandlerStartRequest(m_tpsHandler, m_tpsParameters);
                }

                NB_LocationParametersDestroy(m_tpsParameters);
                m_tpsParameters = NULL;
            }
        }


        if (status != PAL_Ok || err != NE_OK)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "PAL CellID request error");
            // Reset and notify user about general error.
            ResetState(TRUE);
        }
    }
}

void CellIDLocationRequest::Static_PAL_GSM_RequestCallback
                 (PAL_Error status, PAL_RadioGSMInfo* gsmInfo, void* userData)
{
    CellIDLocationRequest* thisRequest = (CellIDLocationRequest*)userData;
    thisRequest->GSMInfoRequestResult(status, gsmInfo);
}

void CellIDLocationRequest::GSMInfoRequestResult(PAL_Error status, PAL_RadioGSMInfo* gsmInfo)
{
    if (m_cancelled)
    {
        // Do not return anything to user, just complete the work silently.
        ResetState(FALSE);
    }
    else
    {
        m_PALRequestInProgress = FALSE;
        NB_Error err = NE_OK;
        if (status == PAL_Ok)
        {
            m_cellInfoChanged = IsNewInfo(PRSM_GSM, NULL, gsmInfo);
            SetInfo(PRSM_GSM, NULL, gsmInfo);
            NB_LocationParameters* parameters = 0;
            err = NB_LocationParametersCreate(m_reqNBContext,
                                              &parameters);
            err = NB_LocationParametersAddGsm(parameters,
                                              gsmInfo->mobileCountryCode,
                                              gsmInfo->mobileNetworkCode,
                                              gsmInfo->locationAreaCode,
                                              gsmInfo->cellId,
                                              gsmInfo->signalStrength);

            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID request: cell ID=%d", gsmInfo->cellId));
            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID request: signal strength=%d", gsmInfo->signalStrength));

            if (err == NE_OK)
            {
                NB_RequestHandlerCallback callback = { Static_TPS_RequestCallback, (void*)this };

                err = NB_LocationHandlerCreate(m_reqNBContext,
                                               &callback,
                                               &m_tpsHandler);

                if (err == NE_OK)
                {
                    NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Request);

                    err = NB_LocationHandlerStartRequest(m_tpsHandler, m_tpsParameters);
                }

                NB_LocationParametersDestroy(parameters);
                m_tpsParameters = NULL;
            }
            else
            {
                NB_LocationParametersDestroy(parameters);
            }
        }


        if (status != PAL_Ok || err != NE_OK)
        {
            NB_QaLogAppState(m_locationContext->internalContext, "PAL CellID request error");
            // Reset and notify user about general error.
            ResetState(TRUE);
        }
    }
}

void CellIDLocationRequest::Static_TPS_RequestCallback(void* handler, NB_RequestStatus status, NB_Error err,
                                      nb_boolean up, int percent, void* userData)
{
    CellIDLocationRequest* thisRequest = (CellIDLocationRequest*)userData;
    thisRequest->TPSRequestResult(handler, status, err, up, percent);
}

void CellIDLocationRequest::TPSRequestResult(void* handler, NB_RequestStatus status, NB_Error err,
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

                        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID location: latitude=%f", m_location.latitude));
                        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]CellID location: longitude=%f", m_location.longitude));

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
                    ResetState(TRUE, clientError);
                }
            }
            break;

        case NB_NetworkRequestStatus_Canceled:
            // It means, we cancelled the request by ourselves, so complete the work silently
            ResetState(FALSE);
            break;

        case NB_NetworkRequestStatus_TimedOut:
            // Tell user about timeout
            ResetState(TRUE, NESERVER_TIMEDOUT);
            break;
        case NB_NetworkRequestStatus_Failed:
            if (err == NESERVERX_DAILY_REQUEST_LIMIT_ERROR)
            {
                // Daily request limit exhausted
                ResetState(TRUE, NESERVERX_DAILY_REQUEST_LIMIT_ERROR);
            }
            else if (NB_ContextGetIdenError(m_reqNBContext) == NESERVERX_INVALID_CREDENTIAL_ERROR)
            {
                ResetState(TRUE, NESERVERX_INVALID_CREDENTIAL_ERROR);
            }
            else if (NB_ContextGetIdenError(m_reqNBContext) == NESERVERX_DAILY_USER_LIMIT_ERROR)
            {
                ResetState(TRUE, NESERVERX_DAILY_USER_LIMIT_ERROR);
            }
            else
            {
                // Tell user about some general error
                ResetState(TRUE);
            }
            break;
        default:
            break;

        } // switch(status)

    } // if (percent == 100)

}

void CellIDLocationRequest::ResetState(bool returnError, NB_Error errorCode)
{
    m_cancelled = FALSE;
    m_PALRequestInProgress = FALSE;
    m_state = NBPGS_Initialized;
    if (returnError)
    {
        LocationCallback(errorCode);
    }
}

void CellIDLocationRequest::SimulatorGetCDMANetworkInfo(PAL_RadioCDMAInfo* cdmaInfo)
{
    // Standard location, requested to be returned for all simulators.
    // Some cell tower in Aliso Viejo.
    cdmaInfo->systemId = 2;
    cdmaInfo->networkId = 55;
    cdmaInfo->cellId = 5778;
    cdmaInfo->signalStrength = -5;
}

void CellIDLocationRequest::SetInfo(PAL_RadioSystemMode sysMode, PAL_RadioCDMAInfo* cdmaInfo,
                                       PAL_RadioGSMInfo* gsmInfo)
{
   if (cdmaInfo == NULL && gsmInfo == NULL)
    {
        m_lastCdmaInfo.cellId = 0;
        m_lastCdmaInfo.networkId = 0;
        m_lastCdmaInfo.signalStrength = 0;
        m_lastCdmaInfo.systemId = 0;

        m_lastGsmInfo.cellId = 0;
        m_lastGsmInfo.locationAreaCode = 0;
        m_lastGsmInfo.mobileCountryCode = 0;
        m_lastGsmInfo.mobileNetworkCode = 0;
        m_lastGsmInfo.signalStrength = 0;
    }
    else if (sysMode == PRSM_CDMA && cdmaInfo != NULL)
    {
        m_lastCdmaInfo.cellId = cdmaInfo->cellId;
        m_lastCdmaInfo.networkId = cdmaInfo->networkId;
        m_lastCdmaInfo.signalStrength = cdmaInfo->signalStrength;
        m_lastCdmaInfo.systemId = cdmaInfo->systemId;
    }
    else if (sysMode == PRSM_GSM && gsmInfo != NULL)
    {
        m_lastGsmInfo.cellId = gsmInfo->cellId;
        m_lastGsmInfo.locationAreaCode = gsmInfo->locationAreaCode;
        m_lastGsmInfo.mobileCountryCode = gsmInfo->mobileCountryCode;
        m_lastGsmInfo.mobileNetworkCode = gsmInfo->mobileNetworkCode;
        m_lastGsmInfo.signalStrength = gsmInfo->signalStrength;
    }
}

bool CellIDLocationRequest::IsNewInfo(PAL_RadioSystemMode sysMode, PAL_RadioCDMAInfo* cdmaInfo,
                                         PAL_RadioGSMInfo* gsmInfo)
{
    bool ret = FALSE;
    if (sysMode == PRSM_CDMA && cdmaInfo != NULL)
    {
        ret = m_lastCdmaInfo.cellId != cdmaInfo->cellId ||
              m_lastCdmaInfo.networkId != cdmaInfo->networkId ||
              //m_lastCdmaInfo.signalStrength != cdmaInfo->signalStrength ||
              m_lastCdmaInfo.systemId != cdmaInfo->systemId;
    }
    else if (sysMode == PRSM_GSM && gsmInfo != NULL)
    {
        ret = m_lastGsmInfo.cellId != gsmInfo->cellId ||
              m_lastGsmInfo.locationAreaCode != gsmInfo->locationAreaCode ||
              m_lastGsmInfo.mobileCountryCode != gsmInfo->mobileCountryCode ||
              //m_lastGsmInfo.signalStrength != gsmInfo->signalStrength ||
              m_lastGsmInfo.mobileNetworkCode != gsmInfo->mobileNetworkCode;
    }
    return ret;
}

bool CellIDLocationRequest::CellInfoChanged()
{
    return m_cellInfoChanged;
}

/*! @} */
