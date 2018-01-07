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

    @file nbresurfacemanager.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_SURFACE_MANAGER_H_
#define _NBRE_SURFACE_MANAGER_H_
#include "nbrerendersurface.h"
#include "nbretexture.h"
#include "nbrecontext.h"

class NBRE_RenderTarget;

/** \addtogroup NBRE_Scene
*  @{
*/

class NBRE_SurfaceManager: public NBRE_ITextureEventListener
{

public:
    NBRE_SurfaceManager(NBRE_Context& context);
    virtual ~NBRE_SurfaceManager();

public:
    NBRE_RenderSurface* CreateSurface();

    NBRE_RenderSurfaceList& CurrentRenderSurfaceList();
    void UpdateSurfaceList(NBRE_RenderSurface* surface);

    NBRE_RenderTarget* GetRenderTargetFromTexture(NBRE_Texture* texture, int32 face, int32 mipmap, nb_boolean useDepthTexture);

    void SurfaceAttachToRenderTarget(NBRE_RenderSurface* surface, NBRE_RenderTarget* renderTarget);
    void SurfaceDetachRenderTarget(NBRE_RenderSurface* surface, NBRE_RenderTarget* renderTarget);
    void SurfaceSwitchRenderTarget(NBRE_RenderTarget* oldTarget, NBRE_RenderTarget* newTarget);
    void RemoveTexture(NBRE_Texture* texture);

public:
    ///Derive From NBRE_ITextureEventListener
    virtual void OnDestroyed(NBRE_Texture* texture);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_SurfaceManager);

private:
    void RemoveRenderTarget(NBRE_RenderTarget* renderTarget);

private:
    typedef NBRE_Pair<int32, int32> FaceMipmapKey;
    typedef NBRE_Map<FaceMipmapKey, NBRE_RenderTarget*> FaceRenderTargetMap;
    typedef NBRE_Map<NBRE_Texture*, FaceRenderTargetMap> TextureRenderTargetMap;

    typedef NBRE_Set<NBRE_RenderSurface*> SurfaceSet;
    typedef NBRE_Map<NBRE_RenderTarget*, SurfaceSet> TargetSurfaceMap;

    NBRE_Context& mContext;
    TextureRenderTargetMap mTextureTargetMap;
    TargetSurfaceMap mTargetSurfaceMap;  
    NBRE_RenderSurfaceList mCurrentSurfaceList;

    NBRE_RenderSurfaceList mSurfaceList;

    nb_boolean mCanDetach;
};



/** @} */
#endif
