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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

#pragma once

#include "NBGMTestView.h"
#include "nbrerenderengine.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgm.h"
#include "nbgmmapviewimpl.h"
#include "nbgmmapcamerahelper.h"

class NBRE_PalHolder;
class NBRE_Win32RenderEnginePlugin;

class CTestPinLayerView : public CNBGMUITestSuit
{
public:
    CTestPinLayerView();
    ~CTestPinLayerView();

public:
    CString GetWindowName(){ return _T("Test Pin Layer");}
    int GetResourceID()    { return IDR_TEST_PIN;}

private:
    virtual void OnRender();
    void CreateScene();
    int OnCreate();
    void OnSize(UINT nType, int cx, int cy);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnMenu(UINT cmdId);
    void OnLButtonUp(UINT nFlags, CPoint point);

    void ReadPinMaterial();
    void LoadPinFile(const NBRE_String& filePath, uint8 layerId);

private:
    int m_ScreenHeight;
    int m_ScreenWidth;

    NBGM_MapViewImpl* m_MapView;

    CPoint m_RightClickedPt;
    NBRE_RenderEngine* m_renderEngine;
    bool mLBtnDown;

    NBGM_MapCameraHelper* m_Camera;
    bool m_IsTouched;
    NBRE_Vector2d m_TouchedPosition;
};