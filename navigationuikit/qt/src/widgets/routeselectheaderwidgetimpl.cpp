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
    @file routeselectheaderwidgetimpl.cpp
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

#include "routeselectheaderwidgetimpl.h"
#include "palclock.h"
#include "navigationuikitwrapper.h"
#include <algorithm>
#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

using namespace nbnav;
using namespace nkui;
using namespace locationtoolkit;

RouteSelectHeaderWidgetImpl::RouteSelectHeaderWidgetImpl()
    : WidgetBase(nkui::WGT_ROUTE_SELECT_HEADER)
    , mRouteSelectHeaderViewer(NULL)
    , mRouteSelectHeaderObject(NULL)
    , mSelectRouteName("")
    , mSelectRouteTime("")
    , mSelectRouteDistance("")
    , mSelectRouteType("")
{
}

RouteSelectHeaderWidgetImpl::~RouteSelectHeaderWidgetImpl()
{
      delete mRouteSelectHeaderViewer;
}

void RouteSelectHeaderWidgetImpl::SetTransportationMode(nbnav::TransportationMode mode)
{
}

void RouteSelectHeaderWidgetImpl::SetRouteInfomation(std::vector<RouteInfoData>& routesData, int selectedIndex)
{
    SetSelectRouteName(QString(routesData[selectedIndex].majorRoad.c_str()));
    SetSelectRouteTime(QString(routesData[selectedIndex].tripTime.c_str()));
    SetSelectRouteDistance(QString(routesData[selectedIndex].tripDistance.c_str()));
    SetSelectRouteType(QString("Fastest route"));

    if (mRouteSelectHeaderViewer && mRouteSelectHeaderViewer->rootContext())
    {
        mRouteSelectHeaderViewer->rootContext()->setContextProperty("selectrouteinformationcontroller", this);
    }
}

void RouteSelectHeaderWidgetImpl::RouteSelected(uint32 index)
{
}

void RouteSelectHeaderWidgetImpl::CreateNativeView(QWidget* parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/RouteSelectHeader.qml";
    mRouteSelectHeaderViewer = new QDeclarativeView(parentView);
    mRouteSelectHeaderViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mRouteSelectHeaderViewer->setFixedSize(mRouteSelectHeaderViewer->size());
    mRouteSelectHeaderViewer->setStyleSheet("background-color:bule");

    mRouteSelectHeaderObject = (QObject* )mRouteSelectHeaderViewer->rootObject();
}

QWidget* RouteSelectHeaderWidgetImpl::GetNativeView()
{
    return mRouteSelectHeaderViewer;
}

void RouteSelectHeaderWidgetImpl::OnSetPresenter()
{
}

void RouteSelectHeaderWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

WidgetID RouteSelectHeaderWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void RouteSelectHeaderWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void RouteSelectHeaderWidgetImpl::Show()
{
    if(!mVisible)
    {
          GetNativeView()->show();
          mVisible = true;
    }
}

void RouteSelectHeaderWidgetImpl::SetSelectRouteName(const QString& SelectRouteName)
{
    mSelectRouteName = SelectRouteName;
    emit SendSelectRouteName();
}

QString RouteSelectHeaderWidgetImpl::GetSelectRouteName()
{
    return mSelectRouteName;
}

void RouteSelectHeaderWidgetImpl::SetSelectRouteTime(const QString& SelectRouteTime)
{
    mSelectRouteTime = SelectRouteTime;
    emit SendSelectRouteTime();
}

QString RouteSelectHeaderWidgetImpl::GetSelectRouteTime()
{
    return mSelectRouteTime;
}

void RouteSelectHeaderWidgetImpl::SetSelectRouteDistance(const QString& SelectRouteDistance)
{
    mSelectRouteDistance = SelectRouteDistance;
    emit SendSelectRouteDistance();
}

QString RouteSelectHeaderWidgetImpl::GetSelectRouteDistance()
{
    return mSelectRouteDistance;
}

void RouteSelectHeaderWidgetImpl::SetSelectRouteType(const QString& SelectRouteType)
{
    mSelectRouteType = SelectRouteType;
    emit SendSelectRouteType();
}

QString RouteSelectHeaderWidgetImpl::GetSelectRouteType()
{
    return mSelectRouteType;
}

/*! @} */
