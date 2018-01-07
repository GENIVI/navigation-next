#!/bin/sh

export LD_LIBRARY_PATH=./:/usr/local/lib:/usr/local/libpng/lib:../../library/lib:$LD_LIBRARY_PATH
export EGL_DRIVER=egl_dri2
./navigationuikit_sample
