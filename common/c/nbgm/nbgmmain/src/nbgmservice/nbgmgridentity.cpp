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
#include "nbgmgridentity.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"

NBGM_GridEntity::NBGM_GridEntity(NBRE_Context& nbreContext, const uint32* buffer, const NBRE_Vector2i& cellCount, float cellSize)
    :mNBREContext(nbreContext)
    ,mUsedCellColor(1.0f, 0.0f, 0.0f, 0.3f)
    ,mEmptyCellColor(0.8f, 0.8f, 0.8f, 0.1f)
    ,mNeedUpdate(TRUE)
    ,mVisible(TRUE)
    ,mParentNode(NULL)
    ,mOverlayId(-1)
{   
    mPassCache[0] = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    mPassCache[0]->SetEnableCullFace(FALSE);
    mPassCache[0]->SetLineWidth(1.0f);
    mPassCache[0]->SetEnableBlend(TRUE);

    mPassCache[1] = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    NBRE_TextureUnit tu;
    tu.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, TRUE));
    NBRE_FontMaterial fm;
    tu.SetTexture(mNBREContext.mFontManager->GetTexture(fm));
    mPassCache[1]->GetTextureUnits().push_back(tu);
    mPassCache[1]->SetEnableBlend(TRUE);
    mPassCache[1]->SetEnableAlphaTest(FALSE);
    mPassCache[1]->SetEnableDepthTest(FALSE);
    mPassCache[1]->SetEnableDepthWrite(FALSE);
    mPassCache[1]->SetEnableCullFace(FALSE);
    

    mBuffer = buffer;
    mCellCount = cellCount;
    mCellSize = cellSize;
    CreateBuffer();
}

NBGM_GridEntity::~NBGM_GridEntity()
{
}

void 
NBGM_GridEntity::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if (!mVisible)
    {
        return;
    }

    for (uint32 i = 0; i < mMesh->GetSubMeshNumber(); ++i)
    {
        NBRE_SubMesh* sub = mMesh->GetSubMesh(i);
        nbre_assert(sub->GetIndexData() != NULL);
        NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();
        renderOp.SetVertexData(sub->GetVertexData());
        renderOp.SetIndexData(sub->GetIndexData());
        renderOp.SetOperationType(sub->GetOperationType());
        renderOp.SetPass(mPassCache[i].get());
        renderOp.SetTransform(&mTransform);
    }
}

void 
NBGM_GridEntity::CreateBuffer()
{
    NBRE_Vector<NBRE_Vector2i> cells;
    for (uint16 y = 0; y < mCellCount.y; ++y)
    {
        for (uint16 x = 0; x < mCellCount.x; ++x)
        {
            cells.push_back(NBRE_Vector2i(x, y));
        }
    }

    mMesh = NBRE_MeshPtr(NBRE_NEW NBRE_Mesh());

    {
        // Create vertex buffer
        NBRE_HardwareVertexBuffer* vertextBuff = mNBREContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (2 + 4), cells.size() * 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexDeclaration* decalration = mNBREContext.mRenderPal->CreateVertexDeclaration();
        NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
        decalration->GetVertexElementList().push_back(posElem);
        NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (2) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
        decalration->GetVertexElementList().push_back(colorElem);

        NBRE_VertexData* mVertexData = NBRE_NEW NBRE_VertexData(0);
        mVertexData->AddBuffer(vertextBuff);
        mVertexData->SetVertexDeclaration(decalration);

        // Create index buffer
        NBRE_HardwareIndexBuffer* indexBuff = mNBREContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, cells.size() * 12, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

        // Update buffer data
        GridEntityVertex* pVertex = static_cast<GridEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
        uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

        uint32 i = 0;
        for (i = 0; i < cells.size(); ++i)
        {
            int32 x = cells[i].x;
            int32 y = cells[i].y;

            GridEntityVertex* pv = pVertex + i * 4;
            uint16 baseIndex = static_cast<uint16>(i) * 4;
            uint16* pi = pIndex + static_cast<uint16>(i) * 12;

            pv[0].color = mEmptyCellColor;
            pv[1].color = mEmptyCellColor;
            pv[2].color = mEmptyCellColor;
            pv[3].color = mEmptyCellColor;

            float x0 = static_cast<float>(x * mCellSize);
            float x1 = static_cast<float>((x + 1) * mCellSize);
            float y0 = static_cast<float>(y * mCellSize);
            float y1 = static_cast<float>((y + 1) * mCellSize);

            pv[0].position.x = x0; pv[0].position.y = y0;
            pv[1].position.x = x1; pv[1].position.y = y0;
            pv[2].position.x = x1; pv[2].position.y = y1;
            pv[3].position.x = x0; pv[3].position.y = y1;

            pi[0] = baseIndex + 0;
            pi[1] = baseIndex + 1;
            pi[2] = baseIndex + 1;
            pi[3] = baseIndex + 2;
            pi[4] = baseIndex + 2;
            pi[5] = baseIndex + 3;
            pi[6] = baseIndex + 3;
            pi[7] = baseIndex + 0;
            pi[8] = baseIndex + 0;
            pi[9] = baseIndex + 2;
            pi[10] = baseIndex + 1;
            pi[11] = baseIndex + 3;
        }


        vertextBuff->UnLock();
        indexBuff->UnLock();
        mMesh->CreateSubMesh(mVertexData, mIndexData, NBRE_PMT_LINE_LIST);
    }

    {
        // Create vertex buffer
        NBRE_HardwareVertexBuffer* vertextBuff = mNBREContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (2 + 2 + 4), (1) * 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexDeclaration* decalration = mNBREContext.mRenderPal->CreateVertexDeclaration();
        NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
        decalration->GetVertexElementList().push_back(posElem);
        NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(0, (2) * sizeof(float), NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
        decalration->GetVertexElementList().push_back(texElem);
        NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (2 + 2) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
        decalration->GetVertexElementList().push_back(colorElem);

        NBRE_VertexData* mVertexData = NBRE_NEW NBRE_VertexData(0);
        mVertexData->AddBuffer(vertextBuff);
        mVertexData->SetVertexDeclaration(decalration);

        // Create index buffer
        NBRE_HardwareIndexBuffer* indexBuff = mNBREContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, (cells.size()) * 6, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

        // Update buffer data
        QuadVertex* pVertex = static_cast<QuadVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
        uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

        QuadVertex* pv = pVertex;
        uint16 baseIndex = 0;
        uint16* pi = pIndex;
        NBRE_Color fontColor(0, 0, 0.5f, 0.4f);

        pv[0].color = fontColor;
        pv[1].color = fontColor;
        pv[2].color = fontColor;
        pv[3].color = fontColor;

        int32 d = mCellCount.x < mCellCount.y ? mCellCount.x : mCellCount.y;
        float x0 = static_cast<float>(0);
        float x1 = static_cast<float>(d * mCellSize);
        float y0 = static_cast<float>(0);
        float y1 = static_cast<float>(d * mCellSize);

        pv[0].position.x = x0; pv[0].position.y = y0;
        pv[1].position.x = x1; pv[1].position.y = y0;
        pv[2].position.x = x1; pv[2].position.y = y1;
        pv[3].position.x = x0; pv[3].position.y = y1;

        pv[0].texcoord.x = 0; pv[0].texcoord.y = 0;
        pv[1].texcoord.x = 1; pv[1].texcoord.y = 0;
        pv[2].texcoord.x = 1; pv[2].texcoord.y = 1;
        pv[3].texcoord.x = 0; pv[3].texcoord.y = 1;

        pi[0] = baseIndex + 0;
        pi[1] = baseIndex + 1;
        pi[2] = baseIndex + 2;
        pi[3] = baseIndex + 0;
        pi[4] = baseIndex + 2;
        pi[5] = baseIndex + 3;

        vertextBuff->UnLock();
        indexBuff->UnLock();
        mMesh->CreateSubMesh(mVertexData, mIndexData, NBRE_PMT_TRIANGLE_LIST);
    }

    mMesh->CalculateBoundingBox();
}

void 
NBGM_GridEntity::UpdateGrid()
{
    NBRE_HardwareVertexBuffer* vertextBuff = mMesh->GetSubMesh(0)->GetVertexData()->GetBuffer(0);

    GridEntityVertex* pVertex = static_cast<GridEntityVertex*>(vertextBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint32 num = vertextBuff->GetNumVertices() / 4;

    for (uint32 i = 0; i < num; ++i)
    {
        GridEntityVertex* pv = pVertex + i * 4;

        if (mBuffer[i])
        {
            NBRE_Color c(mBuffer[i] & 0x1 ? 1.0f : 0.0f
                , mBuffer[i] & 0x2 ? 1.0f : 0.0f
                , mBuffer[i] & 0x4 ? 1.0f : 0.0f
                , mUsedCellColor.a);

            pv[0].color = c;
            pv[1].color = c;
            pv[2].color = c;
            pv[3].color = c;
        }
        else
        {
            pv[0].color = mEmptyCellColor;
            pv[1].color = mEmptyCellColor;
            pv[2].color = mEmptyCellColor;
            pv[3].color = mEmptyCellColor;
        }
    }

    vertextBuff->UnLock();
}

void 
NBGM_GridEntity::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

int32 
NBGM_GridEntity::OverlayId() const
{
    return mOverlayId;
}

nb_boolean 
NBGM_GridEntity::Visible() const
{
    return TRUE;
}

void
NBGM_GridEntity::UpdateSurfaceList()
{
}

const NBRE_AxisAlignedBox3d&
NBGM_GridEntity::AxisAlignedBox() const
{
    return mBoundingBox;
}

void 
NBGM_GridEntity::NotifyAttached(NBRE_Node* node)
{
    mParentNode = node;
}

void 
NBGM_GridEntity::NotifyDetatch()
{
    mParentNode = NULL;
}

void 
NBGM_GridEntity::NotifyUpdatePosition()
{
    mBoundingBox = NBRE_TypeConvertd::Convert(mMesh->AxisAlignedBox());
    NBRE_Transformationd::Transform(mParentNode->WorldTransform(), mBoundingBox);

    mBoundingBox.minExtend.x = -NBRE_Math::Infinity;
    mBoundingBox.minExtend.y = -NBRE_Math::Infinity;
    mBoundingBox.maxExtend.x = NBRE_Math::Infinity;
    mBoundingBox.maxExtend.y = NBRE_Math::Infinity;

    mTransform = mParentNode->WorldTransform();
}

NBRE_ISpatialObject* 
NBGM_GridEntity::SpatialObject()
{
    return this;
}

void 
NBGM_GridEntity::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBREContext.mSurfaceManager);
    visitor.Visit(static_cast<NBRE_ISpatialObject*>(this));
    visitor.Visit(static_cast<NBRE_IMovableObject*>(this));
}

void
NBGM_GridEntity::SetOverlayId(int32 overlayId)
{
    mOverlayId = overlayId;
}

void
NBGM_GridEntity::UpdateBufferSize(const uint32* buffer, const NBRE_Vector2i& cellCount, float cellSize)
{
    if (buffer != mBuffer || cellCount != mCellCount || cellSize != mCellSize)
    {
        mBuffer = buffer;
        mCellCount = cellCount;
        mCellSize = cellSize;
        CreateBuffer();
    }
}
