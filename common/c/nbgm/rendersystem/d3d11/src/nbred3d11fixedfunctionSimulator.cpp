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
#include "nbred3d11fixedfunctionSimulator.h"
#include "nbrelog.h"
#include "nbred3d11renderpal.h"
#include "nbretypeconvert.h"

#define BIT_SET(x,n)    ((x) |=  (0x01<<(n)))
#define BIT_CLEAR(x,n)  ((x) &= ~(0x01<<(n)))
#define BIT(x,n)        (((x) & (0x01<<(n)))?TRUE:FALSE)

static NBRE_Matrix4x4f gIdentityMatrix(1.f,   0,   0,  0,
                                         0, 1.f,   0,  0,
                                         0,   0, 1.f,  0,
                                         0,   0,   0, 1.f);
static D3D11_TEXTURE_ADDRESS_MODE
ToD3DAddressMode(NBRE_TextureAddressMode mode)
{
    switch(mode)
    {
    case NBRE_TAM_REPEAT:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case NBRE_TAM_CLAMP:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    default:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    }
};

static D3D11_FILTER
ToD3DFilter(const NBRE_TextureState &state)
{
    if(state.GetMagFilter() == NBRE_TFT_POINT && state.GetMinFilter() == NBRE_TFT_POINT)
        return D3D11_FILTER_MIN_MAG_MIP_POINT;

    if(state.GetMagFilter() == NBRE_TFT_LINEAR && state.GetMinFilter() == NBRE_TFT_POINT)
        return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

    if(state.GetMagFilter() == NBRE_TFT_POINT && state.GetMinFilter() == NBRE_TFT_LINEAR)
        return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;

    if(state.GetMagFilter() == NBRE_TFT_LINEAR && state.GetMinFilter() == NBRE_TFT_LINEAR)
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

static uint32
GetTextureStateKey(const NBRE_TextureState& state)
{
    uint32 key = 0;
    if(state.GetWrapS() == NBRE_TAM_REPEAT)
    {
        BIT_SET(key, 0);
    }

    if(state.GetWrapT() == NBRE_TAM_REPEAT)
    {
        BIT_SET(key, 1);
    }

    if(state.GetMinFilter() == NBRE_TFT_POINT)
    {
        BIT_SET(key, 2);
    }

    if(state.GetMagFilter() == NBRE_TFT_POINT)
    {
        BIT_SET(key, 3);
    }

    if(state.UsingAnisotropic())
    {
        BIT_SET(key, 4);
    }

    return key;
}

static ID3D11SamplerState*
BuildSampleState(const NBRE_TextureState& state, ID3D11Device* device)
{
    ID3D11SamplerState* sampler = NULL;
    D3D11_SAMPLER_DESC des;
    ZeroMemory(&des, sizeof(D3D11_SAMPLER_DESC));
    if(state.UsingAnisotropic())
    {
        des.MaxAnisotropy = 4;
        des.Filter = D3D11_FILTER_ANISOTROPIC;
    }
    else
    {
        des.MaxAnisotropy = 1;
        des.Filter = ToD3DFilter(state);
    }
    des.AddressU = ToD3DAddressMode(state.GetWrapS());
    des.AddressV = ToD3DAddressMode(state.GetWrapT());
    des.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    des.ComparisonFunc = D3D11_COMPARISON_NEVER;
    des.MinLOD = 0;
    des.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&des, &sampler);
    return sampler;
}

NBRE_D3D11FixedFuncSimulator::NBRE_D3D11FixedFuncSimulator(NBRE_D3D11RenderPal *pal):mPal(pal),
    mWorldBuffer(sizeof(NBRE_Matrix4x4f), 0, pal->GetActiveD3D11Device()),
    mViewBuffer(sizeof(NBRE_Matrix4x4f), 1, pal->GetActiveD3D11Device()),
    mProjectionBuffer(sizeof(NBRE_Matrix4x4f), 2, pal->GetActiveD3D11Device()),
    mTextureBuffer(sizeof(mState->mMatrix.texture), 3, pal->GetActiveD3D11Device()),
    mTextureEnableBuffer(sizeof(mState->mTextureEnbale), 4, pal->GetActiveD3D11Device()),
    mLightEnableBuffer(sizeof(mState->mLightEnbale), 5, pal->GetActiveD3D11Device()),
    mColorBuffer(sizeof(mState->mCurrentColor), 6, pal->GetActiveD3D11Device()),
    mMaterialBuffer(sizeof(mState->mMat), 7, pal->GetActiveD3D11Device()),
    mLightsBuffer(sizeof(mState->mLights), 8, pal->GetActiveD3D11Device()),
    mTextureTypeBuffer(sizeof(mState->mTextureType), 9, pal->GetActiveD3D11Device()),
    mLastLights(0)
{
    InitState();
}

NBRE_D3D11FixedFuncSimulator::~NBRE_D3D11FixedFuncSimulator()
{
    for( SamplerCache::iterator it = mSamplerCache.begin(); it != mSamplerCache.end(); it++)
    {
        NBRE_SAFE_RELEASE(it->second);
    }
    NBRE_DELETE mState;
}

void
NBRE_D3D11FixedFuncSimulator::InitState()
{
    mState = NBRE_NEW NBRE_D3D11RenderState();
    nsl_memset(mState, 0, sizeof(NBRE_D3D11RenderState));
    float texMatrix[4] = {1,0,1,0};
    for(int i = 0; i < NBRE_D3D11_TEXTRUE_NUM; i++)
    {
        nsl_memcpy(mState->mMatrix.texture + i*4, texMatrix, sizeof(texMatrix));
        mState->mTexState[i].key = 0xFFFFFFFF;
    }
    mState->mMat.mat = NBRE_Material();
    mState->mTextureType = 0xFFFFFFFF;
    mState->mCurrentColor[0] = 1.f;
    mState->mCurrentColor[1] = 1.f;
    mState->mCurrentColor[2] = 1.f;
    mState->mCurrentColor[3] = 1.f;
    UpdataGeneralBuffer();
}

PAL_Error
NBRE_D3D11FixedFuncSimulator::LoadShader(NBRE_D3D11GpuProgramPtr shader)
{
    nbre_assert(!shader.IsNull());
    PAL_Error err = PAL_Ok;
    switch(shader->GetType())
    {
    case NBRE_GPT_VERTEX_PROGRAM:
        if(mVertexShader.Get() != shader.Get())
        {
            mVertexShader = shader;
            err = shader->Load();
            if(err == PAL_Ok)
            {
                mPal->GetImmediateContext()->VSSetShader(mVertexShader->GetVertexShader(), NULL, 0);
            }
        }
        break;
    case NBRE_GPT_PIXEL_PROGRAM:
        if(mPixelShader.Get() != shader.Get())
        {
            mPixelShader = shader;
            err = shader->Load();
            if(err == PAL_Ok)
            {
                mPal->GetImmediateContext()->PSSetShader(mPixelShader->GetPixelShader(), NULL, 0);
            }
        }
        break;
    case NBRE_GPT_GEOMETRY_PROGRAM:
        if(mGeometryShader.Get() != shader.Get())
        {
            mGeometryShader = shader;
            err = shader->Load();
            if(err == PAL_Ok)
            {
                mPal->GetImmediateContext()->GSSetShader(mGeometryShader->GetGeometryShader(), NULL, 0);
            }
        }
        break;
    default:
        nbre_assert(0);
    }

    return err;
}

void
NBRE_D3D11FixedFuncSimulator::UpdataGeneralBuffer()
{
    mPal->GetImmediateContext()->UpdateSubresource(mWorldBuffer.GetBuffer(), 0, NULL, &(gIdentityMatrix), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mViewBuffer.GetBuffer(), 0, NULL, &(gIdentityMatrix), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mProjectionBuffer.GetBuffer(), 0, NULL, &(gIdentityMatrix), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mTextureBuffer.GetBuffer(), 0, NULL, &(mState->mMatrix.texture), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mTextureEnableBuffer.GetBuffer(), 0, NULL, &(mState->mTextureEnbale), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mLightEnableBuffer.GetBuffer(), 0, NULL, &(mState->mLightEnbale), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mMaterialBuffer.GetBuffer(), 0, NULL, &(mState->mMat), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mColorBuffer.GetBuffer(), 0, NULL, &(mState->mCurrentColor), 0, 0);
    mPal->GetImmediateContext()->UpdateSubresource(mTextureTypeBuffer.GetBuffer(), 0, NULL, &(mState->mTextureType), 0, 0);
}

void
NBRE_D3D11FixedFuncSimulator::SetProjection(const NBRE_Matrix4x4f &m)
{
    mPal->GetImmediateContext()->UpdateSubresource(mProjectionBuffer.GetBuffer(), 0, NULL, m[0], 0, 0);
}

void
NBRE_D3D11FixedFuncSimulator::SetWorld(const NBRE_Matrix4x4d &m)
{
    mWorldMatrix = m;
    mPal->GetImmediateContext()->UpdateSubresource(mWorldBuffer.GetBuffer(), 0, NULL, NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix)[0], 0, 0);
}

void
NBRE_D3D11FixedFuncSimulator::SetView(const NBRE_Matrix4x4d &m)
{
    mViewMatrix = m;
    mPal->GetImmediateContext()->UpdateSubresource(mWorldBuffer.GetBuffer(), 0, NULL, NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix)[0], 0, 0);
}

void
NBRE_D3D11FixedFuncSimulator::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& m )
{
   float mat[] = { m[0][0], m[0][3],m[1][1], m[1][3]};
   if(nsl_memcmp(mState->mMatrix.texture + stage*4, mat, sizeof(mat)))
   {
       nsl_memcpy(mState->mMatrix.texture + stage*4, mat, sizeof(mat));
       mPal->GetImmediateContext()->UpdateSubresource(mTextureBuffer.GetBuffer(), 0, NULL, &(mState->mMatrix.texture), 0, 0);
   }
}

void
NBRE_D3D11FixedFuncSimulator::BindGeneralBuffer()
{
    ID3D11Buffer* buf = mWorldBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mWorldBuffer.GetSlot(), 1, &buf);
    buf = mViewBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mViewBuffer.GetSlot(), 1, &buf);
    buf = mProjectionBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mProjectionBuffer.GetSlot(), 1, &buf);
    buf = mTextureBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mTextureBuffer.GetSlot(), 1, &buf);
    buf = mLightEnableBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mLightEnableBuffer.GetSlot(), 1, &buf);
    mPal->GetImmediateContext()->PSSetConstantBuffers(mLightEnableBuffer.GetSlot(), 1, &buf);
    buf = mTextureEnableBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mTextureEnableBuffer.GetSlot(), 1, &buf);
    mPal->GetImmediateContext()->PSSetConstantBuffers(mTextureEnableBuffer.GetSlot(), 1, &buf);
    buf = mColorBuffer.GetBuffer();
    mPal->GetImmediateContext()->VSSetConstantBuffers(mColorBuffer.GetSlot(), 1, &buf);
    buf = mMaterialBuffer.GetBuffer();
    mPal->GetImmediateContext()->PSSetConstantBuffers(mMaterialBuffer.GetSlot(), 1, &buf);
    buf = mLightsBuffer.GetBuffer();
    mPal->GetImmediateContext()->PSSetConstantBuffers(mLightsBuffer.GetSlot(), 1, &buf);
    buf = mTextureTypeBuffer.GetBuffer();
    mPal->GetImmediateContext()->PSSetConstantBuffers(mTextureTypeBuffer.GetSlot(), 1, &buf);
}

void
NBRE_D3D11FixedFuncSimulator::EnableTexture(uint32 stage, nb_boolean used)
{
    if(BIT(mState->mTextureEnbale, stage) != used )
    {
        if(used)
            BIT_SET(mState->mTextureEnbale, stage);
        else
            BIT_CLEAR(mState->mTextureEnbale, stage);
        mPal->GetImmediateContext()->UpdateSubresource(mTextureEnableBuffer.GetBuffer(), 0, NULL, &(mState->mTextureEnbale), 0, 0);
    }
}

void
NBRE_D3D11FixedFuncSimulator::EnableLight(uint32 id, nb_boolean used)
{
    if(BIT(mState->mLightEnbale, id) != used)
    {
        if(used)
            BIT_SET(mState->mLightEnbale, id);
        else
            BIT_CLEAR(mState->mLightEnbale, id);

        mPal->GetImmediateContext()->UpdateSubresource(mLightEnableBuffer.GetBuffer(), 0, NULL, &(mState->mLightEnbale), 0, 0);
    }
}

void
NBRE_D3D11FixedFuncSimulator::UseLights( const NBRE_LightList& lights )
{
    if(lights.size() == mLastLights && mLastLights == 0)
    {
        return;
    }
    mLastLights = lights.size();
    NBRE_LightList::const_iterator i, iend;
    uint32 num = 0;
    iend = lights.end();
    for (i = lights.begin(); i != iend ; ++i,++num)
    {
        NBRE_Light* lt = *i;
        float w = 0.f;
        switch(lt->GetType())
        {
        case NBRE_LT_POINT:
            w = 1.f;
            break;
        case NBRE_LT_DIRECTIONAL:
            w = 0.f;
            break;
        case NBRE_LT_SPOTLIGHT:
            //TODO...
            nbre_assert(0);
            break;
        default:
            nbre_assert(0);
        }
        mState->mLights.lightPosition[num] = NBRE_Vector4f(lt->GetPosition(),w);
        mState->mLights.lightDirection[num] = NBRE_Vector4f(lt->GetDirection(),0);
        mState->mLights.lightDiffuse[num] = lt->GetDiffuse();
        mState->mLights.lightSpecular[num] = lt->GetSpecular();
        mState->mLights.lightAmbient[num] = lt->GetAmbient();
        EnableLight(num, TRUE);
    }
    mPal->GetImmediateContext()->UpdateSubresource(mLightsBuffer.GetBuffer(), 0, NULL, &(mState->mLights), 0, 0);
    for (; num < NBRE_D3D11_LIGHT_NUM; ++num)
    {
        EnableLight(num, FALSE);
    }
}


void
NBRE_D3D11FixedFuncSimulator::SelectTexture(uint32 stage, NBRE_Texture::TextureType type, ID3D11ShaderResourceView *tex)
{
    if(mState->mTexState[stage].tex != tex)
    {
        mState->mTexState[stage].tex = tex;
        nb_boolean tex2D = (type == NBRE_Texture::TT_CUBE_MAP )?FALSE:TRUE;
        if(BIT(mState->mTextureType, stage) != tex2D)
        {
            if(tex2D)
                BIT_SET(mState->mTextureType, stage);
            else
                BIT_CLEAR(mState->mTextureType, stage);
            mPal->GetImmediateContext()->UpdateSubresource(mTextureTypeBuffer.GetBuffer(), 0, NULL, &(mState->mTextureType), 0, 0);
        }
        mPal->GetImmediateContext()->PSSetShaderResources( tex2D?stage:stage + NBRE_D3D11_TEXTRUE_NUM, 1, &tex );
    }
}

void
NBRE_D3D11FixedFuncSimulator::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    uint32 key = GetTextureStateKey(state);
    if(mState->mTexState[stage].key != key)
    {
        SamplerCache::iterator it = mSamplerCache.find(key);
        if(it == mSamplerCache.end())
        {
            ID3D11SamplerState *ss = BuildSampleState(state, mPal->GetActiveD3D11Device());
            if(ss)
            {
                mSamplerCache.insert(SamplerCache::value_type(key, ss));
                mState->mTexState[stage].sampleState = ss;
            }
        }
        else
        {
            mState->mTexState[stage].sampleState = it->second;
        }

        mPal->GetImmediateContext()->PSSetSamplers(stage, 1,  &mState->mTexState[stage].sampleState);
        mState->mTexState[stage].key = key;
    }
}

void
NBRE_D3D11FixedFuncSimulator::SetColor(float r, float g, float b, float a)
{
    if(r != mState->mCurrentColor[0] || g != mState->mCurrentColor[1] ||  b != mState->mCurrentColor[2] || a !=  mState->mCurrentColor[3])
    {
        mState->mCurrentColor[0] = r;
        mState->mCurrentColor[1] = g;
        mState->mCurrentColor[2] = b;
        mState->mCurrentColor[3] = a;
        mPal->GetImmediateContext()->UpdateSubresource(mColorBuffer.GetBuffer(), 0, NULL, &(mState->mCurrentColor), 0, 0);
    }
}

void
NBRE_D3D11FixedFuncSimulator::SetMaterial(const NBRE_Material& material)
{
    mPal->GetImmediateContext()->UpdateSubresource(mMaterialBuffer.GetBuffer(), 0, NULL, &(mState->mMat), 0, 0);
}
