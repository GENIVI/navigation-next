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
 @file         BackgroundPresenter.cpp
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

#include "BackgroundPresenter.h"
#include "NavApiNavUtils.h"
#include "NKUITimerCallbackGuard.h"
extern "C"
{
#include "paluitaskqueue.h"
}

using namespace nkui;

const int DEFAULT_MIN_VALID_DISTANCE        = 50;           /*! meter */
const int DEFAULT_TIMER_INTERVAL_BACKGROUND = 5*60*1000;    /*! millisecond */

/*! implement NKUISetBackgroundParameter */
NKUISetBackgroundParameter::NKUISetBackgroundParameter()
    : m_timerInterval(DEFAULT_TIMER_INTERVAL_BACKGROUND),
      m_minValidDistance(DEFAULT_MIN_VALID_DISTANCE)
{
}

NKUISetBackgroundParameter::NKUISetBackgroundParameter(int timerInterval, int distance)
    :m_timerInterval(timerInterval),
     m_minValidDistance(distance)
{
}

BackgroundPresenter::BackgroundPresenter()
    :m_inBackground(false),
     m_lastGpsLocation(0,0,0,0,0,0,false,0),
     m_isArrived(false),
     m_isValid(NKUI_NEW bool(true))
{
}

BackgroundPresenter::~BackgroundPresenter()
{
    if (m_isValid)
    {
        *m_isValid = false;
    }
}

void BackgroundPresenter::HandleEvent(NKEventID id, void* data)
{
    PresenterBase::HandleEvent(id, data);
}

void BackgroundPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_CURRENTLOCATION_UPDATED:
        {
            if (m_isArrived)
            {
                return;
            }
            const NKUILocation location = m_pNavUIContex->GetCurrentLocation();
            if (m_lastGpsLocation.Valid() == 0 || !m_inBackground)
            {
                m_lastGpsLocation = location;
            }
            else if(IsGpsValidInBackground(location))
            {
                m_lastGpsLocation = location;
                /*! cancel the timer to notity and show dialog*/
                PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                                (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                                this);
                CancelEndTripTimer();

                /*! reset the timer to notity and show dialog*/
                PAL_TimerSet(m_pNavUIContex->GetPalInstance(),
                             m_backgroundParameter.m_timerInterval,
                             (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                             this);
            }
            break;
        }
        case EVT_ENTER_BACKGROUND:
        {
            SetBackground(true);
            break;
        }
        case EVT_ENTER_FOREGROUND:
        {
            SetBackground(false);
            break;
        }
        case EVT_ARRIVAL:
        {
            PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                            (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                            this);
            m_isArrived = true;
        }
        case EVT_STILL_DRIVING:
        {
            CancelEndTripTimer();
            break;
        }
        default:
            break;
    }
}

void BackgroundPresenter::OnActive()
{
    m_pNKUIController->RegisterEvent(EVT_CURRENTLOCATION_UPDATED, this);
    m_pNKUIController->RegisterEvent(EVT_ENTER_BACKGROUND, this);
    m_pNKUIController->RegisterEvent(EVT_ENTER_FOREGROUND, this);
    m_pNKUIController->RegisterEvent(EVT_STILL_DRIVING, this);
    m_pNKUIController->RegisterEvent(EVT_ARRIVAL, this);
}

void BackgroundPresenter::OnDeactivate()
{
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                    this);
    CancelEndTripTimer();
}

void BackgroundPresenter::OnSetWidget()
{

}

void BackgroundPresenter::SetBackground(bool isBackground)
{
    m_inBackground = isBackground;
    if (isBackground && !m_isArrived)
    {
        /*! set timer to notity and show dialog*/
        PAL_TimerSet(m_pNavUIContex->GetPalInstance(),
                     m_backgroundParameter.m_timerInterval,
                     (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                     this);
    }
    else
    {
        PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                        (PAL_TimerCallback *)EventThread_TimerIdleThresholdReached,
                        this);
    }
}

void BackgroundPresenter::CancelEndTripTimer()
{
    /*! cancel the timer to end trip */
    PAL_TimerCancel(m_pNavUIContex->GetPalInstance(),
                    (PAL_TimerCallback *)EventThread_TimerToEndTripCallback,
                    this);
}

bool BackgroundPresenter::IsGpsValidInBackground(const NKUILocation& location) const
{
    double distance = nbnav::NavUtils::CalculateDistance(location.Latitude(),
                                                         location.Longitude(),
                                                         m_lastGpsLocation.Latitude(),
                                                         m_lastGpsLocation.Longitude(),
                                                         NULL);
    if (distance > m_backgroundParameter.m_minValidDistance)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void BackgroundPresenter::EventThread_TimerIdleThresholdReached(PAL_Instance* pal,
                                                                void *userData,
                                                                PAL_TimerCBReason reason)
{
    if (pal && userData && (reason == PTCBR_TimerFired))
    {
        BackgroundPresenter* pThis = static_cast<BackgroundPresenter*>(userData);
        if (pThis->m_isValid && *pThis->m_isValid)
        {
            NKUITimerCallbackGuard<BackgroundPresenter>* param = NKUI_NEW NKUITimerCallbackGuard<BackgroundPresenter>(*pThis,
                                                                                                                      pThis->m_isValid);
            if (param)
            {
                PAL_Error error =  PAL_UiTaskQueueAdd(pal, UIThread_IdleThresholdReachedCallback, param);
                if (error != PAL_Ok)
                {
                    delete param;
                }
            }
        }
    }
}

void BackgroundPresenter::UIThread_IdleThresholdReachedCallback(PAL_Instance* pal, void *userData)
{
    NKUITimerCallbackGuard<BackgroundPresenter>* param = static_cast<NKUITimerCallbackGuard<BackgroundPresenter>*>(userData);
    if (!pal || !param)
    {
        return;
    }
    if (param->m_isValid && *param->m_isValid)
    {
        nbnav::TransportationMode currentMode = param->m_instance.m_pNavUIContex->GetRouteOptions().GetTransportationMode();
        NKUIEventData data = {&currentMode, NULL};
        param->m_instance.m_pNKUIController->SendEvent(EVT_IDLE_THRESHOLD_REACHED, data);
        /*! set timer to end trip */
        PAL_TimerSet(pal,
                     param->m_instance.m_backgroundParameter.m_timerInterval,
                     (PAL_TimerCallback *)EventThread_TimerToEndTripCallback,
                     &param->m_instance);
    }
    delete param;
}

void BackgroundPresenter::EventThread_TimerToEndTripCallback(PAL_Instance* pal,
                                                             void *userData,
                                                             PAL_TimerCBReason reason)
{
    if (pal && userData && (reason == PTCBR_TimerFired))
    {
        BackgroundPresenter* pThis = static_cast<BackgroundPresenter*>(userData);
        if (pThis->m_isValid && *pThis->m_isValid)
        {
            NKUITimerCallbackGuard<BackgroundPresenter>* param = NKUI_NEW NKUITimerCallbackGuard<BackgroundPresenter>(*pThis,
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

void BackgroundPresenter::UIThread_EndTripCallback(PAL_Instance* pal, void *userData)
{
    NKUITimerCallbackGuard<BackgroundPresenter>* param = static_cast<NKUITimerCallbackGuard<BackgroundPresenter>*>(userData);
    if (!pal || !param)
    {
        return;
    }
    if (param->m_isValid && *param->m_isValid)
    {
        nkui::NKUIEventData data = {0};
        /*! close the alert dialog and End Trip */
        param->m_instance.m_pNKUIController->SendEvent(EVT_ALERT_IDLE_THRESHOLD_REACHED, data);
    }
    delete param;
}
/*! @} */
