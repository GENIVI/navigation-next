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
 @file     MapViewAnimation.cpp
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
#include "MapViewCamera.h"
#include "palstdlib.h"
#include "palmath.h"
#include "paldisplay.h"
#include "NBUIConfig.h"
#include "Frame.h"
#include "StringUtility.h"
#include "nbmacros.h"
#include "nbgmmapcamerahelper.h"
#include "nbgmlinearinterpolator.h"
#include "nbgmdecelerateinterpolator.h"
#include "nbgmmapscaleutility.h"
#include "nbgmacceleratedecelerateinterpolator.h"
#include "MapViewCameraListener.h"
#include "nbgmaccelerateinterpolator.h"

extern "C"
{
#include "transformutility.h"
}

// const for relative zoom level and to calculate relative camera height
#define DEFAULT_ZOOM_LEVEL 17
#define MIN_ZOOMLEVEL_CAN_ROTATE_AND_TILT 8.0
#define MIN_VALID_TAN      0.0174550    // tan(1degree)
#define MIN_VALID_DISTANCE 1.5707963e-4 // 1000/RADIUS_EARTH_METERS
//It is a mercator value of infinite close to latitude 85.
#define MERCATOR_Y_BOUND 3.1313013314716454

//static const uint32 MAX_VISIBLE_TILE_MULTIPLE = 2;
static const float TILT_SNAPBACK_IN_DEGREE = 2.0f;
static const float ROTATE_SNAPBACK_IN_DEGREE = 15.0f;
static const float ZOOM_SNAPBACK = 0.2f;
static const float NONE_ROTATION_ZOOM_LEVEL = 4.0f;
static const double MIN_ZOOM_LEVEL = 3.5;
static const float MIN_TILTABLE_ZOOM_LEVEL = 7.0f;
static const float CAMERA_DISTANCE_EPSILON = 0.05f;
static const int MAX_SCREEN_HEIGHT_1080 = 1500;
static const int MAX_SCREEN_HEIGHT_2K   = 2000;
static const float MIN_TILT_ANLGE_1080  = 26;
static const float MIN_TILT_ANLGE_2K    = 33;

using namespace nbmap;

MapViewCamera::MapViewCamera(MapViewCameraListener * listener)
      : m_nbgmMapCameraHelper(NULL),
        m_lock(NULL),
        m_AnimationDuration(DEFAULT_ANIMATION_DURATION),
        m_AnimationAccelerationType(AAT_DECELERATION),
        m_combinedAnimation(listener),
        m_minCalculatedZoomLevel(0.0),
        m_rotateIsBounceingBack(false),
        m_isInRotationGesture(false),
        m_enableReference(FALSE),
        m_referenceX(0.0),
        m_referenceY(0.0),
        m_rotationCenterX(0.0),
        m_rotationCenterY(0.0),
        m_zoomLevelBounce(FALSE),
        m_zoomLevelBounceBack(FALSE),
        m_zoomCenterX(0.0f),
        m_zoomCenterY(0.0f),
        m_listener(listener),
        m_currentTiltRatio(1.f),
        m_tiltAnimationFlag(false),
        m_timestamp(0),
        m_lockForViewCenter(NULL),
		m_enableZoomCenter(false),
        m_scaleFactor(1.f),
        m_tiltIsBounceingBack(false),
        m_lastHorizonDistance(0.f),
        m_width(0),
        m_height(0)
{
}

MapViewCamera::~MapViewCamera()
{
    PAL_LockDestroy(m_lock);
    PAL_LockDestroy(m_lockForViewCenter);
}

NB_Error
MapViewCamera::Initiallize(PAL_Instance* palInstance)
{
    NBGM_CreateMapCameraHelper(&m_nbgmMapCameraHelper);

    m_minCalculatedZoomLevel = MIN_ZOOM_LEVEL;
    m_Fov = (float)NBGM_MapScaleUtility::CalculateFOV(m_height*m_scaleFactor);

    m_nbgmMapCameraHelper->SetViewPointDistance((float)CalculateDistanceFromZoomLevel(DEFAULT_ZOOM_LEVEL));
    
    PAL_LockCreate(palInstance, &m_lock);
    PAL_LockCreate(palInstance, &m_lockForViewCenter);
    return NE_OK;
}


void MapViewCamera::AnimateViewTransformation(uint32 currentTime)
{
    m_tiltAnimationFlag = false;
    if(m_combinedAnimation.IsAnimating(MapViewAnimation::AT_TILT))
    {
        m_tiltAnimationFlag = true;
        float tiltAngle = (float)m_combinedAnimation.GetInterpolation(MapViewAnimation::AT_TILT, currentTime);

        // Don't validate the tilt-angle in this call since we might animate passed the minimum
        SetTiltAngle(tiltAngle, false, false, false, 0);
    }

    if(m_combinedAnimation.IsAnimating(MapViewAnimation::AT_ROTATE))
    {
        float rotationAngle = (float)m_combinedAnimation.GetInterpolation(MapViewAnimation::AT_ROTATE, currentTime);
        Turn(rotationAngle - m_nbgmMapCameraHelper->GetRotateAngle());
    }

    if(m_combinedAnimation.IsAnimating(MapViewAnimation::AT_ZOOM))
    {
        double distance = m_combinedAnimation.GetInterpolation(MapViewAnimation::AT_ZOOM, currentTime);
        SetCameraDistance(distance, m_zoomLevelBounceBack);
    }

    if(m_combinedAnimation.IsAnimating(MapViewAnimation::AT_SET_CENTER_X) || m_combinedAnimation.IsAnimating(MapViewAnimation::AT_SET_CENTER_Y))
    {
        double x = m_combinedAnimation.GetInterpolation(MapViewAnimation::AT_SET_CENTER_X, currentTime);
        double y = m_combinedAnimation.GetInterpolation(MapViewAnimation::AT_SET_CENTER_Y, currentTime);
        SetViewCenter(x, y);
    }
}

static float
RoundingAngle(float angle)
{
    if(angle == NSL_INFINITY || nsl_isnan(angle))
    {
        return 0;
    }
    int32 count = 0;
    if(angle > 360)
    {
        count = static_cast<int32> (angle/360);
    }
    else if(angle < 0)
    {
        count = static_cast<int32> ( (angle-360)/360 );
    }
    angle -= (count * 360);
    return angle;
}

void
MapViewCamera::SetRotationAngle(float angle)
{
    angle = RoundingAngle(angle);

    double currentZoomLevel = GetZoomLevel();
    if (currentZoomLevel > 0)
    {
        // Check if we should limit the rotation angle based on the current zoom
        float limit = 0;
        if (CheckRotationAngle(limit))
        {
            m_listener->OnCameraLimited(MVCLT_ROTATION);
            if (m_isInRotationGesture)
            {
                if (angle > 180)
                {
                    float maxValue = 360 - ROTATE_SNAPBACK_IN_DEGREE;
                    angle = max(angle, maxValue);
                }
                else
                {
                    angle = min(angle, ROTATE_SNAPBACK_IN_DEGREE);
                }
            }
            else
            {
                angle = 0;
            }
        }
    }

    Turn(angle - m_nbgmMapCameraHelper->GetRotateAngle());
}

void
MapViewCamera::SetCameraDistance(double distance, bool gesture)
{
    double maxZoomLevel = GetMaxZoomLevel();
    double minZoomLevel = GetMinZoomLevel();
    if(gesture)
    {
        maxZoomLevel += ZOOM_SNAPBACK;
        minZoomLevel -= ZOOM_SNAPBACK;
    }

    if (distance < CalculateDistanceFromZoomLevel(maxZoomLevel))
    {
        distance = CalculateDistanceFromZoomLevel(maxZoomLevel);
    }
    else if (distance > CalculateDistanceFromZoomLevel(minZoomLevel))
    {
        distance = CalculateDistanceFromZoomLevel(minZoomLevel);
    }

    m_zoomLevelBounce = FALSE;
    if (distance < CalculateDistanceFromZoomLevel(NBUIConfig::getMaxZoom()))
    {
        m_zoomLevelBounce = TRUE;
    }
    else if (distance > CalculateDistanceFromZoomLevel(m_minCalculatedZoomLevel))
    {
        m_zoomLevelBounce = TRUE;
    }

    if(m_enableReference)
    {
        m_zoomCenterX = m_referenceX;
        m_zoomCenterY = m_referenceY;
    }

    float preDistance = m_nbgmMapCameraHelper->GetViewPointDistance();
    if(m_enableReference || m_enableZoomCenter)
    {
        double mx = 0.0;
        double my = 0.0;
        double new_mx = 0.0;
        double new_my = 0.0;
        m_nbgmMapCameraHelper->ScreenToMapPosition(m_zoomCenterX, m_zoomCenterY, mx, my);
        m_nbgmMapCameraHelper->SetViewPointDistance((float)distance);
        m_nbgmMapCameraHelper->ScreenToMapPosition(m_zoomCenterX, m_zoomCenterY, new_mx, new_my);

        double x, y;
        m_nbgmMapCameraHelper->GetViewCenter(x, y);

        SetViewCenter(x - (new_mx - mx), y - (new_my - my));
    }
    else
    {
        m_nbgmMapCameraHelper->SetViewPointDistance((float)distance);
    }


    // When the zoom changes, we also have to ensure that the tilt is still within its range. The tilt can be limited
    // based on the zoom.
    float tiltAngle = 90.0f - m_nbgmMapCameraHelper->GetTiltAngle();
    SetTiltAngle(tiltAngle, false, true, false, 0);

    // Limit the rotation angle, if necessary
    if(!m_zoomLevelBounceBack)
    {
        SetRotationAngle(m_nbgmMapCameraHelper->GetRotateAngle());
    }

    // Also ensure that the map center gets adjusted, if necessary. This is only necessary in higher zoom levels (zoom 2, 3, 4, ...) and if
    // the map is at the "edge of the world". In that case the center needs to be adjusted to not show a grey area at the top/bottom of the map
    double x, y;
    m_nbgmMapCameraHelper->GetViewCenter(x, y);
    SetViewCenter(x, y);

    // Ensure that zoom out then zoom in shall result in the same tilt.
    if(!m_tiltAnimationFlag &&  nsl_fabs(preDistance - distance) >= CAMERA_DISTANCE_EPSILON)
    {
        m_nbgmMapCameraHelper->SetTiltAngle(90.f - GetTiltAngleFromTiltRatio());
    }
}

/*!
 We will check the mercator point before invoke 'm_nbgmMapView->SetViewCenter'
 if to do so, we will avoid the white space displaying during the zoom level at map boundary(North+South).
 Additionally, check the mercator point only when no tilt and rotate, if tilt and rotate is available(begin at zoom level 8),
 and then we set map center directly.
 */
void 
MapViewCamera::SetViewCenter(double mercatorX, double mercatorY)
{
    if(!m_zoomLevelBounce)
    {
        if(CheckMapCenterData(mercatorX, mercatorY))
        {
            m_listener->OnCameraLimited(MVCLT_LOCATION);
        }
    }
    m_nbgmMapCameraHelper->SetViewCenter(mercatorX, mercatorY);
}

bool 
MapViewCamera::CheckMapCenterData(double &mercatorX, double &mercatorY)
{
    static double DOUBLE_ZERO = 1/RADIUS_EARTH_METERS;

    double currentZoomLevel = GetZoomLevel();
    bool isLimited = false;

    /*!
     Because the map view can be rotate or tilt, so this function is NOT fit for these case.
     We set the map center directly when zoom level > 8
     */
    if(currentZoomLevel >= MIN_ZOOMLEVEL_CAN_ROTATE_AND_TILT)
    {
        return isLimited;
    }

    double mtop = 0.0;
    double mleft = 0.0;
    double mbottom = 0.0;
    double mcenterx = 0.0;
    double mcentery = 0.0;

    ScreenToMapPosition(0, 0, mleft, mtop);
    ScreenToMapPosition(m_width/2.0f, m_height/2.0f, mcenterx, mcentery);
    double len = mtop - mcentery;
    mbottom = mcentery - len;
    double mNewCentery = mercatorY;
    double det = mercatorY - mcentery;

    // Pan map from up to down.
    if(det > DOUBLE_ZERO)
    {
        //Handle the map top already out of up range.
        if(mtop >= (MERCATOR_Y_BOUND) )
        {
            mNewCentery = mcentery - (mtop - MERCATOR_Y_BOUND);
            isLimited = true;
        }
        else
        {
            //Handle case that the map will display the white space in the north.
            if(mercatorY + len >= (MERCATOR_Y_BOUND))
            {
                mNewCentery = MERCATOR_Y_BOUND -len;
                isLimited = true;
            }

            //Handle case that the map will display the white space in the south.
            if(mercatorY - len <= -MERCATOR_Y_BOUND)
            {
                mNewCentery = -MERCATOR_Y_BOUND + len;
                isLimited = true;
            }
        }
    }

    // Pan map from down to up.
    else if(det < -DOUBLE_ZERO)
    {
        //Handle the map bottom already out of down range.
        if( mbottom <= (-MERCATOR_Y_BOUND) )
        {
            mNewCentery = mcentery + (-mbottom - MERCATOR_Y_BOUND);
            isLimited = true;
        }
        else
        {
            //Handle case that the map will display the white space in the south.
            if(mercatorY - len <= -MERCATOR_Y_BOUND)
            {
                mNewCentery = -MERCATOR_Y_BOUND + len;
                isLimited = true;
            }

            //Handle case that the map will display the white space in the north.
            if(mercatorY + len >= (MERCATOR_Y_BOUND))
            {
                mNewCentery = MERCATOR_Y_BOUND -len;
                isLimited = true;
            }
        }
    }
    else if(det > -DOUBLE_ZERO && det < DOUBLE_ZERO)
    {
        /*!
          This case only handle the white space displaying during the screen toggle to landscape or portrait.
          this case often happen when device screen switching at zoom level 2 or 3.
          if check the top or bottom out of boundary, we will resume it.
         */
        if(mcentery >= 0)
        {
            if(mtop > MERCATOR_Y_BOUND)
            {
                //North
                mNewCentery = MERCATOR_Y_BOUND -len;
                isLimited = true;
            }
        }
        else
        {
            if(mbottom < -MERCATOR_Y_BOUND)
            {
                //South
                mNewCentery = -MERCATOR_Y_BOUND + len;
                isLimited = true;
            }
        }
    }

    mercatorY = mNewCentery;
    return isLimited;
}

bool
MapViewCamera::ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY)
{
    Lock lock(m_lock);
    return m_nbgmMapCameraHelper->ScreenToMapPosition(screenX,screenY,mercatorX,mercatorY);
}

bool
MapViewCamera::MapPositionToWindow(double mercatorX, double mercatorY, float* dX, float* dY)
{
    Lock lock(m_lock);
    return m_nbgmMapCameraHelper->MapPositionToScreen(*dX, *dY, mercatorX, mercatorY);
}

void
MapViewCamera::Turn(float rotationAngle)
{
    float limit = 0.f;
    bool isRotationLimited = CheckRotationAngle(limit);
    if(isRotationLimited)
    {
        m_listener->OnCameraLimited(MVCLT_ROTATION);
    }
    if(m_enableReference)
    {
        m_nbgmMapCameraHelper->SetRotateCenter(m_referenceX, m_referenceY);
    }
    else
    {
        m_nbgmMapCameraHelper->SetRotateCenter(float(m_rotationCenterX), float(m_rotationCenterY));
    }
    // Rotate if allowed
    if(!isRotationLimited)
    {
        m_nbgmMapCameraHelper->Rotate(rotationAngle);
    }
    else
    {
        float prevAngle = m_nbgmMapCameraHelper->GetRotateAngle();
        float nextAngle = prevAngle + rotationAngle;
        if (nextAngle > ROTATE_SNAPBACK_IN_DEGREE &&
            nextAngle < 360.0f - ROTATE_SNAPBACK_IN_DEGREE)
        {
            nextAngle = nextAngle > 180.0f ? 360.0f - ROTATE_SNAPBACK_IN_DEGREE : ROTATE_SNAPBACK_IN_DEGREE;
            rotationAngle = nextAngle - prevAngle;
        }
        m_nbgmMapCameraHelper->Rotate(rotationAngle);
    }
}

void
MapViewCamera::Zoom(double zoomIndex, bool gesture)
{
    double distance = m_nbgmMapCameraHelper->GetViewPointDistance()*zoomIndex;
    SetCameraDistance(distance, gesture);
}

void
MapViewCamera::SetMapCenter(double mx, double my, bool bAnimated, bool stopExistingAnimation, uint32 duration)
{
    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    if (bAnimated)
    {
        double x, y;
        m_nbgmMapCameraHelper->GetViewCenter(x, y);
        if((mx -  x) > PI)
        {
            mx -= TWO_PI;
        }
        else if((mx -  x) < -PI)
        {
            mx += TWO_PI;
        }

        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_SET_CENTER_X,
                                           x,
                                           mx,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_SET_CENTER_Y,
                                           y,
                                           my,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }

    SetViewCenter(mx, my);
}

NBGM_Interpolator<double>*
MapViewCamera::GenerateInterpolator(AnimationAccelerationType type)
{
    NBGM_Interpolator<double>* interpolator = NULL;
    switch(type)
    {
    case AAT_LINEAR:
        interpolator = new NBGM_LinearInterpolator<double>();
        break;
    case AAT_DECELERATION:
        interpolator = new NBGM_DecelerateInterpolator<double>();
        break;
    case AAT_ACCELERATEDECELERATE:
        interpolator = new NBGM_AccelerateDecelerateInterpolator<double>();
        break;
    case AAT_ACCELERATION:
        interpolator = new NBGM_AccelerateInterpolator<double>();
        break;
    default:
        break;
    }
    return interpolator;
}

void
MapViewCamera::SetRotationAngle(float angle, bool bAnimated, bool stopExistingAnimation)
{
    Lock lock(m_lock);
    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_X);
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_Y);
    }

    if (bAnimated)
    {
        //we will always make the short turn
        //if the ture angle is bigger than 180
        //the turn will be reversed

        float deltaAngle = RoundingAngle(angle - m_nbgmMapCameraHelper->GetRotateAngle());
        if(deltaAngle > 180)
        {
            deltaAngle -= 360;
        }
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_ROTATE,
                                           m_nbgmMapCameraHelper->GetRotateAngle(),
                                           m_nbgmMapCameraHelper->GetRotateAngle() + deltaAngle,
                                           m_AnimationDuration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }
    SetRotationAngle(angle);
}

float
MapViewCamera::GetTiltAngle()
{
    Lock lock(m_lock);
    return 90.0f - m_nbgmMapCameraHelper->GetTiltAngle();
}

float
MapViewCamera::GetRotateAngle()
{
    Lock lock(m_lock);
    return m_nbgmMapCameraHelper->GetRotateAngle();
}

/* See header for description */
void
MapViewCamera::SetTiltAngle(float& tiltAngle, bool stopAllAnimation, bool validate, bool gesture, uint32 animationTime)
{
    // No need to lock, this always gets called from the render thread.
    Lock lock(m_lock);
    if (stopAllAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    if (gesture)
    {
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_X);
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_Y);
    }

    if (animationTime != 0)
    {
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_TILT,
                                           90.0f - m_nbgmMapCameraHelper->GetTiltAngle(),
                                           tiltAngle,
                                           animationTime,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }

    if (validate)
    {
        tiltAngle = RoundingAngle(tiltAngle);

        double currentZoomLevel = GetZoomLevel();
        if (currentZoomLevel > 0)
        {
            float angleMinimum = GetMinTiltAngle();

            // Get minimum tilt angle based on the current zoom level.
            if (gesture)
            {
                angleMinimum = GetMinTiltAngleDuringGesture(angleMinimum);
            }

            float angleMaximum = NBUIConfig::getMaxTiltAngle();

            tiltAngle = min(tiltAngle, angleMaximum);
            tiltAngle = max(tiltAngle, angleMinimum);
        }
    }

    if(gesture)
    {
        UpdateTiltRatio(INVALID_CAMERA_ZOOMLEVEL, tiltAngle);
    }

    if(m_enableReference)
    {
        double mx = 0.0;
        double my = 0.0;
        double new_mx = 0.0;
        double new_my = 0.0;
        double x, y;

        Lock lk(m_lockForViewCenter);
        m_nbgmMapCameraHelper->ScreenToMapPosition(m_referenceX, m_referenceY, mx, my);
        m_nbgmMapCameraHelper->SetTiltAngle(90 - tiltAngle);
        m_nbgmMapCameraHelper->ScreenToMapPosition(m_referenceX, m_referenceY, new_mx, new_my);
        m_nbgmMapCameraHelper->GetViewCenter(x, y);
        SetViewCenter(x - (new_mx - mx), y - (new_my - my));
    }
    else
    {
        m_nbgmMapCameraHelper->SetTiltAngle(90 - tiltAngle);
    }
}

/* See header for description */
void
MapViewCamera::BeginMove(double dX, double dY, bool stopExistingAnimation)
{
    Lock lock(m_lock);
    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }
    m_previousScreenX = dX;
    m_previousScreenY = dY;
    m_isTouched = m_nbgmMapCameraHelper->ScreenToMapPosition((float)dX, (float)dY, m_previousMercatorX, m_previousMercatorY, false);
}

/* See header for description */
void
MapViewCamera::MoveTo(double x, double y, bool bAnimated, bool stopExistingAnimation, uint32 duration)
{
    Lock lock(m_lock);
    if(!m_isTouched)
    {
        return;
    }

    double mercatorX, mercatorY = 0.0;
    bool isMoved = m_nbgmMapCameraHelper->ScreenToMapPosition((float)x, (float)y, mercatorX, mercatorY, false);

    if(!isMoved && bAnimated)
    {
        float direction = 0.0f;
        isMoved = m_nbgmMapCameraHelper->ScreenToMapDirection((float) m_previousScreenX, (float) m_previousScreenY, (float) x, (float) y, direction);
        if(!isMoved)
        {
            return;
        }
        double distance = nsl_sqrt((x - m_previousScreenX) * (x - m_previousScreenX) + (y - m_previousScreenY) * (y - m_previousScreenY));
        double currentZoomLevel = GetZoomLevel();
        distance = NBGM_MapScaleUtility::MetersPerPixel(currentZoomLevel) * distance /RADIUS_EARTH_METERS;
        direction = 90.0f - direction;
        while(direction > 360)
        {
            direction -= 360;
        }

        while(direction < 0)
        {
            direction += 360;
        }
        mercatorX = m_previousMercatorX + distance * nsl_cos(TO_RAD(direction));
        mercatorY = m_previousMercatorY + distance * nsl_sin(TO_RAD(direction));
    }

    if(!isMoved)
    {
        return;
    }

    double currentMercatorX, currentMercatorY = 0.0;
    m_nbgmMapCameraHelper->GetViewCenter(currentMercatorX, currentMercatorY);
    double destinationMercatorX, destinationMercatorY = 0.0;

    destinationMercatorX = currentMercatorX + m_previousMercatorX - mercatorX;
    destinationMercatorY = currentMercatorY + m_previousMercatorY - mercatorY;

    CheckMapCenterData(destinationMercatorX, destinationMercatorY);

    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    if (bAnimated)
    {
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_SET_CENTER_X,
                                           currentMercatorX,
                                           destinationMercatorX,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_SET_CENTER_Y,
                                           currentMercatorY,
                                           destinationMercatorY,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }
    SetViewCenter(destinationMercatorX, destinationMercatorY);
}

/* See header for description */
void
MapViewCamera::Turn(double x, double y, float rotationAngle, bool bAnimated, bool stopExistingAnimation, uint32 duration)
{
    Lock lock(m_lock);
    m_rotationCenterX = x;
    m_rotationCenterY = y;

    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    if (bAnimated)
    {
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_ROTATE,
                                           m_nbgmMapCameraHelper->GetRotateAngle(),
                                           m_nbgmMapCameraHelper->GetRotateAngle() + rotationAngle,
                                           duration,
                                           GenerateInterpolator(AAT_ACCELERATEDECELERATE));
        return;
    }

    Turn(rotationAngle);
}

/* See header for description */
void
MapViewCamera::Zoom(double zoomIndex, bool bAnimated, bool stopExistingAnimation, bool gesture, uint32 duration)
{
    Lock lock(m_lock);
    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    //sometimes, Mapview will pass a invalid zoom index, and this will case map zoom out of controll
    //@todo: comment following condition statement if something is wrong.
    /*
    if(zoomIndex < 0.001 || zoomIndex == 1)
    {
        return;
    }
    */
    m_enableZoomCenter = false;
    if (bAnimated)
    {
        double dis = m_nbgmMapCameraHelper->GetViewPointDistance() * zoomIndex;
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_ZOOM,
                                           m_nbgmMapCameraHelper->GetViewPointDistance(),
                                           dis,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }

    Zoom(zoomIndex, gesture);
}

/* See header for description */
void
MapViewCamera::SetZoom(double zoom, bool animated, bool stopExistingAnimation)
{
    Lock lock(m_lock);
    double currentZoomLevel = GetZoomLevel();
    if (zoom == currentZoomLevel)
    {
        return;
    }

    double scale = pow(2.0, (currentZoomLevel - zoom));
    Zoom(scale, animated, stopExistingAnimation);
}

void
MapViewCamera::DoubleTapZoom(double zoomIndex, float locationInViewX, float locationInViewY, bool bAnimated, bool stopExistingAnimation, uint32 duration)
{
    Lock lock(m_lock);
    if(m_enableReference)
    {
        ZoomAroundGestureCenter(zoomIndex, m_referenceX, m_referenceY, bAnimated, stopExistingAnimation,false, duration);
    }
    else
    {
        CombinedAnimatedZoomAndPanTo(zoomIndex, locationInViewX, locationInViewY, bAnimated, stopExistingAnimation, duration);
    }
}

/* See header for description */
void
MapViewCamera::ZoomAroundGestureCenter(double zoomIndex, float locationInViewX, float locationInViewY, bool bAnimated, bool stopExistingAnimation, bool gesture, uint32 duration)
{
    Lock lock(m_lock);
    if(m_enableReference)
    {
        m_zoomCenterX = m_referenceX;
        m_zoomCenterY = m_referenceY;
    }
    else
    {
        m_zoomCenterX = locationInViewX;
        m_zoomCenterY = locationInViewY;
        m_enableZoomCenter = true;
    }

    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    if (bAnimated)
    {
        double dis = m_nbgmMapCameraHelper->GetViewPointDistance() * zoomIndex;
        m_combinedAnimation.StartAnimation(MapViewAnimation::AT_ZOOM,
                                           m_nbgmMapCameraHelper->GetViewPointDistance(),
                                           dis,
                                           duration,
                                           GenerateInterpolator(m_AnimationAccelerationType));
        return;
    }

    double mx = 0.0;
    double my = 0.0;
    double new_mx = 0.0;
    double new_my = 0.0;
    m_nbgmMapCameraHelper->ScreenToMapPosition(m_zoomCenterX, m_zoomCenterY, mx, my);
    Zoom(zoomIndex, gesture);
    m_nbgmMapCameraHelper->ScreenToMapPosition(m_zoomCenterX, m_zoomCenterY, new_mx, new_my);

    double x, y;
    m_nbgmMapCameraHelper->GetViewCenter(x, y);

    SetViewCenter(x - (new_mx - mx), y - (new_my - my));
}

/* See header for description */
void
MapViewCamera::CombinedAnimatedZoomAndPanTo(double zoomIndex, double x, double y, bool bAnimated, bool stopExistingAnimation, uint32 duration)
{
    Lock lock(m_lock);
    if(stopExistingAnimation)
    {
        m_combinedAnimation.StopAnimations();
    }

    double mx = 0;
    double my = 0;
    Zoom(zoomIndex, true, false, false, duration);
    if(m_nbgmMapCameraHelper->ScreenToMapPosition((float)x, (float)y, mx, my))
    {
        SetMapCenter(mx, my, true, false, duration);
    }
}

/* See header for description */
bool
MapViewCamera::IsAnimating(MapViewAnimation::AnimationType type) const
{
    Lock lock(m_lock);
    return m_combinedAnimation.IsAnimating(type);
}

/* See header for description */
bool
MapViewCamera::IsAnimating()
{
    Lock lock(m_lock);
    return m_combinedAnimation.IsAnimating();
}

/* See header for description */
void
MapViewCamera::GetCurrentPosition(double &currX, double &currY)
{
    Lock lock(m_lock);
    m_nbgmMapCameraHelper->GetViewCenter(currX, currY);
}


/* See header for description */
double
MapViewCamera::GetZoomLevel()
{
    Lock lock(m_lock);
    return NBGM_MapScaleUtility::CalculateZoomLevel(m_nbgmMapCameraHelper->GetViewPointDistance());
}

double
MapViewCamera::CalculateDistanceFromZoomLevel(double zoomLevel)
{
    return NBGM_MapScaleUtility::CalculateCameraDistance(zoomLevel);
}

double
MapViewCamera::CalculateZoomLevelFromDistance(double distance)
{
    return NBGM_MapScaleUtility::CalculateZoomLevel(distance);
}

void
MapViewCamera::OnScreenChanged()
{
    UpdateFovAngle();
    m_nbgmMapCameraHelper->SetViewSize(0, 0, (unsigned)m_width, (unsigned)m_height);
    m_nbgmMapCameraHelper->SetPerspective(m_Fov, m_width/(float)m_height);

    double tanFov = tan(TO_RAD(m_Fov/2));
    if(tanFov > MIN_VALID_TAN)
    {
        double maxCameraHegiht = RADIUS_EARTH_METERS*MERCATOR_Y_BOUND/tanFov;
        m_minCalculatedZoomLevel = MAX(m_minCalculatedZoomLevel, NBGM_MapScaleUtility::CalculateZoomLevel(maxCameraHegiht));
    }

    CheckBoundary();
    float tiltAngle = GetTiltAngle();
    SetTiltAngle(tiltAngle, false, true, false, 0);
}

/* See header file for description */
void
MapViewCamera::UpdateFovAngle()
{
    m_Fov = (float)NBGM_MapScaleUtility::CalculateFOV(m_height*m_scaleFactor);
}

void
MapViewCamera::SetFrameBufferSize(int width, int height)
{
    Lock lock(m_lock);
    if (m_width == width && m_height == height)
    {
        return;
    }

    m_width = width;
    m_height = height;
    OnScreenChanged();
}

bool
MapViewCamera::GetFrustum(vector<NBGM_Point2d64>& frustum,
                               vector<NBGM_Point2d64>& frustumRasterNear)
{
    /*
     For now always get all frustums needed. Could be optimized.
     */

     /*
        @TODO: Why we input -1 for both frustum and frustumRasterNear?
        If we input -1 for the first parameter the frustum weil be same value, so could we remove one of them?
    */
    // Get frustum for vector
    Lock lock(m_lock);
    if (! m_nbgmMapCameraHelper->GetFrustumPositionInWorld(-1, frustum))
    {
        return false;
    }

    // Get frustum for raster near
    if (! m_nbgmMapCameraHelper->GetFrustumPositionInWorld(-1, frustumRasterNear))
    {
        return false;
    }

    return true;
}

void 
MapViewCamera::StopAnimations(int animationType)
{
    Lock lock(m_lock);
    if(animationType == MVAT_NONE)
    {
        return;
    }
    // Tilt back animation cannot be stopped.
    if(m_tiltIsBounceingBack)
    {
        animationType &= (~MVAT_TILT);
    }

    // Rotate back animation cannot be stopped.
    if (m_rotateIsBounceingBack)
    {
        animationType &= (~MVAT_ROTATE);
    }

    if (m_zoomLevelBounceBack)
    {
        animationType &= (~MVAT_ZOOM);
    }

    if(animationType == MVAT_TOTAL)
    {
        m_combinedAnimation.StopAnimations();
        return;
    }

    vector<MapViewAnimation::AnimationType> anmations;
    if(animationType & MVAT_TILT)
    {
        anmations.push_back(MapViewAnimation::AT_TILT);
    }
    if(animationType & MVAT_ROTATE)
    {
        anmations.push_back(MapViewAnimation::AT_ROTATE);
    }
    if(animationType & MVAT_ZOOM)
    {
        anmations.push_back(MapViewAnimation::AT_ZOOM);
    }
    if(animationType & MVAT_MOVE)
    {
        anmations.push_back(MapViewAnimation::AT_SET_CENTER_X);
        anmations.push_back(MapViewAnimation::AT_SET_CENTER_Y);
    }

    m_combinedAnimation.StopAnimations(anmations);
}

void
MapViewCamera::SetCamera(double lat, double lon, double heading, double zoomLevel, double tilt, uint32 timestamp, bool animated, uint32 duration, AnimationAccelerationType animationAccelerationType, int cameraAnimationId)
{
    Lock lock(m_lock);
    if(timestamp > m_timestamp)
    {
        m_timestamp = timestamp;
    }
    else
    {
        if(animated && cameraAnimationId != 0)
        {
            m_listener->OnCameraAnimationDone(cameraAnimationId, MVAST_CANCELED);
        }
        return;
    }

    m_combinedAnimation.TraceAnimation(cameraAnimationId);

    // treat height as distance directly, in order to avoid interdependent
    m_AnimationDuration = duration;
    m_AnimationAccelerationType = animationAccelerationType;

    if (zoomLevel != INVALID_CAMERA_ZOOMLEVEL)
    {
        if(m_zoomLevelBounceBack)
        {
            m_combinedAnimation.StopAnimation(MapViewAnimation::AT_ZOOM);
            m_tiltIsBounceingBack = false;
        }
        double distance = CalculateDistanceFromZoomLevel(zoomLevel);
        double zoomzIndex = distance/m_nbgmMapCameraHelper->GetViewPointDistance();
        Zoom(zoomzIndex, animated, false, false, duration);
    }

    float tiltAngle = (float)tilt;
    if (tiltAngle != INVALID_CAMERA_TILT)
    {
        if(m_tiltIsBounceingBack)
        {
            m_combinedAnimation.StopAnimation(MapViewAnimation::AT_TILT);
            m_tiltIsBounceingBack = false;
        }

        if(animated)
        {
            SetTiltAngle(tiltAngle, false, true, false, m_AnimationDuration);
        }
        else
        {
            SetTiltAngle(tiltAngle, false, true, false, 0);
        }
        UpdateTiltRatio((float)zoomLevel, tiltAngle);
    }

    if (heading != INVALID_CAMERA_HEADING)
    {
        if(m_rotateIsBounceingBack)
        {
            m_combinedAnimation.StopAnimation(MapViewAnimation::AT_ROTATE);
            m_rotateIsBounceingBack = false;
        }

        m_rotationCenterX = m_width / 2.0;
        m_rotationCenterY = m_height / 2.0;
        SetRotationAngle((float)heading, animated, false);
    }

    if (lat != INVALID_CAMERA_LOCATION && lon != INVALID_CAMERA_LOCATION)
    {
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_X);
        m_combinedAnimation.StopAnimation(MapViewAnimation::AT_SET_CENTER_Y);
        double mx=0;
        double my=0;
        mercatorForward(lat, lon, &mx, &my);
        Lock lk(m_lockForViewCenter);
        SetMapCenter(mx, my, animated, false, m_AnimationDuration);
    }

    UpdateFrustumHeight();
    m_AnimationDuration = DEFAULT_ANIMATION_DURATION;
    m_AnimationAccelerationType = AAT_DECELERATION;
}

void
MapViewCamera::UpdateFrustumHeight()
{
    // No need to lock here. We only read the shared data m_currentZoomLevel. m_lastHorizonDistance is only acessed from the render thread.

    double viewDistance = 0;
    double viewDistanceRasterNear = 0;  // Not needed here
    double currentZoomLevel = GetZoomLevel();
    NBUIConfig::getFrustumDistance(currentZoomLevel, viewDistance, viewDistanceRasterNear);

    // Set the distance to the horizon in NBGM.

    if (viewDistance != m_lastHorizonDistance)
    {
        //m_nbgmMapCameraHelper->SetHorizonDistance(viewDistance);
        m_lastHorizonDistance = viewDistance;
    }
}

void
MapViewCamera::SetCurrentPositon(double lat, double lon, bool animated, bool stopExistingAnimation)
{
    Lock lock(m_lock);
    double mx = 0;
    double my = 0;
    mercatorForward(lat, lon, &mx, &my);
    SetMapCenter(mx, my, animated, stopExistingAnimation, DEFAULT_ANIMATION_DURATION);
}


void 
MapViewCamera::GetCamera(double& lat, double& lon, float& zoomLevel, float& heading, float& tilt)
{
    Lock lock(m_lock);
    AnimateViewTransformation(PAL_ClockGetTimeMs());
    double x, y;
    m_nbgmMapCameraHelper->GetViewCenter(x, y);
    mercatorReverse(x, y, &lat, &lon);
    tilt = 90.0f - m_nbgmMapCameraHelper->GetTiltAngle();
    heading = m_nbgmMapCameraHelper->GetRotateAngle();
    zoomLevel = (float)GetZoomLevel();
}

void 
MapViewCamera::GetRenderCamera(double& mx, double& my, double& altitude, float& heading, float& tilt)
{
    Lock lock(m_lock);
    AnimateViewTransformation(PAL_ClockGetTimeMs());
    m_nbgmMapCameraHelper->GetViewCenter(mx, my);
    tilt = 90.0f - m_nbgmMapCameraHelper->GetTiltAngle();
    heading = m_nbgmMapCameraHelper->GetRotateAngle();
    altitude = m_nbgmMapCameraHelper->GetViewPointDistance();
}

float 
MapViewCamera::GetFov()
{
    Lock lock(m_lock);
    return m_Fov;
}

void 
MapViewCamera::GetScreenSize(int& width, int& height)
{
    Lock lock(m_lock);
    width = m_width;
    height = m_height;
}

bool 
MapViewCamera::GetTiltIsBouncingBackFlag()  
{ 
    bool animating = m_combinedAnimation.IsAnimating(MapViewAnimation::AT_TILT);
    if(m_tiltIsBounceingBack && !animating)
    {
        m_tiltIsBounceingBack = false;
    }
    return m_tiltIsBounceingBack;
}

bool 
MapViewCamera::GetRotateIsBouncingBackFlag()  
{ 
    bool animating = m_combinedAnimation.IsAnimating(MapViewAnimation::AT_ROTATE);
    if(m_rotateIsBounceingBack && !animating)
    {
        m_rotateIsBounceingBack = false;
    }
    return m_rotateIsBounceingBack;
}

bool
MapViewCamera::GetZoomIsBouncingBackFlag()
{
    bool animating = m_combinedAnimation.IsAnimating(MapViewAnimation::AT_ZOOM);
    if(m_zoomLevelBounceBack && !animating)
    {
        m_zoomLevelBounceBack = false;
    }
    return m_zoomLevelBounceBack;
}

int8
MapViewCamera::GetRelativeZoomLevel()
{
    return static_cast<int8>(NBGM_MapScaleUtility::GetRelativeZoomLevel());
}

float
MapViewCamera::GetRelativeCameraHeight()
{
    return static_cast<float>(NBGM_MapScaleUtility::GetRelativeCameraDistance());
}

float
MapViewCamera::GetMetersPerPixelOnRelativeZoomLevel()
{
    return static_cast<float>(NBGM_MapScaleUtility::MetersPerPixelOnRelativeZoomLevel());
}

float MapViewCamera::GetLimitedTiltAnlge(double zoomLevel)
{
    float tiltAngle = NBUIConfig::getMinTiltAngle(zoomLevel);
    if(m_height >= MAX_SCREEN_HEIGHT_1080 && m_height < MAX_SCREEN_HEIGHT_2K)
    {
        if(tiltAngle < MIN_TILT_ANLGE_1080)
        {
            tiltAngle = MIN_TILT_ANLGE_1080;
        }
    }
    else if(m_height >= MAX_SCREEN_HEIGHT_2K)
    {
        if(tiltAngle < MIN_TILT_ANLGE_2K)
        {
            tiltAngle = MIN_TILT_ANLGE_2K;
        }
    }
    return tiltAngle;
}

float MapViewCamera::GetMinTiltAngle()
{
    return GetLimitedTiltAnlge(GetZoomLevel());
}

float MapViewCamera::GetMinTiltAngleDuringGesture(float minTiltAngle)
{
    // Check m_enableReference in order to avoid map shaking issue.
    if(GetZoomLevel() > MIN_TILTABLE_ZOOM_LEVEL && !m_enableReference)
    {
        return minTiltAngle - TILT_SNAPBACK_IN_DEGREE;
    }
    return minTiltAngle;
}

void
MapViewCamera::SetReference(float screenX, float screenY)
{
    Lock lock(m_lock);
    m_referenceX = screenX;
    m_referenceY = screenY;
}

void
MapViewCamera::EnableReference(nb_boolean enable)
{
    Lock lock(m_lock);
    m_enableReference = enable;
}

void MapViewCamera::CheckAnimationStatus()
{
    Lock lock(m_lock);
    m_combinedAnimation.CheckAnimationStatus();
}

double
MapViewCamera::GetMaxZoomLevel()
{
    return NBUIConfig::getMaxZoom();
}

double
MapViewCamera::GetMinZoomLevel()
{
    Lock lock(m_lock);
    return m_minCalculatedZoomLevel;
}

bool
MapViewCamera::CheckRotationAngle(float& rotationAngle)
{
    if (GetZoomLevel() >= NONE_ROTATION_ZOOM_LEVEL)
    {
        return false;
    }

    rotationAngle = 0.f;
    return true;
}

void
MapViewCamera::SetZoomIsBouncingBackFlag(bool flag)
{
    if(!flag)
    {
        CheckBoundary();
    }
    m_zoomLevelBounceBack = flag;
}


void MapViewCamera::CheckBoundary()
{
    Lock lock(m_lock);
    double x, y;
    m_nbgmMapCameraHelper->GetViewCenter(x, y);
    CheckMapCenterData(x, y);
    m_nbgmMapCameraHelper->SetViewCenter(x, y);
}

void MapViewCamera::UpdateTiltRatio(float zoomLevel, float tiltAngle)
{
    if(zoomLevel == INVALID_CAMERA_ZOOMLEVEL)
    {
        zoomLevel = (float)GetZoomLevel();
    }

    m_currentTiltRatio = 0.f;
    float minTiltAngle = GetLimitedTiltAnlge(zoomLevel);
    if(tiltAngle >= 90.f)
    {
        return;
    }

    if(tiltAngle <= minTiltAngle)
    {
        m_currentTiltRatio = 1.f;
        return;
    }

    if(minTiltAngle < 90.f)
    {
        m_currentTiltRatio = 1.f - (tiltAngle - minTiltAngle)/(90.f - minTiltAngle);
    }
}


float MapViewCamera::GetTiltAngleFromTiltRatio()
{
    float tiltAngle = 90.f;
    if(m_currentTiltRatio <= 0.f)
    {
        return tiltAngle;
    }

    float minTiltAngle = GetLimitedTiltAnlge(GetZoomLevel());
    if(m_currentTiltRatio >= 1.f)
    {
        return minTiltAngle;
    }

    if(minTiltAngle < 90.f)
    {
        tiltAngle = 90.f - m_currentTiltRatio*(90.f - minTiltAngle);
    }
    return tiltAngle;
}

void MapViewCamera::SetScaleFactor(float scaleFactor)
{
    m_scaleFactor = scaleFactor;
}
