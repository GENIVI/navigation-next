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
    @file         RetryPresenter.cpp
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
    --------------------------------------------------------------------------
*/

/*! @{ */
#include "nkui_macros.h"
#include "RetryPresenter.h"

using namespace std;
using namespace nkui;

RetryPresenter::RetryPresenter()
    : m_pParameters(NULL)
{
}

RetryPresenter::~RetryPresenter()
{
    delete m_pParameters;
}

void
RetryPresenter::HandleEvent(NKEventID event, void* data)
{
    switch (event)
    {
        case EVT_NAV_RETRY_TRIGGERED:
        {
            m_pNavUIContex->RetryEnd();
            m_pNKUIController->Retry();
            break;
        }
        default:
            break;
    }
    PresenterBase::HandleEvent(event, data);
}

void
RetryPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    if (event == EVT_NAV_RETRY)
    {
        if (!m_pParameters)
        {
            m_pParameters                    = NKUI_NEW RetryWidgetParameters;
            m_pParameters->title             = m_pStringHelper->GetLocalizedString("IDS_ROUTE_ERROR");
            m_pParameters->message           = m_pStringHelper->GetLocalizedString("IDS_RETRY_IN_SECONDS");
            m_pParameters->cancelButtonText  = m_pStringHelper->GetLocalizedString("IDS_CANCEL");
            m_pParameters->confirmButtonText = m_pStringHelper->GetLocalizedString("IDS_RETRY");
            m_pParameters->countdown         = 5;
        }

        m_pRetryWidget->ShowWithParameters(*m_pParameters);
    }
}

void
RetryPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNKUIController->RegisterEvent(EVT_NAV_RETRY, this);
    m_pNKUIController->RegisterEvent(EVT_NAV_RETRY_TRIGGERED, this);
}

void
RetryPresenter::OnDeactivate()
{
    PresenterBase::OnDeactivate();
}

void
RetryPresenter::OnSetWidget()
{
    m_pRetryWidget = dynamic_cast<RetryWidget*>(m_pWidget);
}
