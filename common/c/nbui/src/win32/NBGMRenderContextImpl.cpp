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
 
     @file nbgmrendercontextimpl.cpp
 
 */
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.
 
    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

#include "NBGMRenderContextImpl.h"
#include "openglview.h"
#include "palstdlib.h"

NBGM_RenderContextImpl::NBGM_RenderContextImpl(HDC dc, NBGM_RenderSystemType renderSystemType):
    m_dc(dc),
    m_renderSystemType(renderSystemType),
    m_OpenGLView(NULL)
{
}


NBGM_RenderContextImpl::~NBGM_RenderContextImpl()
{
}

void
NBGM_RenderContextImpl::Initialize()
{
    switch (m_renderSystemType)
    {
    case NBGM_RS_GL:
        m_OpenGLView = new OpenGLView(m_dc);
        m_OpenGLView->InitDrawEnv();
        break;
    case NBGM_RS_GLES:
    case NBGM_RS_GLES20:
    case NBGM_RS_D3D9:
    case NBGM_RS_D3D11:
    default:
        break;
    }
}

void
NBGM_RenderContextImpl::Finalize()
{
    delete m_OpenGLView;
    m_OpenGLView = NULL;
}

bool
NBGM_RenderContextImpl::BeginRender()
{
    m_OpenGLView->OnRenderBegine();
    return true;
}

bool
NBGM_RenderContextImpl::EndRender()
{
    m_OpenGLView->OnRenderEnd();
    return true;
}

void
NBGM_RenderContextImpl::Reset()
{
}

bool
NBGM_RenderContextImpl::SwitchTo()
{
    m_OpenGLView->OnRenderBegine();
    return true;
}

const void*
NBGM_RenderContextImpl::GetConfiguration(const char* name)
{

    /*!
     Get render context attributes; The following attributes are valid:
     * WINDOW         The NativeWindowType target for rendering
     * VIEW           The EGLView object that is drawn into
     * GLCONTEXT      The GLESContext used for rendering
     * GL2CONTEXT     The GLES2Context used for rendering
     * D3D9DEVICE     The D3D9 driver
     * D3D11DEVICE    The D3D11 driver
     */
    if(nsl_strcmp(name, "RENDER_SYSTEM_TYPE") == 0)
    {
        return &m_renderSystemType;
    }

    return NULL;
}

//@todo: please implement it in win32 platform
void
NBGM_RenderContextImpl::EnterBackground(bool val)
{
}
