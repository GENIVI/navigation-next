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
    @file     navigationsession.h
    @date     09/25/2014
    @defgroup navkitsample
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef __NAVIGATION_IMPL_H__
#define __NAVIGATION_IMPL_H__
#include <QObject>
#include "mainwindow.h"
#include "navigation.h"
#include "navigationtypes.h"
#include "locationprovider.h"
#include "interpolator.h"
#include "routeinformation.h"
#include "ttsplayer.h"

/*class RouteInformation;
class DestinationStreetSide;
class TrafficEvent;
class TrafficInformation;
class Coordinates;
class ManeuverList;
class Announcement;
class PositionInfo;
class Maneuver;
class NavEventTypeMask;
class LaneInformation;
class RoadSign;
class SpeedLimitInformation;*/

using namespace locationtoolkit;

class NavigationSession: public QObject, public locationtoolkit::LocationListener
{
    Q_OBJECT
public:
    NavigationSession(QWidget *parent = 0);
    ~NavigationSession();

    /*! interface defined by LocationListener */
    virtual void LocationUpdated( const Location& location );
    virtual void ProviderStateChanged( LocationProviderState state );
    virtual void OnLocationError(int errorCode);

Q_SIGNALS:
    void ShowWidget(int widgetNum);
    void RouteReceived();
    void RouteProgress();
    void RouteError();
    void TripRemainingTime(quint32 time);
    void TripRemainingDistance(qreal distance);
    void ManeuverRemainingDistance(qreal distance);
    void UpdateManeuverList(const ManeuverList& maneuverlist);
    void ShowMessageBox(QString message, MessageType type);

public slots:
    void OnStartNavigation(LTKContext& ltkcontext, const Place& place, const RouteOptions& routeoption, const Preferences& preference);
    void OnStopNavigation();
    void OnPlayAnounce(int index);
    void OnUpdateGpsFile(QString);
    void OnEnableTTS(bool);

public slots:
    /*! Emit when the locations being received by the navigation session are not considered on route to destination. */
    void OnOffRoute();

    /*! Emit when the locations being received by the navigation session are considered on route to destination. */
    void OnRoute();

    /*! Emit when received the list of possible routes for the current navigation session. */
    void OnRouteReceived(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes);

    /*! Emit when new route requested */
    void OnRouteRequested(RouteRequestReason reason);

    /*! Emit when the route generation in progress */
    void OnRouteProgress(qint32 progress);

    void OnRouteError(NavigateRouteError error);

    /*! Emit when the current navigation arriving to destination. */
    void OnArrivingToDestination(DestinationStreetSide streetSide);

    /*! Emit when the current navigation session ends. The client will not receive any route updates. */
    void OnRouteFinish();

    /*! Emit when the heading to origin off route. */
    void OnOffRoutePositionUpdate(qreal headingToRoute);


    /*! Emit when trip remaining time updated */
    void OnTripRemainingTime(quint32 time);

    /*! Emit when trip remaining delay updated */
    void OnTripRemainingDelay(quint32 time);

    /*! Emit when trip remaining distance updated */
    void OnTripRemainingDistance(const qreal& distance);

    /*! Emit when the name of current road updated. */
    void OnCurrentRoadName(const QString& primaryName, const QString& secondaryName);

    /*! Emit when the exit number of current maneuver updated. */
    void OnManeuverExitNumber(const QString& exitNumber);

    /*! Emit when the name of next road updated. */
    void OnNextRoadName(const QString& primaryName, const QString& secondaryName);

    /*! Emit when the type of current updated. */
    void OnManeuverType(const QString& type);

    /*! Emit when remaining time of maneuver updated. */
    void OnManeuverRemainingTime(quint32 time);

    /*! Emit when the remaining time delay of current maneuver updated. */
    void OnManeuverRemainingDelay(quint32 delay);

    /*! Emit when the remaining distance of the current maneuver updated. */
    void OnManeuverRemainingDistance(const qreal& distance);

    /*! Emit when maneuver image ID returned. Use MapUtils for getting bitmap. */
    void OnManeuverImageId(const QString& imageId);

    /*! Emit when the stack maneuver of turn image text returned. */
    void OnStackTurnImageTTF(const QString& stackImageTTF);

    /*! Emit when position on route has been changed. */
    void OnPositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading);

    /*! Emit when the turn point coordinate of maneuver changed. */
    void OnManeuverPoint(const Coordinates& point);

    /*! Emit when updating upcoming maneuver list when maneuver changed. */
    void OnUpdateManeuverList(const ManeuverList& maneuvers);

    /*! Emit when a traffic event is detected within a specified distance (default is 10km). */
    void OnTrafficAlerted(const TrafficEvent& trafficEvent);

    /*! Emit when traffic information is updated */
    void OnTrafficChanged(const TrafficInformation& trafficInfo);

    /*! Emit when disable the traffic alert. */
    void OnDisableTrafficAlerted();

    /*************************************************** AnnouncementSignals *************************************************************************/
    /*! provides the announcement related information output stream to get audio data to be played audio text for TTS support */
    void OnAnnounce(const Announcement& announcement);

#if 0

    /*************************************************** PositionUpdateSignals *************************************************************************/
    /*! Emit when position updates on route. */
    void PositionUpdate(const PositionInfo& positionInfo);

    /*************************************************** ManeuverUpdateSignals *************************************************************************/
    /*! Emit when maneuvers on route updates. */
    void ManeuverUpdate(const Maneuver& maneuver);

    /*************************************************** NavEventSignals *************************************************************************/
    /*! Emit when navigation events updates. */
    void NavEvent(NavEventTypeMask navEventTypeMask, const LaneInformation& laneInfo,
                            const RoadSign& signInfo, const SpeedLimitInformation& speedLimitInfo,
                            const TrafficEvent& trafficEvent, const TrafficInformation& trafficInfo);

#endif

private:
    int         mNavigationStatus;
    bool        mEnableTTS;
    Navigation* mNavigationImpl;
    LTKContext* mLTKContext;
    LocationConfiguration mLocationConfiguration;
    QString mCurrentGPSFile;
    TTSPlayer* mTTSPlayer;
    QVector< QSharedPointer<RouteInformation> > mRouteInformationList;
};

#endif // __NAVIGATION_IMPL_H__
