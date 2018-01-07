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

@file nbred3d11gpuprogram.h
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
#ifndef _NBRE_D3D11_GPU_PROGRAM_H_
#define _NBRE_D3D11_GPU_PROGRAM_H_
#include "nbred3d11common.h"
#include "nbresharedptr.h"

struct NBRE_D3D11RenderState;
class NBRE_D3D11CBuffer;
class NBRE_D3D11RenderPal;
struct PAL_Instance;

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D11RenderPal
*  @{
*/

enum NBRE_GpuProgramType
{
    NBRE_GPT_VERTEX_PROGRAM,
    NBRE_GPT_PIXEL_PROGRAM,
    NBRE_GPT_GEOMETRY_PROGRAM
};

enum NBRE_GpuProgramLoadMethod
{
    NBRE_GPLM_STRING,   // Compile from string
    NBRE_GPLM_SRC_FILE, // Compile from srouce file
    NBRE_GPLM_BIN_FILE, // Load from binary file
};

class NBRE_D3D11GPUPorgram
{
public:
    NBRE_D3D11GPUPorgram(NBRE_GpuProgramType type, NBRE_GpuProgramLoadMethod method, ID3D11Device *d3d11, PAL_Instance * pal);
    virtual ~NBRE_D3D11GPUPorgram();

public:
    NBRE_GpuProgramType GetType(void) const { return mType; }

    PAL_Error Load();
    void Unload();
    nb_boolean IsLoaded()const { return mLoaded; }

    ID3D11VertexShader* GetVertexShader(void) const;
    ID3D11PixelShader* GetPixelShader(void) const;
    ID3D11GeometryShader* GetGeometryShader(void) const;

    void* GetMicroCode(void) const      { nbre_assert(mMicroCode); return mMicroCode; }
    uint32 GetMicroCodeSize(void) const { return mMicroCodeSize; }

    void SetEntryPoint(const NBRE_String& entryPoint) { mEntryPoint = entryPoint; }
    const NBRE_String& GetEntryPoint(void) const      { return mEntryPoint; }

    void SetTarget(const NBRE_String& target) { mTarget = target; }
    const NBRE_String& GetTarget(void) const  { return mTarget; }

    void SetFilePath(const NBRE_String& file)  { mFilePath = file; }
    const NBRE_String& GetFilePath(void) const { return mFilePath; }

    void SetSource(byte* sourceCode, uint32 size) { nbre_assert(sourceCode); mSourceCode = sourceCode; mSourceCodeSize = size; }

private:
    PAL_Error ComplieShaderFromBinFile();
    PAL_Error CreateShader();
    PAL_Error CreateVertexShader();
    PAL_Error CreatePixelShader();
    PAL_Error CreateGeometryShader();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_D3D11GPUPorgram);

private:
    NBRE_GpuProgramType mType;
    NBRE_GpuProgramLoadMethod mLoadMethod;
    nb_boolean mLoaded;

    NBRE_String mFilePath;
    NBRE_String mEntryPoint;
    NBRE_String mTarget;
    byte*  mSourceCode;
    uint32 mSourceCodeSize;
    void*  mMicroCode;
    uint32 mMicroCodeSize;

    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader*  mPixelShader;
    ID3D11GeometryShader* mGeometryShader;
    ID3D11Device *mD3D11;
    PAL_Instance *mPal;
};

typedef NBRE_SharedPtr<NBRE_D3D11GPUPorgram> NBRE_D3D11GpuProgramPtr;

/*! @} */
/*! @} */
#endif