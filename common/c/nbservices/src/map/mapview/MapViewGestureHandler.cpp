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
    @file     MapViewGestureHandler.cpp
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

#include "MapViewGestureHandler.h"
#include "palstdlib.h"
#include "palclock.h"
#include "palmath.h"
#include "NBUIConfig.h"
#include "MapView.h"

static const uint32 MILLISECOND_PER_SECOND = 1000; // The interval to check night mode is one hour
static const uint32 REBOUND_ANIMATION_TIME = 250;
static const float VELOCITY_EPSILON = 0.01f;
static const double SCALE_EPSILON = 0.001;
static const uint32 GESTURE_INTERVAL = 1100;
static const uint32 MIN_ANIMATION_TIME = 250;
static const uint32 MAX_ANIMATION_TIME = 1000;
static const double MIN_SCALE_VALUE = 0.2;
static const double MAX_SCALE_VALUE = 5.0;
//static const float ANIMATION_TIME_DECREASE = 0.75f;
static const uint32 MAX_ACCELERATION_COUNTS = 4;
using namespace nbmap;



MapViewGestureHandler::MapViewGestureHandler()
    :m_previousGestureParam(NULL)
    ,m_mapView(NULL)
    ,m_mapViewCamera(NULL)
{
}

MapViewGestureHandler::~MapViewGestureHandler()
{
    if(m_previousGestureParam)
    {
        delete m_previousGestureParam;
    }
}

NB_Error
MapViewGestureHandler::Initiallize(MapView* mapView, MapViewCamera* mapViewCamera, int* viewSizeWidth, int* viewSizeHeight)
{
    m_mapView = mapView;
    m_mapViewCamera = mapViewCamera;
    m_viewSizeWidth = viewSizeWidth;
    m_viewSizeHeight = viewSizeHeight;
    m_previousGestureParam = new MapView_GestureParameters();
    NBUIConfig::getGestureParameters(m_gestureParameters.moveMultiplier,
                                    m_gestureParameters.moveVelocityMultiplier,
                                    m_gestureParameters.pinchMultiplier,
                                    m_gestureParameters.pinchVelocityMultiplier,
                                    m_gestureParameters.tiltMultiplier,
                                    m_gestureParameters.rotationVelocityMultiplier);
    return NE_OK;
}

NB_Error
MapViewGestureHandler::HandleGesture(const MapView_GestureParameters* parameters)
{
    if(parameters->state == MapView_GestureStateBegan)
    {
        m_mapView->UpdateDopplerByGesture(true);
    }
    else if(parameters->state == MapView_GestureStateEnded)
    {
        m_mapView->UpdateDopplerByGesture(false);
    }

    bool isTiltAnimating = m_mapViewCamera->GetTiltIsBouncingBackFlag();
    if(isTiltAnimating)
    {
        if(parameters->gestureType != MapViewTransformation_TiltAngle)
        {
            float minAngle = m_mapViewCamera->GetMinTiltAngle();
            m_mapViewCamera->SetTiltAngle(minAngle, true, false, true, 0);
            m_mapViewCamera->SetTiltIsBouncingBackFlag(false);
        }
    }

    if(m_mapViewCamera->GetZoomIsBouncingBackFlag())
    {
        if (m_mapViewCamera->GetZoomLevel() < m_mapViewCamera->GetMinZoomLevel())
        {
            m_mapViewCamera->SetZoom(m_mapViewCamera->GetMinZoomLevel(), false, false);
        }
        else if (m_mapViewCamera->GetZoomLevel() > m_mapViewCamera->GetMaxZoomLevel())
        {
            m_mapViewCamera->SetZoom(m_mapViewCamera->GetMaxZoomLevel(), false, false);
        }
        m_mapViewCamera->SetZoomIsBouncingBackFlag(false);
    }

    //handle scale gesture even do rotation bounce back.
    if(m_mapViewCamera->GetRotateIsBouncingBackFlag())
    {
        m_mapViewCamera->SetRotationAngle(0.f, false, false);
        m_mapViewCamera->SetRotateIsBouncingBackFlag(false);
    }

    switch (parameters->gestureType)
    {
        case MapViewTransformation_Move:
        {
            HandleMoveGesture(parameters);
            break;
        }
        case MapViewTransformation_TiltAngle:
        {
            HandleTiltAngleGesture(parameters);
            break;
        }
        case MapViewTransformation_RotateAngle:
        {
            HandleRotationAngleGesture(parameters);
            break;
        }
        case MapViewTransformation_Scale:
        {
            HandleScaleGesture(parameters);
            break;
        }
        case MapViewTransformation_Tap:
        {
            HandleTapGesture(parameters);
            break;
        }
        case MapViewTransformation_DoubleTap:
        {
            HandleDoubleTapGesture(parameters);
            break;
        }
        case MapViewTransformation_TwoFingerTap:
        {
            HandleTwoFingerTapGesture(parameters);
            break;
        }
        case MapViewTransformation_LongPress:
        {
            HandleLongPressGesture(parameters);
            break;
        }
        case MapViewTransformation_TwoFingerDoubleTap:
        {
            HandleTwoFingerDoubleTapGesture(parameters);
            break;
        }
        default: // Added just to remove compiler's warning.
        {
            break;
        }
    }

    m_mapView->SetGestureProcessing((parameters->state == MapView_GestureStateEnded)?false:true);

    return NE_OK;
}

void
MapViewGestureHandler::HandleMoveGesture(const MapView_GestureParameters *parameters)
{
    switch (parameters->state)
    {
        case MapView_GestureStatePossible:
        case MapView_GestureStateFailed:
        case MapView_GestureStateRecognized:
        {
            break;
        }

        case MapView_GestureStateBegan:
        {
            m_previousGestureParam->locationInView = parameters->locationInView;
            m_mapViewCamera->BeginMove(parameters->locationInView.x, parameters->locationInView.y);
            break;
        }

        case MapView_GestureStateChanged:
        {
            m_mapViewCamera->MoveTo(parameters->locationInView.x, parameters->locationInView.y, false, true, 0);
            m_previousGestureParam->locationInView = parameters->locationInView;
            break;
        }

        case MapView_GestureStateEnded:
        case MapView_GestureStateCancelled:
        {
            if(nsl_fabs(parameters->velocityInView.x) > VELOCITY_EPSILON || nsl_fabs(parameters->velocityInView.y) > VELOCITY_EPSILON)
            {
                m_mapViewCamera->BeginMove(parameters->locationInView.x, parameters->locationInView.y);
                static const float DECELERATION = 6000.0f;
                float speed = (float)nsl_sqrt(nsl_pow(parameters->velocityInView.x, 2) + nsl_pow(parameters->velocityInView.y, 2));
                float t = speed/DECELERATION;
                float deltaDistanceX = parameters->velocityInView.x * t / 2;
                float deltaDistanceY = parameters->velocityInView.y * t / 2;
                double x = parameters->locationInView.x  + deltaDistanceX;
                double y = parameters->locationInView.y  + deltaDistanceY;
                m_mapViewCamera->MoveTo(x, y, true, true, static_cast<uint32>(t*MILLISECOND_PER_SECOND));
            }
            break;
        }
    }
}

void
MapViewGestureHandler::HandleTiltAngleGesture(const MapView_GestureParameters* parameters)
{
    switch (parameters->state)
    {
        case MapView_GestureStatePossible:
        case MapView_GestureStateFailed:
        case MapView_GestureStateRecognized:
        {
            break;
        }

        case MapView_GestureStateBegan:
        {
            float tilt = m_mapViewCamera->GetTiltAngle();
            m_previousGestureParam->tiltAngle = parameters->tiltAngle;

            // Stop all other animation when we start the gesture
            m_mapViewCamera->SetTiltAngle(tilt, true, false, true, 0);
            break;
        }

        case MapView_GestureStateChanged:
        {
            float angle = m_mapViewCamera->GetTiltAngle();
            angle += (parameters->tiltAngle - m_previousGestureParam->tiltAngle) * static_cast<float>(m_gestureParameters.tiltMultiplier);
            m_mapViewCamera->SetTiltAngle(angle, false, true, true, 0);

            m_previousGestureParam->tiltAngle = parameters->tiltAngle;
            break;
        }

        case MapView_GestureStateEnded:
        case MapView_GestureStateCancelled:
        {
            float angle = m_mapViewCamera->GetTiltAngle();
            float minAngle = m_mapViewCamera->GetMinTiltAngle();

            // Double check that the angle didn't go below the minimum (which is possible if snapback is active)
            if (angle < minAngle)
            {
                // Animate back to the minimum angle
                m_mapViewCamera->SetTiltAngle(minAngle, false, true, true, REBOUND_ANIMATION_TIME);
                m_mapViewCamera->SetTiltIsBouncingBackFlag(true);
            }
            break;
        }
    }
}

void
MapViewGestureHandler::HandleRotationAngleGesture(const MapView_GestureParameters* parameters)
{
    double x = parameters->locationInView.x*(float)m_gestureParameters.moveMultiplier;
    double y = parameters->locationInView.y*(float)m_gestureParameters.moveMultiplier;

    switch (parameters->state)
    {
        case MapView_GestureStatePossible:
        case MapView_GestureStateFailed:
        case MapView_GestureStateRecognized:
        {
            break;
        }

        case MapView_GestureStateBegan:
        {
            m_previousGestureParam->rotaionAngle = parameters->rotaionAngle;
            m_mapViewCamera->SetIsInRotationGesture(true);
            break;
        }

        case MapView_GestureStateChanged:
        {
            /* also here it is needed to change frame size for loading new tiles */
            m_mapViewCamera->Turn(x, y, parameters->rotaionAngle - m_previousGestureParam->rotaionAngle, false, true, 0);
            m_previousGestureParam->rotaionAngle = parameters->rotaionAngle;
            break;
        }

        case MapView_GestureStateEnded:
        case MapView_GestureStateCancelled:
        {
            m_mapViewCamera->SetIsInRotationGesture(false);
            {
                float angle = m_mapViewCamera->GetRotateAngle();
                float angleLimit = 0;
                if (m_mapViewCamera->CheckRotationAngle(angleLimit)
                    && nsl_fabs(angle) >= angleLimit)// when angle & limit are both 0,  don't go "else" branch
                {
                    // exceed angle limit, perform bounce back behavior
                    float turnAngle = angle > 180.0f ? 360.0f - angle : -angle; // shortest turn
                    m_mapViewCamera->Turn(x, y, turnAngle, true, true, REBOUND_ANIMATION_TIME);
                    m_mapViewCamera->SetRotateIsBouncingBackFlag(true);
                }
                else
                {
                    static const float DECELERATION = 400.0f;
                    if(nsl_fabs(parameters->rotationVelocity) > VELOCITY_EPSILON)
                    {
                        float t = (float) nsl_fabs(parameters->rotationVelocity/DECELERATION);
                        float deltRotate = parameters->rotationVelocity*t*t*t/3;
                        m_mapViewCamera->Turn(x, y, deltRotate, true, true, static_cast<uint32>(t*MILLISECOND_PER_SECOND));
                    }
                }
            }

            /*
                :KLUDGE:
                If we perform both a tilt and rotating gesture at the same time on the iPhone then we don't get
                the end-tilt-gesture notification, but instead two rotation-end-gesture notifications. In order to
                handle the snapback for tilt correctly, we animate the tilt back to its minimum value. We should
                fix the UI handling, if possible.

                This code does nothing in most cases.
             */

            float angle = m_mapViewCamera->GetTiltAngle();
            float minAngle = m_mapViewCamera->GetMinTiltAngle();

            // Double check that the angle didn't go below the minimum (which is possible if snapback is active)
            if (angle < minAngle)
            {
                // Animate back to the minimum angle
                m_mapViewCamera->SetTiltAngle(minAngle, false, true, true, REBOUND_ANIMATION_TIME);
            }
            break;
        }
    }
}

void
MapViewGestureHandler::HandleScaleGesture(const MapView_GestureParameters* parameters)
{
    double x = parameters->locationInView.x * m_gestureParameters.moveMultiplier;
    double y = parameters->locationInView.y * m_gestureParameters.moveMultiplier;

    switch (parameters->state)
    {
        case MapView_GestureStatePossible:
        case MapView_GestureStateFailed:
        case MapView_GestureStateRecognized:
        {
            break;
        }

        case MapView_GestureStateBegan:
        {
            m_previousGestureParam->scale = parameters->scale;
            break;
        }
        case MapView_GestureStateChanged:
        {
            double scale = m_previousGestureParam->scale / parameters->scale * m_gestureParameters.pinchMultiplier;

            if (x > 0 && x < *m_viewSizeWidth && y > 0 && y < *m_viewSizeHeight)
            {
                m_mapViewCamera->ZoomAroundGestureCenter(scale, float(x), float(y), false, true, true);
            }
            else
            {
                m_mapViewCamera->Zoom(scale, false, true, true);
            }
            m_previousGestureParam->scale = parameters->scale;

            break;
        }
        case MapView_GestureStateEnded:
        case MapView_GestureStateCancelled:
        {
            /*
             For zoom bounce back implementation.
             */
            if (m_mapViewCamera->GetZoomLevel() > m_mapViewCamera->GetMaxZoomLevel())
            {
                double scale = pow(2.0, (m_mapViewCamera->GetZoomLevel() - m_mapViewCamera->GetMaxZoomLevel()));
                m_mapViewCamera->Zoom(scale, true, false, false, REBOUND_ANIMATION_TIME);
                m_mapViewCamera->SetZoomIsBouncingBackFlag(true);
                break;
            }

            if (m_mapViewCamera->GetZoomLevel() < m_mapViewCamera->GetMinZoomLevel())
            {
                double scale = pow(2.0, (m_mapViewCamera->GetZoomLevel() - m_mapViewCamera->GetMinZoomLevel()));
                m_mapViewCamera->Zoom(scale, true, false, false, REBOUND_ANIMATION_TIME);
                m_mapViewCamera->SetZoomIsBouncingBackFlag(true);
                break;
            }

            m_mapViewCamera->CheckBoundary();
            if(nsl_fabs(parameters->scaleVelocity) < VELOCITY_EPSILON)
            {
                // Ignore scale animation.
                break;
            }
            /*
                Limit zoom scale between 0.5 and 2.0 which means double/half the zoom in/out. We could put these in the configuration in NBUI
             */
            const double SCALE_MAX = 2.0;
            const double SCALE_MIN = 0.5;
            double scale = 1.0;

            // divisor used to calculate new zoom scale
            double divisor = parameters->scale + (parameters->scaleVelocity * m_gestureParameters.pinchVelocityMultiplier);

            // Resulting scale will be below 1.0
            if (parameters->scaleVelocity >= 0)
            {
                scale = SCALE_MIN;

                // Check if scale is in range
                if (((divisor * SCALE_MIN) < m_previousGestureParam->scale) && (divisor != 0))
                {
                    scale = m_previousGestureParam->scale / divisor;
                }
            }
            // Resulting scale will be above 1.0
            else if(parameters->scaleVelocity < 0)
            {
                scale = SCALE_MAX;

                // Check if the scale is in range
                if ((divisor * SCALE_MAX) > m_previousGestureParam->scale)
                {
                    scale = m_previousGestureParam->scale / divisor;
                }
            }
            AccelerateZoomAnimation(scale, m_ScaleInfo);
            if (x > 0 && x < *m_viewSizeWidth && y > 0 && y < *m_viewSizeHeight)
            {
                m_mapViewCamera->ZoomAroundGestureCenter(m_ScaleInfo.animationValue, float(x), float(y), true, true, false, m_ScaleInfo.animationTime);
            }
            else
            {
                m_mapViewCamera->Zoom(m_ScaleInfo.animationValue, true, true, false, m_ScaleInfo.animationTime);
            }

            break;
        }
    }
}

void
MapViewGestureHandler::HandleTapGesture(const MapView_GestureParameters* parameters)
{
    float screenX = parameters->locationInView.x * (float)m_gestureParameters.moveMultiplier;
    float screenY = parameters->locationInView.y * (float)m_gestureParameters.moveMultiplier;
    m_mapView->SelectAvatarAndPin(screenX, screenY);
}

void
MapViewGestureHandler::HandleDoubleTapGesture(const MapView_GestureParameters* parameters)
{
    double previousZoomLevel = m_mapViewCamera->GetZoomLevel();
    double newZoomLevel = previousZoomLevel + 1;
    double previousHeight = MapViewCamera::CalculateDistanceFromZoomLevel(previousZoomLevel);
    double newHeight = MapViewCamera::CalculateDistanceFromZoomLevel(newZoomLevel);
    double zoomIndex = newHeight/previousHeight;
    AccelerateZoomAnimation(zoomIndex, m_DoubleTapInfo);
    m_mapViewCamera->DoubleTapZoom(m_DoubleTapInfo.animationValue, float(parameters->locationInView.x * m_gestureParameters.moveMultiplier),
        float(parameters->locationInView.y * m_gestureParameters.moveMultiplier), true, true, m_DoubleTapInfo.animationTime);
}

void
MapViewGestureHandler::HandleTwoFingerTapGesture(const MapView_GestureParameters* parameters)
{
    double previousZoomLevel = m_mapViewCamera->GetZoomLevel();
    double newZoomLevel = previousZoomLevel - 1;
    double previousHeight = MapViewCamera::CalculateDistanceFromZoomLevel(previousZoomLevel);
    double newHeight = MapViewCamera::CalculateDistanceFromZoomLevel(newZoomLevel);
    double zoomIndex = newHeight/previousHeight;
    AccelerateZoomAnimation(zoomIndex, m_TwoFingersTapInfo);
    m_mapViewCamera->DoubleTapZoom(m_TwoFingersTapInfo.animationValue, float(parameters->locationInView.x * m_gestureParameters.moveMultiplier),
        float(parameters->locationInView.y * m_gestureParameters.moveMultiplier), true, true, m_TwoFingersTapInfo.animationTime);
}

void
MapViewGestureHandler::HandleLongPressGesture(const MapView_GestureParameters* parameters)
{
    double mercatorX;
    double mercatorY;
    m_mapViewCamera->ScreenToMapPosition(parameters->locationInView.x * (float)m_gestureParameters.moveMultiplier,
                                              parameters->locationInView.y * (float)m_gestureParameters.moveMultiplier,
                                              mercatorX,mercatorY);
    double latitude;
    double longitude;

    NB_SpatialConvertMercatorToLatLong(mercatorX,mercatorY, &latitude, &longitude);

    m_mapView->OnLongPressed(latitude, longitude);
}

void
MapViewGestureHandler::HandleTwoFingerDoubleTapGesture(const MapView_GestureParameters* parameters)
{
    double previousZoomLevel = m_mapViewCamera->GetZoomLevel();
    double newZoomLevel = previousZoomLevel - 2;
    double previousHeight = MapViewCamera::CalculateDistanceFromZoomLevel(previousZoomLevel);
    double newHeight = MapViewCamera::CalculateDistanceFromZoomLevel(newZoomLevel);
    double zoomIndex = newHeight/previousHeight;
    AccelerateZoomAnimation(zoomIndex, m_TwoFingersDoubleTapInfo);
    m_mapViewCamera->DoubleTapZoom(m_TwoFingersDoubleTapInfo.animationValue, float(parameters->locationInView.x * m_gestureParameters.moveMultiplier),
        float(parameters->locationInView.y * m_gestureParameters.moveMultiplier), true, true, m_TwoFingersDoubleTapInfo.animationTime);
}

void
MapViewGestureHandler::AccelerateZoomAnimation(double scaleValue, GestureInfo& gestrueInfo)
{
    uint32 currentTime = PAL_ClockGetTimeMs();

    if((gestrueInfo.lastTime != 0) && (currentTime >= gestrueInfo.lastTime) && (currentTime - gestrueInfo.lastTime) < GESTURE_INTERVAL)
    {
        if(gestrueInfo.count < MAX_ACCELERATION_COUNTS)
        {
            ++gestrueInfo.count;
        }
    }
    else
    {
        gestrueInfo.count = 0;
    }

    if(gestrueInfo.originalValue != 0.0)
    {
        if((gestrueInfo.originalValue > 1.0 && scaleValue < 1.0) || (gestrueInfo.originalValue < 1.0 && scaleValue > 1.0))
        {
            // Reset the count if zoom direction changes.
            gestrueInfo.count = 0;
        }
    }

    gestrueInfo.originalValue = scaleValue;
    gestrueInfo.lastTime = currentTime;
    gestrueInfo.animationTime = DEFAULT_ANIMATION_DURATION;
    gestrueInfo.animationValue  = scaleValue;
    if(gestrueInfo.count == 0 || nsl_fabs(gestrueInfo.animationValue - 1.0) <= SCALE_EPSILON)
    {
        return;
    }

    double factor = gestrueInfo.count/(double)(MAX_ACCELERATION_COUNTS);
    gestrueInfo.animationTime = (uint32)(DEFAULT_ANIMATION_DURATION*(1-factor));
    if(gestrueInfo.animationValue > 1.0)
    {
        gestrueInfo.animationValue += (MAX_SCALE_VALUE - 1.0)*factor;
    }
    else
    {
        gestrueInfo.animationValue -= (1.0 - MIN_SCALE_VALUE)*factor;
    }

    gestrueInfo.animationTime  = (gestrueInfo.animationTime < MIN_ANIMATION_TIME)?MIN_ANIMATION_TIME:gestrueInfo.animationTime;
    gestrueInfo.animationTime  = (gestrueInfo.animationTime > MAX_ANIMATION_TIME)?MAX_ANIMATION_TIME:gestrueInfo.animationTime;
    gestrueInfo.animationValue = (gestrueInfo.animationValue > MAX_SCALE_VALUE)?MAX_SCALE_VALUE:gestrueInfo.animationValue;
    gestrueInfo.animationValue = (gestrueInfo.animationValue < MIN_SCALE_VALUE)?MIN_SCALE_VALUE:gestrueInfo.animationValue;
}
