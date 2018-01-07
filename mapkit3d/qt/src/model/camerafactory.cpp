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
#include "camerafactory.h"
#include "camerahelper.h"


using namespace locationtoolkit;
CameraFactory::CameraFactory()
{
}

CameraFactory::~CameraFactory()
{
}
/* Creates a CameraParameters object.*/
QSharedPointer<CameraParameters> CameraFactory::CreateCamera(int width, int height, const LatLngBound& latlngBound)
{
    nbmap::Camera camera;
    nbmap::LatLngBound nbmapLatlngBound;
    nbmapLatlngBound.topLeftLatitude  = latlngBound.topLeftLatitude;
    nbmapLatlngBound.topLeftLongitude = latlngBound.topLeftLongitude;
    nbmapLatlngBound.bottomRightLatitude      = latlngBound.bottomRightLatitude;
    nbmapLatlngBound.bottomRightLongitude     = latlngBound.bottomRightLongitude;
    nbmap::CameraHelper::CreateCamera(width, height, 45.0, nbmapLatlngBound, camera);
    double lat = 0.0;    double lon = 0.0;
    camera.GetViewCenter(lat,lon);
    Coordinates coordinates(lat,lon);
    QSharedPointer<CameraParameters> cameraParameters( new  CameraParameters(coordinates,camera.GetZoomLevel(),camera.GetTiltAngle(),camera.GetRotateAngle()) );
    return cameraParameters;
}

/* Create a CameraParameters object to proper position by certain LatLngBound. */
 QSharedPointer<CameraParameters> CameraFactory::CreateCamera(int width, int height, const LatLngBound& latlngBound,
                                                              double xPixelOffset, double yPixelOffset, double viewportWidth, double viewportHeight)
 {
     nbmap::Camera camera;
     nbmap::LatLngBound nbmapLatlngBound;
     nbmapLatlngBound.topLeftLatitude  = latlngBound.topLeftLatitude;
     nbmapLatlngBound.topLeftLongitude = latlngBound.topLeftLongitude;
     nbmapLatlngBound.bottomRightLatitude      = latlngBound.bottomRightLatitude;
     nbmapLatlngBound.bottomRightLongitude     = latlngBound.bottomRightLongitude;
     nbmap::CameraHelper::CreateCamera(width, height, 45.0, nbmapLatlngBound, xPixelOffset, yPixelOffset, viewportWidth, viewportHeight, camera);
     double lat = 0.0;    double lon = 0.0;
     camera.GetViewCenter(lat,lon);
     Coordinates coordinates(lat,lon);
     QSharedPointer<CameraParameters> cameraParameters( new  CameraParameters(coordinates,camera.GetZoomLevel(),camera.GetTiltAngle(),camera.GetRotateAngle()) );
     return cameraParameters;
 }


/*Creates a CameraParameters object which is created with proper position by certain camera setting. */
QSharedPointer<CameraParameters> CameraFactory::CreateCameraParametersForCoordinates( int screenWidth, int screenHeight,const Coordinates& coordinates,
                                                                                  const CameraSetting & camerasetting , float heading)
{
    nbmap::Camera camera;
    nbmap::CameraSetting nbmapCameraSetting;
    nbmapCameraSetting.cameraHeight = camerasetting.cameraHeight;
    nbmapCameraSetting.cameraToAvatar = camerasetting.cameraToAvatar;
    nbmapCameraSetting.avatarToHorizon = camerasetting.avatarToHorizon;
    nbmapCameraSetting.hFov = camerasetting.hFov;
    nbmapCameraSetting.avatarToBottom = camerasetting.avatarToBottom;
    nbmapCameraSetting.avatarScale = camerasetting.avatarScale;
    nbmap::CameraHelper::CreateCamera(coordinates.longitude,coordinates.latitude,heading ,screenWidth,screenHeight, nbmapCameraSetting,camera);
    QSharedPointer<CameraParameters> cameraParameters( new  CameraParameters(coordinates,camera.GetZoomLevel(),camera.GetTiltAngle(),camera.GetRotateAngle()) );
    return cameraParameters;
}

QSharedPointer<CameraParameters> CameraFactory::CreateCameraParametersForCoordinatesAtPosition(const QPoint& screenPostion, int screenWidth, int screenHeight, const Coordinates& coordinates, float tilt, float zoomLevel, float heading )
{
    nbmap::Camera nbcamera;
    nbcamera.SetRotateAngle( heading );
    nbcamera.SetTiltAngle( tilt );
    nbcamera.SetZoomLevel( zoomLevel );
    nbcamera.SetViewCenter( coordinates.latitude, coordinates.longitude );

    nbmap::Camera scrolledCamera;
    nbmap::CameraHelper::ScrollCamera( nbcamera, screenWidth, screenHeight, screenPostion.x(), screenPostion.y(), scrolledCamera );

    double newLat, newLon;
    scrolledCamera.GetViewCenter( newLat, newLon );
    Coordinates newCoord( newLat, newLon );

    QSharedPointer<CameraParameters> newCamera( new CameraParameters( newCoord, scrolledCamera.GetZoomLevel(), scrolledCamera.GetTiltAngle(), scrolledCamera.GetRotateAngle() ) );
    return newCamera;
}

