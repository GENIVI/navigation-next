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
    @file navigationuimapinterface.h
    @date 10/15/2014
    @addtogroup navigationuikit
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
#ifndef LOCATIONTOOLKIT_NAVIGATIONUI_MAPINTERFACE_H
#define LOCATIONTOOLKIT_NAVIGATIONUI_MAPINTERFACE_H

#include <QObject>
#include <QtGlobal>
#include <QPoint>
#include <QVector>
#include <QRect>
#include "location.h"
#include "coordinate.h"
#include "data/nkuitypes.h"

namespace locationtoolkit
{

/**
 *  The map interface is used to handle the operations of map by the NavKit UI.
 *
 *  This interface should be implemented by the client/application which uses the
 *  NavgationUI. The application should implement the codes to operate and display
 *  the elements of map. The NavgationUI should provide  enough informations
 *  for map display when navigating.
 *  The application also can use the signals to tell NKUIKit while anything about map changed.
 */

class NavigationUIMapInterface:public QObject
{
    Q_OBJECT
public:
    /**
     * Set the camera settings.
     *
     * @param camera The camera setting is used to set.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None
     */
    virtual void SetCamera(const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation)=0;

    /**
     * Move the current camera according to a bounding box.
     *
     * This function sets the map center to the center of bounding box and
     * calculates the right zoom level (altitude) to display the whole bounding
     * box.
     *
     * @param boundingbox The bounding box is used to calculate the right zoom
     *                    level.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None
     */
    virtual void MoveCamera(const LTKNKUICoordinateBounds& boundingbox, const LTKNKUIAnimationParameters& animation)=0;

    /**
     * Set the avatar mode.
     *
     * This gives Map a chance to change avatar icons if needed.
     *
     * @param avatarMode The avatar mode is used to set.
     * @return None.
     */
    virtual void SetAvatarMode(AvatarMode avatarMode)=0;

    /**
     * Set the avatar postion and camera postion at the same time.
     *
     * This interface is called whenever NavkitUI wants to set Avatar location and camera
     * Position at same time. Implementation of LTKNUMapInterface may use this API to do some
     * optimization internally.
     *
     * @param location Location of avatar to set.
     * @param camera The camera setting is used to set.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None.
     */
    virtual void SetAvatarLocationAndCameraPosition(const Location& location, const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation)=0;

    /**
     *  Set the avatar location.
     *
     *  This function is only used to set the location to display the avatar. It
     *  should not affect the camera setting (map center display).
     *
     * @param location Location of avatar to set.
     * @return None.
     */
    virtual void SetAvatarLocation(const Location& location)=0;

    /**
     * And a new route.
     *
     * @param polyLineParameter The parameter of the route.
     * @return The LTKNKUIPolyline object.
     */
    virtual LTKNKUIPolyline* AddRoute(const LTKNKUIPolylineParameter& polyLineParameter)=0;

    /**
     * Remove the routes.
     *
     * @param routes The LTKNKUIPolyline objects are specified to remove.
     * @return None.
     */
    virtual void RemoveRoutes(QVector<LTKNKUIPolyline*>& polyline)=0;

    /**
     * And a maneuver arrow.
     *
     * @param maneuverArrowParameter The parameter of the maneuver.
     * @return The LTKNKUIPolyline object.
     */
    virtual LTKNKUIPolyline* AddManeuverArrow(const LTKNKUIManeuverArrowParameter& maneuverArrowParameter)=0;

    /**
     * Remove a maneuver arrow.
     *
     * @param routes The LTKNKUIPolyline object is specified to remove.
     * @return None.
     */
    virtual void RemoveManeuverArrow(LTKNKUIPolyline* maneuverArrow)=0;

    /**
     * Add a pin.
     *
     * @param pinParameters The parameter of the pin.
     * @return Navpin object.
     */
    virtual LTKNKUIPin* AddPin(const LTKNKUIPinParameter& pinParameters)=0;

    /**
     * Remove all pins.
     *
     * @return None.
     */
    virtual void RemovePins()=0;

    /**
     * Adds a BreadCrumb object to the map.
     *
     * @param breadCrumbParameters parameters of the BreadCrumb
     * @return a NavBreadCrumb object, null if failed.
     */
    virtual LTKNKUIBreadCrumb* AddBreadCrumb(const LTKNKUIBreadCrumbParameter& breadCrumbParameters)=0;

    /**
     * Removes all the BreadCrumb objects.
     *
     * @return None.
     */
    virtual void RemoveAllBreadCrumb()=0;

     /**
     * Set the map to the lock or unlock mode.
     *
     * @param locked The flag is used to lock or unlock the map.
     * @return None.
     */
    virtual void LockCameraPosition(bool locked)=0;

    /**
     * Retrieve the value of meters per pixel.
     *
     * @param expectedLatitude The expected latitude to retrieve the value of
     *                         meters per pixel.
     * @return The value of meters per pixel if success, otherwise a negative
     *         value (-1.0) is returned.
     */
    virtual qreal MetersPerPixel(qreal expectedLatitude)=0;

    /**
     * Change the coordinate to pixel point of the screen.
     *
     * @param coordinates The coordinate in real word.
     * @return The point shows on the screen.
     */
    virtual QPoint ToScreenLocation(const Coordinates& coordinate)=0;

     /**
     * Change the point of the screen to coordinate.
     *
     * @param The point shows on the screen.
     * @return coordinates The coordinate in real word.
     */
    virtual Coordinates FromScreenLocation(const QPoint& point)=0;

    /**
     * Enable or Disable the compass with set it's position.
     * @param isEnable- enable or disable compass.
     * @param screenX - position on screen x-coordinate (pixel).
     * @param screenY - position on screen y-coordinate (pixel).
     * @return None.
     */
    virtual void SetCompassStateAndPosition(bool isEnable, qreal screenX, qreal screenY)=0;

   /**
     * Prefetching tiles.
     *
     * @param  points - The points of the expected prefetch.
     * @param  zoomLevel - Current zoomLevel.
     *
     * @return None.
     */
    virtual void Prefetch(QVector<Coordinates> points, qreal zoomLevel)=0;

    /**
     * EnableReferenceCenter - Enable or disable reference center.
     *
     * If reference center is enabled, map should use alternative point as center to zoom
     * in/out. In most cases, the alternative point should be avatar location.
     *
     * @param enable - Flag to indicate enable or not.
     * @return void
     */
    virtual void EnableReferenCecenter(bool enable)=0;

    /**
    * Calculate camera based on a bounding box.
    *
    * @param boundingbox The bounding box is used to calculate camera.
    * @return None
    */
    virtual LTKNKUICameraPosition CameraFromBoundingBox(const LTKNKUICoordinateBounds& boundingbox)=0;

    /**
    * Set the geometry of the map widget.
    *
    * @param rect - The rect where the map will be placed in.
    * @return None
    */
    virtual void SetGeometry( const QRect& rect ) = 0;

Q_SIGNALS:
    /**
     * Callback when the Camera update.
     *
     * @param camera The camera updated.
     * @return None.
     */
    void CameraUpdated(const LTKNKUICameraPosition& camera);

     /**
     * Notify that the camera position of map is unlocked.
     *
     * @return None.
     */
    void Unlocked();

     /**
     * Notify camera animation complete.
     */
    void CameraAnimationDone(qint32 animationId, AnimationState state);

    /**
     * Notify that the click LTKNKUIPolyline of map.
     *
     * @param polylines the clicked polylines.
     * @return None.
     */
    void PolylineClicked(QVector<LTKNKUIPolyline*> polylines);

    /**
     *  Notify the pin is selected.
     *
     *  @param pin: the selected pin
     *  @return: None.
     */
    void PinSelected(LTKNKUIPin* pin);

    /**
     *  Notify that tap on the map
     *
     *  @param coordinate: The coordinate where touched
     *  @return: None.
     */
    void DidTapAtCoordinate(const Coordinates& coordinate);
};
}  // namespace locationtoolkit
#endif
/*! @} */
