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
#include "nbred3d11rendertexture.h"
#include "nbred3d11texture.h"
#include "nbred3d11renderpal.h"

NBRE_D3D11RenderTexture::NBRE_D3D11RenderTexture(NBRE_D3D11RenderPal *pal, NBRE_D3D11Texture *texture, int32 face):mRenderPal(pal)
    ,mRenderTargetView(NULL)
    ,mDepthStencilView(NULL)
    ,mTexture(texture)
{
    Initialize();
}

NBRE_D3D11RenderTexture::~NBRE_D3D11RenderTexture()
{
    NBRE_SAFE_RELEASE(mRenderTargetView);
    NBRE_SAFE_RELEASE(mDepthStencilView);
}

void
NBRE_D3D11RenderTexture::BeginRender()
{
    mRenderPal->GetImmediateContext()->OMGetRenderTargets(1, &mOldRenderTargetView, &mOldDepthStencilView);
    mRenderPal->ChangeD3D11View(mRenderTargetView, mDepthStencilView);
    mRenderPal->GetImmediateContext()->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
}

void
NBRE_D3D11RenderTexture::EndRender()
{
    mRenderPal->ChangeD3D11View(mOldRenderTargetView, mOldDepthStencilView);
    mRenderPal->GetImmediateContext()->OMSetRenderTargets(1, &mOldRenderTargetView, mOldDepthStencilView);
    NBRE_SAFE_RELEASE(mOldRenderTargetView);
    NBRE_SAFE_RELEASE(mOldDepthStencilView);
}

void
NBRE_D3D11RenderTexture::Initialize()
{
    // Create the render target view.
    HRESULT hr = mRenderPal->GetActiveD3D11Device()->CreateRenderTargetView(mTexture->GetTex2D(), NULL, &mRenderTargetView);
    nbre_assert(SUCCEEDED(hr));
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = mTexture->GetWidth();
    depthBufferDesc.Height = mTexture->GetHeight();
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;
    ID3D11Texture2D* depthStencilBuffer = NULL;
    hr = mRenderPal->GetActiveD3D11Device()->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
    nbre_assert(SUCCEEDED(hr));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = mRenderPal->GetActiveD3D11Device()->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &mDepthStencilView);
    depthStencilBuffer->Release();
    nbre_assert(SUCCEEDED(hr));
}