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
#include "resource.h"
#include "TestNavVectorView.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbrerendersurface.h"
#include "nbresubmesh.h"
#include "nbretransformation.h"
#include <time.h>
#include "nbgmTestUtilities.h"
#include "nbretextureunit.h"
#include "nbrepngtextureimage.h"
#include "nbretexturemanager.h"
#include "nbrefilestream.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmcommon.h"
#include "nbgmnavvectorview.h"
#include "nbgmmapviewimpl.h"
#include "nbgmrendercontext32.h"
#include <string>
#include <sstream>
#include <strstream>
#include "nbrelog.h"
#include "nbgmbuildutility.h"
#include "nbuitaskqueue.h"
#include "nbgmmapcamerahelper.h"

using namespace std;

static NBRE_PassPtr CreateShaderAndGetDefaultPass(NBRE_ShaderManager& sm, const char* shaderName)
{
    NBRE_PassPtr passPtr = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(passPtr);
    sm.SetShader(shaderName, shaderPtr);
    return passPtr;
}

static NBRE_TexturePtr CreateTexture(NBRE_Context& context, const char* name)
{
    char fullPath[MAX_PATH + 1] = {0};
    char *folder = GetExePath();
    sprintf_s(fullPath, MAX_PATH + 1, "%s\\%s", folder, name);
    NBRE_DELETE []folder;
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*context.mRenderPal, context.mPalInstance, fullPath, 0, FALSE, NBRE_Texture::TT_2D, TRUE); 
    if(texture.get())
    {
        texture->Load();
    }
    return texture;
}

static void CreateMaterial(NBRE_Context& context)
{
    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(*context.mShaderManager, "POI");
        pass1->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        pass1->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        NBRE_TextureUnit unit0;
        unit0.SetTexture(CreateTexture(context, "ACC.png"));
        unit0.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
        pass1->GetTextureUnits().push_back(unit0);
    }

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(*context.mShaderManager, "POI-Selected");
        pass1->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        pass1->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        NBRE_TextureUnit unit0;
        unit0.SetTexture(CreateTexture(context, "ACC_SEL.png"));
        unit0.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
        pass1->GetTextureUnits().push_back(unit0);
    }
}

CTestNavVectorView::CTestNavVectorView()
{
    m_ScreenHeight = 0;
    m_ScreenWidth = 0;

    m_renderEngine = NULL;
    m_NavVectorView = NULL;
    m_Camera = NULL;

    nsl_memset(&m_NavCameraSetting, 0 , sizeof(m_NavCameraSetting));

    mLBtnDown = false;
    m_IsTouched = false;
}

CTestNavVectorView::~CTestNavVectorView()
{
    NBRE_DELETE m_NavVectorView;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
    NBRE_DELETE m_Camera;
}

void CTestNavVectorView::ReadCommonMaterial()
{
    const char* pathDay = "T_DMAT_1_NBM_23_-1_-1";
    const char* pathNight = "common_material_night.nbm";
    shared_ptr<NBRE_FileStream> fsDay(NBRE_NEW NBRE_FileStream(GetPal(), pathDay, 2048));
    m_NavVectorView->LoadCommonMaterial(NBGM_COMMON_MATERIAL, "day", fsDay);

    shared_ptr<NBRE_FileStream> fsNight(NBRE_NEW NBRE_FileStream(GetPal(), pathNight, 2048));
    m_NavVectorView->LoadCommonMaterial(NBGM_COMMON_MATERIAL, "night", fsNight);

    m_NavVectorView->SetCurrentCommonMaterial(NBGM_COMMON_MATERIAL, "day");
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
    GVA ,
    GVR ,
    DVA ,
    DVR ,
    B3D ,
    LM3D,
    POINT_LABEL ,
    TRAFFIC ,
    PIN ,
};

int CTestNavVectorView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_Camera);
    m_renderEngine = CreateRenderEngine(m_RenderPal);

    NBGM_MapViewConfig viewConfig;
    nsl_memset(&viewConfig, 0, sizeof(NBGM_MapViewConfig));
    viewConfig.renderInterval = 20;
    viewConfig.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(m_renderEngine->Context().mPalInstance));
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());

    NBGM_NavViewConfig navViewConfig;
    navViewConfig.mapViewConfig = viewConfig;
    navViewConfig.isUseAnimation = FALSE;

    m_NavVectorView = NBRE_NEW NBGM_NavVectorView(navViewConfig, m_renderEngine, view->GetResourceManager());
    m_NavVectorView->InitializeEnvironment();
    NBRE_SetDebugLogSeverity(PAL_LogSeverityDebug);
    m_Camera->SetViewPointDistance(1000.0f);
    m_NavVectorView->GetMapViewImpl()->SetViewPointDistance(1000.0f/static_cast<float>(RADIUS_EARTH_METERS));

    ReadCommonMaterial();
    CreateMaterial(m_renderEngine->Context());

    m_NavCameraSetting.cameraHeight = 1000;
    m_NavCameraSetting.cameraToAvatar = 90;
    m_NavCameraSetting.avatarToHorizon = 1200;
    m_NavCameraSetting.hFov = 45;
    m_NavCameraSetting.avatarToBottom = 70;
    m_NavCameraSetting.avatarScale = 1.0f;
    m_NavCameraSetting.projectionType = NBGM_CPT_PERSPECTIVE;
    return 0;
}

void CTestNavVectorView::OnSize(UINT nType, int cx, int cy)
{
    m_NavVectorView->OnSizeChanged(0, 0, cx, cy);
    this->CameraSettingChanged(m_NavCameraSetting);
}

void CTestNavVectorView::OnRender()
{
    double centerX, centerY;
    m_Camera->GetViewCenter(centerX, centerY);
    m_NavVectorView->GetMapViewImpl()->SetViewCenter(centerX, centerY);
    m_NavVectorView->GetMapViewImpl()->SetViewPointDistance(METER_TO_MERCATOR(m_Camera->GetViewPointDistance()));
    m_NavVectorView->GetMapViewImpl()->SetTiltAngle(m_Camera->GetTiltAngle());
    m_NavVectorView->GetMapViewImpl()->SetRotateAngle(m_Camera->GetRotateAngle());
    //m_NavVectorView->RenderFrame();
}

void CTestNavVectorView::OnMouseMove(UINT nFlags, CPoint point)
{
    if((nFlags & MK_LBUTTON) && (mLBtnDown))
    {
        if(m_IsTouched)
        {
            NBRE_Vector2d position;
            bool isMoved = m_Camera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), position.x, position.y);
            if(isMoved)
            {
                NBRE_Vector2d currentViewCenter;
                m_Camera->GetViewCenter(currentViewCenter.x, currentViewCenter.y);
                NBRE_Vector2d destinationViewCenter;
                destinationViewCenter = currentViewCenter + m_TouchedPosition - position;
                m_Camera->SetViewCenter(destinationViewCenter.x, destinationViewCenter.y);
            }
        }
        m_NavVectorView->GetMapViewImpl()->RequestRenderFrame();
    }
    if(nFlags & MK_RBUTTON)
    {
        m_Camera->Rotate(static_cast<float>(point.x - m_RightClickedPt.x));
        m_Camera->Tilt(static_cast<float>(point.y - m_RightClickedPt.y));
        m_RightClickedPt = point;
        m_NavVectorView->GetMapViewImpl()->RequestRenderFrame();
    }
}

void CTestNavVectorView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;
    m_IsTouched = m_Camera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), m_TouchedPosition.x, m_TouchedPosition.y);
}

void CTestNavVectorView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

void CTestNavVectorView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_Camera->SetRotateCenter(static_cast<float>(point.x), static_cast<float>(point.y));
    m_RightClickedPt = point;
}

void CTestNavVectorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
    {
    case 'D':
        m_NavVectorView->SetCurrentCommonMaterial(NBGM_COMMON_MATERIAL, "day");
        break;

    case 'N':
        m_NavVectorView->SetCurrentCommonMaterial(NBGM_COMMON_MATERIAL, "night");
        break;

    case 'R':
        m_NavVectorView->SetCurrentCommonMaterial(NBGM_COMMON_MATERIAL, "invalid");
        break;

    default:
        break;
    }
}

void CTestNavVectorView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(zDelta > 0)
    {
        m_Camera->Zoom(-METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/16);
        m_NavVectorView->GetMapViewImpl()->RequestRenderFrame();
    }
    else
    {
        m_Camera->Zoom(METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/16);
        m_NavVectorView->GetMapViewImpl()->RequestRenderFrame();
    }
}

static char* StringToChar(const CString& str)
{
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,str,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText = NBRE_NEW char[dwNum];

    WideCharToMultiByte (CP_ACP,NULL, str,-1,psText,dwNum,NULL,FALSE);
    return psText;
}

struct NBMTileInfo
{
    int                 x;
    int                 y;
    int                 z;
    uint8        layerType;
};

void CTestNavVectorView::LoadNBMTile(const char* tilePath, NBMTileInfo& tileInfo)
{
    m_NavVectorView->GetMapViewImpl()->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
    NBGM_Location64 loc = {0};
    loc.position.x = TileToMercatorX(tileInfo.x, 0.5, tileInfo.z);
    loc.position.y = TileToMercatorY(tileInfo.y, 0.5, tileInfo.z);
    loc.position.z = 0;
    loc.heading = 0;
    m_NavVectorView->GetMapViewImpl()->SetAvatarLocation(loc);

    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), tilePath, 2048);

    clock_t c = clock();
    m_NavVectorView->LoadNBMTile(tilePath, NBGM_COMMON_MATERIAL, tileInfo.layerType, 0, fs);
    m_loadTile.push_back(tilePath);

    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadNbmDataByTile = %d", clock()-c);
}

void CTestNavVectorView::LoadBinTile(const char* tilePath)
{
    m_NavVectorView->GetMapViewImpl()->SetViewCenter(1400, 1800);
    m_NavVectorView->GetMapViewImpl()->SetViewPointDistance(1000);

    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), tilePath, 2048);
    NBGM_NBMDataLoadInfo info;
    info.id = tilePath;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = FALSE;

    clock_t c = clock();
    m_NavVectorView->GetMapViewImpl()->LoadBinData(info);
    m_loadTile.push_back(tilePath);

    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadBinTile = %d", clock()-c);
}


void CTestNavVectorView::CameraSettingChanged(NBGM_NavCameraSetting navCameraSetting)
{
    m_NavCameraSetting = navCameraSetting;
    m_NavVectorView->SetCameraSetting(m_NavCameraSetting);
}

PAL_Error NavGetNBMTileInfo(const CString& filePath, NBMTileInfo& tileInfo)
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
            err = PAL_Ok;
        }
        else if(fileType == "ROAD")
        {
            tileInfo.layerType = DVR;
            err = PAL_Ok;
        }
        else if(fileType == "3DLM")
        {
            tileInfo.layerType = LM3D;
            err = PAL_Ok;
        }
        else if(fileType == "3DUT")
        {
            tileInfo.layerType = B3D;
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
                tileInfo.z = 15;
            }
            else if(pStr[1] == "DVA")
            {
                tileInfo.layerType = DVA;
                tileInfo.z = 15;
            }
            else if(pStr[1] == "GVR")
            {
                tileInfo.layerType = GVR;
                tileInfo.z = 12;
            }
            else if(pStr[1] == "GVA")
            {
                tileInfo.layerType = GVA;
                tileInfo.z = 12;
            }
            else if(pStr[1] == "B3D")
            {
                tileInfo.layerType = B3D;
                tileInfo.z = 15;
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
            else
            {
                err = PAL_ErrUnsupported;
            }
        }
        delete[] pStr;
    }

    return err;
}

#define FILE_NAME_BUF_SIZE 255*255
void CTestNavVectorView::OnMenu(UINT cmdId)
{
    switch(cmdId )
    {
    case ID_LOAD_NBM:
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
                    NavGetNBMTileInfo(path, tileInfo);
                    char* filePath = StringToChar(path);
                    LoadNBMTile(filePath, tileInfo);
                    NBRE_DELETE_ARRAY filePath;
                }
            }
            NBRE_DELETE_ARRAY fileDlg.m_ofn.lpstrFile;
        }
        break;
    case ID_SET_CAMERA:
        {
            CDlgCameraSetting dlg(*this, m_NavCameraSetting, this->GetWindow());
            dlg.DoModal();
        }
        break;
    case ID_LOAD_BIN:
        {
            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, _T("Map BIN Files(*.*)|*.*|All Files(*)|*||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select a BIN file");
            fileDlg.m_ofn.lpstrFile = NBRE_NEW TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            if ( fileDlg.DoModal() == IDOK)
            {
                POSITION pos ( fileDlg.GetStartPosition() );
                while( pos )
                {
                    CString path = fileDlg.GetNextPathName( pos );
                    char* filePath = StringToChar(path);
                    LoadBinTile(filePath);
                    NBRE_DELETE_ARRAY filePath;
                }
            }
            NBRE_DELETE_ARRAY fileDlg.m_ofn.lpstrFile;
        }
        break;
    case ID_UNLOAD_NBM:
        {
            for(std::list<std::string>::iterator i = m_loadTile.begin(); i != m_loadTile.end(); ++i)
            {
                m_NavVectorView->UnLoadTile(i->c_str());
            }
            m_loadTile.clear();
        }
        break;
    }
}



