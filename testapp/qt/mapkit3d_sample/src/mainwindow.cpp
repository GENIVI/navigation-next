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

MainWindow::MainWindow(locationtoolkit::MapWidget *mapWidget, QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow),
    mMapWidget(mapWidget),
    mFollowMeWidget(NULL),
    mCurrentGPS( "LA_staples_center.gps" )
{
    mUi->setupUi(this);

    mLayout = new QStackedLayout;
    mLayout->setMargin( 0 );
    centralWidget()->setLayout( mLayout );

    MenuWidget* menuWindow = new MenuWidget( this );
    mFollowMeWidget = GreateFollowMeWidget();

    mLayout->addWidget( menuWindow );
    mLayout->addWidget( new BasicMapWidget( this ) );
    mLayout->addWidget( new PoiPinMapWidget( this ) );
    mLayout->addWidget( new LocationMapWidget( this ) );
    mLayout->addWidget( new CameraMapWidget( this ) );
    mLayout->addWidget( new AvatarMapWidget( this ) );
    mLayout->addWidget( mFollowMeWidget );
    mLayout->addWidget( new OptionalLayersMapWidget( mMapWidget, this ) );
    mLayout->addWidget( new PoiListMapWidget( this ) );

    mLayout->setCurrentIndex( 0 );
    connect( menuWindow, SIGNAL(SigShowWidget(MenuWidget::WIDGET_TYPE)), this, SLOT(OnShowWidget(MenuWidget::WIDGET_TYPE)) );
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
            QString text = "Version: " + GetLTKContext()->GetVersion() \
                         + "\nServer: " + GlobalSetting::GetInstance().GetCurrentServer();
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
                QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
                pCurrentContainer->SetMapWidget( mMapWidget, dir.absolutePath() + "/resource/gpsfiles/" + mCurrentGPS );
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
    mMapWidget->SetNightMode(locationtoolkit::MapWidget::NM_DAY);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QaLogUploadDialog qaDlg( GetLTKContext()->GetQaLogDelegate(),this );
    qaDlg.exec();
}

FollowMeMapWidget *MainWindow::GreateFollowMeWidget()
{
    QString defaultGPS = QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath() +
            "/resource/gpsfiles/" + mCurrentGPS;

    FollowMeMapWidget* followmeWidget = new FollowMeMapWidget( this );
    followmeWidget->SetDefaultGPS( defaultGPS );

    return followmeWidget;
}

void MainWindow::ShowDevtoolDialog()
{
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QStringList filter;
    filter << "*.gps";
    QDir dirGPS( dir.absolutePath() + "/resource/gpsfiles" );
    QStringList gpsFiles = dirGPS.entryList( filter );

    DevToolDialog dlg;
    dlg.SetServerList( GlobalSetting::GetInstance().GetServerList(),
                       GlobalSetting::GetInstance().GetCurrentServer() );
    dlg.SetGPSFileList( gpsFiles, mCurrentGPS );
    int closeCode = dlg.exec();
    if( closeCode == QDialog::Accepted )
    {
        // if server changed
        if( dlg.GetSelectedServer() != GlobalSetting::GetInstance().GetCurrentServer() )
        {
            QMessageBox msgBox( QMessageBox::Information, "System prompt", "changing server will cause the application to exit, you can then restart it to connect to the new server" );
            msgBox.exec();

            GetLTKContext()->MasterClear();
            GlobalSetting::GetInstance().SetCurrentServer( dlg.GetSelectedServer() );
            close();
        }

        // if GPS changed
        if( dlg.GetSelectedGPS() != mCurrentGPS )
        {
            mCurrentGPS = dlg.GetSelectedGPS();
            mFollowMeWidget->SetDefaultGPS( dir.absolutePath() + "/resource/gpsfiles/" + mCurrentGPS );
        }
    }
}

