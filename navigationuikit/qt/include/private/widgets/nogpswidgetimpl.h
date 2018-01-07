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
    @file nogpswidgetimpl.h
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

#ifndef _NOGPSWIDGETIMPL_H_
#define _NOGPSWIDGETIMPL_H_

#include "NoGPSWidget.h"
#include "widgetbase.h"
#include <QtDeclarative/QDeclarativeView>

namespace locationtoolkit
{
class NoGPSWidgetImpl : public nkui::NoGPSWidget,
        public WidgetBase
{
public:
    explicit NoGPSWidgetImpl(nkui::WidgetID widgetID);
    ~NoGPSWidgetImpl();

private:
    /*! hide a widget. */
    virtual void Hide();
    /*! get widget id . */
    virtual nkui::WidgetID GetWidgetID();
    /*! set presenter to widget. */
    virtual void SetPresenter(nkui::NKUIPresenter* presenter);
    /*! show a widget */
    virtual void Show();

    /** Update No GPS label

     @param label: new label.
     @return none.
     **/
    virtual void SetLabel(const std::string label);

    /** Show or hide the NoGPS view

     @param bVisible: true if visible.
     @return none.
     **/
    virtual void SetVisible(bool bVisible);

    /** get if the NoGPS view is visible

     @return true if bisible.
     **/
    virtual bool IsVisible();

    /*
     * implement widget interface
     */
    virtual QWidget* GetNativeView();

    virtual void CreateNativeView(QWidget* parentView);

private:
    nkui::WidgetID mWidgetID;
    nkui::NKUIPresenter *mPresenter;

    QDeclarativeView* mGpsStatusViewer;
    QObject* mGpsStatusObject;
    QObject* mGpsStatus;
};
}

#endif // _NOGPSWIDGETIMPL_H_
