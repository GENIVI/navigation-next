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
#include "nbred3d9renderpal.h"
#include "nbred3d9hardwareindexbuffer.h"
#include "nbred3d9hardwarevertexbuffer.h"
#include "nbreitextureimage.h"
#include "nbred3d9vertexdeclaration.h"
#include "nbrematerial.h"
#include "nbred3d9renderwindow.h"
#include "nbretransformation.h"
#include "nbred3d9texture.h"
#include "nbred3d9rendertexture.h"
#include "nbretypeconvert.h"

#define  MAX_LIGHT  8

static D3DBLEND
ToD3D9BlendFunction(NBRE_BlendFunction bf)
{
    switch (bf)
    {
    case NBRE_BF_ZERO:
        return D3DBLEND_ZERO;
    case NBRE_BF_ONE:
        return D3DBLEND_ONE;
    case NBRE_BF_SRC_COLOR:
        return D3DBLEND_SRCCOLOR;
    case NBRE_BF_ONE_MINUS_SRC_COLOR:
        return D3DBLEND_INVSRCCOLOR;
    case NBRE_BF_SRC_ALPHA:
        return D3DBLEND_SRCALPHA;
    case NBRE_BF_ONE_MINUS_SRC_ALPHA:
        return D3DBLEND_INVSRCALPHA;
    case NBRE_BF_DST_COLOR:
        return D3DBLEND_DESTCOLOR;
    case NBRE_BF_ONE_MINUS_DST_COLOR:
        return D3DBLEND_INVDESTCOLOR;
    case NBRE_BF_DST_ALPHA:
        return D3DBLEND_DESTALPHA;
    case NBRE_BF_ONE_MINUS_DST_ALPHA:
        return D3DBLEND_INVDESTALPHA;
    default:
        return D3DBLEND_ONE;
    }
}

static D3DFOGMODE
ToD3D9FogMode(NBRE_FogMode fm)
{
    switch(fm)
    {
    case NBRE_FM_LINEAR:
        return D3DFOG_LINEAR;
    case NBRE_FM_EXP:
        return D3DFOG_EXP;
    case NBRE_FM_EXP2:
        return D3DFOG_EXP2;
    default:
        return D3DFOG_LINEAR;
    }
}

static DWORD
ToD3D9CompareFunction(NBRE_CompareFunction cf)
{
    switch(cf)
    {
    case NBRE_CF_NEVER:
        return D3DCMP_NEVER;
    case NBRE_CF_LESS:
        return D3DCMP_LESS;
    case NBRE_CF_LESS_EQUAL:
        return D3DCMP_LESSEQUAL;
    case NBRE_CF_EQUAL:
        return D3DCMP_EQUAL;
    case NBRE_CF_GREATER:
        return D3DCMP_GREATER;
    case NBRE_CF_NOT_EQUAL:
        return D3DCMP_NOTEQUAL;
    case NBRE_CF_GREATER_EQUAL:
        return D3DCMP_GREATEREQUAL;
    case NBRE_CF_ALWAYS:
        return D3DCMP_ALWAYS;
    default:
        return D3DCMP_ALWAYS;
    }
}

void
NBRE_D3D9RenderPal::GetD3D9Capabilities(IDirect3D9* d3d9, IDirect3DDevice9* device, NBRE_RenderPalCapabilities& cap)
{
    nbre_assert(d3d9 && device);
    D3DCAPS9 d3dcaps;
    device->GetDeviceCaps(&d3dcaps);
    if (d3dcaps.MaxActiveLights == 0xFFFFFFFF)
    {
        d3dcaps.MaxActiveLights = MAX_LIGHT;
    }
    nb_boolean canAutoMipmaps = FALSE;
    if (d3dcaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
    {
        HRESULT hr = d3d9->CheckDeviceFormat(D3DADAPTER_DEFAULT,
                                             D3DDEVTYPE_HAL,
                                             D3DFMT_X8R8G8B8,
                                             D3DUSAGE_AUTOGENMIPMAP,
                                             D3DRTYPE_TEXTURE,
                                             D3DFMT_A8R8G8B8);
        canAutoMipmaps = SUCCEEDED(hr);
    }
    cap.SetAutoMipmaps(canAutoMipmaps);
    cap.SetMaxLights(d3dcaps.MaxActiveLights);
    cap.SetMaxTextureUnits(d3dcaps.MaxTextureBlendStages);
    cap.SetMaxStreams(d3dcaps.MaxStreams);
    if(d3dcaps.MaxAnisotropy > 0)
    {
        cap.SetSupportAnisotropy(TRUE);
    }
    mMaxAnisotropy = d3dcaps.MaxAnisotropy;
}

static D3DCOLORVALUE
ToD3D9Color(const NBRE_Color& color)
{
    D3DCOLORVALUE v = {color.r, color.g, color.b, color.a};
    return v;
}

static D3DVECTOR
ToD3D9Vector(const NBRE_Vector3f& vec)
{
    D3DVECTOR v = { vec.x, vec.y, vec.z };
    return v;
}

static D3DMATRIX
ToD3D9Matrix(const NBRE_Matrix4x4f& mat)
{
    D3DMATRIX m = {mat[0][0], mat[1][0], mat[2][0], mat[3][0],
                   mat[0][1], mat[1][1], mat[2][1], mat[3][1],
                   mat[0][2], mat[1][2], mat[2][2], mat[3][2],
                   mat[0][3], mat[1][3], mat[2][3], mat[3][3]};
    return m;
}

static D3DMATRIX
ToD3DPerspectiveMatrix(const NBRE_Matrix4x4f& mat)
{
    D3DMATRIX m = {mat[0][0],  mat[1][0],   (mat[2][0] + mat[3][0])/2.f,   mat[3][0],
                   mat[0][1],  mat[1][1],   (mat[2][1] + mat[3][1])/2.f,   mat[3][1],
                  -mat[0][2], -mat[1][2],  -(mat[2][2] + mat[3][2])/2.f,  -mat[3][2],
                   mat[0][3],  mat[1][3],   (mat[2][3] + mat[3][3])/2.f,   mat[3][3]};
    return m;
};

DWORD
ToD3DAddressMode(NBRE_TextureAddressMode mode)
{
    switch(mode)
    {
    case NBRE_TAM_REPEAT:
        return D3DTADDRESS_WRAP;
    case NBRE_TAM_CLAMP:
        return D3DTADDRESS_CLAMP;
    default:
        return D3DTADDRESS_CLAMP;
    }
};

DWORD
ToD3DFilterType(NBRE_TextureFilterType type)
{
    switch(type)
    {
    case NBRE_TFT_POINT:
        return D3DTEXF_POINT;
    case NBRE_TFT_LINEAR:
        return D3DTEXF_LINEAR;
    case NBRE_TFT_MIPMAP_POINT:
        return D3DTEXF_POINT;
    case NBRE_TFT_MIPMAP_LINEAR:
        return D3DTEXF_LINEAR;
    case NBRE_TFT_MIPMAP_TRILINEAR:
        return D3DTEXF_ANISOTROPIC;
    default:
        return D3DTEXF_POINT;
    }
}

static void
RestoreVertexClientState(IDirect3DDevice9* device, const NBRE_RenderPalCapabilities& cap)
{
    for (uint32 i = 0; i < cap.GetMaxStreams(); ++i)
    {
        device->SetStreamSource(i, NULL, 0, 0);
    }
}

void
NBRE_D3D9RenderPal::SetVertexBuffer(NBRE_VertexData* vertexData )
{
    nbre_assert(vertexData);
    mCurrentVertexData = vertexData;
    RestoreVertexClientState(mD3D9Device, mCapabilities);

    const NBRE_D3D9VertexDeclaration *del = static_cast<const NBRE_D3D9VertexDeclaration*>(vertexData->GetVertexDeclaration());
    nbre_assert(del);
    const NBRE_VertexDeclaration::VertexElementList& decl = del->GetVertexElementList();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elem, elemEnd;
    elemEnd = decl.end();
    for (elem = decl.begin(); elem != elemEnd; ++elem)
    {
        const NBRE_VertexElement* element = *elem;
        NBRE_D3D9HardwareVertexBuffer *d3d9buf = static_cast<NBRE_D3D9HardwareVertexBuffer*>(const_cast<NBRE_HardwareVertexBuffer*>(vertexData->GetBuffer(element->Source())));
        nbre_assert(d3d9buf);
        mD3D9Device->SetStreamSource(element->Source(), d3d9buf->GetD3DVertextBuffer(), 0, static_cast<UINT>(d3d9buf->GetVertexSize()));
    }
    mD3D9Device->SetVertexDeclaration(del->GetD3D9Declaration(mD3D9Device));
}

void
NBRE_D3D9RenderPal::DrawIndexedPrimitive( NBRE_PrimitiveType operationType, const NBRE_IndexData* indexData )
{
    nbre_assert(indexData && mCurrentVertexData);
    DWORD primCount = 0;
    D3DPRIMITIVETYPE primType = D3DPT_POINTLIST;
    switch (operationType)
    {
    case NBRE_PMT_POINT_LIST:
        primType  = D3DPT_POINTLIST;
        primCount = indexData->IndexCount();
        break;
    case NBRE_PMT_LINE_LIST:
        primType = D3DPT_LINELIST;
        primCount = indexData->IndexCount()/2;
        break;
    case NBRE_PMT_LINE_STRIP:
        primType = D3DPT_LINESTRIP;
        primCount = indexData->IndexCount()-1;
        break;
    case NBRE_PMT_TRIANGLE_LIST:
        primType = D3DPT_TRIANGLELIST;
        primCount = indexData->IndexCount()/3;
        break;
    case NBRE_PMT_TRIANGLE_STRIP:
        primType = D3DPT_TRIANGLESTRIP;
        primCount = indexData->IndexCount()-2;
        break;
    case NBRE_PMT_TRIANGLE_FAN:
        primCount = indexData->IndexCount()-2;
        primType = D3DPT_TRIANGLEFAN;
        break;
    default:
        nbre_assert(FALSE);
    }

    NBRE_D3D9HardwareIndexBuffer* indexBuffer = static_cast<NBRE_D3D9HardwareIndexBuffer*>(indexData->IndexBuffer());
    if(indexBuffer != NULL)
    {
        mD3D9Device->SetIndices(indexBuffer->GetD3DIndexBuffer());
        mD3D9Device->DrawIndexedPrimitive(primType, mCurrentVertexData->GetVertexStart(), 0, mCurrentVertexData->GetVertexCount(), indexData->IndexStart(), primCount);
    }
    else
    {
        mD3D9Device->DrawPrimitive(primType, mCurrentVertexData->GetVertexStart(), primCount);
    }
}

NBRE_D3D9RenderPal::NBRE_D3D9RenderPal(IDirect3D9* d3d9,IDirect3DDevice9* device): mD3D9(d3d9)
,mD3D9Device(device)
,mCurrentVertexData(NULL)
,mClearDepth(1.f)
,mClearStencil(0)
,mD3D9RenderWindow(NULL)
,mDisabledTexUnitsFrom(0)
{
    nbre_assert(d3d9 && mD3D9Device);
    GetD3D9Capabilities(mD3D9, mD3D9Device, mCapabilities);
    mClearDepth = 1.f;
    mD3D9Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    mD3D9Device->SetRenderState(D3DRS_AMBIENT, 0xFF333333);
    mCurrentRenderTarget = NULL;
    mTextureTypes = NBRE_NEW int32[mCapabilities.GetMaxTextureUnits()];

}

NBRE_D3D9RenderPal::~NBRE_D3D9RenderPal()
{
    NBRE_DELETE_ARRAY mTextureTypes;
    NBRE_DELETE mD3D9RenderWindow;
}

void
NBRE_D3D9RenderPal::SetProjectionTransform( const NBRE_Matrix4x4f& mtrx )
{
    D3DMATRIX m = ToD3DPerspectiveMatrix(mtrx*NBRE_Transformationf::BuildScaleMatrix(1.f, 1.f, -1.f));
    mD3D9Device->SetTransform(D3DTS_PROJECTION, &m);
}

void
NBRE_D3D9RenderPal::SetViewTransform( const NBRE_Matrix4x4d& mtrx )
{
    mViewMatrix = mtrx;

    D3DMATRIX m = ToD3D9Matrix(NBRE_Matrix4x4f());
    mD3D9Device->SetTransform(D3DTS_VIEW, &m);

    m = ToD3D9Matrix(NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix));
    mD3D9Device->SetTransform(D3DTS_WORLD, &m);
}

void
NBRE_D3D9RenderPal::SetWorldTransform( const NBRE_Matrix4x4d& mtrx )
{
    mWorldMatrix = mtrx;

    D3DMATRIX m = ToD3D9Matrix(NBRE_Matrix4x4f());
    mD3D9Device->SetTransform(D3DTS_VIEW, &m);

    m = ToD3D9Matrix(NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix));
    mD3D9Device->SetTransform(D3DTS_WORLD, &m);
}

void
NBRE_D3D9RenderPal::SetClearColor(float r, float g, float b, float a)
{
    mClearColor.r = r;
    mClearColor.g = g;
    mClearColor.b = b;
    mClearColor.a = a;
}

void
NBRE_D3D9RenderPal::SetClearDepth(float val)
{
    mClearDepth = val;
}

void
NBRE_D3D9RenderPal::SetClearStencil(int32 val)
{
    mClearStencil = val;
}

void
NBRE_D3D9RenderPal::Clear(uint32 flags)
{
    DWORD mask = 0;
    if (flags & NBRE_CLF_CLEAR_COLOR)
    {
        mask |= D3DCLEAR_TARGET;
    }
    if (flags & NBRE_CLF_CLEAR_DEPTH)
    {
        mD3D9Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        mask |= D3DCLEAR_ZBUFFER;
    }
    if (flags & NBRE_CLF_CLEAR_STENCIL)
    {
        mask |= D3DCLEAR_STENCIL;
    }
    mD3D9Device->Clear(0, NULL, mask, mClearColor.GetAsARGB(), mClearDepth, mClearStencil);
}

void
NBRE_D3D9RenderPal::EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha )
{
    DWORD val = 0;
    if (enableRed)
        val |= D3DCOLORWRITEENABLE_RED;
    if (enableGreen)
        val |= D3DCOLORWRITEENABLE_GREEN;
    if (enableBlue)
        val |= D3DCOLORWRITEENABLE_BLUE;
    if (enableAlpha)
        val |= D3DCOLORWRITEENABLE_ALPHA;
    mD3D9Device->SetRenderState(D3DRS_COLORWRITEENABLE, val);
}

void
NBRE_D3D9RenderPal::EnableDepthWrite( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_ZWRITEENABLE, bEnable);
}

void
NBRE_D3D9RenderPal::EnableStencilWrite( uint32 maskBits )
{
    mD3D9Device->SetRenderState(D3DRS_STENCILWRITEMASK, maskBits);
}

void
NBRE_D3D9RenderPal::EnableFog( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_FOGENABLE, bEnable);
}

void
NBRE_D3D9RenderPal::EnableBlend( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, bEnable);
}

void
NBRE_D3D9RenderPal::EnableLighting( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_LIGHTING, bEnable);
}

void
NBRE_D3D9RenderPal::EnableCullFace( nb_boolean bEnable )
{
    if (bEnable)
    {
        mD3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    }
    else
    {
        mD3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    }
}

void
NBRE_D3D9RenderPal::EnableDepthTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        mD3D9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    }
    else
    {
        mD3D9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    }
}

void
NBRE_D3D9RenderPal::EnableAlphaTest( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_ALPHATESTENABLE, bEnable);
}

void
NBRE_D3D9RenderPal::EnableStencilTest( nb_boolean bEnable )
{
    mD3D9Device->SetRenderState(D3DRS_STENCILENABLE, bEnable);
}

void
NBRE_D3D9RenderPal::SetDepthFunc(NBRE_CompareFunction cf)
{
    mD3D9Device->SetRenderState(D3DRS_ZFUNC, ToD3D9CompareFunction(cf));
}

void
NBRE_D3D9RenderPal::SetAlphaFunc(NBRE_CompareFunction cf, float refValue)
{
    mD3D9Device->SetRenderState(D3DRS_ALPHAFUNC, ToD3D9CompareFunction(cf));
    mD3D9Device->SetRenderState(D3DRS_ALPHAREF,  *((LPDWORD)(&refValue)));
}

void
NBRE_D3D9RenderPal::SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha)
{
    NBRE_Color clr(red, green, blue, alpha);
    mD3D9Device->SetRenderState(D3DRS_FOGTABLEMODE,   ToD3D9FogMode(mode));
    mD3D9Device->SetRenderState(D3DRS_FOGVERTEXMODE,  D3DFOG_NONE);
    mD3D9Device->SetRenderState(D3DRS_FOGCOLOR,       clr.GetAsARGB());
    mD3D9Device->SetRenderState(D3DRS_FOGSTART,   *((LPDWORD)(&fogStart)));
    mD3D9Device->SetRenderState(D3DRS_FOGEND,     *((LPDWORD)(&fogEnd)));
    mD3D9Device->SetRenderState(D3DRS_FOGDENSITY, *((LPDWORD)(&density)));
}

void
NBRE_D3D9RenderPal::SelectRenderTarget( NBRE_RenderTarget* rt )
{
    mCurrentRenderTarget = rt;
}

void
NBRE_D3D9RenderPal::SetMaterial(const NBRE_Material& material)
{
    D3DMATERIAL9 mat = { ToD3D9Color(material.Diffuse()),
                         ToD3D9Color(material.Ambient()),
                         ToD3D9Color(material.Specular()),
                         ToD3D9Color(material.Emissive()),
                         material.Shiness() };
    mD3D9Device->SetMaterial(&mat);
}

void
NBRE_D3D9RenderPal::SetViewPort( int32 left, int32 top, uint32 width , uint32 height )
{
    D3DVIEWPORT9 viewPort;
    viewPort.X      = left;
    viewPort.Y      = top;
    viewPort.Width  = width;
    viewPort.Height = height;
    viewPort.MinZ   = 0.f;
    viewPort.MaxZ   = 1.f;
    mD3D9Device->SetViewport(&viewPort);
}
void
NBRE_D3D9RenderPal::SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor)
{
    mD3D9Device->SetRenderState(D3DRS_SRCBLEND,  ToD3D9BlendFunction(srcFactor));
    mD3D9Device->SetRenderState(D3DRS_DESTBLEND, ToD3D9BlendFunction(dstFactor));
}

nb_boolean
NBRE_D3D9RenderPal::BeginScene()
{
    mD3D9Device->BeginScene();
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->BeginRender();
    }
    return TRUE;
}

void
NBRE_D3D9RenderPal::EndScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->EndRender();
    }
    mD3D9Device->EndScene();
}

NBRE_RenderTarget*
NBRE_D3D9RenderPal::GetDefaultRenderTarget()
{
    if(mD3D9RenderWindow == NULL)
    {
        mD3D9RenderWindow = NBRE_NEW NBRE_D3D9RenderWindow();
    }
    return mD3D9RenderWindow;
}


NBRE_HardwareVertexBuffer*
NBRE_D3D9RenderPal::CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_D3D9HardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, this);
}

NBRE_HardwareIndexBuffer*
NBRE_D3D9RenderPal::CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_D3D9HardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, this);
}

NBRE_VertexDeclaration*
NBRE_D3D9RenderPal::CreateVertexDeclaration(void)
{
    return NBRE_NEW NBRE_D3D9VertexDeclaration();
}

void
NBRE_D3D9RenderPal::UseLights( const NBRE_LightList& lights )
{
    nbre_assert(lights.size() <= mCapabilities.GetMaxLights());
    D3DLIGHT9 d3dLight;
    NBRE_LightList::const_iterator i, iend;
    uint32 num = 0;
    iend = lights.end();
    for (i = lights.begin(); i != iend ; ++i,++num)
    {
        ZeroMemory(&d3dLight, sizeof(d3dLight));
        NBRE_Light* lt = *i;
        switch(lt->GetType())
        {
        case NBRE_LT_POINT:
            d3dLight.Type = D3DLIGHT_POINT;
            break;

        case NBRE_LT_DIRECTIONAL:
            d3dLight.Type = D3DLIGHT_DIRECTIONAL;
            break;

        case NBRE_LT_SPOTLIGHT:
            d3dLight.Type    = D3DLIGHT_SPOT;
            d3dLight.Falloff = lt->GetExponent();
            d3dLight.Theta   = NBRE_Math::DegToRad(lt->GetCutoff())*0.5f;
            d3dLight.Phi     = NBRE_Math::DegToRad(lt->GetCutoff());
            break;
        default:
            nbre_assert(0);
        }

        d3dLight.Diffuse = ToD3D9Color(lt->GetDiffuse());
        d3dLight.Specular = ToD3D9Color(lt->GetSpecular());

        if( lt->GetType() != NBRE_LT_DIRECTIONAL )
        {
            d3dLight.Position = ToD3D9Vector(lt->GetPosition());
        }
        if( lt->GetType() != NBRE_LT_POINT )
        {
            d3dLight.Direction = ToD3D9Vector(lt->GetDirection());
        }

        d3dLight.Range = lt->GetRange();
        d3dLight.Attenuation0 = 0.f;
        d3dLight.Attenuation1 = 1.f;
        d3dLight.Attenuation2 = 0.f;

        mD3D9Device->SetLight(static_cast<DWORD>(num), &d3dLight);
        mD3D9Device->LightEnable(num, TRUE);
    }

    for (; num < mCapabilities.GetMaxLights(); ++num)
    {
        mD3D9Device->LightEnable(num, FALSE);
    }
}

void
NBRE_D3D9RenderPal::SetActiveLights( const NBRE_LightIndexList& indices )
{
    nbre_assert(indices.size() <= mCapabilities.GetMaxLights());

    nb_boolean* lights = NBRE_NEW nb_boolean[mCapabilities.GetMaxLights()];
    nsl_memset(lights, 0, sizeof(nb_boolean) * mCapabilities.GetMaxLights());

    for (NBRE_LightIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        lights[*it] = TRUE;
    }

    for (uint32 i = 0; i < mCapabilities.GetMaxLights(); ++i)
    {
        mD3D9Device->LightEnable(i, lights[i]);
    }
    NBRE_DELETE_ARRAY lights;
}

void
NBRE_D3D9RenderPal::SetWireframeMode(nb_boolean bLine)
{
    if (bLine)
    {
        mD3D9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    else
    {
        mD3D9Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
}

void
NBRE_D3D9RenderPal::EnableTexture( uint32 stage, nb_boolean bEnable )
{
    if (bEnable)
    {
        mD3D9Device->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
        mD3D9Device->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        mD3D9Device->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
        mD3D9Device->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

void
NBRE_D3D9RenderPal::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx )
{
    D3DTRANSFORMSTATETYPE d3dType;
    D3DMATRIX d3dMat;
    NBRE_Matrix4x4f matrix(mtrx[0][0], mtrx[0][1], mtrx[0][3], 0,
                           mtrx[1][0], mtrx[1][1], mtrx[1][3], 0,
                           0, 0,  1, 0,
                           0, 0,  0, 1);
    DWORD val = 0;
    switch(mTextureTypes[stage])
    {
    case NBRE_Texture::TT_1D:
        {
            val = D3DTTFF_COUNT1;
        }
        break;
    case NBRE_Texture::TT_2D:
        {
            val = D3DTTFF_COUNT2;
        }
        break;
    case NBRE_Texture::TT_3D:
        {
            val = D3DTTFF_COUNT3;
        }
        break;
    case NBRE_Texture::TT_CUBE_MAP:
        {
            val = D3DTTFF_COUNT3;
        }
        break;
    case NBRE_Texture::TT_2D_RENDERTARGET:
        {
            val = D3DTTFF_COUNT2;
            matrix *= NBRE_Matrix4x4f(
						1,  0, 0, 0,
						0, -1, 1, 0,
						0,  0, 1, 0,
						0,  0, 0, 1);
        }
        break;
    case NBRE_Texture::TT_CUBE_MAP_RENDERTARTE:
        {
            val = D3DTTFF_COUNT3;
        }
        break;
    default:
        nbre_assert(FALSE);
    }
    mD3D9Device->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, val);
    d3dType = (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage );
    d3dMat  = ToD3D9Matrix(matrix);
    mD3D9Device->SetTransform(d3dType, &d3dMat);
}

void 
NBRE_D3D9RenderPal::SetTextureCoordSet(uint32 stage, uint32 index)
{
    mD3D9Device->SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, index);
}

void
NBRE_D3D9RenderPal::DisableTextureUnitsFrom( uint32 stage )
{
    for (uint32 i = stage; i < mDisabledTexUnitsFrom; ++i)
    {
        mD3D9Device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        mD3D9Device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    mDisabledTexUnitsFrom = stage;
}

void
NBRE_D3D9RenderPal::SelectTexture(uint32 stage, NBRE_Texture* texture)
{
    nbre_assert(texture);
    texture->Load();
    NBRE_D3D9Texture *d3dTexture = static_cast<NBRE_D3D9Texture*>(texture);
    if (d3dTexture)
    {
        mTextureTypes[stage] = texture->GetType();
        mD3D9Device->SetTexture(stage, d3dTexture->GetD3DTexture());

        if(stage == 0)
        {
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
        }
        else
        {
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            mD3D9Device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
        }
    }
}

void
NBRE_D3D9RenderPal::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    switch(state.GetMinFilter())
    {
    case NBRE_TFT_POINT:
    case NBRE_TFT_LINEAR:
        {
            mD3D9Device->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        }
        break;
    case NBRE_TFT_MIPMAP_POINT:
    case NBRE_TFT_MIPMAP_LINEAR:
    case NBRE_TFT_MIPMAP_TRILINEAR:
        {
           mD3D9Device->SetSamplerState(stage, D3DSAMP_MIPFILTER, ToD3DFilterType(state.GetMinFilter()));
        }
        break;
    default:
        nbre_assert(0);
    }
    mD3D9Device->SetSamplerState(stage, D3DSAMP_MINFILTER, ToD3DFilterType(state.GetMinFilter()));
    mD3D9Device->SetSamplerState(stage, D3DSAMP_MAGFILTER, ToD3DFilterType(state.GetMagFilter()));

    mD3D9Device->SetSamplerState(stage, D3DSAMP_ADDRESSU, ToD3DAddressMode(state.GetWrapS()));
    mD3D9Device->SetSamplerState(stage, D3DSAMP_ADDRESSV, ToD3DAddressMode(state.GetWrapT()));

    if(mCapabilities.GetSupportAnisotropy() && state.UsingAnisotropic())
    {
        mD3D9Device->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, mMaxAnisotropy);
    }
    else
    {
        mD3D9Device->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, 0);
    }
}

NBRE_Texture*
NBRE_D3D9RenderPal::CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    return NBRE_NEW NBRE_D3D9Texture(this, &info, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_D3D9RenderPal::CreateTexture( NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name)
{
    nbre_assert(image != NULL && numMipmaps != 0);
    return NBRE_NEW NBRE_D3D9Texture(this, image, faceCount, numMipmaps, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_D3D9RenderPal::CreateTexture( NBRE_ITextureImage* textureLoader, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    nbre_assert(textureLoader != NULL);
    return NBRE_NEW NBRE_D3D9Texture(this, textureLoader, texType, isUsingMipmap, name);
}

NBRE_RenderTarget*
NBRE_D3D9RenderPal::CreateRenderTargetFromTexture( NBRE_Texture* texture, int32 face )
{
    nbre_assert(texture);
    return NBRE_NEW NBRE_D3D9RenderTexture(mD3D9Device, static_cast<NBRE_D3D9Texture*>(texture), face);
}

void
NBRE_D3D9RenderPal::SetColor(float r, float g, float b, float a)
{
#ifdef _DEBUG
    D3DMATERIAL9 mat = {0};
    D3DCOLORVALUE clr = {r, g, b, a};
    mat.Emissive = clr;
    mD3D9Device->SetMaterial(&mat);
#endif
}

NBRE_GpuProgram*
NBRE_D3D9RenderPal::LoadGpuProgram( const char* vertShaderSrc, const char* fragShaderSrc )
{
    return NULL;
}

void
NBRE_D3D9RenderPal::ReadPixels(int x, int y, uint32 width, uint32 height, uint8 *buffer) const
{
    //TODO...
}

void
NBRE_D3D9RenderPal::SetLineWidth(float width)
{
    //TODO...
}
