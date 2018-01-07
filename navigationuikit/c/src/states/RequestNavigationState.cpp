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
@file         RequestNavigationState.cpp
@defgroup     nkui
*/
/*
(C) Copyright 2014 by TeleCommunications Systems, Inc.

The information contained herein is confidential, proprietary to
TeleCommunication Systems, Inc., and considered a trade secret as defined
in section 499C of the penal code of the State of California. Use of this
information by anyone other than authorized employees of TeleCommunication
Systems is granted only under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "RequestNavigationState.h"
#include "StartingNavigationState.h"
#include "RTSState.h"
#include "NavigationState.h"
#include "PedestrianNavigationState.h"

using namespace nkui;

static MapSetting s_RequestNavigationMapSetting =
{NAM_Remain, NZTAT_None, NNM_Remain, NCM_Remain, NCTM_Remain, FALSE, FALSE};

RequestNavigationState::RequestNavigationState(NKUIContext* pContext, NKUIState* formerState)
    : NKUIState(pContext, formerState, NAVSTATE_REQUEST_NAVIGATION, s_RequestNavigationMapSetting)
{
    m_locationFeedType = LFT_Map | LFT_Nav;
}

RequestNavigationState::~RequestNavigationState()
{
}

NKUIState*
RequestNavigationState::DoTransition(NKEventID event)
{
    switch (event)
    {
        case EVT_ENT_RTS:
        {
            return NKUI_NEW RTSState(m_pNKUIContext, this);
        }
        case EVT_START_NAV:
        {
            if (m_pNKUIContext->GetRouteOptions().GetTransportationMode() != nbnav::Pedestrian)
            {
                return NKUI_NEW NavigationState(m_pNKUIContext, this);
            }
            else
            {
                return NKUI_NEW PedestrianNavigationState(m_pNKUIContext, this);
            }
        }
        default:
            return NKUIState::DoTransition(event);
    }
}

/*! @} */
