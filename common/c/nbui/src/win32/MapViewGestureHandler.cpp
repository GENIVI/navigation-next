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

#include "MapViewGestureHandler.h"

class MapViewPanGestureListener: public IPanGestureListener
{
public:
    MapViewPanGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewPanGestureListener() {}
    virtual bool OnPan(int state, float locationX, float locationY, float velocityX, float velocityY);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewPanGestureListener::OnPan(int state, float locationX, float locationY, float velocityX, float velocityY)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = MapViewTransformation_Move;
    gestureParam.state = static_cast<MapView_GestureRecognizerState>(state);
    gestureParam.locationInView.x = locationX;
    gestureParam.locationInView.y = locationY;
    gestureParam.velocityInView.x = velocityX;
    gestureParam.velocityInView.y = velocityY;
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

class MapViewScaleGestureListener: public IScaleGestureListener
{
public:
    MapViewScaleGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewScaleGestureListener() {}
    virtual bool OnScale(int state,  float scale, float velocity);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewScaleGestureListener::OnScale(int state,  float scale, float velocity)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = MapViewTransformation_Scale;
    gestureParam.state = static_cast<MapView_GestureRecognizerState>(state);
    gestureParam.scale = scale;
    gestureParam.scaleVelocity = velocity;
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

class MapViewRotateGestureListener: public IRotateGestureListener
{
public:
    MapViewRotateGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewRotateGestureListener() {}
    virtual bool OnRotate(int state, float rotateAngle, float rotateVelocity);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewRotateGestureListener::OnRotate(int state, float rotateAngle, float rotateVelocity)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = MapViewTransformation_RotateAngle;
    gestureParam.state = static_cast<MapView_GestureRecognizerState>(state);
    gestureParam.rotaionAngle = rotateAngle;
    gestureParam.rotationVelocity = rotateVelocity;
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

class MapViewTiltGestureListener: public ITiltGestureListener
{
public:
    MapViewTiltGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewTiltGestureListener() {}
    virtual bool OnTilt(int state, float tiltAngle);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewTiltGestureListener::OnTilt(int state, float tiltAngle)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = MapViewTransformation_TiltAngle;
    gestureParam.state = static_cast<MapView_GestureRecognizerState>(state);
    gestureParam.tiltAngle = tiltAngle;
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

class MapViewDoubleTapGestureListener: public IDoublTapGestureListener
{
public:
    MapViewDoubleTapGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewDoubleTapGestureListener() {}
    virtual bool OnDoubleTap(bool zoomIn, int locationInViewX, int locationInViewY, int viewSizeX, int viewSizeY);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewDoubleTapGestureListener::OnDoubleTap(bool zoomIn, int locationInViewX, int locationInViewY, int viewSizeX, int viewSizeY)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = zoomIn?MapViewTransformation_DoubleTap:MapViewTransformation_TwoFingerTap;
    gestureParam.locationInView.x = static_cast<float>(locationInViewX);
    gestureParam.locationInView.y = static_cast<float>(locationInViewY);
    gestureParam.viewSize.x = static_cast<float>(viewSizeX);
    gestureParam.viewSize.y = static_cast<float>(viewSizeY);
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

class MapViewLongPressGestureListener : public ILongPressGestureListener
{
public:
    MapViewLongPressGestureListener(MapViewUIInterface *mapView):m_pMapView(mapView) { nsl_assert(mapView); }
    virtual ~MapViewLongPressGestureListener() {}
    virtual bool OnLongPress(int locationInViewX, int locationInViewY);

private:
    MapViewUIInterface *m_pMapView;
};

bool
MapViewLongPressGestureListener::OnLongPress(int locationInViewX, int locationInViewY)
{
    MapView_GestureParameters gestureParam;
    gestureParam.gestureType = MapViewTransformation_LongPress;
    gestureParam.locationInView.x = static_cast<float>(locationInViewX);
    gestureParam.locationInView.y = static_cast<float>(locationInViewY);
    m_pMapView->UI_HandleViewGesture(&gestureParam);
    return true;
}

void
MapViewGestureHandler::StartDetector()
{
    m_tiltDetector.SetListener(new MapViewTiltGestureListener(m_pMapView));
    m_rotateDetector.SetListener(new MapViewRotateGestureListener(m_pMapView));
    m_panDetector.SetListener(new MapViewPanGestureListener(m_pMapView));
    m_scaleDetector.SetListener(new MapViewScaleGestureListener(m_pMapView));
    m_doubleTapDetector.SetListener(new MapViewDoubleTapGestureListener(m_pMapView));
    m_longPressDetector.SetListener(new MapViewLongPressGestureListener(m_pMapView));
    m_isActived = true;
}

void
MapViewGestureHandler::StopDetector()
{
    m_isActived = false;
    m_tiltDetector.SetListener(NULL);
    m_rotateDetector.SetListener(NULL);
    m_panDetector.SetListener(NULL);
    m_scaleDetector.SetListener(NULL);
    m_doubleTapDetector.SetListener(NULL);
    m_longPressDetector.SetListener(NULL);
}

bool
MapViewGestureHandler::OnTouchEvent(const GestureEvent &event)
{
    if(event.message == WM_CLOSE)
    {
        StopDetector();
    }

    if(!m_isActived)
    {
        return false;
    }

    bool handled = false;
    handled = m_doubleTapDetector.OnTouchEvent(event) || handled;
    handled = m_tiltDetector.OnTouchEvent(event) || handled;
    handled = m_rotateDetector.OnTouchEvent(event) || handled;
    handled = m_panDetector.OnTouchEvent(event) || handled;
    handled = m_scaleDetector.OnTouchEvent(event) || handled;
    handled = m_longPressDetector.OnTouchEvent(event) || handled;
    return handled;
}