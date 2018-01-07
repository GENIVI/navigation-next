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
    @file overflowmenuwidgetimpl.cpp
    @date 10/31/2014
    @addtogroup navigationuikit
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

#include "overflowmenuwidgetimpl.h"
#include "NKUIEvent.h"
#include "localizedstringhelperimpl.h"
#include <QWidget>
#include <QDebug>

#define DEFAULT_MENU_WIDTH          110.f
#define DEFAULT_MENU_HEIGHT         30.f
#define BOTTOM_MARGIN               40.f

using namespace nkui;
using namespace locationtoolkit;

OverflowMenuWidgetImpl::OverflowMenuWidgetImpl()
    : mWidget(NULL)
    , WidgetBase(nkui::WGT_OVERFLOW_MENU)
{
}

OverflowMenuWidgetImpl::~OverflowMenuWidgetImpl()
{
}

void OverflowMenuWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

nkui::WidgetID OverflowMenuWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void OverflowMenuWidgetImpl::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void OverflowMenuWidgetImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

QWidget*
OverflowMenuWidgetImpl::GetNativeView()
{
    return mWidget;
}

void
OverflowMenuWidgetImpl::CreateNativeView(QWidget* parentView)
{
    mWidget = new QWidget(parentView);
    mWidget->setStyleSheet("background-color:red");
}

void
OverflowMenuWidgetImpl::AddMenuOptions(const std::vector<MenuOption>& options)
{
}

void
OverflowMenuWidgetImpl::RemoveOpeions()
{

}

void OverflowMenuWidgetImpl::MenuClicked(int index)
{
}

void OverflowMenuWidgetImpl::MenuClosed()
{
}

/*! @} */
