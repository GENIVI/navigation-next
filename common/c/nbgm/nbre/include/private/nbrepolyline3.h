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

    @file nbrepolyline3.h
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

#ifndef _NBRE_POLYLINE3_H_
#define _NBRE_POLYLINE3_H_

#include "paltypes.h"
#include "nbrevector3.h"
#include "nbreaxisalignedbox3.h"
#include "nbretypes.h"
/** \addtogroup NBRE_Core
*  @{
*/

/** Define a point on polyline in parameter coordinates
*/
template <typename T>
struct NBRE_Polyline3Position
{
public:
    NBRE_Polyline3Position():segmentIndex(0), t(0) {}
    NBRE_Polyline3Position(uint32 segmentIndex, T t):segmentIndex(segmentIndex), t(t) {}
public:
    uint32 segmentIndex;
    T t;
};

typedef NBRE_Polyline3Position<float> NBRE_Polyline3Positionf;
typedef NBRE_Polyline3Position<double> NBRE_Polyline3Positiond;

/** Polyline 3d class
*/
template <typename T>
class NBRE_Polyline3
{
public:
    typedef NBRE_Vector< NBRE_Vector3<T> > Point3List;
    typedef NBRE_Vector<T> NBRE_SegmentLengthList;

public:
    NBRE_Polyline3(const NBRE_Vector3<T>* vertices, uint32 vertexCount);
    NBRE_Polyline3(const Point3List& vertices);
    NBRE_Polyline3(uint32 vertexCount);
    ~NBRE_Polyline3();

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
    NBRE_Polyline3Position<T> ConvertOffsetToParameterCoordinate(T offset) const;
    /// Get offset from parameter coordinates point(from beginning)
    T ConvertParameterCoordinateToOffset(const NBRE_Polyline3Position<T>& pos) const;
    /// Get segment direction
    NBRE_Vector3<T> SegmentDirection(uint32 index) const;
    /// Get vertex
    const NBRE_Vector3<T>& Vertex(uint32 index) const;
    /// Set vertex
    void SetVertex(uint32 index, const NBRE_Vector3<T>& value);
    /// Get position from parameter coordinates
    NBRE_Vector3<T> PointAt(const NBRE_Polyline3Position<T>& position) const;
    /// Get position from offset
    NBRE_Vector3<T> PointAt(T offset) const;
    /// Get min distance to point & closest point on polyline
    T DistanceTo(const NBRE_Vector3<T>& point, NBRE_Polyline3Position<T>* pClosestPoint) const;
    /// Recalculate polyline length
    void UpdateLength();
    /// Get bounding box
    const NBRE_AxisAlignedBox3<T>& AABB() const;
    /// Get a part of polyline
    NBRE_Polyline3<T> SubPolyline(T beginOffset, T endOffset) const;

private:
    Point3List mVertices;
    T mLength;
    NBRE_SegmentLengthList mSegmentLengths;
    NBRE_AxisAlignedBox3<T> mAABB;
};

#include "nbrepolyline3.inl"

typedef NBRE_Polyline3<float> NBRE_Polyline3f;
typedef NBRE_Polyline3<double> NBRE_Polyline3d;
typedef NBRE_Polyline3<int32> NBRE_Polyline3i;
/** @} */
#endif
