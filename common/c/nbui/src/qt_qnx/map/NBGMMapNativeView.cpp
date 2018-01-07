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

/*!--------------------------------------------------------------------------
    @file     NBGMMapNativeView.cpp
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <bps/screen.h>
#include <bps/bps.h>
#include <EGL/egl.h>
#include <screen/screen.h>
#include <sys/platform.h>
#include "input/screen_helpers.h"
#include "palstdlib.h"
#include "palfont.h"
#include "MapNativeView.h"
#include "NBGMMapNativeView.h"
#include "MapViewUIInterface.h"
#include "MapNativeViewInterfaces.h"
#include "MapViewInterface.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "paldisplay.h"
#include <bb/platform/geo/Point>
#include "nbgmnativerendercontext.h"
#include "paltestlog.h"

using namespace bb;
using namespace platform;
using namespace geo;

enum m_updateFlags
{
    NBGMVIEW_UPDATE_NIGHTMODE = 0x1
    , NBGMVIEW_UPDATE_MANEUVER = 0x2
    , NBGMVIEW_UPDATE_MAPDATA = 0x4
    , NBGMVIEW_UPDATE_VIEWSIZE = 0x8
    , NBGMVIEW_UPDATE_ZOOM = 0x10
};
static void UpdateViewTaskCallback(void* param);

NBGMMapNativeView::NBGMMapNativeView(MapViewUIInterface* mapView, MapNativeView* mapNativeView, int zorderLevel)
    : m_pMapView(mapView),
      m_mapView(NULL),
      m_pMapContainer(NULL),
      m_mapNativeView(mapNativeView),
      nativeMapView(NULL),
      m_renderEngine(NULL),
      m_isCovered(false),
      m_renderEnginePrivate(NULL),
      m_zorder(zorderLevel)
{
    m_id = "NBUI";
    PAL_DisplayInitialize();
    m_width = PAL_DisplayGetWidth();
    m_height = PAL_DisplayGetHeight();
    m_lastTimeval.tv_sec = 0;
    m_lastTimeval.tv_usec = 0;
    CreateView();
}

NBGMMapNativeView::~NBGMMapNativeView()
{
}

void
NBGMMapNativeView::CreateView()
{
    m_renderEngine = new RenderEngine3d();
    if (m_renderEngine != NULL)
    {
        m_renderEnginePrivate = m_renderEngine->getRenderEnginePrivate();
        m_renderEnginePrivate->setZorder(m_zorder);
        m_renderEnginePrivate->setMapViewInterface(dynamic_cast<MapViewInterface*>(m_pMapView));
    }
}


/*! process gestures events*/
void
NBGMMapNativeView::HandleNavigatorEvent(bps_event_t *event)
{
    switch (bps_event_get_code(event))
    {
    case NAVIGATOR_EXIT:
        m_isExitMap = true;
        break;
    }
}


void
NBGMMapNativeView::Start()
{

}

NB_Error
NBGMMapNativeView::Initialize()
{
    m_pMapView->UI_SetViewPort(m_width, m_height);
    return NE_OK;
}

void
NBGMMapNativeView::Finallize()
{
    if (m_pMapContainer != NULL)
    {
        // TODO: this should be analyzed in future
        // deleting of m_pMapContainer in NBGMMapNativeView causes app crash on exit
        //delete m_pMapContainer;
        m_pMapContainer = NULL;
    }

    if (m_renderEnginePrivate != NULL)
    {
        m_renderEnginePrivate->renderFinalize();
    }
    if(m_renderEnginePrivate != NULL)
    {
        m_renderEnginePrivate->Destroy();
        m_renderEnginePrivate = NULL;
    }
}
/*
NB_Error
void
NBGMMapNativeView::CreateFontBuffer(PAL_Instance* pal, MapFontType fontType, uint8** buffer, uint32& bufferSize)
{
	switch(fontType)
	{
	case MFT_SANS:
		{
			SetFont(pal, "DejaVuSans", "normal", "en", buffer, &bufferSize);
		}
		break;
	case MFT_SAN_BLOD:
		{
			SetFont(pal, "DejaVuSans", "normal", "en", buffer, &bufferSize);
		}
		break;
	case MFT_SERIF:
		{
			SetFont(pal, "DejaVuSans", "normal", "en", buffer, &bufferSize);
		}
		break;
	case MFT_SERFI_BLOD:
		{
			SetFont(pal, "DejaVuSans", "normal", "en", buffer, &bufferSize);
		}
		break;
	default:
		break;
	}
}
*/
NBGM_NativeRenderContext*
NBGMMapNativeView::CreateNativeRenderContext()
{
    return new NBGM_NativeRenderContext((RenderInterface*)m_renderEnginePrivate);
}

MapView *
NBGMMapNativeView::getNativeView()
{
    return m_mapView;
}

//********private functions************
void
NBGMMapNativeView::Render()
{
    /*
       rend start to init current egl context
       call nbservice:Render to render
       rend end
    */
    if (!m_isCovered)
    {
        //call nbservice:Render to 
        m_pMapView->UI_SetViewPort(m_width, m_height);
        timeval tv = {0};
        gettimeofday(&tv, NULL);
        int deltaMilliSeconds = (tv.tv_sec * 1000 + tv.tv_usec / 1000) - (m_lastTimeval.tv_sec * 1000 + m_lastTimeval.tv_usec / 1000);
        m_lastTimeval = tv;
        m_pMapView->UI_Render(deltaMilliSeconds);
    }
    if (m_pMapView->IsFirstTilesLoaded())
    {
        if ( nativeMapView != NULL )
        {
            nativeMapView->HideMapLoadingIndicator();
        }
    }
}

void
NBGMMapNativeView::ShowZoomLevelInfo(const char* text){
    if ( nativeMapView != NULL )
    {
        nativeMapView->ShowZoomLevelInfo(text);
    }
}

void
NBGMMapNativeView::TaskUpdateViewSize()
{
    if ( m_updateFlag & NBGMVIEW_UPDATE_VIEWSIZE )
    {
        //m_pMapView->UI_SetScreenOrientation(m_isPortrait); //after integration map3d iteration2
        m_updateFlag &= ~NBGMVIEW_UPDATE_VIEWSIZE;
    }
}

void
NBGMMapNativeView::TaskUpdate()
{
    Render();
}

/*******Add public interface to UIMapcontroller******/
void
NBGMMapNativeView::setMapNativeView(MapNativeViewInterfaces* _nativeMapView)
{
	//prepare components
    m_pMapContainer = new Container();
    m_mapView = new MapView();

//    m_mapView->setPreferredSize(m_width, m_height);   // Do this in App part
    m_mapView->setAltitude(300);
    m_mapView->setRenderEngine(m_renderEngine);

    // Put MapView into container for store reference
    // Container will respond to destroy MapView
    m_pMapContainer->add(m_mapView);

    nativeMapView = _nativeMapView;
    nativeMapView->setMapNativeView(this);
}

void
NBGMMapNativeView::Update()
{
    UpdateViewTaskCallback(this);
}

void
NBGMMapNativeView::SetCurrentLocation(double lat, double lon)
{
    MapViewUIInterface* local = (MapViewUIInterface*)m_pMapView;
    m_mapView->setLatitude(lat);
    m_mapView->setLongitude(lon);
}

void
NBGMMapNativeView::SetCovered(bool isCovered)
{
    m_isCovered = isCovered;
    m_renderEnginePrivate->SetBackground(isCovered);
}

//********callbacks************
static void
UpdateViewTaskCallback(void* param)
{
    NBGMMapNativeView* pView = (NBGMMapNativeView*)param;
    pView->TaskUpdate();
}

void
NBGMMapNativeView::TearDown()
{
    m_renderEngine->teardown();
}

void
NBGMMapNativeView::setMapObjects(shared_ptr<nbmap::PinManager> pinManager, OptionalLayersAgentPtr pLayersAgent)
{
    m_renderEnginePrivate->setMapObjects(pinManager, pLayersAgent);
}

void
NBGMMapNativeView::setAvatarHeading(double heading)
{
    m_renderEngine->setAvatarHeading(heading);
}

void
NBGMMapNativeView::resetScreen()
{
    m_renderEnginePrivate->resetScreen();
}

void
NBGMMapNativeView::SetViewSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void
NBGMMapNativeView::UpdateTilt(float tilt)
{
    m_mapView->setTilt(tilt);
}

void
NBGMMapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    if ( nativeMapView == NULL )
        return;

    switch ( type )
    {
        case TTT_None:
            nativeMapView->HideNoTrafficDataToast();
            break;

        case TTT_NoData:
            nativeMapView->ShowNoTrafficDataToast();
            break;

        case TTT_ZoomIn:
        case TTT_ZoomOut:
            nativeMapView->HideNoTrafficDataToast();
            break;
    }
}

/*! @} */
