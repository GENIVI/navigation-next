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

    @file nbgmmapmaterialmanager.h
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
#ifndef _NBRE_MAP_MATERIAL_MANAGER_H_
#define _NBRE_MAP_MATERIAL_MANAGER_H_
#include "nbretypes.h"
#include "nbgmmapmaterial.h"
#include "nbredefaultshadergroup.h"
#include "nbrecontext.h"
#include "nbgmmapdataimpl.h"

class NBRE_MapMateriaGroup
{
public:
    NBRE_MapMateriaGroup(const NBRE_String& name);
    ~NBRE_MapMateriaGroup();

public:
    PAL_Error Serialize(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_IOStream& is, const NBRE_String& domName, uint32 nbmOffset, const NBRE_Vector<const NBRE_MapMateriaGroup*>& dependsGroup);
    PAL_Error Serialize(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_Vector<NBGM_MapMaterialImpl*>& materialTable, const NBRE_String& domName);
    const NBRE_MapMaterial& GetMapMaterial(uint32 id) const;
    const NBRE_MapMaterial& GetMapMaterial(uint16 chunkFlag, uint32 id) const;
    uint16 GetMapMaterialPriority(uint16 chunkFlag, uint32 id) const;
    const NBRE_Map<uint32, NBRE_MapMaterial*>& GetMapMaterials() const;
    const NBRE_IShaderGroupPtr& GetShaderGroup() const;
    uint16 GetChunkFlag() const;

private:
    PAL_Error SerializeMTRL(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const NBRE_MapMateriaGroup* dependsGroup, NBRE_DOM* dom, NBRE_DOM_MTRL* domMTRL);

private:
    NBRE_String mName;
    NBRE_Map<uint32, NBRE_MapMaterial*> mMapMaterials;
    NBRE_Map<uint32, uint16> mDrawOrders;
    NBRE_MapMaterial* mDefaultMaterial;
    NBRE_IShaderGroupPtr mShaderGroup;
    uint16 mChunkFlag;
};

class NBRE_MapMaterialManager
{
public:
    NBRE_MapMaterialManager();
    ~NBRE_MapMaterialManager();

public:
    NBRE_IShaderGroupPtr AddGroup(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_IOStream& is, const NBRE_String& categoryName, const NBRE_String& materialName, const NBRE_Vector<NBRE_String>& dependsMaterial);
    NBRE_IShaderGroupPtr AddGroup(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_Vector<NBGM_MapMaterialImpl*>& materialTable, const NBRE_String& categoryName, const NBRE_String& materialName);
    PAL_Error RemoveGroup(const NBRE_String& categoryName, const NBRE_String& materialName);
    PAL_Error SetCurrentGroup(const NBRE_String& categoryName, const NBRE_String& materialName);

    const NBRE_MapMateriaGroup* GetGroup(const NBRE_String& categoryName, const NBRE_String& materialName);
    const NBRE_MapMateriaGroup& GetCurrentGroup(const NBRE_String& categoryName);

    nb_boolean Ready() const;
    nb_boolean Ready(const NBRE_String& categoryName) const;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_MapMaterialManager);

private:
    typedef  NBRE_Map<NBRE_String, NBRE_MapMateriaGroup*> MaterialMap;
    MaterialMap mLoadedMaterialGroup;//id---MaterialGroup
    MaterialMap mCurrentMaterialGroup;//category---MaterialGroup
};

#endif
