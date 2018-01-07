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
    @file routeselectstatusbarimpl.cpp
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

#include "routeselectstatusbarimpl.h"
#include "navigationuikitwrapper.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QVariant>

using namespace nkui;
using namespace locationtoolkit;

RouteSelectStatusBarImpl::RouteSelectStatusBarImpl()
    : WidgetBase(nkui::WGT_ROUTE_SELECT_STATUS_BAR),
      mRouteSelectStatusBarViewer(NULL),
      mStatusBarObject(NULL),
      mPresenter(NULL),
      mCurrentIndex(-1)
{
    qDebug()<<"RouteSelectStatusBarImpl::RouteSelectStatusBarImpl ";
    mTextBackground.clear();
    mMouseArea.clear();
}
RouteSelectStatusBarImpl::~RouteSelectStatusBarImpl()
{
    delete mRouteSelectStatusBarViewer;
}

// override WidgetBase
QWidget* RouteSelectStatusBarImpl::GetNativeView()
{
    return mRouteSelectStatusBarViewer;
}

void RouteSelectStatusBarImpl::CreateNativeView(QWidget *parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/RouteSelectStatusBar.qml";

    mRouteSelectStatusBarViewer =  new QDeclarativeView(parentView);
    mRouteSelectStatusBarViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mRouteSelectStatusBarViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    mStatusBarObject = (QObject* )mRouteSelectStatusBarViewer->rootObject();
    bool result = QObject::connect(mStatusBarObject, SIGNAL(routeSelected(int)), this, SLOT(OnRouteSelect(int)) );
    qDebug() << "mRouteSelectStatusBarViewer connect result =" <<result;
    result = QObject::connect(this, SIGNAL(setCurrentRoute(int)), mStatusBarObject, SIGNAL(setCurrentRoute(int)) );
    qDebug() << "mRouteSelectStatusBarViewer connect qml result =" <<result;
    for(int i = 1; i <= 3;i++)
    {
        char textBackground[20]="textBackground";
        sprintf(textBackground,"%s%d",textBackground,i);
        char mouseArea[20]="mouseArea";
        sprintf(mouseArea,"%s%d",mouseArea,i);
        QObject*  textBackgroundObj = mStatusBarObject->findChild<QObject*>(textBackground);
        mTextBackground.push_back(textBackgroundObj);
        QObject*  mouseAreaObj = mStatusBarObject->findChild<QObject*>(mouseArea);
        mMouseArea.push_back(mouseAreaObj);
    }


}

void RouteSelectStatusBarImpl::OnSetPresenter()
{
    qDebug()<<"RouteSelectStatusBarImpl::OnSetPresenter";
}

void RouteSelectStatusBarImpl::Hide()
{
    qDebug()<<"RouteSelectStatusBarImpl::Hide";
    mRouteSelectStatusBarViewer->hide();
}

WidgetID RouteSelectStatusBarImpl::GetWidgetID()
{
    return nkui::WGT_ROUTE_SELECT_STATUS_BAR;
}

void RouteSelectStatusBarImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void RouteSelectStatusBarImpl::Show()
{
    qDebug()<<"RouteSelectStatusBarImpl::Show";
    mRouteSelectStatusBarViewer->show();
}

/*! set route count */
void RouteSelectStatusBarImpl::SetRoute(int count, bool detour)
{
    qDebug()<<"RouteSelectStatusBarImpl::SetRoute count"<<count<<" detour:"<<detour;

    for(int i = 0 ; i < mMouseArea.size() && i< count ;i++)
    {
        mMouseArea[i]->setProperty("visible", QVariant(true));

    }
    for(int left = count ; left < mMouseArea.size() ;left++)
    {
        mMouseArea[left]->setProperty("visible", QVariant(false));
    }
}
/*! set transportation mode. */
void RouteSelectStatusBarImpl::SetCurrentRoute(int index)
{
    qDebug()<<"RouteSelectStatusBarImpl::SetCurrentRoute index"<<index;
    if(index >= mMouseArea.size())
    {
        return;
    }
    mCurrentIndex = index;
    emit setCurrentRoute(index);

}
void RouteSelectStatusBarImpl::OnRouteSelect(int selectIndex)
{
    qDebug()<<"RouteSelectStatusBarImpl::OnRouteSelect selectIndex"<<selectIndex;
    if(mPresenter)
    {
        if(mCurrentIndex != selectIndex)
        {
            int data = selectIndex;
            mPresenter->HandleEvent(EVT_NEW_ROUTE_SELECTED,&data);
            mCurrentIndex = selectIndex;

        }
    }
}

/*! @} */
