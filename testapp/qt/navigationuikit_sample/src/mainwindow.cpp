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

#include <stdio.h>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QHBoxLayout>

using namespace locationtoolkit;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mMenu(new QMenu(this)),
    mNavigtionUIKit(NULL),
    mLTKContext(NULL),
    mLTKMapKit(NULL),
    mNavigationUIMapImpl(NULL),
    mNavigationUIMapController(NULL),
    mActionPreferences(new QAction(tr("&Preferences"), this)),
    mActionRouteOptions(new QAction(tr("&RouteOptions"), this)),
    mActionStart(new QAction(tr("&Start"), this)),
    mActionStop(new QAction(tr("&Stop"), this))
{
    ui->setupUi(this);

    connect(mActionPreferences, SIGNAL(triggered()), this, SLOT(OnPeferences()));
    connect(mActionRouteOptions, SIGNAL(triggered()), this, SLOT(OnRouteOptions()));
    connect(mActionStart, SIGNAL(triggered()), this, SLOT(OnStart()));
    connect(mActionStop, SIGNAL(triggered()), this, SLOT(OnStop()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mMenu;
    delete mNavigationUIMapImpl;
    delete mNavigationUIMapController;
    delete mNavigtionUIKit;
    delete mActionPreferences;
    delete mActionRouteOptions;
    delete mActionStart;
    delete mActionStop;
}

int MainWindow::InitializeNavigationuikit()
{
    qDebug("InitializeNavigationkit.....");
    UserPreferences curUserPreference;

    NavUIPreferences NavPreference;
    NavPreference.SetMeasurement(NavUIPreferences::Metric);
    NavPreference.SetOffRouteIgnoreCount(1);
    NavPreference.SetLaneGuidance(QBool(true));
    NavPreference.SetNaturalGuidance(QBool(false));
    NavPreference.SetRealisticSign(QBool(true));
    NavPreference.SetSpeedLimitSignEnabled(QBool(true));
    NavPreference.SetSpeedLimitAlertEnabled(QBool(true));
    NavPreference.SetSpeedingDisplayAlertEnabled(QBool(true));
    NavPreference.SetSpeedingWarningLevel(1);
    NavPreference.SetSchoolZoneAlertEnabled(QBool(true));
    NavPreference.SetMultipleRoutes(QBool(false));

    NavUIRouteOptions NavUIRouteOptions;
    NavUIRouteOptions.SetRouteType(NavUIRouteOptions::RT_Fastest);
    NavUIRouteOptions.SetTransportationMode(NavUIRouteOptions::TM_Car);
    NavUIRouteOptions.SetAvoidFeatures(NavUIRouteOptions::RA_Unpaved);
    NavUIRouteOptions.SetPronunciationStyle("AAC");

    QString resourcepath = QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath() +
            "/resource/";
    QString defaultGPSFile = resourcepath + "gpsfiles/LA_staples_center.gps";
    QString configPath = resourcepath + "WidgetLayoutConfig.xml";

    mLTKLocationConfiguration.emulationMode = QBool(true);
    mLTKLocationConfiguration.locationFilename = defaultGPSFile;
    LocationProvider& LocationProvider = LocationProvider::GetInstance( mLTKLocationConfiguration );

    mLTKMapKit = GetMapWidget();
    mLTKMapKit->setParent( centralWidget() );
    centralWidget()->setGeometry(rect());
    mLTKMapKit->setGeometry(centralWidget()->rect());
    mNavigationUIMapController = new NavigationUIMapController(mLTKMapKit, this);

    mNavigationUIMapImpl = new NavigationUIMapImpl(mNavigationUIMapController);
    mLTKContext = GetLTKContext();

    curUserPreference.parentView = centralWidget();
    curUserPreference.navPreferences = NavPreference;
    curUserPreference.navRouteOptions = NavUIRouteOptions;
    curUserPreference.ltkContext = mLTKContext;
    curUserPreference.ltkLocationProvider = &LocationProvider;
    curUserPreference.mapInterface = mNavigationUIMapImpl;
    curUserPreference.enableTurnByTurnNavigation = true;
    curUserPreference.isPlanTrip = false;
    curUserPreference.configFilePath = configPath.toStdString();
    curUserPreference.deviceType = "Qt";

    Place curPlaceOrig;
    Place curPlaceDest;

    GetDefaultDestinationAndOrigin(defaultGPSFile.toStdString(), &curPlaceDest, &curPlaceOrig);

    mNavigtionUIKit = NavigationUIKit::GetNavigationUIKit(curUserPreference, curPlaceDest, curPlaceOrig);
    if (mNavigtionUIKit != NULL)
    {
        qDebug("Get NavigationUIKit ok!");
        const NavigationUISignals& navigationUISignals = mNavigtionUIKit->GetNavigationUISignals();
        connect(&navigationUISignals,SIGNAL(OnRouteOptionsChanged()), this, SLOT(OnRouteOptions()));
    }
    else
    {
        QMessageBox msgBox( QMessageBox::Information, "NavigationUIKit Sample", "navigation ui kit instance initialize failed!" );
        msgBox.exec();
        return 0;
    }

    return 1;
}

void MainWindow::contextMenuEvent( QContextMenuEvent * event )
{
    mMenu->addAction(mActionPreferences);
    mMenu->addAction(mActionRouteOptions);
    mMenu->addAction(mActionStart);
    mMenu->addAction(mActionStop);
    mMenu->move(cursor().pos());
    mMenu->show();
}

void MainWindow::OnPeferences()
{

}

void MainWindow::OnRouteOptions()
{
    qDebug("OnRouteOptions choosed.....");
}

void MainWindow::OnStart()
{
    if (mNavigtionUIKit != NULL)
    {
        qDebug("NavigationUIKit start.....");
        mNavigtionUIKit->Start();
    }
    else
    {
        QMessageBox msgBox( QMessageBox::Information, "NavigationUIKit Sample", "no any navigation ui kit instance, please check it out!" );
        msgBox.exec();
    }
}

void MainWindow::OnStop()
{

}
