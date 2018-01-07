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
    @file statusbarimpl.cpp
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

#include "statusbarimpl.h"
#include "navigationuikitwrapper.h"
#include <QWidget>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>

using namespace nkui;
using namespace locationtoolkit;

StatusBarImpl::StatusBarImpl()
    : mStatusBarViewer(NULL)
    ,mStatusBarContext(NULL)
    ,mStatusBarObject(NULL)
    ,WidgetBase(nkui::WGT_STATUS_BAR)
    ,mNavigationStatusString("")
{
}

StatusBarImpl::~StatusBarImpl()
{
    delete mStatusBarViewer;
}

void StatusBarImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

QWidget* StatusBarImpl::GetNativeView()
{
    return mStatusBarViewer;
}

void StatusBarImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

WidgetID StatusBarImpl::GetWidgetID()
{
    return mWidgetID;
}

void StatusBarImpl::CreateNativeView(QWidget* parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/StatusBar.qml";

    mStatusBarViewer = new QDeclarativeView(parentView);
    mStatusBarViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mStatusBarViewer->setFixedSize(mStatusBarViewer->size());

    mStatusBarObject = (QObject* )mStatusBarViewer->rootObject();
    mStatusBarContext = mStatusBarViewer->rootContext();
}

void StatusBarImpl::OnSetPresenter()
{

}

void StatusBarImpl::SetNaviationStatusString(QString curNavigationStatusString)
{
    mNavigationStatusString = curNavigationStatusString;
    if (mStatusBarContext)
    {
        mStatusBarContext->setContextProperty("navstatuscontroller",this);
    }
}

QString StatusBarImpl::GetNavigationStatusString()
{
    return mNavigationStatusString;
}

/*! @} */
