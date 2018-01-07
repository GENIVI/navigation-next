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
 @file     NBGMMapViewAndroid.h
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.                
 
 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems, is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/
/*! @{ */
#ifndef ANDROIDNATIVEHOSTVIEW_H_
#define ANDROIDNATIVEHOSTVIEW_H_

#include "MapViewUIInterface.h"
#include "palgl.h"
#include "paltypes.h"
#include "pallock.h"
#include <pthread.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include "nbgmrendercontext.h"
#include "nbgmrenderconfig.h"
#include "MapViewInterface.h"
#include "MapLegendInfo.h"

typedef enum
{
	RE_OK = 0,
	RE_NO_SURFACE,
	RE_NO_EGL_CONTEXT,
	RE_NO_EGL_SURFACE,
} Render_Error;

typedef enum
{
	S_NO_INITIALIZED = 0,
	S_INITIALIZING,
	S_INITIALIZED,
}NBGMMapView_State;

class MapViewController;

class AndroidNativeHostView
{
public:
    AndroidNativeHostView();
    ~AndroidNativeHostView();

    NB_Error     Initialize(PAL_Instance* pal);
    void*        GetMapViewContext();
    void         DrawFrame();
    void         EnableDebugInfo(bool);
    void         SetDebugInfo(const char*);
    Render_Error Render(bool);
    void         UpdateState(NBGMMapView_State);
    unsigned int*GetDefaultFrameBufferID();
    void         GetViewDimension(int&, int&);
    void         SetUIInterface(MapViewUIInterface* mapView);
    void         UpdateTimestampLabel(uint32);
    void         UpdateButtonState(bool animationCanPlay);
    void         OnMapInitialized();
    void         ShowMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegend);
    void         HideMapLegend();
    void         ShowTip(int tipId);

    NB_Error SetViewSize(int, int);
    NB_Error SetViewPort(int, int);
    NB_Error HandleViewGesture(MapView_GestureParameters*);

    void SetWindow(ANativeWindow* window);
    ANativeWindow* GetWindow();
    PAL_Instance* GetPal();
    void UpdateSurface(ANativeWindow* window);
    void DestroySurface();

    MapViewUIInterface* GetUIInterface() const;
    void SetMapSurfaceViewContext(void* viewContext);
    NBGM_RenderConfig GetRenderConfig()const { return m_renderingConfig; }

private:
    void CreateMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegendInfo);

    ANativeWindow* m_window;
    MapViewUIInterface* m_mapViewUI;
    MapViewController* m_mapviewController;

    // GL Surface width/height
    int m_width;
    int m_height;

    uint32 m_lastUpdateTime;
    PAL_Instance *m_pal;

    bool m_mapLegendCreated;
    jobject m_androidObject;
    NBGM_RenderConfig m_renderingConfig;
};

struct MapSurfaceViewContext
{
    AndroidNativeHostView* nativeHostView;
    jobject frameLayoutObject;
    jobject nativeContentObject;
    jobject nativeMapControllerObject;
    jobject mapControllerObject;
    jobject mapToolbarObject;
    NBGM_RenderConfig renderingConfig;
};

#endif /* ANDROIDNATIVEHOSTVIEW_H_ */
