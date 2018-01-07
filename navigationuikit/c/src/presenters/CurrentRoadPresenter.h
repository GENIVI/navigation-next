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
@file         CurrentRoadPresenter.h
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

#ifndef CurrentRoadPresenter__
#define CurrentRoadPresenter__

#include <iostream>
#include "PresenterBase.h"
#include "CurrentRoadWidget.h"

namespace nkui
{
class CurrentRoadPresenter: public PresenterBase,
                            public nbnav::RoutePositionListener,
                            public nbnav::NavEventListener
{
public:
    CurrentRoadPresenter();
    virtual ~CurrentRoadPresenter();
    virtual void NotifyEvent(NKEventID event, NKUIEventData data);

    //override RoutePositionListener
    virtual void CurrentRoadName(std::string primaryName, std::string secondaryName);
protected:
    //override PresenterBase
    virtual void OnActive();
    virtual void OnDeactivate();
    virtual void OnSetWidget();
private:

    /*!
     * Check whether current road name should be hidden.
     *
     * Don't display current road if:
     *  1. SAR and LaneGuidance are both displayed, or
     *  2. It is off-route.
     *
     * @return true if it should be hidden, or false otherwise.
     */
    bool ShouldHideCurrentRoad()
    {
        return ((m_status & NWS_SAR) && (m_status & NWS_LaneGuidance)) ||
                (m_status & NWS_OffRoute) || (m_status & NWS_HideCurrentRoad);
    }

    enum NavWidgetStatus
    {
        NWS_None         = 0,
        NWS_SAR          = 1,
        NWS_LaneGuidance = 1 << 1,
        NWS_OffRoute     = 1 << 2,
        NWS_HideCurrentRoad = 1 <<3,
    };

    CurrentRoadWidget* m_pCurrentRoad;
    uint32             m_status;
};
}

#endif /* defined(CurrentRoadPresenter__) */


/*! @} */
