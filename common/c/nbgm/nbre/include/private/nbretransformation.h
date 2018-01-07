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

    @file nbretransformation.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_TRANSFORMATION_H_
#define _NBRE_TRANSFORMATION_H_

#include "paltypes.h"
#include "nbrevector2.h"
#include "nbrevector3.h"
#include "nbrematrix3x3.h"
#include "nbrematrix4x4.h"
#include "nbresegment2.h"
#include "nbresegment3.h"
#include "nbreray3.h"
#include "nbreplane.h"
#include "nbreaxisalignedbox3.h"
#include "nbrefrustum3.h"
#include "nbretriangle3.h"
#include "palmath.h"
#include "nbrenoneaxisalignedbox3.h"

/** \addtogroup NBRE_Core
*  @{
*/
//! Geometry transform algorithms
/** This class use OpenGL style transformation:
1. vector is column matrix V' = M * V
2. matrix order is right to left: M(Final) = M(Viewport) * M(Projection) * M(View) * M(Model);
    [ Xx Yx Zx Tx ] [ x ]
    [ Xy Yy Zy Ty ] [ y ]
    [ Xz Yz Zz Tz ] [ z ]
    [  0  0  0  1 ] [ 1 ]
3. right handed coordinates (x, y, z is CCW order)
        y
        |
        |
        |______ x
       /
      /
     /
    z
*/
template <typename Real>
class NBRE_Transformation
{
public:
    // OpenGL style transformation
    // V is a COLUMN vector: V' = M * V
    // concatenate order: M = Mn * Mn-1 * ... * M1
    // If row vector is used
    // transpose the matrix and concatenate with inverse order V' = V * M1T * M2T * ... * MnT    (T means Transpose)
    // 3D Transformations
    static NBRE_Matrix4x4<Real> BuildTranslateMatrix(Real x, Real y, Real z);
    static NBRE_Matrix4x4<Real> BuildTranslateMatrix(const NBRE_Vector3<Real>& translate);
    static NBRE_Matrix4x4<Real> BuildScaleMatrix(Real x, Real y, Real z);
    static NBRE_Matrix4x4<Real> BuildScaleMatrix(const NBRE_Vector3<Real>& scale);
    static NBRE_Matrix4x4<Real> BuildRotateMatrix(Real radians, Real x, Real y, Real z);
    static NBRE_Matrix4x4<Real> BuildRotateMatrix(Real radians, const NBRE_Vector3<Real>& axis);
    static NBRE_Matrix4x4<Real> BuildRotateDegreesMatrix(Real degrees, Real x, Real y, Real z);
    static NBRE_Matrix4x4<Real> BuildRotateDegreesMatrix(Real degrees, const NBRE_Vector3<Real>& axis);
    static NBRE_Matrix4x4<Real> BuildRotateAtMatrix(Real radians, const NBRE_Vector3<Real>& axis, const NBRE_Vector3<Real>& position);
    static NBRE_Vector3<Real> CoordinatesGetTranslate(const NBRE_Matrix4x4<Real>& matrix);
    static NBRE_Vector3<Real> CoordinatesGetXAxis(const NBRE_Matrix4x4<Real>& matrix);
    static NBRE_Vector3<Real> CoordinatesGetYAxis(const NBRE_Matrix4x4<Real>& matrix);
    static NBRE_Vector3<Real> CoordinatesGetZAxis(const NBRE_Matrix4x4<Real>& matrix);
    
    // 2D Transformations
    static NBRE_Matrix3x3<Real> BuildTranslateMatrix2D(Real x, Real y);
    static NBRE_Matrix3x3<Real> BuildTranslateMatrix2D(const NBRE_Vector2<Real>& translate);
    static NBRE_Matrix3x3<Real> BuildScaleMatrix2D(Real x, Real y);
    static NBRE_Matrix3x3<Real> BuildScaleMatrix2D(const NBRE_Vector2<Real>& scale);
    static NBRE_Matrix3x3<Real> BuildRotateMatrix2D(Real radians);
    static NBRE_Matrix3x3<Real> BuildRotateDegreesMatrix2D(Real degrees);
    /// Right handed coordinates camera matrix
    static NBRE_Matrix4x4<Real> BuildLookAtMatrix(const NBRE_Vector3<Real>& eye, const NBRE_Vector3<Real>& center, const NBRE_Vector3<Real>& up);
    /// Ortho projection matrix
    static NBRE_Matrix4x4<Real> BuildOrthoMatrix(Real left, Real right, Real bottom, Real top, Real zNear, Real zFar);
    /// Perspective projection matrix
    static NBRE_Matrix4x4<Real> BuildFrustumMatrix(Real left, Real right, Real bottom, Real top, Real zNear, Real zFar);
    /// Perspective projection matrix
    static NBRE_Matrix4x4<Real> BuildPerspectiveMatrix(Real fovYDegrees, Real aspect, Real nearZ, Real farZ);
    /// Viewport matrix
    static NBRE_Matrix4x4<Real> BuildViewportMatrix(Real x, Real y, Real width, Real height);

    // Transform geometry objects
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Vector3<Real>& vec3);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Vector4<Real>& vec4);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Ray3<Real>& ray);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Segment3<Real>& segment);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Plane<Real>& plane);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_AxisAlignedBox3<Real>& aabb);
    static void Transform(NBRE_Matrix4x4<Real> matrix, NBRE_NoneAxisAlignedBox3<Real>& box);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Frustum3<Real>& frustum);
    static void Transform(const NBRE_Matrix4x4<Real>& matrix, NBRE_Triangle3<Real>& triangle);
    //! Convert a world point to screen point
    /** 
    @remark If the point is behind eye, result is incorrect
    @param modelViewProjectViewportMatrix merged matrix
    @param position the point in world space
    @return NBRE_Vector2<Real> position in screen space
    */
    static NBRE_Vector2<Real> WorldToScreen(const NBRE_Matrix4x4<Real>& modelViewProjectViewportMatrix, const NBRE_Vector3<Real>& position);
    static NBRE_Vector2<Real> WorldToScreen(const NBRE_Matrix4x4<Real>& modelViewProjectViewportMatrix, const NBRE_Vector3<Real>& position, Real* pW);

    //! Convert a screen point to world
    /**
    @param modelViewMatrix model view matrix
    @param projectMatrix projection matrix
    @param viewportMatrix viewport matrix
    @param position screen position
    @param depth depth of point, 0 is on near plane, 1 is on far plane
    @return NBRE_Vector3<Real> world position of screen point
    */
    static NBRE_Vector3<Real> ScreenToWorld(const NBRE_Matrix4x4<Real>& modelViewMatrix, const NBRE_Matrix4x4<Real>& projectMatrix, const NBRE_Matrix4x4<Real>& viewportMatrix, const NBRE_Vector2<Real>& position, Real depth);

    //! Get a picking ray from give screen point
    /**
    @param modelViewMatrix model view matrix
    @param projectMatrix projection matrix
    @param viewportMatrix viewport matrix
    @param position screen position
    @return NBRE_Ray3<Real> picking ray in world space
    */
    static NBRE_Ray3<Real> GetPickingRay(const NBRE_Matrix4x4<Real>& modelViewMatrix, const NBRE_Matrix4x4<Real>& projectMatrix, const NBRE_Matrix4x4<Real>& viewportMatrix, const NBRE_Vector2<Real>& position);

    //! Get pixels per unit at give world position
    /**
    #remark This function calculate pixel size by placing a billboard at given position
    @param modelView model view matrix
    @param project projection matrix
    @param viewport viewport matrix
    @param position world position
    @param unit unit length in world space
    @return Real pixels of unit
    */
    static Real GetPixelsPerUnit(const NBRE_Matrix4x4<Real>& modelView, const NBRE_Matrix4x4<Real>& project, const NBRE_Matrix4x4<Real>& viewport, const NBRE_Vector3<Real>& position, Real unit);
};

#include "nbretransformation.inl"

typedef NBRE_Transformation<float> NBRE_Transformationf;
typedef NBRE_Transformation<double> NBRE_Transformationd;
/** @} */
#endif
