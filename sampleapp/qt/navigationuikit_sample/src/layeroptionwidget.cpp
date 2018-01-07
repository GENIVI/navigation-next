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

#include "layeroptionwidget.h"

LayerOptionWidget::LayerOptionWidget(QWidget *parent, locationtoolkit::MapWidget *mapWidget) :
    QDeclarativeView(parent),
    mMapWidget(mapWidget),
    mFootprintsVisible(false),
    mWeatherVisible(false),
    mLandmarksVisible(false),
    mPointsOfInterestVisible(false),
    mSatelliteVisible(false),
    mTrafficVisible(false)
{
    const QList<locationtoolkit::MapWidget::LayerNameAndEnabled> layers = mMapWidget->GetLayerNameAndStates();
    for( int i = 0; i < layers.size(); i++ )
    {
        if( layers[i].name == "Building Footprints" )
        {
            mLayerTypeToName[ LAYER_FOOT_PRINTS ] = layers[i].name;
            mFootprintsChecked = layers[i].enabled ? true : false;
            mFootprintsVisible = true;
        }
        else if( layers[i].name == "Weather" )
        {
            mLayerTypeToName[ LAYER_WEATHER ] = layers[i].name;
            mWeatherChecked = layers[i].enabled ? true : false;
            mWeatherVisible = true;
        }
        else if( layers[i].name == "3D Landmarks & Buildings" )
        {
            mLayerTypeToName[ LAYER_lAND_MARK ] = layers[i].name;
            mLandmarksChecked = layers[i].enabled ? true : false;
            mLandmarksVisible = true;
        }
        else if( layers[i].name == "Points of Interest" )
        {
            mLayerTypeToName[ LAYER_POI ] = layers[i].name;
            mPointsOfInterestChecked = layers[i].enabled ? true : false;
            mPointsOfInterestVisible = true;
        }
        else if( layers[i].name == "Satellite" )
        {
            mLayerTypeToName[ LAYER_SATELLITE ] = layers[i].name;
            mSatelliteChecked = layers[i].enabled ? true : false;
            mSatelliteVisible = true;
        }
    }
    mLayerTypeToName[ LAYER_TRAFFIC ] = "Traffic";
    mTrafficChecked = false;
    mTrafficVisible = true;

    this->rootContext()->setContextProperty( "layerOptionModel", this );
    QDir dir = QFileInfo(QCoreApplication::applicationFilePath()).dir();
    QString appPath = dir.absolutePath();
    QString qmlFilePath = appPath + "/resource/qml/LayerOption.qml";
    setSource(QUrl::fromLocalFile(qmlFilePath));
    setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QObject* obj = (QObject *)rootObject();
    connect( obj, SIGNAL(layerOptionWidgetClosed()), this, SLOT(OnClose()) );
}

bool LayerOptionWidget::footprintsChecked() const
{
    return mFootprintsChecked;
}

bool LayerOptionWidget::weatherChecked() const
{
    return mWeatherChecked;
}

bool LayerOptionWidget::landmarksChecked() const
{
    return mLandmarksChecked;
}

bool LayerOptionWidget::pointsOfInterestChecked() const
{
    return mPointsOfInterestChecked;
}

bool LayerOptionWidget::satelliteChecked() const
{
    return mSatelliteChecked;
}

bool LayerOptionWidget::trafficChecked() const
{
    return mTrafficChecked;
}

bool LayerOptionWidget::footprintsVisible() const
{
    return mFootprintsVisible;
}

bool LayerOptionWidget::weatherVisible() const
{
    return mWeatherVisible;
}

bool LayerOptionWidget::landmarksVisible() const
{
    return mLandmarksVisible;
}

bool LayerOptionWidget::pointsOfInterestVisible() const
{
    return mPointsOfInterestVisible;
}

bool LayerOptionWidget::satelliteVisible() const
{
    return mSatelliteVisible;
}

bool LayerOptionWidget::trafficVisible() const
{
    return mTrafficVisible;
}

void LayerOptionWidget::setFootprintsChecked( bool b )
{
    mFootprintsChecked = b;
    mMapWidget->ShowOptionalLayer( mLayerTypeToName[LAYER_FOOT_PRINTS], b );
}

void LayerOptionWidget::setWeatherChecked( bool b )
{
    mWeatherChecked = b;
    mMapWidget->ShowOptionalLayer( mLayerTypeToName[LAYER_WEATHER], b );
}

void LayerOptionWidget::setLandmarksChecked( bool b )
{
    mLandmarksChecked = b;
    mMapWidget->ShowOptionalLayer( mLayerTypeToName[LAYER_lAND_MARK], b );
}

void LayerOptionWidget::setPointsOfInterestChecked( bool b )
{
    mPointsOfInterestChecked = b;
    mMapWidget->ShowOptionalLayer( mLayerTypeToName[LAYER_POI], b );
}

void LayerOptionWidget::setSatelliteChecked( bool b )
{
    mSatelliteChecked = b;
    mMapWidget->ShowOptionalLayer( mLayerTypeToName[LAYER_SATELLITE], b );
}

void LayerOptionWidget::setTrafficChecked( bool b )
{
    mTrafficChecked = b;
    mMapWidget->ShowTrafficLayer( b );
}

void LayerOptionWidget::OnClose()
{
    hide();
}
