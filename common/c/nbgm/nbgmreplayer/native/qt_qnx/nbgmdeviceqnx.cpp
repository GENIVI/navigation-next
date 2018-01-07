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

#include "nbgmdeviceqnx.h"
#include <sys/platform.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "nbgm.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbrerenderengine.h"
#include "nbrememorystream.h"
#include "nbgmmapviewprotected.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmcontext.h"
#include "nbreglesrenderpal.h"
#include "nbregles2renderpal.h"
#include "nbgmmapviewprotected.h"
#include "nbgmnavviewprotected.h"
#include "nbgmconst.h"
#include "nbgmgles11configuration.h"
#include "nbgmrendercontext.h"
#include "nbgmmapviewimpl.h"
#include "nbrelog.h"
#include "nbgmanimation.h"
#include "nbrefilestream.h"
#include "pallock.h"
#include <bb/Application>
#include "nbgmgesturehandler.h"
#include "png.h"

using bb::Application;

#define MAX_BUFFER_SIZE  2048*2048*4

static NBGMDeviceQNX * gDevice = NULL;

NBGMDeviceQNX* GetDevice()
{
    if(gDevice ==  NULL)
    {
        static NBGMDeviceQNX device;
        gDevice = &device;
    }
    return gDevice;
}

class NBGM_RenderContextGLESBB10:public NBGM_RenderContext
{
public:
	NBGM_RenderContextGLESBB10( screen_context_t screencontext ):mScreenContext(screencontext),
    mDisplay(NULL),
    mSurface(NULL),
    mContext(NULL),
    mScreenWindow(NULL),
    mScreenDisplay(NULL),
    mBuffers(2),
    mInitialized(0){}
    virtual ~NBGM_RenderContextGLESBB10() {}

public:
    virtual void Finalize();
    virtual void Initialize( NBGM_RenderSystemType /*renderSystemType*/ );
    virtual const void* GetConfiguration( const char* name ){ return NULL;}
    virtual void BeginRender() {  eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);  }
    virtual void EndRender()
    {
        eglSwapBuffers(mDisplay, mSurface);
        gDevice->ScreenShot();
    }
    virtual void Reset(){}
    virtual void SwitchTo(){ eglMakeCurrent(mDisplay, mSurface, mSurface, mContext); }

    bool GetSize(int &width, int & height)
    {
		bool result = false;
		EGLint surface_width, surface_height;

		if(mDisplay != NULL && mSurface != NULL)
		{
			eglQuerySurface(mDisplay, mSurface, EGL_WIDTH, &surface_width);
			eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &surface_height);
			width = surface_width;
			height = surface_height;
			result = true;
		}
		return result;
    }

    bool IsInited()const
    {
    	return mInitialized == 1;
    }

    screen_context_t mScreenContext;
    screen_window_t  mScreenWindow;
    screen_display_t mScreenDisplay;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    EGLConfig mConfig;
    int mBuffers;
    int mInitialized;
};

void NBGM_RenderContextGLESBB10::Initialize( NBGM_RenderSystemType renderSystemType )
{
	if(mInitialized == 1)
	{
		return;
	}
    int usage;
	int format = SCREEN_FORMAT_RGBX8888;
	EGLint interval = 1;
	int rc, num_configs;

	EGLint attrib_list[]= { EGL_RED_SIZE,        8,
							EGL_GREEN_SIZE,      8,
							EGL_BLUE_SIZE,       8,
							EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
							EGL_RENDERABLE_TYPE, 0,
							EGL_NONE};
	usage = SCREEN_USAGE_ROTATION |((renderSystemType == NBGM_RS_GLES)?SCREEN_USAGE_OPENGL_ES1:SCREEN_USAGE_OPENGL_ES2);
	attrib_list[9] = (renderSystemType == NBGM_RS_GLES)?EGL_OPENGL_ES_BIT:EGL_OPENGL_ES2_BIT;
	EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	const EGLint *attr_list = (renderSystemType == NBGM_RS_GLES)?NULL:attributes;

	mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (mDisplay == EGL_NO_DISPLAY)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglGetDisplay failed!");
		Finalize();
	}

	rc = eglInitialize(mDisplay, NULL, NULL);
	if (rc != EGL_TRUE)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglInitialize failed!");
		Finalize();
		return;
	}

	rc = eglBindAPI(EGL_OPENGL_ES_API);
	if (rc != EGL_TRUE)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglBindApi failed!");
		Finalize();
		return;
	}

	if(!eglChooseConfig(mDisplay, attrib_list, &mConfig, 1, &num_configs))
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglChooseConfig failed!");
		Finalize();
		return;
	}

	mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, attr_list);

	if (mContext == EGL_NO_CONTEXT)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglCreateContext failed!");
		Finalize();
		return;
	}

	rc = screen_create_window(&mScreenWindow, mScreenContext);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_create_window failed!");
		Finalize();
		return;
	}

	rc = screen_set_window_property_iv(mScreenWindow, SCREEN_PROPERTY_FORMAT, &format);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT) failed!");
		Finalize();
		return;
	}

	rc = screen_set_window_property_iv(mScreenWindow, SCREEN_PROPERTY_USAGE, &usage);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed!");
		Finalize();
		return;
	}

	rc = screen_get_window_property_pv(mScreenWindow, SCREEN_PROPERTY_DISPLAY, (void **)&mScreenDisplay);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_get_window_property_pv failed!");
		Finalize();
		return;
	}

	int screen_resolution[2];

	rc = screen_get_display_property_iv(mScreenDisplay, SCREEN_PROPERTY_SIZE, screen_resolution);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_get_display_property_iv failed!");
		Finalize();
		return;
	}

	int angle = atoi(getenv("ORIENTATION"));

	screen_display_mode_t screen_mode;
	rc = screen_get_display_property_pv(mScreenDisplay, SCREEN_PROPERTY_MODE, (void**)&screen_mode);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_get_display_property_pv failed!");
		Finalize();
		return;
	}

	int size[2];
	rc = screen_get_window_property_iv(mScreenWindow, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_get_window_property_iv failed!");
		Finalize();
		return;
	}

	int buffer_size[2] = {size[0], size[1]};

	if ((angle == 0) || (angle == 180))
	{
		if (((screen_mode.width > screen_mode.height) && (size[0] < size[1])) ||
			((screen_mode.width < screen_mode.height) && (size[0] > size[1])))
		{
				buffer_size[1] = size[0];
				buffer_size[0] = size[1];
		}
	}
	else if ((angle == 90) || (angle == 270))
	{
		if (((screen_mode.width > screen_mode.height) && (size[0] > size[1])) ||
			((screen_mode.width < screen_mode.height && size[0] < size[1])))
		{
				buffer_size[1] = size[0];
				buffer_size[0] = size[1];
		}
	}
	else
	{
		 NBGMDeviceQNX::DebugLog("CreateGL: Navigator returned an unexpected orientation angle!");
		 Finalize();
		 return;
	}

	rc = screen_set_window_property_iv(mScreenWindow, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_set_window_property_iv failed!");
		Finalize();
		return;
	}

	rc = screen_set_window_property_iv(mScreenWindow, SCREEN_PROPERTY_ROTATION, &angle);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_set_window_property_iv failed!");
		Finalize();
		return;
	}

	rc = screen_create_window_buffers(mScreenWindow, mBuffers);
	if (rc)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: screen_create_window_buffers failed!");
		Finalize();
		return;
	}

	mSurface = eglCreateWindowSurface(mDisplay, mConfig, mScreenWindow, NULL);
	if (mSurface == EGL_NO_SURFACE)
	{
		NBGMDeviceQNX::DebugLog("CreateGL: eglCreateWindowSurface failed!");
		Finalize();
		return;
	}

	//----------
	rc = eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
	if (rc != EGL_TRUE)
	{
		NBGMDeviceQNX::DebugLog("InitGLContext: eglMakeCurrent failed!");
		Finalize();
		return;
	}

	rc = eglSwapInterval(mDisplay, interval);
	if (rc != EGL_TRUE)
	{
		NBGMDeviceQNX::DebugLog("InitGLContext: eglSwapInterval failed!");
		Finalize();
		return;
	}
	//----------

	mInitialized = 1;
}

void NBGM_RenderContextGLESBB10::Finalize()
{
    //Typical EGL cleanup
	if (mDisplay != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (mSurface != EGL_NO_SURFACE)
		{
			eglDestroySurface(mDisplay, mSurface);
			mSurface = EGL_NO_SURFACE;
		}
		if (mContext != EGL_NO_CONTEXT)
		{
			eglDestroyContext(mDisplay, mContext);
			mContext = EGL_NO_CONTEXT;
		}
		if (mScreenWindow != NULL)
		{
			screen_destroy_window(mScreenWindow);
			mScreenWindow = NULL;
		}
		eglTerminate(mDisplay);
		mDisplay = EGL_NO_DISPLAY;
	}
	eglReleaseThread();
}


bool NBGMDeviceQNX::InitContext(NBGM_MapViewConfig& c)
{
	if(mRenderSystemType == NBGM_RS_GLES || mRenderSystemType == NBGM_RS_GLES20)
	{
		 NBGM_RenderContextGLESBB10 *context = NBRE_NEW NBGM_RenderContextGLESBB10(mScreenContext);
		 nsl_assert(context);
		 c.renderContext = shared_ptr<NBGM_RenderContextGLESBB10>(context);
		 c.renderContext->Initialize(mRenderSystemType);
		 if(context->IsInited() && context->GetSize(mWidth, mHeight))
		 {
			 return true;
		 }
	}
	return false;
}

NBGM_RenderSystemType NBGMDeviceQNX::GetRenderSystem(bool opengl)
{
    return opengl?NBGM_RS_GLES20:NBGM_RS_GLES20;
}


NBGMDeviceQNX::NBGMDeviceQNX(void):mScreenContext(NULL),mGestureHandler(NULL)
{
    int argc = 1;
    char* argv[1] = {"NBGMDevice"};
    static Application app(argc, argv);
}

NBGMDeviceQNX::~NBGMDeviceQNX(void)
{
}

bool NBGMDeviceQNX::CreateNativeWindow()
{
    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&mScreenContext, 0);
    return true;
}

void NBGMDeviceQNX::DestroyNativeWindow()
{
    screen_destroy_context(mScreenContext);
}

void handleScreenEvent(bps_event_t *event)
{
    screen_event_t screen_event = screen_event_get_event(event);

    int screen_val;
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
    case SCREEN_EVENT_MTOUCH_MOVE:
    case SCREEN_EVENT_MTOUCH_RELEASE:
        break;
    }
}

void NBGMDeviceQNX::Run()
{
   int exit_application = 0;

   //Initialize BPS library
   bps_initialize();

   //Signal BPS library that navigator and screen events will be requested
   if (BPS_SUCCESS != screen_request_events(mScreenContext))
   {
	   DebugLog("Run: screen_request_events failed!");
	   return;
   }

   if (BPS_SUCCESS != navigator_request_events(0)) {
	   DebugLog("Run: navigator_request_events failed!");
	   return;
   }

   //Signal BPS library that navigator orientation is not to be locked
   if (BPS_SUCCESS != navigator_rotation_lock(false))
   {
	   DebugLog("Run: navigator_rotation_lock failed!");
	   return;
   }

   mGestureHandler = NBRE_NEW NBGM_GestureHandler(*mMapView);

   while (!exit_application)
   {
       //Request and process all available BPS events
       bps_event_t *event = NULL;

       for(;;)
       {
           if (BPS_SUCCESS != bps_get_event(&event, 0))
           {
               break;
           }

           if (event) 
           {
               int domain = bps_event_get_domain(event);
               if (domain == screen_get_domain()) 
               {
                   screen_event_t screen_event = screen_event_get_event(event);
                   mGestureHandler->OnScreenEvent(screen_event);
                   handleScreenEvent(event);

               } 
               else if ((domain == navigator_get_domain()) && (NAVIGATOR_EXIT == bps_event_get_code(event)))
               {
                       exit_application = 1;
               }
           }
           else
           {
               break;
           }
       }
       
       if (mInited && mContinuousDraw)
       {
			Invalidate();
			sleep(0.01);
       }
   }
    //Stop requesting events from libscreen
    screen_stop_events(mScreenContext);
    //Shut down BPS library for this process
    bps_shutdown();

    NBRE_DELETE mGestureHandler;
    mGestureHandler = NULL;
}

bool SaveBitmapToFile(int width, int height, int bits, uint8 *graph, const char *file_name);

void NBGMDeviceQNX::ScreenShot()
{
   if(!mNeedScreenShot)
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
		DebugLog("ScreenShotOpenGL::mImageBuffer is null, no memory!");
		return;
	}

	uint32 imagedatasize = mWidth*mHeight*4;
	if(imagedatasize > MAX_BUFFER_SIZE)
	{
		DebugLog("ScreenShotOpenGL::imagedatasize > MAX_BUFFER_SIZE !");
		mNeedScreenShot = false;
		return;
	}

	GLubyte *bmpBuffer = (GLubyte*)mImageBuffer;
	mMapView->SnapScreenshot(bmpBuffer, imagedatasize);

    bool ret = SaveBitmapToFile(mWidth, mHeight, 8, bmpBuffer, file.c_str());
	if(!ret)
	{
		DebugLog("ScreenShotOpenGL::SaveBitmapToFile failed !");
	}
	mNeedScreenShot = false;
}

bool SaveBitmapToFile(int width, int height, int bits, uint8 *graph, const char *file_name)
{
	/* initialize stuff */
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		NBGMDeviceQNX::DebugLog("png_create_write_struct failed");
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		NBGMDeviceQNX::DebugLog("png_create_info_struct failed");
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		NBGMDeviceQNX::DebugLog("SaveBitmapToFile:: Error during init_io");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	FILE *fp = fopen(file_name, "wb");
	if (!fp)
	{
		NBGMDeviceQNX::DebugLog("SaveBitmapToFile:: PNG File could not be opened for writing");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		NBGMDeviceQNX::DebugLog("SaveBitmapToFile:: Error during writing header");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_set_IHDR(png_ptr, info_ptr, width, height, bits, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		NBGMDeviceQNX::DebugLog("SaveBitmapToFile:: Error during writing bytes");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	int i = 0;
    int temp = 4*width;
	png_bytep *row_pointers = (png_bytep*)nsl_malloc(height*sizeof(png_bytep));
	if(row_pointers == NULL)
	{
		NBGMDeviceQNX::DebugLog("SaveBitmapToFile:: Error during writing bytes");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}
	for(i = 0; i < height; i++)
	{
		row_pointers[i] = graph+ i*temp;
	}

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("SaveBitmapToFile:: Error during end of write");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}
	png_write_end(png_ptr, NULL);

	nsl_free(row_pointers);
	fclose(fp);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return true;
}
