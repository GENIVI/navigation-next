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
#include "nbrerenderpal.h"
#include "nbgmresourcemanager.h"

#define TEST_VIEW_DEFAULT_WIDTH  800
#define TEST_VIEW_DEFAULT_HEIGHT 600

class CNBGMTestView;

class CNBGMUITestSuit
{
public:
    virtual ~CNBGMUITestSuit(){;}

public:
    void SetWindow(CNBGMTestView* wnd){ mParentWnd = wnd; }
    CNBGMTestView* GetWindow(){ return mParentWnd; }

    void SetRenderer(NBRE_IRenderPal* renderer){m_RenderPal = renderer;}

    virtual CString GetWindowName(){ return _T(""); };
    virtual int GetResourceID()    { return 0; }

    virtual void GetMapCenter(double& x, double& y){}
    virtual void GetCurrentZoomLevel(int8& zlevel){}

public:
    virtual void OnRender() = 0;
    virtual int  OnCreate() = 0;
    virtual void OnSize(UINT nType, int cx, int cy) = 0;
    virtual void InitRenderState(){;}
    virtual void OnTimer(UINT_PTR /*nIDEvent*/){;}
    virtual void OnKeyDown(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/){;}
    virtual void OnMenu(UINT /*cmdId*/)                {;}
    virtual void OnLButtonDown(UINT nFlags, CPoint point){};
    virtual void OnRButtonDown(UINT nFlags, CPoint point){};
    virtual void OnMouseMove(UINT nFlags, CPoint point){};
    virtual void OnLButtonUp(UINT nFlags, CPoint point){};
    virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt){};

protected:
    NBRE_IRenderPal* m_RenderPal;
private:
    CNBGMTestView* mParentWnd;
};

// CNBGMTestView

class CNBGMTestView : public CWnd
{
    DECLARE_DYNAMIC(CNBGMTestView)

public:
    CNBGMTestView(CNBGMUITestSuit *suit);
    virtual ~CNBGMTestView();

public:
    NBRE_IRenderPal* GetRenderer() const;
    NBGM_ResourceManager* GetResourceManager() const;
    void Render();
    void StartRenderCycle(UINT nElapse);
    void EndRenderCycle();
    virtual BOOL OnMenu(UINT cmdId);
    virtual CString GetRenderSystemInfo()const { return _T("");}
    virtual void OnRenderBegine()       = 0;
    virtual void OnRenderEnd()          = 0;

protected:
    DECLARE_MESSAGE_MAP()

    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

private:
    virtual BOOL InitDrawEnv()          = 0;
    virtual void OnSize(int cx, int cy) = 0;
    virtual CString GetInitFailedInfo()const { return _T("Initialize graphics library failed!");}
    void UpdateFps();

private:
    friend class NBGM_RenderContextW32;

protected:
    CNBGMUITestSuit* m_TestSuit;
    BOOL             m_bRenderCycleStarted;
    NBRE_IRenderPal* m_RenderPal;
    NBGM_ResourceManager* m_ResourceManager;
    __int64          m_Freq;
    __int64          m_LastUpdateTime;
    uint32           m_FrameCount;
    uint32           m_Fps;
    BOOL             m_EnableRender;
};


