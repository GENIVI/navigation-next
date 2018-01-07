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

    @file nbgmroadlabel.h
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

#ifndef _NBGM_ROAD_LABEL_H_
#define _NBGM_ROAD_LABEL_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbgmlayoutmanager.h"
#include "nbrepolyline2.h"
#include "nbrepolyline3.h"
#include "nbretextentity.h"
#include "nbgmpolyline2entity.h"
#include "nbreentity.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** road label displayed along a polyline.
*/
class NBGM_RoadLabel:
    public NBGM_LayoutElement
{
    struct PolylineEntityVertex
    {
        NBRE_Vector3d position;
        NBRE_Color color;
    };

public:
    NBGM_RoadLabel(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, const NBRE_Polyline2f& polyline, float width);
    virtual ~NBGM_RoadLabel();

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& color);
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Refresh element material
    virtual void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    /// Refresh font material list
    virtual void GetFontMaterialIds(NBRE_Set<uint32>& materialIds);
    /// Get font material
    const NBRE_FontMaterial& GetFontMaterial() { return mFontMaterial; }
    /// Get track width
    float GetWidth() { return mWidth; }
    /// Get polyline
    NBRE_Polyline2f GetPolyline();
    /// On update
    virtual void OnUpdate();

private:
    nb_boolean IsRevert();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_RoadLabel);

private:
    /// surface view
    NBRE_SurfaceSubView* mSubView;
    /// overlay manager
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    /// text overlay
    int32 mOverlayId;
    /// world polyline
    NBRE_Polyline3d mPolyline;
    /// anchor point
    NBRE_Vector3d mPosition;
    /// Font height
    float mFontHeight;
    /// Font material
    NBRE_FontMaterial mFontMaterial;
    /// font material id
    uint32 mFontMaterialId;
    /// text entity
    NBRE_TextPathEntity* mTextPathEntity;
    /// align
    NBRE_FontAlign mAlign;
    /// road width
    float mWidth;
    /// try to display label close to this point
    NBRE_Vector2d mLayoutRefPoint;
    /// layout polyline
    NBGM_LayoutPolyline mLayoutPolyline;
    /// screen polyline
    NBRE_Polyline2f mScreenPolyline;
    /// layout polyline offset
    double mLayoutOffset[2];
    /// transparency
    float mAlpha;
    /// is visible
    nb_boolean mVisible;
    /// text direction
    NBRE_Vector2f mTextDirection;
    /// Parent Node
    NBRE_Node* mParentNode;
    /// string width
    float mBaseStringWidth;
};

/*! @} */
#endif
