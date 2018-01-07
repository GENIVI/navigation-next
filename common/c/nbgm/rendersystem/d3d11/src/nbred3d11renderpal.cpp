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
#include "nbred3d11renderpal.h"
#include "nbred3d11hardwareindexbuffer.h"
#include "nbred3d11hardwarevertexbuffer.h"
#include "nbreitextureimage.h"
#include "nbred3d11vertexdeclaration.h"
#include "nbrematerial.h"
#include "nbred3d11renderwindow.h"
#include "nbretransformation.h"
#include "nbred3d11fixedfunctionSimulator.h"
#include "nbred3d11gpuprogramfactory.h"
#include "nbred3d11texture.h"
#include "nbred3d11rendertexture.h"
#include "nbretypeconvert.h"

static D3D11_PRIMITIVE_TOPOLOGY
ToD3D11PRIMITIVE(NBRE_PrimitiveType opt)
{
    switch (opt)
    {
    case NBRE_PMT_POINT_LIST:
        return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case NBRE_PMT_LINE_LIST:
        return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case NBRE_PMT_LINE_STRIP:
        return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case NBRE_PMT_TRIANGLE_LIST:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case NBRE_PMT_TRIANGLE_STRIP:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case NBRE_PMT_TRIANGLE_FAN:
    default:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

static D3D11_BLEND
ToD3D11BlendFunction(NBRE_BlendFunction bf)
{
    switch (bf)
    {
    case NBRE_BF_ZERO:
        return D3D11_BLEND_ZERO;
    case NBRE_BF_ONE:
        return D3D11_BLEND_ONE;
    case NBRE_BF_SRC_COLOR:
        return D3D11_BLEND_SRC_COLOR;
    case NBRE_BF_ONE_MINUS_SRC_COLOR:
        return D3D11_BLEND_INV_SRC_COLOR;
    case NBRE_BF_SRC_ALPHA:
        return D3D11_BLEND_SRC_ALPHA;
    case NBRE_BF_ONE_MINUS_SRC_ALPHA:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case NBRE_BF_DST_COLOR:
        return D3D11_BLEND_DEST_COLOR;
    case NBRE_BF_ONE_MINUS_DST_COLOR:
        return D3D11_BLEND_INV_DEST_COLOR;
    case NBRE_BF_DST_ALPHA:
        return D3D11_BLEND_DEST_ALPHA;
    case NBRE_BF_ONE_MINUS_DST_ALPHA:
        return D3D11_BLEND_INV_DEST_ALPHA;
    default:
        return D3D11_BLEND_ONE;
    }
}

static D3D11_COMPARISON_FUNC
ToD3D11CompareFunction(NBRE_CompareFunction cf)
{
    switch(cf)
    {
    case NBRE_CF_NEVER:
        return D3D11_COMPARISON_NEVER;
    case NBRE_CF_LESS:
        return D3D11_COMPARISON_LESS;
    case NBRE_CF_LESS_EQUAL:
        return D3D11_COMPARISON_LESS_EQUAL;
    case NBRE_CF_EQUAL:
        return D3D11_COMPARISON_EQUAL;
    case NBRE_CF_GREATER:
        return D3D11_COMPARISON_GREATER;
    case NBRE_CF_NOT_EQUAL:
        return D3D11_COMPARISON_NOT_EQUAL;
    case NBRE_CF_GREATER_EQUAL:
        return D3D11_COMPARISON_GREATER_EQUAL;
    case NBRE_CF_ALWAYS:
        return D3D11_COMPARISON_ALWAYS;
    default:
        return D3D11_COMPARISON_ALWAYS;
    }
}

static void
GetD3D11Capabilities(ID3D11Device* device, NBRE_RenderPalCapabilities& cap)
{
    nbre_assert(device);
    cap.SetAutoMipmaps(TRUE);
    cap.SetMaxLights(NBRE_D3D11_LIGHT_NUM);
    cap.SetMaxTextureUnits(NBRE_D3D11_TEXTRUE_NUM);
    cap.SetMaxStreams(NBRE_D3D11_BASE_SUPPORT_NUM);
    cap.SetSupportCubeMap(FALSE);
}

static NBRE_Matrix4x4f
ToD3DPerspectiveMatrix(const NBRE_Matrix4x4f& mat)
{
    return NBRE_Matrix4x4f ( mat[0][0],  mat[0][1],   -mat[0][2], mat[0][3],
                             mat[1][0],  mat[1][1],   -mat[1][2], mat[1][3],
     (mat[2][0] + mat[3][0])/2.f,   (mat[2][1] + mat[3][1])/2.f, -(mat[2][2] + mat[3][2])/2.f, (mat[2][3] + mat[3][3])/2.f,
                             mat[3][0],  mat[3][1],   -mat[3][2], mat[3][3]);
};

static void
Init(ID3D11Device* device, ID3D11DeviceContext *context,  D3D11_RASTERIZER_DESC &state)
{
    nsl_memset(&state, 0, sizeof(state));
    state.FillMode = D3D11_FILL_SOLID;
    state.CullMode =  D3D11_CULL_BACK;
    state.FrontCounterClockwise = TRUE;
    state.DepthClipEnable = TRUE;
    ID3D11RasterizerState *rs = NULL;
    device->CreateRasterizerState(&state, &rs);
    context->RSSetState(rs);
    NBRE_SAFE_RELEASE(rs);
}

static void
Init(ID3D11Device* device, ID3D11DeviceContext *context,  D3D11_DEPTH_STENCIL_DESC &state)
{
    nsl_memset(&state, 0, sizeof(state));
    state.DepthEnable = TRUE;
    state.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    state.DepthFunc = D3D11_COMPARISON_LESS;
    state.StencilEnable = FALSE;
    state.StencilReadMask = TRUE;
    state.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    state.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    state.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
    state.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    state.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    state.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    state.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
    state.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    state.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    ID3D11DepthStencilState *ds = NULL;
    device->CreateDepthStencilState(&state, &ds);
    context->OMSetDepthStencilState(ds, 1);
    NBRE_SAFE_RELEASE(ds);
}

static
void Init(ID3D11Device* device, ID3D11DeviceContext *context,  D3D11_BLEND_DESC& state )
{
     nsl_memset(&state, 0, sizeof(state));
     state.AlphaToCoverageEnable = FALSE;
     state.IndependentBlendEnable = FALSE;
     state.RenderTarget[0].BlendEnable = FALSE;
     state.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
     state.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
     state.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
     state.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
     state.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
     state.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
     state.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
     ID3D11BlendState *bs = NULL;
     static FLOAT factors[4] = { 1.f, 1.f, 1.f, 1.f };
     device->CreateBlendState(&state, &bs);
     context->OMSetBlendState(bs, factors, 0xffffffff);
     NBRE_SAFE_RELEASE(bs);
}

static void
RestoreVertexClientState(ID3D11DeviceContext* context, const NBRE_RenderPalCapabilities& cap)
{
    for (uint32 i = 0; i < cap.GetMaxStreams(); ++i)
    {
        context->IASetVertexBuffers(i, 0, NULL, NULL, NULL);
    }
}

NBRE_D3D11RenderPal::NBRE_D3D11RenderPal(const NBRE_D3D11RenderEnv &env):mClearDepth(1.f)
,mClearStencil(0)
,mD3D11RenderWindow(NULL)
,mCurrentRenderTarget(NULL)
,mCurrentPrimitiveType(NBRE_PMT_TRIANGLE_LIST)
,mCurrentLayout(NULL)
,mInputVertexFormat(0)
,mNeedNormalInput(FALSE)
,mDepthStateCacheValid(TRUE)
,mBlendStateCacheValid(TRUE)
,mRasterStateCacheValid(TRUE)
{
    mEnv = env;
    Initialize();
}

NBRE_D3D11RenderPal::~NBRE_D3D11RenderPal()
{
    NBRE_DELETE_ARRAY mTextureTypes;
    NBRE_DELETE_ARRAY mTextureEnables;

    NBRE_DELETE mD3D11RenderWindow;
    NBRE_DELETE mShaderFactory;
    NBRE_DELETE mSimulator;

    for(uint32 i = 0; i < mDepthStateCache.size(); i++)
    {
        NBRE_SAFE_RELEASE(mDepthStateCache[i].state);
    }

    for(uint32 i = 0; i < mBlendStateCache.size(); i++)
    {
        NBRE_SAFE_RELEASE(mBlendStateCache[i].state);
    }

    for(uint32 i = 0; i < mRasterStateCache.size(); i++)
    {
        NBRE_SAFE_RELEASE(mRasterStateCache[i].state);
    }
}

void
NBRE_D3D11RenderPal::ChangeD3D11View(ID3D11RenderTargetView* targetView, ID3D11DepthStencilView *depthView)
{
    nbre_assert( targetView && depthView );
    mEnv.mRenderTargetView = targetView;
    mEnv.mDepthStecilView = depthView;
}

void
NBRE_D3D11RenderPal::Initialize()
{
    GetD3D11Capabilities(mEnv.mD3D11Device, mCapabilities);
    mTextureTypes = NBRE_NEW int32[mCapabilities.GetMaxTextureUnits()];
    mTextureEnables = NBRE_NEW nb_boolean[mCapabilities.GetMaxTextureUnits()];
    nsl_memset(mTextureTypes, 0, sizeof(int32)*mCapabilities.GetMaxTextureUnits());
    nsl_memset(mTextureEnables, 0, sizeof(nb_boolean)*mCapabilities.GetMaxTextureUnits());
    ZeroMemory(&mViewPort, sizeof(mViewPort));

    mSimulator = NBRE_NEW NBRE_D3D11FixedFuncSimulator(this);
    mShaderFactory = NBRE_NEW NBRE_D3D11GPUPorgramFactory(this);
    InitShaders();
    Init(mEnv.mD3D11Device, mEnv.mImmediateContext, mRasterizerDesc);
    Init(mEnv.mD3D11Device, mEnv.mImmediateContext, mDepthStencilDesc);
    Init(mEnv.mD3D11Device, mEnv.mImmediateContext, mBlendDesc);
    CacheDepthStencialState();
    CacheBlendState();
    CacheRasterState();
    GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void
NBRE_D3D11RenderPal::CacheDepthStencialState()
{
    DepthState st;

    mDepthStencilDesc.DepthEnable = TRUE;
    mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ID3D11DepthStencilState *ds = NULL;
    mEnv.mD3D11Device->CreateDepthStencilState(&mDepthStencilDesc, &ds);
    st.DepthEnable = mDepthStencilDesc.DepthEnable;
    st.DepthWriteMask = mDepthStencilDesc.DepthWriteMask;
    st.state = ds;
    if(ds)
    {
        mDepthStateCache.push_back(st);
    }

    mDepthStencilDesc.DepthEnable = TRUE;
    mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    ds = NULL;
    mEnv.mD3D11Device->CreateDepthStencilState(&mDepthStencilDesc, &ds);
    st.DepthEnable = mDepthStencilDesc.DepthEnable;
    st.DepthWriteMask = mDepthStencilDesc.DepthWriteMask;
    st.state = ds;
    if(ds)
    {
        mDepthStateCache.push_back(st);
    }

    mDepthStencilDesc.DepthEnable = FALSE;
    mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds = NULL;
    mEnv.mD3D11Device->CreateDepthStencilState(&mDepthStencilDesc, &ds);
    st.DepthEnable = mDepthStencilDesc.DepthEnable;
    st.DepthWriteMask = mDepthStencilDesc.DepthWriteMask;
    st.state = ds;
    if(ds)
    {
        mDepthStateCache.push_back(st);
    }

    mDepthStencilDesc.DepthEnable = FALSE;
    mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    ds = NULL;
    mEnv.mD3D11Device->CreateDepthStencilState(&mDepthStencilDesc, &ds);
    st.DepthEnable = mDepthStencilDesc.DepthEnable;
    st.DepthWriteMask = mDepthStencilDesc.DepthWriteMask;
    st.state = ds;
    if(ds)
    {
        mDepthStateCache.push_back(st);
    }

    //restore to default state
    mDepthStencilDesc.DepthEnable = TRUE;
    mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
}

void
NBRE_D3D11RenderPal::CacheBlendState()
{
    BlendState st;
    ID3D11BlendState *bs = NULL;
    mBlendDesc.RenderTarget[0].BlendEnable = FALSE;
    mBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    mEnv.mD3D11Device->CreateBlendState(&mBlendDesc, &bs);
    st.state = bs;
    st.BlendEnable = FALSE;
    st.ColorMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if(bs)
    {
        mBlendStateCache.push_back(st);
    }

    bs = NULL;
    mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    mBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    mEnv.mD3D11Device->CreateBlendState(&mBlendDesc, &bs);
    st.state = bs;
    st.BlendEnable = TRUE;
    st.ColorMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if(bs)
    {
        mBlendStateCache.push_back(st);
    }

    bs = NULL;
    mBlendDesc.RenderTarget[0].BlendEnable = FALSE;
    mBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
    mEnv.mD3D11Device->CreateBlendState(&mBlendDesc, &bs);
    st.state = bs;
    st.BlendEnable = FALSE;
    st.ColorMask = 0;
    if(bs)
    {
        mBlendStateCache.push_back(st);
    }

    bs = NULL;
    mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    mBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
    mEnv.mD3D11Device->CreateBlendState(&mBlendDesc, &bs);
    st.state = bs;
    st.BlendEnable = TRUE;
    st.ColorMask = 0;
    if(bs)
    {
        mBlendStateCache.push_back(st);
    }

    //restore to default state
    mBlendDesc.RenderTarget[0].BlendEnable = FALSE;
    mBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

void
NBRE_D3D11RenderPal::CacheRasterState()
{
    RasterState st;
    ID3D11RasterizerState *rs = NULL;

    mRasterizerDesc.CullMode =  D3D11_CULL_BACK;
    mEnv.mD3D11Device->CreateRasterizerState(&mRasterizerDesc, &rs);
    st.CullMode = D3D11_CULL_BACK;
    st.state = rs;
    if(rs)
    {
        mRasterStateCache.push_back(st);
    }

    rs = NULL;
    mRasterizerDesc.CullMode =  D3D11_CULL_NONE;
    mEnv.mD3D11Device->CreateRasterizerState(&mRasterizerDesc, &rs);
    st.CullMode = D3D11_CULL_NONE;
    st.state = rs;
    if(rs)
    {
        mRasterStateCache.push_back(st);
    }

    rs = NULL;
    mRasterizerDesc.CullMode =  D3D11_CULL_FRONT;
    mEnv.mD3D11Device->CreateRasterizerState(&mRasterizerDesc, &rs);
    st.CullMode = D3D11_CULL_FRONT;
    st.state = rs;
    if(rs)
    {
        mRasterStateCache.push_back(st);
    }

    //restore to default state
    mRasterizerDesc.CullMode =  D3D11_CULL_BACK;
}


PAL_Error
NBRE_D3D11RenderPal::InitShaders()
{
    PAL_Error err = PAL_Ok;
    mShaderFactory->Initialize();
    err = mSimulator->LoadShader(mShaderFactory->GetDefaultVS(0));
    err = (err == PAL_Ok)?mSimulator->LoadShader(mShaderFactory->GetDefaultPS(0)):err;
    mSimulator->BindGeneralBuffer();
    return err;
}

void
NBRE_D3D11RenderPal::ResetRasterizerState()
{
    ID3D11RasterizerState *state = NULL;
    mEnv.mD3D11Device->CreateRasterizerState(&mRasterizerDesc, &state);
    mEnv.mImmediateContext->RSSetState(state);
    NBRE_SAFE_RELEASE(state);
}

void
NBRE_D3D11RenderPal::ResetDepthStencilState()
{
    ID3D11DepthStencilState *state = NULL;
    mEnv.mD3D11Device->CreateDepthStencilState(&mDepthStencilDesc, &state);
    mEnv.mImmediateContext->OMSetDepthStencilState(state, 1);
    NBRE_SAFE_RELEASE(state);
}

void
NBRE_D3D11RenderPal::ResetBlendState()
{
    ID3D11BlendState *bs = NULL;
    static FLOAT alphas[4] = { 1.f, 1.f, 1.f, 1.f };
    mEnv.mD3D11Device->CreateBlendState(&mBlendDesc, &bs);
    mEnv.mImmediateContext->OMSetBlendState(bs, alphas, 0xffffffff);
    NBRE_SAFE_RELEASE(bs);
}

uint32
NBRE_D3D11RenderPal::FilterVertexInputFormat(uint32 format)
{
    if(!mNeedNormalInput)
    {
        format &= ~NBRE_VSIF_NORMAL;
    }
    if(!mTextureEnables[0])
    {
        format &= ~ NBRE_VSIF_TEXCORD0;
    }

    if(!mTextureEnables[1])
    {
        format &= ~ NBRE_VSIF_TEXCORD1;
    }

    return format;
}

void
NBRE_D3D11RenderPal::SetVertexBuffer(NBRE_VertexData* vertexData )
{
    nbre_assert(vertexData);
    RestoreVertexClientState(GetImmediateContext(), mCapabilities);
    const NBRE_D3D11VertexDeclaration *del = static_cast<const NBRE_D3D11VertexDeclaration*>(vertexData->GetVertexDeclaration());
    uint32 vformtat = FilterVertexInputFormat(del->GetVertexInputFormat());
    if(vformtat != mInputVertexFormat)
    {
        mSimulator->LoadShader(mShaderFactory->GetDefaultVS(vformtat));
        mSimulator->LoadShader(mShaderFactory->GetDefaultPS(vformtat));
        mInputVertexFormat = vformtat;
    }
    const NBRE_VertexDeclaration::VertexElementList& decl = del->GetVertexElementList();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin(); 
    NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd = decl.end();
    for (elem; elem != elemEnd; ++elem)
    {
        const NBRE_VertexElement* element = *elem;
        if(element->Semantic() == NBRE_VertexElement::VES_NORMAL || element->Semantic() == NBRE_VertexElement::VES_USER_DEFINE)
            continue;
        NBRE_D3D11HardwareVertexBuffer *d3d11buf = static_cast<NBRE_D3D11HardwareVertexBuffer*>(const_cast<NBRE_HardwareVertexBuffer*>(vertexData->GetBuffer(element->Source())));
        ID3D11Buffer *buffer = d3d11buf->GetD3DVertextBuffer();
        UINT stride = static_cast<UINT>(d3d11buf->GetVertexSize());
        UINT offset = 0;
        GetImmediateContext()->IASetVertexBuffers(element->Source(), 1, &buffer, &stride, &offset);
    }
    ID3D11InputLayout* layout = del->GetInputLayoutByShader(mSimulator->GetVS().Get());
    if(mCurrentLayout != layout)
    {
         GetImmediateContext()->IASetInputLayout(layout);
         mCurrentLayout = layout;
    }
}

void
NBRE_D3D11RenderPal::DrawIndexedPrimitive( NBRE_PrimitiveType operationType, const NBRE_IndexData* indexData )
{
    nbre_assert(indexData);

    if(mCurrentPrimitiveType != operationType)
    {
        mCurrentPrimitiveType = operationType;
        GetImmediateContext()->IASetPrimitiveTopology(ToD3D11PRIMITIVE(operationType));
    }
    NBRE_D3D11HardwareIndexBuffer *buffer = static_cast<NBRE_D3D11HardwareIndexBuffer *>(indexData->IndexBuffer());
    if(buffer)
    {
        GetImmediateContext()->IASetIndexBuffer( buffer->GetD3DIndexBuffer(), buffer->GetD3DFormat(), 0);
        GetImmediateContext()->DrawIndexed(indexData->IndexCount(), indexData->IndexStart(), 0);
    }
    else
    {
        GetImmediateContext()->Draw(indexData->IndexCount(), 0);
    }
}

void
NBRE_D3D11RenderPal::SetProjectionTransform( const NBRE_Matrix4x4f& mtrx )
{
    mSimulator->SetProjection(ToD3DPerspectiveMatrix(mtrx*NBRE_Transformationf::BuildScaleMatrix(1.f, 1.f, -1.f)));
}

void
NBRE_D3D11RenderPal::SetViewTransform( const NBRE_Matrix4x4d& mtrx )
{
    mSimulator->SetView(mtrx);
}

void
NBRE_D3D11RenderPal::SetWorldTransform( const NBRE_Matrix4x4d& mtrx )
{
    mSimulator->SetWorld(mtrx);
}

void
NBRE_D3D11RenderPal::SetClearColor(float r, float g, float b, float a)
{
    mClearColor.r = r;
    mClearColor.g = g;
    mClearColor.b = b;
    mClearColor.a = a;
}

void
NBRE_D3D11RenderPal::SetClearDepth(float val)
{
    mClearDepth = val;
}

void
NBRE_D3D11RenderPal::SetClearStencil(int32 val)
{
    mClearStencil = val;
}

void
NBRE_D3D11RenderPal::Clear(uint32 flags)
{
    if (flags & NBRE_CLF_CLEAR_COLOR)
    {
        mEnv.mImmediateContext->ClearRenderTargetView(mEnv.mRenderTargetView, &mClearColor.r);
    }
    UINT ClearFlags = 0;
    if (flags & NBRE_CLF_CLEAR_DEPTH)
    {
        ClearFlags |= D3D11_CLEAR_DEPTH;
    }
    if (flags & NBRE_CLF_CLEAR_STENCIL)
    {
        ClearFlags |= D3D11_CLEAR_STENCIL;
    }
    if (ClearFlags)
    {
        mEnv.mImmediateContext->ClearDepthStencilView(mEnv.mDepthStecilView, ClearFlags, mClearDepth, static_cast<UINT8>(mClearStencil));
    }
}

void
NBRE_D3D11RenderPal::EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha )
{
     UINT8 mask = 0;
     if(enableRed)
     {
         mask |= D3D11_COLOR_WRITE_ENABLE_RED;
     }
     if(enableGreen)
     {
         mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
     }
     if(enableBlue)
     {
         mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
     }
     if(enableAlpha)
     {
         mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
     }
     if(mask != mBlendDesc.RenderTarget[0].RenderTargetWriteMask)
     {
        mBlendDesc.RenderTarget[0].RenderTargetWriteMask = mask;

        if(mBlendStateCacheValid)
        {
            for(uint32 i = 0; i < mBlendStateCache.size(); i++)
            {
                if(mBlendStateCache[i].ColorMask == mask && mBlendStateCache[i].BlendEnable == mBlendDesc.RenderTarget[0].BlendEnable)
                {
                    static FLOAT alphas[4] = { 1.f, 1.f, 1.f, 1.f };
                    mEnv.mImmediateContext->OMSetBlendState(mBlendStateCache[i].state, alphas, 0xffffffff);
                    return;
                }
            }
        }
        ResetBlendState();
     }
}

void
NBRE_D3D11RenderPal::EnableDepthWrite( nb_boolean bEnable )
{
    D3D11_DEPTH_WRITE_MASK mask = bEnable?D3D11_DEPTH_WRITE_MASK_ALL:D3D11_DEPTH_WRITE_MASK_ZERO;
    if(mask != mDepthStencilDesc.DepthWriteMask)
    {
        mDepthStencilDesc.DepthWriteMask = mask;
        if(mDepthStateCacheValid)
        {
            for(uint32 i = 0; i < mDepthStateCache.size(); i++)
            {
                if(mDepthStateCache[i].DepthWriteMask == mask && mDepthStateCache[i].DepthEnable == mDepthStencilDesc.DepthEnable)
                {
                    mEnv.mImmediateContext->OMSetDepthStencilState(mDepthStateCache[i].state, 1);
                    return;
                }
            }
        }
        ResetDepthStencilState();
    }
}


void
NBRE_D3D11RenderPal::EnableDepthTest( nb_boolean bEnable )
{
    if(mDepthStencilDesc.DepthEnable != bEnable)
    {
        mDepthStencilDesc.DepthEnable = bEnable;
        if(mDepthStateCacheValid)
        {
            for(uint32 i = 0; i < mDepthStateCache.size(); i++)
            {
                if(mDepthStateCache[i].DepthWriteMask == mDepthStencilDesc.DepthWriteMask && mDepthStateCache[i].DepthEnable == bEnable)
                {
                    mEnv.mImmediateContext->OMSetDepthStencilState(mDepthStateCache[i].state, 1);
                    return;
                }
            }
        }
        ResetDepthStencilState();
    }
}

void
NBRE_D3D11RenderPal::EnableStencilTest( nb_boolean bEnable )
{
    if(mDepthStencilDesc.StencilEnable != bEnable)
    {
        mDepthStencilDesc.StencilEnable = bEnable;
        ResetDepthStencilState();
        mDepthStateCacheValid = FALSE;
    }
}

void
NBRE_D3D11RenderPal::EnableStencilWrite( uint32 maskBits )
{
    UINT8 mask = static_cast<UINT8>(maskBits);
    if(mask != mDepthStencilDesc.StencilWriteMask)
    {
        mDepthStencilDesc.StencilWriteMask = mask;
        ResetDepthStencilState();
        mDepthStateCacheValid = FALSE;
    }
}

void
NBRE_D3D11RenderPal::EnableBlend( nb_boolean bEnable )
{
    if(mBlendDesc.RenderTarget[0].BlendEnable != bEnable)
    {
        mBlendDesc.RenderTarget[0].BlendEnable = bEnable;

        if(mBlendStateCacheValid)
        {
            for(uint32 i = 0; i < mBlendStateCache.size(); i++)
            {
                if(mBlendStateCache[i].BlendEnable == bEnable && mBlendStateCache[i].ColorMask == mBlendDesc.RenderTarget[0].RenderTargetWriteMask)
                {
                    static FLOAT alphas[4] = { 1.f, 1.f, 1.f, 1.f };
                    mEnv.mImmediateContext->OMSetBlendState(mBlendStateCache[i].state, alphas, 0xffffffff);
                    return;
                }
            }
        }
        ResetBlendState();
    }
}


void
NBRE_D3D11RenderPal::EnableCullFace( nb_boolean bEnable )
{
    D3D11_CULL_MODE cull =  bEnable?D3D11_CULL_BACK:D3D11_CULL_NONE;
    if(mRasterizerDesc.CullMode != cull)
    {
        mRasterizerDesc.CullMode = cull;
        if(mRasterStateCacheValid)
        {
            for(uint32 i = 0; i < mRasterStateCache.size(); i++)
            {
                if(mRasterStateCache[i].CullMode == cull)
                {
                    mEnv.mImmediateContext->RSSetState(mRasterStateCache[i].state);
                    return;
                }
            }
        }
        ResetRasterizerState();
    }
}

void
NBRE_D3D11RenderPal::SetDepthFunc(NBRE_CompareFunction cf)
{
    D3D11_COMPARISON_FUNC dcf = ToD3D11CompareFunction(cf);
    if(mDepthStencilDesc.DepthFunc != dcf)
    {
        mDepthStencilDesc.DepthFunc = dcf;
        ResetDepthStencilState();
        mDepthStateCacheValid = FALSE;
    }
}

void
NBRE_D3D11RenderPal::SelectRenderTarget( NBRE_RenderTarget* rt )
{
    mCurrentRenderTarget = rt;
}

void
NBRE_D3D11RenderPal::SetViewPort( int32 left, int32 top, uint32 width , uint32 height )
{
    if((int32)mViewPort.TopLeftX != left || (int32)mViewPort.TopLeftY != top ||(uint32) mViewPort.Width != width  || (uint32)mViewPort.Height != height )
    {
        mViewPort.TopLeftX = static_cast<FLOAT>(left);
        mViewPort.TopLeftY = static_cast<FLOAT>(top);
        mViewPort.Width = static_cast<FLOAT>(width);
        mViewPort.Height = static_cast<FLOAT>(height);
        mViewPort.MinDepth = 0.0f;
        mViewPort.MaxDepth = 1.0f;
        mEnv.mImmediateContext->RSSetViewports(1, &mViewPort);
    }
}
void
NBRE_D3D11RenderPal::SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor)
{
    D3D11_BLEND src = ToD3D11BlendFunction(srcFactor);
    D3D11_BLEND dst = ToD3D11BlendFunction(dstFactor);
    if(mBlendDesc.RenderTarget[0].SrcBlend != src || mBlendDesc.RenderTarget[0].DestBlend != dst)
    {
        mBlendDesc.RenderTarget[0].SrcBlend = src;
        mBlendDesc.RenderTarget[0].DestBlend = dst;
        mBlendDesc.RenderTarget[0].SrcBlendAlpha = src;
        mBlendDesc.RenderTarget[0].DestBlendAlpha = dst;
        ResetBlendState();
        mBlendStateCacheValid = FALSE;
    }
}

nb_boolean
NBRE_D3D11RenderPal::BeginScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->BeginRender();
    }
    return TRUE;
}

void
NBRE_D3D11RenderPal::EndScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->EndRender();
    }
}

NBRE_RenderTarget*
NBRE_D3D11RenderPal::GetDefaultRenderTarget()
{
    if(mD3D11RenderWindow == NULL)
    {
        mD3D11RenderWindow = NBRE_NEW NBRE_D3D11RenderWindow();
    }
    return mD3D11RenderWindow;
}

NBRE_HardwareVertexBuffer*
NBRE_D3D11RenderPal::CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_D3D11HardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, usage, this);
}

NBRE_HardwareIndexBuffer*
NBRE_D3D11RenderPal::CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_D3D11HardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, usage, this);
}

NBRE_VertexDeclaration*
NBRE_D3D11RenderPal::CreateVertexDeclaration(void)
{
    return NBRE_NEW NBRE_D3D11VertexDeclaration(this);
}

void
NBRE_D3D11RenderPal::SetWireframeMode(nb_boolean bLine)
{
    D3D11_FILL_MODE mode = bLine?D3D11_FILL_WIREFRAME:D3D11_FILL_SOLID;
    if(mRasterizerDesc.FillMode != mode)
    {
        mRasterizerDesc.FillMode = mode;
        ResetRasterizerState();
        mRasterStateCacheValid = FALSE;
    }
}

void
NBRE_D3D11RenderPal::EnableTexture( uint32 stage, nb_boolean bEnable )
{
    nbre_assert(stage < mCapabilities.GetMaxTextureUnits());
    mTextureEnables[stage] = bEnable;
}

void
NBRE_D3D11RenderPal::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx )
{
     if(mTextureTypes[stage] == NBRE_Texture::TT_2D_RENDERTARGET)
     {
         mSimulator->SetTextureTransform(stage, mtrx*NBRE_Matrix4x4f(
                        1,  0, 0, 0,
                        0, -1, 0, 1,
                        0,  0, 1, 0,
                        0,  0, 0, 1));
     }
     else
     {
         mSimulator->SetTextureTransform(stage, mtrx);
     }
}

void
NBRE_D3D11RenderPal::DisableTextureUnitsFrom( uint32 stage )
{
    for (uint32 i = stage; i < mCapabilities.GetMaxTextureUnits(); ++i)
    {
        EnableTexture(stage, FALSE);
    }
}

void
NBRE_D3D11RenderPal::SelectTexture(uint32 stage, NBRE_Texture* texture)
{
    nbre_assert(texture && stage < mCapabilities.GetMaxTextureUnits());
    texture->Load();
    NBRE_D3D11Texture *d3dTexture = static_cast<NBRE_D3D11Texture*>(texture);
    if (d3dTexture)
    {
        EnableTexture(stage, TRUE);
        mSimulator->SelectTexture(stage, texture->GetType(), d3dTexture->GetTexture());
        mTextureTypes[stage] = texture->GetType();
    }
}

void
NBRE_D3D11RenderPal::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    mSimulator->SetTextureState(stage, state);
}

NBRE_Texture*
NBRE_D3D11RenderPal::CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    return NBRE_NEW NBRE_D3D11Texture(&info, texType, isUsingMipmap, name, this);
}

NBRE_Texture*
NBRE_D3D11RenderPal::CreateTexture( NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name)
{
    nbre_assert(image && numMipmaps != 0);
    return NBRE_NEW NBRE_D3D11Texture(image, faceCount, numMipmaps, texType, isUsingMipmap, name, this);
}

NBRE_Texture*
NBRE_D3D11RenderPal::CreateTexture( NBRE_ITextureImage* textureLoader, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    nbre_assert(textureLoader);
    return NBRE_NEW NBRE_D3D11Texture(textureLoader, texType, isUsingMipmap, name, this);
}

NBRE_RenderTarget*
NBRE_D3D11RenderPal::CreateRenderTargetFromTexture( NBRE_Texture* texture, int32 face )
{
    nbre_assert(texture);
    return NBRE_NEW NBRE_D3D11RenderTexture(this, static_cast<NBRE_D3D11Texture*>(texture), face);
}

void
NBRE_D3D11RenderPal::SetTextureCoordSet(uint32 stage, uint32 index)
{
}

NBRE_GpuProgram*
NBRE_D3D11RenderPal::LoadGpuProgram( const char* vertShaderSrc, const char* fragShaderSrc )
{
    return NULL;
}

void
NBRE_D3D11RenderPal::EnableLighting( nb_boolean bEnable )
{
    mNeedNormalInput = bEnable;
}

void
NBRE_D3D11RenderPal::SetColor(float r, float g, float b, float a)
{
    //mSimulator->SetColor(r, g, b, a);
}

void
NBRE_D3D11RenderPal::SetMaterial(const NBRE_Material& material)
{
    //mSimulator->SetMaterial(material);
}


void
NBRE_D3D11RenderPal::UseLights( const NBRE_LightList& lights )
{
    //nbre_assert(lights.size() <= mCapabilities.GetMaxLights());
    //mSimulator->UseLights(lights);
}

void
NBRE_D3D11RenderPal::SetActiveLights( const NBRE_LightIndexList& indices )
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
        mSimulator->EnableLight(i, lights[i]);
    }
    NBRE_DELETE_ARRAY lights;
}

void
NBRE_D3D11RenderPal::EnableAlphaTest( nb_boolean bEnable )
{
    //TODO...
}

void
NBRE_D3D11RenderPal::SetAlphaFunc(NBRE_CompareFunction cf, float refValue)
{
    //TODO...
}

void
NBRE_D3D11RenderPal::EnableFog( nb_boolean bEnable )
{
    //TODO...
}

void
NBRE_D3D11RenderPal::SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha)
{
    //TODO...
}

void
NBRE_D3D11RenderPal::SetLineWidth(float width)
{
    //TODO...
}
