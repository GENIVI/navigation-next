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
    @file cameraparameters.h
    @date 08/11/2014
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

#ifndef __CAMERA_PARAMETERS_H__
#define __CAMERA_PARAMETERS_H__

#include "coordinate.h"

/*!
 CameraParameters defines parameters for a camera.
 To make camera parameters taking effect, users have to call {@link MapWidget#moveTo(CameraParameters)}.
 */

namespace locationtoolkit
{
class CameraParameters
{
public:
    const static double INVALID_LATITUED/* = -999.0*/;   /*! Invalid latitude value, the camera will ignore this value */
    const static double INVALID_LONGTITUED/* = (-999.0)*/;  /*! Invalid longitude value, the camera will ignore this value */
    const static float INVALID_ZOOMLEVEL_VALUE/* = (-999.0)*/;  /*! Invalid zoomLevel value, the camera will ignore this value */
    const static float INVALID_HEADING_VALUE/* = (-999.0)*/;  /*! Invalid heading value, the camera will ignore this value */
    const static float INVALID_TILT_VALUE/* = (-999.0)*/;  /*! Invalid tilt value, the camera will ignore this value */

public:
    /*! CameraParameters constructor.
     Constructor to use when creating a CameraParameters object.

     @param pos The geographic center of the camera.
     @param zoomLevel The zoom level.
     @param tiltAngle The tilt angle in degrees.
     @param headingAngle The heading angle in degrees.
     */
    CameraParameters(const Coordinates& pos,
                     float zoomLevel,
                     float tiltAngle,
                     float headingAngle);

    /*! CameraParameters constructor.<br>
     Constructor to use when creating a CameraParameters object only if the position needs change.<br>
     Please note that the other parameters of the current camera will remain unchanged.

     @param pos The geographic center of the camera.
     */
    CameraParameters(const Coordinates& pos);

    ~CameraParameters();
public:
    /*! Gets the position of the camera.
     @return The geographic center of the camera.
     */
    Coordinates GetPosition() const;

    /*! Sets the position of the camera.
     @param position The geographic center of the camera.
     */
    CameraParameters& SetPosition(const Coordinates& pos);

    /*! Gets the zoom level.
     @return The zoom level value.
     */
    float GetZoomLevel() const;

    /*! Sets the zoom level.
     @param zoomLevel The zoom level value.
     */
    CameraParameters& SetZoomLevel(float zoomLevel);

    /*! Gets the tilt angle.
     @return The tilt angle of the camera in degrees.
     */
    float GetTiltAngle() const;

    /*! Sets the tilt angle.
     @param tiltAngle The tilt angle of camera in degrees.
     */
    CameraParameters& SetTiltAngle(float tiltAngle);

    /*! Gets the heading angle.
     @return The heading angle of the camera in degrees.
     */
    float GetHeadingAngle() const;

    /*! Sets the heading angle.
     @param headingAngle The heading angle of the camera in degrees.
     */
    CameraParameters& SetHeadingAngle(float headingAngle);

private:
    Coordinates mPosition;
    float mZoomLevel;
    float mTiltAngle;
    float mHeadingAngle;
};
}
#endif // __CAMERA_PARAMETERS_H__

/*! @} */
