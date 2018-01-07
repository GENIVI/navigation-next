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

    @file     cellidinformation.cpp
    @date     07/07/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        CellIDInformation class implementation

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


#include "cellidinformation.h"

CellIDInformation::CellIDInformation(PAL_RadioContext* radioContext, PAL_Instance* pal,
                                     CellIDInformationReceivedCb  listener, void* userData)
    : m_radioContext(radioContext),
      m_pal(pal),
      m_listener(listener),
      m_userData(userData)
{
    SetInfo(PRSM_NoService, NULL, NULL);
}

CellIDInformation::~CellIDInformation(void)
{
}

PAL_Error CellIDInformation::RequestInformation(void)
{
    if (m_radioContext == NULL || m_pal == NULL || m_listener == NULL)
    {
        return PAL_ErrNoInit;
    }

    return PAL_RadioGetSystemMode(m_radioContext, Static_PAL_SystemMode_RequestCallback, this);
}

void CellIDInformation::Static_PAL_SystemMode_RequestCallback(PAL_Error status,
                                                              PAL_RadioSystemMode systemMode,
                                                              void* userData)
{
    CellIDInformation* pThis = static_cast<CellIDInformation*>(userData);
    pThis->SystemModeRequestResult(status, systemMode);
}

void CellIDInformation::SystemModeRequestResult(PAL_Error status, PAL_RadioSystemMode systemMode)
{
    if (PAL_IsSimulator(m_pal))
    {
        // Simulate radio interface
        PAL_RadioCDMAInfo simCdmaInfo = {0};
        SimulatorGetCDMANetworkInfo(&simCdmaInfo);
        SetInfo(PRSM_CDMA, &simCdmaInfo, NULL);

        m_listener(status, this, m_userData);
        return;
    }

    PAL_Error err = status;
    if (status == PAL_Ok && systemMode != PRSM_NoService)
    {
        // Continue our work. Now we know what our system is; go and retrieve the cell information.
        m_lastSystemMode = systemMode;

        if (systemMode == PRSM_CDMA)
        {
            err = PAL_RadioGetCDMANetworkInfo(m_radioContext, Static_PAL_CDMA_RequestCallback, this);
        }
        else // PRSM_GSM
        {
            err = PAL_RadioGetGSMNetworkInfo(m_radioContext, Static_PAL_GSM_RequestCallback, this);
        }
    }

    if (err != PAL_Ok)
    {
        // Reset and notify user about general error.
        SetInfo(systemMode, NULL, NULL);

        m_listener(status, this, m_userData);
    }
}

void CellIDInformation::Static_PAL_CDMA_RequestCallback(PAL_Error status,
                                                        PAL_RadioCDMAInfo* cdmaInfo,
                                                        void* userData)
{
    CellIDInformation* pThis = static_cast<CellIDInformation*>(userData);
    pThis->CDMAInfoRequestResult(status, cdmaInfo);
}

void CellIDInformation::CDMAInfoRequestResult(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo)
{
    if (status == PAL_Ok)
    {
        SetInfo(PRSM_CDMA, cdmaInfo, NULL);
    }
    else
    {
        SetInfo(PRSM_CDMA, NULL, NULL);
    }

    m_listener(status, this, m_userData);
}

void CellIDInformation::Static_PAL_GSM_RequestCallback(PAL_Error status,
                                                       PAL_RadioGSMInfo* gsmInfo,
                                                       void* userData)
{
    CellIDInformation* pThis = static_cast<CellIDInformation*>(userData);
    pThis->GSMInfoRequestResult(status, gsmInfo);
}

void CellIDInformation::GSMInfoRequestResult(PAL_Error status, PAL_RadioGSMInfo* gsmInfo)
{
    if (status == PAL_Ok)
    {
        SetInfo(PRSM_GSM, NULL, gsmInfo);
    }
    else
    {
        SetInfo(PRSM_GSM, NULL, NULL);
    }

    m_listener(status, this, m_userData);
}

void CellIDInformation::SimulatorGetCDMANetworkInfo(PAL_RadioCDMAInfo* cdmaInfo)
{
    // Standard location, requested to be returned for all simulators.
    // Some cell tower in Aliso Viejo, CA.
    cdmaInfo->systemId = 2;
    cdmaInfo->networkId = 55;
    cdmaInfo->cellId = 5778;
    cdmaInfo->signalStrength = -5;
}

void CellIDInformation::SetInfo(PAL_RadioSystemMode sysMode, PAL_RadioCDMAInfo* cdmaInfo, PAL_RadioGSMInfo* gsmInfo)
{
    m_lastSystemMode = sysMode;
    if (cdmaInfo == NULL && gsmInfo == NULL)
    {
        m_lastCdmaInfo.cellId         = 0;
        m_lastCdmaInfo.networkId      = 0;
        m_lastCdmaInfo.signalStrength = 0;
        m_lastCdmaInfo.systemId       = 0;

        m_lastGsmInfo.cellId            = 0;
        m_lastGsmInfo.locationAreaCode  = 0;
        m_lastGsmInfo.mobileCountryCode = 0;
        m_lastGsmInfo.mobileNetworkCode = 0;
        m_lastGsmInfo.signalStrength    = 0;
    }
    else if (sysMode == PRSM_CDMA && cdmaInfo != NULL)
    {
        m_lastCdmaInfo.cellId         = cdmaInfo->cellId;
        m_lastCdmaInfo.networkId      = cdmaInfo->networkId;
        m_lastCdmaInfo.signalStrength = cdmaInfo->signalStrength;
        m_lastCdmaInfo.systemId       = cdmaInfo->systemId;
    }
    else if (sysMode == PRSM_GSM && gsmInfo != NULL)
    {
        m_lastGsmInfo.cellId            = gsmInfo->cellId;
        m_lastGsmInfo.locationAreaCode  = gsmInfo->locationAreaCode;
        m_lastGsmInfo.mobileCountryCode = gsmInfo->mobileCountryCode;
        m_lastGsmInfo.mobileNetworkCode = gsmInfo->mobileNetworkCode;
        m_lastGsmInfo.signalStrength    = gsmInfo->signalStrength;
    }
}

/*! @} */
