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
#include "TestNavSplineLayerView.h"

#include <time.h>
#include <string>
#include <sstream>
#include <strstream>

#include "nbgmTestUtilities.h"
#include "nbgmmapviewconfig.h"
#include "nbrecommon.h"
#include "nbrelog.h"
#include "nbgmmapviewimpl.h"
#include "nbgmconst.h"
#include "nbgmrendercontext32.h"
#include "nbuitaskqueue.h"
#include "nbgmmapcamerahelper.h"

using namespace std;

CTestNavSplineLayerView::CTestNavSplineLayerView()
{
    m_ScreenHeight = 0;
    m_ScreenWidth = 0;

    m_NavView = NULL;
    m_ManeuverId = 0;
    m_GPSProvider = NULL;
    m_renderEngine = NULL;
    m_Camera = NULL;

    mLBtnDown = false;
    m_IsTouched = false;
}

CTestNavSplineLayerView::~CTestNavSplineLayerView()
{
    NBRE_DELETE m_NavView;
    NBRE_DELETE m_GPSProvider;

    std::vector<NBGM_SplineData*>:: iterator pEcmData = m_NavData.ecmNavData.splines.begin();
    std::vector<NBGM_SplineData*>:: iterator pEcmDataEnd = m_NavData.ecmNavData.splines.end();

    for(; pEcmData != pEcmDataEnd; ++pEcmData)
    {
        NBRE_DELETE (*pEcmData)->data;
        NBRE_DELETE (*pEcmData);
    }
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
    NBRE_DELETE m_Camera;
}

int CTestNavSplineLayerView::OnCreate()
{
    NBGM_CreateMapCameraHelper(&m_Camera);
    NBGM_MapViewConfig viewConfig;
    nsl_memset(&viewConfig, 0, sizeof(NBGM_MapViewConfig));

    m_renderEngine = CreateRenderEngine(m_RenderPal);

    viewConfig.renderInterval = 20;
    viewConfig.renderTaskQueue = shared_ptr<UiTaskQueue>(NBRE_NEW UiTaskQueue(m_renderEngine->Context().mPalInstance));
    CNBGMTestView* view = static_cast<CNBGMTestView*> (this->GetWindow());

    m_NavView = NBRE_NEW NBGM_NavEcmView(viewConfig, m_renderEngine, view->GetResourceManager());
    m_NavView->InitializeEnvironment();

    m_Camera->SetViewPointDistance(250.0f);
    m_NavView->GetMapViewImpl()->SetViewPointDistance(METER_TO_MERCATOR(250.0f));

    NBRE_SetDebugLogSeverity(PAL_LogSeverityDebug);
    return 0;
}

void CTestNavSplineLayerView::OnSize(UINT nType, int cx, int cy)
{
    m_NavView->OnSizeChanged(0, 0, cx, cy);
    m_Camera->SetViewSize(0, 0, cx, cy);
}

void CTestNavSplineLayerView::OnRender()
{
    double centerX, centerY;
    m_Camera->GetViewCenter(centerX, centerY);
    m_NavView->GetMapViewImpl()->SetViewCenter(centerX, centerY);
    m_NavView->GetMapViewImpl()->SetViewPointDistance(METER_TO_MERCATOR(m_Camera->GetViewPointDistance()));
    m_NavView->GetMapViewImpl()->SetTiltAngle(m_Camera->GetTiltAngle());
    m_NavView->GetMapViewImpl()->SetRotateAngle(m_Camera->GetRotateAngle());
    m_NavView->RenderFrame();
}

void CTestNavSplineLayerView::OnMouseMove(UINT nFlags, CPoint point)
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

void CTestNavSplineLayerView::OnLButtonDown(UINT nFlags, CPoint point)
{
    mLBtnDown = true;
    m_IsTouched = m_Camera->ScreenToMapPosition(static_cast<float>(point.x), static_cast<float>(point.y), m_TouchedPosition.x, m_TouchedPosition.y);
}

void CTestNavSplineLayerView::OnLButtonUp(UINT nFlags, CPoint point)
{
    mLBtnDown = false;
}

void CTestNavSplineLayerView::OnRButtonDown(UINT nFlags, CPoint point)
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

static NBGM_SplineData* ReadSplineFile(const CString& fileName)
{
    CStdioFile file;
    CFileException except; 
    file.Open(fileName, CFile::modeRead|CFile::typeBinary, &except);

    NBGM_SplineData* data = new NBGM_SplineData;
    data->size = static_cast<uint32>(file.GetLength());
    data->data = new uint8[data->size];

    file.Read(data->data, data->size);
    file.Close();

    return data;
}

#define FILE_NAME_BUF_SIZE 255*255
void CTestNavSplineLayerView::OnMenu(UINT cmdId)
{
    switch(cmdId )
    {
    case ID_FILE_LOAD_SPLINE:
        {
            CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY, _T("Route Spline Files (*.spl)|*.spl||"), GetWindow());
            fileDlg.m_ofn.lpstrTitle = _T("Select route spline files");
            fileDlg.m_ofn.lpstrFile=new TCHAR[FILE_NAME_BUF_SIZE];
            nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
            fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

            INT_PTR re = fileDlg.DoModal();
            if ( re == IDOK)
            {
                POSITION pos ( fileDlg.GetStartPosition() );
                std::vector<NBGM_SplineData*>& routeSpline = m_NavData.ecmNavData.splines;
                for(std::vector<NBGM_SplineData*>::iterator i = routeSpline.begin(); i != routeSpline.end(); ++i)
                {
                    delete[] (*i)->data;
                    delete (*i);
                }
                routeSpline.clear();

                while( pos )
                {
                    routeSpline.push_back(ReadSplineFile(fileDlg.GetNextPathName( pos )));
                }

                NBGM_EcmManeuver maneuver;

                maneuver.id = 0;
                maneuver.position.x = 0.035720404f / ECM_MAP_MODEL_SCALE_FACTOR;
                maneuver.position.y = -0.53067797f / ECM_MAP_MODEL_SCALE_FACTOR;
                maneuver.position.z = 0.19054602f / ECM_MAP_MODEL_SCALE_FACTOR;
                m_NavData.ecmNavData.maneuvers.push_back(maneuver);

                m_NavView->GetMapViewImpl()->SetViewCenter(0.0034530901f / ECM_MAP_MODEL_SCALE_FACTOR , -1.0897200f / ECM_MAP_MODEL_SCALE_FACTOR );
                m_NavView->UpdateNavData(m_NavData, NBGM_NDF_ECM_ROUTE);
            }
            delete[] fileDlg.m_ofn.lpstrFile;
        }
        break;

    case ID_LOAD_GPS_FILE:
        {
            OnFileLoadGpsfile();
            m_NavView->GetMapViewImpl()->SetAvatarState(NBGM_AS_NAV);
        }
        break;

    default:
        break;
    }
}

void CTestNavSplineLayerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if(zDelta > 0)
    {
        m_Camera->Zoom(- METER_TO_MERCATOR(m_Camera->GetViewPointDistance())/8);
    }
    else
    {
        m_Camera->Zoom( (m_Camera->GetViewPointDistance())/8);
    }
}


void CTestNavSplineLayerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch(nChar)
    {
    // set current maneuver
    case 'M':
        m_NavData.currentManeuver = m_ManeuverId;
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_CURRENT_MANEUVER);
        ++m_ManeuverId;
        break;

    case ' ':
        if(m_GPSProvider != NULL)
        {
            GPSLocation* loc = m_GPSProvider->GetNextGPSLocation();
            SetGPSLocation(loc);
        }
        break;

    default:
        break;
    }
}

void CTestNavSplineLayerView::OnFileLoadGpsfile()
{
    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("GPS File (*.GPS)|*.GPS||"), GetWindow());
    fileDlg.m_ofn.lpstrTitle = _T("Select GPS file");
    fileDlg.m_ofn.lpstrFile=new TCHAR[FILE_NAME_BUF_SIZE];
    nsl_memset(fileDlg.m_ofn.lpstrFile,0,FILE_NAME_BUF_SIZE);
    fileDlg.m_ofn.nMaxFile = FILE_NAME_BUF_SIZE;

    INT_PTR re = fileDlg.DoModal();
    if ( re == IDOK)
    {
        CString fileName = fileDlg.GetFileName();
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

void CTestNavSplineLayerView::SetGPSLocation(GPSLocation* loc)
{
    if(loc != NULL)
    {
        m_NavData.ecmNavData.avatarLocation.position.x = loc->x;
        m_NavData.ecmNavData.avatarLocation.position.y = loc->y;
        m_NavData.ecmNavData.avatarLocation.position.z = loc->z;
        m_NavData.ecmNavData.avatarLocation.heading = 90.0 - loc->heading;
        m_NavData.ecmNavData.viewHeading = static_cast<float>(90-loc->heading);
        m_NavView->UpdateNavData(m_NavData, NBGM_NDF_LOCATION);
    }
}
