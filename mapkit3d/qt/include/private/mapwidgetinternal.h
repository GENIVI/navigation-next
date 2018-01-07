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

/*!--------------------------------------------------------------------------

    @file mapwidgetinternal.h
    @date 08/06/2014
    @addtogroup mapkit3d
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
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __MAP_WIDGET_INTERNAL_H__
#define __MAP_WIDGET_INTERNAL_H__

#include <QMap>
#include <QDebug>
#include "pinimpl.h"
#include "mapoptions.h"
#include "mapdecoration.h"
#include "mapcontroller.h"
#include "mappin.h"
#include "mapwidget.h"
#include "camerahelper.h"
#include "polylineimpl.h"
#include "MapNativeViewImpl.h"
#include "mapdecorationsignals.h"
#include "prefetchconfiguration.h"

namespace locationtoolkit
{
class LTKContext;
class MapDecorationSignals;
class MapWidgetInternal:public QObject, public nbmap::MapListener, public MapNativeViewImpl
{
    Q_OBJECT
public:
    /** On heads up mode(NGM_FOLLOW_ME), zoom level should be 18.6, tilt angle should be 38.18 */
    static const float FOLLOW_ME_MODE_ZOOM_LEVEL/* = 18.6*/;
    static const float FOLLOW_ME_MODE_TILT_ANGLE/* = 38.18*/;
    static const float FOLLOW_ME_ANDHEADING_ZOOM_LEVEL/* = 16.0f*/;
    static const int GPS_MODE_SWITCH_ANIMATION_TIME_SHORT/* = 480*/;
    static const int GPS_MODE_SWITCH_ANIMATION_TIME_LONG/* = 620*/;

    /** Invalid latitude value, the camera will ignore this value*/
    static const double INVALID_LATITUED/* = -999.*/;
    /** Invalid longitude value, the camera will ignore this value*/
    static const double INVALID_LONGTITUED/* = -999.*/;
    /** Invalid zoomLevel value, the camera will ignore this value*/
    static const float INVALID_ZOOMLEVEL_VALUE/* = -999.f*/;
    /** Invalid heading value, the camera will ignore this value*/
    static const float INVALID_HEADING_VALUE /*= -999.f*/;
    /** Invalid tilt value, the camera will ignore this value*/
    static const float INVALID_TILT_VALUE/* = -999.f*/;

    static const float FOLLOW_ME_MODE_FONT_SCALE/* = 1.5*/;
    static const float DEFAULT_FONT_SCALE/* = 1.5*/;

public:
    MapWidgetInternal(const MapOptions& mapOptions, LTKContext& ltkContext, QQuickItem* parent);
    ~MapWidgetInternal();
public:
    void CreateMap();
    /*! Sets a camera without animation for the map. */
    void MoveTo(const CameraParameters& camera);
    /*! Sets a camera with animation for the map. */
    void AnimateTo(const CameraParameters& camera, const AnimationParameters& animation);
    void AnimateTo(const Coordinates& pos);
    /*! Gets the current position of the camera. */
    bool GetCameraPosition(CameraParameters& camerapara) const;
    void Prefetch(const CameraParameters& cameraParam);
    void Prefetch(QVector<Coordinates> points, const PrefetchConfiguration& configuration);
    void LockCameraPosition(bool lock);
    void SetAvatarLocation(const Location& avatarLocation);
    /*! Sets the night mode for the map. */
    void SetNightMode(uint mode);
    /*! Checks whether the map is in night mode. */
    bool IsNightMode();
    void SetAvatarMode(int mode);
    /*! Sets the scale value of the avatar. */
    void SetAvatarScale(float scale);
    /*! Sets the Mapview background mode */
    void SetBackground(bool background);
    /*! Set display screen.
        @param screenIndex screen index. If screen doesn't exist, do nothing.
        @return None
     */
    void SetDisplayScreen(int screenIndex);
    /*! Set DPI.
     */
    void SetDPI(float dpi);

    // from MapNativeViewImpl
    /*! Gps mode. */
    virtual void SetGpsMode(NB_GpsMode mode, bool animated);
    virtual bool IsCameraLocked() const;
    virtual void UnLockCameraPosition();
    NB_GpsMode GetGpsMode() const;
    virtual locationtoolkit::MapDecorationSignals& GetMapDecorationSignals();
    virtual void ZoomIn();
    virtual void ZoomOut();
    virtual void ShowLayerOption();
    virtual void OnCompassClicked(float needleOrientation);

    void stopAnimations(int type);

    void ShowOptionalLayer(QString layerName, bool show);
    void ShowTrafficLayer(bool enable);
    void ShowDebugView(bool enable);

    Pin* CreatePin(const PinParameters& para);
    void RemovePin(Pin* pin);
    void DeleteAllPins();
    nbmap::MapController& GetMapController() const;
    nbmap::GeoPolyline* GetGeoPolyline() const;
    void GetLayerNameAndStates(QList<QString>& names, QList<bool>& enables );
    void ResizeEvent(QResizeEvent * event);
    void ZoomToBoundingBox(int width,
                           int height,
                           double topLeftLatitude,
                           double topLeftLongitude,
                           double bottomRightLatitude,
                           double bottomRightLongitude);
    void ZoomToBoundingBox(int width,
                           int height,
                           double topLeftLatitude,
                           double topLeftLongitude,
                           double bottomRightLatitude,
                           double bottomRightLongitude,
                           int xOffset,
                           int yOffset,
                           int viewportWidth,
                           int viewportHeight);
    Polyline& AddPolyline(const PolylineParameters& parameters);
    void RemovePolyline(Polyline *polyline);
    void Clear();

    void EnableReferencCenter(bool enable);
    void SetReferenceCenter(const QPoint &position);
    void BeginAtomicUpdate();
    void EndAtomicUpdate();
    MapDecoration* GetMapDecoration();

Q_SIGNALS:
    void MapCreated();
    void CameraUpdated(const locationtoolkit::CameraParameters&);
    void CameraAnimationDone(int cameraAnimationId, int animationStatus);
    void Unlocked();
    void MapClicked(const locationtoolkit::Coordinates&);
    void MapLongClicked(const locationtoolkit::Coordinates&);
    void PinClicked(const locationtoolkit::Pin* pin);
    void AvatarClicked(const locationtoolkit::Coordinates&);
    void NightModeChanged(bool);
    void LayersCreated();
    void PolylineClicked(const QList<locationtoolkit::Polyline*> polylines);
    void LayerOptionButtonClicked();
    void Gesture(int type, int state, int time);
    void StaticPOIClicked(const QString& id, const QString& name, double latitude, double longitude);
    void CompassClicked(float needleOrientation);

private:
    bool OnSingleTap(float screenX,       /*!< tapped position. */
                float screenY,       /*!< tapped position. */
                std::set<nbmap::Geographic*>& graphics /*!< objects under the tap point */
                );
    void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt);
    void OnOptionalLayerUpdated(const std::vector<nbmap::LayerAgentPtr>& layers);
    void OnPinClicked(nbmap::MapPin* pin);

    void OnTrafficIncidentPinClicked(double latitude, double longitude);
    void OnStaticPOIClicked(const std::string& id, const std::string& name, double latitude, double longitude);
    void OnAvatarClicked(double lat, double lon);

    /*! Called when the map is created.
     Implementations of this method are always invoked on the main thread.
    */
    void OnMapCreate();
    void OnMapReady();

    void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus);

    void OnNightModeChanged(bool isNightMode);

    void OnTap(double latitude, double longitude);

    void OnCameraLimited(MapViewCameraLimitedType type);
    void OnMarkerClicked(int markerId);
    /*! Called when the user makes a long-press gesture on the map. */
    void OnLongPressed(double latitude, double longitude);
    void OnGesture(MapViewGestureType type, MapViewGestureState state, int time);
    /*! convert polyline parameter. */
    nbmap::GeoPolylineOption* ConvertPolylinePara(const PolylineParameters& parameters);

    inline uint32 ConvertColor(const QColor& color)
    {
        return color.red() << 24 | color.green() << 16 | color.blue() << 8 | color.alpha();
    }

private Q_SLOTS:
    void SwitchGpsMode();
    void OnCompassEnabledChanged(bool enabled);
    void OnLocateMeButtonEnabledChanged(bool enabled);
    void OnZoomButtonEnabledChanged(bool enabled);
    void OnLayerOptionButtonEnabledChanged(bool enabled);
    void OnCompassPathChanged(const QString& dayModeIconPath, const QString& nightModeIconPath);
    void OnCompassPositionChanged(float positionX, float positionY);

private:
    QQuickItem* mParentWindow;
    nbmap::MapController* mMapController;
    nbmap::GeoPolyline* mGeoPolyline;
    LTKContext&  mLtkContext;
    MapOptions  mMapOptions;

    QList<Pin*> mPinList;
    QMap<nbmap::GeoPolyline*, PolylineImpl*> mPolylineMap;
    MapProjection* mMapProjection;
    bool mIsCameraPositionLocked;
    MapDecorationSignals mDecorationSignals;
    MapDecoration* mMapDecoration;

    NB_GpsMode mGpsMode;
    bool mEnableReferenceCenter;
    QPoint mReferenceCenter;
    float mZoomLevel;

    static const char* ANTI_ALIASING_PROPERTY_NAME;
    static const char* ANISOTROPIC_PROPERTY_NAME;
    static const char* MAPVIEW_ONLY_PROPERTY_NAME;
};

}  //namespace locationtoolkit
#endif // __MAP_WIDGET_INTERNAL_H__

/*! @} */
