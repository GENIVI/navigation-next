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
#include "TestNavRouteLayerView.h"

#include <time.h>
#include <string>
#include <sstream>
#include <strstream>

#include "nbrecommon.h"
#include "nbgmTestUtilities.h"
#include "nbgmconst.h"
#include "nbgmmapviewimpl.h"
#include "nbrelog.h"
#include "nbrefilestream.h"
#include "nbrepngtextureimage.h"
#include "nbgmrendercontext32.h"
#include "nbgmbuildutility.h"
#include "nbuitaskqueue.h"
#include "nbgmmapcamerahelper.h"

using namespace std;

#define FILE_NAME_BUF_SIZE 255*255

CTestNavRouteLayerView::CTestNavRouteLayerView()
{
    m_ScreenHeight = 0;
    m_ScreenWidth = 0;

    m_NavView = NULL;
    m_GPSProvider = NULL;

    m_renderEngine = NULL;
    m_Camera = NULL;
    mLBtnDown = false;
    mSwitchManeuver = false;

    mPrevZoomLevel = 0;
    m_IsTouched = false;
}

CTestNavRouteLayerView::~CTestNavRouteLayerView()
{
    delete m_GPSProvider;
    NBRE_DELETE m_NavView;

    std::vector<NBGM_VectorRouteData*>:: iterator pRouteData = m_NavData.vectorNavData.routePolyline.begin();
    std::vector<NBGM_VectorRouteData*>:: iterator pRouteDataEnd = m_NavData.vectorNavData.routePolyline.end();

    for(; pRouteData != pRouteDataEnd; ++pRouteData)
    {
        NBRE_DELETE (*pRouteData)->data;
        NBRE_DELETE (*pRouteData);
    }
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
    NBRE_DELETE m_Camera;
}

void CTestNavRouteLayerView::ReadCommonMaterial()
{
    const char* pathDay = "T_DMAT_1_NBM_23_-1_-1";
    const char* pathNight = "common_material_night.nbm";
    shared_ptr<NBRE_FileStream> fsDay(NBRE_NEW NBRE_FileStream(GetPal(), pathDay, 2048));
    m_NavView->LoadCommonMaterial(NBGM_COMMON_MATERIAL, "day", fsDay);

    shared_ptr<NBRE_FileStream> fsNight(NBRE_NEW NBRE_FileStream(GetPal(), pathNight, 2048));
    m_NavView->LoadCommonMaterial(NBGM_COMMON_MATERIAL, "night", fsNight);

    m_NavView->SetCurrentCommonMaterial(NBGM_COMMON_MATERIAL, "day");
}

struct NBMTileInfo
{
    int                 x;
    int                 y;
    int                 z;
    uint8        layerType;
    uint8        labelType;
};

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
    DVR_LABEL ,
    DVA_LABEL ,
    POINT_LABEL ,
    TRAFFIC ,
    PIN ,
};

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

int CTestNavRouteLayerView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_Camera);
    m_renderEngine = CreateRenderEngine(m_RenderPal);

    NBGM_MapViewConfig viewConfig;
    nsl_memset(&viewConfig, 0, sizeof(NBGM_MapViewConfig));
    viewConfig.drawFlag = TRUE;
    viewConfig.drawSky = TRUE;
    viewConfig.drawAvatar = TRUE;
    viewConfig.renderInterval = 20;
    viewConfig.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(m_renderEngine->Context().mPalInstance));
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());

    NBGM_NavViewConfig navViewConfig;
    navViewConfig.mapViewConfig = viewConfig;
    navViewConfig.isUseAnimation = FALSE;

    m_NavView = NBRE_NEW NBGM_NavVectorView(navViewConfig, m_renderEngine, view->GetResourceManager());
    m_NavView->InitializeEnvironment();

    //NBGM_CameraSetting cameraSetting;
    //cameraSetting.cameraHeight = 100.0f/static_cast<float>(RADIUS_EARTH_METERS);
    //cameraSetting.cameraToAvatar = 90.0f/static_cast<float>(RADIUS_EARTH_METERS);
    //cameraSetting.avatarToHorizon = 1200.0f/static_cast<float>(RADIUS_EARTH_METERS);
    //cameraSetting.hfov = 45;
    //cameraSetting.avatarToBottom = 70;
    //m_NavView->GetMapViewImpl()->SetPerspectiveCameraSetting(cameraSetting);

    m_NavView->GetMapViewImpl()->SetIsNavMode(TRUE);

    //int8 zoomLevel = CalcZoomLevel(m_NavView->GetMapViewImpl()->ViewPointDistance());
    //m_NavView->GetMapViewImpl()->SetCurrentZoomLevel(zoomLevel);

    ReadCommonMaterial();
    CreateMaterial(m_renderEngine->Context());

    NBRE_SetDebugLogSeverity(PAL_LogSeverityImportant);

    return 0;
}

void CTestNavRouteLayerView::OnSize(UINT nType, int cx, int cy)
{
    m_NavView->OnSizeChanged(0, 0, cx, cy);
    m_Camera->SetViewSize(0, 0, cx, cy);
    m_ScreenHeight = cy;
}

void CTestNavRouteLayerView::OnRender()
{
    double centerX, centerY;
    m_Camera->GetViewCenter(centerX, centerY);
    m_NavView->GetMapViewImpl()->SetViewCenter(centerX, centerY);
    m_NavView->GetMapViewImpl()->SetViewPointDistance(METER_TO_MERCATOR(m_Camera->GetViewPointDistance()));
    m_NavView->GetMapViewImpl()->SetTiltAngle(m_Camera->GetTiltAngle());
    m_NavView->GetMapViewImpl()->SetRotateAngle(m_Camera->GetRotateAngle());
    m_NavView->RenderFrame();
}

void CTestNavRouteLayerView::OnMouseMove(UINT nFlags, CPoint point)
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
    }
    if(nFlags & MK_RBUTTON)
    {
        m_Camera->Rotate(static_cast<float>(point.x - m_RightClickedPt.x));
        m_Camera->Tilt(static_cast<float>(point.y - m_RightClickedPt.y));
        m_RightClickedPt = point;
    }
}

void CTestNavRouteLayerView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;
    m_IsTouched = m_Camera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), m_TouchedPosition.x, m_TouchedPosition.y);
}

void CTestNavRouteLayerView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

void CTestNavRouteLayerView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_Camera->SetRotateCenter(static_cast<float>(point.x), static_cast<float>(point.y));
    m_RightClickedPt = point;
}

static char* StringToChar(const CString& str)
{
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,str,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText = NBRE_NEW char[dwNum];

    WideCharToMultiByte (CP_ACP,NULL, str,-1,psText,dwNum,NULL,FALSE);
    return psText;
}

static NBGM_VectorRouteData* ReadPolylineFile(const CString& fileName)
{
    CStdioFile file;
    CFileException except; 
    file.Open(fileName, CFile::modeRead|CFile::typeBinary, &except);

    NBGM_VectorRouteData* data = new NBGM_VectorRouteData;
    uint32 inta = static_cast<uint32>(file.GetLength());
    data->count = static_cast<uint32>((file.GetLength() - sizeof(float) * 2) / sizeof(float));
    
    float offset[2] = {0};
    file.Read(offset, sizeof(float) * 2);

    data->offset[0] = offset[0];
    data->offset[1] = offset[1];

    data->data = new float[inta - sizeof(float) * 2];
    file.Read(data->data, inta - sizeof(float) * 2);

    data->maneuverID = static_cast<uint32>( _wtol(file.GetFileName()));

    file.Close();

    return data;
}

void CTestNavRouteLayerView::SetGPSLocation(GPSLocation* loc)
{
    if(loc != NULL)
    {
        m_NavData.vectorNavData.avatarLocation.position.x = loc->x;
        m_NavData.vectorNavData.avatarLocation.position.y = loc->y;
        m_NavData.vectorNavData.avatarLocation.position.z = 0;
        m_NavData.vectorNavData.avatarLocation.heading = 90-loc->heading;
        m_NavData.vectorNavData.viewHeading = static_cast<float>(90-loc->heading);
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_LOCATION);
    }
}

void CTestNavRouteLayerView::OnFileLoadGpsfile()
{
    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("GPS File (*.GPS)|*.GPS||"), GetWindow());
    fileDlg.m_ofn.lpstrTitle = _T("Select GPS file");
    fileDlg.m_ofn.lpstrFile=new TCHAR[FILE_NAME_BUF_SIZE];
    nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
    fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

    INT_PTR re = fileDlg.DoModal();
    if ( re == IDOK)
    {
        CString fileName = fileDlg.GetPathName();
        if(m_GPSProvider == NULL)
        {
            m_GPSProvider = new GPSLocationProvider();
        }
        m_GPSProvider->Reset();
        m_GPSProvider->CreateFromGPSFile(fileName);
        SetGPSLocation(m_GPSProvider->GetNextGPSLocation());

    }
    delete[] fileDlg.m_ofn.lpstrFile;
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

void CTestNavRouteLayerView::LoadNBMTile(const char* tilePath, NBMTileInfo& tileInfo)
{
    m_Camera->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
    m_NavView->GetMapViewImpl()->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
    NBGM_Location64 loc = {0};
    loc.position.x = TileToMercatorX(tileInfo.x, 0.5, tileInfo.z);
    loc.position.y = TileToMercatorY(tileInfo.y, 0.5, tileInfo.z);
    loc.position.z = 0;
    loc.heading = 0;
    m_NavView->GetMapViewImpl()->SetAvatarLocation(loc);

    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), tilePath, 2048);
    NBGM_NBMDataLoadInfo info;
    info.id = tilePath;
    info.materialCategoryName = NBGM_COMMON_MATERIAL;
    info.baseDrawOrder = tileInfo.layerType;
    info.labelDrawOrder = tileInfo.labelType;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = FALSE;

    clock_t c = clock();
    m_NavView->GetMapViewImpl()->LoadNbmData(info);
    m_loadTile.push_back(tilePath);

    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadNbmDataByTile = %d", clock()-c);
}

void CTestNavRouteLayerView::OnMenu(UINT cmdId)
{
    switch(cmdId )
    {
    case ID_FILE_VECTOR_ROUTE:
        {
            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, _T("Route Polyline Files (*.ply)|*.ply||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select route polyline files");
            fileDlg.m_ofn.lpstrFile=new TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            INT_PTR re = fileDlg.DoModal();
            if ( re == IDOK)
            {
                POSITION pos ( fileDlg.GetStartPosition() );
                std::vector<NBGM_VectorRouteData*>& routePolyline = m_NavData.vectorNavData.routePolyline;
                for(std::vector<NBGM_VectorRouteData*>::iterator i = routePolyline.begin(); i != routePolyline.end(); ++i)
                {
                    delete[] (*i)->data;
                    delete (*i);
                }
                routePolyline.clear();
                while( pos )
                {
                    routePolyline.push_back(ReadPolylineFile(fileDlg.GetNextPathName( pos )));
                }
                NBGM_VectorRouteData* routeData = routePolyline.at(0);
                m_NavView->GetMapViewImpl()->SetViewCenter(routeData->offset[0], routeData->offset[1]);
                m_NavData.vectorNavData.startFlagLoc.position.x = routeData->offset[0];
                m_NavData.vectorNavData.startFlagLoc.position.y = routeData->offset[1];
                m_NavData.vectorNavData.startFlagLoc.position.z = 0;
                m_NavData.vectorNavData.startFlagLoc.heading = 0;
                
                routeData = routePolyline.at(1);
                m_NavData.vectorNavData.endFlagLoc.position.x = routeData->offset[0];
                m_NavData.vectorNavData.endFlagLoc.position.y = routeData->offset[1];
                m_NavData.vectorNavData.endFlagLoc.position.z = 0;
                m_NavData.vectorNavData.endFlagLoc.heading = 0;
                m_NavView->UpdateNavData(m_NavData, NBGM_NDF_VECTOR_ROUTE|NBGM_NDF_START_FLAG|NBGM_NDF_END_FLAG);
            }
            delete[] fileDlg.m_ofn.lpstrFile;
        }
        break;

    case ID_FILE_RESET_VECTOR_ROUTE:
        {
			m_NavView->UpdateNavData(m_NavData, NBGM_NDF_RESET_VECTOR_ROUTE);
        }
        break;

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
                    GetNBMTileInfo(path, tileInfo);
                    char* filePath = StringToChar(path);
                    LoadNBMTile(filePath, tileInfo);
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
                m_NavView->UnLoadTile(i->c_str());
            }
            m_loadTile.clear();
        }
        break;
    case ID_LOAD_GPS:
        {
            OnFileLoadGpsfile();
            m_NavView->SetAvatarState(NBGM_AS_NAV);
        }
        break;
    default:
        break;
    }
}

void CTestNavRouteLayerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(zDelta > 0)
    {
        m_Camera->Zoom(- METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/8);
    }
    else
    {
        m_Camera->Zoom( METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/8);
    }

    int8 zoomLevel = CalcZoomLevel(m_Camera->GetViewPointDistance());
    if (zoomLevel != mPrevZoomLevel)
    {
        m_NavView->GetMapViewImpl()->SetCurrentZoomLevel(zoomLevel);
        mPrevZoomLevel = zoomLevel;
    }
}

void CTestNavRouteLayerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
    {
    case 'M':
        m_NavData.currentManeuver = 0;
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_CURRENT_MANEUVER);
        break;
    case ' ':
        {
            if(m_GPSProvider != NULL)
            {
                GPSLocation* loc = m_GPSProvider->GetNextGPSLocation();
                SetGPSLocation(loc);
            }
        }
        break;

	case 'S':
        {
			mSwitchManeuver = !mSwitchManeuver;

			if(mSwitchManeuver)
			{
				m_NavView->ShowManaeuver(0);
			}
			else
			{
				m_NavView->ShowNavMainView();
			}
        }
        break;
    case '1':
        m_NavData.avatarMode = NBGM_AM_NAV_CAR;
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_AVATAR_MODE_FLAG);
        break;
    case '2':
        m_NavData.avatarMode = NBGM_AM_MAP_FOLLOW_ME;
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_AVATAR_MODE_FLAG);
        break;
    case '3':
        m_NavData.avatarMode = NBGM_AM_NAV_BYCICLY;
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_AVATAR_MODE_FLAG);
        break;

    default:
        break;
    }
}
