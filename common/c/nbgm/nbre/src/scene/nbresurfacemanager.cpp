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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbresurfacemanager.h"
#include "nbrerendertarget.h"

NBRE_SurfaceManager::NBRE_SurfaceManager(NBRE_Context& context)
    :mContext(context)
    ,mCanDetach(TRUE)
{
}

NBRE_SurfaceManager::~NBRE_SurfaceManager()
{
    NBRE_RenderSurfaceList::iterator pSurface = mSurfaceList.begin();
    for(; pSurface != mSurfaceList.end(); ++pSurface)
    {
        NBRE_DELETE *pSurface;
    }
}

NBRE_RenderSurfaceList&
NBRE_SurfaceManager::CurrentRenderSurfaceList()
{
    return mCurrentSurfaceList;
}

void
NBRE_SurfaceManager::RemoveTexture(NBRE_Texture* texture)
{
    TextureRenderTargetMap::iterator iter = mTextureTargetMap.find(texture);
    if(iter != mTextureTargetMap.end())
    {
        for(FaceRenderTargetMap::iterator pFaceRenderTarget = iter->second.begin(); pFaceRenderTarget != iter->second.end(); ++pFaceRenderTarget)
        {
            RemoveRenderTarget(pFaceRenderTarget->second);
            NBRE_DELETE pFaceRenderTarget->second;
        }
        mTextureTargetMap.erase(iter);
    }
}

void
NBRE_SurfaceManager::RemoveRenderTarget(NBRE_RenderTarget* renderTarget)
{
    mCanDetach = FALSE;
    TargetSurfaceMap::iterator iter = mTargetSurfaceMap.find(renderTarget);
    if(iter != mTargetSurfaceMap.end())
    {
        for(SurfaceSet::iterator pSurface = iter->second.begin(); pSurface != iter->second.end(); ++pSurface)
        {
            (*pSurface)->AttachTarget(NULL);
        }
        mTargetSurfaceMap.erase(iter);
    }
    mCanDetach = TRUE;
}

NBRE_RenderTarget*
NBRE_SurfaceManager::GetRenderTargetFromTexture(NBRE_Texture* texture,
                                                int32 face,
                                                int32 mipmap,
                                                nb_boolean useDepthTexture)
{
    TextureRenderTargetMap::iterator iter = mTextureTargetMap.find(texture);
    if(iter != mTextureTargetMap.end())
    {
        FaceRenderTargetMap::iterator pFaceRenderTarget = iter->second.find(FaceMipmapKey(face, mipmap));
        if(pFaceRenderTarget != iter->second.end())
        {
            return pFaceRenderTarget->second;
        }
    }
    texture->Load();
    NBRE_RenderTarget* result = mContext.mRenderPal->CreateRenderTargetFromTexture(texture, face, useDepthTexture);
    texture->SetEventListener(this);
    mTextureTargetMap[texture][FaceMipmapKey(face, mipmap)] = result;
    return result;
}

void
NBRE_SurfaceManager::SurfaceAttachToRenderTarget(NBRE_RenderSurface* surface, NBRE_RenderTarget* renderTarget)
{
    nbre_assert(surface && renderTarget);
    mTargetSurfaceMap[renderTarget].insert(surface);
}

void
NBRE_SurfaceManager::SurfaceDetachRenderTarget(NBRE_RenderSurface* surface, NBRE_RenderTarget* renderTarget)
{
    if(mCanDetach)
    {
        TargetSurfaceMap::iterator iter = mTargetSurfaceMap.find(renderTarget);
        if(iter != mTargetSurfaceMap.end())
        {
            iter->second.erase(surface);
        }
    }
}

void
NBRE_SurfaceManager::UpdateSurfaceList(NBRE_RenderSurface* surface)
{
    for(NBRE_RenderSurfaceList::iterator pSurface = mCurrentSurfaceList.begin(); pSurface != mCurrentSurfaceList.end(); ++pSurface)
    {
        if(surface == *pSurface)
        {
            return;
        }
    }
    mCurrentSurfaceList.push_front(surface);
    surface->Update();
}

NBRE_RenderSurface*
NBRE_SurfaceManager::CreateSurface()
{
    NBRE_RenderSurface* surface = NBRE_NEW NBRE_RenderSurface(mContext);
    mSurfaceList.push_back(surface);
    return surface;
}

void
NBRE_SurfaceManager::OnDestroyed(NBRE_Texture* texture)
{
    RemoveTexture(texture);
}

void
NBRE_SurfaceManager::SurfaceSwitchRenderTarget(NBRE_RenderTarget* oldTarget, NBRE_RenderTarget* newTarget)
{
    if(!oldTarget || !newTarget)
    {
        return;
    }

    mCanDetach = FALSE;
    TargetSurfaceMap::iterator iter = mTargetSurfaceMap.find(oldTarget);
    if(iter != mTargetSurfaceMap.end())
    {
        SurfaceSet& surfaceSet = iter->second;
        SurfaceSet newSurfaceSet = surfaceSet;

        mTargetSurfaceMap.erase(iter);
        mTargetSurfaceMap[newTarget] = newSurfaceSet;
    }
    mCanDetach = TRUE;
}
