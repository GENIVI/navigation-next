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

    @file nbgmmapcamera.h
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

#ifndef _NBGM_MAP_CAMERA_H_
#define _NBGM_MAP_CAMERA_H_

#include "paltypes.h"

/*! Map View camera controller.
    @remarks
        NBGM_MapCamera contains a collection of map view control function.
    @par
        NBGM_MapCamera is not threading save.
*/

class NBGM_MapCamera
{

public:
    virtual ~NBGM_MapCamera(){}

public:
    /*! Set a location as map view center, camera move and rotate base on this center.
     @param mercatorX center x, unit is Mercator.
     @param mercatorY center y, unit is Mercator.
     @return none
     */
    virtual void SetViewCenter(double mercatorX, double mercatorY) = 0;

    /*! Set the distance from screen bottom to the sky on the horizontal plane
     @param horizonDistance which used to set the sky distance and limit the frustum height, unit is Meters
     @return none
    */
    virtual void SetHorizonDistance(double horizonDistance) = 0;

    /*! Set rotate angle, base on map view center and axis is vertical with map ground.
     @param angle unit is degree, north is zero, south is 180 degree, direction is counter clockwise
     @return none
     */
    virtual void SetRotateAngle(float angle) = 0;

    /*! Set distance between camera and map view center
     @param distance Set a absolute distance, unit is Meters
     @return none
     */
    virtual void SetViewPointDistance(float distance) = 0;

    /*! Set tilt angle
     @param angle unit is degree
     @return none
    */
    virtual void SetTiltAngle(float angle) = 0;

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
};

#endif