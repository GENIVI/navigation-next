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

    @file nbgmlayoutpolyline.h
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
#ifndef _NBGM_LAYOUT_POLYLINE_H_
#define _NBGM_LAYOUT_POLYLINE_H_
#include "nbretypes.h"
#include "nbreplane.h"
#include "nbrepolyline3.h"
#include "nbrepolyline2.h"
#include "nbreaxisalignedbox2.h"
#include "nbgmcontext.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** A layout polyline vertex.
*/
class NBGM_LayoutVertex
{
    friend class NBGM_LayoutPolyline;
public:
    NBGM_LayoutVertex();
    NBGM_LayoutVertex(NBRE_Vector3d worldPosition);
    ~NBGM_LayoutVertex();

public:
    NBGM_LayoutVertex Lerp(const NBGM_LayoutVertex& v, double t) const;
    const NBRE_Vector2d& ScreenPosition() const { return mScreenPosition; }
    const NBRE_Vector3d& WorldPosition() const { return mWorldPosition; }
    double W() const { return mW; }
    double Width() const { return mWidth; }

private:
    NBRE_Vector2d mScreenPosition;
    NBRE_Vector3d mWorldPosition;
    double mW;
    double mWidth;
};
typedef NBRE_Vector<NBGM_LayoutVertex> NBGM_LayoutVertexList;

/** Polyline position in parameter coordinates
*/
struct NBGM_LayoutPolylinePosition
{
public:
    NBGM_LayoutPolylinePosition():segmentIndex(0), t(0) {}
    NBGM_LayoutPolylinePosition(uint32 segmentIndex, double t):segmentIndex(segmentIndex), t(t) {}
public:
    uint32 segmentIndex;
    double t;
};

class NBGM_LayoutPolyline;
typedef NBRE_Vector<NBGM_LayoutPolyline> NBGM_LayoutPolylineList;

/** Layout polyline contains both 2d and 3d information.
It can be converted between 2d & 3d.
*/
class NBGM_LayoutPolyline
{
public:
    typedef NBRE_Vector<double> FloatArray;

public:
    NBGM_LayoutPolyline(NBGM_Context* nbgmContext, uint32 vertexCount);
    NBGM_LayoutPolyline(NBGM_Context* nbgmContext, const NBRE_Polyline3d& worldPolyline, double width);
    NBGM_LayoutPolyline(NBGM_Context* nbgmContext, const NBGM_LayoutVertexList& vertices);
    ~NBGM_LayoutPolyline();

public:
    /// Get parameter coordinates point from polyline offset(from beginning)
    NBGM_LayoutPolylinePosition ConvertOffsetToParameterCoordinate(double offset) const;
    /// Get parameter coordinates point from polyline offset(from beginning)
    double ConvertParameterCoordinateToOffset(NBGM_LayoutPolylinePosition pos) const;
    /// Get vertex
    NBGM_LayoutVertex Vertex(uint32 index) const;
    /// Get vertex count
    uint32 VertexCount() const { return mScreenPolyline.VertexCount(); }
    /// Set vertex
    void SetVertex(uint32 index, const NBGM_LayoutVertex& value);
    /// Get position from parameter coordinates
    NBGM_LayoutVertex PointAt(const NBGM_LayoutPolylinePosition& position) const;
    /// Get position from offset
    NBGM_LayoutVertex PointAt(double offset) const;
    /// Get sub line
    NBGM_LayoutPolyline SubPolyline(double beginOffset, double endOffset) const;
    /// Min distance to point
    double DistanceTo(const NBRE_Vector2d& point, NBGM_LayoutPolylinePosition* pClosestPoint) const;
    /// Convert screen offset to world
    NBGM_LayoutPolylinePosition ConvertScreenCoordinateToWorld(NBGM_LayoutPolylinePosition screenPos) const;
    /// Convert world offset to screen
    NBGM_LayoutPolylinePosition ConvertWorldCoordinateToScreen(NBGM_LayoutPolylinePosition worldPos) const;
    /// Convert world offset to screen
    double ConvertWorldOffsetToScreen(double offset) const;
    /// Convert screen offset to world
    double ConvertScreenOffsetToWorld(double offset) const;
    /// Get polyline in screen space
    const NBRE_Polyline2d& GetScreenPolyline() const { return mScreenPolyline; }
    /// Get polyline in world space
    const NBRE_Polyline3d& GetWorldPolyline() const { return mWorldPolyline; }
    /// Convert polyline from world space to screen space, return FALSE if part of polyline behind near plane
    nb_boolean UpdateScreenPositionFromWorldPosition();
    /// Get polyline parts without any collision in screen
    void GetScreenAvaliableParts(int32 lineWidth, uint32 mask, NBGM_LayoutPolylineList& result);
    /// Get polyline parts in rect
    void ClipByRect(const NBRE_AxisAlignedBox2d& rect, NBGM_LayoutPolylineList& result);

private:
    NBGM_Context* mNBGMContext;
    NBRE_Polyline2d mScreenPolyline;
    NBRE_Polyline3d mWorldPolyline;
    FloatArray mWArray;
    FloatArray mWidthArray;
};

/*! @} */
#endif
