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
 @file     nbgmnativerendercontext.cpp
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/
#include "nbgmnativerendercontext.h"
#include "NBUIConfig.h"
#import "NBUI_GLESDrawContext.h"
#import "NBUI_GLESFrameBuffer.h"

NBGM_NativeRenderContext::NBGM_NativeRenderContext(UIView* uiView)
    :m_view(uiView),
    m_width(0),
    m_height(0),
    m_drawContext(nil),
    m_frameBuffer(nil)
{
}

NBGM_NativeRenderContext::~NBGM_NativeRenderContext()
{
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::Initialize(NBGM_RenderSystemType renderSystemType)
{
    EAGLRenderingAPI renderingAPI = kEAGLRenderingAPIOpenGLES1;
    switch (renderSystemType)
    {
        case NBGM_RS_GLES:
            renderingAPI = kEAGLRenderingAPIOpenGLES1;
            break;
        case NBGM_RS_GLES20:
            renderingAPI = kEAGLRenderingAPIOpenGLES2;
            break;
        default:
            break;
    }
    m_drawContext = [[NBUI_GLESDrawContext alloc] init:renderingAPI];
    if (!m_drawContext)
    {
        Shutdown();
        return;
    }
    CreateFrameBuffer();
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::Shutdown()
{
    ReleaseFrameBuffer();
    [m_drawContext release];
    m_drawContext = nil;
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::BeginRender()
{
    if ((m_width != m_view.bounds.size.width) || (m_height != m_view.bounds.size.height))
    {
        ReleaseFrameBuffer();
        CreateFrameBuffer();
    }

    [m_drawContext renderStart];
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::EndRender()
{
    [m_drawContext renderEnd];
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::Reset()
{
    //do nothing now
}

/* See nbgmrendercontext.h for description */
void
NBGM_NativeRenderContext::SwitchTo()
{
    [m_drawContext makeCurrent];
}

void
NBGM_NativeRenderContext::CreateFrameBuffer()
{
    m_width = m_view.frame.size.width;
    m_height = m_view.frame.size.height;

    m_frameBuffer = [[NBUI_GLESFrameBuffer alloc] init:m_drawContext
                                              withView:m_view
                                        withFrameScale:(int)[UIScreen mainScreen].scale];
    if (!m_frameBuffer)
    {
        Shutdown();
        return;
    }
    m_drawContext.currentFrameBuffer = m_frameBuffer;
}

void
NBGM_NativeRenderContext::ReleaseFrameBuffer()
{
    if (m_frameBuffer)
    {
        [m_frameBuffer release];
        m_frameBuffer = nil;
        m_drawContext.currentFrameBuffer = nil;
        m_width = 0;
        m_height = 0;
    }
}
