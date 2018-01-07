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
#include "OpenGLView.h"
#include "nbrecommon.h"
#include "nbregles2renderpal.h"
#include "nbgm.h"
#include "nbgmrendercontext32.h"
#include "palgl2.h"

unsigned char threeto8[8] = 
{
    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

unsigned char twoto8[4] = 
{
    0, 0x55, 0xaa, 0xff
};

unsigned char oneto8[2] = 
{
    0, 255
};

static int defaultOverride[13] = 
{
    0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

static PALETTEENTRY defaultPalEntry[20] = 
{
    { 0,   0,   0,    0 },
    { 0x80,0,   0,    0 },
    { 0,   0x80,0,    0 },
    { 0x80,0x80,0,    0 },
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 },

    { 192, 220, 192,  0 },
    { 166, 202, 240,  0 },
    { 255, 251, 240,  0 },
    { 160, 160, 164,  0 },

    { 0x80,0x80,0x80, 0 },
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }
};

COpenGLView::COpenGLView(CNBGMUITestSuit *suit):CNBGMTestView(suit),m_pOldPalette(NULL),m_GLRC(NULL),m_pDC(NULL)
{
}

COpenGLView::~COpenGLView()
{
}

void COpenGLView::CreateRGBPalette()
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;

    n = ::GetPixelFormat(m_pDC->GetSafeHdc());
    ::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE) new char[sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY)];

        ASSERT(pPal != NULL);

        pPal->palVersion = 0x300;
        pPal->palNumEntries = (WORD)n;
        for (i=0; i<n; i++)
        {
            pPal->palPalEntry[i].peRed =
                ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal->palPalEntry[i].peGreen =
                ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal->palPalEntry[i].peBlue =
                ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal->palPalEntry[i].peFlags = 0;
        }

        /* fix up the palette to include the default GDI palette */
        if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
            )
        {
            for (i = 1 ; i <= 12 ; i++)
                pPal->palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
        }

        m_cPalette.CreatePalette(pPal);
        NBRE_DELETE[] pPal;

        m_pOldPalette = m_pDC->SelectPalette(&m_cPalette, FALSE);
        m_pDC->RealizePalette();
    }
}

unsigned char COpenGLView::ComponentFromIndex(int i, UINT nbits, UINT shift)
{
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) 
    {

    case 1:
        val &= 0x1;
        return oneto8[val];
    case 2:
        val &= 0x3;
        return twoto8[val];
    case 3:
        val &= 0x7;
        return threeto8[val];

    default:
        return 0;
    }
}

BOOL COpenGLView::bSetupPixelFormat()
{
    static PIXELFORMATDESCRIPTOR pfd = 
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                              // version number
        PFD_DRAW_TO_WINDOW |            // support window
        PFD_SUPPORT_OPENGL |            // support OpenGL
        PFD_DOUBLEBUFFER,               // double buffered
        PFD_TYPE_RGBA,                  // RGBA type
        24,                             // 24-bit color depth
        0, 0, 0, 0, 0, 0,               // color bits ignored
        8,                              // 8 bit alpha buffer
        0,                              // shift bit ignored
        0,                              // no accumulation buffer
        0, 0, 0, 0,                     // accum bits ignored
        24,                             // 32-bit z-buffer
        8,                              // no stencil buffer
        0,                              // no auxiliary buffer
        PFD_MAIN_PLANE,                 // main layer
        0,                              // reserved
        0, 0, 0                         // layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
    {
        MessageBox(_T("ChoosePixelFormat failed"));
        return FALSE;
    }

    if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
    {
        MessageBox(_T("SetPixelFormat failed"));
        return FALSE;
    }

    return TRUE;
}

BOOL COpenGLView::InitDrawEnv()
{
    PIXELFORMATDESCRIPTOR pfd;
    int         n;

    m_pDC= new CClientDC(this);

    ASSERT(m_pDC != NULL);

    if (!bSetupPixelFormat())
        return FALSE;

    n = ::GetPixelFormat(m_pDC->GetSafeHdc());
    ::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

    CreateRGBPalette();

    m_GLRC = wglCreateContext(m_pDC->GetSafeHdc());
    wglMakeCurrent(m_pDC->GetSafeHdc(), m_GLRC);
    glewInit() ;
    // Get Pointers To The GL Functions
    glGenBuffers = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
    glBindBuffer = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
    glBufferData = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
    glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
//     #ifndef glRenderbufferStorageMultisampleEXT
//     glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXT)wglGetProcAddress("glRenderbufferStorageMultisampleEXT");
//     #endif
//     #ifndef glFramebufferTexture2DMultisampleEXT
//     glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXT)wglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
//     #endif
    NBGM_RenderContextW32* renderContext = NBRE_NEW NBGM_RenderContextW32(this, NBGM_RS_GLES20);
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

void COpenGLView::OnSize(int /*cx*/, int /*cy*/)
{
    wglMakeCurrent(m_pDC->GetSafeHdc(), m_GLRC);
}

CString COpenGLView::GetInitFailedInfo()const
{
    return _T("Initialize OpenGL 3.0 failed, please make sure your graphic card support OpenGL 3.0 or above!");
}

void COpenGLView::OnRenderEnd()
{
    SwapBuffers(m_pDC->GetSafeHdc());
    wglMakeCurrent(m_pDC->GetSafeHdc(), NULL);
}

void COpenGLView::OnRenderBegine()
{
    wglMakeCurrent(m_pDC->GetSafeHdc(), m_GLRC);
    if(m_EnableRender)
    {
        m_TestSuit->InitRenderState();
    }
}

void COpenGLView::PostNcDestroy()
{
    m_EnableRender = FALSE;
    NBRE_DELETE m_TestSuit;
    if(m_ResourceManager != NULL)
    {
        NBGM_DestroyResourceManager(&m_ResourceManager);
    }
    delete m_pOldPalette;
    wglDeleteContext(m_GLRC);
    delete m_pDC;

    CNBGMTestView::PostNcDestroy();
}
