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
#include "nbgmlinearinterpolate.h"

template <typename T>
NBGM_Polyline2<T>::NBGM_Polyline2(const NBGM_Vector2<T>* vertices, uint32 vertexCount)
{
    nbre_assert(vertexCount >= 2);
    for (uint32 i = 0; i < vertexCount; ++i)
    {
        mVertices.push_back(vertices[i]);
    }
    UpdateLength();
}

template <typename T>
NBGM_Polyline2<T>::NBGM_Polyline2(const NBGM_Vector<NBGM_Vector2<T>>& vertices)
:mVertices(vertices)
{
    nbre_assert(vertices.size() >= 2);
    UpdateLength();
}

template <typename T>
NBGM_Polyline2<T>::~NBGM_Polyline2()
{
}

template <typename T> uint32
NBGM_Polyline2<T>::VertexCount() const
{
    return static_cast<uint32>(mVertices.size());
}

template <typename T> void
NBGM_Polyline2<T>::UpdateLength()
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
}

template <typename T> T 
NBGM_Polyline2<T>::Length() const
{
    return mLength;
}

template <typename T> T 
NBGM_Polyline2<T>::SegmentLength(uint32 index) const
{
    return mSegmentLengths[index];
}

template <typename T> T 
NBGM_Polyline2<T>::SubLength(uint32 beginIndex, uint32 endIndex) const
{
    T result = 0;
    for (uint32 i = beginIndex; i <= endIndex; ++i)
    {
        result += mSegmentLengths[i];
    }
    return result;
}

template <typename T> NBGM_Polyline2Position<T>
NBGM_Polyline2<T>::ConvertOffsetToParameterCoordinate(T offset) const
{
    NBGM_Polyline2Position<T> result;
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

template <typename T> NBGM_Vector2<T> 
NBGM_Polyline2<T>::SegmentDirection(uint32 index) const
{
    NBGM_Vector2<T> result = mVertices[index + 1] - mVertices[index];
    result.Normalise();
    return result;
}

template <typename T> const NBGM_Vector2<T>& 
NBGM_Polyline2<T>::Vertex(uint32 index) const
{
    return mVertices[index];
}

template <typename T> NBGM_Vector2<T> 
NBGM_Polyline2<T>::PointAt(const NBGM_Polyline2Position<T>& position) const
{
    return NBGM_LinearInterpolate<T>::Lerp(mVertices[position.segmentIndex], mVertices[position.segmentIndex + 1], position.t);
}
