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
#include "nbred3d11vertexdeclaration.h"
#include "nbred3d11renderpal.h"
#include "nbred3d11gpuprogram.h"

static LPCSTR
GetSemantic(NBRE_VertexElement::VertexElementSemantic sem)
{
    switch (sem)
    {
    case NBRE_VertexElement::VES_BLEND_INDICES:
        return ("BLENDINDICES");
    case NBRE_VertexElement::VES_BLEND_WEIGHTS:
        return ("BLENDWEIGHT");
    case NBRE_VertexElement::VES_DIFFUSE:
        return ("COLOR");
    case NBRE_VertexElement::VES_SPECULAR:
        return ("COLOR");
    case NBRE_VertexElement::VES_NORMAL:
        return ("NORMAL");
    case NBRE_VertexElement::VES_POSITION:
        return ("POSITION");
    case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
        return ("TEXCOORD");
    case NBRE_VertexElement::VES_BINORMAL:
        return ("BINORMAL");
    case NBRE_VertexElement::VES_TANGENT:
        return ("TANGENT");
    case NBRE_VertexElement::VES_USER_DEFINE:
        return ("USERDEFINE");
    default:
        nbre_assert(FALSE);
        return ("POSITION");
    }
}

static NBRE_VertexInputFormat
ToVertexInputFormat(NBRE_VertexElement::VertexElementSemantic sem, uint32 index)
{
    switch (sem)
    {
    case NBRE_VertexElement::VES_POSITION:
        return NBRE_VSIF_POSITION;
    case NBRE_VertexElement::VES_DIFFUSE:
        return NBRE_VSIF_COLOR;
    case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
        {
            switch (index)
            {
            case 0:
                return NBRE_VSIF_TEXCORD0;
            case 1:
                return NBRE_VSIF_TEXCORD1;
            default:
                nbre_assert(0);
                return NBRE_VSIF_TEXCORD0;
            }
        }
    case NBRE_VertexElement::VES_NORMAL:
        //TODO...return NBRE_VSIF_NORMAL;
    case NBRE_VertexElement::VES_SPECULAR:
    case NBRE_VertexElement::VES_BLEND_INDICES:
    case NBRE_VertexElement::VES_BLEND_WEIGHTS:
    case NBRE_VertexElement::VES_BINORMAL:
    case NBRE_VertexElement::VES_TANGENT:
    default:
        return NBRE_VSIF_POSITION;
    }
}


static DXGI_FORMAT
GetType(NBRE_VertexElement::VertexElementType vType)
{
    switch (vType)
    {
    case NBRE_VertexElement::VET_COLOUR:
    case NBRE_VertexElement::VET_COLOUR_ABGR:
    case NBRE_VertexElement::VET_COLOUR_ARGB:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case NBRE_VertexElement::VET_FLOAT1:
        return DXGI_FORMAT_R32_FLOAT;
    case NBRE_VertexElement::VET_FLOAT2:
        return DXGI_FORMAT_R32G32_FLOAT;
    case NBRE_VertexElement::VET_FLOAT3:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case NBRE_VertexElement::VET_FLOAT4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case NBRE_VertexElement::VET_SHORT2:
        return DXGI_FORMAT_R16G16_SINT;
    case NBRE_VertexElement::VET_SHORT4:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case NBRE_VertexElement::VET_UBYTE4:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    default:
        nbre_assert(0);
        return DXGI_FORMAT_R32G32B32_FLOAT;
    }

}

NBRE_D3D11VertexDeclaration::NBRE_D3D11VertexDeclaration(NBRE_D3D11RenderPal *pal):mPal(pal),
    mD3delems(NULL),
    mVertexInputFormat(0)
{
}

NBRE_D3D11VertexDeclaration::~NBRE_D3D11VertexDeclaration()
{
    ShaderToILayoutMapIterator iter = mLayoutMap.begin();
    ShaderToILayoutMapIterator iterE = mLayoutMap.end();
    for ( ; iter != iterE ; iter++)
    {
        iter->second->Release();
    }

    if(mD3delems)
    {
        NBRE_DELETE_ARRAY mD3delems;
    }
}

D3D11_INPUT_ELEMENT_DESC*
NBRE_D3D11VertexDeclaration::GetD3D11Declaration()const
{
    if (!mD3delems)
    {
        uint32 iNumElements = mElementList.size();
        D3D11_INPUT_ELEMENT_DESC* D3delems = NBRE_NEW D3D11_INPUT_ELEMENT_DESC[iNumElements];
        VertexElementList::const_iterator i, iend;
        uint32 idx;
        iend = mElementList.end();
        for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
        {
            D3delems[idx].SemanticName          = GetSemantic((*i)->Semantic());
            D3delems[idx].SemanticIndex         = (*i)->Index();
            D3delems[idx].Format                = GetType((*i)->Type());
            D3delems[idx].InputSlot             = (*i)->Source();
            D3delems[idx].AlignedByteOffset     = static_cast<WORD>((*i)->Offset());
            D3delems[idx].InputSlotClass        = D3D11_INPUT_PER_VERTEX_DATA;
            D3delems[idx].InstanceDataStepRate  = 0;
            mVertexInputFormat |= ToVertexInputFormat((*i)->Semantic(), (*i)->Index());
        }

        mD3delems = D3delems;
    }

    return mD3delems;
}

ID3D11InputLayout*
NBRE_D3D11VertexDeclaration::GetInputLayoutByShader(NBRE_D3D11GPUPorgram *program)const
{
    nbre_assert(program);
    ShaderToILayoutMapIterator foundIter = mLayoutMap.find(program);
    ID3D11InputLayout*  pVertexLayout = 0;
    if (foundIter == mLayoutMap.end())
    {
        // if not found - create
        D3D11_INPUT_ELEMENT_DESC * pVertexDecl = GetD3D11Declaration();
        HRESULT hr = mPal->GetActiveD3D11Device()->CreateInputLayout(
            pVertexDecl,
            (UINT)mElementList.size(),
            program->GetMicroCode(),
            program->GetMicroCodeSize(),
            &pVertexLayout);

        if (FAILED(hr) || pVertexLayout == NULL)
        {
            return NULL;
        }
        mLayoutMap[program] = pVertexLayout;
    }
    else
    {
        pVertexLayout = foundIter->second;
    }

    return pVertexLayout;
}

uint32
NBRE_D3D11VertexDeclaration::GetVertexInputFormat()const
{
    GetD3D11Declaration();
    return mVertexInputFormat;
}
