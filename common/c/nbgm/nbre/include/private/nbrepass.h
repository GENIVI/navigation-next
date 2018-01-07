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

    @file nbrepass.h
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
#ifndef _NBRE_PASS_H_
#define _NBRE_PASS_H_
#include "nbrematerial.h"
#include "nbretextureunit.h"
#include "nbrelight.h"
#include "nbregpuprogram.h"
#include "nbrerendertarget.h"
#include "nbrecamera.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/

struct NBRE_HBAOParams
{
    NBRE_HBAOParams():
        mInputTarget(NULL),
        mOutputTarget(NULL),
        mBlurTarget(NULL),
        mAoTarget(NULL),
        mCamera(NULL),
        mEnable(TRUE),
        mMetersToViewSpaceUnits(1.0f),
        mRadius(1.0f),
        mBias(0.0f),
        mPowerExponent(1.2f),
        mDetailAO(0.0f),
        mCoarseAO(1.0f),
        mEnableBlur(TRUE),
        mBlurRadius(8),
        mBlurSharpness(1.0f)
    {
    }

public:
    NBRE_RenderTarget* mInputTarget;
    NBRE_RenderTarget* mOutputTarget;
    NBRE_RenderTarget* mBlurTarget;
    NBRE_RenderTarget* mAoTarget;
    NBRE_Camera* mCamera;
    nb_boolean mEnable;
    float mMetersToViewSpaceUnits;
    float mRadius;
    float mBias;
    float mPowerExponent;
    float mDetailAO;
    float mCoarseAO;
    nb_boolean mEnableBlur;
    int mBlurRadius;
    float mBlurSharpness;
};

typedef shared_ptr<NBRE_HBAOParams> NBRE_HBAOParamsPtr;

class NBRE_Pass
{
public:
    NBRE_Pass():
      mEnableDepthTest(FALSE)
      ,mEnableDepthWrite(FALSE)
      ,mEnableBlend(FALSE)
      ,mEnableLighting(FALSE)
      ,mEnableAlphaTest(FALSE)
      ,mEnableColorRedWrite(TRUE)
      ,mEnableColorGreenWrite(TRUE)
      ,mEnableColorBlueWrite(TRUE)
      ,mEnableColorAlphaWrite(TRUE)
      ,mEnableFog(FALSE)
      ,mEnableCullFace(TRUE)
      ,mColor(1, 1, 1, 1)
      ,mSrcBlend(NBRE_BF_SRC_ALPHA)
      ,mDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA)
      ,mLineWidth(1.0f)
      ,mGpuProgramName("")
    {
    }

    ~NBRE_Pass(){}

public:
    nb_boolean GetEnableDepthTest() const { return mEnableDepthTest; }
    void SetEnableDepthTest(nb_boolean val) { mEnableDepthTest = val; }

    nb_boolean GetEnableDepthWrite() const { return mEnableDepthWrite; }
    void SetEnableDepthWrite(nb_boolean val) { mEnableDepthWrite = val; }

    nb_boolean GetEnableBlend() const { return mEnableBlend; }
    void SetEnableBlend(nb_boolean val) { mEnableBlend = val; }

    NBRE_BlendFunction GetSrcBlend() const { return mSrcBlend; }
    void SetSrcBlend(NBRE_BlendFunction val) { mSrcBlend = val; }

    NBRE_BlendFunction GetDstBlend() const { return mDstBlend; }
    void SetDstBlend(NBRE_BlendFunction val) { mDstBlend = val; }

    nb_boolean GetEnableLighting() const { return mEnableLighting; }
    void SetEnableLighting(nb_boolean val) { mEnableLighting = val; }

    const NBRE_Material& GetMaterial() const { return mMaterial; }
    NBRE_Material& GetMaterial() { return mMaterial; }
    void SetMaterial(const NBRE_Material& val) { mMaterial = val; }

    const NBRE_TextureUnitList& GetTextureUnits() const { return mTextureUnits; }
    NBRE_TextureUnitList& GetTextureUnits() { return mTextureUnits; }
    void SetTextureUnits(const NBRE_TextureUnitList& val) { mTextureUnits = val; }

    nb_boolean GetEnableAlphaTest() const { return mEnableAlphaTest; }
    void SetEnableAlphaTest(nb_boolean val) { mEnableAlphaTest = val; }

    nb_boolean GetEnableColorRedWrite() const { return mEnableColorRedWrite; }
    void SetEnableColorRedWrite(nb_boolean val) { mEnableColorRedWrite = val; }

    nb_boolean GetEnableColorGreenWrite() const { return mEnableColorGreenWrite; }
    void SetEnableColorGreenWrite(nb_boolean val) { mEnableColorGreenWrite = val; }

    nb_boolean GetEnableColorBlueWrite() const { return mEnableColorBlueWrite; }
    void SetEnableColorBlueWrite(nb_boolean val) { mEnableColorBlueWrite = val; }

    nb_boolean GetEnableColorAlphaWrite() const { return mEnableColorAlphaWrite; }
    void SetEnableColorAlphaWrite(nb_boolean val) { mEnableColorAlphaWrite = val; }

    nb_boolean GetEnableFog() const { return mEnableFog; }
    void SetEnableFog(nb_boolean val) { mEnableFog = val; }

    const NBRE_Color& GetColor() const { return mColor; }
    void SetColor(NBRE_Color val) { mColor = val; }

    nb_boolean GetEnableCullFace() const { return mEnableCullFace; }
    void SetEnableCullFace(nb_boolean val) { mEnableCullFace = val; }

    float GetLineWidth() const { return mLineWidth; }
    void SetLineWidth(float lineWidth) { mLineWidth = lineWidth; }

    const NBRE_String& GetGpuProgramName() const { return mGpuProgramName; }
    void SetGpuProgram(const NBRE_String& name) { mGpuProgramName = name; }
    
    void SetGpuProgramParam(const NBRE_String& name, NBRE_GpuProgramParam param) { mGpuParams[name] = param; }
    const NBRE_Map<NBRE_String, NBRE_GpuProgramParam>& GetGpuProgramParams() const { return mGpuParams; }
    NBRE_Map<NBRE_String, NBRE_GpuProgramParam>& GetGpuProgramParams() { return mGpuParams; }

    void SetHBAOParams(NBRE_HBAOParamsPtr params) { mHBAOParams = params; }
    NBRE_HBAOParamsPtr GetHBAOParams() const { return mHBAOParams; }

private:
    nb_boolean mEnableDepthTest;
    nb_boolean mEnableDepthWrite;
    nb_boolean mEnableBlend;
    nb_boolean mEnableLighting;
    nb_boolean mEnableAlphaTest;
    nb_boolean mEnableColorRedWrite;
    nb_boolean mEnableColorGreenWrite;
    nb_boolean mEnableColorBlueWrite;
    nb_boolean mEnableColorAlphaWrite;
    nb_boolean mEnableFog;
    nb_boolean mEnableCullFace;
    
    NBRE_Color mColor;
    NBRE_BlendFunction mSrcBlend;
    NBRE_BlendFunction mDstBlend;
    NBRE_TextureUnitList mTextureUnits;
    NBRE_Material mMaterial;

    float mLineWidth;

    NBRE_String mGpuProgramName;
    NBRE_Map<NBRE_String, NBRE_GpuProgramParam> mGpuParams;

    NBRE_HBAOParamsPtr mHBAOParams;
};

typedef shared_ptr<NBRE_Pass> NBRE_PassPtr;

/** @} */
/** @} */
#endif
