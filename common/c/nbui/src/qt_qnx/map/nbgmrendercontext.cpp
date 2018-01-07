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
 (C) Copyright 2012 by TeleCommunications Systems, Inc.
 
 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/
#include <stdio.h>

#include "nbgmrendercontextimpl.h"
#include "nbgmnativerendercontext.h"
#include "palstdlib.h"

NBGM_RenderContextImpl::NBGM_RenderContextImpl(NBGM_NativeRenderContext* nativeRenderContext, NBGM_RenderSystemType renderSystemType)
    :m_initialized(false),
     m_nativeContext(nativeRenderContext),
     m_renderSystemType(renderSystemType)
{
}


NBGM_RenderContextImpl::~NBGM_RenderContextImpl()
{
    delete m_nativeContext;
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::Initialize()
{
    if(m_initialized)
    {
        return;
    }
    if(m_nativeContext)
    {
        m_nativeContext->Initialize(m_renderSystemType);
        m_initialized = true;
    }
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::Finalize()
{
    if(m_initialized)
    {
        m_nativeContext->Shutdown();
    }
    m_initialized = false;
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::BeginRender()
{
    if(m_initialized)
    {
        m_nativeContext->BeginRender();
    }
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::EndRender()
{
    if(m_initialized)
    {
        m_nativeContext->EndRender();
    }
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::Reset()
{
    if(m_initialized)
    {
        m_nativeContext->Reset();
    }
}

/* See description of all the functions in "NBGMRenderContext.h" */
void
NBGM_RenderContextImpl::SwitchTo()
{
    if(m_initialized)
    {
        m_nativeContext->SwitchTo();
    }
}

/* See description of all the functions in "NBGMRenderContext.h" */
const void*
NBGM_RenderContextImpl::GetConfiguration(const char* name)
{
    if(nsl_strcmp(name, "RENDER_SYSTEM_TYPE") == 0)
    {
        return &m_renderSystemType;
    }

    return NULL;
}
