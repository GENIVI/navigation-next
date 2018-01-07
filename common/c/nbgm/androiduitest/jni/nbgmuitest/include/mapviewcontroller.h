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


#ifndef _NBGM_MAP_VIEW_CONTROLLER_H
#define _NBGM_MAP_VIEW_CONTROLLER_H

#include "pal.h"
#include "palevent.h"
#include "nbgm.h"
#include "nbgmmapview.h"

class NBGMAndroidEGL;
class WorkerTaskQueue;
struct ANativeWindow;

typedef struct MapViewControllerConfiguration
{
	int preferredLanguageCode;
	bool enableLOD;
	float deviceDPI;
	std::string workSpace;
	NBGM_RenderSystemType renderSystemType;
	bool drawSky;
	bool drawFlag;
	bool drawAvatar;
	int renderInterval;
}MapViewControllerConfiguration;

class MapViewController
{
public:
	MapViewController();
    virtual  ~MapViewController();

public:
    bool Initialize(JNIEnv* env, jobject surface, MapViewControllerConfiguration& configuration);
    void Finalize();
    void Invalidate();
    void SetBackground(bool background);

    void SetViewSize(int x, int y, int width, int height);
    void SetPerspective(float fov, float aspect);

    void SetViewCenter(double mercatorX, double mercatorY);
    void SetViewPointDistance(float distance);

	void SurfaceCreated(ANativeWindow *window);
	void SurfaceDestroyed();
    void SetSurfaceSize(int width, int height);

    void OnTouchEvent(float screenX, float screenY);
    void OnPaning(float screenX, float screenY);

    int LoadCommonMaterial(std::string materialName, std::string filePath);
    int SetCurrentCommonMaterial(std::string materialName);
    int LoadNBMTile(std::string nbmName, int baseDrawOrder, int labelDrawOrder, std::string filePath);
    void UnLoadTile(std::string nbmName);

private:
    bool InitializeMapView();
    void FinalizeMapView();
    void OnSurfaceCreated(ANativeWindow *window);
    void OnDestroySurface();
    void OnChangeSurfaceSize();

private:
    friend class InitMapViewTask;
    friend class FinalizeMapViewTask;
    friend class CreateSurfaceTask;
    friend class DestroySurfaceTask;
    friend class ChangeSurfaceSizeTask;

    PAL_Instance*			mPalInstance;
    PAL_Event*              mInitializeEvent;
    PAL_Event*              mFinalizeEvent;
    PAL_Event*              mEixtEvent;

    NBGM_Instance*          mNBGMInstance;
    NBGM_MapView*           mMapView;
    NBGMAndroidEGL*			mEGL;

    NBGM_DeviceLocation*	mDefaultAvatar;

    int                     mSurfaceWidth;
    int                     mSurfaceHeight;

    shared_ptr<WorkerTaskQueue> mRenderTaskQueue;
};

#endif // ifndef _NBGM_UI_TEST_H_
