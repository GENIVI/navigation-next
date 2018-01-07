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

#include "mapviewcontroller.h"
#include "mapwidget.h"
#include "util.h"
#include "globalsetting.h"
#include <QtDBus/QtDBus>
#include "gpsfile.h"
#include "mercator.h"
#include "custombubble.h"
#include "confirmboxcontroller.h"
#include "messageboxcontroller.h"
#include "GPSFile.h"
#include "dataobject.h"
#include "maneuverdataobject.h"
#include <QQmlProperty>

#define SERVICE_CONSOLE_CLUSTER "com.locationstudio.qtnavigator.consoletocluster"
#define SERVICE_CLUSTER_CONSOLE "com.locationstudio.qtnavigator.clustertoconsole"
#define GPS_FILE "/opt/comtech/gps/demo.gps"
#define INTERPOLATER_TIME_GAP 20

const int MapViewController::mInterpolateTimerGap(20);

MapViewController::MapViewController(QQuickItem *parent):
    QQuickItem(parent),
    mInitDone(false),
    mMapReady(false),
    mAvatarPosition(-117.689f,33.604f),
    mMapCenter(-117.689f, 33.604f),
    mAvatarHeading(0.0f),
    mTilt(parent->property("tilt").toFloat()),
    mZoom(parent->property("zoom").toFloat()),
    mIsFollowMe(parent->property("isFollowMe").toBool()),
    mClient(parent->property("client").toString()),
    mAvatarLocated(false),
    autoNavMode(true),
    mLastPolyline(NULL),
    mManeuversListController(NULL)
    //iface(SERVICE_CONSOLE_CLUSTER "/", "", QDBusConnection::sessionBus())
{
    qDebug()<<"MapViewController::MapViewController "<<mIsFollowMe;
    setFlag(ItemHasContents, true);

    mNavSession = new locationtoolkit::NavigationSession(this);
    //    mNavSession = new locationtoolkit::NavigationSession(this);
    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");

    QString GpsFileName = settings.value(gGpsFileName,"startup.gps").toString();
    QString gpsPath = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + GpsFileName;

    mNavSession->OnUpdateGpsFile(gpsPath);

    Place destination, origin;

    //Coordinates mapCenter = getMapCenter();
    GetDefaultDestinationAndOrigin(gpsPath.toStdString(), &destination, &origin);
    mReferenceCenter = origin;
    mLastLocation.latitude = origin.GetLocation().center.latitude;
    mLastLocation.longitude = origin.GetLocation().center.longitude;
    settings.endGroup();

    connect( &mInterpolatorTimer, SIGNAL(timeout()), this, SLOT(onTimer()) );
}

//added for navigation
/*#########################################################################################

    Get the Maneuver Details Controller in order to setup further signal and slots

 *#########################################################################################*/
void MapViewController::setupManeuverDetailSignalAndSlots() {

    //  Create Maneuver Details View and UI

    mManeuverDetailsController = new ManeuverDetailController();
    QQmlEngine::setObjectOwnership(mManeuverDetailsController, QQmlEngine::CppOwnership);
    mManeuverDetailsController->setParentItem(this->parentItem());

    QQmlEngine::setContextForObject(mManeuverDetailsController, QQmlEngine::contextForObject(this));
    int maneuverWidth = width()*0.3;
    int maneuverHeight = height() - height()/5 - (height()/5 - height()/8.5);
    mManeuverDetailsController->setPosition(QPointF(40, 30 + height()/6));
    //  mManeuverDetailsController->setPosition(QPointF(20, height()-maneuverHeight - 50));
    mManeuverDetailsController->setWidth(maneuverWidth);
    mManeuverDetailsController->setHeight(maneuverHeight);

    qDebug()<<"mManeuverDetailsController::"<<mManeuverDetailsController;
    mManeuverDetailsController->setUp();
    mManeuverDetailsController->hide();

    //  add navigation buttons
    //navigationButtons();

    //  Map Signal-Slots for Maneuver Details Controller
    QObject::connect(this,SIGNAL(TripRemainingTime(quint32)),               mManeuverDetailsController, SLOT(OnTripRemainingTime(quint32)));
    QObject::connect(this,SIGNAL(TripRemainingDistance(qreal)),             mManeuverDetailsController, SLOT(OnTripRemainingDistance(qreal)));
    QObject::connect(this,SIGNAL(CurrentRoadName(const QString&,const QString&)),         mManeuverDetailsController, SLOT(OnCurrentRoadName(const QString&,const QString&)));
    QObject::connect(this,SIGNAL(NextRoadName(const QString&,const QString&)),            mManeuverDetailsController, SLOT(OnNextRoadName(const QString&,const QString&)));
    QObject::connect(this,SIGNAL(StackTurnImageTTF(const QString&)),               mManeuverDetailsController, SLOT(OnStackTurnImageTTF(const QString&)));
    QObject::connect(this,SIGNAL(ManeuverImageId(const QString&)),               mManeuverDetailsController, SLOT(OnManeuverImageId(const QString&)));
    QObject::connect(this, SIGNAL(TripTrafficColor(char )),                                 mManeuverDetailsController, SLOT(OnTripTrafficColor(char)));
    QObject::connect(this,SIGNAL(ManeuverRemainingDistance(qreal)),         mManeuverDetailsController, SLOT(OnManeuverRemainingDistance(qreal)) );
    //QObject::connect(this,SIGNAL(UpdateManeuverList(const ManeuverList&)),  mManeuverDetailsController, SLOT(OnUpdateManeuverList(const ManeuverList&)));
    //QObject::connect(mManeuverDetailsController,SIGNAL(AddManeuverArrow(const Maneuver*)) ,this, SLOT(onAddManeuverArrow(const Maneuver*)) );

    //    mManeuverDetailsController->hide();

    //    mManeuverList = new ManeuversList(this);
    //    mManeuverList->setGeometry(0,0,this->width(), this->height());
    //    QObject::connect(this,SIGNAL(TripRemainingTime(quint32)),               mManeuverList, SLOT(OnTripRemainingTime(quint32)));
    //    QObject::connect(this,SIGNAL(TripRemainingDistance(qreal)),             mManeuverList, SLOT(OnTripRemainingDistance(qreal)));
    //    QObject::connect(this,SIGNAL(ManeuverRemainingDistance(qreal)),         mManeuverList, SLOT(OnManeuverRemainingDistance(qreal)) );
    //    QObject::connect(this,SIGNAL(UpdateManeuverList(const ManeuverList&)),  mManeuverList, SLOT(OnUpdateManeuverList(const ManeuverList&)));
    //    QObject::connect(mManeuverList,SIGNAL(backButtonClicked()),                      this, SLOT(showPinInfoPage()));
    //    QObject::connect(mManeuverList,SIGNAL(closeButtonClicked()),                     this, SLOT(abortRouteNavigation()));

    //    mManeuverList->hide();

    // navigation header-bar

    //    QQmlComponent component(engine(), QUrl("qrc:/navigationGoScreenHeader.qml"));
    //    mNavHeader = qobject_cast<QQuickItem*>(component.create());
    //    connect( mNavHeader, SIGNAL(navBackButtonClicked()), this, SLOT(abortRouteNavigation()) );
    //    QQmlEngine::setObjectOwnership(mNavHeader, QQmlEngine::CppOwnership);
    //    mNavHeader->setParentItem(rootObject());
    //    mNavHeader->setParent(this);
    //    mNavHeader->setPosition(QPoint(0, 0));
    //    mNavHeader->setSize(QSize(this->width(), 80));
    //    mNavHeader->setVisible(false);

}

void MapViewController::setupManeuverListSignalAndSlots(){

    mManeuversListController = new ManeuversListController();
    QQmlEngine::setObjectOwnership(mManeuversListController, QQmlEngine::CppOwnership);
    mManeuversListController->setParentItem(this->parentItem());

    QQmlEngine::setContextForObject(mManeuversListController, QQmlEngine::contextForObject(this));
    mManeuversListController->setWidth(width());
    mManeuversListController->setHeight(height());
    qDebug()<<"mManeuversListController::"<<mManeuversListController;
    mManeuversListController->setUp();
    mManeuversListController->setDestinationName(mDestName);
    mManeuversListController->setManeuverList(mManeuverDataList);
    mManeuversListController->show();

    QObject::connect(this,SIGNAL(TripRemainingTime(quint32)),mManeuversListController, SLOT(OnTripRemainingTime(quint32)));
    QObject::connect(this,SIGNAL(TripRemainingDistance(qreal)),mManeuversListController, SLOT(OnTripRemainingDistance(qreal)));
    //QObject::connect(this,SIGNAL(UpdateManeuverList(const ManeuverList&)),  mManeuversListController, SLOT(OnUpdateManeuverList(const ManeuverList&)));
    QObject::connect(mManeuversListController,SIGNAL(destroyManeuverList()),  this, SLOT(onDestroyManeuverList()));

}
void MapViewController::onDestroyManeuverList()
{

    if(mManeuversListController != NULL)
    {
        mManeuversListController->hide();
        delete mManeuversListController;
        mManeuversListController = NULL;
    }
}

//void MapViewController::onAddManeuverArrow(const Maneuver* maneuver) {

//    //  Remove all previously added maneuverPolyLines
////    for(int i = 0 ; i < mArrowPolyLines.size(); ++i) {
////        mArrowPolyLines.removeAt(i);
////    }
////    //  clear objects from QList
////    mArrowPolyLines.clear();
//    clearManeuvers();

//    if (mStartNavGoClicked && !maneuver->IsDestination()) {
//        qDebug()<<"MapViewController::onAddManeuverArrow";
//        QVector<Coordinates*> points = maneuver->GetPolyline();

//        CapParameter mStartCap;
//        mStartCap.type = locationtoolkit::CPT_Round;
//        mStartCap.radius = 6;
//        mStartCap.width = 0;
//        mStartCap.length = 0;

//        CapParameter mEndCap;
//        mEndCap.type = locationtoolkit::CPT_Arrow;
//        mEndCap.radius = 6;
//        mEndCap.width = 24;
//        mEndCap.length = 24;

//        QList<Coordinates> polyPoints;
//        QList<SegmentAttribute*> segAttrs;
//        for( int i = 0; i < points.size(); i++ )
//        {
//            polyPoints.append( *points[i] );
//            ColorSegment* segattr = new ColorSegment(points.size()-1,
//                                                     QColor(Qt::black));
//            segAttrs.append( segattr );
//        }

//        PolylineParameters para;
//        para.SetPoints(polyPoints);
//        para.SetSegmentAttributes(segAttrs);
//        para.SetUnhighlightColor(QColor(Qt::black));
//        para.SetWidth(12);
//        para.SetStartCap(mStartCap);
//        para.SetEndCap(mEndCap);
//        para.SetAttributeType(PolylineParameters::SAT_ColorSegment);
//        for(int i = 0; i < segAttrs.size(); ++i)
//        {
//            if(segAttrs[i])
//            {
//                delete segAttrs[i];
//                segAttrs[i] = NULL;
//            }
//        }

//        segAttrs.clear();
//    }

//}
//for navigation
void MapViewController :: SetupMapWidget()
{
    qDebug()<<"mLTKMapKit..."<<mLTKMapKit;

    //    mLTKMapKit->setParentItem(this);
    //    mLTKMapKit->setSize(QSize(width(),height()-100));


    //  Get Map Layer
    //    MainItem *mapItemObject = this->rootObject()->findChild<MainItem *>("mapObject");
    //    mLTKMapKit = mapItemObject->mGlobalMapWidget;

    //  Map Signal-Slots for Side Menu Controller
    if(mLTKMapKit) {
        connect( mLTKMapKit, SIGNAL(MapCreated()),                                              this, SLOT(OnMapCreated()) );
        connect( mLTKMapKit, SIGNAL(LayersCreated()),                                           this, SLOT(OnLayersCreated()) );
        connect( mLTKMapKit, SIGNAL(LayerOptionButtonClicked()),                                this, SLOT(OnLayerOptionButtonClicked()) );
        connect( mLTKMapKit, SIGNAL(PinClicked(const locationtoolkit::Pin*)),                   this, SLOT(GetPinInfo(const locationtoolkit::Pin*)) );
        connect( mLTKMapKit, SIGNAL(StaticPOIClicked(const QString& , const QString& , double , double )),this, SLOT(onStaticPOIClicked(const QString& , const QString& , double , double)));
        connect( mLTKMapKit, SIGNAL(MapClicked(const locationtoolkit::Coordinates&)),           this, SLOT(onMapClicked()));
        connect( mLTKMapKit, SIGNAL(CameraUpdated(const locationtoolkit::CameraParameters&)),   this, SLOT(onCameraUpdated(const locationtoolkit::CameraParameters&)));
        connect( mLTKMapKit, SIGNAL(PolylineClicked(const QList<locationtoolkit::Polyline*>)),  this, SLOT(onPolylineClicked(const QList<locationtoolkit::Polyline*>)));
        connect( mLTKMapKit, SIGNAL(Unlocked()),                                                this, SLOT(onUnlock()));

    }
}
/*#########################################################################################

    Direct Slots pertaining to the MapWidget

 *#########################################################################################*/
void MapViewController::OnMapCreated() {

    mMapReady = true;
    qDebug() << "OnMapCreated Called";
    //  Map Decorations include zoom in/out buttons, locate me button, compass button
    //  and layer options button.
    globalMapWidget->EnableReferenceCenter(true);
    globalMapWidget->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));
    //globalMapWidget->SetReferenceCenter(mReferenceCenter);
    locationtoolkit::Coordinates referenceCenter;
    referenceCenter.latitude = mReferenceCenter.GetLocation().center.latitude;
    referenceCenter.longitude = mReferenceCenter.GetLocation().center.longitude;
    locationtoolkit::CameraParameters param(referenceCenter);
    param.SetPosition( referenceCenter );
    param.SetTiltAngle(mTilt);
    param.SetZoomLevel(mZoom);
    mLTKMapKit->MoveTo( param );
    mMapCenter = param.GetPosition();

    locationtoolkit::Location source;
    source.latitude = mReferenceCenter.GetLocation().center.latitude;
    source.longitude = mReferenceCenter.GetLocation().center.longitude;
    source.heading = 60.0;
    source.valid = 507;
    qDebug()<<"MapViewController::OnMapCreated isFollowMe"<<isFollowMe();

    if(isFollowMe())
    {
        mMapCameraLocked = true;
        mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME,false);
    }

    mLTKMapKit->GetAvatar().SetMode( locationtoolkit::Avatar::AM_ARROW );
    mLTKMapKit->GetAvatar().SetLocation(source);
    mLTKMapKit->GetAvatar().SetScale(2);


    MapDecoration* mapDecoration = mLTKMapKit->GetMapDecoration();
    
    mapDecoration->SetLocateMeButtonEnabled( false );
    mapDecoration->SetZoomButtonEnabled( true );
    mapDecoration->SetCompassEnabled(true);
    mapDecoration->SetCompassPosition(width()-50, 50);

    QDir dir = QFileInfo( LTKSampleUtil::GetWorkFolder() ).dir();
    QString strRoot = dir.absolutePath();
    QString dayPath = strRoot + "/resource/TEXTURE/COMPASS_DAY_TEX_128x128";
    QString nightPath = strRoot + "/resource/TEXTURE/COMPASS_NIGHT_TEX_128x128";
    mapDecoration->SetCompassPath(dayPath, nightPath);

    //  TODO: Temporary for Navigation selection. Needs to be updated with progress!
    //    showPins();
    mLTKMapKit->SetNightMode(GlobalSetting::GetInstance().nightMode());
}

void MapViewController::OnLayersCreated() {

    qDebug() << "OnLayersCreated Called";

    // temporary enable satellite mode as soon as layers created.
    //    mLTKMapKit->ShowOptionalLayer("Satellite",true);
    //    mLayerOptionsButton->show();

    //  create layer option widget
    //    mLayerOptionWidget = new LayerOptionWidget( this, mLTKMapKit );
    //    connect( mLayerOptionWidget, SIGNAL(toggleMap3DView(bool)), this, SLOT(onToggleMap3DView(bool)) );
    //    mLayerOptionWidget->hide();

    // enable layer option button
    MapDecoration* mapdecoration = mLTKMapKit->GetMapDecoration();
    mapdecoration->SetLayerOptionButtonEnabled( true );

    mZoomInButton->setVisible(true);
    mZoomOutButton->setVisible(true);
    mLayerOptionsButton->setVisible(true);

}
void MapViewController::OnLayerOptionButtonClicked() {

    //  Close all side menus - Side Menu controller and near me controller
    //closeSideMenus();

    //  Position the layer option widget as needed
    //    if (mLayerOptionWidget != NULL) {
    // //        mLayerOptionWidget->move((width()-mLTKMapKit->width()) + (mLTKMapKit->width()-mLayerOptionWidget->width())/2, 20);
    //        mLayerOptionWidget->move((mLTKMapKit->width()-mLayerOptionWidget->width())/2, (mLTKMapKit->height()-mLayerOptionWidget->height())/2);

    //        if( mLayerOptionWidget->isHidden() )
    //        {
    //            mLayerOptionWidget->show();
    //        }
    //    }

}

void MapViewController::GetPinInfo(const locationtoolkit::Pin* pin) {

    if (!mIsNavigationStarted) {
        emit SignalPinInfoClicked(pin);
    }

}
void MapViewController::onStaticPOIClicked(const QString& id, const QString& name, double latitude, double longitude)
{
    qDebug()<<"MapViewController::onStaticPOIClicked";
    qDebug()<<"id::"<<id;
    qDebug()<<"name::"<<name;
    qDebug()<<"latitude::"<<latitude;
    qDebug()<<"longitude::"<<longitude;


}

void MapViewController::onMapClicked()
{

    emit HideSideMenu();
}

void MapViewController :: SetUpNavigationRouteController()
{
    mIsNavigationStarted = false;
    mStartNavGoClicked = false;
    //  Create Navigation Route View and UI
    mNavigationRoutesController = new navigationRouteController();
    QQmlEngine::setObjectOwnership(mNavigationRoutesController, QQmlEngine::CppOwnership);
    mNavigationRoutesController->setParentItem(this->parentItem());

    QQmlEngine::setContextForObject(mNavigationRoutesController, QQmlEngine::contextForObject(this));
    mNavigationRoutesController->setWidth(width());
    mNavigationRoutesController->setHeight(height());
    qDebug()<<"mNavRouteController::"<<mNavigationRoutesController;
    mNavigationRoutesController->setUp();
    mNavigationRoutesController->hide();


    //  Map Signal-Slots for Navigation Route Controller
    connect(mNavigationRoutesController, SIGNAL(highlightRoute(int)),         this, SLOT(onPolylineClicked(int)))  ;
    connect(mNavigationRoutesController, SIGNAL(startNavigationOnRoute(int)), this, SLOT(onStartNavigationOnRoute(int)));
    //  Map Signal-Slots for Navigation Implementation Controller
    connect(mNavSession,SIGNAL(addPolyLinesForPath(RouteRequestReason,QVector<QSharedPointer<RouteInformation> >)), this, SLOT(onAddPolyLinesForPath(RouteRequestReason,QVector<QSharedPointer<RouteInformation> >)));
    connect(mNavSession,SIGNAL(onLocationUpdated(Location)),                                                        this, SLOT(onLocationUpdated(Location)));
    connect(mNavSession,SIGNAL(PositionUpdated(Coordinates,qint32,qint32)),                                         this, SLOT(onPositionUpdated(Coordinates,qint32,qint32)));
    connect(mNavSession,SIGNAL(OffRoutePositionUpdate(qreal)),                                                      this, SLOT(onOffRoutePositionUpdate(qreal)) );
    connect(mNavSession,SIGNAL(RouteError(LTKError)),                                                               this, SLOT(onRouteError(LTKError)) );
    connect(mNavSession,SIGNAL(RouteFinish()),                                                                      this, SLOT(onRouteFinish()) );
    connect(mNavSession,SIGNAL(SpeedLimit(SpeedLimitInformation)),                                                  this, SLOT(OnSpeedLimit(const SpeedLimitInformation&)));
    connect(mNavSession,SIGNAL(DisableSpeedLimit()),                                                                this, SLOT(OnDisableSpeedLimit()) );
    connect(mNavSession,SIGNAL(TrafficAlerted(TrafficEvent)),                                                       this, SLOT(OnTrafficAlerted(TrafficEvent)) );
    connect(mNavSession,SIGNAL(TrafficChanged(TrafficInformation)),                                                 this, SLOT(OnTrafficChanged(TrafficInformation)) );
    connect(mNavSession,SIGNAL(DisableTrafficAlerted()),                                                            this, SLOT(OnDisableTrafficAlerted()) );
    connect(mNavSession,SIGNAL(RoadSign(const locationtoolkit::RoadSign&)),                                         this, SLOT(OnRoadSign(const locationtoolkit::RoadSign&)));
    connect(mNavSession,SIGNAL(LaneInformation(const locationtoolkit::LaneInformation&)),                           this, SLOT(OnLaneInformation(const locationtoolkit::LaneInformation&)));

}
void MapViewController::onPolylineClicked(const QList<locationtoolkit::Polyline*> polylines) {

    if (!mStartNavGoClicked) {
        //  Make all polyLines passive
        for (int i = 0; i < activePolylines.count(); i++) {
            Polyline* polyLine = activePolylines[i];
            polyLine->SetZOrder(15);
        }

        //  Active/Raise selected polyline
        Polyline* polyLine = polylines[0];
        polyLine->SetZOrder(16);
    }

}
void MapViewController::onLocationUpdated( const Location& location ) {

    //qDebug()<<"MapViewController::onLocationUpdated";

    //if(mStartNavGoClicked) {

    mLatInterpolator.Set( 1000, mLastLocation.latitude, location.latitude );
    mLonInterpolator.Set( 1000, mLastLocation.longitude, location.longitude );
    mHeadingInterpolator.Set( 1000, mLastLocation.heading, location.heading );
    mLastLocation = location;

    //  Start imterpolator to get map fixes and simulate avatar motion
    mInterpolatorTime.start();

    if(!mInterpolatorTimer.isActive()) {
        mInterpolatorTimer.start(INTERPOLATER_TIME_GAP);
    }
    //}

}

// Position updates from NavKit SDK
void MapViewController::onPositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading) {

    // update last heading
    mLastHeading = heading;

}

void MapViewController::onOffRoutePositionUpdate(qreal headingToRoute) {

    // update last heading
    mLastHeading = headingToRoute;

}


void MapViewController::onRouteError(LTKError error) {

    qDebug()<<"MapViewController::onRouteError"<<error.description();
    //closeAllPopUpWindows();

    //    if (!mWaitDialog->isHidden()) {
    //        mWaitDialog->hide();
    //    }

    // perform navigation end actions
    OnNavigationEnd();

}

void MapViewController::onRouteFinish() {

    // perform navigation end actions
    //    prepareNavigationEnd();
    OnNavigationEnd();
    if(mlowFuelMode == true && mClient == "Automotive")
    {
        setFuelAlertVisibility();
    }

    mlowFuelMode = false;
    if(mInNavMode)
    {
        mDestName = mUserDestName;
        ConfirmBoxController *mConfirmBoxController = new ConfirmBoxController();
        QQmlEngine::setObjectOwnership(mConfirmBoxController, QQmlEngine::CppOwnership);
        mConfirmBoxController->setParentItem(this->parentItem());
        QQmlEngine::setContextForObject(mConfirmBoxController, QQmlEngine::contextForObject(this->parentItem()));
        mConfirmBoxController->setSize(QSize(this->parentItem()->width(), this->parentItem()->height()));

        QString var1,var2;
        var1="Do you wish to continue to";
        var2=mUserDestName;
        mConfirmBoxController->messageStrings(var1,var2);
        mConfirmBoxController->setUp();
        mConfirmBoxController->setVisible(true);

        connect(mConfirmBoxController,SIGNAL(confirmBoxNotification(bool)),this,SLOT(onLowFuelConfirmBoxNotification(bool)));

    }
    else
    {
        QString message = "Arrived at destination";
        Messageboxcontroller *mMessageBox= new Messageboxcontroller();
        QQmlEngine::setObjectOwnership(mMessageBox, QQmlEngine::CppOwnership);
        mMessageBox->setParentItem(this->parentItem());
        QQmlEngine::setContextForObject(mMessageBox, QQmlEngine::contextForObject(this));
        mMessageBox->setWidth(width());
        mMessageBox->setHeight(height());
        mMessageBox->messageStrings(message,"");
        mMessageBox->setUp();
    }

    //    //  Hide any MessageBoxController if visible
    //    QListIterator<QObject *> alertWidget(this->children());

    //    //  iterate through each UI element of type MessageBoxController
    //    while (alertWidget.hasNext()) {
    //        QObject* alertObject = alertWidget.next();
    //        MessageBoxController* alert = qobject_cast<MessageBoxController*>( alertObject );

    //        if (alert) {
    //            if (!alert->isHidden()) {
    //                alert->hide();
    //            }
    //        }
    //    }

    //    //  Show destination arrived alert
    //    MessageBoxController* messageBox = new MessageBoxController(this);
    //    messageBox->setGeometry(0,0, this->width(), this->height());

    //    QJsonObject button1;
    //    button1["buttonTitle"] = "OK";
    //    button1["context"] = "closePopUp";
    //    QVariant button1Info = button1;

    //    messageBox->setUpUI("","You have arrived at the destination.",button1Info,0,false);
    //    const QPoint global = this->mapToGlobal(rect().center());
    //    messageBox->move(global.x() - messageBox->width() / 2, global.y() - messageBox->height() / 2);
    //    messageBox->setWindowFlags(Qt::Popup);
    //    messageBox->show();

}
//  slot executed when reach gas station on low fuel alert and wish to continue to actual destintion
void MapViewController::onLowFuelConfirmBoxNotification(bool val)
{

    mInNavMode = false;
    if(val)
    {
        startNavigationWithNavKit();
    }


}

//  slot executed when speed limit changes
void MapViewController::OnSpeedLimit(const SpeedLimitInformation& speedInfo) {
    qDebug() << "=====================OnSpeedLimit=====================";
    qDebug() << "CurrentSpeed: " << speedInfo.CurrentSpeed() << ", SpeedLimit: " << speedInfo.SpeedLimit();
    int speedLimitValue = qCeil((speedInfo.SpeedLimit() * 2.2369));
    QString speedLimit = QString("%1").arg(speedLimitValue);
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("speedLimitText", speedLimit);

    mSpeedLimitView->setVisible(true);

    updateSpeedLimit(speedLimit);

}

//  slot executed when no need to display speed limit information
void MapViewController::OnDisableSpeedLimit() {

    qDebug() << "=====================OnDisableSpeedLimit=====================";
    mSpeedLimitView->setVisible(false);
    updateSpeedLimit(" ");//to disable speedlimit in cluster

}

void MapViewController::OnTrafficAlerted(const TrafficEvent& trafficEvent) {

    if (!trafficEvent.GetDescription().isEmpty()) {
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        engine->rootContext()->setContextProperty("trafficAlertText", trafficEvent.GetDescription());
        mTrafficViewController->setVisible(true);
    }

}

void MapViewController::OnTrafficChanged(const TrafficInformation& trafficInfo) {

}

void MapViewController::OnDisableTrafficAlerted() {

    //    mTrafficViewController->hide();

}

//  slot executed when road sign changes
void MapViewController::OnRoadSign(const locationtoolkit::RoadSign& roadSign) {

    qDebug() << "=====================OnRoadSign=====================";
    qDebug() << "SignInformation: " << roadSign.SignInformation() << ", PortraitSignInformation: " << roadSign.PortraitSignInformation();

    QImage image = roadSign.GetImageData();

    if (!image.isNull()) {
        QImage roadSignImage = image.scaled(mRoadSignViewController->width(), mRoadSignViewController->height(), Qt::KeepAspectRatio);
        QPixmap roadSignPixmap = QPixmap::fromImage(roadSignImage);
        QString roadSignImageString = "image://pixmap/" + QString::number((qulonglong)&roadSignPixmap);
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        engine->rootContext()->setContextProperty("roadSignImage", roadSignImageString);
        mRoadSignViewController->setVisible(true);
    }

}

//  slot executed when lane information changes
void MapViewController::OnLaneInformation(const locationtoolkit::LaneInformation& laneInfo) {

    qDebug() << "=====================OnLaneInformation=====================";
    qDebug() << laneInfo.SelectedLanes();

}

void MapViewController::onAddPolyLinesForPath(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes) {

    qDebug()<<"MapViewController::onAddPolyLinesForPath";
    QString fastestRouteID;
    QString shortestRouteID;

    qreal lowestDistance = routes.at(0)->GetDistance();
    quint32 lowestTime = routes.at(0)->GetTime();

    //  TODO: Logic error - Does not always work - Remove in final Publish
    for (int i = 0; i < routes.count(); i++) {
        QSharedPointer<RouteInformation> routeInfo = routes.at(i);
        if (routeInfo->GetDistance() < lowestDistance) {
            lowestDistance = routeInfo->GetDistance();
            fastestRouteID = routeInfo->GetRouteID();
        }
        if (routeInfo->GetTime() < lowestTime) {
            lowestTime = routeInfo->GetTime();
            shortestRouteID = routeInfo->GetRouteID();
        }
    }

    if (routes.count() == 1 ) {
        // only have 1 fastest route
        shortestRouteID.clear();
    }

    BoundingBox box;
    LatLngBound* latlongBounds;
    qreal distances[3] = {-1, -1, -1};
    qreal totalTimes[3] = {-1, -1, -1};

    //  Remove all active polyLines
    //  clear objects from QList

    int n = activePolylines.size();
    for (int i = 0; i < n; ++i)
    {
        mLTKMapKit->RemovePolyline(activePolylines[i]);
    }
    activePolylines.clear();


    //  Remove all previously added maneuverArrowPolylines
    clearManeuvers();

    for (int i = 0; i < routes.count(); i++) {

        QSharedPointer<RouteInformation> routeInfo = routes.at(i);

        if (reason != RRR_RouteSelector && mStartNavGoClicked) {
            // draw only first route if received from recalculation/detour/calculation
            routeInfo = routes.at(0);
        }

        QVector<Coordinates*> coor_list = routeInfo->GetPolyline();

        //qDebug() << "Route "<< QString::number(i) <<" is: "<<routeInfo->GetRouteID()<< "  ---  "
        //<< QString::number(routeInfo->GetDistance(), 'f', 2)
        //<< ", "<<QString::number(routeInfo->GetTime(), 'f', 10) ;

        PolylineParameters param;// = new PolylineParameters();
        QList<Coordinates> points;
        for(int i = 0; i < coor_list.count(); i++) {
            points.append(*coor_list[i]);
        }
        //to get polyline in cluster
        if(mStartNavGoClicked)
        {
            QJsonObject jPolyline;
            QJsonArray jCoordinates;

            for(int i = 0;i<points.count();i++)
            {
                QJsonObject ob;
                Coordinates val = points.at(i);
                ob.insert("lattitude", QString::number(val.getLatitude(),'f',6));
                ob.insert("longitude", QString::number(val.getLongitude(),'f',6));
                jCoordinates.append(ob);
            }
            jPolyline.insert("coordinates",jCoordinates);
            QJsonDocument doc(jPolyline);
            QString dataToString(doc.toJson(QJsonDocument::Compact));
            setPolyline(dataToString);
        }
        //
        int total = points.count();
        //  Setup parameters for on route polyline
        QList<locationtoolkit::SegmentAttribute*>* mSegAttr = new QList<locationtoolkit::SegmentAttribute*>();
        locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(total - 1, QColor(Qt::green));
        mSegAttr->append(segattr);
        param.SetSegmentAttributes(*mSegAttr);
        param.SetPoints(points);
        param.SetWidth(15);
        param.SetVisible(true);
        param.SetAttributeType(PolylineParameters::SAT_ColorSegment);

        if (/*routeInfo->GetRouteID() == fastestRouteID*/ i == 0) { //  Route No 1
            //param.SetUnhighlightColor(QColor(255,134,0,255));//yellow while on glow
            param.SetUnhighlightColor(QColor(255, 60, 0,255));
            distances[0] = routeInfo->GetDistance();
            totalTimes[0] = routeInfo->GetTime();
        }
        else if (/*routeInfo->GetRouteID() == shortestRouteID*/ i == 1) {   //  Route No 2
            param.SetUnhighlightColor(QColor(0,222,255,255));
            distances[1] = routeInfo->GetDistance();
            totalTimes[1] = routeInfo->GetTime();

        }
        else {  //  Route No 3
            param.SetUnhighlightColor(QColor(0,249,56,255));
            distances[2] = routeInfo->GetDistance();
            totalTimes[2] = routeInfo->GetTime();
        }

        if (reason != RRR_RouteSelector && mStartNavGoClicked) {
            //  route received from recalculation/detour/calculation
            param.SetUnhighlightColor(QColor(0,0,255,255));
        }
        else {
            // hide top-bottom widgets
            OnNavigationStarted();
        }

        param.setOutlineColor(QColor(Qt::darkGray));
        param.setOutlineWidth(15);
        Polyline* polyLine = &(mLTKMapKit->AddPolyline(param));

        //  add polylines to array
        activePolylines.append(polyLine);

        //  create bounding box for showing route info
        box = routeInfo->GetBoundingBox();
        latlongBounds = new LatLngBound(box.point1.latitude,box.point1.longitude,box.point2.latitude,box.point2.longitude);
    }

    // hide progress hud
    //    mWaitDialog->hide();

    if (reason == RRR_RouteSelector) {

        // only show route list if received from routeselector otherwise avoid showing route list if route received from recalculation/detour/calculation

        //  zoom map to bounding box
        mLTKMapKit->ZoomToBoundingBox(latlongBounds->topLeftLatitude,latlongBounds->topLeftLongitude,latlongBounds->bottomRightLatitude,latlongBounds->bottomRightLongitude);

        //  move map to show right part of screen as we have to show route info to left part
        double* currentLat = new double();
        double* currentLon = new double();
        qDebug() << "mainwindow rect: " << this->width()/2 << this->height()/2;
        //mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2,this->height()/2,*currentLat,*currentLon);
        mLTKMapKit->GetMapProjection().FromScreenPosition(this->width(),this->height() + 200,*currentLat,*currentLon);//temp fix
        Coordinates *newCoordinates = new Coordinates();
        newCoordinates->latitude = *currentLat;
        newCoordinates->longitude = *currentLon;

        //QPoint newPoint = QPoint((this->width()+mNavigationRoutesController->width())/2,this->height());
        QPoint newPoint = QPoint((this->width()+mNavigationRoutesController->width())/2,this->height());//temp fix

        CameraParameters *currentCamPara = new CameraParameters(box.point1);
        mLTKMapKit->GetCameraPosition(*currentCamPara);
        //  Setup camera position, view and zoom/tilt
        QSharedPointer<CameraParameters> q_param = CameraFactory::CreateCameraParametersForCoordinatesAtPosition(newPoint,
                                                                                                                 this->width(),
                                                                                                                 this->height(),
                                                                                                                 *newCoordinates,
                                                                                                                 currentCamPara->GetTiltAngle(),
                                                                                                                 currentCamPara->GetZoomLevel() - 0.5,
                                                                                                                 currentCamPara->GetHeadingAngle());
        CameraParameters* newCamPara = static_cast<CameraParameters*>(q_param.data());
        mLTKMapKit->MoveTo(*newCamPara);

        // show route information
        qDebug()<<"mNavigationRoutesController->updateLabels";
        mNavigationRoutesController->updateLabels(distances, totalTimes);
        mNavigationRoutesController->show();

        // show nav header
        mNavHeader->setVisible(true);

        //        mManeuverList->setGeometry(0,0,this->width(), this->height()/7);
        //        mManeuverList->mapToggleSetHidden(true);
        //        mManeuverList->goBackTo(0);
        //        mManeuverList->show();
    }
    if(mlowFuelMode)
    {
        onStartNavigationOnRoute(0);
    }
    qDebug()<<"MapViewController::onAddPolyLinesForPath::end";
}

//  Start simulating Avatar motion on the polyline
void MapViewController::OnNavigationStarted() {

    // hide top-bottom views
    //    ui->topMenuWidget->hide();

    MapDecoration* mapDecoration = mLTKMapKit->GetMapDecoration();
    mapDecoration->SetLocateMeButtonEnabled( false );
    mIsNavigationStarted = true;
}
void MapViewController::onStartNavigationOnRoute(int routeIdx) {


    // D-Bus call
    setNavigationMode(true);
    if(mClient == "Automotive" || mClient == "Neptune")
    {
    QObject *object1 = mNavHeader->findChild<QObject *>("NavModeSwitchObj");
    object1->setProperty("visible", true);
    }
    autoNavMode=true;
    mStartNavGoClicked = true;
    mNavigationRoutesController->hide();

    //    mManeuverList->mapToggleSetHidden(false);
    //    mManeuverList->goBackTo(1);
    mManeuverDetailsController->show();
    emit resetMapCameraButton();
    mMapCameraLockButton->setVisible(true);
    setMapCameraLockButtonPosition(mIsNavigationStarted);
    //mRouteOverviewButton->setVisible(true);
    qDebug()<<"MapViewController::onStartNavigationOnRoute";
    //  Remove all active polyLines

    int n = activePolylines.size();
    for (int i = 0; i < n; ++i)
    {
        mLTKMapKit->RemovePolyline(activePolylines[i]);
    }
    activePolylines.clear();

    //  Get the selected route's polyline
    QSharedPointer<RouteInformation> routeInfo = mNavSession->mRouteInformationList.at(routeIdx);
    QVector<Coordinates*> coor_list = routeInfo->GetPolyline();

    PolylineParameters param;
    QList<Coordinates> points;
    for(int i = 0; i < coor_list.count(); i++) {
        points.append(*coor_list[i]);
    }

    int total = points.count();
    //  Set attributes for the polyLine under consideration
    QList<locationtoolkit::SegmentAttribute*>* mSegAttr = new QList<locationtoolkit::SegmentAttribute*>();
    locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(total - 1, QColor(Qt::green));
    mSegAttr->append(segattr);
    param.SetSegmentAttributes(*mSegAttr);
    param.SetPoints(points);
    param.SetWidth(15);
    param.SetVisible(true);
    param.SetAttributeType(PolylineParameters::SAT_ColorSegment);
    param.SetUnhighlightColor(QColor(0,0,255,255));
    param.setOutlineColor(QColor(Qt::darkGray));
    param.setOutlineWidth(15);
    Polyline* polyLine = &(mLTKMapKit->AddPolyline(param));

    //to get polyline in cluster
    QJsonObject jPolyline;
    QJsonArray jCoordinates;

    for(int i = 0;i<points.count();i++)
    {
        QJsonObject ob;
        Coordinates val = points.at(i);
        ob.insert("lattitude", QString::number(val.getLatitude(),'f',6));
        ob.insert("longitude", QString::number(val.getLongitude(),'f',6));
        jCoordinates.append(ob);
    }
    jPolyline.insert("coordinates",jCoordinates);
    QJsonDocument doc(jPolyline);
    QString dataToString(doc.toJson(QJsonDocument::Compact));
    setPolyline(dataToString);
    //

    //  add polylines to active plyline's array
    activePolylines.append(polyLine);

    //  enable reference center to adjust navigation simulation on right side
    mLTKMapKit->EnableReferenceCenter(true);
    //QPoint pos = QPoint((this->width()+mNavigationRoutesController->width())/2,this->height() - 200);
    QPoint pos = QPoint((this->width()+mManeuverDetailsController->width())/2,this->height()*2/3);//temp fix
    mLTKMapKit->SetReferenceCenter( pos );

    //  get settings object

    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    //QSettings settings(gOrganization, gApplication);

    //  Set Route type needed
    QString defaultViewType = settings.value(gDefaultViewType).toString();

    if (defaultViewType == "2D Trip Overview") {
        //onNavigationMapOverview();
        mMapCameraLocked = true;
        // follow me mode for camera lock behind avatar arrow
        mLTKMapKit->SetGpsMode(MapWidget::GM_FOLLOW_ME,false);
        //        mManeuverList->showMapForList(false);
    }
    else if (defaultViewType == "List View") {
        mMapCameraLocked = true;
        // follow me mode for camera lock behind avatar arrow
        mLTKMapKit->SetGpsMode(MapWidget::GM_FOLLOW_ME,false);
        //        mManeuverList->showMapForList(false);
    }
    else {
        // default is already set to Perspective View
        mMapCameraLocked = true;
        // follow me mode for camera lock behind avatar arrow
        mLTKMapKit->SetGpsMode(MapWidget::GM_FOLLOW_ME,false);
    }
    settings.endGroup();

    MapDecoration* mapdecoration = mLTKMapKit->GetMapDecoration();
    mapdecoration->SetCompassEnabled(true);

    emit startNavigationOnRoute(routeIdx);

}

void MapViewController ::InitializeNavigationHeaderBar()
{
    // navigation header-bar

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent componentNavBar(engine, QUrl(appDirPath+"/resource/navigationGoScreenHeader.qml"));
    mNavHeader = qobject_cast<QQuickItem*>(componentNavBar.create());
    connect( mNavHeader, SIGNAL(navBackButtonClicked()), this, SLOT(onNavStopButtonClicked()));
    connect(mNavHeader, SIGNAL(maneuverListClicked()), this, SLOT(onManeuverListClicked()));
    connect(mNavHeader, SIGNAL(setClusterUINavMode(int)),this,SLOT(onSetClusterUINavMode(int)));
    QQmlEngine::setObjectOwnership(mNavHeader, QQmlEngine::CppOwnership);
    mNavHeader->setParentItem(this->parentItem());
    //mNavHeader->setParent(this);
    mNavHeader->setPosition(QPoint(0, 0));
    qDebug() << "mNavHeader Height :: " << this->parentItem()->height()/9;
    mNavHeader->setSize(QSize(this->parentItem()->width(), this->parentItem()->height()/9));
    mNavHeader->setVisible(false);
}

void MapViewController::onSideItemVisibility(bool visible)
{
    mZoomInButton->setVisible(visible);
    mZoomOutButton->setVisible(visible);
    mLayerOptionsButton->setVisible(visible);
    mMapCameraLockButton->setVisible(visible);
}

void MapViewController::onSetClusterUINavMode(int mode){
    qDebug() << "----------------------------- MapViewController ::onSetClusterUINavMode()------------------------------" << mode ;
    // D-Bus call
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if(ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("setClusterUIMode", mode);
        //if(reply.isValid()) {
        //}
        //else {
        //    qDebug("call onSetClusterUINavMode");
        //}

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("setClusterUIMode", mode);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
}

void MapViewController::dbusCallFinishedSlot(QDBusPendingCallWatcher* watcher)
{

}

void MapViewController ::onManeuverListClicked(){
    qDebug() << "----------------------------- MapViewController ::onManeuverListClicked()------------------------------" ;
    //mManeuversListController->show();
    setupManeuverListSignalAndSlots();
    //  mManeuversListController->setManeuverList(mManeuverDetailsController->getManeuverList());

}
void MapViewController::onNavStopButtonClicked()
{
    ConfirmBoxController *mConfirmBoxController = new ConfirmBoxController();
    QQmlEngine::setObjectOwnership(mConfirmBoxController, QQmlEngine::CppOwnership);
    mConfirmBoxController->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mConfirmBoxController, QQmlEngine::contextForObject(this->parentItem()));
    mConfirmBoxController->setSize(QSize(this->parentItem()->width(), this->parentItem()->height()));

    QString var1,var2;
    var1="Do you wish stop";
    var2="Navigation";
    mConfirmBoxController->messageStrings(var1,var2);
    mConfirmBoxController->setUp();
    mConfirmBoxController->setVisible(true);

    connect(mConfirmBoxController,SIGNAL(confirmBoxNotification(bool)),this,SLOT(onConfirmBoxNotification(bool)));

}

void MapViewController::onConfirmBoxNotification(bool val)
{
    qDebug()<<"MapViewController::onConfirmBoxNotification";
    if(val)
        abortRouteNavigation();

}

void MapViewController::abortRouteNavigation() {

    //    // hide navigation related screens
    //    mNavigationRoutesController->hide();
    //    mManeuverDetailsController->hide();
    //    mNavHeader->setVisible(false);
    //    mLTKMapKit->DeleteAllPins();
    //    mLTKMapKit->EnableReferenceCenter(false);
    //    mPolyPoints.clear();
    //    mIsNavigationStarted = false;
    //  Hide avatar
    //mLTKMapKit->GetAvatar().SetMode(locationtoolkit::Avatar::AM_NONE);


    if (mStartNavGoClicked) {

        OnNavigationEnd();

        //  Navigation is running, show confirmation alert for stop navigation
        //        MessageBoxController* messageBox = new MessageBoxController(this);
        //        messageBox->setGeometry(0,0, this->width(), this->height());
        //        connect( messageBox, SIGNAL(messageButtonClicked(QPushButton*)), this, SLOT(onNavStopConfirmation(QPushButton*)));

        //        QJsonObject button1;
        //        button1["buttonTitle"] = "Yes";
        //        button1["context"] = "stopNavigation";
        //        QVariant button1Info = button1;

        //        QJsonObject button2;
        //        button2["buttonTitle"] = "No";
        //        QVariant button2Info = button2;

        //        messageBox->setUpUI("","Stop Navigation?",button1Info,button2Info);
        //        messageBox->show();
    }
    else {

        //  Navigation is not running, perform navigation end actions
        OnNavigationEnd();

    }
    if(mlowFuelMode == true && mClient == "Automotive")
    {
        setFuelAlertVisibility();
    }
     mlowFuelMode = false;
}
//  Actions performed navigation end
void MapViewController::OnNavigationEnd() {


    // D-Bus call
    setNavigationMode(false);
    QObject *object1 = mNavHeader->findChild<QObject *>("NavModeSwitchObj");
    object1->setProperty("visible", false);
    
    //  check if pre-navigation end actions are taken or not
    onDestroyManeuverList();
    if (mIsNavigationStarted) {
        prepareNavigationEnd();
    }
    mIsNavigationStarted = false;
    mStartNavGoClicked = false;
    emit ToggleTitleBarVisibility(true);
    //  Clear Map
    mLTKMapKit->Clear();
    onUpdateCameraParam();
    autoNavMode = true;
    clearPolyline();

    //OnMapCreated();
    //  Hide ManeuverList
    //    if(!mManeuverList->isHidden()) {
    //        mManeuverList->setHidden(true);
    //    }

    //mManeuverDetailsController->clearFields();
    //    mManeuverList->clearFields();

    // show top-bottom views
    //    ui->topMenuWidget->show();

}
//  Actions performed just before calling OnNavigationEnd() method
void MapViewController::prepareNavigationEnd() {

    qDebug()<<"MapViewController::prepareNavigationEnd";
    mLTKMapKit->EnableReferenceCenter(false);
    //mMapCameraLocked = false;

    //    // hide camera lock and route overview buttons
    mMapCameraLockButton->setVisible(false);
    //mRouteOverviewButton->setVisible(false);
    mSpeedLimitView->setVisible(false);
    mTrafficViewController->setVisible(false);
    mRoadSignViewController->setVisible(false);

    // hide navigation related screens
    mNavigationRoutesController->hide();
    mNavHeader->setVisible(false);
    mManeuverDetailsController->hide();
    //emit ToggleTitleBarVisibility();

    //  Hide avatar
    //mLTKMapKit->GetAvatar().SetMode(locationtoolkit::Avatar::AM_NONE);

    //  Remove all active polyLines
    for(int i = 0 ; i < activePolylines.size(); ++i) {
        activePolylines.removeAt(i);
    }
    activePolylines.clear();

    //    mLTKMapKit->setGeometry(centralWidget()->rect());

    MapDecoration* mapdecoration = mLTKMapKit->GetMapDecoration();
    mapdecoration->SetLocateMeButtonEnabled( false );
    mapdecoration->SetZoomButtonEnabled( true );
    mapdecoration->SetLayerOptionButtonEnabled( true );
    mapdecoration->SetCompassEnabled(false);

    //    if (mInterpolatorTimer.isActive()) {
    //        mInterpolatorTimer.stop();
    //    }

    //  Signal Stop Navigation to release object from memory in NavigationSession class
    emit StopNavigation();

    //    mIsNavigationStarted = false;
    //    mStartNavGoClicked = false;

}

void MapViewController::OnStartNavigationToDestination(QString destName, int id, QVariant placeData) {

    qDebug()<<"MapViewController::OnStartNavigationToDestination";
    mDestName = destName;
    //mManeuversListController->setDestinationName(destName);
    mNavHeader->setVisible(true);
    //mNavigationRoutesController->show();
    //    switch(id) {
    //        case 0:
    ////            mNavStartedFromWdiget = mPinInformationWidget;
    //            break;
    //        case 1:
    //            mNavStartedFromWdiget = mCategorySearchController;
    //            break;
    //        case 2:
    ////            mNavStartedFromWdiget = mSearchController;
    //            break;
    //        default:
    //        break;
    //    }

    if (placeData != 0) {
        mSearchedPinInfo = placeData;
    }

    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    
    settings.beginGroup("AppSettings");
    //QSettings settings(gOrganization, gApplication);
    QString GpsFileName = settings.value(gGpsFileName,"demo.gps").toString();

    if(destName == "Levi's Stadium") {
        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "IS-Levis.gps";
    }
    else if(destName == "Shoreline Amphitheatre") {
        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "IS-Shoreline.gps";
    }
    else if(destName == "San Jose Airport") {
        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "IS-SJC.gps";
    }
    else if(destName == "San Jose State University") {
        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "IS-SJState.gps";
    }
    else {
        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + GpsFileName; //"enterExitHwyOnRight.gps";
        //        mGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "FastPOI_NIM_Alicia5Fwy.gps";
    }
    settings.endGroup();
    // start navigation using navKit

    startNavigationWithNavKit();



}
// Start navigation using navkit lirbary
void MapViewController::startNavigationWithNavKit() {

    //to stop navigation if already in navigation while on low fuel mode
    if(mInNavMode)
    {
        qDebug()<<"End Navigation";
        //mInNavMode = false;
        OnNavigationEnd();

    }
    //    //  close all screens
    //    closeAllPopUpWindows();

    //    //  disable weather
    //    onToggleWeather(false);

    //  show loading screen
    //    mWaitDialog->show();

    //  clear Map
    mLTKMapKit->Clear();
    autoNavMode = false;
    //mNavSession->modifyLocationServices(false);
    //  get GPS file configurations

    QString mCurrentGPSFile = mGPSFile;
    //    QString mCurrentGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + "enterExitHwyOnRight.gps";
    qDebug() << "mCurrentGPSFile:"<< mCurrentGPSFile;

    Place dest, org;

    GetDefaultDestinationAndOrigin(mGPSFile.toStdString(), &dest, &org);

    // show source and destination location
    locationtoolkit::Location source;
    source.latitude = mLastLocation.latitude;
    source.longitude = mLastLocation.longitude;
    source.heading = 60.0;
    source.valid = 507;

    locationtoolkit::Coordinates destination;
    //    destination.latitude = dest.GetLocation().center.latitude;
    //    destination.longitude = dest.GetLocation().center.longitude;
    if(!mlowFuelMode && mInNavMode)
    {
        destination.latitude = mUserDestPoint.x();
        destination.longitude = mUserDestPoint.y();
    }
    else
    {
        destination.latitude = mNavDestPoint.x();
        destination.longitude = mNavDestPoint.y();
    }
    qDebug()<<"destination::"<<destination.latitude<<"::"<<destination.longitude;

    QPixmap selectImage, unselectImage;
    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    //    selectImage.load(resourceFolder + "images/pin_normal_on.png");
    //    unselectImage.load(resourceFolder + "images/pin_end.png");
    selectImage.load(resourceFolder + "images/pin_select.png");
    unselectImage.load(resourceFolder + "images/pin_unselect.png");

    locationtoolkit::PinImageInfo selectedImage;
    selectedImage.SetPixmap(selectImage);
    selectedImage.SetPinAnchor(50, 100);
    locationtoolkit::PinImageInfo unSelectedImage;
    unSelectedImage.SetPixmap(unselectImage);
    unSelectedImage.SetPinAnchor(50, 100);

    locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);
    locationtoolkit::Bubble* bubble = new CustomBubble();

    locationtoolkit::PinParameters pinpara( destination,
                                            selectedImage,
                                            unSelectedImage,
                                            radiusPara,
                                            "",
                                            "",
                                            bubble,
                                            true);

    pinpara.SetSelectedImage(&selectedImage);
    pinpara.SetUnselectedImage(&unSelectedImage);

    //  Create pin and display it on the map
    Pin *createdPin = mLTKMapKit->CreatePin(pinpara);
    locationtoolkit::CameraParameters param(destination);
    param.SetPosition( destination );
    mLTKMapKit->MoveTo( param );

    //  get settings object
    //QSettings settings(gOrganization, gApplication);
    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");

    //  Set Route type needed
    QString routeType = settings.value(gRouteType).toString();
    RouteType mCurrentRouteType;

    if (routeType == "Fastest") {
        mCurrentRouteType = RT_Fastest;
    }
    else if (routeType == "Shortest") {
        mCurrentRouteType = RT_Shortest;
    }/*
    else if (routeType == "ECO") {
        mCurrentRouteType = RT_Easiest;
    }*/
    else {
        mCurrentRouteType = RT_Fastest;
    }

    TransportationMode mCurrentVehicleType = TM_Car;

    // set avoid
    QStringList avoidList = settings.value(gAvoidType).toStringList();
    quint32 mCurrentAvoid = 0;

    if (avoidList.contains("Toll Roads")) {
        mCurrentAvoid = mCurrentAvoid | RA_Toll;
    }
    else {
        mCurrentAvoid = mCurrentAvoid & (~RA_Toll);
    }

    if (avoidList.contains("Ferries")) {
        mCurrentAvoid = mCurrentAvoid | RA_Ferry;
    }
    else {
        mCurrentAvoid = mCurrentAvoid & (~RA_Ferry);
    }

    if (avoidList.contains("Hov Lanes")) {
        mCurrentAvoid = mCurrentAvoid | RA_HOV;
    }
    else {
        mCurrentAvoid = mCurrentAvoid & (~RA_HOV);
    }

    if (avoidList.contains("Highways")) {
        mCurrentAvoid = mCurrentAvoid | RA_Highway;
    }
    else {
        mCurrentAvoid = mCurrentAvoid & (~RA_Highway);
    }

    //  Make route request with following route options
    RouteOptions* routeOption = new RouteOptions(mCurrentRouteType,mCurrentVehicleType,mCurrentAvoid);

    Place placeDestination;
    MapLocation maplocation;
    maplocation.center.latitude  = destination.latitude;
    maplocation.center.longitude = destination.longitude;
    placeDestination.SetLocation(maplocation);

    //  Set measurement units and accuracy
    Preferences::Measurement mCurrentMeasurmentOption = Preferences::NonMetric;
    quint32 mCurrentOffRouteSensitivity = QString("1").toUInt();

    Preferences preference;
    preference.SetMeasurement(mCurrentMeasurmentOption);
    preference.SetOffRouteIgnoreCount(mCurrentOffRouteSensitivity);
    preference.SetMultipleRoutes(true);
    preference.SetSpeedLimitSignEnabled(true);
    preference.SetTrafficAnnouncements(true);
    preference.SetRealisticSign(true);

    QString mCurrentVoiceGuide = "Lane Guidance";

    //  Set voice guidance type
    if (mCurrentVoiceGuide == "Regular") {
        preference.SetLaneGuidance(false);
        preference.SetNaturalGuidance(false);
    }
    else if (mCurrentVoiceGuide == "Natural Guidance") {
        preference.SetNaturalGuidance(true);
    }
    else if (mCurrentVoiceGuide == "Lane Guidance") {
        preference.SetLaneGuidance(true);
    }
    else if (mCurrentVoiceGuide == "Natural and Lane Guidance") {
        preference.SetLaneGuidance(true);
        preference.SetNaturalGuidance(true);
    }
    else {
        preference.SetLaneGuidance(false);
        preference.SetNaturalGuidance(false);
    }

    settings.endGroup();
    //  Ensure GPS file to be used is correctly updated
    //emit UpdateGpsFile(mCurrentGPSFile);

    //  Start navigation based on the route options and user specifications
    emit StartNavigation(*mLTKContext, placeDestination, *routeOption, preference);

}


MapViewController::~MapViewController()
{
    delete mNavSession;
}

//void MapViewController::startNavigation()
//{
//    using namespace locationtoolkit;
//    LTKContext* ltkContext = GetLTKContext();

//    // hardcode parameters
//    QString mCurrentVoiceGuide = "Regular";
//    RouteType mCurrentRouteType = RT_Fastest;
//    TransportationMode mCurrentVehicleType = TM_Car;
//    quint32 mCurrentAvoid = 0;
//    Preferences::Measurement mCurrentMeasurmentOption = Preferences::Metric;
//    int mCurrentOffRouteSensitivity = 1;

//    RouteOptions routeOption(mCurrentRouteType,mCurrentVehicleType,mCurrentAvoid);

//    MapLocation maplocation;
//    Place place;
//    maplocation.center = mPolyPoints.last();
//    place.SetLocation(maplocation);

//    Preferences preference;
//    preference.SetMeasurement(mCurrentMeasurmentOption);
//    preference.SetOffRouteIgnoreCount(mCurrentOffRouteSensitivity);


//    if(mCurrentVoiceGuide == "Regular")
//    {
//        preference.SetLaneGuidance(false);
//        preference.SetNaturalGuidance(false);
//    }
//    else if(mCurrentVoiceGuide == "Natural Guidance")
//    {
//        preference.SetNaturalGuidance(true);
//    }
//    else if(mCurrentVoiceGuide == "Lane Guidance")
//    {
//        preference.SetLaneGuidance(true);
//    }
//    else if(mCurrentVoiceGuide == "Natural and Lane Guidance")
//    {
//        preference.SetLaneGuidance(true);
//        preference.SetNaturalGuidance(true);
//    }
//    else
//    {
//        preference.SetLaneGuidance(false);
//        preference.SetNaturalGuidance(false);
//    }

//    mNavSession->OnStartNavigation(*ltkContext, place, routeOption, preference);
//}

void MapViewController::stopNavigation()
{

    mNavSession->OnStopNavigation();
}

static double distance(locationtoolkit::Coordinates& p0, locationtoolkit::Coordinates& p1)
{
    double x0 = lon2x_m(p0.getLongitude());
    double y0 = lat2y_m(p0.getLatitude());
    double x1 = lon2x_m(p1.getLongitude());
    double y1 = lat2y_m(p1.getLatitude());
    double x = x1 - x0;
    double y = y1 - y0;
    return sqrt(x * x + y * y);
}

static locationtoolkit::Coordinates lerp(locationtoolkit::Coordinates& p0, locationtoolkit::Coordinates& p1, double t)
{
    double x0 = lon2x_d(p0.getLongitude());
    double y0 = lat2y_d(p0.getLatitude());
    double x1 = lon2x_d(p1.getLongitude());
    double y1 = lat2y_d(p1.getLatitude());
    double invT = 1.0 - t;
    double x = x2lon_d(x0 * invT + x1 * t);
    double y = y2lat_d(y0 * invT + y1 * t);
    locationtoolkit::Coordinates result(y, x);
    return result;
}

static void cutPolyline(const QVector<locationtoolkit::Coordinates*>& polyline, double maxLength, bool cutFromHead, QVector<locationtoolkit::Coordinates>& result)
{
    double total = 0;
    int n = polyline.size();
    if (n == 0)
    {
        return;
    }

    result.push_back(cutFromHead ? *polyline.first() : *polyline.last());
    for (int i = 1; i < n; ++i)
    {
        locationtoolkit::Coordinates& p0 = cutFromHead ? *polyline[i - 1] : *polyline[n - i];
        locationtoolkit::Coordinates& p1 = cutFromHead ? *polyline[i] : *polyline[n - 1 - i];
        if (p0 == p1)
        {
            continue;
        }
        double segLength = distance(p0, p1);
        if (total + segLength <= maxLength)
        {
            result.push_back(p1);
            total += segLength;
        }
        else
        {
            float t = (maxLength - total) / segLength;
            result.push_back(lerp(p0, p1, t));
            break;
        }
    }
}

void MapViewController::maneuverListUpdated(const locationtoolkit::ManeuverList& l)
{
    qDebug()<<"################## MapViewController::maneuverListUpdated "<<l.GetNumberOfManeuvers();

    saveManeuverList(l);
    for(int i = 0; i < l.GetNumberOfManeuvers(); i++) {

        const locationtoolkit::Maneuver* m = l.GetManeuver(i);

        qDebug()<<"Maneuver Details "<<m->GetCommand()<<" "<<m->GetDistance()<<" "<<m->GetManeuverID()<<" "<<m->GetPrimaryStreet()<<" "<<m->GetRoutingTTF()<<" "<<m->GetSecondaryStreet();
    }

    qDebug()<<"##############################################";

    const locationtoolkit::Maneuver* m = l.GetManeuver(0);
    QString distance = LTKSampleUtil::FormatDistance(m->GetDistance());

    QString ttf = m->GetRoutingTTF();
    
    qDebug()<<"##### MapViewController::updateManeuverList "<<ttf<<" "<<m->GetPrimaryStreet()<<" "<<distance;

    if(mStartNavGoClicked && !m->IsDestination())
    {

        setManeuver(ttf, m->GetPrimaryStreet(), distance, "");

        // update arrows
        clearManeuvers();
        int n = l.GetNumberOfManeuvers();
        if (n > 1)
        {
            const QVector<locationtoolkit::Coordinates*>& pl0 = l.GetManeuver(0)->GetPolyline();
            const QVector<locationtoolkit::Coordinates*>& pl1 = l.GetManeuver(1)->GetPolyline();
            if (pl1.size() >= 2 && pl0.size() >= 2)
            {
                const double MAX_MANEUVER_ARROW_TAIL_LENGTH   = 90.0; // meters
                const double MAX_MANEUVER_ARROW_HEADER_LENGTH = 46.0; // meters

                QVector<locationtoolkit::Coordinates> ptsTail;
                QVector<locationtoolkit::Coordinates> pts;
                cutPolyline(pl0, MAX_MANEUVER_ARROW_TAIL_LENGTH, false, ptsTail);
                int numTail = ptsTail.size();
                if (numTail > 0)
                {
                    for (int i = 0; i < numTail - 1; ++i)//ignore dup 1st point
                    {
                        pts.push_back(ptsTail[numTail - 1 - i]);
                    }
                }
                cutPolyline(pl1, MAX_MANEUVER_ARROW_HEADER_LENGTH, true, pts);

                addManeuverArrow(pts);
            }
        }
    }
}

void MapViewController::saveManeuverList(const ManeuverList &maneuverlist)
{
    qDebug()<<"MapViewController::saveManeuverList :: " << mManeuverDataList.length();

    if(mManeuverDataList.length() > 0)
    {
        mManeuverDataList.clear();
    }
    quint32 maneuverCount = maneuverlist.GetNumberOfManeuvers();
    for(int i = 0; i < (int)maneuverCount; ++i) {
        const Maneuver* maneuver = maneuverlist.GetManeuver(i);

        // Add Search Result Row to List View Widget
        //maneuverItemWidget* maneuverWidget = new maneuverItemWidget();
        //maneuverWidget->setGeometry(0,0,mManeuverWidgetItemWidth,mManeuverWidgetItemHeight);
        QString streetName;
        if(i!=(maneuverCount - 1)) {
            streetName = maneuver->GetPrimaryStreet();
        }
        else {
            streetName = "You have arrived at your destination";
        }
        QString arrowImageString = /*"image://pixmap/" +*/ maneuver->GetRoutingTTF();

        mManeuverDataList.append(new ManeuverDataObject(arrowImageString,streetName, LTKSampleUtil::FormatDistance(maneuver->GetDistance())));

    }
    if(mManeuversListController != NULL)
    {
        mManeuversListController->setManeuverList(mManeuverDataList);
        mManeuversListController->show();
    }


}

void MapViewController::ManeuverRemainingDistanceUpdated(qreal rDistance)
{
    QString distance = LTKSampleUtil::FormatDistance(rDistance);

    setRemainingManeuverDistance(distance);

}

void MapViewController::updatePosition(const locationtoolkit::Location& location)
{
    //LocationUpdated(location);
}

void MapViewController::routeFinish()
{
    //    stopNavigation();
    //    startNavigation();
}

void MapViewController::routeReceived(const QVector< QSharedPointer<locationtoolkit::RouteInformation> >& routes)
{
    using namespace locationtoolkit;
    mRoutePolyPoints.clear();
    if(!routes.isEmpty())
    {
        QSharedPointer<RouteInformation> routeInfo = routes.at(0);
        const QVector<Coordinates*>& pl = routeInfo->GetPolyline();
        foreach(Coordinates* c, pl)
        {
            mRoutePolyPoints.push_back(*c);
        }
    }
    addRoute();
}

void MapViewController::clearManeuvers()
{
    int n = mManeuverArrows.size();
    for (int i = 0; i < n; ++i)
    {
        globalMapWidget->RemovePolyline(mManeuverArrows[i]);
    }
    mManeuverArrows.clear();
}

void MapViewController::addManeuverArrow(const QVector<locationtoolkit::Coordinates>& pts)
{
    if (!globalMapWidget)
    {
        return;
    }

    const float MANEUVER_ARROW_MANEUVER_WIDTH    = 12.0f;
    const float MANEUVER_ARROW_ARROW_WIDTH       = MANEUVER_ARROW_MANEUVER_WIDTH * 2;
    const float MANEUVER_ARROW_ARROW_LENGTH      = MANEUVER_ARROW_ARROW_WIDTH;

    QColor MANEUVER_COLOR(0,0,0);

    locationtoolkit::CapParameter mStartCap;
    mStartCap.type = locationtoolkit::CPT_Round;
    mStartCap.radius = MANEUVER_ARROW_MANEUVER_WIDTH /2;
    mStartCap.width = 0;
    mStartCap.length = 0;
    locationtoolkit::CapParameter mEndCap;
    mEndCap.type = locationtoolkit::CPT_Arrow;
    mEndCap.radius = MANEUVER_ARROW_MANEUVER_WIDTH /2;
    mEndCap.width = MANEUVER_ARROW_ARROW_WIDTH;
    mEndCap.length = MANEUVER_ARROW_ARROW_LENGTH;

    QList<locationtoolkit::Coordinates> polyPoints;
    QList<locationtoolkit::SegmentAttribute*> segAttrs;
    for( int i = 0; i < pts.size(); i++ )
    {
        polyPoints.append(pts[i]);
        locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(pts.size()-1,
                                                                                   QColor(MANEUVER_COLOR));
        segAttrs.append( segattr );
    }

    locationtoolkit::PolylineParameters para;
    para.SetPoints(polyPoints);
    para.SetSegmentAttributes(segAttrs);
    para.SetUnhighlightColor(QColor(MANEUVER_COLOR));
    para.SetWidth(MANEUVER_ARROW_MANEUVER_WIDTH);
    para.SetStartCap(mStartCap);
    para.SetEndCap(mEndCap);
    para.SetAttributeType(locationtoolkit::PolylineParameters::SAT_ColorSegment);
    para.SetZOrder(255);
    locationtoolkit::Polyline& polylineObj = globalMapWidget->AddPolyline(para);
    mManeuverArrows.push_back(&polylineObj);


    //to get maneuverArrow in cluster
    QJsonObject jPolyline;
    QJsonArray jCoordinates;

    for(int i = 0;i<pts.count();i++)
    {
        QJsonObject ob;
        Coordinates val = pts.at(i);
        ob.insert("lattitude", QString::number(val.getLatitude(),'f',6));
        ob.insert("longitude", QString::number(val.getLongitude(),'f',6));
        jCoordinates.append(ob);
    }
    jPolyline.insert("coordinates",jCoordinates);
    QJsonDocument doc(jPolyline);
    QString dataToString(doc.toJson(QJsonDocument::Compact));
    setManeuverArrow(dataToString);
    //

    for(int i = 0; i < segAttrs.size(); ++i)
    {
        if(segAttrs[i])
        {
            delete segAttrs[i];
            segAttrs[i] = NULL;
        }
    }

    segAttrs.clear();
}

void MapViewController::addRoute()
{
    //    double pts[] = {-118.25,34.05,
    //                    -118.24,34.05,
    //                    -118.23,34.15};
    //    int n = sizeof(pts)/sizeof(double)/2;
    //    for (int i = 0; i < n; ++i)
    //    {
    //        locationtoolkit::Coordinates c(pts[i * 2 + 1], pts[i * 2]);
    //        mPolyPoints.push_back(c);
    //    }

    if (mLastPolyline)
    {
        globalMapWidget->RemovePolyline(mLastPolyline);
        mLastPolyline = NULL;
    }

    if (mRoutePolyPoints.size() == 0)
    {
        return;
    }

    int polylineWidth = 15;
    locationtoolkit::CapParameter mStartCap;
    mStartCap.type = locationtoolkit::CPT_Nothing;
    mStartCap.radius = polylineWidth /2;
    mStartCap.width = 0;
    mStartCap.length = 0;
    locationtoolkit::CapParameter mEndCap;
    mEndCap.type = locationtoolkit::CPT_Nothing;
    mEndCap.radius = polylineWidth /2;
    mEndCap.width = 0;
    mEndCap.length = 0;
    locationtoolkit::PolylineParameters para;

    para.SetPoints(mRoutePolyPoints);

    QList<locationtoolkit::SegmentAttribute*>* mSegAttr = new QList<locationtoolkit::SegmentAttribute*>();;
    locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(mRoutePolyPoints.count()-1, QColor(Qt::black));
    mSegAttr->append(segattr);
    para.SetSegmentAttributes(*mSegAttr);
    para.SetUnhighlightColor(QColor(0,50,90));
    para.SetWidth(polylineWidth);
    para.SetZOrder(16);
    para.SetVisible(true);
    para.SetStartCap(mStartCap);
    para.SetEndCap(mEndCap);
    para.setOutlineColor(QColor(Qt::black));
    para.setOutlineWidth(0);
    para.SetAttributeType(locationtoolkit::PolylineParameters::SAT_ColorSegment);

    mLastPolyline = &globalMapWidget->AddPolyline(para);

    globalMapWidget->DeleteAllPins();
    addPin(mRoutePolyPoints.first());
    addPin(mRoutePolyPoints.last());
}

void MapViewController::addPin(const locationtoolkit::Coordinates &coordinate)
{
    locationtoolkit::CameraParameters param( coordinate );
    QPixmap selectImage, unselectImage;

    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    selectImage.load(resourceFolder + "images/pin_blue.png");
    unselectImage.load(resourceFolder + "images/pin_normal.png");

    locationtoolkit::PinImageInfo selectedImage;
    selectedImage.SetPixmap(selectImage);
    selectedImage.SetPinAnchor(50, 100);
    locationtoolkit::PinImageInfo unSelectedImage;
    unSelectedImage.SetPixmap(unselectImage);
    unSelectedImage.SetPinAnchor(50, 100);

    locationtoolkit::RadiusParameters radiusPara(0, 0x6721D826);

    QString title("Title");
    QString subtitle("Sub Title");
    locationtoolkit::Bubble* bubble = NULL;
    bool visible = true;

    locationtoolkit::PinParameters pinpara(coordinate,
                                           selectedImage,
                                           unSelectedImage,
                                           radiusPara,
                                           title,
                                           subtitle,
                                           bubble,
                                           visible);
    globalMapWidget->CreatePin(pinpara);
}

void MapViewController::onTimer()
{

    //qDebug()<<"MapViewController::onTimer"<<mLastLocation.latitude<<"::"<<mLastLocation.longitude;
    locationtoolkit::Location pos( mLastLocation );
    pos.latitude = mLatInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.longitude = mLonInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.heading = mHeadingInterpolator.GetAngleValue( mInterpolatorTime.elapsed() );
    if(autoNavMode)
    {
        updateAvatarLocation( pos );
    }
}

//void MapViewController::ProviderStateChanged( LocationProviderState state )
//{

//}

//void MapViewController::OnLocationError(int errorCode)
//{
//    qDebug()<<"MapViewController::OnLocationError::"<<errorCode;
//    mPolylineReady = true;
//}

QSGNode* MapViewController::updatePaintNode(QSGNode *n, UpdatePaintNodeData *d)
{
    qDebug("MapViewController::updatePaintNode Width : %lf Height : %lf", width(), height());

    if (!mInitDone)
    {
        QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    }
    return QQuickItem::updatePaintNode(n, d);
}

void MapViewController::init()
{
    if(mInitDone)
        return;

    qDebug("MapViewController::init\n");

    //    GlobalSetting::InitInstance();
    //    GlobalSetting& settings = GlobalSetting::GetInstance();
    //    locationtoolkit::LTKContext* ltkContext = GetLTKContext();
    //
    //    InitLTKHybridManager(true, "/opt/comtech/data/mapdata");

    globalMapWidget = GetMapWidget();
    if( globalMapWidget == NULL )
    {
        qDebug( "create mapkit3d widget failed!!!\n" );
        return;
    }
    globalMapWidget->setParentItem(this);
    globalMapWidget->setSize(QSize(width(),height()));

    //connect(globalMapWidget, &locationtoolkit::MapWidget::MapCreated, this, &MapViewController::onMapReady);

    //    mLocationConfiguration.emulationMode = true;
    //    mLocationConfiguration.locationFilename = GPS_FILE;
    //    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( mLocationConfiguration );
    //    locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );

    // Temporarily commented
    //    startNavigation();
    mLTKMapKit = globalMapWidget;
    mLTKContext = GetLTKContext();

    SetupMapWidget();
    InitializeNavigationHeaderBar();
    SetUpNavigationRouteController();
    setupManeuverDetailSignalAndSlots();
    //setupManeuverListSignalAndSlots();
    setUp_sideBarItems();
    navigationButtons();

    //startWeatherService();

    mInitDone = true;
    mSettingFileName = LTKSampleUtil::GetResourceFolder()+"sampleapp.ini";

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                        "To start it, run:\n"
                        "\teval `dbus-launch --auto-syntax`\n");
        return;
    }
    if(mClient=="Automotive"){
        qDebug("SessionBus Connected MapViewController\n");

        if (!QDBusConnection::sessionBus().registerService(SERVICE_CLUSTER_CONSOLE)) {
            qDebug("SessionBus registerService failed %d ::",QDBusConnection::sessionBus().lastError().type()/*.message()*/);

            fprintf(stderr, "%s\n",
                    qPrintable(QDBusConnection::sessionBus().lastError().message()));
            exit(1);
        }
        qDebug("SessionBus registerService MapViewController\n");


        qDebug("SessionBus registered service MapViewController \n");

        bool registerflag = QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);

        qDebug("SessionBus registered MapViewController \n :: %d",registerflag);
    }
}

void MapViewController::setWorkFolder(QString path)
{
    qDebug("MapViewController::SetWorkFolder %s", path.toStdString().c_str());
    LTKSampleUtil::SetWorkFolder(path);
}

QString MapViewController::workFolder()
{
    return LTKSampleUtil::GetWorkFolder();
}

void MapViewController::onMapReady()
{
    qDebug()<<"MapViewController::onMapReady follow me true";
    mMapReady = true;

    if (mIsFollowMe)
    {
        qDebug()<<"MapViewController::onMapReady follow me true";
        globalMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME,false);
    }
    globalMapWidget->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));
    // updateAvatarLocation();

    locationtoolkit::Coordinates pos;
    locationtoolkit::CameraParameters cp(pos);
    globalMapWidget->GetCameraPosition(cp);
    cp.SetPosition(pos);
    cp.SetTiltAngle(mTilt);
    cp.SetZoomLevel(mZoom);
    globalMapWidget->MoveTo(cp);

    globalMapWidget->SetNightMode(locationtoolkit::MapWidget::NM_NIGHT);
    connect(globalMapWidget, SIGNAL(Unlocked()), this, SLOT(onUnlock()));
}

void MapViewController::onUnlock()
{
    qDebug()<<"MapViewController::onUnlock";
    globalMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY,false);
    mMapCameraLocked = false;
    if(isFollowMe())
    {
        if(!mMapCameraLockButton->isVisible())
        {
            mMapCameraLockButton->setVisible(true);
            setMapCameraLockButtonPosition(mIsNavigationStarted);

        }
    }

    //emit showFollowMeOption();
}

void MapViewController::onlock()
{
    globalMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME,false);
}

void MapViewController::onCameraUpdated(const CameraParameters &cameraParameters)
{
    mMapCenter = cameraParameters.GetPosition();
    //qDebug()<<"MapViewController::onCameraUpdated "<<mMapCenter.latitude<<" "<<mMapCenter.longitude;
}

void MapViewController::updateAvatarLocation(const locationtoolkit::Location& location)
{
    if (mMapReady)
    {
        locationtoolkit::Location loc;
        loc.latitude = location.latitude;
        loc.longitude = location.longitude;
        loc.heading = location.heading;
        loc.valid = 1 | 2 | 8;
        globalMapWidget->GetAvatar().SetLocation(loc);

        QString lat, lon, heading;

        lat.setNum(loc.latitude, 'g', 16);
        lon.setNum(loc.longitude, 'g', 16);
        heading.setNum(loc.heading);
        
        setPosition(lat, lon, heading);

        //	Maneuver m = maneuverList.at(mManeuverIndex);

        //        if(getDistanceFromLatLonInKm(loc.latitude, loc.longitude, m.lat, m.lon) <= 0.2) {

        //		qDebug("getDistanceFromLatLonInKm <= 50");
        //		mManeuverIndex++;
        //                if(mManeuverIndex == maneuverList.size()) {
        //		    mManeuverIndex = 0;
        //		    m = maneuverList.at(mManeuverIndex);
        //		}

        //		qDebug("MapViewConsole::SetManeuver %s %s %f", m.routingTTF.toStdString().c_str(), m.primaryStreet.toStdString().c_str(), m.distance);

        //                QString distance = FormatDistance(m.distance, false, false);
        //	   	setManeuver(m.routingTTF, m.primaryStreet, distance, QString("Slow traffic"));
        //	}
        //        else {
        //                if(mManeuverFirsttime == false) {

        //                    qDebug("setManeuver First time");
        //                    mManeuverFirsttime = true;
        //                    qDebug("MapViewConsole::SetManeuver %s %s %f", m.routingTTF.toStdString().c_str(), m.primaryStreet.toStdString().c_str(), m.distance);
        //                    QString distance = FormatDistance(m.distance, false, false);

        //                    setManeuver(m.routingTTF, m.primaryStreet, distance, QString("Slow traffic"));
        //                    mManeuverIndex++;
        //                }
        //        }
    }
}

QPointF MapViewController::avatarPosition() const
{
    return mAvatarPosition;
}

void MapViewController::setAvatarPosition(QPointF p)
{
    mAvatarPosition = p;
    //updateAvatarLocation();
}

float MapViewController::avatarHeading() const
{
    return mAvatarHeading;
}

void MapViewController::setAvatarHeading(float t)
{
    mAvatarHeading = t;
    //updateAvatarLocation();
}

float MapViewController::tilt()
{
    return mTilt;
}

void MapViewController::setTilt(float t)
{
    mTilt = t;
}

float MapViewController::zoom()
{
    return mZoom;
}

void MapViewController::setZoom(float t)
{
    mZoom = t;
}

bool MapViewController::isFollowMe()
{
    return mIsFollowMe;
}

void MapViewController::setIsFollowMe(bool v)
{
    mIsFollowMe = v;
}

bool MapViewController::setManeuver(const QString& maneuverIcon, const QString& streetName, const QString& distance, const QString& trafficWarning)
{
    qDebug("MapViewController::setManeuver maneuver : %s streetName : %s distance : %s trafficWarning : %s \n",
           maneuverIcon.toStdString().c_str(), streetName.toStdString().c_str(),
           distance.toStdString().c_str(), trafficWarning.toStdString().c_str());

    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("onManeuverUpdated", maneuverIcon, streetName, distance, trafficWarning);
        //if (reply.isValid()) {
        //    printf("onManeuverUpdated Reply was: %s\n", qPrintable(reply.value()));
        //    return true;
        //}

        //fprintf(stderr, "Call onManeuverUpdated failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("onManeuverUpdated", maneuverIcon, streetName, distance, trafficWarning);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));

    }
    else {
        qDebug("MapViewController::setManeuver interface invalid");
    }

    return true;
}

bool MapViewController::updateSpeedLimit(const QString& speedLimit)
{
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("onUpdateSpeedLimit", speedLimit);
        //if (reply.isValid()) {
        //    printf("onupdateSpeedLimit Reply was: %s\n", qPrintable(reply.value()));

        //    return true;
        //}

        //fprintf(stderr, "Call onUpdateSpeedLimit failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("onUpdateSpeedLimit", speedLimit);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
    else {
        qDebug("MapViewController::updateSpeedLimit interface invalid");
    }

    return true;
}


bool MapViewController::setPosition(const QString& lat, const QString& lon, const QString& heading)
{
    // qDebug("MapViewController::setPosition lat : %s lon : %s heading : %s", lat.toStdString().c_str(), lon.toStdString().c_str(), heading.toStdString().c_str());

    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("updateGpsPosition", lat, lon, heading);
        //if (reply.isValid()) {
        //    //printf("updateGpsPosition Reply was: %s\n", qPrintable(reply.value()));
        //    return true;
        //}

        //fprintf(stderr, "Call updateGpsPosition failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("updateGpsPosition", lat, lon, heading);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));

    }
    else {
        qDebug("MapViewController::setPosition interface invalid");
    }

    return true;
}

void MapViewController::setNavigationMode(bool navMode) 
{
    qDebug("MapViewController::setNavigationMode %d", navMode);
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if(ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("setNavigationMode", navMode);
        //if(reply.isValid()) {
        //}
        //else {
        //    qDebug("call setNavigationMode");
        //}

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("setNavigationMode", navMode);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
}

bool MapViewController::setRemainingManeuverDistance(const QString& distance)
{
    // qDebug("MapViewController::setPosition lat : %s lon : %s heading : %s", lat.toStdString().c_str(), lon.toStdString().c_str(), heading.toStdString().c_str());

    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());
    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("updateRemainingManeuverDistance", distance);
        //if (reply.isValid()) {
        //printf("updateRemainingManeuverDistance Reply was: %s\n", qPrintable(reply.value()));
        //    return true;
        //}

        //fprintf(stderr, "Call updateRemainingManeuverDistance failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("updateRemainingManeuverDistance", distance);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
    else {
        qDebug("MapViewController::setRemainingManeuverDistance interface invalid");
    }

    return true;
}

bool MapViewController::setPolyline(const QString &polyline)
{
    qDebug()<<"MapViewController::setPolyline"<<polyline;
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("updatePolyline", polyline);
        //if (reply.isValid()) {
        //    return true;
        //}

        //fprintf(stderr, "Call updatePolyline failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("updatePolyline", polyline);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));

    }
    else {
        qDebug("MapViewController::updatePolyline interface invalid");
    }

    return true;
}
bool MapViewController::clearPolyline()
{
    qDebug()<<"MapViewController::clearPolyline";
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("clearPolyline");
        //if (reply.isValid()) {
        //    return true;
        //}

        //fprintf(stderr, "Call clearPolyline failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("clearPolyline");

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
    else {
        qDebug("MapViewController::clearPolyline interface invalid");
    }

    return true;
}

bool MapViewController::setManeuverArrow(const QString &pts)
{
    qDebug()<<"MapViewController::setManeuverArrow";
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if (ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("updateManeuverArrow",pts);
        //if (reply.isValid()) {
        //    return true;
        //}

        //fprintf(stderr, "Call updateManeuverArrow failed: %s\n", qPrintable(reply.error().message()));
        //return false;

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("updateManeuverArrow", pts);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));

    }
    else {
        qDebug("MapViewController::updateManeuverArrow interface invalid");
    }

    return true;
}
void MapViewController::navigationButtons() {

    qDebug()<<"MapViewController::navigationButtons";
    // Add button to lock camera when navigation is running
    QQmlEngine *engine = QQmlEngine::contextForObject(this->parentItem())->engine();

    QString mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    //QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/ManeuverDetail.qml"));

    QQmlComponent componentCameraLockButton(engine,QUrl(mAppDirPath+"/resource/MapCameraLockButton.qml"));
    mMapCameraLockButton = qobject_cast<QQuickItem*>(componentCameraLockButton.create());
    connect( mMapCameraLockButton, SIGNAL(cameraLockButtonClicked()), this, SLOT(onCameraLockButtonClicked()) );
    connect( mMapCameraLockButton, SIGNAL(navigationMapOverview()), this, SLOT(onNavigationMapOverview()) );
    QQmlEngine::setObjectOwnership(mMapCameraLockButton, QQmlEngine::CppOwnership);
    mMapCameraLockButton->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mMapCameraLockButton, QQmlEngine::contextForObject(this->parentItem()));
    //mMapCameraLockButton->setParent(this->parent());
    //mMapCameraLockButton->setPosition(QPoint(mManeuverDetailsController->x() + mManeuverDetailsController->width() + 20, mManeuverDetailsController->y() + mManeuverDetailsController->height() - 70));
    //mMapCameraLockButton->setPosition(QPoint(390,520));//temp fix
    int y = mManeuverDetailsController->position().y() + mManeuverDetailsController->height() - 65;
    int x = mManeuverDetailsController->position().x() + mManeuverDetailsController->width() + 20;
    //mMapCameraLockButton->setPosition(QPoint(x,y));
    mMapCameraLockButton->setPosition(QPoint(this->width() - 90, this->height()/1.2/*1.3*/));

    mMapCameraLockButton->setSize(QSize(70, 70));
    mMapCameraLockButton->setVisible(false);
    connect( this, SIGNAL(resetMapCameraButton()),mMapCameraLockButton,SIGNAL(resetButton()));
    connect(mLTKMapKit, SIGNAL(Unlocked()), mMapCameraLockButton, SIGNAL(panView()));

    // Add button to switch to route overview view
    //    QQmlComponent componentRouteOverviewButton(engine,QUrl(mAppDirPath+"/resource/RouteOverviewButton.qml"));
    //    mRouteOverviewButton = qobject_cast<QQuickItem*>(componentRouteOverviewButton.create());
    //    connect( mRouteOverviewButton, SIGNAL(navigationMapOverview()), this, SLOT(onNavigationMapOverview()) );
    //    QQmlEngine::setObjectOwnership(mRouteOverviewButton, QQmlEngine::CppOwnership);
    //    mRouteOverviewButton->setParentItem(this->parentItem());
    //    //mRouteOverviewButton->setParent(this->parent());
    //    //mRouteOverviewButton->setPosition(QPoint(mManeuverDetailsConroller->x() + mManeuverDetailsConrollerConroller->width() + 20, mMapCameraLockButton->y() - mMapCameraLockButton->height() - 20));
    //    mRouteOverviewButton->setPosition(QPoint(390,420));
    //    mRouteOverviewButton->setSize(QSize(70, 70));
    //    mRouteOverviewButton->setVisible(false);

    // Add SpeedLimitView to show speed limit alerts
    QQmlComponent componentSpeedLimitView(engine,QUrl(mAppDirPath+"/resource/SpeedLimitView.qml"));
    mSpeedLimitView = qobject_cast<QQuickItem*>(componentSpeedLimitView.create());
    QQmlEngine::setObjectOwnership(mSpeedLimitView, QQmlEngine::CppOwnership);
    mSpeedLimitView->setParentItem(this->parentItem());
    //mSpeedLimitView->setParent(this);
    //mSpeedLimitView->setPosition(QPoint(this->width() - this->height()/8, this->height()/3));
    int posY = mLayerOptionsButton->height() + mLayerOptionsButton->position().y() + 10;
    mSpeedLimitView->setPosition(QPoint(this->width() - 90, posY));
    mSpeedLimitView->setSize(QSize(72, 90));
    mSpeedLimitView->setVisible(false);

    // Add traffic widget to show traffic alerts
    QQmlComponent componentTrafficViewController(engine,QUrl(mAppDirPath+"/resource/TrafficViewController.qml"));
    mTrafficViewController = qobject_cast<QQuickItem*>(componentTrafficViewController.create());
    QQmlEngine::setObjectOwnership(mTrafficViewController, QQmlEngine::CppOwnership);
    mTrafficViewController->setParentItem(this->parentItem());
    //mTrafficViewController->setParent(this);
    int xPos = mManeuverDetailsController->x() + mManeuverDetailsController->width() + 40;
    mTrafficViewController->setPosition(QPoint(xPos, mManeuverDetailsController->y()));
    mTrafficViewController->setSize(QSize(this->width() - xPos - 100, mManeuverDetailsController->height()/4.5));
    mTrafficViewController->setVisible(false);

    // Add roadsign view
    QQmlComponent componentRoadSignViewController(engine,QUrl(mAppDirPath+"/resource/RoadSignViewController.qml"));
    mRoadSignViewController = qobject_cast<QQuickItem*>(componentRoadSignViewController.create());
    QQmlEngine::setObjectOwnership(mRoadSignViewController, QQmlEngine::CppOwnership);
    mRoadSignViewController->setParentItem(this->parentItem());
    //mRoadSignViewController->setParent(this);
    mRoadSignViewController->setPosition(QPoint(xPos, mManeuverDetailsController->y() + mTrafficViewController->height() + 10));
    mRoadSignViewController->setSize(QSize(this->width() - xPos - 100, mManeuverDetailsController->height()/3));
    mRoadSignViewController->setVisible(false);

}
void MapViewController::onCameraLockButtonClicked() {

    qDebug()<<" MapViewController::onCameraLockButtonClicked";
    mMapCameraLocked = true;
    mLTKMapKit->SetGpsMode(MapWidget::GM_FOLLOW_ME,false);
    if(!mIsNavigationStarted)
    {
        onUpdateCameraParam();
        mMapCameraLockButton->setVisible(false);
    }

}
void MapViewController::onNavigationMapOverview() {

    qDebug()<<" MapViewController::onNavigationMapOverview";
    mMapCameraLocked = false;
    mLTKMapKit->SetGpsMode(MapWidget::GM_STANDBY,false);

    if (!mNavSession->mRouteInformationList.isEmpty()) {

        QSharedPointer<RouteInformation> routeInfo = mNavSession->mRouteInformationList.at(0);

        //  create bounding box for showing route info
        BoundingBox box = routeInfo->GetBoundingBox();
        LatLngBound* latlongBounds = new LatLngBound(box.point1.latitude,box.point1.longitude,box.point2.latitude,box.point2.longitude);

        //  zoom map to bounding box
        mLTKMapKit->ZoomToBoundingBox(latlongBounds->topLeftLatitude,latlongBounds->topLeftLongitude,latlongBounds->bottomRightLatitude,latlongBounds->bottomRightLongitude);

        //  move map to show right part of screen as we have to show route info to left part
        double* currentLat = new double();
        double* currentLon = new double();
        //mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2,this->height()/2,*currentLat,*currentLon);
        mLTKMapKit->GetMapProjection().FromScreenPosition(this->width(),this->height() - 200 ,*currentLat,*currentLon);//temp fix
        Coordinates *newCoordinates = new Coordinates();
        newCoordinates->latitude = *currentLat;
        newCoordinates->longitude = *currentLon;

        QPoint newPoint = QPoint((this->width()+mNavigationRoutesController->width())/2,this->height()/2);

        CameraParameters *currentCamPara = new CameraParameters(box.point1);
        mLTKMapKit->GetCameraPosition(*currentCamPara);
        //  Setup camera position, view and zoom/tilt
        QSharedPointer<CameraParameters> q_param = CameraFactory::CreateCameraParametersForCoordinatesAtPosition(newPoint,
                                                                                                                 this->width(),
                                                                                                                 this->height(),
                                                                                                                 *newCoordinates,
                                                                                                                 currentCamPara->GetTiltAngle(),
                                                                                                                 currentCamPara->GetZoomLevel() - 0.5,
                                                                                                                 currentCamPara->GetHeadingAngle());
        CameraParameters* newCamPara = static_cast<CameraParameters*>(q_param.data());
        mLTKMapKit->MoveTo(*newCamPara);
    }

}

void MapViewController::setUp_sideBarItems() {

    QQmlEngine *engine = QQmlEngine::contextForObject(this->parentItem())->engine();
    QString mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();

    QQmlComponent componentZoomInButton(engine, QUrl(mAppDirPath+"/resource/ZoomInButton.qml"));
    mZoomInButton = qobject_cast<QQuickItem*>(componentZoomInButton.create());
    connect( mZoomInButton, SIGNAL(mapZoomIn()), this, SLOT(mapZoomIn()) );
    QQmlEngine::setObjectOwnership(mZoomInButton, QQmlEngine::CppOwnership);
    mZoomInButton->setParentItem(this->parentItem());
    // mZoomInButton->setParent(this);
    //mZoomInButton->setPosition(QPoint(this->width() - this->height()/8, this->height()/2.5));//temp fix
    mZoomInButton->setPosition(QPoint(this->width() - 90, this->height()/2.5));

    mZoomInButton->setSize(QSize(70, 70));
    mZoomInButton->setVisible(false);

    QQmlComponent componentZoomOutButton(engine, QUrl(mAppDirPath+"/resource/ZoomOutButton.qml"));
    mZoomOutButton = qobject_cast<QQuickItem*>(componentZoomOutButton.create());
    connect( mZoomOutButton, SIGNAL(mapZoomOut()), this, SLOT(mapZoomOut()) );
    QQmlEngine::setObjectOwnership(mZoomOutButton, QQmlEngine::CppOwnership);
    mZoomOutButton->setParentItem(this->parentItem());
    //mZoomOutButton->setParent(this);
    mZoomOutButton->setPosition(QPoint(this->width() - 90, mZoomInButton->y() + mZoomInButton->height() + 10));
    mZoomOutButton->setSize(QSize(70, 70));
    mZoomOutButton->setVisible(false);

    QQmlComponent componentLayerOptionsButton(engine, QUrl(mAppDirPath+"/resource/LayerOptionsButton.qml"));
    mLayerOptionsButton = qobject_cast<QQuickItem*>(componentLayerOptionsButton.create());
    connect( mLayerOptionsButton, SIGNAL(mapLayerOptions()), this, SLOT(mapLayerOptions()) );
    QQmlEngine::setObjectOwnership(mLayerOptionsButton, QQmlEngine::CppOwnership);
    mLayerOptionsButton->setParentItem(this->parentItem());
    //mLayerOptionsButton->setParent(this);
    mLayerOptionsButton->setPosition(QPoint(this->width() - 90, mZoomOutButton->y() + mZoomOutButton->height() + 10));
    mLayerOptionsButton->setSize(QSize(70, 70));
    mLayerOptionsButton->setVisible(false);

    QQmlComponent componentLayerOptionWidget(engine, QUrl(mAppDirPath+"/resource/LayerOption.qml"));
    mLayerOptionWidget = qobject_cast<QQuickItem*>(componentLayerOptionWidget.create());
    connect( mLayerOptionWidget, SIGNAL(viewOptionClicked(bool)), this, SLOT(onToggleMap3DView(bool)) );
    connect( mLayerOptionWidget, SIGNAL(landmarkBuildingsClicked(bool)), this, SLOT(onLandmarkBuildingsClicked(bool)) );
    connect( mLayerOptionWidget, SIGNAL(poiClicked(bool)), this, SLOT(onPoiClicked(bool)) );
    connect( mLayerOptionWidget, SIGNAL(satelliteClicked(bool)), this, SLOT(onSatelliteClicked(bool)) );
    connect( mLayerOptionWidget, SIGNAL(trafficClicked(bool)), this, SLOT(onTrafficClicked(bool)) );
    connect( mLayerOptionWidget, SIGNAL(layerOptionWidgetClosed()), this, SLOT(onLayerOptionWidgetClosed()) );

    QQmlEngine::setObjectOwnership(mLayerOptionWidget, QQmlEngine::CppOwnership);
    mLayerOptionWidget->setParentItem(this->parentItem());
    //mLayerOptionWidget->setParent(this);
    mLayerOptionWidget->setPosition(QPoint(0, 0));
    mLayerOptionWidget->setSize(QSize(this->width(),this->height()));
    mLayerOptionWidget->setVisible(false);

    const QList<locationtoolkit::MapWidget::LayerNameAndEnabled> layers = mLTKMapKit->GetLayerNameAndStates();
    qDebug()<<"layers.size()"<<layers.size();
    for( int i = 0; i < layers.size(); i++ )
    {
        //mToolBar->AddLayerList( layers[i].name, layers[i].enabled );
        qDebug()<<"layers.size()"<<layers.size();
        qDebug()<<"layers.name()"<<layers[i].name<<layers[i].enabled;


        if(layers[i].name == "Points of Interest")
        {
            QObject *object1 = mLayerOptionWidget->findChild<QObject *>("chkPOI");
            object1->setProperty("checked", layers[i].enabled);
        }

        if(layers[i].name == "Traffic")
        {
            QObject *object2 = mLayerOptionWidget->findChild<QObject *>("chkTraffic");
            object2->setProperty("checked", layers[i].enabled);
        }

        if(layers[i].name =="3D View")
        {
            QObject *object3 = mLayerOptionWidget->findChild<QObject *>("chk3DView");
            object3->setProperty("checked", layers[i].enabled);
        }

        if(layers[i].name =="Landmarks & Buildings")
        {
            QObject *object4 = mLayerOptionWidget->findChild<QObject *>("chk3DLandMarks");
            object4->setProperty("checked", layers[i].enabled);
        }

        if(layers[i].name =="Satellite")
        {
            QObject *object5 = mLayerOptionWidget->findChild<QObject *>("chkSatellite");
            object5->setProperty("checked", layers[i].enabled);
        }

    }
    // QQmlComponent componentWeather(engine, QUrl(mAppDirPath+"/resource/Weather.qml"));
    // mWeather = qobject_cast<QQuickItem*>(componentWeather.create());
    // QQmlEngine::setObjectOwnership(mWeather, QQmlEngine::CppOwnership);
    // mWeather->setParentItem(this->parentItem());
    // mWeather->setParent(this);
    //mWeather->setPosition(QPoint(this->width() - 240, this->height() - 220));
    // mWeather->setPosition(QPoint(0, 0));
    // mWeather->setSize(QSize(220, 120));
    // mWeather->setVisible(false);

    QQmlComponent componentAlertBox(engine, QUrl(mAppDirPath+"/resource/AlertMessageBox.qml"));
    mAlertBox = qobject_cast<QQuickItem*>(componentAlertBox.create());
    connect( mAlertBox, SIGNAL(mapLayerOptions()), this, SLOT(mapLayerOptions()) );
    QQmlEngine::setObjectOwnership(mAlertBox, QQmlEngine::CppOwnership);
    mAlertBox->setParentItem(this->parentItem());
    //mLayerOptionsButton->setParent(this);
    engine->rootContext()->setContextProperty("alertTextMsg", "Time to fill gas, go to nearest Fuel Station");

    mAlertBox->setPosition(QPoint(this->width(), this->height()/2/*mLayerOptionsButton->y() + mLayerOptionsButton->height() + 45*/));

    mAlertBox->setVisible(false);
    connect( mAlertBox, SIGNAL(alertBoxValueClicked(bool)), this, SLOT(onAlertBoxValueClicked(bool)) );


}
void MapViewController::setFuelAlertVisibility(/*bool flag*/){
    // D-Bus call
    QDBusInterface ifaceToCluster(SERVICE_CONSOLE_CLUSTER, "/", "", QDBusConnection::sessionBus());

    if(ifaceToCluster.isValid()) {
        //QDBusReply<QString> reply = iface.call("hideFuelAlert", false,mlowFuelMode);
        //if(reply.isValid()) {
        //}
        //else {
        //    qDebug("call hideFuelAlert");
        //}

        QDBusPendingCall pcall = ifaceToCluster.asyncCall("hideFuelAlert", false, mlowFuelMode);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
}

void MapViewController::onAlertBoxValueClicked(bool flag){

    qDebug() << "MapViewController::onAlertBoxValueClicked :: " << flag;
    if(flag){
        //call slot to further functions
        fuelAlertAcceptClicked();
        setFuelAlertVisibility();
    }
    else{
        mAlertBox->setVisible(false);
    }
    qDebug() << "mlowFuelMode ::" << mlowFuelMode;
}

void MapViewController::fuelAlertAcceptClicked(){

    qDebug() << "MapViewController::fuelAlertAcceptClicked";
    mAlertBox->setVisible(false);
    mlowFuelMode = true;
    if(mIsNavigationStarted)
        mInNavMode = true;
    QJsonObject categoryParameter;
    categoryParameter["code"] = "ACC";
    categoryParameter["name"] = "Fuel Stations";
    categoryParameter["lattitude"] = QString::number(mLastLocation.latitude,'f',6);
    categoryParameter["longitude"] = QString::number(mLastLocation.longitude,'f',6);
    QVariant parameter = categoryParameter;
    mUserDestPoint = mNavDestPoint;
    mUserDestName = mNaveDestName;
    emit fuelSearchRequest(parameter);
}

QString MapViewController::showFuelAlert(bool Mode)
{
    qDebug() << "MapViewController::showFuelAlert :: " << Mode;

    mAlertBox->setVisible(Mode);

    return QString("showFuelAlert got called");
}

void MapViewController::onToggleMap3DView(bool b) {

    if (b) {
        double latitude, longitude;
        //mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2, this->height()/2, latitude, longitude);
        mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2, (this->height()*2)/3, latitude, longitude);
        Coordinates coor(latitude, longitude);
        CameraParameters param(coor);
        param.SetTiltAngle(45);
        locationtoolkit::AnimationParameters animParam( locationtoolkit::AnimationParameters::AC_Deceleration, 1000 );
        mLTKMapKit->AnimateTo(param,animParam);
    }
    else {
        double latitude, longitude;
        //mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2, this->height()/2, latitude, longitude);
        mLTKMapKit->GetMapProjection().FromScreenPosition(this->width()/2, (this->height()*2)/3, latitude, longitude);
        Coordinates coor(latitude, longitude);
        CameraParameters param(coor);
        param.SetTiltAngle(mTilt);
        locationtoolkit::AnimationParameters animParam( locationtoolkit::AnimationParameters::AC_Deceleration, 1000 );
        mLTKMapKit->AnimateTo(param,animParam);
    }

}

void MapViewController::onLandmarkBuildingsClicked(bool isChecked) {

    mLTKMapKit->ShowOptionalLayer( "3D Landmarks & Buildings", isChecked );

}

void MapViewController::onPoiClicked(bool isChecked) {

    qDebug()<<"on poi clicked"<<isChecked;
    mLTKMapKit->ShowOptionalLayer( "Points of Interest", isChecked );

}

void MapViewController::onSatelliteClicked(bool isChecked) {

    mLTKMapKit->ShowOptionalLayer( "Satellite", isChecked );

}

void MapViewController::onTrafficClicked(bool isChecked) {

    //    mLTKMapKit->ShowOptionalLayer( "Traffic", isChecked );
    mLTKMapKit->ShowTrafficLayer(isChecked);

}

void MapViewController::onLayerOptionWidgetClosed() {

    mLayerOptionWidget->setVisible(false);

}
void MapViewController::mapZoomIn() {

    //    int x = mLTKMapKit->rect().width() / 2;
    //    int y = mLTKMapKit->rect().height() / 2;

    double d = 0.0;
    locationtoolkit::Coordinates coordin( d, d );
    locationtoolkit::CameraParameters param( coordin );
    mLTKMapKit->GetCameraPosition( param );

    const locationtoolkit::MapProjection& projection = mLTKMapKit->GetMapProjection();

    param.SetZoomLevel( param.GetZoomLevel() + 1 );

    bool isAnimate = true;
    if( isAnimate == true ) {
        locationtoolkit::AnimationParameters aniParam(locationtoolkit::AnimationParameters::AC_Deceleration, 1000);
        mLTKMapKit->AnimateTo( param, aniParam );
    }
    else {
        mLTKMapKit->MoveTo( param );
    }
}

void MapViewController::mapZoomOut() {

    //    int x = mLTKMapKit->rect().width() / 2;
    //    int y = mLTKMapKit->rect().height() / 2;

    double d = 0.0;
    locationtoolkit::Coordinates coordin( d, d );
    locationtoolkit::CameraParameters param( coordin );
    mLTKMapKit->GetCameraPosition( param );

    const locationtoolkit::MapProjection& projection = mLTKMapKit->GetMapProjection();

    param.SetZoomLevel( param.GetZoomLevel() - 1 );

    bool isAnimate = true;
    if( isAnimate == true ) {
        locationtoolkit::AnimationParameters aniParam(locationtoolkit::AnimationParameters::AC_Deceleration, 1000);
        mLTKMapKit->AnimateTo( param, aniParam );
    }
    else {
        mLTKMapKit->MoveTo( param );
    }
}


void MapViewController::mapLayerOptions() {
    mLayerOptionWidget->setVisible(true);
    //    mLayerOptionWidget->show();
    //    this->OnLayerOptionButtonClicked();
}

void MapViewController::onChangedCentreCoordinate(QString gpsfile)
{
    mLTKMapKit->EnableReferenceCenter(true);
    mLTKMapKit->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));

    QString gpsPath = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + gpsfile;
    Place destination, origin;

    //Coordinates mapCenter = getMapCenter();
    GetDefaultDestinationAndOrigin(gpsPath.toStdString(), &destination, &origin);
    locationtoolkit::Coordinates referenceCenter;
    referenceCenter.latitude = origin.GetLocation().center.latitude;
    referenceCenter.longitude = origin.GetLocation().center.longitude;
    locationtoolkit::CameraParameters param(referenceCenter);
    param.SetPosition( referenceCenter );
    param.SetTiltAngle(mTilt);
    param.SetZoomLevel(mZoom);
    mLTKMapKit->MoveTo( param );
    mMapCenter = param.GetPosition();
    mReferenceCenter = origin;
    mLastLocation.latitude = origin.GetLocation().center.latitude;
    mLastLocation.longitude = origin.GetLocation().center.longitude;
    locationtoolkit::Location source;
    source.latitude = origin.GetLocation().center.latitude;
    source.longitude = origin.GetLocation().center.longitude;
    source.heading = 60.0;
    source.valid = 507;
    mLTKMapKit->GetAvatar().SetMode( locationtoolkit::Avatar::AM_ARROW );
    mLTKMapKit->GetAvatar().SetLocation(source);

    mNavSession->OnUpdateGpsFile(gpsPath);

    //mLTKMapKit->SetReferenceCenter(QPointF(origin.GetLocation().center.latitude,origin.GetLocation().center.longitude));
}
void MapViewController::onUpdateCameraParam()
{
    //    mLTKMapKit->EnableReferenceCenter(true);
    //    mLTKMapKit->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));

    //    if(isFollowMe())
    //    {
    //        mMapCameraLocked = true;
    //        mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME);
    //    }
    //    locationtoolkit::Location source;
    //    source.latitude = mReferenceCenter.GetLocation().center.latitude;
    //    source.longitude = mReferenceCenter.GetLocation().center.longitude;
    //    source.heading = 60.0;
    //    source.valid = 507;
    //    mLTKMapKit->GetAvatar().SetMode( locationtoolkit::Avatar::AM_ARROW );
    //    mLTKMapKit->GetAvatar().SetLocation(source);
    //    //updateAvatarLocation(source);

    //    locationtoolkit::Coordinates referenceCenter;
    //    referenceCenter.latitude = mReferenceCenter.GetLocation().center.latitude;
    //    referenceCenter.longitude = mReferenceCenter.GetLocation().center.longitude;
    //    locationtoolkit::CameraParameters param(referenceCenter);
    //    param.SetPosition( referenceCenter );
    //    param.SetTiltAngle(mTilt);
    //    param.SetZoomLevel(mZoom);
    //    mLTKMapKit->MoveTo( param );
    globalMapWidget->EnableReferenceCenter(true);
    globalMapWidget->SetReferenceCenter(QPoint(width()/2,height() * 2 / 3));
    //globalMapWidget->SetReferenceCenter(mReferenceCenter);
    locationtoolkit::Coordinates referenceCenter;
    referenceCenter.latitude = mLastLocation.latitude;
    referenceCenter.longitude = mLastLocation.longitude;
    locationtoolkit::CameraParameters param(referenceCenter);
    param.SetPosition( referenceCenter );
    param.SetTiltAngle(mTilt);
    param.SetZoomLevel(mZoom);
    mLTKMapKit->MoveTo( param );
    mMapCenter = param.GetPosition();

    locationtoolkit::Location source;
    source.latitude = mReferenceCenter.GetLocation().center.latitude;
    source.longitude = mReferenceCenter.GetLocation().center.longitude;
    source.heading = 60.0;
    source.valid = 507;
    if(isFollowMe())
    {
        mMapCameraLocked = true;
        mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME,false);
    }
    else
    {
        mMapCameraLocked = false;
        mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY,false);
    }
    setMapCameraLockButtonPosition(mIsNavigationStarted);
    mLTKMapKit->GetAvatar().SetMode( locationtoolkit::Avatar::AM_ARROW );
    mLTKMapKit->GetAvatar().SetLocation(source);
    //    mLTKMapKit->GetAvatar().SetScale(2);
}
void MapViewController::setMapCameraLockButtonPosition(bool isNav)
{
    if(isNav)
    {
        int y = mManeuverDetailsController->position().y() + mManeuverDetailsController->height() - 65;
        int x = mManeuverDetailsController->position().x() + mManeuverDetailsController->width() + 20;
        mMapCameraLockButton->setPosition(QPoint(x,y));
    }
    else
    {
        int y = this->height()-90-65;
        int x = this->width()/10;
        mMapCameraLockButton->setPosition(QPoint(this->width() - 90, this->height()/1.2/*1.3*/));

        //  mMapCameraLockButton->setPosition(QPoint(x,y));
    }
}
void MapViewController::setPOIPosition(QPointF pos, QString placeName)
{
    qDebug()<<"MapViewController::setPOIPosition::POI Position::"<<pos.x()<<"::"<<pos.y();
    mNavDestPoint = pos;
    mNaveDestName = placeName;

}
