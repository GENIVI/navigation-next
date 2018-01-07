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

@file nbred3d11fixedfuncstate.h
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
#ifndef _NBRE_D3D11_FIXEDFUNCSTATE_H_
#define _NBRE_D3D11_FIXEDFUNCSTATE_H_
#include "nbrerenderpal.h"
#include "nbred3d11common.h"
#include "nbrematerial.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D11RenderPal
*  @{
*/

/** This struct holds the state of fixed pipeline */
struct NBRE_D3D11RenderState
{
    /// structure holding transform matrix
    struct TransformMatrix
    {
        NBRE_Matrix4x4f  worldView;
        NBRE_Matrix4x4f  view;
        NBRE_Matrix4x4f  projection;
        float texture[4 * NBRE_D3D11_TEXTRUE_NUM];
    }mMatrix;

    /// structure holding texture unit settings for every stage
    struct TextureStages
    {
        uint32 key;
        ID3D11SamplerState *sampleState;
        ID3D11ShaderResourceView *tex;
    } mTexState[NBRE_D3D11_TEXTRUE_NUM];

    int32 mTextureEnbale;
    int32 mTextureType;
    int32 mLightEnbale;

    /// current color
    float mCurrentColor[4];

    struct MaterialStatus
    {
         NBRE_Material mat;
         float unused[3];
    }mMat;

    struct LightsStatus
    {
         NBRE_Vector4f lightPosition[NBRE_D3D11_LIGHT_NUM];
         NBRE_Vector4f lightDirection[NBRE_D3D11_LIGHT_NUM];
         NBRE_Color lightDiffuse[NBRE_D3D11_LIGHT_NUM];
         NBRE_Color lightAmbient[NBRE_D3D11_LIGHT_NUM];
         NBRE_Color lightSpecular[NBRE_D3D11_LIGHT_NUM];
    }mLights;
};

/*! @} */
/*! @} */
#endif
