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
#include "menuwidget.h"
#include "basicmapwidget.h"
#include "poipinmapwidget.h"
#include "locationmapwidget.h"
#include "optionallayersmapwidget.h"
#include "poilistmapwidget.h"
#include "devtooldialog.h"
#include "globalsetting.h"
#include "cameramapwidget.h"
#include "avatarmapwidget.h"
#include "qaloguploaddialog.h"
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

#define DEFAULT_LATITUDE 33.604
#define DEFAULT_LONGITUE -117.689
#define DEFAULT_ZOOMLEVEL 16.f
#define DEFAULT_HEADING 0.f
#define DEFAULT_TILT_ANGLE 90.f

MainWindow::MainWindow(locationtoolkit::MapWidget *mapWidget, QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow),
    mMapWidget(mapWidget),
    mFollowMeWidget(NULL),
    mCurrentGPS( "LA_staples_center.gps" )
{
//    mUi->setupUi(this);
//    mLayout = new QStackedLayout();
//    mLayout->setMargin( 0 );
//    centralWidget()->setLayout( mLayout );
//    if(GlobalSetting::GetInstance().MapviewOnly())
//    {
//        mLayout->addWidget(mMapWidget);
//    }
//    else
//    {

//        MenuWidget* menuWindow = new MenuWidget( this );
//        mFollowMeWidget = GreateFollowMeWidget();

//        mLayout->addWidget( menuWindow );
//        mLayout->addWidget( new BasicMapWidget( this ) );
//        mLayout->addWidget( new PoiPinMapWidget( this ) );
//        mLayout->addWidget( new LocationMapWidget( this ) );
//        mLayout->addWidget( new CameraMapWidget( this ) );
//        mLayout->addWidget( new AvatarMapWidget( this ) );
//        mLayout->addWidget( mFollowMeWidget );
//        mLayout->addWidget( new OptionalLayersMapWidget( mMapWidget, this ) );
//        mLayout->addWidget( new PoiListMapWidget( this ) );

//        connect( menuWindow, SIGNAL(SigShowWidget(MenuWidget::WIDGET_TYPE)), this, SLOT(OnShowWidget(MenuWidget::WIDGET_TYPE)) );
//    }
//    mLayout->setCurrentIndex( 0 );
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::OnShowWidget(MenuWidget::WIDGET_TYPE type)
{
    MapContainer* pCurrentContainer = dynamic_cast<MapContainer *>( mLayout->currentWidget() );
    if( pCurrentContainer != NULL )
    {
        pCurrentContainer->ReleaseMapWidget();
    }

    switch ( type )
    {
    case MenuWidget::WIDGET_SETTINGS:
        ShowDevtoolDialog();
        break;

    case MenuWidget::WIDGET_ABOUT:
        {
            QString text = "Version: " + GetLTKContext()->GetVersion();
            QMessageBox aboutBox( QMessageBox::NoIcon, "Sample app", text );
            aboutBox.exec();
        }
        break;

    default:
        {
            int index = (int)type;
            mLayout->setCurrentIndex( index );

            pCurrentContainer = dynamic_cast<MapContainer *>( mLayout->currentWidget() );
            if( pCurrentContainer != NULL )
            {
                pCurrentContainer->SetMapWidget( mMapWidget,
                                                 LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + mCurrentGPS );
            }
        }
        break;
    }
}

void MainWindow::OnBackButtonClicked()
{
    OnShowWidget( MenuWidget::WIDGET_MENU );
    mMapWidget->Clear();
    mMapWidget->GetAvatar().SetMode(locationtoolkit::Avatar::AM_NONE);
    mMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY);
    double lat = DEFAULT_LATITUDE;
    double lon = DEFAULT_LONGITUE;
    locationtoolkit::Coordinates coordinate(lat, lon);
    locationtoolkit::CameraParameters defaultCamera(coordinate, DEFAULT_ZOOMLEVEL, DEFAULT_TILT_ANGLE, DEFAULT_HEADING);
    mMapWidget->MoveTo(defaultCamera);
    mMapWidget->SetNightMode(locationtoolkit::MapWidget::NM_DAY);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QaLogUploadDialog qaDlg( GetLTKContext()->GetQaLogDelegate(),this );
    qaDlg.exec();
}

FollowMeMapWidget *MainWindow::GreateFollowMeWidget()
{
    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    QString defaultGPS =  resourceFolder + "gpsfiles/" + mCurrentGPS;

    FollowMeMapWidget* followmeWidget = new FollowMeMapWidget( this );
    followmeWidget->SetDefaultGPS( defaultGPS );

    return followmeWidget;
}

void MainWindow::ShowDevtoolDialog()
{
    QString resourceFolder = LTKSampleUtil::GetResourceFolder();

    QStringList filter;
    filter << "*.gps";
    QDir dirGPS( resourceFolder + "gpsfiles" );
    QStringList gpsFiles = dirGPS.entryList( filter );

    DevToolDialog dlg;
    dlg.SetGPSFileList( gpsFiles, mCurrentGPS );
    int closeCode = dlg.exec();
    if( closeCode == QDialog::Accepted )
    {
        // if GPS changed
        if( dlg.GetSelectedGPS() != mCurrentGPS )
        {
            mCurrentGPS = dlg.GetSelectedGPS();
            mFollowMeWidget->SetDefaultGPS( resourceFolder + "gpsfiles/" + mCurrentGPS );
        }
    }
}

