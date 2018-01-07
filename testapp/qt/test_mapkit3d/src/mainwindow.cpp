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
#include <QVBoxLayout>
#include <QDir>
#include <QDebug>
using namespace locationtoolkit;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);
    mMapView = new MapWidget();    
    mUserOptionWidget = new UserOptions();

    int nMapWidth = this->geometry().width() - mUserOptionWidget->geometry().width();
    mMapView->setMinimumWidth( nMapWidth );

    QHBoxLayout* pLayout = new QHBoxLayout( centralWidget() );
    pLayout->addWidget( mMapView );
    pLayout->addWidget( mUserOptionWidget );
    centralWidget()->setLayout( pLayout );

    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();
    qDebug() << strRoot;
    locationtoolkit::MapOptions mapOption;
    mapOption.mResourceFolder = strRoot + "/resource/";
    mapOption.mWorkFolder = strRoot + "/";
    QString empty("");
    mLtkContext = new LTKContext( empty, empty );
    mMapView->Initialize( mapOption,  *mLtkContext );

    mUserOptionWidget->SetMainWindow( this );
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::OnCameraMove(UserOptions::CAMERA_MOVE_TYPE moveType )
{
    Coordinates pos;
    CameraParameters cameraPos( pos );
    mMapView->GetCameraPosition( cameraPos );
    pos = cameraPos.GetPosition();
    qDebug() << "lat = " << pos.latitude << "lon = " << pos.longitude << "heading = " << cameraPos.GetHeadingAngle() << "tilt = " << cameraPos.GetTiltAngle();

    switch (moveType)
    {
    case UserOptions::CMT_UP:
        pos.latitude += 0.001;
        break;
    case UserOptions::CMT_DOWN:
        pos.latitude -= 0.001;
        break;
    case UserOptions::CMT_LEFT:
        pos.longitude -= 0.001;
        break;
    case UserOptions::CMT_RIGHT:
        pos.longitude += 0.001;
        break;
    default:
        break;
    }

    cameraPos.SetPosition( pos );
    mMapView->MoveTo( cameraPos );
}

void MainWindow::OnZoomChange( UserOptions::CAMERA_ZOOM_TYPE zoomType )
{}

void MainWindow::OnTiltChange( UserOptions::CAMERA_TILT_TYPE tiltType )
{}

void MainWindow::OnShowNightMode( bool bNight )
{}

void MainWindow::OnShowCompass( bool bShow )
{}

void MainWindow::OnShowDoppler( bool bShow )
{}

void MainWindow::OnShowTraffic( bool bShow )
{}
