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

#include "mapcontainer.h"

MapContainer::MapContainer(QWidget *parent) :
    QWidget(parent),
    mMapWidget( NULL ),
    mLayout(NULL)
{
}

void MapContainer::SetMapWidget(locationtoolkit::MapWidget *mapwidget, const QString &gpsfile)
{
    mMapWidget = mapwidget;
    if( mLayout != NULL && mapwidget != NULL )
    {
        mLayout->addWidget( mapwidget );
    }

    locationtoolkit::LocationConfiguration config;
    config.emulationMode = QBool( true );
    config.locationFilename = gpsfile;
    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( config );
    locProvider.GetOneFix( static_cast<locationtoolkit::LocationListener&>(*this), locationtoolkit::Location::LFT_Normal );

    // reset location provider
    config.locationFilename = "";
    locationtoolkit::LocationProvider::GetInstance( config );
}

void MapContainer::ReleaseMapWidget()
{
    if( mLayout != NULL && mMapWidget != NULL )
    {
        mLayout->removeWidget( mMapWidget );
    }
}

void MapContainer::LocationUpdated(const locationtoolkit::Location &location)
{
    if( mMapWidget )
    {
        double movetolat = location.latitude;
        double movetolon = location.longitude;
        locationtoolkit::Coordinates coordin( movetolat, movetolon );
        locationtoolkit::CameraParameters param( coordin );
        mMapWidget->GetCameraPosition( param );
        param.SetPosition( coordin );
        mMapWidget->MoveTo( param );
    }
}

void MapContainer::ProviderStateChanged(locationtoolkit::LocationListener::LocationProviderState state)
{}

void MapContainer::OnLocationError(int errorCode)
{}
