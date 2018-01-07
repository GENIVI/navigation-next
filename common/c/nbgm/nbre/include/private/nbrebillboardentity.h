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

    @file nbreentitynode.h
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
#ifndef _NBRE_BILLBOARD_ENTITY_H_
#define _NBRE_BILLBOARD_ENTITY_H_
#include "nbrenode.h"
#include "nbrevisitor.h"
#include "nbreaxisalignedbox2.h"
#include "nbretexture.h"
#include "nbrevector3.h"
#include "nbreshader.h"
#include "nbremath.h"
#include "nbrefontmaterial.h"
#include "nbretypes.h"
#include "nbretextentity.h"
#include "nbrerenderengine.h"
#include "nbrecontext.h"

class NBRE_VertexData;
class NBRE_IndexData;

/*! \addtogroup NBRE_Scene
*  @{
*/

/** Minimum billboard rendering unit.
@TODO: use sprite sheet to improve performance. 
( Packing texture to a large one and merge vertex buffers. )
*/
class NBRE_TransparentQuad
{
public:
    struct QuadVertex
    {
        NBRE_Vector3f position;
        NBRE_Vector2f texcoord;
        NBRE_Color color;
    };
public:
    NBRE_TransparentQuad(NBRE_Context& context, const NBRE_Vector3f& position, const NBRE_Vector2f& size, NBRE_PassPtr pass, const NBRE_AxisAlignedBox2i& textureRect
        , const NBRE_Color& color, const NBRE_Vector2f& offset);
    ~NBRE_TransparentQuad();

public:
    void UpdateTexture(NBRE_TexturePtr texture, const NBRE_AxisAlignedBox2i& textureRect);
    void UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform);
    void SetColor(const NBRE_Color& color);
    const NBRE_Color& GetColor() {return mColor;}

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TransparentQuad);

private:
    /// render engine context
    NBRE_Context& mContext;
    /// texture coordinates
    NBRE_AxisAlignedBox2i mTextureRect;
    /// vertex data
    NBRE_VertexData* mVertexData;
    /// index data
    NBRE_IndexData* mIndexData;
    /// pass
    NBRE_PassPtr mPassCache;
    /// color
    NBRE_Color mColor;
    /// is color updated
    nb_boolean mNeedUpdateColor;
};
typedef NBRE_Vector<NBRE_TransparentQuad*> NBRE_TransparentQuadList;

/// Billboard elements
class NBRE_BillboardEntity
{
    friend class NBRE_BillboardNode;
public:
    NBRE_BillboardEntity(){}
    virtual ~NBRE_BillboardEntity() {}
    /// Update render queue
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform) = 0;
    virtual const NBRE_AxisAlignedBox3f& RenderBoundingBox() const = 0;
    virtual void SetColor(const NBRE_Color& color) = 0;
    virtual const NBRE_Color& GetColor() = 0;
    virtual void UpdateFontRenderQueue() {}
    virtual void UpdateFontTexture() {}
    virtual void SetVisible(nb_boolean value) = 0;
};
typedef shared_ptr<NBRE_BillboardEntity> NBRE_BillboardEntityPtr;
typedef NBRE_Vector<NBRE_BillboardEntityPtr> NBRE_BillboardEntityList;

/// Text area billboard element
class NBRE_TextAreaEntity:
    public NBRE_BillboardEntity
{
public:
    NBRE_TextAreaEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight, PAL_TEXT_ALIGN align);
    virtual ~NBRE_TextAreaEntity();

public:
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform);
    virtual const NBRE_AxisAlignedBox3f& RenderBoundingBox() const;
    virtual void UpdateFontRenderQueue();
    virtual void UpdateFontTexture();
    virtual void SetColor(const NBRE_Color& color);
    virtual const NBRE_Color& GetColor();
    virtual void SetVisible(nb_boolean value);
    void Rebuild();
    void SetFontMaterial(const NBRE_FontMaterial& fontMaterial);
    /// Set local transform
    void SetTransform(const NBRE_Matrix4x4d& transform) { mTransform = transform; }

    virtual uint32 GetPriority() const { return 0; }
private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextAreaEntity);

private:
    /// render engine context
    NBRE_Context& mContext;
    /// text
    NBRE_TextPointEntity* mEntity;
    /// local transform
    NBRE_Matrix4x4d mTransform;
    /// is element visible
    nb_boolean mVisible;
};

/// Billboard image element
class NBRE_ImageEntity:
    public NBRE_BillboardEntity
{
public:
    NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_Color& color);
    NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_AxisAlignedBox2i& textureRect);
    NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader);
    NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_Vector2f& offset);
    virtual ~NBRE_ImageEntity();

public:
    /// Update render queue
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform);
    virtual const NBRE_AxisAlignedBox3f& RenderBoundingBox() const;
    virtual void SetColor(const NBRE_Color& color);
    virtual const NBRE_Color& GetColor();
    virtual void SetVisible(nb_boolean value);
    void SetTextureRect(const NBRE_AxisAlignedBox2i& textureRect);
    /// Set local transform
    void SetTransform(const NBRE_Matrix4x4f& transform) { mTransform = transform; }
    const NBRE_Vector2f& GetOffset() { return mOffset; }

private:
    void CreateVertexBuffers();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_ImageEntity);

private:
    NBRE_Context& mContext;
    /// billboard width and height in 3d space
    NBRE_Vector2f mSize;
    /// shader of billboard
    NBRE_ShaderPtr mShader;
    /// texture coordinates
    NBRE_AxisAlignedBox2i mTextureRect;
    /// vertex data
    NBRE_TransparentQuad* mQuad;
    /// bounding box
    NBRE_AxisAlignedBox3f mAABB;
    /// local transform
    NBRE_Matrix4x4f mTransform;
    /// is element visible
    nb_boolean mVisible;
    /// percentage offset from image center
    NBRE_Vector2f mOffset;

};

/*! @} */
#endif
