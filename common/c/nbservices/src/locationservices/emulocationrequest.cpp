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

    @file     EmuLocationRequest.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        EmuLocationRequest class implementation

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



#include "emulocationrequest.h"
#include "nmeafilereader.h"
#include "nbqalog.h"

#define NBEMULATION_REQUEST_TIMEOUT 10

EmuLocationRequest::EmuLocationRequest(NB_LocationContext* locationContext,
                          char* emulationFilename, int emulationPlayStart)
    : LocationRequest(locationContext),
      m_fileReader(NULL),
      m_state(NBPGS_Undefined),
      m_firstRequest(TRUE)
{
    nsl_memset(&m_location, 0, sizeof(m_location));

    // Here, we should initialize our "player" object
    m_fileReader = new NMEAFileReader(locationContext->m_PAL_Instance,
        emulationFilename, emulationPlayStart);
    if (m_fileReader && m_fileReader->IsInitializedOk())
    {
        m_state = NBPGS_Initialized;
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            100,
            StaticLocationInitCallback, (void*)this);
    }

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Emulation request instance created, state=%d", m_state));
}

EmuLocationRequest::~EmuLocationRequest(void)
{
    // Cancel all possible timers
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticLocationInitCallback, (void*)this);
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticLocationTimerCallback, (void*)this);
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticLocationRequestCallback, (void*)this);

    delete m_fileReader;
}

NB_Error EmuLocationRequest::GetState( NB_LS_LocationState * state )
{
    *state = m_state;
    return NE_OK;
}

int EmuLocationRequest::GetSpecificTimeout(NB_LS_FixType fixType)
{
    return NBEMULATION_REQUEST_TIMEOUT;
}

NB_Error EmuLocationRequest::StartLocationRequest()
{
    if (m_state == NBPGS_Undefined)
    {
        return NE_GPS;
    }
    else if (m_state == NBPGS_Oneshot)
    {
        return NE_GPS_BUSY;
    }

    int timeoutToNext = -1;

    if (m_firstRequest)
    {
        m_firstRequest = FALSE;
        if ((m_fileReader->ReadNextLocation(m_location, timeoutToNext) == NE_OK) &&
            (timeoutToNext >= 0))
        {
            // Start timer to next location
            PAL_TimerSet(m_locationContext->m_PAL_Instance,
                timeoutToNext*1000,
                StaticLocationTimerCallback, (void*)this);
        }
    }

    PAL_Error err = PAL_TimerSet(m_locationContext->m_PAL_Instance,
        100, // "immediate" timer
        StaticLocationRequestCallback, (void*)this);

    if (err == PAL_Ok)
    {
        m_state = NBPGS_Oneshot;
    }

    return NE_OK;
}

bool EmuLocationRequest::IsInProgress()
{
    return (m_state == NBPGS_Oneshot);
}

void EmuLocationRequest::CancelLocationRequest()
{
    m_state = NBPGS_Initialized;

    // Cancel request calback, but do not cancel timer callback
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticLocationRequestCallback, (void*)this);
}

NB_LS_Location* EmuLocationRequest::RetrieveLocation()
{
    return &m_location;
}

void EmuLocationRequest::StaticLocationTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    EmuLocationRequest* thisRequest = (EmuLocationRequest*)userData;

    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        thisRequest->LocationTimerCallbackResult();
    }
}

void EmuLocationRequest::LocationTimerCallbackResult()
{
   int timeoutToNext = -1;

   if ((m_fileReader->ReadNextLocation(m_location, timeoutToNext) == NE_OK) &&
       (timeoutToNext >= 0))
   {
       // Start timer to next location
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            timeoutToNext*1000,
            StaticLocationTimerCallback, (void*)this);
   }
}

void EmuLocationRequest::StaticLocationRequestCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    EmuLocationRequest* thisRequest = (EmuLocationRequest*)userData;

    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        thisRequest->LocationRequestCallbackResult();
    }
}

void EmuLocationRequest::LocationRequestCallbackResult()
{
    NB_Error retStatus = NE_OK;
    m_state = NBPGS_Initialized;

    if (!m_location.valid)
    {
        retStatus = NE_ERROR_NO_LOCATION_AVAILABLE;
    }

    LocationCallback(retStatus);
}

void EmuLocationRequest::StaticLocationInitCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    EmuLocationRequest* thisRequest = (EmuLocationRequest*)userData;

    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        thisRequest->LocationInitCallbackResult();
    }
}

void EmuLocationRequest::LocationInitCallbackResult()
{
    if (m_locationContext->initializeCallback)
    {
        NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LocationInitializeCallback");
        m_locationContext->initializeCallback((void*)m_locationContext->callbackData, m_locationContext);
        NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LocationInitializeCallback");
    }
}

/*! @} */
