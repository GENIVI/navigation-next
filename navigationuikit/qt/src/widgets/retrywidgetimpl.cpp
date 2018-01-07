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
    @file retrywidgetimpl.cpp
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

#include "retrywidgetimpl.h"
#include "navigationuikitwrapper.h"
#include "NKUIEvent.h"
#include <QWidget>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

using namespace nkui;
using namespace locationtoolkit;

RetryWidgetImpl::RetryWidgetImpl()
    : mWidget(NULL)
    ,mRetryViewer(NULL)
    ,mRetryObject(NULL)
    ,WidgetBase(nkui::WGT_RETRY)
    ,mTimer(NULL)
{
}

RetryWidgetImpl::~RetryWidgetImpl()
{
    delete mRetryViewer;
    delete mWidget;
    DisableTimer();
}


WidgetID RetryWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void RetryWidgetImpl::Show()
{
    //Do nothing
}

void RetryWidgetImpl::Hide()
{

}

void RetryWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

QWidget*
RetryWidgetImpl::GetNativeView()
{
    return mRetryViewer;
}

void
RetryWidgetImpl::CreateNativeView(QWidget* parentView)
{
    // @todo just add temporarily for developing and debuging normally
    //       and will remove when do implementation.
    mWidget = new QWidget(parentView);
    mWidget->setStyleSheet("background-color:grey");
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/Retry.qml";

    mRetryViewer = new QDeclarativeView(parentView);
    mRetryViewer->rootContext()->setContextProperty("controller",this);
    mRetryViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mRetryViewer->setFixedSize(mRetryViewer->size());

    mRetryObject = (QObject* )mRetryViewer->rootObject();

    QObject::connect(mRetryObject, SIGNAL(navrequestretrysignal()), this, SLOT(OnNavigationRequestRetry()));
    QObject::connect(mRetryObject, SIGNAL(navrequestcancelsignal()), this, SLOT(OnNavigationRequestCancel()));
}

void
RetryWidgetImpl::OnSetPresenter()
{
}

void
RetryWidgetImpl::ShowWithParameters(RetryWidgetParameters& parameters)
{
    mTimer = new QTimer(this);
    connect(mTimer,SIGNAL(timeout()),this,SLOT(TimerUpDate()));
    mTimer->setInterval(1000);
    mTimer->start();
    mRetryPar = parameters;
    SetCancelText(mRetryPar.cancelButtonText.c_str());
    SetConfirmText(mRetryPar.confirmButtonText.c_str());
    QString format(mRetryPar.message.c_str());
    QString message = format.arg(mRetryPar.countdown--);
    SetMessage(message);
    GetNativeView()->show();
    GetNativeView()->raise();
}

void RetryWidgetImpl::TimerUpDate()
{
    if(mRetryPar.countdown)
    {
        QString format(mRetryPar.message.c_str());
        QString message = format.arg(mRetryPar.countdown--);
        SetMessage(message);
    }
    else
    {
        OnNavigationRequestRetry();
    }
}

void RetryWidgetImpl::DisableTimer()
{
    if(mTimer)
    {
        mTimer->stop();
        delete mTimer;
        mTimer = NULL;
    }
}

void RetryWidgetImpl::OnNavigationRequestRetry()
{
    DisableTimer();
    GetNativeView()->hide();
    mPresenter->HandleEvent(EVT_NAV_RETRY_TRIGGERED, NULL);
}

void RetryWidgetImpl::OnNavigationRequestCancel()
{
    DisableTimer();
    GetNativeView()->hide();
    mPresenter->HandleEvent(EVT_END_TRIP, NULL);
}

QString RetryWidgetImpl::GetMessage() const
{
    return mMessage;
}
QString RetryWidgetImpl::GetConfirmText() const
{
    return mConfirmText;
}
QString RetryWidgetImpl::GetCancelText() const
{
    return mCancelText;
}

void RetryWidgetImpl::SetMessage(const QString& message)
{
    mMessage = message;
    emit MessageChange(mMessage);
}

void RetryWidgetImpl::SetConfirmText(const QString& confirm)
{
    mConfirmText = confirm;
    emit ConfirmTextChange(mConfirmText);
}

void RetryWidgetImpl::SetCancelText(const QString& cancel)
{
    mCancelText = cancel;
    emit CancelTextChange(mCancelText);
}

/*! @} */
