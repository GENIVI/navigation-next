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

#include "optionallayersmapwidget.h"
#include <QMessageBox>

float OptionalLayersMapWidget::mMaxmunDopplerZoomLevel = 14.0;

OptionalLayersMapWidget::OptionalLayersMapWidget(locationtoolkit::MapWidget *mapWidget, QWidget *parent) :
    MapContainer(parent)
{
    mMapWidget = mapWidget;
    mToolBar = new ToolBarOptionalLayers( parent );
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mLayout = new QVBoxLayout;
    mLayout->setMargin( 0 );
    mLayout->setSpacing( 0 );
    mLayout->addWidget( mToolBar );
    setLayout( mLayout );

    connect( mToolBar, SIGNAL(SigShowLayer(QString,QBool)), this, SLOT(OnShowLayer(QString,QBool)) );
    connect( mMapWidget,SIGNAL(LayersCreated()), this, SLOT(OnLayerCreated()) );
}

void OptionalLayersMapWidget::ReleaseMapWidget()
{
    MapContainer::ReleaseMapWidget();
    if( mMapWidget != NULL )
    {
        disconnect( mMapWidget,SIGNAL(LayersCreated()), this, SLOT(OnLayerCreated()) );
    }
}

void OptionalLayersMapWidget::OnShowLayer(QString layerName, QBool show)
{
    if( layerName == "Traffic" )
    {
        mMapWidget->ShowTrafficLayer( show );
    }
    else
    {
        mMapWidget->ShowOptionalLayer( layerName, show );
    }
}

void OptionalLayersMapWidget::OnLayerCreated()
{
    const QList<locationtoolkit::MapWidget::LayerNameAndEnabled> layers = mMapWidget->GetLayerNameAndStates();
     for( int i = 0; i < layers.size(); i++ )
     {
         mToolBar->AddLayerList( layers[i].name, layers[i].enabled );
     }
     // add the traffic layer
     mToolBar->AddLayerList( "Traffic", QBool(false) );
}
