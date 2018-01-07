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

#include "MapNativeView.h"
#include "NBGMMapNativeView.h"
#include "MapViewUIInterface.h"
#include "MapNativeViewInterfaces.h"
#include "MapViewInterface.h"
#include "NBUIConfig.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "paldisplay.h"
#include "NBGMRenderContextImpl.h"
#include "openglview.h"
#include <Windows.h>
#include "MapViewGestureHandler.h"

using namespace nbmap;

enum UpdateFlags
{
    NBGMVIEW_UPDATE_VIEWSIZE = 0x8
};

static void UpdateViewTaskCallback(void* param);
static NBGMMapNativeView* gMapView = NULL;

NBGMMapNativeView::NBGMMapNativeView(MapViewUIInterface* mapView, MapNativeView* mapNativeView, int zorderLevel)
 :  m_nativeMapView(NULL),
    m_width(0),
    m_height(0),
    m_isExitMap(false),
    m_zorder(zorderLevel),

    m_pMapView(mapView),
    m_isCovered(false),

    m_isTilesLoaded(false),
    m_isPortrait(false),
    m_updateFlag(0),
    m_mapNativeView(mapNativeView),
    m_lastTickCount(0),
    m_window(NULL),
    m_dc(NULL),
    m_openGLView(NULL),
    m_gestureHandler(NULL)
{
    gMapView = this;
    m_gestureHandler = new MapViewGestureHandler(mapView);
    PAL_DisplayInitialize(NULL); //todo: set PAL_Instatnce here
    m_width = PAL_DisplayGetWidth();
    m_height = PAL_DisplayGetHeight();
    CreateView();
}

NBGMMapNativeView::~NBGMMapNativeView()
{

}

void
NBGMMapNativeView::OnTouchEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GestureEvent event;
    event.wnd = hWnd;
    event.message = message;
    event.wParam = wParam;
    event.lParam = lParam;
    m_gestureHandler->OnTouchEvent(event);
}

LRESULT
NBGMMapNativeView::UIThreadWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(gMapView)
    {
        if(message == WM_SIZE)
        {
            gMapView->SetViewSize(LOWORD(lParam), HIWORD(lParam));
        }
        gMapView->OnTouchEvent(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void
NBGMMapNativeView::CreateView()
{
    const UINT VIEW_DEFAULT_HEIGHT = 800;
    const UINT VIEW_DEFAULT_WIDTH =  1200;

    const WCHAR* CLASS_NAME = TEXT("Mapkit3dWindow");
    const WCHAR* WINDOW_TITLE = TEXT("Mapkit3d Win32 Test Window");

    WNDCLASS   wc = {0};      // Windows Class Structure
    RECT       windowRect = {0};    // Grabs Rectangle Upper Left / Lower Right Values

    windowRect.left   = 0;
    windowRect.right  = VIEW_DEFAULT_WIDTH;
    windowRect.top    = 0;
    windowRect.bottom = VIEW_DEFAULT_HEIGHT;

    HMODULE hInstance  = GetModuleHandle(NULL);                // Grab An Instance For Our Window
    wc.style           = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;   // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc     = (WNDPROC) UIThreadWindowProcedure;    // WndProc Handles Messages
    wc.cbClsExtra      = 0;                                    // No Extra Window Data
    wc.cbWndExtra      = 0;                                    // No Extra Window Data
    wc.hInstance       = hInstance;                            // Set The Instance
    wc.hIcon           = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
    wc.hbrBackground   = NULL;                                 // No Background Required For GL
    wc.lpszMenuName    = NULL;                                 // We Don't Want A Menu
    wc.lpszClassName   = CLASS_NAME;                           // Set The Class Name
    wc.style &= ~WS_THICKFRAME;
    if (!RegisterClass(&wc))                                   // Attempt To Register The Window Class
    {
        return;
    }

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;         // Window Extended Style
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;                          // Windows Style

    //            AdjustwindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);  // Adjust Window To True Requested Size

    // Create The Window
    m_window = CreateWindowEx(dwExStyle,                         // Extended Style For The Window
                                CLASS_NAME,                        // Class Name
                                WINDOW_TITLE,                      // Window Title
                                dwStyle |                          // Defined Window Style
                                WS_CLIPSIBLINGS |                  // Required Window Style
                                WS_CLIPCHILDREN,                   // Required Window Style
                                50, 50,                            // Window Position
                                windowRect.right - windowRect.left,// Calculate Window Width
                                windowRect.bottom - windowRect.top,// Calculate Window Height
                                NULL,                              // No Parent Window
                                NULL,                              // No Menu
                                hInstance,                         // Instance
                                NULL );
    if (m_window == NULL)
    {
        return;
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),                // size of this pfd
        1,                                            // version
        PFD_DRAW_TO_WINDOW |                          // The buffer can draw to a window
        PFD_SUPPORT_OPENGL |                          // The buffer supports OpenGL drawing.
        PFD_DOUBLEBUFFER,                             // The buffer is double-buffered.
        PFD_TYPE_RGBA,                                // RGBA pixels. Each pixel has four components in this order: red, green, blue, and alpha
        24,                                           // 24-bit color depth
        0, 0, 0, 0, 0, 0,                             // color bits ignored
        8,                                            // 8 bit alpha buffer
        0,                                            // shift bit ignored
        0,                                            // no accumulation buffer
        0, 0, 0, 0,                                   // accum bits ignored
        32,                                           // 32-bit z-buffer
        0,                                            // no stencil buffer
        0,                                            // no auxiliary buffer
        PFD_MAIN_PLANE,                               // main layer
        0,                                            // reserved
        0, 0, 0                                       // layer masks ignored
    };

    m_dc = GetDC(m_window);
    if (m_dc == NULL)
    {
        return;
    }
    int pf = ChoosePixelFormat(m_dc, &pfd);
    SetPixelFormat(m_dc, pf, &pfd);

    m_gestureHandler->StartDetector();
}

void
NBGMMapNativeView::Start()
{

}

NB_Error
NBGMMapNativeView::Initialize()
{
    ShowWindow(m_window, SW_SHOW); // Show The Window
    SetForegroundWindow(m_window); // Slightly Higher Priority
    SetFocus(m_window);            // Sets Keyboard Focus To The Window
    UpdateWindow(m_window);

    m_pMapView->UI_SetViewPort(m_width, m_height);
    return NE_OK;
}

void
NBGMMapNativeView::Finallize()
{
    //DestroyWindow(m_window);
    m_gestureHandler->StopDetector();
}


NBGM_RenderContext*
NBGMMapNativeView::CreateNativeRenderContext()
{
    return new NBGM_RenderContextImpl(m_dc, NBUIConfig::GetRenderSystemType());
}


//********private functions************
void
NBGMMapNativeView::Render()
{
    // @todo: Do we need this?
//    m_pMapView->UI_SetViewPort(m_width, m_height);

    // Calculate time since last render call, this is used for the animation
    DWORD tickCount = GetTickCount();
    int deltaMilliSeconds = (m_lastTickCount == 0) ? 0 : (tickCount - m_lastTickCount);
    m_lastTickCount = tickCount;

    // Call render in MapView
    m_pMapView->UI_Render(deltaMilliSeconds);
}

void
NBGMMapNativeView::ShowZoomLevelInfo(const char* text)
{
    if ( m_nativeMapView != NULL )
    {
        m_nativeMapView->ShowZoomLevelInfo(text);
    }
}

void
NBGMMapNativeView::TaskUpdateViewSize()
{
    // @todo: The flag never gets set

    if ( m_updateFlag & NBGMVIEW_UPDATE_VIEWSIZE )
    {
        //m_pMapView->UI_SetScreenOrientation(m_isPortrait); //after integration map3d iteration2
        m_updateFlag &= ~NBGMVIEW_UPDATE_VIEWSIZE;
    }
}


/*******Add public interface to UIMapcontroller******/
void
NBGMMapNativeView::SetMapNativeView(MapNativeViewInterfaces* nativeMapView)
{
//  //prepare components
//    m_pMapContainer = new Container();
//    m_mapView = new MapView();
//
////    m_mapView->setPreferredSize(m_width, m_height);   // Do this in App part
//    m_mapView->setAltitude(300);
//    m_mapView->setRenderEngine(m_renderEngine);
//
//    // Put MapView into container for store reference
//    // Container will respond to destroy MapView
//    m_pMapContainer->add(m_mapView);
//
//    m_nativeMapView = _nativeMapView;
//    m_nativeMapView->setMapNativeView(this);
}


void
NBGMMapNativeView::SetCurrentLocation(double lat, double lon)
{
    //MapViewUIInterface* local = (MapViewUIInterface*)m_pMapView;
    //m_mapView->setLatitude(lat);
    //m_mapView->setLongitude(lon);
}

void
NBGMMapNativeView::SetCovered(bool isCovered)
{
}

void
NBGMMapNativeView::TearDown()
{
    //m_renderEngine->teardown();
}

void
NBGMMapNativeView::setMapObjects(shared_ptr<PinManager> pinManager, OptionalLayersAgentPtr pLayersAgent)
{
    //m_renderEnginePrivate->setMapObjects(pinManager, pLayersAgent);
}

void
NBGMMapNativeView::setAvatarHeading(double heading)
{
    //m_renderEngine->setAvatarHeading(heading);
}

void
NBGMMapNativeView::ResetScreen()
{
    //m_renderEnginePrivate->resetScreen();
}

void
NBGMMapNativeView::SetViewSize(int width, int height)
{
    if(width > 1 && height > 1)
    {
        m_width = width;
        m_height = height;
        m_pMapView->UI_SetViewPort(m_width, m_height);
    }
}

void
NBGMMapNativeView::UpdateTilt(float tilt)
{
    //m_mapView->setTilt(tilt);
}

void
NBGMMapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    if ( m_nativeMapView == NULL )
        return;

    switch ( type )
    {
        case TTT_None:
//            m_nativeMapView->HideNoTrafficDataToast();
            break;

        case TTT_NoData:
//            m_nativeMapView->ShowNoTrafficDataToast();
            break;

        case TTT_ZoomIn:
        case TTT_ZoomOut:
//            m_nativeMapView->HideNoTrafficDataToast();
            break;
    }
}

/*! @} */
