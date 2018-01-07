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

    @file nbgmcustomobjectscontext.h
*/
/*
    (C) Copyright 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_CUSTOM_OBJECTS_CONTEXT_H_
#define _NBGM_CUSTOM_OBJECTS_CONTEXT_H_
#include "nbrevector3.h"
#include "nbremesh.h"
#include "nbreshader.h"
#include "nbgmcontext.h"
#include "nbgmcommon.h"


/*! \addtogroup NBGM_Service
*  @{
*/

enum NBGM_MeshType
{
    NBGM_MT_UNIT_SQUAR2D,
    NBGM_MT_UNIT_CIRCLE2D,
    NBGM_MT_CUSTOM = 1000,
    NBGM_MT_UNSOPPORTED
};

class NBGM_CustomObjectsContext
{
public:
    NBGM_CustomObjectsContext(NBGM_Context& nbgmContext);
    ~NBGM_CustomObjectsContext();

public:
    /// Add a texture
    bool AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData);
    /// Remove a texture
    void RemoveTexture(NBGM_TextureId textureId);
    /// Remove all textures
    void RemoveAllTextures();
    /// Get a texture
    NBRE_TexturePtr GetTexture(NBGM_TextureId textureId);
    /// Get a mesh
    NBRE_MeshPtr GetMesh(NBGM_MeshType meshType);
    /// Remove all cached textures & mesh
    void Clean();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_CustomObjectsContext);

private:
    NBGM_Context& mNBGMContext;
    typedef NBRE_Map<NBGM_TextureId, NBRE_TexturePtr> TextureMap;
    TextureMap  mTextures;
    typedef NBRE_Map<NBGM_MeshType, NBRE_MeshPtr> MeshMap;
    MeshMap mMeshes;
};
typedef shared_ptr<NBGM_CustomObjectsContext> NBGM_CustomObjectsContextPtr;

/*! @} */
#endif
