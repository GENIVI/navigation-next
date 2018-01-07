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

    @file     StateManager.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        StateManager class implementation

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



#include "statemanager.h"
#include "locationcontext.h"

extern "C" {
  #include <nbqalog.h>
}

StateManager::StateManager(NB_LocationContext* locationContext)
    : m_locationContext(locationContext),
      m_oneFixState(NBPGS_Undefined),
      m_trackerState(NBPGS_Undefined),
      m_callbackData(NULL)
{
}

StateManager::~StateManager()
{
}

void StateManager::SetUserCallback(NB_LocationDeviceStateCallback callback, void* data)
{
    m_devStateCallback = callback;
    m_callbackData = data;
}

bool StateManager::IsAlreadyInitialized()
{
    if ((m_oneFixState >= NBPGS_Initialized) &&
        (m_trackerState >= NBPGS_Initialized))
    {
        return true;
    }

    return false;
}

void StateManager::Initialize()
{
    if ((m_oneFixState != NBPGS_Initialized) || (m_trackerState != NBPGS_Initialized))
    {
        m_oneFixState  = NBPGS_Initialized;
        m_trackerState = NBPGS_Initialized;

        if (m_devStateCallback)
        {
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LocationDeviceStateCallback");
            m_devStateCallback(m_callbackData, m_locationContext, NBPGS_Initialized);
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LocationDeviceStateCallback");
        }
    }
}

bool StateManager::TrackerIsActive()
{
    if (m_trackerState == NBPGS_Resume || m_trackerState == NBPGS_Suspended ||
        m_trackerState == NBPGS_Tracking_GPS || m_trackerState == NBPGS_Tracking_Network)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_LS_LocationState StateManager::GetState()
{
    if (TrackerIsActive())
    {
        // Active tracker overrides "single-fix" states
        return m_trackerState;
    }
    else
    {
        return m_oneFixState;
    }
}

void StateManager::SetOneFixState(NB_LS_LocationState newState)
{
    if (m_oneFixState != newState)
    {
        m_oneFixState = newState;

        // During active tracker, we do not broadcast one-fix states
        if (!TrackerIsActive() && m_devStateCallback)
        {
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LocationDeviceStateCallback");
            m_devStateCallback(m_callbackData, m_locationContext, m_oneFixState);
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LocationDeviceStateCallback");
        }
    }

}

void StateManager::SetTrackerState(NB_LS_LocationState newState)
{
    if (m_trackerState != newState)
    {
        m_trackerState = newState;

        // If tracker is turned on, or tracker state is switched while tracker is still active,
        // then we translate tracker state.
        NB_LS_LocationState globalState = m_trackerState;

        // If tracker is turned off, then we translate the OneFix state - it could be still active
        if (!TrackerIsActive())
        {
            globalState = m_oneFixState;
        }

        if (m_devStateCallback)
        {
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LocationDeviceStateCallback");
            m_devStateCallback(m_callbackData, m_locationContext, globalState);
            NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LocationDeviceStateCallback");
        }

    }
}

/*! @} */
