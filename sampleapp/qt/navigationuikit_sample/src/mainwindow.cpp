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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "navigationuimapimpl.h"
#include "util.h"
#include "GPSFile.h"
#include "devtooldialog.h"
#include "aboutdialog.h"
#include "positioninputdialog.h"
#include "modeselectordialog.h"
#include "globalsetting.h"

#include <stdio.h>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QHBoxLayout>

using namespace locationtoolkit;

#define INVALID_LATITUDE 999.0
#define INTERPOLATER_TIME_GAP 20

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mMenu(new QMenu(this)),
    mNavigtionUIKit(NULL),
    mLTKContext(NULL),
    mHybridManager(NULL),
    mLTKMapKit(NULL),
    mNavigationUIMapImpl(NULL),
    mNavigationUIMapController(NULL),
    mIsDebugViewShown( false ),
    mIsNavigationStarted(false)
{
    ui->setupUi(this);
    setGeometry(200, 50, 64*16, 64*9);
    setFixedSize(width(), height());
    mLTKContext = GetLTKContext();
    ShowModeSelector();

    // do not use layout to place map widget, because the geometry of this
    // widget will be reset by NavUIKit when navigation starts up
    mLTKMapKit = GetMapWidget();
    mLTKMapKit->setParent( centralWidget() );
    centralWidget()->setGeometry(rect());
    mLTKMapKit->setGeometry(centralWidget()->rect());
    connect( mLTKMapKit, SIGNAL(MapCreated()), this, SLOT(OnMapCreated()) );
    connect( mLTKMapKit, SIGNAL(LayersCreated()), this, SLOT(OnLayersCreated()) );
    connect( mLTKMapKit, SIGNAL(LayerOptionButtonClicked()), this, SLOT(OnLayerOptionButtonClicked()) );
    connect( mLTKMapKit, SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)),
             this, SLOT(OnMapLongClick(const locationtoolkit::Coordinates&)) );
    connect( mLTKMapKit, SIGNAL(Unlocked()), this, SLOT(OnMapUnlocked()) );

    // create route option widget
    mNavUIRouteOptions.SetRouteType(NavUIRouteOptions::RT_Fastest);
    mNavUIRouteOptions.SetTransportationMode(NavUIRouteOptions::TM_Car);
    mNavUIRouteOptions.SetPronunciationStyle("AAC");
    mRouteOptionWidget = new RouteOptionWidget( this, mNavUIRouteOptions.GetAvoidFeatures() );
    mRouteOptionWidget->hide();
    connect( mRouteOptionWidget, SIGNAL(newAvoidOptions(quint32)), this, SLOT(OnNewAvoidOptions(quint32)) );
    connect( &mInterpolatorTimer, SIGNAL(timeout()), this, SLOT(OnTimer()) );
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    mGPSFilePath = dir.absolutePath() + "/resource/gpsfiles/";
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mMenu;
    delete mNavigationUIMapImpl;
    delete mNavigationUIMapController;
    delete mNavigtionUIKit;
}

int MainWindow::InitializeNavigationuikit(const Place& destination, bool enableRts)
{
    qDebug("InitializeNavigationkit.....");
    UserPreferences currentUserPreference;

    NavUIPreferences currentNavPreferences;
    currentNavPreferences.SetMeasurement(NavUIPreferences::NonMetric);
    currentNavPreferences.SetOffRouteIgnoreCount(1);
    currentNavPreferences.SetLaneGuidance(true);
    currentNavPreferences.SetNaturalGuidance(true);
    currentNavPreferences.SetRealisticSign(true);
    currentNavPreferences.SetSpeedLimitSignEnabled(true);
    currentNavPreferences.SetSpeedLimitAlertEnabled(true);
    currentNavPreferences.SetSpeedingDisplayAlertEnabled(true);
    currentNavPreferences.SetSpeedingWarningLevel(1);
    currentNavPreferences.SetSchoolZoneAlertEnabled(true);
    currentNavPreferences.SetPositionUpdateRate(20);
    currentNavPreferences.SetEnableEnhancedNavigationStartup(true);

    if (enableRts)
    {
        currentNavPreferences.SetMultipleRoutes(true);
    }
    else
    {
        currentNavPreferences.SetMultipleRoutes(false);
    }

    QString resourcepath = QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath() +
            "/resource/";
    QString configPath = resourcepath + "WidgetLayoutConfig.xml";

    mLTKLocationConfiguration.emulationMode = true;
    mLTKLocationConfiguration.locationFilename = mGPSFilePath + mGPSFile;
    LocationProvider& LocationProvider = LocationProvider::GetInstance( mLTKLocationConfiguration );

    mNavigationUIMapController = new NavigationUIMapController(mLTKMapKit, this);

    mNavigationUIMapImpl = new NavigationUIMapImpl(mNavigationUIMapController);

    currentUserPreference.parentView = centralWidget();
    currentUserPreference.navPreferences = currentNavPreferences;
    currentUserPreference.navRouteOptions = mNavUIRouteOptions;
    currentUserPreference.ltkContext = mLTKContext;
    currentUserPreference.ltkLocationProvider = &LocationProvider;
    currentUserPreference.mapInterface = mNavigationUIMapImpl;
    currentUserPreference.enableTurnByTurnNavigation = true;
    currentUserPreference.isPlanTrip = false;
    currentUserPreference.configFilePath = configPath.toStdString();
    currentUserPreference.deviceType = "Qt";
    currentUserPreference.resourcePath = resourcepath.toStdString();

    mNavigtionUIKit = NavigationUIKit::GetNavigationUIKit(currentUserPreference, mPlaceDest, mPlaceOrig);
    if (mNavigtionUIKit != NULL)
    {
        qDebug("Get NavigationUIKit ok!");
        const NavigationUISignals& navigationUISignals = mNavigtionUIKit->GetNavigationUISignals();
        connect(&navigationUISignals,SIGNAL(OnNavigationStart()), this, SLOT(OnNavigationStarted()) );
        connect(&navigationUISignals,SIGNAL(OnRouteOptionsChanged()), this, SLOT(OnRouteOptions()));
        connect(&navigationUISignals,SIGNAL(OnNavigationEnd()), this, SLOT(OnNavigationEnd()));
        mRouteOptionWidget->hide();
    }
    else
    {
        QMessageBox msgBox( QMessageBox::Information, "NavigationUIKit Sample", "navigation ui kit instance initialize failed!" );
        msgBox.exec();
        return 0;
    }

    return 1;
}

void MainWindow::StartFollowMe()
{
    mLTKMapKit->GetAvatar().SetMode(locationtoolkit::Avatar::AM_MAP);
    mLTKMapKit->GetAvatar().SetScale(NavigationUIMapImpl::AVATAR_MAP_SCALE);
    mLTKMapKit->SetNightMode(locationtoolkit::MapWidget::NM_AUTO);
    QPoint pos( mLTKMapKit->width()/2, mLTKMapKit->height() - NavigationUIMapImpl::AVATAR_REFERENCE_BOTTOM_GAP );
    mLTKMapKit->SetReferenceCenter( pos );
    LocationConfiguration locationConfiguration;
    locationConfiguration.emulationMode = true;
    locationConfiguration.locationFilename = mGPSFilePath + mGPSFile;

    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( locationConfiguration );
    locProvider.StartReceivingFixes( static_cast<locationtoolkit::LocationListener&>(*this) );

    mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME);
    mLastLocation.latitude = INVALID_LATITUDE;
}

void MainWindow::StopFollowMe()
{
    LocationConfiguration locationConfiguration;
    locationConfiguration.emulationMode = true;
    locationConfiguration.locationFilename = mGPSFilePath + mGPSFile;

    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( locationConfiguration );
    locProvider.StopReceivingFixes( static_cast<locationtoolkit::LocationListener&>(*this) );

    mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY);
    mInterpolatorTimer.stop();
}

void MainWindow::LocationUpdated(const Location &location)
{
    if( mLastLocation.latitude > 90.0f )
    {
        mLastLocation = location;
    }
    else
    {
        mLatInterpolator.Set( 1000, mLastLocation.latitude, location.latitude );
        mLonInterpolator.Set( 1000, mLastLocation.longitude, location.longitude );
        mHeadingInterpolator.Set( 1000, mLastLocation.heading, location.heading );
        mLastLocation = location;

        mInterpolatorTime.start();
        if( !mInterpolatorTimer.isActive() )
        {
            mInterpolatorTimer.start( INTERPOLATER_TIME_GAP );
        }
    }
}

void MainWindow::ProviderStateChanged(LocationListener::LocationProviderState state)
{
}

void MainWindow::OnLocationError(int errorCode)
{
}

void MainWindow::contextMenuEvent( QContextMenuEvent * event )
{
    mMenu->move(cursor().pos());
    mMenu->show();
}

void MainWindow::OnRouteOptions()
{
    mRouteOptionWidget->move((width()-mLTKMapKit->width()) + (mLTKMapKit->width()-mRouteOptionWidget->width())/2, height()- mRouteOptionWidget->height() - 60);
    if( mRouteOptionWidget->isHidden() )
    {
        mRouteOptionWidget->show();
    }
}

void MainWindow::OnNavigationEnd()
{
    menuBar()->show();
    mLTKMapKit->setGeometry(centralWidget()->rect());
    if (mNavigtionUIKit != NULL)
    {
        qDebug("NavigationUIKit End.....");
        mNavigtionUIKit->Delete();
    }

    MapDecoration* mapdecoration = mLTKMapKit->GetMapDecoration();
    mapdecoration->SetLocateMeButtonEnabled( true );
    mapdecoration->SetZoomButtonEnabled( true );
    mapdecoration->SetLayerOptionButtonEnabled( true );
    mapdecoration->SetCompassEnabled(true);
    mRouteOptionWidget->hide();

    StartFollowMe();
    mIsNavigationStarted = false;
}

void MainWindow::on_actionStart_triggered()
{
    StartNavigation( mPlaceDest.GetLocation().center.latitude,
                     mPlaceDest.GetLocation().center.longitude,
                     mEnableRTS);
}

void MainWindow::on_actionRouteOption_triggered()
{
    OnRouteOptions();
}

void MainWindow::on_actionExit_triggered()
{
    if (mNavigtionUIKit != NULL)
    {
        qDebug("NavigationUIKit stop.....");
        mNavigtionUIKit->Delete();
    }
    exit(0);
}

void MainWindow::OnMapCreated()
{
    MapDecoration* mapDecoration = mLTKMapKit->GetMapDecoration();
    mapDecoration->SetLocateMeButtonEnabled( true );
    mapDecoration->SetZoomButtonEnabled( true );
    mapDecoration->SetLayerOptionButtonEnabled( true );
    mapDecoration->SetCompassEnabled(true);
    //set compass icon
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();
    QString dayPath = strRoot + "/resource/TEXTURE/COMPASS_DAY_TEX_64x64";
    QString nightPath = strRoot + "/resource/TEXTURE/COMPASS_NIGHT_TEX_64x64";
    mapDecoration->SetCompassPath(dayPath, nightPath);
    mLTKMapKit->GetAvatar().SetScale(NavigationUIMapImpl::AVATAR_MAP_SCALE);
    mLTKMapKit->setGeometry(centralWidget()->rect());

    StartFollowMe();
}

void MainWindow::OnMapUnlocked()
{
    mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY);
}

void MainWindow::OnLayersCreated()
{
    // create layer option widget
    mLayerOptionWidget = new LayerOptionWidget( this, mLTKMapKit );
    mLayerOptionWidget->hide();
}

void MainWindow::OnTimer()
{
    locationtoolkit::Location pos( mLastLocation );
    pos.latitude = mLatInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.longitude = mLonInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.heading = mHeadingInterpolator.GetAngleValue( mInterpolatorTime.elapsed() );

    mLTKMapKit->GetAvatar().SetLocation( pos );
}


void MainWindow::OnNewAvoidOptions(quint32 options)
{
    mNavUIRouteOptions.SetAvoidFeatures( options );
    if( mNavigtionUIKit != NULL )
    {
        qDebug("recaculating route");
        mNavigtionUIKit->Recalculate( mNavUIRouteOptions );
    }
}

void MainWindow::OnNavigationStarted()
{
    MapDecoration* mapDecoration = mLTKMapKit->GetMapDecoration();
    mapDecoration->SetLocateMeButtonEnabled( false );
    mIsNavigationStarted = true;
}

void MainWindow::on_actionAbout_triggered()
{
    QString text = "Version: ";
    if(mLTKContext)
    {
        text = "Version: " + mLTKContext->GetVersion();
    }
    AboutDialog aboutBox;
    aboutBox.SetVersion(text);
    aboutBox.exec();
}


void MainWindow::on_actionGPS_Select_triggered()
{
}

void MainWindow::OnLayerOptionButtonClicked()
{
    mLayerOptionWidget->move((width()-mLTKMapKit->width()) + (mLTKMapKit->width()-mLayerOptionWidget->width())/2, 20);

    if( mLayerOptionWidget->isHidden() )
    {
        mLayerOptionWidget->show();
    }
}

void MainWindow::on_actionDebug_View_triggered()
{
    mIsDebugViewShown = !mIsDebugViewShown;
    mLTKMapKit->ShowDebugView(mIsDebugViewShown);
}

void MainWindow::OnMapLongClick(const Coordinates &coordinate)
{
    if(!mIsNavigationStarted)
    {
        mLTKMapKit->DeleteAllPins();

        locationtoolkit::CameraParameters param( coordinate );
        QPixmap selectImage, unselectImage;
        QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
        QString strRoot = dir.absolutePath();
        QString imgPath = strRoot + "/resource/images/pin_blue.png";
        selectImage.load(imgPath);
        imgPath = strRoot + "/resource/images/pin_normal.png";
        unselectImage.load(imgPath);

        locationtoolkit::PinImageInfo selectedImage;
        selectedImage.SetPixmap(selectImage);
        selectedImage.SetPinAnchor(50, 100);
        locationtoolkit::PinImageInfo unSelectedImage;
        unSelectedImage.SetPixmap(unselectImage);
        unSelectedImage.SetPinAnchor(50, 100);

        locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);

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
        mLTKMapKit->CreatePin(pinpara);

        MapLocation mapLocation;
        mapLocation.center.latitude = coordinate.latitude;
        mapLocation.center.longitude = coordinate.longitude;
        mPlaceDest.SetLocation(mapLocation);
    }
}

void MainWindow::StartNavigation(double latitude, double longitude, bool enableRTS)
{
    StopFollowMe();

    menuBar()->hide();
    centralWidget()->setGeometry(rect());
    mLTKMapKit->setGeometry(centralWidget()->rect());

    MapLocation mapLocation;
    mapLocation.center.latitude = latitude;
    mapLocation.center.longitude = longitude;
    Place destination;
    destination.SetLocation(mapLocation);

    InitializeNavigationuikit(destination, enableRTS);

    if (mNavigtionUIKit != NULL)
    {
        qDebug("NavigationUIKit start with rts.....");
        mNavigtionUIKit->Start();
    }
    else
    {
        QMessageBox msgBox( QMessageBox::Information, "NavigationUIKit Sample", "no any navigation ui kit instance, please check it out!" );
        msgBox.exec();
    }
}

void MainWindow::ShowModeSelector()
{
    RunMode mode;
    int items = MSD_NavMode | MSD_GpsSelector | MSD_Destination;
    ModeSelectorDialog dlg(mode, items);
    dlg.exec();
    mGPSFile = mode.gpsFile;
    mEnableRTS = mode.enableRTS;
    mPlaceDest = mode.dest;

    GlobalSetting& settings = GlobalSetting::GetInstance();
    if(mode.dataFile.isEmpty()){
        mode.dataFile = settings.GetMapDataPath();
    }
    InitLTKHybridManager(mode.enableOnboard, mode.dataFile);
}
