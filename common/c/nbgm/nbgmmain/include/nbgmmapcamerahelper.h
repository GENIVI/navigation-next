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

    @file nbgmmapcamerahelper.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_MAP_CAMERA_HELPER_H
#define _NBGM_MAP_CAMERA_HELPER_H

#include "nbgmtypes.h"

#include <vector>

/*! Provides map camera functions to help NBServices and UI layers to handle camera event and calculation.
    @remarks
        NBGM_MapCameraHelper is supposed to assist to users, it can support
        some basic camera control funtions including move, tilt and zomm, but also can
        provide some gesture functions such as panding etc. From map view camera,
        you can get the data which you can transfer to map view to control it.
    @par
        NBGM_MapCameraHelper is not threading save.
*/

class NBGM_MapCamera;

class NBGM_MapCameraHelper
{

public:
    virtual ~NBGM_MapCameraHelper(){}

public:
    /*! Sync the data to camera.
     @param mapCamera, the camera which is needed to be sync.
     @return none
     */
    virtual void SyncToCamera(NBGM_MapCamera& mapCamera) = 0;

    /*! Set a location as map view center, camera move and rotate base on this center.
     @param mercatorX center x, unit is Mercator.
     @param mercatorY center y, unit is Mercator.
     @return none
     */
    virtual void SetViewCenter(double mercatorX, double mercatorY) = 0;

    /*! Get a location as map view center.
     @param mercatorX center x, unit is Mercator.
     @param mercatorY center y, unit is Mercator.
     @return none
     */
    virtual void GetViewCenter(double& mercatorX, double& mercatorY) = 0;

    /*! Set the distance from screen bottom to the sky on the horizontal plane
     @param horizonDistance which used to set the sky distance and limit the frustum height, unit is Meters
     @return none
    */
    virtual void SetHorizonDistance(double horizonDistance) = 0;

    /*! Get the distance from screen bottom to the sky on the horizontal plane
     @return horizonDistance which used to set the sky distance and limit the frustum height, unit is Meters
    */
    virtual double GetHorizonDistance() = 0;

    /*! Set rotate angle, base on map view center and axis is vertical with map ground.
     @param angle unit is degree, north is zero, south is 180 degree, direction is counter clockwise
     @return none
     */
    virtual void SetRotateAngle(float angle) = 0;

    /*! Get rotate angle, base on map view center and axis is vertical with map ground.
     @return angle, unit is degree, north is zero, counter clockwise
     */
    virtual float GetRotateAngle() = 0;

    /*! Set distance between camera and map view center
     @param distance Set a absolute distance, unit is Meters
     @return none
     */
    virtual void SetViewPointDistance(float distance) = 0;

    /*! Get distance between camera and map view center
     @return Distance from camera to map view center, unit is Meters
     */
    virtual float GetViewPointDistance() = 0;

    /*! Get height of camera, unit is Mercator
        @return Camera height from ground
     */
    virtual float GetCameraHeight() = 0;

    /*! Set tilt angle
     @param angle unit is degree
     @return none
    */
    virtual void SetTiltAngle(float angle) = 0;

    /*! Get tilt angle between vertical line and view angle
     @return angle, unit is degree
     */
    virtual float GetTiltAngle() = 0;

    /*! Notify map view that view size has been changed;
     @param x Specifies view origin point x
     @param y Specifies view origin point y
     @param width Specifies view width
     @param height Specifies view height
     @return none
    */
    virtual void SetViewSize(int32 x, int32 y, uint32 width, uint32 height) = 0;

    /*! Set up map view perspective projection.
     @param fov Specifies the field of view angle, in degrees, in the y(height) direction.
     @param aspect Specifies the aspect ratio that determines the field of view
            in the x direction. The aspect ratio is the ratio of x (width) to y (height).
     @return none
    */
    virtual void SetPerspective(float fov, float aspect) = 0;

    /*! start movement, it is used for moving map to record start point.
     @param screenX, screenY unit is pixel
     @return true/false, whether the point can be projected on the ground.
     */
    virtual bool StartMove(float screenX, float screenY) = 0;

    /*! do movement,it is used for moving map
     @param screenX, screenY unit is pixel
     @return true/false, whether the point can be projected on the ground.
     */
    virtual bool MoveTo(float screenX, float screenY) = 0;

    /*! Set Rotate center, it will project from screen point to the map ground, map will rotate base on this point.
    @param x, screen x coordinate point
    @param y, screen y coordinate point
    @return none
    */
    virtual void SetRotateCenter(float screenX, float screenY) = 0;

    /*! Rotate Map, rotate center is SetRotateCenter's point, axis is vertical with ground.
    @param angle it is a delta angle, unit is degree
    @return none
    */
    virtual void Rotate(float angle) = 0;

    /*! Tilt Map, rotate center is SetRotateCenter's point, axis is horizonal with ground.
    @param angle it is a delta angle, unit is degree
    @return none
    */
    virtual void Tilt(float angle) = 0;

    /*! Zoom Map
    @param deltaH, Add a height base on current camera height, unit is Mercator.
    @return none
    */
    virtual void Zoom(float deltaH) = 0;

    /*! Project screen point to the map ground.
     @param screenX screen x coordinate point
     @param screenY screen y coordinate point
     @param mercatorX Mercator x
     @param mercatorY Mercator y
     @param useLimitation It is used to determine that the return value is limited in -Pi and Pi.
     @return true/false. If this screen position is on the map return true, else return false.
     */
    virtual bool ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY, bool useLimitation = true) = 0;

    /*! Project map osition to screen point.
     @param screenX screen x coordinate point
     @param screenY screen y coordinate point
     @param mercatorX Mercator x
     @param mercatorY Mercator y
     @return true/false. If this map position is on the screen return true, else return false.
     */
    virtual bool MapPositionToScreen(float& screenX, float& screenY, double mercatorX, double mercatorY) = 0;

    /*! Retrieve current frustum position in world coordinate, to calculate visible tiles
     @param frustumHeight Distance from frustum bottom to top. This value will be used to limit
              the frustum size when showing sky, unit is Meters
     @param positions 4 points, which are map view corners projected to world, Mercator coordinate
            These 4 points will compose a trapezium:
            positions[0]------------- positions[1]
                  \                       / |
                   \                     /  |
                    \                   / frustumHeight
                     \                 /    |
                  positions[3]-----positions[2]
       @return true for get a usable value, false for got a singular value.
     */
    virtual bool GetFrustumPositionInWorld(double frustumHeight, std::vector<NBGM_Point2d64>& positions) = 0;

    /*! Project direction of screen vector on map through current camera.
     @param screenX1 screen1 x coordinate point
     @param screenY1 screen1 y coordinate point
     @param screenX2 screen2 x coordinate point
     @param screenY2 screen2 y coordinate point
     @param direction direction on the map.
     @return true/false. If this direction exists return true, otherwise return false.
     */
    virtual bool ScreenToMapDirection(float screen1X, float screen1Y, float screen2X, float screen2Y, float& direction) = 0;
};

#endif