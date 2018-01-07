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
 
 @file nbgmrendercontext.h
 */
/*
 (C) Copyright 2010 by Networks In Motion, Inc.
 
 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/
#ifndef _NBGM_RENDER_CONTEXT_H_
#define _NBGM_RENDER_CONTEXT_H_
#include "nbgmrendersystemtype.h"

/*!
 A render context is render resource holder, provided by a native view.
 User this context, you can initialize a EGL/D3D environment, controll 3D API driver(GLX/WGL/EGL/D3D)
 */
class NBGM_RenderContext
{
public:
    NBGM_RenderContext(){}
    virtual ~NBGM_RenderContext(){}

public:
    virtual int GetRenderTarget() = 0;
    /*!
     Initialize the 3D API driver: create swap chain, create default frame buffer, etc.     
     @remarks
     For now, there is no parameter for this function, but in future, if needed, will add some 
     control to control depthe buffer value, stencil buffer, alpha buffer, etc.
     Called in render thread
     */
    virtual void Initialize() = 0;
    /*!
     Close this context. Clean up all render resource.
     @remarks
     Called in render thread
     */
    virtual void Finalize() = 0;
    /*!
     Prepare to render a frame
     */
    virtual bool BeginRender() = 0;
    /*!
     End a frame
     */
    virtual bool EndRender() = 0;

    /*!
     Reset render state to default. Including type, size, and the format of the swap chain
     */
    virtual void Reset() = 0;
    
    /*!
     Switch current native render context to this context
     */
    virtual bool SwitchTo() = 0;

    /*!
     Get render context attributes; The following attributes are valid:
     * WINDOW         The NativeWindowType target for rendering
     * VIEW           The EGLView object that is drawn into
     * GLCONTEXT      The GLESContext used for rendering
     * GL2CONTEXT     The GLES2Context used for rendering
     * D3D9DRIVER     The D3D9 driver
     * D3D9DEVICE     The D3D9 device
     * D3D11DEVICE    The D3D11 driver
     * RENDER_SYSTEM_TYPE The render system type
     */
    virtual const void* GetConfiguration(const char* name) = 0;
};
#endif
