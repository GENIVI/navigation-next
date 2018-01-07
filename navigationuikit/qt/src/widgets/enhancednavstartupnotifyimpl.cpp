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
    @file enhancednavstartupnotifyimpl.cpp
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

#include "enhancednavstartupnotifyimpl.h"
#include "navigationuikitwrapper.h"
#include <QWidget>
#include <QVariant>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

using namespace nkui;
using namespace locationtoolkit;

EnhancedNavStartupNotifyImpl::EnhancedNavStartupNotifyImpl()
    : mWidgetID(nkui::WGT_ENHANCE_NAV_NOTIFICATION)
    , WidgetBase(nkui::WGT_ENHANCE_NAV_NOTIFICATION)
    , mEnhancedNavStartupNotifyViewer(NULL)
    , mEnhancedNavStartupNotifyObject(NULL)
    , mEnhancedNavStartupNotify(NULL)
    , mPresenter(NULL)
{

}

EnhancedNavStartupNotifyImpl::~EnhancedNavStartupNotifyImpl()
{
    if (mEnhancedNavStartupNotifyViewer)
    {
        delete mEnhancedNavStartupNotifyViewer;
        mEnhancedNavStartupNotifyViewer = NULL;
    }
}

void EnhancedNavStartupNotifyImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

WidgetID EnhancedNavStartupNotifyImpl::GetWidgetID()
{
    return mWidgetID;
}

void EnhancedNavStartupNotifyImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void EnhancedNavStartupNotifyImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

void EnhancedNavStartupNotifyImpl::ShowMessage(const std::string& message)
{
    mEnhancedNavStartupNotify->setProperty("text", QVariant(QString(message.c_str())));
}

QWidget* EnhancedNavStartupNotifyImpl::GetNativeView()
{
    return mEnhancedNavStartupNotifyViewer;
}

void EnhancedNavStartupNotifyImpl::CreateNativeView(QWidget* parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/EnhancedNavStartupNotify.qml";

    mEnhancedNavStartupNotifyViewer = new QDeclarativeView(parentView);
    mEnhancedNavStartupNotifyViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mEnhancedNavStartupNotifyViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    mEnhancedNavStartupNotifyObject = (QObject* )mEnhancedNavStartupNotifyViewer->rootObject();
    mEnhancedNavStartupNotify = mEnhancedNavStartupNotifyObject->findChild<QObject*>("enhancedNavStartupNotify");
}


/*! @} */
