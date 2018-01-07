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
#include "nbgmicon.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbretransformutil.h"
#include "nbgmbuildutility.h"
#include "nbgmcommon.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderengine.h"
#include "nbrememorystream.h"

static NBRE_EntityPtr
CreateHaloEntity(NBGM_Context& context, int32 overlayId, float radius)
{
    const uint32 N = 90;
    const float Det = NBRE_Math::PiTwo/N;
    const uint32 VertexCount = N+2;
    const uint32 indexCount0 = N*3;

    float* vertices = NBRE_NEW float[VertexCount*3];
    float* texCoords = NBRE_NEW float[VertexCount*2];
    uint16* indices0 = NBRE_NEW uint16[indexCount0];
    nsl_memset(vertices, 0, VertexCount*3*sizeof(float));
    nsl_memset(texCoords, 0, VertexCount*2*sizeof(float));
    nsl_memset(indices0, 0, indexCount0*sizeof(uint16));

    // build vertex buffer
    for(uint32 i = 1; i < VertexCount; i++)
    {
        const uint32 index  = i*3;
        const float angle = Det*(i-1);
        vertices[index]   = nsl_cos(angle) * radius;
        vertices[index+1] = nsl_sin(angle) * radius;
        vertices[index+2] = 0;
    }

    // build index buffer for the inner of the halo
    for(uint16 i = 0, j = 1; i < N; i++, j++)
    {
       const uint16 index = i*3;
       indices0[index]   = 0;
       indices0[index+1] = j;
       indices0[index+2] = j+1;
    }

    NBRE_IRenderPal *renderPal = context.renderingEngine->Context().mRenderPal;
    NBRE_HardwareVertexBuffer* vertextBuff = renderPal->CreateVertexBuffer(sizeof(float)*3, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float)*3*VertexCount, vertices, TRUE);
    NBRE_HardwareVertexBuffer* texCoordBuff = renderPal->CreateVertexBuffer(sizeof(float)*2, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    texCoordBuff->WriteData(0, sizeof(float)*2*VertexCount, texCoords, TRUE);

    NBRE_VertexDeclaration* decalration = renderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texCoordElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texCoordElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texCoordBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff0 = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount0, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff0->WriteData(0, sizeof(uint16)*indexCount0, indices0, TRUE);
    NBRE_IndexData* indexData0 = NBRE_NEW NBRE_IndexData(indexBuff0, 0, indexBuff0->GetNumIndexes());

    NBRE_MeshPtr mesh(NBRE_NEW NBRE_Mesh(vertexData));
    mesh->CreateSubMesh(indexData0, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(context.renderingEngine->Context(), model, overlayId));

    NBRE_DELETE_ARRAY vertices;
    NBRE_DELETE_ARRAY texCoords;
    NBRE_DELETE_ARRAY indices0;
    return entity;
}

static NBRE_ShaderPtr
CreateHaloShader(NBGM_Context& context, const NBRE_Color& clr, const NBRE_String& texName, float lineWidth)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructColorTexture(*(context.renderingEngine->Context().mRenderPal), clr, texName);
    NBRE_TextureUnit texUnit;
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit.SetTexture(texture);

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->GetTextureUnits().push_back(texUnit);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
    if(lineWidth > 0.0f)
    {
        pass->SetLineWidth(lineWidth);
    }

    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}


static NBRE_ShaderPtr
CreateIconShader(NBGM_Context& nbgmContext, NBRE_IOStream* stream)
{
    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_PngTextureImage(*(nbgmContext.renderingEngine->Context().mPalInstance), stream, 0, FALSE);
    NBRE_TexturePtr texture(nbgmContext.renderingEngine->Context().mRenderPal->CreateTexture(textureImage, NBRE_Texture::TT_2D, TRUE, "") );

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

NBGM_Icon::NBGM_Icon(NBGM_Context& nbgmContext,
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
                     const NBRE_Vector2f& bubbleOffset)
    :NBGM_LayoutElement(nbgmContext)
    ,mBillboardSet(billboardSet)
    ,mSize(size)
    ,mSelectedTexSize(selectedTexsize)
    ,mPosition(position)
    ,mIsInFrustum(FALSE)
    ,mScale(1.0f)
    ,mId(id)
    ,mSelected(FALSE)
    ,mParentNode(parentNode)
    ,mShader(shader)
    ,mSelectedShader(selectedShader)
    ,mBaseSize(1.0)
    ,mIsPin(isPin)
    ,mImageOffset(imageOffset)
    ,mSelectedImageOffset(selectedImageOffset)
    ,mBubbleOffset(bubbleOffset)
    ,mHasHalo(FALSE)
    ,mHaloRadius(0)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mFatherNode(NBRE_NEW NBRE_Node())
{
    nbre_assert(parentNode != NULL && billboardSet != NULL);
    if (isPin)
    {
        SetTypeId(NBGM_LET_PIN);
    }
    else
    {
        SetTypeId(NBGM_LET_POI);
    }
}


NBGM_Icon::NBGM_Icon(NBGM_Context& nbgmContext,
                     NBRE_Node* parentNode,
                     NBRE_BillboardSet* billboardSet,
                     const NBRE_Vector2f& size,
                     const NBRE_Vector2f& selectedTexsize,
                     const NBRE_Vector3d& position,
                     const NBRE_String& id,
                     nb_boolean isPin,
                     const NBRE_Vector2f& imageOffset,
                     const NBRE_Vector2f& selectedImageOffset,
                     const NBRE_Vector2f& bubbleOffset)
    :NBGM_LayoutElement(nbgmContext)
    ,mBillboardSet(billboardSet)
    ,mSize(size)
    ,mSelectedTexSize(selectedTexsize)
    ,mPosition(position)
    ,mIsInFrustum(FALSE)
    ,mScale(1.0f)
    ,mId(id)
    ,mSelected(FALSE)
    ,mParentNode(parentNode)
    ,mBaseSize(1.0)
    ,mIsPin(isPin)
    ,mImageOffset(imageOffset)
    ,mSelectedImageOffset(selectedImageOffset)
    ,mBubbleOffset(bubbleOffset)
    ,mHasHalo(FALSE)
    ,mHaloRadius(0)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mFatherNode(NBRE_NEW NBRE_Node())
{
    nbre_assert(parentNode != NULL && billboardSet != NULL);
    if (isPin)
    {
        SetTypeId(NBGM_LET_PIN);
    }
    else
    {
        SetTypeId(NBGM_LET_POI);
    }
}

NBGM_Icon::~NBGM_Icon()
{
    if(mNode)
    {
        mFatherNode->RemoveChild(mNode.get());
        if(mHasHalo)
        {
            mFatherNode->RemoveChild(mHaloNode.get());
        }
        mBillboardSet->RemoveBillboard(mNode);
    }

    if(mExtendNode)
    {
        mFatherNode->RemoveChild(mExtendNode.get());
        if(mHasHalo)
        {
            mFatherNode->RemoveChild(mExtendHaloNode.get());
        }
        mBillboardSet->RemoveBillboard(mExtendNode);
    }

    mParentNode->RemoveChild(mFatherNode.get());
}

void
NBGM_Icon::Initialize(const NBGM_BinaryBuffer& unSelectedTextureBuffer,
                const NBGM_BinaryBuffer& selectedTextureBuffer,
                NBRE_Color unSelectedCircleInteriorColor,
                NBRE_Color unSelectedCircleOutlineBitOnColor,
                NBRE_Color /*unSelectedCircleOutlineBitOffColor*/,
                NBRE_Color /*selectedCircleInteriorColor*/,
                NBRE_Color /*selectedCircleOutlineBitOnColor*/,
                NBRE_Color /*selectedCircleOutlineBitOffColor*/,
                uint8 circleOutlineWidth,
                float radius
                )
{
    if(unSelectedTextureBuffer.empty() || selectedTextureBuffer.empty())
    {
        return;
    }

    NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(unSelectedTextureBuffer.addr, unSelectedTextureBuffer.size, TRUE);
    mShader = CreateIconShader(mNBGMContext, memoryIO);

    memoryIO = NBRE_NEW NBRE_MemoryStream(selectedTextureBuffer.addr, selectedTextureBuffer.size, TRUE);
    mSelectedShader = CreateIconShader(mNBGMContext, memoryIO);

    if (radius)
    {
        mHasHalo = TRUE;
        mHaloInteriorShader = CreateHaloShader(mNBGMContext, unSelectedCircleInteriorColor, "", 0.0f);
        mHaloOutlineShader = CreateHaloShader(mNBGMContext, unSelectedCircleOutlineBitOnColor, "", DP_TO_PIXEL(circleOutlineWidth));
        mHaloRadius = radius;
    }
}

void
NBGM_Icon::InitializeHalo(const NBRE_ShaderPtr& interiorShader, const NBRE_ShaderPtr& outlineShader, float radias)
{
    mHasHalo = TRUE;
    mHaloInteriorShader = interiorShader;
    mHaloOutlineShader = outlineShader;
    mHaloRadius = radias;
}

void
NBGM_Icon::Update()
{
    if (!mNode)
    {
        return;
    }

    int32 drawIndexOffset = mIsPin ? 2 : 0;

    if (mSelected)
    {
        mNode->GetEntities()[0]->SetVisible(FALSE);
        mNode->GetEntities()[1]->SetVisible(TRUE);
        mNode->SetDrawIndex(1 + drawIndexOffset);

        mExtendNode->GetEntities()[0]->SetVisible(FALSE);
        mExtendNode->GetEntities()[1]->SetVisible(TRUE);
        mExtendNode->SetDrawIndex(1 + drawIndexOffset);
    }
    else
    {
        mNode->GetEntities()[0]->SetVisible(TRUE);
        mNode->GetEntities()[1]->SetVisible(FALSE);
        mNode->SetDrawIndex(0 + drawIndexOffset);

        mExtendNode->GetEntities()[0]->SetVisible(TRUE);
        mExtendNode->GetEntities()[1]->SetVisible(FALSE);
        mExtendNode->SetDrawIndex(0 + drawIndexOffset);
    }

    if(mHasHalo)
    {
        mHaloEntity->SetPriority(mSelected ? 1 : 0);
        mExtendHaloEntity->SetPriority(mSelected ? 1 : 0);
    }

    mIsInFrustum = FALSE;
    if (!IsEnable())
    {
        mScale = 0;
        mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
        mExtendNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
        return;
    }

    mFatherNode->SetPosition(mPosition);

    double pixelsPerUnit = 0;
    NBRE_BillboardNodePtr node = GetNodeInFrustum();
    if (mNBGMContext.transUtil->PixelsPerUnit(node->WorldPosition(), pixelsPerUnit))
    {
        mScale = UpdateScale(pixelsPerUnit);
        mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
        mExtendNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));

        NBRE_Vector2d screenPosition;
        if(mNBGMContext.transUtil->WorldToScreen(node->WorldPosition(), screenPosition))
        {
            UpdateScreenRect(screenPosition, pixelsPerUnit);
            mIsInFrustum = TRUE;
        }
    }
}

nb_boolean
NBGM_Icon::IsElementVisible()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    mIsInFrustum = FALSE;

    NBRE_BillboardNodePtr node = GetNodeInFrustum();
    if (!mIsPin)
    {
        if (!IsEnable())
        {
            return FALSE;
        }

        if (!layoutManager.IsInFrustum(node->WorldPosition()))
        {
            return FALSE;
        }

        if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
        {
            double pixelsPerUnit = 0;
            if (!layoutManager.PixelsPerUnit(node->WorldPosition(), pixelsPerUnit))
            {
                return FALSE;
            }
        }
    }

    mIsInFrustum = TRUE;
    return TRUE;
}

void
NBGM_Icon::UpdateRegion()
{
    if (mIsPin)
    {
        return;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    CalculateScreenRectRegion(mScreenRect);
    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, 0x1);
}

nb_boolean
NBGM_Icon::IsElementRegionAvailable()
{
    if (mIsPin)
    {
        return TRUE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    NBRE_Vector2d screenPosition;
    NBRE_BillboardNodePtr node = GetNodeInFrustum();
    if(!layoutManager.WorldToScreen(node->WorldPosition(), screenPosition))
    {
        return FALSE;
    }

    double pixelsPerUnit = 0;
    if (!layoutManager.PixelsPerUnit(node->WorldPosition(), pixelsPerUnit))
    {
        return FALSE;
    }

    mScale = UpdateScale(pixelsPerUnit);
    mBaseSize = pixelsPerUnit;
    UpdateScreenRect(screenPosition, pixelsPerUnit);

    if (!NBRE_Intersectiond::HitTest(mScreenRect, layoutManager.GetScreenRect()))
    {
        return FALSE;
    }

    if (mIsPin)
    {
        return TRUE;
    }

    return layoutManager.GetLayoutBuffer().IsRegionAvailable(mScreenRect, 0x3);
}

NBRE_Vector3d
NBGM_Icon::GetPosition()
{
    return mPosition;
}

void
NBGM_Icon::SetPosition(const NBRE_Vector3d& position)
{
    mPosition = position;
}

void
NBGM_Icon::SetColor(const NBRE_Color& color)
{
    if (mIsPin)
    {
        mNode->SetColor(NBRE_Color(1, 1, 1, 1));
        mExtendNode->SetColor(NBRE_Color(1, 1, 1, 1));
    }
    else
    {
        mNode->SetColor(color);
        mExtendNode->SetColor(color);
    }
}

double
NBGM_Icon::UpdateScale(double pixelsPerUnit)
{
    return (mSelected?mSelectedTexSize.x:mSize.x)/ pixelsPerUnit;
}

void
NBGM_Icon::SetSelected(nb_boolean val)
{
    mSelected = val;
}

nb_boolean
NBGM_Icon::GetScreenPosition(NBRE_Vector2f& position)
{
    if(!mIsInFrustum)
    {
        return FALSE;
    }
    NBRE_Vector2d screenPosition;
    mNBGMContext.transUtil->UpdateCameraMatrix();
    if (mNBGMContext.transUtil->WorldToScreen(GetNodeInFrustum()->WorldPosition(),screenPosition))
    {
        position.x = static_cast<float>(screenPosition.x);
        position.y = static_cast<float>(screenPosition.y);
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBGM_Icon::GetScreenBottomPosition(NBRE_Vector2f& position)
{
    if(!mIsInFrustum)
    {
        return FALSE;
    }
    const NBRE_Vector2d& c = mScreenRect.GetCenter();
    position.x = static_cast<float>(c.x);
    position.y = static_cast<float>(mScreenRect.minExtend.y);
    return TRUE;
}

nb_boolean
NBGM_Icon::GetScreenBubblePosition(NBRE_Vector2f& position)
{
    NBRE_Vector2f pos;
    if(GetScreenPosition(pos))
    {
        position.x = pos.x + mBubbleOffset.x*mSelectedTexSize.x;
        position.y = pos.y + mBubbleOffset.y*mSelectedTexSize.y;
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBGM_Icon::HitTest(const NBRE_Vector2f& screenPosition)
{
    if (!mIsInFrustum)
    {
        return FALSE;
    }

    if (!NBRE_Intersectiond::HitTest(mScreenRect, NBRE_TypeConvertd::Convert(screenPosition)))
    {
        return FALSE;
    }

    return TRUE;
}

void
NBGM_Icon::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    mNode = mBillboardSet->AddBillboard();
    NBRE_Vector2f imgSize(1.0f, mSize.y / mSize.x);
    NBRE_ImageEntity* imageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), imgSize, mShader, mImageOffset);
    mNode->AddEntity(NBRE_BillboardEntityPtr(imageEntity));

    mExtendNode = mBillboardSet->AddBillboard();
    NBRE_ImageEntity* extendImageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), imgSize, mShader, mImageOffset);
    mExtendNode->AddEntity(NBRE_BillboardEntityPtr(extendImageEntity));

    NBRE_Vector2f selectedImgSize(1.0f, mSelectedTexSize.y / mSelectedTexSize.x);
    NBRE_ImageEntity* selectedImageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), selectedImgSize, mSelectedShader, mSelectedImageOffset);
    mNode->AddEntity(NBRE_BillboardEntityPtr(selectedImageEntity));

    NBRE_ImageEntity* extendSelectedImageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), selectedImgSize, mSelectedShader, mSelectedImageOffset);
    mExtendNode->AddEntity(NBRE_BillboardEntityPtr(extendSelectedImageEntity));

    selectedImageEntity->SetVisible(FALSE);
    extendSelectedImageEntity->SetVisible(FALSE);

    if (!mIsPin)
    {
        mNode->SetVisible(FALSE);
    }
    mExtendNode->SetVisible(FALSE);

    mParentNode->AddChild(mFatherNode);
    mFatherNode->AddChild(mNode);
    mFatherNode->AddChild(mExtendNode);

    if (mIsPin)
    {
        mScale = 0.0f;
    }
    mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
    mExtendNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));

    if(mHasHalo)
    {
        mHaloNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
        mExtendHaloNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
        mHaloEntity = CreateHaloEntity(mNBGMContext, mBillboardSet->OverlayId(), mHaloRadius);
        mExtendHaloEntity = NBRE_EntityPtr(mHaloEntity->Duplicate());
        mHaloEntity->GetSubEntity(0).SetShader(mHaloInteriorShader);
        mExtendHaloEntity->GetSubEntity(0).SetShader(mHaloInteriorShader);
        mExtendHaloEntity->SetVisible(FALSE);
        mHaloNode->AttachObject(mHaloEntity);
        mExtendHaloNode->AttachObject(mExtendHaloEntity);
        mFatherNode->AddChild(mHaloNode);
        mFatherNode->AddChild(mExtendHaloNode);
    }
}

void
NBGM_Icon::SetVisible(nb_boolean value)
{
    //if (mIsPin)
    //{
        //return;
    //}
    mNode->SetVisible(value);
    mExtendNode->SetVisible(value);
    if(mHasHalo)
    {
        mHaloEntity->SetVisible(value);
        mExtendHaloEntity->SetVisible(value);
    }
}

void
NBGM_Icon::UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit)
{
    NBRE_BillboardEntityPtr mEntity = GetNodeInFrustum()->GetEntities()[0];
    NBRE_Vector2d offset = NBRE_TypeConvertd::Convert(((NBRE_ImageEntity*)mEntity.get())->GetOffset());
    NBRE_Vector3d size = NBRE_TypeConvertd::Convert(mEntity->RenderBoundingBox().maxExtend - mEntity->RenderBoundingBox().minExtend);
    NBRE_Vector2d hs(size.x * mScale, size.y * mScale);
    hs *= 0.5f * pixelsPerUnit;
    offset.x *= hs.x * 2.0f;
    offset.y *= hs.y * 2.0f;
    mScreenRect.minExtend = screenPosition - hs + offset;
    mScreenRect.maxExtend = screenPosition + hs + offset;
}

NBGM_LayoutElement*
NBGM_Icon::Clone(NBRE_Node* parentNode)
{
    NBGM_Icon* icon = NBRE_NEW NBGM_Icon(mNBGMContext, parentNode, mBillboardSet, mSize, mSelectedTexSize, mShader, mSelectedShader
        , mPosition, mId, mIsPin, mImageOffset, mSelectedImageOffset,mBubbleOffset);
    icon->SetPriority(mPriority);
    if(mHasHalo)
    {
        icon->InitializeHalo(mHaloInteriorShader, mHaloOutlineShader, mHaloRadius);
    }
    return icon;
}

nb_boolean
NBGM_Icon::CalculateScreenRectRegion(NBRE_AxisAlignedBox2d& rect)
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    NBRE_Vector2d screenPosition;
    NBRE_BillboardNodePtr node = GetNodeInFrustum();
    if(!layoutManager.WorldToScreen(node->WorldPosition(), screenPosition))
    {
        return FALSE;
    }

    double pixelsPerUnit = 0;
    if (!layoutManager.PixelsPerUnit(node->WorldPosition(), pixelsPerUnit))
    {
        return FALSE;
    }

    mScale = UpdateScale(pixelsPerUnit);
    mBaseSize = pixelsPerUnit;
    UpdateScreenRect(screenPosition, pixelsPerUnit);

    rect = mScreenRect;
    return TRUE;
}

void
NBGM_Icon::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if(mExtendType != extendType)
    {
        switch(extendType)
        {
            case NBGM_TMBET_NONE:
                mExtendNode->SetVisible(FALSE);
                if(mHasHalo)
                {
                    mExtendHaloEntity->SetVisible(FALSE);
                }
                break;
            case NBGM_TMBET_LEFT:
                mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
                mExtendNode->SetVisible(TRUE);
                if(mHasHalo)
                {
                    mExtendHaloNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
                    mExtendHaloEntity->SetVisible(TRUE);
                }
                break;
            case NBGM_TMBET_RIGHT:
                mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
                mExtendNode->SetVisible(TRUE);
                if(mHasHalo)
                {
                    mExtendHaloNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
                    mExtendHaloEntity->SetVisible(TRUE);
                }
                break;
            default:
                break;
        }
        mExtendType = extendType;
    }
}

NBRE_BillboardNodePtr
NBGM_Icon::GetNodeInFrustum() const
{
    nb_boolean isExtendNode = !mNBGMContext.transUtil->IsInFrustum(mExtendNode->WorldPosition())
                              || mExtendType == NBGM_TMBET_NONE
                              ? FALSE : TRUE;

    if (isExtendNode)
    {
        return mExtendNode;
    }
    else
    {
        return mNode;
    }
}
