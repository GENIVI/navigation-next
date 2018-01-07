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

// NBGMTestView.cpp : implementation file
//

#include "stdafx.h"
#include "nbgmtest.h"
#include "NBGMTestView.h"


// CNBGMTestView

IMPLEMENT_DYNAMIC(CNBGMTestView, CWnd)

CNBGMTestView::CNBGMTestView(CNBGMUITestSuit *suit):m_TestSuit(suit),m_bRenderCycleStarted(FALSE),m_RenderPal(NULL),m_ResourceManager(NULL),m_EnableRender(FALSE)
{
    ASSERT(m_TestSuit);
    m_TestSuit->SetWindow(this);
    m_TestSuit->SetRenderer(m_RenderPal);
    CnbgmtestApp* testApp = static_cast<CnbgmtestApp*>(AfxGetApp());
    testApp->AddTestView(this);
    QueryPerformanceFrequency((LARGE_INTEGER*)&m_Freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&m_LastUpdateTime);
    m_Fps = 0;
    m_FrameCount = 0;
}

CNBGMTestView::~CNBGMTestView()
{
    CnbgmtestApp* testApp = static_cast<CnbgmtestApp*>(AfxGetApp());
    testApp->RemoveTestView(this);
}

BEGIN_MESSAGE_MAP(CNBGMTestView, CWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CNBGMTestView message handlers

int CNBGMTestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (InitDrawEnv() == FALSE)
    {
        MessageBox(GetInitFailedInfo());
        return -1;
    }
    m_TestSuit->SetRenderer(m_RenderPal);

    if (m_TestSuit->OnCreate() == -1)
    {
        return -1;
    }

    StartRenderCycle(0);
    return 0;
}

void CNBGMTestView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (cx > 1 && cy > 1)
    {
        OnSize(cx, cy);
        m_TestSuit->OnSize(nType, cx, cy);
    }
}

void CNBGMTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_TestSuit->OnKeyDown(nChar, nRepCnt, nFlags);
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CNBGMTestView::OnTimer(UINT_PTR nIDEvent)
{
    m_TestSuit->OnTimer(nIDEvent);
    CWnd::OnTimer(nIDEvent);
}

static double
SpatialBoundMercator(double x)
{
    if (x < -PI)
        x = -PI;
    if (x > PI)
        x = PI;
    return x;
}

void
SpatialConvertMercatorToLatLong(double xMercator, double yMercator, double& latitude, double& longitude)
{
    xMercator = SpatialBoundMercator(xMercator);
    yMercator = SpatialBoundMercator(yMercator);

    longitude = NBRE_Math::RadToDeg(xMercator);
    latitude = NBRE_Math::RadToDeg(atan(sinh(yMercator)));
}

void CNBGMTestView::UpdateFps()
{
    __int64 now = 0;
    ++m_FrameCount;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
    float secondsPassed = static_cast<float>(now - m_LastUpdateTime) / m_Freq;
    if (secondsPassed >= 1.0f)
    {
        m_LastUpdateTime = now;
        m_Fps = m_FrameCount;
        m_FrameCount = 0;
    }

    double mapCenterX = 0.0;
    double mapCenterY = 0.0;
    double longtitude = 0.0;
    double lantitude = 0.0;

    m_TestSuit->GetMapCenter(mapCenterX, mapCenterY);

    SpatialConvertMercatorToLatLong(mapCenterX, mapCenterY, lantitude, longtitude);
    CString strLantitude;
    strLantitude.Format(_T(" Lan: %06f"), lantitude);
    CString strLongtitude;
    strLongtitude.Format(_T(" Long: %06f"), longtitude);

    int8 zoomLevel = 0;
    m_TestSuit->GetCurrentZoomLevel(zoomLevel);
    CString strZoomLevel;
    strZoomLevel.Format(_T(" ZLevel: %03u"), zoomLevel);

    CString fps;
    fps.Format(_T(" FPS: %03u"), m_Fps);
    this->GetParentFrame()->SetWindowText(m_TestSuit->GetWindowName() + _T(" ( ") + GetRenderSystemInfo() + _T(" )") + fps + _T("  ")
                                          + strLantitude + _T("  ") + strLongtitude + _T("  ") + strZoomLevel);
}

NBRE_IRenderPal* CNBGMTestView::GetRenderer() const
{
    return m_RenderPal;
}

NBGM_ResourceManager* CNBGMTestView::GetResourceManager() const
{
    return m_ResourceManager;
}

void CNBGMTestView::Render()
{
    m_TestSuit->OnRender();

    UpdateFps();
}

void CNBGMTestView::StartRenderCycle(UINT nElapse)
{
    if(!m_bRenderCycleStarted)
    {
        SetTimer(1, nElapse, NULL);
        m_bRenderCycleStarted = TRUE;
    }
}

void CNBGMTestView::EndRenderCycle()
{
    if(m_bRenderCycleStarted)
    {
        KillTimer(1);
        m_bRenderCycleStarted = FALSE;
    }
}

BOOL CNBGMTestView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (nCode == CN_COMMAND)
    {
        return TRUE;
    }
    return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CNBGMTestView::OnMenu(UINT cmdId)
{
    m_TestSuit->OnMenu(cmdId);
    return TRUE;
}

void CNBGMTestView::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_TestSuit->OnLButtonDown(nFlags, point);

    CWnd::OnLButtonDown(nFlags, point);
}

void CNBGMTestView::OnMouseMove(UINT nFlags, CPoint point)
{
    m_TestSuit->OnMouseMove(nFlags, point);

    CWnd::OnMouseMove(nFlags, point);
}

void CNBGMTestView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_TestSuit->OnRButtonDown(nFlags, point);

    CWnd::OnRButtonDown(nFlags, point);
}

void CNBGMTestView::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_TestSuit->OnLButtonUp(nFlags, point);

    CWnd::OnLButtonUp(nFlags, point);
}

BOOL CNBGMTestView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_TestSuit->OnMouseWheel(nFlags, zDelta, pt);

    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
