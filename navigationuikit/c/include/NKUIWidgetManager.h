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
@file         NKUIWidgetManager.h
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

#ifndef __NKUI_NKUIWidgetManager_h
#define __NKUI_NKUIWidgetManager_h

#include "NKUIStateID.h"
#include "WidgetID.h"
#include "Widget.h"
#include "smartpointer.h"
#include "NKUIData.h"
#include <set>
#include <list>

namespace nkui
{
/*! widget manager interface
    this manager will be responsible for create widget and presenter and layout widgets.
    this interface implement is platform depended
 */
class NKUIWidgetManager
{
public:
    /*! after state change, nkuicontroller will call this to relayout the widgets. */
    virtual void LayoutWidgets(const std::set<Widget*>& activeWidget, NKUIStateID state) = 0;
    /*! screen size changed. */
    virtual void ScreenSizeChanged() = 0;
    /*! get a widget by widget id. */
    virtual shared_ptr<Widget> GetWidget(WidgetID widgetID) = 0;
    /*! get presenter by widgetid. */
    /*! layout widgets. */
    virtual void LayoutWidgets() = 0;
    /**
     *  read widget layout data from config file
     *
     *  @return if load config failed return false
     */
    virtual bool InitLayoutConfigs() = 0;
    /*! set nkui controller to presenter, then a presenter can ad listern and send event to controller. */
    virtual void SetNKUIController(NKUIController* controller) = 0;

    /*!
     * get the position of widget by widget id and navigation state
     *
     * @param widgetID  widget id
     * @param state     navigation state
     * @return the position of the widget in the given state
     */
    virtual const WidgetPosition GetWidgetPosition(WidgetID widgetID, NKUIStateID state) const = 0;

    /*!
     * get the state widget list
     *
     * @param state state id
     * @return if this state have widget list return true
     */
    virtual bool GetWidgetList(NKUIStateID state, std::list<WidgetID>& list) = 0;

    /*!
     * get if screen mode is landscape
     *
     * @return true for landscape, false for protrait.
     */
    virtual bool IsLandscape() const = 0;
};

typedef shared_ptr<NKUIWidgetManager> NKUIWidgetManagerPtr;

}

#endif


/*! @} */
