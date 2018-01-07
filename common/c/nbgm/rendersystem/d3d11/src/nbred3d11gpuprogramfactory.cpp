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
#include "nbred3d11gpuprogramfactory.h"
#include "nbred3d11gpuprogram.h"
#include "nbred3d11fixedfuncstate.h"
#include "nbred3d11vertexdeclaration.h"
#include "palfile.h"
#include "nbred3d11renderpal.h"

#define  DEFAULT_VS_ENTRY          "vs_main.cso"
#define  DEFAULT_VS_ENTRY_ID        0

#define  DEFAULT_VS_ENTRY_C        "vs_main_c.cso"
#define  DEFAULT_VS_ENTRY_C_ID      1

#define  DEFAULT_VS_ENTRY_CT       "vs_main_ct.cso"
#define  DEFAULT_VS_ENTRY_CT_ID     2

#define  DEFAULT_VS_ENTRY_T1       "vs_main_t.cso"
#define  DEFAULT_VS_ENTRY_T1_ID     3

#define  DEFAULT_VS_ENTRY_T2        "vs_main_t2.cso"
#define  DEFAULT_VS_ENTRY_T2_ID     4

#define  DEFAULT_PS_ENTRY          "ps_main.cso"
#define  DEFAULT_PS_ENTRY_ID        0

#define  DEFAULT_PS_ENTRY_T1       "ps_main_t.cso"
#define  DEFAULT_PS_ENTRY_T1_ID     1

#define  DEFAULT_PS_ENTRY_T2       "ps_main_t2.cso"
#define  DEFAULT_PS_ENTRY_T2_ID     2

static uint32
GetVSKeyByInputFormat(uint32 format)
{
    switch (format)
    {
    case NBRE_VSIF_COLOR|NBRE_VSIF_TEXCORD0:
        return DEFAULT_VS_ENTRY_CT_ID;
    case NBRE_VSIF_TEXCORD0:
    case NBRE_VSIF_TEXCORD1:
        return DEFAULT_VS_ENTRY_T1_ID;
    case NBRE_VSIF_TEXCORD0|NBRE_VSIF_TEXCORD1:
        return DEFAULT_VS_ENTRY_T2_ID;
    case NBRE_VSIF_COLOR:
        return DEFAULT_VS_ENTRY_C_ID;
    case NBRE_VSIF_POSITION:
        return DEFAULT_VS_ENTRY_ID;
    default:
        return DEFAULT_VS_ENTRY_ID;
    }
}

static uint32
GetPSKeyByInputFormat(uint32 format)
{
    switch (format)
    {
    case NBRE_VSIF_TEXCORD0:
    case NBRE_VSIF_TEXCORD1:
    case NBRE_VSIF_COLOR|NBRE_VSIF_TEXCORD0:
        return DEFAULT_PS_ENTRY_T1_ID;
    case NBRE_VSIF_TEXCORD0|NBRE_VSIF_TEXCORD1:
        return DEFAULT_PS_ENTRY_T2_ID;
    case NBRE_VSIF_COLOR:
    case NBRE_VSIF_POSITION:
        return DEFAULT_PS_ENTRY_ID;
    default:
        return DEFAULT_PS_ENTRY_ID;
    }
}


NBRE_D3D11GPUPorgramFactory::NBRE_D3D11GPUPorgramFactory(NBRE_D3D11RenderPal *pal):mPal(pal)
{
    nbre_assert(mPal);
}

NBRE_D3D11GPUPorgramFactory::~NBRE_D3D11GPUPorgramFactory()
{
    mVertexShaders.clear();
    mPixelShaders.clear();
}

void
NBRE_D3D11GPUPorgramFactory::Initialize()
{
     CreateAllDefaultShader();
}

void
NBRE_D3D11GPUPorgramFactory::CreateDefaultVSByBinFile(const char* filePath, uint32 key)
{
	NBRE_D3D11GpuProgramPtr vs(NBRE_NEW NBRE_D3D11GPUPorgram(NBRE_GPT_VERTEX_PROGRAM, NBRE_GPLM_BIN_FILE, mPal->GetActiveD3D11Device(), mPal->GetNBPal()), NBRE_SPFM_DELETE);
    vs->SetFilePath(mPal->GetWorkPath() + filePath);
    vs->SetEntryPoint(filePath);
    vs->SetTarget("vs_4_0");
    vs->Load();
    ShaderCache::iterator it = mVertexShaders.find(key);
    if(it == mVertexShaders.end())
    {
        mVertexShaders.insert(ShaderCache::value_type(key, vs));
    }
}

void
 NBRE_D3D11GPUPorgramFactory::CreateDefaultPSByBinFile(const char* filePath, uint32 key)
{
    NBRE_D3D11GpuProgramPtr ps(NBRE_NEW NBRE_D3D11GPUPorgram(NBRE_GPT_PIXEL_PROGRAM, NBRE_GPLM_BIN_FILE, mPal->GetActiveD3D11Device(), mPal->GetNBPal()), NBRE_SPFM_DELETE);
    ps->SetFilePath(mPal->GetWorkPath() + filePath);
    ps->SetEntryPoint(filePath);
    ps->SetTarget("ps_4_0");
    ps->Load();
    ShaderCache::iterator it = mPixelShaders.find(key);
    if(it == mPixelShaders.end())
    {
        mPixelShaders.insert(ShaderCache::value_type(key, ps));
    }
}

void
NBRE_D3D11GPUPorgramFactory::CreateAllDefaultShader()
{
    const char* vertexShaders[] =
    {
        DEFAULT_VS_ENTRY,
        DEFAULT_VS_ENTRY_C,
        DEFAULT_VS_ENTRY_CT,
        DEFAULT_VS_ENTRY_T1,
        DEFAULT_VS_ENTRY_T2
    };

    uint32 count = sizeof(vertexShaders) / sizeof(char*);
    for(uint32 i = 0; i < count; i++)
    {
        CreateDefaultVSByBinFile(vertexShaders[i], i);
    }

    const char* pixelShaders[] =
    {
        DEFAULT_PS_ENTRY,
        DEFAULT_PS_ENTRY_T1,
        DEFAULT_PS_ENTRY_T2,
    };

    count = sizeof(pixelShaders) / sizeof(char*);
     for(uint32 i = 0; i < count; i++)
    {
        CreateDefaultPSByBinFile(pixelShaders[i], i);
    }
}

NBRE_D3D11GpuProgramPtr
NBRE_D3D11GPUPorgramFactory::GetDefaultVS(uint32 inputFormat)
{
    ShaderCache::iterator it = mVertexShaders.find(GetVSKeyByInputFormat(inputFormat));
    if(it != mVertexShaders.end())
    {
        return it->second;
    }
    return NBRE_D3D11GpuProgramPtr();
}

NBRE_D3D11GpuProgramPtr
NBRE_D3D11GPUPorgramFactory::GetDefaultPS(uint32 inputFormat)
{
    ShaderCache::iterator it = mPixelShaders.find(GetPSKeyByInputFormat(inputFormat));
    if(it != mPixelShaders.end())
    {
        return it->second;
    }
    return NBRE_D3D11GpuProgramPtr();
}