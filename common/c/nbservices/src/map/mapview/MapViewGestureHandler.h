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
 @file     MapViewCamera.h
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

#ifndef _MAP_VIEW_GESTURE_HANDLER_H_
#define _MAP_VIEW_GESTURE_HANDLER_H_

#include "nberror.h"
#include "MapViewCamera.h"
#include "MapViewUIInterface.h"

namespace nbmap
{

class MapView;

/*! Parameters for gesture handling.*/
class MapViewGestureParameters
{
public:
    MapViewGestureParameters():
    moveMultiplier(0.0),
    moveVelocityMultiplier(0.0),
    pinchMultiplier(0.0),
    pinchVelocityMultiplier(0.0),
    tiltMultiplier(0.0),
    rotationVelocityMultiplier(0.0){}

    double moveMultiplier;
    double moveVelocityMultiplier;
    double pinchMultiplier;
    double pinchVelocityMultiplier;
    double tiltMultiplier;
    double rotationVelocityMultiplier;
};


/*! Map view Gesture Handler class.

    This class is used to handle gesture event from MapView.
*/
class MapViewGestureHandler
{
public:
    MapViewGestureHandler();
    ~MapViewGestureHandler();

public:
    NB_Error Initiallize(MapView* mapView, MapViewCamera* mapViewCamera, int* viewSizeWidth, int* viewSizeHeight);

    NB_Error HandleGesture(const MapView_GestureParameters* parameters);

private:
    struct GestureInfo
    {
        GestureInfo():lastTime(0),count(0),originalValue(0.), animationTime(0), animationValue(0.){}

        uint32 lastTime;
        uint32 count;
        double  originalValue;
        uint32 animationTime;
        double  animationValue;
    };

    /*! Handle pan gesture according current MapViewCamera
        gesture parameters received from nbui and some config settings.
     */
    void HandleMoveGesture(const MapView_GestureParameters* parameters);

    /*! Handle rotation gorizontal angle(tilt angle) gesture according current MapViewCamera
        gesture parameters received from nbui and some config settings.
     */
    void HandleTiltAngleGesture(const MapView_GestureParameters* parameters);

    /*! Handle rotation gesture according current MapViewCamera
        gesture parameters received from nbui.
     */
    void HandleRotationAngleGesture(const MapView_GestureParameters* parameters);

    /*! Handle pinch(zoom) gesture according current MapViewCamera
        gesture parameters received from nbui.
     */
    void HandleScaleGesture(const MapView_GestureParameters* parameters);

    /*! Handle tap gesture gesture parameters received from nbui. */
    void HandleTapGesture(const MapView_GestureParameters* parameters);

    /*! Handle double tap gesture gesture parameters received from nbui. */
    void HandleDoubleTapGesture(const MapView_GestureParameters* parameters);

    /*! Handle two finger tap gesture gesture parameters received from nbui.  */
    void HandleTwoFingerTapGesture(const MapView_GestureParameters* parameters);

    /*! Handle long press Gesture*/
    void HandleLongPressGesture(const MapView_GestureParameters* parameters);

    /*! Handle two finger double tap gesture gesture parameters received from nbui.  */
    void HandleTwoFingerDoubleTapGesture(const MapView_GestureParameters* parameters);

    /*! Update zoom animation.  */
    static void AccelerateZoomAnimation(double scaleValue, GestureInfo& info);

private:
    /*! previous gesture information*/
    MapView_GestureParameters* m_previousGestureParam;

    /*! These get retrieved from NBUIConfig during initialization */
    MapViewGestureParameters m_gestureParameters;

    MapView* m_mapView;

    MapViewCamera* m_mapViewCamera;

    int* m_viewSizeWidth;
    int* m_viewSizeHeight;

    GestureInfo m_DoubleTapInfo;
    GestureInfo m_TwoFingersTapInfo;
    GestureInfo m_TwoFingersDoubleTapInfo;
    GestureInfo m_ScaleInfo;
};

}


#endif //_MAP_VIEW_GESTURE_HANDLER_H_

/*! @} */
