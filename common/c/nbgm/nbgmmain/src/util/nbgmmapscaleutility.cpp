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

    @file nbgmmapscaleutility.cpp
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/* @{ */

#include "palmath.h"
#include "paldisplay.h"
#include "nbgmmapscaleutility.h"
#include "nbgmconst.h"

#define BASE_DPI                                                    326
#define RELATIVE_ZOOM_LEVEL                                         0
#define METERS_PER_PIXEL_ON_RELATIVE_ZOOM_LEVEL                     156543.034

#define TO_DEG(x)                                                   nsl_mul(x, 180/PI)
#define TO_RAD(x)                                                   nsl_mul(x, PI/180)


static const float RELATIVE_CAMERA_DISTANCE = (111860800.0f * NBGM_RELATIVE_CAMERA_DISTANCE_MULTIPLE);

//
// Interal helper functions
//
static PAL_Error _GetDPI(uint32* dpi)
{
    static uint32 _dpi = (uint32)-1;
    PAL_Error palErr = PAL_Ok;

    if (_dpi == (uint32)-1)
    {
        uint32 width = (uint32)-1;
        uint32 height = (uint32)-1;
        palErr = PAL_DisplayGetInfo(PD_Main, &width, &height, &_dpi);
        if (palErr == PAL_Ok)
        {
            *dpi = _dpi;
        }
        else
        {
            *dpi = (uint32)-1;
        }
    }
    else
    {
        *dpi = _dpi;
    }

    return palErr;
}

static double _GetMetersPerPixelOnRelativeZoomLevel()
{
    double mpp = 1.0;

    uint32 dpi = (uint32)-1;
    PAL_Error palErr = _GetDPI(&dpi);
    if (palErr == PAL_Ok)
    {
        mpp = METERS_PER_PIXEL_ON_RELATIVE_ZOOM_LEVEL;
    }

    return mpp;
}

//
// NBGM_MapScaleUtility
//
double NBGM_MapScaleUtility::CalculateFOV(double viewHeight)
{
    //fov = 2*atan((C*H/2)/h)
    return TO_DEG(2 * nsl_atan((_GetMetersPerPixelOnRelativeZoomLevel() * viewHeight / 2) / RELATIVE_CAMERA_DISTANCE));
}

double NBGM_MapScaleUtility::CalculateZoomLevel(double cameraHeight)
{
    return RELATIVE_ZOOM_LEVEL - nsl_log2(cameraHeight / RELATIVE_CAMERA_DISTANCE);
}

double NBGM_MapScaleUtility::CalculateCameraDistance(double zoomLevel)
{
    return RELATIVE_CAMERA_DISTANCE * nsl_pow(2, RELATIVE_ZOOM_LEVEL - zoomLevel);
}

double NBGM_MapScaleUtility::MetersPerPixel(double zoomLevel)
{
    return _GetMetersPerPixelOnRelativeZoomLevel() / nsl_pow(2, zoomLevel);
}

double NBGM_MapScaleUtility::MetersPerPixelOnRelativeZoomLevel()
{
    return _GetMetersPerPixelOnRelativeZoomLevel();
}

double NBGM_MapScaleUtility::GetRelativeZoomLevel()
{
    return RELATIVE_ZOOM_LEVEL;
}

double NBGM_MapScaleUtility::GetRelativeCameraDistance()
{
    return RELATIVE_CAMERA_DISTANCE;
}

float NBGM_MapScaleUtility::GetReferenceAvatarHeight()
{
    return REFERENCE_AVATAR_BASIC_HEIGHT*NBGM_RELATIVE_CAMERA_DISTANCE_MULTIPLE;
}

/* @} */

