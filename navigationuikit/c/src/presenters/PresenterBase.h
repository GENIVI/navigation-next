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
@file         PresenterBase.h
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

#ifndef PresenterBase__
#define PresenterBase__

#include <iostream>
#include "NKUIPresenter.h"
#include "Widget.h"
#include "NKUIController.h"
#include "NKUIStringHelper.h"
#include "NKUIContext.h"

namespace nkui
{
class PresenterBase: public NKUIPresenter, public NKUIEventListener
{
public:
    PresenterBase();
    virtual ~PresenterBase();
    /*! active a widget. */
    virtual void Activate(NKUIContext* navUIContext);
    /*! deactive a widget. */
    virtual void Deactivate();
    virtual bool IsActive();
    /*! widget can send a event to presenter by this function. */
    virtual void HandleEvent(NKEventID id, void* data);
    /*! NKUIController call this and set itself to presenter. */
    virtual void SetNKUIController(NKUIController* controller);
    /*! set a widget to presenter. */
    virtual void SetWidget(Widget* widget);
    /*! set string helper, then can use this to get localized string. */
    virtual void SetStringHelper(NKUIStringHelperPtr stringHelper);
protected:
    /*! when active presenter inherited from presenterbase can use this function do extra things like register a listener or a event. */
    virtual void OnActive();
    /*! when deactive presenter inherited from presenterbase can use this function do extra things like unregister a listener or a event. */
    virtual void OnDeactivate();
    /*! if a presenter inherited from presenterbase need set presenter to native view can add code at this function*/
    virtual void OnSetWidget() = 0;

protected:
    Widget*             m_pWidget;
    NKUIController*     m_pNKUIController;
    NKUIContext*        m_pNavUIContex;
    NKUIStringHelperPtr m_pStringHelper;
private:
    bool                m_bIsActive;
};
}

#endif /* defined(PresenterBase__) */


/*! @} */
