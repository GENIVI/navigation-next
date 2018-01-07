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

(C) Copyright 2014 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmcustomobjectscontext.h"
#include "nbretransformation.h"
#include "nbrerenderpal.h"
#include "nbretypeconvert.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderengine.h"
#include "nbgmprotected.h"
#include "nbgmconst.h"
#include "nbrememorystream.h"
#include "nbreutility.h"

NBGM_CustomObjectsContext::NBGM_CustomObjectsContext(NBGM_Context& nbgmContext)
    :mNBGMContext(nbgmContext)
{
}

NBGM_CustomObjectsContext::~NBGM_CustomObjectsContext()
{
}

bool
NBGM_CustomObjectsContext::AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData)
{
    if(textureData.empty())
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_CustomObjectsContext::AddTexture failed! TextureData is empty.");
        return false;
    }

    TextureMap::iterator it = mTextures.find(textureId);
    if(it != mTextures.end())
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_CustomObjectsContext::AddTexture failed! Texture already existed.");
        return false;
    }

    NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(textureData.addr, textureData.size, TRUE);
    if(memoryIO == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_CustomObjectsContext::AddTexture failed! Memeory is not enough.");
        return false;
    }

    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_PngTextureImage(*(mNBGMContext.renderingEngine->Context().mPalInstance), memoryIO, 0, FALSE);
    if(textureImage == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_CustomObjectsContext::AddTexture failed! Memeory is not enough.");
        return false;
    }

    NBRE_TexturePtr texture(mNBGMContext.renderingEngine->Context().mRenderPal->CreateTexture(textureImage, NBRE_Texture::TT_2D, TRUE, "") );
    if(!texture)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_CustomObjectsContext::AddTexture failed! Memeory is not enough.");
        return false;
    }

    mTextures.insert(TextureMap::value_type(textureId, texture));
    return true;
}

NBRE_TexturePtr
NBGM_CustomObjectsContext::GetTexture(NBGM_TextureId textureId)
{
    NBRE_TexturePtr texture;
    TextureMap::iterator it = mTextures.find(textureId);
    if(it != mTextures.end())
    {
        texture = it->second;
    }
    return texture;
}

void
NBGM_CustomObjectsContext::RemoveTexture(NBGM_TextureId textureId)
{
    TextureMap::iterator it = mTextures.find(textureId);
    if(it == mTextures.end())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_CustomObjectsContext::RemoveTexture failed! Cannot find this texture.");
        return;
    }
    mTextures.erase(it);
}

void
NBGM_CustomObjectsContext::RemoveAllTextures()
{
    mTextures.clear();
}

NBRE_MeshPtr
NBGM_CustomObjectsContext::GetMesh(NBGM_MeshType meshType)
{
    //1. First find the mesh by type from the cached map.
    NBRE_MeshPtr mesh;
    MeshMap::iterator it = mMeshes.find(meshType);
    if(it != mMeshes.end())
    {
        mesh = it->second;
        return mesh;
    }

    //2. If not find the the mesh, then try to create it.
    switch (meshType)
    {
    case NBGM_MT_UNIT_SQUAR2D:
        {
            mesh.reset(NBRE_Utility::CreateRect3D(mNBGMContext, 1.f, 0.f, 1.f));
            mMeshes.insert(MeshMap::value_type(meshType, mesh));
        }
        break;
    case NBGM_MT_UNIT_CIRCLE2D:
        //@todo: implement this if necessary
        break;
    case NBGM_MT_UNSOPPORTED:
        break;
    default:
        break;
    }
    return mesh;
}

void
NBGM_CustomObjectsContext::Clean()
{
    mMeshes.clear();
    mTextures.clear();
}
