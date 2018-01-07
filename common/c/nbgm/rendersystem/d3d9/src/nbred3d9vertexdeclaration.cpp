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
#include "nbred3d9vertexdeclaration.h"
#include "nbrecommon.h"
#include "nbred3d9renderpal.h"


static D3DDECLTYPE
GetType(NBRE_VertexElement::VertexElementType vType)
{
    switch (vType)
    {
    case NBRE_VertexElement::VET_COLOUR:
    case NBRE_VertexElement::VET_COLOUR_ABGR:
    case NBRE_VertexElement::VET_COLOUR_ARGB:
        return D3DDECLTYPE_D3DCOLOR;
    case NBRE_VertexElement::VET_FLOAT1:
        return D3DDECLTYPE_FLOAT1;
    case NBRE_VertexElement::VET_FLOAT2:
        return D3DDECLTYPE_FLOAT2;
    case NBRE_VertexElement::VET_FLOAT3:
        return D3DDECLTYPE_FLOAT3;
    case NBRE_VertexElement::VET_FLOAT4:
        return D3DDECLTYPE_FLOAT4;
    case NBRE_VertexElement::VET_SHORT2:
        return D3DDECLTYPE_SHORT2;
    case NBRE_VertexElement::VET_SHORT4:
        return D3DDECLTYPE_SHORT4;
    case NBRE_VertexElement::VET_UBYTE4:
        return D3DDECLTYPE_UBYTE4;
    default:
        nbre_assert(0);
        return D3DDECLTYPE_FLOAT3;
    }
}

static D3DDECLUSAGE
GetSemantic(NBRE_VertexElement::VertexElementSemantic sem)
{
    switch (sem)
    {
    case NBRE_VertexElement::VES_BLEND_INDICES:
        return D3DDECLUSAGE_BLENDINDICES;
    case NBRE_VertexElement::VES_BLEND_WEIGHTS:
        return D3DDECLUSAGE_BLENDWEIGHT;
    case NBRE_VertexElement::VES_DIFFUSE:
        return D3DDECLUSAGE_COLOR;
    case NBRE_VertexElement::VES_SPECULAR:
        return D3DDECLUSAGE_COLOR;
    case NBRE_VertexElement::VES_NORMAL:
        return D3DDECLUSAGE_NORMAL;
    case NBRE_VertexElement::VES_POSITION:
        return D3DDECLUSAGE_POSITION;
    case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
        return D3DDECLUSAGE_TEXCOORD;
    case NBRE_VertexElement::VES_BINORMAL:
        return D3DDECLUSAGE_BINORMAL;
    case NBRE_VertexElement::VES_TANGENT:
        return D3DDECLUSAGE_TANGENT;
    case NBRE_VertexElement::VES_USER_DEFINE:
        return D3DDECLUSAGE_BINORMAL;
    default:
        nbre_assert(0);
        return D3DDECLUSAGE_POSITION;
    }
}

NBRE_D3D9VertexDeclaration::NBRE_D3D9VertexDeclaration()
    :mD3D9Declaration(NULL)
{
}

NBRE_D3D9VertexDeclaration::~NBRE_D3D9VertexDeclaration()
{
    if (mD3D9Declaration)
    {
        mD3D9Declaration->Release();
    }
}

IDirect3DVertexDeclaration9*
NBRE_D3D9VertexDeclaration::GetD3D9Declaration(IDirect3DDevice9 *device)const
{
    nbre_assert(device);
    if (mD3D9Declaration == NULL)
    {
        const NBRE_VertexDeclaration::VertexElementList& decl = mElementList;
        NBRE_VertexDeclaration::VertexElementList::const_iterator elem, elemEnd;
        elemEnd = decl.end();

        D3DVERTEXELEMENT9* d3delems = NBRE_NEW D3DVERTEXELEMENT9[decl.size() + 1];
        unsigned int idx;
        for (idx = 0, elem = decl.begin(); elem != elemEnd; ++elem, ++idx)
        {
            const NBRE_VertexElement* element = *elem;
            d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
            d3delems[idx].Offset = static_cast<WORD>(element->Offset());
            d3delems[idx].Stream = static_cast<WORD>(element->Source());
            d3delems[idx].Type   = static_cast<BYTE>(GetType(element->Type()));
            d3delems[idx].Usage  = static_cast<BYTE>(GetSemantic(element->Semantic()));
            if (element->Semantic() == NBRE_VertexElement::VES_SPECULAR)
            {
                d3delems[idx].UsageIndex = 1;
            }
            else if (element->Semantic() == NBRE_VertexElement::VES_DIFFUSE)
            {
                d3delems[idx].UsageIndex = 0;
            }
            else
            {
                d3delems[idx].UsageIndex = static_cast<BYTE>(element->Index());
            }
        }
        // Add terminator
        d3delems[idx].Stream     = 0xff;
        d3delems[idx].Offset     = 0;
        d3delems[idx].Type       = D3DDECLTYPE_UNUSED;
        d3delems[idx].Method     = 0;
        d3delems[idx].Usage      = 0;
        d3delems[idx].UsageIndex = 0;

        device->CreateVertexDeclaration(d3delems, &mD3D9Declaration);
        NBRE_DELETE_ARRAY d3delems;
    }
    return mD3D9Declaration;
}