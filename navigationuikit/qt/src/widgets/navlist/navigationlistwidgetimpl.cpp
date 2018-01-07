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
    @file navigationlistwidgetimpl.cpp
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

#include "navigationlistwidgetimpl.h"
#include "navigationuikitwrapper.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include "laneguidanceimageprovider.h"

using namespace nkui;
using namespace locationtoolkit;

NavigationListWidgetImpl::NavigationListWidgetImpl()
    : mNavlistViewer(NULL)
    , mNavlistObject(NULL)
    , mTripDistance("")
    , mArrivalTime("")
    , mTraffic("")
    , WidgetBase(nkui::WGT_NAVIGATION_LIST)
{
}

NavigationListWidgetImpl::~NavigationListWidgetImpl()
{

}

void NavigationListWidgetImpl::Hide()
{
    if(mVisible)
    {
        GetNativeView()->hide();
        mVisible = false;
    }
}

WidgetID NavigationListWidgetImpl::GetWidgetID()
{
    return mWidgetID;
}

void NavigationListWidgetImpl::SetPresenter(NKUIPresenter *presenter)
{
    mPresenter = presenter;
}

void NavigationListWidgetImpl::Show()
{
    if(!mVisible)
    {
        GetNativeView()->show();
        mVisible = true;
    }
}

QWidget* NavigationListWidgetImpl::GetNativeView()
{
    return mNavlistViewer;
}

void NavigationListWidgetImpl::CreateNativeView(QWidget* parentView)
{
    QString qmlFilePath = NavigaitonUIKitWrapper::GetResourcePath() + "/qml/NavList.qml";
    mNavlistViewer = new QDeclarativeView(parentView);
    mNavlistViewer->setSource(QUrl::fromLocalFile(qmlFilePath));
    mNavlistViewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    mNavlistObject = (QObject* )mNavlistViewer->rootObject();
    mNavlistContext = mNavlistViewer->rootContext();

    if(mNavlistViewer && mNavlistContext)
    {
        mNavlistContext->setContextProperty("navlistcontroller",this);
        mNavlistContext->engine()->addImageProvider(QLatin1String("laneimage"),  new LaneguidanceImageProvider);
    }
}

void NavigationListWidgetImpl::OnSetPresenter()
{
}

void NavigationListWidgetImpl::SetDestinationText(const std::string& destinationSide, const std::string& destinationStreet)
{
    SetArrivingTip(QString(destinationSide.c_str()));
    SetDestinationText(QString(destinationStreet.c_str()));
}

QString NavigationListWidgetImpl::GetTripDistance() const
{
    return mTripDistance;
}

QString NavigationListWidgetImpl::GetArrivalTime() const
{
    return mArrivalTime;
}

QString NavigationListWidgetImpl::GetTraffic() const
{
    return mTraffic;
}

bool NavigationListWidgetImpl::GetIsArriving() const
{
    return mIsArriving;
}

QString NavigationListWidgetImpl::GetDestinationChar() const
{
    return mDestinationChar;
}

QString NavigationListWidgetImpl::GetArrivingTip() const
{
    return mArrivingTip;
}

QString NavigationListWidgetImpl::GetDestinationText() const
{
    return mDestinationText;
}

void NavigationListWidgetImpl::SetTripDistance(const QString& TripDistance)
{
    mTripDistance = TripDistance;
    emit TripDistanceChanged();
}

void NavigationListWidgetImpl::SetArrivalTime(const QString& ArrivalTime)
{
    mArrivalTime = ArrivalTime;
    emit ArrivalTimeChanged();
}

void NavigationListWidgetImpl::SetTraffic(const QString& Traffic)
{
    mTraffic = Traffic;
    emit TrafficChanged();
}

QString NavigationListWidgetImpl::TranslateTrafficEevent(nbnav::TrafficEvent::Type type, nbnav::TrafficEvent::TrafficSeverity severity)
{
    QString imagePath;
    switch (type)
    {
    case nbnav::TrafficEvent::Incident:
    {
        switch (severity)
        {
        case nbnav::TrafficEvent::Unknown:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/incident_yellow.png";
            break;
        }
        case nbnav::TrafficEvent::Moderate:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/incident_orange.png";
            break;
        }
        case nbnav::TrafficEvent::Severe:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/incident_red.png";
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    case nbnav::TrafficEvent::Congestion:
    {
        switch (severity)
        {
        case nbnav::TrafficEvent::Unknown:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/congestion_yellow.png";
            break;
        }
        case nbnav::TrafficEvent::Moderate:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/congestion_orange.png";
            break;
        }
        case nbnav::TrafficEvent::Severe:
        {
            imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/congestion_red.png";
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return imagePath;
}

QString NavigationListWidgetImpl::TranslateDelayTime(string delayText, string unit, string delayText2, string unit2)
{
    QString delaytime;
    if(!(delayText.empty() || delayText == "0"))
    {
        delaytime += delayText.c_str();
        delaytime += " ";
        delaytime += unit.c_str();
        delaytime += " ";

        if(!delayText2.empty())
        {
            delaytime += delayText2.c_str();
            delaytime += " ";
            delaytime += unit2.c_str();
            delaytime += " ";
        }

        delaytime += "delay";
    }

    return delaytime;
}

QString NavigationListWidgetImpl::TranslateTrafficSeverity(nbnav::TrafficEvent::TrafficSeverity severity)
{
    QString color;
    switch (severity)
    {
    case nbnav::TrafficEvent::Unknown:
    {
        color = "yellow";
        break;
    }
    case nbnav::TrafficEvent::Moderate:
    {
        color = "orange";
        break;
    }
    case nbnav::TrafficEvent::Severe:
    {
        color = "red";
        break;
    }
    default:
    {
        break;
    }
    }

    return color;
}

void NavigationListWidgetImpl::SetIsArriving(const bool isArriving)
{
    mIsArriving = isArriving;
    emit IsArrivingChanged();
}

void NavigationListWidgetImpl::SetDestinationChar(const QString& destinationChar)
{
    mDestinationChar = destinationChar;
    emit DestinationCharChanged();
}

void NavigationListWidgetImpl::SetArrivingTip(const QString& arrivingTip)
{
    mArrivingTip = arrivingTip;
    emit ArrivingTipChanged();
}

void NavigationListWidgetImpl::SetDestinationText(const QString& destinationText)
{
    mDestinationText = destinationText;
    emit DestinationTextChanged();
}

void NavigationListWidgetImpl::SetListener(NavigationListWidgetListener *listener)
{
}

void NavigationListWidgetImpl::UpdateManeuvers(const std::vector<ManeuverListData>& maneuvers)
{
    if (maneuvers.empty())
    {
        return;
    }

    bool isArriving = maneuvers.size() < 2;
    if(isArriving != mIsArriving)
    {
        SetIsArriving(isArriving);
    }

    if (isArriving)
    {
        const ManeuverListData& maneuver = maneuvers[0];
        SetDestinationChar(QString(maneuver.nextTurnCharacter));
    }
    else
    {
        QList<QSharedPointer<ManeuverDataObject> > dataList;
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
            if(i == 0)
            {
                nmaneuverData->SetIsNormalItem(false);
            }
            else
            {
                nmaneuverData->SetIsNormalItem(true);
            }
            dataList.push_back( nmaneuverData );
        }

        mManeuverDataModel.Clear();
        mManeuverDataModel.AddData( dataList );

        mNavlistContext->setContextProperty("ManeuverModel", &mManeuverDataModel);
    }
}

void NavigationListWidgetImpl::UpdateTrafficEvents(const std::vector<TrafficListData>& trafficEvents)
{
    bool update = false;
    for(size_t i = 0; i < trafficEvents.size(); ++i)
    {
        const TrafficListData& trafficEvent = trafficEvents[i];

        QString delaytime = TranslateDelayTime(trafficEvent.delayText,
                                               trafficEvent.delayUnit,
                                               trafficEvent.delayText2,
                                               trafficEvent.delayUnit2);

        if(delaytime.isEmpty())
        {
            continue;
        }

        QSharedPointer<ManeuverDataObject> maneuverData = mManeuverDataModel.Find(trafficEvent.maneuverID);
        if(!maneuverData.isNull())
        {

            maneuverData.data()->SetDelayTime(delaytime);
            maneuverData.data()->SetTraffic(TranslateTrafficEevent(trafficEvent.type, trafficEvent.severity));
            maneuverData.data()->SetSplitlineColor(TranslateTrafficSeverity(trafficEvent.severity));
            update= true;
        }
    }

    if(update)
    {
        mNavlistContext->setContextProperty("ManeuverModel", &mManeuverDataModel);
    }
}

void NavigationListWidgetImpl::UpdateTitle(const std::string& hint, const std::string& roadName)
{
}

void NavigationListWidgetImpl::PopManeuver()
{
    mNavlistViewer->show();
}

void NavigationListWidgetImpl::UpdateTripDistance(std::string distance)
{
    SetTripDistance(QString(tr(distance.data())));
}

void NavigationListWidgetImpl::UpdateArrivalTime(std::string time)
{
    SetArrivalTime(QString(tr(time.data())));
}

void NavigationListWidgetImpl::UpdateTraffic(char traffic)
{
    QString imagePath;
    switch (traffic)
    {
    case 'R':
    {
        imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_red.png";
        break;
    }
    case 'G':
    {
        imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_green.png";
        break;
    }
    case 'Y':
    {
        imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_yellow.png";
        break;
    }
    default:
    {
        imagePath = NavigaitonUIKitWrapper::GetResourcePath() + "/images/traffic_delay_gray.png";
        break;
    }
    }
    SetTraffic(imagePath);
}

void NavigationListWidgetImpl::UpdateDistance(const char* distance, const char* unit)
{
    QString dis( distance );
    dis += unit;
    mManeuverDataModel.UpdateDistance( dis );
}

/*! @} */
