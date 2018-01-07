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
#include "TestMapView.h"
#include "nbgm.h"
#include "nbgmcommon.h"
#include "NBGMTestUtilities.h"
#include <string>
#include <sstream>
#include <strstream>
#include "nbuitaskqueue.h"
#include "nbgmmapcamerahelper.h"

using namespace std;

CTestMapView::CTestMapView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_currentMapView(NULL),
    m_firstMapView(NULL),
    m_secondMapView(NULL),
    mLBtnDown(FALSE),
    mPreviousCameraDistance(0),
    mPrevZoomLevel(0),
    m_currentCamera(NULL),
    m_firstCamera(NULL),
    m_secondCamera(NULL)
{
}

CTestMapView::~CTestMapView()
{
    m_firstMapView->Finalize();
    NBRE_DELETE m_firstMapView;
    m_secondMapView->Finalize();
    NBRE_DELETE m_secondMapView;
    NBRE_DELETE m_firstCamera;
    NBRE_DELETE m_secondCamera;
}

enum LAYER_ID
{
    BR2 = 1,
    BR3 ,
    BR4 ,
    BR5 ,
    BR6 ,
    BR7 ,
    BR8 ,
    BR9 ,
    BR10,
    BR11,
    BR12,
    BR13,
    BR14,
    BR15,
    GVA ,
    GVR ,
    DVA ,
    DVR ,
    B3D ,
    LM3D,
    DVR_LABEL ,
    DVA_LABEL ,
    POINT_LABEL ,
    TRAFFIC ,
    PIN ,
    RASTR,
    BUILDING_2D
};

int CTestMapView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_firstCamera);
    NBGM_CreateMapCameraHelper(&m_secondCamera);
    NBGM_MapViewConfig config;
    config.drawAvatar = TRUE;
    config.drawFlag = FALSE;
    config.drawSky = TRUE;
    config.renderInterval = 10;
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());
    NBGM_ResourceManager* resourceManager = view->GetResourceManager();
    config.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(resourceManager->GetResourceContext().palInstance));
    config.renderContext = resourceManager->GetConfig().renderContext;

    NBGM_CreateMapViewWithSharedResource(&config, resourceManager, &m_firstMapView);
    NBGM_CreateMapViewWithSharedResource(&config, resourceManager, &m_secondMapView);
    m_firstMapView->Initialize();
    m_secondMapView->Initialize();
    NBRE_SetDebugLogSeverity(PAL_LogSeverityMajor);

    m_firstCamera->SetViewPointDistance(1000.0f);
    m_secondCamera->SetViewPointDistance(1000.0f);

    m_firstCamera->SetHorizonDistance(10000.0f);
    m_secondCamera->SetHorizonDistance(10000.0f);

    m_currentMapView = m_firstMapView;
    m_currentCamera = m_firstCamera;

    return 0;
}

void CTestMapView::OnSize(UINT nType, int cx, int cy)
{
    m_firstCamera->SetViewSize(0, 0, cx, cy);
    m_secondCamera->SetViewSize(0, 0, cx, cy);

    m_firstCamera->SetPerspective(45, static_cast<float>(cx)/cy);
    m_secondCamera->SetPerspective(45, static_cast<float>(cx)/cy);
    m_ScreenHeight = cy;
}

void CTestMapView::OnRender()
{
    m_currentCamera->SyncToCamera(m_currentMapView->GetMapCamera());
    m_currentMapView->Invalidate();
}

void CTestMapView::OnMouseMove(UINT nFlags, CPoint point)
{
    if((nFlags & MK_LBUTTON) && (mLBtnDown))
    {
        if(m_IsTouched)
        {
            NBRE_Vector2d position;
            bool isMoved = m_currentCamera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), position.x, position.y);
            if(isMoved)
            {
                NBRE_Vector2d currentViewCenter;
                m_currentCamera->GetViewCenter(currentViewCenter.x, currentViewCenter.y);
                NBRE_Vector2d destinationViewCenter;
                destinationViewCenter = currentViewCenter + m_TouchedPosition - position;
                m_currentCamera->SetViewCenter(destinationViewCenter.x, destinationViewCenter.y);
            }
        }
    }
    if(nFlags & MK_RBUTTON)
    {
        m_currentCamera->Rotate(static_cast<float>(point.x - m_RightClickedPt.x));
        m_currentCamera->Tilt(static_cast<float>(point.y - m_RightClickedPt.y));
        m_RightClickedPt = point;
    }
}

void CTestMapView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;
    m_IsTouched = m_currentCamera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), m_TouchedPosition.x, m_TouchedPosition.y);
}

void CTestMapView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

void CTestMapView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_currentCamera->SetRotateCenter(static_cast<float>(point.x), static_cast<float>(point.y));
    m_RightClickedPt = point;
}

void CTestMapView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

struct NBMTileInfo
{
    int                 x;
    int                 y;
    int                 z;
    uint8              layerType;
    uint8              labelType;
};

void CTestMapView::LoadNBMTile(const char* tilePath, NBMTileInfo& tileInfo)
{
    m_currentCamera->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
    NBGM_Location64 loc = {0};
    loc.position.x = TileToMercatorX(tileInfo.x, 0.5, tileInfo.z);
    loc.position.y = TileToMercatorY(tileInfo.y, 0.5, tileInfo.z);
    loc.position.z = 0;
    loc.heading = 0;
    m_currentMapView->SetAvatarLocation(loc);

    string nbmFile(tilePath);
    clock_t c = clock();
    m_currentMapView->LoadNBMTile(nbmFile, NBGM_COMMON_MATERIAL, "", tileInfo.layerType, 0, tileInfo.labelType, nbmFile, NBGM_NLF_NONE);

    if(m_currentMapView == m_firstMapView)
    {
        m_firstLoadTile.push_back(nbmFile);
    }
    else
    {
        m_secondLoadTile.push_back(nbmFile);
    }

    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadNbmDataByTile = %d", clock()-c);
}

static PAL_Error GetNBMTileInfo(const CString& filePath, NBMTileInfo& tileInfo)
{
    PAL_Error err = PAL_ErrUnsupported;
    //1. try to parse 09371125363DLM.nbm
    if(err != PAL_Ok)
    {
        int npos = filePath.ReverseFind('.');
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1,npos - pos-1);
        CString fileType = fileTitle.Mid(10, 4);
        if(fileType == "AREA")
        {
            tileInfo.layerType = DVA;
            tileInfo.labelType = DVA_LABEL;
            err = PAL_Ok;
        }
        else if(fileType == "ROAD")
        {
            tileInfo.layerType = DVR;
            tileInfo.labelType = DVR_LABEL;
            err = PAL_Ok;
        }
        else if(fileType == "3DLM" || fileType == "LM3D")
        {
            tileInfo.layerType = LM3D;
            tileInfo.labelType = DVA_LABEL;
            err = PAL_Ok;
        }
        else if(fileType == "3DUT")
        {
            tileInfo.layerType = B3D;
            tileInfo.labelType = DVA_LABEL;
            err = PAL_Ok;
        }
        else if(fileType == "DMAT" || fileType == "NMAT")
        {
            err = PAL_Ok;
        }
        else if(fileType == "LABE")
        {
            //tileInfo.layerType = POINT_LABEL;
            err = PAL_Ok;
        }
        else if(fileType == "BR2")
        {
            tileInfo.layerType = BR2;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 2;
            return PAL_Ok;
        }
        else if(fileType == "BR3")
        {
            tileInfo.layerType = BR3;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 3;
            return PAL_Ok;
        }
        else if(fileType == "BR4")
        {
            tileInfo.layerType = BR4;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 4;
            return PAL_Ok;
        }
        else if(fileType == "BR5")
        {
            tileInfo.layerType = BR5;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 5;
            return PAL_Ok;
        }
        else if(fileType == "BR6")
        {
            tileInfo.layerType = BR6;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 6;
            return PAL_Ok;
        }
        else if(fileType == "BR7")
        {
            tileInfo.layerType = BR7;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 7;
            return PAL_Ok;
        }
        else if(fileType == "BR8")
        {
            tileInfo.layerType = BR8;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 8;
            return PAL_Ok;
        }
        else if(fileType == "BR9")
        {
            tileInfo.layerType = BR9;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 9;
            return PAL_Ok;
        }
        else if(fileType == "BR10")
        {
            tileInfo.layerType = BR10;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 10;
            return PAL_Ok;
        }
        else if(fileType == "BR11")
        {
            tileInfo.layerType = BR11;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 11;
            return PAL_Ok;
        }
        else if(fileType == "BR12")
        {
            tileInfo.layerType = BR12;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 12;
            return PAL_Ok;
        }
        else if(fileType == "BR13")
        {
            tileInfo.layerType = BR13;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 13;
            return PAL_Ok;
        }
        else if(fileType == "BR14")
        {
            tileInfo.layerType = BR14;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 14;
            return PAL_Ok;
        }
        else if(fileType == "BR15")
        {
            tileInfo.layerType = BR15;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 15;
            return PAL_Ok;
        }
        else if(fileType == "TRAF")
        {
            tileInfo.layerType = TRAFFIC;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 11;
            return PAL_Ok;
        }
        else if(fileType == "RLLP")
        {
            tileInfo.layerType = POINT_LABEL;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 8;
            return PAL_Ok;
        }
        else if(fileType == "RUTE")
        {
            //tileInfo.layerType = DVRT;
            err = PAL_Ok;
        }
        else if(fileType == "RAST")
        {
            tileInfo.layerType = RASTR;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 15;
            return PAL_Ok;
        }
        else if(fileType == "LBLT")
        {
            tileInfo.layerType = RASTR;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 15;
            return PAL_Ok;
        }
        else if(fileType == "B2DT")
        {
            tileInfo.layerType = BUILDING_2D;
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 15;
            return PAL_Ok;
        }
        else
        {
            err = PAL_ErrUnsupported;
        }
        if(err == PAL_Ok)
        {
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 15;
        }
    }

    //2. try to parse T_DVA_1_NBM_23_5899_12853
    //3. try to parse T_BR2_1_PNG_23_0_0
    if(err != PAL_Ok)
    {
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1, filePath.GetLength() - pos - 1);
        int count=0;
        CString* pStr=SplitString(fileTitle,'_',count);
        if(count != 7)
        {
            err = PAL_ErrUnsupported;
        }
        else if(pStr[0] != _T("T"))
        {
            err = PAL_ErrUnsupported;
        }
        else
        {
            err = PAL_Ok;
            //x
            wstring strX(pStr[5]);
            wstringstream(strX)>>tileInfo.x;
            //y
            wstring strY(pStr[6]);
            wstringstream(strY)>>tileInfo.y;
            //z
            //type
            if(pStr[1] == "DVR")
            {
                tileInfo.layerType = DVR;
                tileInfo.labelType = DVR_LABEL;
                tileInfo.z = 15;
            }
            else if(pStr[1] == "DVA")
            {
                tileInfo.layerType = DVA;
                tileInfo.labelType = DVA_LABEL;
                tileInfo.z = 15;
            }
            else if(pStr[1] == "GVR")
            {
                tileInfo.layerType = GVR;
                tileInfo.labelType = DVR_LABEL;
                tileInfo.z = 12;
            }
            else if(pStr[1] == "GVA")
            {
                tileInfo.layerType = GVA;
                tileInfo.labelType = DVA_LABEL;
                tileInfo.z = 12;
            }
            else if(pStr[1] == "B3D")
            {
                tileInfo.layerType = B3D;
                tileInfo.labelType = DVA_LABEL;
                tileInfo.z = 15;
                err = PAL_Ok;
            }
            else if(pStr[1] == "B2D")
            {
                tileInfo.layerType = DVA;
                tileInfo.labelType = DVA_LABEL;
                tileInfo.z = 15;
                err = PAL_Ok;
            }
            else if(pStr[1] == "3DLM" || pStr[1] == "LM3D")
            {
                tileInfo.layerType = LM3D;
                tileInfo.labelType = DVA_LABEL;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR2")
            {
                tileInfo.layerType = BR2;
                tileInfo.z = 2;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR3")
            {
                tileInfo.layerType = BR3;
                tileInfo.z = 3;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR4")
            {
                tileInfo.layerType = BR4;
                tileInfo.z = 4;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR5")
            {
                tileInfo.layerType = BR5;
                tileInfo.z = 5;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR6")
            {
                tileInfo.layerType = BR6;
                tileInfo.z = 6;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR7")
            {
                tileInfo.layerType = BR7;
                tileInfo.z = 7;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR8")
            {
                tileInfo.layerType = BR8;
                tileInfo.z = 8;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR9")
            {
                tileInfo.layerType = BR9;
                tileInfo.z = 9;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR10")
            {
                tileInfo.layerType = BR10;
                tileInfo.z = 10;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR11")
            {
                tileInfo.layerType = BR11;
                tileInfo.z = 11;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR12")
            {
                tileInfo.layerType = BR12;
                tileInfo.z = 12;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR13")
            {
                tileInfo.layerType = BR13;
                tileInfo.z = 13;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR14")
            {
                tileInfo.layerType = BR14;
                tileInfo.z = 14;
                err = PAL_Ok;
            }
            else if(pStr[1] == "BR15")
            {
                tileInfo.layerType = BR15;
                tileInfo.z = 15;
                err = PAL_Ok;
            }
            else if(pStr[1] == "DVRT")
            {
                tileInfo.layerType = DVR;
                tileInfo.z = 15;
                err = PAL_Ok;
            }
            else
            {
                err = PAL_ErrUnsupported;
            }
        }
        delete[] pStr;
    }

    //try to parse 000010000103RAST
    if(err != PAL_Ok)
    {
        int npos = filePath.ReverseFind('.');
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1,npos - pos-1);
        CString fileType = fileTitle.Mid(12, 4);
        if(fileType == "RAST")
        {
            tileInfo.layerType = BR2;
            err = PAL_Ok;
        }
        else if(fileType == "LBLT")
        {
            tileInfo.layerType = POINT_LABEL;
            err = PAL_Ok;
        }
        else
        {
            err = PAL_ErrUnsupported;
        }
        if(err == PAL_Ok)
        {
            LPCTSTR p = fileTitle;
            wstring strX(p, 5);
            wstring strY(p+5, 5);
            wstring strZ(p+10, 2);
            int x, y, z;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            wstringstream(strZ)>>z;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = z;
        }
    }
    return err;
}

#define FILE_NAME_BUF_SIZE 255*255
void CTestMapView::OnMenu(UINT cmdId)
{
    switch(cmdId )
    {
    case ID_FILE_LOADNBMFILE:
        {
            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, _T("Map NBM Files(*.*)|*.*|All Files(*)|*||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select a NBM file");
            fileDlg.m_ofn.lpstrFile = NBRE_NEW TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            if ( fileDlg.DoModal() == IDOK)
            {
                POSITION pos ( fileDlg.GetStartPosition() );
                while( pos )
                {
                    NBMTileInfo tileInfo;
                    CString path = fileDlg.GetNextPathName( pos );
                    if(GetNBMTileInfo(path, tileInfo) == PAL_Ok)
                    {
                        char* filePath = StringToChar(path);
                        LoadNBMTile(filePath, tileInfo);
                        NBRE_DELETE_ARRAY filePath;
                    }
                }
            }
            NBRE_DELETE_ARRAY fileDlg.m_ofn.lpstrFile;
        }
        break;
    case ID_FILE_LOADCOMMONMATERIAL:
        {
            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Map NBM Files(*.*)|*.*|All Files(*)|*||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select a NBM file");
            fileDlg.m_ofn.lpstrFile = NBRE_NEW TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            if ( fileDlg.DoModal() == IDOK)
            {
                CString path = fileDlg.GetPathName();
                char* filePath = StringToChar(path);
                string materialName(filePath);
                NBGM_MaterialParameters param;
                param.categoryName = NBGM_COMMON_MATERIAL;
                param.materialName = materialName;
                param.filePath = materialName;
                m_currentMapView->LoadExternalMaterial(param, NBGM_MLF_NONE);
                NBRE_DELETE_ARRAY filePath;
            }
            NBRE_DELETE_ARRAY fileDlg.m_ofn.lpstrFile;
        }
        break;
    case ID_FILE_SWITCHMAPVIEW:
        {
            if(m_currentMapView == m_firstMapView)
            {
                m_firstMapView->SetBackground(true);
                m_secondMapView->SetBackground(false);
                m_currentMapView = m_secondMapView;
                m_currentCamera = m_secondCamera;
            }
            else
            {
                m_firstMapView->SetBackground(false);
                m_secondMapView->SetBackground(true);
                m_currentMapView = m_firstMapView;
                m_currentCamera = m_firstCamera;
            }
        }
        break;
    case ID_FILE_UNLOADALL:
        {
            if(m_currentMapView == m_firstMapView)
            {
                for(std::list<std::string>::iterator i = m_firstLoadTile.begin(); i != m_firstLoadTile.end(); ++i)
                {
                    m_currentMapView->UnLoadTile(i->c_str());
                }
                m_firstLoadTile.clear();
            }
            else
            {
                for(std::list<std::string>::iterator i = m_secondLoadTile.begin(); i != m_secondLoadTile.end(); ++i)
                {
                    m_currentMapView->UnLoadTile(i->c_str());
                }
                m_secondLoadTile.clear();
            }
        }
        break;
    case ID_FILE_CLEARALL:
        {
            m_currentMapView->UnLoadAllTiles();
        }
        break;
    default:
        break;
    }
}

void CTestMapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(zDelta > 0)
    {
        m_currentCamera->Zoom(METER_TO_MERCATOR(-m_currentCamera->GetViewPointDistance()/16));
    }
    else
    {
        m_currentCamera->Zoom(METER_TO_MERCATOR(m_currentCamera->GetViewPointDistance()/16));
    }
}

void
CTestMapView::GetMapCenter(double& mx, double& my)
{
   m_currentCamera->GetViewCenter(mx, my);
}

void
CTestMapView::GetCurrentZoomLevel(int8& zlevel)
{
    zlevel = CalcZoomLevel(m_currentCamera->GetViewPointDistance());
}
