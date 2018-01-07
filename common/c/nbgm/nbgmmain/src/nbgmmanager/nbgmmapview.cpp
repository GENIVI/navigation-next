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
#include "nbgmmapviewprotected.h"
#include "nbgmmapviewimpl.h"
#include "nbrefilestream.h"
#include "nbrememorystream.h"
#include "palstdlib.h"
#include "palfile.h"
#include "nbgmcontext.h"
#include "nbrecontext.h"
#include "nbrerenderengine.h"
#include "nbrelog.h"
#include "nbgmconst.h"
#if GL_CAPABILITY_TEST
#include "nbgmfakemapview.h"
#endif
#include "nbgm.h"
#include "nbgmprotected.h"
#include "nbgmrendercontext.h"
#include "nbgmmapviewimpltask.h"
#include "palclock.h"
#include "paltestlog.h"
#include "nbretypeconvert.h"
#include "nbretransformation.h"
#include "nbgmmapcameraimpl.h"
#include "nbgmmapdataimpl.h"

static const uint32 UpdateCameraInterval = 16;

class UpdateCameraListener : public NBGM_FrameListener
{
public:
    UpdateCameraListener(NBGM_FrameListener* l, NBGM_MapViewInternal* mapview)
        :mListener(l), mMapview(mapview)
    {
    }

    virtual void OnRenderFrameBegin()
    {
        mMapview->UpdateCamera();
        if(mListener)
        {
            mListener->OnRenderFrameBegin();
        }
    }

    virtual void OnRenderFrameEnd()
    {
        if(mListener)
        {
            mListener->OnRenderFrameEnd();
        }
        NBRE_DELETE this;
    }

    virtual void OnDiscard()
    {
        if(mListener)
        {
            mListener->OnDiscard();
        }
        NBRE_DELETE this;
    }

private:
    NBGM_FrameListener* mListener;
    NBGM_MapViewInternal* mMapview;
};

NBGM_MapViewInternal::NBGM_MapViewInternal( const NBGM_MapViewConfig& config, PAL_Instance* pal):
    mRenderEngine(NULL),
    mImpl(NULL),
    mPrevZoomLevel(INVALID_CHAR_INDEX),
    mScreenHeight(1),
    mScreenWidth(1),
    mUseSharedResource(FALSE),
    mResourceManager(NULL),
    mIsAvatarUpdate(FALSE)
{
    mConfig = config;
    mPal = pal;
}

NBGM_MapViewInternal::NBGM_MapViewInternal(const NBGM_MapViewConfig& config, PAL_Instance* pal, NBGM_ResourceManager* resourcManager):
    mRenderEngine(NULL),
    mImpl(NULL),
    mPrevZoomLevel(INVALID_CHAR_INDEX),
    mScreenHeight(1),
    mScreenWidth(1),
    mUseSharedResource(TRUE),
    mResourceManager(resourcManager),
    mIsAvatarUpdate(FALSE)
{
    mConfig = config;
    mPal = pal;
}

NBGM_MapViewInternal::~NBGM_MapViewInternal()
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::~NBGM_MapViewInternal");
}

class NBGM_CameraUpdateTask : public NBGM_Task
{
public:
    NBGM_CameraUpdateTask(NBGM_MapViewInternal* mapView): mMapView(mapView) {};
    virtual ~NBGM_CameraUpdateTask() {};
public:
    virtual void Execute()
    {
        mMapView->UpdateCamera();
    }
private:
    NBGM_MapViewInternal* mMapView;
};

void NBGM_MapViewInternal::CameraUpdateCallBack(PAL_Instance * /*pal*/, void *userData, PAL_TimerCBReason reason)
{
    NBGM_MapViewInternal* thiz = (NBGM_MapViewInternal*) userData;
    if(thiz != NULL && reason == PTCBR_TimerFired)
    {
        thiz->UpdateCamera();
    }
}

void NBGM_MapViewInternal::Initialize()
{
    uint32 start = PAL_ClockGetTimeMs();
    NBGM_MapViewImplConfig c;
    c.viewConfig = mConfig;
    c.drawNavPoi = FALSE;
    c.drawVectorNavRoute = FALSE;
    c.modelScaleFactor = VECTOR_MAP_MODEL_SCALE_FACTOR;
    c.avatarSizeFactor = METER_TO_MERCATOR(VECTOR_AVATA_SIZE_FACTOR);
    c.referenceAvatarHeight = METER_TO_MERCATOR(NBGM_MapScaleUtility::GetReferenceAvatarHeight());
    c.maxLabels = 0xffffffff;
    c.maxAreaBuildingLabels = 0xffffffff;
    c.renderInterval = mConfig.renderInterval;
    if(!mUseSharedResource)
    {
        NBGM_CreateInternalResourceManager(&mConfig, mPal, &mResourceManager);
    }
    c.resourceManager = mResourceManager;

    mRenderEngine = NBGM_CreateRenderEingne(mResourceManager->GetResourceContext().renderPal, mPal);

#if GL_CAPABILITY_TEST
    mImpl = NBRE_NEW NBGM_FakeMapView(c, mRenderEngine);
#else
    mImpl = NBRE_NEW NBGM_MapViewImpl(c, mRenderEngine);
#endif
    mImpl->InitializeEnvironment();

    mCameraImpl = NBRE_NEW NBGM_MapCameraImpl();

    uint32 end = PAL_ClockGetTimeMs();
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM initialization time = %dms", end - start);

    mImpl->AddTimerCallback(CameraUpdateCallBack, this);
    mUpdateExpectedTime = PAL_ClockGetTimeMs() + UpdateCameraInterval;
}

void
NBGM_MapViewInternal::Finalize()
{
    mResourceManager->GetConfig().renderContext->SwitchTo();
    NBRE_DELETE mCameraImpl;
    mCameraImpl = NULL;
    mImpl->Finalize();
    NBRE_DELETE mImpl;
    mImpl = NULL;
    if(mRenderEngine)
    {
        mRenderEngine->Deinitialize();
        NBRE_DELETE mRenderEngine;
    }
    if(!mUseSharedResource && mResourceManager != NULL)
    {
        NBGM_DestroyResourceManager(&mResourceManager);
    }
    mResourceManager = NULL;
}

void NBGM_MapViewInternal::Invalidate()
{
    mImpl->AddTask(NBRE_NEW NBGM_InvalidateTask(mImpl));
}

void NBGM_MapViewInternal::RequestRender(NBGM_FrameListener* listener)
{
    mImpl->RequestRenderFrame(NBRE_NEW UpdateCameraListener(listener, this));
}

void NBGM_MapViewInternal::SetRenderInterval(uint32 interval)
{
    mImpl->SetRenderInterval(interval);
}

NBGM_MapCamera& NBGM_MapViewInternal::GetMapCamera()
{
    return *mCameraImpl;
}

void NBGM_MapViewInternal::SetBackground(bool background)
{
    mImpl->SetBackground(background);
}

PAL_Error NBGM_MapViewInternal::LoadExternalMaterial(const NBGM_MaterialParameters& params, uint32 /*flags*/)
{
    if (params.filePath.empty())
    {
        return PAL_ErrBadParam;
    }
    shared_ptr<NBRE_FileStream> fs(NBRE_NEW NBRE_FileStream(NBGM_GetConfig()->pal, params.filePath.c_str(), 2048));
    mImpl->LoadExternalMaterial(params.categoryName, params.materialName, fs, params.dependsMaterial);
//    NBRE_DebugLog(PAL_LogSeverityDebug,
//        "NBGM_MapViewInternal::LoadCommonMaterial, materialName = %s, filePath = %s",
//        materialName.c_str(), filePath.c_str());
    return PAL_Ok;
}

PAL_Error NBGM_MapViewInternal::LoadExternalMaterialFromBuffer(const NBGM_MaterialParameters& params, uint32 /*flags*/)
{
    if(params.buffer == NULL || params.bufferSize == 0)
    {
        return PAL_ErrBadParam;
    }
    shared_ptr<NBRE_MemoryStream> ms(NBRE_NEW NBRE_MemoryStream(params.buffer, params.bufferSize, TRUE));
    mImpl->LoadExternalMaterial(params.categoryName, params.materialName, ms, params.dependsMaterial);
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::LoadCommonMaterialFromBuffer, materialName = %s, buffer = %p, bufferSize = %u, err = 0x%08x", materialName.c_str(), buffer, bufferSize, err);
    return PAL_Ok;
}

PAL_Error NBGM_MapViewInternal::LoadExternalMaterialFromMemory(const std::string& categoryName, const std::string& materialName, std::vector<NBGM_IMapMaterial*>& materialTable)
{
    mImpl->LoadExternalMaterial(categoryName, materialName, materialTable);
    return PAL_Ok;
}

PAL_Error NBGM_MapViewInternal::SetCurrentExternalMaterial(const std::string& categoryName, const std::string& materialName)
{
    mImpl->SetCurrentExternalMaterial(categoryName, materialName);
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetCurrentCommonMaterial, materialName = %s, err = 0x%08x", materialName.c_str(), err);
    return PAL_Ok;
}

PAL_Error NBGM_MapViewInternal::UnloadExternalMaterial(const std::string& categoryName, const std::string& materialName)
{
    mImpl->UnloadExternalMaterial(categoryName, materialName);
    return PAL_Ok;
}

bool NBGM_MapViewInternal::IsNbmTileLoaded( const std::string& nbmName )
{
    bool res = mImpl->IsNbmDataExist(nbmName) == TRUE;
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::IsNbmTileLoaded = %d , nbmName = %s", res, nbmName.c_str());
    return res;
}

PAL_Error NBGM_MapViewInternal::LoadNBMTile(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const std::string& filePath, uint32 flags, NBGM_TileLoadingListener* loadListener)
{
    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(NBGM_GetConfig()->pal, filePath.c_str(), 4096);
    NBGM_NBMDataLoadInfo info;
    info.id = nbmName;
    info.materialCategoryName = categoryName;
    info.groupName = groupName;
    info.baseDrawOrder = baseDrawOrder;
    info.subDrawOrder = subDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = flags & NBGM_NLF_ENABLE_LOG ? TRUE : FALSE;
    info.enablePicking = flags & NBGM_NLF_ENABLE_PICKING ? TRUE : FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;
    info.loadListener = loadListener;
    mImpl->AddTask(NBRE_NEW NBGM_LoadNBMTileTask(mImpl, info));

//    NBRE_DebugLog(PAL_LogSeverityDebug,
//        "NBGM_MapViewInternal::LoadNBMTile, nbmName=%s, baseDrawOrder=%u, labelDrawOrder=%u, filePath = %s",
//        nbmName.c_str(), baseDrawOrder, labelDrawOrder, filePath.c_str());
    return PAL_Ok;
}

#ifdef USE_NB_TILE_OBJECT
PAL_Error NBGM_MapViewInternal::LoadNBMTile(const std::vector<nbmap::TilePtr>& tiles)
{
    NBRE_Vector<NBGM_NBMDataLoadInfo> tileInfos;
    uint32 largestBufferSize = 0;
    for(std::vector<nbmap::TilePtr>::const_iterator iter = tiles.begin(); iter != tiles.end(); ++iter)
    {
        uint32 bufferSize = (*iter)->GetData()->GetDataSize();
        if(bufferSize>largestBufferSize)
        {
            largestBufferSize = bufferSize;
        }
    }
    nbre_assert(largestBufferSize > 0);
    uint8* buffer = NBRE_NEW uint8[largestBufferSize];
    for(std::vector<nbmap::TilePtr>::const_iterator iter = tiles.begin(); iter != tiles.end(); ++iter)
    {
        uint32 bufferSize = (*iter)->GetData()->GetDataSize();
        uint32 receivedDataSize = (*iter)->GetData()->GetData(buffer, 0, bufferSize);
        if(receivedDataSize != bufferSize)
        {
            continue;
        }
        NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE);
        NBGM_NBMDataLoadInfo info;
        info.id = *((*iter)->GetContentID());
        info.baseDrawOrder = (*iter)->GetDrawOrder();
        info.labelDrawOrder = (*iter)->GetLabelDrawOrder();
        info.stream = shared_ptr<NBRE_IOStream>(ms);
        info.enableLog = FALSE;
        tileInfos.push_back(info);
    }
    NBRE_DELETE[] buffer;
    mImpl->AddTask(NBRE_NEW NBGM_LoadNBMTilesTask(mImpl, tileInfos));
    return PAL_Ok;
}
#endif

PAL_Error NBGM_MapViewInternal::LoadNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* loadListener)
{
    PAL_Error err = PAL_Ok;

    if(buffer == NULL || bufferSize == 0)
    {
        return PAL_ErrBadParam;
    }
    //it will be deleted in loading tile task queue
    NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE);
    NBGM_NBMDataLoadInfo info;
    info.id = nbmName;
    info.materialCategoryName = categoryName;
    info.groupName = groupName;
    info.baseDrawOrder = baseDrawOrder;
    info.subDrawOrder = subDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    info.stream = shared_ptr<NBRE_IOStream>(ms);
    info.enableLog = flags & NBGM_NLF_ENABLE_LOG ? TRUE : FALSE;
    info.enablePicking = flags & NBGM_NLF_ENABLE_PICKING ? TRUE : FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;
    info.loadListener = loadListener;
    mImpl->AddTask(NBRE_NEW NBGM_LoadNBMTileTask(mImpl, info));
//    NBRE_DebugLog(PAL_LogSeverityDebug,
//        "NBGM_MapViewInternal::LoadNBMTileFromBuffer, nbmName = %s, baseDrawOrder = %u, labelDrawOrder = %u, buffer = %p, bufferSize = %u, err = 0x%08x",
//        nbmName.c_str(), baseDrawOrder, labelDrawOrder, buffer, bufferSize, err);
    return err;
}

NBGM_IMapMaterial* NBGM_MapViewInternal::CreateStandardColorMaterial(uint32 color)
{
    return NBRE_NEW NBGM_StandardColorMapMaterialImpl(color);
}

NBGM_IMapData* NBGM_MapViewInternal::CreateMapData(const NBGM_Point2d& refCenter)
{
    return NBRE_NEW NBGM_MapDataImpl(refCenter);
}

void NBGM_MapViewInternal::DestroyMapData(NBGM_IMapData* mapData)
{
    NBRE_DELETE mapData;
}

PAL_Error NBGM_MapViewInternal::LoadMapData(const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint32 flags, NBGM_IMapData* mapdata)
{
    PAL_Error result = PAL_Ok;

    if(mapdata == NULL)
    {
        return PAL_ErrBadParam;
    }

    //it will be deleted in loading tile task queue
    NBGM_NBMDataLoadInfo info;
    info.id = nbmName;
    info.materialCategoryName = categoryName;
    info.baseDrawOrder = baseDrawOrder;
    info.subDrawOrder = subDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    info.mapData = mapdata;
    info.enableLog = flags & NBGM_NLF_ENABLE_LOG ? TRUE : FALSE;
    info.enablePicking = flags & NBGM_NLF_ENABLE_PICKING ? TRUE : FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;
    mImpl->AddTask(NBRE_NEW NBGM_LoadNBMTileTask(mImpl, info));

    return result;
}

void NBGM_MapViewInternal::UnLoadTile( const std::string& nbmName )
{
    mImpl->AddTask(NBRE_NEW NBGM_UnloadTileTask(mImpl, nbmName));
}

void NBGM_MapViewInternal::UnLoadTile(const std::vector<std::string>& nbmNames)
{
    mImpl->AddTask(NBRE_NEW NBGM_UnloadTilesTask(mImpl, nbmNames));
}

void NBGM_MapViewInternal::UnLoadAllTiles()
{
    mImpl->AddTask(NBRE_NEW NBGM_UnloadALLTilesTask(mImpl));
}

void NBGM_MapViewInternal::SetAvatarLocation( const NBGM_Location64& location )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetAvatarLocation, x = %f, y = %f, z = %f, time = %u, speed = %f, accuracy = %f, heading = %f", location.position.x, location.position.y, location.position.z, location.time, location.speed, location.accuracy, location.heading);
    mAvatarLocation = location;
    mIsAvatarUpdate = TRUE;
}

float NBGM_MapViewInternal::GetFontScale( void )
{
    //    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetFontScale, scaleValue = %f", scaleValue);
    return mImpl->GetFontScale();
}

void NBGM_MapViewInternal::SetFontScale( float scaleValue )
{
    //    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetFontScale, scaleValue = %f", scaleValue);
    mImpl->SetFontScale(scaleValue);
}

void NBGM_MapViewInternal::SetAvatarScale( float scaleValue )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetAvatarScale, scaleValue = %f", scaleValue);
    mImpl->SetAvatarScale(scaleValue);
}

bool NBGM_MapViewInternal::GetAvatarScreenPosition( float& screenX, float& screenY )
{
    NBRE_Point2f position;
    bool res = mImpl->GetAvatarScreenPosition(position) == TRUE;
    if(res)
    {
        screenX = position.x;
        screenY = position.y;
    }
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetAvatarScreenPosition, result = %d, screenX = %f, screenY = %f", res, screenX, screenY);
    return res;
}

bool NBGM_MapViewInternal::IsAvatarInScreen(float mercatorX, float mercatorY)
{
    NBRE_Vector3d pos(mercatorX, mercatorY, 0);
    return mImpl->IsAvatarInScreen(pos) ? true : false;
}

void NBGM_MapViewInternal::GetAvatarScreenPolygons( std::vector<float>& /*screenPolygons*/ )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetAvatarScreenPolygons Not Supported");
}

void NBGM_MapViewInternal::SetAvatarState( NBGM_AvatarState state )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetAvatarState, state = %d", state);
    mImpl->SetAvatarState(state);
    mImpl->SetIsNavMode(FALSE);
}

void NBGM_MapViewInternal::SetAvatarMode( NBGM_AvatarMode mode )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SetAvatarMode, mode = %d", mode);
    mImpl->SetAvatarMode(mode);
}

bool NBGM_MapViewInternal::SelectAndTrackAvatar(float x, float y)
{
    return mImpl->SelectAndTrackAvatar(x, y) ? TRUE : FALSE;
}

void NBGM_MapViewInternal::EnableLayer( uint8 /*drawOrder*/, bool /*enable*/ )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::EnableLayer Not Supported");
}

void NBGM_MapViewInternal::EnableProfiling( uint32 flags )
{
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::EnableProfiling, flags = %u", flags);
    if(flags & NBGM_PF_ENABLE)
    {
        mImpl->EnableProfiling(TRUE);
    }
    else
    {
        mImpl->EnableProfiling(FALSE);
    }

    if(flags & NBGM_PF_ENABLE_VERBOSE)
    {
        mImpl->EnableVerboseProfiling(TRUE);
    }
    else
    {
        mImpl->EnableVerboseProfiling(FALSE);
    }
}

bool NBGM_MapViewInternal::GetPoiPosition( const std::string& poiId, NBGM_Point2d& screenPos )
{
    nb_boolean found = FALSE;
    NBRE_Point2f pos;
    found = mImpl->GetPoiPosition(poiId, pos);
    screenPos.x = pos.x;
    screenPos.y = pos.y;
    bool res = found == TRUE;
//    NBRE_DebugLog(PAL_LogSeverityDebug,
//                  "NBGM_MapViewInternal::GetPoiPosition, found = %d, poiId = %s, x = %f, y = %f",
//                  res, poiId.c_str(), screenPos.x, screenPos.y);
    return res;
}

bool NBGM_MapViewInternal::GetPoiBubblePosition(const std::string& poiId, NBGM_Point2d& coordinates)
{
    nb_boolean found = FALSE;
    NBRE_Point2f pos;
    found = mImpl->GetPoiBubblePosition(poiId, pos);
    coordinates.x = pos.x;
    coordinates.y = pos.y;
    bool res = found == TRUE;
//    NBRE_DebugLog(PAL_LogSeverityDebug,
//                  "NBGM_MapViewInternal::GetPoiBubblePosition, found = %d, poiId = %s, x = %f, y = %f",
//                  res, poiId.c_str(), coordinates.x, coordinates.y);
    return res;

}

bool NBGM_MapViewInternal::SelectAndTrackPoi( const std::string& poiId )
{
   bool res = mImpl->SelectAndTrackPoi(poiId) == TRUE;
   //    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::SelectAndTrackPoi = %d, poiId = %s", res, poiId.c_str());
   return res;
}

bool NBGM_MapViewInternal::UnselectAndStopTrackingPoi( const std::string& poiId )
{
    bool res = mImpl->UnselectAndStopTrackingPoi(poiId) == TRUE;
    //    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::UnselectAndStopTrackingPoi = %d, poiId = %s", res, poiId.c_str());
    return res;
}

uint32 NBGM_MapViewInternal::GetInteractedPois( std::vector<std::string>& poiList, float x, float y )
{
    NBRE_Point2f pos(x, y);
    uint32 res = mImpl->GetInteractedPois(poiList, pos);

//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetInteractedPois, poiList.size = %d", poiList.size());
//    for(std::vector<std::string>::const_iterator i = poiList.begin(); i != poiList.end(); i++)
//    {
//        NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetInteractedPois, poiString = %s", i->c_str());
//    }
//    NBRE_DebugLog(PAL_LogSeverityDebug, "NBGM_MapViewInternal::GetInteractedPois = %u, x = %f, y = %f", res, x, y);
    return res;
}

bool NBGM_MapViewInternal::GetStaticPoiInfo(const std::string& poiId, NBGM_StaticPoiInfo& info)
{
    return mImpl->GetStaticPoiInfo(poiId, info) == TRUE;
}

void NBGM_MapViewInternal::SelectStaticPoi(const std::string& id, nb_boolean selected)
{
    mImpl->SelectStaticPoi(id, selected);
}

void NBGM_MapViewInternal::GetStaticPois( std::vector<std::string>& poiList, float x, float y )
{
    NBRE_Point2f pos(x, y);
    mImpl->GetStaticPoiAt(poiList, pos);
}

void NBGM_MapViewInternal::GetElementsAt(float x, float y, std::set<std::string>& ids)
{
    mImpl->GetElementsAt(NBRE_Point2f(x, y), PICKING_ELEMENT_MASK_STANDARD, ids);
}

void NBGM_MapViewInternal::EnableCollisionDetection(const std::string& layerId, bool enable)
{
    mImpl->EnableCollisionDetection(layerId, enable ? TRUE : FALSE);
}

void
NBGM_MapViewInternal::DumpProfile(NBGM_MapViewProfile& profile)
{
    mImpl->DumpProfile(profile);
}

void
NBGM_MapViewInternal::SetSkyDayNight(nb_boolean isDay)
{
    mImpl->SetSkyDayNight(isDay);
}

void
NBGM_MapViewInternal::SetSatelliteMode(nb_boolean isSatellite)
{
    mImpl->SetIsSatelliteMode(isSatellite);
}

NBGM_TileAnimationLayer*
NBGM_MapViewInternal::CreateTileAnimationLayer()
{
    return mImpl->CreateTileAnimationLayer();
}

void
NBGM_MapViewInternal::RemoveTileAnimationLayer(NBGM_TileAnimationLayer* layer)
{
    mImpl->RemoveTileAnimationLayer(layer);
}

NBGM_OptionalLayer*
NBGM_MapViewInternal::CreateOptionalLayer()
{
    return mImpl->CreateOptionalLayer();
}

void
NBGM_MapViewInternal::RemoveOptionalLayer(NBGM_OptionalLayer* layer)
{
    mImpl->RemoveOptionalLayer(layer);
}

NBGM_DeviceLocation* NBGM_MapViewInternal::CreateDeviceLocation(std::string ID)
{
    return mImpl->CreateDeviceLocation(ID);
}

void NBGM_MapViewInternal::ReleaseDeviceLocation(NBGM_DeviceLocation* device)
{
    mImpl->ReleaseDeviceLocation(device);
}

PAL_Error
NBGM_MapViewInternal::SnapScreenshot(uint8* buffer, uint32 bufferSize, int32 x, int32 y, uint32 width, uint32 height) const
{
    if(buffer == NULL || bufferSize < width*height*4)
    {
        return PAL_ErrBadParam;
    }
    return mImpl->SnapScreenshot(x, y, width, height, buffer);
}

PAL_Error
NBGM_MapViewInternal::AsyncSnapScreenshot(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot)
{
    NBGM_RenderingThreadAsyncSnapScreenshotTask* task = new NBGM_RenderingThreadAsyncSnapScreenshotTask(mImpl, snapScreenshot);
    mImpl->AddTask(task);
    return PAL_Ok;
}

void
NBGM_MapViewInternal::UpdateCamera()
{
    if(mCameraImpl->GetCameraChanged())
    {
        NBRE_Vector2d viewCenter = mCameraImpl->GetViewCenter();
        mImpl->SetViewCenter(viewCenter.x, viewCenter.y);
        mImpl->SetRotateAngle(mCameraImpl->GetRotateAngle());
        mImpl->SetTiltAngle(mCameraImpl->GetTiltAngle());
        mImpl->SetViewPointDistance(METER_TO_MERCATOR(mCameraImpl->GetViewPortDisantce()));
        mImpl->SetHorizonDistance(METER_TO_MERCATOR(mCameraImpl->GetHorizonDistance()));
        mImpl->SetCurrentZoomLevel(CalcZoomLevel(mCameraImpl->GetViewPortDisantce()));
        mCameraImpl->ResetCameraChangedFlag();
    }

    if(mCameraImpl->GetViewPortChanged())
    {
        NBRE_AxisAlignedBox2i viewSize = mCameraImpl->GetViewSize();
        NBRE_PerspectiveConfig config = mCameraImpl->GetPerspectiveConfig();
        mImpl->OnSizeChanged(viewSize.minExtend.x, viewSize.minExtend.y, viewSize.GetSize().x, viewSize.GetSize().y);
        mImpl->SetPerspective(config.mFov, config.mAspect);
        mCameraImpl->ResetViewPortChangedFlag();
    }

    if(mIsAvatarUpdate)
    {
        mImpl->SetAvatarLocation(mAvatarLocation);
        mIsAvatarUpdate = FALSE;
    }
}

PAL_Error
NBGM_MapViewInternal::ReloadNBMTileFromBuffer(const std::string& needUnloadedNbmName, const std::string& needLoadedNbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 /*subDrawOrder*/, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags, NBGM_TileLoadingListener* listener)
{
    if(buffer == NULL || bufferSize == 0 || needUnloadedNbmName.empty())
    {
        return PAL_ErrBadParam;
    }

    NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE);
    NBGM_NBMDataLoadInfo info;
    info.id = needLoadedNbmName;
    info.materialCategoryName = categoryName;
    info.groupName = groupName;
    info.baseDrawOrder = baseDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    info.stream = shared_ptr<NBRE_IOStream>(ms);
    info.enableLog = flags & NBGM_NLF_ENABLE_LOG ? TRUE : FALSE;
    info.enablePicking = flags & NBGM_NLF_ENABLE_PICKING ? TRUE : FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;
    info.loadListener = listener;
    mImpl->AddTask(NBRE_NEW NBGM_ReloadTileTask(mImpl, info, needUnloadedNbmName));

    return PAL_Ok;
}

PAL_Error
NBGM_MapViewInternal::UpdateNBMTileFromBuffer(const std::string& nbmName, const std::string& categoryName, const std::string& groupName, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize, uint32 flags)
{
    if(nbmName.empty())
    {
        return PAL_ErrBadParam;
    }

    NBGM_NBMDataLoadInfo info;
    info.id = nbmName;
    info.materialCategoryName = categoryName;
    info.groupName = groupName;
    info.baseDrawOrder = baseDrawOrder;
    info.subDrawOrder = subDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    if(buffer)
    {
        NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE);
        info.stream = shared_ptr<NBRE_IOStream>(ms);
    }
    info.enableLog = flags & NBGM_NLF_ENABLE_LOG ? TRUE : FALSE;
    info.enablePicking = flags & NBGM_NLF_ENABLE_PICKING ? TRUE : FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;
    mImpl->AddTask(NBRE_NEW NBGM_UpdateTileTask(mImpl, info));

    return PAL_Ok;
}


nb_boolean
NBGM_MapViewInternal::TapCompass(float screenX, float screenY)
{
    if(!mImpl) return FALSE;
    return mImpl->TapCompass(screenX, screenY);
}

void
NBGM_MapViewInternal::EnableCompass(nb_boolean value)
{
    if(!mImpl) return;
    mImpl->EnableCompass(value);
}

void
NBGM_MapViewInternal::SetCompassPosition(float screenX, float screenY)
{
    if(!mImpl) return;
    mImpl->SetCompassPosition(screenX, screenY);
}

void
NBGM_MapViewInternal::SetCompassDayNightMode(nb_boolean isDay)
{
    if(!mImpl) return;
    mImpl->SetCompassDayNightMode(isDay);
}

void
NBGM_MapViewInternal::SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath)
{
    if(!mImpl) return;
    mImpl->SetCompassIcons(dayModeIconPath, nightModeIconPath);
}

void
NBGM_MapViewInternal::GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY) const
{
    if(!mImpl) return;
    mImpl->GetCompassBoundingBox(leftBottomX, leftBottomY, rightTopX, rightTopY);
}

NBGM_Circle*
NBGM_MapViewInternal::AddCircle(int circleId, const NBGM_CircleParameters &circlePara)
{
    if(!mImpl) return NULL;
    return mImpl->AddCircle(circleId, circlePara);
}

void
NBGM_MapViewInternal::RemoveCircle(NBGM_Circle* circle)
{
    if(!mImpl) return;
    mImpl->RemoveCircle(circle);
}

void
NBGM_MapViewInternal::RemoveAllCircles()
{
    if(!mImpl) return;
    mImpl->RemoveAllCircles();
}

NBGM_CustomRect2d*
NBGM_MapViewInternal::AddRect2d(NBGM_Rect2dId id, const NBGM_Rect2dParameters &para)
{
    if(!mImpl) return NULL;
    return mImpl->AddRect2d(id, para);
}

void
NBGM_MapViewInternal::RemoveRect2d(NBGM_CustomRect2d* rect)
{
    if(!mImpl) return;
    mImpl->RemoveRect2d(rect);
}

void
NBGM_MapViewInternal::RemoveAllRect2ds()
{
    if(!mImpl) return;
    mImpl->RemoveAllRect2ds();
}

bool
NBGM_MapViewInternal::AddTexture(NBGM_TextureId id, const NBGM_BinaryBuffer &textureData)
{
    if(!mImpl) return FALSE;
    return mImpl->AddTexture(id, textureData);
}

void
NBGM_MapViewInternal::RemoveTexture(NBGM_TextureId id)
{
    if(!mImpl) return;
    mImpl->RemoveTexture(id);
}

void
NBGM_MapViewInternal::RemoveAllTextures()
{
    mImpl->RemoveAllTextures();
}

void
NBGM_MapViewInternal::AddPins(const std::vector<NBGM_PinParameters>& pinParameters)
{
    mImpl->AddPins(pinParameters);
}

void
NBGM_MapViewInternal::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    mImpl->RemovePins(pinIDs);
}

void
NBGM_MapViewInternal::RemoveAllPins()
{
    mImpl->RemoveAllPins();
}

void
NBGM_MapViewInternal::UpdatePinPosition(shared_ptr<NBRE_String> pinID, double locationX, double locationY)
{
    mImpl->UpdatePinPosition(pinID, locationX, locationY);
}

void
NBGM_MapViewInternal::SetHBAOParameters(const NBGM_HBAOParameters& parameters)
{
    mImpl->SetHBAOParameters(parameters);
}

void
NBGM_MapViewInternal::SetGlowParameters(const NBGM_GlowParameters& parameters)
{
    mImpl->SetGlowParameters(parameters);
}

void
NBGM_MapViewInternal::SetDPI(float dpi)
{
    mImpl->SetDPI(dpi);
}
