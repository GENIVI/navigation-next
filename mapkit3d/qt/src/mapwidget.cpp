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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */


#include "mapwidget.h"
#include "avatarimpl.h"
#include "mapprojectionimpl.h"

using namespace locationtoolkit;

MapWidget::MapWidget()
    : mMapWidgetInternal(NULL),
      mAvatar(NULL),
      mMapProjection(NULL)
{
    setFlag(ItemHasContents, true);
}

MapWidget::~MapWidget()
{
    delete mMapWidgetInternal;
    delete mAvatar;
    delete mMapProjection;
}

/*! Initialize the MapWidget. */
void MapWidget::Initialize(const MapOptions& mapConfigration, LTKContext& ltkContext)
{
    if(!mMapWidgetInternal)
    {
        mMapWidgetInternal  = new MapWidgetInternal(mapConfigration, ltkContext, this);
        mMapWidgetInternal->CreateMap();
        if(!mAvatar)
        {
             mAvatar = new AvatarImpl(mMapWidgetInternal);
        }
        if( !mMapProjection )
        {
            mMapProjection = new MapProjectionImpl(mMapWidgetInternal);
        }

        connect(mMapWidgetInternal,
                SIGNAL(MapCreated()),
                SIGNAL(MapCreated()) );
        connect(mMapWidgetInternal,
                SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)),
                SIGNAL(MapLongClicked(const locationtoolkit::Coordinates&)) );
        connect(mMapWidgetInternal,
                SIGNAL(PolylineClicked(const QList<locationtoolkit::Polyline*>)),
                SIGNAL(PolylineClicked(const QList<locationtoolkit::Polyline*>)) );
        connect(mMapWidgetInternal,
                SIGNAL(LayersCreated()),
                SIGNAL(LayersCreated()) );
        connect(mMapWidgetInternal,
                SIGNAL(CameraUpdated(locationtoolkit::CameraParameters)),
                SIGNAL(CameraUpdated(locationtoolkit::CameraParameters)) );
        connect(mMapWidgetInternal,
                SIGNAL(CameraAnimationDone(int,int)), this,
                SLOT(OnInternalCameraAnimationDone(int,int)) );
        connect(mMapWidgetInternal,
                SIGNAL(Unlocked()), SIGNAL(Unlocked()) );
        connect(mMapWidgetInternal,
                SIGNAL(PinClicked(const locationtoolkit::Pin*)),
                SIGNAL(PinClicked(const locationtoolkit::Pin*)) );
        connect(mMapWidgetInternal,
                SIGNAL(MapClicked(locationtoolkit::Coordinates)),
                SIGNAL(MapClicked(locationtoolkit::Coordinates)) );
        connect(mMapWidgetInternal,
                SIGNAL(LayerOptionButtonClicked()),
                SIGNAL(LayerOptionButtonClicked()) );
        connect(mMapWidgetInternal,
                SIGNAL(NightModeChanged(bool)),
                SIGNAL(NightModeChanged(bool)) );
        connect(mMapWidgetInternal,
                SIGNAL(Gesture(int,int,int)),
                this,
                SLOT(OnGesture(int,int,int)) );
        connect(mMapWidgetInternal,
                SIGNAL(StaticPOIClicked(const QString&, const QString&, double, double)),
                this,
                SLOT(OnStaticPOIClicked(const QString&, const QString&, double, double)) );
        connect(mMapWidgetInternal,
                SIGNAL(CompassClicked(float)),
                this,
                SIGNAL(CompassClicked(float)) );
    }
}

/*! Sets a camera without animation for the map. */
void MapWidget::MoveTo(const CameraParameters& camera)
{
    mMapWidgetInternal->MoveTo(camera);
}

/*! Sets a camera with animation for the map. */
void MapWidget::AnimateTo(const CameraParameters& camera, const AnimationParameters& animation)
{
    mMapWidgetInternal->AnimateTo(camera, animation);
}

/*! Convenient method for AnimateTo */
void MapWidget::AnimateTo(const Coordinates& pos)
{
    mMapWidgetInternal->AnimateTo(pos);
}

/*! Gets the current position of the camera. */
bool MapWidget::GetCameraPosition(CameraParameters& camerapara)
{
    return mMapWidgetInternal->GetCameraPosition(camerapara);
}

/*! Creates a camera factory. */
//bool MapWidget::CreateCameraFactory(CameraFactory& cameraFactory)
//{
//
//}

/*! Prefetches map tiles based on camera parameters. */
void MapWidget::Prefetch(const CameraParameters& cameraParam)
{
    mMapWidgetInternal->Prefetch(cameraParam);
}

void MapWidget::Prefetch(QVector<Coordinates> points, const PrefetchConfiguration& configuration)
{
    mMapWidgetInternal->Prefetch(points, configuration);
}

/* ----------------------Implement map options ---------------- */
/*! Sets the night mode for the map. */
void MapWidget::SetNightMode(NightMode mode)
{
    mMapWidgetInternal->SetNightMode((uint)mode);
}

/*! Checks whether the map is in night mode. */
bool MapWidget::IsNightMode()
{
    return mMapWidgetInternal->IsNightMode();
}

/*! Sets the Gps mode for the map. */
void MapWidget::SetGpsMode(GPSMode mode, bool animated)
{
    switch (mode)
    {
    case GM_STANDBY:
        {
            mMapWidgetInternal->SetGpsMode(NGM_STAND_BY, animated);
        }
        break;
    case GM_FOLLOW_ME_ANY_HEADING:
        {
            mMapWidgetInternal->SetGpsMode(NGM_FOLLOW_ME_ANY_HEADING, animated);
        }
        break;
    case GM_FOLLOW_ME:
        {
            mMapWidgetInternal->SetGpsMode(NGM_FOLLOW_ME, animated);
        }
        break;
    default:
        break;
    }
}


/*! Gets the current GPS mode. */
MapWidget::GPSMode MapWidget::GetGpsMode()
{
    return (GPSMode)mMapWidgetInternal->GetGpsMode();
}

/*! Turns the specified layer on or off. */
void MapWidget::ShowOptionalLayer(QString layerName, bool show)
{
    mMapWidgetInternal->ShowOptionalLayer( layerName, show );
}

void MapWidget::ShowTrafficLayer(bool enable)
{
    mMapWidgetInternal->ShowTrafficLayer( enable );
}

Avatar& MapWidget::GetAvatar()
{
    return *mAvatar;
}

/* ----------------------Implement pin options ---------------- */
/*! Create a pin to this map. */
Pin* MapWidget::CreatePin(const PinParameters& para)
{
    return mMapWidgetInternal->CreatePin(para);
}

/*! Remove a pin. */
void MapWidget::RemovePin(Pin* pin)
{
    return mMapWidgetInternal->RemovePin(pin);
}

/*! Removes all the pins from the map. */
void MapWidget::DeleteAllPins()
{
    mMapWidgetInternal->DeleteAllPins();
}

void MapWidget::ShowDebugView(bool enable)
{
    mMapWidgetInternal->ShowDebugView( enable );
}

const QList<MapWidget::LayerNameAndEnabled> MapWidget::GetLayerNameAndStates()
{
    QList<QString> names;
    QList<bool>   enabels;
    mMapWidgetInternal->GetLayerNameAndStates( names, enabels );

    QList<MapWidget::LayerNameAndEnabled> output;
    for( int i = 0; i < names.size() && i < enabels.size(); i++ )
    {
        LayerNameAndEnabled obj;
        obj.name = names[i];
        obj.enabled = enabels[i];
        output.push_back( obj );
    }

    return output;
}

void MapWidget::ZoomToBoundingBox(double topLeftLatitude, double topLeftLongitude, double bottomRightLatitude, double bottomRightLongitude)
{
    mMapWidgetInternal->ZoomToBoundingBox( this->width(), this->height(),
                                           topLeftLatitude, topLeftLongitude,
                                           bottomRightLatitude, bottomRightLongitude );
}

/*! Make the specified section be shown in map within a screen area. */
void MapWidget::ZoomToBoundingBox(double topLeftLatitude,
                                  double topLeftLongitude,
                                  double bottomRightLatitude,
                                  double bottomRightLongitude,
                                  int xOffset,
                                  int yOffset,
                                  int width,
                                  int height)
{
    mMapWidgetInternal->ZoomToBoundingBox( this->width(), this->height(),
                                          topLeftLatitude, topLeftLongitude,
                                          bottomRightLatitude, bottomRightLongitude,
                                          xOffset, yOffset, width, height );
}


void MapWidget::resizeEvent(QResizeEvent* event)
{
    mMapWidgetInternal->ResizeEvent(event);
}
void MapWidget::Resize(const QSize& size)
{
    setSize(size);
    if (childItems().size() > 0)
    {
        childItems().front()->setSize(size);
    }
}
void MapWidget::OnInternalCameraAnimationDone(int cameraAnimationId, int animationStatus)
{
    AnimationStatus state = (AnimationStatus)animationStatus;
    emit CameraAnimationDone( cameraAnimationId, state );
}

void MapWidget::OnGesture(int type, int state, int time)
{
    emit Gesture((MapWidget::GestureType)type, (MapWidget::GestureState)state, time);
}

/*! Adds a polyline to this map.
 @param parameters A polyline parameters object which defines how to render this polyline.
 @return The Polyline object that was added to this map.
 */
Polyline& MapWidget::AddPolyline(const PolylineParameters& parameters)
{
    return mMapWidgetInternal->AddPolyline(parameters);
}

/*! Remove a polyline from map.
 @param a polyline object that is to be removed
 */
void MapWidget::RemovePolyline(Polyline *polyline)
{
    mMapWidgetInternal->RemovePolyline(polyline);
}

/*! Removes all pins, polylines, polygons, etc from the map.
 @return None
 */
void MapWidget::Clear()
{
    mMapWidgetInternal->Clear();
}

const MapProjection &MapWidget::GetMapProjection() const
{
    return *mMapProjection;
}

void MapWidget::EnableReferenceCenter(bool enable)
{
    mMapWidgetInternal->EnableReferencCenter( enable );
}

void MapWidget::SetReferenceCenter(const QPoint &position)
{
    mMapWidgetInternal->SetReferenceCenter( position );
}

void MapWidget::BeginAtomicUpdate()
{
    mMapWidgetInternal->BeginAtomicUpdate();
}

void MapWidget::EndAtomicUpdate()
{
    mMapWidgetInternal->EndAtomicUpdate();
}

MapDecoration* MapWidget::GetMapDecoration()
{
    return mMapWidgetInternal->GetMapDecoration();
}

/*! Sets the Mapview background mode */
void MapWidget::SetBackground(bool background)
{
    mMapWidgetInternal->SetBackground(background);
}

void MapWidget::SetDisplayScreen(int screenIndex)
{
    mMapWidgetInternal->SetDisplayScreen(screenIndex);
}

void MapWidget::SetDPI(float dpi)
{
    mMapWidgetInternal->SetDPI(dpi);
}

void MapWidget::OnStaticPOIClicked(const QString& id, const QString& name, double latitude, double longitude)
{
    qDebug() << "MapWidget::OnStaticPOIClicked()";
    emit StaticPOIClicked(id, name, latitude, longitude);
}
