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

    @file nbrepolyline2.h
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

#ifndef _NBRE_POLYLINE2_H_
#define _NBRE_POLYLINE2_H_

#include "paltypes.h"
#include "nbrevector2.h"
#include "nbretypes.h"
#include "nbreaxisalignedbox2.h"
/** \addtogroup NBRE_Core
*  @{
*/

/** Define a point on polyline in parameter coordinates
*/
template <typename T>
struct NBRE_Polyline2Position
{
public:
    NBRE_Polyline2Position():segmentIndex(0), t(0) {}
    NBRE_Polyline2Position(uint32 segmentIndex, T t):segmentIndex(segmentIndex), t(t) {}
public:
    uint32 segmentIndex;
    T t;
};

typedef NBRE_Polyline2Position<float> NBRE_Polyline2Positionf;
typedef NBRE_Polyline2Position<double> NBRE_Polyline2Positiond;

/** Polyline 2d class
*/
template <typename T>
class NBRE_Polyline2
{
public:
    struct Polyline2Segment
    {
    public:
        NBRE_Vector2<T> vertices[4];
        NBRE_Vector2<T> xAxis;
        NBRE_Vector2<T> yAxis;
        T xIntervals[2];
        T yIntervals[2];
        NBRE_AxisAlignedBox2<T> aabb;
    };
    typedef NBRE_Vector<Polyline2Segment> SegmentList;
    typedef NBRE_Vector< NBRE_Vector2<T> > Point2List;
    typedef NBRE_Vector<T> NBRE_SegmentLengthList;

public:
    NBRE_Polyline2(const NBRE_Vector2<T>* vertices, uint32 vertexCount);
    NBRE_Polyline2(const Point2List& vertices);
    NBRE_Polyline2(uint32 vertexCount);
    ~NBRE_Polyline2();

public:
    /// Get vertex count
    uint32 VertexCount() const;
    /// Total length
    T Length() const;
    /// Segment length
    T SegmentLength(uint32 index) const;
    /// Sub line length
    T SubLength(uint32 beginIndex, uint32 endIndex) const;
    /// Get parameter coordinates point from polyline offset(from beginning)
    NBRE_Polyline2Position<T> ConvertOffsetToParameterCoordinate(T offset) const;
    /// Get offset from parameter coordinates point(from beginning)
    T ConvertParameterCoordinateToOffset(const NBRE_Polyline2Position<T>& pos) const;
    /// Get segment direction
    NBRE_Vector2<T> SegmentDirection(uint32 index) const;
    /// Get vertex
    const NBRE_Vector2<T>& Vertex(uint32 index) const;
    /// Set vertex
    void SetVertex(uint32 index, const NBRE_Vector2<T>& value);
    /// Get position from parameter coordinates
    NBRE_Vector2<T> PointAt(const NBRE_Polyline2Position<T>& position) const;
    /// Get position from offset
    NBRE_Vector2<T> PointAt(T offset) const;
    /// Get min distance to point & closest point on polyline
    T DistanceTo(const NBRE_Vector2<T>& point, NBRE_Polyline2Position<T>* pClosestPoint) const;
    /// Recalculate polyline length
    void UpdateLength();
    /// Get a part of polyline
    NBRE_Polyline2<T> SubPolyline(T beginOffset, T endOffset) const;
    /// Reset polyline vertices
    void Reset(const NBRE_Vector2<T>* vertices, uint32 vertexCount);
    /// Reset polyline vertices
    void Reset(const Point2List& vertices);
    /// Reset polyline vertices
    void Reset(uint32 vertexCount);
    /// Get segments which do not collide with polyline
    void ClipByPolyline(NBRE_Polyline2<T>& pl, NBRE_Vector<NBRE_Polyline2<T> >& result);
    /// Test if wide polyline hit aabb
    nb_boolean WideLineHitTest(const NBRE_AxisAlignedBox2<T>& aabb);
    /// Test if wide polyline hit polyline
    nb_boolean WideLineHitTest(NBRE_Polyline2<T>& pl);
    /// Get polyline width
    T GetWidth() const;
    /// Set polyline width
    void SetWidth(T value);
    /// Smooth the polyline by radius
    void Smooth(T radius);
public:
    static bool CompareIntervals(const NBRE_Vector2<T>& v1, const NBRE_Vector2<T>& v2);
    void UpdateSegments();

private:
    Point2List mVertices;
    T mLength;
    NBRE_SegmentLengthList mSegmentLengths;
    T mWidth;
    SegmentList mSegments;
};

#include "nbrepolyline2.inl"

typedef NBRE_Polyline2<float> NBRE_Polyline2f;
typedef NBRE_Polyline2<double> NBRE_Polyline2d;
typedef NBRE_Vector< NBRE_Polyline2<float> > NBRE_Polyline2fList;
typedef NBRE_Vector< NBRE_Polyline2<double> > NBRE_Polyline2dList;

typedef NBRE_Vector<NBRE_Vector2i> NBGM_Point2iList;
/** @} */
#endif
