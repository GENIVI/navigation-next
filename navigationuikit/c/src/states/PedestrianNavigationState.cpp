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
    @file         PedestrianNavigationState.cpp
    @defgroup     LNUNavigationUIKit
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
    --------------------------------------------------------------------------
*/

/*! @{ */

#include "nkui_macros.h"

#include "PedestrianNavigationState.h"

#include "NKUIContext.h"

#include "PedestrianTripOverviewState.h"
#include "PedestrianNavigationUnlockedState.h"
#include "ArrivalState.h"
#include "PedestrianNavigationListState.h"
#include "DetourState.h"
#include "NavigationState.h"
#include "NavigationEndTripState.h"

using namespace nkui;

static MapSetting s_PedestrianMapSetting = {NAM_NavMode, NZTAT_None, NNM_TurnByTurn, NCM_PedestrianMode, NCTM_Location, TRUE, TRUE};

PedestrianNavigationState::PedestrianNavigationState(NKUIContext* pContext, NKUIState* formerState)
    : NKUIState(pContext, formerState, NAVSTATE_PEDESTRIAN, s_PedestrianMapSetting)
{
}

PedestrianNavigationState::~PedestrianNavigationState()
{
}

NKUIState*
PedestrianNavigationState::DoTransition(NKEventID event)
{
    switch (event)
    {
        case EVT_MAP_CAMERA_UNLOCKED:
        {
            return NKUI_NEW PedestrianNavigationUnlockedState(m_pNKUIContext, this);
        }
        case EVT_TAP_MINIMAP:
        {
            return NKUI_NEW PedestrianTripOverviewState(m_pNKUIContext, this);
        }
        case EVT_TRIPOVERVIEW_SHOW:
            return NKUI_NEW PedestrianTripOverviewState(m_pNKUIContext, this);
        case EVT_ARRIVAL:
            return NKUI_NEW ArrivalState(m_pNKUIContext, this);
        case EVT_LIST_OPENED:
            return NKUI_NEW PedestrianNavigationListState(m_pNKUIContext, this);
        case EVT_DETOUR_SUCCESS:
            return NKUI_NEW DetourState(m_pNKUIContext, this);
        case EVT_TAP_ON_MAP:
            return NKUI_NEW NavigationEndTripState(m_pNKUIContext, this);
        case EVT_RECALCING:
        case EVT_ROUTE_UPDATE:
        {
            if (m_pNKUIContext->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
            {
                return this;
            }
            else
            {
                return NKUI_NEW NavigationState(m_pNKUIContext, this);
            }
        }
        default:
            return NKUIState::DoTransition(event);
    }
}

/*! @} */
