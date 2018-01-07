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
 @file     camerahelper.h
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef CAMERAHELPER_H_
#define CAMERAHELPER_H_

#include "paltypes.h"
#include "camera.h"

namespace nbmap
{

/*! Type of LatLngBounds
    define a rectangle of latitude & longitude
 */
struct LatLngBound
{
    double topLeftLatitude;
    double topLeftLongitude;
    double bottomRightLatitude;
    double bottomRightLongitude;
};

struct CameraSetting
{
    float cameraHeight;
    float cameraToAvatar;
    float avatarToHorizon;
    float hFov;
    float avatarToBottom;
    float avatarScale;
};

/*! Type of CameraHelper
    enhance camera functionality
 */
class CameraHelper
{

public:
    /**
     * @name CreateCamera - Create camera to proper position by certain LatLngBound.
     * @param width
     * @param height
     * @param hFov
     * @param latlngBound
     * @param camera - returned camera
     * @return None.
     */
    static void CreateCamera(int width, int height, float hFov, const LatLngBound& latlngBound, Camera& camera, float scaleFactorFOV = 1.f);
    
    /**
     * @name CreateCamera - Create camera to proper position by certain LatLngBound.
     * @param width
     * @param height
     * @param hFov
     * @param latlngBound
     * @param xPixelOffset
     * @param yPixelOffset
     * @param camera - returned camera
     * @return None.
     */
    static void CreateCamera(int width, int height, float hFov, const LatLngBound& latlngBound, double xPixelOffset, double yPixelOffset, double viewportWidth, double viewportHeight, Camera& camera);

    /**
     * @name CreateCamera - Create camera to proper position by certain camera setting.
     * @param lon
     * @param lat
     * @param heading - current heading
     * @param width
     * @param height
     * @param cameraSetting - target cameraSetting
     * @param camera - returned camera
     * @return None.
     */
    static void CreateCamera(double lon, double lat, float heading, int width, int height, const CameraSetting& cameraSetting, Camera& camera, float scaleFactorFOV = 1.f);

    /**
     * @name CreateCamera - Create camera to proper position by certain camera setting.
     * @param camera - current camera
     * @param screenWidth - current screen width
     * @param screenHeight - current screen height
     * @param xPixel - x offset by screen center
     * @param yPixel - y offset by screen center
     * @param result - returned camera
     * @return None.
     */
    static void ScrollCamera(const Camera& camera, int screenWidth, int screenHeight, int xPixel, int yPixel, Camera& result, float scaleFactorFOV = 1.f);

private:
    static double CalculateDistanceFromZoomLevel(double zoomLevel);
    static double CalculateZoomLevelFromDistance(double distance);

};

}

#endif /* CAMERAHELPER_H_ */
