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
#include "nbgm.h"
#include "nbgmnavvectorview.h"
#include "nbgmnavdata.h"
#include "gpsprovider.h"

struct NBMTileInfo;
class NBGM_MapCameraHelper;

class CTestNavRouteLayerView : public CNBGMUITestSuit
{
public:
    CTestNavRouteLayerView();
    ~CTestNavRouteLayerView();

public:
    CString GetWindowName(){ return _T("Test Nav Route Layer");}
    int GetResourceID()    { return IDR_TEST_NAV_ROUTE;}

private:
    virtual void OnRender();
    int OnCreate();
    void OnSize(UINT nType, int cx, int cy);
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnMenu(UINT cmdId);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnRButtonDown(UINT nFlags, CPoint point);
    void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void LoadNBMTile(const char* tilePath, NBMTileInfo& tileInfo);
    void ReadCommonMaterial();
    void OnFileLoadGpsfile();
    void SetGPSLocation(GPSLocation* loc);

private:
    int m_ScreenHeight;
    int m_ScreenWidth;

    NBGM_NavVectorView* m_NavView;
    NBGM_NavData m_NavData;

    GPSLocationProvider* m_GPSProvider;

    CPoint m_RightClickedPt;
    NBRE_RenderEngine* m_renderEngine;
    bool mLBtnDown;
    std::list<std::string> m_loadTile;
    bool mSwitchManeuver;

    int8 mPrevZoomLevel;

    NBGM_MapCameraHelper* m_Camera;
    bool m_IsTouched;
    NBRE_Vector2d m_TouchedPosition;
};
