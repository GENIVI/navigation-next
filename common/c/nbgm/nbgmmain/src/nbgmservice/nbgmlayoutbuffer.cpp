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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmlayoutbuffer.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreclipping.h"

class CheckLineCallback:
    public NBGM_ISetPixelCallback
{
public:
    CheckLineCallback(uint32* buffer, NBRE_Vector2i cellCount, uint32 mask, int32 lineWidth):mBuffer(buffer), mCellCount(cellCount), mMask(mask), mLineWidth(lineWidth), mIsEmpty(TRUE)
    {
    }
public:
    virtual nb_boolean OnSetPixel(int32 x, int32 y, nb_boolean IsXDominant)
    {
        int32 radius = mLineWidth / 2;
        nb_boolean hit = FALSE;

        for (int32 r = 0; r <= radius; ++r)
        {
            if (IsXDominant)
            {
                if (!IsPixelEmpty(x, y + r))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x, y - r))
                {
                    hit = TRUE;
                    break;
                }
            }
            else
            {
                if (!IsPixelEmpty(x + r, y))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x - r, y))
                {
                    hit = TRUE;
                    break;
                }
            }
        }

        if (hit)
        {
            mIsEmpty = FALSE;
            return FALSE;
        }

        return TRUE;
    }

    nb_boolean IsPixelEmpty(int32 x, int32 y)
    {
        if (x >= 0 && x < mCellCount.x && y >= 0 && y < mCellCount.y)
        {
            if (mBuffer[mCellCount.x * y + x] & mMask)
            {
                return FALSE;
            }
        }
        return TRUE;
    }

    nb_boolean IsLineRegionEmpty()
    {
        return mIsEmpty;
    }

private:
    uint32* mBuffer;
    NBRE_Vector2i mCellCount;
    uint32 mMask;
    int32 mLineWidth;
    nb_boolean mIsEmpty;
};

class UpdateLineCallback:
    public NBGM_ISetPixelCallback
{
public:
    UpdateLineCallback(uint32* buffer, NBRE_Vector2i cellCount, uint32 mask, int32 lineWidth):mBuffer(buffer), mCellCount(cellCount), mMask(mask), mLineWidth(lineWidth)
    {
    }
public:
    virtual nb_boolean OnSetPixel(int32 x, int32 y, nb_boolean IsXDominant)
    {
        int32 radius = mLineWidth / 2;
        for (int32 r = 0; r <= radius; ++r)
        {
            if (IsXDominant)
            {
                UpdatePixel(x, y + r);

                if (r > 0)
                {
                    UpdatePixel(x, y - r);
                }
            }
            else
            {
                UpdatePixel(x + r, y);

                if (r > 0)
                {
                    UpdatePixel(x - r, y);
                }
            }
        }
        return TRUE;
    }

    void UpdatePixel(int32 x, int32 y)
    {
        if (x >= 0 && x < mCellCount.x && y >= 0 && y < mCellCount.y)
        {
            mBuffer[mCellCount.x * y + x] |= mMask;
        }
    }

private:
    uint32* mBuffer;
    NBRE_Vector2i mCellCount;
    uint32 mMask;
    int32 mLineWidth;
};

class SplitLineCallback:
    public NBGM_ISetPixelCallback
{
public:
    typedef NBRE_Vector<int32> StepList;
    struct SubSegment
    {
    public:
        SubSegment(int32 beginStep, int32 endStep)
            :beginStep(beginStep)
            ,endStep(endStep)
        {
        }

    public:
        int32 beginStep;
        int32 endStep;
    };
    typedef NBRE_Vector<SubSegment> SubSegmentList;

public:
    SplitLineCallback(uint32* buffer, NBRE_Vector2i cellCount, uint32 mask, int32 lineWidth)
        :mBuffer(buffer), mCellCount(cellCount), mMask(mask), mLineWidth(lineWidth), mTotalSteps(0)
    {
    }

public:
    virtual nb_boolean OnSetPixel(int32 x, int32 y, nb_boolean IsXDominant)
    {
        int32 radius = mLineWidth / 2;
        nb_boolean hit = FALSE;

        for (int32 r = 0; r <= radius; ++r)
        {   
            if (IsXDominant)
            {
                if (!IsPixelEmpty(x, y + r))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x, y - r))
                {
                    hit = TRUE;
                    break;
                }
            }
            else
            {
                if (!IsPixelEmpty(x + r, y))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x - r, y))
                {
                    hit = TRUE;
                    break;
                }
            }
        }

        if (hit)
        {
            if (mSegment.size() > 2)
            {
                mSegments.push_back(SubSegment(mSegment[0], mSegment[mSegment.size() - 1]));
            }
            mSegment.clear();
        }
        else
        {
            mSegment.push_back(mTotalSteps);
        }

        ++mTotalSteps;
        return TRUE;
    }

    nb_boolean IsPixelEmpty(int32 x, int32 y)
    {
        if (x >= 0 && x < mCellCount.x && y >= 0 && y < mCellCount.y)
        {
            if (mBuffer[mCellCount.x * y + x] & mMask)
            {
                mBuffer[mCellCount.x * y + x] |= 0x1000;
                return FALSE;
            }

            
        }
        return TRUE;
    }

    int32 GetTotalSteps()
    {
        return mTotalSteps;
    }

    const SubSegmentList& GetSegments()
    {
        // Add last segment
        if (mSegment.size() > 2)
        {
            mSegments.push_back(SubSegment(mSegment[0], mSegment[mSegment.size() - 1]));
        }
        mSegment.clear();
        return mSegments;
    }

private:
    uint32* mBuffer;
    NBRE_Vector2i mCellCount;
    uint32 mMask;
    int32 mLineWidth;
    SubSegmentList mSegments;
    StepList mSegment;
    int32 mTotalSteps;
};


class StaticSplitLineCallback:
    public NBGM_ISetPixelCallback
{
public:
    typedef NBRE_Vector<NBRE_Vector2i> StepList;
    struct SubSegment
    {
    public:
        SubSegment(const NBRE_Vector2i& beginStep, const NBRE_Vector2i& endStep)
            :beginStep(beginStep)
            ,endStep(endStep)
        {
        }

    public:
        NBRE_Vector2i beginStep;
        NBRE_Vector2i endStep;
    };
    typedef NBRE_Vector<SubSegment> SubSegmentList;

public:
    StaticSplitLineCallback(uint32* buffer, NBRE_Vector2i cellCount, uint32 mask, int32 lineWidth)
        :mBuffer(buffer), mCellCount(cellCount), mMask(mask), mLineWidth(lineWidth), mTotalSteps(0)
    {
    }

public:
    virtual nb_boolean OnSetPixel(int32 x, int32 y, nb_boolean IsXDominant)
    {
        int32 radius = mLineWidth / 2;
        nb_boolean hit = FALSE;

        for (int32 r = 0; r <= radius; ++r)
        {   
            if (IsXDominant)
            {
                if (!IsPixelEmpty(x, y + r))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x, y - r))
                {
                    hit = TRUE;
                    break;
                }
            }
            else
            {
                if (!IsPixelEmpty(x + r, y))
                {
                    hit = TRUE;
                    break;
                }

                if (r > 0 && !IsPixelEmpty(x - r, y))
                {
                    hit = TRUE;
                    break;
                }
            }
        }

        if (hit)
        {
            if (mSegment.size() > 0)
            {
                mSegments.push_back(SubSegment(mSegment[0], mSegment[mSegment.size() - 1]));
            }
            mSegment.clear();
        }
        else
        {
            mSegment.push_back(NBRE_Vector2i(x, y));
        }

        ++mTotalSteps;
        return TRUE;
    }

    nb_boolean IsPixelEmpty(int32 x, int32 y)
    {
        if (x >= 0 && x < mCellCount.x && y >= 0 && y < mCellCount.y)
        {
            if (mBuffer[mCellCount.x * y + x] & mMask)
            {
                return FALSE;
            }


        }
        return TRUE;
    }

    int32 GetTotalSteps()
    {
        return mTotalSteps;
    }

    const SubSegmentList& GetSegments()
    {
        // Add last segment
        if (mSegment.size() > 0)
        {
            mSegments.push_back(SubSegment(mSegment[0], mSegment[mSegment.size() - 1]));
        }
        mSegment.clear();
        return mSegments;
    }

private:
    uint32* mBuffer;
    NBRE_Vector2i mCellCount;
    uint32 mMask;
    int32 mLineWidth;
    SubSegmentList mSegments;
    StepList mSegment;
    int32 mTotalSteps;
};


NBGM_LayoutBuffer::NBGM_LayoutBuffer(NBGM_Context& nbgmContext, float width, float height, float cellSize)
    :mNBGMContext(nbgmContext)
    ,mBuffer(NULL)
    ,mSize(width, height)
    ,mCellCount(int32(width / cellSize), int32(height / cellSize))
    ,mCellSize(cellSize)
{
    nbre_assert(cellSize != 0);

    if (mCellCount.x > 0 && mCellCount.y > 0)
    {
        mBuffer = NBRE_NEW uint32[mCellCount.x * mCellCount.y];
        nsl_memset(mBuffer, 0, mCellCount.x * mCellCount.y * sizeof(uint32));
    }
}

NBGM_LayoutBuffer::~NBGM_LayoutBuffer()
{
    NBRE_DELETE_ARRAY mBuffer;
}

void
NBGM_LayoutBuffer::Clear()
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return;
    }
    nsl_memset(mBuffer, 0, mCellCount.x * mCellCount.y * sizeof(uint32));
}

void
NBGM_LayoutBuffer::Clear(uint32 mask)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return;
    }

    uint32 n = mCellCount.x * mCellCount.y;
    uint32 v = ~mask;
    for (uint32 i = 0; i < n; ++i)
    {
        mBuffer[i] &= v;
    }
}

void
NBGM_LayoutBuffer::Resize(float width, float height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    int32 xCount = (int32)(width / mCellSize);
    int32 yCount = (int32)(height / mCellSize);
    if (xCount < 1)
    {
        xCount = 1;
    }
    if (yCount < 1)
    {
        yCount = 1;
    }

    if (xCount != mCellCount.x || yCount != mCellCount.y)
    {
        NBRE_DELETE_ARRAY mBuffer;
        mCellCount.x = xCount;
        mCellCount.y = yCount;
        mSize.x = width;
        mSize.y = height;
        mBuffer = NBRE_NEW uint32[mCellCount.x * mCellCount.y];
    }
    Clear();
}

nb_boolean
NBGM_LayoutBuffer::IsRegionAvailable(const NBRE_AxisAlignedBox2d& rect, uint32 value)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return FALSE;
    }

    if (rect.IsNull())
    {
        return FALSE;
    }

    NBRE_AxisAlignedBox2i elementRect;
    if (!TransformScreenRectToBufferRect(rect, elementRect))
    {
        return FALSE;
    }

    if (!ClipRect(elementRect))
    {
        return FALSE;
    }

    for (int32 y = elementRect.minExtend.y; y <= elementRect.maxExtend.y; ++y)
    {
        for (int32 x = elementRect.minExtend.x; x <= elementRect.maxExtend.x; ++x)
        {
            if (mBuffer[y * mCellCount.x + x] & value)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void
NBGM_LayoutBuffer::UpdateRegion(const NBRE_AxisAlignedBox2d& rect, uint32 value)
{
    UpdateRegion(rect, value, 0);
}

void
NBGM_LayoutBuffer::UpdateRegion(const NBRE_AxisAlignedBox2d& rect, uint32 value, int32 expand)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return;
    }

    if (rect.IsNull())
    {
        return;
    }

    NBRE_AxisAlignedBox2i elementRect;
    if (!TransformScreenRectToBufferRect(rect, elementRect))
    {
        return;
    }

    if (expand > 0)
    {
        if (!elementRect.IsNull())
        {
            --elementRect.minExtend.x;
            --elementRect.minExtend.y;
            ++elementRect.maxExtend.x;
            ++elementRect.maxExtend.y;
        }
    }
    else if (expand < 0)
    {
        if (!elementRect.IsNull())
        {
            if (elementRect.maxExtend.x + elementRect.minExtend.x > 2)
            {
                ++elementRect.minExtend.x;
                --elementRect.maxExtend.x;
            }

            if (elementRect.maxExtend.y + elementRect.minExtend.y > 2)
            {
                ++elementRect.minExtend.y;
                --elementRect.maxExtend.y;
            }
        }
    }

    if (!ClipRect(elementRect))
    {
        return;
    }

    for (int32 y = elementRect.minExtend.y; y <= elementRect.maxExtend.y; ++y)
    {
        for (int32 x = elementRect.minExtend.x; x <= elementRect.maxExtend.x; ++x)
        {
            mBuffer[y * mCellCount.x + x] |= value;
        }
    }
}

nb_boolean
NBGM_LayoutBuffer::IsRegionAvailable(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 value)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return FALSE;
    }

    nbre_assert(polyline.VertexCount() >= 2);
    NBGM_Point2iList transPolyline = TransformPolyline(polyline);
    if (transPolyline.size() < 2)
    {
        return FALSE;
    }
    
    NBRE_AxisAlignedBox2i rc;
    NBRE_Vector2i half(lineWidth / 2, lineWidth / 2);

    for (uint32 i = 0; i < transPolyline.size() - 1; ++i)
    {
        NBRE_Vector2i p0 = transPolyline[i];
        NBRE_Vector2i p1 = transPolyline[i + 1];
        NBRE_LineClipResult result = NBRE_Clippingi::ClipLineByRect(NBRE_AxisAlignedBox2i(0, 0, mCellCount.x - 1, mCellCount.y - 1), p0, p1);
        if (result != NBRE_LCR_OUTSIDE)
        {   
            rc.minExtend = p0 - half;
            rc.maxExtend = p0 + half;
            if (!CheckRect(rc, value))
            {
                return FALSE;
            }
            rc.minExtend = p1 - half;
            rc.maxExtend = p1 + half;
            if (!CheckRect(rc, value))
            {
                return FALSE;
            }

            CheckLineCallback checkLine(mBuffer, mCellCount, value, lineWidth);
            NBGM_Rasterize::DrawLine(p0.x, p0.y, p1.x, p1.y, &checkLine);
            if (!checkLine.IsLineRegionEmpty())
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void
NBGM_LayoutBuffer::UpdateRegion(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 value)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return;
    }

    nbre_assert(polyline.VertexCount() >= 2);
    NBGM_Point2iList transPolyline = TransformPolyline(polyline);
    if (transPolyline.size() < 2)
    {
        return;
    }
    UpdateLineCallback updateLine(mBuffer, mCellCount, value, lineWidth);
    NBGM_Rasterize::DrawPolyline(transPolyline, &updateLine);

    NBRE_AxisAlignedBox2i rc;
    int32 halfS = lineWidth / 2;
    NBRE_Vector2i half(halfS, halfS);
    for (uint32 i = 0; i < transPolyline.size(); ++i)
    {
        const NBRE_Vector2i& pt = transPolyline[i];
        rc.minExtend = pt - half;
        rc.maxExtend = pt + half;
        UpdateRect(rc, value);
    }
}

void
NBGM_LayoutBuffer::UpdateRegion(const NBRE_Vector2d& p0, const NBRE_Vector2d& p1, int32 lineWidth, uint32 mask)
{
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return;
    }

    float dx = static_cast<float>(mCellCount.x) / mSize.x;
    float dy = static_cast<float>(mCellCount.y) / mSize.y;
    NBRE_Vector2i pb0(static_cast<int32>(p0.x * dx), static_cast<int32>(p0.y * dy));
    NBRE_Vector2i pb1(static_cast<int32>(p1.x * dx), static_cast<int32>(p1.y * dy));

    UpdateLineCallback updateLine(mBuffer, mCellCount, mask, lineWidth);
    NBGM_Rasterize::DrawLine(pb0.x, pb0.y, pb1.x, pb1.y, &updateLine);

    NBRE_AxisAlignedBox2i rc;
    int32 halfS = lineWidth / 2;
    
    NBRE_Vector2i half(halfS, halfS);
    rc.minExtend = pb0 - half;
    rc.maxExtend = pb0 + half;
    UpdateRect(rc, mask);
    rc.minExtend = pb1 - half;
    rc.maxExtend = pb1 + half;
    UpdateRect(rc, mask);
}

NBGM_LayoutPolylineList
NBGM_LayoutBuffer::GetRegionAvailableParts(const NBGM_LayoutPolyline& layoutPolyline, int32 lineWidth, uint32 mask)
{
    NBGM_LayoutPolylineList result;
    if (mCellCount.x == 0 || mCellCount.y == 0)
    {
        return result;
    }

    NBRE_Polyline2d polyline = layoutPolyline.GetScreenPolyline();
    nbre_assert(polyline.VertexCount() >= 2);

    NBGM_Point2iList transPolyline = TransformPolyline(polyline);
    if (transPolyline.size() < 2)
    {
        return result;
    }

    SplitLineCallback splitLine(mBuffer, mCellCount, mask, lineWidth);
    NBGM_Rasterize::DrawPolyline(transPolyline, &splitLine);
    int32 steps = splitLine.GetTotalSteps();
    SplitLineCallback::SubSegmentList segments = splitLine.GetSegments();
    double length = polyline.Length();
    for (uint32 segIndex = 0; segIndex < segments.size(); ++segIndex)
    {
        const SplitLineCallback::SubSegment& seg = segments[segIndex];
        NBGM_LayoutPolylinePosition beginPos = layoutPolyline.ConvertOffsetToParameterCoordinate(seg.beginStep * length / steps);
        NBGM_LayoutPolylinePosition endPos = layoutPolyline.ConvertOffsetToParameterCoordinate((seg.endStep + 1) * length / steps);
        NBGM_LayoutVertexList pts;

        pts.push_back(layoutPolyline.PointAt(beginPos));
        for (uint32 i = beginPos.segmentIndex + 1; i <= endPos.segmentIndex; ++i)
        {
            pts.push_back(layoutPolyline.Vertex(i));
        }
        pts.push_back(layoutPolyline.PointAt(endPos));
        if (pts.size() >= 2)
        {
            result.push_back(NBGM_LayoutPolyline(&mNBGMContext, pts));
        }
    }
    return result;
}

static inline uint32 CheckCell(uint32* buffer, NBRE_Vector2i mCellCount, int32 x, int32 y, uint32 mask)
{
    return buffer[y * mCellCount.x + x] & mask;
}

nb_boolean
NBGM_LayoutBuffer::GetAvailablePolylineParts(const NBRE_Polyline2d& polyline, int32 lineWidth, uint32 mask, NBRE_Vector<NBRE_Polyline2d>& result)
{
    if (mCellCount.x == 0 || mCellCount.y == 0 || polyline.VertexCount() < 2)
    {
        return FALSE;
    }

    NBGM_Point2iList transPolyline = TransformPolyline(polyline);
    if (transPolyline.size() < 2)
    {
        return FALSE;
    }

    result.clear();
    //int32 radius = lineWidth / 2;

    NBRE_Vector<NBRE_Vector2i> rasterPts;
    NBRE_Vector<nb_boolean> rasterFree;
    NBRE_Vector<NBRE_Vector2d> polylinePts;

    for (uint32 vIdx = 1; vIdx < transPolyline.size(); ++vIdx)
    {
        const NBRE_Vector2i& p0 = transPolyline[vIdx - 1];
        const NBRE_Vector2i& p1 = transPolyline[vIdx];

        rasterPts.clear();

        NBGM_Rasterize::DrawLine(p0.x, p0.y, p1.x, p1.y, mCellCount.x, mCellCount.y, rasterPts);
        rasterFree.resize(rasterPts.size());

        for (uint32 ri = 0; ri < rasterPts.size(); ++ri)
        {
            const NBRE_Vector2i& rasterPoint = rasterPts[ri];
            if ( CheckCell(mBuffer, mCellCount, rasterPoint.x, rasterPoint.y, mask)
                || CheckCell(mBuffer, mCellCount, rasterPoint.x - 1, rasterPoint.y, mask)
                || CheckCell(mBuffer, mCellCount, rasterPoint.x + 1, rasterPoint.y, mask)
                || CheckCell(mBuffer, mCellCount, rasterPoint.x, rasterPoint.y - 1, mask)
                || CheckCell(mBuffer, mCellCount, rasterPoint.x, rasterPoint.y + 1, mask)
                )
            {
                // conflict found
                rasterFree[ri] = FALSE;
            }
            else
            {
                rasterFree[ri] = TRUE;
            }
        }

        for (uint32 ri = 0; ri < rasterPts.size(); ++ri)
        {
            uint32 idxBegin = ri;
            uint32 idxEnd = ri;
            while(ri + 1 < rasterPts.size() && rasterFree[++ri])
            {
                ++idxEnd;
            }

            NBRE_Polyline2Positiond pos;
            NBRE_Vector2d pt = polyline.Vertex(vIdx - 1);
            if (idxBegin != 0)
            {
                if (polylinePts.size() >= 2)
                {
                    NBRE_Polyline2d subpl(polylinePts);
                    result.push_back(subpl);
                }
                polylinePts.clear();

                pt = NBRE_LinearInterpolated::Lerp(polyline.Vertex(vIdx - 1), polyline.Vertex(vIdx), (double)(idxBegin) / rasterPts.size());
                polylinePts.push_back(pt);
            }
            else
            {
                if (polylinePts.size() == 0 || polylinePts.back() != pt)
                {
                    polylinePts.push_back(pt);
                }
            }

            pt = polyline.Vertex(vIdx);
            if (idxEnd != rasterPts.size() - 1)
            {
                pt = NBRE_LinearInterpolated::Lerp(polyline.Vertex(vIdx - 1), polyline.Vertex(vIdx), (double)(idxEnd + 1) / rasterPts.size());
                polylinePts.push_back(pt);

                if (polylinePts.size() >= 2)
                {
                    NBRE_Polyline2d subpl(polylinePts);
                    result.push_back(subpl);
                }
                polylinePts.clear();
            }
            else
            {
                if (polylinePts.size() == 0 || polylinePts.back() != pt)
                {
                    polylinePts.push_back(pt);
                }
            }
        }
    }

    if (polylinePts.size() >= 2)
    {
        NBRE_Polyline2d subpl(polylinePts);
        result.push_back(subpl);
    }
    polylinePts.clear();

    double rectRadius = mCellSize * lineWidth * 0.5;
    NBRE_Vector2d halfSize(rectRadius, rectRadius);
    for (int32 i = 0; i < (int32)result.size(); ++i)
    {
        const NBRE_Polyline2d& pl = result[i];
        double beginOffset = 0;
        double endOffset = pl.Length();
        
        NBRE_AxisAlignedBox2d rect(pl.Vertex(0) - halfSize, pl.Vertex(0) + halfSize);
        if (!IsRegionAvailable(rect, mask))
        {
            beginOffset = rectRadius;
        }

        rect.minExtend = pl.Vertex(pl.VertexCount() - 1) - halfSize;
        rect.maxExtend = pl.Vertex(pl.VertexCount() - 1) + halfSize;
        if (!IsRegionAvailable(rect, mask))
        {
            endOffset = pl.Length() - rectRadius;
        }

        if (beginOffset >= endOffset)
        {
            result.erase(result.begin() + i);
            --i;
        }
        else
        {
            if (beginOffset != 0 || endOffset != pl.Length())
            {
                result[i] = pl.SubPolyline(beginOffset, endOffset);
            }
        }
    }
    
    return result.size() > 0 ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutBuffer::TransformScreenRectToBufferRect(const NBRE_AxisAlignedBox2d& rect, NBRE_AxisAlignedBox2i& result) const 
{
    if (rect.maxExtend.x < 0 ||
        rect.maxExtend.y < 0 ||
        rect.minExtend.x > mCellSize * mCellCount.x ||
        rect.minExtend.y > mCellSize * mCellCount.y)
    {
        return FALSE;
    }

    float dx = static_cast<float>(mCellCount.x) / mSize.x;
    float dy = static_cast<float>(mCellCount.y) / mSize.y;
    result.minExtend.x = static_cast<int32>(rect.minExtend.x * dx);
    result.minExtend.y = static_cast<int32>(rect.minExtend.y * dy);
    result.maxExtend.x = static_cast<int32>(rect.maxExtend.x * dx);
    result.maxExtend.y = static_cast<int32>(rect.maxExtend.y * dy);
    return TRUE;
}

NBGM_Point2iList
NBGM_LayoutBuffer::TransformPolyline(const NBRE_Polyline2d& polyline)
{
    double dx = static_cast<double>(mCellCount.x) / mSize.x;
    double dy = static_cast<double>(mCellCount.y) / mSize.y;
    uint32 vertexCount = polyline.VertexCount();
    NBRE_Vector<NBRE_Vector2i> transVertices;

    for (uint32 i = 0; i < vertexCount; ++i)
    {
        if (polyline.Vertex(i).IsNaN())
        {
            continue;
        }
        NBRE_Vector2i v(
            static_cast<int32>((polyline.Vertex(i).x) * dx), 
            static_cast<int32>((polyline.Vertex(i).y) * dy));

        transVertices.push_back(v);
    }

    return transVertices;
}

NBRE_Vector2d
NBGM_LayoutBuffer::BufferToWorld(const NBRE_Vector2i& position)
{
    return NBRE_Vector2d(position.x * mSize.x / mCellCount.x, position.y * mSize.y / mCellCount.y);
}

nb_boolean
NBGM_LayoutBuffer::ClipRect(NBRE_AxisAlignedBox2i& elementRect)
{
    if (elementRect.minExtend.x >= mCellCount.x ||
        elementRect.minExtend.y >= mCellCount.y ||
        elementRect.maxExtend.x < 0 ||
        elementRect.maxExtend.y < 0
        )
    {
        return FALSE;
    }

    if (elementRect.minExtend.x < 0)
    {
        elementRect.minExtend.x = 0;
    }
    if (elementRect.minExtend.y < 0)
    {
        elementRect.minExtend.y = 0;
    }
    if (elementRect.maxExtend.x >= mCellCount.x)
    {
        elementRect.maxExtend.x = mCellCount.x - 1;
    }
    if (elementRect.maxExtend.y >= mCellCount.y)
    {
        elementRect.maxExtend.y = mCellCount.y - 1;
    }
    return TRUE;
}

nb_boolean
NBGM_LayoutBuffer::CheckRect(const NBRE_AxisAlignedBox2i& rect, uint32 mask)
{
    NBRE_AxisAlignedBox2i elementRect = rect;
    if (!ClipRect(elementRect))
    {
        return TRUE;
    }

    for (int32 y = elementRect.minExtend.y; y <= elementRect.maxExtend.y; ++y)
    {
        for (int32 x = elementRect.minExtend.x; x <= elementRect.maxExtend.x; ++x)
        {
            if (mBuffer[y * mCellCount.x + x] & mask)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void
NBGM_LayoutBuffer::UpdateRect(const NBRE_AxisAlignedBox2i& rect, uint32 mask)
{
    NBRE_AxisAlignedBox2i elementRect = rect;
    if (!ClipRect(elementRect))
    {
        return;
    }

    for (int32 y = elementRect.minExtend.y; y <= elementRect.maxExtend.y; ++y)
    {
        for (int32 x = elementRect.minExtend.x; x <= elementRect.maxExtend.x; ++x)
        {
            mBuffer[y * mCellCount.x + x] |= mask;
        }
    }
}
