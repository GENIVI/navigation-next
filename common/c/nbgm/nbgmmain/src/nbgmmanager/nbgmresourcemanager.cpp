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
#include "nbgmresourcemanager.h"
#include "nbreutility.h"
#include "palclock.h"
#include "nbrefilelogger.h"
#include "nbgmconst.h"
#include "nbgm.h"
#include "nbgmprotected.h"
#include "nbgmmapviewimpltask.h"
#include "nbgmrendercontext.h"

NBGM_ResourceManager::NBGM_ResourceManager(const NBGM_ResourceManagerConfig& config, PAL_Instance* palInstance)
    :mConfig(config)
{
    mConfig.renderContext->Initialize();
    mConfig.renderContext->SwitchTo();
    if(NBGM_CreateRenderPal(&mConfig, palInstance, &mResourceContext.renderPal) == PAL_Ok)
    {
        mResourceContext.palInstance = palInstance;
        mResourceContext.buildModelContext = NBRE_NEW NBGM_BuildModelContext();
        mResourceContext.mapMaterialManager = NBRE_NEW NBRE_MapMaterialManager();
        mResourceContext.textureManager = NBRE_NEW NBRE_TextureManager(mResourceContext.renderPal);
        mResourceContext.SetModelScaleFactor(VECTOR_MAP_MODEL_SCALE_FACTOR);
    }
    else
    {
        nbre_assert(mResourceContext.renderPal);
    }

    uint32 now = PAL_ClockGetTimeMs();
    NBRE_String loadingLog = NBRE_Utility::FormatString("logs/NBGM_LoadingLog_%d.csv", now);
    mResourceContext.fileLogger = NBRE_NEW NBRE_FileLogger(palInstance, loadingLog.c_str());

    Initialize();
}

NBGM_ResourceManager::~NBGM_ResourceManager()
{
    mConfig.renderContext->SwitchTo();
    NBRE_DELETE mResourceContext.buildModelContext;
    NBRE_DELETE mResourceContext.mapMaterialManager;
    NBRE_DELETE mResourceContext.textureManager;
    NBRE_DELETE mResourceContext.fileLogger;
    NBRE_DELETE mResourceContext.renderPal;
    mNBMDataMap.clear();
    mConfig.renderContext->Finalize();
}

void
NBGM_ResourceManager::Initialize()
{
    std::string* threadId = NBRE_NEW std::string(NBRE_Utility::FormatString("NBGM_loading_%p", this));
    WorkerTaskQueue* loadThread = NBRE_NEW WorkerTaskQueue(mResourceContext.palInstance, shared_ptr<std::string>(threadId));
    mLoadingThread.reset(loadThread);
}

void
NBGM_ResourceManager::RequestNBMData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue)
{
    NBRE_Vector<NBGM_NBMDataLoadResult> result;
    NBRE_Vector<NBGM_NBMDataLoadInfo> loadTile;
    for(NBRE_Vector<NBGM_NBMDataLoadInfo>::const_iterator iter = tileInfo.begin(); iter != tileInfo.end(); ++iter)
    {
        NBGM_NBMDataLoadInfo info = *iter;
        NBMDataMap::iterator nbmDataIter = mNBMDataMap.find(info.id);
        if(nbmDataIter != mNBMDataMap.end())
        {
            NBGM_NBMDataLoadResult r(info);
            r.nbmData = nbmDataIter->second;
            result.push_back(r);
        }
        else
        {
            if(info.stream)
            {
                loadTile.push_back(info);
            }
        }
    }

    if(result.size() > 0 && listener != NULL)
    {
        NBGM_ActiveNBMTask* task = NBRE_NEW NBGM_ActiveNBMTask(result, listener, currentTaskQueue);
        mLoadingThread->AddTask(task);
    }

    if(loadTile.size() > 0 && listener != NULL)
    {
        NBGM_LoadNBMTask* task = NBRE_NEW NBGM_LoadNBMTask(loadTile, this, listener, currentTaskQueue);
        mLoadingThread->AddTask(task);
    }
}

void
NBGM_ResourceManager::RequestBINData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue)
{
    NBRE_Vector<NBGM_NBMDataLoadResult> result;
    NBRE_Vector<NBGM_NBMDataLoadInfo> loadTile;
    for(NBRE_Vector<NBGM_NBMDataLoadInfo>::const_iterator iter = tileInfo.begin(); iter != tileInfo.end(); ++iter)
    {
        NBGM_NBMDataLoadInfo info = *iter;
        NBMDataMap::iterator nbmDataIter = mNBMDataMap.find(info.id);
        if(nbmDataIter != mNBMDataMap.end())
        {
            NBGM_NBMDataLoadResult r(info);
            r.nbmData = nbmDataIter->second;
            result.push_back(r);
        }
        else
        {
            loadTile.push_back(info);
        }
    }

    if(result.size() > 0 && listener != NULL)
    {
        NBGM_ActiveNBMTask* task = NBRE_NEW NBGM_ActiveNBMTask(result, listener, currentTaskQueue);
        mLoadingThread->AddTask(task);
    }

    if(loadTile.size() > 0 && listener != NULL)
    {
        NBGM_LoadBINTask* task = NBRE_NEW NBGM_LoadBINTask(loadTile, this, listener, currentTaskQueue);
        mLoadingThread->AddTask(task);
    }
}

void
NBGM_ResourceManager::RequestUpdateNBMData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& currentTaskQueue)
{
    if(tileInfo.size() > 0 && listener != NULL)
    {
        NBGM_LoadNBMTask* task = NBRE_NEW NBGM_LoadNBMTask(tileInfo, this, listener, currentTaskQueue, TRUE);
        mLoadingThread->AddTask(task);
    }
}

void
NBGM_ResourceManager::ReleaseData(const NBGM_NBMDataId& nbmDataId)
{
    if(mNBMDataMap.find(nbmDataId) != mNBMDataMap.end())
    {
        mNBMDataMap.erase(nbmDataId);
    }
}

void
NBGM_ResourceManager::ReleaseData(const NBRE_Vector<NBGM_NBMDataId>& nbmDataId)
{
    for(NBRE_Vector<NBGM_NBMDataId>::const_iterator iter = nbmDataId.begin(); iter != nbmDataId.end(); ++iter)
    {
        const NBGM_NBMDataId& id = *iter;
        ReleaseData(id);
    }
}

void
NBGM_ResourceManager::RecieveNBMDataFromTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener)
{
    for(NBRE_Vector<NBGM_NBMDataLoadResult>::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        NBGM_NBMDataLoadResult r = *iter;
        if(r.nbmData == NULL)
        {
            continue;
        }
        mNBMDataMap[r.tileInfo.id] = r.nbmData;
    }


    if(result.size() > 0 && listener != NULL)
    {
        listener->OnResponseNBMData(result);
    }
}

void
NBGM_ResourceManager::RecieveUpdateNBMDataFromTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener)
{
    for(NBRE_Vector<NBGM_NBMDataLoadResult>::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        NBGM_NBMDataLoadResult r = *iter;
        if(r.nbmData)
        {
            mNBMDataMap[r.tileInfo.id] = r.nbmData;
        }
    }

    if(result.size() > 0 && listener != NULL)
    {
        listener->OnUpdateNBMData(result);
    }
}