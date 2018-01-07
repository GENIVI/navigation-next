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
#include "TestPinLayerView.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include <time.h>
#include "nbreintersection.h"
#include "nbgmTestUtilities.h"
#include "nbrefilestream.h"
#include "nbgmcommon.h"
#include "nbgmrendercontext32.h"
#include "nbuitaskqueue.h"

#include <string>
#include <sstream>
#include <strstream>


enum LAYER_ID
{
    GVA = 100,
    GVR = 110,
    DVA = 120,
    DVR = 130,
    LM3D = 140,
    B3D = 150,
    PIN = 160
};

using namespace std;

CTestPinLayerView::CTestPinLayerView()
{
    m_ScreenHeight = 0;
    m_ScreenWidth = 0;
    mLBtnDown = false;
    m_MapView = NULL;
    m_renderEngine = NULL;
    m_Camera = NULL;
    m_IsTouched = false;
}

CTestPinLayerView::~CTestPinLayerView()
{
    m_MapView->Finalize();
    NBRE_DELETE m_MapView;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
    NBRE_DELETE m_Camera;
}

void  CTestPinLayerView::LoadPinFile(const NBRE_String& filePath, uint8 layerId)
{
    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(GetPal(), filePath.c_str(), 2048);
    NBGM_NBMDataLoadInfo info;
    info.id = filePath;
    info.materialCategoryName = NBGM_PIN_MATERIAL;
    info.baseDrawOrder = layerId;
    info.stream = shared_ptr<NBRE_IOStream>(fs);
    info.enableLog = FALSE;
    m_MapView->LoadNbmData(info);
}

void CTestPinLayerView::ReadPinMaterial()
{
    char* path = "PinTile\\PIN_MATERIAL";
    shared_ptr<NBRE_FileStream> fs(NBRE_NEW NBRE_FileStream(GetPal(), path, 2048));
    NBRE_Vector<NBRE_String> vec;
    m_MapView->LoadExternalMaterial(NBGM_PIN_MATERIAL, "default", fs, vec);
}

int CTestPinLayerView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_Camera);
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    NBGM_MapViewImplConfig c;
    nsl_memset(&c, 0, sizeof(NBGM_MapViewImplConfig));
    c.modelScaleFactor = 100.f;
    c.renderInterval = 20;
    c.viewConfig.drawSky = TRUE;
    c.viewConfig.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(m_renderEngine->Context().mPalInstance));
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());
    c.resourceManager = view->GetResourceManager();
    m_MapView = NBRE_NEW NBGM_MapViewImpl(c, m_renderEngine);
    m_MapView->InitializeEnvironment();
    m_Camera->SetViewCenter(-1.240385589, 0.8175406678);
    m_Camera->SetViewPointDistance(1000.f);
    NBRE_Vector<uint8> layers;
    layers.push_back(PIN);

    //m_MapView->SetLayersDrawOrder(layers);

    ReadPinMaterial();

    return 0;
}

void CTestPinLayerView::OnSize(UINT nType, int cx, int cy)
{
    m_Camera->SetViewSize(0, 0, cx, cy);
    m_MapView->OnSizeChanged(0, 0, cx, cy);
}

void CTestPinLayerView::OnRender()
{
    double centerX, centerY;
    m_Camera->GetViewCenter(centerX, centerY);
    m_MapView->SetViewCenter(centerX, centerY);
    m_MapView->SetViewPointDistance(METER_TO_MERCATOR(m_Camera->GetViewPointDistance()));
    m_MapView->SetTiltAngle(m_Camera->GetTiltAngle());
    m_MapView->SetRotateAngle(m_Camera->GetRotateAngle());
    m_MapView->RenderFrame();
}

void CTestPinLayerView::OnMouseMove(UINT nFlags, CPoint point)
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

void CTestPinLayerView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;
    const float PIN_DEFAULT_HALF_SIZE = 24.0f;

    NBRE_Vector<NBRE_String> pois;
    NBRE_Point2f pos(static_cast<float>(point.x), static_cast<float>(point.y));
    uint32 count = m_MapView->GetInteractedPois(pois, pos);

    if(count == 0)
    {
        m_MapView->UnselectAndStopTrackingPoi("TestPinLayer:0");
        return;
    }

    NBRE_Point2f screenPos;
    m_MapView->GetPoiPosition(pois[0], screenPos);

    if(((pos.x >= screenPos.x - PIN_DEFAULT_HALF_SIZE) || (pos.x <= screenPos.x + PIN_DEFAULT_HALF_SIZE)) &&
       ((pos.y >= screenPos.y - PIN_DEFAULT_HALF_SIZE) || (pos.y <= screenPos.y + PIN_DEFAULT_HALF_SIZE)))
    {
        nb_boolean visible = m_MapView->SelectAndTrackPoi("TestPinLayer:0");
        if (!visible)
        {
            assert(0);
        }
    }
    else
    {
        nb_boolean visible = m_MapView->UnselectAndStopTrackingPoi("TestPinLayer:0");
        if (!visible)
        {
            assert(0);
        }
    }
}

void CTestPinLayerView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

static char* StringToChar(const CString& str)
{
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,str,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText = NBRE_NEW char[dwNum];

    WideCharToMultiByte (CP_ACP,NULL, str,-1,psText,dwNum,NULL,FALSE);
    return psText;
}

void CTestPinLayerView::OnMenu(UINT cmdId)
{
    switch(cmdId )
    {
    case ID_FILE_EM_PIN:
        {
            LoadPinFile("PinTile\\pin-tile-material-index", PIN);
        }
        break;
    case ID_FILE_IM_PIN:
        {
            LoadPinFile("PinTile\\pin-tile-custom-image", PIN);
        }
        break;
    }
}
