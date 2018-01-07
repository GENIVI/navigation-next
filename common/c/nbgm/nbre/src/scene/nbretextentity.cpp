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

#include "nbretextentity.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbrefontmanager.h"
#include "nbretypeconvert.h"
#include "nbrerenderengine.h"

static const float ROTATE_COS_TOLERANCE = 0.94f;
static const double DIRECTION_COS_TOLERANCE = 0.87;

NBRE_TextEntity::NBRE_TextEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight)
    :mContext(context)
    ,mText(text)
    ,mFontMaterial(fontMaterial)
    ,mFontHeight(fontHeight)
    ,mVertexData(NULL)
    ,mIndexData(NULL)
    ,mNeedUpdateColor(FALSE)
    ,mVisible(TRUE)
    ,mNeedUpdateVertex(TRUE)
{
}

NBRE_TextEntity::~NBRE_TextEntity()
{
    NBRE_DELETE mVertexData;
    mVertexData = NULL;
    NBRE_DELETE mIndexData;
    mIndexData = NULL;
}

void
NBRE_TextEntity::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if (!mVisible)
    {
        return;
    }

    if (mNeedUpdateVertex)
    {
        UpdateVertexBuffer();
    }
    else
    {
        if (mNeedUpdateColor)
        {
            // Update buffer data
            NBRE_HardwareVertexBuffer* colorBuff = mVertexData->GetBuffer(2);
            NBRE_Color* pColor = static_cast<NBRE_Color*>(colorBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

            uint32 numChars = colorBuff->GetNumVertices() / 8;
            NBRE_Color backColor = mColor * mFontMaterial.backColor;
            NBRE_Color foreColor = mColor * mFontMaterial.foreColor;
            for (uint16 i = 0; i < numChars; ++i)
            {
                // set background vertex
                NBRE_Color* pC = pColor + i * 4;
                pC[0] = backColor;
                pC[1] = backColor;
                pC[2] = backColor;
                pC[3] = backColor;

                // set foreground vertex
                pC = pColor + (numChars + i) * 4;
                pC[0] = foreColor;
                pC[1] = foreColor;
                pC[2] = foreColor;
                pC[3] = foreColor;
            }
            colorBuff->UnLock();
            mNeedUpdateColor = FALSE;
        }
    }

    if (mVertexData == NULL)
    {
        return;
    }

    // Add render data to overlay
    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();
    renderOp.SetVertexData(mVertexData);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_TRIANGLE_LIST);
    renderOp.SetPass(mPassCache.get());
    renderOp.SetTransform(&mTransform);
}

void 
NBRE_TextEntity::UpdateFontTexture()
{
}

void 
NBRE_TextEntity::SetColor(const NBRE_Color& color)
{
    mColor = color;
    mNeedUpdateColor = TRUE;
}

const NBRE_Color&
NBRE_TextEntity::GetColor() const
{
    return mColor;
}

void
NBRE_TextEntity::SetTransform(const NBRE_Matrix4x4d& transform)
{
    mTransform = transform;
}

int32 
NBRE_TextEntity::OverlayId() const
{
    return -1;
}

void 
NBRE_TextEntity::UpdateFontRenderQueue()
{
}

void 
NBRE_TextEntity::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

nb_boolean 
NBRE_TextEntity::Visible() const
{
    return TRUE;
}

const NBRE_AxisAlignedBox3f& 
NBRE_TextEntity::AxisAlignedBox() const
{
    return mAABB;
}

float
NBRE_TextEntity::GetStringWidth(NBRE_Context& contex, const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text)
{
    return contex.mFontManager->GetStringWidth(fontMaterial, fontHeight, text, PAL_TA_CENTER);
}

NBRE_TextPathEntity::NBRE_TextPathEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight, NBRE_Polyline2f* polyline, float offset)
    :NBRE_TextEntity(context, text, fontMaterial, fontHeight)
    ,mPolyline(polyline)
    ,mOffset(offset)
    ,mIsRevert(FALSE)
    ,mRotateEnable(FALSE)
{
    mAABB.SetNull();
    for (uint32 i = 0; i < mPolyline->VertexCount(); ++i)
    {
        const NBRE_Vector2f& v = mPolyline->Vertex(i);
        mAABB.Merge(v.x, v.y, 0);
    }
    mAABB.minExtend.x -= fontHeight;
    mAABB.minExtend.y -= fontHeight;
    mAABB.maxExtend.x += fontHeight;
    mAABB.maxExtend.y += fontHeight;
    
    if (polyline->VertexCount() >= 2)
    {
        mLayoutEndPoints[0] = polyline->Vertex(0);
        mLayoutEndPoints[1] = polyline->Vertex(polyline->VertexCount() - 1);
    }
}

NBRE_TextPathEntity::~NBRE_TextPathEntity()
{
}

void
NBRE_TextPathEntity::Rebuild(float fontHeight, nb_boolean isRevert, NBRE_Polyline2f* polyline, float offset)
{
    mFontHeight = fontHeight;
    mIsRevert = isRevert;
    mPolyline = polyline;
    mOffset = offset;
    mNeedUpdateVertex = TRUE;
    UpdateVertexBuffer();
}

void
NBRE_TextPathEntity::SetFontMaterial(const NBRE_FontMaterial& fontMaterial)
{
    mFontMaterial = fontMaterial;
    mNeedUpdateVertex = TRUE;
}

void
NBRE_TextPathEntity::SetTransform(const NBRE_Matrix4x4d& transform)
{   
    mWorldTransform = transform;
    mTransform = mWorldTransform * mLocalTransform;
}

void
NBRE_TextPathEntity::UpdateVertexBuffer()
{
    if (!mNeedUpdateVertex)
    {
        return;
    }

    NBRE_VisualGlyphList glyphList;
    if (!mContext.mFontManager->GetTextLayout(mFontMaterial, mText, mFontHeight, FALSE, PAL_TA_CENTER, glyphList))
    {
        return;
    }

    float maxX = 0;
    uint16 numChars = static_cast<uint16>(glyphList.size());
    for (uint16 i = 0; i < numChars; ++i)
    {
        NBRE_VisualGlyph& vg = glyphList[numChars - 1 - i];
        NBRE_Glyph* glyph = mContext.mFontManager->GetGlyph(mFontMaterial, vg.fontId, vg.glyphId).get();
        if (glyph == NULL)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "Can't get glyph for %s[%d]", this->mText.c_str(), i);
            return;
        }
        if(vg.positions.maxExtend.x > maxX)
        {
            maxX = vg.positions.maxExtend.x;
        }
    }
    float polylineOffset = (mPolyline->Length() - maxX) * 0.5f;

    // clean up
    mAABB.SetNull();

    if (mVertexData == NULL)
    {
        // init pass
        mPassCache = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
        mPassCache->GetTextureUnits().push_back(NBRE_TextureUnit());
        NBRE_TextureUnit tu;
        tu.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, TRUE));
        tu.SetTexture(mContext.mFontManager->GetTexture(mFontMaterial));
        tu.SetTextureCoordSet(1);
        mPassCache->GetTextureUnits().push_back(tu);
        mPassCache->SetEnableBlend(TRUE);
        mPassCache->SetEnableAlphaTest(FALSE);
        mPassCache->SetEnableDepthTest(TRUE);
        mPassCache->SetEnableDepthWrite(FALSE);
        mPassCache->SetEnableCullFace(FALSE);

        // Create vertex buffer
        NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 3, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
        NBRE_HardwareVertexBuffer* texcoordBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 2, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
        NBRE_HardwareVertexBuffer* colorBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 4, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);

        NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
        NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
        decalration->GetVertexElementList().push_back(posElem);
        NBRE_VertexElement* texcoordElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 1);
        decalration->GetVertexElementList().push_back(texcoordElem1);
        NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
        decalration->GetVertexElementList().push_back(colorElem);

        mVertexData = NBRE_NEW NBRE_VertexData(0);
        mVertexData->AddBuffer(vertextBuff);
        mVertexData->AddBuffer(texcoordBuff);
        mVertexData->AddBuffer(colorBuff);
        mVertexData->SetVertexDeclaration(decalration);

        // Create index buffer
        NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 6 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
        mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

        uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
        for (uint16 i = 0; i < numChars; ++i)
        {
            {
                // set background vertex
                uint16 baseIndex = i * 4;
                uint16* pI = pIndex + i * 6;
                // set index
                *pI++ = baseIndex + 0;
                *pI++ = baseIndex + 1;
                *pI++ = baseIndex + 2;
                *pI++ = baseIndex + 0;
                *pI++ = baseIndex + 2;
                *pI++ = baseIndex + 3;
            }

            {
                // set foreground vertex
                uint16 baseIndex = (numChars + i) * 4;
                uint16* pI = pIndex + (numChars + i) * 6;
                // set index
                *pI++ = baseIndex + 0;
                *pI++ = baseIndex + 1;
                *pI++ = baseIndex + 2;
                *pI++ = baseIndex + 0;
                *pI++ = baseIndex + 2;
                *pI++ = baseIndex + 3;
            }
        }
        indexBuff->UnLock();
    }

    NBRE_Vector<NBRE_Vector2f> dirsOld;
    NBRE_Vector<NBRE_Vector2f> dirs;
    NBRE_Vector<NBRE_Vector2f> centers;
    NBRE_Vector<NBRE_Vector2f> foreSize;
    NBRE_Vector<NBRE_Vector2f> backSize;


    // Update buffer data
    NBRE_Vector3f* pVertex = static_cast<NBRE_Vector3f*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Vector2f* pTexcoord = static_cast<NBRE_Vector2f*>(mVertexData->GetBuffer(1)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Color* pColor = static_cast<NBRE_Color*>(mVertexData->GetBuffer(2)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    NBRE_Polyline2f* pl = mPolyline;

    NBRE_Color backColor = mColor * mFontMaterial.backColor;
    NBRE_Color foreColor = mColor * mFontMaterial.foreColor;
    for (uint16 i = 0; i < numChars; ++i)
    {
        NBRE_VisualGlyph& vg = glyphList[i];
        NBRE_Glyph* glyph = mContext.mFontManager->GetGlyph(mFontMaterial, vg.fontId, vg.glyphId).get();
        const NBRE_Vector2f& foregroundSize = glyph->GetForegroundSize(mFontHeight);
        const NBRE_Vector2f& backgroundSize = glyph->GetBackgroundSize(mFontHeight);
        float halfW = foregroundSize.x * 0.5f;
        float centerX = vg.positions.GetCenter().x;
        float totalX = polylineOffset + (mIsRevert ? maxX - centerX : centerX);

        NBRE_Polyline2Positionf plT = pl->ConvertOffsetToParameterCoordinate(totalX);
        NBRE_Vector2f right = pl->SegmentDirection(plT.segmentIndex);
        if (plT.segmentIndex + 2 < pl->VertexCount())
        {
            float exceedLength = totalX + halfW - pl->SubLength(0, plT.segmentIndex);
            if (exceedLength > 0)
            {
                float t = (foregroundSize.x - exceedLength) / foregroundSize.x;
                right = NBRE_LinearInterpolatef::Lerp(right, pl->SegmentDirection(plT.segmentIndex + 1), t);
                right.Normalise();
            }
        }
        if (mIsRevert)
        {
            right *= -1;
        }

        dirsOld.push_back(right);
        centers.push_back(pl->PointAt(plT));
        foreSize.push_back(foregroundSize * 0.5f);
        backSize.push_back(backgroundSize * 0.5f);

        {
            // set background vertex
            NBRE_Vector2f* pT = pTexcoord + i * 4;
            NBRE_Color* pC = pColor + i * 4;
            // set vertex colors
            pC[0] = backColor;
            pC[1] = backColor;
            pC[2] = backColor;
            pC[3] = backColor;
            // set vertex texcoord
            const NBRE_Vector2f* backTexcoord = glyph->GetAtlasBackgroundTexcoord();
            pT[0] = backTexcoord[0];
            pT[1] = backTexcoord[1];
            pT[2] = backTexcoord[2];
            pT[3] = backTexcoord[3];
        }

        {
            // set foreground vertex
            NBRE_Vector2f* pT = pTexcoord + (numChars + i) * 4;
            NBRE_Color* pC = pColor + (numChars + i) * 4;
            // set vertex colors
            pC[0] = foreColor;
            pC[1] = foreColor;
            pC[2] = foreColor;
            pC[3] = foreColor;
            // set vertex texcoord
            const NBRE_Vector2f* foreTexcoord = glyph->GetAtlasForegroundTexcoord();
            pT[0] = foreTexcoord[0];
            pT[1] = foreTexcoord[1];
            pT[2] = foreTexcoord[2];
            pT[3] = foreTexcoord[3];
        }
    }

    if (numChars > 0)
    {
        for (uint16 i = 0; i < numChars; ++i)
        {
            dirs.push_back(dirsOld[i]);
        }
    }

    if (numChars > 0)
    {
        mRotateEnable = TRUE;
        for (uint16 i = 0; i < numChars; ++i)
        {
            if (i >= 1)
            {
                if (dirs[i].DotProduct(dirs[0]) < ROTATE_COS_TOLERANCE)
                {
                    mRotateEnable = FALSE;
                    break;
                }
                if (i >= 2 && dirs[i].DotProduct(dirs[i - 1]) < ROTATE_COS_TOLERANCE)
                {
                    mRotateEnable = FALSE;
                    break;
                }
            }
        }

        if (mRotateEnable)
        {
            mRotateCenter.x = 0;
            mRotateCenter.y = 0;
            mRotateAxis.x = 0;
            mRotateAxis.y = 0;

            for (uint16 i = 0; i < numChars; ++i)
            {
                mRotateCenter += centers[i];
                mRotateAxis += dirs[i];
            }

            mRotateCenter /= numChars;
            mRotateAxis /= numChars;
        }

    }
    else
    {
        mRotateEnable = FALSE;
    }

    for (uint16 i = 0; i < numChars; ++i)
    {
        NBRE_VisualGlyph& vg = glyphList[i];
        const NBRE_Vector2f& right = dirs[i];
        const NBRE_Vector2f& up = right.Perpendicular();
        const NBRE_Vector2f& center = centers[i];
        NBRE_Vector2f uOff = up * vg.positions.GetCenter().y;

        {
            // set background vertex
            NBRE_Vector3f* pV = pVertex + i * 4;
            // set vertex positions
            NBRE_Vector2f r = right * backSize[i].x;
            NBRE_Vector2f u = up * backSize[i].y;

            pV[0] = center - r - u + uOff;
            pV[1] = center + r - u + uOff;
            pV[2] = center + r + u + uOff;
            pV[3] = center - r + u + uOff;
            /// update aabb
            mAABB.Merge(pV[0].x, pV[0].y, 0);
            mAABB.Merge(pV[1].x, pV[1].y, 0);
            mAABB.Merge(pV[2].x, pV[2].y, 0);
            mAABB.Merge(pV[3].x, pV[3].y, 0);
        }

        {
            // set foreground vertex
            NBRE_Vector3f* pV = pVertex + (numChars + i) * 4;
            // set vertex positions
            NBRE_Vector2f r = right * foreSize[i].x;
            NBRE_Vector2f u = up * foreSize[i].y;
            pV[0] = center - r - u + uOff;
            pV[1] = center + r - u + uOff;
            pV[2] = center + r + u + uOff;
            pV[3] = center - r + u + uOff;
        }
    }

    mVertexData->GetBuffer(0)->UnLock();
    mVertexData->GetBuffer(1)->UnLock();
    mVertexData->GetBuffer(2)->UnLock();
    mNeedUpdateVertex = FALSE;
}

void 
NBRE_TextPathEntity::UpdateTextRotate(const NBRE_Vector3d& eyePosition, const NBRE_Vector3d& cameraRight, float tilt)
{
#define PATH_LABEL_DIRECTION_COS_TOLERANCE 0.4
#define PATH_LABEL_MAX_TILE_ANGLE          45.f

    if (!mRotateEnable)
    {
        return;
    }
 
    NBRE_Vector3d rotAxis(mRotateAxis.x, mRotateAxis.y, 0);
    double cosAxisRight = cameraRight.DotProduct(rotAxis);

    if( tilt <= PATH_LABEL_MAX_TILE_ANGLE || (cosAxisRight < PATH_LABEL_DIRECTION_COS_TOLERANCE && cosAxisRight > -PATH_LABEL_DIRECTION_COS_TOLERANCE))
    {
        mLocalTransform.Identity();
        mTransform = mWorldTransform;
        return;
    }
    
    NBRE_Vector3d rotCenter(mRotateCenter.x, mRotateCenter.y, 0);
    NBRE_Ray3d axis(rotCenter, rotAxis);
    const NBRE_Vector3d& projectPoint = axis.GetClosestPointTo(eyePosition);

    NBRE_Vector3d toEye = eyePosition - projectPoint;
    toEye.Normalise();

    NBRE_Vector3d zAxis(0, 0, 1);
    NBRE_Vector3d xAxis = zAxis.CrossProduct(rotAxis);
    xAxis.Normalise();

    double rad = nsl_acos(zAxis.DotProduct(toEye));

    if( xAxis.DotProduct(toEye) > 0)
    {
        rad = -rad;
    }        
    mLocalTransform = NBRE_Transformationd::BuildRotateAtMatrix(rad, rotAxis, rotCenter);
    mTransform = mWorldTransform * mLocalTransform;
}

void 
NBRE_TextPathEntity::UpdateTextDirection()
{
}

NBRE_TextPointEntity::NBRE_TextPointEntity(NBRE_Context& context, const NBRE_WString& text, const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_Vector2f& position, PAL_TEXT_ALIGN align)
    :NBRE_TextEntity(context, text, fontMaterial, fontHeight)
    ,mPosition(position)
    ,mAlign(align)
{
    UpdateVertexBuffer();
}

NBRE_TextPointEntity::~NBRE_TextPointEntity()
{
}

void
NBRE_TextPointEntity::UpdateVertexBuffer()
{
    if (!mNeedUpdateVertex)
    {
        return;
    }

    // clean up
    NBRE_DELETE mVertexData;
    mVertexData = NULL;
    NBRE_DELETE mIndexData;
    mIndexData = NULL;
    mAABB.SetNull();

    // init pass
    mPassCache = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    mPassCache->GetTextureUnits().push_back(NBRE_TextureUnit());
    NBRE_TextureUnit tu;
    tu.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, TRUE));
    tu.SetTexture(mContext.mFontManager->GetTexture(mFontMaterial));
    tu.SetTextureCoordSet(1);
    mPassCache->GetTextureUnits().push_back(tu);
    mPassCache->SetEnableBlend(TRUE);
    mPassCache->SetEnableAlphaTest(FALSE);
    mPassCache->SetEnableDepthTest(TRUE);
    mPassCache->SetEnableDepthWrite(FALSE);
    mPassCache->SetEnableCullFace(FALSE);


    NBRE_VisualGlyphList glyphList;
    if (!mContext.mFontManager->GetTextLayout(mFontMaterial, mText, mFontHeight, TRUE, mAlign, glyphList))
    {
        return;
    }

    uint16 numChars = static_cast<uint16>(glyphList.size());

    NBRE_AxisAlignedBox2f glyphAABB;
    for (uint16 i = 0; i < numChars; ++i)
    {
        NBRE_VisualGlyph& vg = glyphList[numChars - 1 - i];
        NBRE_Glyph* glyph = mContext.mFontManager->GetGlyph(mFontMaterial, vg.fontId, vg.glyphId).get();
        if (glyph == NULL)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "Can't get glyph for %s[%d]", this->mText.c_str(), i);
            return;
        }
        glyphAABB.Merge(vg.positions);
    }
    const NBRE_Vector2f& glyphCenter = glyphAABB.GetCenter();

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 3, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
    NBRE_HardwareVertexBuffer* texcoordBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 2, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
    NBRE_HardwareVertexBuffer* colorBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 4, 4 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);


    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texcoordElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 1);
    decalration->GetVertexElementList().push_back(texcoordElem1);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->AddBuffer(texcoordBuff);
    mVertexData->AddBuffer(colorBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 6 * numChars * 2, FALSE, NBRE_HardwareBuffer::HBU_DYNAMIC);
    mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    NBRE_Vector3f* pVertex = static_cast<NBRE_Vector3f*>(vertextBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Vector2f* pTexcoord = static_cast<NBRE_Vector2f*>(texcoordBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Color* pColor = static_cast<NBRE_Color*>(colorBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(indexBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    NBRE_Color backColor = mColor * mFontMaterial.backColor;
    NBRE_Color foreColor = mColor * mFontMaterial.foreColor;
    for (uint16 i = 0; i < numChars; ++i)
    {
        NBRE_VisualGlyph& vg = glyphList[i];
        NBRE_Glyph* glyph = mContext.mFontManager->GetGlyph(mFontMaterial, vg.fontId, vg.glyphId).get();
        const NBRE_Vector2f& foregroundSize = glyph->GetForegroundSize(mFontHeight);
        const NBRE_Vector2f& backgroundSize = glyph->GetBackgroundSize(mFontHeight);
        float halfW = foregroundSize.x * 0.5f;
        float halfH = foregroundSize.y * 0.5f;
        float backHalfW = backgroundSize.x * 0.5f;
        float backHalfH = backgroundSize.y * 0.5f;

        {
            // set background vertex
            NBRE_Vector3f* pV = pVertex + i * 4;
            NBRE_Vector2f* pT = pTexcoord + i * 4;
            NBRE_Color* pC = pColor + i * 4;

            uint16 baseIndex = i * 4;
            uint16* pI = pIndex + i * 6;
            // set vertex positions
            float halfDw = backHalfW - halfW;
            float halfDh = backHalfH - halfH;
            pV[0].x = vg.positions.minExtend.x - halfDw; pV[0].y = vg.positions.minExtend.y - halfDh;
            pV[1].x = vg.positions.maxExtend.x + halfDw; pV[1].y = vg.positions.minExtend.y - halfDh;
            pV[2].x = vg.positions.maxExtend.x + halfDw; pV[2].y = vg.positions.maxExtend.y + halfDh;
            pV[3].x = vg.positions.minExtend.x - halfDw; pV[3].y = vg.positions.maxExtend.y + halfDh;
            pV[0] -= glyphCenter;
            pV[1] -= glyphCenter;
            pV[2] -= glyphCenter;
            pV[3] -= glyphCenter;
            // set vertex colors
            pC[0] = backColor;
            pC[1] = backColor;
            pC[2] = backColor;
            pC[3] = backColor;
            // set vertex texcoord
            const NBRE_Vector2f* backTexcoord = glyph->GetAtlasBackgroundTexcoord();
            pT[0] = backTexcoord[0];
            pT[1] = backTexcoord[1];
            pT[2] = backTexcoord[2];
            pT[3] = backTexcoord[3];
            // set index
            *pI++ = baseIndex + 0;
            *pI++ = baseIndex + 1;
            *pI++ = baseIndex + 2;
            *pI++ = baseIndex + 0;
            *pI++ = baseIndex + 2;
            *pI++ = baseIndex + 3;
            /// update aabb
            mAABB.Merge(pV[0].x, pV[0].y, 0);
            mAABB.Merge(pV[1].x, pV[1].y, 0);
            mAABB.Merge(pV[2].x, pV[2].y, 0);
            mAABB.Merge(pV[3].x, pV[3].y, 0);
        }

        {
            // set foreground vertex
            NBRE_Vector3f* pV = pVertex + (numChars + i) * 4;
            NBRE_Vector2f* pT = pTexcoord + (numChars + i) * 4;
            NBRE_Color* pC = pColor + (numChars + i) * 4;

            uint16 baseIndex = (numChars + i) * 4;
            uint16* pI = pIndex + (numChars + i) * 6;
            // set vertex positions
            pV[0].x = vg.positions.minExtend.x; pV[0].y = vg.positions.minExtend.y;
            pV[1].x = vg.positions.maxExtend.x; pV[1].y = vg.positions.minExtend.y;
            pV[2].x = vg.positions.maxExtend.x; pV[2].y = vg.positions.maxExtend.y;
            pV[3].x = vg.positions.minExtend.x; pV[3].y = vg.positions.maxExtend.y;
            pV[0] -= glyphCenter;
            pV[1] -= glyphCenter;
            pV[2] -= glyphCenter;
            pV[3] -= glyphCenter;
            // set vertex colors
            pC[0] = foreColor;
            pC[1] = foreColor;
            pC[2] = foreColor;
            pC[3] = foreColor;
            // set vertex texcoord
            const NBRE_Vector2f* foreTexcoord = glyph->GetAtlasForegroundTexcoord();
            pT[0] = foreTexcoord[0];
            pT[1] = foreTexcoord[1];
            pT[2] = foreTexcoord[2];
            pT[3] = foreTexcoord[3];
            // set index
            *pI++ = baseIndex + 0;
            *pI++ = baseIndex + 1;
            *pI++ = baseIndex + 2;
            *pI++ = baseIndex + 0;
            *pI++ = baseIndex + 2;
            *pI++ = baseIndex + 3;
        }
    }

    vertextBuff->UnLock();
    texcoordBuff->UnLock();
    colorBuff->UnLock();
    indexBuff->UnLock();

    mNeedUpdateVertex = FALSE;
}

void
NBRE_TextPointEntity::Rebuild()
{
    mNeedUpdateVertex = TRUE;
}

void
NBRE_TextPointEntity::SetFontMaterial(const NBRE_FontMaterial& fontMaterial)
{
    mFontMaterial = fontMaterial;
    mNeedUpdateVertex = TRUE;
}
