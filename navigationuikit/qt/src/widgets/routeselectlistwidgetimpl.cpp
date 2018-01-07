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
    @file routeselectlistwidgetimpl.cpp
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

#include "routeselectlistwidgetimpl.h"
#include "maneuverdataobject.h"
#include "navigationuikitwrapper.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include "laneguidanceimageprovider.h"

using namespace nkui;
using namespace locationtoolkit;

RouteSelectListWidgetImpl::RouteSelectListWidgetImpl()
    :WidgetBase(nkui::WGT_ROUTE_DETALLS_LIST),
      mRouteSelectlistViewer(NULL),
      mRouteSelectlistObject(NULL),
      mRouteSelectlistContext(NULL),
      mSelectedRoute(0)
{
}

RouteSelectListWidgetImpl::~RouteSelectListWidgetImpl()
{
    delete mRouteSelectlistViewer;
}

QWidget*
RouteSelectListWidgetImpl::GetNativeView()
{
    return mRouteSelectlistViewer;
}

int RouteSelectListWidgetImpl::IsRouteTipsEmpty()
{
    if(mRouteTips.count() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void
RouteSelectListWidgetImpl::CreateNativeView(QWidget* parentView)
{  
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/RouteSelectList.qml";
    mRouteSelectlistViewer = new QDeclarativeView(parentView);
    mRouteSelectlistViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mRouteSelectlistViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    mRouteSelectlistObject = (QObject* )mRouteSelectlistViewer->rootObject();
    QObject::connect(mRouteSelectlistObject, SIGNAL(swiproute(int)), this, SLOT(OnSwipRoute(int)) );
    mRouteSelectlistContext = mRouteSelectlistViewer->rootContext();
    if(mRouteSelectlistViewer && mRouteSelectlistContext)
    {
        mRouteSelectlistContext->setContextProperty("rtslistcontroller",this);
        mRouteSelectlistContext->engine()->addImageProvider(QLatin1String("laneimage"),  new LaneguidanceImageProvider);
    }
}

void
RouteSelectListWidgetImpl::OnSwipRoute(int offset)
{
    int data = (mSelectedRoute + mRoutesData.size() + offset) % mRoutesData.size();
    mPresenter->HandleEvent(EVT_NEW_ROUTE_SELECTED,&data);
}

void
RouteSelectListWidgetImpl::OnSetPresenter()
{
    qDebug()<<"RouteSelectListWidgetImpl::OnSetPresenter";
}

void
RouteSelectListWidgetImpl::UpdateManeuvers(const std::vector<ManeuverListData>& maneuvers)
{
    if (maneuvers.empty())
    {
        return;
    }

    QList<QSharedPointer<ManeuverDataObject> > dataList;
    //the first one is rts header's placeholder
    QSharedPointer<ManeuverDataObject> nmaneuverData( new ManeuverDataObject );
    dataList.push_back( nmaneuverData );
    for(size_t i = 0; i < maneuvers.size(); ++i)
    {
        QSharedPointer<ManeuverDataObject> nmaneuverData( new ManeuverDataObject );

        const ManeuverListData& maneuver = maneuvers[i];
        QString distance(maneuver.distanceText.c_str());
        distance += maneuver.distanceUnit.c_str();
        nmaneuverData->SetDistance(distance);
        nmaneuverData->SetCharacter(QString(maneuver.nextTurnCharacter));
        nmaneuverData->SetStreet(QString(maneuver.primary.c_str()));
        nmaneuverData->SetManeuverId(maneuver.maneuverID);
        nmaneuverData->SetSplitlineColor(QString("#ffffff"));
        nmaneuverData->SetCharacterColor(QString("#ffffff"));
        nmaneuverData->SetCharacterColorLast(QString("red"));
        nmaneuverData->SetIsNormalItem(true);
        dataList.push_back( nmaneuverData );
    }

    mManeuverDataModel.Clear();
    mManeuverDataModel.AddData( dataList );

    mRouteSelectlistContext->setContextProperty("ManeuverModel", &mManeuverDataModel);
}

void
RouteSelectListWidgetImpl::UpdateTrafficEvents(const std::vector<TrafficListData>& trafficEvents)
{
    qDebug()<<"RouteSelectListWidgetImpl::UpdateTrafficEvents";
}

void
RouteSelectListWidgetImpl::UpdateTitle(const std::string& hint, const std::string& roadName)
{
    SetStartOn(hint.c_str());
    SetCurrentRoad(roadName.c_str());
}

void
RouteSelectListWidgetImpl::SetTransportationMode(nbnav::TransportationMode mode)
{
    qDebug()<<"RouteSelectListWidgetImpl::SetTransportationMode:"<<mode;
}

void
RouteSelectListWidgetImpl::SetRouteInfomation(std::vector<RouteInfoData>& routesData, int selectedIndex)
{
    mRoutesData = routesData;
    mSelectedRoute = selectedIndex;
    RouteSelected(mSelectedRoute);
}

void
RouteSelectListWidgetImpl::RouteSelected(uint32 index)
{
    mSelectedRoute = index;
    if(mRoutesData.size() > mSelectedRoute)
    {
        RouteInfoData& info = mRoutesData[mSelectedRoute];
        SetTripDistance(info.tripDistance.c_str());
        SetTripTime(info.tripTime.c_str());
        SetViaRoadName(QString(info.majorRoadPrefix.c_str()) + " " + QString(info.majorRoad.c_str()));
        SetRouteTip(info.routeTipText.c_str());
        QString trafficPath;
        switch (info.traffic)
        {
        case 'R':
        {
            trafficPath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_red.png";
            break;
        }
        case 'G':
        {
            trafficPath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_green.png";
            break;
        }
        case 'Y':
        {
            trafficPath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_yellow.png";
            break;
        }
        default:
        {
            trafficPath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_gray.png";
            break;
        }
        }
        SetTraffic(trafficPath);
    }
}

void
RouteSelectListWidgetImpl::UpdateDistance(const char* distance, const char* unit)
{
    qDebug()<<"RouteSelectListWidgetImpl::UpdateDistance ";
}

void
RouteSelectListWidgetImpl::Hide()
{
    qDebug()<<"RouteSelectListWidgetImpl::Hide ";
    if(mRouteSelectlistViewer)
    {
        mRouteSelectlistViewer->hide();
    }

}

WidgetID RouteSelectListWidgetImpl::GetWidgetID()
{
    return nkui::WGT_ROUTE_DETALLS_LIST;
}

void RouteSelectListWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void
RouteSelectListWidgetImpl::Show()
{
    qDebug()<<"RouteSelectListWidgetImpl::Show ";
    if(mRouteSelectlistViewer)
    {
        mRouteSelectlistViewer->show();
    }
}

void RouteSelectListWidgetImpl::OnScreenModeChanged()
{
    qDebug()<<"RouteSelectListWidgetImpl::OnScreenModeChanged ";
}

QString RouteSelectListWidgetImpl::GetTripDistance() const
{
    return mTripDistance;
}

QString RouteSelectListWidgetImpl::GetTripTime() const
{
    return mTripTime;
}

QString RouteSelectListWidgetImpl::GetTraffic() const
{
    return mTraffic;
}

QString RouteSelectListWidgetImpl::GetViaRoadName() const
{
    return mViaRoad;
}

QString RouteSelectListWidgetImpl::GetRouteTip() const
{
    return mRouteTips;
}

QString RouteSelectListWidgetImpl::GetStartOn() const
{
    return mStartOn;
}

QString RouteSelectListWidgetImpl::GetCurrentRoad() const
{
    return mCurrentRoad;
}

void RouteSelectListWidgetImpl::SetTripDistance(const QString& TripDistance)
{
    mTripDistance = TripDistance;
    emit TripDistanceChanged();
}

void RouteSelectListWidgetImpl::SetTripTime(const QString& TripTime)
{
    mTripTime = TripTime;
    emit TripTimeChanged();
}

void RouteSelectListWidgetImpl::SetTraffic(const QString& traffic)
{
    mTraffic = traffic;
    emit TrafficChanged();
}

void RouteSelectListWidgetImpl::SetViaRoadName(const QString& ViaRoad)
{
    mViaRoad = ViaRoad;
    emit ViaRoadNameChanged();
}

void RouteSelectListWidgetImpl::SetRouteTip(const QString& RouteTips)
{
    mRouteTips = RouteTips;
    emit RouteTipChanged();
}

void RouteSelectListWidgetImpl::SetStartOn(const QString& StartOn)
{
    mStartOn = StartOn;
    emit StartOnChanged();
}

void RouteSelectListWidgetImpl::SetCurrentRoad(const QString& CurrentRoad)
{
    mCurrentRoad = CurrentRoad;
    emit CurrentRoadChanged();
}

/*! @} */
