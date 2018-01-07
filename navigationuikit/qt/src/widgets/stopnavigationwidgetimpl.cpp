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
    @file stopnavigationwidgetimpl.cpp
    @date 10/31/2014
    @addtogroup navigationuikit
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
   --------------------------------------------------------------------------*/

/* See description in header file. */
/*! @{ */

#include "stopnavigationwidgetimpl.h"
#include "navigationuikitwrapper.h"
#include <QWidget>
#include <QDir>
#include <QCoreApplication>

using namespace nkui;
using namespace locationtoolkit;

StopNavigationWidgetImpl::StopNavigationWidgetImpl()
    : WidgetBase(nkui::WGT_STOP_NAVIGATION)
    , mMessageContent("")
    , mConfirmButtonText("")
    , mCancelButtonText("")
{
}

/* See description in header file. */
StopNavigationWidgetImpl::~StopNavigationWidgetImpl()
{
}

void StopNavigationWidgetImpl::SetMessageContent(const QString& messageContent)
{
    mMessageContent = messageContent;
    emit MessageContentChanged();
}

QString StopNavigationWidgetImpl::GetMessageContent()
{
    return mMessageContent;
}

void StopNavigationWidgetImpl::SetConfirmButtonText(const QString& confirmButtonText)
{
    mConfirmButtonText = confirmButtonText;
    emit ConfirmButtonTextChanged();
}

QString StopNavigationWidgetImpl::GetConfirmButtonText()
{
    return mConfirmButtonText;
}

void StopNavigationWidgetImpl::SetCancelButtonText(const QString& cancelButtonText)
{
    mCancelButtonText = cancelButtonText;
    emit CancelButtonTextChanged();
}

QString StopNavigationWidgetImpl::GetCancelButtonText()
{
    return mCancelButtonText;
}

void StopNavigationWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }

}

WidgetID StopNavigationWidgetImpl::GetWidgetID()
{

}

void StopNavigationWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void StopNavigationWidgetImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

/* See description in header file. */
QWidget* StopNavigationWidgetImpl::GetNativeView()
{
    return mEndTripViewer;
}

/* See description in header file. */
void StopNavigationWidgetImpl::CreateNativeView(QWidget* parentView)
{
    mEndTripViewer = new QDeclarativeView(parentView);
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/EndTrip.qml";
    mEndTripViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mEndTripViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    QObject* endTripObject = (QObject* )mEndTripViewer->rootObject();

    QObject::connect(endTripObject, SIGNAL(endTripSignal(bool)), this, SLOT(OnEndTrip(bool)));
    QDeclarativeContext* endTripContext = mEndTripViewer->rootContext();
    if (endTripContext)
    {
        endTripContext->setContextProperty("endtripcontroller",this);
    }
}

void StopNavigationWidgetImpl::OnEndTrip(bool endTrip)
{
    Hide();
    if(endTrip)
    {
        mPresenter->HandleEvent(nkui::EVT_END_TRIP, NULL);
    }
}

/* See description in header file. */
void StopNavigationWidgetImpl::OnSetPresenter()
{
}

/* See description in header file. */
void StopNavigationWidgetImpl::ShowWithParameter(StopNavigationParameter & parameter)
{
    if(!parameter.m_message.empty())
    {
        SetMessageContent(QString(parameter.m_message.data()));
    }
    if(!parameter.m_confirmButtonText.empty())
    {
        SetConfirmButtonText(QString(parameter.m_confirmButtonText.data()));
    }
    if(!parameter.m_cancelButtonText.empty())
    {
        SetCancelButtonText(QString(parameter.m_cancelButtonText.data()));
    }
    GetNativeView()->setGeometry(mEndTripViewer->parentWidget()->width() / 4,
                                 mEndTripViewer->parentWidget()->height() / 3,
                                 mEndTripViewer->parentWidget()->width() / 2,
                                 mEndTripViewer->parentWidget()->height() / 3);
    GetNativeView()->raise();
    Show();
}

/*! @} */
