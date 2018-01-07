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

#include "stdafx.h"
#include "nbgmtest.h"
#include "resource.h"
#include "nbgmtestmainview.h"
#include "nbgmTestUtilities.h"
#include "TestGLPal.h"
#include "nbgmTestMainFrame.h"
#include "TestRenderEnginelView.h"
#include "TestMeshView.h"
#include "TestCameraView.h"
#include "TestTexture.h"
#include "TestCameraControllerView.h"
#include "HitTestView.h"
#include "TestBillboardView.h"
#include "TestLightView.h"
#include "TestSkyView.h"
#include "TestRenderTargetView.h"
#include "D3D9View.h"
#include "OpenGLView.h"
#include "TestD3D9Pal.h"
#include "TestCubeTextureView.h"
#include "TestVectorMapView.h"
#include "TestPinLayerView.h"
#include "TestNavRouteLayerView.h"
#include "TestNavVectorView.h"
#include "TestNavSplineLayerView.h"
#include "nbrecommon.h"
#include "RenderSystemDlg.h"
#include "TestMapView.h"

CNBGMTestMainView::CNBGMTestMainView():m_ScreenHeight(NULL), m_ScreenWidth(NULL)
{
}

CNBGMTestMainView::~CNBGMTestMainView()
{
    DestoryPal();
}

BEGIN_MESSAGE_MAP(CNBGMTestMainView, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_TEST_GL_RENDER_PAL, &CNBGMTestMainView::OnTestGLRenderPal)
    ON_COMMAND(ID_TEST_RENDER_ENGINE, &CNBGMTestMainView::OnTestRenderEngine)
    ON_COMMAND(ID_TEST_MESH, &CNBGMTestMainView::OnTestMesh)
    ON_COMMAND(ID_TEST_CAMERA, &CNBGMTestMainView::OnTestCamera)
    ON_COMMAND(ID_TEST_TEXTURE, &CNBGMTestMainView::OnTestTexture)
    ON_COMMAND(ID_TEST_CAMERA_CONTROLLER, &CNBGMTestMainView::OnTestCameraController)
    ON_COMMAND(ID_HIT_TEST, &CNBGMTestMainView::OnHitTest)
    ON_COMMAND(ID_TEST_BILLBOARD, &CNBGMTestMainView::OnTestBillboard)
    ON_COMMAND(ID_TEST_LIGHT, &CNBGMTestMainView::OnTestLight)
    ON_COMMAND(ID_TEST_SKY, &CNBGMTestMainView::OnTestSky)
    ON_COMMAND(ID_TEST_RENDER_TARGET, &CNBGMTestMainView::OnTestRenderTarget)
//    ON_COMMAND(ID_TESTSUITE_TESTD3D9RENDERPAL, &CNBGMTestMainView::OnTestsuiteTestd3d9renderpal)
    ON_COMMAND(ID_TEST_CUBE_TEXTURE, &CNBGMTestMainView::OnTestCubeTexture)
    ON_COMMAND(ID_TEST_VECTORMAP, &CNBGMTestMainView::OnTestVectormap)
    ON_COMMAND(ID_TEST_PIN, &CNBGMTestMainView::OnTestPin)
    ON_COMMAND(ID_TEST_NAV_ROUTE, &CNBGMTestMainView::OnTestNavRoute)
    ON_COMMAND(ID_TESTSUITE_TESTNAVVECTORVIEW, &CNBGMTestMainView::OnTestNavVectorView)
    ON_COMMAND(ID_TEST_NAV_SPLINE, &CNBGMTestMainView::OnTestNavSpline)
    ON_COMMAND(ID_TEST_MAPVIEW, &CNBGMTestMainView::OnTestMapView)
END_MESSAGE_MAP()

BOOL CNBGMTestMainView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
        ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

    return TRUE;
}

int CNBGMTestMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    InitializeNBGM();
    return 0;
}


void CNBGMTestMainView::OnRender()
{
}

void CNBGMTestMainView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    CRect rect;
    GetClientRect(&rect);
    m_ScreenWidth = rect.Width();
    m_ScreenHeight = rect.Height();
}

// Let the user choose which version to run
RenderSystemType CNBGMTestMainView::GetUserChoice()
{
	// If we only support OpenGL, then don't bother with the dialog
	return RST_OPENGL;
	/*
    CRenderSystemDlg dlg;
    int ret = static_cast<int>(dlg.DoModal());
    switch (ret)
    {
    case ID_OPENGL:
        return RST_OPENGL;
    case ID_D3D9:
        return RST_D3D9;
    default:
        return RST_UNKNOWN;
    }*/
}

template<class TestSuit>
void ShowTestView(RenderSystemType type)
{
    if(type == RST_UNKNOWN)
    {
        return;
    }
    CNBGMUITestSuit *suit= NBRE_NEW TestSuit();
    CNBGMTestView *view = NULL;
    switch(type)
    {
    case RST_OPENGL:
        view= new COpenGLView(suit);
        break;
    case RST_D3D9:
        view= new CD3D9View(suit);
        break;
    case RST_OPENGLES:
    case RST_D3D10:
    case RST_D3D11:
    default:
         ASSERT(FALSE);
         break;
    }
    
    CNBGMTestMainFrame* pFrame= new CNBGMTestMainFrame(view);
    if (!pFrame)
        return ;
    // create and load the frame with its resources
    pFrame->LoadFrame(suit->GetResourceID(), WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

    HDC hDC=::GetDC(HWND(NULL));
    int sw=::GetDeviceCaps(hDC,HORZRES);
    int sh=::GetDeviceCaps(hDC,VERTRES);
    ::ReleaseDC(HWND(NULL),hDC);
    int w = TEST_VIEW_DEFAULT_WIDTH;
    int h = TEST_VIEW_DEFAULT_HEIGHT;
    w = w + GetSystemMetrics(SM_CXFRAME) * 2;
    h = h + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYFRAME) * 2;
    pFrame->SetWindowPos(NULL, (sw-TEST_VIEW_DEFAULT_WIDTH)/2, (sh-TEST_VIEW_DEFAULT_HEIGHT)/2, w, h, SWP_SHOWWINDOW | SWP_NOZORDER );
    pFrame->UpdateWindow();
    pFrame->SetWindowText(suit->GetWindowName() + _T(" ( ") + view->GetRenderSystemInfo() + _T(" )"));
    // Force to avoid device reset for D3D 9.0
    if (type == RST_D3D9)
    {
        pFrame->ModifyStyle(WS_THICKFRAME, 0);
    }
}

void CNBGMTestMainView::OnTestsuiteTestd3d9renderpal()
{
    ShowTestView<CTestD3D9Pal>(RST_D3D9);
}

void CNBGMTestMainView::OnTestGLRenderPal()
{
    ShowTestView<CTestGLPal>(RST_OPENGL);
}

void CNBGMTestMainView::OnTestRenderEngine()
{
    ShowTestView<CTestRenderEngineView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestMesh()
{
     ShowTestView<CTestMeshView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestCamera()
{
    ShowTestView<CTestCameraView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestTexture()
{
    ShowTestView<CTestTexture>(GetUserChoice());
}

void CNBGMTestMainView::OnTestCameraController()
{
    ShowTestView<CTestCameraControllerView>(GetUserChoice());
}

void CNBGMTestMainView::OnHitTest()
{
    ShowTestView<CHitTestView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestBillboard()
{
    ShowTestView<CTestBillboardView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestLight()
{
    ShowTestView<CTestLightView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestSky()
{
    ShowTestView<CTestSkyView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestRenderTarget()
{
    ShowTestView<CTestRenderTargetView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestCubeTexture()
{
    ShowTestView<CTestCubeTextureView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestVectormap()
{
    ShowTestView<CTestVectorMapView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestPin()
{
    ShowTestView<CTestPinLayerView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestNavRoute()
{
    ShowTestView<CTestNavRouteLayerView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestNavVectorView()
{
    ShowTestView<CTestNavVectorView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestNavSpline()
{
    ShowTestView<CTestNavSplineLayerView>(GetUserChoice());
}

void CNBGMTestMainView::OnTestMapView()
{
    ShowTestView<CTestMapView>(GetUserChoice());
}