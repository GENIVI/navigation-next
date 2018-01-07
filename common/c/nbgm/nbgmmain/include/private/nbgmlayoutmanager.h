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

    @file nbgmlayoutmanager.h
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
#ifndef _NBGM_LAYOUT_MANAGER_H_
#define _NBGM_LAYOUT_MANAGER_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "palerror.h"
#include "nbgmlayoutbuffer.h"
#include "nbreveiwport.h"
#include "nbreplane.h"
#include "nbgmlayoutpolyline.h"
#include "nbgmlayoutelement.h"
#include "nbrefrustum3.h"
#include "nbgmcontext.h"
#include "palclock.h"

/*! \addtogroup NBGM_Service
*  @{
*/


class StaticLayoutLevelInfo
{
public:
    StaticLayoutLevelInfo(NBGM_Context& nbgmContext, int8 minZoomLevel, int8 maxZoomLevel, int8 subLevels);
    int32 GetLayoutLevel(double cameraDistance);
    double GetLevelMercatorPerPixel(int32 level);
    float GetLevelPathArrowScale(int32 level);

private:
    NBRE_Vector<double> mMercatorPerPixelTable;
};

/** Layout element provider provide layout elements before layout begins.
Custom frustum culling logic can be implemented here.
*/
class NBGM_ILayoutProvider
{
public:
    virtual ~NBGM_ILayoutProvider() {}
    virtual void UpdateLayoutList(NBGM_LayoutElementList& layoutElements) = 0;
};

/** Layout manager manages elements which cannot overlap each other.
Like labels, shield, poi...
*/
class NBGM_LayoutManager
{
public:
    typedef NBRE_Vector<NBRE_Polyline2d> Polyline2dList;
    typedef NBRE_Vector<NBRE_Polyline3d> Polyline3dList;
    friend class NBGM_LayoutElement;
    typedef NBRE_Set<NBRE_WString> FilterSet;
    typedef NBRE_Vector<NBGM_ILayoutProvider*> LayoutProviderList;

public:
    NBGM_LayoutManager(NBGM_Context& nbgmContext, NBRE_Viewport* viewport, uint32 cellSize);
    ~NBGM_LayoutManager();

public:
    /// resize layout buffer
    void ResizeBuffer(uint32 width, uint32 height);
    /// update elements, return TRUE if need more update cycles
    nb_boolean Update(uint32 interval);
    /// update elements, return TRUE if need more update cycles
    nb_boolean UpdateForMap(uint32 interval);
    /// update elements, return TRUE if need more update cycles
    nb_boolean UpdateForNav();
    /// update per frame
    void UpdateVisibleElements();
    /// Get max label count
    uint32 GetMaxLabels() const { return mMaxLabels; }
    /// Set max label count
    void SetMaxLabels(uint32 val) { mMaxLabels = val; }
    /// Get max building and area label count
    uint32 GetMaxAreaBuildingLabels() const { return mMaxAreaBuildingLabels; }
    /// Set max building and area label count
    void SetMaxAreaBuildingLabels(uint32 val) { mMaxAreaBuildingLabels = val; }

    // Internal used only methods.
    // Generally should only be called by classes inherit from NBGM_LayoutElement.

    /// Get camera
    const NBRE_CameraPtr& GetCamera() const { return mViewport->Camera(); }
    /// Get plane side
    nb_boolean IsInNearPlaneFrontSide(const NBRE_Vector3d& position);
    /// Is polyline in near plane front side
    nb_boolean IsInNearPlaneFrontSide(const NBRE_Polyline3d& polyline);
    /// Is point in frustum
    nb_boolean IsInFrustum(const NBRE_Vector3d& position);
    /// Is aabb in frustum
    nb_boolean IsInFrustum(const NBRE_AxisAlignedBox3d& aabb);
    /// Is polyline in frustum
    nb_boolean IsInFrustum(const NBRE_Polyline3d& polyline);
    /// Is aabb in frustum
    nb_boolean IsInFOV(const NBRE_AxisAlignedBox3d& aabb);
    /// Project a point from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition);
    /// Project a point from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition, double& w);
    /// Project a polyline from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Polyline3d& polyline, NBRE_Vector<NBRE_Polyline2d>& screenPolylines);
    /// Project a polyline from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Polyline3d& polyline, double width, int32 lineWidth, uint32 mask, double minPolylineLength, NBGM_LayoutPolylineList& result);
    /// Project a polyline from world to screen, return false if failed (behind near plane)
    nb_boolean WorldToScreen(const NBRE_Polyline3d& polyline, NBGM_LayoutPolylineList& result);
    /// Get pixels per unit at position
    nb_boolean PixelsPerUnit(const NBRE_Vector3d& position, double& pixelsPerUnit);
    /// Get eye position
    const NBRE_Vector3d& GetEyePosition() { return mEyePosition; }
    /// Get camera right
    const NBRE_Vector3d& GetCameraRight() { return mCameraRight; }
    /// Get layout buffer
    NBGM_LayoutBuffer& GetLayoutBuffer() { return mLayoutBuffer; }
    /// Get screen boundary
    const NBRE_AxisAlignedBox2d& GetScreenRect() { return mScreenRect; }
    /// Is name displayed
    nb_boolean IsShieldDisplayed(const NBRE_WString& text);
    /// Is name displayed
    nb_boolean IsRoadLabelDisplayed(const NBRE_WString& text);
    /// Is name displayed
    nb_boolean IsPointLabelDisplayed(const NBRE_WString& text);
    /// Add a label name
    void AddShield(const NBRE_WString& text);
    /// Add a label name
    void AddRoadLabel(const NBRE_WString& text);
    /// Add a label name
    void AddPointLabel(const NBRE_WString& text);
    /// Set is nav mode
    void SetIsNavMode(nb_boolean value) { mIsNavMode = value; }
    /// Get is nav mode
    nb_boolean IsNavMode() { return mIsNavMode; }
    /// Get screen center pixels per unit
    double ScreenCenterPixelsPerUnit() { return mScreenCenterPixelsPerUnit; }
    /// Remove current labels, re-layout
    void Invalidate();
    /// Update layout buffer with polyline
    void UpdateRegion(const NBRE_Polyline3d& polyline, int32 lineWidth, uint32 mask);

    /// add a layout provider in layout manager
    void AddLayoutProvider(NBGM_ILayoutProvider* provider);
    /// remove a layout provider in layout manager
    void RemoveLayoutProvider(NBGM_ILayoutProvider* provider);

    /// Get layout zoom level info
    StaticLayoutLevelInfo& GetLayoutLevels() { return mLayoutLevels; }

    /// remove a element in layout manager
    void RemoveElement(NBGM_LayoutElement* element);

    /// clear all elements
    void RemoveAllElements();

    /// Get visible layout elements in current layout cycle
    NBRE_Vector<NBGM_LayoutElement*>& GetVisibleSet() { return mVisibleElements; }

    const NBRE_String& GetCurrentLayer() const { return mCurrentLayer; }
    void SetCurrentLayer(const NBRE_String& layerId) { mCurrentLayer = layerId; }

private:
    /// add a element in layout manager
    void AddElement(NBGM_LayoutElement* element);

    /// update camera matrix
    void UpdateCameraMatrix();
    /// find visible elements
    void FindVisibleObjects(NBGM_LayoutElementList& pvs);

    void ResetScreenRect();

    void ResizeNameFilterSize();

    void AddNewElement();
    void AddNavNewElement();

    void ResetVisibleElements();

    void CheckPointShieldDistance();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LayoutManager);

private:
    /// nbgm context
    NBGM_Context& mNBGMContext;
    /// a grid to monitor available screen space
    NBGM_LayoutBuffer mLayoutBuffer;
    /// layout providers
    LayoutProviderList mLayoutProviders;
    /// layout elements
    NBGM_LayoutElementList mLayoutElements;
    /// layout elements
    NBGM_LayoutElementList mVisibleElements;
    /// layout elements
    NBGM_LayoutElementList mDisplayElements;
    /// viewport
    NBRE_Viewport* mViewport;
    /// screen rect
    NBRE_AxisAlignedBox2d mScreenRect;
    /// shield name filter
    FilterSet mShieldFilter;
    /// road name filter
    FilterSet mRoadFilter;
    /// area name filter
    FilterSet mAreaBuildingFilter;
    /// max label count
    uint32 mMaxLabels;
    /// max area building labels
    uint32 mMaxAreaBuildingLabels;
    /// label count
    uint32 mLabels;
    /// area building labels
    uint32 mAreaBuildingLabels;
    /// view matrix
    NBRE_Matrix4x4d mViewMatrix;
    /// project matrix
    NBRE_Matrix4x4d mProjectMatrix;
    /// viewport matrix
    NBRE_Matrix4x4d mViewportMatrix;
    /// mvp
    NBRE_Matrix4x4d mViewProjectViewportMatrix;
    /// frustum
    NBRE_Frustum3d mFrustum;
    /// camera direction
    NBRE_Vector3d mEyePosition;
    /// camera right
    NBRE_Vector3d mCameraRight;
    /// is navigation mode(camera always moves)
    nb_boolean mIsNavMode;
    /// last update time, in seconds
    uint32 mLastUpdateTime;
    /// items to check
    NBGM_LayoutElementList mUpdateList;
    /// screen center pixels per unit
    double mScreenCenterPixelsPerUnit;
    /// fov aabb
    NBRE_AxisAlignedBox3d mFovAABB;
    /// refresh timer
    uint32 mNextRefreshTime;
    uint32 mAccumulativeTime;
    uint32 mSecondsFromLastUpdate;
    uint32 mSecondsFromMapViewStartup;

    NBRE_Vector3d mPrevoiusEyePosition;
    nb_boolean mNeedUpdate;
    nb_boolean mResetVisibleElement;
    nb_boolean mHasUpdated;
    nb_boolean mHasAddedElement;
    nb_boolean mStopForAWhile;

    uint32 mMapViewStartupTime;
    uint32 mDeadline;
    uint32 mUpdateIndex;
    uint32 mNow;

    /// mvp
    NBRE_Matrix4x4d mOldViewProjectViewportMatrix;
    uint32 mLastCameraUpdateTime;

    /// Utility to calculate layout zoom levels
    StaticLayoutLevelInfo mLayoutLevels;
    NBRE_String mCurrentLayer;
};


/*! @} */
#endif
