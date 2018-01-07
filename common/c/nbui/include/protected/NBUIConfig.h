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
 @file     NBUIConfig.h
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.                
 
 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems, is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _NBUI_CONFIG_
#define _NBUI_CONFIG_
#include "nbgmrendersystemtype.h"

/*
    This file contains platform specific configuration for MapKit3D. Each platform can modify the returned values if desired.
*/

/* 
    For iPhone: wrap to static class (C++ code) to use it in .mm files (MapNativeHostView.mm or MapNativeView.mm)
    separate function (C code) won't be compiled if this function will be used in .mm files
*/

class NBUIConfig
{
public:
    
    /*! Get maximum tilt angle */
    static float getMaxTiltAngle();

    /*! Get the current minimum tilt angle based on the current zoom. 
     
        The tilt angle can be dependent on zoom level based on the configuration. It can be a funciton or a fixed value.
     
        @return minimum tilt angle
     */
    static float getMinTiltAngle(double zoom);

    /*! Get the minimum tilt angle during a tilt gesture.
     
        This angle can differ from getMinTiltAngle(). If it differs (and is less than the value returned by getMinTiltAngle()),
        then the map "snaps back" to the value returned by getMinTiltAngle(). The snapback is animated. 
     
        If no snap-back is desired, then this function should return the same value as getMinTiltAngle().
     */
    static float getMinTiltAngleDuringGesture(double zoom);

    /*! Get the frustum distance to the horizon based on the given zoom.
     
        This function calculates the distance to the horizon based on the given zoom. Keep in mind that based on the values returned by
        getMinTiltAngle() and getMinTiltAngleDuringGesture(), the horizon might not be visible at certain zoom levels.
     */
    static void getFrustumDistance(double zoom,                         /*!< Input: zoom */
                                   double& frustumDistance,             /*!< Output: frustum distance in meters (distance to horizon) */
                                   double& frustumDistanceRasterNear    /*!< Output: frustum distance, in meters, used for raster tiles close to the camera */
                                   );
    
    /*! Get the rotation limitation (if any) based on the given zoom level.
     
        If the rotation should be limited at certain zoom levels, then this function should return the rotation angle limitation.
    
        @return 'true' if the rotation angle is limited, 'false' if not
     */
    static bool limitRotationAngle(double zoom,                         /*!< Input: current zoom level */
                                   float& angleLimitation               /*!< Output: Limitation in degree of the rotation angle in +/- from zero degrees
                                                                                     (north up). Only valid if the function returns 'true'. */
                                   );
    
    // Zoom
    static float getMinZoom();
    static float getMaxZoom();

    // Inertia
    static float getCurrentInertia();

    // Field of view
    static float getPortraitVFov();
    static float getLandscapeVFov();
    
    /*! Get gesture parameters.
            
        Return parameters to modify the gesture handling in MapKit3D
     */
    static void getGestureParameters(double& moveMultiplier,                /*!< Determines the pan/move gesture speed */
                                     double& moveVelocityMultiplier,        /*!< Determines the animation for the pan/move gesture */
                                     double& pinchMultiplier,               /*!< Determines the pinch (zoom) gesture */
                                     double& pinchVelocityMultiplier,       /*!< Determines the animation for the pinch (zoom) gesture */
                                     double& tiltMultiplier,                /*!< Determines the tilt gesture speed */
                                     double& rotationVelocityMultiplier     /*!< Determines the animation speed for rotation */
                                     /* Add more parameters as needed */
                                     );
    
    static bool GetRotateByGestureSupported();
    static bool GetTiltByGestureSupported();
    static bool GetInlineTrafficSupported();

    static NBGM_RenderSystemType GetRenderSystemType();
};

#endif // _NBUI_CONFIG_
/*! @} */ 
