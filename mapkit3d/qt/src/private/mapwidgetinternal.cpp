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

#include "mapwidgetinternal.h"
#include "ltkcontextimpl.h"
#include "mapwidget.h"
#include "LayerAgent.h"
#include "GeoPolyline.h"
#include "patternsegment.h"
#include "defaultmapbubbleresolver.h"
#include "defaultbubble.h"
#include <QDir>
#include <QCoreApplication>
#include "camerafactory.h"
#include "latlngbound.h"
#include "MapViewProtocol.h"
#include "camerahelper.h"
#include "camera.h"

using namespace locationtoolkit;

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#define UNLOCK_CAMERA_SCALE_THRESHOLD 14
const char* MapWidgetInternal::ANTI_ALIASING_PROPERTY_NAME = "enableFullScreenAntiAliasing";
const char* MapWidgetInternal::ANISOTROPIC_PROPERTY_NAME = "enableAnisotropicFiltering";
const char* MapWidgetInternal::MAPVIEW_ONLY_PROPERTY_NAME = "mapviewOnly";

const float MapWidgetInternal::FOLLOW_ME_MODE_ZOOM_LEVEL = 17.6;
const float MapWidgetInternal::FOLLOW_ME_MODE_TILT_ANGLE = 38.19;
const float MapWidgetInternal::FOLLOW_ME_ANDHEADING_ZOOM_LEVEL = 17.6f;
const int MapWidgetInternal::GPS_MODE_SWITCH_ANIMATION_TIME_SHORT = 480;
const int MapWidgetInternal::GPS_MODE_SWITCH_ANIMATION_TIME_LONG = 620;
const double MapWidgetInternal::INVALID_LATITUED = -999.;
const double MapWidgetInternal::INVALID_LONGTITUED = -999.;
const float MapWidgetInternal::INVALID_ZOOMLEVEL_VALUE = -999.f;
const float MapWidgetInternal::INVALID_HEADING_VALUE = -999.f;
const float MapWidgetInternal::INVALID_TILT_VALUE = -999.f;

const float MapWidgetInternal::FOLLOW_ME_MODE_FONT_SCALE = 1.5f;
const float MapWidgetInternal::DEFAULT_FONT_SCALE = 1.5f;

/*! constructor of MapWidgetInternal. */
MapWidgetInternal::MapWidgetInternal(const MapOptions& mapOptions, LTKContext& ltkContext, QQuickItem* parent):
    mParentWindow(parent),
    mMapController(NULL),
    mGeoPolyline(NULL),
    mLtkContext(ltkContext),
    mMapOptions(mapOptions),
    mIsCameraPositionLocked( false ),
    mDecorationSignals( NULL ),
    mMapDecoration(NULL),
    mEnableReferenceCenter(false)
{
    mZoomLevel = INVALID_ZOOMLEVEL_VALUE;
}

/*! destructor of MapWidgetInternal. */
MapWidgetInternal::~MapWidgetInternal()
{
    // @todo fixed bug: Bug 187290 - QT Mapkit3D Win32 sample app run time exception when close the main window
    //       warning:  fixed temporarily. if not destroy map, there would be memory leak when mapWidget was created frequently.
    DeleteAllPins();
    if(mMapController)
    {
        mMapController->Destroy();
        delete mMapController;
    }

    if(mMapDecoration)
    {
        delete mMapDecoration;
    }
}

/*! call map create interface of CCC */
void MapWidgetInternal::CreateMap()
{
    if(!mMapController)
    {
        LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( mLtkContext );
        mMapController = new nbmap::MapController( contextImpl.GetNBContext(),
                                                   contextImpl.GetPalInstance());
        mMapController->SetMapListener( this );
        nbmap::MapViewConfiguration mapViewConfig;

        mapViewConfig.defaultLatitude                 = mMapOptions.mDefaultLatitude;
        mapViewConfig.defaultLongitude                = mMapOptions.mDefaultLongitude;
        mapViewConfig.defaultTiltAngle                = mMapOptions.mDefaultTiltAngle;
        mapViewConfig.maximumCachingTileCount         = mMapOptions.mMaximumCachingTileCount;
        mapViewConfig.maximumTileRequestCountPerLayer = mMapOptions.mMaximumTileRequestCountPerLayer;
        mapViewConfig.workFolder                      = mMapOptions.mWorkFolder.toStdString();
        mapViewConfig.resourceFolder                  = mMapOptions.mResourceFolder.toStdString();
        mapViewConfig.zorderLevel                     = mMapOptions.mZoomLevel;
        mapViewConfig.enableSSAO                      = mMapOptions.mEnableSSAO;
        mapViewConfig.enableGlow                      = mMapOptions.mEnableGlowRoute;
        if(mMapOptions.mLanguageCode.isEmpty())
        {
            mapViewConfig.languageCode                = contextImpl.GetLanguage().toStdString();
        }
        else
        {
            mapViewConfig.languageCode                = mMapOptions.mLanguageCode.toStdString();
        }

        if(!mMapOptions.mProductClass.isEmpty())
        {
            mapViewConfig.productClass                = mMapOptions.mProductClass.toStdString();
        }

//        mParentWindow->setProperty( ANTI_ALIASING_PROPERTY_NAME, mMapOptions.mEnableFullScreenAntiAliasing );
//        mParentWindow->setProperty( ANISOTROPIC_PROPERTY_NAME, mMapOptions.mEnableAnisotropicFiltering );
//        mParentWindow->setProperty(MAPVIEW_ONLY_PROPERTY_NAME, mMapOptions.mMapviewOnly);

        MapViewProtocol* protocol = new MapViewProtocol(this, mParentWindow);
        mMapController->Create(mapViewConfig, protocol);
        mMapDecoration = new MapDecoration;
        connect( mMapDecoration, SIGNAL(CompassEnabledChanged(bool)), this, SLOT(OnCompassEnabledChanged(bool)) );
        connect( mMapDecoration, SIGNAL(LocateMeButtonEnabledChanged(bool)), this, SLOT(OnLocateMeButtonEnabledChanged(bool)) );
        connect( mMapDecoration, SIGNAL(ZoomButtonEnabledChanged(bool)), this, SLOT(OnZoomButtonEnabledChanged(bool)) );
        connect( mMapDecoration, SIGNAL(LayerOptionButtonEnabledChanged(bool)), this, SLOT(OnLayerOptionButtonEnabledChanged(bool)) );
        connect( mMapDecoration, SIGNAL(CompassPathChanged(const QString&, const QString&)), this, SLOT(OnCompassPathChanged(const QString&, const QString&)) );
        connect( mMapDecoration, SIGNAL(CompassPositionChanged(float, float)), this, SLOT(OnCompassPositionChanged(float, float)) );
    }
}

/*! Sets a camera without animation for the map. */
void MapWidgetInternal::MoveTo(const CameraParameters& camera) //First implement
{
    mMapController->SetCamera(camera.GetPosition().latitude,
                              camera.GetPosition().longitude,
                              camera.GetZoomLevel(),
                              camera.GetHeadingAngle(),
                              camera.GetTiltAngle(),
                              FALSE,
                              0,
                              nbmap::CAAT_LINEAR,
                              0);
}

/*! Sets a camera with animation for the map. */
void MapWidgetInternal::AnimateTo(const CameraParameters& camera, const AnimationParameters& animation)
{
    mMapController->SetCamera(camera.GetPosition().latitude,
                              camera.GetPosition().longitude,
                              camera.GetZoomLevel(),
                              camera.GetHeadingAngle(),
                              camera.GetTiltAngle(),
                              TRUE,
                              animation.GetDuration(),
                              (nbmap::CameraAnimationAccelerationType)animation.GetAccelerationType(),
                              animation.GetAnimationId());
}

void MapWidgetInternal::AnimateTo(const Coordinates& pos)
{
    CameraParameters param( pos );
    GetCameraPosition( param );
    param.SetPosition(pos);

    AnimationParameters aniParam( AnimationParameters::AC_Deceleration, 1000 );
    AnimateTo( param, aniParam );
}

/*! Gets the current position of the camera. */
bool MapWidgetInternal::GetCameraPosition(CameraParameters& camerapara) const
{
    double latitude = 0.0, longitude = 0.0;
    float zoomLevel = 0.0, headingAngle = 0.0, tiltAngle = 0.0;
    if (mMapController->GetCamera(latitude, longitude, zoomLevel,headingAngle,tiltAngle))
    {
        Coordinates pos(latitude, longitude);
        camerapara.SetPosition(pos).SetZoomLevel(zoomLevel).SetHeadingAngle(headingAngle).SetTiltAngle(tiltAngle);
        return true;
    }
    else
    {
        return false;
    }
}

void MapWidgetInternal::Prefetch(const CameraParameters& cameraParam)
{
    mMapController->Prefetch(cameraParam.GetPosition().latitude,
                             cameraParam.GetPosition().longitude,
                             cameraParam.GetZoomLevel(),
                             cameraParam.GetHeadingAngle(),
                             cameraParam.GetTiltAngle());
}

void MapWidgetInternal::Prefetch(QVector<Coordinates> points, const PrefetchConfiguration& configuration)
{
    shared_ptr<vector<pair<double, double> > > vecPoints =
            shared_ptr<vector<pair<double, double> > >(new vector<pair<double, double> >);
    foreach (Coordinates point, points)
    {
        vecPoints->push_back(pair<double, double>(point.latitude, point.longitude));
    }
    mMapController->Prefetch(vecPoints,
                             configuration.extensionLengthMeters,
                             configuration.extensionWidthMeters,
                             configuration.zoomLevel);
}

void MapWidgetInternal::LockCameraPosition(bool lock)
{
    mIsCameraPositionLocked = lock;
}

void MapWidgetInternal::SetAvatarLocation(const Location& avatarLocation)
{
    NB_GpsLocation location;

    location.altitude = avatarLocation.altitude;
    location.status = (NB_Error)avatarLocation.status;
    location.gpsTime = avatarLocation.gpsTime;
    location.valid = avatarLocation.valid;
    location.latitude = avatarLocation.latitude;
    location.longitude = avatarLocation.longitude;
    location.heading = avatarLocation.heading;
    location.horizontalVelocity = avatarLocation.horizontalVelocity;
    location.altitude = avatarLocation.altitude;
    location.verticalVelocity = avatarLocation.verticalVelocity;
    location.horizontalUncertaintyAngleOfAxis = avatarLocation.horizontalUncertaintyAngleOfAxis;
    location.horizontalUncertaintyAlongAxis = avatarLocation.horizontalUncertaintyAlongAxis;
    location.horizontalUncertaintyAlongPerpendicular = avatarLocation.horizontalUncertaintyAlongPerpendicular;
    location.verticalUncertainty = avatarLocation.verticalUncertainty;
    location.utcOffset = avatarLocation.utcOffset;
    location.numberOfSatellites = avatarLocation.numberOfSatellites;
    location.gpsHeading = avatarLocation.gpsHeading;
    location.compassHeading = avatarLocation.compassHeading;
    location.compassAccuracy = avatarLocation.compassAccuracy;

    if(mEnableReferenceCenter && (mGpsMode == NGM_FOLLOW_ME_ANY_HEADING ||mGpsMode == NGM_FOLLOW_ME) )
    {
        double latitude;
        double longitude;
        float rotateAngle;
        float tiltAngle;
        float zoomLevel;
        if(mMapController->GetCamera(latitude, longitude, zoomLevel, rotateAngle, tiltAngle))
        {
            nbmap::Camera camera;
            if(mGpsMode == NGM_FOLLOW_ME_ANY_HEADING)
            {
                camera.SetRotateAngle(rotateAngle);
            }
            else
            {
                camera.SetRotateAngle(location.heading);
            }
            camera.SetViewCenter(location.latitude, location.longitude);
            camera.SetTiltAngle(tiltAngle);
            camera.SetZoomLevel(zoomLevel);

            nbmap::Camera scrolledCamera;
            nbmap::CameraHelper::ScrollCamera(camera,
                                              mParentWindow->width(), mParentWindow->height(),
                                              mReferenceCenter.x(), mReferenceCenter.y(),
                                              scrolledCamera);

            scrolledCamera.GetViewCenter(latitude, longitude);
            tiltAngle = scrolledCamera.GetTiltAngle();
            zoomLevel = scrolledCamera.GetZoomLevel();
            rotateAngle = scrolledCamera.GetRotateAngle();

            BeginAtomicUpdate();
            mMapController->SetCamera(latitude,
                                      longitude,
                                      zoomLevel,
                                      rotateAngle,
                                      tiltAngle,
                                      false,
                                      GPS_MODE_SWITCH_ANIMATION_TIME_SHORT,
                                      nbmap::CAAT_LINEAR);
            mMapController->SetAvatarLocation(location);
            EndAtomicUpdate();
        }
        else
        {
            mMapController->SetAvatarLocation(location);
        }
    }
    else
    {
        mMapController->SetAvatarLocation(location);
    }

}

void MapWidgetInternal::SetNightMode(uint mode)
{
    mMapController->SetNightMode(mode);
}

bool MapWidgetInternal::IsNightMode()
{
    return mMapController->IsNightMode();
}

void MapWidgetInternal::SetAvatarMode(int mode)
{
    mMapController->SetAvatarMode((MapViewAvatarMode)mode);
}

/*! Sets the scale value of the avatar. */
void MapWidgetInternal::SetAvatarScale(float scale)
{
    mMapController->SetAvatarScale(scale);
}

/*! Sets the Mapview background mode */
void MapWidgetInternal::SetBackground(bool background)
{
    mMapController->SetBackground(background);
}

void MapWidgetInternal::SetGpsMode(NB_GpsMode mode, bool animated)
{
    mGpsMode = mode;
    switch (mode)
    {
    case NGM_STAND_BY:
    {
        mMapController->SetGpsMode(mode);
        EnableReferencCenter(FALSE);
        break;
    }
    case NGM_FOLLOW_ME_ANY_HEADING:
    {
        mMapController->SetCamera(INVALID_LATITUED,
                                  INVALID_LONGTITUED,
                                  FOLLOW_ME_ANDHEADING_ZOOM_LEVEL,
                                  INVALID_HEADING_VALUE,
                                  90.f,
                                  animated,
                                  GPS_MODE_SWITCH_ANIMATION_TIME_SHORT,
                                  nbmap::CAAT_LINEAR);
        mMapController->SetGpsMode(mode);
        EnableReferencCenter(TRUE);
        break;
    }
    case NGM_FOLLOW_ME:
    {
        mMapController->SetCamera(INVALID_LATITUED,
                                  INVALID_LONGTITUED,
                                  FOLLOW_ME_MODE_ZOOM_LEVEL,
                                  INVALID_HEADING_VALUE,
                                  FOLLOW_ME_MODE_TILT_ANGLE,
                                  animated,
                                  GPS_MODE_SWITCH_ANIMATION_TIME_LONG,
                                  nbmap::CAAT_DECELERATION);
        mMapController->SetGpsMode(mode);
        EnableReferencCenter(TRUE);
        break;
    }
    default:
    {
        EnableReferencCenter(FALSE);
        break;
    }
    }
}

bool MapWidgetInternal::IsCameraLocked() const
{
    return mIsCameraPositionLocked;
}

void MapWidgetInternal::UnLockCameraPosition()
{
    mIsCameraPositionLocked = false;
    emit Unlocked();
}


void MapWidgetInternal::SwitchGpsMode()
{
    switch (GetGpsMode())
    {
    case NGM_STAND_BY:
    {
        SetGpsMode(NGM_FOLLOW_ME_ANY_HEADING, true);
        break;
    }
    case NGM_FOLLOW_ME_ANY_HEADING:
    {
        SetGpsMode(NGM_FOLLOW_ME, true);
        break;
    }
    case NGM_FOLLOW_ME:
    {
        SetGpsMode(NGM_FOLLOW_ME_ANY_HEADING, true);
        break;
    }
    default:
    {
        break;
    }
    }
}

/*! Gets the current GPS mode. */
NB_GpsMode MapWidgetInternal::GetGpsMode() const
{
    return mMapController->GetGpsMode();
}

MapDecorationSignals& MapWidgetInternal::GetMapDecorationSignals()
{
    return mDecorationSignals;
}

void MapWidgetInternal::ZoomIn()
{
    Coordinates coordin;
    CameraParameters param( coordin );
    GetCameraPosition( param );
    param.SetZoomLevel( param.GetZoomLevel() + 1 );

    AnimationParameters aniParam( AnimationParameters::AC_Deceleration, 1000 );
    AnimateTo( param, aniParam );
}

void MapWidgetInternal::ZoomOut()
{
    Coordinates coordin;
    CameraParameters param( coordin );
    GetCameraPosition( param );
    param.SetZoomLevel( param.GetZoomLevel() - 1 );

    AnimationParameters aniParam( AnimationParameters::AC_Deceleration, 1000 );
    AnimateTo( param, aniParam );
}

void MapWidgetInternal::ShowLayerOption()
{
    emit LayerOptionButtonClicked();
}

void MapWidgetInternal::OnCompassClicked(float needleOrientation)
{
    emit CompassClicked(needleOrientation);
}

void MapWidgetInternal::stopAnimations(int type)
{
    mMapController->StopAnimations(type);
}

void MapWidgetInternal::ShowOptionalLayer(QString layerName, bool show)
{
    std::vector<nbmap::LayerAgentPtr> layerPtr;
    layerPtr = mMapController->GetOptionalLayers();

    std::vector<nbmap::LayerAgentPtr>::iterator it;
    for(it = layerPtr.begin(); it != layerPtr.end(); ++it)
    {
        nbmap::LayerAgentPtr& layerAgent = *it;
        QString curLayerName = QString::fromStdString( *layerAgent->GetName() );
        if( curLayerName == layerName )
        {
            layerAgent->SetEnabled(show);
            break;
        }
    }
}

void MapWidgetInternal::ShowTrafficLayer(bool enable)
{
    mMapController->ShowTraffic( enable );
}

Pin* MapWidgetInternal::CreatePin(const PinParameters& para)
{
    QString content;
    content.sprintf( "lat: %.3f, lon: %.3f", para.GetPosition().latitude, para.GetPosition().longitude );

    shared_ptr<BubbleInternal> bubbleImpl;
//    if(para.GetBubble())
//    {
//        bubbleImpl =  shared_ptr<BubbleInternal>(new BubbleInternal( para.GetBubble() ) );
//    }
//    else
//    {
//        QString bubbleImagePath = mMapOptions.mResourceFolder + "/images/map_bubble.9.png";
//        DefaultBubble* bubbleImp = new DefaultBubble( "Location:", content, bubbleImagePath, mParentWindow );
//        bubbleImpl =  shared_ptr<BubbleInternal>(new BubbleInternal( bubbleImp ) );
//    }

    PinImpl* pin = new PinImpl(this, this->GetMapController(), para.GetPosition(), para.GetSelectedImage(),
                                      para.GetUnselectedImage(), para.GetRadiusParameters(),
                                      para.GetTitle(), para.GetSubtitle(),
                                      bubbleImpl, para.IsVisible(), para.GetGroupId());
    mPinList.append(pin);
    return pin;
}

void MapWidgetInternal::RemovePin(Pin* pin)
{
    if(mPinList.contains(pin))
    {
        mPinList.removeOne(pin);
        delete pin;
    }
}

void MapWidgetInternal::DeleteAllPins()
{
    for(int i = 0; i < mPinList.count(); i++)
    {
        delete mPinList[i];
    }
    mPinList.clear();
}

nbmap::MapController& MapWidgetInternal::GetMapController() const
{
    return *mMapController;
}

nbmap::GeoPolyline* MapWidgetInternal::GetGeoPolyline() const
{
    return mGeoPolyline;
}

void MapWidgetInternal::GetLayerNameAndStates(QList<QString> &names, QList<bool> &enables)
{
    std::vector<nbmap::LayerAgentPtr> layerPtr;
    layerPtr = mMapController->GetOptionalLayers();

    std::vector<nbmap::LayerAgentPtr>::iterator it;
    for(it = layerPtr.begin(); it != layerPtr.end(); ++it)
    {
        nbmap::LayerAgentPtr& layerAgent = *it;
        names.push_back( QString::fromStdString( *layerAgent->GetName() ) );
        enables.push_back(layerAgent->IsEnabled());
    }
}

bool MapWidgetInternal::OnSingleTap(float /*screenX*/, float /*screenY*/, std::set<nbmap::Geographic*>& graphics)
{
    QList<Polyline*> tapPolylineList;
    if(!mPolylineMap.empty())
    {
        QList<nbmap::GeoPolyline*> polylinelist = mPolylineMap.keys();
        for (std::set<nbmap::Geographic*>::iterator it = graphics.begin(); it != graphics.end(); ++it)
        {
            nbmap::Geographic* rcvp = *it;
            for(int i = 0; i < polylinelist.count(); ++i)
            {
                nbmap::GeoPolyline* p = polylinelist.at(i);
                if(rcvp == p)
                {
                    PolylineImpl* polyImpl = mPolylineMap.value(p);
                    tapPolylineList.append(polyImpl);
                }
            }
        }
        if(!tapPolylineList.empty())
        {
            emit PolylineClicked(tapPolylineList);
            return true;
        }
    }
    return false;
}

void MapWidgetInternal::OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    Coordinates coord( lat, lon );
    CameraParameters param( coord, zoomLevel, tilt, heading );
    if(INVALID_ZOOMLEVEL_VALUE == mZoomLevel)
    {
        mZoomLevel = zoomLevel;
    }
    if(IsCameraLocked() && zoomLevel < UNLOCK_CAMERA_SCALE_THRESHOLD && mZoomLevel > zoomLevel)
    {
        UnLockCameraPosition();
    }
    mZoomLevel = zoomLevel;
    emit CameraUpdated( param );
}

void MapWidgetInternal::OnOptionalLayerUpdated(const std::vector<nbmap::LayerAgentPtr>& /*layers*/)
{
    emit LayersCreated();
}

void MapWidgetInternal::OnPinClicked(nbmap::MapPin* pin)
{
    for(int i = 0; i < mPinList.count(); i++)
    {
        PinImpl* pinImpl = static_cast<PinImpl*>(mPinList[i]);
        if(pinImpl->GetMapPin() == pin)
        {
            emit PinClicked(mPinList[i]);
            break;
        }
    }
}

void MapWidgetInternal::OnTrafficIncidentPinClicked(double /*latitude*/, double /*longitude*/)
{

}

void MapWidgetInternal::OnStaticPOIClicked(const std::string& id, const std::string& name, double latitude, double longitude)
{
    qDebug() << "LTK StaticPOI id: " << id.c_str();
    qDebug() << "LTK StaticPOI name: " << name.c_str();
    qDebug() << "LTK StaticPOI latitude: " << latitude;
    qDebug() << "LTK StaticPOI longitude: " << longitude;
    emit StaticPOIClicked(QString(id.c_str()), QString(name.c_str()), latitude, longitude);
}

void MapWidgetInternal::OnAvatarClicked(double /*lat*/, double /*lon*/)
{

}

void MapWidgetInternal::OnMapCreate()
{
    SetGpsMode(NGM_STAND_BY, false);
    mMapController->ShowCompass(true);
    mMapController->SetCompassPosition(mParentWindow->width()-100, 100);
//    mMapController->SetPinBubbleResolver( shared_ptr<nbmap::PinBubbleResolver>
//                                          ( new DefaultMapBubbleResolver(mMapOptions.mResourceFolder, mParentWindow) ) );
    mMapController->SetFontScale(DEFAULT_FONT_SCALE);
    emit MapCreated();
}

void MapWidgetInternal::OnMapReady()
{

}

void MapWidgetInternal::OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus)
{
    int aniState = (int)animationStatus;
    emit CameraAnimationDone( animationId, aniState );
}

void MapWidgetInternal::OnNightModeChanged(bool isNightMode)
{
    emit NightModeChanged(isNightMode);
}

void MapWidgetInternal::OnTap(double latitude, double longitude)
{
    Coordinates coord( latitude, longitude );
    emit MapClicked( coord );
}

void MapWidgetInternal::OnCameraLimited(MapViewCameraLimitedType /*type*/)
{

}

void MapWidgetInternal::OnMarkerClicked(int /*markerId*/)
{

}

/*! Called when the user makes a long-press gesture on the map. */
void MapWidgetInternal::OnLongPressed(double latitude, double longitude)
{
    Coordinates coordinate(latitude,longitude);
    emit MapLongClicked(coordinate);
}

void MapWidgetInternal::OnGesture(MapViewGestureType type, MapViewGestureState state, int time)
{
    emit Gesture((int)type, (int)state, time);
}

void MapWidgetInternal::ShowDebugView(bool enable)
{
    mMapController->EnableDebugView( enable );
}

void MapWidgetInternal::ResizeEvent(QResizeEvent * event)
{
    if((event->size().width() != -1)&&(event->size().height() != -1))
    {
        mMapController->SetCompassPosition(mParentWindow->width()-100, 100);
    }
}


void MapWidgetInternal::ZoomToBoundingBox(int width,
                                          int height,
                                          double topLeftLatitude,
                                          double topLeftLongitude,
                                          double bottomRightLatitude,
                                          double bottomRightLongitude,
                                          int xOffset,
                                          int yOffset,
                                          int viewportWidth,
                                          int viewportHeight)
{
    double centerLat = 0.0;
    double centerLon = 0.0;

    LatLngBound latlngBound;
    latlngBound.topLeftLatitude  = topLeftLatitude;
    latlngBound.topLeftLongitude = topLeftLongitude;
    latlngBound.bottomRightLatitude      = bottomRightLatitude;
    latlngBound.bottomRightLongitude     = bottomRightLongitude;

    QSharedPointer<CameraParameters>  cameraParameters =  CameraFactory::CreateCamera(width, height, latlngBound, xOffset, yOffset, viewportWidth, viewportHeight);
    if( cameraParameters != NULL )
    {
        centerLat = cameraParameters->GetPosition().latitude;
        centerLon = cameraParameters->GetPosition().longitude;
        float zoomlevel = cameraParameters->GetZoomLevel()-0.6;// cut down zoom level to avoid the pins be put on screen's edge

        mMapController->SetCamera( centerLat,
                                   centerLon,
                                   zoomlevel,
                                   cameraParameters->GetHeadingAngle(),
                                   cameraParameters->GetTiltAngle(),
                                   true,
                                   GPS_MODE_SWITCH_ANIMATION_TIME_LONG,
                                   nbmap::CAAT_LINEAR);
    }
}

void MapWidgetInternal::ZoomToBoundingBox(int width, int height, double topLeftLatitude, double topLeftLongitude, double bottomRightLatitude, double bottomRightLongitude)
{
    ZoomToBoundingBox(width, height, topLeftLatitude, topLeftLongitude, bottomRightLatitude, bottomRightLongitude, 0, 0, width, height);
}

/*! Adds a polyline to this map. */
Polyline& MapWidgetInternal::AddPolyline(const PolylineParameters& parameters)
{
    nbmap::GeoPolylineOption* geoPolylineOption = ConvertPolylinePara(parameters);
    mGeoPolyline = mMapController->AddPolyline(*geoPolylineOption);
    PolylineImpl* polylineImpl = new PolylineImpl(parameters, mGeoPolyline, mMapController);
    mPolylineMap.insert(mGeoPolyline, polylineImpl);
    return *polylineImpl;
}

void MapWidgetInternal::RemovePolyline(Polyline* polyline)
{
    QList<nbmap::GeoPolyline*> polylinelist = mPolylineMap.keys();
    for(int i = 0; i < polylinelist.count(); ++i)
    {
        nbmap::GeoPolyline* p = polylinelist.at(i);
        if(mPolylineMap[p] == polyline)
        {
            vector<nbmap::Geographic*> geographicVec;
            geographicVec.push_back(p);
            mPolylineMap.remove(p);
            delete polyline;
            mMapController->RemoveGeographic(geographicVec);
        }
    }
}

/*! convert polyline parameter. */
nbmap::GeoPolylineOption* MapWidgetInternal::ConvertPolylinePara(const PolylineParameters& parameters)
{
    shared_ptr<vector<GeoPoint> > points(new vector<GeoPoint>());
    shared_ptr<vector<nbmap::GeoPolylineAttributeEx*> > geopolyattrex(new vector<nbmap::GeoPolylineAttributeEx*>());

    points->clear();
    for(int i = 0; i < parameters.GetPoints().count(); ++i)
    {
        GeoPoint geopoint;
        Coordinates coord = parameters.GetPoints().at(i);
        geopoint.latitude = coord.latitude;
        geopoint.longitude = coord.longitude;
        geopoint.altitude = 0;
        points->push_back(geopoint);
    }

    const QList<SegmentAttribute*>& segattrs = parameters.GetSegmentAttributes();
    for(int i = 0; i < segattrs.count(); ++i)
    {
        nbmap::GeoPolylineAttributeEx* attr = NULL;
        if(typeid(*(segattrs.at(i))) == typeid(ColorSegment))
        {
            ColorSegment* segment =  (ColorSegment*)segattrs.at(i);
            uint32 segmentColor = ConvertColor(segment->GetColor());
            attr = new nbmap::GeoPolylineColorAttribute(segmentColor, (uint32)segment->GetEndPointIndex());
        }
        else if(typeid(*(segattrs.at(i))) == typeid(PatternSegment))
        {
            PatternSegment* segment =  (PatternSegment*)segattrs.at(i);
             QByteArray byteArray;
             QFile file(segment->GetBitmapPath());
             QPixmap image = QPixmap();
             if (file.open(QIODevice::ReadOnly) && image.load(segment->GetBitmapPath()))
             {
                 byteArray = file.readAll();
                 file.close();
                 nbcommon::DataStreamPtr data = nbcommon::DataStream::Create();
                 NB_Error err = data->AppendData((const uint8*)byteArray.data(), byteArray.length());
                 if(err == NE_OK)
                 {
                     attr = new nbmap::GeoPolylinePatternAttribute(segment->GetEndPointIndex(),
                                                                   segment->GetDistance(),
                                                                   std::string("textureName"),
                                                                   data,
                                                                   image.width(),
                                                                   image.height());
                 }
                 else
                 {
                     return NULL;
                 }
             }

        }
        if(attr)
        {
            geopolyattrex->push_back(attr);
        }
    }

    nbmap::GeoPolylineCapPtr startCap;
    nbmap::GeoPolylineCapPtr endCap;

    /*! Round type*/
    if(parameters.GetStartCap().type == CPT_Round)
    {
        startCap = nbmap::GeoPolylineCap::CreateRoundCap(parameters.GetStartCap().radius);
    }
    /*! Arrow type*/
    else if(parameters.GetStartCap().type == CPT_Arrow)
    {
        startCap = nbmap::GeoPolylineCap::CreateArrowCap(parameters.GetStartCap().width, parameters.GetStartCap().length);
    }

    /*! Round type*/
    if(parameters.GetEndCap().type == CPT_Round)
    {
        endCap = nbmap::GeoPolylineCap::CreateRoundCap(parameters.GetEndCap().radius);
    }
    /*! Arrow type*/
    else if(parameters.GetEndCap().type == CPT_Arrow)
    {
        endCap = nbmap::GeoPolylineCap::CreateArrowCap(parameters.GetEndCap().width, parameters.GetEndCap().length);
    }

    uint32 outlineColor = ConvertColor(parameters.GetOutlineColor());
    uint32 unhighlightedColor = ConvertColor(parameters.GetUnhighlightColor());
    nbmap::GeoPolylineOption* option = new nbmap::GeoPolylineOption(points,
                                                                    geopolyattrex,
                                                                    (uint32)parameters.GetWidth(),
                                                                    unhighlightedColor,
                                                                    (int32)parameters.GetZOrder(),
                                                                    outlineColor,
                                                                    (uint8)parameters.getOutlineWidth(),
                                                                    startCap,
                                                                    endCap);

    return option;
}

/*! Removes all pins, polylines, polygons, etc from the map. */
void MapWidgetInternal::Clear()
{
    DeleteAllPins();
    if(!mPolylineMap.empty())
    {
        QList<nbmap::GeoPolyline*> polylinelist = mPolylineMap.keys();
        vector<nbmap::Geographic*> currPolylinelist;
        for(int i = 0; i < polylinelist.count(); ++i)
        {
            nbmap::Geographic* p = polylinelist.at(i);
            currPolylinelist.push_back(p);
        }
        QList<PolylineImpl*> polylineImplList = mPolylineMap.values();
        for(int i = 0; i < polylineImplList.count(); ++i)
        {
            PolylineImpl* p = polylineImplList.at(i);
            delete p;
        }
        mMapController->RemoveGeographic(currPolylinelist);
        mPolylineMap.clear();
    }
}

void MapWidgetInternal::EnableReferencCenter(bool enable)
{
    mEnableReferenceCenter = enable;
    mIsCameraPositionLocked = enable;
    mMapController->EnableReferenceCenter(enable);
}

void MapWidgetInternal::SetReferenceCenter(const QPoint &position)
{
    mReferenceCenter = position;
    mMapController->SetReferenceCenter( position.x(), position.y() );
}

void MapWidgetInternal::BeginAtomicUpdate()
{
    mMapController->BeginAtomicUpdate();
}

void MapWidgetInternal::EndAtomicUpdate()
{
    mMapController->EndAtomicUpdate();
}

MapDecoration*  MapWidgetInternal::GetMapDecoration()
{
    return mMapDecoration;
}

void MapWidgetInternal::OnCompassEnabledChanged(bool enabled)
{
    mMapController->ShowCompass(enabled);
}

void MapWidgetInternal::OnLocateMeButtonEnabledChanged(bool enabled)
{
    emit mDecorationSignals.ShowLocateMeButton(enabled);
}

void MapWidgetInternal::OnZoomButtonEnabledChanged(bool enabled)
{
    emit mDecorationSignals.ShowZoomButton(enabled);
}

void MapWidgetInternal::OnLayerOptionButtonEnabledChanged(bool enabled)
{
    emit mDecorationSignals.ShowLayerOptionButton(enabled);
}

void MapWidgetInternal::OnCompassPathChanged(const QString& dayModeIconPath, const QString& nightModeIconPath)
{
    if(mMapController)
    {
        mMapController->SetCompassIcons(dayModeIconPath.toStdString(), nightModeIconPath.toStdString());
    }
}

void MapWidgetInternal::OnCompassPositionChanged(float positionX, float positionY)
{
    if(mMapController)
    {
        mMapController->SetCompassPosition(positionX, positionY);
    }
}

void MapWidgetInternal::SetDisplayScreen(int screenIndex)
{
    if (mMapController)
    {
        mMapController->SetDisplayScreen(screenIndex);
    }
}

void MapWidgetInternal::SetDPI(float dpi)
{
    if (mMapController)
    {
        mMapController->SetDPI(dpi);
    }
}

