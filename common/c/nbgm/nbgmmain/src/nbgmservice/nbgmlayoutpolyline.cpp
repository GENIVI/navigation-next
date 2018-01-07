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
#include "nbgmlayoutpolyline.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreclipping.h"
#include "nbgmlayoutmanager.h"
#include "nbretransformutil.h"

NBGM_LayoutVertex::NBGM_LayoutVertex(): mW(0.0), mWidth(0.0)
{
}

NBGM_LayoutVertex::NBGM_LayoutVertex(NBRE_Vector3d /*worldPosition*/): mW(0.0), mWidth(0.0)
{
}

NBGM_LayoutVertex::~NBGM_LayoutVertex()
{

}

NBGM_LayoutVertex
NBGM_LayoutVertex::Lerp(const NBGM_LayoutVertex& v, double t) const
{
    NBGM_LayoutVertex result;
    result.mScreenPosition = NBRE_LinearInterpolated::Lerp(mScreenPosition, v.mScreenPosition, t);
    double tW = mW * t / (v.mW + (mW - v.mW) * t);
    result.mW = NBRE_LinearInterpolated::Lerp(mW, v.mW, tW);
    result.mWorldPosition = NBRE_LinearInterpolated::Lerp(mWorldPosition, v.mWorldPosition, tW);
    result.mWidth = NBRE_LinearInterpolated::Lerp(mWidth, v.mWidth, tW);
    return result;
}

NBGM_LayoutPolyline::NBGM_LayoutPolyline(NBGM_Context* nbgmContext, uint32 vertexCount)
:mNBGMContext(nbgmContext)
,mScreenPolyline(vertexCount)
,mWorldPolyline(vertexCount)
{
}

NBGM_LayoutPolyline::NBGM_LayoutPolyline(NBGM_Context* nbgmContext, const NBRE_Polyline3d& worldPolyline, double width)
    :mNBGMContext(nbgmContext)
    ,mScreenPolyline(worldPolyline.VertexCount())
    ,mWorldPolyline(worldPolyline)
{
    for (uint32 i = 0; i < worldPolyline.VertexCount(); ++i)
    {
        mWArray.push_back(1.0f);
        mWidthArray.push_back(width);
    }
    UpdateScreenPositionFromWorldPosition();
    mScreenPolyline.UpdateLength();
    mWorldPolyline.UpdateLength();
}

NBGM_LayoutPolyline::NBGM_LayoutPolyline(NBGM_Context* nbgmContext, const NBGM_LayoutVertexList& vertices)
    :mNBGMContext(nbgmContext)
    ,mScreenPolyline(vertices.size())
    ,mWorldPolyline(vertices.size())
{
    for (uint32 i = 0; i < vertices.size(); ++i)
    {
        mScreenPolyline.SetVertex(i, vertices[i].mScreenPosition);
        mWorldPolyline.SetVertex(i, vertices[i].mWorldPosition);
        mWArray.push_back(vertices[i].mW);
        mWidthArray.push_back(vertices[i].mWidth);
    }
    mScreenPolyline.UpdateLength();
    mWorldPolyline.UpdateLength();
}

NBGM_LayoutPolyline::~NBGM_LayoutPolyline()
{
}

void
NBGM_LayoutPolyline::ClipByRect(const NBRE_AxisAlignedBox2d& rect, NBGM_LayoutPolylineList& result)
{
    NBRE_Vector<NBGM_LayoutVertex> pl;
    for (uint32 i = 0; i < mScreenPolyline.VertexCount() - 1; ++i)
    {
        NBRE_Vector2d p0 = mScreenPolyline.Vertex(i);
        NBRE_Vector2d p1 = mScreenPolyline.Vertex(i + 1);
        NBRE_LineClipResult clipResult = NBRE_Clippingd::ClipLineByRect(rect, p0, p1);

        if (clipResult == NBRE_LCR_OUTSIDE)
        {
            continue;
        }

        double segmentLength = mScreenPolyline.SegmentLength(i);

        if (pl.size() == 0)
        {
            double t1 = p0.DistanceTo(mScreenPolyline.Vertex(i)) / segmentLength;
            pl.push_back(Vertex(i).Lerp(Vertex(i + 1), t1));
        }

        double t2 = p1.DistanceTo(mScreenPolyline.Vertex(i)) / segmentLength;
        pl.push_back(Vertex(i).Lerp(Vertex(i + 1), t2));
        if (clipResult == NBRE_LCR_CLIP_END || clipResult == NBRE_LCR_CLIP_BOTH)
        {
            result.push_back(NBGM_LayoutPolyline(mNBGMContext, pl));
            pl.clear();
        }
    }
    // if polyline ended inside
    if (pl.size() > 0)
    {
        nbre_assert(pl.size() >= 2);
        result.push_back(NBGM_LayoutPolyline(mNBGMContext, pl));
        pl.clear();
    }
}

void
NBGM_LayoutPolyline::GetScreenAvaliableParts(int32 lineWidth, uint32 mask, NBGM_LayoutPolylineList& result)
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext->layoutManager);

    NBGM_LayoutPolylineList pls;
    ClipByRect(layoutManager.GetScreenRect(), pls);

    for (uint32 i = 0; i < pls.size(); ++i)
    {
        NBGM_LayoutPolylineList als = layoutManager.GetLayoutBuffer().GetRegionAvailableParts(pls[i], lineWidth, mask);
        for (uint32 j = 0; j < als.size(); ++j)
        {
            result.push_back(als[j]);
        }
    }
}

nb_boolean
NBGM_LayoutPolyline::UpdateScreenPositionFromWorldPosition()
{
    if (!mNBGMContext->transUtil->IsInNearPlaneFrontSide(mWorldPolyline))
    {
        return FALSE;
    }

    for (uint32 i = 0; i < mWorldPolyline.VertexCount(); ++i)
    {
        NBRE_Vector2d sp;
        double w = 0;
        mNBGMContext->transUtil->WorldToScreen(mWorldPolyline.Vertex(i), sp, w);
        mScreenPolyline.SetVertex(i, sp);
        mWArray[i] = w;
    }
    mScreenPolyline.UpdateLength();

    return TRUE;
}

NBGM_LayoutPolylinePosition
NBGM_LayoutPolyline::ConvertOffsetToParameterCoordinate(double offset) const
{
    NBRE_Polyline2Positiond p = mScreenPolyline.ConvertOffsetToParameterCoordinate(offset);
    return NBGM_LayoutPolylinePosition(p.segmentIndex, p.t);
}

double
NBGM_LayoutPolyline::ConvertParameterCoordinateToOffset(NBGM_LayoutPolylinePosition pos) const
{
    return mScreenPolyline.ConvertParameterCoordinateToOffset(NBRE_Polyline2Positiond(pos.segmentIndex, pos.t));
}

NBGM_LayoutVertex
NBGM_LayoutPolyline::Vertex(uint32 index) const
{
    NBGM_LayoutVertex result;
    result.mScreenPosition = mScreenPolyline.Vertex(index);
    result.mWorldPosition = mWorldPolyline.Vertex(index);
    result.mW = mWArray[index];
    result.mWidth = mWidthArray[index];
    return result;
}

void
NBGM_LayoutPolyline::SetVertex(uint32 index, const NBGM_LayoutVertex& value)
{
    mScreenPolyline.SetVertex(index, value.mScreenPosition);
    mWorldPolyline.SetVertex(index, value.mWorldPosition);
    mWArray[index] = value.mW;
    mWidthArray[index] = value.mWidth;
}

NBGM_LayoutVertex
NBGM_LayoutPolyline::PointAt(const NBGM_LayoutPolylinePosition& position) const
{
    return Vertex(position.segmentIndex).Lerp(Vertex(position.segmentIndex + 1), position.t);
}

NBGM_LayoutVertex
NBGM_LayoutPolyline::PointAt(double offset) const
{
    return PointAt(ConvertOffsetToParameterCoordinate(offset));
}

NBGM_LayoutPolyline
NBGM_LayoutPolyline::SubPolyline(double beginOffset, double endOffset) const
{
    if (beginOffset < 0)
    {
        beginOffset = 0;
    }
    if (endOffset > mScreenPolyline.Length())
    {
        endOffset = mScreenPolyline.Length();
    }

    NBGM_LayoutPolylinePosition beginPos = ConvertOffsetToParameterCoordinate(beginOffset);
    NBGM_LayoutPolylinePosition endPos = ConvertOffsetToParameterCoordinate(endOffset);

    NBGM_LayoutVertexList vs;
    vs.push_back(PointAt(beginPos));
    for (uint32 i = beginPos.segmentIndex + 1; i <= endPos.segmentIndex; ++i)
    {
        vs.push_back(Vertex(i));
    }
    vs.push_back(PointAt(endPos));
    return NBGM_LayoutPolyline(mNBGMContext, vs);
}

double
NBGM_LayoutPolyline::DistanceTo(const NBRE_Vector2d& point, NBGM_LayoutPolylinePosition* pClosestPoint) const
{
    NBRE_Polyline2Positiond cp2;
    double dist = mScreenPolyline.DistanceTo(point, &cp2);
    if (pClosestPoint)
    {
        pClosestPoint->segmentIndex = cp2.segmentIndex;
        pClosestPoint->t = cp2.t;
    }
    return dist;
}

NBGM_LayoutPolylinePosition
NBGM_LayoutPolyline::ConvertScreenCoordinateToWorld(NBGM_LayoutPolylinePosition screenPos) const
{
    double w0 = mWArray[screenPos.segmentIndex];
    double w1 = mWArray[screenPos.segmentIndex + 1];
    double t = screenPos.t;
    double worldT = w0 * t / (w1 + (w0 - w1) * t);
    return NBGM_LayoutPolylinePosition(screenPos.segmentIndex, worldT);
}

NBGM_LayoutPolylinePosition
NBGM_LayoutPolyline::ConvertWorldCoordinateToScreen(NBGM_LayoutPolylinePosition worldPos) const
{
    double w0 = mWArray[worldPos.segmentIndex];
    double w1 = mWArray[worldPos.segmentIndex + 1];
    double t = worldPos.t;
    double worldT = w1 * t / (w0 + (w1 - w0) * t);
    return NBGM_LayoutPolylinePosition(worldPos.segmentIndex, worldT);
}

double
NBGM_LayoutPolyline::ConvertWorldOffsetToScreen(double offset) const
{
    NBRE_Polyline3Positiond pos = mWorldPolyline.ConvertOffsetToParameterCoordinate(offset);
    NBGM_LayoutPolylinePosition p(pos.segmentIndex, pos.t);
    p = ConvertWorldCoordinateToScreen(p);
    return mScreenPolyline.ConvertParameterCoordinateToOffset(NBRE_Polyline2Positiond(p.segmentIndex, p.t));
}

double
NBGM_LayoutPolyline::ConvertScreenOffsetToWorld(double offset) const
{
    NBRE_Polyline2Positiond pos = mScreenPolyline.ConvertOffsetToParameterCoordinate(offset);
    NBGM_LayoutPolylinePosition p(pos.segmentIndex, pos.t);
    p = ConvertScreenCoordinateToWorld(p);
    return mWorldPolyline.ConvertParameterCoordinateToOffset(NBRE_Polyline3Positiond(p.segmentIndex, p.t));
}
