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

#ifndef _NBGM_ANDROID_RENDER_CONTEXT_H_
#define _NBGM_ANDROID_RENDER_CONTEXT_H_
#include "nbgmrendercontext.h"

class NBGMAndroidEGL;

class NBGM_AndroidRenderContext : public NBGM_RenderContext
{
public:
	NBGM_AndroidRenderContext(NBGMAndroidEGL* egl);
    virtual ~NBGM_AndroidRenderContext();

private:
    /*!
     Initialize the 3D API driver: create swap chain, create default frame buffer, etc.
     @remarks
     For now, there is no parameter for this function, but in future, if needed, will add some
     control to control depthe buffer value, stencil buffer, alpha buffer, etc.
     Called in render thread
     */
    virtual void Initialize(NBGM_RenderSystemType renderSystemType);
    /*!
     Close this context. Clean up all render resource.
     @remarks
     Called in render thread
     */
    virtual void Finalize();
    /*!
     Prepare to render a frame
     */
    virtual void BeginRender();
    /*!
     End a frame
     */
    virtual void EndRender();

    /*!
     Reset render state to default. Including type, size, and the format of the swap chain
     */
    virtual void Reset();

    /*!
     Switch current native render context to this context
     */
    virtual void SwitchTo();

    /*!
     Get render context attributes; The following attributes are valid:
     * WINDOW         The NativeWindowType target for rendering
     * VIEW           The EGLView object that is drawn into
     * GLCONTEXT      The GLESContext used for rendering
     * GL2CONTEXT     The GLES2Context used for rendering
     * D3D9DEVICE     The D3D9 driver
     * D3D11DEVICE    The D3D11 driver
     */
    virtual const void* GetConfiguration(const char* name);

private:
    NBGMAndroidEGL* mEGL;
};
#endif
