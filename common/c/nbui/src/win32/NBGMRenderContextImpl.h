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
 
     @file nbgmrendercontextimpl.h
 
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

#pragma once
#include "nbgmrendercontext.h"
#include <Windows.h>

class OpenGLView;

/*!
     A render context is render resource holder, provided by a native view.
    User this context, you can initialize a EGL/D3D environment, controll 3D API driver(GLX/WGL/EGL/D3D)
 */
class NBGM_RenderContextImpl: public NBGM_RenderContext
{
public:
    NBGM_RenderContextImpl(HDC dc, NBGM_RenderSystemType renderSystemType);
    virtual ~NBGM_RenderContextImpl();

    /*
        See description of all the functions in "NBGMRenderContext.h"
     */

    virtual void Initialize();
    virtual void Finalize();

    virtual bool BeginRender();
    virtual bool EndRender();

    virtual void Reset();
    virtual bool SwitchTo();

    virtual const void* GetConfiguration(const char* name);

    virtual void EnterBackground(bool val);
private:
    HDC m_dc;
    NBGM_RenderSystemType m_renderSystemType;
    OpenGLView* m_OpenGLView;
    bool m_initialized;
};


