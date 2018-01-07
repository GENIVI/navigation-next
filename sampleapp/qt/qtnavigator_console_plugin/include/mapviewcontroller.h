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

#ifndef MAPVIEWCONTROLLER_H
#define MAPVIEWCONTROLLER_H

#include <QQuickItem>
#include "locationprovider.h"
#include "location.h"
#include "interpolator.h"
#include <QTime>
#include <QTimer>
#include <QVector>
#include <QtDBus/QtDBus>



// Navigation
#include "interpolator.h"
#include "locationlistener.h"
#include "locationconfiguration.h"
#include "locationprovider.h"
#include "dataobject.h"
#include "coordinate.h"
#include "pin.h"
#include "place.h"
#include "routeoptions.h"
#include "preferences.h"
#include "pinparameters.h"
#include "cameraparameters.h"
#include "avatar.h"
#include "mapwidget.h"
#include "navigationroutecontroller.h"
#include "navigationtypes.h"
#include "latlngbound.h"
#include "camerafactory.h"
#include "navigationsession.h"
#include "maneuverdetailcontroller.h"
#include "maneuverslistcontroller.h"


#include<QQmlEngine>
#include<QQmlContext>
namespace locationtoolkit {
class MapWidget;
class Location;
class Coordinates;
class NavigationSession;
class ManeuverList;
class RouteInformation;
class Polyline;
class Maneuver;
}

using namespace locationtoolkit;


class MapViewController : public QQuickItem//, public locationtoolkit::LocationListener
{
    Q_OBJECT
    Q_DISABLE_COPY(MapViewController)

    Q_PROPERTY(QString workFolder READ workFolder WRITE setWorkFolder)
    Q_PROPERTY(QPointF avatarPosition READ avatarPosition WRITE setAvatarPosition)
    Q_PROPERTY(float avatarHeading READ avatarHeading WRITE setAvatarHeading)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom)
    Q_PROPERTY(bool isFollowMe READ isFollowMe WRITE setIsFollowMe)

public:
    MapViewController(QQuickItem *parent = 0);
    ~MapViewController();

    void setWorkFolder(QString path);
    QString workFolder();

    QPointF avatarPosition() const;
    void setAvatarPosition(QPointF p);

    float avatarHeading() const;
    void setAvatarHeading(float t);

    float tilt();
    void setTilt(float t);

    float zoom();
    void setZoom(float t);

    bool isFollowMe();
    void setIsFollowMe(bool v);
    void setPOIPosition(QPointF,QString);
    QPointF mNavDestPoint;
    QString mNaveDestName;
    QPointF mUserDestPoint;
    QString mUserDestName;
    bool mInNavMode = false;
    bool mlowFuelMode = false;

    /*! interface defined by LocationListener */
    //       virtual void LocationUpdated( const locationtoolkit::Location& location );
    //       virtual void ProviderStateChanged( LocationProviderState state );
    //       virtual void OnLocationError(int errorCode);

    Coordinates getMapCenter() { return mMapCenter; }

signals:

    void refreshManeuver(const QString& maneuver);
    void showFollowMeOption();
    void fuelSearchRequest(QVariant);

public slots:
    void init();
    void onMapReady();
    bool setManeuver(const QString& maneuverIcon, const QString& streetName, const QString& distance, const QString& trafficWarning);
    bool setPosition(const QString& lat, const QString& lon, const QString& heading);
    void setNavigationMode(bool navMode);
    bool setRemainingManeuverDistance(const QString& distance);
    void onTimer();
    bool setPolyline(const QString &polyline);
    bool clearPolyline();
    bool setManeuverArrow(const QString &pts);
    bool updateSpeedLimit(const QString &speed);

    void dbusCallFinishedSlot(QDBusPendingCallWatcher* watcher);

    //void startNavigation();
    void stopNavigation();
    void maneuverListUpdated(const ManeuverList&);
    void updatePosition(const locationtoolkit::Location&);
    void routeFinish();
    void routeReceived(const QVector< QSharedPointer<locationtoolkit::RouteInformation> >& routes);
    void ManeuverRemainingDistanceUpdated(qreal rDistance);

    void onUnlock();
    void onlock();
    
    void onSideItemVisibility(bool);
    Q_SCRIPTABLE QString showFuelAlert(bool Mode);
    void fuelAlertAcceptClicked();
protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void updateAvatarLocation(const locationtoolkit::Location& location);
    void addRoute();
    void addPin(const locationtoolkit::Coordinates &coordinate);
    void addManeuverArrow(const QVector<locationtoolkit::Coordinates>& pts);
    void clearManeuvers();

private:
    bool mInitDone;
    bool mMapReady;
public:
    locationtoolkit::MapWidget* globalMapWidget;
private:
    locationtoolkit::LocationConfiguration mLocationConfiguration;
    QPointF mAvatarPosition;
    float mAvatarHeading;
    float mTilt;
    float mZoom;
    bool mIsFollowMe;
    QString mClient;
    QTime mInterpolatorTime;
    QTimer mInterpolatorTimer;
    locationtoolkit::Location mLastLocation;
    Interpolator mLatInterpolator;
    Interpolator mLonInterpolator;
    Interpolator mHeadingInterpolator;
    static const int mInterpolateTimerGap;
    bool mAvatarLocated;
    QList<locationtoolkit::Coordinates> mRoutePolyPoints;
    locationtoolkit::Polyline* mLastPolyline;
    QList<locationtoolkit::Polyline*> mManeuverArrows;

    //QDBusInterface iface;

    locationtoolkit::NavigationSession* mNavSession;
    //added for navigation
private:
    QVariant mSearchedPinInfo;
    QString mGPSFile;
    MapWidget* mLTKMapKit;
    LTKContext* mLTKContext;
    bool mPolylineReady;
    navigationRouteController *mNavigationRoutesController;
    QQuickItem* mNavHeader;
    QQuickItem*  mToggleHeaderBar;
    bool mStartNavGoClicked;
    bool mIsNavigationStarted;
    bool mMapCameraLocked;
    bool autoNavMode;
    QString mSettingFileName;
    Coordinates mMapCenter;
    Place mReferenceCenter;

    QQuickItem* mZoomInButton;
    QQuickItem* mZoomOutButton;
    QQuickItem* mLayerOptionsButton;
    QQuickItem* mLayerOptionWidget;
    QQuickItem* mWeather;
    QQuickItem* mAlertBox;

    QQuickItem* mMapCameraLockButton;
    QQuickItem* mRouteOverviewButton;
    QQuickItem* mSpeedLimitView;
    QQuickItem* mTrafficViewController;
    QQuickItem* mRoadSignViewController;
    ManeuverDetailController* mManeuverDetailsController;
    ManeuversListController *mManeuversListController;
    QList<locationtoolkit::Polyline*> activePolylines;
    qint32                mLastHeading;
    QJsonArray            searchedLocations;
    QString mDestName;
    QList<QObject *> mManeuverDataList;
    //QList<Polyline *> mArrowPolyLines;
    void startNavigationWithNavKit();
    void InitializeNavigationHeaderBar();
    void SetUpNavigationRouteController();
    void prepareNavigationEnd();
    void SetupMapWidget();
    void setupManeuverDetailSignalAndSlots();
    void navigationButtons();
    void setupManeuverListSignalAndSlots();
    void saveManeuverList(const ManeuverList& maneuverlist);
    void setMapCameraLockButtonPosition(bool);
    void setFuelAlertVisibility(/*bool*/);
public:
    void setUp_sideBarItems();



Q_SIGNALS:
    void SignalNavigationButtonClicked(QString, int, QVariant);
    void StartNavigation(LTKContext&, const Place&, const RouteOptions&, const Preferences&);
    void UpdateGpsFile(QString gpsfile);
    void StopNavigation();
    void ToggleTitleBarVisibility(bool);
    void startNavigationOnRoute(int);
    //for maneuver
    void TripRemainingTime(quint32 time);
    void TripRemainingDistance(qreal);
    void CurrentRoadName(const QString& primaryName, const QString& secondaryName);
    void NextRoadName(const QString& primaryName, const QString& secondaryName);
    void StackTurnImageTTF(const QString& stackImageTTF);
    void ManeuverRemainingDistance(qreal distance);
    void UpdateManeuverList(const ManeuverList& maneuverlist);
    void ManeuverImageId(const QString&);
    void TripTrafficColor(char );
    void locationUpdationDone();
    void MapClicked();
    void SignalPinInfoClicked(const locationtoolkit::Pin* pin);
    void resetMapCameraButton();
    void HideSideMenu();

private slots:
    void OnStartNavigationToDestination(QString, int, QVariant);
    void OnNavigationEnd();
    void abortRouteNavigation();
    void onStartNavigationOnRoute(int);
    void onAddPolyLinesForPath(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes);
    void OnNavigationStarted();
    void onLocationUpdated( const Location& location );
    //void onAddManeuverArrow(const Maneuver* maneuver);
    void onPolylineClicked(const QList<locationtoolkit::Polyline*> polylines);
    void onCameraLockButtonClicked();
    void onNavigationMapOverview();
    void SetAvatarLocationAndCameraPosition(locationtoolkit::Coordinates coord);
    void onPositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading);
    void onOffRoutePositionUpdate(qreal headingToRoute);
    void onRouteError(LTKError error);
    void onRouteFinish();
    void OnSpeedLimit(const SpeedLimitInformation& speedInfo);
    void OnDisableSpeedLimit();
    void OnTrafficAlerted(const TrafficEvent& trafficEvent);
    void OnTrafficChanged(const TrafficInformation& trafficInfo);
    void OnDisableTrafficAlerted();
    void OnRoadSign(const locationtoolkit::RoadSign& roadSign);
    void OnLaneInformation(const locationtoolkit::LaneInformation& laneInfo);
    void onManeuverListClicked();
    void onSetClusterUINavMode(int mode);
    void onConfirmBoxNotification(bool);
    void onLowFuelConfirmBoxNotification(bool);
    void onNavStopButtonClicked();

    void OnMapCreated();
    void OnLayersCreated();
    void OnLayerOptionButtonClicked();
    void GetPinInfo(const locationtoolkit::Pin* pin);
    void onMapClicked();
    void onCameraUpdated(const locationtoolkit::CameraParameters& cameraParameters);
    //void onPolylineClicked(int);
    void mapZoomIn();
    void mapZoomOut();
    void mapLayerOptions();
    void onToggleWeather(bool b);
    void onToggleMap3DView(bool b);
    void onLandmarkBuildingsClicked(bool isChecked);
    void onPoiClicked(bool isChecked);
    void onSatelliteClicked(bool isChecked);
    void onTrafficClicked(bool isChecked);
    void onLayerOptionWidgetClosed();
    void onChangedCentreCoordinate(QString);
    void onStaticPOIClicked(const QString& , const QString& , double , double );
    void onDestroyManeuverList();
    void onAlertBoxValueClicked(bool);
public slots:
    void onUpdateCameraParam();

    //void onGetUpdatedLocation(Location&);

};

#endif // MAPVIEWCONTROLLER_H
