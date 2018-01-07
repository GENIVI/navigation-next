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

    @file nbgmdevicewin32.h
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
#ifndef _NBGM_DEVICE_WIN32_H_
#define _NBGM_DEVICE_WIN32_H_

#include <Windows.h>
#include "d3d9.h"
#include "..\nbgmdeviceimpl.h"


class NBGMDeivceWin32: public NBGMDeviceImpl
{
public:
    NBGMDeivceWin32(void);
    virtual ~NBGMDeivceWin32(void);

    virtual void Run();
    virtual bool CreateNativeWindow();
    virtual void DestroyNativeWindow();
    virtual void ScreenShotOpenGL();
    virtual void ScreenShotD3D9();
    virtual bool InitContext(NBGM_MapViewConfig& c);
    virtual NBGM_RenderSystemType GetRenderSystem(bool opengl);

    virtual void LoadCommonMaterial(const char* materialName, const char* filePath);
    virtual void LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath);

    void OnSizeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnMouseMove(UINT nFlags, float x, float y);
    void OnLButtonDown(UINT nFlags, float x, float y);
    void OnLButtonUp(UINT nFlags, float x, float y);
    void OnRButtonDown(UINT nFlags, float x, float y);
    void OnMouseWheel(UINT nFlags, short zDelta, float x, float y);
    void OnCreate(HWND hWnd);
    void OnTimer(UINT nID);
    void UpdateFPS();
    void UpdateStatusBar();
    void ScreenShot();

public:
    HINSTANCE mInstance;
    HWND mWnd;
    HWND mStatusBar;
    float m_RightClickedPtx;
    float m_RightClickedPty;
    float mMouseX;
    float mMouseY;
    bool mLBtnDown;
    float mPreviousCameraDistance;
    int8 mPrevZoomLevel;
    int mBarHeight;
    IDirect3DDevice9* m_D3D9Device;
};

#endif
