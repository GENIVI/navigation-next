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

#ifndef _MAP_VIEW_CAMERA_H_
#define _MAP_VIEW_CAMERA_H_
#include "nberror.h"
#include <string>
#include <list>
#include <set>
#include <vector>
#include "paltypes.h"
#include "TileKey.h"
#include "Tile.h"
#include "smartpointer.h"
#include "BubbleListenerInterface.h"
#include "MapViewProfiling.h"
#include "nbgm.h"
#include "nbgmtypes.h"
#include "MapViewAnimation.h"
#include "MapViewAnimationLayer.h"  // For ANIMATION_DATA
#include "ExternalMaterial.h"
#include "MapViewInterface.h"
#include "pallock.h"


#define DEFAULT_ANIMATION_DURATION 1000


namespace nbmap
{

class MapViewCameraListener;

/*! Described camera of map view, including animation */
class MapViewCamera
{
public:
    MapViewCamera(MapViewCameraListener * listener);
    ~MapViewCamera();

public:
    enum AnimationAccelerationType
    {
        AAT_LINEAR,
        AAT_DECELERATION,
        AAT_ACCELERATION,
        AAT_ACCELERATEDECELERATE,
    };

    static int8 GetRelativeZoomLevel();
    static float GetRelativeCameraHeight();
    static float GetMetersPerPixelOnRelativeZoomLevel();

    static double CalculateZoomLevelFromDistance(double distance);
    static double CalculateDistanceFromZoomLevel(double zoomLevel);

    NB_Error Initiallize(PAL_Instance* palInstance);

    bool ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY);
    bool MapPositionToWindow(double mercatorX, double mercatorY, float* dX, float* dY);

    void SetFrameBufferSize(int width, int height);

    void SetCurrentPositon(double lat, double lon, bool animated, bool stopExistingAnimation);
    /*! Get current avatar position

        @return current avatar position
     */
    void GetCurrentPosition(double &currX, double &currY);

    /*! Calculate zoom level based on value of camera height

        @return zoom level
     */
    double GetZoomLevel();

    /*! Get current horizontal angle(tilt angle)

        @return current tilt angle;
     */
    float GetTiltAngle();

    float GetRotateAngle();

    /*! Get the frustum for the current screen from NBGM.

        @return 'true' on success, 'false' otherwise.
     */
    bool GetFrustum(vector<NBGM_Point2d64>& frustum,            /*!< On return the 4 points in mercator coordinates describing the frustum */
                    vector<NBGM_Point2d64>& frustumRasterNear   /*!< Used for raster calculation */
                    );

    /*! Set tilt angle. */
    void SetTiltAngle(float& tiltAngle,
                      bool stopAllAnimation,    /*!< Set to true to stop all animation */
                      bool validate,            /*!< Validate the given tilt value */
                      bool gesture,             /*!< Called during a tilt gesture, used when snap-back is activated */
                      uint32 animationTime);    /*!< If no animation, set 0 to animationTime. Unit is ms.*/

    /*! Rotate nbgm scene on given angle in vertical plane

        @return none
     */
    void Turn(double x, double y, float rotationAngle, bool bAnimated, bool stopExistingAnimation, uint32 duration);


    /*! Set camera orientation.

     @return none
     */
    void SetCamera(double lat, double lon, double heading, double zoomLevel, double tilt, uint32 timestamp, bool animated, uint32 duration = DEFAULT_ANIMATION_DURATION, AnimationAccelerationType animationAccelerationType = AAT_LINEAR, int cameraAnimationId = 0);

    /*! Get the state of camera */
    void GetCamera(double& lat, double& lon, float& zoomLevel, float& heading, float& tilt);

    void GetRenderCamera(double& mx, double& my, double& altitude, float& heading, float& tilt);

    /*! Stop the animaitons */
    void StopAnimations(int animationType);

    /*! Update current zoom level

        @return zoom level
     */
    //double UpdateCurrentZoomLevel();

    void SetTiltIsBouncingBackFlag(bool flag) { m_tiltIsBounceingBack = flag; }
    bool GetTiltIsBouncingBackFlag() ;

    void SetRotateIsBouncingBackFlag(bool flag) { m_rotateIsBounceingBack = flag; }
    bool GetRotateIsBouncingBackFlag() ;

    void SetZoomIsBouncingBackFlag(bool flag);
    bool GetZoomIsBouncingBackFlag() ;

    /*! If rotating, zoom level change will not reset angle to zero*/
    void SetIsInRotationGesture(bool isRotating) { m_isInRotationGesture = isRotating; }

    /*! Move nbgm scene.

        @return none
     */
    void BeginMove(double dX, double dY, bool stopExistingAnimation = true);

    /*! Move nbgm scene.

        @return none
     */
    void MoveTo(double dX, double dY, bool bAnimated, bool stopExistingAnimation, uint32 duration);

    /*! Zoom using an absolute zoom level

        @return none
     */
    void SetZoom(double zoom,                   /*!< Absolute zoom level */
                 bool animated,
                 bool stopExistingAnimation);
    /*! Zoom using a relative scale.

        @return none
     */
    void Zoom(double zoomIndex,                 /*!< This is the the zoom level, but a relative scale for zooming */
              bool bAnimated,
              bool stopExistingAnimation,
              bool gesture = false,
              uint32 duration = DEFAULT_ANIMATION_DURATION);

    /* Double tap zoom
      @return none
      */
    void DoubleTapZoom(double zoomIndex, float locationInViewX, float locationInViewY, bool bAnimated, bool stopExistingAnimation, uint32 duration);

    /*! Zoom around fingers center.

        @return none
     */
    void ZoomAroundGestureCenter(double zoomIndex,                 /*!< This is the zoom level, but a relative scale for zooming */
                                 float locationInViewX,
                                 float locationInViewY,
                                 bool bAnimated,
                                 bool stopExistingAnimation,
                                 bool gesutre = false,
                                 uint32 duration = DEFAULT_ANIMATION_DURATION);

    void CombinedAnimatedZoomAndPanTo(double zoomIndex, double x, double y, bool bAnimated, bool stopExistingAnimation, uint32 duration);

    /*! Check animation status

        @return 'true' if animation is in progress
     */
    bool IsAnimating();
    bool IsAnimating(MapViewAnimation::AnimationType type) const;

    float GetFov();
    void GetScreenSize(int& width, int& height);

    float GetMinTiltAngle();

    /*! Set Reference
       @return none
     */
    void SetReference(float screenX, float screenY);

    /*! Enable Reference
       @return none
     */
    void EnableReference(nb_boolean enable);

    /*! Check current animation status
       @return none
     */
    void CheckAnimationStatus();

    double GetMaxZoomLevel();

    double GetMinZoomLevel();

    void SetRotationAngle(float angle, bool animated, bool stopExistingAnimation);

    bool CheckRotationAngle(float& rotationAngle);

    void CheckBoundary();

    void SetScaleFactor(float scaleFactor);

private:

    // Private functions ......................................................................................................
    void SetViewCenter(double mercatorX, double mercatorY);
    void SetRotationAngle(float angle);
    void SetCameraDistance(double distance, bool gesture);

    /*! Animate map transformation */
    void AnimateViewTransformation(uint32 currentTime);

    /*!
     We will check the mercator point before invoke 'm_nbgmMapView->SetViewCenter'
     if to do so, we will avoid the white space displaying during the zoom level at map boundary(North+South).
     Additionally, check the mercator point only when no tilt and rotate, if tilt and rotate is available(begin at zoom level 8),
     and then we set map center directly.
     */
    bool CheckMapCenterData(double &mercatorX, double &mercatorY);

    /*!
       Generate interpolator through acceleration type.
     */
    NBGM_Interpolator<double>* GenerateInterpolator(AnimationAccelerationType type);


    void Turn(float rotationAngle);
    void Zoom(double zoomIndex, bool gesture);
    void SetMapCenter(double mx, double my, bool animated, bool stopExistingAnimation, uint32 duration);


    /*!  Update camera FOV, and NBGM camera setting after screen changed
    */
    void OnScreenChanged();

    /*! Update Fov after Screen changed*/
    void UpdateFovAngle();


    /*! Updates the frustum height (distance to horizon) in NBGM

        This has to be called each time the tilt angle or zoom changes.
     */
    void UpdateFrustumHeight();

    float GetMinTiltAngleDuringGesture(float minTiltAngle);
    void UpdateTiltRatio(float currentZoomLevel, float currentTiltAngle);
    float GetTiltAngleFromTiltRatio();
    float GetLimitedTiltAnlge(double zoomLevel);

private:
    enum ModifiedFlag
    {
        POSITION_MODIFIED           = 0x0004,
        SCREEN_MODIFIED             = 0x0008,
        AVATAR_MODE_MODIFIED        = 0x0010,
        AVATAR_STATE_MODIFIED       = 0x0020,
        AVATAR_LOCATION_MODIFIED    = 0x0040,
        LAYER_VISIBILITY_MODIFIED   = 0x0080,
        BACKGROUND_MODIFIED         = 0x0100
    };
    
    



private:
    /*< do calculation about camera. */
    NBGM_MapCameraHelper* m_nbgmMapCameraHelper;

    PAL_Lock* m_lock;

    /*! animation duration. */
    uint32                              m_AnimationDuration;

    /*! animation acceleration type. */
    AnimationAccelerationType           m_AnimationAccelerationType;

    MapViewAnimation                    m_combinedAnimation;

    /*! the mininum zoomlevel based on camera  */
    double                              m_minCalculatedZoomLevel;


    /*! The width/height is size of the nbgm frame buffer in the device resolution (and not the screen size in pixel) */
    int                                 m_width;
    int                                 m_height;

    /*! Mouse move history, used by MoveTo() */
    double                              m_previousScreenX;
    double                              m_previousScreenY;
    double                              m_previousMercatorX;
    double                              m_previousMercatorY;
    bool                                m_isTouched;

    float                               m_Fov;

    bool                                m_tiltIsBounceingBack;
    bool                                m_rotateIsBounceingBack;
    bool                                m_isInRotationGesture;

    /*! Distance to sky/horizon. Changes based on tilt/camera height */
    double                              m_lastHorizonDistance;

    nb_boolean                          m_enableReference;
    float                               m_referenceX;
    float                               m_referenceY;

    double                              m_rotationCenterX;
    double                              m_rotationCenterY;

    nb_boolean                          m_zoomLevelBounce;
    bool                                m_zoomLevelBounceBack;

    float                               m_zoomCenterX;
    float                               m_zoomCenterY;

    MapViewCameraListener*              m_listener;
    float                               m_currentTiltRatio;
    bool                                m_tiltAnimationFlag;

    uint32                              m_timestamp;

    bool                                m_enableZoomCenter;
    PAL_Lock*                           m_lockForViewCenter;
    float                               m_scaleFactor;
};
}


#endif //_MAP_VIEW_ANIMATION_H_

/*! @} */
