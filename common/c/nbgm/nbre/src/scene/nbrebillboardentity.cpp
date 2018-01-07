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

#include "nbrebillboardentity.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbrefontmanager.h"

NBRE_TransparentQuad::NBRE_TransparentQuad(NBRE_Context& context, const NBRE_Vector3f& position, const NBRE_Vector2f& size, NBRE_PassPtr pass, const NBRE_AxisAlignedBox2i& textureRect
                                           , const NBRE_Color& color, const NBRE_Vector2f& offset)
    :mContext(context)
    ,mTextureRect(textureRect)
    ,mVertexData(NULL)
    ,mIndexData(NULL)
    ,mNeedUpdateColor(FALSE)
{
    mPassCache = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    *mPassCache.get() = *pass.get();
    NBRE_TexturePtr texture = pass->GetTextureUnits()[0].GetTexture(mContext);
    mPassCache->SetEnableBlend(TRUE);
    mPassCache->SetEnableAlphaTest(FALSE);
    mPassCache->SetEnableDepthTest(FALSE);
    mPassCache->SetEnableDepthWrite(FALSE);

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 3, 4, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
    NBRE_HardwareVertexBuffer* texcoordBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
    NBRE_HardwareVertexBuffer* colorBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 4, 4, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);


    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texcoordElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texcoordElem1);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->AddBuffer(texcoordBuff);
    mVertexData->AddBuffer(colorBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 6, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    NBRE_Vector3f* pVertex = static_cast<NBRE_Vector3f*>(vertextBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Vector2f* pTexcoord = static_cast<NBRE_Vector2f*>(texcoordBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Color* pColor = static_cast<NBRE_Color*>(colorBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(indexBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    NBRE_Vector3f* pV = pVertex;
    NBRE_Vector2f* pT = pTexcoord;
    NBRE_Color* pC = pColor;

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;
    // set vertex positions
    float dx = size.x * offset.x;
    float dy = size.y * offset.y;
    pV[0].x = position.x - halfW + dx; pV[0].y = position.y - halfH + dy;
    pV[1].x = position.x + halfW + dx; pV[1].y = position.y - halfH + dy;
    pV[2].x = position.x + halfW + dx; pV[2].y = position.y + halfH + dy;
    pV[3].x = position.x - halfW + dx; pV[3].y = position.y + halfH + dy;

    if (texture.get() != NULL)
    {
        // set vertex texture coordinates
        float tw = static_cast<float>(texture->GetWidth());
        float th = static_cast<float>(texture->GetHeight());
        float s0 = mTextureRect.minExtend.x / tw;
        float t0 = mTextureRect.minExtend.y / th;
        float s1 = (mTextureRect.maxExtend.x + 1.0f) / tw;
        float t1 = (mTextureRect.maxExtend.y + 1.0f) / th;
        pT[0].x = s0; pT[0].y = t0;
        pT[1].x = s1; pT[1].y = t0;
        pT[2].x = s1; pT[2].y = t1;
        pT[3].x = s0; pT[3].y = t1;
    }

    // set vertex colors
    pC[0] = color;
    pC[1] = color;
    pC[2] = color;
    pC[3] = color;

    *pIndex++ = 0;
    *pIndex++ = 1;
    *pIndex++ = 2;
    *pIndex++ = 0;
    *pIndex++ = 2;
    *pIndex++ = 3;

    vertextBuff->UnLock();
    texcoordBuff->UnLock();
    colorBuff->UnLock();
    indexBuff->UnLock();
}

void
NBRE_TransparentQuad::UpdateTexture(NBRE_TexturePtr texture, const NBRE_AxisAlignedBox2i& textureRect)
{   
    mPassCache->GetTextureUnits()[0].SetTexture(texture);
    mTextureRect = textureRect;
    // Update buffer data
    NBRE_HardwareVertexBuffer* texcoordBuff = mVertexData->GetBuffer(1);
    NBRE_Vector2f* pTexcoord = static_cast<NBRE_Vector2f*>(texcoordBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Vector2f* pT = pTexcoord;
    // set vertex texture coordinates
    float tw = static_cast<float>(texture->GetWidth());
    float th = static_cast<float>(texture->GetHeight());
    float s0 = mTextureRect.minExtend.x / tw;
    float t0 = mTextureRect.minExtend.y / th;
    float s1 = (mTextureRect.maxExtend.x + 1.0f) / tw;
    float t1 = (mTextureRect.maxExtend.y + 1.0f) / th;
    pT[0].x = s0; pT[0].y = t0;
    pT[1].x = s1; pT[1].y = t0;
    pT[2].x = s1; pT[2].y = t1;
    pT[3].x = s0; pT[3].y = t1;
    texcoordBuff->UnLock();
}

NBRE_TransparentQuad::~NBRE_TransparentQuad()
{
    NBRE_DELETE mVertexData;
    mVertexData = NULL;
    NBRE_DELETE mIndexData;
    mIndexData = NULL;
}

void
NBRE_TransparentQuad::UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform)
{
    if (mNeedUpdateColor)
    {
        // Update buffer data
        NBRE_HardwareVertexBuffer* colorBuff = mVertexData->GetBuffer(2);
        NBRE_Color* pC = static_cast<NBRE_Color*>(colorBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
        // set vertex texture coordinates
        pC[0] = mColor;
        pC[1] = mColor;
        pC[2] = mColor;
        pC[3] = mColor;
        colorBuff->UnLock();
        mNeedUpdateColor = FALSE;
    }
    // Add render data to overlay
    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();
    renderOp.SetVertexData(mVertexData);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_TRIANGLE_LIST);
    renderOp.SetPass(mPassCache.get());
    renderOp.SetTransform(&transform);
}

void
NBRE_TransparentQuad::SetColor(const NBRE_Color& color)
{
    if (mColor == color)
    {
        return;
    }
    mColor = color;
    mNeedUpdateColor = TRUE;
}

//************************************NBRE_TextAreaEntity***********************************
NBRE_TextAreaEntity::NBRE_TextAreaEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight, PAL_TEXT_ALIGN align)
:mContext(context),mVisible(TRUE)
{
    mEntity = NBRE_NEW NBRE_TextPointEntity(mContext, text, fontMaterial, fontHeight, NBRE_Vector2f(0, 0), align);
    mEntity->GetPass()->SetEnableDepthTest(TRUE);
}

NBRE_TextAreaEntity::~NBRE_TextAreaEntity()
{
    NBRE_DELETE mEntity;
}

void
NBRE_TextAreaEntity::UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform)
{
    if (!mVisible)
    {
        return;
    }

    mEntity->SetTransform(transform * mTransform);
    mEntity->UpdateRenderQueue(overlay);
}

const NBRE_AxisAlignedBox3f& 
NBRE_TextAreaEntity::RenderBoundingBox() const
{
    return mEntity->AxisAlignedBox();
}

void 
NBRE_TextAreaEntity::UpdateFontTexture()
{
    if (!mVisible)
    {
        return;
    }
    mEntity->UpdateFontTexture();
}

void
NBRE_TextAreaEntity::UpdateFontRenderQueue()
{
    if (!mVisible)
    {
        return;
    }
    mEntity->UpdateFontRenderQueue();
}

void 
NBRE_TextAreaEntity::SetColor(const NBRE_Color& color)
{
    mEntity->SetColor(color);
}

const NBRE_Color& 
NBRE_TextAreaEntity::GetColor()
{
    return mEntity->GetColor();
}

void 
NBRE_TextAreaEntity::SetVisible(nb_boolean value)
{
    mVisible = value;
}

void
NBRE_TextAreaEntity::Rebuild()
{
    mEntity->Rebuild();
}

void
NBRE_TextAreaEntity::SetFontMaterial(const NBRE_FontMaterial& fontMaterial)
{
    mEntity->SetFontMaterial(fontMaterial);
}

//************************************NBRE_ImageEntity***********************************
NBRE_ImageEntity::NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_Color& color)
    :mContext(context)
    ,mSize(size)
    ,mShader(shader)
    ,mVisible(TRUE)
{
    nbre_assert(mShader->GetPassCount() > 0);
    NBRE_PassPtr pass = mShader->GetPass(0);
    nbre_assert(pass->GetTextureUnits().size() > 0);
    NBRE_TexturePtr texture = pass->GetTextureUnits()[0].GetTexture(mContext);
    mTextureRect.minExtend.x = 0;
    mTextureRect.minExtend.y = 0;
    mTextureRect.maxExtend.x = texture->GetWidth() - 1;
    mTextureRect.maxExtend.y = texture->GetHeight() - 1;
    CreateVertexBuffers();
    SetColor(color);
}

NBRE_ImageEntity::NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_AxisAlignedBox2i& textureRect)
    :mContext(context)
    ,mSize(size)
    ,mShader(shader)
    ,mTextureRect(textureRect)
    ,mVisible(TRUE)
{
    CreateVertexBuffers();
}

NBRE_ImageEntity::NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader)
    :mContext(context)
    ,mSize(size)
    ,mShader(shader)
    ,mVisible(TRUE)
{
    nbre_assert(mShader->GetPassCount() > 0);
    NBRE_PassPtr pass = mShader->GetPass(0);
    nbre_assert(pass->GetTextureUnits().size() > 0);
    NBRE_TexturePtr texture = pass->GetTextureUnits()[0].GetTexture(mContext);
    mTextureRect.minExtend.x = 0;
    mTextureRect.minExtend.y = 0;
    mTextureRect.maxExtend.x = texture->GetWidth() - 1;
    mTextureRect.maxExtend.y = texture->GetHeight() - 1;
    CreateVertexBuffers();
}

NBRE_ImageEntity::NBRE_ImageEntity(NBRE_Context& context, const NBRE_Vector2f& size, NBRE_ShaderPtr shader, const NBRE_Vector2f& offset)
    :mContext(context)
    ,mSize(size)
    ,mShader(shader)
    ,mVisible(TRUE)
    ,mOffset(offset)
{
    nbre_assert(mShader->GetPassCount() > 0);
    NBRE_PassPtr pass = mShader->GetPass(0);
    nbre_assert(pass->GetTextureUnits().size() > 0);
    NBRE_TexturePtr texture = pass->GetTextureUnits()[0].GetTexture(mContext);
    mTextureRect.minExtend.x = 0;
    mTextureRect.minExtend.y = 0;
    mTextureRect.maxExtend.x = texture->GetWidth() - 1;
    mTextureRect.maxExtend.y = texture->GetHeight() - 1;
    CreateVertexBuffers();
}

NBRE_ImageEntity::~NBRE_ImageEntity()
{
    NBRE_DELETE mQuad;
}

void
NBRE_ImageEntity::UpdateRenderQueue(NBRE_Overlay& overlay, const NBRE_Matrix4x4d& transform)
{
    if (!mVisible)
    {
        return;
    }
    mQuad->UpdateRenderQueue(overlay, transform);
}

const NBRE_AxisAlignedBox3f& 
NBRE_ImageEntity::RenderBoundingBox() const
{
    return mAABB;
}

void
NBRE_ImageEntity::CreateVertexBuffers()
{
    nbre_assert(mShader->GetPassCount() > 0);
    NBRE_PassPtr pass = mShader->GetPass(0);
    nbre_assert(pass->GetTextureUnits().size() > 0);
    //NBRE_TexturePtr texture = pass->GetTextureUnits()[0].GetTexture();
    mQuad = NBRE_NEW NBRE_TransparentQuad(mContext, NBRE_Vector3f(0, 0, 0), mSize, pass, mTextureRect
        , NBRE_Color(1,1,1,1), mOffset);

    float halfW = mSize.x * 0.5f;
    float halfH = mSize.y * 0.5f;
    mAABB.minExtend.x = -halfW + mOffset.x * mSize.x;
    mAABB.minExtend.y = -halfH + mOffset.y * mSize.y;
    mAABB.minExtend.z = 0;
    mAABB.maxExtend.x = halfW + mOffset.x * mSize.x;
    mAABB.maxExtend.y = halfH + mOffset.y * mSize.y;
    mAABB.maxExtend.z = 0;
}

void 
NBRE_ImageEntity::SetColor(const NBRE_Color& color)
{
    mQuad->SetColor(color);
}

const NBRE_Color& 
NBRE_ImageEntity::GetColor()
{
    return mQuad->GetColor();
}

void 
NBRE_ImageEntity::SetVisible(nb_boolean value)
{
    mVisible = value;
}
