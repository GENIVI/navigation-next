#!/bin/sh
export SVGA_VGPU10=0
export QT_XCB_GL_INTEGRATION=xcb_egl
export QT_IM_MODULE=qtvirtualkeyboard
export PATH=/opt/Qt5.8.0/bin:$PATH

export LD_LIBRARY_PATH=/opt/locationstudio/lib/png:/opt/locationstudio/libs/ncdb:$LD_LIBRARY_PATH
export EGL_DRIVER=egl_dri2
./MainViewLoader
