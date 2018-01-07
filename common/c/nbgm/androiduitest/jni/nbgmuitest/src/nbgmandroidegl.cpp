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

#include "nbgmandroidegl.h"
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

NBGMAndroidEGL::NBGMAndroidEGL(ANativeWindow* window):
    m_window(window),
    m_display(EGL_NO_DISPLAY),
    m_surface(EGL_NO_SURFACE),
    m_context(EGL_NO_CONTEXT),
    m_config((EGLConfig)0)
{
}

NBGMAndroidEGL::~NBGMAndroidEGL()
{
    DestroySurface();
    Finish();
}

bool

NBGMAndroidEGL::Initilize(int renderSystem)
{
    uint32 errorLine = 0;
    int32* contextAttrs = NULL;
    EGLint numConfigs;
    ANativeWindow* window = m_window;
    m_window = NULL;

    EGLint config16bpp[] = {
        EGL_RENDERABLE_TYPE, renderSystem,
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    if(renderSystem == EGL_OPENGL_ES2_BIT)
    {
        contextAttrs = new int[3];
        contextAttrs[0] = EGL_CONTEXT_CLIENT_VERSION;
        contextAttrs[1] = 2;
        contextAttrs[2] = EGL_NONE;
    }

    if ((m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if (!eglInitialize(m_display, 0, 0))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if (!eglChooseConfig(m_display, config16bpp, &m_config, 1, &numConfigs))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if (!(m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttrs)))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    delete[] contextAttrs;

    CreateSurface(window);
    return true;

HandleError:
    Finish();
    return false;
}

bool
NBGMAndroidEGL::CreateSurface(ANativeWindow* window)
{
    uint32 errorLine;
    EGLint format;

    if(m_window == window)
    {
        return true;
    }
    m_window = window;

    if (!eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    ANativeWindow_setBuffersGeometry(m_window, 0, 0, format);

    if (!(m_surface = eglCreateWindowSurface(m_display, m_config, m_window, 0)))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    return true;

HandleError:
    DestroySurface();
    return false;
}

void
NBGMAndroidEGL::DestroySurface()
{
    if (m_display != EGL_NO_DISPLAY && m_surface != EGL_NO_SURFACE)
    {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
        m_window = NULL;
    }
}

bool
NBGMAndroidEGL::MakeCurrent()
{
    if(m_context != EGL_NO_CONTEXT)
	{
		if(eglGetCurrentContext() != m_context)
		{
			eglMakeCurrent(m_display, m_surface, m_surface, m_context);
		}
		return true;
	}
	return false;
}

bool
NBGMAndroidEGL::Swap()
{
	if (m_display != EGL_NO_DISPLAY && m_surface != EGL_NO_SURFACE)
	{
		return eglSwapBuffers(m_display, m_surface);
	}
	return false;
}

void
NBGMAndroidEGL::Finish()
{
	if (m_context != EGL_NO_CONTEXT)
	{
		eglDestroyContext(m_display, m_context);
		m_context = EGL_NO_CONTEXT;
	}

	if (m_display != EGL_NO_DISPLAY)
	{
		eglTerminate(m_display);
		m_display = EGL_NO_DISPLAY;
	}
}
