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

#include "mapview.h"
#include "mapwidget.h"
#include "util.h"
#include "globalsetting.h"
#include <QtDBus/QtDBus>
#include <QTime>
#include "custombubble.h"

#define SERVICE_CONSOLE_CLUSTER "com.locationstudio.qtnavigator.consoletocluster"
#define SERVICE_CLUSTER_CONSOLE "com.locationstudio.qtnavigator.clustertoconsole"

MapView::MapView(QQuickItem *parent):
    QQuickItem(parent),
    mInitDone(false),
    mMapReady(false),
    mAvatarPosition(-117.689f,33.604f),
    mAvatarHeading(0.0f),
    mTilt(15.0f),
    mZoom(19.0f),
    mIsFollowMe(false),
    mClient("")
    //iface(SERVICE_CLUSTER_CONSOLE, "/", "", QDBusConnection::sessionBus())
{
    setFlag(ItemHasContents, true);
}

MapView::~MapView()
{
}

QSGNode* MapView::updatePaintNode(QSGNode *n, UpdatePaintNodeData *d)
{
    if (!mInitDone)
    {
        QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    }
    return QQuickItem::updatePaintNode(n, d);
}

void MapView::init()
{
    qDebug("MapView::init\n");
    GlobalSetting::InitInstance();
    GlobalSetting& settings = GlobalSetting::GetInstance();

    // setToken
    if(mToken.length() > 0)
        settings.setToken(mToken);

    GetLTKContext()->StartQaLog();

    //InitLTKHybridManager(true, "/opt/comtech/data/mapdata");
    //added to set bool for obBoard data
    QSettings setting(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    setting.beginGroup("hybrid");
    QString dataPath = setting.value(gOnBoardDataPath).toString();
    bool IsOnBoard = setting.value("onBoard","false").toBool();

    qDebug()<<"MainViewController::init::"<<dataPath<<IsOnBoard;
    if(IsOnBoard)
        InitLTKHybridManager(IsOnBoard, dataPath);
    else
        InitLTKHybridManager(IsOnBoard);

    setting.endGroup();
    if(mClient=="Automotive"){
        setupConfirmationBox();
    }
    setupSpeedLimitView();
    globalMapWidget = GetMapWidget();
    if(globalMapWidget == NULL)
    {
        qDebug( "c1reate mapkit3d widget failed!!!\n" );
        return;
    }

    globalMapWidget->setParentItem(this);
    globalMapWidget->setSize(QSize(width(),height()));

    connect(globalMapWidget, &locationtoolkit::MapWidget::MapCreated, this, &MapView::onMapReady);

    setupManeuverDetailSignalAndSlots();

    //mLocationConfiguration.emulationMode = true;
    //mLocationConfiguration.locationFilename = "/opt/comtech/gps/demo.gps";
    //locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( mLocationConfiguration );
    //locProvider.StartReceivingFixes( static_cast<LocationListener&>(*this) );

    // Register for D-Bus service

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                        "To start it, run:\n"
                        "\teval `dbus-launch --auto-syntax`\n");
        return;
    }

    qDebug("SessionBus Connected\n");

    if (!QDBusConnection::sessionBus().registerService(SERVICE_CONSOLE_CLUSTER)) {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }
    qDebug("SessionBus registered service\n");

    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);

    qDebug("SessionBus registered\n");
    mTime.start();

    mInitDone = true;
}

void MapView::setWorkFolder(QString path)
{
    qDebug("MapView::SetWorkFolder %s", path.toStdString().c_str());
    LTKSampleUtil::SetWorkFolder(path);
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("applicationDirPath1", path);
}

QString MapView::workFolder()
{
    return LTKSampleUtil::GetWorkFolder();
}

void MapView::onMapReady()
{
    emit showManeuverView(false);

    mMapReady = true;

    if (mIsFollowMe)
    {
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

    globalMapWidget->GetAvatar().SetMode( locationtoolkit::Avatar::AM_ARROW );
    globalMapWidget->GetAvatar().SetScale(2);
    locationtoolkit::MapDecoration* mapDecoration = globalMapWidget->GetMapDecoration();
    mapDecoration->SetCompassEnabled(true);
    mapDecoration->SetCompassPosition(width()-50, 50);
    QDir dir = QFileInfo( LTKSampleUtil::GetWorkFolder() ).dir();
    QString strRoot = dir.absolutePath();
    QString dayPath = strRoot + "/resource/TEXTURE/COMPASS_DAY_TEX_128x128";
    QString nightPath = strRoot + "/resource/TEXTURE/COMPASS_NIGHT_TEX_128x128";
    mapDecoration->SetCompassPath(dayPath, nightPath);
    globalMapWidget->SetNightMode(GlobalSetting::GetInstance().nightMode());
}

void MapView::updateAvatarLocation(const locationtoolkit::Location& location)
{
    if (mMapReady)
    {
        locationtoolkit::Location loc;
        loc.latitude = location.latitude;
        loc.longitude = location.longitude;
        loc.heading = location.heading;
        loc.valid = 1 | 2 | 8;
        globalMapWidget->GetAvatar().SetLocation(loc);
    }
}


QPointF MapView::avatarPosition() const
{
    return mAvatarPosition;
}

void MapView::setAvatarPosition(QPointF p)
{
    mAvatarPosition = p;
    //updateAvatarLocation();
}

float MapView::avatarHeading() const
{
    return mAvatarHeading;
}

void MapView::setAvatarHeading(float t)
{
    mAvatarHeading = t;
    //updateAvatarLocation();
}

float MapView::tilt()
{
    return mTilt;
}

void MapView::setTilt(float t)
{
    mTilt = t;
}

float MapView::zoom()
{
    return mZoom;
}

void MapView::setZoom(float t)
{
    mZoom = t;
}

QString MapView::token()
{
    return mToken;
}

void MapView::setToken(QString t)
{
    mToken = t;
}

bool MapView::isFollowMe()
{
    return mIsFollowMe;
}

void MapView::setIsFollowMe(bool v)
{
    mIsFollowMe = v;
}

QString MapView::client()
{
    return mClient;
}

void MapView::setClient(QString client)
{
    mClient = client;
}

QString MapView::onManeuverUpdated(const QString& maneuverIcon, const QString& streetName, const QString& distance, const QString& trafficWarning)
{
    qDebug("MapView::onManeuverUpdated maneuverIcon : %s streetName : %s distance : %s trafficWarning : %s",
           maneuverIcon.toStdString().c_str(), streetName.toStdString().c_str(),
           distance.toStdString().c_str(), trafficWarning.toStdString().c_str());

    emit refreshManeuver(maneuverIcon, streetName, distance);

    return QString("onManeuverUpdated(\"%1, %2, %3\") got called").arg(maneuverIcon, distance, streetName);
}

QString MapView::updateGpsPosition(const QString& lat, const QString& lon, const QString& heading)
{
    //qDebug("%d--MapView::updateGpsPosition lat : %s lon : %s heading: %s", mTime.elapsed(), lat.toStdString().c_str(), lon.toStdString().c_str(), heading.toStdString().c_str());
    mTime.restart();
    //For fuel alert in neptune
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    bool isSet = engine->rootContext()->contextProperty("myflag").toBool();
    if(!isSet)
        engine->rootContext()->setContextProperty("myflag",true);
    //end
    locationtoolkit::Location loc;

    loc.latitude = lat.toFloat();
    loc.longitude = lon.toFloat();
    loc.heading = heading.toFloat();
    updateAvatarLocation(loc);

    return QString("updateGpsPosition(\"%1, %2, %3\") got called").arg(lat, lon, heading);
}


QString  MapView::setNavigationMode(bool navMode)
{
    qDebug("MapView::setNavigationMode : %d", navMode);

    emit showManeuverView(navMode);

    return QString("setNavigationMode got called");
}
QString MapView::setClusterUIMode(int mode)
{
    qDebug("MapView::setClusterUIMode : %d", mode);

    emit showClusterUI(mode);

    return QString("setClusterUIMode got called");
}

QString MapView::updateRemainingManeuverDistance(const QString& distance)
{
    emit refreshManeuverDistance(distance);
    return QString("updateRemainingManeuverDistance(\"%1\") got called").arg(distance);
}
QString MapView::updatePolyline(const QString &polylineString)
{
    qDebug()<<" MapView::updatePolyline"<<polylineString;
    QJsonDocument doc = QJsonDocument::fromJson(polylineString.toUtf8());
    QJsonObject polyline;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            qDebug() << "Document is an object" << endl;
            polyline = doc.object();
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
        }
    }
    QJsonArray coordinates = polyline.value("coordinates").toArray();
    qDebug()<<"array"<<coordinates;
    QList<locationtoolkit::Coordinates> points;
    locationtoolkit::Coordinates destination;
    int i = 0;
    foreach (QJsonValue jValue, coordinates) {

        qDebug()<<"value"<<jValue;
        QJsonObject ob = jValue.toObject();
        QString str = ob.value("lattitude").toString();
        qDebug()<<"object"<<ob<<"obvalue"<<str<<str.toDouble();
        locationtoolkit::Coordinates val;
        val.setLatitude(ob.value("lattitude").toString().toDouble());
        val.setLongitude(ob.value("longitude").toString().toDouble());
        points.append(val);
        qDebug()<<"points......."<<val.getLatitude()<<val.getLongitude();

        if(i == coordinates.count()-1)
        {
            destination.latitude = val.getLatitude();
            destination.longitude = val.getLongitude();
        }

        i++;
    }

    int n = activePolylines.size();
    for (int i = 0; i < n; ++i)
    {
        globalMapWidget->RemovePolyline(activePolylines[i]);
    }
    activePolylines.clear();

    n = mManeuverArrows.size();
    for (int i = 0; i < n; ++i)
    {
        globalMapWidget->RemovePolyline(mManeuverArrows[i]);
    }
    mManeuverArrows.clear();

    int total = points.count();
    locationtoolkit::PolylineParameters param;
    //  Set attributes for the polyLine under consideration
    QList<locationtoolkit::SegmentAttribute*>* mSegAttr = new QList<locationtoolkit::SegmentAttribute*>();
    locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(total - 1, QColor(Qt::green));
    mSegAttr->append(segattr);
    param.SetSegmentAttributes(*mSegAttr);
    param.SetPoints(points);
    param.SetWidth(15);
    param.SetVisible(true);
    param.SetAttributeType(locationtoolkit::PolylineParameters::SAT_ColorSegment);
    param.SetUnhighlightColor(QColor(0,0,255,255));
    param.setOutlineColor(QColor(Qt::darkGray));
    param.setOutlineWidth(15);
    locationtoolkit::Polyline* polyLine = &(globalMapWidget->AddPolyline(param));
    //  add polylines to active plyline's array
    activePolylines.append(polyLine);

    //add destination pin
    QPixmap selectImage, unselectImage;
    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    selectImage.load(resourceFolder + "images/pin_select.png");
    unselectImage.load(resourceFolder + "images/pin_unselect.png");

    locationtoolkit::PinImageInfo selectedImage;
    selectedImage.SetPixmap(selectImage);
    selectedImage.SetPinAnchor(50, 100);
    locationtoolkit::PinImageInfo unSelectedImage;
    unSelectedImage.SetPixmap(unselectImage);
    unSelectedImage.SetPinAnchor(50, 100);

    locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);
    locationtoolkit::Bubble* bubble = new locationtoolkit::CustomBubble();

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
    locationtoolkit::Pin *createdPin = globalMapWidget->CreatePin(pinpara);
    locationtoolkit::CameraParameters param1(destination);
    param1.SetPosition( destination );
    globalMapWidget->MoveTo( param1 );

    return QString("updatePolyline got called");
}

QString MapView::clearPolyline()
{
    globalMapWidget->Clear();
    mSpeedLimitView->setVisible(false);
    return QString("clearPolyline got called");
}

QString MapView::updateManeuverArrow(const QString &pts)
{

    qDebug()<<" MapView::updateManeuverArrow"<<pts;
    QJsonDocument doc = QJsonDocument::fromJson(pts.toUtf8());
    QJsonObject polyline;
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            qDebug() << "Document is an object" << endl;
            polyline = doc.object();
        }
        else
        {
            qDebug() << "Document is not an object" << endl;
        }
    }
    QJsonArray coordinates = polyline.value("coordinates").toArray();
    qDebug()<<"array"<<coordinates;
    QList<locationtoolkit::Coordinates> points;

    foreach (QJsonValue jValue, coordinates) {

        qDebug()<<"value"<<jValue;
        QJsonObject ob = jValue.toObject();
        QString str = ob.value("lattitude").toString();
        qDebug()<<"object"<<ob<<"obvalue"<<str<<str.toDouble();
        locationtoolkit::Coordinates val;
        val.setLatitude(ob.value("lattitude").toString().toDouble());
        val.setLongitude(ob.value("longitude").toString().toDouble());
        points.append(val);
        qDebug()<<"points......."<<val.getLatitude()<<val.getLongitude();

    }



    //to draw maeuver arrow
    int n = mManeuverArrows.size();
    for (int i = 0; i < n; ++i)
    {
        globalMapWidget->RemovePolyline(mManeuverArrows[i]);
    }
    mManeuverArrows.clear();
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
    QList<locationtoolkit::SegmentAttribute*> segAttrs;
    for( int i = 0; i < points.size(); i++ )
    {
        locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(points.size()-1,
                                                                                   QColor(MANEUVER_COLOR));
        segAttrs.append( segattr );
    }

    locationtoolkit::PolylineParameters para;
    para.SetPoints(points);
    para.SetSegmentAttributes(segAttrs);
    para.SetUnhighlightColor(QColor(MANEUVER_COLOR));
    para.SetWidth(MANEUVER_ARROW_MANEUVER_WIDTH);
    para.SetStartCap(mStartCap);
    para.SetEndCap(mEndCap);
    para.SetAttributeType(locationtoolkit::PolylineParameters::SAT_ColorSegment);
    para.SetZOrder(255);
    locationtoolkit::Polyline& polylineObj = globalMapWidget->AddPolyline(para);
    mManeuverArrows.push_back(&polylineObj);
    return QString("updateManeuverArrow got called");
}
QString MapView::onUpdateSpeedLimit(const QString& speedLimit)
{

    qDebug()<<"onUpdateSpeedLimit1::"<<speedLimit;
    if(speedLimit == " ")
    {
        mSpeedLimitView->setVisible(false);
        return QString("onUpdateSpeedLimit");
    }
    mSpeedLimitView->setVisible(true);
    QQmlEngine *engine = QQmlEngine::contextForObject(this->parentItem())->engine();
    engine->rootContext()->setContextProperty("speedLimitText",speedLimit);
    return QString("onUpdateSpeedLimit");
}

void MapView::setupManeuverDetailSignalAndSlots()
{
    qDebug()<<"MapView::init add ManeuverDetailController start";
    mManeuverController = new ManeuverDetailController(this);

    QQmlEngine::setObjectOwnership(mManeuverController, QQmlEngine::CppOwnership);
    mManeuverController->setParentItem(this);
    QQmlEngine::setContextForObject(mManeuverController, QQmlEngine::contextForObject(this));

    mManeuverController->setWidth(width());
    mManeuverController->setHeight(height());

    mManeuverController->setUp();
    connect(this, SIGNAL(refreshManeuver(QString,QString,QString)), mManeuverController, SLOT(onRefreshManeuver(QString,QString,QString)) );
    connect(this, SIGNAL(showManeuverView(bool)), mManeuverController, SLOT(onShowManeuverView(bool)) );
    connect(this, SIGNAL(refreshManeuverDistance(QString)), mManeuverController, SLOT(onRrefreshManeuverDistance(QString)) );
    connect(this, SIGNAL(showClusterUI(int)),this,SLOT(onSetClusterUI(int)));
    qDebug()<<"MapView::init add ManeuverDetailController stop";
}
void MapView::onSetClusterUI(int mode){
    qDebug("MapView::onSetClusterUI : %d", mode);
    switch (mode) {
    case 0:
        emit showManeuverView(true);
        break;
    case 1:
        emit showManeuverView(false);
        break;
    default:
        emit showManeuverView(true);
        break;
    }
}

void MapView::setupConfirmationBox()
{
    qDebug()<<"MapView::init add setupConfirmationBox start" << width();
    mConfirmationController = new ConfirmationBoxController(this);

    QQmlEngine::setObjectOwnership(mConfirmationController, QQmlEngine::CppOwnership);
    mConfirmationController->setParentItem(this);
    QQmlEngine::setContextForObject(mConfirmationController, QQmlEngine::contextForObject(this));
    //For fuel alert in neptune
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("myflag",false);
    //end
    mConfirmationController->setWidth(width());
    mConfirmationController->setHeight(height());

    mConfirmationController->setUp();
    connect(mConfirmationController, SIGNAL(lowFuelLimit(bool)),this,SLOT(onLowFuelLimit(bool)));
}

void MapView::onLowFuelLimit(bool flag)
{
    qDebug()<<"MapView::onLowFuelLimit" << flag;

    QDBusInterface ifaceToConsole(SERVICE_CLUSTER_CONSOLE, "/", "", QDBusConnection::sessionBus());
    if(ifaceToConsole.isValid()) {
        //QDBusReply<QString> reply = iface.call("showFuelAlert", flag);

        //if(reply.isValid()) {
        //}
        //else {
        //    qDebug("call showFuelAlert failed");
        //}

        QDBusPendingCall pcall = ifaceToConsole.asyncCall("showFuelAlert", flag);

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                         this, SLOT(dbusCallFinishedSlot(QDBusPendingCallWatcher*)));
    }
}

void MapView::dbusCallFinishedSlot(QDBusPendingCallWatcher* watcher)
{

}

QString MapView::hideFuelAlert(bool mode, bool navmode)
{
    qDebug() << "MapView::hideFuelAlert " << mode;
    if(mConfirmationController)
    {
        mConfirmationController->isNavMode(navmode);
        mConfirmationController->setVisibility(mode);
    }
    return QString("hideFuelAlert got called");
}

void MapView::setupSpeedLimitView()
{
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    QString mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath1").toString();
    QQmlComponent componentSpeedLimit(engine,QUrl(mAppDirPath+"/resource/SpeedLimitView.qml"));
    mSpeedLimitView = qobject_cast<QQuickItem*>(componentSpeedLimit.create());
    QQmlEngine::setObjectOwnership(mSpeedLimitView, QQmlEngine::CppOwnership);
    mSpeedLimitView->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mSpeedLimitView, QQmlEngine::contextForObject(this));
    mSpeedLimitView->setVisible(false);
    if(mClient=="Neptune")
        mSpeedLimitView->setPosition(QPointF(width()+350,520));
    else if(mClient=="Automotive")
        mSpeedLimitView->setPosition(QPointF(width()+250,520));
}
