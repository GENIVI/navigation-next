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

#include "useroptions.h"
#include "ui_useroptions.h"
#include "mainwindow.h"
#include <QColorDialog>

UserOptions::UserOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserOptions),
    mIsNightMode(false),
    mIsCompassShown(false),
    mIsDopplerShown(false),
    mIsTrafficShown(false),
    mCircleFillColor( 255, 0, 0 ),
    mCircleStrokeColor( 0, 0, 255 )
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex( 0 );
}

UserOptions::~UserOptions()
{
    delete ui;
}

void UserOptions::SetMainWindow( QWidget* mainwin )
{
    MainWindow* mw = static_cast<MainWindow*>( mainwin );
    connect( this, SIGNAL(SigMoveCamera(UserOptions::CAMERA_MOVE_TYPE)), mw, SLOT(OnCameraMove(UserOptions::CAMERA_MOVE_TYPE)) );
    connect( ui->zoomInButton, SIGNAL(clicked()), mw, SLOT(OnZoomChange(MainWindow::CZT_IN)) );
    connect( ui->zoomOutButton, SIGNAL(clicked()), mw, SLOT(OnZoomChange(MainWindow::CZT_OUT)) );
    connect( ui->titltUpButton, SIGNAL(clicked()), mw, SLOT(OnTiltChange(MainWindow::CTT_UP)) );
    connect( ui->titltDownButton, SIGNAL(clicked()), mw, SLOT(OnTiltChange(MainWindow::CTT_DOWN)) );

    connect( this, SIGNAL(SigShowNight(bool)), mw, SLOT(OnShowNightMode(bool)) );
    connect( this, SIGNAL(SigShowCompass(bool)), mw, SLOT(OnShowCompass(bool)) );
    connect( this, SIGNAL(SigShowDoppler(bool)), mw, SLOT(OnShowDoppler(bool)) );

    connect( ui->backButton_2, SIGNAL(clicked()), this, SLOT(on_backButton_clicked()) );
    connect( ui->backButton_3, SIGNAL(clicked()), this, SLOT(on_backButton_clicked()) );
}

void UserOptions::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex( 0 );
}

void UserOptions::on_camaraBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex( 1 );
}

void UserOptions::on_generalButton_clicked()
{
    ui->stackedWidget->setCurrentIndex( 2 );
}

void UserOptions::on_polylineButton_clicked()
{
    ui->stackedWidget->setCurrentIndex( 3 );
}

void UserOptions::on_switchDayNightButton_clicked()
{
    mIsNightMode = !mIsNightMode;
    if( mIsNightMode == true )
    {
        ui->switchDayNightButton->setText( "day" );
    }
    else
    {
        ui->switchDayNightButton->setText( "night" );
    }
    emit SigShowNight( mIsNightMode );
}

void UserOptions::on_showCompassButton_clicked()
{
    mIsCompassShown = !mIsCompassShown;
    if( mIsCompassShown == true )
    {
        ui->showCompassButton->setText( "hide" );
    }
    else
    {
        ui->showCompassButton->setText( "show" );
    }
    emit SigShowCompass( mIsCompassShown );
}

void UserOptions::on_showDopplerButton_clicked()
{
    mIsDopplerShown = !mIsDopplerShown;
    if( mIsDopplerShown == true )
    {
        ui->showDopplerButton->setText( "hide" );
    }
    else
    {
        ui->showDopplerButton->setText( "show" );
    }
    emit SigShowDoppler( mIsDopplerShown );
}

void UserOptions::on_showTrafficButton_clicked()
{
    mIsTrafficShown = !mIsTrafficShown;
    if( mIsTrafficShown == true )
    {
        ui->showTrafficButton->setText( "hide" );
    }
    else
    {
        ui->showTrafficButton->setText( "show" );
    }
    emit SigShowTraffic( mIsTrafficShown );
}

void UserOptions::on_circleFillColorButton_clicked()
{
    QColorDialog dlg;
    QColor color = dlg.getColor();
    QString style;
    style.sprintf( "QPushButton { background-color: rgb(%d, %d, %d); }", color.red(), color.green(), color.blue() );
    ui->circleFillColorButton->setStyleSheet( style );
    repaint();
}

void UserOptions::on_circleStrokeColorButton_clicked()
{
    QColorDialog dlg;
    QColor color = dlg.getColor();
    QString style;
    style.sprintf( "QPushButton { background-color: rgb(%d, %d, %d); }", color.red(), color.green(), color.blue() );
    ui->circleStrokeColorButton->setStyleSheet( style );
    repaint();
}

void UserOptions::on_upButton_clicked()
{
    emit SigMoveCamera(UserOptions::CMT_UP);
}

void UserOptions::on_leftButton_clicked()
{
    emit SigMoveCamera(UserOptions::CMT_LEFT);
}

void UserOptions::on_rightButton_clicked()
{
    emit SigMoveCamera(UserOptions::CMT_RIGHT);
}

void UserOptions::on_downButton_clicked()
{
    emit SigMoveCamera(UserOptions::CMT_DOWN);
}
