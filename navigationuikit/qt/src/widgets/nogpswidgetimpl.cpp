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
    @file nogpswidgetimpl.cpp
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

#include "nogpswidgetimpl.h"
#include "navigationuikitwrapper.h"
#include <QVariant>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

using namespace locationtoolkit;

NoGPSWidgetImpl::NoGPSWidgetImpl(nkui::WidgetID widgetID)
    : mWidgetID(widgetID)
    , mPresenter(NULL)
    , mGpsStatusViewer(NULL)
    , mGpsStatusObject(NULL)
    , mGpsStatus(NULL)
    , WidgetBase(widgetID)
{
}

NoGPSWidgetImpl::~NoGPSWidgetImpl()
{
    if(mGpsStatusViewer != NULL)
    {
        delete mGpsStatusViewer;
        mGpsStatusViewer = NULL;
    }
}

void NoGPSWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

nkui::WidgetID NoGPSWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void NoGPSWidgetImpl::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void NoGPSWidgetImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

void NoGPSWidgetImpl::SetLabel(const std::string label)
{
    if (mGpsStatus)
    {
        mGpsStatus->setProperty("text", QVariant(QString(label.c_str())));
        int width = mGpsStatusObject->property("width").toInt();
        int height = mGpsStatusObject->property("height").toInt();
        mGpsStatusViewer->setFixedSize(width, height);
    }
}

void NoGPSWidgetImpl::SetVisible(bool bVisible)
{
    if(bVisible)
    {
        mGpsStatusViewer->show();
    }
    else
    {
        mGpsStatusViewer->hide();
    }
}

bool NoGPSWidgetImpl::IsVisible()
{
    mGpsStatusViewer->isVisible();
}

QWidget *NoGPSWidgetImpl::GetNativeView()
{
    return mGpsStatusViewer;
}

void NoGPSWidgetImpl::CreateNativeView(QWidget *parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/GpsStatus.qml";
    mGpsStatusViewer = new QDeclarativeView(parentView);
    mGpsStatusViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mGpsStatusViewer->setFixedSize(mGpsStatusViewer->size());
    mGpsStatusViewer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mGpsStatusObject = (QObject* )mGpsStatusViewer->rootObject();
    mGpsStatus = mGpsStatusObject->findChild<QObject*>("gpsStatus");
}
