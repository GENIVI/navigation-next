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

@file nbred3d11fixedfunctionSimulator.h
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
#ifndef _NBRE_D3D11_FIXED_FUNCTION_Simulator_H_
#define _NBRE_D3D11_FIXED_FUNCTION_Simulator_H_
#include "nbred3d11gpuprogram.h"
#include "nbred3d11fixedfuncstate.h"
#include "nbred3d11cbuffer.h"

class NBRE_D3D11RenderPal;

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D11RenderPal
*  @{
*/

class NBRE_D3D11FixedFuncSimulator
{
public:
    NBRE_D3D11FixedFuncSimulator(NBRE_D3D11RenderPal *pal);
    ~NBRE_D3D11FixedFuncSimulator();

public:
    PAL_Error LoadShader(NBRE_D3D11GpuProgramPtr shader);
    void BindGeneralBuffer();
    void UpdataGeneralBuffer();

    void SetProjection(const NBRE_Matrix4x4f &m);
    void SetWorld(const NBRE_Matrix4x4d &m);
    void SetView(const NBRE_Matrix4x4d &m);
    void SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx );
    void EnableTexture(uint32 stage, nb_boolean used);
    void EnableLight(uint32 id, nb_boolean used);
    void SelectTexture(uint32 stage, NBRE_Texture::TextureType type, ID3D11ShaderResourceView *tex);
    void SetTextureState(uint32 stage, const NBRE_TextureState& state);
    void SetColor(float r, float g, float b, float a);
    void SetMaterial(const NBRE_Material& material);
    void UseLights( const NBRE_LightList& lights );
    NBRE_D3D11GpuProgramPtr GetVS() { return mVertexShader;  }
    NBRE_D3D11GpuProgramPtr GetPS() { return mPixelShader;   }
    NBRE_D3D11GpuProgramPtr GetGS() { return mGeometryShader;}

private:
    void InitState();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_D3D11FixedFuncSimulator);

private:
    NBRE_D3D11RenderPal     *mPal;
    NBRE_D3D11GpuProgramPtr mVertexShader;
    NBRE_D3D11GpuProgramPtr mPixelShader;
    NBRE_D3D11GpuProgramPtr mGeometryShader;
    NBRE_D3D11RenderState   *mState;
    NBRE_D3D11CBuffer        mWorldBuffer;
    NBRE_D3D11CBuffer        mViewBuffer;
    NBRE_D3D11CBuffer        mProjectionBuffer;
    NBRE_D3D11CBuffer        mTextureBuffer;
    NBRE_D3D11CBuffer        mTextureEnableBuffer;
    NBRE_D3D11CBuffer        mLightEnableBuffer;
    NBRE_D3D11CBuffer        mColorBuffer;
    NBRE_D3D11CBuffer        mMaterialBuffer;
    NBRE_D3D11CBuffer        mLightsBuffer;
    NBRE_D3D11CBuffer        mTextureTypeBuffer;
    NBRE_Matrix4x4d          mWorldMatrix;
    NBRE_Matrix4x4d          mViewMatrix;
    NBRE_Matrix4x4f          mProjectionMatrix;
    uint32                   mLastLights;
    typedef NBRE_Map<uint32, ID3D11SamplerState*> SamplerCache ;
    SamplerCache mSamplerCache;
};

/*! @} */
/*! @} */
#endif