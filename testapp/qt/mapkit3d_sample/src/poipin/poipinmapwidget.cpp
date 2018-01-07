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

#include "poipinmapwidget.h"
#include "coordinate.h"
#include <QDir>
#include <QCoreApplication>

PoiPinMapWidget::PoiPinMapWidget(QWidget *parent) :
    MapContainer(parent),
    mToolBar(NULL)
{
    mToolBar = new ToolBarPoiPin( parent );
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mLayout = new QVBoxLayout;
    mLayout->setMargin( 0 );
    mLayout->setSpacing( 0 );
    mLayout->addWidget( mToolBar );
    setLayout( mLayout );
}

void PoiPinMapWidget::SetMapWidget(locationtoolkit::MapWidget *mapwidget, const QString &gpsfile)
{
    MapContainer::SetMapWidget( mapwidget, gpsfile );

    connect(mMapWidget, SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)),
            this, SLOT(OnMapLongClick(const locationtoolkit::Coordinates&)) );
}

void PoiPinMapWidget::ReleaseMapWidget()
{
    MapContainer::ReleaseMapWidget();

    if( mMapWidget != NULL )
    {
        disconnect( mMapWidget, SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)),
                    this, SLOT(OnMapLongClick(const locationtoolkit::Coordinates&)) );
    }
}

void PoiPinMapWidget::OnMapLongClick(const locationtoolkit::Coordinates &coordinate)
{
    mMapWidget->RemoveAllPins();

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
    mMapWidget->AddPin(pinpara);
}
