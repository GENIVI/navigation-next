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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "stdafx.h"
#include "D3D9View.h"
#include "nbgmtest.h"
#include "nbred3d9renderpal.h"
#include "nbrecommon.h"
#include "nbgm.h"
#include "nbgmrendercontext32.h"

CD3D9View::CD3D9View(CNBGMUITestSuit *suit):CNBGMTestView(suit),m_D3D9(NULL),m_D3D9Device(NULL)
{
     ZeroMemory(&mD3DPP,sizeof(mD3DPP));
}

CD3D9View::~CD3D9View()
{
    m_EnableRender = FALSE;
    NBRE_DELETE m_TestSuit;
    if( m_ResourceManager != NULL)
    {
        NBGM_DestroyResourceManager(&m_ResourceManager);
    }
    if( m_D3D9Device != NULL )
        m_D3D9Device->Release();

    if( m_D3D9 != NULL )
        m_D3D9->Release();
}

void CD3D9View::OnRenderEnd()
{
    m_D3D9Device->Present( NULL, NULL, NULL, NULL );
}

void CD3D9View::OnRenderBegine()
{
    if(m_EnableRender)
    {
        m_TestSuit->InitRenderState();
    }
}

BOOL CD3D9View::InitDrawEnv()
{
    // Create the D3D object, which is needed to create the D3DDevice.
    if( NULL == ( m_D3D9 = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return FALSE;

    // Set up the structure used to create the D3DDevice. Most parameters are
    // zeroed out. We set Windowed to TRUE, since we want to do D3D in a
    // window, and then set the SwapEffect to "discard", which is the most
    // efficient method of presenting the back buffer to the display.  And 
    // we request a back buffer format that matches the current desktop display 
    // format.
    mD3DPP.Windowed = TRUE;
    mD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
    mD3DPP.BackBufferFormat = D3DFMT_UNKNOWN;
    mD3DPP.BackBufferHeight = TEST_VIEW_DEFAULT_HEIGHT;
    mD3DPP.BackBufferWidth = TEST_VIEW_DEFAULT_WIDTH;
    mD3DPP.EnableAutoDepthStencil = TRUE;
    mD3DPP.AutoDepthStencilFormat = D3DFMT_D16;
    mD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    // Create the Direct3D device. Here we are using the default adapter (most
    // systems only have one, unless they have multiple graphics hardware cards
    // installed) and requesting the HAL (which is saying we want the hardware
    // device rather than a software one). Software vertex processing is 
    // specified since we know it will work on all cards. On cards that support 
    // hardware vertex processing, though, we would see a big performance gain 
    // by specifying hardware vertex processing.
    if( FAILED( m_D3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetSafeHwnd(),
        D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_FPU_PRESERVE,
        &mD3DPP, &m_D3D9Device ) ) )
    {
        return FALSE;
    }
    
    NBGM_RenderContextW32* renderContext = NBRE_NEW NBGM_RenderContextW32(this, NBGM_RS_D3D9);
    renderContext->AssignD3DDevice(m_D3D9, m_D3D9Device);
    NBGM_ResourceManagerConfig config;
    config.renderContext = shared_ptr<NBGM_RenderContextW32>(renderContext);
    NBGM_ResourceManager* resourceManager = NULL;
    PAL_Error err = NBGM_CreateResourceManager(&config, &resourceManager);
    if(err != PAL_Ok)
    {
        return FALSE;
    }

    m_ResourceManager = resourceManager;
    m_RenderPal = m_ResourceManager->GetResourceContext().renderPal;

    m_EnableRender = TRUE;

    return TRUE;
}

void CD3D9View::OnSize(int cx, int cy)
{
    if (m_D3D9Device)
    {
        //mD3DPP.BackBufferWidth  = cx;
        //mD3DPP.BackBufferHeight = cy;
        //m_D3D9Device->Reset(&mD3DPP);
    }
}

CString CD3D9View::GetInitFailedInfo()const
{
    return _T("Initialize Direct3D 9.0 failed, please make sure your graphic card support Direct3D 9.0 or above!");
}