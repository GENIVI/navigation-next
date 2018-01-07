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

    @file nbgmstaticpoi.h
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

#ifndef _NBGM_STATIC_POI_H_
#define _NBGM_STATIC_POI_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"
#include "nbgmcontext.h"
#include "nbreentity.h"
#include "nbgmnbmdata.h"

class NBGM_StaticPoiLabel;
/*! \addtogroup NBGM_Service
*  @{
*/

/** A static poi contains a icon and optional label.
Click on it will show a bubble.
*/
class NBGM_StaticPoi:
    public NBGM_LayoutElement
{
public:
    NBGM_StaticPoi(NBGM_Context& nbgmContext,
              NBRE_Node* parentNode,
              NBRE_SurfaceSubView* subView,
              int32 overlayId,
              const NBRE_Vector2f& size,
              NBRE_ShaderPtr shader,
              NBRE_ShaderPtr selectedShader,
              NBRE_ShaderPtr unselectedShader,
              const NBRE_Vector3d& position,
              const NBRE_String& id,
              nb_boolean showLabel,
              const NBRE_Vector2f& imageOffset,
              const NBRE_Vector2f& bubbleOffset,
              const NBRE_String& text,
              const NBRE_FontMaterial& fontMaterial,
              uint32 materialId,
              float distanceToPoi,
              float distanceToLabel,
              float distanceToAnother,
              int32 labelPriority);
    virtual ~NBGM_StaticPoi();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();
    /// Set priority
    void SetPriority(int32 value) { mPriority = value; }
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& color);
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Get icon id
    const NBRE_String& GetId() const { return mId; }
    /// Is selected
    nb_boolean GetSelected() const { return mSelected; }
    /// Set select state
    void SetSelected(nb_boolean selected);
    /// Get icon screen position, if success, return TRUE and set position,
    /// else return FALSE
    nb_boolean GetScreenPosition(NBRE_Vector2f& position);
    /// Get icon bubble screen position for this icon, if success,
    /// return TRUE and set position, else return FALSE
    nb_boolean GetScreenBubblePosition(NBRE_Vector2f& position);
    /// return bubble offset from anchor points, in pixels
    NBRE_Vector2f GetScreenBubbleOffset();
    /// Is icon within radius around point
    nb_boolean HitTest(const NBRE_Vector2f& screenPosition, float radius);
    /// Is element too close
    nb_boolean IsTooCloseTo(NBGM_LayoutElement* elem);
    /// Update icon scale
    virtual void OnUpdate();
    /// Refresh element material
    virtual void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    /// Refresh font material list
    virtual void GetFontMaterialIds(NBRE_Set<uint32>& materialIds);
    /// Get minimum distance to poi
    float GetDistanceToPoi() const { return mDistanceToPoi; }
    /// Get minimum distance to label
    float GetDistanceToLabel() const { return mDistanceToLabel; }
    /// Get minimum distance to others
    float GetDistanceToAnother() const { return mDistanceToAnother; }
    /// Get poi label
    NBGM_StaticPoiLabel* GetLabel() { return mPoiLabel; }

private:
    /// Adjust text scale to font min/max range
    double UpdateScale(double pixelsPerUnit);
    /// Update screen rect
    void UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit);


private:
    DISABLE_COPY_AND_ASSIGN(NBGM_StaticPoi);
    friend class NBGM_StaticPoiLabel;

private:
    /// surface view
    NBRE_SurfaceSubView* mSubView;
    /// overlay id
    int32 mOverlayId;
    /// Pixel size
    NBRE_Vector2f mSize;
    /// Text node
    NBRE_BillboardNodePtr mNode;
    /// World position
    NBRE_Vector3d mPosition;
    /// Is label visible
    nb_boolean mIsInFrustum;
    /// Scale for size limit
    double mScale;
    /// Icon id
    NBRE_String mId;
    /// Is selected
    nb_boolean mSelected;
    /// parent node
    NBRE_Node* mParentNode;
    /// Shader
    NBRE_ShaderPtr mShader;
    /// Shader when selected
    NBRE_ShaderPtr mSelectedShader;
    /// Shader when not selected
    NBRE_ShaderPtr mUnselectedShader;
    /// Base size
    double mBaseSize;
    /// Is label displayed
    nb_boolean mShowLabel;
    /// Offset(%)
    NBRE_Vector2f mImageOffset;
    /// bubble offset(%)
    NBRE_Vector2f mBubbleOffset;
    /// label material
    NBRE_FontMaterial mFontMaterial;
    /// font material id
    uint32 mFontMaterialId;
    /// is label shown
    nb_boolean mLabelAvailable;
    /// space between elements
    float mDistanceToPoi;
    /// space between elements
    float mDistanceToLabel;
    /// space between elements
    float mDistanceToAnother;
    /// poi label
    NBGM_StaticPoiLabel* mPoiLabel;
    /// poi original priority
    int32 mOriginalPriority;
};
typedef shared_ptr<NBGM_StaticPoi> NBGM_StaticPoiPtr;

/** Class for poi label. A poi label can show on four alternative positions.
* Label is a separated class because it has different layout priority.
* However it can't show without poi icon.
*/
class NBGM_StaticPoiLabel:
    public NBGM_LayoutElement
{
public:
    NBGM_StaticPoiLabel(NBGM_Context& nbgmContext,
        NBGM_StaticPoi* poi,
        const NBRE_String& text,
        const NBRE_FontMaterial& fontMaterial,
        uint32 materialId);
    virtual ~NBGM_StaticPoiLabel();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible() { return TRUE; }
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition() { return mPoi->GetPosition(); }
    /// Set priority
    void SetPriority(int32 value) { mPriority = value; }
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& /*color*/) {}
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Refresh element material
    virtual void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    /// Refresh font material list
    virtual void GetFontMaterialIds(NBRE_Set<uint32>& materialIds);
    /// Point hit test
    nb_boolean HitTest(const NBRE_Vector2d& screenPosition);
    /// Update screen rect
    void UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit);
    /// Is selected
    nb_boolean GetSelected() const { return mSelected; }
    /// Set select state
    void SetSelected(nb_boolean selected);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_StaticPoiLabel);

    enum LabelPlacement
    {
        LP_NONE,
        LP_TOP,
        LP_BOTTOM,
        LP_LEFT,
        LP_RIGHT
    };
    /// Get label offset
    NBRE_Vector2d GetLabelOffset(LabelPlacement placement);
private:
    NBGM_StaticPoi* mPoi;
    /// label offset
    NBRE_Vector2d mLabelOffset;
    /// label place
    LabelPlacement mLabelPlace;
    /// text entities for static pois, with center/left/right alignment
    NBRE_TextAreaEntity* mTextAreaEntity[3];
    /// label material
    NBRE_FontMaterial mFontMaterial;
    /// font material id
    uint32 mFontMaterialId;
    /// label base size
    NBRE_Vector2f mBaseStringSize;
    /// Is selected
    nb_boolean mSelected;
    /// poi label original priority
    int32 mOriginalPriority;
};

/*! @} */
#endif
