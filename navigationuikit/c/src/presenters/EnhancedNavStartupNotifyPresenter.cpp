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
@file         EnhancedNavStartupNotifyPresenter.cpp
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

#include "EnhancedNavStartupNotifyPresenter.h"

using namespace nkui;
using namespace std;
using namespace nbnav;

EnhancedNavStartupNotifyPresenter::EnhancedNavStartupNotifyPresenter() : m_pNotification(NULL)
{
}

EnhancedNavStartupNotifyPresenter::~EnhancedNavStartupNotifyPresenter()
{

}

void EnhancedNavStartupNotifyPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if(event == EVT_EXIT_ENHANCED_STARTUP)
    {
        m_pWidget->Hide();
    }
}

void EnhancedNavStartupNotifyPresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void EnhancedNavStartupNotifyPresenter::OnActive()
{
    if(m_pNavUIContex->IsEnhancedNavigationStartup())
    {
        m_pWidget->Show();
    }
    else
    {
        m_pWidget->Hide();
    }
    m_pNKUIController->RegisterEvent(EVT_EXIT_ENHANCED_STARTUP, this);
}

void EnhancedNavStartupNotifyPresenter::OnDeactivate()
{
    m_pWidget->Hide();
}

void EnhancedNavStartupNotifyPresenter::OnSetWidget()
{
    m_pNotification = dynamic_cast<NotificationWidget*>(m_pWidget);
    m_pNotification->ShowMessage(m_pStringHelper->GetLocalizedString("IDS_ERROR_FINDING_YOUR_LOCATION"));
}

/*! @} */
