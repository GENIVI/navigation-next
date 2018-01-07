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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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
#include "nbgmrendercontext32.h"
#include "paltypes.h"
#include "palstdlib.h"

NBGM_RenderContextW32::NBGM_RenderContextW32(CNBGMTestView* nbgmNativeView, const NBGM_RenderSystemType& renderSystemType)
:mNBGMNativeView(nbgmNativeView)
,mRenderSystemType(renderSystemType)
,mD3D9(NULL)
,mD3D9Device(NULL)
{

}

NBGM_RenderContextW32::~NBGM_RenderContextW32()
{

}

void NBGM_RenderContextW32::AssignD3DDevice(IDirect3D9* d3D9, IDirect3DDevice9* d3D9Device)
{
    mD3D9 = d3D9;
    mD3D9Device = d3D9Device;
}

void NBGM_RenderContextW32::Finalize()
{

}

void NBGM_RenderContextW32::Initialize()
{

}

bool NBGM_RenderContextW32::EndRender()
{
    if (mNBGMNativeView)
    {
        mNBGMNativeView->OnRenderEnd();
        return true;
    }
    return false;

}

const void* NBGM_RenderContextW32::GetConfiguration( const char* name )
{
    if(nsl_strcmp(name, "RENDER_SYSTEM_TYPE") == 0)
    {
        return &mRenderSystemType;
    }
    if(nsl_strcmp(name, "D3D9DRIVER") == 0)
    {
        return mD3D9;
    }
    if(nsl_strcmp(name, "D3D9DEVICE") == 0)
    {
        return mD3D9Device;
    }
    return NULL;
}

bool NBGM_RenderContextW32::BeginRender()
{
    if (mNBGMNativeView)
    {
        mNBGMNativeView->OnRenderBegine();
        return true;
    }
    return false;
}

void NBGM_RenderContextW32::Reset()
{

}

bool NBGM_RenderContextW32::SwitchTo()
{
    if (mNBGMNativeView)
    {
        mNBGMNativeView->OnRenderBegine();
        return true;
    }
    return false;
}
