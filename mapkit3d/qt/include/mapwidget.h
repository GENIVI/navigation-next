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
    @file mapwidget.h
    @date 08/06/2014
    @defgroup mapkit3d Mapkit3d
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
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __MAP_WIDGET_H__
#define __MAP_WIDGET_H__

#include <QObject>
#include <QList>
#include <QString>
#include <QQuickItem>
#include <QResizeEvent>
#include "avatar.h"
#include "location.h"
#include "ltkcontext.h"
#include "mapoptions.h"
#include "mapdecoration.h"
#include "animationparameters.h"
#include "pin.h"
#include "pinparameters.h"
#include "polyline.h"
#include "polylineparameters.h"
#include "mapwidgetlistener.h"
#include "mapprojection.h"
#include "prefetchconfiguration.h"

namespace locationtoolkit
{
class MapWidgetInternal;
class MapWidget: public QQuickItem
{
    Q_OBJECT
public:
    /*! GPS mode. */
    enum GPSMode{
        GM_INVALID = 0,
        GM_STANDBY,
        GM_FOLLOW_ME_ANY_HEADING,
        GM_FOLLOW_ME
    };

    /*! DayNight mode. */
    enum NightMode {
        NM_AUTO = 0,
        NM_DAY,
        NM_NIGHT
    };
    /*! Status of camera animation. */
    enum AnimationStatus
    {
        AS_FINISHED = 0,
        AS_INTERRUPTED,
        AS_UNKNOWN
    };

    enum HybridMode
    {
        ON_BOARDMODE,
        OFF_BOARDMODE
    };

    struct LayerNameAndEnabled
    {
        LayerNameAndEnabled() : enabled(false) {}
        QString name;
        bool   enabled;
    };

    enum GestureType
    {
        GT_Invalid,

        GT_Move,
        GT_TiltAngle,
        GT_RotateAngle,
        GT_Scale,
        GT_Tap,
        GT_DoubleTap,
        GT_TwoFingerTap,
        GT_LongPress,
        GT_TwoFingerDoubleTap
    };

    enum GestureState
    {
        GS_Possible,
        GS_Began,
        GS_Changed,
        GS_Ended,
        GS_Cancelled,
        GS_Failed,
        GS_Recognized
    };

public:
    /*! MapWidget constructor.
        Simple constructor to use when creating a view from code.
    */
    explicit MapWidget();
    ~MapWidget();

public:
    /*! Initialize the MapWidget. */
    void Initialize(const MapOptions& mapConfigration, LTKContext& ltkContext);

    /*! Sets a camera without animation for the map. */
    void MoveTo(const CameraParameters& camera);
    /*! Sets a camera with animation for the map. */
    void AnimateTo(const CameraParameters& camera, const AnimationParameters& animation);
    /*! Convenient method for AnimateTo */
    void AnimateTo(const Coordinates& pos);
    /*! Gets the current position of the camera. */
    bool GetCameraPosition(CameraParameters& camerapara);
    /*! Creates a camera factory. */
    //bool CreateCameraFactory(CameraFactory& cameraFactory);

    /*! Prefetches map tiles based on camera parameters. */
    void Prefetch(const CameraParameters& cameraParam);

    /*! Prefetches map tiles for nav. */
    void Prefetch(QVector<Coordinates> points, const PrefetchConfiguration& configuration);

    /*! Sets the night mode for the map. */
    void SetNightMode(NightMode mode);
    /*! Checks whether the map is in night mode. */
    bool IsNightMode();

    /*! Sets the Gps mode for the map. */
    void SetGpsMode(GPSMode mode, bool animated);
    /*! Gets the current GPS mode. */
    GPSMode  GetGpsMode();

    /*! Turns the specified layer on or off. */
    void ShowOptionalLayer(QString layerName, bool show);
    /*! Turns the traffic layer on or off. */
    void ShowTrafficLayer(bool enable);

    /*! Gets the avatar. */
    Avatar& GetAvatar();

    /*! Create a pin to this map. */
    Pin* CreatePin(const PinParameters& para);
    /*! Remove a pin. */
    void RemovePin(Pin* pin);
    /*! Removes all the pins from the map. */
    void DeleteAllPins();

    /*! Turns the debug view on or off. */
    void ShowDebugView(bool enable);

    /*! Sets the Mapview background mode */
    void SetBackground(bool background);

    /*! Turns the names and enabled state of layers. */
    const QList<LayerNameAndEnabled> GetLayerNameAndStates();

    /*! Make the specified section be shown in map. */
    void ZoomToBoundingBox(double topLeftLatitude,
                           double topLeftLongitude,
                           double bottomRightLatitude,
                           double bottomRightLongitude);
    
    /*! Make the specified section be shown in map within a screen area. */
    void ZoomToBoundingBox(double topLeftLatitude,
                           double topLeftLongitude,
                           double bottomRightLatitude,
                           double bottomRightLongitude,
                           int xOffset,
                           int yOffset,
                           int width,
                           int height);

    /*! Add a polyline to map.
     @param parameters A polyline parameters object which defines how to render this polyline.
     @return The Polyline object that was added to this map.
     */
    Polyline& AddPolyline(const PolylineParameters& parameters);

    /*! Remove a polyline from map.
     @param a polyline object that is to be removed
     */
    void RemovePolyline(Polyline* polyline);

    /*! Removes all pins, polylines, polygons, etc from the map.
     @return None
     */
    void Clear();

    /*! Get the map projection object. */
    const MapProjection& GetMapProjection() const;

    /*! Enable/disable reference center functionality.
     @return None
     */
    void EnableReferenceCenter( bool enable );

    /*! Set avatar to the specified position.
     @return None
     */
    void SetReferenceCenter( const QPoint& position );

    /*! Begin to atomic update.
     @return None
     */
    void BeginAtomicUpdate();

    /*! End atomic update.
     @return None
     */
    void EndAtomicUpdate();

    /*! Get map decoration configurations.
     * return  MapDecoration
     */
    MapDecoration* GetMapDecoration();

    /*! Set display screen.
        @param screenIndex screen index. If screen doesn't exist, do nothing.
        @return None
     */
    void SetDisplayScreen(int screenIndex);

    /*! Set display screen.
        @param dpi screen dpi.
        @return None
     */
    void SetDPI(float dpi);
    void Resize(const QSize& size);
protected:
    void resizeEvent(QResizeEvent* event);

Q_SIGNALS:
    void MapCreated();
    /*! Emitted when the camera changes.
        @param cameraParameters See {@link CameraParameters} for more.
     */
    void CameraUpdated(const locationtoolkit::CameraParameters& cameraParameters);

    /*! Emitted when a camera animation is finished or interrupted.
       @param cameraAnimationId Indicate which animation is done.
       @param animationStatus Indicate the cause for the end of the animation.
     */
    void CameraAnimationDone(int cameraAnimationId, locationtoolkit::MapWidget::AnimationStatus animationStatus);

    /*! Emitted when the map becomes into unlocked mode.
     */
    void Unlocked();

    /*! Emitted when the user makes a tap gesture on the map.
     @param coordinate The point on the ground(projected from the screen point) that was tapped.
     */
    void MapClicked(const locationtoolkit::Coordinates& coordinate);

    /*! Emitted when the user makes a long-press gesture on the map.
     @param coordinate The point on the ground(projected from the screen point) that was tapped.
     */
    void MapLongClicked(const locationtoolkit::Coordinates& coordinate);

    /*! Emitted when the user selects a pin, but only if the pin is added by users.
     The map will decide which pin should be highlighted if two pins overlap.
     @param pin The pin that was selected. See {@link Pin} for more.
     */
    void PinClicked(const locationtoolkit::Pin* pin);

    /*! Emitted when the user clicks on the avatar.
     @param coordinate The coordinates of the avatar.
     */
    void AvatarClicked(const locationtoolkit::Coordinates&);

    /*! Emitted when the night mode changes.
     @param isNightMode True if the map is in night mode, false otherwise.
     */
    void NightModeChanged(bool);

    /*! Emitted when the layers are created.
     */
    void LayersCreated();

    /*! when a polyline is selected. */
    void PolylineClicked(const QList<locationtoolkit::Polyline*> polylines);

    /*! Inform app to show layer option widget. */
    void LayerOptionButtonClicked();

    void Gesture(MapWidget::GestureType type, MapWidget::GestureState state, int time);

    void StaticPOIClicked(const QString& id, const QString& name, double latitude, double longitude);

    void CompassClicked(float needleOrientation);

private Q_SLOTS:
    void OnInternalCameraAnimationDone(int cameraAnimationId, int animationStatus);
    void OnGesture(int type, int state, int time);
    void OnStaticPOIClicked(const QString& id, const QString& name, double latitude, double longitude);

private:
    MapWidgetInternal* mMapWidgetInternal;
    Avatar* mAvatar;
    MapProjection* mMapProjection;
};
}
#endif // __MAP_WIDGET_H__

/*! @} */
