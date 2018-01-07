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

#include "openglview.h"
#include "pal.h"
#include "nbgm.h"
#include "NBGMTestView.h"

enum RenderSystemType
{
    RST_OPENGL,
    RST_OPENGLES,
    RST_D3D9,
    RST_D3D10,
    RST_D3D11,
    RST_UNKNOWN
};

class CNBGMTestMainView : public CWnd
{
    // Construction
public:
    CNBGMTestMainView();
    virtual ~CNBGMTestMainView();

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnRender();

private:
    PAL_Error CreateNBGM();
    RenderSystemType GetUserChoice();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTestGLRenderPal();
    afx_msg void OnTestRenderEngine();
    afx_msg void OnTestMesh();
    afx_msg void OnTestCamera();
    afx_msg void OnTestCameraController();
    afx_msg void OnTestTexture();
    afx_msg void OnHitTest();
    afx_msg void OnTestBillboard();
    afx_msg void OnTestLight();
    afx_msg void OnTestSky();
    afx_msg void OnTestRenderTarget();
    afx_msg void OnTestsuiteTestd3d9renderpal();
    afx_msg void OnTestCubeTexture();
    afx_msg void OnTestVectormap();
    afx_msg void OnTestPin();
    afx_msg void OnTestNavRoute();
    afx_msg void OnTestNavVectorView();
    afx_msg void OnTestNavSpline();
    afx_msg void OnTestMapView();

private:
    //NBGM resource
    int m_ScreenHeight;
    int m_ScreenWidth;
};

