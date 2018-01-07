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

    @file nbgmmapviewimpltask.h
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

#ifndef _NBGM_MAP_VIEW_IMPL_TASK_H_
#define _NBGM_MAP_VIEW_IMPL_TASK_H_

#include "nbgmmapviewprofiler.h"
#include "nbgmmaplayer.h"
#include "nbgmvectortile.h"
#include "palevent.h"
#include "nbgmtaskqueue.h"

class NBRE_IOStream;
class NBGM_MapViewImpl;
class NBGM_IResourceListener;
class NBGM_ResourceManager;

struct NBGM_NBMDataLoadInfo
{
    NBGM_NBMDataLoadInfo():baseDrawOrder(0), subDrawOrder(0), labelDrawOrder(0), enableLog(FALSE), enablePicking(FALSE), selectMask(0), mapData(NULL), loadListener(NULL)
    {
    }
    NBRE_String                         id;
    NBRE_String                         materialCategoryName;
    NBRE_String                         groupName;
    uint8                               baseDrawOrder;
    int32                               subDrawOrder;
    uint8                               labelDrawOrder;
    shared_ptr<NBRE_IOStream>           stream;
    nb_boolean                          enableLog;
    nb_boolean                          enablePicking;
    uint32                              selectMask;
    NBGM_IMapData*                      mapData;
    NBGM_TileLoadingListener*           loadListener;
};

struct NBGM_NBMDataLoadResult
{
    NBGM_NBMDataLoadResult(NBGM_NBMDataLoadInfo info): tileInfo(info)
    {
    }
    NBGM_NBMDataLoadInfo tileInfo;
    shared_ptr<NBGM_NBMData> nbmData;
};

struct NBGM_LoadMaterialResult
{
    NBGM_LoadMaterialResult() {}
    NBRE_IShaderGroupPtr                shaderGroup;
    NBRE_String                         groupID;
};

struct NBGM_SetMaterialResult
{
    NBGM_SetMaterialResult() : haveBackgroundColor(FALSE) {}
    NBRE_String                         groupID;
    nb_boolean                          haveBackgroundColor;
    NBRE_Color                          backgroundColor;
};

class NBGM_LoadNBMTask : public NBGM_Task
{
public:
    NBGM_LoadNBMTask(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue, nb_boolean isUpdate = FALSE);
    virtual ~NBGM_LoadNBMTask();

public:
    virtual void Execute();

private:
    PAL_Error ParseNbmData(NBGM_NBMDataLoadInfo& tileInfo, NBGM_NBMData** nbmData);

private:
    NBRE_Vector<NBGM_NBMDataLoadInfo> mTileInfo;
    NBGM_ResourceManager* mResourceManager;
    NBGM_IResourceListener* mListener;
    shared_ptr<NBGM_TaskQueue> mTaskQueue;
    NBRE_Vector<NBGM_NBMDataLoadResult> mResult;
    nb_boolean mIsUpdate;
};

/**Task used for loading BIN data
*/
class NBGM_LoadBINTask : public NBGM_Task
{
public:
    NBGM_LoadBINTask(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tileInfo, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue);
    virtual ~NBGM_LoadBINTask();

public:
    virtual void Execute();

private:
    PAL_Error ParseBinData(const NBGM_NBMDataLoadInfo& tileInfo, NBGM_NBMData** nbmData);
    PAL_Error MapdataCreate(NBGM_ResourceContext& resourceContext, NBRE_Vector<NBRE_ModelPtr>* models, NBRE_Mapdata** mapdata);

private:
    NBRE_Vector<NBGM_NBMDataLoadInfo> mTileInfo;
    NBGM_ResourceManager* mResourceManager;
    NBGM_IResourceListener* mListener;
    shared_ptr<NBGM_TaskQueue> mTaskQueue;
    NBRE_Vector<NBGM_NBMDataLoadResult> mResult;
};

/**Task used for sync
*/
class NBGM_SyncTask : public NBGM_Task
{
public:
    NBGM_SyncTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_ResourceManager* resourceManager, NBGM_IResourceListener* listener, nb_boolean isUpdate = FALSE);
    virtual ~NBGM_SyncTask();

public:
    virtual void Execute();

private:
    NBRE_Vector<NBGM_NBMDataLoadResult> mResult;
    NBGM_ResourceManager* mResourceManager;
    NBGM_IResourceListener* mListener;
    nb_boolean mIsUpdate;
};

class NBGM_ActiveNBMTask : public NBGM_Task
{
public:
    NBGM_ActiveNBMTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener, const shared_ptr<NBGM_TaskQueue>& taskQueue);
    virtual ~NBGM_ActiveNBMTask();

public:
    virtual void Execute();

private:
    NBRE_Vector<NBGM_NBMDataLoadResult> mResult;
    NBGM_IResourceListener* mListener;
    shared_ptr<NBGM_TaskQueue> mTaskQueue;
};

class NBGM_SyncActiveNBMTask : public NBGM_Task
{
public:
    NBGM_SyncActiveNBMTask(const NBRE_Vector<NBGM_NBMDataLoadResult>& result, NBGM_IResourceListener* listener);
    virtual ~NBGM_SyncActiveNBMTask();

public:
    virtual void Execute();

private:
    NBRE_Vector<NBGM_NBMDataLoadResult> mResult;
    NBGM_IResourceListener* mListener;
    shared_ptr<NBGM_TaskQueue> mTaskQueue;
};

class NBGM_UnloadTask : public NBGM_Task
{
public:
    NBGM_UnloadTask(NBGM_MapViewImpl* impl, const shared_ptr<NBGM_TaskQueue>& taskQueue, nb_boolean unloadAll);
    virtual ~NBGM_UnloadTask();

public:
    void AssignUnloadIds(const NBRE_Vector<NBGM_VectorTileID>& unloadIds);
    virtual void Execute();

private:
    NBRE_Vector<NBGM_VectorTileID> mIds;
    NBGM_MapViewImpl* mImpl;
    shared_ptr<NBGM_TaskQueue> mTaskQueue;
    nb_boolean mUnloadAll;
};

class NBGM_SyncUnloadTask : public NBGM_Task
{
public:
    NBGM_SyncUnloadTask(NBGM_MapViewImpl* impl, nb_boolean unloadAll);
    virtual ~NBGM_SyncUnloadTask();

public:
    void AssignUnloadIds(const NBRE_Vector<NBGM_VectorTileID>& unloadIds);
    virtual void Execute();

private:
    NBRE_Vector<NBGM_VectorTileID> mIds;
    NBGM_MapViewImpl* mImpl;
    nb_boolean mUnloadAll;
};

/**Task used for loading material
*/
class NBGM_LoadMaterialTask : public NBGM_Task
{
public:
    NBGM_LoadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> stream, shared_ptr<NBGM_TaskQueue> renderTaskQueue, const NBRE_Vector<NBRE_String>& dependsMaterial);
    NBGM_LoadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, std::vector<NBGM_IMapMaterial*>& materialTable, shared_ptr<NBGM_TaskQueue> renderTaskQueue);
    virtual ~NBGM_LoadMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_ResourceContext mLoadingContext;
    NBGM_Context mRenderContext;
    NBRE_String mCatergoryName;
    NBRE_String mMaterialName;
    shared_ptr<NBRE_IOStream> mStream;
    NBRE_Vector<NBGM_MapMaterialImpl*>* mMaterialTable;
    shared_ptr<NBGM_TaskQueue> mRenderTaskQueue;
    NBRE_Vector<NBRE_String> mDependsMaterial;
};

/**Task used for sync material
*/
class NBGM_SyncMaterialTask : public NBGM_Task
{
public:
    NBGM_SyncMaterialTask(NBGM_Context& renderContext, const NBGM_LoadMaterialResult& result);
    ~NBGM_SyncMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_Context mRenderContext;
    NBGM_LoadMaterialResult mResult;
};

/**Task used for set current material
*/
class NBGM_SeCurrenttMaterialTask : public NBGM_Task
{
public:
    NBGM_SeCurrenttMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_MapViewImpl* mapViewImpl, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBGM_TaskQueue> renderTaskQueue, NBRE_Overlay* backgroundOverlay, NBRE_Set<uint32>* fontIds);
    virtual ~NBGM_SeCurrenttMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_ResourceContext mLoadingContext;
    NBGM_Context mRenderContext;
    NBRE_String mCatergoryName;
    NBRE_String mMaterialName;
    shared_ptr<NBGM_TaskQueue> mRenderTaskQueue;
    NBRE_Overlay* mBackgroundOverlay;
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_Set<uint32>* mFontIds;
};

/**Task used by sync current material
*/
class NBGM_SyncCurrentMaterialTask : public NBGM_Task
{
public:
    NBGM_SyncCurrentMaterialTask(NBGM_Context& renderContext, NBGM_MapViewImpl* mapViewImpl, const NBGM_SetMaterialResult& result, NBRE_Overlay* backgroundOverlay, const NBRE_String& catergoryName, NBRE_Map<uint32, NBRE_FontMaterial>* fontMaterials);
    virtual ~NBGM_SyncCurrentMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_Context mRenderContext;
    NBGM_SetMaterialResult mResult;
    NBRE_Overlay* mBackgroundOverlay;
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_String mCatergoryName;
    NBRE_Map<uint32, NBRE_FontMaterial>* mFontMaterials;
};

class NBGM_UnloadMaterialTask : public NBGM_Task
{
public:
    NBGM_UnloadMaterialTask(NBGM_ResourceContext& loadingContext, NBGM_Context& renderContext, const NBRE_String& catergoryName, const NBRE_String& materialName, shared_ptr<NBGM_TaskQueue> renderTaskQueue);
    virtual ~NBGM_UnloadMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_ResourceContext mLoadingContext;
    NBGM_Context mRenderContext;
    NBRE_String mCatergoryName;
    NBRE_String mMaterialName;
    shared_ptr<NBGM_TaskQueue> mRenderTaskQueue;
};

class NBGM_SyncUnloadedMaterialTask : public NBGM_Task
{
public:
    NBGM_SyncUnloadedMaterialTask(NBGM_Context& renderContext, const NBRE_String& groupId);
    virtual ~NBGM_SyncUnloadedMaterialTask();

public:
    virtual void Execute();

private:
    NBGM_Context mRenderContext;
    NBRE_String mGroupId;
};

/**Task used by NBGM_MapViewImpl to notify that all task in the loading thread has been executed.
*/
class NBGM_FinalizeTask : public NBGM_Task
{

public:
    NBGM_FinalizeTask(PAL_Event* finalizeEvent);
    virtual ~NBGM_FinalizeTask();

public:
    virtual void Execute();

private:
    PAL_Event* mFinalizeEvent;
};

class NBGM_RenderTask : public NBGM_Task
{
public:
    NBGM_RenderTask(NBGM_MapViewImpl* mapViewImpl);
    virtual ~NBGM_RenderTask() {};
public:
    virtual void Execute();
private:
    NBGM_MapViewImpl* mMapViewImpl;
};

class NBGM_InvalidateTask : public NBGM_Task
{
public:
    NBGM_InvalidateTask(NBGM_MapViewImpl* mapViewImpl);
    virtual ~NBGM_InvalidateTask();
public:
    virtual void Execute();
private:
    NBGM_MapViewImpl* mMapViewImpl;
};

class NBGM_SetAvatarLoacationTask : public NBGM_Task
{
public:
    NBGM_SetAvatarLoacationTask(NBGM_MapViewImpl* mapViewImpl, const NBGM_Location64& location);
    virtual ~NBGM_SetAvatarLoacationTask();
public:
    virtual void Execute();
private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_Location64 mLocation;
};

class NBGM_LoadingThreadScreenshotTask : public NBGM_Task
{
public:
    NBGM_LoadingThreadScreenshotTask(shared_ptr<NBGM_TaskQueue> renderTaskQueue, NBGM_MapViewImpl* mapView);
    virtual ~NBGM_LoadingThreadScreenshotTask();

public:
    virtual void Execute();

private:
    shared_ptr<NBGM_TaskQueue> mRenderTaskQueue;
    NBGM_MapViewImpl* mMapViewImpl;
};

class NBGM_RenderingThreadScreenshotTask : public NBGM_Task
{
public:
    NBGM_RenderingThreadScreenshotTask(NBGM_MapViewImpl* mapView);
    virtual ~NBGM_RenderingThreadScreenshotTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
};

class NBGM_LoadNBMTileTask : public NBGM_Task
{
public:
    NBGM_LoadNBMTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info);
    virtual ~NBGM_LoadNBMTileTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_NBMDataLoadInfo mInfo;
};

class NBGM_LoadNBMTilesTask : public NBGM_Task
{
public:
    NBGM_LoadNBMTilesTask(NBGM_MapViewImpl* mapView, const NBRE_Vector<NBGM_NBMDataLoadInfo>& infos);
    virtual ~NBGM_LoadNBMTilesTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_Vector<NBGM_NBMDataLoadInfo> mInfos;
};

class NBGM_LoadBINTileTask : public NBGM_Task
{
public:
    NBGM_LoadBINTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info);
    virtual ~NBGM_LoadBINTileTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_NBMDataLoadInfo mInfo;
};

class NBGM_UnloadTileTask : public NBGM_Task
{
public:
    NBGM_UnloadTileTask(NBGM_MapViewImpl* mapView, const NBRE_String& name);
    virtual ~NBGM_UnloadTileTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_String mName;
};

class NBGM_UnloadTilesTask : public NBGM_Task
{
public:
    NBGM_UnloadTilesTask(NBGM_MapViewImpl* mapView, const NBRE_Vector<NBRE_String>& names);
    virtual ~NBGM_UnloadTilesTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_Vector<NBRE_String> mNames;
};

class NBGM_UnloadALLTilesTask : public NBGM_Task
{
public:
    NBGM_UnloadALLTilesTask(NBGM_MapViewImpl* mapView);
    virtual ~NBGM_UnloadALLTilesTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
};

class NBGM_ReloadTileTask : public NBGM_Task
{
public:
    NBGM_ReloadTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info, const NBRE_String& unloadTileName);
    virtual ~NBGM_ReloadTileTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_NBMDataLoadInfo mInfo;
    NBRE_String mUnloadTileName;
};

class NBGM_UpdateTileTask : public NBGM_Task
{
public:
    NBGM_UpdateTileTask(NBGM_MapViewImpl* mapView, const NBGM_NBMDataLoadInfo& info);
    virtual ~NBGM_UpdateTileTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_NBMDataLoadInfo mInfo;
};

class NBGM_RenderingThreadAsyncSnapScreenshotTask : public NBGM_Task
{
public:
    NBGM_RenderingThreadAsyncSnapScreenshotTask(NBGM_MapViewImpl* mapView, const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot);
    virtual ~NBGM_RenderingThreadAsyncSnapScreenshotTask();

public:
    virtual void Execute();

private:
    NBGM_MapViewImpl* mMapViewImpl;
    shared_ptr<NBGM_AsyncSnapScreenshotCallback> mSnapScreenshot;
};

#endif
