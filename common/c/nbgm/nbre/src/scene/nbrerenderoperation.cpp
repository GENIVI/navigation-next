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
#include "nbrerenderoperation.h"
#include "nbrerenderpal.h"
#include "nbrepass.h"

void NBRE_RenderOperation::ResetLastRenderState(NBRE_Context& context)
{
    if(context.mLastRenderState == NULL)
    {
        context.mLastRenderState = NBRE_NEW NBRE_LastRenderState;
    }
    context.mLastRenderState->mLastPass = NULL;
    context.mLastRenderState->mLastVertexData = NULL;
    context.mLastRenderState->mLastTransform = NULL;
    context.mLastRenderState->mLastColor = NBRE_Color(1,1,1,1);
    context.mLastRenderState->mLastSrcBlend = NBRE_BF_ONE;
    context.mLastRenderState->mLastDstBlend = NBRE_BF_ZERO;
    context.mLastRenderState->mLastTextureUnits = NULL;
}

NBRE_RenderOperation::NBRE_RenderOperation(NBRE_Context& context):
    mContext(context),
    mOperationType(NBRE_PMT_TRIANGLE_STRIP),
    mVertexData(NULL),
    mIndexData(NULL),
    mTransform(NULL),
    mPass(NULL),
    mTriangleNum(0)
{
    ResetLastRenderState(mContext);
}

NBRE_RenderOperation::~NBRE_RenderOperation()
{
}

void
NBRE_RenderOperation::CalculTriangleNum()
{
    if(mIndexData != NULL)
    {
        switch(mOperationType)
        {
        case NBRE_PMT_TRIANGLE_LIST:
            mTriangleNum = mIndexData->IndexCount()/3;
            break;

        case NBRE_PMT_LINE_STRIP:
            mTriangleNum = mIndexData->IndexCount()-1;
            break;

        case NBRE_PMT_POINT_LIST:
             mTriangleNum = mIndexData->IndexCount();
             break;

        case NBRE_PMT_TRIANGLE_STRIP:
        case NBRE_PMT_LINE_LIST:
        case NBRE_PMT_TRIANGLE_FAN:
             mTriangleNum = mIndexData->IndexCount()-2;
             break;

        default:
            mTriangleNum = 0;
            break;
        }
    }
}

void
NBRE_RenderOperation::SetOperationType(NBRE_PrimitiveType type)
{
    mOperationType = type;
    CalculTriangleNum();
}

void
NBRE_RenderOperation::SetIndexData(NBRE_IndexData* indexData)
{
    mIndexData = indexData;
    CalculTriangleNum();
}

void NBRE_RenderOperation::Render()
{
#ifdef DEBUG
    if(!Verify())
    {
        return;
    }
#endif
    NBRE_IRenderPal& renderPal = *(mContext.mRenderPal);

    NBRE_HBAOParamsPtr hp = mPass->GetHBAOParams();
    if (hp && hp->mEnable)
    {
        float pm[16] = {0};
        hp->mCamera->GetFrustum().ExtractMatrix().CopyTo(pm, FALSE);
        renderPal.RenderAO(hp->mOutputTarget, hp->mInputTarget, hp->mBlurTarget, hp->mAoTarget, pm, hp->mMetersToViewSpaceUnits, hp->mRadius, hp->mBias, hp->mPowerExponent
                            , hp->mDetailAO, hp->mCoarseAO, hp->mEnableBlur, hp->mBlurRadius, hp->mBlurSharpness);
    }

    ApplyPass();
    if(mContext.mLastRenderState->mLastTransform != mTransform)
    {
        renderPal.SetWorldTransform(*mTransform);
        mContext.mLastRenderState->mLastTransform = mTransform;
    }

    if (!mPass->GetGpuProgramName().empty())
    {
        renderPal.SetGpuProgram(mPass->GetGpuProgramName());
        NBRE_Map<NBRE_String, NBRE_GpuProgramParam>::const_iterator it;

        for (it = mPass->GetGpuProgramParams().begin(); it != mPass->GetGpuProgramParams().end(); ++it)
        {
            renderPal.SetGpuProgramParam(it->first, it->second);
        }

        for (it = mGpuParams.begin(); it != mGpuParams.end(); ++it)
        {
            renderPal.SetGpuProgramParam(it->first, it->second);
        }
    }
    if(mContext.mLastRenderState->mLastVertexData != mVertexData)
    {
        renderPal.SetVertexBuffer(mVertexData);
        mContext.mLastRenderState->mLastVertexData = mVertexData;
    }
    renderPal.DrawIndexedPrimitive(mOperationType, mIndexData);
    if (!mPass->GetGpuProgramName().empty())
    {
        renderPal.SetGpuProgram("");
    }
}

nb_boolean NBRE_RenderOperation::Verify()
{
    if(!mPass || !mVertexData)
    {
        return FALSE;
    }

    NBRE_VertexDeclaration *del = mVertexData->GetVertexDeclaration();

    const NBRE_VertexDeclaration::VertexElementList& decl = del->GetVertexElementList();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd = decl.end();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin();
    for (; elem != elemEnd; ++elem)
    {
        const NBRE_VertexElement* element = *elem;
        NBRE_VertexElement::VertexElementSemantic semantic = element->Semantic();
        switch (semantic)
        {
            case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
            {
                if(mPass->GetTextureUnits().size() == 0)
				{
                    //NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_RenderOperation::Verify failed: Missed texture object!");
				    return FALSE;
                }
            }
                break;
            case NBRE_VertexElement::VES_POSITION:
            case NBRE_VertexElement::VES_NORMAL:
            default:
                break;
        }
    }
    return TRUE;
}

uint32 NBRE_RenderOperation::GetTriangleNum() const
{
    return mTriangleNum;
}

void NBRE_RenderOperation::ApplyPass()
{
    if(mPass == mContext.mLastRenderState->mLastPass)
    {
        return;
    }
    mContext.mLastRenderState->mLastPass = mPass;
    NBRE_IRenderPal& renderPal = *(mContext.mRenderPal);

    renderPal.EnableAlphaTest(mPass->GetEnableAlphaTest());
    renderPal.EnableBlend(mPass->GetEnableBlend());
    renderPal.EnableCullFace(mPass->GetEnableCullFace());
    renderPal.EnableColorWrite(mPass->GetEnableColorRedWrite(), mPass->GetEnableColorGreenWrite(), mPass->GetEnableColorBlueWrite(), mPass->GetEnableColorAlphaWrite());
    renderPal.EnableDepthTest(mPass->GetEnableDepthTest());
    renderPal.EnableDepthWrite(mPass->GetEnableDepthWrite());
    renderPal.EnableFog(mPass->GetEnableFog());
    renderPal.EnableLighting(mPass->GetEnableLighting());

    renderPal.SetColor(mPass->GetColor().r, mPass->GetColor().g, mPass->GetColor().b, mPass->GetColor().a);
    if (mPass->GetSrcBlend() != mContext.mLastRenderState->mLastSrcBlend || mPass->GetDstBlend() != mContext.mLastRenderState->mLastDstBlend)
    {
        renderPal.SetBlendFunc(mPass->GetSrcBlend(), mPass->GetDstBlend());
        mContext.mLastRenderState->mLastSrcBlend = mPass->GetSrcBlend();
        mContext.mLastRenderState->mLastDstBlend = mPass->GetDstBlend();
    }
    if (mPass->GetEnableLighting())
    {
        renderPal.SetMaterial(mPass->GetMaterial());
    }

    // set textures
    const NBRE_TextureUnitList& textureUnits = mPass->GetTextureUnits();
    uint32 stage = 0;
    if(mContext.mLastRenderState->mLastTextureUnits)
    {
        for (NBRE_TextureUnitList::const_iterator i = textureUnits.begin(); i != textureUnits.end(); ++i)
        {
            const NBRE_TextureUnit& u = *i;
            NBRE_Texture* tex = u.GetTexture(mContext).get();
            
            if(mContext.mLastRenderState->mLastTextureUnits->size() > stage)
            {
                const NBRE_TextureUnit& lastUnit = (*mContext.mLastRenderState->mLastTextureUnits)[stage];
                if (lastUnit.GetTexture(mContext).get() != tex)
                {
                    renderPal.SelectTexture(stage, tex);
                    renderPal.SetTextureState(stage, u.GetState());
                }
                else if (lastUnit.GetState() != u.GetState())
                {
                    renderPal.SetTextureState(stage, u.GetState());
                }
            }
            else
            {
                renderPal.SelectTexture(stage, tex);
                renderPal.SetTextureState(stage, u.GetState());
            }
            renderPal.SetTextureTransform(stage, u.GetTransform());
            renderPal.SetTextureCoordSet(stage, u.TextureCoordSet());

            ++stage;
        }
        renderPal.DisableTextureUnitsFrom(stage);
    }
    else
    {
        for (NBRE_TextureUnitList::const_iterator i = textureUnits.begin(); i != textureUnits.end(); ++i)
        {
            const NBRE_TextureUnit& u = *i;
            NBRE_Texture* tex = u.GetTexture(mContext).get();
            renderPal.SelectTexture(stage, tex);
            renderPal.SetTextureState(stage, u.GetState());
            renderPal.SetTextureTransform(stage, u.GetTransform());
            renderPal.SetTextureCoordSet(stage, u.TextureCoordSet());
            ++stage;
        }
        renderPal.DisableTextureUnitsFrom(stage);
    }
    mContext.mLastRenderState->mLastTextureUnits = &textureUnits;

    float lineWidth = mPass->GetLineWidth();
    if (mContext.mLastRenderState->mLastLineWidth != lineWidth)
    {
        renderPal.SetLineWidth(lineWidth);
        mContext.mLastRenderState->mLastLineWidth = lineWidth;
    }
}

void NBRE_RenderOperation::SetGpuProgramParam(const NBRE_String& name, const NBRE_GpuProgramParam& param)
{
    mGpuParams[name] = param;
}
