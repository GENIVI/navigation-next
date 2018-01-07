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

    @file     networklocationrequest.cpp
    @date     07/08/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        NetworkLocationRequest class implementation

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
 #include "nberror.h"
 #include <nbqalog.h>
 #include "palclock.h"
 #include "transformutility.h"
}
#include "palmath.h"

 #include "palmath.h"
#include "locationcontext.h"
#include "networklocationrequest.h"
#include "radioinformation.h"

// This value is not used after retrieving the source from request.
// Introduced just to retrieve the string.
#define NETWORK_LOCATION_SOURCE_LEN 128


static void MovePoint(double* latitude, double* longitude, int32 offset, int32 angle);

NetworkLocationRequest::NetworkLocationRequest(NB_LocationContext* locationContext, NB_Context* nbContext)
    : LocationRequest(locationContext),
      m_cancelled(false),
      m_PALRequestInProgress(false),
      m_firstPass(true),
      m_tpsHandler(NULL),
      m_reqNBContext(nbContext),
      m_lastSysMode(PRSM_NoService)
{
    nsl_memset(&m_location, 0, sizeof(m_location));
    nsl_memset(&m_lastCdmaInfo, 0, sizeof(m_lastCdmaInfo));
    nsl_memset(&m_lastGsmInfo, 0, sizeof(m_lastGsmInfo));

    if (!m_locationContext->m_PAL_RadioContext || !m_locationContext->m_radioInformation)
    {
        m_state = NBPGS_Undefined;
    }
    else
    {
        m_state = NBPGS_Initialized;
    }

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Network request instance created, state=%d", m_state));
}

NetworkLocationRequest::~NetworkLocationRequest(void)
{
    CancelLocationRequest();

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerDestroy(m_tpsHandler);
        m_tpsHandler = NULL;
    }
}

NB_Error NetworkLocationRequest::GetState( NB_LS_LocationState * state )
{
    if ( NULL == state )
    {
        return NE_INVAL;
    }

    *state = m_state;
    return NE_OK;
}

int NetworkLocationRequest::GetSpecificTimeout(NB_LS_FixType fixType)
{
    return m_locationContext->networkLocationTimeout;
}

bool NetworkLocationRequest::IsCachedLocationValid(NB_LS_FixType fixType)
{
    int maxAge = 0;
    switch (fixType)
    {
        case NBFixType_Fast:
        case NBFixType_Normal:
            /* Use here max age for Wifi fix. Wifi info is changed more often than Cell ID info. */
            maxAge = m_locationContext->maxWiFiAge;
            break;
        default:
            break;
    }

    return IsLastFixValid(maxAge);
}

NB_Error NetworkLocationRequest::StartLocationRequest()
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
    nsl_memset(&m_lastCdmaInfo, 0, sizeof(m_lastCdmaInfo));
    nsl_memset(&m_lastGsmInfo, 0, sizeof(m_lastGsmInfo));
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
        m_locationContext->m_radioInformation->SetListener(Static_RadioInformationCallback, this);
        err = m_locationContext->m_radioInformation->RequestInformation();
    }

    m_cancelled = false;
    m_firstPass = true;

    if (err != PAL_Ok)
    {
        return NEGPS_GENERAL;
    }
    else
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Network location request started", 1));
        m_PALRequestInProgress = true;
        m_state = NBPGS_Oneshot;
        return NE_OK;
    }
}

bool NetworkLocationRequest::IsInProgress()
{
    return (m_state == NBPGS_Oneshot);
}

void NetworkLocationRequest::CancelLocationRequest()
{
    // Mark the request as cancelled.
    if (m_PALRequestInProgress == true)
    {
        m_cancelled = true;
    }
    m_state = NBPGS_Initialized;
    m_firstPass = true;

    if (m_tpsHandler != NULL)
    {
        NB_LocationHandlerCancelRequest(m_tpsHandler);
    }
}

NB_LS_Location* NetworkLocationRequest::RetrieveLocation()
{
    return &m_location;
}

void NetworkLocationRequest::Static_RadioInformationCallback(PAL_Error status, RadioInformation* information,
                                                             void* userData)
{
    NetworkLocationRequest* thisRequest = static_cast<NetworkLocationRequest*>(userData);
    thisRequest->RadioInformationReceived(status, information);
}

void NetworkLocationRequest::RadioInformationReceived(PAL_Error status, RadioInformation* information)
{
    if (m_cancelled)
    {
        // Do not return anything to user, just complete the work silently.
        ResetState(false);
        return;
    }
    if (status != PAL_Ok)
    {
        NB_QaLogAppState(m_locationContext->internalContext, "PAL radio info error");
        // Reset and notify user about general error.
        ResetState(true);
    }

    m_PALRequestInProgress = FALSE;

    information->GetCellIdInformation(m_lastSysMode, m_lastCdmaInfo, m_lastGsmInfo);
    ObtainLocation();
}

void NetworkLocationRequest::Static_TPS_RequestCallback(void* handler, NB_RequestStatus status, NB_Error err,
                                                        nb_boolean up, int percent, void* userData)
{
    NetworkLocationRequest* thisRequest = static_cast<NetworkLocationRequest*>(userData);
    thisRequest->TPSRequestResult(handler, status, err, up, percent);
}

void NetworkLocationRequest::TPSRequestResult(void* handler, NB_RequestStatus status, NB_Error err,
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
                NB_LocationHandler* locHandler = static_cast<NB_LocationHandler*>(handler);
                NB_LocationInformation* information = NULL;

                err = NB_LocationHandlerGetInformation(locHandler, &information);

                if (information != NULL && err == NE_OK)
                {
                    ExtractLocation(information);

                    int32 tilesCount = 0;
                    NB_LocationRequestStatus status = NB_LRS_Success;

                    err = NB_LocationInformationGetRequestStatus(information, &status);
                    if (err == NE_OK && status == NB_LRS_Success)
                    {
                        err = NB_LocationInformationGetLocationTilesCount(information, &tilesCount);

                        for (int i = 0; i < tilesCount && err == NE_OK; ++i)
                        {
                            uint32 tileID = 0;
                            NB_LocationTile locationTile = {0};
                            err = NB_LocationInformationGetLocationTile(information, i, &locationTile);
                            err = err ? err :
                                m_locationContext->m_cacheDb->AddTile(locationTile.tileX,
                                                                      locationTile.tileY,
                                                                      locationTile.dbFragment,
                                                                      locationTile.dbFragmentSize,
                                                                      tileID);
                           if (err == NE_OK && m_location.valid != 0)
                            {
                                // Set position to appropriate tile record
                                if (m_lastSysMode == PRSM_CDMA)
                                {
                                    m_locationContext->m_cacheDb->SetCoordinates(tileID, m_lastCdmaInfo, m_location);
                                }
                                else if (m_lastSysMode == PRSM_GSM)
                                {
                                    m_locationContext->m_cacheDb->SetCoordinates(tileID, m_lastGsmInfo, m_location);
                                }
                            }
                        }
                    } // if (err == NE_OK && status == NB_LRS_Success)

                    NB_QaLogCellIdReply(m_locationContext->internalContext,
                        m_location.latitude,
                        m_location.longitude,
                        static_cast<uint32>(m_location.horizontalUncertaintyAlongAxis),
                        status);

                    (void)NB_LocationInformationDestroy(information);

                    if (m_location.valid != 0 && tilesCount >= 0)
                    {
                        // Server returns us the location calculated from the tiles.
                        // We can return it to user without any additional processing.
                        // or
                        // Server doesn't return tiles, but return location(e.g. from Navizon).
                        // We return this location to user.
                        m_state = NBPGS_Initialized;

                        NB_QaLogLocationReply(m_locationContext->internalContext,
                            m_location.latitude,
                            m_location.longitude,
                            static_cast<unsigned short>(m_location.horizontalUncertaintyAlongAxis),
                            NB_QLLST_TCSCellID);

                        LocationCallback(NE_OK);
                    }
                    else if (err == NE_OK && m_location.valid == 0)
                    {
                        // We added just received tiles to cache, but didn't receive location from server.
                        // Try to calculate it on device.
                        ObtainLocation();
                    }
                } // if (information != NULL && err == NE_OK)

                if (err != NE_OK)
                {
                    // Some error with information - notify user
                    ResetState(true, clientError);
                }
            }
            break;

        case NB_NetworkRequestStatus_Canceled:
            // It means, we cancelled the request by ourselves, so complete the work silently
            ResetState(false);
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

void NetworkLocationRequest::ExtractLocation(NB_LocationInformation* information)
{
    NB_Error err = NE_INVAL;
    NB_LocationRequestStatus requestStatus = NB_LRS_NotFound;
    uint32 accuracyMeters = 0;
    NB_LatitudeLongitude position = {0};
    char networkLocationSource[NETWORK_LOCATION_SOURCE_LEN] = {0};

    err = NB_LocationInformationGetNetworkPosition(information,
                                                   &requestStatus,
                                                   &position,
                                                   &accuracyMeters,
                                                   networkLocationSource,
                                                   NETWORK_LOCATION_SOURCE_LEN);

    if (err == NE_OK && requestStatus == NB_LRS_Success)
    {
        // Copy all useful data
        m_location.latitude = position.latitude;
        m_location.longitude = position.longitude;
        m_location.horizontalUncertaintyAlongAxis = accuracyMeters;
        m_location.horizontalUncertaintyAlongPerpendicular = accuracyMeters;
        m_location.valid = NBPGV_Latitude | NBPGV_Longitude | NBPGV_AxisUncertainty | NBPGV_PerpendicularUncertainty;

        NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Reply);

        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Network location: lat=%f", m_location.latitude));
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Network location: lon=%f", m_location.longitude));
    }
}

void NetworkLocationRequest::ObtainLocation(void)
{
    NB_Error err = NE_INVAL;
    uint32 tileID = 0;
    CellIdRecord cellId;
    NB_QaLogLocationSourceType source = NB_QLLST_TCSCellID;
    nsl_memset(&cellId, 0, sizeof(CellIdRecord));

    if (m_lastSysMode == PRSM_CDMA)
    {
        err = m_locationContext->m_cacheDb->FindMatches(m_lastCdmaInfo, cellId, tileID, m_location);
    }
    else if (m_lastSysMode == PRSM_GSM)
    {
        err = m_locationContext->m_cacheDb->FindMatches(m_lastGsmInfo, cellId, tileID, m_location);
    }

    if(m_firstPass)
    {
        source = NB_QLLST_TCSCellIDLocal;
        if (m_lastSysMode == PRSM_CDMA)
        {
            NB_QaLogLocationRequestByCellID(m_locationContext->internalContext,
                m_lastCdmaInfo.systemId, m_lastCdmaInfo.networkId,
                m_lastCdmaInfo.cellId, m_lastCdmaInfo.signalStrength);
        }
        else if (m_lastSysMode == PRSM_GSM)
        {
            NB_QaLogLocationRequestByCellID(m_locationContext->internalContext,
                m_lastGsmInfo.mobileCountryCode, m_lastGsmInfo.mobileNetworkCode,
                m_lastGsmInfo.cellId, m_lastGsmInfo.signalStrength);
        }
    }
    else
    {
        source = NB_QLLST_TCSCellID;
    }

    if (err == NE_NOENT)
    {
        // No matches in the cache DB
        if (m_firstPass)
        {
            // Request location tiles from the server
            err = RequestLocationTiles();
        }
        else
        {
            // This is the second pass. It means we already retrieved the location tiles and
            // added them to DB. Since we don't find match in DB, check whether server returned us the location.
            err = NE_OK;
            m_state = NBPGS_Initialized;
            LocationCallback( m_location.valid != 0 ? NE_OK : NE_ERROR_NO_LOCATION_AVAILABLE );
        }
    }
    else if (err == NE_OK)
    {
        //DBGPRINTF("=== OBT_LOC loc.valid=%d", m_location.valid);
        // Match found
        if (m_location.valid == 0)
        {
            // Tile record doesn't contain location, so we calculate it and store to the record.
            CalculatePositionByCellId(cellId, m_location);

            // Set calculated position to appropriate tile record
            if (m_lastSysMode == PRSM_CDMA)
            {
                m_locationContext->m_cacheDb->SetCoordinates(tileID, m_lastCdmaInfo, m_location);
            }
            else if (m_lastSysMode == PRSM_GSM)
            {
                m_locationContext->m_cacheDb->SetCoordinates(tileID, m_lastGsmInfo, m_location);
            }
        }

        // Update tile's timestamp
        m_locationContext->m_cacheDb->UpdateTimestamp(tileID);

        // Notify user
        m_state = NBPGS_Initialized;

        NB_QaLogLocationReply(m_locationContext->internalContext,
            m_location.latitude, m_location.longitude,
            static_cast<unsigned short>(m_location.horizontalUncertaintyAlongAxis),
            source);

        LocationCallback(NE_OK);
    }

    m_firstPass = false;

    if (err != NE_OK)
    {
        NB_QaLogAppState(m_locationContext->internalContext, "Network request error");
        // Reset and notify user about general error.
        ResetState(true);
    }
}

NB_Error NetworkLocationRequest::RequestLocationTiles(void)
{
    NB_Error err = NE_INVAL;
    NB_LocationParameters* parameters = NULL;
    err = NB_LocationParametersCreate(m_reqNBContext, &parameters);

    if (err == NE_OK)
    {
        if (m_lastSysMode == PRSM_CDMA)
        {
            err = NB_LocationParametersAddCdma(parameters,
                                               m_lastCdmaInfo.systemId,
                                               m_lastCdmaInfo.networkId,
                                               m_lastCdmaInfo.cellId,
                                               m_lastCdmaInfo.signalStrength);

            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Location query: cell ID=%d", m_lastCdmaInfo.cellId));
            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Location query: signal strength=%d", m_lastCdmaInfo.signalStrength));
        }
        else if (m_lastSysMode == PRSM_GSM)
        {
            err = NB_LocationParametersAddGsm(parameters,
                                              m_lastGsmInfo.mobileCountryCode,
                                              m_lastGsmInfo.mobileNetworkCode,
                                              m_lastGsmInfo.locationAreaCode,
                                              m_lastGsmInfo.cellId,
                                              m_lastGsmInfo.signalStrength);

            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Location query: cell ID=%d", m_lastGsmInfo.cellId));
            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Location query: signal strength=%d", m_lastGsmInfo.signalStrength));
        }

        err = NB_LocationParametersAskWanted(parameters, TRUE, FALSE);

        if (err == NE_OK)
        {
            NB_RequestHandlerCallback callback = { Static_TPS_RequestCallback, this };

            err = NB_LocationHandlerCreate(m_reqNBContext, &callback, &m_tpsHandler);

            if (err == NE_OK)
            {
                NB_QaLogLocationEvent(m_locationContext->internalContext, NB_QLLE_Request);

                if (m_lastSysMode == PRSM_CDMA)
                {
                    NB_QaLogCellIdRequest(m_locationContext->internalContext, NB_QLLNT_CDMA, m_lastCdmaInfo.signalStrength,
                        m_lastCdmaInfo.systemId, m_lastCdmaInfo.networkId,
                        0, m_lastCdmaInfo.cellId);
                }
                else if (m_lastSysMode == PRSM_GSM)
                {
                    NB_QaLogCellIdRequest(m_locationContext->internalContext, NB_QLLNT_GSM, m_lastGsmInfo.signalStrength,
                        m_lastGsmInfo.mobileCountryCode, m_lastGsmInfo.mobileNetworkCode,
                        m_lastGsmInfo.locationAreaCode, m_lastGsmInfo.cellId);
                }

                err = NB_LocationHandlerStartRequest(m_tpsHandler, parameters);
            }
        }

        NB_LocationParametersDestroy(parameters);
    } // if (err == NE_OK)

    return err;
}

void NetworkLocationRequest::CalculatePositionByCellId(const CellIdRecord& cellIdRecord, NB_LS_Location& location)
{
    double mar = (double)cellIdRecord.antennaMAR;
    double lat = cellIdRecord.antennaLatitude;
    double lon = cellIdRecord.antennaLongitude;
    double accuracy = 0.0;

    if (cellIdRecord.antennaOpening < 180)
    {
        double alpha = TO_RAD(cellIdRecord.antennaOpening/2);
        int offset = 0;
        if (cellIdRecord.antennaOpening <= 90)
        {
            // offset = int(mar / cos(alpha) * 0.5)
            offset = (int)nsl_div( mar, nsl_cos(alpha) ) / 2;
            accuracy = offset + cellIdRecord.antennaHEPE;
        }
        else
        {
            // offset = int(mar * cos(alpha))
            // accuracy = int(mar * sin(alpha) + HEPE)
            offset = (int)nsl_mul( mar, nsl_cos(alpha) );
            accuracy = (int)nsl_mul( mar, nsl_sin(alpha) ) + cellIdRecord.antennaHEPE;
        }
        MovePoint(&lat, &lon, offset, -cellIdRecord.antennaOrientation);
    }
    else
    {
        accuracy = mar + cellIdRecord.antennaHEPE;
    }

    location.latitude = lat;
    location.longitude = lon;
    location.horizontalUncertaintyAlongAxis = accuracy;
    location.horizontalUncertaintyAlongPerpendicular = accuracy;
    location.valid = NBPGV_Latitude | NBPGV_Longitude | NBPGV_AxisUncertainty | NBPGV_PerpendicularUncertainty;
}

void NetworkLocationRequest::ResetState(bool returnError, NB_Error errorCode)
{
    m_cancelled = FALSE;
    m_PALRequestInProgress = FALSE;
    m_state = NBPGS_Initialized;
    m_firstPass = true;
    if (returnError)
    {
        LocationCallback(errorCode);
    }
}

// This procedure calculates new point which is moved from the passed point to distance "offset"
// and angle "angle". Result will be returned to latitude, longitude parameters.
void MovePoint(double* latitude, double* longitude, int32 offset, int32 angle)
{
    if (latitude == NULL || longitude == NULL)
    {
        return;
    }

    double lat = TO_RAD(*latitude);
    double lon = TO_RAD(*longitude);
    double off =  nsl_div(offset, RADIUS_EARTH_METERS); // offset / RADIUS_EARTH_METERS
    double ang = TO_RAD(angle);

    double i = 0, j = 0; // temporary variables

    // lat = PI/2 - acos( sin(lat)*cos(off) + cos(lat)*sin(off)*cos(ang) );
    i = nsl_mul( nsl_sin(lat), nsl_cos(off) );
    j = nsl_mul( nsl_mul(nsl_cos(lat), nsl_sin(off)), nsl_cos(ang));
    i = nsl_acos( nsl_add(i, j) );
    lat = nsl_sub( PI_OVER_TWO, i );

    if (lat != PI_OVER_TWO && lat != -PI_OVER_TWO)
    {
        // mod(y,x) = y - x*floor(y/x)
        // mod is used here to make lon < 2*PI
        // lo = mod(lon + acos( sin(tc)*sin(d)/cos(la) ) + PI/2, 2*PI) - PI
        i = nsl_mul( nsl_sin(ang), nsl_sin(off) );
        j = nsl_div( i, nsl_cos(lat) );
        i = nsl_add( lon, nsl_acos(j) );
        j = nsl_add( i, PI_OVER_TWO );

        // mod(j, TWO_PI)
        i = nsl_div(j, TWO_PI);
        i = nsl_mul(TWO_PI, (int)i);
        j = nsl_sub(j, i);

        lon = nsl_sub(j, PI);
    }

    *latitude = TO_DEG(lat);
    *longitude = TO_DEG(lon);
}

/*! @} */
