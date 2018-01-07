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

    @file nbgmdeviceimpl.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_DEVICE_IMPLE_H_
#define _NBGM_DEVICE_IMPLE_H_


#include "nbtaskqueue.h"
#include "nbgmrendersystemtype.h"


struct PAL_Instance;
struct NBGM_Instance;
class NBRE_IRenderPal;
class NBRE_RenderEngine;
class NBGM_MapView;
class NBGM_Animation;
class NBGM_CreateTask;
class NBGM_DestroyTask;
struct PAL_Event;
struct PAL_Lock;
class NBGM_RenderContext;
struct NBGM_MapViewConfig;

class NBGMDeviceImpl
{
    friend class NBGM_CreateTask;
    friend class NBGM_DestroyTask;

public:
    NBGMDeviceImpl(void);
    virtual ~NBGMDeviceImpl(void);

    virtual bool CreateDevice(char* resFolder, char* tilesFolder, int width, int height, int dpi, bool opengl);
    virtual void DestroyDevice();
    virtual void Run() = 0;

    virtual bool CreateNBGM();
    virtual void DestroyNBGM();
    virtual int IsKeyDown(int nKey);
    virtual void SnapScreenShot(char *outputfile, char *info);

    // -----------------------------------NBGM C API-------------------------------------------------------------------------
    virtual void Invalidate();
    virtual void SetBackground(bool background);
    virtual void SetViewCenter(double mercatorX, double mercatorY);
    virtual void SetHorizonDistance(double horizonDistance);
    virtual void OnTouchEvent(float screenX, float screenY);
    virtual void OnPaning(float screenX, float screenY);
    virtual void SetRotateAngle(float angle);
    virtual void SetViewPointDistance(float distance);
    virtual void SetTiltAngle(float angle);
    virtual void SetViewSize(int32 x, int32 y, uint32 width, uint32 height);
    virtual void SetPerspective(float fov, float aspect);
    virtual void LoadCommonMaterial(const char* materialName, const char* filePath);
    virtual void SetCurrentCommonMaterial(const char* materialName);
    virtual void SetSkyDayNight(nb_boolean isDay);
    virtual void LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath);
    virtual void UnLoadTile(const char* nbmName);
    virtual void UnLoadTiles(const char* nbmNames);
    virtual void SetAvatarLocation(double x, double y, double z, uint64 time, double speed, double accury, double heading);
    virtual void SetAvatarScale(float scaleValue);
    virtual void SetAvatarState(int32 state);
    virtual void SetAvatarMode(int32 mode);
    virtual void SelectAndTrackAvatar(float x, float y);
    virtual  void LoadPinMaterial(const char* pinMaterialName, const char* filePath);
    virtual void SelectAndTrackPoi(const char* poiId);
    virtual void UnselectAndStopTrackingPoi(const char* poiId);
    virtual void Move(float dx, float dy);
    virtual  void SetRotateCenter(float screenX, float screenY);
    virtual void Rotate(float angle);
    virtual void Zoom(float deltaH);
    virtual void Tilt(float angle);

    virtual float GetTiltAngle(); 
    virtual float GetRotateAngle();
    virtual float GetViewPointDistance();
    virtual float GetCameraHeight();

    // ----------------------------------------------------------------------------------------------------------------------

    void ChangeSize(uint32 width, uint32 height);
    static void DebugLog(const char* p);
    static double bound_mercator(double x);
    static void mercator_reverse(double x, double y, double& plat, double& plon);
    static string * SplitString(string str, char split, int& iSubStrs);
    virtual void ScreenShot() = 0;

protected:
    virtual void CreateNBGMImpl();
    virtual void DestroyNBGMImpl();
    virtual bool CreateNativeWindow() = 0;
    virtual void DestroyNativeWindow() = 0;
    virtual bool InitContext(NBGM_MapViewConfig& c) = 0;
    virtual NBGM_RenderSystemType GetRenderSystem(bool opengl) = 0;

protected:
    PAL_Instance*      mPal;
    NBGM_Instance*     mNBGM;  
    NBRE_IRenderPal*   mRenderer;
    NBRE_RenderEngine* mEnigne;
    NBGM_MapView*      mMapView;
    NBGM_Animation*    mAnimation;
    char* mResoucePath;
    char* mTilesPath;
    int   mWidth;
    int   mHeight;
    int   mDpi;
    PAL_Event*         mNBGMCreateEvent;
    PAL_Event*         mNBGMDestroyEvent;
    PAL_Lock*          mKeyLock;
    shared_ptr<TaskQueue> mRenderQueue;
    bool mInited;
    NBGM_RenderSystemType mRenderSystemType;
    bool mNeedScreenShot;
    char* mScreenShotFilePath;
    char* mScreenShotInfo;
    uint8* mImageBuffer;
    bool mContinuousDraw;
    bool *mKeyMap;
};

#endif
