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
@file         TripOverviewState.cpp
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

#include "ArrivalState.h"
#include "DetourState.h"
#include "NKUIContext.h"
#include "NavigationListState.h"
#include "NavigationSARState.h"
#include "NavigationState.h"
#include "PedestrianNavigationState.h"
#include "PedestrianTripOverviewState.h"
#include "TripOverviewUnlockedState.h"

using namespace nkui;

static MapSetting s_TripOverviewMapSetting =
{NAM_MapMode, NZTAT_FitSingleRoute, NNM_TripOverView, NCM_MapMode, NCTM_Location, TRUE, FALSE};

TripOverviewState::TripOverviewState(NKUIContext* pContext, NKUIState* formerState)
    : NKUIState(pContext, formerState, NAVSTATE_TRIP_OVERVIEW, s_TripOverviewMapSetting)
{
}

TripOverviewState::~TripOverviewState()
{
}

NKUIState*
TripOverviewState::DoTransition(NKEventID event)
{
    switch (event)
    {
        case EVT_MAP_CAMERA_UNLOCKED:
        case EVT_MAP_ZOOM_UPDATED:
        {
            return NKUI_NEW TripOverviewUnlockedState(m_pNKUIContext, this);
        }
        case EVT_TAP_MINIMAP:
            // if previous state is SAR state and still in SAR region then return SAR state.
            if (m_pFormerState->GetStateID() == NAVSTATE_NAVIGATION_SAR &&
                m_pNKUIContext->IsRoadSignShown())
            {
                return NKUI_NEW NavigationSARState(m_pNKUIContext, this);
            }
            else if (m_pNKUIContext->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
            {
                return NKUI_NEW PedestrianNavigationState(m_pNKUIContext, this);
            }
            else
            {
                return NKUI_NEW NavigationState(m_pNKUIContext, this);
            }
        case EVT_ZOOM_FIT_ROUTE:
            return this;
        case EVT_LIST_OPENED:
            return NKUI_NEW NavigationListState(m_pNKUIContext, this);
        case EVT_ARRIVAL:
            return NKUI_NEW ArrivalState(m_pNKUIContext, this);
        case EVT_BACK_PRESSED:
            return GetFormerState();
        case EVT_DETOUR_SUCCESS:
            return NKUI_NEW DetourState(m_pNKUIContext, this);
        case EVT_RECALCING:
        case EVT_GETTING_ROUTE:
        {
            if (m_pNKUIContext->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
            {
                return NKUI_NEW PedestrianTripOverviewState(m_pNKUIContext, this);
            }
            else
            {
                return this;
            }
        }
        case EVT_LEAVE_CALLING_STATE:
            return NKUI_NEW NavigationState(m_pNKUIContext, this);
        case EVT_SAR_HIDE:
            return this;
        default:
            return NKUIState::DoTransition(event);
    }
}

/*! @} */
