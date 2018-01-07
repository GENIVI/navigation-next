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

#include "followmemapwidget.h"

using namespace locationtoolkit;
const int FollowMeMapWidget::mInterpolateTimerGap(20);

FollowMeMapWidget::FollowMeMapWidget(QWidget *parent) :
    MapContainer(parent),
    mPolylineReady(false)
{
    mToolBar = new ToolBarFollowMe( parent );
    mToolBar->setMaximumHeight( mToolBar->geometry().height() );

    mSegAttr = new QList<locationtoolkit::SegmentAttribute>();

    mLayout = new QVBoxLayout;
    mLayout->setMargin( 0 );
    mLayout->setSpacing( 0 );
    mLayout->addWidget( mToolBar );
    setLayout( mLayout );

    connect( &mInterpolatorTimer, SIGNAL(timeout()), this, SLOT(OnTimer()) );
}

void FollowMeMapWidget::SetDefaultGPS(QString gps)
{
    mDefaultGPS = gps;
}

void FollowMeMapWidget::LocationUpdated( const locationtoolkit::Location& location )
{
    if( mLastLocation.latitude > 90.0f )
    {
        mLastLocation = location;
    }
    else
    {
        mLatInterpolator.Set( 1000, mLastLocation.latitude, location.latitude );
        mLonInterpolator.Set( 1000, mLastLocation.longitude, location.longitude );
        mHeadingInterpolator.Set( 1000, mLastLocation.heading, location.heading );
        mLastLocation = location;

        mInterpolatorTime.start();
        if( !mInterpolatorTimer.isActive() )
        {
            mInterpolatorTimer.start( mInterpolateTimerGap );
        }

        if(!mPolylineReady)
        {
            locationtoolkit::Coordinates coord;
            coord.latitude = location.latitude;
            coord.longitude = location.longitude;
            mPolyPoints.append(coord);
            locationtoolkit::ColorSegment* segattr = new locationtoolkit::ColorSegment(mPolyPoints.count()-1, QColor(Qt::green));
            mSegAttr->append(*segattr);
        }
    }
}

void FollowMeMapWidget::ProviderStateChanged( LocationProviderState state )
{}

void FollowMeMapWidget::OnLocationError(int errorCode)
{
    mPolylineReady = QBool(true);
}

void FollowMeMapWidget::OnTimer()
{
    locationtoolkit::Location pos( mLastLocation );
    pos.latitude = mLatInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.longitude = mLonInterpolator.GetLinearValue( mInterpolatorTime.elapsed() );
    pos.heading = mHeadingInterpolator.GetAngleValue( mInterpolatorTime.elapsed() );

    mMapWidget->GetAvatar().SetLocation( pos );
}

void FollowMeMapWidget::OnPolylineClick(const QList<Polyline *> polyline)
{
    if(!polyline.empty())
    {
        for(int i = 0; i< polyline.size(); ++i)
        {
            locationtoolkit::Polyline* p = polyline.at(i);
            p->SetColor(QColor(Qt::blue));
        }
    }
}

void FollowMeMapWidget::StartFollowMe(QString gpsFile)
{
    mMapWidget->GetAvatar().SetMode(locationtoolkit::Avatar::AM_MAP);
    mMapWidget->SetNightMode(locationtoolkit::MapWidget::NM_AUTO);

    mMapWidget->RemoveAllPins();

    if(mLocationConfiguration.locationFilename != gpsFile)
    {
        mPolylineReady = QBool(false);
        mPolyPoints.clear();
        mSegAttr->clear();
    }
    mLocationConfiguration.emulationMode = QBool(true);
    mLocationConfiguration.locationFilename = gpsFile;

    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( mLocationConfiguration );

    if(!mPolylineReady)
    {
        mPolylineReady = QBool( false );
        while(!mPolylineReady)
        {
            locProvider.GetOneFix( static_cast<locationtoolkit::LocationListener&>(*this), Location::LFT_Normal);
        }
        mPolyPoints.removeLast();
        mSegAttr->removeLast();
    }

    if(mPolylineReady && (!mPolyPoints.empty()))
    {
        int polylineWidth = 10;
        CapParameter mStartCap;
        mStartCap.type = locationtoolkit::CPT_Round;
        mStartCap.radius = polylineWidth /2;
        mStartCap.width = 0;
        mStartCap.length = 0;
        CapParameter mEndCap;
        mEndCap.type = locationtoolkit::CPT_Round;
        mEndCap.radius = polylineWidth /2;
        mEndCap.width = 0;
        mEndCap.length = 0;
        PolylineParameters para;

        para.SetPoints(mPolyPoints).SetSegmentAttributes(*mSegAttr).SetUnhighlightColor(QColor(Qt::green)).SetWidth(polylineWidth).SetZOrder(16).SetVisible(true).SetStartCap(mStartCap).SetEndCap(mEndCap).setOutlineColor(QColor(Qt::gray)).setOutlineWidth(5);
        mMapWidget->AddPolyline(para);
    }


    locProvider.StartReceivingFixes( static_cast<locationtoolkit::LocationListener&>(*this) );
    mMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME);
    mLastLocation.latitude = 100.0f;
}

void FollowMeMapWidget::SetMapWidget(MapWidget *mapwidget, const QString &gpsfile)
{
    MapContainer::SetMapWidget( mapwidget, gpsfile );

    connect( mMapWidget, SIGNAL(PolylineClick(const QList<locationtoolkit::Polyline*>)),
            this, SLOT(OnPolylineClick(const QList<locationtoolkit::Polyline*> )) );
    if( mDefaultGPS.length() > 0 )
    {
        StartFollowMe( mDefaultGPS );
    }
}

void FollowMeMapWidget::ReleaseMapWidget()
{
    MapContainer::ReleaseMapWidget();
    if( mMapWidget )
    {
        StopFollowMe();
    }
    mLastLocation.latitude = 100.0f;
}

void FollowMeMapWidget::StopFollowMe()
{
    mMapWidget->RemoveAllPins();
    locationtoolkit::LocationProvider& locProvider = locationtoolkit::LocationProvider::GetInstance( mLocationConfiguration );
    locProvider.StopReceivingFixes( static_cast<locationtoolkit::LocationListener&>(*this) );
    mMapWidget->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY);
    mInterpolatorTimer.stop();
}
