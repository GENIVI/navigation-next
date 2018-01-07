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
    @file arrivalheaderwidgetimpl.cpp
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

#include "arrivalheaderwidgetimpl.h"
#include "navigationuikitwrapper.h"
#include "palclock.h"
#include <algorithm>
#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QtDeclarative/QDeclarativeEngine>
#include "laneguidanceimageprovider.h"

using namespace locationtoolkit;
using namespace nkui;

ArrivalHeaderWidgetImpl::ArrivalHeaderWidgetImpl()
    : mHeaderViewer(NULL)
    , mHeaderObject(NULL)
    , mArrivalTip("")
    , mDestination("")
    , WidgetBase(nkui::WGT_ARRIVAL_HEADER)
{
}

ArrivalHeaderWidgetImpl::~ArrivalHeaderWidgetImpl()
{
}

void ArrivalHeaderWidgetImpl::SetText(const std::string& tip, const std::string& destination)
{
    SetArrivalTip(QString(tip.data()));
    SetDestination(QString(destination.data()));
}

QString ArrivalHeaderWidgetImpl::GetArrivalTip() const
{
    return mArrivalTip;
}

QString ArrivalHeaderWidgetImpl::GetDestination() const
{
    return mDestination;
}

void ArrivalHeaderWidgetImpl::SetArrivalTip(const QString& ArrivalTip)
{
    mArrivalTip = ArrivalTip;
    emit ArrivalTipChanged();
}

void ArrivalHeaderWidgetImpl::SetDestination(const QString& Destination)
{
    mDestination = Destination;
    emit DestinationChanged();
}

void ArrivalHeaderWidgetImpl::CreateNativeView(QWidget* parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/ArrivalHeader.qml";
    mHeaderViewer = new QDeclarativeView(parentView);
    if(mHeaderViewer)
    {
        mHeaderViewer->rootContext()->engine()->addImageProvider(QLatin1String("laneimage1"),  new LaneguidanceImageProvider);
        mHeaderViewer->rootContext()->setContextProperty("arrivalheadercontroller",this);
    }
    mHeaderViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mHeaderViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    mHeaderObject = (QObject* )mHeaderViewer->rootObject();
}

QWidget* ArrivalHeaderWidgetImpl::GetNativeView()
{
    return mHeaderViewer;
}

void ArrivalHeaderWidgetImpl::Hide()
{
    if(mVisible)
    {
        mHeaderViewer->hide();
        mVisible = false;
    }
}

WidgetID ArrivalHeaderWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void ArrivalHeaderWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void ArrivalHeaderWidgetImpl::Show()
{
    if(!mVisible)
    {
        mHeaderViewer->show();
        mVisible = true;
    }
}

/*! @} */
