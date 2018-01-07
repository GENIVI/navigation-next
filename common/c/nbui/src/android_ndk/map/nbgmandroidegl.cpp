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
#include "palgl2.h"
#include <string>

#define MULTISAMPLE_MAX_COUNT           8

NBGMAndroidEGL::NBGMAndroidEGL(ANativeWindow* window, PAL_Instance *pal, const NBGM_RenderConfig &config):
    m_window(window),
    m_display(EGL_NO_DISPLAY),
    m_surface(EGL_NO_SURFACE),
    m_context(EGL_NO_CONTEXT),
    m_config((EGLConfig)0),
    m_newSurface(EGL_NO_SURFACE),
    m_format(0),
    m_hasSurface(false),
    m_lock(NULL),
    m_needResetSurface(false),
    m_renderConfig(config)
{
    PAL_LockCreate(pal, &m_lock);
}

NBGMAndroidEGL::~NBGMAndroidEGL()
{
    CleanSurface();
    Finish();
    PAL_LockDestroy(m_lock);
}

bool
NBGMAndroidEGL::Initilize(int renderSystem)
{
    uint32 errorLine = 0;
    int32 contextAttrs[] = {EGL_NONE, EGL_NONE, EGL_NONE};
    EGLint numConfigs;
    ANativeWindow* window = m_window;
    m_window = NULL;

    EGLint sampleCount = m_renderConfig.enableFullScreenAntiAliasing?MULTISAMPLE_MAX_COUNT:0;
    EGLint config16bpp[] = {
        EGL_RENDERABLE_TYPE, renderSystem,
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, m_renderConfig.enableFullScreenAntiAliasing?1:0,
        EGL_SAMPLES, sampleCount,
        EGL_NONE
    };

    if(renderSystem == EGL_OPENGL_ES2_BIT)
    {
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

    // Must check numConfigs in addition here, or else EGL will fail to initialize on Samsun SMG900V.
    // On most Anroid devices, eglCreateContext would return a null context if failed.
    // So we can detect this failure and recreate it.
    // But on Samsun SMG900V, eglCreateContext would return a non-null context if failed.
    while ((numConfigs == 0) || !(m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttrs)))
    {
        if(sampleCount == 0)
        {
            break;
        }
        sampleCount /= 2;
        config16bpp[17] = sampleCount;
        if(sampleCount < 2)
        {
            // Try to open FSAA, if failed, close it.
            sampleCount = 0;
            config16bpp[15] = 0;
            config16bpp[17] = 0;
        }
        eglChooseConfig(m_display, config16bpp, &m_config, 1, &numConfigs);
    }

    if(!m_context)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if(CreateSurface(window))
    {
        eglMakeCurrent(m_display, m_surface, m_surface, m_context);
        InitializeGLES2();
        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(m_display, m_surface);
        m_hasSurface = true;
        return true;
    }

HandleError:
    Finish();
    return false;
}

bool
NBGMAndroidEGL::CreateSurface(ANativeWindow* window)
{
    uint32 errorLine;

    if(m_window == window)
    {
        return true;
    }
    m_window = window;

    if (!eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &m_format))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    ANativeWindow_setBuffersGeometry(m_window, 0, 0, m_format);

    if (!(m_surface = eglCreateWindowSurface(m_display, m_config, m_window, 0)))
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    return true;

HandleError:
    CleanSurface();
    return false;
}

bool
NBGMAndroidEGL::MakeCurrent()
{
    if (m_context == EGL_NO_CONTEXT || m_display == EGL_NO_DISPLAY || m_surface == EGL_NO_SURFACE)
    {
        return false;
    }

    if(!m_hasSurface)
    {
        return false;
    }

    if(m_needResetSurface)
    {
        ResetSurface();
        m_needResetSurface = false;
    }

    bool ret = true;
    if(eglGetCurrentContext() != m_context || eglGetCurrentSurface(EGL_DRAW) != m_surface)
    {
        ret = (eglMakeCurrent(m_display, m_surface, m_surface, m_context) == EGL_TRUE);
    }
    return ret;
}

void
NBGMAndroidEGL::ResetSurface()
{
    bool changed = false;
    PAL_LockLock(m_lock);
    if(m_newSurface != EGL_NO_DISPLAY && m_newSurface != m_surface)
    {
        eglDestroySurface(m_display, m_surface);
        m_surface = m_newSurface;
        m_newSurface = EGL_NO_DISPLAY;
        changed = true;
    }
    PAL_LockUnlock(m_lock);
    if(changed)
    {
        eglMakeCurrent(m_display, m_surface, m_surface, m_context);
    }
}

bool
NBGMAndroidEGL::Swap()
{
    if (m_context == EGL_NO_CONTEXT || m_display == EGL_NO_DISPLAY || m_surface == EGL_NO_SURFACE)
    {
        return false;
    }

    if(!m_hasSurface || m_needResetSurface)
    {
        return false;
    }

    return (eglSwapBuffers(m_display, m_surface) == EGL_TRUE);
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

void
NBGMAndroidEGL::CleanSurface()
{
    if (m_display == EGL_NO_DISPLAY || m_surface != EGL_NO_SURFACE)
    {
        return;
    }
    
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(m_display, m_surface);
    m_surface = EGL_NO_SURFACE;
    m_window = NULL;
}

void
NBGMAndroidEGL::DestroySurface()
{
    if (m_display == EGL_NO_DISPLAY || m_surface == EGL_NO_SURFACE)
    {
        return;
    }

    m_hasSurface = false;
    PAL_LockLock(m_lock);
    if(m_newSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(m_display, m_newSurface);
        m_newSurface = EGL_NO_SURFACE;
    }
    PAL_LockUnlock(m_lock);
    m_window = NULL;
    m_needResetSurface = false;
}

void NBGMAndroidEGL::UpdateSurface(ANativeWindow* window)
{
    if (m_display == EGL_NO_DISPLAY || m_surface == EGL_NO_SURFACE || m_window == window)
    {
        return;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, m_format);
    PAL_LockLock(m_lock);
    m_newSurface = eglCreateWindowSurface(m_display, m_config, window, 0);
    PAL_LockUnlock(m_lock);
    m_window = window;
    m_hasSurface = true;
    m_needResetSurface = true;
}
