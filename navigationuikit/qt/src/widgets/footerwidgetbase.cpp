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
    @file footerwidgetbase.cpp
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

#include "footerwidgetbase.h"
#include "navigationuikitwrapper.h"
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>
#include <QVariant>
#include <QDebug>

using namespace nkui;
using namespace locationtoolkit;

FooterWidgetBase::FooterWidgetBase(WidgetID widgetID)
    : WidgetBase(widgetID)
    , mFooterViewer(NULL)
    , mFooterOptionVisible(false)
{
}

FooterWidgetBase::~FooterWidgetBase()
{

}

QString FooterWidgetBase::GetLeftText() const
{
    return mLeftText;
}

QString FooterWidgetBase::GetMidText() const
{
    return mMidText;
}

QString FooterWidgetBase::GetRightText() const
{
    return mRightText;
}

QString FooterWidgetBase::GetStatusText() const
{
    return mStatusText;
}

bool FooterWidgetBase::GetFooterOptionVisible() const
{
    return mFooterOptionVisible;
}

void FooterWidgetBase::SetLeftText(const QString& leftText)
{
    mLeftText = leftText;
    emit LeftTextChanged();
}

void FooterWidgetBase::SetMidText(const QString& midText)
{
    mMidText = midText;
    emit MidTextChanged();
}

void FooterWidgetBase::SetRightText(const QString& rightText)
{
    mRightText = rightText;
    emit RightTextChanged();
}

void FooterWidgetBase::SetStatusText(const QString &statusText)
{
    mStatusText = statusText;
    emit StatusTextChanged();
}

void FooterWidgetBase::SetFooterOptionVisible(bool visible)
{
    mFooterOptionVisible = visible;
    emit FooterOptionVisibleChanged();
}

void FooterWidgetBase::Hide()
{
    if(mVisible)
    {
        if(mFooterViewer)
        {
            mFooterViewer->hide();
        }
        mVisible = false;
    }
}

nkui::WidgetID FooterWidgetBase::GetWidgetID()
{
    return mWidgetID;
}

void FooterWidgetBase::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void FooterWidgetBase::Show()
{
    if(!mVisible)
    {
        if(mFooterViewer)
        {
            mFooterViewer->show();
        }
        mVisible = true;
    }
}

QWidget* FooterWidgetBase::GetNativeView()
{
    return mFooterViewer;
}

void FooterWidgetBase::CreateNativeView(QWidget *parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/Footer.qml";
    mFooterViewer = new QDeclarativeView(parentView);
    mFooterViewer->rootContext()->setContextProperty("footercontroller",this);
    mFooterViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mFooterViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QObject* footerObject = (QObject* )mFooterViewer->rootObject();

    QObject::connect(footerObject, SIGNAL(footerButtonSignal(int)),
                     this, SLOT(OnFooterButtonSignal(int)));
}

void FooterWidgetBase::ShowProgress(float progress)
{
}

void FooterWidgetBase::HideProgress()
{
}

void FooterWidgetBase::ShowIndicator()
{
}

void FooterWidgetBase::HideIndicator()
{
}

void FooterWidgetBase::ShowMessage(const std::string& message)
{
    SetFooterOptionVisible(false);
    SetStatusText(QString(tr(message.c_str())));
}

void FooterWidgetBase::HideMessage()
{
    SetFooterOptionVisible(true);
    SetStatusText(QString(""));
}

/*! show menu button
*/
void FooterWidgetBase::ShowMenuButton()
{
}

/*! Hide menu button
*/
void FooterWidgetBase::HideMenuButton()
{
}

/*! show list button.
*/
void FooterWidgetBase::ShowListButton()
{
}

/*! Hide list button.
*/
void FooterWidgetBase::HideListButton()
{
}

/*! show cancel button.
*/
void FooterWidgetBase::ShowcancelButton()
{
}

/*! Hide cancel button.
*/
void FooterWidgetBase::HidecancelButton()
{
}

/*! See header file for detail */
void FooterWidgetBase::SetListButtonStatus(bool isOpen)
{
}

void FooterWidgetBase::SetTitle(std::string title)
{
}

/*! @} */
