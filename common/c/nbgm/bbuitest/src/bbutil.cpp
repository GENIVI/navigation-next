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
 * Copyright (c) 2011-2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bbutil.h"
#include "stdlib.h"
#include <QThread>

static void
bbutil_egl_perror(const char *msg) {
    static const char *errmsg[] = {
        "function succeeded",
        "EGL is not initialized, or could not be initialized, for the specified display",
        "cannot access a requested resource",
        "failed to allocate resources for the requested operation",
        "an unrecognized attribute or attribute value was passed in an attribute list",
        "an EGLConfig argument does not name a valid EGLConfig",
        "an EGLContext argument does not name a valid EGLContext",
        "the current surface of the calling thread is no longer valid",
        "an EGLDisplay argument does not name a valid EGLDisplay",
        "arguments are inconsistent",
        "an EGLNativePixmapType argument does not refer to a valid native pixmap",
        "an EGLNativeWindowType argument does not refer to a valid native window",
        "one or more argument values are invalid",
        "an EGLSurface argument does not name a valid surface configured for rendering",
        "a power management event has occurred",
        "unknown error code"
    };

    int message_index = eglGetError() - EGL_SUCCESS;

    if (message_index < 0 || message_index > 14)
        message_index = 15;

    fprintf(stderr, "%s: %s\n", msg, errmsg[message_index]);
}

NBGM_Egl::NBGM_Egl(screen_context_t ctx, const QString &group, int x, int y, int w, int h):
		screen_ctx(ctx), egl_disp(NULL), egl_surf(NULL), egl_ctx(NULL), screen_win(NULL), screen_disp(NULL), nbuffers(2), mGroupId(group), mX(x), mY(y), mWidth(w), mHeight(h), initialized(0)
{
	memset(&egl_conf, 0, sizeof(egl_conf));

}

NBGM_Egl::~NBGM_Egl()
{

}

int NBGM_Egl::init(int screenUsage)
{
    QByteArray groupArr = mGroupId.toAscii();

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

    if(screenUsage == SCREEN_USAGE_OPENGL_ES1)
    {
        usage = SCREEN_USAGE_OPENGL_ES1 | SCREEN_USAGE_ROTATION;
        attrib_list[9] = EGL_OPENGL_ES_BIT;
    }
    else if(screenUsage == SCREEN_USAGE_OPENGL_ES2)
    {
        usage = SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_ROTATION;
        attrib_list[9] = EGL_OPENGL_ES2_BIT;
    }
    else
    {
    	return EXIT_FAILURE;
    }


    egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_disp == EGL_NO_DISPLAY) {
        bbutil_egl_perror("eglGetDisplay");
        terminate();
        return EXIT_FAILURE;
    }

    rc = eglInitialize(egl_disp, NULL, NULL);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglInitialize");
        terminate();
        return EXIT_FAILURE;
    }

    rc = eglBindAPI(EGL_OPENGL_ES_API);

    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglBindApi");
        terminate();
        return EXIT_FAILURE;
    }

    if(!eglChooseConfig(egl_disp, attrib_list, &egl_conf, 1, &num_configs)) {
        terminate();
        return EXIT_FAILURE;
    }

    if(screenUsage == SCREEN_USAGE_OPENGL_ES1)
    {
    	egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, NULL);
    }
    else
    {
	   EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	   egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, attributes);
    }

    if (egl_ctx == EGL_NO_CONTEXT) {
        bbutil_egl_perror("eglCreateContext");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window_type(&screen_win, screen_ctx, SCREEN_CHILD_WINDOW);
    if (rc) {
        perror("screen_create_window");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_join_window_group(screen_win, groupArr.constData()) ;
    if (rc) {
        perror("screen_create_window_group");
        terminate();
        return EXIT_FAILURE;
    }

    int z = -1;
	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ZORDER, &z);
    if (rc) {
        perror("screen_set_window_property_iv");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
    if (rc) {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    if (rc) {
        perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp);
    if (rc) {
        perror("screen_get_window_property_pv");
        terminate();
        return EXIT_FAILURE;
    }

    // Set the window position on screen.
    int pos[2] = { mX, mY };
    if (screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_POSITION, pos) != 0) {
        return false;
    }

    int size[2] = { mWidth, mHeight };

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) {
        perror("screen_set_window_property_iv");
        terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window_buffers(screen_win, nbuffers);
    if (rc) {
        perror("screen_create_window_buffers");
        terminate();
        return EXIT_FAILURE;
    }

    egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
    if (egl_surf == EGL_NO_SURFACE) {
        bbutil_egl_perror("eglCreateWindowSurface");
        terminate();
        return EXIT_FAILURE;
    }

    rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglMakeCurrent");
        terminate();
        return EXIT_FAILURE;
    }

    rc = eglSwapInterval(egl_disp, interval);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglSwapInterval");
        terminate();
        return EXIT_FAILURE;
    }

    initialized = 1;

    return EXIT_SUCCESS;
}

void NBGM_Egl::terminate()
{
    //Typical EGL cleanup
    if (egl_disp != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_surf != EGL_NO_SURFACE) {
            eglDestroySurface(egl_disp, egl_surf);
            egl_surf = EGL_NO_SURFACE;
        }
        if (egl_ctx != EGL_NO_CONTEXT) {
            eglDestroyContext(egl_disp, egl_ctx);
            egl_ctx = EGL_NO_CONTEXT;
        }
        if (screen_win != NULL) {
            screen_leave_window_group(screen_win);
            screen_destroy_window_buffers(screen_win);
            screen_destroy_window(screen_win);
            screen_win = NULL;
        }
        eglTerminate(egl_disp);
        egl_disp = EGL_NO_DISPLAY;
    }
    eglReleaseThread();

    initialized = 0;
}

int NBGM_Egl::swap()
{
    int rc = eglSwapBuffers(egl_disp, egl_surf);
    if (rc != EGL_TRUE) {
        bbutil_egl_perror("eglSwapBuffers");
    }
    return rc;
}

int NBGM_Egl::make_current()
{
	int rc  = EGL_FALSE;
	if (egl_disp != EGL_NO_DISPLAY) {
	    rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
	    if (rc != EGL_TRUE) {
	        bbutil_egl_perror("eglMakeCurrent");
	    }
	}
	return rc;
}

int NBGM_Egl::make_current_null()
{
    int rc  = EGL_FALSE;
    if (egl_disp != EGL_NO_DISPLAY) {
        rc = eglMakeCurrent(egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
        }
    }
    return rc;
}

int NBGM_Egl::calculate_dpi()
{
	return 0;
}

int NBGM_Egl::rotate_screen_surface(int angle)
{
    int rc, rotation, skip = 1, temp;;
    EGLint interval = 1;
    int size[2];

    if ((angle != 0) && (angle != 90) && (angle != 180) && (angle != 270)) {
        fprintf(stderr, "Invalid angle\n");
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &rotation);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    switch (angle - rotation) {
        case -270:
        case -90:
        case 90:
        case 270:
            temp = size[0];
            size[0] = size[1];
            size[1] = temp;
            skip = 0;
            break;
    }

    if (!skip) {
        rc = eglMakeCurrent(egl_disp, NULL, NULL, NULL);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = eglDestroySurface(egl_disp, egl_surf);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_SIZE, size);
        if (rc) {
            perror("screen_set_window_property_iv");
            return EXIT_FAILURE;
        }

        rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
        if (rc) {
            perror("screen_set_window_property_iv");
            return EXIT_FAILURE;
        }
        egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
        if (egl_surf == EGL_NO_SURFACE) {
            bbutil_egl_perror("eglCreateWindowSurface");
            return EXIT_FAILURE;
        }

        rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = eglSwapInterval(egl_disp, interval);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglSwapInterval");
            return EXIT_FAILURE;
        }
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &angle);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int NBGM_Egl::get_window_size(int* width, int* height)
{
	int result = 0;
	EGLint surface_width, surface_height;

	if(egl_disp != NULL && egl_surf != NULL)
	{
	    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
	    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);
	    result = 1;
	}

	*width = surface_width;
	*height = surface_height;
    return result;
}
