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

    @file nbregles2ssaoprogram.h
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
#ifndef _NBRE_GLES2_SSAO_PROGRAM_H_
#define _NBRE_GLES2_SSAO_PROGRAM_H_

#include "nbregles2common.h"
#include "nbresharedptr.h"
#include "nbregles2renderpal.h"

class NBRE_GLES2Texture;
class NBRE_GLES2RenderTexture;
class NBRE_IOStream;
/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLES2RenderPal
*  @{
*/

class NBRE_GLES2SSAOProgram
{
public:
    NBRE_GLES2SSAOProgram(NBRE_GLES2RenderPal* renderPal, int maxSamples, int randTextureSize, float radius, float falloff, float bias);
    ~NBRE_GLES2SSAOProgram();

    void SetSamples(int maxSample);
    void Render(NBRE_GLES2RenderTexture* out, NBRE_GLES2RenderTexture* depthBuffer, NBRE_GLES2RenderTexture* blurBuffer, NBRE_GLES2RenderTexture* aoBuffer, const float* projMatrix, const float* projMatrixInv, float radius, float falloff, float bias, float density);

private:
    void Build();

private:
    NBRE_GLES2RenderPal* mRenderPal;
    int mMaxSamples;
    int mRandTextureSize;
    float mRadius;
    float mFalloff;
    float mBias;
    NBRE_GLES2GpuProgramPtr mAOProgram;
    shared_ptr<NBRE_GLES2Texture> mRandTexture;
    std::vector<float> mRandDirs;
    NBRE_GLES2GpuProgramPtr mBlurHProgram;
    NBRE_GLES2GpuProgramPtr mBlurVProgram;
};

/*! @} */
/*! @} */
#endif