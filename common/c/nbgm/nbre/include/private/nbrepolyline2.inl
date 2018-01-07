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
#include "nbresegment2.h"
#include "nbreaxisalignedbox2.h"

template <typename T>
NBRE_Polyline2<T>::NBRE_Polyline2(const NBRE_Vector2<T>* vertices, uint32 vertexCount)
:mWidth(0)
{
    Reset(vertices, vertexCount);
}

template <typename T>
NBRE_Polyline2<T>::NBRE_Polyline2(const Point2List& vertices):
    mVertices(vertices),
    mLength(0),
    mWidth(0)
{
    Reset(vertices);
}

template <typename T>
NBRE_Polyline2<T>::NBRE_Polyline2(uint32 vertexCount)
:mWidth(0)
{
    Reset(vertexCount);
}

template <typename T>
NBRE_Polyline2<T>::~NBRE_Polyline2()
{
}

template <typename T> uint32
NBRE_Polyline2<T>::VertexCount() const
{
    return static_cast<uint32>(mVertices.size());
}

template <typename T> void
NBRE_Polyline2<T>::UpdateLength()
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
NBRE_Polyline2<T>::Length() const
{
    return mLength;
}

template <typename T> T 
NBRE_Polyline2<T>::SegmentLength(uint32 index) const
{
    return mSegmentLengths[index];
}

template <typename T> T 
NBRE_Polyline2<T>::SubLength(uint32 beginIndex, uint32 endIndex) const
{
    T result = 0;
    for (uint32 i = beginIndex; i <= endIndex; ++i)
    {
        result += mSegmentLengths[i];
    }
    return result;
}

template <typename T> NBRE_Polyline2Position<T>
NBRE_Polyline2<T>::ConvertOffsetToParameterCoordinate(T offset) const
{
    NBRE_Polyline2Position<T> result;
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
NBRE_Polyline2<T>::ConvertParameterCoordinateToOffset(const NBRE_Polyline2Position<T>& pos) const
{
    T offset = mSegmentLengths[pos.segmentIndex] * pos.t;
    for (uint32 i = 0; i < pos.segmentIndex; ++i)
    {
        offset += mSegmentLengths[i];
    }
    return offset;
}

template <typename T> NBRE_Vector2<T> 
NBRE_Polyline2<T>::SegmentDirection(uint32 index) const
{
    NBRE_Vector2<T> result = mVertices[index + 1] - mVertices[index];
    result.Normalise();
    return result;
}

template <typename T> const NBRE_Vector2<T>& 
NBRE_Polyline2<T>::Vertex(uint32 index) const
{
    return mVertices[index];
}

template <typename T> void 
NBRE_Polyline2<T>::SetVertex(uint32 index, const NBRE_Vector2<T>& value)
{
    mVertices[index] = value;
}

template <typename T> NBRE_Vector2<T> 
NBRE_Polyline2<T>::PointAt(const NBRE_Polyline2Position<T>& position) const
{
    return NBRE_LinearInterpolate<T>::Lerp(mVertices[position.segmentIndex], mVertices[position.segmentIndex + 1], position.t);
}

template <typename T> NBRE_Vector2<T> 
NBRE_Polyline2<T>::PointAt(T offset) const
{
    return PointAt(ConvertOffsetToParameterCoordinate(offset));
}

template <typename T> T 
NBRE_Polyline2<T>::DistanceTo(const NBRE_Vector2<T>& point, NBRE_Polyline2Position<T>* pClosestPoint) const
{
    T minDist = 0;
    NBRE_Polyline2Position<T> closestPoint;

    for (uint32 i = 0; i < mVertices.size() - 1; ++i)
    {
        NBRE_Segment2<T> seg(mVertices[i], mVertices[i + 1]);
        T t = seg.GetClosestT(point);
        const NBRE_Vector2<T>& cp = NBRE_LinearInterpolate<T>::Lerp(mVertices[i], mVertices[i + 1], t);
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

template <typename T> NBRE_Polyline2<T> 
NBRE_Polyline2<T>::SubPolyline(T beginOffset, T endOffset) const
{
    Point2List vs;
    NBRE_Polyline2Position<T> coordBegin = ConvertOffsetToParameterCoordinate(beginOffset);
    NBRE_Polyline2Position<T> coordEnd = ConvertOffsetToParameterCoordinate(endOffset);

    vs.push_back(PointAt(coordBegin));
    for (uint32 i = coordBegin.segmentIndex + 1; i <= coordEnd.segmentIndex; ++i)
    {
        vs.push_back(mVertices[i]);
    }
    vs.push_back(PointAt(coordEnd));

    NBRE_Polyline2<T> result(vs);
    result.mWidth = mWidth;
    return result;
}

template <typename T> void
NBRE_Polyline2<T>::Reset(const NBRE_Vector2<T>* vertices, uint32 vertexCount)
{
    nbre_assert(vertexCount >= 2);
    mVertices.clear();
    mLength = 0;
    mSegmentLengths.clear();

    for (uint32 i = 0; i < vertexCount; ++i)
    {
        mVertices.push_back(vertices[i]);
    }
    UpdateLength();
}

template <typename T> void
NBRE_Polyline2<T>::Reset(const Point2List& vertices)
{
    nbre_assert(vertices.size() >= 2);
    mVertices = vertices;
    mLength = 0;
    mSegmentLengths.clear();
    UpdateLength();
}

template <typename T> void
NBRE_Polyline2<T>::Reset(uint32 vertexCount)
{
    nbre_assert(vertexCount >= 2);
    mVertices.clear();
    mLength = 0;
    mSegmentLengths.clear();

    for (uint32 i = 0; i < vertexCount; ++i)
    {
        mVertices.push_back(NBRE_Vector2<T>(0,0));
        if (i < vertexCount - 1)
        {
            mSegmentLengths.push_back(0);
        }
    }
}

template<typename T> nb_boolean
LineClipping(const NBRE_AxisAlignedBox2<T>& rect, NBRE_Vector2<T>& np1, NBRE_Vector2<T>& np2)
{
    // Original position before clipping
    T x1 = np1.x;
    T x2 = np2.x;
    T y1 = np1.y;
    T y2 = np2.y;
    // P-Code for p1 & p2
    uint8 p1code = 0;
    uint8 p2code = 0;

    if (y1 < rect.minExtend.y)
    {
        p1code |= 8;
    }
    else if (y1 > rect.maxExtend.y)
    {
        p1code |= 4;  
    }

    if (x1 < rect.minExtend.x)
    {
        p1code |= 1;  
    }
    else if (x1 > rect.maxExtend.x)
    {
        p1code |= 2;
    }

    if (y2 < rect.minExtend.y)
    {
        p2code |= 8;
    }
    else if (y2 > rect.maxExtend.y)
    {
        p2code |= 4;  
    }

    if (x2 < rect.minExtend.x)
    {
        p2code |= 1;
    }
    else if (x2 > rect.maxExtend.x)
    {
        p2code |= 2;
    }

    // Both end points in the same side
    if ((p1code & p2code))  
        return FALSE;

    // Both end points in center area
    if (p1code == 0 && p2code == 0)  
    {  
        np1.x = x1;  
        np1.y = y1;  
        np2.x = x2;
        np2.y = y2;        
        return TRUE;
    }  

    // Get new position of p1 (np1.x, np1.y)
    switch(p1code)  
    {  
    case 0: // C   
        {  
            np1.x = x1;  
            np1.y = y1;  
        } 
        break;  
    case 8: // N   
        {  
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 4: // S   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;
    case 1: // W   
        {  
            np1.x = rect.minExtend.x;  
            np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 2: // E   
        {  
            np1.x = rect.maxExtend.x;  
            np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 9: // NW   
        {  
            // Assume intersect with yMin
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            // If wrong, intersect with xMin
            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)
            {  
                np1.x = rect.minExtend.x;  
                np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 10: // NE   
        {  
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.maxExtend.x;  
                np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 6: // SE   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.maxExtend.x;  
                np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 5: // SW   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.minExtend.x;  
                np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    }  

    // Get new position of p2 (np2.x, np2.y), same as p1
    switch(p2code)  
    {  
    case 0: // C   
        {  
            np2.x = x2;  
            np2.y = y2;  
        } 
        break;  
    case 8: // N
        {  
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 4: // S   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 1: // W   
        {  
            np2.x = rect.minExtend.x;  
            np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 2: // E   
        {  
            np2.x = rect.maxExtend.x;  
            np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 9: // NW   
        {
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)
            {  
                np2.x = rect.minExtend.x;  
                np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 10: // NE   
        {  
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.maxExtend.x;  
                np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 6: // SE   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.maxExtend.x;  
                np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 5: // SW   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.minExtend.x;  
                np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    }  

    if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x ||  
        np1.y < rect.minExtend.y || np1.y > rect.maxExtend.y ||  
        np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x ||  
        np2.y < rect.minExtend.y || np2.y > rect.maxExtend.y)  
    {  
        return FALSE;
    }

    if (p1code == 0)
    {
        return TRUE;
    }
    else if (p2code == 0)
    {
        return TRUE;
    }
    else
    {
        return TRUE;
    }
}

template<typename T> nb_boolean
IsPointInQuad(const NBRE_Vector2<T>& p, NBRE_Vector2<T>* quad)
{
    uint32 intersectionCount = 0;
    for (uint32 i = 0; i < 4; ++i)
    {
        const NBRE_Vector2<T>& p1 = quad[i];
        const NBRE_Vector2<T>& p2 = quad[(i + 1) % 4];
        if (p1.y == p2.y
            || (p.y < p1.y && p.y < p2.y)
            || (p.y >= p1.y && p.y >= p2.y))
        {
            continue;
        }
        T intersecX = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
        if (intersecX > p.x )
        {
            ++intersectionCount;
        }
    }

    return intersectionCount % 2 ? TRUE : FALSE;
}

template <typename T> bool
NBRE_Polyline2<T>::CompareIntervals(const NBRE_Vector2<T>& v1, const NBRE_Vector2<T>& v2)
{
    return v1.x < v2.x;
}

template <typename T> void
NBRE_Polyline2<T>::ClipByPolyline(NBRE_Polyline2<T>& pl, NBRE_Vector<NBRE_Polyline2<T> >& result)
{
    if (mSegments.size() == 0)
    {
        UpdateSegments();
    }
    if (pl.mSegments.size() == 0)
    {
        pl.UpdateSegments();
    }

    NBRE_Vector<NBRE_Vector2<T> > intervals;
    T halfW = mWidth / 2;
    uint32 vc = mSegments.size();
    uint32 vc2 = pl.mSegments.size();
    
    NBRE_AxisAlignedBox2<T> clipRect;
    clipRect.minExtend.y = -halfW;
    clipRect.maxExtend.y = halfW;
    clipRect.minExtend.x = 0;
    T baseOffset = 0;

    for (uint32 i = 0; i < vc; ++i)
    {
        const Polyline2Segment& seg = mSegments[i];
        const NBRE_Vector2<T>& xAxis = seg.xAxis;
        const NBRE_Vector2<T>& yAxis = seg.yAxis;
        T xLength = mSegmentLengths[i];

        // clip rect
        clipRect.maxExtend.x = xLength;
        NBRE_Vector2<T> cp00(clipRect.minExtend.x, clipRect.minExtend.y);
        NBRE_Vector2<T> cp10(clipRect.maxExtend.x, clipRect.minExtend.y);
        NBRE_Vector2<T> cp11(clipRect.maxExtend.x, clipRect.maxExtend.y);
        NBRE_Vector2<T> cp01(clipRect.minExtend.x, clipRect.maxExtend.y);

        
        for (uint32 j = 0; j < vc2; ++j)
        {
            const Polyline2Segment& seg2 = pl.mSegments[j];

            if (seg.aabb.minExtend.x > seg2.aabb.maxExtend.x ||
                seg.aabb.minExtend.y > seg2.aabb.maxExtend.y ||
                seg.aabb.maxExtend.x < seg2.aabb.minExtend.x ||
                seg.aabb.maxExtend.y < seg2.aabb.minExtend.y)
            {
                continue;
            }

            NBRE_Vector2<T> pts[4];
            nsl_memcpy(pts, seg2.vertices, sizeof(seg2.vertices));
            // Convert to local coordinates
            for (uint32 k = 0; k < 4; ++k)
            {
                const NBRE_Vector2<T>& offset = pts[k] - mVertices[i];
                T xTrans = offset.DotProduct(xAxis);
                T yTrans = offset.DotProduct(yAxis);
                pts[k].x = xTrans;
                pts[k].y = yTrans;
            }

            T minX = NBRE_Math::Infinity;
            T maxX = -NBRE_Math::Infinity;

            if (IsPointInQuad(cp00, pts)
                || IsPointInQuad(cp01, pts))
            {
                minX = 0;
            }
            if (IsPointInQuad(cp10, pts)
                || IsPointInQuad(cp11, pts))
            {
                maxX = xLength;
            }

            for (uint32 k = 0; k < 4; ++k)
            {
                // Clipping and project to x axis
                NBRE_Vector2<T> segBeg = pts[k];
                NBRE_Vector2<T> segEnd = pts[(k + 1) % 4];
                if (LineClipping(clipRect, segBeg, segEnd))
                {
                    T x0 = segBeg.x;
                    T x1 = segEnd.x;
                    if (x0 < minX)
                    {
                        minX = x0;
                    }
                    if (x0 > maxX)
                    {
                        maxX = x0;
                    }
                    if (x1 < minX)
                    {
                        minX = x1;
                    }
                    if (x1 > maxX)
                    {
                        maxX = x1;
                    }
                }
            }
            if (minX < 0)
            {
                minX = 0;
            }
            if (maxX > xLength)
            {
                maxX = xLength;
            }

            if (minX <= maxX)
            {
                // Found collision interval
                T iBegin = baseOffset + minX;
                T iEnd = baseOffset + maxX;
                intervals.push_back(NBRE_Vector2<T>(iBegin, iEnd));
            }
        }
        baseOffset += xLength;
    }

    if (intervals.size() == 0)
    {
        // no collision found, return whole polyline
        result.push_back(NBRE_Polyline2<T>(*this));
        return;
    }

    NBRE_Vector<NBRE_Vector2<T> > collideIntervals;
    if (intervals.size() == 1)
    {
        collideIntervals.push_back(intervals[0]);
    }
    if (intervals.size() > 1)
    {
        // Merge overlapped intervals
        sort(intervals.begin(), intervals.end(), CompareIntervals);
        T iBegin = intervals[0].x;
        T iEnd = intervals[0].y;

        for (uint32 iInterval = 1; iInterval < intervals.size(); ++iInterval)
        {
            T intveral0 = intervals[iInterval].x;
            T interval1 = intervals[iInterval].y;
            if (iEnd < intveral0)
            {
                collideIntervals.push_back(NBRE_Vector2<T>(iBegin, iEnd));
                iBegin = intveral0;
                iEnd = interval1;
                continue;
            }

            if (interval1 > iEnd)
            {
                iEnd = interval1;
            }
        }
        collideIntervals.push_back(NBRE_Vector2<T>(iBegin, iEnd));
    }

    // find no collision intervals
    T iBegin = 0;
    for (uint32 i = 0; i < collideIntervals.size(); ++i)
    {
        if (collideIntervals[i].x > iBegin)
        {
            result.push_back(SubPolyline(iBegin, collideIntervals[i].x));
        }
        iBegin = collideIntervals[i].y;
    }
    if (iBegin < mLength)
    {
        result.push_back(SubPolyline(iBegin, mLength));
    }
}

template<typename T> void
GetIntervalOnAxis(const NBRE_Vector2<T>& axis, const NBRE_Vector2<T>* pts, uint32 ptCount, T& minValue, T& maxValue)
{
    minValue = maxValue = pts[0].DotProduct(axis);
    for (uint32 i = 1; i < ptCount; ++i)
    {
        T proj = pts[i].DotProduct(axis);
        if (proj < minValue)
        {
            minValue = proj;
        }
        else if (proj > maxValue)
        {
            maxValue = proj;
        }
    }
}

template<typename T> nb_boolean
FindSeparationAxis(const NBRE_Vector2<T>& axis, T min1, T max1, const NBRE_Vector2<T>* p2, uint32 count2)
{
    T min2;
    T max2;
    GetIntervalOnAxis(axis, p2, count2, min2, max2);

    if (min1 > max2 || max1 < min2)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

template<typename T> nb_boolean
FindSeparationAxis(const NBRE_Vector2<T>& axis, const NBRE_Vector2<T>* p1, uint32 count1, const NBRE_Vector2<T>* p2, uint32 count2)
{
    T min1;
    T max1;
    GetIntervalOnAxis(axis, p1, count1, min1, max1);

    T min2;
    T max2;
    GetIntervalOnAxis(axis, p2, count2, min2, max2);

    if (min1 > max2 || max1 < min2)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

template <typename T> nb_boolean
NBRE_Polyline2<T>::WideLineHitTest(const NBRE_AxisAlignedBox2<T>& aabb)
{
    if (mSegments.size() == 0)
    {
        UpdateSegments();
    }

    uint32 vc = mSegments.size();
    NBRE_Vector2<T> aabbVs[4];
    aabbVs[0].x = aabb.minExtend.x; aabbVs[0].y = aabb.minExtend.y;
    aabbVs[1].x = aabb.maxExtend.x; aabbVs[1].y = aabb.minExtend.y;
    aabbVs[2].x = aabb.maxExtend.x; aabbVs[2].y = aabb.maxExtend.y;
    aabbVs[3].x = aabb.minExtend.x; aabbVs[3].y = aabb.maxExtend.y;

    for (uint32 i = 0; i < vc; ++i)
    {
        const Polyline2Segment& seg = mSegments[i];

        if (seg.aabb.minExtend.x > aabb.maxExtend.x ||
            seg.aabb.minExtend.y > aabb.maxExtend.y ||
            seg.aabb.maxExtend.x < aabb.minExtend.x ||
            seg.aabb.maxExtend.y < aabb.minExtend.y)
        {
            continue;
        }

        if (FindSeparationAxis(seg.xAxis, seg.xIntervals[0], seg.xIntervals[1], aabbVs, 4))
        {
            continue;
        }
        if (FindSeparationAxis(seg.yAxis, seg.yIntervals[0], seg.yIntervals[1], aabbVs, 4))
        {
            continue;
        }

        return TRUE;
    }

    return FALSE;
}

template <typename T> nb_boolean
NBRE_Polyline2<T>::WideLineHitTest(NBRE_Polyline2<T>& pl)
{
    if (mSegments.size() == 0)
    {
        UpdateSegments();
    }
    if (pl.mSegments.size() == 0)
    {
        pl.UpdateSegments();
    }

    uint32 vc = mSegments.size();
    uint32 plVc = pl.mSegments.size();

    for (uint32 i = 0; i < vc; ++i)
    {
        const Polyline2Segment& seg = mSegments[i];
        for (uint32 j = 0; j < plVc; ++j)
        {
            const Polyline2Segment& seg2 = pl.mSegments[j];

            if (seg.aabb.minExtend.x > seg2.aabb.maxExtend.x ||
                seg.aabb.minExtend.y > seg2.aabb.maxExtend.y ||
                seg.aabb.maxExtend.x < seg2.aabb.minExtend.x ||
                seg.aabb.maxExtend.y < seg2.aabb.minExtend.y)
            {
                continue;
            }

            if (FindSeparationAxis(seg.xAxis, seg.xIntervals[0], seg.xIntervals[1], seg2.vertices, 4))
            {
                continue;
            }
            if (FindSeparationAxis(seg.yAxis, seg.yIntervals[0], seg.yIntervals[1], seg2.vertices, 4))
            {
                continue;
            }
            if (FindSeparationAxis(seg2.xAxis, seg2.xIntervals[0], seg2.xIntervals[1], seg.vertices, 4))
            {
                continue;
            }
            if (FindSeparationAxis(seg2.yAxis, seg2.yIntervals[0], seg2.yIntervals[1], seg.vertices, 4))
            {
                continue;
            }

            return TRUE;
        }
    }

    return FALSE;
}

template <typename T> void
NBRE_Polyline2<T>::UpdateSegments()
{
    Polyline2Segment segment;
    T halfW = mWidth / 2;
    uint32 vc = mVertices.size();
    for (uint32 i = 1; i < vc; ++i)
    {
        const NBRE_Vector2<T>& p1 = mVertices[i - 1];
        const NBRE_Vector2<T>& p2 = mVertices[i];
        segment.xAxis = p2 - p1;
        segment.xAxis.Normalise();
        segment.yAxis = segment.xAxis.Perpendicular();
        NBRE_Vector2<T> dy = segment.yAxis;
        dy *= halfW;

        NBRE_Vector2<T>* pts = segment.vertices;
        pts[0] = p1 - dy;
        pts[1] = p2 - dy;
        pts[2] = p2 + dy;
        pts[3] = p1 + dy;

        segment.aabb.SetNull();
        segment.aabb.Merge(pts[0]);
        segment.aabb.Merge(pts[1]);
        segment.aabb.Merge(pts[2]);
        segment.aabb.Merge(pts[3]);

        GetIntervalOnAxis(segment.xAxis, pts, 4, segment.xIntervals[0], segment.xIntervals[1]);
        GetIntervalOnAxis(segment.yAxis, pts, 4, segment.yIntervals[0], segment.yIntervals[1]);

        mSegments.push_back(segment);
    }
}

template <typename T> T
NBRE_Polyline2<T>::GetWidth() const
{
    return mWidth;
}

template <typename T> void
NBRE_Polyline2<T>::SetWidth(T value)
{
    if (mWidth != value && mSegments.size() > 0)
    {
        mSegments.clear();
    }
    mWidth = value;
}

template <typename T> void
NBRE_Polyline2<T>::Smooth(T radius)
{
    if (mVertices.size() <= 2)
    {
        return;
    }
    UpdateLength();

    Point2List vs;
    T d = radius * (T)2;
    vs.push_back(mVertices[0]);
    for (size_t i = 1; i < mVertices.size(); ++i)
    {
        T segLength = SegmentLength(i - 1);
        int divide = (int)ceil((double)(segLength / d));
        if (divide >= 2)
        {
            for (int j = 0; j < divide - 1; ++j)
            {
                vs.push_back(NBRE_LinearInterpolate<T>::Lerp(mVertices[i - 1], mVertices[i], (j + 1) / (T)divide));
            }
        }
        vs.push_back(mVertices[i]);
    }
    mVertices.clear();
    mVertices.push_back(vs[0]);
    for (size_t i = 1; i < vs.size() - 1; ++i)
    {
        mVertices.push_back((vs[i - 1] + vs[i] + vs[i + 1]) / T(3));
    }
    mVertices.push_back(vs.back());
    
    UpdateLength();
}
