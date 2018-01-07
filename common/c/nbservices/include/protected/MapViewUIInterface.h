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
    @file     MapViewUIInterface.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef _MAP_VIEW_UI_INTERFACE_
#define _MAP_VIEW_UI_INTERFACE_

#include "nberror.h"
#include "paltypes.h"
#include "nbgpstypes.h"
#include "smartpointer.h"

namespace nbmap
{
class BubbleInterface;
}

// gesture types enum 
typedef enum MapView_TransformationType
{
    MapViewTransformation_None = 0,
    
    MapViewTransformation_Move,
    MapViewTransformation_TiltAngle,
    MapViewTransformation_RotateAngle,
    MapViewTransformation_Scale,
    MapViewTransformation_Tap,
    MapViewTransformation_DoubleTap,
    MapViewTransformation_TwoFingerTap,
    MapViewTransformation_LongPress,
    MapViewTransformation_TwoFingerDoubleTap
} MapView_TransformationType;

class MapView_Point2D
{
public:
    MapView_Point2D():x(0), y(0){};
    
    float x;
    float y;
    
};

//common structure to describe gesture recognizer state
typedef enum MapView_GestureRecognizerState
{
    /* :TRICKY:
       The current nbui code for iPhone assumes that this matches the iPhone gesture state exactly.
       The nbui code should be changed and NOT make such assumptions!
    */
    MapView_GestureStatePossible,
    MapView_GestureStateBegan,
    MapView_GestureStateChanged,
    MapView_GestureStateEnded,
    MapView_GestureStateCancelled,
    MapView_GestureStateFailed,
    MapView_GestureStateRecognized
} MapView_GestureRecognizerState;

/*! gesture parameters translated from nbui */
class MapView_GestureParameters
{
public:
    
    MapView_GestureParameters()
        : gestureType(MapViewTransformation_None),
          state(MapView_GestureStatePossible),
          scale(0),
          rotaionAngle(0),
          tiltAngle(0),
          scaleVelocity(0),
          rotationVelocity(0){}
    
    MapView_TransformationType        gestureType;       // describe gesture type
    MapView_GestureRecognizerState    state;             // describe current gesture state
    float                             scale;
    float                             rotaionAngle;
    float                             tiltAngle;
    MapView_Point2D                   locationInView;
    MapView_Point2D                   velocityInView;
    float                             scaleVelocity;
    float                             rotationVelocity;
    MapView_Point2D                   viewSize;
    
};

typedef enum MapViewUI_CameraAnimationAccelertaionType
{
    MVUICAAT_LINEAR,
    MVUICAAT_DECELERATION
}MapViewUI_CameraAnimationAccelertaionType;

/*! MapViewUIInterface.

    This is the interface of the map view exposed to nbui.
*/
class MapViewUIInterface 
{
public:
    MapViewUIInterface(){};
    virtual ~MapViewUIInterface(){};

    /*! Render new frame.

        @return NB_Error
    */
    virtual NB_Error UI_Render(uint32 interval) = 0;
    
    /*! update current avatar/map position.

        @return NB_Error
    */
    virtual NB_Error UI_SetCurrentPosition(double lat, double lon) = 0;
    
    /*! set view port size for nbgm manager. View port sizes are used to configure camera frustum

        @return NB_Error
    */
    virtual NB_Error UI_SetViewPort(int width, int height) = 0;

    /*! set NBGM view size.

        @return NB_Error
    */
    virtual NB_Error UI_SetViewSize(int width, int height) = 0;

    /*! set screen orientation for nbgm.

        @return NB_Error
     */
    virtual NB_Error UI_SetScreenOrientation(bool bPortrait) = 0;

    /*! Handle of basic gestures. This function is common for all types of gestures.

        @return NB_Error
     */

    virtual NB_Error UI_HandleViewGesture(const MapView_GestureParameters* parameters) = 0;
    
    /*! set camera position and heading.

        @return none
     */
    virtual void UI_SetCamera(double lat, double lon, double zoomLevel, double heading, double tilt, bool animated = false,
                              uint32 duration = 0, MapViewUI_CameraAnimationAccelertaionType type = MVUICAAT_LINEAR) = 0;

    /*! get camera rotation angle .

      @return camera rotation angle
      */
    virtual float UI_GetCameraRotationAngle() = 0;

    /*! Initialize NBGM. Should be called in case MapNativeView initialization

        was failed (with NE_AGAIN error code) at first attempt.

        This method should be called when MapNativeView is ready for rendering

        (i.e. MapNativeView can be initialized)

     @return NB_Error
     */
    virtual NB_Error UI_InitializeMapView() = 0;
    virtual bool IsFirstTilesLoaded(void) = 0;

    /*! destroy the rendering surface.

        @return none
     */
    virtual void UI_DestroySurface() = 0;

    /*! update the rendering surface.

        @return none
     */
    virtual void UI_UpdateSurface() = 0;
    
    /*! Sets the GPS mode.

        @return none
     */
    virtual void UI_SetGpsMode(NB_GpsMode mode) = 0;

    /*! notify that map native view is created.

        @return none
     */
    virtual void UI_MapNativeViewCreated() = 0;

    /*! Sets the opcaity value of the Doppler layer.
        @param opacity The opacity value, range [0, 100].
        @return none
     */
    virtual void UI_SetDopplerOpacity(uint32 opacity) = 0;

    /*! Sets the play state of the Doppler layer.
        @param play True to paly Doppler animation, false otherwise.
        @return none
     */
    virtual void UI_SetDopplerPlayState(bool play) = 0;

    /*! Get bubble indicated by the pinID
        @param pinID A string indicating the pinID
        @return Pointer to the bubble Object that the pinID belongs to. Null
                if the pinID doesnt exist.
    */
    virtual shared_ptr<nbmap::BubbleInterface> UI_GetBubble(shared_ptr<std::string> pinID) = 0;

    /*! Get Compass Bounding Box
        @param leftBottomX, compass boudingbox left-bottom x-point in pixel.
        @param leftBottomY, compass boudingbox left-bottom y-point in pixel.
        @param rightTopX, compass boudingbox right-top x-point in pixel.
        @param rightTopY, compass boudingbox right-top y-point in pixel.
    */
    virtual void UI_GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY) const = 0;
};

#endif

/*! @} */ 
