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
    @file         NoGPSPresenter.cpp
    @defgroup     NavigationUIKit
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

#include "NoGPSPresenter.h"
#include "NKUITimerCallbackGuard.h"
extern "C"
{
#include "paluitaskqueue.h"
}

using namespace nkui;

const int NOGPS_SHOW_TIME       = 25000;    // 25 seconds.
const int NOGPS_END_TRIP_TIME   = 600000;   // 10 minutes

NoGPSPresenter::NoGPSPresenter()
    :m_pNoGPSWidget(NULL),
     m_isVisible(false),
     m_isSetEndTripTimer(false),
     m_isValid(NKUI_NEW bool(true))
{
}

NoGPSPresenter::~NoGPSPresenter()
{
    if (m_isValid)
    {
        *m_isValid = false;
    }
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_NoGPSTimerCallback,
                    this);
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_EndTripTimerCallback,
                    this);
}

void NoGPSPresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void NoGPSPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_CURRENTLOCATION_UPDATED:
        {
            if (IsVisible())
            {
                SetVisible(false);
            }
            PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                            (PAL_TimerCallback *)EventThread_NoGPSTimerCallback,
                            this);
            if (m_isSetEndTripTimer)
            {
                PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                                (PAL_TimerCallback *)EventThread_EndTripTimerCallback,
                                this);
            }
            PAL_TimerSet(m_pNavUIContex->GetPalInstance(),
                         NOGPS_SHOW_TIME,
                         (PAL_TimerCallback *)EventThread_NoGPSTimerCallback,
                         this);
            break;
        }
        default:
            break;
    }
}

void NoGPSPresenter::OnActive()
{
    m_pNKUIController->RegisterEvent(EVT_CURRENTLOCATION_UPDATED, this);
    m_pNoGPSWidget->SetLabel(m_pStringHelper->GetLocalizedString("IDS_NO_GPS"));
    SetVisible(m_isVisible);
    PAL_TimerSet(m_pNavUIContex->GetPalInstance(),
                 NOGPS_SHOW_TIME,
                 (PAL_TimerCallback *)EventThread_NoGPSTimerCallback,
                 this);
}

void NoGPSPresenter::OnDeactivate()
{
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_NoGPSTimerCallback,
                    this);
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_EndTripTimerCallback,
                    this);
    m_isSetEndTripTimer = false;
    m_pWidget->Hide();
}

void NoGPSPresenter::OnSetWidget()
{
    m_pNoGPSWidget = dynamic_cast<NoGPSWidget*>(m_pWidget);
}

void NoGPSPresenter::SetVisible(bool bVisible)
{
    m_isVisible = bVisible;
    if (m_isVisible)
    {
        m_pWidget->Show();
    }
    else
    {
        m_pWidget->Hide();
    }
}

bool NoGPSPresenter::IsVisible()
{
    return m_isVisible;
}

void NoGPSPresenter::EventThread_NoGPSTimerCallback(PAL_Instance* pal,
                                                    void *userData,
                                                    PAL_TimerCBReason reason)
{
    if (pal && userData && (reason == PTCBR_TimerFired))
    {
        NoGPSPresenter* pThis = static_cast<NoGPSPresenter*>(userData);
        /*! as NoGps should running in UIThread, but TimerCallback runs in event task thread,
         so we should switch threads */
        if (pThis->m_isValid && *pThis->m_isValid)
        {
            NKUITimerCallbackGuard<NoGPSPresenter>* param = NKUI_NEW NKUITimerCallbackGuard<NoGPSPresenter>(*pThis,
                                                                                                            pThis->m_isValid);
            if (param)
            {
                PAL_Error error =  PAL_UiTaskQueueAdd(pal, UIThread_NoGpsCallback, param);
                if (error != PAL_Ok)
                {
                    delete param;
                }
            }
        }
    }
}

void NoGPSPresenter::UIThread_NoGpsCallback(PAL_Instance* pal, void *userData)
{
    NKUITimerCallbackGuard<NoGPSPresenter>* param = static_cast<NKUITimerCallbackGuard<NoGPSPresenter>*>(userData);
    if (!pal || !param)
    {
        return;
    }
    if (param->m_isValid && *param->m_isValid)
    {
        if (param->m_instance.IsActive())
        {
            param->m_instance.SetVisible(true);
            PAL_TimerSet(pal,
                         NOGPS_END_TRIP_TIME,
                         (PAL_TimerCallback *)EventThread_EndTripTimerCallback,
                         &param->m_instance);
            param->m_instance.m_isSetEndTripTimer = true;
        }
    }
    delete param;
}

void NoGPSPresenter::EventThread_EndTripTimerCallback(PAL_Instance* pal,
                                                      void *userData,
                                                      PAL_TimerCBReason reason)
{
    if (pal && userData && (reason == PTCBR_TimerFired))
    {
        NoGPSPresenter* pThis = static_cast<NoGPSPresenter*>(userData);
        /*! as NoGps should running in UIThread, but this call back runs in event task thread,
         so we should switch threads */
        if (pThis->m_isValid && *pThis->m_isValid)
        {
            pThis->m_isSetEndTripTimer = false;
            NKUITimerCallbackGuard<NoGPSPresenter>* param = NKUI_NEW NKUITimerCallbackGuard<NoGPSPresenter>(*pThis,
                                                                                                            pThis->m_isValid);
            if (param)
            {
                PAL_Error error =  PAL_UiTaskQueueAdd(pal, UIThread_EndTripCallback, param);
                if (error != PAL_Ok)
                {
                    delete param;
                }
            }
        }
    }
}

void NoGPSPresenter::UIThread_EndTripCallback(PAL_Instance* pal, void *userData)
{
    NKUITimerCallbackGuard<NoGPSPresenter>* param = static_cast<NKUITimerCallbackGuard<NoGPSPresenter>*>(userData);
    if (!pal || !param)
    {
        return;
    }
    if (param->m_isValid && *param->m_isValid)
    {
        if (param->m_instance.IsActive())
        {
            NKUIEventData data = {0};
            param->m_instance.m_pNKUIController->SendEvent(EVT_END_TRIP, data);
        }
    }
    delete param;
}

/*! @} */
