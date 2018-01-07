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

// nbgmtest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "nbgmtest.h"
#include "nbgmTestMainFrame.h"
#include "nbgmtestmainview.h"
#include "NBGMTestView.h"

// CnbgmtestApp

BEGIN_MESSAGE_MAP(CnbgmtestApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CnbgmtestApp::OnAppAbout)
END_MESSAGE_MAP()


// CnbgmtestApp construction

CnbgmtestApp::CnbgmtestApp()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
    //_CrtSetBreakAlloc(1610); 
}

// The one and only CnbgmtestApp object

CnbgmtestApp theApp;


// CnbgmtestApp initialization

BOOL CnbgmtestApp::InitInstance()
{
    CWinApp::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    //SetRegistryKey(_T("Local AppWizard-Generated Applications"));
    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object
    CNBGMTestMainFrame* pFrame= new CNBGMTestMainFrame(new CNBGMTestMainView);
    if (!pFrame)
        return FALSE;
    m_pMainWnd = pFrame;
    // create and load the frame with its resources
    pFrame->LoadFrame(IDR_MAINFRAME,
        WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
        NULL);

    // The one and only window has been initialized, so show and update it
    pFrame->MoveWindow(0, 0, HTC_WIDTH_P + DET_WIDTH, HTC_HEIGHT_P + DET_HEIGHT);
    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    return TRUE;
}

BOOL CnbgmtestApp::OnIdle(LONG lCount)
{
    if (CWinApp::OnIdle(lCount))
    {
        return TRUE;
    }
    for (uint32 i = 0; i < mTestViews.size(); ++i)
    {
        mTestViews[i]->Render();
    }
    return TRUE;
}

void CnbgmtestApp::AddTestView(CNBGMTestView* suit)
{
    mTestViews.push_back(suit);
}

void CnbgmtestApp::RemoveTestView(CNBGMTestView* suit)
{
    for (TestViewList::iterator i = mTestViews.begin(); i != mTestViews.end(); ++i)
    {
        if (*i == suit)
        {
            mTestViews.erase(i);
            break;
        }
    }
}

// CnbgmtestApp message handlers




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CnbgmtestApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

