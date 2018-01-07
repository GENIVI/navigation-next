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
#include "TestVectorMapView.h"
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
#include "nbgmconst.h"
#include "nbgmmapviewimpl.h"
#include <string>
#include <sstream>
#include <strstream>
#include "nbrelog.h"
#include "nbgmanimation.h"
#include "nbgmrendercontext32.h"
#include "nbuitaskqueue.h"
#include "nbgmmapcamerahelper.h"

using namespace std;

CTestVectorMapView::CTestVectorMapView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_mapView(NULL),
    m_Animation(NULL),
    mLBtnDown(FALSE),
    mScreenshot(FALSE),
    mPreviousCameraDistance(0),
    mPrevZoomLevel(0),
    mDebugNode(NBRE_NEW NBRE_Node()),
    m_Camera(NULL),
    m_IsTouched(false)
{
}

CTestVectorMapView::~CTestVectorMapView()
{
    m_mapView->Finalize();
	NBRE_DELETE m_Animation;
    NBRE_DELETE m_mapView;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
    NBRE_DELETE m_Camera;
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
    RASTR,
    GVA ,
    GVR ,
    DVA ,
    DVR ,
    DVRT,
    BUILDING_2D,
    B3D ,
    LM3D,
    LBL ,
    DVR_LABEL ,
    DVA_LABEL ,
    POINT_LABEL ,
    TRAFFIC ,
    PIN ,
    STATIC_POI
};

int CTestVectorMapView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_Camera);
    m_renderEngine = CreateRenderEngine(m_RenderPal);

    NBGM_MapViewImplConfig c;
    c.viewConfig.drawSky = TRUE;
    c.drawNavPoi = FALSE;
    c.viewConfig.drawFlag = FALSE;
    c.viewConfig.drawAvatar = TRUE;
    c.modelScaleFactor = VECTOR_MAP_MODEL_SCALE_FACTOR;
    c.maxAreaBuildingLabels = 0xffffffff;
    c.maxLabels = 0xffffffff;
    c.viewConfig.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(m_renderEngine->Context().mPalInstance));
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());
    c.resourceManager = view->GetResourceManager();
    m_mapView = NBRE_NEW NBGM_MapViewImpl(c, m_renderEngine);

    m_mapView->InitializeEnvironment();
    NBRE_SetDebugLogSeverity(PAL_LogSeverityInfo);

    m_Camera->SetViewPointDistance(1000.0f);
    m_mapView->SetViewPointDistance(1000.0f/static_cast<float>(RADIUS_EARTH_METERS));

	m_Animation = NBRE_NEW NBGM_Animation(m_renderEngine);
	m_Animation->Initialize();

    m_mapView->SetIsNavMode(FALSE);
    m_Camera->SetHorizonDistance(10000.0f);
    m_mapView->SetHorizonDistance(METER_TO_MERCATOR(10000.0f));

    m_mapView->GetRootNode()->AddChild(mDebugNode);

    m_mapView->SetAvatarMode(NBGM_AM_MAP_FOLLOW_ME);
    m_mapView->SetAvatarState(NBGM_AS_MAP_FOLLOW_ME);

    m_mapView->SetCompassPosition(100, 100);
    m_mapView->EnableCompass(TRUE);

    return 0;
}

void CTestVectorMapView::OnSize(UINT nType, int cx, int cy)
{
    m_mapView->OnSizeChanged(0, 0, cx, cy);
    m_mapView->SetPerspective(45, static_cast<float>(cx)/cy);
	m_Animation->OnSizeChanged(0, 0, cx, cy);
    m_Camera->SetViewSize(0, 0, cx, cy);
    m_Camera->SetPerspective(45, static_cast<float>(cx)/cy);
    m_ScreenHeight = cy;


    int8 zoomLevel = CalcZoomLevel(m_Camera->GetViewPointDistance());
    m_mapView->SetCurrentZoomLevel(zoomLevel);

    //m_mapView->Invalidate();
    m_mapView->SetCompassPosition(0.9f * (float)cx, 0.1f * (float)cy);
}

void CTestVectorMapView::OnRender()
{
    clock_t c = clock();

	m_Animation->Move(c/10%1000/1000.0f);

    double centerX = 0, centerY = 0;
    m_Camera->GetViewCenter(centerX, centerY);
    m_mapView->SetViewCenter(centerX, centerY);
    m_mapView->SetViewPointDistance(METER_TO_MERCATOR(m_Camera->GetViewPointDistance()));
    m_mapView->SetTiltAngle(m_Camera->GetTiltAngle());
    m_mapView->SetRotateAngle(m_Camera->GetRotateAngle());

    m_mapView->RenderFrame();
}

void CTestVectorMapView::OnMouseMove(UINT nFlags, CPoint point)
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
        //m_mapView->Invalidate();
    }
    if(nFlags & MK_RBUTTON)
    {
        m_Camera->Rotate(static_cast<float>(point.x - m_RightClickedPt.x));
        m_Camera->Tilt(static_cast<float>(point.y - m_RightClickedPt.y));
        m_RightClickedPt = point;
        //m_mapView->Invalidate();
    }
}

void CTestVectorMapView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;

    m_IsTouched = m_Camera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), m_TouchedPosition.x, m_TouchedPosition.y);

    if(m_mapView->TapCompass(static_cast<float>(point.x), static_cast<float>(point.y)))
    {
        m_Camera->SetRotateAngle(0.0f);
    }
}

void CTestVectorMapView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

void CTestVectorMapView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_Camera->SetRotateCenter(static_cast<float>(point.x), static_cast<float>(point.y));
    m_RightClickedPt = point;
}

void CTestVectorMapView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
    {
    case '1':
        {
            static nb_boolean isSatellite = FALSE;
            isSatellite = !isSatellite;
            m_mapView->SetIsSatelliteMode(isSatellite);
        }
        break;
    case '5':
        {
            DebugFrustumProjection();
        }
    default:
        break;
    }
}

struct NBMTileInfo
{
    int                 x;
    int                 y;
    int                 z;
    uint8              layerType;
    uint8              labelType;
};

void CTestVectorMapView::LoadNBMTile(const char* tilePath, NBMTileInfo& tileInfo)
{
    m_Camera->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
    NBGM_Location64 loc = {0};
    loc.position.x = TileToMercatorX(tileInfo.x, 0.5, tileInfo.z);
    loc.position.y = TileToMercatorY(tileInfo.y, 0.5, tileInfo.z);
    loc.position.z = 0;
    loc.heading = 0;
    m_mapView->SetAvatarLocation(loc);

    vector<std::string> types;
    types.push_back(NBGM_COMMON_MATERIAL);
    types.push_back("STATIC_POI");
    types.push_back("DVRT");

    int tid = 0;
    if (tileInfo.layerType == STATIC_POI)
    {
        tid = 0;
    }
    else if (strstr(tilePath, "DVRT") != NULL)
    {
        tid = 2;
    }

    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), tilePath, 2048);
    NBGM_NBMDataLoadInfo info;
    info.id = tilePath;
    info.materialCategoryName = types[tid];
    info.baseDrawOrder = tileInfo.layerType;
    info.labelDrawOrder = tileInfo.labelType;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = FALSE;
    info.enablePicking = FALSE;
    info.selectMask = PICKING_ELEMENT_MASK_STANDARD;

    if(m_loadTile.find(info.id) == m_loadTile.end())
    {
        m_mapView->LoadNbmData(info);
    }
    else
    {
        m_mapView->UpdateNBMTile(info);
    }
    m_loadTile.insert(tilePath);

    clock_t c = clock();
    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadNbmDataByTile = %d", clock()-c);
}

void CTestVectorMapView::LoadBinTile(const char* tilePath)
{
    m_Camera->SetViewCenter(1400, 1800);
    m_Camera->SetViewPointDistance(1000);

    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), tilePath, 2048);
    NBGM_NBMDataLoadInfo info;
    info.id = tilePath;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = FALSE;

    clock_t c = clock();
    m_mapView->LoadBinData(info);
    m_loadTile.insert(tilePath);

    NBRE_DebugLog(PAL_LogSeverityInfo, "LoadBinTile = %d", clock()-c);
}

static PAL_Error GetNBMTileInfo(const CString& filePath, NBMTileInfo& tileInfo)
{
    PAL_Error err = PAL_ErrUnsupported;
    //1. try to parse 09371125363DLM.nbm
    //   try to parse 0516912505POIS.nbm
    if(err != PAL_Ok)
    {
        int npos = filePath.ReverseFind('.');
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1,npos - pos-1); 
        CString fileType = fileTitle.Right(4);
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
        else if(fileType == "POIS")
        {
            tileInfo.layerType = LBL;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "POI0")
        {
            tileInfo.layerType = STATIC_POI;
            tileInfo.labelType = STATIC_POI;
            CString numParts = fileTitle.Left(fileTitle.GetLength() - 4);
            LPCTSTR p = numParts;
            int len = numParts.GetLength() / 2;
            wstring strX(p, len);
            wstring strY(p+len, len);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 12;
            return PAL_Ok;
        }
        else if(fileType == "POI1")
        {
            tileInfo.layerType = STATIC_POI;
            tileInfo.labelType = STATIC_POI;
            CString numParts = fileTitle.Left(fileTitle.GetLength() - 4);
            LPCTSTR p = numParts;
            int len = numParts.GetLength() / 2;
            wstring strX(p, len);
            wstring strY(p+len, len);
            int x, y;
            wstringstream(strX)>>x;
            wstringstream(strY)>>y;
            tileInfo.x = x;
            tileInfo.y = y;
            tileInfo.z = 16;
            return PAL_Ok;
        }
        else if(fileType == "DVRT")
        {
            tileInfo.layerType = LBL;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "3DLM" || fileType == "LM3D")
        {
            tileInfo.layerType = LM3D;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 15;
            err = PAL_Ok;
        }
        else if(fileType == "3DUT")
        {
            tileInfo.layerType = B3D;
            tileInfo.labelType = DVA_LABEL;
            tileInfo.z = 15;
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
            tileInfo.layerType = LBL;
            tileInfo.labelType = LBL;
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
            tileInfo.labelType = LBL;
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
            CString numParts = fileTitle.Left(fileTitle.GetLength() - 4);
            LPCTSTR p = numParts;
            int len = numParts.GetLength() / 2;
            wstring strX(p, len);
            wstring strY(p+len, len);
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
    //3. try to parse T_BR2_1_PNG_23_0_0_
    if(err != PAL_Ok)
    {
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1, filePath.GetLength() - pos - 1); 
        fileTitle = fileTitle.Trim('_');
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
            else if(pStr[1] == "POI0")
            {
                tileInfo.layerType = STATIC_POI;
                tileInfo.labelType = STATIC_POI;
                tileInfo.z = 2;
            }
            else if(pStr[1] == "POI1")
            {
                tileInfo.layerType = STATIC_POI;
                tileInfo.labelType = STATIC_POI;
                tileInfo.z = 2;
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
            else if(pStr[1] == "LBL0")
            {
                tileInfo.layerType = LBL;
                tileInfo.labelType = LBL;
                tileInfo.z = 3;
                err = PAL_Ok;
            }
            else if(pStr[1] == "LBL1")
            {
                tileInfo.layerType = LBL;
                tileInfo.labelType = LBL;
                tileInfo.z = 7;
                err = PAL_Ok;
            }
            else if(pStr[1] == "LBL2")
            {
                tileInfo.layerType = LBL;
                tileInfo.labelType = LBL;
                tileInfo.z = 12;
                err = PAL_Ok;
            }
            else if(pStr[1] == "LBL3")
            {
                tileInfo.layerType = LBL;
                tileInfo.labelType = LBL;
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
                 //056651312615RAST
                 //09917121193DUT
    
    if(err != PAL_Ok)
    {
        int npos = filePath.ReverseFind('.');
        int pos = filePath.ReverseFind('\\');
        CString fileTitle = filePath.Mid(pos+1, npos - pos-1); 
        CString fileType = fileTitle.Mid(12, 4);
        if(fileType == "RAST")
        {
            tileInfo.layerType = BR2;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "LBLT")
        {
            tileInfo.layerType = POINT_LABEL;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "POIS")
        {
            tileInfo.layerType = LBL;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "3DUT")
        {
            tileInfo.layerType = B3D;
            tileInfo.labelType = LBL;
            err = PAL_Ok;
        }
        else if(fileType == "3DLM")
        {
            tileInfo.layerType = LM3D;
            tileInfo.labelType = LBL;
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
void CTestVectorMapView::OnMenu(UINT cmdId)
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
    case ID_FILE_SWITCHCOMMONMATERIAL:
        {
            static NBRE_Vector<NBRE_String> baseMats;

            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Map NBM Files(*.*)|*.*|All Files(*)|*||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select a NBM file");
            fileDlg.m_ofn.lpstrFile = NBRE_NEW TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            if ( fileDlg.DoModal() == IDOK)
            {
                CString path = fileDlg.GetPathName();
                char* filePath = StringToChar(path);
                path.MakeLower();
                vector<std::string> types;
                types.push_back(NBGM_COMMON_MATERIAL);
                types.push_back("STATIC_POI");
                types.push_back("DVRT");
                int id = 0;
                if (path.Find(_T("pbmat")) >= 0 || path.Find(_T("pmat")) >= 0)
				//	if (path.Find(_T("poi_material.nbm")) >= 0)
                {
                    id = 1;
                }
                else if (path.Find(_T("dvrt")) >= 0)
                {
                    id = 2;
                }
                m_mapView->LoadExternalMaterial(types[id], filePath, 
                    shared_ptr<NBRE_IOStream>(NBRE_NEW NBRE_FileStream(GetPal(), filePath, 1024)), baseMats);
                if (baseMats.size() < 2)
                {
                    string s(filePath);
                    if (std::find(baseMats.begin(), baseMats.end(), s) == baseMats.end())
                    {
                        baseMats.push_back(s);
                    }
                }
                NBRE_DELETE_ARRAY filePath;
            }
            NBRE_DELETE_ARRAY fileDlg.m_ofn.lpstrFile;
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
            for(std::set<std::string>::iterator i = m_loadTile.begin(); i != m_loadTile.end(); ++i)
            {
                m_mapView->UnLoadTile(i->c_str());
            }
            m_loadTile.clear();
        }
        break;
    case ID_SCREENSHOT:
        {
            if(mTileInfo1.subDrawOrder)
            {
                mTileInfo1.subDrawOrder = 0;
                mTileInfo2.subDrawOrder = 1;
            }
            else
            {
                mTileInfo1.subDrawOrder = 1;
                mTileInfo2.subDrawOrder = 0;
            }
            m_mapView->UpdateNBMTile(mTileInfo1);
            m_mapView->UpdateNBMTile(mTileInfo2);
			//mScreenshot = !mScreenshot;
			//if(mScreenshot)
			//{
			//	m_Animation->ShootPicture1(m_mapView->Surface());
			//	m_Animation->ShootPicture2(m_mapView->Surface());
			//	m_Animation->Begin();
			//}
			//else
			//{
	  //          m_mapView->ActiveSurface();
			//}
        }
        break;
    }
}

void CTestVectorMapView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(zDelta > 0)
    {
        m_Camera->Zoom(-METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/4);
        //m_mapView->Invalidate();
    }
    else
    {
        m_Camera->Zoom(METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/4);
        //m_mapView->Invalidate();
    }

    int8 zoomLevel = CalcZoomLevel(m_Camera->GetViewPointDistance());
    if (zoomLevel != mPrevZoomLevel)
    {
        m_mapView->SetCurrentZoomLevel(zoomLevel);
        mPrevZoomLevel = zoomLevel;
    }
    NBRE_DebugLog(PAL_LogSeverityInfo, "OnMouseWheel  zoomlevel = %d", zoomLevel);
}

void CTestVectorMapView::DebugFrustumProjection()
{
    NBRE_Vector<NBGM_Point2d64> position;
    m_Camera->GetFrustumPositionInWorld(-1, position);

    float pos[12] = {
        (float) m_mapView->GetNBGMContext().WorldToModel(position[0].x), (float) m_mapView->GetNBGMContext().WorldToModel(position[0].y), 0.f,
        (float) m_mapView->GetNBGMContext().WorldToModel(position[1].x), (float) m_mapView->GetNBGMContext().WorldToModel(position[1].y), 0.f,
        (float) m_mapView->GetNBGMContext().WorldToModel(position[2].x), (float) m_mapView->GetNBGMContext().WorldToModel(position[2].y), 0.f,
        (float) m_mapView->GetNBGMContext().WorldToModel(position[3].x), (float) m_mapView->GetNBGMContext().WorldToModel(position[3].y), 0.f
    };

    //create vertex data
    NBRE_VertexElement* positionElement = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_HardwareVertexBuffer* vertexBuffer = m_mapView->GetNBGMContext().renderingEngine->Context().mRenderPal->CreateVertexBuffer(sizeof(float)*NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VET_FLOAT3), 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(vertexBuffer)->WriteData(0, sizeof(pos), pos, FALSE);

    NBRE_VertexDeclaration* vertexDeclaration = m_mapView->GetNBGMContext().renderingEngine->Context().mRenderPal->CreateVertexDeclaration();
    vertexDeclaration->GetVertexElementList().push_back(positionElement);
    
    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertexBuffer);
    vertexData->SetVertexDeclaration(vertexDeclaration);

    //create mesh
    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    static uint16 indics[6] = 
    {
        0, 3, 1, 1, 3, 2
    };

    NBRE_HardwareIndexBuffer* indexBuffer = m_mapView->GetNBGMContext().renderingEngine->Context().mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 6, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(indexBuffer)->WriteData(0, sizeof(indics), indics, FALSE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuffer, 0, 6);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    //create shader
    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetColor(NBRE_Color(1.0f, 0.0f, 0.0f, 0.5f));
    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh));
    model->SetShader(NBRE_ShaderPtr(shader));

    //create entity
    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(m_mapView->GetNBGMContext().renderingEngine->Context(), NBRE_ModelPtr(model), 0);
    int32 id = m_mapView->GetOverlayManager().AssignOverlayId(DrawOrderStruct(255, 0, 0));
    ent->SetOverlayId(id);
    ent->SetName("Debug Frustum");

    mDebugNode->DetachAllObjects();
    mDebugNode->AttachObject(NBRE_EntityPtr(ent));
}

void
CTestVectorMapView::GetMapCenter(double& mx, double& my)
{
    float x = m_mapView->GetScreenSize().x * 0.5f;
    float y = m_mapView->GetScreenSize().y * 0.5f;
    m_Camera->ScreenToMapPosition(x, y, mx, my);
}

void
CTestVectorMapView::GetCurrentZoomLevel(int8& zlevel)
{
    zlevel = CalcZoomLevel(m_Camera->GetViewPointDistance());
}
