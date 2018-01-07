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
 @file         BackgroundPresenter.h
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

#ifndef __NavigationUIKit__BackgroundPresenter__
#define __NavigationUIKit__BackgroundPresenter__

extern "C"
{
#include "paltimer.h"
}

#include "PresenterBase.h"

namespace nkui
{
class NKUISetBackgroundParameter
{
public:
    NKUISetBackgroundParameter();
    NKUISetBackgroundParameter(int timerInterval,
                               int distance);
    /*! The time to show dailog when NavUIKit was sent to background. millisecond */
    int m_timerInterval;
    /*!
     * minimum valid distance of gps with the location when NavUIKit was sent to background.
     * if distance >= m_minValidDistance , gps is valid and timer will be reset.
     */
    int m_minValidDistance;
};

class BackgroundPresenter : public PresenterBase
{
public:
    BackgroundPresenter();
    virtual ~BackgroundPresenter();

    // override PresenterBase
    virtual void NotifyEvent(NKEventID event, NKUIEventData data);
    virtual void HandleEvent(NKEventID id, void* data);
protected:
    virtual void OnActive();
    virtual void OnDeactivate();
    virtual void OnSetWidget();
private:
    /*! NavUIKit was sent to background or brought to foreground
     *
     * @param isBackground true means sent to background otherwise brought to foreground
     * @return none.
     */
    void SetBackground(bool isBackground);
    /*! cancel the timer which will end trip
     *
     * @param none.
     * @return none.
     */
    void CancelEndTripTimer();

    /*! methods for idling 5 minutes */
    /*! calcute the distance between the location when NavUIKit sent to background and current location.
     *
     * @param location - current location.
     * @return true if distance more then m_minValidDistance of NKUISetBackgroundParameter, otherwise false.
     */
    bool IsGpsValidInBackground(const NKUILocation& location) const;
    /*! callback to notify idle threshold reached */
    static void EventThread_TimerIdleThresholdReached(PAL_Instance* pal,
                                                      void *userData,
                                                      PAL_TimerCBReason reason);
    static void UIThread_IdleThresholdReachedCallback(PAL_Instance* pal, void *userData);
    /*! callback to end trip if dialog show after 5 minutes */
    static void EventThread_TimerToEndTripCallback(PAL_Instance* pal,
                                                   void *userData,
                                                   PAL_TimerCBReason reason);
    static void UIThread_EndTripCallback(PAL_Instance* pal, void *userData);

    /*! flags if NavUIkit is in background or foreground */
    bool                    m_inBackground;
    /*! stores the last valid gps location */
    NKUILocation            m_lastGpsLocation;
    NKUISetBackgroundParameter m_backgroundParameter;
    bool                    m_isArrived;
    shared_ptr<bool>        m_isValid;
};
}

#endif /* defined(__NavigationUIKit__BackgroundPresenter__) */
/*! @} */