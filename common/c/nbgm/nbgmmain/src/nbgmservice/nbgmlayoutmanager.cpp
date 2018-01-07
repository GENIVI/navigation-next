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
#include "nbgmplatformconfig.h"
#include "nbgmlayoutmanager.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreclipping.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"
#include "nbgmconst.h"
#include "palclock.h"
#include "nbretransformutil.h"
#include "nbgmconst.h"
#include "nbgmmaproadlabel.h"
#include "nbgmstaticpoi.h"
#include "nbgmcustompin.h"

static inline uint32 Now()
{
    return PAL_ClockGetTimeMs();
}

static bool
CompareLayoutElement(NBGM_LayoutElement* v1, NBGM_LayoutElement* v2)
{
    int32 l = v1->GetLayerId().compare(v2->GetLayerId());
    if (l == 0)
    {
        int32 p1 = v1->GetLayoutPriority();
        int32 p2 = v2->GetLayoutPriority();
        if (p1 == p2)
        {
            return v1->GetDrawIndex() < v2->GetDrawIndex();
        }
        else
        {
            return p1 < p2;
        }
    }
    else
    {
        return l < 0;
    }
}

static bool
CompareNavLayoutElement(NBGM_LayoutElement* v1, NBGM_LayoutElement* v2)
{
    return v1->GetLayoutPriority() < v2->GetLayoutPriority();
}


NBGM_LayoutManager::NBGM_LayoutManager(NBGM_Context& nbgmContext, NBRE_Viewport* viewport, uint32 cellSize)
:mNBGMContext(nbgmContext)
,mLayoutBuffer(nbgmContext, (float)viewport->GetRect().maxExtend.x,
               (float)viewport->GetRect().maxExtend.y,
               (float)cellSize)
               ,mViewport(viewport)
               ,mMaxLabels(NBRE_INVALID_INDEX)
               ,mMaxAreaBuildingLabels(NBRE_INVALID_INDEX)
               ,mLabels(0)
               ,mAreaBuildingLabels(0)
               ,mIsNavMode(FALSE)
               ,mLastUpdateTime(0)
               ,mScreenCenterPixelsPerUnit(0.0)
               ,mNextRefreshTime(0)
               ,mAccumulativeTime(0)
               ,mSecondsFromLastUpdate(0)
               ,mSecondsFromMapViewStartup(0)
               ,mNeedUpdate(TRUE)
               ,mResetVisibleElement(FALSE)
               ,mHasUpdated(FALSE)
               ,mHasAddedElement(FALSE)
               ,mStopForAWhile(FALSE)
               ,mUpdateIndex(0)
               ,mLastCameraUpdateTime(0)
               ,mLayoutLevels(nbgmContext, MIN_LAYOUT_ZOOM_LEVEL, MAX_LAYOUT_ZOOM_LEVEL, SUB_LEVELS_PER_ZOOM_LEVEL - 1)
{
    ResetScreenRect();
    mMapViewStartupTime = PAL_ClockGetTimeMs();
}

NBGM_LayoutManager::~NBGM_LayoutManager()
{
}

void
NBGM_LayoutManager::ResizeBuffer(uint32 width, uint32 height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    NBRE_Vector2f oldSize = mLayoutBuffer.Size();
    if ((float)width != oldSize.x || (float)height != oldSize.y)
    {
        // Refresh layout
        Invalidate();
        mLayoutBuffer.Resize((float)width, (float)height);
        ResetScreenRect();
    }
}

nb_boolean
NBGM_LayoutManager::Update(uint32 interval)
{
    return mIsNavMode ? UpdateForNav() : UpdateForMap(interval);
}

nb_boolean
NBGM_LayoutManager::UpdateForMap(uint32 intervalValue)
{
    // Calculate elapsed time
    uint32 now = Now();
    mNow = now;
    mDeadline = now + 1000 / 60;


    if (mUpdateIndex == 0)
    {
        uint32 interval = intervalValue;
        if (mNow < mLastUpdateTime + interval)
        {
            return mNeedUpdate;
        }

        UpdateCameraMatrix();
        mLayoutElements.clear();
        for (uint32 i = 0; i < mLayoutProviders.size(); ++i)
        {
            NBGM_ILayoutProvider* provider = mLayoutProviders[i];
            provider->UpdateLayoutList(mLayoutElements);
        }

        mNeedUpdate = FALSE;

        const NBRE_Matrix4x4d& mat = mNBGMContext.transUtil->GetViewProjectViewportMatrix();
        if (!(mOldViewProjectViewportMatrix == mat))
        {
            // Camera changed
            mLastCameraUpdateTime = mNow;
            mOldViewProjectViewportMatrix = mat;
            mNeedUpdate = TRUE;
        }
        if (!mNeedUpdate)
        {
            for (uint32 i = 0; i < mLayoutElements.size(); ++i)
            {
                NBGM_LayoutElement* elem = mLayoutElements[i];
                if (elem->mLastLayoutTime <= elem->mUpdateTime
                    || elem->mLastLayoutTime < mLastCameraUpdateTime)
                {
                    mNeedUpdate = TRUE;
                    break;
                }
            }
        }

        if (mNeedUpdate)
        {
            ResetVisibleElements();
            mLayoutBuffer.Clear();
            ResizeNameFilterSize();

            FindVisibleObjects(mUpdateList);
            mResetVisibleElement = FALSE;
            mHasAddedElement = FALSE;
            mCurrentLayer.clear();
        }
    }
    else
    {
        mNeedUpdate = TRUE;
    }
    
    AddNewElement();
    return mNeedUpdate;
}

nb_boolean
NBGM_LayoutManager::UpdateForNav()
{
    // Calculate elapsed time
    uint32 now = Now();
    mNow = now;
    mDeadline = now + 1000 / 30;

    UpdateCameraMatrix();

    if (mUpdateIndex == 0)
    {   
        mLayoutElements.clear();
        mUpdateList.clear();

        if (mNextRefreshTime != 0 && now >= mNextRefreshTime)
        {
            for (uint32 i = 0; i < mDisplayElements.size(); ++i)
            {
                NBGM_LayoutElement* elem = mDisplayElements[i];
                elem->mDrawIndex = NBGM_LayoutElement::INVALID_DRAW_INDEX;
                elem->SetVisible(FALSE);
            }
            mDisplayElements.clear();
            mNextRefreshTime = 0;
        }

        for (uint32 i = 0; i < mLayoutProviders.size(); ++i)
        {
            NBGM_ILayoutProvider* provider = mLayoutProviders[i];
            provider->UpdateLayoutList(mLayoutElements);
        }

        {
            ResetVisibleElements();
            ResizeNameFilterSize();

            for (uint32 i = 0; i < mLayoutElements.size(); ++i)
            {
                NBGM_LayoutElement* elem = mLayoutElements[i];
                if (elem->mDrawIndex == NBGM_LayoutElement::INVALID_DRAW_INDEX)
                {
                    mUpdateList.push_back(elem);
                }
            }

            if (mUpdateList.size() > 1)
            {
                sort(mUpdateList.begin(), mUpdateList.end(), CompareNavLayoutElement);
            }

            mResetVisibleElement = FALSE;
            mHasAddedElement = FALSE;
        }
    }

    AddNavNewElement();
    return TRUE;
}

void
NBGM_LayoutManager::UpdateVisibleElements()
{
    for (uint32 i = 0; i < mDisplayElements.size(); ++i)
    {
        mDisplayElements[i]->OnUpdate();
    }
}

void
NBGM_LayoutManager::FindVisibleObjects(NBGM_LayoutElementList& pvs)
{
    for (uint32 i = 0; i < mLayoutElements.size(); ++i)
    {
        NBGM_LayoutElement* elem = mLayoutElements[i];
        pvs.push_back(elem);
    }

    if (pvs.size() > 1)
    {
        sort(pvs.begin(), pvs.end(), CompareLayoutElement);
    }
}

void
NBGM_LayoutManager::AddElement(NBGM_LayoutElement* element)
{
    for (NBGM_LayoutElementList::iterator i = mLayoutElements.begin(); i != mLayoutElements.end(); ++i)
    {
        if (*i == element)
        {
            return;
        }
    }

    mLayoutElements.push_back(element);
    mHasAddedElement = TRUE;
}

void
NBGM_LayoutManager::RemoveElement(NBGM_LayoutElement* element)
{
    for (NBGM_LayoutElementList::iterator i = mVisibleElements.begin();
        i != mVisibleElements.end(); ++i)
    {
        if (*i == element)
        {
            mVisibleElements.erase(i);
            break;
        }
    }

    for (uint32 i = 0; i < mUpdateList.size(); ++i)
    {
        if (mUpdateList[i] == element)
        {
            if (i <= mUpdateIndex && mUpdateIndex > 0)
            {
                --mUpdateIndex;
            }
            NBGM_LayoutElementList::iterator it = mUpdateList.begin() + i;
            mUpdateList.erase(it);            
            break;
        }
    }

    for (NBGM_LayoutElementList::iterator i = mDisplayElements.begin();
        i != mDisplayElements.end(); ++i)
    {
        if (*i == element)
        {
            mDisplayElements.erase(i);
            break;
        }
    }
}

void
NBGM_LayoutManager::RemoveAllElements()
{
    for (uint32 i = 0; i < mDisplayElements.size(); ++i)
    {
        NBGM_LayoutElement* elem = mDisplayElements[i];
        elem->mDrawIndex = NBGM_LayoutElement::INVALID_DRAW_INDEX;
        elem->SetVisible(FALSE);
    }
    mDisplayElements.clear();
    mNextRefreshTime = 0;
    mLayoutElements.clear();
    mUpdateIndex = 0;
    mUpdateList.clear();
    mLastUpdateTime = Now();
}

nb_boolean
NBGM_LayoutManager::IsInNearPlaneFrontSide(const NBRE_Vector3d& position)
{
    return mFrustum.GetNear().GetSide(position) != NBRE_PS_BACK_SIDE ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::IsInNearPlaneFrontSide(const NBRE_Polyline3d& polyline)
{
    const NBRE_Planed& nearPlane = mFrustum.GetNear();

    for (uint32 i = 0; i < polyline.VertexCount(); ++i)
    {
        if (nearPlane.GetSide(polyline.Vertex(i)) == NBRE_PS_BACK_SIDE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

nb_boolean
NBGM_LayoutManager::IsInFrustum(const NBRE_Vector3d& position)
{
    return NBRE_Intersectiond::HitTest(mFrustum, position);
}

nb_boolean
NBGM_LayoutManager::IsInFrustum(const NBRE_AxisAlignedBox3d& aabb)
{
    return NBRE_Intersectiond::HitTest(mFrustum, aabb);
}

nb_boolean
NBGM_LayoutManager::IsInFrustum(const NBRE_Polyline3d& polyline)
{
    return NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB());
}

nb_boolean
NBGM_LayoutManager::IsInFOV(const NBRE_AxisAlignedBox3d& aabb)
{
    return NBRE_Intersectiond::HitTest(mFovAABB, aabb);
}

nb_boolean
NBGM_LayoutManager::WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    screenPosition = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, position);
    return TRUE;
}

nb_boolean
NBGM_LayoutManager::WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition, double& w)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    screenPosition = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, position, &w);
    return TRUE;
}

nb_boolean
NBGM_LayoutManager::WorldToScreen(const NBRE_Polyline3d& polyline, NBRE_Vector<NBRE_Polyline2d>& screenPolylines)
{
    if (!NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB()))
    {
        return FALSE;
    }

    const NBRE_Planed& nearPlane = mFrustum.GetNear();
    Polyline3dList pls;
    NBRE_Clippingd::ClipByPlane(nearPlane, polyline, pls);
    for (uint32 i = 0; i < pls.size(); ++i)
    {
        NBRE_Polyline3d& pl = pls[i];
        NBRE_Polyline2d screenPl(pl.VertexCount());
        for (uint32 j = 0; j < pl.VertexCount(); ++j)
        {
            screenPl.SetVertex(j, NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, pl.Vertex(j)));
        }
        screenPl.UpdateLength();
        screenPolylines.push_back(screenPl);
    }

    return screenPolylines.size() > 0 ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::WorldToScreen(const NBRE_Polyline3d& polyline, double width, int32 lineWidth, uint32 mask, double minPolylineLength, NBGM_LayoutPolylineList& result)
{
    if (!NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB()))
    {
        return FALSE;
    }
    const NBRE_Planed& nearPlane = mFrustum.GetNear();
    Polyline3dList pls;
    NBRE_Clippingd::ClipByPlane(nearPlane, polyline, pls);
    for (uint32 i = 0; i < pls.size(); ++i)
    {
        NBGM_LayoutPolyline pl(&mNBGMContext, pls[i], width);
        if (pl.GetScreenPolyline().Length() < minPolylineLength)
        {
            continue;
        }
        NBGM_LayoutPolylineList clipPls;
        pl.GetScreenAvaliableParts(lineWidth, mask, clipPls);
        for (NBGM_LayoutPolylineList::iterator j = clipPls.begin(); j != clipPls.end(); ++j)
        {
            result.push_back(*j);
        }
    }

    return result.size() > 0 ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::WorldToScreen(const NBRE_Polyline3d& polyline, NBGM_LayoutPolylineList& result)
{
    if (!NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB()))
    {
        return FALSE;
    }
    const NBRE_Planed& nearPlane = mFrustum.GetNear();
    Polyline3dList pls;
    NBRE_Clippingd::ClipByPlane(nearPlane, polyline, pls);
    for (uint32 i = 0; i < pls.size(); ++i)
    {
        NBGM_LayoutPolyline pl(&mNBGMContext, pls[i], 1);

        NBGM_LayoutPolylineList clipPls;
        pl.ClipByRect(mScreenRect, clipPls);
        for (NBGM_LayoutPolylineList::const_iterator j = clipPls.begin(); j != clipPls.end(); ++j)
        {
            result.push_back(*j);
        }
    }

    return result.size() > 0 ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::PixelsPerUnit(const NBRE_Vector3d& position, double& pixelsPerUnit)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    pixelsPerUnit = NBRE_Transformationd::GetPixelsPerUnit(mViewMatrix, mProjectMatrix, mViewportMatrix, position, 1);
    return TRUE;
}

nb_boolean
NBGM_LayoutManager::IsShieldDisplayed(const NBRE_WString& text)
{
    return mShieldFilter.find(text) != mShieldFilter.end() ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::IsRoadLabelDisplayed(const NBRE_WString& text)
{
    if (mLabels >= mMaxLabels)
    {
        return TRUE;
    }
    return mRoadFilter.find(text) != mRoadFilter.end() ? TRUE : FALSE;
}

nb_boolean
NBGM_LayoutManager::IsPointLabelDisplayed(const NBRE_WString& text)
{
    if (mAreaBuildingLabels >= mMaxAreaBuildingLabels ||
        mLabels >= mMaxLabels)
    {
        return TRUE;
    }

    return mAreaBuildingFilter.find(text) != mAreaBuildingFilter.end() ? TRUE : FALSE;
}

void
NBGM_LayoutManager::AddShield(const NBRE_WString& text)
{
    mShieldFilter.insert(text);
}

void
NBGM_LayoutManager::AddRoadLabel(const NBRE_WString& text)
{
    mRoadFilter.insert(text);
    ++mLabels;
}

void
NBGM_LayoutManager::AddPointLabel(const NBRE_WString& text)
{
    mAreaBuildingFilter.insert(text);
    ++mAreaBuildingLabels;
    ++mLabels;
}
const double MIN_VISIBLE_ANGLE_TO_EYE_FROM_GROUND_TANGENT = nsl_tan(25.0 * 3.1415926 / 180.0);
void
NBGM_LayoutManager::UpdateCameraMatrix()
{
    mViewMatrix = mViewport->Camera()->ExtractMatrix();
    mProjectMatrix = NBRE_TypeConvertd::Convert(mViewport->Camera()->GetFrustum().ExtractMatrix());
    mViewportMatrix = NBRE_TypeConvertd::Convert(mViewport->ExtractMatrix());
    NBRE_Matrix4x4d compositeMatrix = mViewportMatrix * mProjectMatrix * mViewMatrix;
    mViewProjectViewportMatrix = compositeMatrix;

    mFrustum = NBRE_Frustum3d(mProjectMatrix * mViewMatrix);
    const NBRE_Matrix4x4d& cameraMatrix = mViewport->Camera()->WorldTransform();
    mCameraRight.x = cameraMatrix[0][0];
    mCameraRight.y = cameraMatrix[1][0];
    mCameraRight.z = cameraMatrix[2][0];

    mEyePosition = mViewport->Camera()->WorldPosition();

    double maxVisibleDistance = mEyePosition.z / MIN_VISIBLE_ANGLE_TO_EYE_FROM_GROUND_TANGENT;
    NBRE_Vector3d halfVisibleSize(maxVisibleDistance, maxVisibleDistance, maxVisibleDistance);
    NBRE_Vector3d fovGroundCenter(mEyePosition.x, mEyePosition.y, 0);
    mFovAABB.minExtend = fovGroundCenter - halfVisibleSize;
    mFovAABB.maxExtend = fovGroundCenter + halfVisibleSize;
    const NBRE_AxisAlignedBox3d& frustumAABB = mFrustum.GetAABB();
    if (frustumAABB.minExtend.x > mFovAABB.minExtend.x)
    {
        mFovAABB.minExtend.x = frustumAABB.minExtend.x;
    }
    if (frustumAABB.minExtend.y > mFovAABB.minExtend.y)
    {
        mFovAABB.minExtend.y = frustumAABB.minExtend.y;
    }
    if (frustumAABB.minExtend.z > mFovAABB.minExtend.z)
    {
        mFovAABB.minExtend.z = frustumAABB.minExtend.z;
    }

    if (frustumAABB.maxExtend.x < mFovAABB.maxExtend.x)
    {
        mFovAABB.maxExtend.x = frustumAABB.maxExtend.x;
    }
    if (frustumAABB.maxExtend.y < mFovAABB.maxExtend.y)
    {
        mFovAABB.maxExtend.y = frustumAABB.maxExtend.y;
    }
    if (frustumAABB.maxExtend.z < mFovAABB.maxExtend.z)
    {
        mFovAABB.maxExtend.z = frustumAABB.maxExtend.z;
    }
}

void
NBGM_LayoutManager::Invalidate()
{
    // Add a delay to refresh, in ms.
    // This value cannot be too small because some action like screen rotation may be deferred.
    mNextRefreshTime = Now() + 700;
}

void
NBGM_LayoutManager::UpdateRegion(const NBRE_Polyline3d& polyline, int32 lineWidth, uint32 mask)
{
    if (!NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB()))
    {
        return;
    }
    const NBRE_Planed& nearPlane = mFrustum.GetNear();
    uint32 vc = polyline.VertexCount();
    for (uint32 i = 0; i < vc - 1; ++i)
    {
        NBRE_Vector3d p0 = polyline.Vertex(i);
        NBRE_Vector3d p1 = polyline.Vertex(i + 1);
        NBRE_AxisAlignedBox3d aabb;
        aabb.Merge(p0);
        aabb.Merge(p1);
        if (NBRE_Intersectiond::HitTest(mFrustum, aabb))
        {
            if (NBRE_Clippingd::ClipByPlane(nearPlane, p0, p1) != NBRE_LCR_OUTSIDE)
            {
                NBRE_Vector2d sp0 = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, p0);
                NBRE_Vector2d sp1 = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, p1);
                if( NBRE_Clippingd::ClipLineByRect(mScreenRect, sp0, sp1) != NBRE_LCR_OUTSIDE)
                {
                    mLayoutBuffer.UpdateRegion(sp0, sp1, lineWidth, mask);
                }
            }
        }
    }
}

void
NBGM_LayoutManager::ResetScreenRect()
{
    NBRE_AxisAlignedBox2i rc(mViewport->GetRect());
    mScreenRect.minExtend.x = static_cast<double>(rc.minExtend.x);
    mScreenRect.minExtend.y = static_cast<double>(rc.minExtend.y);
    mScreenRect.maxExtend.x = static_cast<double>(rc.maxExtend.x);
    mScreenRect.maxExtend.y = static_cast<double>(rc.maxExtend.y);
};

void
NBGM_LayoutManager::ResizeNameFilterSize()
{
    mLabels = 0;
    mAreaBuildingLabels = 0;

    mShieldFilter.clear();
    mRoadFilter.clear();
    mAreaBuildingFilter.clear();
}


static nb_boolean
IsShieldTooClose(NBGM_LayoutElement* e1, NBGM_LayoutElement* e2)
{
    float sameNameMaxDistance = DP_TO_PIXEL(POINT_SHIELD_SAME_NAME_DISTANCE_DP);

    const NBRE_Vector2d& p1 = e1->GetScreenRect().GetCenter();
    const NBRE_Vector2d& p2 = e2->GetScreenRect().GetCenter();

    double distance = p1.DistanceTo(p2);
    if (e1->GetText() == e2->GetText())
    {
        if (distance <= sameNameMaxDistance)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void
NBGM_LayoutManager::AddNewElement()
{
    uint32 count = mUpdateList.size();

    if (count == 0)
    {
        return;
    }

    while ( mUpdateIndex < count )
    {
        NBGM_LayoutElement* elem = mUpdateList[mUpdateIndex];
        const NBRE_String& layerId = elem->GetLayerId();
        if (mCurrentLayer != layerId)
        {
            GetLayoutBuffer().Clear();
            mCurrentLayer = layerId;
        }

        if (elem->IsElementRegionAvailable())
        {
            nb_boolean isReject = FALSE;
            NBGM_LayoutElementType type = elem->GetTypeId();
            if (type == NBGM_LET_POINT_SHIELD)
            {
                for (uint32 i = 0; i < mVisibleElements.size(); ++i)
                {
                    NBGM_LayoutElement* de = mVisibleElements[i];
                    if (de->GetTypeId() == NBGM_LET_POINT_SHIELD 
                        && IsShieldTooClose(elem, de))
                    {
                        isReject = TRUE;
                        break;
                    }
                }
            }
            else if(type == NBGM_LET_STATIC_POI)
            {
                NBGM_StaticPoi* poi = (NBGM_StaticPoi*)elem;
                for (uint32 i = 0; i < mVisibleElements.size(); ++i)
                {
                    NBGM_LayoutElement* de = mVisibleElements[i];
                    if (poi->IsTooCloseTo(de))
                    {
                        isReject = TRUE;
                        break;
                    }
                }
            }
            else if(type == NBGM_LET_CUSTOM_PIN)
            {
                NBGM_CustomPin* poi = (NBGM_CustomPin*)elem;
                for (uint32 i = 0; i < mVisibleElements.size(); ++i)
                {
                    NBGM_LayoutElement* de = mVisibleElements[i];
                    if (poi->IsTooCloseTo(de))
                    {
                        isReject = TRUE;
                        break;
                    }
                }
            }

            if (!isReject)
            {
                elem->UpdateRegion();
                mVisibleElements.push_back(elem);
            }
        }
        
        elem->mLastLayoutTime = mNow;
        ++mUpdateIndex;

        uint32 now = Now();
        if (now > mDeadline)
        {
            NBRE_DebugLog(PAL_LogSeverityMinorInfo, "Layout update step %u/%u, time = %u", mUpdateIndex - 1, count, now - mNow);
            return;
        }
    }
    
    uint32 drawIndex = 0;
    for (uint32 i = 0; i < mDisplayElements.size(); ++i)
    {
        NBGM_LayoutElement* elem = mDisplayElements[i];
        nb_boolean found = FALSE;
        for (uint32 j = 0; j < mVisibleElements.size(); ++j)
        {
            if (elem == mVisibleElements[j])
            {
                elem->mDrawIndex = drawIndex++;
                found = TRUE;
                break;
            }
        }
        if (!found)
        {
            elem->mDrawIndex = NBGM_LayoutElement::INVALID_DRAW_INDEX;
            elem->SetVisible(FALSE);
        }
    }
    mDisplayElements.clear();

    for (uint32 i = 0; i < mVisibleElements.size(); ++i)
    {
        NBGM_LayoutElement* elem = mVisibleElements[i];
        if (elem->mDrawIndex == NBGM_LayoutElement::INVALID_DRAW_INDEX)
        {
            elem->mDrawIndex = drawIndex++;
        }
        elem->SetVisible(TRUE);
        elem->mLastLayoutTime = mNow;
        mDisplayElements.push_back(elem);
    }

    NBRE_DebugLog(PAL_LogSeverityMinorInfo, "Layout update step %u/%u, time = %u", count, count, Now() - mNow);

    mUpdateIndex = 0;
    mVisibleElements.clear();
    mUpdateList.clear();
    mLastUpdateTime = Now();
}

void
NBGM_LayoutManager::AddNavNewElement()
{
    uint32 count = mUpdateList.size();

    if (count == 0)
    {
        return;
    }

    uint32 drawIndex = 0;
    mLayoutBuffer.Clear();
    for (int32 i = 0; i < (int32)mDisplayElements.size(); ++i)
    {
        NBGM_LayoutElement* elem = mDisplayElements[i];
        if (!IsInFrustum(elem->GetAABB()))
        {
            elem->mDrawIndex = NBGM_LayoutElement::INVALID_DRAW_INDEX;
            elem->SetVisible(FALSE);
            mDisplayElements.erase(mDisplayElements.begin() + i);
            --i;
        }
        else
        {
            elem->UpdateRegion();
            elem->mDrawIndex = drawIndex++;
            elem->SetVisible(TRUE);
            elem->mLastLayoutTime = mNow;
        }
    }

    while ( mUpdateIndex < count )
    {
        NBGM_LayoutElement* elem = mUpdateList[mUpdateIndex];

        if (elem->IsElementRegionAvailable())
        {
            elem->UpdateRegion();
            elem->mDrawIndex = drawIndex++;
            elem->SetVisible(TRUE);
            elem->mLastLayoutTime = mNow;
            mDisplayElements.push_back(elem);
        }

        elem->mLastLayoutTime = mNow;
        ++mUpdateIndex;

        uint32 now = Now();
        if (mUpdateIndex < count && now > mDeadline)
        {
            NBRE_DebugLog(PAL_LogSeverityMinorInfo, "Layout update step %u/%u, time = %u", mUpdateIndex - 1, count, now - mNow);
            return;
        }
    }

    mUpdateIndex = 0;
    mUpdateList.clear();
    mLastUpdateTime = Now();
}

void
NBGM_LayoutManager::ResetVisibleElements()
{
    {
        NBRE_Vector3d frutumCorners[8];
        mFrustum.GetCorners(frutumCorners);
        NBRE_Vector3d nearCenter = (frutumCorners[0] + frutumCorners[2]) * 0.5;
        NBRE_Vector3d farCenter = (frutumCorners[4] + frutumCorners[6]) * 0.5;
        double t = 0.5;
        if (nearCenter.z != farCenter.z)
        {
            t = NBRE_Math::Abs<double>(nearCenter.z / (farCenter.z - nearCenter.z));
        }
        NBRE_Vector3d screenCenter = NBRE_LinearInterpolated::Lerp(nearCenter, farCenter, t);
        if (!PixelsPerUnit(screenCenter, mScreenCenterPixelsPerUnit))
        {
            return;
        }
    }
}

void 
NBGM_LayoutManager::AddLayoutProvider(NBGM_ILayoutProvider* provider)
{
    for (uint32 i = 0; i < mLayoutProviders.size(); ++i)
    {
        if (provider == mLayoutProviders[i])
        {
            return;
        }
    }
    mLayoutProviders.push_back(provider);
}

void
NBGM_LayoutManager::RemoveLayoutProvider(NBGM_ILayoutProvider* provider)
{
    for (uint32 i = 0; i < mLayoutProviders.size(); ++i)
    {
        if (provider == mLayoutProviders[i])
        {
            mLayoutProviders.erase(mLayoutProviders.begin() + i);
            --i;
        }
    }
}

StaticLayoutLevelInfo::StaticLayoutLevelInfo(NBGM_Context& nbgmContext, int8 minZoomLevel, int8 maxZoomLevel, int8 subLevels)
{
    for (int8 i = minZoomLevel; i <= maxZoomLevel; ++i)
    {
        double pixelResolution = (CalcPixelResolution(i));
        double mercatorPerPixel = nbgmContext.WorldToModel(METER_TO_MERCATOR(pixelResolution));

        mMercatorPerPixelTable.push_back(mercatorPerPixel);

        if (subLevels > 0)
        {
            double mercatorPerPixelNextLevel = nbgmContext.WorldToModel(METER_TO_MERCATOR(CalcPixelResolution(i + 1)));

            for (int8 j = 0; j < subLevels; ++j)
            {
                double subLevelValue = NBRE_LinearInterpolated::Lerp(mercatorPerPixel, mercatorPerPixelNextLevel, (double)(j + 1) / (subLevels + 1));
                mMercatorPerPixelTable.push_back(subLevelValue);
            }
        }
    }
}

int32
StaticLayoutLevelInfo::GetLayoutLevel(double cameraDistance)
{
    float zoomLevel = CalcZoomLevelFloat((float)cameraDistance);
    int32 layoutLevel = (int32)(zoomLevel * SUB_LEVELS_PER_ZOOM_LEVEL) - MIN_LAYOUT_ZOOM_LEVEL * SUB_LEVELS_PER_ZOOM_LEVEL;
    int32 levelCount = (int32)mMercatorPerPixelTable.size();
    if (layoutLevel >= levelCount)
    {
        layoutLevel = levelCount - 1;
    }
    else if (layoutLevel < 0)
    {
        layoutLevel = 0;
    }
    return layoutLevel;
}

double
StaticLayoutLevelInfo::GetLevelMercatorPerPixel(int32 level)
{
    return mMercatorPerPixelTable[level];
}

float
StaticLayoutLevelInfo::GetLevelPathArrowScale(int32 level)
{
    int32 zoomLevel0 = level / SUB_LEVELS_PER_ZOOM_LEVEL + MIN_LAYOUT_ZOOM_LEVEL;
    int32 zoomLevel1 = zoomLevel0 + 1;
    if (zoomLevel1 > MAX_LAYOUT_ZOOM_LEVEL)
    {
        zoomLevel1 = MAX_LAYOUT_ZOOM_LEVEL;
    }
    float t = (level % SUB_LEVELS_PER_ZOOM_LEVEL) / (float)SUB_LEVELS_PER_ZOOM_LEVEL;
    return GetPolylineScaleFactor(static_cast<int8>(zoomLevel0), FALSE) * (1.0f - t) + GetPolylineScaleFactor(static_cast<int8>(zoomLevel1), FALSE) * t;
}
