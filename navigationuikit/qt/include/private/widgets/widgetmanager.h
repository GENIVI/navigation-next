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
    @file widgetmanager.h
    @date 10/20/2014
    @addtogroup navigationuikit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef _WIDGETMANAGER_H_
#define _WIDGETMANAGER_H_

#include "NKUIWidgetManager.h"
#include "NKUIWidgetLayoutConfig.h"
#include <map>
#include <QWidget>

namespace locationtoolkit
{
class WidgetManager : public nkui::NKUIWidgetManager
{
public:
    WidgetManager(QWidget* parent, nkui::WidgetLayoutConfig* config);
    ~WidgetManager();

public:
    /*! after state change, nkuicontroller will call this to relayout the widgets. */
    virtual void LayoutWidgets(const std::set<nkui::Widget*>& activeWidget, nkui::NKUIStateID state);
    /*! screen size changed. */
    virtual void ScreenSizeChanged();
    /*! get a widget by widget id. */
    virtual shared_ptr<nkui::Widget> GetWidget(nkui::WidgetID widgetID);
    /*! get presenter by widgetid. */
    /*! layout widgets. */
    virtual void LayoutWidgets();
    /**
     *  read widget layout data from config file
     *
     *  @return if load config failed return false
     */
    virtual bool InitLayoutConfigs();
    /*! set nkui controller to presenter, then a presenter can ad listern and send event to controller. */
    virtual void SetNKUIController(nkui::NKUIController* controller);

    /*!
     * get the position of widget by widget id and navigation state
     *
     * @param widgetID  widget id
     * @param state     navigation state
     * @return the position of the widget in the given state
     */
    virtual const nkui::WidgetPosition GetWidgetPosition(nkui::WidgetID widgetID, nkui::NKUIStateID state) const;

    /*!
     * get the state widget list
     *
     * @param state state id
     * @return if this state have widget list return true
     */
    virtual bool GetWidgetList(nkui::NKUIStateID state, std::list<nkui::WidgetID>& list);

    /*!
     * get if screen mode is landscape
     *
     * @return true for landscape, false for protrait.
     */
    virtual bool IsLandscape() const;

private:
    QWidget*                                mParentWidget;
    nkui::WidgetLayoutConfig*               mLayoutConfig;
    std::map<nkui::WidgetID, nkui::Widget*> mWidgets;
    nkui::NKUIController*                   mNavController;
    bool                                    mIsLandscape;
    nkui::NKUIStateID                       mState;
    std::set<nkui::Widget*>                 mActiveWidgets;
};
}

#endif // _WIDGETMANAGER_H_
