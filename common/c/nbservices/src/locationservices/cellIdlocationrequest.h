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

    @file     cellidlocationrequest.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    CellIDLocationRequest class declaration
        This class retrieves cellular network data
        (primarily, cell ID and signal strength),
        and makes request for location to NB servers.

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

#include "locationrequest.h"
extern "C" {
  #include "palradio.h"
}

class CellIDLocationRequest : public LocationRequest
{
public:
    CellIDLocationRequest(NB_LocationContext* locationContext, NB_Context* nbContext);
    virtual ~CellIDLocationRequest(void);

    virtual NB_Error GetState( NB_LS_LocationState * state );
    bool CellInfoChanged();
    virtual void CancelLocationRequest();
protected:
    virtual NB_Error StartLocationRequest();
    virtual bool IsInProgress();

    virtual NB_LS_Location* RetrieveLocation();
    virtual int GetSpecificTimeout(NB_LS_FixType fixType);

    void ResetState(bool returnError, NB_Error errorCode = NEGPS_GENERAL);
    void SimulatorGetCDMANetworkInfo(PAL_RadioCDMAInfo* cdmaInfo);
    void SetInfo(PAL_RadioSystemMode sysMode, PAL_RadioCDMAInfo* cdmaInfo, PAL_RadioGSMInfo* gsmInfo);
    bool IsNewInfo(PAL_RadioSystemMode sysMode, PAL_RadioCDMAInfo* cdmaInfo, PAL_RadioGSMInfo* gsmInfo);

    static void Static_PAL_SystemMode_RequestCallback(PAL_Error status, PAL_RadioSystemMode systemMode, void* userData);
    void SystemModeRequestResult(PAL_Error status, PAL_RadioSystemMode systemMode);
    static void Static_PAL_CDMA_RequestCallback(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo, void* userData);
    void CDMAInfoRequestResult(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo);
    static void Static_PAL_GSM_RequestCallback(PAL_Error status, PAL_RadioGSMInfo* gsmInfo, void* userData);
    void GSMInfoRequestResult(PAL_Error status, PAL_RadioGSMInfo* gsmInfo);
    static void Static_TPS_RequestCallback(void* handler, NB_RequestStatus status, NB_Error err,
                                      nb_boolean up, int percent, void* userData);
    void TPSRequestResult(void* handler, NB_RequestStatus status, NB_Error err,
                                      nb_boolean up, int percent);


    bool                    m_cancelled;
    bool                    m_PALRequestInProgress;
    NB_LS_LocationState     m_state;
    NB_LocationHandler*     m_tpsHandler;
    NB_LocationParameters*  m_tpsParameters;
    NB_Context*             m_reqNBContext;
    NB_LS_Location          m_location;

    PAL_RadioCDMAInfo       m_lastCdmaInfo;
    PAL_RadioGSMInfo        m_lastGsmInfo;
    bool                    m_cellInfoChanged;
};
/*! @} */
