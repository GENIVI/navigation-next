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
#include "nbrelinearinterpolate.h"
#include "nbresegment3.h"

template <typename T>
NBRE_Polyline3<T>::NBRE_Polyline3(const NBRE_Vector3<T>* vertices, uint32 vertexCount)
{
    nbre_assert(vertexCount >= 2);
    for (uint32 i = 0; i < vertexCount; ++i)
    {
        mVertices.push_back(vertices[i]);
    }
    UpdateLength();
}

template <typename T>
NBRE_Polyline3<T>::NBRE_Polyline3(const Point3List& vertices)
:mVertices(vertices)
{
    nbre_assert(vertices.size() >= 2);
    UpdateLength();
}

template <typename T>
NBRE_Polyline3<T>::NBRE_Polyline3(uint32 vertexCount):
    mLength(0)
{
    nbre_assert(vertexCount >= 2);
    for (uint32 i = 0; i < vertexCount; ++i)
    {
        mVertices.push_back(NBRE_Vector3<T>(0, 0, 0));
        if (i < vertexCount - 1)
        {
            mSegmentLengths.push_back(0);
        }
    }
}

template <typename T>
NBRE_Polyline3<T>::~NBRE_Polyline3()
{
}

template <typename T> uint32
NBRE_Polyline3<T>::VertexCount() const
{
    return static_cast<uint32>(mVertices.size());
}

template <typename T> void
NBRE_Polyline3<T>::UpdateLength()
{
    mLength = 0;
    mSegmentLengths.clear();
    int32 num = static_cast<int32>(mVertices.size()) - 1;
    for (int32 i = 0; i < num; ++i)
    {
        T l = mVertices[i].DistanceTo(mVertices[i + 1]);
        mSegmentLengths.push_back(l);
        mLength += l;
    }
    mAABB.Set(&mVertices.front(), mVertices.size());
}

template <typename T> T 
NBRE_Polyline3<T>::Length() const
{
    return mLength;
}

template <typename T> T 
NBRE_Polyline3<T>::SegmentLength(uint32 index) const
{
    return mSegmentLengths[index];
}

template <typename T> T 
NBRE_Polyline3<T>::SubLength(uint32 beginIndex, uint32 endIndex) const
{
    T result = 0;
    for (uint32 i = beginIndex; i <= endIndex; ++i)
    {
        result += mSegmentLengths[i];
    }
    return result;
}

template <typename T> NBRE_Polyline3Position<T>
NBRE_Polyline3<T>::ConvertOffsetToParameterCoordinate(T offset) const
{
    NBRE_Polyline3Position<T> result;
    int32 vertexCount = static_cast<int32>(mVertices.size());
    T dx = 0;
    int32 i = 0;
    for (i = 0; i < vertexCount - 1; ++i)
    {
        if (dx + mSegmentLengths[i] >= offset)
        {
            result.segmentIndex = i;
            result.t = (offset - dx) / mSegmentLengths[i];
            return result;
        }
        dx += mSegmentLengths[i];
    }

    if (i == vertexCount - 1)
    {
        result.segmentIndex = vertexCount - 2;
        result.t = (offset - (mLength - mSegmentLengths.back())) / mSegmentLengths.back();
    }
    return result;
}

template <typename T> T
NBRE_Polyline3<T>::ConvertParameterCoordinateToOffset(const NBRE_Polyline3Position<T>& pos) const
{
    T offset = mSegmentLengths[pos.segmentIndex] * pos.t;
    for (uint32 i = 0; i < pos.segmentIndex; ++i)
    {
        offset += mSegmentLengths[i];
    }
    return offset;
}

template <typename T> NBRE_Vector3<T> 
NBRE_Polyline3<T>::SegmentDirection(uint32 index) const
{
    NBRE_Vector3<T> result = mVertices[index + 1] - mVertices[index];
    result.Normalise();
    return result;
}

template <typename T> const NBRE_Vector3<T>& 
NBRE_Polyline3<T>::Vertex(uint32 index) const
{
    return mVertices[index];
}

template <typename T> void 
NBRE_Polyline3<T>::SetVertex(uint32 index, const NBRE_Vector3<T>& value)
{
    mVertices[index] = value;
}

template <typename T> NBRE_Vector3<T> 
NBRE_Polyline3<T>::PointAt(const NBRE_Polyline3Position<T>& position) const
{
    return NBRE_LinearInterpolate<T>::Lerp(mVertices[position.segmentIndex], mVertices[position.segmentIndex + 1], position.t);
}

template <typename T> NBRE_Vector3<T> 
NBRE_Polyline3<T>::PointAt(T offset) const
{
    return PointAt(ConvertOffsetToParameterCoordinate(offset));
}

template <typename T> T 
NBRE_Polyline3<T>::DistanceTo(const NBRE_Vector3<T>& point, NBRE_Polyline3Position<T>* pClosestPoint) const
{
    T minDist = 0;
    NBRE_Polyline3Position<T> closestPoint;

    for (uint32 i = 0; i < mVertices.size() - 1; ++i)
    {
        NBRE_Segment3<T> seg(mVertices[i], mVertices[i + 1]);
        T t = seg.GetClosestT(point);
        const NBRE_Vector3<T>& cp = NBRE_LinearInterpolate<T>::Lerp(mVertices[i], mVertices[i + 1], t);
        T dist = cp.DistanceTo(point);

        if (i == 0 || dist < minDist)
        {
            closestPoint.segmentIndex = i;
            closestPoint.t = t;
            minDist = dist;
        }
    }
    if (pClosestPoint != NULL)
    {
        *pClosestPoint = closestPoint;
    }
    return minDist;
}

template <typename T> const NBRE_AxisAlignedBox3<T>& 
NBRE_Polyline3<T>::AABB() const
{
    return mAABB;
}

template <typename T> NBRE_Polyline3<T> 
NBRE_Polyline3<T>::SubPolyline(T beginOffset, T endOffset) const
{
    Point3List vs;
    NBRE_Polyline3Position<T> coordBegin = ConvertOffsetToParameterCoordinate(beginOffset);
    NBRE_Polyline3Position<T> coordEnd = ConvertOffsetToParameterCoordinate(endOffset);

    vs.push_back(PointAt(coordBegin));
    for (uint32 i = coordBegin.segmentIndex + 1; i <= coordEnd.segmentIndex; ++i)
    {
        vs.push_back(mVertices[i]);
    }
    vs.push_back(PointAt(coordEnd));

    return NBRE_Polyline3<T>(vs);
}
