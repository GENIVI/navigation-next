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
    @file navigationheaderwidgetimpl.cpp
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
#include "navigationheaderwidgetimpl.h"
#include "navigationuikitwrapper.h"
#include <QVariant>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

using namespace locationtoolkit;

NavigationHeaderWidgetImpl::NavigationHeaderWidgetImpl(nkui::WidgetID widgetID)
    : mWidgetID(widgetID)
    , mPresenter(NULL)
    , mHeaderViewer(NULL)
    , mHeaderObject(NULL)
    , mDistance(NULL)
    , mTurn(NULL)
    , mPrimaryStreet(NULL)
    , mSecondaryStreet(NULL)
    , WidgetBase(widgetID)
{
}

NavigationHeaderWidgetImpl::~NavigationHeaderWidgetImpl()
{
    if(mHeaderViewer != NULL)
    {
        delete mHeaderViewer;
        mHeaderViewer = NULL;
    }
}

void NavigationHeaderWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

nkui::WidgetID NavigationHeaderWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void NavigationHeaderWidgetImpl::SetPresenter(nkui::NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void NavigationHeaderWidgetImpl::Show()
{
}

void NavigationHeaderWidgetImpl::Update(const nkui::UINavigationHeaderData &data)
{
    if (mDistance)
    {
        mDistance->setProperty("text", QVariant(QString(data.distance.c_str())));
    }

    if (mPrimaryStreet)
    {
        mPrimaryStreet->setProperty("text", QVariant(QString(data.primaryText.c_str())));
    }

    if (mSecondaryStreet)
    {
        mSecondaryStreet->setProperty("text", QVariant(QString(data.secondText.c_str())));
    }

    if (mTurn)
    {
        mTurn->setProperty("text", QVariant(QString(QChar(data.turnArrow.character))));
        char colorHX[12];
        nsl_memset(colorHX, 0, sizeof(colorHX));
        snprintf(colorHX, sizeof(colorHX), "#%x", data.turnArrow.color);
        QString str = QString(QLatin1String(colorHX));
        mTurn->setProperty("color", str);
    }

    if (mExit)
    {
        mExit->setProperty("text", QVariant(QString(data.exitText.c_str())));
        mExit->setProperty("visible", QVariant(true));
    }

    // @todo need confirm
    data.viewStyle;

    int width = mHeaderObject->property("width").toInt();
    int height = mHeaderObject->property("height").toInt();
    mHeaderViewer->setFixedSize(width, height);
}

void NavigationHeaderWidgetImpl::UpdateTripDistance(const string &distance)
{
    if (mDistance)
    {
        mDistance->setProperty("text", QVariant(QString(distance.c_str())));
        int width = mHeaderObject->property("width").toInt();
        int height = mHeaderObject->property("height").toInt();
        mHeaderViewer->setFixedSize(width, height);
    }
}

QWidget *NavigationHeaderWidgetImpl::GetNativeView()
{
    return mHeaderViewer;
}

void NavigationHeaderWidgetImpl::CreateNativeView(QWidget *parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/Header.qml";
    mHeaderViewer = new QDeclarativeView(parentView);
    mHeaderViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mHeaderViewer->setFixedSize(mHeaderViewer->size());
    mHeaderViewer->setStyleSheet("background-color:bule");

    mHeaderObject = (QObject* )mHeaderViewer->rootObject();
    mDistance = mHeaderObject->findChild<QObject*>("distance");
    mTurn = mHeaderObject->findChild<QObject*>("turn");
    mPrimaryStreet = mHeaderObject->findChild<QObject*>("primaryStreet");
    mSecondaryStreet = mHeaderObject->findChild<QObject*>("secondaryStreet");
    mExit = mHeaderObject->findChild<QObject*>("exit");
}
