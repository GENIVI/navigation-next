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

#include "cameramapwidget.h"

#define CAMERA_MOVE_STEP 100

CameraMapWidget::CameraMapWidget(QWidget *parent) :
    MapContainer(parent),
    mAniParam( locationtoolkit::AnimationParameters::AC_Deceleration, 1000 )
{
    mToolBar = new ToolBarCamera( parent );
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mLayout = new QVBoxLayout;
    mLayout->setMargin( 0 );
    mLayout->setSpacing( 0 );
    mLayout->addWidget( mToolBar );
    setLayout( mLayout );

    double lat = 33.76985;
    double lon = -118.19302;
    locationtoolkit::Coordinates coord1( lat, lon );
    mSydniCamera = new locationtoolkit::CameraParameters( coord1, 15.5f, 50.f, 300.f );

    lat = 40.7160;
    lon = -74.0028;
    locationtoolkit::Coordinates coord2( lat, lon );
    mBondiCamera = new locationtoolkit::CameraParameters( coord2, 15.5f, 25.f, 0.f );

    connect( mToolBar, SIGNAL(CameraActivity(ToolBarCamera::CameraActivityType,QBool)), this, SLOT(OnCameraActivity(ToolBarCamera::CameraActivityType,QBool)) );
    connect( mToolBar, SIGNAL(MoveCameraTo(QString,QBool)), this, SLOT(OnMoveCameraTo(QString,QBool)) );
}

CameraMapWidget::~CameraMapWidget()
{
    if( mSydniCamera )
    {
        delete mSydniCamera;
    }
    if( mBondiCamera )
    {
        delete mBondiCamera;
    }
}

void CameraMapWidget::OnCameraActivity(ToolBarCamera::CameraActivityType type, QBool isAnimate)
{
    int x = mMapWidget->rect().width() / 2;
    int y = mMapWidget->rect().height() / 2;

    double d = 0.0;
    locationtoolkit::Coordinates coordin( d, d );
    locationtoolkit::CameraParameters param( coordin );
    mMapWidget->GetCameraPosition( param );

    const locationtoolkit::MapProjection& projection = mMapWidget->GetMapProjection();

    switch ( type ) {
    case ToolBarCamera::ACTIVITY_UP:
        y -= CAMERA_MOVE_STEP;
        projection.FromScreenPosition( x, y, coordin.latitude, coordin.longitude );
        param.SetPosition( coordin );
        break;
    case ToolBarCamera::ACTIVITY_DOWN:
        y += CAMERA_MOVE_STEP;
        projection.FromScreenPosition(  x, y, coordin.latitude, coordin.longitude );
        param.SetPosition( coordin );
        break;
    case ToolBarCamera::ACTIVITY_LEFT:
        x -= CAMERA_MOVE_STEP;
        projection.FromScreenPosition(  x, y, coordin.latitude, coordin.longitude );
        param.SetPosition( coordin );
        break;
    case ToolBarCamera::ACTIVITY_RIGHT:
        x += CAMERA_MOVE_STEP;
        projection.FromScreenPosition(  x, y, coordin.latitude, coordin.longitude );
        param.SetPosition( coordin );
        break;
    case ToolBarCamera::ACTIVITY_ZOOM_IN:
        param.SetZoomLevel( param.GetZoomLevel() + 1 );
        break;
    case ToolBarCamera::ACTIVITY_ZOOM_OUT:
        param.SetZoomLevel( param.GetZoomLevel() - 1 );
        break;

    default:
        break;
    }

    if( isAnimate == true )
    {
        mMapWidget->AnimateTo( param, mAniParam );
    }
    else
    {
        mMapWidget->MoveTo( param );
    }
}

void CameraMapWidget::OnMoveCameraTo(QString place, QBool isAnimate)
{
    locationtoolkit::CameraParameters* param = mSydniCamera;
    if( place == "Bondi" )
    {
        param = mBondiCamera;
    }

    if( isAnimate == true )
    {
        mMapWidget->AnimateTo( *param, mAniParam );
    }
    else
    {
        mMapWidget->MoveTo( *param );
    }
}
