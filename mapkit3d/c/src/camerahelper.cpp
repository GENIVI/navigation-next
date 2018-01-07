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
 @file     camerahelper.cpp
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

#include "camerahelper.h"
#include "palmath.h"
#include "transformutility.h"
#include "nbgmmapcamerahelper.h"
#include "nbgm.h"
#include "nbgmmapscaleutility.h"
#include "NBUIConfig.h"

using namespace nbmap;

#define METER_TO_MERCATOR(x) ((float)((x)/RADIUS_EARTH_METERS))

void CameraHelper::CreateCamera(int width, int height, float hFov, const LatLngBound& latlngBound, Camera& camera, float scaleFactor)
{
    double topLeftX = 0;
    double topLeftY = 0;
    double bottomRightX = 0;
    double bottomRightY = 0;

    mercatorForward(latlngBound.topLeftLatitude, latlngBound.topLeftLongitude, &topLeftX, &topLeftY);
    mercatorForward(latlngBound.bottomRightLatitude, latlngBound.bottomRightLongitude, &bottomRightX, &bottomRightY);

    double mercator_width = 0.0;
    double xDistance = nsl_fabs(topLeftX - bottomRightX);
    double yDistance = nsl_fabs(topLeftY - bottomRightY);
    yDistance *= width;
    yDistance /= height;

    if (xDistance > yDistance)
    {
        mercator_width = xDistance;
    }
    else
    {
        mercator_width = yDistance;
    }

    float halfVFov = (float)(NBGM_MapScaleUtility::CalculateFOV(width*scaleFactor) / 2);
    double cameraHeight = RADIUS_EARTH_METERS * (mercator_width/2.0)/nsl_tan(TO_RAD(halfVFov));

    double centerLat = 0;
    double centerLon = 0;
    mercatorReverse((topLeftX+bottomRightX)/2, (topLeftY+bottomRightY)/2, &centerLat, &centerLon);

    camera.SetViewCenter(centerLat, centerLon);
    camera.SetRotateAngle(0);
    camera.SetTiltAngle(90.0);
    camera.SetZoomLevel(CalculateZoomLevelFromDistance(cameraHeight));
}

void CameraHelper::CreateCamera(int width, int height, float hFov, const LatLngBound& latlngBound, double xPixelOffset, double yPixelOffset, double viewportWidth, double viewportHeight, Camera& camera)
{
    double topLeftX = 0;
    double topLeftY = 0;
    double bottomRightX = 0;
    double bottomRightY = 0;

    mercatorForward(latlngBound.topLeftLatitude, latlngBound.topLeftLongitude, &topLeftX, &topLeftY);
    mercatorForward(latlngBound.bottomRightLatitude, latlngBound.bottomRightLongitude, &bottomRightX, &bottomRightY);

    double mercator_width = 0.0;
    double xDistance = nsl_fabs(topLeftX - bottomRightX) * width / viewportWidth;
    double yDistance = nsl_fabs(topLeftY - bottomRightY) * height / viewportHeight;
    yDistance *= viewportWidth;
    yDistance /= viewportHeight;

    if (xDistance > yDistance)
    {
        mercator_width = xDistance;
    }
    else
    {
        mercator_width = yDistance;
    }

    float halfVFov = (float)(NBGM_MapScaleUtility::CalculateFOV(width) / 2);
    double cameraHeight = RADIUS_EARTH_METERS * (mercator_width/2.0)/nsl_tan(TO_RAD(halfVFov));

    double centerLat = 0;
    double centerLon = 0;
    
    double centerMercatorX = (topLeftX+bottomRightX)/2;
    double centerMercatorY = (topLeftY+bottomRightY)/2;
    double zoomLevel = CalculateZoomLevelFromDistance(cameraHeight);
    double mercatorPerPixel = NBGM_MapScaleUtility::MetersPerPixel(zoomLevel) / RADIUS_EARTH_METERS;
    centerMercatorX -= xPixelOffset * mercatorPerPixel;
    centerMercatorY += yPixelOffset * mercatorPerPixel;

    mercatorReverse(centerMercatorX, centerMercatorY, &centerLat, &centerLon);

    camera.SetViewCenter(centerLat, centerLon);
    camera.SetRotateAngle(0);
    camera.SetTiltAngle(90.0);
    camera.SetZoomLevel(zoomLevel);
}

void CameraHelper::CreateCamera(double lon, double lat, float heading,int width, int height,
        const CameraSetting& cameraSetting, Camera& camera, float scaleFactor)
{
    float halfVFov = (float)(NBGM_MapScaleUtility::CalculateFOV(height*scaleFactor) / 2);
    double d = height * 0.5 / nsl_tan(halfVFov);
    double angleAvatarToEye = nsl_atan((height * 0.5 - cameraSetting.avatarToBottom) / d);
    double angleEyeToAvatar = nsl_atan(cameraSetting.cameraToAvatar / cameraSetting.cameraHeight);
    double angleEye = angleAvatarToEye + angleEyeToAvatar;
    double eyeToViewPoint = nsl_fabs(cameraSetting.cameraHeight * nsl_tan(angleEye));
    double mx = 0;
    double my = 0;
    mercatorForward(lat, lon, &mx, &my);

    double viewPointOffset = METER_TO_MERCATOR(eyeToViewPoint - cameraSetting.cameraToAvatar);
    mx += viewPointOffset*nsl_sin(TO_RAD(heading));
    my += viewPointOffset*nsl_cos(TO_RAD(heading));

    mercatorReverse(mx, my, &lat, &lon);

    camera.SetViewCenter(lat, lon);
    camera.SetRotateAngle(heading);
    camera.SetTiltAngle(static_cast<float>(90 - TO_DEG(angleEye)));
    camera.SetZoomLevel(CalculateZoomLevelFromDistance(cameraSetting.cameraHeight/nsl_cos(angleEye)));
}

void CameraHelper::ScrollCamera(const Camera& camera, int screenWidth, int screenHeight, int xPixel, int yPixel, Camera& result, float scaleFactor)
{
    NBGM_MapCameraHelper* cameraHelper = NULL;
    if(NBGM_CreateMapCameraHelper(&cameraHelper) == PAL_Ok)
    {
        double lat = 0;
        double lon = 0;
        camera.GetViewCenter(lat, lon);
        double mx = 0;
        double my = 0;
        mercatorForward(lat, lon, &mx, &my);
        cameraHelper->SetViewCenter(mx, my);
        cameraHelper->SetRotateAngle(camera.GetRotateAngle());
        cameraHelper->SetTiltAngle(90.f - camera.GetTiltAngle());
        cameraHelper->SetViewPointDistance(CalculateDistanceFromZoomLevel(camera.GetZoomLevel()));
        cameraHelper->SetViewSize(0, 0, screenWidth, screenHeight);
        double horizonDistance = 0;
        double horizonDistanceRasterNear = 0;  // Not needed here
        NBUIConfig::getFrustumDistance(camera.GetZoomLevel(), horizonDistance, horizonDistanceRasterNear);
        cameraHelper->SetHorizonDistance(horizonDistance);
        float fov = (float)NBGM_MapScaleUtility::CalculateFOV(screenHeight*scaleFactor);
        cameraHelper->SetPerspective(fov, static_cast<float>(screenWidth) / static_cast<float>(screenHeight));
        cameraHelper->StartMove(static_cast<float>(screenWidth/2), static_cast<float>(screenHeight/2));
        cameraHelper->MoveTo(xPixel, yPixel);

        cameraHelper->GetViewCenter(mx, my);
        mercatorReverse(mx, my, &lat, &lon);
        result.SetViewCenter(lat, lon);
        result.SetZoomLevel(CalculateZoomLevelFromDistance(cameraHelper->GetViewPointDistance()));
        result.SetRotateAngle(cameraHelper->GetRotateAngle());
        result.SetTiltAngle(90.f - cameraHelper->GetTiltAngle());

        delete cameraHelper;
    }
}

double CameraHelper::CalculateDistanceFromZoomLevel(double zoomLevel)
{
    return NBGM_MapScaleUtility::CalculateCameraDistance(zoomLevel);
}

double CameraHelper::CalculateZoomLevelFromDistance(double distance)
{
    return NBGM_MapScaleUtility::CalculateZoomLevel(distance);
}
