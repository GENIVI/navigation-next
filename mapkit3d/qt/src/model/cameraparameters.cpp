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

#include "cameraparameters.h"
using namespace locationtoolkit;

const double CameraParameters::INVALID_LATITUED = -999.0;   /*! Invalid latitude value, the camera will ignore this value */
const double CameraParameters::INVALID_LONGTITUED = (-999.0);  /*! Invalid longitude value, the camera will ignore this value */
const float CameraParameters::INVALID_ZOOMLEVEL_VALUE = (-999.0);  /*! Invalid zoomLevel value, the camera will ignore this value */
const float CameraParameters::INVALID_HEADING_VALUE = (-999.0);  /*! Invalid heading value, the camera will ignore this value */
const float CameraParameters::INVALID_TILT_VALUE = (-999.0);  /*! Invalid tilt value, the camera will ignore this value */

/*! CameraParameters constructor.
 Constructor to use when creating a CameraParameters object.

 @param pos The geographic center of the camera.
 @param zoomLevel The zoom level.
 @param tiltAngle The tilt angle in degrees.
 @param headingAngle The heading angle in degrees.
 */
CameraParameters::CameraParameters(const Coordinates& pos, float zoomLevel, float tiltAngle, float headingAngle)
    :mZoomLevel(zoomLevel),
     mTiltAngle(tiltAngle),
     mHeadingAngle(headingAngle)
{
    mPosition.latitude = pos.latitude;
    mPosition.longitude = pos.longitude;
}

/*! CameraParameters constructor.<br>
 Constructor to use when creating a CameraParameters object only if the position needs change.<br>
 Please note that the other parameters of the current camera will remain unchanged.

 @param pos The geographic center of the camera.
 */
CameraParameters::CameraParameters(const Coordinates& pos)
    :mZoomLevel(INVALID_ZOOMLEVEL_VALUE),
     mTiltAngle(INVALID_TILT_VALUE),
     mHeadingAngle(INVALID_HEADING_VALUE)
{
    //mPosition = pos;
    mPosition.latitude = pos.latitude;
    mPosition.longitude = pos.longitude;
}

CameraParameters::~CameraParameters()
{

}

/*! Gets the position of the camera.
 @return The geographic center of the camera.
 */
Coordinates CameraParameters::GetPosition() const
{
    return mPosition;
}

/*! Sets the position of the camera.
 @param position The geographic center of the camera.
 */
CameraParameters& CameraParameters::SetPosition(const Coordinates& pos)
{
    mPosition.latitude = pos.latitude;
    mPosition.longitude = pos.longitude;
    return *this;
}

/*! Gets the zoom level.
 @return The zoom level value.
 */
float CameraParameters::GetZoomLevel() const
{
    return mZoomLevel;
}

/*! Sets the zoom level.
 @param zoomLevel The zoom level value.
 */
CameraParameters& CameraParameters::SetZoomLevel(float zoomLevel)
{
    mZoomLevel = zoomLevel;
    return *this;
}

/*! Gets the tilt angle.
 @return The tilt angle of the camera in degrees.
 */
float CameraParameters::GetTiltAngle() const
{
    return mTiltAngle;
}

/*! Sets the tilt angle.
 @param tiltAngle The tilt angle of camera in degrees.
 */
CameraParameters& CameraParameters::SetTiltAngle(float tiltAngle)
{
    mTiltAngle = tiltAngle;
    return *this;
}

/*! Gets the heading angle.
 @return The heading angle of the camera in degrees.
 */
float CameraParameters::GetHeadingAngle() const
{
    return mHeadingAngle;
}

/*! Sets the heading angle.
 @param headingAngle The heading angle of the camera in degrees.
 */
CameraParameters& CameraParameters::SetHeadingAngle(float headingAngle)
{
    mHeadingAngle = headingAngle;
    return *this;
}
