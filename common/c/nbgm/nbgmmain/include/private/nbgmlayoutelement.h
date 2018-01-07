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

    @file nbgmlayoutelement.h
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
#ifndef _NBGM_LAYOUT_ELEMENT_H_
#define _NBGM_LAYOUT_ELEMENT_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrecolor.h"
#include "nbgmcontext.h"
#include "nbrecommon.h"
#include "nbreaxisalignedbox2.h"
#include "nbreaxisalignedbox3.h"
#include "nbrefontmaterial.h"

// Note: uncomment this to see debug info
//#define ENABLE_LABEL_DEBUG

class NBRE_Node;
class NBGM_LayoutBuffer;
/*! \addtogroup NBGM_Service
*  @{
*/

enum NBGM_LayoutElementType
{
    NBGM_LET_UNKNOWN,
    NBGM_LET_SKY,
    NBGM_LET_MAP_ROAD_LABEL,
    NBGM_LET_NAV_ROAD_LABEL,
    NBGM_LET_SHIELD,
    NBGM_LET_POINT_LABEL,
    NBGM_LET_POINT_SHIELD,
    NBGM_LET_AREA_LABEL,
    NBGM_LET_BUILDING_LABEL,
    NBGM_LET_PIN,
    NBGM_LET_POI,
    NBGM_LET_STATIC_POI,
    NBGM_LET_STATIC_POI_LABEL,
    NBGM_LET_CUSTOM_PIN
};

// Conflict update & check mask
#define MAP_ROAD_LABEL_UPDATE_MASK      0x4
#define POINT_LABEL_UPDATE_MASK         0x1
#define SHIELD_UPDATE_MASK              0x1
#define NAV_ROAD_LABEL_UPDATE_MASK      0x1
#define NAV_ROUTE_UPDATE_MASK           0x2
#define NAV_MANEUVER_UPDATE_MASK        0x8
#define STATIC_POI_UPDATE_MASK          0x10
#define CUSTOM_PIN_UPDATE_MASK          0x20
#define POINT_WATERMARK_UPDATE_MASK     0x0
#define SKY_UPDATE_MASK                 (MAP_ROAD_LABEL_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|NAV_ROAD_LABEL_UPDATE_MASK|NAV_ROUTE_UPDATE_MASK|STATIC_POI_UPDATE_MASK)

#define MAP_ROAD_LABEL_CHECK_MASK       (POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|STATIC_POI_UPDATE_MASK)
#define POINT_LABEL_CHECK_MASK          (MAP_ROAD_LABEL_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|NAV_ROAD_LABEL_UPDATE_MASK|STATIC_POI_UPDATE_MASK)
#define SHIELD_CHECK_MASK               (MAP_ROAD_LABEL_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|NAV_ROAD_LABEL_UPDATE_MASK|NAV_MANEUVER_UPDATE_MASK|STATIC_POI_UPDATE_MASK)
#define NAV_ROAD_LABEL_CHECK_MASK       (NAV_ROUTE_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|STATIC_POI_UPDATE_MASK)
#define POINT_WATERMARK_CHECK_MASK      0x0
#define STATIC_POI_CHECK_MASK           (MAP_ROAD_LABEL_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|NAV_ROAD_LABEL_UPDATE_MASK|STATIC_POI_UPDATE_MASK)
#define STATIC_POI_LABEL_CHECK_MASK     (MAP_ROAD_LABEL_UPDATE_MASK|POINT_LABEL_UPDATE_MASK|SHIELD_UPDATE_MASK|NAV_ROAD_LABEL_UPDATE_MASK)
#define CUSTOM_PIN_CHECK_MASK           (CUSTOM_PIN_UPDATE_MASK)

#define HIGHEST_PRIORITY                0x80000000
/** Any element which cannot overlap with each other should implement this interface
and add to layout manager.
*/
class NBGM_LayoutElement
{
    friend class NBGM_LayoutManager;
public:
    NBGM_LayoutElement(NBGM_Context& nbgmContext);
    NBGM_LayoutElement(NBGM_Context& nbgmContext, const NBRE_WString& text);
    virtual ~NBGM_LayoutElement();
public:
    /// Is element visible(in frustum)
    virtual nb_boolean IsElementVisible() = 0;
    /// Update layout buffer region where element taken
    virtual void UpdateRegion() = 0;
    /// Is element region in layout buffer free to use
    virtual nb_boolean IsElementRegionAvailable() = 0;
    /// Get position in world space, the elements near the camera will be displayed first
    virtual NBRE_Vector3d GetPosition() = 0;
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& /*color*/) {}
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Per frame element update
    virtual void OnUpdate() {}
    /// Set element visibility
    virtual void SetVisible(nb_boolean /*value*/) {}
    /// Refresh element material
    virtual void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& /*materials*/) {}
    /// Refresh font material list
    virtual void GetFontMaterialIds(NBRE_Set<uint32>& /*materialIds*/) {}
    /// Get clone element
    virtual NBGM_LayoutElement* Clone(NBRE_Node* /*parentNode*/) { return NULL; }
    /// Get text
    virtual const NBRE_WString& GetText() { return mText; }
    /// Get screen rect
    virtual const NBRE_AxisAlignedBox2d& GetScreenRect() { return mScreenRect; }
    /// Get layer id. No collision detection between different layers
    virtual const NBRE_String& GetLayerId() const { return EMPTY_LAYER_ID; }

    virtual void FindConflictLabels(NBRE_Vector<NBGM_LayoutElement*>& /*conflictedLables*/){}
    virtual void ClearRegionInLayoutBuffer(){}
    virtual void CreateConflictLabel(){}
    virtual nb_boolean CalculateScreenRectRegion(NBRE_AxisAlignedBox2d& /*rect*/){return TRUE;}
    virtual void UpdateRegion(const NBRE_AxisAlignedBox2d& /*box*/){}

    /// Remove item from layout visible set
    void RemoveFromVisibleSet();
    /// Get squared distance to eye
    double GetSquaredDistanceToEye() const { return mSquaredDistanceToEye; }
    /// Get visible seconds
    uint32 GetDrawIndex() const { return mDrawIndex; }
    /// Enable element
    void SetEnable(nb_boolean value);
    /// Is element enabled
    nb_boolean IsEnable() const { return mEnable; }
    /// Set type id
    void SetTypeId(NBGM_LayoutElementType value) { mTypeId = value; }
    /// Get type id
    NBGM_LayoutElementType GetTypeId() { return mTypeId; }

    /// Priority, less value has higher priority
    int32 GetLayoutPriority() const {return mPriority;}
    void SetLayoutPriority(int32 value) {mPriority = value;}

    virtual void Reshape() {}

    double GetMinPixelsPerUnit() { return mMinPixelPerUnit; }
    const NBRE_AxisAlignedBox3d& GetAABB() { return mBoundingBox; }

    void EnableLayout(nb_boolean value) { mEnableLayout = value; }
    nb_boolean IsLayoutEnabled() { return mEnableLayout; }
    const NBRE_AxisAlignedBox2d& GetScreenRect() const { return mScreenRect; }

    void InvalidateLayout();

public:
    static const uint32 INVALID_DRAW_INDEX;
    static const NBRE_String EMPTY_LAYER_ID;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LayoutElement);

protected:
    NBGM_Context& mNBGMContext;
    int32 mPriority;
    NBRE_WString mText;
    NBRE_AxisAlignedBox2d mScreenRect;

    float mMinPixelPerUnit;
    NBRE_AxisAlignedBox3d mBoundingBox;

private:
    double mSquaredDistanceToEye;
    uint32 mVisibleMs;
    uint32 mDrawIndex;
    nb_boolean mEnable;
    uint32 mUpdateTime;
    uint32 mLastLayoutTime;
    NBGM_LayoutElementType mTypeId;
    nb_boolean mEnableLayout;
};
typedef NBRE_Vector<NBGM_LayoutElement*> NBGM_LayoutElementList;

/*! @} */
#endif
