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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#include "nbgmdevicewin32.h"
#include "nbgm.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbrerenderengine.h"
#include "nbrememorystream.h"
#include "nbgmmapviewprotected.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmcontext.h"
#include "nbreglrenderpal.h"
#include "nbreglesrenderpal.h"
#include "nbred3d9renderpal.h"
#include "nbgmmapviewprotected.h"
#include "nbgmnavviewprotected.h"
#include "nbgmconst.h"
#include "nbgmglconfiguration.h"
#include "nbgmd3d9configuration.h"
#include "nbgmrendercontext.h"
#include "nbgmmapviewimpl.h"
#include <string>
#include <sstream>
#include <strstream>
#include "nbrelog.h"
#include "nbgmanimation.h"
#include "nbrefilestream.h"
#include "pallock.h"
#include <time.h>
#include <commctrl.h>
#include <iostream>
#include "Shlwapi.h"

#pragma warning(disable:4706)
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define MAX_BUFFER_SIZE  2048*2048*4

static NBGMDeivceWin32 * gDevice = NULL;

NBGMDeviceImpl* GetDevice()
{
    if(gDevice ==  NULL)
    {
        static NBGMDeivceWin32 device;
        gDevice = &device;
    }
    return gDevice;
}


BOOL SaveBitmapToFile( uint32 width, uint32 height, WORD pixelBits, uint8* data, uint32 dataSize, const char* szPath, const char* info, HWND wnd);

class NBGM_RenderContextW32GL:public NBGM_RenderContext
{
public:
    NBGM_RenderContextW32GL(HWND wnd):mWnd(wnd), mDC(NULL), mRC(NULL), mInited(false){ ; }
    virtual ~NBGM_RenderContextW32GL() {}

public:
    virtual void Finalize();
    virtual void Initialize( NBGM_RenderSystemType /*renderSystemType*/ );
    virtual const void* GetConfiguration( const char* name ){ return NULL;}
    virtual void BeginRender() {  wglMakeCurrent(mDC, mRC); }
    virtual void EndRender()
    {
        gDevice->ScreenShot();
        SwapBuffers(mDC);
    }
    virtual void Reset(){}
    virtual void SwitchTo(){  wglMakeCurrent(mDC, mRC); }
    bool IsInited()const { return mInited; }

    HWND mWnd;
    HDC mDC;
    HGLRC mRC;
    bool mInited;
};

void NBGM_RenderContextW32GL::Finalize()
{
    if (mRC)
    {
        wglDeleteContext(mRC);
        mRC = NULL;
    }

    if(mDC)
    {
        ReleaseDC(mWnd, mDC);
        mDC  = NULL;
    }
}

void NBGM_RenderContextW32GL::Initialize( NBGM_RenderSystemType /*renderSystemType*/ )
{
    if(mInited)
    {
        return;
    }
    GLuint PixelFormat; // Holds The Results After Searching For A Match
    static PIXELFORMATDESCRIPTOR pfd = 
    {
        sizeof(PIXELFORMATDESCRIPTOR),              // Size Of This Pixel Format Descriptor
        1,                                          // Version Number
        PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
        PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                           // Must Support Double Buffering
        PFD_TYPE_RGBA,                              // Request An RGBA Format
        32,                                         // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                           // Color Bits Ignored
        0,                                          // No Alpha Buffer
        0,                                          // Shift Bit Ignored
        0,                                          // No Accumulation Buffer
        0, 0, 0, 0,                                 // Accumulation Bits Ignored
        16,                                         // 16Bit Z-Buffer (Depth Buffer)  
        0,                                          // No Stencil Buffer
        0,                                          // No Auxiliary Buffer
        PFD_MAIN_PLANE,                             // Main Drawing Layer
        0,                                          // Reserved
        0, 0, 0                                     // Layer Masks Ignored
    };

    if (!(mDC = GetDC(mWnd))) // Did We Get A Device Context?
    {
        Finalize();                                
        return;                            
    }

    if (!(PixelFormat = ChoosePixelFormat(mDC, &pfd))) // Did Windows Find A Matching Pixel Format?
    {
        Finalize();
        return;                        
    }

    if(!SetPixelFormat(mDC, PixelFormat, &pfd)) // Are We Able To Set The Pixel Format?
    {
        Finalize();                        
        return;                                
    }

    if (!(mRC = wglCreateContext(mDC))) // Are We Able To Get A Rendering Context?
    {
        Finalize();
        return;
    }

    wglMakeCurrent(mDC, mRC);
    mInited = true;
}

class NBGM_RenderContextW32D3D9:public NBGM_RenderContext
{
public:
    NBGM_RenderContextW32D3D9(HWND  wnd, int width, int height):mWnd(wnd), mWidth(width), mHeight(height),m_D3D9Device(NULL), m_D3D9(NULL), mInited(false){}
    virtual ~NBGM_RenderContextW32D3D9() {}

public:
    virtual void Finalize();
    virtual void Initialize( NBGM_RenderSystemType /*renderSystemType*/ );
    virtual const void* GetConfiguration( const char* name ){ return NULL;}
    virtual void BeginRender() { ; }
    virtual void EndRender()
    {
        gDevice->ScreenShot();
        m_D3D9Device->Present(NULL, NULL, NULL, NULL);
    }
    virtual void Reset(){}
    virtual void SwitchTo(){}
    bool IsInited()const { return mInited; }

    HWND mWnd;
    int mWidth;
    int mHeight;
    IDirect3DDevice9* m_D3D9Device;
    IDirect3D9* m_D3D9;
    D3DPRESENT_PARAMETERS mD3DPP;
    bool mInited;

};

void NBGM_RenderContextW32D3D9::Initialize( NBGM_RenderSystemType /*renderSystemType*/ )
{
    if(mInited)
    {
        return;
    }
    m_D3D9 = Direct3DCreate9( D3D_SDK_VERSION );
    if(m_D3D9 == NULL)
    {
        return;
    }

    ZeroMemory(&mD3DPP,sizeof(mD3DPP));

    // Set up the structure used to create the D3DDevice. 
    mD3DPP.Windowed = TRUE;
    mD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
    mD3DPP.BackBufferFormat = D3DFMT_UNKNOWN;
    mD3DPP.BackBufferHeight = mHeight;
    mD3DPP.BackBufferWidth = mWidth;
    mD3DPP.EnableAutoDepthStencil = TRUE;
    mD3DPP.AutoDepthStencilFormat = D3DFMT_D16;
    mD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    mD3DPP.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    // Create the Direct3D device.
    if( FAILED( m_D3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_FPU_PRESERVE,
        &mD3DPP, &m_D3D9Device )))
    {
        return;
    }
    mInited = true;
}

void NBGM_RenderContextW32D3D9::Finalize( )
{
    if( m_D3D9Device != NULL )
    {
        m_D3D9Device->Release();
        m_D3D9Device = NULL;
    }

    if( m_D3D9 != NULL )
    {
        m_D3D9->Release();
        m_D3D9 = NULL;
    }
}

NBGM_RenderSystemType NBGMDeivceWin32::GetRenderSystem(bool opengl)
{
    return opengl?NBGM_RS_GL:NBGM_RS_D3D9;
}


bool NBGMDeivceWin32::InitContext(NBGM_MapViewConfig& c)
{
    if(mRenderSystemType == NBGM_RS_GL)
    {
        NBGM_RenderContextW32GL *context = NBRE_NEW NBGM_RenderContextW32GL(mWnd);
        context->Initialize(NBGM_RS_GL);
        if(!context->IsInited())
        {
            NBRE_DELETE context;
            return false;
        }
        else
        {
            c.renderContext = shared_ptr<NBGM_RenderContextW32GL>(context);
        }
    }
    else if(mRenderSystemType == NBGM_RS_D3D9)
    {
        NBGM_RenderContextW32D3D9 *context = NBRE_NEW NBGM_RenderContextW32D3D9(mWnd, mWidth, mHeight);
        context->Initialize(NBGM_RS_D3D9);
        if(!context->IsInited())
        {
            NBRE_DELETE context;
            return false;
        }
        else
        {
            c.m_D3D9 = context->m_D3D9;
            c.m_D3D9Device = context->m_D3D9Device;
            c.renderContext = shared_ptr<NBGM_RenderContextW32D3D9>(context);
            m_D3D9Device = context->m_D3D9Device;
        }
    }
    else
    {
        nsl_assert(0);
    }
    return true;
}


NBGMDeivceWin32::NBGMDeivceWin32(void):mInstance(NULL),
    mWnd(NULL),
    mStatusBar(NULL),
    mLBtnDown(false),
    m_RightClickedPtx(0.f),
    m_RightClickedPty(0.f),
    mMouseX(0),
    mMouseY(0),
    mBarHeight(30),
    m_D3D9Device(NULL)
{
}

NBGMDeivceWin32::~NBGMDeivceWin32(void)
{
}

void NBGMDeivceWin32::OnCreate(HWND hWnd)
{
    nbre_assert(hwnd);
    InitCommonControls();
    mStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP , 0, 0, 0, 0, hWnd, NULL, mInstance, NULL); 
    if(mStatusBar != NULL)
    {
        int statwidths[] = {60, 248, 458, -1};
        SendMessage(mStatusBar, SB_SETPARTS, 4, (LPARAM)statwidths);
        SendMessage(mStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT(" FPS: 60"));
    }
}


LRESULT CALLBACK WndProc(HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_CREATE:
        if(gDevice)
        {
            gDevice->OnCreate(hWnd);
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_ERASEBKGND:
        return 0;

    case WM_SYSCOMMAND:                     
        {
            switch (wParam)              
            {
            case SC_SCREENSAVE:     // Screensaver Trying To Start?
            case SC_MONITORPOWER:   // Monitor Trying To Enter Powersave?
                return 0;           // Prevent From Happening
            }
            break;                             
        }

    case WM_CLOSE:                            
        {
            PostQuitMessage(0);    
            return 0;                  
        }

    case WM_SIZE:
        {  
            if(gDevice)
            {
                gDevice->OnSizeChanged(uMsg, wParam, lParam);
            }
            return 0;
        }

    case WM_LBUTTONDOWN:
        {
            if(gDevice)
            {
                gDevice->OnLButtonDown((UINT)wParam,  (float)LOWORD(lParam),  (float)HIWORD(lParam));
            }
            return 0;
        }
    case WM_LBUTTONUP:
        {
            if(gDevice)
            {
                gDevice->OnLButtonUp((UINT)wParam,  (float)LOWORD(lParam),  (float)HIWORD(lParam));
            }
            return 0;
        }
    case WM_RBUTTONDOWN:
        {
            if(gDevice)
            {
                gDevice->OnRButtonDown((UINT)wParam,  (float)LOWORD(lParam),  (float)HIWORD(lParam));
            }
            return 0;
        }
    case WM_MOUSEMOVE:
        {
            if(gDevice)
            {
                gDevice->OnMouseMove((UINT)wParam,  (float)LOWORD(lParam),  (float)HIWORD(lParam));
            }
            return 0;
        }
    case 0x020A:
        {
            if(gDevice)
            {
                gDevice->OnMouseWheel((UINT)LOWORD(wParam), (short)HIWORD(wParam), (float)LOWORD(lParam),  (float)HIWORD(lParam));
            }
            return 0;
        }
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void NBGMDeivceWin32::DestroyNativeWindow()
{
    if(mStatusBar)
    {
        if(!DestroyWindow(mStatusBar))
        {
            return;
        }
        mStatusBar  = NULL;
    }

    if(mWnd)
    {
        if(!DestroyWindow(mWnd))
        {
            return;
        }
        mWnd  = NULL;
    }

    if(mInstance)
    {
        if(!UnregisterClass(TEXT("Window3D"), mInstance))
        {
            return;
        }
        mInstance = NULL;
    }
}

bool NBGMDeivceWin32::CreateNativeWindow()
{
    uint32 w = mWidth;
    uint32 h = mHeight;

    WNDCLASS   wc;            // Windows Class Structure
    DWORD      dwExStyle;     // Window Extended Style
    DWORD      dwStyle;       // Window Style
    RECT       WindowRect;    // Grabs Rectangle Upper Left / Lower Right Values

    WindowRect.left   = (LONG)0;    
    WindowRect.right  = (LONG)w;
    WindowRect.top    = (LONG)0;    
    WindowRect.bottom = (LONG)h;

    mInstance          = GetModuleHandle(NULL);                // Grab An Instance For Our Window
    wc.style           = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc     = (WNDPROC) WndProc;                    // WndProc Handles Messages
    wc.cbClsExtra      = 0;                                    // No Extra Window Data
    wc.cbWndExtra      = 0;                                    // No Extra Window Data
    wc.hInstance       = mInstance;                            // Set The Instance
    wc.hIcon           = LoadIcon(NULL, IDI_WINLOGO);          // Load The Default Icon
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);          // Load The Arrow Pointer
    wc.hbrBackground   = NULL;                                 // No Background Required For GL
    wc.lpszMenuName    = NULL;                                 // We Don't Want A Menu
    wc.lpszClassName   = TEXT("Window3D");                       // Set The Class Name
    wc.style &= ~WS_THICKFRAME;
    if (!RegisterClass(&wc))                                   // Attempt To Register The Window Class
    {
        return false;                                          // Return FALSE
    }

    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;         // Window Extended Style
    dwStyle = WS_OVERLAPPEDWINDOW;                          // Windows Style

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);  // Adjust Window To True Requested Size

    // Create The Window
    if (!(mWnd = CreateWindowEx(dwExStyle, // Extended Style For The Window
        TEXT("Window3D"),                  // Class Name
        TEXT("NBGM Replayer"),             // Window Title
        dwStyle |                          // Defined Window Style
        WS_CLIPSIBLINGS |                  // Required Window Style
        WS_CLIPCHILDREN,                   // Required Window Style
        50, 50,                            // Window Position
        WindowRect.right-WindowRect.left,  // Calculate Window Width
        WindowRect.bottom-WindowRect.top,  // Calculate Window Height
        NULL,                              // No Parent Window
        NULL,                              // No Menu
        mInstance,                         // Instance
        NULL )))                           // Dont Pass Anything To WM_CREATE
    {
        return false;                            
    }

   
    //RECT rect;
    //GetClientRect(mStatusBar, &rect);
    //mBarHeight = rect.bottom - rect.top;
    //w +=  GetSystemMetrics(SM_CXFRAME) * 2;
    //h += GetSystemMetrics(SM_CYCAPTION) +  mBarHeight + GetSystemMetrics(SM_CYFRAME) * 2;
    //SetWindowPos(mWnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

    ShowWindow(mWnd, SW_SHOW); // Show The Window
    SetForegroundWindow(mWnd); // Slightly Higher Priority
    SetFocus(mWnd);            // Sets Keyboard Focus To The Window

    if(mRenderSystemType == NBGM_RS_GL)
    {
        SetWindowText(mWnd, TEXT("NBGM Replayer (OpenGL)"));
    }
    else if(mRenderSystemType == NBGM_RS_D3D9)
    {
        SetWindowText(mWnd, TEXT("NBGM Replayer (D3D9)"));
    }
    else
    {
        SetWindowText(mWnd, TEXT("NBGM Replayer (Unknown)"));
    }
    return true;
}


void NBGMDeivceWin32::UpdateFPS()
{
    if(!mInited)
    {
        return;
    }

    Invalidate();

    //static TCHAR buffer[20] = TEXT("");
    //static int frames = 0;
    //static clock_t last = clock();
    //++frames;
    //clock_t now = clock();
    //if (now - last >= CLOCKS_PER_SEC)
    //{
    //    wsprintf(buffer, TEXT(" FPS: %d"), frames);
    //    SendMessage(mStatusBar, SB_SETTEXT, 0, (LPARAM)buffer);
    //    last = now;
    //    frames = 0;
    //}
}

void NBGMDeivceWin32::UpdateStatusBar()
{
    if(!mInited)
    {
        return;
    }

    static TCHAR buffer[256] = TEXT("");
    double mx = 0.f;
    double my = 0.f;
    double cx = 0.f;
    double cy = 0.f;
    double x = 0.f;
    double y = 0.f;
    bool ret = mMapView->ScreenToMapPosition(mMouseX, mMouseY, mx, my);
    ret = ret?mMapView->ScreenToMapPosition(mWidth/2.f, mHeight/2.f, cx, cy):false;
    if(ret)
    {
        mercator_reverse(mx, my, x, y);
        swprintf(buffer, 256,  TEXT(" Mouse ( %.5f, %.5f )"), x, y);
        SendMessage(mStatusBar, SB_SETTEXT, 1, (LPARAM)buffer);
        mercator_reverse(cx, cy, x, y);
        swprintf(buffer, 256,  TEXT(" MapCenter ( %.5f, %.5f )"), x, y);
        SendMessage(mStatusBar, SB_SETTEXT, 2, (LPARAM)buffer);
    }
    swprintf(buffer, 256,  TEXT("Camera:  Tilt( %.2f )   Roate( %.2f )   Height( %.2f )"), mMapView->GetTiltAngle(), mMapView->GetRotateAngle(), mMapView->GetCameraHeight());
    SendMessage(mStatusBar, SB_SETTEXT, 3, (LPARAM)buffer);
}

void CALLBACK UpdateBar(HWND /*hwnd*/, UINT /*message*/, UINT /*timerID*/, DWORD /*time*/)
{
    gDevice->UpdateStatusBar();
}

void CALLBACK UpdateWindow(HWND /*hwnd*/, UINT /*message*/, UINT /*timerID*/, DWORD /*time*/)
{
    gDevice->UpdateFPS();
}

void NBGMDeivceWin32::Run()
{
    MSG msg;
    bool quit = false;

    SetTimer(NULL, 1000, 400, UpdateBar);
    SetTimer(NULL, 2000, 200, UpdateWindow);

    while(!quit)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // Translate The Message
            DispatchMessage(&msg);  // Dispatch The Message

            if (msg.message == WM_QUIT)
            {
                quit = true;
            }
            if(msg.message == WM_CHAR)
            {
                if(msg.wParam > 0 && msg.wParam < 256)
                {
                    mKeyMap[msg.wParam] = TRUE;
                }
            }
        }
    }

    KillTimer(NULL, 1000);
    KillTimer(NULL, 2000);

}

//---------------------------------------------------------------------------------------------------------------------------


void NBGMDeivceWin32::LoadCommonMaterial(const char* materialName, const char* filePath)
{
    nbre_verify(mMapView);
    std::string realPath(filePath);
    if(!PathFileExistsA(filePath))
    {
         realPath = mTilesPath;
         realPath += PathFindFileNameA(filePath);
    }
    mMapView->LoadCommonMaterial(materialName, realPath);
}

void NBGMDeivceWin32::LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath)
{
    nbre_verify(mMapView);
    std::string realPath(filePath);
    if(!PathFileExistsA(filePath))
    {
         realPath = mTilesPath;
         realPath += PathFindFileNameA(filePath);
    }
   
    mMapView->LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, realPath);
}

void NBGMDeivceWin32::OnSizeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SendMessage(mStatusBar, uMsg, wParam, lParam);
    int w = (int)LOWORD(lParam);
    int h = (int)HIWORD(lParam);
    ChangeSize(w, h);
}

void NBGMDeivceWin32::OnLButtonDown(UINT nFlags, float x, float y)
{
    if(mMapView)
    {
        mMapView->OnTouchEvent(x, y);
        mLBtnDown = true;
    }
}

void NBGMDeivceWin32::OnLButtonUp(UINT nFlags, float x, float y)
{
    mLBtnDown = false;
}

void NBGMDeivceWin32::OnRButtonDown(UINT nFlags, float x, float y)
{
    if(mMapView)
    {
        mMapView->SetRotateCenter(x, y);
        m_RightClickedPtx = x;
        m_RightClickedPty = y;
    }
}

void NBGMDeivceWin32::OnMouseMove(UINT nFlags,float x, float y)
{
    if(mMapView)
    {
        if((nFlags & MK_LBUTTON) && (mLBtnDown))
        {
            mMapView->OnPaning(x, y);
        }
        if(nFlags & MK_RBUTTON)
        {
            mMapView->Rotate(x - m_RightClickedPtx);
            mMapView->Tilt(y - m_RightClickedPty);
            m_RightClickedPtx = x;
            m_RightClickedPty = y;
        }
        mMouseX = x;
        mMouseY = y;
    }
}

void NBGMDeivceWin32::OnMouseWheel(UINT nFlags, short zDelta, float x, float y)
{
    if(mMapView)
    {
        if(zDelta > 0)
        {
            mMapView->Zoom(-METER_TO_MERCATOR(mMapView->GetViewPointDistance()/16));
        }
        else
        {
            mMapView->Zoom(METER_TO_MERCATOR(mMapView->GetViewPointDistance()/16));
        }
    }
}

void NBGMDeivceWin32::ScreenShot()
{
    if(!mNeedScreenShot || !mInited)
    {
        return;
    }

    PAL_LockLock(mKeyLock);
    NBRE_String file(mScreenShotFilePath);
    NBRE_String info(mScreenShotInfo);
    PAL_LockUnlock(mKeyLock);

    if(file.empty())
    {
        return;
    }

    if(mImageBuffer == NULL)
    {
        mImageBuffer = (uint8*)nsl_malloc(MAX_BUFFER_SIZE);
    }

    if(mImageBuffer == NULL)
    {
        DebugLog("NBGMDeivceWin32::mImageBuffer is null, no memory!");
        return;
    }

    uint32 imagedatasize = mWidth*mHeight*3;
    if(imagedatasize > MAX_BUFFER_SIZE)
    {
        DebugLog("ScreenShot::imagedatasize > MAX_BUFFER_SIZE!");
        mNeedScreenShot = false;
        return;
    }

    if(mRenderSystemType == NBGM_RS_GL)
    {
        ScreenShotOpenGL();
    }
    else if(mRenderSystemType == NBGM_RS_D3D9)
    {
        ScreenShotD3D9();
    }
    else
    {
        nsl_assert(0);
    }

    BOOL ret = SaveBitmapToFile(mWidth, mHeight, 24, mImageBuffer, imagedatasize, file.c_str(), info.c_str(), mWnd);
    if(!ret)
    {
        DebugLog("ScreenShotOpenGL::SaveBitmapToFile failed !");
    }
     mNeedScreenShot = false;
}

void NBGMDeivceWin32::ScreenShotD3D9()
{
    //IDirect3DSurface9 *back = NULL;
    //if( FAILED( m_D3D9Device->CreateOffscreenPlainSurface(mWidth, mHeight, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &back, NULL)))
    //{
    //    DebugLog("ScreenShotD3D9::CreateOffscreenPlainSurface failed!");
    //    mNeedScreenShot = false;
    //    return;
    //}

    //if( FAILED( m_D3D9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back) ))
    //{
    //    DebugLog("ScreenShotD3D9::GetBackBuffer failed!");
    //    mNeedScreenShot = false;
    //    back->Release();
    //    return;
    //}

    //D3DLOCKED_RECT rect;
    //if( FAILED( back->LockRect (&rect, NULL, D3DLOCK_READONLY)))
    //{
    //    DebugLog("ScreenShotD3D9::LockRect failed!");
    //    mNeedScreenShot = false;
    //    back->Release();
    //    return;
    //}
    //for (int i = 0; i < mHeight; i++)
    //{
    //    for (int j = 0; j < mWidth; j++)
    //    {
    //        for (int k = 0;  k < 3; k++)
    //        {
    //            int src_offset = i*rect.Pitch + j*4 + k;
    //            int dst_offset = (mHeight-i-1)*mWidth*3 + j*3 + k;
    //            mImageBuffer[dst_offset] = *((uint8*)rect.pBits + src_offset);
    //        }
    //    }
    //}
}

void NBGMDeivceWin32::ScreenShotOpenGL()
{
    GLubyte *bmpBuffer = (GLubyte*)mImageBuffer;
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, (GLint)mWidth, (GLint)mHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmpBuffer);
}

BOOL SaveBitmapToFile( uint32 width, uint32 height, WORD pixelBits, uint8* data, uint32 dataSize, const char* szPath, const char* info, HWND wnd)
{
    BITMAPINFOHEADER bmInfoHeader = {0};
    bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfoHeader.biWidth = width;
    bmInfoHeader.biHeight = height;
    bmInfoHeader.biPlanes = 1;
    bmInfoHeader.biBitCount = pixelBits;

    BITMAPFILEHEADER bmFileHeader = {0};
    bmFileHeader.bfType = 0x4d42;  //bmp
    bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmFileHeader.bfSize = bmFileHeader.bfOffBits + ((bmInfoHeader.biWidth * bmInfoHeader.biHeight) * (pixelBits/8));

    HANDLE hFile = CreateFileA(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == NULL)
    {
        return FALSE;
    }
    DWORD dwWrite = 0;
    WriteFile(hFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwWrite, NULL);
    WriteFile(hFile, &bmInfoHeader, sizeof(BITMAPINFOHEADER),&dwWrite, NULL);
    WriteFile(hFile, data, dataSize, &dwWrite,NULL);
    CloseHandle(hFile);
    return TRUE;
}

//void NBGMDeivceWin32::ScreenShot()
//{
//    if(!mNeedScreenShot || !mInited)
//    {
//        return;
//    }
//
//    PAL_LockLock(mKeyLock);
//    NBRE_String file(mScreenShotFilePath);
//    NBRE_String info(mScreenShotInfo);
//    PAL_LockUnlock(mKeyLock);
//
//    if(file.empty())
//    {
//        return;
//    }
//
//    if(mImageBuffer == NULL)
//    {
//        mImageBuffer = (uint8*)nsl_malloc(MAX_BUFFER_SIZE);
//    }
//
//    if(mImageBuffer == NULL)
//    {
//        DebugLog("ScreenShot::mImageBuffer is null, no memory!");
//        return;
//    }
//
//    RECT rect;
//    GetClientRect(mWnd, &rect);
//    int width = rect.right - rect.left;
//    int height = rect.bottom - rect.top - mBarHeight;
//
//    uint32 imagedatasize = width*height*3;
//    if(imagedatasize > MAX_BUFFER_SIZE)
//    {
//        DebugLog("ScreenShot::imagedatasize > MAX_BUFFER_SIZE!");
//        mNeedScreenShot = false;
//        return;
//    }
//
//    HDC hdc = GetDC(mWnd);
//    HDC MemDC = CreateCompatibleDC(hdc);
//    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
//    SelectObject(MemDC, hBitmap);
//    BitBlt(MemDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
//
//    if(!info.empty())
//    {
//        RECT rect = {15, 15, 500, 500};
//        SetTextColor(MemDC, RGB(0, 0, 255));
//        int old = SetBkMode(MemDC, TRANSPARENT);
//        DrawTextA(MemDC, info.c_str(), info.size(), &rect, DT_LEFT | DT_TOP);
//        SetBkMode(MemDC, old);
//    }
//
//    BITMAPINFO bmpInfo = {0};
//    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//    bmpInfo.bmiHeader.biWidth = width;
//    bmpInfo.bmiHeader.biHeight = height;
//    bmpInfo.bmiHeader.biPlanes = 1;
//    bmpInfo.bmiHeader.biBitCount = 24;
//    bmpInfo.bmiHeader.biCompression = BI_RGB;
//    bmpInfo.bmiHeader.biSizeImage = 0;
//    GetDIBits(MemDC, hBitmap, 0, mHeight, mImageBuffer, &bmpInfo, DIB_RGB_COLORS);
//    BOOL ret = SaveBitmapToFile(width, height, 24, mImageBuffer, imagedatasize, file.c_str(), info.c_str(), mWnd);
//    if(!ret)
//    {
//        DebugLog("ScreenShot::SaveBitmapToFile failed !");
//    }
//
//    ReleaseDC(mWnd, hdc);
//    ReleaseDC(mWnd, MemDC);
//    DeleteObject (hBitmap);
//    DeleteDC (MemDC);
//    DeleteDC (hdc); 
//    mNeedScreenShot = false;
//}
