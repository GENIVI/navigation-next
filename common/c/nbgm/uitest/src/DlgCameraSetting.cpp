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

// D:\Perforce\scm\client\c\core\nbgm\feature_111107_map3d\uitest\src\DlgCameraSetting.cpp : implementation file
//

#include "stdafx.h"
#include "nbgmtest.h"
#include "DlgCameraSetting.h"


// CDlgCameraSetting dialog

IMPLEMENT_DYNAMIC(CDlgCameraSetting, CDialog)

CDlgCameraSetting::CDlgCameraSetting(ICameraSettingObserver& observer, NBGM_NavCameraSetting navCameraSetting, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCameraSetting::IDD, pParent)
    ,mObserver(observer)
    ,mNavCameraSetting(navCameraSetting)
{

}

CDlgCameraSetting::~CDlgCameraSetting()
{
}

void CDlgCameraSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_CAMERA_HEIGHT, mNavCameraSetting.cameraHeight);
    DDX_Text(pDX, IDC_CAMERA_TO_AVATAR, mNavCameraSetting.cameraToAvatar);
    DDX_Text(pDX, IDC_AVATAR_TO_HORIZON, mNavCameraSetting.avatarToHorizon);
    DDX_Text(pDX, IDC_HFOV, mNavCameraSetting.hFov);
    DDX_Text(pDX, IDC_AVATAR_FROM_BOTTOM, mNavCameraSetting.avatarToBottom);
    DDX_Text(pDX, IDC_AVATAR_SIZE, mNavCameraSetting.avatarScale);
}

BOOL CDlgCameraSetting::OnInitDialog()
{
    CDialog::OnInitDialog();
    CButton* poButton = NULL;
    if(mNavCameraSetting.projectionType == NBGM_CPT_PERSPECTIVE)
    {
        poButton = (CButton*)(GetDlgItem(IDC_RADIO_PERSPECTIVE));
        poButton->SetCheck(1);
    }
    else if(mNavCameraSetting.projectionType == NBGM_CPT_ORTHO)
    {
        poButton = (CButton*)(GetDlgItem(IDC_RADIO_ORTHO));
        poButton->SetCheck(1);
    }
    return TRUE;
}


BEGIN_MESSAGE_MAP(CDlgCameraSetting, CDialog)
    ON_BN_CLICKED(IDSAVE, &CDlgCameraSetting::OnBnClickedSave)
    ON_BN_CLICKED(IDC_RADIO_PERSPECTIVE, &CDlgCameraSetting::OnBnClickedRadioPerspective)
    ON_BN_CLICKED(IDC_RADIO_ORTHO, &CDlgCameraSetting::OnBnClickedRadioOrtho)
END_MESSAGE_MAP()

void CDlgCameraSetting::OnBnClickedSave()
{
    UpdateData(TRUE);
    mObserver.CameraSettingChanged(mNavCameraSetting);
    UpdateData(FALSE);
    this->EndDialog(0);
}

void CDlgCameraSetting::OnBnClickedRadioPerspective()
{
    mNavCameraSetting.projectionType = NBGM_CPT_PERSPECTIVE;
    mObserver.CameraSettingChanged(mNavCameraSetting);
}

void CDlgCameraSetting::OnBnClickedRadioOrtho()
{
    mNavCameraSetting.projectionType = NBGM_CPT_ORTHO;
    mObserver.CameraSettingChanged(mNavCameraSetting);
}

// CDlgCameraSetting message handlers
