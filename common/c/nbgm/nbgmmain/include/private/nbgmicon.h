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

    @file nbgmicon.h
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

#ifndef _NBGM_ICON_H_
#define _NBGM_ICON_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutmanager.h"
#include "nbgmcontext.h"
#include "nbreentity.h"
#include "nbgmbinarybuffer.h"
#include "nbgmmapmaterialutility.h"
#include "nbgmmaplayer.h"

/*! \addtogroup NBGM_Service
*  @{
*/

/** A label displayed in the center of anchor point.
such as area & building labels.
*/
class NBGM_Icon:
    public NBGM_LayoutElement
{
public:
    NBGM_Icon(NBGM_Context& nbgmContext,
              NBRE_Node* parentNode,
              NBRE_BillboardSet* billboardSet,
              const NBRE_Vector2f& size,
              const NBRE_Vector2f& selectedTexsize,
              NBRE_ShaderPtr shader,
              NBRE_ShaderPtr selectedShader,
              const NBRE_Vector3d& position,
              const NBRE_String& id,
              nb_boolean isPin,
              const NBRE_Vector2f& imageOffset,
              const NBRE_Vector2f& selectedImageOffset,
              const NBRE_Vector2f& bubbleOffset);

    NBGM_Icon(NBGM_Context& nbgmContext,
              NBRE_Node* parentNode,
              NBRE_BillboardSet* billboardSet,
              const NBRE_Vector2f& size,
              const NBRE_Vector2f& selectedTexsize,
              const NBRE_Vector3d& position,
              const NBRE_String& id,
              nb_boolean isPin,
              const NBRE_Vector2f& imageOffset,
              const NBRE_Vector2f& selectedImageOffset,
              const NBRE_Vector2f& bubbleOffset);
    virtual ~NBGM_Icon();

public:
    void Initialize(const NBGM_BinaryBuffer& unSelectedTextureBuffer,
                    const NBGM_BinaryBuffer& selectedTextureBuffer,
                    NBRE_Color unSelectedCircleInteriorColor,
                    NBRE_Color unSelectedCircleOutlineBitOnColor,
                    NBRE_Color unSelectedCircleOutlineBitOffColor,
                    NBRE_Color selectedCircleInteriorColor,
                    NBRE_Color selectedCircleOutlineBitOnColor,
                    NBRE_Color selectedCircleOutlineBitOffColor,
                    uint8 circleOutlineWidth,
                    float radius
                    );

    /// Initialize Halo
    void InitializeHalo(const NBRE_ShaderPtr& interiorShader, const NBRE_ShaderPtr& outlineShader, float radias);

public:
    /// Is element visible
    virtual nb_boolean IsElementVisible();
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable();
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition();
    /// Set icon position
    virtual void SetPosition(const NBRE_Vector3d& position);
    /// Set priority
    void SetPriority(int32 value) { mPriority = value; }
    /// Implement this to provide fade in/out effect
    virtual void SetColor(const NBRE_Color& color);
    /// Add to Scene for rendering
    virtual void AddToSence();
    /// Set element visibility
    virtual void SetVisible(nb_boolean value);
    /// Get clone element
    virtual NBGM_LayoutElement* Clone(NBRE_Node* parentNode);

    virtual nb_boolean CalculateScreenRectRegion(NBRE_AxisAlignedBox2d& rect);
    /// Get icon id
    const NBRE_String& GetId() const { return mId; }
    /// Is icon selected
    nb_boolean GetSelected() const { return mSelected; }
    /// Set icon select state
    void SetSelected(nb_boolean val);
    /// Get icon screen position, if success, return TRUE and set position,
	/// else return FALSE
    nb_boolean GetScreenPosition(NBRE_Vector2f& position);
    /// Get icon bubble screen position for this icon, if success,
	/// return TRUE and set position, else return FALSE
    nb_boolean GetScreenBubblePosition(NBRE_Vector2f& position);

    nb_boolean GetScreenBottomPosition(NBRE_Vector2f& position);
    /// Is icon at point
    nb_boolean HitTest(const NBRE_Vector2f& screenPosition);
    /// Update icon scale
    void Update();
    /// Notify Icon extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
private:
    /// Adjust text scale to font min/max range
    double UpdateScale(double pixelsPerUnit);
    /// Update screen rect
    void UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit);
    /// Get the node in current frustum
    NBRE_BillboardNodePtr GetNodeInFrustum() const;
private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Icon);

private:
    /// Text node container
    NBRE_BillboardSet* mBillboardSet;
    /// Pixel size
    NBRE_Vector2f mSize;
    /// Pixel size
    NBRE_Vector2f mSelectedTexSize;
    /// Text node
    NBRE_BillboardNodePtr mNode;
    /// Text node
    NBRE_BillboardNodePtr mExtendNode;
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
    /// Base size
    double mBaseSize;
    /// Is pin
    nb_boolean mIsPin;
    /// Offset(%)
    NBRE_Vector2f mImageOffset;
    NBRE_Vector2f mSelectedImageOffset;
    NBRE_Vector2f mBubbleOffset;
    /// has halo
    nb_boolean mHasHalo;
    /// halo node
    NBRE_NodePtr mHaloNode;
    /// halo entity
    NBRE_EntityPtr mHaloEntity;
    /// halo node
    NBRE_NodePtr mExtendHaloNode;
    /// halo entity
    NBRE_EntityPtr mExtendHaloEntity;
    /// halo radius
    float mHaloRadius;
    /// halo interior shader
    NBRE_ShaderPtr mHaloInteriorShader;
    /// halo outline shader
    NBRE_ShaderPtr mHaloOutlineShader;
    /// extend type
    NBGM_TileMapBoundaryExtendType mExtendType;
    /// Father node
    NBRE_NodePtr mFatherNode;
};
typedef shared_ptr<NBGM_Icon> NBGM_IconPtr;
/*! @} */
#endif
