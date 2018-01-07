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

#include "poilistmapwidget.h"
#include <QDir>
#include <QCoreApplication>

PoiListMapWidget::PoiListMapWidget(QWidget *parent) :
    MapContainer(parent)
{
    mToolBar = new ToolBarPoiList( parent );
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mLayout = new QVBoxLayout;
    mLayout->setMargin( 0 );
    mLayout->setSpacing( 0 );
    mLayout->addWidget( mToolBar );
    setLayout( mLayout );

    InvalidPoiPinLimits();

    connect( mToolBar, SIGNAL(SigAddPOIListPin(qreal,qreal)), this, SLOT(OnAddPOIListPin(qreal,qreal)) );
    connect( mToolBar, SIGNAL(SigRemovePoiPins()), this, SLOT(OnRemovePoiPins()) );
    connect( mToolBar, SIGNAL(SigShowPoiPins()), this, SLOT(OnShowPoiPins()) );
}

void PoiListMapWidget::OnAddPOIListPin(qreal latitude, qreal longtitude)
{
    AddPoiListPin( latitude, longtitude );
    MoveCamera( latitude, longtitude );
}

void PoiListMapWidget::OnRemovePoiPins()
{
    InvalidPoiPinLimits();
    mMapWidget->DeleteAllPins();
}

void PoiListMapWidget::OnShowPoiPins()
{
    if( mMinPoiPinLat < 90.0 )
    {
        mMapWidget->ZoomToBoundingBox( mMinPoiPinLat, mMinPoiPinLon, mMaxPoiPinLat, mMaxPoiPinLon );
    }
}

void PoiListMapWidget::MoveCamera(qreal lat, qreal lon)
{
    double movelat = lat;
    double movelon = lon;

    locationtoolkit::Coordinates coordin( movelat, movelon );
    locationtoolkit::CameraParameters param( coordin );
    mMapWidget->GetCameraPosition( param );
    param.SetPosition( coordin );
    mMapWidget->MoveTo( param );
}

void PoiListMapWidget::AddPoiListPin(qreal lat, qreal lon)
{
    if( mMinPoiPinLat > 90.0 )
    {
        mMinPoiPinLat = mMaxPoiPinLat = lat;
        mMinPoiPinLon = mMaxPoiPinLon = lon;
    }
    else
    {
        mMinPoiPinLat = mMinPoiPinLat < lat ? mMinPoiPinLat : lat;
        mMaxPoiPinLat = mMaxPoiPinLat > lat ? mMaxPoiPinLat : lat;
        mMinPoiPinLon = mMinPoiPinLon < lon ? mMinPoiPinLon : lon;
        mMaxPoiPinLon = mMaxPoiPinLon > lon ? mMaxPoiPinLon : lon;
    }

    double cameralat = lat;
    double cameralon = lon;

    locationtoolkit::Coordinates coordin( cameralat, cameralon );
    locationtoolkit::CameraParameters param( coordin );

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

    locationtoolkit::PinParameters pinpara(coordin,
                                          selectedImage,
                                          unSelectedImage,
                                          radiusPara,
                                          title,
                                          subtitle,
                                          bubble,
                                          visible);
    mMapWidget->CreatePin(pinpara);
}

void PoiListMapWidget::InvalidPoiPinLimits()
{
    mMinPoiPinLat = 100.0;
    mMaxPoiPinLat = 100.0;
    mMinPoiPinLon = 200.0;
    mMaxPoiPinLon = 200.0;
}
