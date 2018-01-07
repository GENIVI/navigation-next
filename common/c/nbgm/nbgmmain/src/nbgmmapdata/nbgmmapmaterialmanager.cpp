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
#include "nbgmmapmaterialmanager.h"
#include "nbgmdom.h"
#include "nbgmmapmaterialfactory.h"
#include "nbreshadermanager.h"
#include "nbreishadergroup.h"
#include "nbrelog.h"
#include "nbgmcommon.h"
#include "nbgmconst.h"
#include "nbgmbuildutility.h"

NBRE_MapMateriaGroup::NBRE_MapMateriaGroup(const NBRE_String& name)
    :mName(name),
     mDefaultMaterial(NBRE_NEW NBRE_MapNoneMaterial()),
     mChunkFlag(0)
{
    mShaderGroup.reset(NBRE_NEW NBRE_DefaultShaderGroup());
}

NBRE_MapMateriaGroup::~NBRE_MapMateriaGroup()
{   
    for(NBRE_Map<uint32, NBRE_MapMaterial*>::iterator it = mMapMaterials.begin(); it != mMapMaterials.end(); ++it)
    {
        NBRE_DELETE it->second;
    }
    NBRE_DELETE mDefaultMaterial;
}

PAL_Error
NBRE_MapMateriaGroup::Serialize(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_IOStream& is, const NBRE_String& domName, uint32 nbmOffset, const NBRE_Vector<const NBRE_MapMateriaGroup*>& dependsGroup)
{
    NBRE_DOM* dom = NULL;
    PAL_Error err = NBRE_DOMCreateFromFile(&is, domName, nbmOffset, &dom);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize Error! err = %x, errline = %d", err, __LINE__);
        NBRE_DOMDestroy(dom);
        return err;
    }

    NBRE_DOM_LAYS* domLAYS = NULL;
    err = NBRE_DOMGetChunkById(dom, DOM_ID_LAYS, (NBRE_DOM_Chunk**)&domLAYS);
    if(err == PAL_Ok && domLAYS != NULL)
    {
        // Theme material which includes multiple MTRL chunk
        for (uint32 layerIndex = 0; layerIndex < domLAYS->count; ++layerIndex)
        {
            NBRE_DOM_Layer& layer = domLAYS->layers[layerIndex];
            if (layer.type == NBRE_DLT_Material)
            {
                NBRE_DOM_MTRL* domMTRL = NULL;
                err = NBRE_DOMGetChunk(dom, layer.chunkOffset, (NBRE_DOM_Chunk**)&domMTRL);
                if(err != PAL_Ok || domMTRL == NULL)
                {
                    NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize Error! err = %x, errline = %d", err, __LINE__);
                    NBRE_DOMDestroy(dom);
                    return err;
                }
                const NBRE_MapMateriaGroup* g = NULL;
                for (uint32 dIdx = 0; dIdx < dependsGroup.size(); ++dIdx)
                {
                    if (dependsGroup[dIdx]->mChunkFlag == domMTRL->flag)
                    {
                        g = dependsGroup[dIdx];
                    }
                }
                err = SerializeMTRL(palInstance, renderPal, textureManager, g, dom, domMTRL);
                if (layer.associateLayerIndex != INVALID_CHAR_INDEX)
                {
                    NBRE_DOM_Layer& drodLayer = domLAYS->layers[layer.associateLayerIndex];
                    if (drodLayer.type == NBRE_DLT_Associated)
                    {
                        NBRE_DOM_DROD* domDROD = NULL;
                        err = NBRE_DOMGetChunk(dom, drodLayer.chunkOffset, (NBRE_DOM_Chunk**)&domDROD);
                        if(err != PAL_Ok || domDROD == NULL)
                        {
                            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize Error! err = %x, errline = %d", err, __LINE__);
                            NBRE_DOMDestroy(dom);
                            return err;
                        }
                        
                        for (uint32 idxDrod = 0; idxDrod < domDROD->count; ++idxDrod)
                        {
                            NBRE_DOM_Drod& drod = domDROD->drod[idxDrod];
                            mDrawOrders[((uint32)domMTRL->flag << 16) + drod.materialIndex] = drod.drawOrder;
                        }
                    }
                    else
                    {
                        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize Error! err = %x, errline = %d", err, __LINE__);
                        NBRE_DOMDestroy(dom);
                        return err;
                    }
                }
            }
        }
    }
    else
    {
        // Old single material which includes one MTRL chunk
        NBRE_DOM_MTRL* domMTRL = NULL;
        err = NBRE_DOMGetChunkById(dom, DOM_ID_MTRL, (NBRE_DOM_Chunk**)&domMTRL);
        if(err != PAL_Ok || domMTRL == NULL)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize Error! err = %x, errline = %d", err, __LINE__);
            NBRE_DOMDestroy(dom);
            return err;
        }
        err = SerializeMTRL(palInstance, renderPal, textureManager, NULL, dom, domMTRL);
    }
    
    NBRE_DOMDestroy(dom);

    return err;
}

PAL_Error
NBRE_MapMateriaGroup::SerializeMTRL(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const NBRE_MapMateriaGroup* dependsGroup, NBRE_DOM* dom, NBRE_DOM_MTRL* domMTRL)
{
    PAL_Error err = PAL_Ok;
    NBRE_MapMaterialCreateContext context;
    uint32 chunkOffset = (uint32)domMTRL->flag << 16;
    for(uint16 i = 0; i<domMTRL->count; ++i)
    {
        uint32 matIndex = chunkOffset + i;
        NBRE_MapMaterial* mapMaterial = NULL;
        err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(palInstance, renderPal, textureManager, *dom, i, &context, TRUE, dependsGroup, domMTRL, &mapMaterial);
        if (err == PAL_Ok)
        {
            mMapMaterials[matIndex] = mapMaterial;
            for(int32 j = 0; j < mapMaterial->GetShaderCount(); ++j)
            {
                mShaderGroup->SetShader(mapMaterial->GetShaderName(j), mapMaterial->GetShader(j));
            }
        }
        else
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMateriaGroup::Serialize CreateMapMaterialFromDom Error! err = %x", err);
            mMapMaterials[matIndex] = NULL;
        }
    }

    if (domMTRL->flag == 0)
    {
        mShaderGroup->SetShader(VECTOR_OUTLINE_POLYLINE_TEXTURE_NAME,
            NBGM_BuildShaderUtility::CreatePolylineShader(renderPal, context.paletteImage));
        context.paletteImage = NULL;
    }
    else
    {
        mChunkFlag = domMTRL->flag;
    }

    return err;
}

PAL_Error
NBRE_MapMateriaGroup::Serialize(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_Vector<NBGM_MapMaterialImpl*>& materialTable, const NBRE_String& domName)
{
    PAL_Error err = PAL_Ok;
    nbre_assert(mMapMaterials.size() == 0);

    NBRE_MapMaterialCreateContext context;
    uint32 materialCount = materialTable.size();
    if (materialCount == 0)
    {
        return err;
    }

    NBRE_MapMaterial* mapMaterial = NULL;
    for(uint32 i = 0; i < materialCount; ++i)
    {
        mapMaterial = materialTable[i]->CreateNBREMaterial(palInstance, renderPal, textureManager, domName, TRUE);
        if (mapMaterial)
        {
            mMapMaterials[i] = mapMaterial;
            for(int32 j = 0; j < mapMaterial->GetShaderCount(); ++j)
            {
                mShaderGroup->SetShader(mapMaterial->GetShaderName(j), mapMaterial->GetShader(j));
            }
        }
        else
        {
            mMapMaterials[i] = NULL;
        }
    }
    mShaderGroup->SetShader(VECTOR_OUTLINE_POLYLINE_TEXTURE_NAME,
                            NBGM_BuildShaderUtility::CreatePolylineShader(renderPal, context.paletteImage));
    context.paletteImage = NULL;

    return err;
}

const NBRE_MapMaterial&
NBRE_MapMateriaGroup::GetMapMaterial(uint32 id) const
{
    return GetMapMaterial(0, id);
}

const NBRE_MapMaterial&
NBRE_MapMateriaGroup::GetMapMaterial(uint16 chunkFlag, uint32 id) const
{
    NBRE_Map<uint32, NBRE_MapMaterial*>::const_iterator it = mMapMaterials.find((((uint32)chunkFlag) << 16) +id);
	if(it != mMapMaterials.end())
	{
		return *(it->second);
	}
	return *mDefaultMaterial;
}

uint16
NBRE_MapMateriaGroup::GetMapMaterialPriority(uint16 chunkFlag, uint32 id) const
{
    NBRE_Map<uint32, uint16>::const_iterator it = mDrawOrders.find((((uint32)chunkFlag) << 16) +id);
    if(it != mDrawOrders.end())
    {
        return it->second;
    }
    return 0xFFFF;
}

const NBRE_Map<uint32, NBRE_MapMaterial*>&
NBRE_MapMateriaGroup::GetMapMaterials() const
{
    return mMapMaterials;
}

const NBRE_IShaderGroupPtr& 
NBRE_MapMateriaGroup::GetShaderGroup() const
{
    return mShaderGroup;
}

uint16
NBRE_MapMateriaGroup::GetChunkFlag() const
{
    return mChunkFlag;
}

NBRE_MapMaterialManager::NBRE_MapMaterialManager()
{
}

NBRE_MapMaterialManager::~NBRE_MapMaterialManager()
{
    NBRE_Map<NBRE_String, NBRE_MapMateriaGroup*>::iterator i = mLoadedMaterialGroup.begin();
    while(i != mLoadedMaterialGroup.end())
    {
        NBRE_DELETE i->second;
        ++i;
    }
}

NBRE_IShaderGroupPtr
NBRE_MapMaterialManager::AddGroup(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_IOStream& is, const NBRE_String& categoryName, const NBRE_String& materialName, const NBRE_Vector<NBRE_String>& dependsMaterial)
{
    NBRE_IShaderGroupPtr shaderGroup;
    NBRE_MapMateriaGroup* matGroup = NULL;
    const NBRE_String& groupID = GenerateID(categoryName, materialName);
    if(mLoadedMaterialGroup.find(groupID) == mLoadedMaterialGroup.end())
    {
        matGroup = NBRE_NEW NBRE_MapMateriaGroup(groupID);
        NBRE_Vector<const NBRE_MapMateriaGroup*> dependsGroup;
        for (uint32 i = 0; i < dependsMaterial.size(); ++i)
        {
            const NBRE_MapMateriaGroup* g = GetGroup(categoryName, dependsMaterial[i]);
            if (g != NULL)
            {
                dependsGroup.push_back(g);
            }
        }

        if(matGroup->Serialize(palInstance, renderPal, textureManager, is, groupID, 0, dependsGroup) == PAL_Ok)
        {
            mLoadedMaterialGroup[groupID] = matGroup;
            mCurrentMaterialGroup[categoryName] = matGroup;
            shaderGroup = matGroup->GetShaderGroup();
        }
        else
        {
            NBRE_DELETE matGroup;
        }
    }
    else
    {
        matGroup = mLoadedMaterialGroup[groupID];
        shaderGroup = matGroup->GetShaderGroup();
    }

    return shaderGroup;
}

const NBRE_MapMateriaGroup*
NBRE_MapMaterialManager::GetGroup(const NBRE_String& categoryName, const NBRE_String& materialName)
{
    const NBRE_String& groupID = GenerateID(categoryName, materialName);
    return mLoadedMaterialGroup[groupID];
}

NBRE_IShaderGroupPtr
NBRE_MapMaterialManager::AddGroup(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_Vector<NBGM_MapMaterialImpl*>& materialTable, const NBRE_String& categoryName, const NBRE_String& materialName)
{
    NBRE_IShaderGroupPtr shaderGroup;
    NBRE_MapMateriaGroup* matGroup = NULL;
    const NBRE_String& groupID = GenerateID(categoryName, materialName);
    if(mLoadedMaterialGroup.find(groupID) == mLoadedMaterialGroup.end())
    {
        matGroup = NBRE_NEW NBRE_MapMateriaGroup(groupID);
        if(matGroup->Serialize(palInstance, renderPal, textureManager, materialTable, groupID) == PAL_Ok)
        {
            mLoadedMaterialGroup[groupID] = matGroup;
            mCurrentMaterialGroup[categoryName] = matGroup;
            shaderGroup = matGroup->GetShaderGroup();
        }
        else
        {
            NBRE_DELETE matGroup;
        }
    }
    else
    {
        matGroup = mLoadedMaterialGroup[groupID];
        shaderGroup = matGroup->GetShaderGroup();
    }

    return shaderGroup;
}

PAL_Error
NBRE_MapMaterialManager::RemoveGroup(const NBRE_String& categoryName, const NBRE_String& materialName)
{
    const NBRE_String& groupID = GenerateID(categoryName, materialName);
    if(mLoadedMaterialGroup.find(groupID) == mLoadedMaterialGroup.end())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapMaterialManager::RemoveGroup, Group %s is not exsited!", groupID.c_str());
        return PAL_ErrNotFound;
    }
    else
    {
        if(mCurrentMaterialGroup[categoryName] == mLoadedMaterialGroup[groupID])
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapMaterialManager::RemoveGroup, Group %s is current material!", groupID.c_str());
            return PAL_ErrNotReady;
        }
        else
        {
            mLoadedMaterialGroup.erase(groupID);
        }
    }
    return PAL_Ok;
}

PAL_Error
NBRE_MapMaterialManager::SetCurrentGroup(const NBRE_String& categoryName, const NBRE_String& materialName)
{
    if(!categoryName.empty() && materialName.empty())
    {
        if(mCurrentMaterialGroup.find(categoryName) != mCurrentMaterialGroup.end())
        {
            mCurrentMaterialGroup[categoryName] = NULL;
        }
        return PAL_Ok;
    }
    const NBRE_String& groupID = GenerateID(categoryName, materialName);
    if(mLoadedMaterialGroup.find(groupID) == mLoadedMaterialGroup.end())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapMaterialManager::SetCurrentGroup, Group %s is not exsited!", groupID.c_str());
        return PAL_ErrNotFound;
    }
    else
    {
        mCurrentMaterialGroup[categoryName] = mLoadedMaterialGroup[groupID];
        return PAL_Ok;
    }
}

const NBRE_MapMateriaGroup&
NBRE_MapMaterialManager::GetCurrentGroup(const NBRE_String& categoryName)
{
    NBRE_MapMateriaGroup* materialGroup = NULL;
    if(mCurrentMaterialGroup.find(categoryName) == mCurrentMaterialGroup.end())
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMaterialManager::GetCurrentGroup categoryName: %s is not exsited!", categoryName.c_str());
        nbre_assert(FALSE);
    }
    else
    {
        materialGroup = mCurrentMaterialGroup[categoryName];
    }
    return *materialGroup;
}

nb_boolean
NBRE_MapMaterialManager::Ready() const
{
    return mCurrentMaterialGroup.empty() ? FALSE : TRUE;
}

nb_boolean
NBRE_MapMaterialManager::Ready(const NBRE_String& categoryName) const
{
    MaterialMap::const_iterator it = mCurrentMaterialGroup.find(categoryName);
    return (it != mCurrentMaterialGroup.end() && (it->second) != NULL) ? TRUE : FALSE;
}
