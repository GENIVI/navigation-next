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
 @file         FooterPresenterBase.cpp
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

#include "FooterPresenterBase.h"

#include "FormatController.h"

using namespace nkui;
using namespace std;
using namespace nbnav;

//static const double MileToMeter = 1609.33;
//static const int    DistanceLimit = 500;

FooterPresenterBase::FooterPresenterBase() : m_pFooter(NULL),m_bGettingDetour(false)
{
}

FooterPresenterBase::~FooterPresenterBase()
{

}

void FooterPresenterBase::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_RECALCING_END:
        {
            //when getting detour, ignore this
            if(!m_bGettingDetour)
            {
                m_pFooter->HideMessage();
                m_pFooter->HideProgress();
                m_pFooter->HideIndicator();
                m_pFooter->ShowListButton();
                m_pFooter->ShowMenuButton();
            }
            break;
        }
        case EVT_RECALCING:
        {
            m_pFooter->ShowMessage(m_pStringHelper->GetLocalizedString("IDS_RECALC"));
            m_pFooter->ShowIndicator();
            m_pFooter->HideMenuButton();
            m_pFooter->HideListButton();
            m_pFooter->HidecancelButton();
            break;
        }
        case EVT_GETTING_ROUTE:
        {
            m_pFooter->ShowMessage(m_pStringHelper->GetLocalizedString("IDS_GETTING_ROUTE"));
            m_pFooter->ShowIndicator();
            m_pFooter->HideMenuButton();
            m_pFooter->HideListButton();
            break;
        }
        case EVT_GETTING_DETOUR:
        {
            m_bGettingDetour = true;
            m_pFooter->ShowMessage(m_pStringHelper->GetLocalizedString("IDS_GETTING_DETOUR"));
            m_pFooter->ShowIndicator();
            m_pFooter->HideMenuButton();
            m_pFooter->HideListButton();
            break;
        }
        case EVT_ENDGETTING_DETOUR:
        {
            m_bGettingDetour = false;
            m_pFooter->HideMessage();
            m_pFooter->HideProgress();
            m_pFooter->HideIndicator();
            m_pFooter->ShowListButton();
            m_pFooter->ShowMenuButton();
            break;
        }
        case EVT_ROUTE_PROGRESS:
        {
            int progress = *((int*)data.eventData);
            m_pFooter->ShowProgress((float)progress / 100.f);
            m_pFooter->HideIndicator();
            m_pFooter->HideMenuButton();
            m_pFooter->HideListButton();
            break;
        }
        case EVT_LIST_CLOSED:
        {
            m_pFooter->SetListButtonStatus(false);
            break;
        }
        default:
            break;
    }
}

void FooterPresenterBase::OnActive()
{
    PresenterBase::OnActive();
    m_pNKUIController->RegisterEvent(EVT_RECALCING_END, this);
    m_pNKUIController->RegisterEvent(EVT_RECALCING, this);
    m_pNKUIController->RegisterEvent(EVT_GETTING_DETOUR, this);
    m_pNKUIController->RegisterEvent(EVT_GETTING_ROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_ENDGETTING_DETOUR, this);
    m_pNKUIController->RegisterEvent(EVT_ROUTE_PROGRESS, this);
    m_pNKUIController->RegisterEvent(EVT_LIST_CLOSED, this);
}

void FooterPresenterBase::OnDeactivate()
{
    PresenterBase::OnDeactivate();
}

void FooterPresenterBase::OnSetWidget()
{
    m_pFooter = dynamic_cast<FooterWidget*>(m_pWidget);
}

void FooterPresenterBase::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_END_TRIP_PRESSED)
    {
        NKUIPublicState state = m_pNavUIContex->GetPublicState();
        if(m_pNavUIContex->IsGettingDetour() || state == NPS_Detour)
        {
            id = EVT_BACK_PRESSED;
            m_bGettingDetour = false;

            m_pFooter->HideMessage();
            m_pFooter->HideProgress();
            m_pFooter->HideIndicator();
            m_pFooter->ShowListButton();
            m_pFooter->ShowMenuButton();
        }
    }
    PresenterBase::HandleEvent(id, data);
}
/*! @} */
