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
 @file     NBUIConfig.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "NBUIConfig.h"
#include "math.h"


// The maximum tilt angle is defined here. The minimum tilt angle is calculated in getMinTiltAngle()
const float MAX_TILT_ANGLE = 90.0f;

// Value for tilt angle snapback. Set to zero if no snap-back is desired
const float TILT_SNAPBACK_IN_DEGREE = 4.0f;

// Zoom (camera height)
const float MIN_ZOOM_DEFAULT = 2;
const float MAX_ZOOM_DEFAULT = 22;

// Inertia
const float INERTIA_IN_DEGREE = 300;

// Field of view
const float PORTRAIT_V_FOV = 55;
const float LANDSCAPE_V_FOV = 55;

/* Viewing distance for frustum calculation. This determines how far the frustum is towards the horizon.

 The value FRUSTUM_VIEWING_DISTANCE also determines at what distance the sky is shown in NBGM
 */
const double FRUSTUM_REFERENCE_ZOOM                 = 24.0;     // Reference zoom level to use for calculation below
const double FRUSTUM_VIEWING_DISTANCE               = 20.0;     // In meters for reference zoom level. It doubles for each time a zoom level goes up
const double FRUSTUM_VIEWING_DISTANCE_RASTER_NEAR   = 12.0;     // Distance for the raster tiles close to the user

// The current configuration keeps the frustum distance the same (does not shrink it further) if the given zoom is reached.
// That means below (higher numerical value) that given zoom the horizon distance stays the same.
const double FRUSTUM_CUTOFF_ZOOM                    = 17.0;

// Parameters for gesture handling
const double MOVE_MULTIPLIER                = 1.0;
const double MOVE_VELOCITY_MULTIPLIER       = 0.05;
const double PINCH_MULTIPLIER               = 1.0;
const double PINCH_VELOCITY_MULTIPLIER      = 0.05;
const double TILT_MULTIPLIER                = 0.2;
const double ROTATION_VELOCITY_MULTIPLIER   = 0.2;

/*! Parameters for linear equation. See getMinTiltAngle() for more details. */
class LinearParameters
{
public:
    LinearParameters():zoom1(0), zoom2(0), tilt1(0), tilt2(0){};
    LinearParameters(double z1, double z2, double t1, double t2):zoom1(z1), zoom2(z2), tilt1(t1), tilt2(t2){};

    // cut-off points for linear equation
    double zoom1;
    double zoom2;
    double tilt1;
    double tilt2;
};


// Tilt functions ......................................................................................................

float
NBUIConfig::getMaxTiltAngle()
{
    return MAX_TILT_ANGLE;
}

/* See header for description */
float
NBUIConfig::getMinTiltAngle(double zoom)
{
    /*
        Based on current requirements:
        Zoom 0 -> 6.999:     no tilt(90)
        Zoom 7 -> 22:        tilt = 90 -> 23

        Line slope formula:
        f(tilt) = 90-(zl - 7)*(90 - 23)/(22 - 7)
     */
    return 0;

    if (zoom < 7.0)
    {
        // No tilt
        return 90;
    }

    LinearParameters parameters = LinearParameters(7, 22, 90, 23);
    const double Z1 = parameters.zoom1;
    const double Z2 = parameters.zoom2;
    const double T1 = parameters.tilt1;
    const double T2 = parameters.tilt2;
    double tilt = (((T2 - T1) / (Z2 - Z1)) * (zoom - Z1)) + T1;
    return tilt;
}

/* See header for description */
float
NBUIConfig::getMinTiltAngleDuringGesture(double zoom)
{
    /*
        For now always snap-back the same amount, independent from the zoom. This behavior could be changed if desired.
     */
    return getMinTiltAngle(zoom) - TILT_SNAPBACK_IN_DEGREE;
}

/* See header for description */
void
NBUIConfig::getFrustumDistance(double zoom,
                               double& frustumDistance,
                               double& frustumDistanceRasterNear)
{
    /*
        The viewing distance doubles for every zoom level
     */

    // If the zoom reaches the cut-off value then we don't shrink the horizon distance further for better user experience.
    if (zoom >= FRUSTUM_CUTOFF_ZOOM)
    {
        zoom = FRUSTUM_CUTOFF_ZOOM;
    }

    frustumDistance = FRUSTUM_VIEWING_DISTANCE * pow(2.0, (FRUSTUM_REFERENCE_ZOOM - zoom));

    frustumDistanceRasterNear = FRUSTUM_VIEWING_DISTANCE_RASTER_NEAR * pow(2.0, (FRUSTUM_REFERENCE_ZOOM - zoom));
}


// Rotation functions ...................................................................................................

/* See header for description */
bool
NBUIConfig::limitRotationAngle(double zoom,
                               float& angleLimitation)
{
    /*
        If no rotation limitation is desired then this function should always return false.
     */

    // New requirement is 0-8 No rotation, 8-22 Full rotation.
    const double CUTOFF = 8;

    if (zoom > CUTOFF)
    {
        // no limitation
        return false;
    }

    // Don't allow any rotation
    angleLimitation = 0;
    return true;


    // Gradually reduce the rotation starting from high-cutoff value to the low-cutoff value.

//    double HIGH_CUTOFF = 9.0;
//    double LOW_CUTOFF  = 8.0;
//
//    if (zoom <= LOW_CUTOFF)
//    {
//        angleLimitation = 0;
//        return true;
//    }
//
//    if (zoom >= HIGH_CUTOFF)
//    {
//        // no limitation
//        return false;
//    }
//
//    // y = m * x + b, where y is the limitation angle and x is the zoom.
//    angleLimitation = (180.0 / (HIGH_CUTOFF - LOW_CUTOFF)) * (zoom - LOW_CUTOFF) + LOW_CUTOFF;

    return true;
}


// Zoom functions ......................................................................................................

float
NBUIConfig::getMinZoom()
{
     return MIN_ZOOM_DEFAULT;
}

float
NBUIConfig::getMaxZoom()
{
    return MAX_ZOOM_DEFAULT;
}


// Inertia functions ...................................................................................................

float
NBUIConfig::getCurrentInertia()
{
    return INERTIA_IN_DEGREE;
}


// Field of view functions .............................................................................................

float
NBUIConfig::getPortraitVFov()
{
    return PORTRAIT_V_FOV;
}

float
NBUIConfig::getLandscapeVFov()
{
    return LANDSCAPE_V_FOV;
}

/* See header for description */
void
NBUIConfig::getGestureParameters(double& moveMultiplier,
                                 double& moveVelocityMultiplier,
                                 double& pinchMultiplier,
                                 double& pinchVelocityMultiplier,
                                 double& tiltMultiplier,
                                 double& rotationVelocityMultiplier)
{
    moveMultiplier              = MOVE_MULTIPLIER;
    moveVelocityMultiplier      = MOVE_VELOCITY_MULTIPLIER;
    pinchMultiplier             = PINCH_MULTIPLIER;
    pinchVelocityMultiplier     = PINCH_VELOCITY_MULTIPLIER;
    tiltMultiplier              = TILT_MULTIPLIER;
    rotationVelocityMultiplier  = ROTATION_VELOCITY_MULTIPLIER;
}

bool
NBUIConfig::GetRotateByGestureSupported()
{
    return false;
}

bool
NBUIConfig::GetTiltByGestureSupported()
{
    return false;
}

bool
NBUIConfig::GetInlineTrafficSupported()
{
    return true;
}

/*! @} */
