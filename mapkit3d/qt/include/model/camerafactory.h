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
    @file camerafactory.h
    @date 11/12/2014
    @defgroup mapkit3d
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
#ifndef __CAMERAFACTORY_H__
#define __CAMERAFACTORY_H__
#include "camerasetting.h"
#include "coordinate.h"
#include "latlngbound.h"
#include <QSharedPointer>
#include <QPoint>
#include "cameraparameters.h"
namespace locationtoolkit
{
class CameraFactory
{
public:
    /*! CameraFactory constructor.    */
     CameraFactory();

     /*! CameraFactory destructor.    */
     ~CameraFactory();

     /*!Creates a CameraParameters object.
      * @param width The screen width.
      * @param height The screen height.
      * @param LatLngBound The geographic rectangle that needs to present on the screen.
      * @param return The CameraParameters object. See {@link CameraParameters} for more.
      * @return The result for create cameraParameters object.
    */
     static QSharedPointer<CameraParameters> CreateCamera(int width,  int height,  const LatLngBound& latlngBound);

    /**
      * @name CreateCamera - Create a CameraParameters object to proper position by certain LatLngBound.
      * @param width
      * @param height
      * @param hFov
      * @param latlngBound
      * @param xPixelOffset
      * @param yPixelOffset
      * @return None.
      */
     static  QSharedPointer<CameraParameters> CreateCamera(int width, int height, const LatLngBound& latlngBound, double xPixelOffset, double yPixelOffset, double viewportWidth, double viewportHeight);

     /*!Creates a CameraParameters object.
      * This method will Create camera to proper position by certain camera setting.
      * @param screenPosition The target's position on the screen in terms of DP.
      * @param screenWidth The screen width.
      * @param screenHeight The screen height.
      * @param coordinates The target's geographic position on the earth's surface.
      * @param CameraSetting The cameraSetting.
      * @param heading The heading angel in degrees.
      * @param return The CameraParameters object. See {@link CameraParameters} for more.
      * @return The result for create cameraParameters object.
     */
     static QSharedPointer<CameraParameters> CreateCameraParametersForCoordinates( int screenWidth, int screenHeight, const Coordinates& coordinates,const CameraSetting & camerasetting , float heading);

     /*!Creates a CameraParameters object.
      * This method will Create camera to proper position by certain camera setting.
      * @param screenPosition The target's position on the screen in terms of DP.
      * @param screenWidth The screen width.
      * @param screenHeight The screen height.
      * @param coordinates The target's geographic position on the earth's surface.
      * @param CameraSetting The cameraSetting.
      * @param heading The heading angel in degrees.
      * @param return The CameraParameters object. See {@link CameraParameters} for more.
      * @return The result for create cameraParameters object.
     */
     static QSharedPointer<CameraParameters> CreateCameraParametersForCoordinatesAtPosition(const QPoint& screenPostion, int screenWidth, int screenHeight, const Coordinates& coordinates, float tilt, float zoomLevel, float heading );
};
}
#endif // __CAMERAFACTORY_H__
