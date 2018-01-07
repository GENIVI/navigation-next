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
#include "private/navigationsession.h"
#include "location.h"
#include "ltkcontext.h"
#include "preferences.h"
#include "routeoptions.h"
#include "routeinformation.h"
#include <QDebug>
#include <QMessageBox>

using namespace locationtoolkit;


NavigationSession::NavigationSession(QWidget *parent)
    : mNavigationImpl(NULL),
      mNavigationStatus(0),
      mEnableTTS(false)
{
    MainWindow* mainWin = (MainWindow* )(parent);
    QObject::connect(mainWin, SIGNAL(StartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&)), this, SLOT(OnStartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&)) );
    QObject::connect(mainWin, SIGNAL(StopNavigation()),       this, SLOT(OnStopNavigation()) );
    QObject::connect(mainWin, SIGNAL(PlayAnounce(int)),       this, SLOT(OnPlayAnounce(int)) );
    QObject::connect(mainWin, SIGNAL(UpdateGpsFile(QString)), this, SLOT(OnUpdateGpsFile(QString)) );
    QObject::connect(mainWin, SIGNAL(EnableTTS(bool)), this, SLOT(OnEnableTTS(bool)) );

    QObject::connect(this, SIGNAL(ShowWidget(int)),                         mainWin, SLOT(OnShowWidget(int)) );
    QObject::connect(this, SIGNAL(ShowMessageBox(QString, MessageType)),    mainWin, SLOT(OnShowMessageBox(QString, MessageType)) );
    QObject::connect(this, SIGNAL(TripRemainingTime(quint32)),              mainWin, SIGNAL(TripRemainingTime(quint32)) );
    QObject::connect(this, SIGNAL(TripRemainingDistance(qreal)),            mainWin, SIGNAL(TripRemainingDistance(qreal)) );
    QObject::connect(this, SIGNAL(UpdateManeuverList(const ManeuverList&)), mainWin, SIGNAL(UpdateManeuverList(const ManeuverList&)) );
    QObject::connect(this, SIGNAL(RouteReceived()),                         mainWin, SIGNAL(RouteReceived()) );
    QObject::connect(this, SIGNAL(ManeuverRemainingDistance(qreal)),        mainWin, SIGNAL(ManeuverRemainingDistance(qreal)) );

    qRegisterMetaType<NavigateRouteError>("NavigateRouteError");
}
NavigationSession::~NavigationSession()
{
    if( mLocationConfiguration.locationFilename.length() > 0 )
    {
        LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
        locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    }
}

void NavigationSession::LocationUpdated( const Location& location )
{
    qDebug()<<"location lat, lon: "<<location.latitude<<", "<<location.longitude;
    if (mNavigationImpl)
        mNavigationImpl->UpdatePosition(location);
}

void NavigationSession::ProviderStateChanged( LocationProviderState state )
{

}

void NavigationSession::OnLocationError(int errorCode)
{

}

void NavigationSession::OnStartNavigation(LTKContext& ltkcontext, const Place& place, const RouteOptions& routeoption, const Preferences& preference)
{
    mTTSPlayer = TTSPlayer::GetTTSPlayer(ltkcontext, "");
    if(mNavigationStatus == 0)
    {
        mNavigationImpl = Navigation::CreateNavigation(ltkcontext, place, routeoption, preference);

        if(mNavigationImpl)
        {
            mNavigationStatus = 1;
            mLocationConfiguration.emulationMode = QBool(true);
            mLocationConfiguration.locationFilename = mCurrentGPSFile;
            LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
            locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );

            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(OffRoute()),                                                             this, SLOT(OnOffRoute()) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(OnRoute()),                                                              this, SLOT(OnRoute()) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteReceived( RouteRequestReason, const QVector< QSharedPointer<RouteInformation> >&)), this, SLOT(OnRouteReceived( RouteRequestReason, const QVector< QSharedPointer<RouteInformation> >&)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteRequested(RouteRequestReason)),                                     this, SLOT(OnRouteRequested(RouteRequestReason)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteProgress(qint32)),                                                  this, SLOT(OnRouteProgress(qint32)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteError(NavigateRouteError)),
                              this, SLOT(OnRouteError(NavigateRouteError)),
                              Qt::QueuedConnection );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(ArrivingToDestination(DestinationStreetSide )),                          this, SLOT(OnArrivingToDestination(DestinationStreetSide)) );
            QObject::connect( &mNavigationImpl->GetSessionSignals(), SIGNAL(RouteFinish()),                                                          this, SLOT(OnRouteFinish()) );

            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(TripRemainingTime(quint32)),              this, SLOT(OnTripRemainingTime(quint32)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(TripRemainingDistance(const qreal&)),     this, SLOT(OnTripRemainingDistance(const qreal&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(UpdateManeuverList(const ManeuverList&)), this, SLOT(OnUpdateManeuverList(const ManeuverList&)) );
            QObject::connect( &mNavigationImpl->GetNavigationUpdateSignals(), SIGNAL(ManeuverRemainingDistance(const qreal&)), this, SLOT(OnManeuverRemainingDistance(const qreal&)) );

            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(TrafficAlerted(const TrafficEvent&)), this, SLOT(OnTrafficAlerted(const TrafficEvent&)) );
            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(TrafficChanged(const TrafficInformation&)), this, SLOT(OnTrafficChanged(const TrafficInformation&)) );
            QObject::connect( &mNavigationImpl->GetTrafficSignals(), SIGNAL(DisableTrafficAlerted()), this, SLOT(OnDisableTrafficAlerted()) );

            QObject::connect( &mNavigationImpl->GetAnnouncementSignals(), SIGNAL(Announce(const Announcement&)), this, SLOT(OnAnnounce(const Announcement&)) );
        }
        else
        {
            emit ShowMessageBox("Create navigation failed!", MT_Normal);
        }
    }
}

void NavigationSession::OnStopNavigation()
{
    if(mTTSPlayer)
    {
        mTTSPlayer->Stop();
    }
    if(mNavigationImpl)
    {
        mNavigationImpl->StopSession();
        delete mNavigationImpl;
        mNavigationImpl = NULL;
    }
    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    mLocationConfiguration.locationFilename = "";
}

void NavigationSession::OnPlayAnounce(int index)
{
    mNavigationImpl->Announce((qint32)index);
}

void NavigationSession::OnUpdateGpsFile(QString gpsfile)
{
    mCurrentGPSFile = gpsfile;
}

void NavigationSession::OnEnableTTS(bool ttsStatus)
{
    mEnableTTS = ttsStatus;
}

/*************************************************** SessionSignals *************************************************************************/
/*! Emit when the locations being received by the navigation session are not considered on route to destination. */
void NavigationSession::OnOffRoute()
{

}

/*! Emit when the locations being received by the navigation session are considered on route to destination. */
void NavigationSession::OnRoute()
{

}

/*! Emit when received the list of possible routes for the current navigation session. */
void NavigationSession::OnRouteReceived(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes)
{
    if(!routes.isEmpty())
    {
        mRouteInformationList = routes;
        QSharedPointer<RouteInformation> routeInfo = routes.at(0);
        mNavigationImpl->SetActiveRoute(*routeInfo);
    }
    emit RouteReceived();
    emit ShowWidget(1);
}

/*! Emit when new route requested */
void NavigationSession::OnRouteRequested(RouteRequestReason reason)
{
    switch(reason)
    {
        case RRR_Calculate:
            emit ShowWidget(2);
            break;
        case RRR_Recalculate:
            emit ShowWidget(3);
            break;
        case RRR_RouteSelector:
            break;
        case RRR_Detour:
            emit ShowWidget(4);
            break;
        default:
            break;
    }
    mNavigationStatus = 0;
}

/*! Emit when the route generation in progress */
void NavigationSession::OnRouteProgress(qint32 progress)
{

}

void NavigationSession::OnRouteError(NavigateRouteError error)
{
    const char* errorDescription = NavigateRouteErrorDescription[error - NRE_None];
    QString errorText = "Root Error from the server:";
    errorText += errorDescription;
    errorText += " please check your settings!";
    QMessageBox msgBox( QMessageBox::Information, "Root Error", errorText);
    msgBox.exec();
    OnStopNavigation();
    emit ShowWidget(0);

}

/*! Emit when the current navigation arriving to destination. */
void NavigationSession::OnArrivingToDestination(DestinationStreetSide streetSide)
{

}

/*! Emit when the current navigation session ends. The client will not receive any route updates. */
void NavigationSession::OnRouteFinish()
{
    LocationProvider& locProvider = LocationProvider::GetInstance( mLocationConfiguration );
    locProvider.StopReceivingFixes( static_cast<LocationListener&>(*this) );
    emit ShowMessageBox("Arrived destination!", MT_Normal);
}

/*! Emit when the heading to origin off route. */
void NavigationSession::OnOffRoutePositionUpdate(qreal headingToRoute)
{

}


/*************************************************** NavigationUpdateSignals *************************************************************************/
/*! Emit when trip remaining time updated */
void NavigationSession::OnTripRemainingTime(quint32 time)
{
    emit TripRemainingTime(time);
}

/*! Emit when trip remaining delay updated */
void NavigationSession::OnTripRemainingDelay(quint32 time)
{

}

/*! Emit when trip remaining distance updated */
void NavigationSession::OnTripRemainingDistance(const qreal& distance)
{
    emit TripRemainingDistance(distance);
}

/*! Emit when the name of current road updated. */
void NavigationSession::OnCurrentRoadName(const QString& primaryName, const QString& secondaryName)
{

}

/*! Emit when the exit number of current maneuver updated. */
void NavigationSession::OnManeuverExitNumber(const QString& exitNumber)
{

}

/*! Emit when the name of next road updated. */
void NavigationSession::OnNextRoadName(const QString& primaryName, const QString& secondaryName)
{

}

/*! Emit when the type of current updated. */
void NavigationSession::OnManeuverType(const QString& type)
{

}

/*! Emit when remaining time of maneuver updated. */
void NavigationSession::OnManeuverRemainingTime(quint32 time)
{

}

/*! Emit when the remaining time delay of current maneuver updated. */
void NavigationSession::OnManeuverRemainingDelay(quint32 delay)
{

}

/*! Emit when the remaining distance of the current maneuver updated. */
void NavigationSession::OnManeuverRemainingDistance(const qreal& distance)
{
    emit ManeuverRemainingDistance(distance);
}

/*! Emit when maneuver image ID returned. Use MapUtils for getting bitmap. */
void NavigationSession::OnManeuverImageId(const QString& imageId)
{

}

/*! Emit when the stack maneuver of turn image text returned. */
void NavigationSession::OnStackTurnImageTTF(const QString& stackImageTTF)
{

}

/*! Emit when position on route has been changed. */
void NavigationSession::OnPositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading)
{

}

/*! Emit when the turn point coordinate of maneuver changed. */
void NavigationSession::OnManeuverPoint(const Coordinates& point)
{

}

/*! Emit when updating upcoming maneuver list when maneuver changed. */
void NavigationSession::OnUpdateManeuverList(const ManeuverList& maneuvers)
{
    emit UpdateManeuverList(maneuvers);
}

/*! Emit when a traffic event is detected within a specified distance (default is 10km). */
void NavigationSession::OnTrafficAlerted(const TrafficEvent& trafficEvent)
{
    qDebug()<<"NavigationSession::OnTrafficAlerted(const TrafficEvent& trafficEvent)";
    qDebug()<<"----------------------------------------------------------";
    qDebug()<<"GetDelay(): "<<trafficEvent.GetDelay()<<" GetDescription(): "<<trafficEvent.GetDescription();
    qDebug()<<"GetDistanceToTrafficItem(): "<<trafficEvent.GetDistanceToTrafficItem()<<"GetEndTime(): "<<trafficEvent.GetEndTime();
    qDebug()<<"GetLength(): "<<trafficEvent.GetLength()<<"GetManeuverNumber(): "<<trafficEvent.GetManeuverNumber();
    qDebug()<<"GetReportTime(): "<<trafficEvent.GetReportTime()<<"GetRoadName(): "<<trafficEvent.GetRoadName();
    qDebug()<<"GetSeverity(): "<<trafficEvent.GetSeverity()<<"GetStartFromTrip(): "<<trafficEvent.GetStartFromTrip();
    qDebug()<<"GetType(): "<<trafficEvent.GetType();
    qDebug()<<"----------------------------------------------------------";
}

/*! Emit when traffic information is updated */
void NavigationSession::OnTrafficChanged(const TrafficInformation& trafficInfo)
{
    qDebug()<<"NavigationSession::OnTrafficChanged(const TrafficInformation& trafficInfo)";
    for(int i = 0; i <trafficInfo.GetTrafficEvents().size(); ++i)
    {
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
void NavigationSession::OnDisableTrafficAlerted()
{
    qDebug()<<"NavigationSession::OnDisableTrafficAlerted() was called.";
}



/*************************************************** AnnouncementSignals *************************************************************************/
/*! provides the announcement related information output stream to get audio data to be played audio text for TTS support */
void NavigationSession::OnAnnounce(const Announcement& announcement)
{
    if(mEnableTTS && mTTSPlayer)
    {
        mTTSPlayer->Play(announcement);
    }
}

#if 0
/*************************************************** PositionUpdateSignals *************************************************************************/
/*! Emit when position updates on route. */
void NavigationSession::PositionUpdate(const PositionInfo& positionInfo)
{

}

/*************************************************** ManeuverUpdateSignals *************************************************************************/
/*! Emit when maneuvers on route updates. */
void NavigationSession::ManeuverUpdate(const Maneuver& maneuver)
{

}

/*************************************************** NavEventSignals *************************************************************************/
/*! Emit when navigation events updates. */
void NavigationSession::NavEvent(NavEventTypeMask navEventTypeMask, const LaneInformation& laneInfo,
                        const RoadSign& signInfo, const SpeedLimitInformation& speedLimitInfo,
                        const TrafficEvent& trafficEvent, const TrafficInformation& trafficInfo)
{

}

#endif
