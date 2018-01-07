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

    @file     radioinformation.cpp
    @date     07/07/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        RadioInformation class implementation

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


#include "radioinformation.h"
#include "cellidinformation.h"

typedef struct
{
    RadioInformationReceivedCb   listener;
    void*                        userData;
} ListenerData;

RadioInformation::RadioInformation(PAL_RadioContext* radioContext, PAL_Instance* pal)
    : m_cellIdInfo(NULL),
      m_cellIdInProgress(false)
{
    m_cellIdInfo = new CellIDInformation(radioContext, pal, Static_CellId_ReceivedCallback, this);
    m_listeners = CSL_VectorAlloc(sizeof(ListenerData));
}

RadioInformation::~RadioInformation(void)
{
    if ( NULL != m_cellIdInfo )
    {
        delete m_cellIdInfo;
    }

    if ( NULL != m_listeners )
    {
        CSL_VectorDealloc(m_listeners);
    }
}

PAL_Error RadioInformation::RequestInformation(void)
{
    if (m_cellIdInfo == NULL || m_listeners == NULL)
    {
        return PAL_ErrNoInit;
    }

    PAL_Error err = PAL_Ok;
    // If PAL request is already in progress, don't make another one. Just wait until the ongoing request be
    // completed and notify all listeners.
    if (!m_cellIdInProgress)
    {
        m_cellIdInProgress = true;
        err = m_cellIdInfo->RequestInformation();
    }
    // TODO: request Wifi information here

    return err;
}

void RadioInformation::Static_CellId_ReceivedCallback(PAL_Error status, CellIDInformation* information, void* userData)
{
    RadioInformation* pThis = static_cast<RadioInformation*>(userData);
    pThis->CellIdReceived(status, information);
}

void RadioInformation::CellIdReceived(PAL_Error status, CellIDInformation* information)
{
    ListenerData* listenerStruct = NULL;
    int count = 0;

    if ( m_listeners )
    {
        count = CSL_VectorGetLength(m_listeners);
    }

    m_cellIdInProgress = false;

    for (int i = 0; i < count; ++i)
    {
        // notify all listeners
        listenerStruct = static_cast<ListenerData*>(CSL_VectorGetPointer(m_listeners, i));
        listenerStruct->listener(status, this, listenerStruct->userData);
    }

    // clear listeners list
    if ( m_listeners )
    {
        CSL_VectorRemoveAll(m_listeners);
    }
}

void RadioInformation::GetCellIdInformation(PAL_RadioSystemMode& sysMode, PAL_RadioCDMAInfo& cdmaInfo,
                                            PAL_RadioGSMInfo& gsmInfo) const
{
    sysMode = m_cellIdInfo->GetSystemMode();
    if (sysMode == PRSM_CDMA)
    {
        const PAL_RadioCDMAInfo* cdma = m_cellIdInfo->GetCdma();
        cdmaInfo.cellId = cdma->cellId;
        cdmaInfo.networkId = cdma->networkId;
        cdmaInfo.signalStrength = cdma->signalStrength;
        cdmaInfo.systemId = cdma->systemId;
    }
    else if (sysMode == PRSM_GSM)
    {
        const PAL_RadioGSMInfo* gsm = m_cellIdInfo->GetGsm();
        gsmInfo.cellId = gsm->cellId;
        gsmInfo.locationAreaCode = gsm->locationAreaCode;
        gsmInfo.mobileCountryCode = gsm->mobileCountryCode;
        gsmInfo.mobileNetworkCode = gsm->mobileNetworkCode;
        gsmInfo.signalStrength = gsm->signalStrength;
    }
}

void RadioInformation::SetListener(RadioInformationReceivedCb  listener, void* userData)
{
    if (listener != NULL && m_listeners != NULL)
    {
        ListenerData listenerStruct = {0};
        listenerStruct.listener = listener;
        listenerStruct.userData = userData;
        CSL_VectorAppend(m_listeners, &listenerStruct);
    }
}

/*! @} */
