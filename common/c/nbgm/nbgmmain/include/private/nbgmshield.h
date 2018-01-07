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

    @file nbgmshield.h
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

#ifndef _NBGM_SHIELD_H_
#define _NBGM_SHIELD_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"
#include "nbrepolyline3.h"
#include "nbreentity.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** A label displayed in the center of anchor point.
such as area & building labels.
*/
class NBGM_Shield:
    public NBGM_LayoutElement
{
public:
    NBGM_Shield(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, float fontHeight, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline);
    NBGM_Shield(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, float fontHeight, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Vector3d& position, nb_boolean needUpdate);
    virtual ~NBGM_Shield();

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
    /// Per frame element update
    virtual void OnUpdate();

private:
    /// Create track entity
    void CreateTrackEntity(int32 overlayId);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Shield);

private:
    /// surface view
    NBRE_SurfaceSubView* mSubView;
    /// overlay id
    int32 mOverlayId;
    /// Shield node
    NBRE_BillboardNodePtr mNode;
    /// Shield position in world space
    NBRE_Vector3d mPosition;
    /// Shield position in world space
    NBRE_Polyline3d* mPolyline;
    /// Font height
    float mFontHeight;
    /// Font material
    NBRE_FontMaterial mFontMaterial;
    /// font material id
    uint32 mFontMaterialId;
    /// Is shield init
    nb_boolean mIsInitialized;
    /// Shield scale
    double mScale;
    /// Shield transparency
    float mAlpha;
    /// parent node
    NBRE_Node* mParentNode;
    /// texture image
    NBRE_ShaderPtr mImage;
    /// texture size
    NBRE_Vector2f mTextureSize;
    /// offset
    NBRE_Vector2f mOffset;
    /// is visible
    nb_boolean mVisible;
    /// text entity
    NBRE_TextAreaEntity* mTextAreaEntity;
    /// need upate texture size
    nb_boolean mNeedUpdate;
    /// layout ref point
    NBRE_Vector2d mLayoutRefPoint;
};
typedef shared_ptr<NBGM_Shield> NBGM_ShieldPtr;
/*! @} */
#endif
