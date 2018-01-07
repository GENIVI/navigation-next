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

#include "nbrerenderengine.h"
#include "nbrerenderpal.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretexturemanager.h"
#include "nbrefontmanager.h"
#include "nbrelog.h"
#include "nbrefreetypefont.h"
#include "nbreruntimeinfo.h"

#include "nbregles2renderwindow.h"

NBRE_RenderEngine::NBRE_RenderEngine(PAL_Instance* pal): mInited(FALSE), mRenderSurface(NULL)
{
    nsl_memset(&mContext, 0, sizeof(NBRE_Context));
    mContext.mPalInstance = pal;
}

NBRE_RenderEngine::~NBRE_RenderEngine()
{
    NBRE_DELETE mContext.mLastRenderState;
}

void NBRE_RenderEngine::SetRenderTarget(int id)
{
    NBRE_GLES2RenderWindow* rt = (NBRE_GLES2RenderWindow*)mContext.mRenderPal->GetDefaultRenderTarget();
    rt->SetRenderTarget(id);
}

void NBRE_RenderEngine::SetActiveRenderSurface(NBRE_RenderSurface* surface)
{
    mRenderSurface = surface;
    mRenderSurface->AttachTarget(mContext.mRenderPal->GetDefaultRenderTarget());
}

NBRE_RenderSurface* NBRE_RenderEngine::ActiveRenderSurface()
{
    return mRenderSurface;
}

void NBRE_RenderEngine::Render()
{
    NBRE_RenderSurfaceList& surfaceList = mContext.mSurfaceManager->CurrentRenderSurfaceList();
    surfaceList.clear();
    surfaceList.push_back(mRenderSurface);

    mRenderSurface->Update();

    for(NBRE_RenderSurfaceList::const_iterator pSurface = surfaceList.begin(); pSurface != surfaceList.end(); ++pSurface)
    {
        (*pSurface)->Render();
    }
}

NBRE_ShaderManager* NBRE_RenderEngine::ShaderManager()
{
    return mContext.mShaderManager;
}

NBRE_Context& NBRE_RenderEngine::Context()
{
    return mContext;
}

void NBRE_RenderEngine::SetRenderPal(NBRE_IRenderPal* renderer)
{
    if(renderer == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_RenderEngine::SetRenderPal: Must asign a renderer!");
        nbre_assert(FALSE);
    }
    NBRE_DELETE mContext.mRenderPal;
    mContext.mRenderPal = renderer;
}

void NBRE_RenderEngine::InitContext(const NBRE_RenderConfig& config)
{
    if(mContext.mRenderPal == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_RenderEngine::InitContext: Must asign a renderer!");
        nbre_assert(FALSE);
    }
    mContext.mSurfaceManager = NBRE_NEW NBRE_SurfaceManager(mContext);
    mContext.mDefaultPass = NBRE_NEW NBRE_DefaultPass();
    mContext.mShaderManager = NBRE_NEW NBRE_ShaderManager();
    mContext.mFontManager = NBRE_NEW NBRE_FontManager(&mContext, config.fontFolderPath);
    mContext.mDefaultTexture = CreateDefaultTexture();
}

void NBRE_RenderEngine::DeinitContext()
{
    NBRE_DELETE mContext.mShaderManager;
    mContext.mShaderManager = NULL;

    NBRE_DELETE mContext.mDefaultPass;
    mContext.mDefaultPass = NULL;

    NBRE_DELETE mContext.mFontManager;
    mContext.mFontManager = NULL;

    NBRE_DELETE mContext.mSurfaceManager;
    mContext.mSurfaceManager = NULL;

}

void NBRE_RenderEngine::Initialize(NBRE_IRenderPal* renderer, const NBRE_RenderConfig& config)
{
    if(mInited)
    {
        return;
    }
    //setup renderer if not NULL
    if(renderer != NULL)
    {
        mContext.mRenderPal = renderer;
    }

    InitContext(config);
    mInited = TRUE;
}

void NBRE_RenderEngine::Deinitialize()
{
    if(!mInited)
    {
        return;
    }

    DeinitContext();
    mInited = FALSE;
}

NBRE_TexturePtr NBRE_RenderEngine::CreateDefaultTexture()
{
    NBRE_IRenderPal &renderPal = *(mContext.mRenderPal);
    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];

    NBRE_Image *image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8);
    uint8 *buffer = image->GetImageData();
    buffer[0] = 255;
    buffer[1] = 255;
    buffer[2] = 255;
    buffer[3] = 255;

    images[0] = image;
    NBRE_TexturePtr texture(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "DefaultTexture"));
    return texture;
}
