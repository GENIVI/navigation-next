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
    @file pinbubblelayerimpl.cpp
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#include "pinbubblelayerimpl.h"
#include "NKUIEvent.h"

#include <algorithm>
#include <QWidget>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QtDeclarative/QDeclarativeContext>

using namespace nkui;
using namespace locationtoolkit;

PinBubbleImpl::PinBubbleImpl()
{
}
PinBubbleImpl::~PinBubbleImpl()
{
}

void
PinBubbleImpl::SetPosition(const ScreenPoint& position)
{
}

void
PinBubbleImpl::SetText(const std::string& text)
{
}

void
PinBubbleImpl::Show()
{
}

void
PinBubbleImpl::Hide()
{

}

PinBubbleLayerImpl::PinBubbleLayerImpl()
    : WidgetBase(nkui::WGT_PIN_BUBBLE)
    ,mPinBubbleViewer(NULL)
    ,mPinBubbleObject(NULL)
    ,mPinBubbleText("")
{
}

PinBubbleLayerImpl::~PinBubbleLayerImpl()
{
}

void PinBubbleLayerImpl::Hide()
{
}

WidgetID PinBubbleLayerImpl::GetWidgetID()
{
    return mWidgetID;
}

void PinBubbleLayerImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void PinBubbleLayerImpl::Show()
{
}

QWidget*
PinBubbleLayerImpl::GetNativeView()
{
    return mPinBubbleViewer;
}

void
PinBubbleLayerImpl::CreateNativeView(QWidget* parentView)
{
    mPinBubbleViewer = new QDeclarativeView(parentView);
}

void
PinBubbleLayerImpl::OnSetPresenter()
{
}

PinBubble*
PinBubbleLayerImpl::CreateBubble(const ScreenPoint& position)
{
    PinBubbleImpl* bubble = new PinBubbleImpl();
    mBubbles.push_back(bubble);
    return bubble;
}

static void DeleteBubble(PinBubbleImpl* pBubble)
{
    if(pBubble)
    {
        delete pBubble;
        pBubble = NULL;
    }
}

void
PinBubbleLayerImpl::ClearBubbles()
{
    for(size_t i = 0; i < mBubbles.size(); ++i)
    {
        DeleteBubble(mBubbles[i]);
    }

    mBubbles.clear();
}

void PinBubbleLayerImpl::SetPinBubbleText(const QString& curBubbleText)
{
}

QString PinBubbleLayerImpl::GetPinBubbleText()
{
   return mPinBubbleText;
}

void PinBubbleLayerImpl::SetPinBubbleWidget(PinBubbleDirectionType curPinBubbleDirectionType)
{
}

/*! @} */
