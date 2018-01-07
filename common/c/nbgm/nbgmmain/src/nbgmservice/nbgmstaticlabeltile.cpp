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
#include "nbgmstaticlabeltile.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreaxisalignedbox2.h"
#include "nbretypeconvert.h"
#include "nbrerenderengine.h"
#include "nbgmlabeldebugutil.h"
#include "nbgmcommon.h"
#include "nbgmmaproadlabel.h"
#include "nbgmgridentity.h"
#include "nbgmlabeldebugutil.h"
#include "nbretransformutil.h"
#include "palclock.h"
#include "nbgmshield.h"
#include "nbgmlabellayer.h"
#include "nbgmconst.h"
#include "nbreutility.h"
#include "nbgmbuildutility.h"
#include "nbretypeconvert.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrecollisiondetector2d.h"
#include "nbreuniformgridspatialpartition2d.h"
#include "nbreaabbtree2d.h"

#define STATIC_LAYOUT_ROAD_LABEL_MASK 0x1
#define STATIC_LAYOUT_SHIELD_MASK 0x2
#define STATIC_LAYOUT_PATH_ARROW_MASK 0x4

typedef NBRE_Vector<NBGM_StaticLayoutResult*> NBGM_StaticLayoutResultList;

class NBGM_StaticLayoutTask : public NBGM_Task
{
public:
    NBGM_StaticLayoutTask(NBGM_LabelLayer* labelLayer, shared_ptr<NBGM_TaskQueue> renderThread, uint64 id, int32 level, double mercatorPerPixel, double mMaxFontSize, NBGM_Context* mNBGMContext, const NBRE_AxisAlignedBox3d& aabb
        ,const NBGM_StaticLabelTile::RoadTrackDataList& roadTracks, const NBGM_StaticLabelTile::ShieldTrackDataList& shieldTracks, const NBGM_StaticLabelTile::PathArrowTrackDataList& pathArrows, nb_boolean showPathArrow, float scaleFactor);
    virtual ~NBGM_StaticLayoutTask();
public:
    virtual void Execute();
    NBRE_Entity* CreatePathArrow(const NBRE_Vector<NBRE_Vector2f>& arrowVertices);

private:
    NBGM_LabelLayer* mLabelLayer;
    shared_ptr<NBGM_TaskQueue> mRenderThread;
    uint64 mId;
    int32 mLevel;
    double mMercatorPerPixel;
    double mMaxFontSize;
    NBGM_Context* mNBGMContext;
    NBRE_AxisAlignedBox3d mAABB;
    float mCameraDistance;
    NBGM_StaticLabelTile::RoadTrackDataList mRoadTracks;
    NBGM_StaticLabelTile::ShieldTrackDataList mShieldTracks;
    NBGM_StaticLabelTile::PathArrowTrackDataList mPathArrows;
    float mScaleFactor;
};

class NBGM_StaticLayoutSyncTask : public NBGM_Task
{
public:
    NBGM_StaticLayoutSyncTask(NBGM_LabelLayer* labelLayer, uint64 id, int32 level, NBGM_StaticLayoutResultList* layoutResult);
    virtual ~NBGM_StaticLayoutSyncTask();
public:
    virtual void Execute();
private:
    NBGM_LabelLayer* mLabelLayer;
    uint64 mId;
    int32 mLevel;
    NBGM_StaticLayoutResultList* mLayoutResult;
};

NBGM_StaticLayoutSyncTask::NBGM_StaticLayoutSyncTask(NBGM_LabelLayer* labelLayer, uint64 id, int32 level, NBGM_StaticLayoutResultList* layoutResult)
:mLabelLayer(labelLayer)
,mId(id)
,mLevel(level)
,mLayoutResult(layoutResult)
{
}

NBGM_StaticLayoutSyncTask::~NBGM_StaticLayoutSyncTask()
{
    if (mLayoutResult)
    {
        for (uint32 i = 0; i < mLayoutResult->size(); ++i)
        {
            NBRE_DELETE (*mLayoutResult)[i];
        }
        NBRE_DELETE mLayoutResult;
    }
}

void
NBGM_StaticLayoutSyncTask::Execute()
{
    mLabelLayer->OnLayoutElementsDone(mId, mLevel, mLayoutResult);
    NBGM_Task::Execute();
}

NBGM_StaticLayoutTask::NBGM_StaticLayoutTask(NBGM_LabelLayer* labelLayer, shared_ptr<NBGM_TaskQueue> renderThread, uint64 id, int32 level, double mercatorPerPixel, double mMaxFontSize, NBGM_Context* mNBGMContext, const NBRE_AxisAlignedBox3d& aabb
                                             ,const NBGM_StaticLabelTile::RoadTrackDataList& roadTracks, const NBGM_StaticLabelTile::ShieldTrackDataList& shieldTracks, const NBGM_StaticLabelTile::PathArrowTrackDataList& pathArrows, nb_boolean showPathArrow
                                             ,float scaleFactor)
:mLabelLayer(labelLayer)
,mRenderThread(renderThread)
,mId(id)
,mLevel(level)
,mMercatorPerPixel(mercatorPerPixel)
,mMaxFontSize(mMaxFontSize)
,mNBGMContext(mNBGMContext)
,mAABB(aabb)
,mScaleFactor(scaleFactor)
{
    float zoomLevel = mLevel / (float)SUB_LEVELS_PER_ZOOM_LEVEL + MIN_LAYOUT_ZOOM_LEVEL;
    mCameraDistance = (float)mNBGMContext->WorldToModel(METER_TO_MERCATOR(CalcDistanceFromZoomLevel(zoomLevel)));
    mLabelLayer->OnLayoutElementsStart();
    for (uint32 i = 0; i < roadTracks.size(); ++i)
    {
        NBGM_StaticLabelTile::RoadTrackData* track = NBRE_NEW NBGM_StaticLabelTile::RoadTrackData(*roadTracks[i]);
        mRoadTracks.push_back(track);
    }
    for (uint32 i = 0; i < shieldTracks.size(); ++i)
    {
        NBGM_StaticLabelTile::ShieldTrackData* track = NBRE_NEW NBGM_StaticLabelTile::ShieldTrackData(*shieldTracks[i]);
        track->image = NBRE_ShaderPtr();
        mShieldTracks.push_back(track);
    }

    if (showPathArrow)
    {
        for (uint32 i = 0; i < pathArrows.size(); ++i)
        {
            NBGM_StaticLabelTile::PathArrowTrackData* track = NBRE_NEW NBGM_StaticLabelTile::PathArrowTrackData(*pathArrows[i]);
            track->layerData = pathArrows[i]->layerData->Clone();
            mPathArrows.push_back(track);
        }
    }
}

NBGM_StaticLayoutTask::~NBGM_StaticLayoutTask()
{
    for (uint32 i = 0; i < mRoadTracks.size(); ++i)
    {
        NBRE_DELETE mRoadTracks[i];
    }
    for (uint32 i = 0; i < mShieldTracks.size(); ++i)
    {
        NBRE_DELETE mShieldTracks[i];
    }
    for (uint32 i = 0; i < mPathArrows.size(); ++i)
    {
        NBRE_DELETE mPathArrows[i];
    }
}

struct LayoutPolyline
{
public:
    LayoutPolyline():polyline(2), width(0) {}

public:
    NBRE_Polyline2d polyline;
    double width;
    NBRE_AxisAlignedBox2d aabb;
};

struct RoadTrackSortRecord
{
    typedef NBRE_Map<RoadTrackSortRecord*, NBRE_Vector<LayoutPolyline*> > CollideTrackPolylineMap;

    uint32 index;
    float polylineLength;
    float strHeight;
    NBRE_Vector<RoadTrackSortRecord*> collideTracks;
    CollideTrackPolylineMap collideLayoutPolylines;
    NBRE_Vector<LayoutPolyline*> layoutPolylines;
    NBRE_Polyline2d* polyline;
    NBRE_AxisAlignedBox2d aabb;
};

static bool
CompareRoadTracks(RoadTrackSortRecord* t1, RoadTrackSortRecord* t2)
{
    return t1->polylineLength > t2->polylineLength;
}

static void
CalculatePolylineAABB(const NBRE_Polyline2d& polyline, NBRE_AxisAlignedBox2d& aabb)
{
    aabb.SetNull();
    for (uint32 i = 0; i < polyline.VertexCount(); ++i)
    {
        aabb.Merge(polyline.Vertex(i));
    }
    double hw = polyline.GetWidth() * 0.5;
    aabb.minExtend.x -= hw;
    aabb.minExtend.y -= hw;
    aabb.maxExtend.x += hw;
    aabb.maxExtend.y += hw;
}

static bool
GetClipPolyline(NBRE_Vector<LayoutPolyline*>& layoutPls, const NBRE_Polyline2d& polyline, NBRE_Vector<NBRE_Polyline2d>& result)
{
    result.push_back(polyline);
    for (uint32 i = 0; i < layoutPls.size(); ++i)
    {
        NBRE_Vector<NBRE_Polyline2d> clips;
        for (uint32 j = 0; j < result.size(); ++j)
        {
            NBRE_AxisAlignedBox2d aabb;
            CalculatePolylineAABB(result[j], aabb);

            if (NBRE_Intersectiond::HitTest(aabb, layoutPls[i]->aabb))
            {
                result[j].ClipByPolyline(layoutPls[i]->polyline, clips);
            }
            else
            {
                clips.push_back(result[j]);
            }
        }
        result = clips;
    }
    return result.size() > 0;
}

static void
GetLabelPolyline(NBGM_MapRoadLabel* elem, const NBRE_Vector3d& tileCenter, NBRE_Polyline2d& pl)
{
    const NBRE_Polyline2f& pf = elem->GetPolyline();
    uint32 vc = pf.VertexCount();
    for (uint32 i = 0; i < vc; ++i)
    {
        const NBRE_Vector2f& v = pf.Vertex(i);
        pl.SetVertex(i, NBRE_Vector2d(tileCenter.x + v.x, tileCenter.y + v.y));
    }
    pl.UpdateLength();
    pl.SetWidth(elem->GetWidth());
}

static nb_boolean
HitTest(NBGM_LayoutElement* elem1, NBGM_LayoutElement* elem2, const NBRE_Vector3d& tileCenter1, const NBRE_Vector3d& tileCenter2, NBRE_Context& /*context*/, NBRE_Node* /*n1*/, NBRE_Node* /*n2*/)
{
    if (elem1->GetTypeId() == NBGM_LET_MAP_ROAD_LABEL)
    {
        NBGM_MapRoadLabel* l1 = (NBGM_MapRoadLabel*)elem1;
        NBRE_Polyline2d pl1(l1->GetPolyline().VertexCount());
        GetLabelPolyline(l1, tileCenter1, pl1);

        if (elem2->GetTypeId() == NBGM_LET_MAP_ROAD_LABEL)
        {
            NBGM_MapRoadLabel* l2 = (NBGM_MapRoadLabel*)elem2;
            NBRE_Polyline2d pl2(l2->GetPolyline().VertexCount());
            GetLabelPolyline(l2, tileCenter2, pl2);

            if (pl1.WideLineHitTest(pl2))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static inline nb_boolean
IsRoadLabelTooClose(const NBRE_AxisAlignedBox3d& aabb, const NBRE_AxisAlignedBox3d& aabb2)
{
    const NBRE_Vector3d& size = aabb.GetSize() + aabb2.GetSize();
    double padding = 4.0;
    return aabb.GetCenter().SquaredDistanceTo(aabb2.GetCenter()) < size.SquaredLength() * (padding * padding) ? TRUE : FALSE;
}

void
NBGM_StaticLayoutTask::Execute()
{
    uint32 beginTime = PAL_ClockGetTimeMs();
    NBGM_StaticLayoutResultList* layoutResult = NBRE_NEW NBGM_StaticLayoutResultList;

    NBRE_AxisAlignedBox3d tileAABB = mAABB;
    double tileExtend = (mMaxFontSize > 0 ? mMaxFontSize : 20.0) * mMercatorPerPixel;
    
    NBRE_AxisAlignedBox2d tileAABB2(mAABB.minExtend.x, mAABB.minExtend.y, mAABB.maxExtend.x, mAABB.maxExtend.y);
    const NBRE_Vector3d& aabbSize = mAABB.GetSize();
    int32 xCellCount = (int32)(aabbSize.x / (tileExtend * 10));
    int32 yCellCount = (int32)(aabbSize.y / (tileExtend * 10));
    if (xCellCount < 1)
    {
        xCellCount = 1;
    }
    if (yCellCount < 1)
    {
        yCellCount = 1;
    }
    NBRE_AxisAlignedBox2d spaceAABB(0, 0, tileAABB.maxExtend.x - tileAABB.minExtend.x, tileAABB.maxExtend.y - tileAABB.minExtend.y);
    NBRE_UniformGridSpatialPartition2d space(spaceAABB, NBRE_Vector2i(xCellCount, yCellCount));


    NBRE_Vector<RoadTrackSortRecord*> roadSortList;
    for (uint32 i = 0; i < mRoadTracks.size(); ++i)
    {
        NBGM_StaticLabelTile::RoadTrackData* track = mRoadTracks[i];
        float fontHeight = (float)(track->material.minFontHeightPixels * mMercatorPerPixel);
        fontHeight = fontHeight * mLabelLayer->GetFontScale();
        float strWidth = fontHeight * track->baseStringSize.x;

        track->polyline.Smooth(fontHeight);

        RoadTrackSortRecord* rec = NBRE_NEW RoadTrackSortRecord;
        rec->index = i;
        rec->polylineLength = track->polyline.Length();
        rec->strHeight = fontHeight * track->baseStringSize.y;
        if (rec->polylineLength >= strWidth)
        {
            rec->polyline = NBRE_NEW NBRE_Polyline2d(track->polyline.VertexCount());
            for (uint32 j = 0; j < track->polyline.VertexCount(); ++j)
            {
                const NBRE_Vector2f& p = track->polyline.Vertex(j);
                rec->polyline->SetVertex(j, NBRE_Vector2d(p.x + track->refCenter.x - tileAABB.minExtend.x, p.y + track->refCenter.y - tileAABB.minExtend.y));
            }
            rec->polyline->UpdateLength();
            rec->polyline->SetWidth(rec->strHeight);
            CalculatePolylineAABB(*rec->polyline, rec->aabb);
        }
        else
        {
            rec->polyline = NULL;
        }
        roadSortList.push_back(rec);
    }

    uint32 hitTestTime = PAL_ClockGetTimeMs();
    for (uint32 i = 0; i < roadSortList.size(); ++i)
    {
        RoadTrackSortRecord* rec = roadSortList[i];
        NBRE_Polyline2d* pl = rec->polyline;
        if (pl == NULL)
        {
            continue;
        }

        for (uint32 j = i; j < roadSortList.size(); ++j)
        {
            RoadTrackSortRecord* rec2 = roadSortList[j];
            NBRE_Polyline2d* pl2 = rec2->polyline;
            if (pl2 == NULL)
            {
                continue;
            }

            if (NBRE_Intersectiond::HitTest(rec->aabb, rec2->aabb) &&
                pl->WideLineHitTest(*pl2))
            {
                rec->collideTracks.push_back(rec2);
                rec2->collideTracks.push_back(rec);
            }
        }
    }
    hitTestTime = PAL_ClockGetTimeMs() - hitTestTime;

    sort(roadSortList.begin(), roadSortList.end(), CompareRoadTracks);
    NBRE_Vector<LayoutPolyline*> layoutPls;
    float distance = mNBGMContext->ModelToWorld(MERCATER_TO_METER(mCameraDistance));
    typedef NBRE_Pair<NBRE_String, NBRE_CollisionObject2d*> LabelCollisionObject;
    NBRE_Vector<LabelCollisionObject> rdOjbs;
    NBRE_Vector<LabelCollisionObject> shOjbs;

    for (uint32 si = 0; si < roadSortList.size(); ++si)
    {
        RoadTrackSortRecord* rec = roadSortList[si];
        uint32 i = rec->index;
        const NBGM_StaticLabelTile::RoadTrackData* track = mRoadTracks[i];
        if (!CheckNearFarVisibility(distance, track->nearVisibility, track->farVisibility))
        {
            continue;
        }
        const NBRE_Vector3d& refCenter = track->refCenter;

        float fontHeight = (float)(track->material.minFontHeightPixels * mMercatorPerPixel);
        fontHeight = fontHeight * mLabelLayer->GetFontScale();
        float strWidth = fontHeight * track->baseStringSize.x + fontHeight;
        float strHeight = rec->strHeight;

        if (rec->polyline && rec->polyline->Length() >= strWidth)
        {
            NBRE_Polyline2d& polyline = *rec->polyline;
            NBRE_Vector<NBRE_Polyline2d> result;
            NBRE_Vector<LayoutPolyline*> localLayoutPls;
            for (uint32 ci = 0; ci < rec->collideTracks.size(); ++ci)
            {
                RoadTrackSortRecord* cr = rec->collideTracks[ci];
                RoadTrackSortRecord::CollideTrackPolylineMap::iterator it = cr->collideLayoutPolylines.find(rec);
                if (it != cr->collideLayoutPolylines.end())
                {
                    for (uint32 cri = 0; cri < it->second.size(); ++cri)
                    {
                        localLayoutPls.push_back(it->second[cri]);
                    }
                }
            }
            if (GetClipPolyline(localLayoutPls, polyline, result))
            {
                for (uint32 k = 0; k < result.size(); ++k)
                {
                    const NBRE_Polyline2d& pl = result[k];
                    double plLength = pl.Length();
                    if (plLength >= strWidth)
                    {
                        double padding = fontHeight * 6;
                        if (padding < strWidth)
                        {
                            padding = strWidth;
                        }
                        uint32 repeatCount = (uint32)((plLength) / (strWidth + padding));
                        if (repeatCount < 1)
                        {
                            repeatCount = 1;
                        }
                        double beginOffset = (plLength - repeatCount * strWidth - (repeatCount - 1) * padding) * 0.5;
                        for (uint32 r = 0; r < repeatCount; ++r)
                        {
                            double offset = beginOffset + r * (strWidth + padding);
                            NBRE_Polyline2d labelPolyline = pl.SubPolyline(offset, offset + strWidth);

                            NBRE_Polyline2f labelPl(labelPolyline.VertexCount());
                            for (uint32 li = 0; li < labelPolyline.VertexCount(); ++li)
                            {
                                const NBRE_Vector2d& v = labelPolyline.Vertex(li);
                                labelPl.SetVertex(li,
                                    NBRE_Vector2f((float)(v.x  - refCenter.x + tileAABB.minExtend.x),
                                    (float)(v.y  - refCenter.y + tileAABB.minExtend.y)));
                            }
                            labelPl.UpdateLength();


                            LayoutPolyline* lp = NBRE_NEW LayoutPolyline;
                            lp->polyline = labelPolyline;
                            lp->width = strHeight;
                            CalculatePolylineAABB(lp->polyline, lp->aabb);
                            NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreatePolyline2d(this, lp->polyline, strHeight, NBRE_NEW NBRE_AABBTree2d, STATIC_LAYOUT_ROAD_LABEL_MASK);
                            NBRE_AxisAlignedBox2d coAABB = co->GetAABB();
                            NBRE_Vector2d coSize = coAABB.GetSize();
                            coSize *= 2.0;
                            coAABB.minExtend -= coSize;
                            coAABB.maxExtend += coSize;

                            nb_boolean hitDuplicated = FALSE;
                            for (uint32 ri = 0; ri < rdOjbs.size(); ++ri)
                            {
                                if (rdOjbs[ri].first == track->text)
                                {
                                    if (NBRE_Intersectiond::HitTest(coAABB, rdOjbs[ri].second->GetAABB()))
                                    {
                                        hitDuplicated = TRUE;
                                        break;
                                    }
                                }
                            }

                            if (hitDuplicated)
                            {
                                NBRE_DELETE co;
                                NBRE_DELETE lp;
                                continue;
                            }

                            layoutResult->push_back(NBRE_NEW NBGM_StaticLabelTile::RoadTrackLayoutResult(i, labelPl, fontHeight, strHeight));
                            rec->layoutPolylines.push_back(lp);
                            layoutPls.push_back(lp);
                            space.AddCollisionObject(co);
                            rdOjbs.push_back(LabelCollisionObject(track->text, co));

                            for (uint32 cti = 0; cti < rec->collideTracks.size(); ++cti)
                            {
                                RoadTrackSortRecord* ct = rec->collideTracks[cti];
                                if (NBRE_Intersectiond::HitTest(ct->aabb, rec->aabb)
                                    && ct->polyline->WideLineHitTest(labelPolyline)
                                    )
                                {
                                    if (rec->collideLayoutPolylines.find(ct) == rec->collideLayoutPolylines.end())
                                    {
                                        NBRE_Vector<LayoutPolyline*> rts;
                                        rec->collideLayoutPolylines[ct] = rts;
                                    }
                                    rec->collideLayoutPolylines[ct].push_back(lp);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    uint32 roadTime = PAL_ClockGetTimeMs();

    for (uint32 i = 0; i < mShieldTracks.size(); ++i)
    {
        const NBGM_StaticLabelTile::ShieldTrackData* track = mShieldTracks[i];
        if (!CheckNearFarVisibility(distance, track->nearVisibility, track->farVisibility))
        {
            continue;
        }
        NBRE_Polyline2d polyline(track->polyline.VertexCount());
        float fontHeight = (float)(track->material.minFontHeightPixels * mMercatorPerPixel);
        fontHeight = fontHeight * mLabelLayer->GetFontScale();

        float strWidth = track->baseStringWidth * fontHeight;

        NBRE_Vector2f textureSize(track->textureSize.x * 2 * fontHeight + strWidth, (track->textureSize.y * 2 + 1.0f) * fontHeight);
        if (textureSize.x < textureSize.y)
        {
            textureSize.x = textureSize.y;
        }
        NBRE_Vector2f shieldOffset(textureSize.x * track->offset.x, textureSize.y * track->offset.y);

        float shieldSize = (float)nsl_sqrt(textureSize.x * textureSize.x + textureSize.y * textureSize.y);

        for (uint32 j = 0; j < track->polyline.VertexCount(); ++j)
        {
            const NBRE_Vector3d& p = track->polyline.Vertex(j);
            polyline.SetVertex(j, NBRE_Vector2d(p.x - tileAABB.minExtend.x, p.y - tileAABB.minExtend.y));
        }
        polyline.UpdateLength();

        double plLength = polyline.Length();
        double padding = shieldSize * 2;
        uint32 repeatCount = (uint32)((plLength) / (shieldSize + padding));
        if (repeatCount < 1)
        {
            repeatCount = 1;
        }
        double beginOffset = (plLength - repeatCount * shieldSize - (repeatCount - 1) * padding + shieldSize) * 0.5;
        for (uint32 r = 0; r < repeatCount; ++r)
        {
            const NBRE_Vector2d& layoutPosition = polyline.PointAt(beginOffset + r * (shieldSize + padding));
            NBRE_Vector2d halfSize(shieldSize * 0.5, shieldSize * 0.5);
            NBRE_AxisAlignedBox2d rc(layoutPosition - halfSize, layoutPosition + halfSize);
            NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, rc, STATIC_LAYOUT_SHIELD_MASK);

            if (!NBRE_CollisionDetector2d::HitTest(&space, co, STATIC_LAYOUT_ROAD_LABEL_MASK | STATIC_LAYOUT_SHIELD_MASK))
            {
                halfSize *= 8;
                rc.minExtend -= halfSize;
                rc.maxExtend += halfSize;
                NBRE_CollisionObject2d* co2 = NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, rc, STATIC_LAYOUT_SHIELD_MASK);
                nb_boolean hitDuplicated = FALSE;

                for (uint32 si = 0; si < shOjbs.size(); ++si)
                {
                    LabelCollisionObject& so = shOjbs[si];
                    if (so.first == track->text)
                    {
                        if (so.second->HitTest(co2))
                        {
                            hitDuplicated = TRUE;
                            break;
                        }
                    }
                }

                if (!hitDuplicated)
                {
                    layoutResult->push_back(NBRE_NEW NBGM_StaticLabelTile::ShieldTrackLayoutResult(i,
                        NBRE_Vector3d(layoutPosition.x + tileAABB.minExtend.x, layoutPosition.y + tileAABB.minExtend.y, 0),
                        fontHeight, textureSize, shieldOffset));
                    space.AddCollisionObject(co);
                    shOjbs.push_back(LabelCollisionObject(track->text, co));
                    ++r;
                }
                else
                {
                    NBRE_DELETE co;
                }
                NBRE_DELETE co2;
            }
            else
            {
                NBRE_DELETE co;
            }
        }
    }

    uint32 shieldTime = PAL_ClockGetTimeMs();

    uint32 arrowBuildTime = 0;
    if (mPathArrows.size() > 0)
    {
        // Avoid path arrow overlapping around tile boundary
        const NBRE_Vector3d& aabbSize = mAABB.GetSize();

        NBRE_AxisAlignedBox2d safeBoundX;
        safeBoundX.maxExtend.x = aabbSize.x;
        safeBoundX.maxExtend.y = aabbSize.y;
        safeBoundX.minExtend.x = safeBoundX.maxExtend.x - tileExtend;
        safeBoundX.minExtend.y = 0;
        space.AddCollisionObject(NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, safeBoundX, STATIC_LAYOUT_PATH_ARROW_MASK));

        NBRE_AxisAlignedBox2d safeBoundY;
        safeBoundY.maxExtend.x = aabbSize.x;
        safeBoundY.maxExtend.y = aabbSize.y;
        safeBoundY.minExtend.x = 0;
        safeBoundY.minExtend.y = safeBoundY.maxExtend.y  - tileExtend;
        space.AddCollisionObject(NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, safeBoundY, STATIC_LAYOUT_PATH_ARROW_MASK));

        NBRE_Vector<NBRE_Vector2f> arrowVertices;
        for (uint32 i = 0; i < mPathArrows.size(); ++i)
        {
            const NBGM_StaticLabelTile::PathArrowTrackData* track = mPathArrows[i];
            if (!CheckNearFarVisibility(distance, track->nearVisibility, track->farVisibility))
            {
                continue;
            }
            const NBRE_Vector3d& refCenter = track->refCenter;
            float arrowSize = track->patLength * mScaleFactor;
            float arrowRadius = nsl_sqrt(arrowSize * arrowSize) * 0.5f;
            NBRE_Vector2d halfArrowSize(arrowRadius, arrowRadius);
            float padding = track->patRepeat * mScaleFactor;
            float minDist = arrowSize * 2;

            NBGM_PathArrowLayerData::OneLayerPolylines& polylines = track->layerData->mLayerData;

            for (uint32 j = 0; j < polylines.size(); ++j)
            {
                NBGM_PathArrowLayerData::MergedPolyline& pts = *polylines[j];
                uint32 ptCount = pts.size();
                if (ptCount < 2)
                {
                    continue;
                }
                NBRE_Polyline2d polyline(ptCount);
                for (uint32 k = 0; k < ptCount; ++k)
                {
                    const NBRE_Vector2f& p = pts[k];
                    polyline.SetVertex(k, NBRE_Vector2d(p.x + refCenter.x - tileAABB.minExtend.x, p.y + refCenter.y - tileAABB.minExtend.y));
                }
                polyline.UpdateLength();

                double plLength = polyline.Length();
                if (plLength < arrowSize)
                {
                    continue;
                }
                uint32 repeatCount = (uint32)((plLength) / (arrowSize + padding));
                if (repeatCount < 1)
                {
                    continue;
                }
                double beginOffset = (plLength - repeatCount * arrowSize - (repeatCount - 1) * padding + arrowSize) * 0.5;
                for (uint32 r = 0; r < repeatCount; ++r)
                {
                    NBRE_Polyline2Positiond plT = polyline.ConvertOffsetToParameterCoordinate(beginOffset + r * (arrowSize + padding));
                    const NBRE_Vector2d& layoutPosition = polyline.PointAt(plT);
                    NBRE_AxisAlignedBox2d rc(layoutPosition - halfArrowSize, layoutPosition + halfArrowSize);
                    NBRE_AxisAlignedBox2d box1 = rc;
                    box1.minExtend.x -= minDist;
                    box1.minExtend.y -= minDist;
                    box1.minExtend.x += minDist;
                    box1.minExtend.y += minDist;

                    NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, rc, STATIC_LAYOUT_PATH_ARROW_MASK);
                    NBRE_CollisionObject2d* coExt = NBRE_CollisionObjectBuilder2d::CreateAABB2d(this, box1, STATIC_LAYOUT_PATH_ARROW_MASK);

                    if (!NBRE_CollisionDetector2d::HitTest(&space, co, STATIC_LAYOUT_ROAD_LABEL_MASK | STATIC_LAYOUT_SHIELD_MASK)
                        && !NBRE_CollisionDetector2d::HitTest(&space, coExt, STATIC_LAYOUT_PATH_ARROW_MASK))
                    {
                        space.AddCollisionObject(co);

                        NBRE_Vector2f arrowCenter(
                            (float)(layoutPosition.x  - refCenter.x + tileAABB.minExtend.x),
                            (float)(layoutPosition.y  - refCenter.y + tileAABB.minExtend.y));
                        NBRE_Vector2f dx = NBRE_TypeConvertf::Convert(polyline.Vertex(plT.segmentIndex + 1) - polyline.Vertex(plT.segmentIndex));
                        dx.Normalise();
                        NBRE_Vector2f dy = dx.Perpendicular();
                        dx *= arrowSize * 0.5f;
                        dy *= arrowSize * 0.5f;

                        arrowVertices.push_back(arrowCenter - dx - dy);
                        arrowVertices.push_back(arrowCenter + dx - dy);
                        arrowVertices.push_back(arrowCenter + dx + dy);
                        arrowVertices.push_back(arrowCenter - dx + dy);
                    }
                    else
                    {
                        NBRE_DELETE co;
                    }
                    NBRE_DELETE coExt;

                    arrowBuildTime += 1;
                }
            }

            NBRE_Entity* entity = CreatePathArrow(arrowVertices);

            if (entity)
            {
                layoutResult->push_back(NBRE_NEW NBGM_StaticLabelTile::PathArrowTrackLayoutResult(i, entity));
            }
        }
    }


    uint32 arrowTime = PAL_ClockGetTimeMs();

    NBRE_DebugLog(PAL_LogSeverityMinorInfo, "time:\troad=%5u\tshield=%5u\tarrow=%5u(%5u)\ttotal=%5u\thittest=%5u\ncount:\troad=%5u\tshield=%5u\tarrow=%5u\n"
        , roadTime - beginTime, shieldTime - roadTime, arrowTime - shieldTime, arrowBuildTime, arrowTime - beginTime, hitTestTime
        , mRoadTracks.size(), mShieldTracks.size(), mPathArrows.size());

    for (uint32 i = 0; i < roadSortList.size(); ++i)
    {
        NBRE_Vector<LayoutPolyline*>& lps = roadSortList[i]->layoutPolylines;
        for (uint32 j = 0; j < lps.size(); ++j)
        {
            NBRE_DELETE lps[j];
        }
        NBRE_DELETE roadSortList[i]->polyline;
        NBRE_DELETE roadSortList[i];
    }

    mRenderThread->AddTask(NBRE_NEW NBGM_StaticLayoutSyncTask(mLabelLayer, mId, mLevel, layoutResult));

    NBGM_Task::Execute();
}

NBRE_Entity*
NBGM_StaticLayoutTask::CreatePathArrow(const NBRE_Vector<NBRE_Vector2f>& arrowVertices)
{
    if (arrowVertices.size() == 0)
    {
        return NULL;
    }

    NBRE_IRenderPal* rp = mNBGMContext->renderingEngine->Context().mRenderPal;
    NBRE_VertexDeclaration* decalration = rp->CreateVertexDeclaration();


    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(arrowVertices.size());
    NBRE_HardwareVertexBuffer* vertextBuf = rp->CreateVertexBuffer(sizeof(float) * 2, arrowVertices.size(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuf->WriteData(0, arrowVertices.size()*sizeof(NBRE_Vector2f), &arrowVertices.front(), TRUE);
    vertexData->AddBuffer(vertextBuf);

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_HardwareVertexBuffer* texcoordBuf = rp->CreateVertexBuffer(sizeof(float) * 2, arrowVertices.size(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    float* pTexcoord = (float*)texcoordBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
    for (uint32 i = 0; i < arrowVertices.size() / 4; ++i)
    {
        *pTexcoord++ = 0; *pTexcoord++ = 0;
        *pTexcoord++ = 1; *pTexcoord++ = 0;
        *pTexcoord++ = 1; *pTexcoord++ = 1;
        *pTexcoord++ = 0; *pTexcoord++ = 1;
    }
    texcoordBuf->UnLock();
    vertexData->AddBuffer(texcoordBuf);

    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuf = rp->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, arrowVertices.size() / 4 * 6, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    uint16* pIndex = (uint16*)indexBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
    uint16 baseIndex = 0;
    for (uint32 i = 0; i < arrowVertices.size() / 4; ++i)
    {
        pIndex[0] = baseIndex + 0;
        pIndex[1] = baseIndex + 1;
        pIndex[2] = baseIndex + 2;
        pIndex[3] = baseIndex + 0;
        pIndex[4] = baseIndex + 2;
        pIndex[5] = baseIndex + 3;

        baseIndex += 4;
        pIndex += 6;
    }
    indexBuf->UnLock();
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh));
    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext->renderingEngine->Context(), NBRE_ModelPtr(model), 0);
    return entity;
}

uint32 NBGM_StaticLabelTile::staticId = 0;

NBGM_StaticLabelTile::NBGM_StaticLabelTile(NBGM_Context& nbgmContext, NBRE_Node* parentNode)
:mNbgmContext(nbgmContext)
,mParentNode(parentNode)
,mMaxFontSize(0)
,mVisible(TRUE)
{
    mId = (((uint64)this) << 32) +  staticId++;
    mPathArrowNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mParentNode->AddChild(mPathArrowNode);
}

NBGM_StaticLabelTile::~NBGM_StaticLabelTile()
{
    for (LayoutElementZoomLevelMap::iterator it = mLayoutZoomLevelCache.begin(); it != mLayoutZoomLevelCache.end(); ++it)
    {
        NBGM_LayoutElementList& ls = *(it->second);
        for (uint32 i = 0; i < ls.size(); ++i)
        {
            NBRE_DELETE ls[i];
        }
        NBRE_DELETE it->second;
    }

    for (uint32 i = 0; i < mRoadTracks.size(); ++i)
    {
        NBRE_DELETE mRoadTracks[i];
    }

    for (uint32 i = 0; i < mShieldTracks.size(); ++i)
    {
        NBRE_DELETE mShieldTracks[i];
    }

    for (uint32 i = 0; i < mPathArrows.size(); ++i)
    {
        NBRE_DELETE mPathArrows[i];
    }
    mParentNode->RemoveChild(mPathArrowNode.get());
}

void
NBGM_StaticLabelTile::AddRoadLabel(int32 overlayId, NBRE_Node* parentNode, const NBRE_WString& text, uint32 materialId, const NBRE_FontMaterial& material, const NBRE_Polyline2f& polyline, float width, int32 priority, float nearVisibility, float farVisibility)
{
    RoadTrackData* track = NBRE_NEW RoadTrackData(overlayId, parentNode, text, materialId, material, polyline, width, priority, nearVisibility, farVisibility);
    mRoadTracks.push_back(track);

    const NBRE_Vector3d& refCenter = parentNode->WorldPosition();
    NBRE_AxisAlignedBox3d aabb;
    for (uint32 j = 0; j < track->polyline.VertexCount(); ++j)
    {
        const NBRE_Vector2f& p = track->polyline.Vertex(j);
        aabb.Merge(NBRE_Vector3d(p.x, p.y, 0) + refCenter);
    }
    mAABB.Merge(aabb);
    track->refCenter = refCenter;

    if (material.minFontHeightPixels > mMaxFontSize)
    {
        mMaxFontSize = material.minFontHeightPixels;
    }
}

void
NBGM_StaticLabelTile::AddPathArrow(NBRE_Node* parentNode, NBGM_PathArrowLayerData* layerData, const NBRE_String& shaderName, int32 overlayId, float patLength, float patRepeat, float nearVisibility, float farVisibility)
{
    PathArrowTrackData* track = NBRE_NEW PathArrowTrackData(parentNode, layerData, shaderName, overlayId, patLength, patRepeat, nearVisibility, farVisibility);
    mPathArrows.push_back(track);

    const NBRE_Vector3d& refCenter = parentNode->WorldPosition();
    NBRE_AxisAlignedBox3d aabb;
    for (uint32 j = 0; j < layerData->mLayerData.size(); ++j)
    {
        NBGM_PathArrowLayerData::MergedPolyline& pl = *(layerData->mLayerData[j]);
        for (uint32 i = 0; i < pl.size(); ++i)
        {
            NBRE_Vector2f& p = pl[i];
            aabb.Merge(NBRE_Vector3d(p.x, p.y, 0) + refCenter);
        }
    }
    mAABB.Merge(aabb);
    track->refCenter = refCenter;
}

void
NBGM_StaticLabelTile::AddToScene()
{
    NBRE_FontManager* fontManager = mNbgmContext.renderingEngine->Context().mFontManager;
    for (uint32 i = 0; i < mRoadTracks.size(); ++i)
    {
        RoadTrackData* track = mRoadTracks[i];
        track->baseStringSize = fontManager->GetStringSize(track->material, 1.0f, track->text, FALSE);
    }

    for (uint32 i = 0; i < mShieldTracks.size(); ++i)
    {
        ShieldTrackData* track = mShieldTracks[i];
        track->baseStringWidth = fontManager->GetStringWidth(track->material, 1.0f, track->text);
    }
}

void
NBGM_StaticLabelTile::AddShieldTrack(NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline, int32 priority, float nearVisibility, float farVisibility)
{
    ShieldTrackData* track = NBRE_NEW ShieldTrackData(parentNode, subView, overlayId, text, material, materialId, image, textureSize, offset, polyline, priority, nearVisibility, farVisibility);
    mShieldTracks.push_back(track);

    NBRE_AxisAlignedBox3d aabb;
    for (uint32 j = 0; j < track->polyline.VertexCount(); ++j)
    {
        aabb.Merge(track->polyline.Vertex(j));
    }
    mAABB.Merge(aabb);

    if (material.minFontHeightPixels > mMaxFontSize)
    {
        mMaxFontSize = material.minFontHeightPixels;
    }
}

nb_boolean
NBGM_StaticLabelTile::OnLayoutElementsDone(int32 level, NBGM_StaticLayoutResultList* layoutResult)
{
    if (mOngoingZoomLevels.find(level) != mOngoingZoomLevels.end())
    {
        NBGM_LayoutElementList* elements = NBRE_NEW NBGM_LayoutElementList();
        EntityList pathArrowResults;
        for (uint32 i = 0; i < layoutResult->size(); ++i)
        {
            NBGM_StaticLayoutResult* elem = (*layoutResult)[i];

            switch(elem->type)
            {
            case NBGM_StaticLayoutResult::RoadLabel:
                {
                    RoadTrackLayoutResult* r = (RoadTrackLayoutResult*)elem;
                    RoadTrackData* track = mRoadTracks[elem->dataIndex];
                    NBGM_MapRoadLabel* label = NBRE_NEW NBGM_MapRoadLabel(
                        mNbgmContext,
                        track->overlayId,
                        track->parentNode,
                        track->text,
                        track->material,
                        track->materialId,
                        r->polyline,
                        r->fontHeight,
                        r->width,
                        mId);
                    label->SetLayoutPriority(track->priority);
                    label->AddToSence();
                    elements->push_back(label);
                }
                break;
            case NBGM_StaticLayoutResult::Shield:
                {
                    ShieldTrackLayoutResult* r = (ShieldTrackLayoutResult*)elem;
                    ShieldTrackData* track = mShieldTracks[elem->dataIndex];
                    NBGM_Shield* sh = NBRE_NEW NBGM_Shield(
                        mNbgmContext,
                        track->parentNode,
                        track->subView,
                        track->overlayId,
                        track->text,
                        track->material,
                        track->materialId,
                        r->fontHeight,
                        track->image,
                        r->textureSize,
                        r->shieldOffset,
                        r->position,
                        FALSE
                        );
                    sh->SetLayoutPriority(track->priority);
                    sh->AddToSence();
                    elements->push_back(sh);
                }
                break;
            case NBGM_StaticLayoutResult::PathArrow:
                {
                    PathArrowTrackLayoutResult* r = (PathArrowTrackLayoutResult*)elem;
                    PathArrowTrackData* track = mPathArrows[elem->dataIndex];
                    NBRE_Entity* entity = r->entity;
                    if (entity)
                    {
                        entity->SetShaderName(track->shaderName);
                        entity->SetOverlayId(track->overlayId);
                        entity->SetVisible(TRUE);
                        pathArrowResults.push_back(NBRE_EntityPtr(entity));
                        r->entity = NULL;
                    }
                }
                break;
            }
        }

        mLayoutZoomLevelCache[level] = elements;
        mOngoingZoomLevels.erase(level);
        mPathArrowZoomLevelCache[level] = pathArrowResults;
        return TRUE;
    }
    return FALSE;
}

void
NBGM_StaticLabelTile::ResolveStaticConflicts(int32 level, NBGM_StaticLabelTile* tile)
{
    LayoutElementZoomLevelMap::iterator it = mLayoutZoomLevelCache.find(level);
    if (it == mLayoutZoomLevelCache.end() || it->second == NULL || it->second->size() == 0)
    {
        return;
    }
    LayoutElementZoomLevelMap::iterator it2 = tile->mLayoutZoomLevelCache.find(level);
    if (it2 == tile->mLayoutZoomLevelCache.end() || it2->second == NULL || it2->second->size() == 0)
    {
        return;
    }

    NBGM_LayoutElementList& elements = *it->second;
    NBGM_LayoutElementList& elements2 = *it2->second;

    for (int32 i = 0; i < (int32)elements.size(); ++i)
    {
        NBGM_LayoutElement* elem = elements[i];
        const NBRE_AxisAlignedBox3d& aabb = elem->GetAABB();
        NBGM_LayoutElementType type = elem->GetTypeId();

        for (int32 j = 0; j < (int32)elements2.size(); ++j)
        {
            NBGM_LayoutElement* elem2 = elements2[j];
            const NBRE_AxisAlignedBox3d& aabb2 = elem2->GetAABB();
            NBGM_LayoutElementType type2 = elem->GetTypeId();

            if (type == NBGM_LET_MAP_ROAD_LABEL && type2 == NBGM_LET_MAP_ROAD_LABEL
                && elem->GetText() == elem2->GetText() && elem2->IsLayoutEnabled())
            {
                if (IsRoadLabelTooClose(aabb,aabb2))
                {
                    elem->EnableLayout(FALSE);
                    break;
                }
            }

            if (NBRE_Intersectiond::HitTest(aabb, aabb2))
            {
                const NBRE_Vector3d& tileCenter = mParentNode->WorldPosition();
                const NBRE_Vector3d& tileCenter2 = tile->mParentNode->WorldPosition();

                if (elem2->IsLayoutEnabled() &&
                    HitTest(elem, elem2, tileCenter, tileCenter2, mNbgmContext.renderingEngine->Context(), mParentNode, tile->mParentNode))
                {
                    elem->EnableLayout(FALSE);
                    break;
                }
            }
        }
    }
}

void
NBGM_StaticLabelTile::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    for (RoadTrackDataList::iterator it = mRoadTracks.begin(); it != mRoadTracks.end(); ++it)
    {
        RoadTrackData* t = *it;
        if (t)
        {
            materialIds.insert(t->materialId);
        }
    }

    for (ShieldTrackDataList::iterator it = mShieldTracks.begin(); it != mShieldTracks.end(); ++it)
    {
        ShieldTrackData* t = *it;
        if (t)
        {
            materialIds.insert(t->materialId);
        }
    }
}

void
NBGM_StaticLabelTile::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    for (LayoutElementZoomLevelMap::iterator it = mLayoutZoomLevelCache.begin(); it != mLayoutZoomLevelCache.end(); ++it)
    {
        NBGM_LayoutElementList* elems = it->second;
        if (elems)
        {
            for (NBGM_LayoutElementList::iterator ei = elems->begin(); ei != elems->end(); ++ei)
            {
                NBGM_LayoutElement* elem = *ei;
                if (elem)
                {
                    elem->RefreshFontMaterial(materials);
                }
            }
        }
    }

    for (RoadTrackDataList::iterator it = mRoadTracks.begin(); it != mRoadTracks.end(); ++it)
    {
        RoadTrackData* t = *it;
        if (t)
        {
            NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator mi = materials.find(t->materialId);
            if (mi != materials.end())
            {
                t->material = mi->second;
            }
        }
    }

    for (ShieldTrackDataList::iterator it = mShieldTracks.begin(); it != mShieldTracks.end(); ++it)
    {
        ShieldTrackData* t = *it;
        if (t)
        {
            NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator mi = materials.find(t->materialId);
            if (mi != materials.end())
            {
                t->material = mi->second;
            }
        }
    }
}

void
NBGM_StaticLabelTile::SetVisible(nb_boolean visible)
{
    if(!visible)
    {
        for(LayoutElementZoomLevelMap::iterator it = mLayoutZoomLevelCache.begin(); it != mLayoutZoomLevelCache.end(); ++it)
        {
            NBGM_LayoutElementList& ls = *(it->second);
            for (NBGM_LayoutElementList::iterator el = ls.begin(); el != ls.end(); ++el)
            {
                NBGM_LayoutElement* elem = *el;
                elem->RemoveFromVisibleSet();
            }
        }
        mPathArrowNode->DetachAllObjects();
    }
    mVisible = visible;
}

void
NBGM_StaticLabelTile::UpdateLayoutList(NBGM_LayoutElementList& layoutElements, shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_LabelLayer* labelLayer)
{
    if(!mVisible)
    {
        return;
    }

    NBGM_LayoutManager& layoutManager = *mNbgmContext.layoutManager;
    double cameraDistanceInMeter = MERCATER_TO_METER(mNbgmContext.ModelToWorld(mNbgmContext.transUtil->GetEyeToScreenCenterDistance()));
    int32 level = layoutManager.GetLayoutLevels().GetLayoutLevel(cameraDistanceInMeter);
    double levelMercatorPerPixel = layoutManager.GetLayoutLevels().GetLevelMercatorPerPixel(level);
    float scaleFactor = layoutManager.GetLayoutLevels().GetLevelPathArrowScale(level);

    LayoutElementZoomLevelMap::iterator it = mLayoutZoomLevelCache.find(level);
    if (it != mLayoutZoomLevelCache.end())
    {
        NBGM_LayoutElementList& ls = *(it->second);

        for (NBGM_LayoutElementList::iterator el = ls.begin(); el != ls.end(); ++el)
        {
            NBGM_LayoutElement* elem = *el;
            if (elem->IsLayoutEnabled() && layoutManager.IsInFOV(elem->GetAABB()) && layoutManager.IsInFrustum(elem->GetAABB()))
            {
                layoutElements.push_back(elem);
            }
        }

        EntityListZoomLevelMap::iterator pIt = mPathArrowZoomLevelCache.find(level);
        if (pIt != mPathArrowZoomLevelCache.end())
        {
            EntityList& els = pIt->second;
            mPathArrowNode->DetachAllObjects();
            for (uint32 i = 0; i < els.size(); ++i)
            {
                mPathArrowNode->AttachObject(els[i]);
            }
        }
    }
    else
    {
        nb_boolean showPathArrow = level >= PATH_ARROW_LEVEL_MIN && level <= PATH_ARROW_LEVEL_MAX && mPathArrows.size() > 0 ? TRUE : FALSE;
        if ((mRoadTracks.size() > 0 || mShieldTracks.size() > 0 || showPathArrow)
            && mOngoingZoomLevels.find(level) == mOngoingZoomLevels.end())
        {
            loadingThread->AddTask(NBRE_NEW NBGM_StaticLayoutTask(labelLayer, renderThread, mId, level, levelMercatorPerPixel
                , mMaxFontSize, &mNbgmContext, mAABB, mRoadTracks, mShieldTracks, mPathArrows, showPathArrow, scaleFactor));
            mOngoingZoomLevels.insert(level);
            NBRE_DebugLog(PAL_LogSeverityMinorInfo, "add layout task: level=%d, road=%u, shield=%u", level, mRoadTracks.size(), mShieldTracks.size());
        }
    }
}

NBGM_StaticLabelTile::RoadTrackData::RoadTrackData(int32 overlayId, NBRE_Node* parentNode, const NBRE_WString& text, uint32 materialId, const NBRE_FontMaterial& material, const NBRE_Polyline2f& polyline, float width, int32 priority, float nearVisibility, float farVisibility)
:overlayId(overlayId)
,parentNode(parentNode)
,text(text)
,materialId(materialId)
,material(material)
,polyline(polyline)
,width(width)
,priority(priority)
,nearVisibility(nearVisibility)
,farVisibility(farVisibility)
{
}

NBGM_StaticLabelTile::RoadTrackLayoutResult::RoadTrackLayoutResult(uint32 dataIndex, const NBRE_Polyline2f& polyline, float fontHeight, float width)
:NBGM_StaticLayoutResult(dataIndex, NBGM_StaticLayoutResult::RoadLabel)
,polyline(polyline)
,fontHeight(fontHeight)
,width(width)
{
}

NBGM_StaticLabelTile::ShieldTrackData::ShieldTrackData(NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline, int32 priority, float nearVisibility, float farVisibility)
:parentNode(parentNode)
,subView(subView)
,overlayId(overlayId)
,text(text)
,material(material)
,materialId(materialId)
,image(image)
,textureSize(textureSize)
,offset(offset)
,polyline(polyline)
,priority(priority)
,nearVisibility(nearVisibility)
,farVisibility(farVisibility)
{

}

NBGM_StaticLabelTile::ShieldTrackLayoutResult::ShieldTrackLayoutResult(uint32 dataIndex, const NBRE_Vector3d& position, float fontHeight, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& shieldOffset)
:NBGM_StaticLayoutResult(dataIndex, NBGM_StaticLayoutResult::Shield)
,position(position)
,fontHeight(fontHeight)
,textureSize(textureSize)
,shieldOffset(shieldOffset)
{
}

NBGM_StaticLabelTile::PathArrowTrackData::PathArrowTrackData(NBRE_Node* parentNode, NBGM_PathArrowLayerData* layerData, const NBRE_String& shaderName, int32 overlayId, float patLength, float patRepeat, float nearVisibility, float farVisibility)
:parentNode(parentNode)
,layerData(layerData)
,shaderName(shaderName)
,overlayId(overlayId)
,patLength(patLength)
,patRepeat(patRepeat)
,nearVisibility(nearVisibility)
,farVisibility(farVisibility)
{
}

NBGM_StaticLabelTile::PathArrowTrackData::~PathArrowTrackData()
{
    NBRE_DELETE layerData;
}

NBGM_StaticLabelTile::PathArrowTrackLayoutResult::PathArrowTrackLayoutResult(uint32 dataIndex, NBRE_Entity* entity)
:NBGM_StaticLayoutResult(dataIndex, NBGM_StaticLayoutResult::PathArrow)
,entity(entity)
{
}

NBGM_StaticLabelTile::PathArrowTrackLayoutResult::~PathArrowTrackLayoutResult()
{
    NBRE_DELETE entity;
}
