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
#include "nbred3d11gpuprogram.h"
#include "nbrelog.h"
#include "nbred3d11fixedfuncstate.h"
#include "nbred3d11renderpal.h"
#include "nbrefilestream.h"

NBRE_D3D11GPUPorgram::NBRE_D3D11GPUPorgram(NBRE_GpuProgramType type, NBRE_GpuProgramLoadMethod method, ID3D11Device *d3d11, PAL_Instance *pal):mType(type),
    mLoadMethod(method),
    mLoaded(FALSE),
    mSourceCode(NULL),
    mSourceCodeSize(0),
    mMicroCode(NULL),
    mMicroCodeSize(0),
    mVertexShader(NULL),
    mPixelShader(NULL),
    mGeometryShader(NULL),
    mD3D11(d3d11),
    mPal(pal)
{
    nbre_assert(mD3D11);
}

NBRE_D3D11GPUPorgram::~NBRE_D3D11GPUPorgram()
{
    Unload();
}

void
NBRE_D3D11GPUPorgram::Unload()
{
    if(mLoaded)
    {
        NBRE_SAFE_RELEASE(mVertexShader);
        NBRE_SAFE_RELEASE(mPixelShader);
        NBRE_SAFE_RELEASE(mGeometryShader);
        NBRE_SAFE_DELETE_ARRAY(mMicroCode);
        mLoaded = FALSE;
    }
}

PAL_Error
NBRE_D3D11GPUPorgram::ComplieShaderFromBinFile()
{
    if(mPal == NULL)
    {
        return PAL_Failed;
    }

    PAL_File *palFile = NULL;
    PAL_Error err = PAL_FileOpen(mPal, mFilePath.c_str(), PFM_Read, &palFile);
    if(err != PAL_Ok)
    {
        return err;
    }

    uint32 size = 0;
    err = PAL_FileGetSize(mPal, mFilePath.c_str(), &size);
    if(err != PAL_Ok)
    {
        PAL_FileClose(palFile);
        return err;
    }

    mMicroCode = NBRE_NEW byte[size];
    mMicroCodeSize = size;
    nsl_memset(mMicroCode, 0, size);

    uint32 readSize = 0;
    err = PAL_FileRead(palFile, (uint8*)mMicroCode, size, &readSize);
    if(err != PAL_Ok || size != readSize)
    {
        PAL_FileClose(palFile);
        NBRE_DELETE_ARRAY mMicroCode;
        mMicroCode = NULL;
        mMicroCodeSize = 0;
        return PAL_ErrFileFailed;
    }

    PAL_FileClose(palFile);
    return PAL_Ok;
}

PAL_Error
NBRE_D3D11GPUPorgram::CreateVertexShader()
{
    nbre_assert(mMicroCode && mType == NBRE_GPT_VERTEX_PROGRAM);
    HRESULT hr = mD3D11->CreateVertexShader(
        mMicroCode,
        mMicroCodeSize,
        NULL,
        &mVertexShader);

    if (FAILED(hr) || mVertexShader == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11GPUPorgram::CreateVertexShader failed!");
        return PAL_Failed;
    }

    return PAL_Ok;
}

PAL_Error
NBRE_D3D11GPUPorgram::CreatePixelShader()
{
    nbre_assert(mMicroCode && mType == NBRE_GPT_PIXEL_PROGRAM);
    HRESULT hr = mD3D11->CreatePixelShader(
        mMicroCode,
        mMicroCodeSize,
        NULL,
        &mPixelShader);

    if (FAILED(hr) || mPixelShader == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11GPUPorgram::CreatePixelShader failed!");
        return PAL_Failed;
    }

    return PAL_Ok;
}

PAL_Error
NBRE_D3D11GPUPorgram::CreateGeometryShader()
{
    nbre_assert(mMicroCode && mType == NBRE_GPT_GEOMETRY_PROGRAM);
    HRESULT hr = mD3D11->CreateGeometryShader(
        mMicroCode,
        mMicroCodeSize,
        NULL,
        &mGeometryShader);
    if (FAILED(hr) || mGeometryShader == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11GPUPorgram::CreateGeometryShader failed!");
        return PAL_Failed;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D11GPUPorgram::CreateShader()
{
    PAL_Error err = PAL_Ok;
    switch(mType)
    {
    case NBRE_GPT_VERTEX_PROGRAM:
        err = CreateVertexShader();
        break;
    case NBRE_GPT_PIXEL_PROGRAM:
        err = CreatePixelShader();
        break;
    case NBRE_GPT_GEOMETRY_PROGRAM:
        err = CreateGeometryShader();
        break;
    default:
        nbre_assert(0);
        err = PAL_ErrUnsupported;
    }

    return err;
}

ID3D11VertexShader*
NBRE_D3D11GPUPorgram::GetVertexShader(void)const
{
    nbre_assert(mType == NBRE_GPT_VERTEX_PROGRAM);
    nbre_assert(mVertexShader);
    return mVertexShader;
}

ID3D11PixelShader*
NBRE_D3D11GPUPorgram::GetPixelShader(void)const 
{
    nbre_assert(mType == NBRE_GPT_PIXEL_PROGRAM);
    nbre_assert(mPixelShader);
    return mPixelShader;
}

ID3D11GeometryShader*
NBRE_D3D11GPUPorgram::GetGeometryShader(void)const
{
    nbre_assert(mType == NBRE_GPT_GEOMETRY_PROGRAM);
    nbre_assert(mGeometryShader);
    return mGeometryShader;
}

PAL_Error
NBRE_D3D11GPUPorgram::Load()
{
     if(mLoaded)
     {
         return PAL_Ok;
     }
     PAL_Error err = PAL_Ok;
     switch(mLoadMethod)
     {
     case NBRE_GPLM_BIN_FILE:
         err = ComplieShaderFromBinFile();
         break;
     case NBRE_GPLM_SRC_FILE:
     case NBRE_GPLM_STRING:
     default:
         err = PAL_ErrUnsupported;
         nbre_assert(0);
     }
     err = (err != PAL_Ok)?err:CreateShader();
     if(err == PAL_Ok)
     {
         mLoaded = TRUE;
     }
     return err;
}


