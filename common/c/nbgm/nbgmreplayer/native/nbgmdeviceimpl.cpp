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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbgmdeviceimpl.h"
#include "nbgm.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbrerenderengine.h"
#include "nbrememorystream.h"
#include "nbgmmapviewprotected.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmcontext.h"
#include "nbreglesrenderpal.h"
#include "nbgmmapviewprotected.h"
#include "nbgmnavviewprotected.h"
#include "nbgmconst.h"
#include "nbgmglconfiguration.h"
#include "nbgmrendercontext.h"
#include "nbgmmapviewimpl.h"
#include <string>
#include <sstream>
#include <strstream>
#include "nbrelog.h"
#include "nbgmanimation.h"
#include "nbrefilestream.h"
#include "pallock.h"
#include "palevent.h"
#include <iostream>

struct NBGM_Instance
{
    NBGM_Config  config;
    char*        workspace;
};

class NBGM_CreateTask : public Task
{
public:
    NBGM_CreateTask(NBGMDeviceImpl* device) { nsl_assert(device); mDevice = device; }
    virtual ~NBGM_CreateTask() {};

public:
    virtual void Execute()
    {
        mDevice->CreateNBGMImpl();
        delete this;
    }

private:
    NBGMDeviceImpl* mDevice;
};

class NBGM_DestroyTask : public Task
{
public:
    NBGM_DestroyTask(NBGMDeviceImpl* device) {  nsl_assert(device); mDevice = device; }
    virtual ~NBGM_DestroyTask() {};

public:
    virtual void Execute()
    {
        mDevice->DestroyNBGMImpl();
        delete this;
    }

private:
    NBGMDeviceImpl* mDevice;
};

NBGMDeviceImpl::NBGMDeviceImpl():mPal(NULL),
    mNBGM(NULL),
    mRenderer(NULL),
    mEnigne(NULL),
    mMapView(NULL),
    mAnimation(NULL),
    mResoucePath(NULL),
    mTilesPath(NULL),
    mWidth(0),
    mHeight(0),
    mDpi(96),
    mNBGMCreateEvent(NULL),
    mNBGMDestroyEvent(NULL),
    mKeyLock(NULL),
    mInited(false),
    mRenderSystemType(NBGM_RS_GL),
    mNeedScreenShot(false),
    mScreenShotFilePath(NULL),
    mScreenShotInfo(NULL),
    mImageBuffer(NULL),
    mContinuousDraw(true),
    mKeyMap(NULL)
{
}

NBGMDeviceImpl::~NBGMDeviceImpl(void)
{
}

bool NBGMDeviceImpl::CreateDevice(char* resFolder, char* tilesFolder, int width, int height, int dpi, bool opengl)
{
    if(resFolder == NULL || tilesFolder == NULL || width <= 0 || height <= 0 )
    {
        return false;
    }

    mRenderSystemType = GetRenderSystem(opengl);
    mWidth = width;
    mHeight = height;
    mDpi = dpi;
    mResoucePath = nsl_strdup(resFolder);
    mTilesPath = nsl_strdup(tilesFolder);
    mKeyMap = (bool*)nsl_malloc(256);
    nsl_memset(mKeyMap, 0, 256);

    if(!CreateNativeWindow())
    {
        DestroyDevice();
        return false;
    }

    PAL_Config pc = {0};
	pc.multiThreaded = TRUE;
	mPal = PAL_Create(&pc);
	if (mPal == NULL)
	{
		DestroyDevice();
		return false;
	}

	PAL_Error err = PAL_LockCreate(mPal, &mKeyLock);
	if (err != PAL_Ok)
	{
		DestroyDevice();
		return false;
	}

    DebugLog("NBGMDeviceImpl::CreateDevice finished!");
    return TRUE;
}


bool NBGMDeviceImpl::CreateNBGM()
{
    if(mInited)
    {
        return true;
    }

	PAL_Error err = PAL_EventCreate(mPal, &mNBGMCreateEvent);
	if (err != PAL_Ok)
	{
		DebugLog("CreateNBGM: PAL_EventCreate failed! ");
		return false;
	}

	NBGM_CreateTask *createTask = NBRE_NEW NBGM_CreateTask(this);
	if (!createTask)
	{
		DebugLog("CreateNBGM: NBGM_CreateTask failed! ");
		return false;
	}

	mRenderQueue = shared_ptr<TaskQueue>(NBRE_NEW WorkerTaskQueue(mPal, shared_ptr<std::string>(NBRE_NEW std::string("RenderTaskQueue"))));
	err = mRenderQueue->AddTask(createTask);
	if (err != PAL_Ok)
	{
		DebugLog("CreateNBGM: AddTask failed!");
		return false;
	}

	PAL_EventWaitForEvent(mNBGMCreateEvent);

	mInited = true;
	return true;
}

void NBGMDeviceImpl::DestroyNBGM()
{
    mInited = false;
	PAL_Error err = PAL_EventCreate(mPal, &mNBGMDestroyEvent);
	if(err != PAL_Ok)
	{
		DebugLog("DestroyNBGM: PAL_EventCreate failed! ");
		return;
	}

	NBGM_DestroyTask* destroyTask = NBRE_NEW NBGM_DestroyTask(this);
	if(destroyTask == NULL)
	{
		DebugLog("DestroyNBGM: NBGM_DestroyTask failed!");
		return;
	}

	mRenderQueue->RemoveAllTasks();
	err = mRenderQueue->AddTask(destroyTask);
	if(err != PAL_Ok)
	{
		DebugLog("DestroyNBGM: AddTask failed!");
		return;
	}

	PAL_EventWaitForEvent(mNBGMDestroyEvent);

	if(mNBGMCreateEvent)
	{
		PAL_EventDestroy(mNBGMCreateEvent);
		mNBGMCreateEvent = NULL;
	}

	if(mNBGMDestroyEvent)
	{
		PAL_EventDestroy(mNBGMDestroyEvent);
		mNBGMCreateEvent = NULL;
	}

	mRenderQueue.reset();
}

void NBGMDeviceImpl::DestroyDevice()
{
    DestroyNativeWindow();

    if(mResoucePath)
    {
        nsl_free(mResoucePath);
        mResoucePath = NULL;
    }

    if(mTilesPath)
    {
        nsl_free(mTilesPath);
        mTilesPath = NULL;
    }

    if(mScreenShotFilePath)
    {
        nsl_free(mScreenShotFilePath);
        mScreenShotFilePath = NULL;
    }

    if(mScreenShotInfo)
    {
        nsl_free(mScreenShotInfo);
        mScreenShotInfo = NULL;
    }

    if(mKeyLock)
    {
        PAL_LockDestroy(mKeyLock);
        mKeyLock = NULL;
    }

    if(mPal)
    {
        PAL_Destroy(mPal);
        mPal = NULL;
    }

    if(mImageBuffer)
    {
        nsl_free(mImageBuffer);
        mImageBuffer = NULL;
    }

    if(mKeyMap)
    {
        nsl_free(mKeyMap);
        mKeyMap = NULL;
    }

    DebugLog("NBGMDeviceImpl::DestroyDevice finished!");
}

int NBGMDeviceImpl::IsKeyDown(int c)
{
    int ret = 0;
    if(mKeyMap[c])
    {
        mKeyMap[c] = FALSE;
        ret = 1;
    }
    return ret;
}

void NBGMDeviceImpl::SnapScreenShot(char *outputfile, char *info)
{
    if(outputfile == NULL)
    {
        return;
    }

    PAL_LockLock(mKeyLock);
    if(mScreenShotFilePath)
    {
        nsl_free(mScreenShotFilePath);
        mScreenShotFilePath = NULL;
    }

    mScreenShotFilePath = nsl_strdup(outputfile);

    if(mScreenShotInfo)
    {
        nsl_free(mScreenShotInfo);
        mScreenShotInfo = NULL;
    }

    if(info)
    {
        mScreenShotInfo = nsl_strdup(info);
    }

    PAL_LockUnlock(mKeyLock);
    mNeedScreenShot = true;
}


void NBGMDeviceImpl::CreateNBGMImpl()
{
    NBGM_Config config;
    nsl_memset(&config, 0, sizeof(NBGM_Config));
    config.dpi = (float)mDpi;
    config.pal = mPal;
    config.workSpace = mResoucePath;
    config.nbmOffset = 0;
    config.useRoadBuildingVisibility = TRUE;
    config.preferredLanguageCode = 0;
    config.relativeCameraDistance = 854.0f;  // camera height on zoom level 17, this value comes from Map Layering - Mapkit3D.xlsx.
    config.relativeZoomLevel = 17;
    config.metersPerPixelOnRelativeZoomLevel = 1.194329f;

    NBGM_Initialize(&config);
    NBGM_SetDebugLogSeverity(PAL_LogSeverityInfo);

    NBGM_MapViewConfig c;
    c.drawAvatar = FALSE;
    c.drawFlag = FALSE;
    c.drawSky = TRUE;
    c.renderInterval = 17;
    c.renderSystemType = mRenderSystemType;
    c.renderTaskQueue = mRenderQueue;

    if(!InitContext(c))
    {
    	DebugLog("NBGMDeviceImpl::CreateNBGMImpl InitContext failed!");
        nsl_assert(0);
    }
    PAL_Error err = NBGM_CreateMapView(&c, &mMapView);
    if(err != PAL_Ok)
    {
        DestroyNBGMImpl();
        DebugLog("NBGMDeviceImpl::CreateNBGMImpl NBGM_CreateMapView failed!");
        nsl_assert(0);
    }
    mMapView->Initialize();
    mMapView->SetViewPointDistance(1000.0f);
    mMapView->SetHorizonDistance(100000.0f);
    mMapView->SetViewSize(0, 0, mWidth, mHeight);
    mMapView->SetPerspective(45.0f, static_cast<float>(mWidth)/mHeight);
    mMapView->SetSkyDayNight(TRUE);
    PAL_EventSet(mNBGMCreateEvent);
    DebugLog("NBGMDeviceImpl::CreateNBGMImpl finished!");
}

void NBGMDeviceImpl::DestroyNBGMImpl()
{
    if(mAnimation)
    {
        NBRE_DELETE mAnimation;
        mAnimation = NULL;
    }

    if(mMapView)
    {
        mMapView->Finalize();
        NBRE_DELETE mMapView;
        mMapView = NULL;
    }

    if(mNBGM)
    {
        mNBGM = NULL;
    }

    PAL_EventSet(mNBGMDestroyEvent);
    DebugLog("NBGMDeviceImpl::DestroyNBGMImpl finished!");
}

//---------------------------------------------------------------------------------------------------------------------------


void NBGMDeviceImpl::Invalidate()
{
    nbre_verify(mMapView);
    mMapView->Invalidate();
}

void NBGMDeviceImpl::SetBackground(bool background)
{
    nbre_verify(mMapView);
    mMapView->SetBackground(background);
}

void NBGMDeviceImpl::SetViewCenter(double mercatorX, double mercatorY)
{
    nbre_verify(mMapView);
    mMapView->SetViewCenter(mercatorX, mercatorY);
}

void NBGMDeviceImpl::SetHorizonDistance(double horizonDistance)
{
    nbre_verify(mMapView);
    mMapView->SetHorizonDistance(horizonDistance);
}

void NBGMDeviceImpl::OnTouchEvent(float screenX, float screenY)
{
    nbre_verify(mMapView);
    mMapView->OnTouchEvent(screenX, screenY);
}

void NBGMDeviceImpl::OnPaning(float screenX, float screenY)
{
    nbre_verify(mMapView);
    mMapView->OnPaning(screenX, screenY);
}

void NBGMDeviceImpl::SetRotateAngle(float angle)
{
    nbre_verify(mMapView);
    mMapView->SetRotateAngle(angle);
}

void NBGMDeviceImpl::SetViewPointDistance(float distance)
{
    nbre_verify(mMapView);
    mMapView->SetViewPointDistance(distance);
}

void NBGMDeviceImpl::SetTiltAngle(float angle)
{
    nbre_verify(mMapView);
    mMapView->SetTiltAngle(angle);
}

void NBGMDeviceImpl::SetViewSize(int32 x, int32 y, uint32 width, uint32 height)
{
    nbre_verify(mMapView);
    mMapView->SetViewSize(x, y, width, height);
}

void NBGMDeviceImpl::SetPerspective(float fov, float aspect)
{
    nbre_verify(mMapView);
    mMapView->SetPerspective(fov, aspect);
}

void NBGMDeviceImpl::LoadCommonMaterial(const char* materialName, const char* filePath)
{
    nbre_verify(mMapView);
    mMapView->LoadCommonMaterial(materialName, filePath);
}

void NBGMDeviceImpl::SetCurrentCommonMaterial(const char* materialName)
{
    nbre_verify(mMapView);
    mMapView->SetCurrentCommonMaterial(materialName);
}

void NBGMDeviceImpl::SetSkyDayNight(nb_boolean isDay)
{
    nbre_verify(mMapView);
    mMapView->SetSkyDayNight(isDay);
}

void NBGMDeviceImpl::LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath)
{
    nbre_verify(mMapView);
    mMapView->LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, filePath);
}

void NBGMDeviceImpl::UnLoadTile(const char* nbmName)
{
    nbre_verify(mMapView);
    mMapView->UnLoadTile(nbmName);
}

void NBGMDeviceImpl::UnLoadTiles(const char* nbmNames)
{
    nbre_verify(mMapView);
    int count = 0;
    string *data = SplitString(nbmNames, ';', count);
    if(count > 0)
    {
        std::vector<std::string> vec;
        for(int i = 0; i < count; i++)
        {
            vec.push_back(data[i]);
        }
        mMapView->UnLoadTile(vec);
        NBRE_DELETE_ARRAY data;
    }
}

void NBGMDeviceImpl::SetAvatarLocation(double x, double y, double z, uint64 time, double speed, double accury, double heading)
{
    nbre_verify(mMapView);
    NBGM_Location64 loc = {0};
    loc.position.x = x;
    loc.position.y = y;
    loc.position.z = z;
    loc.time = time;
    loc.speed = speed;
    loc.accuracy = accury;
    loc.heading = heading;
    mMapView->SetAvatarLocation(loc);
}

void NBGMDeviceImpl::SetAvatarScale(float scaleValue)
{
    nbre_verify(mMapView);
    mMapView->SetAvatarScale(scaleValue);
}

void NBGMDeviceImpl::SetAvatarState(int32 state)
{
    nbre_verify(mMapView);
    mMapView->SetAvatarState(NBGM_AvatarState(state));
}

void NBGMDeviceImpl::SetAvatarMode(int32 mode)
{
    nbre_verify(mMapView);
    mMapView->SetAvatarMode(NBGM_AvatarMode(mode));
}

void NBGMDeviceImpl::SelectAndTrackAvatar(float x, float y)
{
    nbre_verify(mMapView);
    mMapView->SelectAndTrackAvatar(x, y);
}

void NBGMDeviceImpl::LoadPinMaterial(const char* pinMaterialName, const char* filePath)
{
    nbre_verify(mMapView);
    mMapView->LoadPinMaterial(pinMaterialName, filePath);
}

void NBGMDeviceImpl::SelectAndTrackPoi(const char* poiId)
{
    nbre_verify(mMapView);
    mMapView->SelectAndTrackPoi(poiId);
}

void NBGMDeviceImpl::UnselectAndStopTrackingPoi(const char* poiId)
{
    nbre_verify(mMapView);
    mMapView->UnselectAndStopTrackingPoi(poiId);
}

void NBGMDeviceImpl::Move(float dx, float dy)
{
    nbre_verify(mMapView);
    mMapView->Move(dx, dy);
}

void NBGMDeviceImpl::SetRotateCenter(float screenX, float screenY)
{
    nbre_verify(mMapView);
    mMapView->SetRotateCenter(screenX, screenY);
}

void NBGMDeviceImpl::Rotate(float angle)
{
    nbre_verify(mMapView);
    mMapView->Rotate(angle);
}

void NBGMDeviceImpl::Zoom(float deltaH)
{
    nbre_verify(mMapView);
    mMapView->Zoom(deltaH);
}

void NBGMDeviceImpl::Tilt(float angle)
{
    nbre_verify(mMapView);
    mMapView->Tilt(angle);
}

float NBGMDeviceImpl::GetTiltAngle()
{
    nbre_verify(mMapView);
    return mMapView->GetTiltAngle();
}

float NBGMDeviceImpl::GetRotateAngle()
{
    nbre_verify(mMapView);
    return mMapView->GetRotateAngle();
}

float NBGMDeviceImpl::GetViewPointDistance()
{
    nbre_verify(mMapView);
    return mMapView->GetViewPointDistance();
}

float NBGMDeviceImpl::GetCameraHeight()
{
    nbre_verify(mMapView);
    return mMapView->GetCameraHeight();
}

//---------------------------------------------------------------------------------------------------------------------------

void NBGMDeviceImpl::ChangeSize(uint32 width, uint32 height)
{
    if(width > 0 && height > 0 && mInited)
    {
        mWidth = width;
        mHeight = height;
        mMapView->SetViewSize(0, 0, mWidth, mHeight);
        mMapView->SetPerspective(45, static_cast<float>(mWidth)/mHeight);
    }
}


void NBGMDeviceImpl::DebugLog(const char* p)
{
    static bool debug_flag = true;
    if(debug_flag && p)
    {
        cout<<"Native: "<< p<<endl;
    }
}

string* NBGMDeviceImpl::SplitString(string str, char split, int& iSubStrs)
{
    int iPos = 0;
    int iNums = 0;
    string strTemp = str;
    string strRight;
    while (iPos != -1)
    {
        iPos = strTemp.find(split);
        if (iPos == -1)
        {
            break;
        }
        strRight = strTemp.substr(iPos + 1, str.length());
        strTemp = strRight;
        iNums++;
    }
    if (iNums == 0)
    {
        iSubStrs = 1;
        return NULL;
    }
    iSubStrs = iNums + 1;
    string* pStrSplit;
    pStrSplit= NBRE_NEW string[iSubStrs];
    strTemp = str;
    string strLeft;
    for (int i = 0; i < iNums; i++)
    {
        iPos = strTemp.find(split);
        strLeft = strTemp.substr(0, iPos);
        strRight = strTemp.substr(iPos + 1, strTemp.length());
        strTemp = strRight;
        pStrSplit[i] = strLeft;
    }
    pStrSplit[iNums] = strTemp;
    return pStrSplit;
}

double NBGMDeviceImpl::bound_mercator(double x)
{
    if (x < -PI)
        x = -PI;
    if (x > PI)
        x = PI;
    return x;
}

void NBGMDeviceImpl::mercator_reverse(double x, double y, double& plat, double& plon)
{
    x = bound_mercator(x);
    y = bound_mercator(y);
    plon = NBRE_Math::RadToDeg(x);
    plat = NBRE_Math::RadToDeg(nsl_atan(nsl_sinh(y)));
}

