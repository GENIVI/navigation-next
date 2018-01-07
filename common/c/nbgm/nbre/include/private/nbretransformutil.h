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

    @file nbretransformutil.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_TRANSFORM_UTIL_H_
#define _NBRE_TRANSFORM_UTIL_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "palerror.h"
#include "nbreveiwport.h"
#include "nbreplane.h"
#include "nbrefrustum3.h"
#include "palclock.h"
#include "nbrepolyline2.h"
#include "nbrepolyline3.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** Layout manager manages elements which cannot overlap each other.
Like labels, shield, poi...
*/
class NBRE_TransformUtil
{
public:
    typedef NBRE_Vector<NBRE_Polyline2d> Polyline2dList;
    typedef NBRE_Vector<NBRE_Polyline3d> Polyline3dList;

public:
    NBRE_TransformUtil(NBRE_Viewport* viewport);
    ~NBRE_TransformUtil();

public:
    /// Update camera matrix
    nb_boolean UpdateCameraMatrix();
    /// Update screen center pixel per unit
    void UpdatePixelScale();
    /// Get camera
    const NBRE_CameraPtr& GetCamera() const { return mViewport->Camera(); }
    /// Get plane side
    nb_boolean IsInNearPlaneFrontSide(const NBRE_Vector3d& position);
    /// Is polyline in near plane front side
    nb_boolean IsInNearPlaneFrontSide(const NBRE_Polyline3d& polyline);
    /// Is point in frustum
    nb_boolean IsInFrustum(const NBRE_Vector3d& position);
    /// Is aabb in frustum
    nb_boolean IsInFrustum(const NBRE_AxisAlignedBox3d& aabb);
    /// Is polyline in frustum
    nb_boolean IsInFrustum(const NBRE_Polyline3d& polyline);
    /// Project a point from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition);
    /// Project a point from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition, double& w);
    /// Project a polyline from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Polyline3d& polyline, NBRE_Polyline2d& screenPolyline);
    /// Get pixels per unit at position
    nb_boolean PixelsPerUnit(const NBRE_Vector3d& position, double& pixelsPerUnit);
    /// Get eye position
    const NBRE_Vector3d& GetEyePosition() { return mEyePosition; }
    /// Get camera right
    const NBRE_Vector3d& GetCameraRight() { return mCameraRight; }
    /// Get screen boundary
    const NBRE_AxisAlignedBox2d& GetScreenRect() { return mScreenRect; }
    /// Get screen center pixels per unit
    double ScreenCenterPixelsPerUnit() { return mScreenCenterPixelsPerUnit; }
    /// Get Matrix
    const NBRE_Matrix4x4d& GetViewProjectViewportMatrix() { return mViewProjectViewportMatrix; }
    /// Get eye to screen center distance
    double GetEyeToScreenCenterDistance() const { return mEyeToScreenCenterDistance; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TransformUtil);

private:
    /// viewport
    NBRE_Viewport* mViewport;
    /// screen rect
    NBRE_AxisAlignedBox2d mScreenRect;
    /// view matrix
    NBRE_Matrix4x4d mViewMatrix;
    /// project matrix
    NBRE_Matrix4x4d mProjectMatrix;
    /// viewport matrix
    NBRE_Matrix4x4d mViewportMatrix;
    /// mvp
    NBRE_Matrix4x4d mViewProjectViewportMatrix;
    /// frustum
    NBRE_Frustum3d mFrustum;
    /// camera direction
    NBRE_Vector3d mEyePosition;
    /// camera right
    NBRE_Vector3d mCameraRight;
    /// screen center pixels per unit
    double mScreenCenterPixelsPerUnit;
    /// eye to screen center distance
    double mEyeToScreenCenterDistance;
};

/*! @} */
#endif
