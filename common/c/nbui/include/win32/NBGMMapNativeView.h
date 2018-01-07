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
 
 @file NBGMMapNativeView.h
 
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#pragma once
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

#include "MapNativeView.h"
#include "MapServicesConfiguration.h"


class MapViewUIInterface;
class MapNativeViewInterfaces;
class OpenGLView;
class MapViewGestureHandler;
/*
    I think this is the native window/view which contains the actual NBGM map view.
*/
class NBGMMapNativeView
{
public:
    NBGMMapNativeView(MapViewUIInterface* mapView, MapNativeView* mapNativeView, int zorder);
    ~NBGMMapNativeView();
    
    /*
        @TODO:
        A lot of these functions are legacy functions and should be removed once we got NBUI for Win32 working.
    */

    void Finallize();
    
    void SetMapNativeView(MapNativeViewInterfaces* nativeMapView);
    void SetCurrentLocation(double lat, double lon);
    void Start();
    NB_Error Initialize();

    void ShowZoomLevelInfo(const char* text);
    void TearDown();
    void SetCovered(bool isCovered);
    void setMapObjects(shared_ptr<nbmap::PinManager> pinManager, nbmap::OptionalLayersAgentPtr pLayersAgent);
    void setAvatarHeading(double heading);
    NBGM_RenderContext* CreateNativeRenderContext();
    void ResetScreen();
    void SetViewSize(int width, int height);
    void UpdateTilt(float tilt);
    void UpdateTrafficTip(TrafficTipType type);

    void CreateView();
    void Render();
    void TaskUpdateViewSize();
    NB_Error SetFont(PAL_Instance* pal, const char* familyName, const char* style, const char* language, uint8** data, uint32* dataSize);

    void CreateNativeWindow();
    void OnTouchEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK UIThreadWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:

    MapNativeViewInterfaces* m_nativeMapView;

    int m_width;
    int m_height;
    bool  m_isExitMap;
    int m_zorder;

    MapViewUIInterface* m_pMapView;
    bool m_isCovered;
    bool m_isTilesLoaded;
    bool m_isPortrait;
    unsigned m_updateFlag;
    MapNativeView* m_mapNativeView;

    /*! Last time stamp/tick count (in milliseconds) when we called Render() the last time. */
    DWORD m_lastTickCount;

    HWND m_window;
    HDC m_dc;
    OpenGLView* m_openGLView;
    MapViewGestureHandler* m_gestureHandler;
};


/*! @} */
