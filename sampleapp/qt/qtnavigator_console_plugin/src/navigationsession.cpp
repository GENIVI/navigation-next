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
 *
 */
#include "navigationsession.h"
#include "location.h"
#include "ltkcontext.h"
#include "preferences.h"
#include "routeoptions.h"
#include "routeinformation.h"
#include "util.h"
#include <QDebug>
#include "coordinate.h"

using namespace locationtoolkit;

// NavigationSession Constructor with default initializations
NavigationSession::NavigationSession(QObject *parent)
    : mNavigationImpl(NULL),
      mNavigationStatus(0),
      mEnableTTS(true),
      mIsArrivaled(false),
      mIsStartOffRoute(true)
{

    MapViewController* mainWin = (MapViewController* )(parent);
    QObject::connect(mainWin, SIGNAL(StartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&)), this, SLOT(OnStartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&)) );
    QObject::connect(mainWin, SIGNAL(StopNavigation()),       this, SLOT(OnStopNavigation()) );
    QObject::connect(mainWin, SIGNAL(PlayAnounce(int)),       this, SLOT(OnPlayAnounce(int)) );
    QObject::connect(mainWin, SIGNAL(UpdateGpsFile(QString)), this, SLOT(OnUpdateGpsFile(QString)) );
    QObject::connect(mainWin, SIGNAL(EnableTTS(bool)), this, SLOT(OnEnableTTS(bool)) );
    QObject::connect(mainWin, SIGNAL(startNavigationOnRoute(int)), this, SLOT(onStartNavigationOnRoute(int)));
    QObject::connect(this,SIGNAL(UpdateManeuverList(const ManeuverList&)),  mainWin, SLOT(maneuverListUpdated(const ManeuverList&)));

    QObject::connect(this, SIGNAL(ShowMessageBox(QString, MessageType)),    mainWin, SLOT(OnShowMessageBox(QString, MessageType)) );
    QObject::connect(this, SIGNAL(RouteReceived()),                         mainWin, SIGNAL(RouteReceived()) );
    QObject::connect(this, SIGNAL(TripRemainingTime(quint32)),              mainWin, SIGNAL(TripRemainingTime(quint32)) );
    QObject::connect(this, SIGNAL(TripRemainingDistance(qreal)),            mainWin, SIGNAL(TripRemainingDistance(qreal)) );
    QObject::connect(this, SIGNAL(ManeuverRemainingDistance(qreal)),            mainWin, SLOT(ManeuverRemainingDistanceUpdated(qreal)) );
    QObject::connect(this, SIGNAL(CurrentRoadName(const QString&,const QString&)),        mainWin, SIGNAL(CurrentRoadName(const QString&,const QString&)));
    QObject::connect(this, SIGNAL(NextRoadName(const QString&,const QString&)),           mainWin, SIGNAL(NextRoadName(const QString&,const QString&)));
    QObject::connect(this, SIGNAL(StackTurnImageTTF(const QString&)),                     mainWin, SIGNAL(StackTurnImageTTF(const QString&)));
    QObject::connect(this, SIGNAL(ManeuverImageId(const QString&)),                       mainWin, SIGNAL(ManeuverImageId(const QString&)));

    //    QObject::connect(this, SIGNAL(LaneInformation(const locationtoolkit::LaneInformation&)),mainWin, SIGNAL(LaneInformation(const locationtoolkit::LaneInformation&)));
    QObject::connect(this, SIGNAL(TripTrafficColor(char )),                                 mainWin, SIGNAL(TripTrafficColor(char)));
    QObject::connect(this, SIGNAL(RoadSign(const locationtoolkit::RoadSign&)),              mainWin, SIGNAL(RoadSign(const locationtoolkit::RoadSign&)));

    QObject::connect(this, SIGNAL(ManeuverRemainingDistance(qreal)),        mainWin, SIGNAL(ManeuverRemainingDistance(qreal)) );
    QObject::connect(this, SIGNAL(UpdateManeuverList(const ManeuverList&)), mainWin, SIGNAL(UpdateManeuverList(const ManeuverList&)) );

    //New Nav Stuff for Maneuver Details
    qRegisterMetaType<LTKError>("LTKError");
}

NavigationSession::~NavigationSession()
{
    if( mLocationConfiguration.locationFilename.length() > 0 )
    {
        LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
        locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    }
}

// slot executed when location update available as per GPS file
void NavigationSession::LocationUpdated( const Location& location ) {


    if (mNavigationImpl) {
        // update navigation position
        mNavigationImpl->UpdatePosition(location);
    }
    // this signal will call appropriate location update slot in main window

    if(mNavigationStatus == false) {
        qDebug()<<"@@@@#### "<<QTime::currentTime().toString()<<" NavigationSession::LocationUpdated";
        emit onLocationUpdated(location);
    }

}

void NavigationSession::ProviderStateChanged( LocationProviderState state ) {

}

void NavigationSession::OnLocationError(int errorCode) {

    //for avatar movement to continue
    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );
}

// slot executed from mainwindow to initilize Navigation Object and fetch routes
void NavigationSession::OnStartNavigation(LTKContext& ltkcontext, const Place& place, const RouteOptions& routeoption, const Preferences& preference) {

    qDebug()<<"NavigationSession::OnStartNavigation";
    if(mNavigationStatus == 0) {

        mIsStartOffRoute = true;
        //        QString workPath = LTKSampleUtil::GetWorkFolder();
        QString workPath = LTKSampleUtil::GetResourceFolder();
        mNavigationImpl = Navigation::CreateNavigationWithWorkFoler(ltkcontext, place, routeoption, preference, workPath);

        if (mNavigationImpl) {

            qDebug()<<"############## NavigationSession::OnStartNavigation mNavigationStatus = 1";
            mNavigationStatus = 1;

            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(OffRoute()),                                                             this, SLOT(OnOffRoute()) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(OnRoute()),                                                              this, SLOT(OnRoute()) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteReceived( RouteRequestReason, const QVector< QSharedPointer<RouteInformation> >&)), this, SLOT(OnRouteReceived( RouteRequestReason, const QVector< QSharedPointer<RouteInformation> >&)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteRequested(RouteRequestReason)),                                     this, SLOT(OnRouteRequested(RouteRequestReason)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteProgress(qint32)),                                                  this, SLOT(OnRouteProgress(qint32)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteError(LTKError)),
                              this, SLOT(OnRouteError(LTKError)),
                              Qt::QueuedConnection );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(ArrivingToDestination(DestinationStreetSide )), this, SLOT(OnArrivingToDestination(DestinationStreetSide)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteFinish()), this, SLOT(OnRouteFinish()) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(OffRoutePositionUpdate(qreal)),  this, SLOT(OnOffRoutePositionUpdate(qreal)));

            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(TripRemainingTime(quint32)),               this, SLOT(OnTripRemainingTime(quint32)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(TripRemainingDistance(const qreal&)),      this, SLOT(OnTripRemainingDistance(const qreal&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(ManeuverRemainingDistance(const qreal&)),  this, SLOT(OnManeuverRemainingDistance(const qreal&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(CurrentRoadName(const QString& ,const QString&)),      this, SLOT(OnCurrentRoadName(const QString& ,const QString&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(NextRoadName(const QString& ,const QString&)),         this, SLOT(OnNextRoadName(const QString& ,const QString&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(StackTurnImageTTF(const QString)),         this, SLOT(OnStackTurnImageTTF(const QString&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(UpdateManeuverList(const ManeuverList&)),  this, SLOT(OnUpdateManeuverList(const ManeuverList&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(PositionUpdated(Coordinates,qint32,qint32)),  this, SLOT(OnPositionUpdated(Coordinates,qint32,qint32)) );

            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(ManeuverImageId(const QString&)),          this, SLOT(OnManeuverImageId(const QString&)));
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(SpeedLimit(const SpeedLimitInformation&)), this, SLOT(OnSpeedLimit(const SpeedLimitInformation&)));
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(DisableSpeedLimit()), this, SLOT(OnDisableSpeedLimit()));
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(LaneInformation(const locationtoolkit::LaneInformation&)), this, SLOT(OnLaneInformation(const locationtoolkit::LaneInformation&)));
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(TripTrafficColor(char)),                   this, SLOT(OnTripTrafficColor(char)));
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(RoadSign(const locationtoolkit::RoadSign&)),this, SLOT(OnRoadSign(const locationtoolkit::RoadSign&)));

            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(TrafficAlerted(const TrafficEvent&)), this, SLOT(OnTrafficAlerted(const TrafficEvent&)) );
            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(TrafficChanged(const TrafficInformation&)), this, SLOT(OnTrafficChanged(const TrafficInformation&)) );
            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(DisableTrafficAlerted()), this, SLOT(OnDisableTrafficAlerted()) );

            QObject::connect( &mNavigationImpl->GetAnnouncementSignals(), SIGNAL(Announce(QSharedPointer<Announcement>)), this, SLOT(OnAnnounce(QSharedPointer<Announcement>)));
        }
        else {
            emit ShowMessageBox("Create navigation failed!", MT_Normal);
        }
    }

}

// stop navigation session and location updates
void NavigationSession::OnStopNavigation() {

    mNavigationStatus = 0;

    if (mNavigationImpl) {
        mNavigationImpl->StopSession();
        delete mNavigationImpl;
        mNavigationImpl = NULL;
    }

    //    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    //    locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    //    mLocationConfiguration.locationFilename = "";

}

// select active route for navigation simulation
void NavigationSession::onStartNavigationOnRoute(int routeIdx) {

    mNavigationStatus = 1;

    // start location updates
    //modifyLocationServices(true);
    RouteInformation* route = static_cast<RouteInformation *>(mRouteInformationList[routeIdx].data());
    qDebug() << "Selected Route is: "<<route->GetRouteID()<< "  ---  "<<QString::number(route->GetDistance(), 'f', 2);
    mNavigationImpl->SetActiveRoute(*route);

}


void NavigationSession::OnPlayAnounce(int index) {

}

// set gps file
void NavigationSession::OnUpdateGpsFile(QString gpsfile) {

    qDebug()<<"NavigationSession::OnUpdateGpsFile"<<gpsfile;
    mCurrentGPSFile = gpsfile;
    mLocationConfiguration.emulationMode = true;
    mLocationConfiguration.locationFilename = mCurrentGPSFile;

    mLocationConfiguration.emulationMode = true;
    mLocationConfiguration.locationFilename = mCurrentGPSFile;
    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );
}

void NavigationSession::OnEnableTTS(bool ttsStatus) {

    mEnableTTS = ttsStatus;

}

/*************************************************** SessionSignals *************************************************************************/
/*! Emit when the locations being received by the navigation session are not considered on route to destination. */
void NavigationSession::OnOffRoute() {

}

/*! Emit when the locations being received by the navigation session are considered on route to destination. */
void NavigationSession::OnRoute() {

    mIsStartOffRoute = false;

}

// slot executed when routes received
/*! Emit when received the list of possible routes for the current navigation session. */
void NavigationSession::OnRouteReceived(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes) {

    qDebug()<<"NavigationSession: OnRouteReceived";
    switch (reason) {
    case RRR_Calculate:
        qDebug()<<"OnRouteReceived: Calculate";

        // stop location fixes to prevent avatar animation
        //modifyLocationServices(false);

        break;
    case RRR_Recalculate:
        qDebug()<<"OnRouteReceived: Recalculate";
        break;
    case RRR_RouteSelector:
        qDebug()<<"OnRouteReceived: RouteSelector";

        // stop location fixes to prevent avatar animation
        //modifyLocationServices(false);

        break;
    case RRR_Detour:
        qDebug()<<"OnRouteReceived: Detour";
        break;
    default:
        break;
    }

    mIsArrivaled = false;

    if (!routes.isEmpty()) {
        mRouteInformationList = routes;

        //Draw PolyLine for routes
        emit addPolyLinesForPath(reason, routes);

        if (reason != RRR_RouteSelector) {
            this->onStartNavigationOnRoute(0);
        }
    }

    emit RouteReceived();

    if (!routes.isEmpty()) {
        emit UpdateManeuverList(*(routes.at(0)->GetManeuverList()));
    }

}

// utility function to start-stop location updates
//void NavigationSession::modifyLocationServices(bool startService) {
//    if (startService) {
//        LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
//        locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );
//    }
//    else {
//        LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
//        locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
//    }
//}

/*! Emit when new route requested */
void NavigationSession::OnRouteRequested(RouteRequestReason reason) {

    switch(reason)
    {
    case RRR_Calculate:
        break;
    case RRR_Recalculate:
        break;
    case RRR_RouteSelector:
        break;
    case RRR_Detour:
        break;
    default:
        break;
    }

    qDebug()<<"NavigationSession::OnRouteRequested mNavigationStatus = 0";
    //mNavigationStatus = 0;
}

/*! Emit when the route generation in progress */
void NavigationSession::OnRouteProgress(qint32 progress) {

}

void NavigationSession::OnRouteError(LTKError error) {
    qDebug()<<"NavigationSession::OnRouteError";
    emit RouteError(error);

    OnStopNavigation();

    QString errorText = "Root Error from the server:";
    errorText += error.description();
    errorText += " please check your settings!";
    //    QMessageBox msgBox( QMessageBox::Information, "Root Error", errorText);
    //    msgBox.exec();
}

/*! Emit when the current navigation arriving to destination. */
void NavigationSession::OnArrivingToDestination(DestinationStreetSide streetSide) {

}

/*! Emit when the current navigation session ends. The client will not receive any route updates. */
void NavigationSession::OnRouteFinish() {

    mIsArrivaled = true;
    //    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    //    locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    emit TripRemainingDistance(0);
    emit TripRemainingTime(0);
    emit ManeuverRemainingDistance(0);
    //    emit ShowMessageBox("Arrived destination!", MT_Normal);
    emit RouteFinish();

}

/*! Emit when the heading to origin off route. */
void NavigationSession::OnOffRoutePositionUpdate(qreal headingToRoute) {

    mIsStartOffRoute = true;
    emit OffRoutePositionUpdate(headingToRoute);

}


/*************************************************** NavigationUpdateSignals *************************************************************************/
/*! Emit when trip remaining time updated */
void NavigationSession::OnTripRemainingTime(quint32 time) {

    if (!mIsArrivaled) {
        emit TripRemainingTime(time);
    }

}

/*! Emit when trip remaining delay updated */
void NavigationSession::OnTripRemainingDelay(quint32 time) {

}

/*! Emit when trip remaining distance updated */
void NavigationSession::OnTripRemainingDistance(const qreal& distance) {

    if (!mIsArrivaled) {
        emit TripRemainingDistance(distance);
    }

}

/*! Emit when the name of current road updated. */
void NavigationSession::OnCurrentRoadName(const QString& primaryName, const QString& secondaryName) {

    if (!mIsArrivaled) {
        emit CurrentRoadName(primaryName, secondaryName);
    }

}

/*! Emit when the exit number of current maneuver updated. */
void NavigationSession::OnManeuverExitNumber(const QString& exitNumber) {

}

/*! Emit when the name of next road updated. */
void NavigationSession::OnNextRoadName(const QString& primaryName, const QString& secondaryName) {

    if (!mIsArrivaled) {
        emit NextRoadName(primaryName, secondaryName);
    }
}

/*! Emit when the type of current updated. */
void NavigationSession::OnManeuverType(const QString& type) {

}

/*! Emit when remaining time of maneuver updated. */
void NavigationSession::OnManeuverRemainingTime(quint32 time) {

}

/*! Emit when the remaining time delay of current maneuver updated. */
void NavigationSession::OnManeuverRemainingDelay(quint32 delay) {

}

/*! Emit when the remaining distance of the current maneuver updated. */
void NavigationSession::OnManeuverRemainingDistance(const qreal& distance) {

    if (!mIsArrivaled && !mIsStartOffRoute) {
        emit ManeuverRemainingDistance(distance);
    }

}

/*! Emit when maneuver image ID returned. Use MapUtils for getting bitmap. */
void NavigationSession::OnManeuverImageId(const QString& imageId) {

    if (!mIsArrivaled) {
        emit ManeuverImageId(imageId);
    }
}

/*! Emit when the stack maneuver of turn image text returned. */
void NavigationSession::OnStackTurnImageTTF(const QString& stackImageTTF) {

    if (!mIsArrivaled) {
        emit StackTurnImageTTF(stackImageTTF);
    }
}

/*! Emit when position on route has been changed. */
void NavigationSession::OnPositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading) {

    emit PositionUpdated(coordinates, speed, heading);
    // this signal will call appropriate location update slot in main window

    if(mNavigationStatus) {

        Coordinates c = coordinates;
        Location loc;
        loc.latitude = c.getLatitude();
        loc.longitude = c.getLongitude();
        loc.heading = heading;
        loc.valid = 1 | 2 | 8;

        qDebug()<<"@@@@#### "<<QTime::currentTime().toString()<<" NavigationSession::onPositionUpdated";
        emit onLocationUpdated(loc);
    }
}

/*! Emit when the turn point coordinate of maneuver changed. */
void NavigationSession::OnManeuverPoint(const Coordinates& point) {

}

/*! Emit when updating upcoming maneuver list when maneuver changed. */
void NavigationSession::OnUpdateManeuverList(const ManeuverList& maneuvers) {

    if (!mIsArrivaled) {
        emit UpdateManeuverList(maneuvers);
    }
}

/*! Emit when a traffic event is detected within a specified distance (default is 10km). */
void NavigationSession::OnTrafficAlerted(const TrafficEvent& trafficEvent) {

    emit TrafficAlerted(trafficEvent);
    qDebug()<<"OnTrafficAlerted - Description: "<<trafficEvent.GetDescription();

}

/*! Emit when traffic information is updated */
void NavigationSession::OnTrafficChanged(const TrafficInformation& trafficInfo) {

    emit TrafficChanged(trafficInfo);

    for (int i = 0; i <trafficInfo.GetTrafficEvents().size(); ++i) {
        TrafficEvent * p = trafficInfo.GetTrafficEvents().at(i);
        qDebug()<<"------------------- "<<i<<" ------------------------------";
        qDebug()<<"GetDelay(): "<<p->GetDelay()<<" GetDescription(): "<<p->GetDescription();
        qDebug()<<"GetDistanceToTrafficItem(): "<<p->GetDistanceToTrafficItem()<<"GetEndTime(): "<<p->GetEndTime();
        qDebug()<<"GetLength(): "<<p->GetLength()<<"GetManeuverNumber(): "<<p->GetManeuverNumber();
        qDebug()<<"GetReportTime(): "<<p->GetReportTime()<<"GetRoadName(): "<<p->GetRoadName();
        qDebug()<<"GetSeverity(): "<<p->GetSeverity()<<"GetStartFromTrip(): "<<p->GetStartFromTrip();
        qDebug()<<"GetType(): "<<p->GetType();
        qDebug()<<"----------------------------------------------------------";
    }

}

/*! Emit when disable the traffic alert. */
void NavigationSession::OnDisableTrafficAlerted() {

    qDebug()<<"NavigationSession::OnDisableTrafficAlerted() was called.";
    emit DisableTrafficAlerted();

}

/*! Emit when Speed Limit Updated. */
void NavigationSession::OnSpeedLimit(const SpeedLimitInformation& speedInfo) {

    if (!mIsArrivaled) {
        emit SpeedLimit(speedInfo);
    }

}

void NavigationSession::OnDisableSpeedLimit() {

    emit DisableSpeedLimit();

}

/*! Emit when Lane Information Updated. */
void NavigationSession::OnLaneInformation(const locationtoolkit::LaneInformation& laneInfo) {

    if (!mIsArrivaled) {
        emit LaneInformation(laneInfo);
    }
}

/*! Emit when Traffic color updated. */
void NavigationSession::OnTripTrafficColor(char color) {
    if (!mIsArrivaled) {
        emit TripTrafficColor(color);
    }
}

/*! Emit when Road Sign updated. */
void NavigationSession::OnRoadSign(const locationtoolkit::RoadSign& roadSign) {
    qDebug() << "NavigationSession::OnRoadSign";
    if (!mIsArrivaled) {
        emit RoadSign(roadSign);
    }
}

/*************************************************** AnnouncementSignals *************************************************************************/
/*! provides the announcement related information output stream to get audio data to be played audio text for TTS support */
void NavigationSession::OnAnnounce(QSharedPointer<Announcement> announcement) {

}

#if 0
/*************************************************** PositionUpdateSignals *************************************************************************/
/*! Emit when position updates on route. */
void NavigationSession::PositionUpdate(const PositionInfo& positionInfo) {

}

/*************************************************** ManeuverUpdateSignals *************************************************************************/
/*! Emit when maneuvers on route updates. */
void NavigationSession::ManeuverUpdate(const Maneuver& maneuver) {

}

/*************************************************** NavEventSignals *************************************************************************/
/*! Emit when navigation events updates. */
void NavigationSession::NavEvent(NavEventTypeMask navEventTypeMask, const LaneInformation& laneInfo,
                                 const RoadSign& signInfo, const SpeedLimitInformation& speedLimitInfo,
                                 const TrafficEvent& trafficEvent, const TrafficInformation& trafficInfo) {

}

#endif
