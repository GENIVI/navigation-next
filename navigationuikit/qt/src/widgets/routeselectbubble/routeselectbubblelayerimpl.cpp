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
    @file routeselectbubblelayerimpl.cpp
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

#include "routeselectbubblelayerimpl.h"
#include "NKUIEvent.h"
#include "routeselectbubblelayouthelper.h"
#include "routeselectbubbleInternal.h"

#include <algorithm>
#include <QWidget>
#include <QTimer>

using namespace nkui;
using namespace locationtoolkit;

RouteSelectBubbleImpl::RouteSelectBubbleImpl(QWidget* parentView)
    : mWidget(NULL)
    , mPosition()
{
    mWidget = new RouteSelectBubbleInternal(parentView);
    mWidget->hide();
    QObject::connect(mWidget, SIGNAL(clicked()), this, SLOT(OnBubbleClicked()));
}

RouteSelectBubbleImpl::~RouteSelectBubbleImpl()
{
}

void RouteSelectBubbleImpl::Update(const RouteSelectBubbleInfo& info)
{
    mWidget->Update(info);
}

void RouteSelectBubbleImpl::Hide()
{
    mWidget->hide();
}

void RouteSelectBubbleImpl::Show()
{
    if(mWidget->isHidden())
    {
        mWidget->show();
        mWidget->update();
    }
}

const QPoint &RouteSelectBubbleImpl::GetPosition()
{
    return mPosition;
}

int RouteSelectBubbleImpl::GetWidth()
{
    return mWidget->width();
}

int RouteSelectBubbleImpl::GetHeight()
{
    return mWidget->height();
}

void RouteSelectBubbleImpl::OnBubbleClicked()
{
    emit BubbleClicked(this);
}

void
RouteSelectBubbleImpl::SetPosition(const ScreenPoint& position)
{
    mPosition.setX(position.x);
    mPosition.setY(position.y);
}

void
RouteSelectBubbleImpl::SetSelected(bool bSelected)
{
    mWidget->SetSelected(bSelected);
}

void
RouteSelectBubbleImpl::SetText(const std::string& text)
{
    mWidget->SetText(QString(text.c_str()));
}

RouteSelectBubbleLayerImpl::RouteSelectBubbleLayerImpl()
    : WidgetBase(nkui::WGT_ROUTE_SELECT_BUBBLE)
    , mWidget(NULL)
    , mHelper(NULL)
    , haveSetFrame(false)
{
}

RouteSelectBubbleLayerImpl::~RouteSelectBubbleLayerImpl()
{
}

void RouteSelectBubbleLayerImpl::Hide()
{
    if(mVisible)
    {
        if(GetNativeView())
        {
            GetNativeView()->hide();
        }
        mVisible = false;
    }
}

nkui::WidgetID RouteSelectBubbleLayerImpl::GetWidgetID()
{
    return mWidgetID;
}

void RouteSelectBubbleLayerImpl::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void RouteSelectBubbleLayerImpl::Show()
{
}

QWidget*
RouteSelectBubbleLayerImpl::GetNativeView()
{
    return mWidget;
}

void
RouteSelectBubbleLayerImpl::CreateNativeView(QWidget* parentView)
{
    // @todo just use native view to get the geometry of bubble layer
    mWidget = new QWidget(parentView);
    mWidget->hide();
    mHelper = new RouteSelectBubbleLayoutHelper(parentView);
}

void
RouteSelectBubbleLayerImpl::OnSetPresenter()
{
}

void RouteSelectBubbleLayerImpl::UpdateLayout()
{
    QPoint point1;
    QPoint point2;
    QPoint point3;
    if(mBubbles.size() == 1)
    {
        point1 = mBubbles[0]->GetPosition();
    }
    else if(mBubbles.size() == 2)
    {
        point1 = mBubbles[0]->GetPosition();
        point2 = mBubbles[1]->GetPosition();
    }
    else if(mBubbles.size() == 3)
    {
        point1 = mBubbles[0]->GetPosition();
        point2 = mBubbles[1]->GetPosition();
        point3 = mBubbles[2]->GetPosition();
    }

    std::vector<RouteSelectBubbleInfo> bestLayout;
    if(mHelper->NeedCalculateNewLayoutOrNot(point1, point2, point3))
    {
        bestLayout = mHelper->GetBestLayouts(point1,
                                             point2,
                                             point3);
    }
    else
    {
        RouteSelectBubbleInfo bubbleInfo;
        if(!point1.isNull())
        {
            bubbleInfo.rect = mHelper->GetRightBottomBubble(point1);
            bubbleInfo.type = RouteSelectBubbleInfo::BT_RightBottomOff;
            bestLayout.push_back(bubbleInfo);
        }

        if(!point2.isNull())
        {
            bubbleInfo.rect = mHelper->GetRightBottomBubble(point2);
            bubbleInfo.type = RouteSelectBubbleInfo::BT_RightBottomOff;
            bestLayout.push_back(bubbleInfo);
        }

        if(!point3.isNull())
        {
            bubbleInfo.rect = mHelper->GetRightBottomBubble(point3);
            bubbleInfo.type = RouteSelectBubbleInfo::BT_RightBottomOff;
            bestLayout.push_back(bubbleInfo);
        }
    }

    for(size_t i = 0; i < bestLayout.size() && i < mBubbles.size(); ++i)
    {
        mBubbles[i]->Update(bestLayout[i]);
        if(!mHelper->IsOutsetScreenArea(bestLayout[i].rect))
        {
            mBubbles[i]->Show();
        }
        else
        {
            mBubbles[i]->Hide();
        }
    }
}

void RouteSelectBubbleLayerImpl::OnBubbleClicked(const RouteSelectBubbleImpl* bubble)
{
    if(mPresenter)
    {
        int i = 0;
        for(; i < mBubbles.size(); ++i)
        {
            if(bubble == mBubbles[i])
            {
                break;
            }
        }

        mPresenter->HandleEvent(nkui::EVT_NEW_ROUTE_SELECTED, &i);
    }
}

RouteSelectBubble*
RouteSelectBubbleLayerImpl::CreateBubble(const ScreenPoint& position)
{
    RouteSelectBubbleImpl* bubble = new RouteSelectBubbleImpl(mWidget->parentWidget());

    bool result = QObject::connect(bubble, SIGNAL(BubbleClicked(const RouteSelectBubbleImpl*)),
                                   this, SLOT(OnBubbleClicked(const RouteSelectBubbleImpl*)));
    bubble->SetPosition(position);

    mBubbles.push_back(bubble);
    return bubble;
}

void
RouteSelectBubbleLayerImpl::ClearBubbles()
{
    for(size_t i = 0; i < mBubbles.size(); ++i)
    {
        if(mBubbles[i])
        {
            mBubbles[i]->Hide();
            delete mBubbles[i];
            mBubbles[i] = NULL;
        }
    }

    mBubbles.clear();
}

void
RouteSelectBubbleLayerImpl::UpdateLayout(bool bRightNow)
{
    mHelper->SetScreenGeometry(mWidget->geometry());

    if(bRightNow)
    {
        UpdateLayout();
    }
    else
    {
        // @todo camera update can be fraquently, while update bubble layout is costly
        // we only update once every 100 milliseconds
        QTimer::singleShot(100, this, SLOT(UpdateLayout()));
    }
}

void RouteSelectBubbleLayerImpl::SetFrame(nkui::WidgetFrame viewFrame)
{
    WidgetBase::SetFrame(viewFrame);
    mHelper->SetScreenGeometry(mWidget->geometry());
    if(mBubbles.size() > 0)
    {
        mHelper->SetBubbleWidth(mBubbles[0]->GetWidth());
        mHelper->SetBubbleHeight(mBubbles[0]->GetHeight());
    }
    haveSetFrame = true;
    UpdateLayout();
}

/*! @} */
