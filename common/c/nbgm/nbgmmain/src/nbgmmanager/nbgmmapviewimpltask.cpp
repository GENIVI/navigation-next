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

    @file nbgmmapviewimpltask.cpp
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

#include "nbgmmapviewimpltask.h"
#include "nbrerenderengine.h"
#include "nbgmconst.h"
#include "nbretexturemanager.h"
#include "nbreiostream.h"
#include "nbgmmapviewimpl.h"
#include "nbgmrendercontext.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmresourcemanager.h"
#include "nbgmbinloader.h"
#include "nbgmmapdataimpl.h"

static NBRE_String
ParseName(const NBRE_String& group)
{
    uint32 index = group.find_first_of('\\');
    if (index == NBRE_String::npos)
    {
        return "";
    }
    return NBRE_String(group, index + 1, group.length());
}

NBGM_LoadNBMTask::NBGM_LoadNBMTask(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue, nb_boolean isUpdate)
    :mTileInfo(tileInfo)
    ,mResourceManager(resourceManager)
    ,mListener(listener)
    ,mTaskQueue(taskQueue)
    ,mIsUpdate(isUpdate)
{
}

NBGM_LoadNBMTask::~NBGM_LoadNBMTask()
{
    for (uint32 i = 0; i < mTileInfo.size(); ++i)
    {
        if (mTileInfo[i].mapData)
        {
            NBRE_DELETE mTileInfo[i].mapData;
        }
    }
}

void
NBGM_LoadNBMTask::Execute()
{
    if(mResourceManager->GetResourceContext().mapMaterialManager->Ready())
    {
        PAL_Error err = PAL_Ok;
        for(NBRE_Vector<NBGM_NBMDataLoadInfo>::iterator iter = mTileInfo.begin(); iter != mTileInfo.end(); ++iter)
        {
            //generate the data that will be passed into render task
            NBGM_NBMData* nbmData = NULL;
            err = ParseNbmData(*iter, &nbmData);
            if(err == PAL_Ok)
            {
                NBGM_NBMDataLoadResult result(*iter);
                result.nbmData = shared_ptr<NBGM_NBMData>(nbmData);
                mResult.push_back(result);
            }
            else
            {
                // need to return the tile info which cannot be parsed to NBMData
                NBGM_NBMDataLoadResult result(*iter);
                mResult.push_back(result);
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_LoadNBMTask::Execute, error = 0x%08x, tile=%s", err, iter->id.c_str());
            }
        }
        if(mResult.size() > 0)
        {
            NBGM_SyncTask* syncTask = NBRE_NEW NBGM_SyncTask(mResult, mResourceManager, mListener, mIsUpdate);
            mTaskQueue->AddTask(syncTask);
        }

    }
    NBGM_Task::Execute();
}

PAL_Error
NBGM_LoadNBMTask::ParseNbmData(NBGM_NBMDataLoadInfo& tileInfo, NBGM_NBMData** nbmData)
{
    uint32 errLine = __LINE__;
    PAL_Error err = PAL_Ok;
    NBRE_DOM* dom = NULL;
    NBRE_Mapdata* mapData = NULL;
    NBGM_NBMData* result = NULL;

    if(nbmData == NULL)
    {
        err = PAL_ErrBadParam;
        errLine = __LINE__;
        goto HandleError;
    }

    if (!mResourceManager->GetResourceContext().mapMaterialManager->Ready(tileInfo.materialCategoryName))
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_LoadNBMTask::ParseNbmData() can't find material categrory %s for tile %s", tileInfo.materialCategoryName.c_str(), tileInfo.id.c_str());
        err = PAL_ErrBadParam;
        errLine = __LINE__;
        goto HandleError;
    }

    if (tileInfo.mapData == NULL)
    {
        err = NBRE_DOMCreateFromFile(tileInfo.stream.get(), tileInfo.id, NBGM_GetConfig()->nbmOffset, &dom);
        if(err != PAL_Ok || dom == NULL)
        {
            errLine = __LINE__;
            goto HandleError;
        }

        err = NBRE_MapdataCreate(mResourceManager->GetResourceContext(), dom, tileInfo.materialCategoryName, &mapData);
        if(err != PAL_Ok || mapData == NULL)
        {
            errLine = __LINE__;
            goto HandleError;
        }
    }
    else
    {
        // Create NBRE_MapData from NBGM_MapData
        err = ((NBGM_MapDataImpl*)tileInfo.mapData)->CreateNBREMapData(mResourceManager->GetResourceContext(), tileInfo.id, &mapData);
        if(err != PAL_Ok || mapData == NULL)
        {
            errLine = __LINE__;
            goto HandleError;
        }
    }

    if(err == PAL_Ok)
    {
        result = NBRE_NEW NBGM_NBMData(tileInfo.id, mapData->refCenter, tileInfo.groupName);
        result->LoadNBMData(mResourceManager->GetResourceContext(), mapData, tileInfo, mapData->flag);
    }

    *nbmData = result;
    NBRE_MapdataDestroy(mapData);
    NBRE_DOMDestroy(dom);
    mResourceManager->GetResourceContext().textureManager->RemoveAllTextures();
    NBRE_DebugLog(PAL_LogSeverityDebug,
                  "NBGM_LoadNBMTask::ParseNbmData( tileId=%s ) succeed, End",
                  tileInfo.id.c_str());
    return err;

HandleError:
    NBRE_MapdataDestoryInternalMaterial(mapData);
    NBRE_MapdataDestroy(mapData);
    NBRE_DOMDestroy(dom);
    mResourceManager->GetResourceContext().textureManager->RemoveAllTextures();
    NBRE_DebugLog(PAL_LogSeverityDebug,
                  "NBGM_LoadNBMTask::ParseNbmData( tileId=%s ) failed, error = 0x%08x, errorLine=%d",
                  tileInfo.id.c_str(), err, errLine);
    return err;
}

NBGM_LoadBINTask::NBGM_LoadBINTask(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue)
    :mTileInfo(tileInfo)
    ,mResourceManager(resourceManager)
    ,mListener(listener)
    ,mTaskQueue(taskQueue)
{
}

NBGM_LoadBINTask::~NBGM_LoadBINTask()
{
}

void
NBGM_LoadBINTask::Execute()
{
    if(mResourceManager->GetResourceContext().mapMaterialManager->Ready())
    {
        PAL_Error err = PAL_Ok;
        for(NBRE_Vector<NBGM_NBMDataLoadInfo>::const_iterator iter = mTileInfo.begin(); iter != mTileInfo.end(); ++iter)
        {
            //generate the data that will be passed into render task
            NBGM_NBMData* nbmData = NULL;
            err = ParseBinData(*iter, &nbmData);
            if(err == PAL_Ok)
            {
                NBGM_NBMDataLoadResult result(*iter);
                result.nbmData = shared_ptr<NBGM_NBMData>(nbmData);;
                mResult.push_back(result);
            }
            else
            {
                // need to return the tile info which cannot be parsed to NBMData
                NBGM_NBMDataLoadResult result(*iter);
                mResult.push_back(result);
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_LoadBINTask::Execute, error = 0x%08x, tile=%s", err, iter->id.c_str());
            }
        }
        if(mResult.size() > 0)
        {
            NBGM_SyncTask* syncTask = NBRE_NEW NBGM_SyncTask(mResult, mResourceManager, mListener);
            mTaskQueue->AddTask(syncTask);
        }

    }
    NBGM_Task::Execute();
}

PAL_Error
NBGM_LoadBINTask::ParseBinData(const NBGM_NBMDataLoadInfo& tileInfo, NBGM_NBMData** nbmData)
{
    if(nbmData == NULL)
    {
        return PAL_ErrBadParam;
    }
    NBGM_NBMData* result = NULL;
    NBRE_Vector<NBRE_ModelPtr>* models = NBRE_NEW NBRE_Vector<NBRE_ModelPtr>;
    PAL_Error err = NBRE_LoadSceneFromStream(*mResourceManager->GetResourceContext().renderPal, mResourceManager->GetResourceContext().palInstance,
        mResourceManager->GetResourceContext().GetModelScaleFactor(), tileInfo.stream.get(), NBGM_GetConfig()->binFileTexturePath, *models);
    if(err != PAL_Ok)
    {
        return err;
    }

    NBRE_Mapdata* mapData = NULL;
    err = MapdataCreate(mResourceManager->GetResourceContext(), models, &mapData);
    if(err != PAL_Ok || mapData == NULL)
    {
        return err;
    }

    result = NBRE_NEW NBGM_NBMData(tileInfo.id, mapData->refCenter, tileInfo.groupName);
    result->LoadNBMData(mResourceManager->GetResourceContext(), mapData, tileInfo, 0);
    *nbmData = result;
    return err;
}

PAL_Error
NBGM_LoadBINTask::MapdataCreate(NBGM_ResourceContext& /*resourceContext*/, NBRE_Vector<NBRE_ModelPtr>* models, NBRE_Mapdata** mapdata)
{
    //error handling
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    //result data
    NBRE_Mapdata* result = NULL;

    uint32 layerIndex = 0;

    if(mapdata == NULL)
    {
        err = PAL_ErrBadParam;
        errLine = __LINE__;
        goto HandleError;
    }

    //create result
    result = (NBRE_Mapdata*)nsl_malloc(sizeof(NBRE_Mapdata));
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_Mapdata));


    result->layerArray = NBRE_ArrayCreate(1);
    if(result->layerArray == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    //map reference center
    result->refCenter.x = 0;
    result->refCenter.y = 0;

    //for each layer
    for(layerIndex=0; layerIndex<1; ++layerIndex)
    {
        NBRE_Layer* layer = (NBRE_Layer*)nsl_malloc(sizeof(NBRE_Layer));
        if(layer == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(layer, 0, sizeof(NBRE_Layer));

        NBRE_LayerMESHData* meshData = (NBRE_LayerMESHData*)nsl_malloc(sizeof(NBRE_LayerMESHData));
        if (meshData == NULL)
        {
            nsl_free(layer);
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(meshData, 0, sizeof(NBRE_LayerMESHData));
        meshData->models = models;

        layer->data = (void **)nsl_malloc(sizeof(void*));
        layer->data[0] = meshData;
        layer->dataCount = 1;
        layer->info.layerDataType = NBRE_LDT_MESH;
        layer->info.layerType = NBRE_DLT_ECM;
        layer->info.nearVisibility = 0;
        layer->info.farVisibility = 10000;

        err = NBRE_ArrayAppend(result->layerArray, layer);
        if(err != PAL_Ok)
        {
            errLine = __LINE__;
            goto HandleError;
        }
    }
    *mapdata = result;
    return PAL_Ok;
HandleError:
    NBRE_MapdataDestoryInternalMaterial(result);
    NBRE_MapdataDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_LoadBINTask::MapdataCreate: err=0x%08x, errLine=%u", err, errLine);
    return err;
}

NBGM_SyncTask::NBGM_SyncTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, nb_boolean isUpdate)
:mResult(result)
,mResourceManager(resourceManager)
,mListener(listener)
,mIsUpdate(isUpdate)
{
}

NBGM_SyncTask::~NBGM_SyncTask()
{
}

static void
NotifyLoadFinished(const NBGM_NBMDataLoadResult& result)
{
    if(result.tileInfo.loadListener)
    {
        result.tileInfo.loadListener->OnLoadingFinished(result.tileInfo.id);
    }
}

void
NBGM_SyncTask::Execute()
{
    if(mIsUpdate)
    {
        mResourceManager->RecieveUpdateNBMDataFromTask(mResult, mListener);
    }
    else
    {
        mResourceManager->RecieveNBMDataFromTask(mResult, mListener);
    }
    for_each(mResult.begin(), mResult.end(), NotifyLoadFinished);
    NBGM_Task::Execute();
}

NBGM_ActiveNBMTask::NBGM_ActiveNBMTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue)
    :mResult(result)
    ,mListener(listener)
    ,mTaskQueue(taskQueue)
{
}

NBGM_ActiveNBMTask::~NBGM_ActiveNBMTask()
{
}

void
NBGM_ActiveNBMTask::Execute()
{
    if(mResult.size() > 0)
    {
        NBGM_SyncActiveNBMTask* syncTask = NBRE_NEW NBGM_SyncActiveNBMTask(mResult, mListener);
        mTaskQueue->AddTask(syncTask);
    }
    NBGM_Task::Execute();
}

NBGM_SyncActiveNBMTask::NBGM_SyncActiveNBMTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener)
    :mResult(result)
    ,mListener(listener)
{
}

NBGM_SyncActiveNBMTask::~NBGM_SyncActiveNBMTask()
{
}

void
NBGM_SyncActiveNBMTask::Execute()
{
    mListener->OnResponseNBMData(mResult);
    for_each(mResult.begin(), mResult.end(), NotifyLoadFinished);
    NBGM_Task::Execute();
}

NBGM_UnloadTask::NBGM_UnloadTask(NBGM_MapViewImpl* impl, const shared_ptr<NBGM_TaskQueue>& taskQueue, nb_boolean unloadAll)
    :mImpl(impl)
    ,mTaskQueue(taskQueue)
    ,mUnloadAll(unloadAll)
{
}

NBGM_UnloadTask::~NBGM_UnloadTask()
{
}

void
NBGM_UnloadTask::AssignUnloadIds(const NBRE_Vector<NBGM_VectorTileID>& unloadIds)
{
    mIds.assign(unloadIds.begin(), unloadIds.end());
}

void
NBGM_UnloadTask::Execute()
{
    NBGM_SyncUnloadTask* task = NBRE_NEW NBGM_SyncUnloadTask(mImpl, mUnloadAll);
    task->AssignUnloadIds(mIds);
    mTaskQueue->AddTask(task);
    NBGM_Task::Execute();
}

NBGM_SyncUnloadTask::NBGM_SyncUnloadTask(NBGM_MapViewImpl* impl, nb_boolean unloadAll)
    :mImpl(impl)
    ,mUnloadAll(unloadAll)
{
}

NBGM_SyncUnloadTask::~NBGM_SyncUnloadTask()
{
}

void
NBGM_SyncUnloadTask::AssignUnloadIds(const NBRE_Vector<NBGM_VectorTileID>& unloadIds)
{
    mIds.assign(unloadIds.begin(), unloadIds.end());
}

void
NBGM_SyncUnloadTask::Execute()
{
    if(mImpl)
    {
        if(mUnloadAll)
        {
            mImpl->UnloadSyncAllTiles();
        }
        else
        {
            mImpl->UnloadSyncTiles(mIds);
        }
    }
    NBGM_Task::Execute();
}

NBGM_LoadMaterialTask::NBGM_LoadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> stream, shared_ptr<NBGM_TaskQueue> renderTaskQueue, const NBRE_Vector<NBRE_String>& dependsMaterial)
    :mLoadingContext(loadingContext)
    ,mRenderContext(renderContext)
    ,mCatergoryName(catergoryName)
    ,mMaterialName(materialName)
    ,mStream(stream)
    ,mMaterialTable(NULL)
    ,mRenderTaskQueue(renderTaskQueue)
    ,mDependsMaterial(dependsMaterial)
{
}

NBGM_LoadMaterialTask::NBGM_LoadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, std::vector<NBGM_IMapMaterial*>& materialTable, shared_ptr<NBGM_TaskQueue> renderTaskQueue)
    :mLoadingContext(loadingContext)
    ,mRenderContext(renderContext)
    ,mCatergoryName(catergoryName)
    ,mMaterialName(materialName)
    ,mRenderTaskQueue(renderTaskQueue)
{
    mMaterialTable = NBRE_NEW NBRE_Vector<NBGM_MapMaterialImpl*>;
    for (uint32 i = 0; i < materialTable.size(); ++i)
    {
        NBGM_MapMaterialImpl* mi = (NBGM_MapMaterialImpl*)materialTable[i];
        // The material index is same as array index
        // This convention comes from NBM common material data structure
        mi->SetId((uint16)i);
        mMaterialTable->push_back(mi);
    }
}

NBGM_LoadMaterialTask::~NBGM_LoadMaterialTask()
{
    if (mMaterialTable)
    {
        uint32 n = mMaterialTable->size();
        for (uint32 i = 0; i < n; ++i)
        {
            NBRE_DELETE (*mMaterialTable)[i];
        }
        NBRE_DELETE mMaterialTable;
    }
}

void
NBGM_LoadMaterialTask::Execute()
{
    mLoadingContext.textureManager->RemoveAllTextures();
    NBRE_MapMaterialManager& materialMgr = *(mLoadingContext.mapMaterialManager);
    NBRE_IShaderGroupPtr shaderGroup;

    if (mMaterialTable == NULL)
    {
        shaderGroup = materialMgr.AddGroup(mLoadingContext.palInstance, *mLoadingContext.renderPal, *mLoadingContext.textureManager, *mStream, mCatergoryName, mMaterialName, mDependsMaterial);
    }
    else
    {
        shaderGroup = materialMgr.AddGroup(mLoadingContext.palInstance, *mLoadingContext.renderPal, *mLoadingContext.textureManager, *mMaterialTable, mCatergoryName, mMaterialName);
    }


    if(shaderGroup)
    {
        NBGM_LoadMaterialResult result;
        result.groupID = GenerateID(mCatergoryName, mMaterialName);
        result.shaderGroup = shaderGroup;
        NBGM_SyncMaterialTask* task = NBRE_NEW NBGM_SyncMaterialTask(mRenderContext, result);
        mRenderTaskQueue->AddTask(task);
    }

    mLoadingContext.textureManager->RemoveAllTextures();

    NBGM_Task::Execute();
}

NBGM_SyncMaterialTask::NBGM_SyncMaterialTask(NBGM_Context& renderContext, const NBGM_LoadMaterialResult& result)
    :mRenderContext(renderContext)
    ,mResult(result)
{
}

NBGM_SyncMaterialTask::~NBGM_SyncMaterialTask()
{
}

void
NBGM_SyncMaterialTask::Execute()
{
    mRenderContext.renderingEngine->Context().mShaderManager->AddGroup(mResult.groupID, mResult.shaderGroup);
    NBGM_Task::Execute();
}

NBGM_SeCurrenttMaterialTask::NBGM_SeCurrenttMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_MapViewImpl* mapViewImpl, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBGM_TaskQueue> renderTaskQueue, NBRE_Overlay* backgroundOverlay, NBRE_Set<uint32>* fontIds)
    :mLoadingContext(loadingContext)
    ,mRenderContext(renderContext)
    ,mCatergoryName(catergoryName)
    ,mMaterialName(materialName)
    ,mRenderTaskQueue(renderTaskQueue)
    ,mBackgroundOverlay(backgroundOverlay)
    ,mMapViewImpl(mapViewImpl)
    ,mFontIds(fontIds)
{
}

NBGM_SeCurrenttMaterialTask::~NBGM_SeCurrenttMaterialTask()
{
    NBRE_DELETE mFontIds;
}

void
NBGM_SeCurrenttMaterialTask::Execute()
{
    NBRE_MapMaterialManager& materialMgr = *(mLoadingContext.mapMaterialManager);
    NBGM_SetMaterialResult result;
    result.groupID = GenerateID(mCatergoryName, mMaterialName);
    if(!mCatergoryName.empty() && mMaterialName.empty())
    {
        materialMgr.SetCurrentGroup(mCatergoryName, mMaterialName);
        NBRE_Map<uint32, NBRE_FontMaterial>* fontMaterials = NBRE_NEW NBRE_Map<uint32, NBRE_FontMaterial>();
        NBGM_SyncCurrentMaterialTask* task = NBRE_NEW NBGM_SyncCurrentMaterialTask(mRenderContext, mMapViewImpl, result, mBackgroundOverlay, mCatergoryName, fontMaterials);
        mRenderTaskQueue->AddTask(task);
        NBGM_Task::Execute();
        return;
    }

    PAL_Error err = materialMgr.SetCurrentGroup(mCatergoryName, mMaterialName);

    if(err == PAL_Ok)
    {
        NBRE_Map<uint32, NBRE_MapMaterial*>::const_iterator pMaterial = materialMgr.GetCurrentGroup(mCatergoryName).GetMapMaterials().begin();
        NBRE_Map<uint32, NBRE_MapMaterial*>::const_iterator pMaterialEnd = materialMgr.GetCurrentGroup(mCatergoryName).GetMapMaterials().end();

        for (; pMaterial != pMaterialEnd; ++pMaterial)
        {
            if((pMaterial->second)->Type() == NBRE_MMT_BackgroundMaterial)
            {
                result.haveBackgroundColor = TRUE;
                result.backgroundColor = (static_cast<NBRE_MapBackgroundColorMaterial*>(pMaterial->second))->GetColor();
                break;
            }
        }

        NBRE_Map<uint32, NBRE_FontMaterial>* fontMaterials = NBRE_NEW NBRE_Map<uint32, NBRE_FontMaterial>();
        const NBRE_MapMateriaGroup& group = materialMgr.GetCurrentGroup(mCatergoryName);

        const NBRE_Map<uint32, NBRE_MapMaterial*>& mats = group.GetMapMaterials();
        for (NBRE_Map<uint32, NBRE_MapMaterial*>::const_iterator it = mats.begin(); it != mats.end(); ++it)
        {
            uint32 matId = it->first;
            const NBRE_MapMaterial& mat = *it->second;
            if (mat.Type() == NBRE_MMT_FontMaterial)
            {
                const NBRE_MapFontMaterial& fm = (const NBRE_MapFontMaterial&)mat;
                (*fontMaterials)[matId] = fm.GetFontMaterial();
            }
        }
        NBGM_SyncCurrentMaterialTask* task = NBRE_NEW NBGM_SyncCurrentMaterialTask(mRenderContext, mMapViewImpl, result, mBackgroundOverlay, mCatergoryName, fontMaterials);
        mRenderTaskQueue->AddTask(task);
    }


    NBGM_Task::Execute();
}

NBGM_SyncCurrentMaterialTask::NBGM_SyncCurrentMaterialTask(NBGM_Context& renderContext, NBGM_MapViewImpl* mapViewImpl, const NBGM_SetMaterialResult& result, NBRE_Overlay* backgroundOverlay, const NBRE_String& catergoryName, NBRE_Map<uint32, NBRE_FontMaterial>* fontMaterials)
    :mRenderContext(renderContext)
    ,mResult(result)
    ,mBackgroundOverlay(backgroundOverlay)
    ,mMapViewImpl(mapViewImpl)
    ,mCatergoryName(catergoryName)
    ,mFontMaterials(fontMaterials)
{
}

NBGM_SyncCurrentMaterialTask::~NBGM_SyncCurrentMaterialTask()
{
    NBRE_DELETE mFontMaterials;
}

void
NBGM_SyncCurrentMaterialTask::Execute()
{
    if(ParseName(mResult.groupID).empty())
    {
        mRenderContext.renderingEngine->ShaderManager()->SetCurrentGroup(mResult.groupID);
        NBGM_Task::Execute();
        return;
    }
    PAL_Error err = mRenderContext.renderingEngine->ShaderManager()->SetCurrentGroup(mResult.groupID);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_SyncCurrentMaterialTask::Execute:%s, err=0x%08x", mResult.groupID.c_str(), err);
    }

    if(mResult.haveBackgroundColor)
    {
        mBackgroundOverlay->SetClearColor(mResult.backgroundColor);
    }

    if(mMapViewImpl)
    {
        mMapViewImpl->RefreshFontMaterial(mCatergoryName, *mFontMaterials);
        mMapViewImpl->InvalidateLayout();
        mMapViewImpl->RequestRenderFrame();
    }

    NBGM_Task::Execute();
}

NBGM_UnloadMaterialTask::NBGM_UnloadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBGM_TaskQueue> renderTaskQueue)
    :mLoadingContext(loadingContext)
    ,mRenderContext(renderContext)
    ,mCatergoryName(catergoryName)
    ,mMaterialName(materialName)
    ,mRenderTaskQueue(renderTaskQueue)
{
}

NBGM_UnloadMaterialTask::~NBGM_UnloadMaterialTask()
{
}

void
NBGM_UnloadMaterialTask::Execute()
{
    NBRE_MapMaterialManager& materialMgr = *(mLoadingContext.mapMaterialManager);
    PAL_Error err = materialMgr.RemoveGroup(mCatergoryName, mMaterialName);
    if(err == PAL_Ok)
    {
        const NBRE_String& groupID = GenerateID(mCatergoryName, mMaterialName);
        NBGM_SyncUnloadedMaterialTask* task = NBRE_NEW NBGM_SyncUnloadedMaterialTask(mRenderContext, groupID);
        mRenderTaskQueue->AddTask(task);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_UnloadMaterialTask::Execute CatergoryName:%s MaterialName:%s, err=0x%08x", mCatergoryName.c_str(), mMaterialName.c_str(), err);
    }

    NBGM_Task::Execute();
}

NBGM_SyncUnloadedMaterialTask::NBGM_SyncUnloadedMaterialTask(NBGM_Context& renderContext, const NBRE_String& groupId)
    :mRenderContext(renderContext)
    ,mGroupId(groupId)
{
}

NBGM_SyncUnloadedMaterialTask::~NBGM_SyncUnloadedMaterialTask()
{
}

void
NBGM_SyncUnloadedMaterialTask::Execute()
{
    PAL_Error err = mRenderContext.renderingEngine->ShaderManager()->RemoveGroup(mGroupId);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_SyncUnloadedMaterialTask::Execute:%s, err=0x%08x", mGroupId.c_str(), err);
    }
}

NBGM_FinalizeTask::NBGM_FinalizeTask(PAL_Event* finalizeEvent)
:mFinalizeEvent(finalizeEvent)
{
}

NBGM_FinalizeTask::~NBGM_FinalizeTask()
{
}

void
NBGM_FinalizeTask::Execute()
{
    PAL_EventSet(mFinalizeEvent);
    NBGM_Task::Execute();
}

NBGM_RenderTask::NBGM_RenderTask(NBGM_MapViewImpl* mapViewImpl)
    :mMapViewImpl(mapViewImpl)
{
}

void
NBGM_RenderTask::Execute()
{
    mMapViewImpl->Render();
    NBGM_Task::Execute();
}

NBGM_InvalidateTask::NBGM_InvalidateTask(NBGM_MapViewImpl* mapViewImpl):
    mMapViewImpl(mapViewImpl)
{

}

NBGM_InvalidateTask::~NBGM_InvalidateTask()
{

}

void
NBGM_InvalidateTask::Execute()
{
    mMapViewImpl->RequestRenderFrame();

    NBGM_Task::Execute();
}

NBGM_SetAvatarLoacationTask::NBGM_SetAvatarLoacationTask(NBGM_MapViewImpl* mapViewImpl, const NBGM_Location64& location):
    mMapViewImpl(mapViewImpl),
    mLocation(location)
{

}

NBGM_SetAvatarLoacationTask::~NBGM_SetAvatarLoacationTask()
{

}

void
NBGM_SetAvatarLoacationTask::Execute()
{
    mMapViewImpl->SetAvatarLocation(mLocation);

    NBGM_Task::Execute();
}

NBGM_LoadingThreadScreenshotTask::NBGM_LoadingThreadScreenshotTask(shared_ptr<NBGM_TaskQueue> renderTaskQueue, NBGM_MapViewImpl* mapView):
    mRenderTaskQueue(renderTaskQueue), mMapViewImpl(mapView)
{
}

NBGM_LoadingThreadScreenshotTask::~NBGM_LoadingThreadScreenshotTask()
{
}

void
NBGM_LoadingThreadScreenshotTask::Execute()
{
    NBGM_RenderingThreadScreenshotTask* syncTask = NBRE_NEW NBGM_RenderingThreadScreenshotTask(mMapViewImpl);
    mRenderTaskQueue->AddTask(syncTask);
    NBGM_Task::Execute();
}

NBGM_RenderingThreadScreenshotTask::NBGM_RenderingThreadScreenshotTask(NBGM_MapViewImpl* mapView):
    mMapViewImpl(mapView)
{
}

NBGM_RenderingThreadScreenshotTask::~NBGM_RenderingThreadScreenshotTask()
{
}

void
NBGM_RenderingThreadScreenshotTask::Execute()
{
    mMapViewImpl->SnapScreenshot();
    NBGM_Task::Execute();
}

NBGM_LoadNBMTileTask::NBGM_LoadNBMTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info)
    :mMapViewImpl(mapView)
    ,mInfo(info)
{
}

NBGM_LoadNBMTileTask::~NBGM_LoadNBMTileTask()
{
}

void
NBGM_LoadNBMTileTask::Execute()
{
    mMapViewImpl->LoadNbmData(mInfo);
    NBGM_Task::Execute();
}

NBGM_LoadNBMTilesTask::NBGM_LoadNBMTilesTask(NBGM_MapViewImpl* mapView, const NBRE_Vector<NBGM_NBMDataLoadInfo>& infos)
    :mMapViewImpl(mapView)
    ,mInfos(infos)
{
}

NBGM_LoadNBMTilesTask::~NBGM_LoadNBMTilesTask()
{
}

void
NBGM_LoadNBMTilesTask::Execute()
{
    mMapViewImpl->LoadNbmData(mInfos);
    NBGM_Task::Execute();
}

NBGM_LoadBINTileTask::NBGM_LoadBINTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info)
    :mMapViewImpl(mapView)
    ,mInfo(info)
{
}

NBGM_LoadBINTileTask::~NBGM_LoadBINTileTask()
{
}

void
NBGM_LoadBINTileTask::Execute()
{
    mMapViewImpl->LoadBinData(mInfo);
    NBGM_Task::Execute();
}


NBGM_UnloadTileTask::NBGM_UnloadTileTask(NBGM_MapViewImpl* mapView, const NBRE_String& name)
    :mMapViewImpl(mapView)
    ,mName(name)
{
}

NBGM_UnloadTileTask::~NBGM_UnloadTileTask()
{
}

void
NBGM_UnloadTileTask::Execute()
{
    mMapViewImpl->UnLoadTile(mName);
    NBGM_Task::Execute();
}

NBGM_UnloadTilesTask::NBGM_UnloadTilesTask(NBGM_MapViewImpl* mapView, const NBRE_Vector<NBRE_String>& names)
    :mMapViewImpl(mapView)
    ,mNames(names)
{
}

NBGM_UnloadTilesTask::~NBGM_UnloadTilesTask()
{
}

void
NBGM_UnloadTilesTask::Execute()
{
    mMapViewImpl->UnLoadTile(mNames);
    NBGM_Task::Execute();
}

NBGM_UnloadALLTilesTask::NBGM_UnloadALLTilesTask(NBGM_MapViewImpl* mapView)
    :mMapViewImpl(mapView)
{
}

NBGM_UnloadALLTilesTask::~NBGM_UnloadALLTilesTask()
{
}

void
NBGM_UnloadALLTilesTask::Execute()
{
    mMapViewImpl->UnLoadAllTiles();
    NBGM_Task::Execute();
}

NBGM_ReloadTileTask::NBGM_ReloadTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info, const NBRE_String& unloadTileName)
    :mMapViewImpl(mapView)
    ,mInfo(info)
    ,mUnloadTileName(unloadTileName)
{
}

NBGM_ReloadTileTask::~NBGM_ReloadTileTask()
{
}

void
NBGM_ReloadTileTask::Execute()
{
    mMapViewImpl->ReloadNBMTile(mInfo, mUnloadTileName);
    NBGM_Task::Execute();
}

NBGM_UpdateTileTask::NBGM_UpdateTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info)
    :mMapViewImpl(mapView)
    ,mInfo(info)
{
}

NBGM_UpdateTileTask::~NBGM_UpdateTileTask()
{
}

void
NBGM_UpdateTileTask::Execute()
{
    mMapViewImpl->UpdateNBMTile(mInfo);
    NBGM_Task::Execute();
}

NBGM_RenderingThreadAsyncSnapScreenshotTask::NBGM_RenderingThreadAsyncSnapScreenshotTask(NBGM_MapViewImpl* mapView, const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot)
    :mMapViewImpl(mapView)
    ,mSnapScreenshot(snapScreenshot)
{
}

NBGM_RenderingThreadAsyncSnapScreenshotTask::~NBGM_RenderingThreadAsyncSnapScreenshotTask()
{
}

void
NBGM_RenderingThreadAsyncSnapScreenshotTask::Execute()
{
    mMapViewImpl->AsyncSnapScreenshot(mSnapScreenshot);
    NBGM_Task::Execute();
}
