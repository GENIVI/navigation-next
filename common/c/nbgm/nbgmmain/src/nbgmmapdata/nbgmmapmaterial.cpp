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
#include "nbgmmapmaterial.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbgmmapmaterialutility.h"
#include "nbrerenderpal.h"
#include "nbrepngtextureimage.h"
#include "nbgm.h"
#include "nbgmconst.h"
#include "nbgmbuildutility.h"
#include "nbgmmapmaterialfactory.h"
#include "nbgmmapmaterialmanager.h"

static NBRE_ShaderPtr
CreateImageOrBMPShader(PAL_Instance* palInstance, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 domTextureId)
{
    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    if (domTextureId != NBRE_INVALID_SHORT_INDEX)
    {
        NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domTextureId);
        NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
        if(texPtr.get() == NULL)
        {
            NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domTextureId, FALSE);
            texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
        }
        NBRE_TextureUnit texUnit;
        texUnit.SetTexture(texPtr);
        texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));

        pass->GetTextureUnits().push_back(texUnit);
    }
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);

    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shader->AddPass(NBRE_PassPtr(pass));

    return shader;
}

static void FillColorToBuffer(uint32 color, uint8* buffer, uint32 index)
{
    NBRE_Color c;
    c.SetAsBGRA(color);
    buffer[index] = static_cast<uint8>(c.r*255);
    buffer[index + 1] = static_cast<uint8>(c.g*255);
    buffer[index + 2] = static_cast<uint8>(c.b*255);
    buffer[index + 3] = static_cast<uint8>(c.a*255);
}

void
NBRE_MapMaterial::Load(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    if(!mLoaded)
    {
        LoadImpl(palInstance, renderPal, textureManager, dom, domMaterial, mtrlID, isCommomMaterial);
        mLoaded = TRUE;
    }
}

void
NBRE_MapStandardTexturedMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domMaterial.stm->texId);
    NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
    if(texPtr.get() == NULL)
    {
        NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domMaterial.stm->texId, TRUE);
        texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
    }
    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(texPtr);
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit);
    pass->SetColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.stm->color));
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(FALSE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapStandardTexturedMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapStandardTexturedMaterial, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back( NBRE_ShaderPtr(shader));
}

NBRE_MapStandardColorMaterial*
NBRE_MapStandardColorMaterial::CreateMemoryInstance(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const char* domName, uint32 domColor, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_MapStandardColorMaterial* mat = NBRE_NEW NBRE_MapStandardColorMaterial(mtrlID);
    mat->LoadImplInternal(palInstance, renderPal, textureManager, domName, domColor, mtrlID, isCommomMaterial);
    return mat;
}

void
NBRE_MapStandardColorMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    LoadImplInternal(palInstance, renderPal, textureManager, NBRE_DOMGetName(&dom), domMaterial.scm->color, mtrlID, isCommomMaterial);
}

void
NBRE_MapStandardColorMaterial::LoadImplInternal(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, const char* domName, uint32 domColor, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_Color color(NBRE_MapMaterialUtility::MakeColor(domColor));

    // area shader
    NBRE_TexturePtr texPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal,
            color,
            NBRE_Utility::FormatString("NBRE_MapStandardColorMaterial, color=%x", domColor));

    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(texPtr);

    NBRE_Pass* areaPass = NBRE_NEW NBRE_Pass();
    areaPass->GetTextureUnits().push_back(texUnit);
    areaPass->SetEnableLighting(FALSE);
    areaPass->SetEnableDepthTest(TRUE);
    areaPass->SetEnableDepthWrite(FALSE);
    areaPass->SetEnableCullFace(FALSE);
    areaPass->SetEnableBlend(TRUE);
    areaPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    areaPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* areaShader = NBRE_NEW NBRE_Shader();
    areaShader->AddPass(NBRE_PassPtr(areaPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_StandardColorMaterial, Area, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_StandardColorMaterial, Area, Internal, FileName=%s, id=%d", domName, mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(areaShader));

    // route shader
    NBRE_TextureUnit texUnit0;
    texUnit0.SetTexture(NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, color, "RouteShader"));
    texUnit0.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit0.SetTextureCoordSet(0);

    NBRE_TextureUnit texUnit1;
    texUnit1.SetTexture(NBGM_BuildTextureUtility::ConstructRoadTexture(renderPal, NBRE_Color(1, 1, 1, 1), VECTOR_POLYLINE_SHAPE_TEXTURE_NAME));
    texUnit1.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    texUnit1.SetTextureCoordSet(1);

    NBRE_Pass* routePass = NBRE_NEW NBRE_Pass();
    routePass->GetTextureUnits().push_back(texUnit0);
    routePass->GetTextureUnits().push_back(texUnit1);
    routePass->SetEnableLighting(FALSE);
    routePass->SetEnableDepthTest(TRUE);
    routePass->SetEnableDepthWrite(FALSE);
    routePass->SetEnableCullFace(FALSE);
    routePass->SetEnableBlend(TRUE);
    routePass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    routePass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* routeShader = NBRE_NEW NBRE_Shader();
    routeShader->AddPass(NBRE_PassPtr(routePass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_StandardColorMaterial, Route, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_StandardColorMaterial, Route, Internal, FileName=%s, id=%d", domName, mtrlID));
    }

    mShaders.push_back(NBRE_ShaderPtr(routeShader));
}

void
NBRE_MapLightedMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domMaterial.lm->texId);
    NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
    if(texPtr.get() == NULL)
    {
        NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domMaterial.lm->texId, TRUE);
        texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
    }
    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(texPtr);

    NBRE_Material mat;
    mat.SetDiffuse(NBRE_MapMaterialUtility::MakeColor(domMaterial.lm->linghting->diffuse));
    mat.SetAmbient(NBRE_MapMaterialUtility::MakeColor(domMaterial.lm->linghting->ambient));
    mat.SetSpecular(NBRE_MapMaterialUtility::MakeColor(domMaterial.lm->linghting->specular));
    mat.SetEmissive(NBRE_MapMaterialUtility::MakeColor(domMaterial.lm->linghting->emission));
    mat.SetShiness(domMaterial.lm->linghting->shininess);

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit);
    pass->SetColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.lm->color));
    pass->SetMaterial(mat);
    pass->SetEnableLighting(TRUE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    mShaders.push_back(NBRE_ShaderPtr(shader));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapLightedMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapLightedMaterial, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
}

void
NBRE_MapFontMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& /*dom*/, NBRE_DOM_Material& domMaterial, uint16 /*mtrlID*/, nb_boolean /*isCommomMaterial*/)
{
    // Note: The font size in common material is based on capital letter height, not normal font height
    static const float fontSizeFactor = 1.31f;
    if (NBGM_GetConfig()->dpi < 160.f)
    {
        float fontMagnifierFactor = NBGM_GetConfig()->fontMagnifierFactor;
        if (fontMagnifierFactor == 0.f)
        {
            fontMagnifierFactor = 1.f;
        }
        mFontMaterial.minFontHeightPixels = DP_TO_PIXEL(domMaterial.fm->minimum + 10 * (1.f - NBGM_GetConfig()->dpi / 160.f)) * fontSizeFactor * fontMagnifierFactor;
        mFontMaterial.maxFontHeightPixels = DP_TO_PIXEL(domMaterial.fm->maximum + 10 * (1.f - NBGM_GetConfig()->dpi / 160.f)) * fontSizeFactor * fontMagnifierFactor;
    }
    else
    {
        mFontMaterial.minFontHeightPixels = DP_TO_PIXEL(domMaterial.fm->minimum) * fontSizeFactor;
        mFontMaterial.maxFontHeightPixels = DP_TO_PIXEL(domMaterial.fm->maximum) * fontSizeFactor;
    }
    mFontMaterial.foreColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.fm->textColor);
    if (mFontMaterial.foreColor.a == 0.0f)
    {
        mFontMaterial.foreColor.a = 1.0f;
    }
    mFontMaterial.backColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.fm->outlineColor);
    mFontMaterial.spacing = domMaterial.fm->latterSpacing/100.f;

    mIsWatermark = domMaterial.fm->style & 0x2 ? TRUE : FALSE;
    uint8 style = domMaterial.fm->style & 0x1;
    uint8 isItalic = domMaterial.fm->style & 0x4 ? TRUE : FALSE;
    if (domMaterial.fm->family == 0)//SansSerif
    {
        if (style == 0)//Normal
        {
            if (isItalic)
            {
                mFontMaterial.font = FONT_NAME_SANS_ITALIC;
            }
            else
            {
                mFontMaterial.font = FONT_NAME_SANS;
            }
        }
        else if (style == 1)//Bold
        {
            if (isItalic)
            {
                mFontMaterial.font = FONT_NAME_SANS_BOLD_ITALIC;
            }
            else
            {
                mFontMaterial.font = FONT_NAME_SANS_BOLD;
            }
        }
    }
    else if (domMaterial.fm->family == 1)//Serif
    {
        if (style == 0)//Normal
        {
            if (isItalic)
            {
                mFontMaterial.font = FONT_NAME_SERIF_ITALIC;
            }
            else
            {
                mFontMaterial.font = FONT_NAME_SERIF;
            }
        }
        else if (style == 1)//Bold
        {
            if (isItalic)
            {
                mFontMaterial.font = FONT_NAME_SERIF_BOLD_ITALIC;
            }
            else
            {
                mFontMaterial.font = FONT_NAME_SERIF_BOLD;
            }
        }
    }
}

void
NBRE_MapShieldMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_TexturePtr texPtr;
    if (mDependGroup == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapShieldMaterial::LoadImpl() failed. No base material loaded.");
        return;
    }

    if (domMaterial.type == NBRE_DMT_ShieldWithYGapMaterial)
    {
        const NBRE_MapMaterial& iconMat = mDependGroup->GetMapMaterial(mDependGroup->GetChunkFlag(), MAKE_NBM_INDEX(domMaterial.sym->iconIndex));
        if (iconMat.Type() != NBRE_MMT_StandardTexturedMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapShieldMaterial::LoadImpl() failed. Can't find icon index=%d", domMaterial.sym->iconIndex);
            return;
        }
        mImageShader = ((const NBRE_MapStandardTexturedMaterial&)iconMat).GetShader(0);
        texPtr = mImageShader->GetPass(0)->GetTextureUnits()[0].GetTexture();

        mFontMaterialIndex = domMaterial.sym->materialIndex;
        mOffset.x = static_cast<float>(domMaterial.sym->xOffset) / texPtr->GetOriginalWidth();
        mOffset.y = -static_cast<float>(domMaterial.sym->yOffset) / texPtr->GetOriginalHeight();
        mPadding.x = domMaterial.sym->xPadding / 100.0f;
        mPadding.y = domMaterial.sym->yPadding / 100.0f;
    }
    else // old shield material without y gap
    {
        const NBRE_MapMaterial& iconMat = mDependGroup->GetMapMaterial(mDependGroup->GetChunkFlag(), MAKE_NBM_INDEX(domMaterial.sm->iconIndex));
        if (iconMat.Type() != NBRE_MMT_StandardTexturedMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapShieldMaterial::LoadImpl() failed. Can't find icon index=%d", domMaterial.sm->iconIndex);
            return;
        }
        mImageShader = ((const NBRE_MapStandardTexturedMaterial&)iconMat).GetShader(0);
        texPtr = mImageShader->GetPass(0)->GetTextureUnits()[0].GetTexture();

        mFontMaterialIndex = domMaterial.sm->materialIndex;
        mOffset.x = static_cast<float>(domMaterial.sm->xOffset) / texPtr->GetOriginalWidth();
        mOffset.y = -static_cast<float>(domMaterial.sm->yOffset) / texPtr->GetOriginalHeight();
        mPadding.x = domMaterial.sm->xPadding / 100.0f;
        mPadding.y = mPadding.x;
    }

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapShieldMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapShieldMaterial, Internal, FileName = %s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
}

void
NBRE_MapBackgroundColorMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.bgm->color);
    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();

    if (domMaterial.bgm->textureIndex != NBRE_INVALID_SHORT_INDEX)
    {
        NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domMaterial.bgm->textureIndex);
        NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
        if(texPtr.get() == NULL)
        {
            NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domMaterial.bgm->textureIndex, TRUE);
            texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
        }
        NBRE_TextureUnit texUnit;
        texUnit.SetTexture(texPtr);
        texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));

        pass->GetTextureUnits().push_back(texUnit);
    }

    pass->SetColor(mColor);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MMT_BackgroundMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MMT_BackgroundMaterial, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(shader));
}

void
NBRE_PinMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mXImageOffset = domMaterial.pinm->xImageOffset/100.f;
    mYImageOffset = domMaterial.pinm->yImageOffset/100.f;
    mXBubbleOffset = domMaterial.pinm->xBubbleOffset/100.f;
    mYBubbleOffset = domMaterial.pinm->yBubbleOffset/100.f;

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();

    if (domMaterial.pinm->iconIndex != NBRE_INVALID_SHORT_INDEX)
    {
        NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domMaterial.pinm->iconIndex);
        NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
        if(texPtr.get() == NULL)
        {
            NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domMaterial.pinm->iconIndex, FALSE);
            texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
        }
        NBRE_TextureUnit texUnit;
        texUnit.SetTexture(texPtr);
        texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));

        pass->GetTextureUnits().push_back(texUnit);
    }
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_PinMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_PinMaterial, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(shader));
}

void NBRE_OutlinedComplexColorFillMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial )
{
    // area interial shader
    NBRE_String interialTexName = NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial Interior id=%d", mtrlID);
    NBRE_Color interialColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.ocfm->interiorFillColor));

    NBRE_TexturePtr interialTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, interialColor, interialTexName);
    NBRE_TextureUnit interialTexUnit;
    interialTexUnit.SetTexture(interialTexPtr);

    NBRE_Pass* interialPass = NBRE_NEW NBRE_Pass();
    interialPass->GetTextureUnits().push_back(interialTexUnit);
    interialPass->SetEnableLighting(FALSE);
    interialPass->SetEnableDepthTest(TRUE);
    interialPass->SetEnableDepthWrite(FALSE);
    interialPass->SetEnableCullFace(FALSE);
    interialPass->SetEnableBlend(TRUE);
    interialPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    interialPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* interialShader = NBRE_NEW NBRE_Shader();
    interialShader->AddPass(NBRE_PassPtr(interialPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Interior, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Interior, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(interialShader));

    // area outlined shader
    NBRE_String outlinedTexName = NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial Outlined id=%d", mtrlID);
    NBRE_Color outlinedColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.ocfm->outlineColor));

    NBRE_TexturePtr outlinedTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, outlinedColor, outlinedTexName);
    NBRE_TextureUnit outlinedTexUnit;
    outlinedTexUnit.SetTexture(outlinedTexPtr);

    NBRE_Pass* outlinedPass = NBRE_NEW NBRE_Pass();
    outlinedPass->GetTextureUnits().push_back(outlinedTexUnit);
    outlinedPass->SetEnableLighting(FALSE);
    outlinedPass->SetEnableDepthTest(TRUE);
    outlinedPass->SetEnableDepthWrite(FALSE);
    outlinedPass->SetEnableCullFace(FALSE);

    float pixels = DP_TO_PIXEL(domMaterial.ocfm->width);
    outlinedPass->SetLineWidth(pixels);

    NBRE_Shader* outlinedShader = NBRE_NEW NBRE_Shader();
    outlinedShader->AddPass(NBRE_PassPtr(outlinedPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Outlined, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Outlined, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(outlinedShader));
}

void NBRE_OutlinedSimpleColorFillMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial )
{
    // area interial shader
    NBRE_String interialTexName = NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial Interior id=%d", mtrlID);
    NBRE_Color interialColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.osfm->interiorFillColor));
    NBRE_TexturePtr interialTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, interialColor, interialTexName);
    NBRE_TextureUnit interialTexUnit;
    interialTexUnit.SetTexture(interialTexPtr);

    NBRE_Pass* interialPass = NBRE_NEW NBRE_Pass();
    interialPass->GetTextureUnits().push_back(interialTexUnit);
    interialPass->SetEnableLighting(FALSE);
    interialPass->SetEnableDepthTest(TRUE);
    interialPass->SetEnableDepthWrite(FALSE);
    interialPass->SetEnableCullFace(FALSE);
    interialPass->SetEnableBlend(TRUE);
    interialPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    interialPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* interialShader = NBRE_NEW NBRE_Shader();
    interialShader->AddPass(NBRE_PassPtr(interialPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial, Interior, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial, Interior, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(interialShader));

    // area outlined shader
    NBRE_String outlinedTexName = NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial Outlined id=%d", mtrlID);
    NBRE_Color outlinedColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.osfm->outlineColor));

    NBRE_TexturePtr outlinedTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, outlinedColor, outlinedTexName);
    NBRE_TextureUnit outlinedTexUnit;
    outlinedTexUnit.SetTexture(outlinedTexPtr);

    NBRE_Pass* outlinedPass = NBRE_NEW NBRE_Pass();
    outlinedPass->GetTextureUnits().push_back(outlinedTexUnit);
    outlinedPass->SetEnableLighting(FALSE);
    outlinedPass->SetEnableDepthTest(TRUE);
    outlinedPass->SetEnableDepthWrite(FALSE);
    outlinedPass->SetEnableCullFace(FALSE);

    float pixels = DP_TO_PIXEL(domMaterial.osfm->width);
    outlinedPass->SetLineWidth(pixels);

    NBRE_Shader* outlinedShader = NBRE_NEW NBRE_Shader();
    outlinedShader->AddPass(NBRE_PassPtr(outlinedPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial, Outlined, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedSimpleColorFillMaterial, Outlined, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(outlinedShader));
}

void NBRE_OutlinedHoleyMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial )
{
    // area interial shader
    NBRE_String interialTexName = NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial Interior id=%d", mtrlID);
    NBRE_Color interialColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.ohm->interiorFillColor));

    NBRE_TexturePtr interialTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, interialColor, interialTexName);
    NBRE_TextureUnit interialTexUnit;
    interialTexUnit.SetTexture(interialTexPtr);

    NBRE_Pass* interialPass = NBRE_NEW NBRE_Pass();
    interialPass->GetTextureUnits().push_back(interialTexUnit);
    interialPass->SetEnableLighting(FALSE);
    interialPass->SetEnableDepthTest(TRUE);
    interialPass->SetEnableDepthWrite(FALSE);
    interialPass->SetEnableCullFace(FALSE);
    interialPass->SetEnableBlend(TRUE);
    interialPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    interialPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_Shader* interialShader = NBRE_NEW NBRE_Shader();
    interialShader->AddPass(NBRE_PassPtr(interialPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Interior, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Interior, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(interialShader));

    // area outlined shader
    NBRE_String outlinedTexName = NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial Outlined id=%d", mtrlID);
    NBRE_Color outlinedColor(NBRE_MapMaterialUtility::MakeColor(domMaterial.ohm->outlineColor));

    NBRE_TexturePtr outlinedTexPtr = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, outlinedColor, outlinedTexName);
    NBRE_TextureUnit outlinedTexUnit;
    outlinedTexUnit.SetTexture(outlinedTexPtr);

    NBRE_Pass* outlinedPass = NBRE_NEW NBRE_Pass();
    outlinedPass->GetTextureUnits().push_back(outlinedTexUnit);
    outlinedPass->SetEnableLighting(FALSE);
    outlinedPass->SetEnableDepthTest(TRUE);
    outlinedPass->SetEnableDepthWrite(FALSE);
    outlinedPass->SetEnableCullFace(FALSE);

    float pixels = DP_TO_PIXEL(domMaterial.ohm->width);
    outlinedPass->SetLineWidth(pixels);

    NBRE_Shader* outlinedShader = NBRE_NEW NBRE_Shader();
    outlinedShader->AddPass(NBRE_PassPtr(outlinedPass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Outlined, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("OutlinedComplexColorFillMaterial, Outlined, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(outlinedShader));
}

void NBRE_RadialPINMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mNormalXImageOffset = domMaterial.rpm->unSelectedCalloutXCenterOffset/100.f;
    mNormalYImageOffset = domMaterial.rpm->unSelectedCalloutYCenterOffset/100.f;
    mNormalXBubbleOffset = domMaterial.rpm->unSelectedBubbleXCenterOffset/100.f;
    mNormalYBubbleOffset = domMaterial.rpm->unSelectedBubbleYCenterOffset/100.f;

    mSelectedXImageOffset = domMaterial.rpm->selectedCalloutXCenterOffset/100.f;
    mSelectedYImageOffset = domMaterial.rpm->selectedCalloutYCenterOffset/100.f;
    mSelectedXBubbleOffset = domMaterial.rpm->selectedBubbleXCenterOffset/100.f;
    mSelectedYBubbleOffset = domMaterial.rpm->selectedBubbleYCenterOffset/100.f;

    mNormalWidth = DP_TO_PIXEL(domMaterial.rpm->displayNormalWidth);
    mNormalHeight = DP_TO_PIXEL(domMaterial.rpm->dispalyNormalHeight);
    mSelectedWidth = DP_TO_PIXEL(domMaterial.rpm->displaySelectedWidth);
    mSelectedHeight = DP_TO_PIXEL(domMaterial.rpm->dispalySelectedHeight);

    mShaders.push_back(CreateImageOrBMPShader(palInstance, textureManager, dom, domMaterial.rpm->unSelectedBMP));
    mShaders.push_back(CreateImageOrBMPShader(palInstance, textureManager, dom, domMaterial.rpm->selectedBMP));

    NBRE_Color color = NBRE_MapMaterialUtility::MakeColor(domMaterial.rpm->unSelectedCircleInteriorColor);
    NBRE_String name = NBRE_Utility::FormatString("NBRE_RadialPINMaterial Normal Interior Halo id=%d", mtrlID);
    mShaders.push_back(CreateHaloShader(renderPal, color, name, 0.0f));

    color = NBRE_MapMaterialUtility::MakeColor(domMaterial.rpm->unSelectedCircleOutlineBitOnColor);
    name = NBRE_Utility::FormatString("NBRE_RadialPINMaterial Normal Outline Halo id=%d", mtrlID);
    mShaders.push_back(CreateHaloShader(renderPal, color, name, DP_TO_PIXEL(domMaterial.rpm->circleOutlineWidth)));

    color = NBRE_MapMaterialUtility::MakeColor(domMaterial.rpm->selectedCircleInteriorColor);
    name = NBRE_Utility::FormatString("NBRE_RadialPINMaterial Selected Interior Halo id=%d", mtrlID);
    mShaders.push_back(CreateHaloShader(renderPal, color, name, 0.0f));

    color = NBRE_MapMaterialUtility::MakeColor(domMaterial.rpm->selectedCircleOutlineBitOnColor);
    name = NBRE_Utility::FormatString("NBRE_RadialPINMaterial Selected Outline Halo id=%d", mtrlID);
    mShaders.push_back(CreateHaloShader(renderPal, color, name, DP_TO_PIXEL(domMaterial.rpm->circleOutlineWidth)));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal Halo Interior External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal Halo Outline External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected Halo Interior External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected Halo Outline External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal Halo Interior Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Normal Halo Outline Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected Halo Interior Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_RadialPINMaterial, Selected Halo Outline Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
}

NBRE_ShaderPtr NBRE_RadialPINMaterial::CreateHaloShader(NBRE_IRenderPal& renderPal, const NBRE_Color& clr, const NBRE_String& texName, float lineWidth)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructColorTexture(renderPal, clr, texName);
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

void
NBRE_MapPathArrowMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& /*dom*/, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_DOM_PathArrowMaterial* pam = domMaterial.pam;

    float pixelResolution = CalcPixelResolution(MAP_PATH_ARROW_ZOOMLEVEL_RANGE[0]);
    mLength = METER_TO_MERCATOR(DP_TO_PIXEL(pam->tailLength) * pixelResolution);
    mRepeat = mLength * (pam->repeat + 1);

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(NBGM_BuildTextureUtility::ConstructPathArrowTexture(renderPal, NBRE_MapMaterialUtility::MakeColor(pam->arrowColor), pam->tailWidth, pam->headWidth, pam->tailLength, pam->headLength, "NBRE_MapPathArrowMaterial,id=%d"));
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    pass->GetTextureUnits().push_back(texUnit);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapPathArrowMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapPathArrowMaterial, Internal, id=%d", mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(shader));
}

void
NBRE_StaticPOIMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mXImageOffset = domMaterial.spm->calloutXCenterOffset/100.f;
    mYImageOffset = domMaterial.spm->calloutYCenterOffset/100.f;
    mXBubbleOffset = domMaterial.spm->bubbleXCenterOffset/100.f;
    mYBubbleOffset = domMaterial.spm->bubbleYCenterOffset/100.f;

    mDistanceToAnother = DP_TO_PIXEL((uint8)domMaterial.spm->distanceToAnother);
    mDistanceToLabel = DP_TO_PIXEL((uint8)domMaterial.spm->distanceToLabel);
    mDistanceToPOI = DP_TO_PIXEL((uint8)domMaterial.spm->distanceToPoi);
    mIconWidth = DP_TO_PIXEL(domMaterial.spm->iconWidth);
    mIconHeight = DP_TO_PIXEL(domMaterial.spm->iconHeight);

    if (!isCommomMaterial)
    {
        NBRE_ShaderPtr normalShader = CreateImageOrBMPShader(palInstance, textureManager, dom, domMaterial.spm->iconBPMIndex);
        NBRE_ShaderPtr selectShader = CreateImageOrBMPShader(palInstance, textureManager, dom, domMaterial.spm->selectedBMPIndex);
        NBRE_ShaderPtr unSelcetShader = CreateImageOrBMPShader(palInstance, textureManager, dom, domMaterial.spm->unSelectedBMPIndex);
        mShaders.push_back(normalShader);
        mShaders.push_back(selectShader);
        mShaders.push_back(unSelcetShader);
    }
    else
    {
        if (mDependGroup == NULL)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_StaticPOIMaterial::LoadImpl() failed. Can't find depend material");
            return;
        }
        uint16 chunkFlag = mDependGroup->GetChunkFlag();
        const NBRE_MapMaterial& iconBPMIndex = mDependGroup->GetMapMaterial(chunkFlag, MAKE_NBM_INDEX(domMaterial.spm->iconBPMIndex));
        if (iconBPMIndex.Type() != NBRE_MMT_StandardTexturedMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_StaticPOIMaterial::LoadImpl() failed. Can't find icon index=%d", domMaterial.spm->iconBPMIndex);
            return;
        }
        const NBRE_MapMaterial& selectedBMPIndex = mDependGroup->GetMapMaterial(chunkFlag, MAKE_NBM_INDEX(domMaterial.spm->selectedBMPIndex));
        if (selectedBMPIndex.Type() != NBRE_MMT_StandardTexturedMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_StaticPOIMaterial::LoadImpl() failed. Can't find icon index=%d", domMaterial.sym->iconIndex);
            return;
        }
        const NBRE_MapMaterial& unSelectedBMPIndex = mDependGroup->GetMapMaterial(chunkFlag, MAKE_NBM_INDEX(domMaterial.spm->unSelectedBMPIndex));
        if (unSelectedBMPIndex.Type() != NBRE_MMT_StandardTexturedMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_StaticPOIMaterial::LoadImpl() failed. Can't find icon index=%d", domMaterial.spm->unSelectedBMPIndex);
            return;
        }
        NBRE_ShaderPtr normalShader = iconBPMIndex.GetShader(0);
        NBRE_ShaderPtr selectShader = selectedBMPIndex.GetShader(0);
        NBRE_ShaderPtr unSelcetShader = unSelectedBMPIndex.GetShader(0);
        mShaders.push_back(normalShader);
        mShaders.push_back(selectShader);
        mShaders.push_back(unSelcetShader);
    }

    if(isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Normal External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Selected External, id=%d", mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Unselected External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Normal Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Selected Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
        mShaderNames.push_back( NBRE_Utility::FormatString("StaticPOIMaterial, Selected Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
}

NBGM_PolylineCapType
NBRE_PolylineCapMaterial::GetCapType(uint8 type) const
{
    NBGM_PolylineCapType capType = NBGM_PCT_Arrow;

    switch (type)
    {
    case 0:
        capType = NBGM_PCT_Arrow;
        break;
    case 1:
        capType = NBGM_PCT_Circle;
        break;
    case 2:
        capType = NBGM_PCT_LineJoin;
        break;
    default:
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_PolylineCapMaterial::GetCapType Unsupported cap type = %u", type);
        break;
    }

    return capType;
}

void
NBRE_PolylineCapMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal,
                                   NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial,
                                   uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mStartType = GetCapType(domMaterial.pcm->startType);
    mEndType = GetCapType(domMaterial.pcm->endType);
    mOutLineWidthPercentage = domMaterial.pcm->outlineWidth/100.f;
    mShapeHeightPercentage = domMaterial.pcm->height/100.f;
    mShapeWidthPercentage = domMaterial.pcm->width/100.f;
    mRadiusPercentage = domMaterial.pcm->radius/100.f;

    if(mOutLineWidthPercentage > 0.5f)
    {
        mOutLineWidthPercentage = 0.5f;
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_PolylineCapMaterial::LoadImpl outlineWidth is too broad!\
                                             outlineWidth = %u%", domMaterial.pcm->outlineWidth);
    }

    if(isCommomMaterial)
    {
        NBRE_ImageInfo imageInfo;
        mContext->paletteImage->GetInfo(imageInfo);
        int32 x, y;
        NBRE_Color c;

        c.SetAsBGRA(domMaterial.pcm->interiorColor);
        x = mContext->palettePixelOffset%imageInfo.mWidth;
        y = mContext->palettePixelOffset/imageInfo.mWidth;
        ++mContext->palettePixelOffset;
        //c.a = 0;
        mContext->paletteImage->SetPixel(x, y, c);
        mInteriorTexcoord.x = (x + 0.5f)/imageInfo.mWidth;
        mInteriorTexcoord.y = (y + 0.5f)/imageInfo.mHeight;

        c.SetAsBGRA(domMaterial.pcm->outlineColor);
        x = mContext->palettePixelOffset%imageInfo.mWidth;
        y = mContext->palettePixelOffset/imageInfo.mWidth;
        ++mContext->palettePixelOffset;
        mContext->paletteImage->SetPixel(x, y, c);
        mOutlineTexcoord.x = (x + 0.5f)/imageInfo.mWidth;
        mOutlineTexcoord.y = (y + 0.5f)/imageInfo.mHeight;
        mShaderNames.push_back(VECTOR_OUTLINE_POLYLINE_TEXTURE_NAME);
    }
    else
    {
        NBRE_Image *image = NBRE_NEW NBRE_Image(2, 1, NBRE_PF_R8G8B8A8);
        uint8 *buffer = image->GetImageData();

        FillColorToBuffer(domMaterial.pcm->interiorColor, buffer, 0);
        FillColorToBuffer(domMaterial.pcm->outlineColor, buffer, 4);

        mInteriorTexcoord.x = 0.25f;
        mInteriorTexcoord.y = 0.5f;
        mOutlineTexcoord.x = 0.75f;
        mOutlineTexcoord.y = 0.5f;

        NBRE_ShaderPtr shader = NBGM_BuildShaderUtility::CreatePolylineShader(renderPal, image);
        mShaders.push_back(shader);
        mShaderNames.push_back(NBRE_Utility::FormatString("NBRE_PolylineCapMaterial, internal, dom=%s, id=%d",
                                                           NBRE_DOMGetName(&dom), mtrlID));
    }
}

void
NBRE_BitMapPatternMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& /*renderPal*/, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.bmpm->color);
    mDistance = domMaterial.bmpm->distance;
    mWidth = domMaterial.bmpm->width;
    mHeight = domMaterial.bmpm->height;

    NBRE_String texName = NBRE_MapMaterialUtility::CreateTexureName(dom, domMaterial.bmpm->textureID);
    NBRE_TexturePtr texPtr = textureManager.GetTexture(texName);
    if(texPtr.get() == NULL)
    {
        NBRE_ITextureImage* texImage = NBRE_MapMaterialUtility::CreateTextureImage(*palInstance, dom, domMaterial.bmpm->textureID, FALSE);
        if (mDistance > 0)
        {
            // Convert the material to 0 distance
            // Add blank in the image
            texImage->Load();
            NBRE_Image* image = ((NBRE_PngTextureImage*)texImage)->GetImage();
            image->Extend(mWidth + mDistance, mHeight);
            // change image size
            mWidth += mDistance;
            mDistance = 0;
        }
        texPtr = textureManager.CreateTexture(texName, texImage, NBRE_Texture::TT_2D, TRUE);
    }
    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(texPtr);
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit);
    pass->SetColor(mColor);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_BitMapPatternMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_BitMapPatternMaterial, Internal, FileName=%s, id=%d", NBRE_DOMGetName(&dom), mtrlID));
    }
    mShaders.push_back( NBRE_ShaderPtr(shader));
}
